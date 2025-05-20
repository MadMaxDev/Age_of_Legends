#include "module.h"
#include "builder.h"
#include "scriptengine.h"
#include "bytecode.h"
#include "texts.h"
#include "restore.h"
#include "scriptcode.h"

psCModule::psCModule(const psCHAR* name, int id, psCScriptEngine* engine)
{
	m_ModuleId		  = id;	
	m_Name			  = name;
	m_pEngine		  = engine;
	m_pCode			  = NULL;
	m_pStringFunction = NULL;
	m_pInitFunction   = NULL;	
	m_pExitFunction	  = NULL;
	m_GlobalMem		  = NULL;
	m_ConstantMem	  = NULL;
	m_IsGlobalVarInitialized = false;
	m_ContexRefCount  = 0;
	m_ConstantMemSize = 0;
	m_ConstantMemCapacity = 0;
	m_GlobalMemSize	  = 0;
	m_IsBuildWithoutErrors = false;
}

psCModule::~psCModule(void)
{
}

void psCModule::Destroy()
{
	Reset();

	if ( m_pCode )
	{
		delete m_pCode;
		m_pCode = NULL;
	}
}

void psCModule::Reset()
{
	CallExit();

	if ( m_pInitFunction )
	{
		delete m_pInitFunction;
		m_pInitFunction = NULL;
	}
	if ( m_pExitFunction )
	{
		delete m_pExitFunction;
		m_pExitFunction = NULL;
	}
	if ( m_pStringFunction )
	{
		delete m_pStringFunction;
		m_pStringFunction = NULL;
	}
	// 删除所有的脚本函数 
	size_t n;
	for( n = 0; n < m_ScriptFunctions.size(); n++ )
		delete m_ScriptFunctions[n];
	m_ScriptFunctions.clear();

	// 删除所有的全局变量
	for( n = 0; n < m_ScriptGlobals.size(); n++ )
		delete m_ScriptGlobals[n];
	m_ScriptGlobals.clear();

	// 删除所有的类
	for (n = 0; n < m_ObjectTypes.size(); ++n)
		delete m_ObjectTypes[n];
	m_ObjectTypes.clear();

	// 删除所有的全局操作符重载
	m_GlobalOperators.clear();

	m_IsGlobalVarInitialized = false;

	// 释放全局变量空间,和常量池
	if (m_GlobalMem)
	{
		free(m_GlobalMem);
		m_GlobalMem = NULL;
	}
	if (m_ConstantMem)
	{
		free(m_ConstantMem);
		m_ConstantMem = NULL;
	}
	m_ConstantMemSize = 0;
	m_ConstantMemCapacity = 0;
	m_GlobalMemSize	  = 0;

	// 删除所有的常量
	m_IntConstants.clear();
	m_FloatConstants.clear();
	m_DoubleConstants.clear();
	m_StrAConstants.clear();
	m_StrWConstants.clear();
}

int psCModule::Build( psITextStream* pTextStream, psIOutputStream* pOutStream )
{
	// 重置脚本模块
	Reset();

	// 如果脚本源代码为空则返回
	if ( m_pCode == NULL )
	{
		return psSUCCESS;
	}
	m_pCode->SetFileName( m_FileName.AddressOf() );

	psCBuilder builder( m_pEngine, this );
	builder.SetOutputStream( pOutStream );
	builder.SetTextStream( pTextStream );

	// 编译脚本
	int r = builder.Build();

	// 删除脚本源代码
	delete m_pCode;
	m_pCode = NULL;

	if( r < 0 )
	{
		// 如果编译失败,重设一下脚本模块
		Reset();
		return r;
	}
	m_IsBuildWithoutErrors = true;

	// Call Init
	if ( !CallInit() )
		return psCANT_INIT_MODULE;

	return r;
}

int psCModule::SetCode( const psCHAR *code, int codeLength)
{
	if( m_pCode == NULL )
	{
		m_pCode = new psCScriptCode;
	}
	m_pCode->SetCode( m_Name.AddressOf(), code, codeLength );
	return psSUCCESS;
}

const psCHAR* psCModule::GetSourceCode() const
{
	if (m_pCode)
		return m_pCode->m_Code.AddressOf();
	else
		return PS_T("");
}


unsigned int psCModule::GetGlobalVarCount() const
{
	return (unsigned int)m_ScriptGlobals.size();
}

const psIProperty* psCModule::GetGlobalVar(int index) const
{
	assert(index >= 0 && index < int(m_ScriptGlobals.size()));
	return m_ScriptGlobals[index];
}

int psCModule::GetGlobalVarIDByDecl(const psCHAR *decl)
{
	if( m_IsBuildWithoutErrors == false )
		return psERROR;

	psCBuilder bld(m_pEngine, this);

	// 分析变量声明
	psSProperty gvar;
	int r = bld.ParseVariableDeclaration(decl, &gvar);
	if (r < 0 )
	{
		return psERROR;
	}

	// 在脚本全局变量中查找
	int id = -1;
	for( int n = 0; n < (int)m_ScriptGlobals.size(); ++n )
	{
		if( gvar.name == m_ScriptGlobals[n]->name && 
			gvar.type == m_ScriptGlobals[n]->type )
		{
			id = n;
			break;
		}
	}

	if( id == -1 ) return psNO_GLOBAL_VAR;

	return m_ModuleId | id;
}

int psCModule::GetGlobalVarIDByName(const psCHAR *name)
{
	if( m_IsBuildWithoutErrors == false )
		return psERROR;

	// 在脚本全局变量中查找
	int id = -1;
	for( int n = 0; n < (int)m_ScriptGlobals.size(); n++ )
	{
		if( m_ScriptGlobals[n]->name == name )
		{
			id = n;
			break;
		}
	}

	if( id == -1 ) return psNO_GLOBAL_VAR;

	return m_ModuleId | id;
}

int psCModule::GetFunctionIDByDecl(const psCHAR* decl) const
{
	if( m_IsBuildWithoutErrors == false )
		return psERROR;

	psCBuilder builder( m_pEngine, (psCModule*)this );

	psCScriptFunction func;
	bool bArgVariational;

	psETokenType tokenType = ttUnrecognizedToken;

	int r = builder.ParseFunctionDeclaration(decl, &func, &bArgVariational);
	if( r < 0 )
	{
		r = builder.ParseOpOverloadDeclaration( decl, &func, tokenType );
		if ( r < 0 )
			return psINVALID_DECLARATION;
	}else
	{	
		// 脚本本身不支持变参函数
		if (bArgVariational)
			return psINVALID_DECLARATION;
	}

	// 在所有的脚本函数中查找
	if ( tokenType == ttUnrecognizedToken )
	{	
		int id = -1;
		for( int n = 0; n < (int)m_ScriptFunctions.size(); ++n )
		{
			if( func.m_Name == m_ScriptFunctions[n]->m_Name && 
				func.m_ReturnType == m_ScriptFunctions[n]->m_ReturnType &&
				func.m_ParamTypes.size() == m_ScriptFunctions[n]->m_ParamTypes.size() )
			{
				bool match = true;
				// 匹配函数参数
				for( size_t p = 0; p < func.m_ParamTypes.size(); ++p )
				{
					if( func.m_ParamTypes[p] != m_ScriptFunctions[n]->m_ParamTypes[p] )
					{
						match = false;
						break;
					}
				}

				if( match )
				{
					if( id == -1 )
						id = n;
					else
						return psMULTIPLE_FUNCTIONS;
				}
			}
		}
		if( id == -1 )
			return psNO_FUNCTION;
		return id;
	}else
	{
		for ( int n = 0; n < (int)m_GlobalOperators.size(); n += 2 )
		{
			if ( m_GlobalOperators[n*2] == tokenType )
			{
				int id = m_GlobalOperators[n*2 + 1];
				psCScriptFunction* matchFunc = m_ScriptFunctions[id];

				if ( func.m_ReturnType == matchFunc->m_ReturnType &&
					 func.m_ParamTypes.size() == matchFunc->m_ParamTypes.size() )
				{
					bool match = true;
					// 匹配函数参数
					for( size_t p = 0; p < func.m_ParamTypes.size(); ++p )
					{
						if( func.m_ParamTypes[p] != matchFunc->m_ParamTypes[p] )
						{
							match = false;
							break;
						}
					}
					if( match )
					{
						return id;
					}
				}
			}
		}
		return psNO_FUNCTION;
	}
}

int psCModule::GetFunctionIDByName(const psCHAR* name) const
{
	if( m_IsBuildWithoutErrors == false )
		return psERROR;

	// 在所有的脚本函数中查找
	int id = -1;
	for( int n = 0; n < (int)m_ScriptFunctions.size(); n++ )
	{
		if( m_ScriptFunctions[n]->m_Name == name )
		{
			if( id == -1 )
				id = n;
			else
				return psMULTIPLE_FUNCTIONS;
		}
	}

	if( id == -1 ) return psNO_FUNCTION;

	return id;
}


const psIScriptFunction* psCModule::GetFunctionByID(int id) const
{
	if ( id < 0 || id > (int)m_ScriptFunctions.size() )
		return NULL;

	return m_ScriptFunctions[id];
}

psCScriptFunction* psCModule::GetScriptFunction(unsigned int id) const
{
	assert ( id < (unsigned int)m_ScriptFunctions.size() );
	return m_ScriptFunctions[id];
}

int psCModule::AllocGlobalMemory(const psCDataType& type)
{
	int size  = type.GetTotalSizeOnStackDWords() * sizeof(psDWORD);
	int index = m_GlobalMemSize;
	m_GlobalMemSize += size;
	return index;
}

void psCModule::DeallocGlobalMemory(const psCDataType& type)
{	
	int size = type.GetTotalSizeOnStackDWords() * sizeof(psDWORD);
	m_GlobalMemSize -= size;
	if ( m_GlobalMemSize < 0 )
		m_GlobalMemSize = 0;
}

int psCModule::AddConstMemSize( int s )
{
	int old_size	   = m_ConstantMemSize;
	m_ConstantMemSize += s;

	if ( m_ConstantMemSize > m_ConstantMemCapacity )
	{
		do	  { m_ConstantMemCapacity += 256; }
		while ( m_ConstantMemCapacity < m_ConstantMemSize );

		m_ConstantMem = (psBYTE*)realloc( m_ConstantMem, m_ConstantMemCapacity );
	}
	return old_size;
}

int psCModule::AddConstantInt(int value)
{
	std::map< int, int >::iterator it = m_IntConstants.find( value );
	if( it != m_IntConstants.end() )
		return it->second;

	int offset = AddConstMemSize(sizeof(int));
		
	*(int*)(m_ConstantMem+offset) = value;

	m_IntConstants[value] = offset;
	return offset;

}

int psCModule::AddConstantFloat(float value)
{
	std::map< float, int >::iterator it = m_FloatConstants.find( value );
	if( it != m_FloatConstants.end() )
		return it->second;

	int offset = AddConstMemSize(sizeof(float));
		
	*(float*)(m_ConstantMem+offset) = value;

	m_FloatConstants[value] = offset;
	return offset;
}

int psCModule::AddConstantDouble( double value )
{
	std::map< double, int >::iterator it = m_DoubleConstants.find( value );
	if( it != m_DoubleConstants.end() )
		return it->second;

	int offset = AddConstMemSize(sizeof(double));

	*(double*)(m_ConstantMem+offset) = value;

	m_DoubleConstants[value] = offset;
	return offset;
}

int psCModule::AddConstantStrA( const char* pStr )
{
	psCStringA str(pStr);

	std::map< psCStringA, int >::iterator it = m_StrAConstants.find(str);
	if ( it != m_StrAConstants.end() )
	{
		return it->second;
	}

	int size       = ((int)str.GetLength() + 1)*sizeof(char);
	int offset	   = AddConstMemSize( size );

	memcpy( m_ConstantMem+offset, pStr, size);

	// Insert string
	m_StrAConstants[str] = offset;

	return offset;
}

int psCModule::AddConstantStrW( const wchar_t* pStr )
{
	psCStringW str(pStr);

	std::map< psCStringW, int >::iterator it = m_StrWConstants.find(str);
	if ( it != m_StrWConstants.end() )
	{
		return it->second;
	}

	int size		   = ((int)str.GetLength() + 1)*sizeof(wchar_t);
	int offset		   = AddConstMemSize(size);

	memcpy( m_ConstantMem+offset, pStr, size);

	// Insert string
	m_StrWConstants[str] = offset;

	return offset;
}

psCScriptFunction* psCModule::AddScriptFunction(int id, const psCHAR* name, psCDataType returnType, std::vector<psCDataType>& paramTypes)
{
	assert(id >= 0);

	// Store the function information
	psCScriptFunction *func = new psCScriptFunction();

	func->m_Name = name;
	func->m_ID   = id;
	func->m_ReturnType = returnType;
	func->m_ParamTypes.resize(paramTypes.size());
	for( size_t n = 0; n < paramTypes.size(); n++ )
	{
		func->m_ParamTypes[n] = paramTypes[n];
	}
	m_ScriptFunctions.push_back(func);

	return func;
}

psCScriptFunction *psCModule::GetSpecialFunction(int funcID)
{
	if( funcID == psFUNC_INIT )
	{
		if ( m_pInitFunction == NULL )
		{
			m_pInitFunction = new psCScriptFunction();
			m_pInitFunction->m_ID   = psFUNC_INIT;
			m_pInitFunction->m_Name = PS_T("&Init");
			m_pInitFunction->m_ReturnType = psCDataType(ttVoid, false, false ); 
		}
		return m_pInitFunction;
	}else if ( funcID == psFUNC_EXIT )
	{
		if ( m_pExitFunction == NULL )
		{
			m_pExitFunction = new psCScriptFunction();
			m_pExitFunction->m_ID   = psFUNC_EXIT;
			m_pExitFunction->m_Name = PS_T("&Exit");
			m_pExitFunction->m_ReturnType = psCDataType(ttVoid, false, false ); 
		}
		return m_pExitFunction;
	}else if ( funcID == psFUNC_STRING )
	{
		if ( m_pStringFunction == NULL )
		{
			m_pStringFunction = new psCScriptFunction();
			m_pStringFunction->m_ID = psFUNC_STRING;
			m_pStringFunction->m_Name = TXT_EXECUTESTRING;
			m_pStringFunction->m_ReturnType = psCDataType(ttVoid, false, false );
		}
		return m_pStringFunction;
	}else
	{
		return NULL;
	}
}
bool psCModule::CallInit()
{
	if( m_IsGlobalVarInitialized )
		return true;

	// 初始化全局内存为0
	if ( m_GlobalMemSize > 0 )
	{
		m_GlobalMem = (psBYTE*)malloc(m_GlobalMemSize);
		memset(&m_GlobalMem[0], 0, m_GlobalMemSize);
	}

	// 如果有初始化函数就动行初始化函数
	if ( m_pInitFunction == NULL || m_pInitFunction->m_BinaryCode == NULL )
	{
		m_IsGlobalVarInitialized = true;
		return true;
	}
	psCContext ctx(m_pEngine, false );

	// TODO: Add error handling
	if ( ctx.PrepareSpecial(m_ModuleId, psFUNC_INIT) != psSUCCESS )
		return false;

	if ( ctx.Execute(0) != psEXECUTION_FINISHED )
		return false;

	m_IsGlobalVarInitialized = true;
	return true;
}

void psCModule::CallExit()
{
	if( !m_IsGlobalVarInitialized ) return;

	// 如果有退出函数就动行退出函数
	if ( m_pExitFunction && m_pExitFunction->m_BinaryCode )
	{
		psCContext ctx(m_pEngine, false );

		// TODO: Add error handling
		ctx.PrepareSpecial(m_ModuleId, psFUNC_EXIT);

		ctx.Execute(0);
	}

	m_IsGlobalVarInitialized = false;
}

psCObjectType* psCModule::FindObjectType(const psCHAR* name) const
{
	for (size_t i = 0; i < m_ObjectTypes.size(); ++i)
		if (m_ObjectTypes[i]->m_Name == name)
			return m_ObjectTypes[i];
	return 0;
}

int psCModule::GetObjectTypeIdx(const psCObjectType* objType)
{
	for (int i = 0; i < (int)m_ObjectTypes.size(); ++i)
		if (m_ObjectTypes[i] == objType)
			return i;

	return -1;
}

void psCModule::FindGlobalOperators(int op, std::vector<int>& funcs)
{
	for (size_t i = 0; i < m_GlobalOperators.size(); i += 2)
		if (m_GlobalOperators[i] == op)
			funcs.push_back(m_GlobalOperators[i+1]);
}

void psCModule::AddGlobalOperator(int op, int func)
{
	m_GlobalOperators.push_back(op);
	m_GlobalOperators.push_back(func);
}

int psCModule::SaveByteCode(psIBinaryStream* out) const
{
	psCRestore stream(out, m_pEngine);

	return stream.SaveModule((psCModule*)this);
}

int psCModule::LoadByteCode(psIBinaryStream* in)
{
	psCRestore stream(in, m_pEngine);

	// 重设脚本模块
	Reset();

	return stream.RestoreModule(this);
}

bool psCModule::ToggleBreakPoint(int lineno , bool& isBreakPoint )
{
	isBreakPoint = false;
	for (size_t i = 0; i < m_ScriptFunctions.size(); ++i)
	{
		psCScriptFunction* func = m_ScriptFunctions[i];
		assert(func);
		if ( !func->m_IsIncluded )
		{
			if ( func->ToggleBreakPoint( lineno, isBreakPoint ) )
				return true;
		}
	}
	return false;
}

bool psCModule::InsertBreakPoint(int lineno)
{
	for (size_t i = 0; i < m_ScriptFunctions.size(); ++i)
	{
		psCScriptFunction* func = m_ScriptFunctions[i];
		assert(func);
		if ( !func->m_IsIncluded )
		{
			if ( func->InsertBreakPoint(lineno) )
				return true;
		}
	}
	return false;  
}

bool psCModule::RemoveBreakPoint(int lineno)
{
	for (size_t i = 0; i < m_ScriptFunctions.size(); ++i)
	{
		psCScriptFunction* func = m_ScriptFunctions[i];
		assert(func);
		if ( func->RemoveBreakPoint(lineno) )
			return true;
	}
	return false;
}

void psCModule::RemoveAllBreakPoint()
{
	for (size_t i = 0; i < m_ScriptFunctions.size(); ++i)
	{
		psCScriptFunction* func = m_ScriptFunctions[i];
		assert(func);
		func->RemoveAllBreakPoints();
	}
}

psIBuffer* psCModule::GetAllBreakPoint( int* outNumBreakPoints )
{
	int numBreakPoints = 0;
	{
		for (size_t i = 0; i < m_ScriptFunctions.size(); ++i)
		{
			psCScriptFunction* func = m_ScriptFunctions[i];
			assert(func);
			numBreakPoints += (int)( func->m_BreakPoints.size() );
		}
	}
	if ( outNumBreakPoints )
		*outNumBreakPoints = numBreakPoints;

	if ( numBreakPoints == 0 )
	{
		return NULL;
	}

	psBuffer* pBuf = new psBuffer;
	pBuf->Alloc( numBreakPoints * sizeof(int) );
	{
		int* lineNumber = (int*)(pBuf->GetBufferPointer());
		for (size_t i = 0; i < m_ScriptFunctions.size(); ++i)
		{
			psCScriptFunction* func = m_ScriptFunctions[i];
			assert(func);
			for (size_t bp = 0; bp < func->m_BreakPoints.size(); ++bp )
			{
				*lineNumber = func->GetLineNumber( func->m_BreakPoints[bp].pos );
				++lineNumber;
			}
		}
	}
	return pBuf;
}


