#include "builder.h"
#include "scriptengine.h"
#include "scriptnode.h"
#include "scriptcode.h"
#include "scriptfunction.h"
#include "module.h"
#include "compiler.h"
#include "parser.h"
#include "objecttype.h"
#include "texts.h"

//---------------------------------------------------------
// simple string
//---------------------------------------------------------
template< class xchar, int n >
class psSimpleString
{
public:
	psSimpleString( size_t length )
	{
		if ( (int)length < n - 2 )
		{
			m_pStr = m_Buf;
		}else
		{
			m_pStr = new xchar[ length + 1];
		}
	}
	~psSimpleString()
	{
		if ( m_pStr != m_Buf )
		{
			delete []m_pStr;
		}
	}
	inline xchar* c_str()  const			 { return m_pStr; }
	inline xchar& operator[](size_t _n)		 { return m_pStr[_n]; }	// ������޸�gcc�ᱨ���ģ�����n��ͻ
protected:
	xchar*  m_pStr;
	xchar   m_Buf[ n - 1 ];
};

//------------------------------------------------------------------------------------
void psProcessTransferChar( const psCHAR* str, size_t len, psCHAR* dst, size_t& real_len)
{	
	// ����ת���ַ�
	size_t final_len = 0;
	for ( size_t i = 0; i < len; ++i )
	{
		psCHAR ch0 = str[i];
		if ( ch0 == PS_T('\\') )
		{
			// ����ת���ַ�
			++i;
			if ( i == len )
				break;

			psCHAR chr = str[i];
			if ( chr == PS_T('"') )
				dst[final_len++] = PS_T('"');
			else if ( chr == PS_T('\'') )
				dst[final_len++] = PS_T('\'');
			else if ( chr == PS_T('n') )
				dst[final_len++] = PS_T('\n');
			else if ( chr == PS_T('r' ))
				dst[final_len++] = PS_T('\r');
			else if ( chr == PS_T('0' ))
				dst[final_len++] = PS_T('\0');
			else if ( chr == PS_T('\\' ))
				dst[final_len++] = PS_T('\\');
			else if ( chr == PS_T('t' ))
				dst[final_len++] = PS_T('\t');
			else
			{
				dst[final_len++] = PS_T('\\');
				--i;
			}		
		}else
		{
			dst[final_len++] = ch0;
		}
	}
	dst[final_len]  = 0;

	real_len = final_len;
}

//---------------------------------------------------------------

psCBuilder::psCBuilder(psCScriptEngine* engine, psCModule* module)
{
	m_NumErrors   = 0;
	m_NumWarnings = 0;
	m_pOut		  = NULL;
	m_pTextStream = NULL;

	m_pModule	  = module;
	m_pEngine	  = engine;
}

psCBuilder::~psCBuilder()
{
	size_t n;

	// ɾ�����еĺ�������
	for( n = 0; n < m_Functions.size(); n++ )
	{
		if( m_Functions[n] )
		{
#ifndef NODE_IN_TEMP_MEMORY
			if( m_Functions[n]->node) 
			{
				delete m_Functions[n]->node;
			}
#endif
			delete m_Functions[n];
		}
	}
	m_Functions.clear();

	// ɾ�����е�ȫ�ֱ�������
	{
		GlobalVariableMap_t::iterator ite = m_GlobalVariables.begin();
		GlobalVariableMap_t::iterator end = m_GlobalVariables.end();
		for ( ; ite != end; ++ite )
		{
			delete ite->second;
		}
		m_GlobalVariables.clear();
	}
	m_CompileGlobalVariables.clear();
}

psCScriptFunction* psCBuilder::AddScriptFunction(const psCString& name, psCDataType* pRetType, std::vector<psCDataType>* pParamTypes)
{
	assert(m_pModule);	
	static psCDataType retType(ttVoid, false, false);
	static std::vector<psCDataType> paramTypes;

	if ( pParamTypes == NULL)
	{
		pParamTypes = &paramTypes;
	}
	if ( pRetType == NULL )
	{
		pRetType = &retType;
	}

	return m_pModule->AddScriptFunction(m_pModule->GetNextFunctionId(), name.AddressOf(), *pRetType, *pParamTypes);
}

int psCBuilder::Build()
{	
	MANAGE_TEMP_MEMORY();

	m_NumErrors = 0;
	m_NumWarnings = 0;  
	m_OptimizeOptions = m_pEngine->GetOptimizeOptions();

	// �����ű�
	ParseScripts();

	// �������еĺ���
	CompileFunctions();

	if (m_NumErrors > 0)
		return psERROR;

	return psSUCCESS;
}

int psCBuilder::BuildString(const psCHAR* str)
{
	MANAGE_TEMP_MEMORY();

	m_NumErrors = 0;
	m_NumWarnings = 0;  
	m_OptimizeOptions = m_pEngine->GetOptimizeOptions();

	// Add the string to the script code
	psCScriptCode script;
	script.SetCode(TXT_EXECUTESTRING, str );
	
	// Parse the string
	psCParser parser(this);
	if( parser.ParseScript(&script) >= 0 )
	{
		// Find the function
		psCScriptNode *node = parser.GetScriptNode();
		node = node->GetFirstChild();
		if( node->GetNodeType() == snFunction )
		{
			node->DisconnectParent();

			int r = psERROR;
			// Compile the function
			psCCompiler compiler;
			if( compiler.CompileFunction( this, &script, node, NULL ) >= 0 )
			{
				psCScriptFunction *func = m_pModule->GetSpecialFunction(psFUNC_STRING);

				// ���ö����ƴ���
				func->SetCode( compiler.GetByteCode() );
				r = psSUCCESS;
			}

			// �ͷ��ڴ�
			#ifndef NODE_IN_TEMP_MEMORY
				delete node;
			#endif

			return r;
		}
		else
		{
			// TODO: An error occurred
			assert(false);
		}
	}	
	return psERROR;
}

int psCBuilder::ParseDataType(const psCHAR* name, psCDataType* result)
{
	MANAGE_TEMP_MEMORY();

	m_NumErrors = 0;
	m_NumWarnings = 0;

	psCScriptCode source;
	source.SetCode(PS_T(""), name);

	psCParser parser(this);
	int r = parser.ParseDataType(&source);
	if( r < 0 )
		return psINVALID_TYPE;

	// �õ��������ͺ�����
	psCScriptNode *dataType = parser.GetScriptNode()->GetFirstChild();

	*result = CreateDataTypeFromNode(dataType, &source);

	if( m_NumErrors > 0 )
		return psINVALID_TYPE;

	return psSUCCESS;
}

int psCBuilder::ParseFunctionDeclaration(const psCHAR *decl, psCScriptFunction *func, bool* pArgVariational)
{
	MANAGE_TEMP_MEMORY();

	m_NumErrors = 0;
	m_NumWarnings = 0;

	// �Ƿ�Ϊ��κ���
	if (pArgVariational)
	{
		*pArgVariational = false;
	}
	
	psCScriptCode source;
	source.SetCode(TXT_SYSTEM_FUNCTION, decl);

	psCParser parser(this);

	int r = parser.ParseFunctionDeclaration(&source);
	if( r < 0 )
	{
		return psINVALID_DECLARATION;
	}
   
	psCScriptNode *node = parser.GetScriptNode();

	// �õ���������
	func->m_ReturnType = CreateDataTypeFromNode(node->GetFirstChild(), &source);
	ModifyDataTypeFromNode(func->m_ReturnType, node->GetFirstChild()->GetNextNode(), &source );

	// �õ�����
	psCScriptNode *n = node->GetFirstChild()->GetNextNode()->GetNextNode();
	func->m_Name.Copy(&source.m_Code[n->GetTokenPos()], n->GetTokenLength());

	psCScriptNode* paramListNode = n->GetNextNode();
	assert(paramListNode);

	// ����Ƿ�Ϊ��κ���(�����б����TokenType���������Ƿ��εı�־)
	if (paramListNode->GetTokenType() == ttEllipsis)
	{
		if (pArgVariational)
			*pArgVariational = true;
	}

	// �õ����еĲ�������
	n = paramListNode->GetFirstChild();
	while( n )
	{
		psCDataType type = CreateDataTypeFromNode(n, &source);
		n = n->GetNextNode();

		ModifyDataTypeFromNode(type, n, &source );
		n = n->GetNextNode();

		assert( n );
		if ( n->GetNodeType() == snIdentifier )
		{
			n = n->GetNextNode();
		}
		assert( n &&  n ->GetNodeType() == snArrayDim );
		ModifyDataTypeFromNode( type, n, &source );
		n = n->GetNextNode();

		// �����������
		func->m_ParamTypes.push_back(type);
	}

	if( m_NumErrors > 0 || m_NumWarnings > 0 )
		return psINVALID_DECLARATION;

	return 0;
}

int psCBuilder::ParseOpOverloadDeclaration(const psCHAR *decl, psCScriptFunction *func, psETokenType& tokenType)
{
	MANAGE_TEMP_MEMORY();

	m_NumErrors = 0;
	m_NumWarnings = 0;

	psCScriptCode source;
	source.SetCode(TXT_SYSTEM_FUNCTION, decl);

	psCParser parser(this);

	int r = parser.ParseOpOverloadDeclaration(&source);
	if( r < 0 )
	{
		return psINVALID_DECLARATION;
	}

	psCScriptNode *node = parser.GetScriptNode();

	// �õ���������
	func->m_ReturnType = CreateDataTypeFromNode(node->GetFirstChild(), &source);
	ModifyDataTypeFromNode(func->m_ReturnType, node->GetFirstChild()->GetNextNode(), &source);

	// �õ�����
	func->m_Name  = PS_T("operator ");
	tokenType     = node->GetTokenType();
	func->m_Name += psGetTokenDefinition( (int)tokenType );

	psCScriptNode* n = node->GetFirstChild()->GetNextNode();
	psCScriptNode* paramListNode = n->GetNextNode();
	assert(paramListNode && paramListNode->GetNodeType() == snParameterList );

	// �õ����еĲ�������
	n = paramListNode->GetFirstChild();
	while( n )
	{
		psCDataType type = CreateDataTypeFromNode(n, &source);
		n = n->GetNextNode();

		ModifyDataTypeFromNode(type, n, &source );
		n = n->GetNextNode();

		assert( n );
		if ( n->GetNodeType() == snIdentifier )
		{
			n = n->GetNextNode();
		}
		assert( n &&  n ->GetNodeType() == snArrayDim );
		ModifyDataTypeFromNode( type, n, &source );
		n = n->GetNextNode();

		// �����������
		func->m_ParamTypes.push_back(type);
	}

	if( m_NumErrors > 0 || m_NumWarnings > 0 )
		return psINVALID_DECLARATION;

	return 0;
}

int psCBuilder::ParseVariableDeclaration(const psCHAR *decl, psSProperty *var)
{
	MANAGE_TEMP_MEMORY();

	m_NumErrors = 0;
	m_NumWarnings = 0;

	psCScriptCode source;
	source.SetCode(TXT_VARIABLE_DECL, decl);

	psCParser parser(this);
	// ������������
	int r = parser.ParsePropertyDeclaration(&source);
	if( r < 0 )
		return psINVALID_DECLARATION;

	psCScriptNode *node = parser.GetScriptNode();

	// �õ�����
	psCScriptNode *n = node->GetFirstChild()->GetNextNode();
	var->name.Copy(&source.m_Code[n->GetTokenPos()], n->GetTokenLength());

	// �õ���������
	var->type = CreateDataTypeFromNode(node->GetFirstChild(), &source);

	if( m_NumErrors > 0 || m_NumWarnings > 0 )
		return psINVALID_DECLARATION;

	return 0;
}

void psCBuilder::ParseScripts()
{
	psCParser parser(this);
	psCCompiler compiler;

	psCByteCode initBC;
	int largestStackNeeded = 0;

	psCScriptCode* pScript = m_pModule->GetCode();
	assert( pScript );

	// �����ű��ļ�
	if (parser.ParseScript( pScript ) >= 0)
	{
		// ����ȫ�ֽ��
		psCScriptNode* node = parser.GetScriptNode();
		
		node = node->GetFirstChild();
		while (node)
		{			
			psCScriptNode* next = node->GetNextNode();
			node->DisconnectParent();

			if (node->GetNodeType() == snGlobalVar)
			{
				// ���ñ�����
				compiler.Reset(this, pScript, NULL);

				// ȫ�ֱ���
				CompileGlobalVariable(compiler, node, pScript, initBC);
				if (compiler.GetLargestStackUsed() > largestStackNeeded)
				{
					largestStackNeeded = compiler.GetLargestStackUsed();
				}
			}else if (node->GetNodeType() == snClassDecl)
			{
				// ���ñ�����
				compiler.Reset(this, pScript, NULL);

				// �ඨ��
				CompileClass(compiler, node, pScript);
			}
			else if (node->GetNodeType() == snFunction || node->GetNodeType() == snOpOverload)
			{			
				// ȫ�ֺ���/ȫ�ֲ��������غ���
				RegisterScriptFunction(m_pModule->GetNextFunctionId(), node, pScript );	
			}else if ( node->GetNodeType() == snEnum )
			{
				// ���ñ�����
				compiler.Reset(this, pScript, NULL);

				// ö�ٳ���
				compiler.CompileEnum( node, NULL, PS_T("") );
			}
			else
			{
				// û���õ��Ľű����
				int r, c;
				psCScriptCode* code = pScript->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

				WriteWarning(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), TXT_UNUSED_SCRIPT_NODE, r, c);
			}	
			#ifndef NODE_IN_TEMP_MEMORY
				if (node->GetNodeType() != snFunction && 
					node->GetNodeType() != snOpOverload )
					delete node;
			#endif

			// �Ƶ���һ�����
			node = next;
		}
	}

	// ����ģ��ĳ�ʼ������������
	if ( initBC.GetSize() > 0 )
	{
		//////////////////////////////////////////////////////////////////////////
		// Generate initialize byte code
		psCByteCode finalInit;
		finalInit.m_LargestStackUsed = largestStackNeeded;
		if (largestStackNeeded > 0)
			finalInit.Push(largestStackNeeded);

		finalInit.AddCode(&initBC);

		if (largestStackNeeded > 0)
			finalInit.Pop(largestStackNeeded);

		finalInit.Ret(0);
		finalInit.Finalize();

		psCScriptFunction* func = m_pModule->GetSpecialFunction(psFUNC_INIT);
		func->SetCode( finalInit );

		//////////////////////////////////////////////////////////////////////////
		// Generate exit byte code
		psCByteCode exitBC;
	
		int nextLabel = 0;
		largestStackNeeded = 0;

		for ( size_t i = 0; i < m_CompileGlobalVariables.size(); ++i )
		{
			psSGlobalVariableDesc* gvar = m_CompileGlobalVariables[i];
			if ( gvar->datatype.IsObject() && !gvar->datatype.IsReference() )
			{
				psSTypeBehaviour *beh = gvar->datatype.GetBehaviour();
				if (!beh->hasDestructor)
					continue;

				int funcId = beh->destruct;
				bool scriptObject = gvar->datatype.ObjectType()->m_bScriptObject;
				
				int num = gvar->datatype.GetNumArrayElements();
				if ( num > 4 )
				{
					psSOperand thisPointer( PSBASE_STACK,  -4 );
					psSOperand loop_i( PSBASE_STACK,	   -8 );
					psSOperand loop_r( PSBASE_STACK,       -12 );
					psSOperand loop_num( PSBASE_CONST,  RegisterConstantInt(num) )  ;
					psSOperand obj_size( PSBASE_CONST,  RegisterConstantInt(gvar->datatype.GetSizeInMemoryBytes()));

					largestStackNeeded = 3;

					int beforeLabel   = nextLabel++;
					exitBC.Instr2(BC_DEFREF, thisPointer, psSOperand(PSBASE_GLOBAL, gvar->property->index) );
					exitBC.Instr2(BC_MOV4,   loop_i,      psSOperand(PSBASE_CONST,  RegisterConstantInt(0) ) );

					exitBC.Label( beforeLabel );

					// Push this pointer
					exitBC.Push4( thisPointer );

					// Call constructor
					if ( scriptObject )
						exitBC.Call( funcId, 0, true );
					else
						exitBC.CallSys( funcId, 0, true );

					exitBC.Instr1(BC_INCi, loop_i );
					exitBC.Instr3(BC_TLi,  loop_r, loop_i, loop_num );
					exitBC.Instr3(BC_ADDi, thisPointer, thisPointer, obj_size);
					exitBC.Instr2(BC_JNZ,  loop_r, beforeLabel );
				}else
				{
					psSOperand thisPointer = psSOperand(PSBASE_GLOBAL, gvar->property->index);
					for (int i = 0; i < num; ++i )
					{
						// Push this pointer
						exitBC.PushRef( thisPointer );

						// Call constructor
						if ( scriptObject )
							exitBC.Call( funcId, 0, true );
						else
							exitBC.CallSys( funcId, 0, true );

						thisPointer.offset += gvar->datatype.GetSizeInMemoryBytes();
					}
				}
			}		
		}
		if ( !exitBC.IsEmpty() )
		{
			psCByteCode finalExit;

			func = m_pModule->GetSpecialFunction( psFUNC_EXIT );
			if ( largestStackNeeded > 0 )
				finalExit.Push( largestStackNeeded );
			finalExit.AddCode( &exitBC );

			if (largestStackNeeded > 0)
				finalExit.Pop(largestStackNeeded);

			finalExit.Ret(0);

			finalExit.Finalize();

			func->SetCode( finalExit );
		}
	}
}

void psCBuilder::CompileFunctions()
{
	psCCompiler compiler;

	// �������е�ȫ�ֺ���
	for (size_t n = 0; n < m_Functions.size(); ++n)
	{
		psSFunctionDesc* desc = m_Functions[n];
		psCScriptNode* node = desc->node;

		assert (desc->funcId >= 0 && desc->funcId < (int)m_pModule->GetFunctionCount() );

		psCScriptFunction* func = m_pModule->GetScriptFunction(desc->funcId);
		if (compiler.CompileFunction(this, desc->script, node, func->m_ObjectType ) >= 0)
		{
			// ���ö����ƴ���
			func->SetCode( compiler.GetByteCode() );

			// �������ű�(���Ҫ���ɵĻ�)
			if (m_OptimizeOptions.bCreateSymbolTable)
			{
				func->SetDebugSymbols(compiler.GetDebugSymbols() );
			}
		}
	}
}

void psCBuilder::CompileGlobalVariable(psCCompiler& compiler, psCScriptNode* node, psCScriptCode* script, psCByteCode& InitBC)
{
	psSGlobalVariableDesc *gvar = new psSGlobalVariableDesc;

	// ����ȫ�ֱ�������������Ƴ�ͻ.
	if ( compiler.CompileGlobalVariable(this, script, node, gvar) >= 0 &&
		 CheckNameConflictGlobal( gvar->name.AddressOf(), node, script ) >= 0 )
	{
		// ������ȫ�ֱ������ű�ģ��
		psSProperty *prop = new psSProperty;
		prop->index      = gvar->index;
		prop->name       = gvar->name;
		prop->base		 = gvar->base;
		prop->type       = gvar->datatype;
		m_pModule->AddGlobalVar(prop);

		gvar->property = prop;

		// ע��ȫ�ֱ���
		m_GlobalVariables[gvar->name.AddressOf()] = gvar;

		// �Ѵ˱�����ӵ������б�
		m_CompileGlobalVariables.push_back( gvar );

		// ��ӵ���ʼ��������
		InitBC.AddCode( &compiler.GetByteCode() );
	}else
	{
		// ����ʧ��!
		delete gvar;
		gvar = 0;
	}
}

void psCBuilder::CompileClass(psCCompiler& compiler, psCScriptNode* node, psCScriptCode* code)
{
	assert(m_pModule);
	std::vector<psCScriptNode*>     methods;
	std::vector<psCScriptFunction*> funcs;

	psCObjectType* objectType = new psCObjectType();

	// ����������
	if (compiler.CompileClassDecl(this, code, node, methods, objectType) < 0)
	{
		return;
	}

	// ����ඨ��,���ű�ģ��
	m_pModule->AddObjectType( objectType );

	size_t i;
	psCScriptFunction* func;
	// ע�����е��෽��
	for (i = 0; i < methods.size(); ++i)
	{
		psCScriptNode* node = methods[i];
		func = RegisterScriptFunction(m_pModule->GetNextFunctionId(), node, code, objectType);

		funcs.push_back(func);

		// NOTE: Ҫ�Ѵ˽���븸������,����ᱻɾ��
		node->DisconnectParent();
	}
		
	func = GetFunctionDescription( objectType->m_Beh.construct );
	if ( !objectType->m_Beh.hasConstructor )
	{
		// ����Ĭ�ϵĹ��캯����������Ҫ�Ķ�ջ��С
		psCByteCode bc;

		// ���ñ�����
		compiler.Reset( this, code, objectType );

		compiler.DefaultObjectConstructor(objectType, &bc);

		int stacksize = compiler.GetLargestStackUsed();
		if ( !bc.IsEmpty()  )
		{
			// ����ű���ĳ�Ա������Ĭ�Ϲ���ɺ����Ļ�,����һ��Ĭ�ϵĹ��캯��
			psCByteCode funcbc;

			func = AddScriptFunction(objectType->m_Name, 0, 0);

			// ��Ӷ����ƴ���
			funcbc.Push(stacksize);

			funcbc.AddCode(&bc);

			funcbc.Pop(stacksize);

			// NOTE: ע��Ҫ����thisָ��
			funcbc.Ret(1);

			funcbc.Finalize( NULL );

			func->SetCode(funcbc);

			// �����ɱ�������ӵĴ���,�����û��Դ��
			func->SetHasSource(  false );

			func->m_ObjectType = objectType;

			// ���ö������Ϊ����
			objectType->m_Beh.construct		 = func->m_ID;
			objectType->m_Beh.hasConstructor = true;
			objectType->m_Beh.constructors.push_back(func->m_ID);
		}
	}

	if ( !objectType->m_Beh.hasCopyConstructor )
	{
		// ����Ĭ�ϵĿ������캯��
		psCByteCode bc;

		// ���ñ�����
		compiler.Reset( this, code, objectType );

		compiler.DefaultObjectCopyConstructor(objectType, &bc );

		int stacksize = compiler.GetLargestStackUsed();
		if ( !bc.IsEmpty() )
		{
			// ����ű���ĳ�Ա������Ĭ�Ͽ�������ɺ����Ļ�,����һ��
			psCByteCode funcbc;

			func = AddScriptFunction(objectType->m_Name, NULL, NULL );
			
			// ���ò�������
			psCDataType dt(ttIdentifier, false, true);
			dt.SetObjectType( objectType );
			func->m_ParamTypes.push_back( dt );

			// ��Ӷ����ƴ���
			funcbc.Push( stacksize );

			funcbc.AddCode(&bc);

			funcbc.Pop( stacksize ) ;

			// NOTE: ע��Ҫ����thisָ��Ͳ���
			funcbc.Ret( 2 );

			funcbc.Finalize( NULL );

			func->SetCode(funcbc);

			// �����ɱ�������ӵĴ���,�����û��Դ��
			func->SetHasSource(  false );

			func->m_ObjectType = objectType;

			// ���ö������Ϊ����
			objectType->m_Beh.copy = func->m_ID;
			objectType->m_Beh.hasCopyConstructor = true;
			objectType->m_Beh.constructors.push_back(func->m_ID);
		}
	}

	if ( !objectType->m_Beh.hasDestructor )
	{		
		// ����Ĭ�ϵ�����������������Ҫ�Ķ�ջ��С
		psCByteCode bc;

		// ���ñ�����
		compiler.Reset( this, code, objectType );

		compiler.DefaultObjectDestructor(objectType, &bc);

		int stacksize = compiler.GetLargestStackUsed();
		if ( !bc.IsEmpty() )
		{
			// ����ű���ĳ�Ա������Ĭ�Ϲ���ɺ����Ļ�,����һ��
			psCByteCode funcbc;
			func = AddScriptFunction( psCString( PS_T("~") ) + objectType->m_Name, 0, 0);

			// ��Ӷ����ƴ���
			funcbc.Push(stacksize);

			funcbc.AddCode(&bc);

			funcbc.Pop(stacksize);

			// NOTE: ע��Ҫ����thisָ��
			funcbc.Ret(1);

			funcbc.Finalize( NULL );

			func->SetCode(funcbc);


			// �����ɱ�������ӵĴ���,�����û��Դ��
			func->SetHasSource( false );

			func->m_ObjectType = objectType;

			// ���ö������Ϊ����
			objectType->m_Beh.destruct = func->m_ID;
			objectType->m_Beh.hasDestructor = true;
		}
	}
}

int psCBuilder::AllocateGlobalVar(const psCDataType& type)
{
	// ����ȫ�ֱ����ռ�
	return m_pModule->AllocGlobalMemory( type );
}


void psCBuilder::DeallocGlobalVar(const psCDataType& type)
{
	// �ͷ�ȫ�ֱ����ռ�
	m_pModule->DeallocGlobalMemory( type );
}

psCScriptFunction* psCBuilder::RegisterScriptFunction(int funcID, psCScriptNode *node, psCScriptCode *script, psCObjectType* objType)
{
	psCScriptNode* first = node->GetFirstChild();

	psCString msg;
	bool bHasError = false;

	// �õ�����ֵ������
	psCDataType returnType(ttVoid, false, false);
	
	if (node->GetNodeType() == snFunction ||
		node->GetNodeType() == snOpOverload)
	{
		returnType = CreateDataTypeFromNode(first, script);
		ModifyDataTypeFromNode(returnType, first->GetNextNode(), script );
	}else
	{
		returnType.SetObjectType( objType );
		returnType.IsReference() = true;
		returnType.IsPointer()	 = false;
	}

	// �õ�������
	psCScriptNode* n = first;
	while (n && n->GetNodeType() != snParameterList)
		n = n->GetNextNode();
	n = n->GetPrevNode();

	psCString name(PS_T(""));
	if (objType == NULL)
	{
		if (node->GetNodeType() == snFunction)
		{	
			// ȫ�ֺ���
			name.Copy(&script->m_Code[n->GetTokenPos()], n->GetTokenLength());
		}else if (node->GetNodeType() == snOpOverload)
		{
			// ȫ�ֲ���������
			name = PS_T("operator ");
			name += psGetTokenDefinition(node->GetTokenType());
		}else
			assert(false);
	}else
	{		
		name.Copy(&script->m_Code[n->GetTokenPos()], n->GetTokenLength());

		// ��ĺ���
		if (node->GetNodeType() == snConstructor)
		{
			if (name != objType->m_Name)
			{
				// ���캯�������Ʊ�����������ͬ
				msg = TXT_CONSTRUCTOR_FUNC_NAME_ERROR;
				bHasError = true;
			}
			name = objType->m_Name;
		}
		else if (node->GetNodeType() == snDestructor)
		{
			if (name != objType->m_Name)
			{
				// �������������Ʊ�����������ͬ
				msg = TXT_DESTRUCTOR_FUNC_NAME_ERROR;
				bHasError = true;
			}
			name = psCString( PS_T("~") ) + objType->m_Name;
		}
		else if (node->GetNodeType() == snOpOverload)
		{
			name = PS_T("operator ");
			name += psGetTokenDefinition(node->GetTokenType());
		}else
		{
			name.Copy(&script->m_Code[n->GetTokenPos()], n->GetTokenLength());
		}
	}

	std::vector<psCDataType> paramTypes;

	// �õ���һ���������
	n = n->GetNextNode()->GetFirstChild();
	while( n )
	{
		psCDataType type = CreateDataTypeFromNode(n, script);
		n = n->GetNextNode();

		ModifyDataTypeFromNode(type, n, script );
		n = n->GetNextNode();

		assert( n );
		if ( n->GetNodeType() == snIdentifier )
		{
			n = n->GetNextNode();
		}
		assert( n &&  n ->GetNodeType() == snArrayDim );
		ModifyDataTypeFromNode( type, n, script );
		n = n->GetNextNode();

		// ������������
		paramTypes.push_back(type);
	}

	if (objType)
	{
		// ��ķ���
		if (node->GetNodeType() == snDestructor)
		{
			// ��������
			if ( objType->m_Beh.hasDestructor )
			{
				// ���������Ѿ�����
				msg = TXT_DESTRUCTOR_ALREADY_EXIST;
				bHasError = true;
			}else
			{
				if (paramTypes.size() > 0)
				{
					// �����Ĵ�С����0
					msg = TXT_DESTRUCTOR_DONT_HAS_ARGS;
					bHasError = true;
				}else
				{
					objType->m_Beh.destruct = funcID;
					objType->m_Beh.hasDestructor = true;
				}
			}
		}else
		{
			// ��Ա����(����Ա�����Ƿ��Ѿ�����)
			if (CheckMethodConflict(objType, name, returnType, paramTypes, node->GetNodeType(), node->GetTokenType()) < 0)
			{	
				if (node->GetNodeType() == snConstructor)
					msg.Format(TXT_CONSTRUCTOR_s_ALREADY_EXIST, name.AddressOf() );
				else if (node->GetNodeType() == snOpOverload)
					msg.Format(TXT_OPOVERLOAD_s_ALREADY_EXIST, name.AddressOf() );
				else
					msg.Format(TXT_METHOD_s_ALREADY_EXIST, name.AddressOf() );
				bHasError = true;
			}else
			{
				if (node->GetNodeType() == snFunction)
				{
					// ��Ա���������Ʋ���Ϊ����
					if (name == objType->m_Name)
					{
						msg.Format(TXT_METHODNAME_s_SAMEAS_CLASSNAME, name.AddressOf() );
						bHasError = true;
					}else
					{
						objType->m_Methods.push_back(funcID);
					}
				}else if (node->GetNodeType() == snConstructor)
				{
					// ���ö�����Ϊ����
					objType->m_Beh.constructors.push_back(funcID);
					if ( paramTypes.size() == 0 )
					{
						objType->m_Beh.construct = funcID;
						objType->m_Beh.hasConstructor = true;

					}
				}else if (node->GetNodeType() == snOpOverload)
				{	
					// �����������
					int op = node->GetTokenType();
						
					if (!psCScriptEngine::IsOpAllowOverload(op, true))
					{
						// ���������ܱ�����
						msg.Format(TXT_OP_s_NOT_ALLOW_OVERLOAD, psGetTokenDefinition(op));
						bHasError = true;
					}else
					{
						int expectParamSize = (op == ttMinus) ? 0 : 1;
						if ( (int)paramTypes.size() > expectParamSize )
						{
							// ���ز������Ĳ�����С����
							msg.Format(TXT_OPOVERLOAD_s_HAS_TOOMANY_PARAM, psGetTokenDefinition(op));
							bHasError = true;
						}else
						{
							// ���ö�����Ϊ����
							objType->m_Beh.operators.push_back(op);
							objType->m_Beh.operators.push_back(funcID);
						}
					}
				}
			}
		}
	}else
	{
		// ȫ�ֺ���
		if (node->GetNodeType() == snFunction)
		{
			// ���ȫ�ֺ����Ƿ����
			bHasError = IsFunctionExist(name, paramTypes);
			if( bHasError )
			{
				msg.Format(TXT_OPOVERLOAD_s_ALREADY_EXIST, name.AddressOf() );
			}
		}else if (node->GetNodeType() == snOpOverload)
		{
			// ȫ�ֲ���������
			int op = node->GetTokenType();
			if ( !psCScriptEngine::IsOpAllowOverload(op, false) )
			{	
				// ���������ز���������
				msg.Format(TXT_OP_s_NOT_ALLOW_OVERLOAD, psGetTokenDefinition(op));
				bHasError = true;
			}else
			{
				if (paramTypes.size() != 2)
				{
					// ȫ�ֲ��������صĲ���ֻ��������
					msg.Format(TXT_OPOVERLOAD_s_ONLY_HAS_TWOPARAM, psGetTokenDefinition(op));
					bHasError = true;
				}else
				{
					// ���ȫ�ֲ������Ƿ��Ѿ�������
					bHasError = IsGlobalOperatorExist(op, paramTypes);
					if (bHasError)
					{
						msg.Format(TXT_OPOVERLOAD_s_ALREADY_EXIST, psGetTokenDefinition(op));
					}
					else
					{
						m_pModule->AddGlobalOperator(op, funcID);
					}
				}
			}
		}
	}

	if (bHasError)
	{
		// ���������Ϣ
		int r, c;
		psCScriptCode* code = script->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);
						
		WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), msg.AddressOf(), r, c);
	}

	// ���һ����������
	psSFunctionDesc* func = new psSFunctionDesc();
	func->name = name;
	func->node = node;
	func->script = script;
	func->funcId = funcID;
	m_Functions.push_back(func);

	// ע�ắ�� 
	psCScriptFunction* scriptFunc = m_pModule->AddScriptFunction(funcID, name.AddressOf(), returnType, paramTypes);
	scriptFunc->m_ObjectType = objType;
	scriptFunc->SetHasSource( true );

	// �õ�Դ�ļ�
	int r, c;
	psCScriptCode* code = script->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);
	if ( code != script )
	{
		// �����������ͷ�ļ���
		scriptFunc->m_IsIncluded = true;
	}else
	{
		scriptFunc->m_IsIncluded = false;
	}
	scriptFunc->SetSourceFile( code->m_FileName );

	if ( node->GetNodeType() == snConstructor)
		scriptFunc->SetConstructor(true);
	else if ( node->GetNodeType() == snDestructor )
		scriptFunc->SetDestructor( true );

	return scriptFunc;
}

void psCBuilder::FindGlobalOperators(int op, std::vector<int>& funcs)
{
	assert( m_pEngine );
	// ��ϵͳ�������в���
	m_pEngine->FindGlobalOperators(op, funcs);

	if (m_pModule)
	{
		// �ڽű�ģ���в���
		m_pModule->FindGlobalOperators(op, funcs);
	}
}

bool psCBuilder::IsFunctionExist( const psCString& name, std::vector<psCDataType>& parameterTypes)
{
	// �õ�ͬ���ĺ���ID
	std::vector<int> funcs;
	GetFunctionDescriptions( name.AddressOf(), funcs );
	
	for( size_t n = 0; n < funcs.size(); ++n )
	{
		psCScriptFunction *func = GetFunctionDescription(funcs[n]);
		
		// ������еĲ�������
		if( parameterTypes.size() == func->m_ParamTypes.size() )
		{
			size_t p;
			for( p = 0; p < parameterTypes.size(); ++p )
			{
				if( parameterTypes[p] != func->m_ParamTypes[p] )
				break;
			}
			if (p == parameterTypes.size())
			{
				return true;
			}
		}
	}
	return false;
}

bool psCBuilder::IsGlobalOperatorExist(int op, std::vector<psCDataType>& parameterTypes)
{
	std::vector<int> funcs;
	// �õ�ͬ�����������غ���
	FindGlobalOperators(op, funcs);
	
	for( size_t n = 0; n < funcs.size(); ++n )
	{
		psCScriptFunction *func = GetFunctionDescription(funcs[n]);

		// ������в�������
		if( parameterTypes.size() == func->m_ParamTypes.size() )
		{
			size_t p;
			for( p = 0; p < parameterTypes.size(); ++p )
			{
				if( parameterTypes[p] != func->m_ParamTypes[p] )
				break;
			}
			if (p == parameterTypes.size())
				return true;
		}
	}
	return false;
}

void psCBuilder::GetAllMacros(MacroListMap_t& macroList)
{
	assert(m_pEngine);

	// �õ��궨���б�(NOTE:�����ڵĺ�û�к��,�������滻�ı�ҲΪ��)
	psSMacro macro;
	macro.text = PS_T("");
	for (size_t i = 0; i < m_pEngine->GetNumMacros(); ++i)
	{
		macroList[ m_pEngine->GetMacroName(i) ] = macro;
	}
}

int psCBuilder::CheckNameConflictObject(const psCHAR *name, psCScriptNode *node, psCScriptCode *script)
{
	// ����Ƿ�����������
	if( GetObjectType( name ) != NULL )
	{
		if( script )
		{
			int r, c;
			psCScriptCode* code = script->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

			psCString str;
			str.Format(TXT_NAME_CONFLICT_s_EXTENDED_TYPE, name);
			WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), str.AddressOf(), r, c);
		}
		return -1;
	}
	return 0;
}

int psCBuilder::CheckNameConflictGlobal(const psCHAR *name, psCScriptNode *node, psCScriptCode *script)
{
	// ����Ƿ���ȫ�ֱ�������
	psSProperty *prop = GetGlobalProperty(name);
	if( prop )
	{
		if( script )
		{
			int r, c;
			psCScriptCode* code = script->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

			psCString str;
			str.Format(TXT_NAME_CONFLICT_s_GLOBAL_PROPERTY, name);

			WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), str.AddressOf(), r, c);
		}
		return -1;
	}

	// �Ƿ���ȫ��ö�ٳ�����ͻ
	static psCString str_empty(PS_T(""));
	EnumConstantMap_t& enumConstants = m_EnumConstants[str_empty];
	if (enumConstants.find( psCString(name) )  != enumConstants.end() )
	{
		if( script )
		{
			int r, c;
			psCScriptCode* code = script->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

			psCString str;
			str.Format(TXT_NAME_CONFILIC_s_ENUM_CONSTANT, name);
			WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), str.AddressOf(), r, c);
		}
		return -1;
	}
	return 0;
}

int psCBuilder::VerifyProperty(psCDataType *dt, const psCHAR *decl, psCString &name, psCDataType &type)
{
	m_NumErrors = 0;
	m_NumWarnings = 0;

	if( dt )
	{
		// ȷ������������һ�����������
		if( dt->GetObjectType() == NULL )
			return psINVALID_OBJECT;
	}

	// ������Ե����ͺ�����
	psCScriptCode source;
	source.SetCode(TXT_PROPERTY, decl);

	psCParser parser(this);
	int r = parser.ParsePropertyDeclaration(&source);
	if( r < 0 )
	{
		return psINVALID_DECLARATION;
	}

	// �õ�������������
	psCScriptNode *dataType = parser.GetScriptNode()->GetFirstChild();
	
	psCScriptNode *nameNode = dataType->GetNextNode()->GetNextNode();

	type = CreateDataTypeFromNode(dataType, &source);

	psCScriptNode* node = dataType->GetNextNode();
	ModifyDataTypeFromNode( type, node, &source );

	name.Copy(&decl[nameNode->GetTokenPos()], nameNode->GetTokenLength());

	// ��������������
	node = node->GetNextNode();
	node = node->GetNextNode();
	while ( node )
	{
		assert(node->GetTokenType() == ttIntConstant ||
			   node->GetTokenType() == ttBitsConstant );

		psCHAR txt[ 256 ];
		if ( node->GetTokenLength() > 255 )
			return psINVALID_DECLARATION;

		memcpy( txt, source.m_Code.AddressOf() + node->GetTokenPos(), sizeof(psCHAR)*node->GetTokenLength() );
		txt[ node->GetTokenLength() ] = 0;

		int arraySize = ps_strtol( txt, NULL, 10 );
		type.PushArraySize( arraySize );

		node = node->GetNextNode();
	}

	// ������Ƴ�ͻ
	if( dt )
	{
		// ����Ƿ����Ա��������
		if( CheckNameConflictMember(*dt, name.AddressOf(), nameNode, &source) < 0 )
		{
			return psINVALID_NAME;
		}
	}

	if( m_NumErrors > 0 )
	{
		return psINVALID_DECLARATION;
	}
	return psSUCCESS;
}

int psCBuilder::CheckNameConflictMember(psCDataType &dt, const psCHAR *name, psCScriptNode *node, psCScriptCode *script)
{
	// ����Ƿ��������Ƴ�ͻ
	if( GetObjectType(name) != 0 )
	{
		if( script )
		{
			int r, c;
			psCScriptCode* code = script->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

			psCString str;
			str.Format(TXT_NAME_CONFLICT_s_EXTENDED_TYPE, name);
			WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), str.AddressOf(), r, c);
		}
		return -1;
	}

	// �����ĳ�Ա����
	psCObjectType *t = dt.ObjectType();
	if (t)
	{
		// ���ÿ����Ա����
		std::vector<psSProperty *> &props = t->m_Properties;
		for( size_t n = 0; n < props.size(); n++ )
		{
			if( props[n]->name == name )
			{
				if( script )
				{
					int r, c;
					psCScriptCode* code = script->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

					psCString str;
					str.Format(TXT_NAME_CONFLICT_s_OBJ_PROPERTY, name);
					WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), str.AddressOf(), r, c);
				}

				return -1;
			}
		}
	}

	return 0;
}

int psCBuilder::CheckMethodConflict(psCObjectType* objType, psCString& name, psCDataType& retType, std::vector<psCDataType>& paramTypes, int type, int tokenType)
{
	if (type == snConstructor)
	{
		// ��鹹�캯���Ƿ��Ѿ�����
		for (size_t n = 0; n < objType->m_Beh.constructors.size(); ++n)
		{
			int funcId = objType->m_Beh.constructors[n];
			
			psCScriptFunction* func = GetFunctionDescription(funcId);
			if (func&&func->m_Name == name && 
				func->m_ReturnType == retType && 
				func->m_ParamTypes.size() == paramTypes.size())
			{
				size_t p;
				for ( p = 0; p < paramTypes.size(); ++p)
					if (paramTypes[p] != func->m_ParamTypes[p])
						break;
					
				if (p == paramTypes.size())
				{	
					return -1;
				}
			}
		}
	}else if (type == snFunction)
	{
		assert(m_pModule);
		
		// ��麯���Ƿ��Ѿ�����
		for (unsigned int n = 0; n < m_pModule->GetFunctionCount(); ++n)
		{
			psCScriptFunction* func = m_pModule->GetScriptFunction(n);

			if (func->m_ObjectType == objType &&
				func->m_Name == name &&
				func->m_ReturnType == retType &&
				func->m_ParamTypes.size() == paramTypes.size())
			{
				size_t p;
				for ( p = 0; p < paramTypes.size(); ++p)
					if (paramTypes[p] != func->m_ParamTypes[p])
						break;
					
				if (p == paramTypes.size())	
					return -1;
			}
		}	
	}else if (type == snOpOverload)
	{
		// �����������غ���
		for (size_t n = 0; n < objType->m_Beh.operators.size(); n += 2)
		{
			int token = objType->m_Beh.operators[n];
			
			if (token == tokenType)
			{
				int funcId = objType->m_Beh.operators[n+1];
				psCScriptFunction* func = GetFunctionDescription(funcId);
				if (func&&func->m_Name == name && 
					func->m_ReturnType == retType && 
					func->m_ParamTypes.size() == paramTypes.size())
				{
					size_t p;
					for ( p = 0; p < paramTypes.size(); ++p)
						if (paramTypes[p] != func->m_ParamTypes[p])
							break;
						
					if (p == paramTypes.size())
						return -1;
				}
			}
		}
	}
	return 0;
}

psSProperty* psCBuilder::GetObjectProperty(const psCObjectType* objectType, const psCHAR *name, int& baseOffset )
{
	baseOffset = 0;

	// �������Ƶõ�����������
	while ( objectType )
	{
		const std::vector<psSProperty *> &props = objectType->m_Properties;

		for( size_t n = 0; n < props.size(); n++ )
			if( props[n]->name == name )
				return props[n];

		baseOffset += objectType->m_BaseOffset;
		objectType = objectType->m_pParent;
	}
	return NULL;
}


psCDataType psCBuilder::CreateDataTypeFromNode(psCScriptNode* node, psCScriptCode* script)
{
	assert(node->GetNodeType() == snDataType);
	
	psCDataType dt;
	psCScriptNode* n = node->GetFirstChild();
	if (n->GetTokenType() == ttConst)
	{
		dt.IsReadOnly() = true;
		n = n->GetNextNode();
	}
	
	psCScriptNode* typeNode = n;
	dt.TokenType() = typeNode->GetTokenType();
	
	if (dt.TokenType() == ttIdentifier)
	{
		// �õ�����
		psCString str;
		str.Copy(&script->m_Code[n->GetTokenPos()], n->GetTokenLength());

		if (m_pModule)
		{
			// �Ƿ�Ϊ�ű��е���
			dt.SetObjectType(m_pModule->FindObjectType(str.AddressOf()));
		}

		if (dt.GetObjectType() == NULL)
		{
			// �Ƿ�Ϊϵͳ����
			dt.SetObjectType(m_pEngine->GetObjectType(str.AddressOf()));
			
			if( dt.GetObjectType() == NULL )
			{
				// ��ʶ�������
				str.Format(TXT_IDENTIFIER_s_NOT_DATA_TYPE, str.AddressOf() );
				
				int r, c;
				psCScriptCode* code = script->ConvertPosToRowCol(n->GetTokenPos(), &r, &c);
				
				WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), str.AddressOf(), r, c);
				
				dt.TokenType() = ttInt;
			}
		}
	}
	return dt;
}

psCObjectType* psCBuilder::GetObjectType(const psCHAR* name)
{
	assert(m_pEngine);
	psCObjectType* objType = m_pEngine->GetObjectType(name);
	if ( objType == NULL )
	{
		if (m_pModule)
		{
			// ����нű�ģ��,���ű�ģ����ȥ��
			return m_pModule->FindObjectType(name);
		}
	}
	return objType;
}

void psCBuilder::ModifyDataTypeFromNode(psCDataType& type, psCScriptNode* node, psCScriptCode* script )
{
	psCScriptNode* n = node->GetFirstChild();
	if ( node->GetNodeType() == snDataType )
	{
		if (n)
		{
			if ( n->GetTokenType() == ttStar )
			{
				type.IsPointer()  = true;
				type.IsReference() = true;
			}else if ( n->GetTokenType() == ttAmp )
			{
				type.IsReference() = true;
				type.IsPointer()	= false;
			}
		}
	}else if ( node->GetNodeType()  == snArrayDim )
	{
		while ( n )
		{
			assert( n->GetNodeType() == snConstant );
			
			psCString str;
			str.Copy( &script->m_Code[n->GetTokenPos()], n->GetTokenLength());

			int arraySize = ps_strtol(str.AddressOf(), NULL, 10 );

			if ( !type.PushArraySize( arraySize ) )
			{
				int r, c;
				psCScriptCode* code = script->ConvertPosToRowCol(n->GetTokenPos(), &r, &c);

				WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), TXT_ARRAY_DIM_TOO_LARGE, r, c);
				return;
			}
			n = n->GetNextNode();
		}
	}
}


psSProperty* psCBuilder::GetGlobalProperty( const psCHAR* name )
{
	// �ӽű�ģ���в���
	GlobalVariableMap_t::iterator ite = m_GlobalVariables.find( name );
	if ( ite != m_GlobalVariables.end() )
		return ite->second->property;

	// ��ϵͳȫ�ֱ����в���
	psSProperty* prop = (psSProperty*)( m_pEngine->FindGlobalProp( name ) );
	if ( prop )
		return prop;

	// û���ҵ�!
	return NULL;
}

bool psCBuilder::DeclareConstantEnum( const psCHAR* _field, const psCHAR* _name, int value )
{
	psCString name( _name );
	psCString field( _field );
	EnumConstantMap_t& constants = m_EnumConstants[ field ];
	EnumConstantMap_t::iterator it = constants.find( name );

	bool bOk = true;
	if ( it != constants.end() )
		bOk = false;
	
	if ( bOk )
	{
		constants[ name ] = value;
		return true;
	}else
	{
		return false;
	}
}

bool psCBuilder::GetConstantEnum( const psCHAR* field, const psCHAR* name, int& value )
{
	EnumConstantMap_t& constants = m_EnumConstants[ psCString(field) ];
	EnumConstantMap_t::iterator it = constants.find( psCString(name) );
	if ( it != constants.end() )
	{
		value = it->second;
		return true;
	}else
	{
		return false;
	}
}

int psCBuilder::RegisterConstantInt(int value)
{
	return m_pModule->AddConstantInt(value);
}

int psCBuilder::RegisterConstantFloat(float value)
{
	return m_pModule->AddConstantFloat(value);
}

int psCBuilder::RegisterConstantDouble(double value)
{
	return m_pModule->AddConstantDouble(value);
}

double psCBuilder::GetConstantDouble(int offset)
{
	return *((double*)(m_pModule->GetConstantMemPtr() + offset));
}

int psCBuilder::GetConstantInt(int offset)
{
	return *((int*)(m_pModule->GetConstantMemPtr() + offset));
}

float psCBuilder::GetConstantFloat(int offset)
{
	return *((float*)(m_pModule->GetConstantMemPtr() + offset));
}


const char* psCBuilder::GetConstantPtr( int offset )
{
	return (const char*)( m_pModule->GetConstantMemPtr() + offset );
}


int psCBuilder::RegisterConstantCharA( psCHAR* str, int &len )
{
	assert( len < 8 );
	psCHAR buf[ 8 ];
	
	size_t final_len = 0;
	psProcessTransferChar( str, len, buf, final_len );

	len = (int)final_len;
	return RegisterConstantInt( (int)((char)buf[1]) );
}

int psCBuilder::RegisterConstantCharW( psCHAR* str, int &len )
{
	assert( len < 8 );
	psCHAR buf[ 8 ];

	size_t final_len = 0;
	psProcessTransferChar( str, len, buf, final_len );

	len = (int)final_len;
	return RegisterConstantInt( (int)((wchar_t)buf[2]) );
}

int psCBuilder::RegisterConstantStrA( const psCString& str  )
{
	// ע��Ansi�ַ���	
	size_t len	     = str.GetLength();
	size_t final_len = 0;

	psSimpleString< psCHAR, 256 > dst( len );	
	psProcessTransferChar( str.AddressOf(), len, dst.c_str(), final_len );

#ifdef PS_UNICODE	
	size_t mb_len = (len + 1) * sizeof(wchar_t);
	psSimpleString< char, 512 > final_dst( mb_len );	
	wcstombs( final_dst.c_str(), dst.c_str(), mb_len );
	return m_pModule->AddConstantStrA( final_dst.c_str() );
#else
	return m_pModule->AddConstantStrA( dst.c_str() );
#endif
}

int psCBuilder::RegisterConstantStrW( const psCString& str  )
{
	// ע����ַ���	
	size_t len	     = str.GetLength();
	size_t final_len = 0;

	psSimpleString< psCHAR, 256 > dst( len );	

	psProcessTransferChar( str.AddressOf(), len, dst.c_str(), final_len );

#ifndef PS_UNICODE	
	psSimpleString< wchar_t, 256 > final_dst( len );	
	mbstowcs( final_dst.c_str(), dst.c_str(), (len + 1) );
	return m_pModule->AddConstantStrW( final_dst.c_str() );
#else
	return m_pModule->AddConstantStrW( dst.c_str() );
#endif

}


psCScriptFunction* psCBuilder::GetFunctionDescription(int id)
{
	// ���ݺ���ID,�õ�����ָ��
	if( id < 0 )
	{
		// ϵͳ����
		return m_pEngine->GetSystemFunctionByID(id);
	}
	else 
	{
		if ( id < (int)m_pModule->GetFunctionCount())
		{
			// �ű�����
			return m_pModule->GetScriptFunction(id);
		}else
		{
			return NULL;
		}
	}
}

void psCBuilder::GetFunctionDescriptions( const psCHAR* name, std::vector<int> &funcs )
{
	// ��ϵͳ�����в���
	{
		const std::vector< int >& sysFuncs = m_pEngine->GetSystemFuncIDByName( psCString(name) );
		for( size_t n = 0; n < sysFuncs.size(); n++ )
		{
			// ����ϵͳȫ�ֺ���
			psCScriptFunction* sysfunc = m_pEngine->GetSystemFunctionByID( sysFuncs[n] );
			if( sysfunc && sysfunc->m_ObjectType == NULL && sysfunc->m_Name == name )
			{
				funcs.push_back( sysfunc->m_ID );
			}
		}
	}

	// �ڽű�ģ���в���
	if (m_pModule)
	{
		for( unsigned int n = 0; n < m_pModule->GetFunctionCount(); n++ )
		{
			// ���ҽű�ȫ�ֺ������߱���ĳ�Ա����
			psCScriptFunction* func = m_pModule->GetScriptFunction(n);
			if ( func->GetObjectType() == NULL && func->m_Name == name )
			{
				funcs.push_back(func->m_ID);
			}
		}
	}
}

void psCBuilder::GetObjectMethodDescrptions(const psCHAR* name, psCObjectType* objType, std::vector<int>& methods)
{
	assert(objType);
	if ( objType->IsScriptObject() )
	{
		// �ڽű�ģ���в���
		if (m_pModule)
		{
			// ������нű���ȫ�ֺ���
			for (unsigned int n = 0; n < m_pModule->GetFunctionCount(); ++n)
			{
				psCScriptFunction* func = m_pModule->GetScriptFunction(n);
				if (func && func->m_ObjectType == objType && func->m_Name == name)
					methods.push_back(func->m_ID);
			}
		}
	}else
	{
		// ��ϵͳ�����в���
		const std::vector< int >& sysFuncs = m_pEngine->GetSystemFuncIDByName( psCString(name) );
		for( size_t n = 0; n < sysFuncs.size(); n++ )
		{
			// ����ϵͳȫ�ֺ���
			psCScriptFunction* sysfunc = m_pEngine->GetSystemFunctionByID( sysFuncs[n] );
			if( sysfunc && sysfunc->m_ObjectType == objType && sysfunc->m_Name == name )
			{
				methods.push_back( sysfunc->m_ID );
			}
		}
	}
}

PS_ARGTYPE_CHECK_FUNC psCBuilder::GetArgTypeCheckFunc()
{
	assert( m_pEngine );
	return m_pEngine->GetArgTypeCheckFunc();
}

bool psCBuilder::ReadFile( const psCString &filename, psCString& code )
{
	if ( m_pTextStream == NULL )
		return false;

	if ( !m_pTextStream->Open( filename.AddressOf(), PS_T("rb" )) )
		return false;
	
	long len =  m_pTextStream->GetLength();
	code.SetLength(len);
	
	m_pTextStream->Read(code.AddressOf(), len);

	// ���¼����ַ����ĳ���
	code.SetLength(ps_strlen(code.AddressOf()));

	m_pTextStream->Close();
	return true;
}

void psCBuilder::WriteError(const psCHAR *scriptname, const psCHAR* filename, const psCHAR *message, int r, int c)
{
	m_NumErrors++;
	if( m_pOut )
	{
		psCString str;
		str.Format(PS_T("%s (%d, %d) : %s : %s\n"), scriptname, r, c, TXT_ERROR, message);
		m_pOut->WriteError(str.AddressOf(), filename, r, c);
	}
}

void psCBuilder::WriteWarning(const psCHAR *scriptname, const psCHAR* filename,const psCHAR *message, int r, int c)
{
	m_NumWarnings++;
	if( m_pOut )
	{
		psCString str;
		str.Format(PS_T("%s (%d, %d) : %s : %s\n"), scriptname, r, c, TXT_WARNING, message);
		m_pOut->WriteWarning(str.AddressOf(), filename, r, c);
	}
}

void psCBuilder::WriteInfo(const psCHAR *scriptname, const psCHAR *message, int r, int c, bool pre)
{
	if( m_pOut )
	{
		psCString str;
		str.Format(PS_T("%s (%d, %d) : %s : %s\n"), scriptname, r, c, TXT_INFO, message);
		m_pOut->Write(str.AddressOf());
	}
}

