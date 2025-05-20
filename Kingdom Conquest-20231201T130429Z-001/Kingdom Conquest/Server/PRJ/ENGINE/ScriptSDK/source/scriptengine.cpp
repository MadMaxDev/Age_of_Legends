#include "scriptengine.h"
#include "bytecode.h"
#include "bytecodedef.h"
#include "builder.h"
#include "callfunc.h"
#include "string.h"
#include "restore.h"
#include "module.h"
#include "context.h"
#include "texts.h"
#include "threadmanager.h"
#include <assert.h>

using namespace pse;


//-------------------------------------------------------------------
// 全局变量
//-------------------------------------------------------------------
const int behave_dual_token[] =
{
	ttPlus,               // psBEHAVE_ADD
	ttMinus,              // psBEHAVE_SUBTRACT
	ttStar,               // psBEHAVE_MULTIPLY
	ttSlash,              // psBEHAVE_DIVIDE
	ttPercent,            // psBEHAVE_MODULO
	ttEqual,              // psBEHAVE_EQUAL
	ttNotEqual,           // psBEHAVE_NOTEQUAL
	ttLessThan,           // psBEHAVE_LESSTHAN
	ttGreaterThan,        // psBEHAVE_GREATERTHAN
	ttLessThanOrEqual,    // psBEHAVE_LEQUAL
	ttGreaterThanOrEqual, // psBEHAVE_GEQUAL
	ttOr,				  // psBEHAVE_LOGIC_OR
	ttAnd,			      // psBEHAVE_LOGIC_AND
	ttBitOr,              // psBEHAVE_BIT_OR
	ttAmp,                // psBEHAVE_BIT_AND
	ttBitXor,             // psBEHAVE_BIT_XOR
	ttBitShiftLeft,       // psBEHAVE_BIT_SLL
	ttBitShiftRight,      // psBEHAVE_BIT_SRL
};
const psCHAR* behave_dual_token_name[] = 
{
	PS_T("+"),
	PS_T("-"),
	PS_T("*"),
	PS_T("/"),
	PS_T("%"),
	PS_T("=="),
	PS_T("!="),
	PS_T("<"),
	PS_T(">"),
	PS_T("<="),
	PS_T(">="),
	PS_T("||"),
	PS_T("&&"),
	PS_T("|"),
	PS_T("&"),
	PS_T("&"),
	PS_T("^"),
	PS_T("<<"),
	PS_T(">>"),
};
const int num_dual_tokens = sizeof(behave_dual_token)/sizeof(int);

const int behave_assign_token[] =
{
	ttAssignment,			// psBEHAVE_ASSIGNMENT
	ttAddAssign,			// psBEHAVE_ADD_ASSIGN
	ttSubAssign,			// psBEHAVE_SUB_ASSIGN
	ttMulAssign,			// psBEHAVE_MUL_ASSIGN
	ttDivAssign,			// psBEHAVE_DIV_ASSIGN
	ttModAssign,			// psBEHAVE_MOD_ASSIGN
	ttOrAssign,				// psBEHAVE_OR_ASSIGN 
	ttAndAssign,			// psBEHAVE_AND_ASSIGN
	ttXorAssign,			// psBEHAVE_XOR_ASSIGN
	ttShiftLeftAssign,		// psBEHAVE_SLL_ASSIGN
	ttShiftRightAssign,		// psBEHAVE_SRL_ASSIGN
};
const psCHAR* behave_assign_token_name[] = 
{
   PS_T("="),
   PS_T("+="),
   PS_T("-="),
   PS_T("*="),
   PS_T("/="),
   PS_T("%="),
   PS_T("|="),
   PS_T("&="),
   PS_T("^="),
   PS_T("<<="),
   PS_T(">>="),
};
const int num_assign_tokens = sizeof(behave_assign_token)/sizeof(int);

const int behave_unary_token[] = 
{
	ttMinus,			// psBEHAVE_UNARY_MINUS 
	ttPlus,				// psBEHAVE_UNARY_PLUS
	ttNot,				// psBEHAVE_UNARY_NOT
	ttBitNot,			// psBEHAVE_UNARY_BITNOT
	ttInc,				// psBEHAVE_INC
	ttDec,				// psBEHAVE_DEC
};

const psCHAR* behave_unary_token_name[] = 
{
	PS_T("-"),
	PS_T("+"),
	PS_T("!"),
	PS_T("~"),
	PS_T("++"),
	PS_T("--"),
};
const int num_unary_tokens = sizeof(behave_unary_token)/sizeof(int);

//----------------------------------------------------------------------//
// 静态函数
//----------------------------------------------------------------------//
bool psCScriptEngine::IsOpAllowOverload(int op, bool object)
{
	int i;
	if (object)
	{
		if (op == ttMinus ||
			op == ttPlus ||
			op == ttOpenBracket ||
			op == ttInc ||
			op == ttDec || 
			op == ttBitNot ||
			op == ttNot )
			return true;

		for (i = 0; i < num_assign_tokens; ++i)
			if (op == behave_assign_token[i])
				return true;
	}else
	{
		for (i = 0; i < num_dual_tokens; ++i)
			if (op == behave_dual_token[i])
				return true;
	}
	return false;
}


//-------------------------------------------------------------------
// 类名: psCScriptEngine
// 说明: 
//-------------------------------------------------------------------
psCScriptEngine::psCScriptEngine(void)
{
	m_RefCount		 = 1;
	m_bConfigFailed  = false;
	m_bIsPrepared    = false;
	m_pLastModule    = NULL;
	m_pStringModule  = NULL;
	m_pDebugger		 = NULL;
	m_pArgTypeCheckFunc = NULL;

	m_pDefaultCtx    = new psCContext(this, false );
	m_strMacros		 = PS_T("");

	m_InitContexStackSize = 256;  // 1K
	m_MaxContexStackSize = 0;	  // No limit
}

psCScriptEngine::~psCScriptEngine(void)
{	   
	// 释放所有脚本模块
	Reset();

	// 释放默认的脚本运行上下文
	if ( m_pDefaultCtx )
	{
		delete m_pDefaultCtx;
		m_pDefaultCtx = NULL;
	}

	// 释放字符串运行模块
	if( m_pStringModule )
	{
		delete m_pStringModule;
		m_pStringModule = NULL;
	}

	size_t n;
	// 释放所有的全局属性
	{
		PropertyMap_t::iterator ite = m_GlobalProps.begin();
		PropertyMap_t::iterator end = m_GlobalProps.end();
		for ( ; ite != end; ++ite )
		{
			delete ite->second;
		}
		m_GlobalProps.clear();	
		m_GlobalPropAddresses.clear();
	}
	// 释放所有的类对象类型
	{
		ObjectTypeMap_t::iterator ite = m_ObjectTypes.begin();
		ObjectTypeMap_t::iterator end = m_ObjectTypes.end();
		for ( ; ite != end; ++ite )
		{
			delete ite->second;
		}	
		m_ObjectTypes.clear();
	}

	// 释放所有的系统函数
	for( n = 0; n < m_SystemFunctions.size(); n++ )
		delete m_SystemFunctions[n];
	m_SystemFunctions.clear();

	// 释放所有的系统函数接口
	for( n = 0; n < m_SystemFunctionInterfaces.size(); n++ )
		delete m_SystemFunctionInterfaces[n];
	m_SystemFunctionInterfaces.clear();

}

long psCScriptEngine::AddRef()
{
	return ++m_RefCount;
}

long psCScriptEngine::Release()
{
	if ( --m_RefCount == 0 )
	{
		delete this;
		return 0;
	}else
	{
		return m_RefCount;
	}
}

void psCScriptEngine::Reset()
{
	size_t n;
	for( n = 0; n < m_ScriptModules.size(); ++n )
	{
		if ( m_ScriptModules[n] )
		{
			m_ScriptModules[n]->Destroy();
			delete m_ScriptModules[n];
		}
	}
	m_ScriptModules.clear();	

	ModuleNameMap_t::iterator it = m_ModuleNameMap.begin();
	for (; it != m_ModuleNameMap.end(); ++it )
	{
		delete []( it->first );
	}
	m_ModuleNameMap.clear();

	m_pLastModule = NULL;
	return;
}

int psCScriptEngine::Compile( psIModule* pModule, psITextStream* pTextStream, psIOutputStream* pOutStream )
{ 
	//if (m_bConfigFailed)
	//{
	//	// 脚本配置出错
	//	if( pOutStream )
	//	{
	//		pOutStream->Write(TXT_INVALID_CONFIGURATION);
	//	}
	//	return psINVALID_CONFIGURATION;
	//}
	if (pModule == NULL) 
		return psNO_MODULE;

	psCModule* mod = static_cast<psCModule*>(pModule);
	if ( !mod->CanReset() )
	{
		return psMODULE_CANT_BE_RESET;
	}

	// 构建此模块
	int r = mod->Build( pTextStream, pOutStream );
	if ( m_pDebugger )
	{
		m_pDebugger->OnCompileModule( mod, r );
	}
	return r;
}

int psCScriptEngine::ExecuteString(psIScriptContext* ctx, const psCHAR *script, psIOutputStream* pOutStream )
{
	//if (m_bConfigFailed)
	//{
	//	// 脚本配置出错
	//	if( pOutStream )
	//	{
	//		pOutStream->Write(TXT_INVALID_CONFIGURATION);
	//	}
	//	return psINVALID_CONFIGURATION;
	//}

	// 得到运行字符串的模块
	if (m_pStringModule == NULL)
	{
		m_pStringModule = new psCModule(TXT_EXECUTESTRING, -1, this);
	}

	// 编译字符串函数
	psCBuilder builder( this, m_pStringModule);

	builder.SetOutputStream( pOutStream );

	psCString str(PS_T("void "));
	str += TXT_EXECUTESTRING;
	str += PS_T("(){\n");
	str += script;
	str += PS_T(";}");

	int r = builder.BuildString(str.AddressOf() );
	if( r < 0 )
	{
		return psERROR;
	}

	if ( ctx == NULL )
	{
		ctx = m_pDefaultCtx;
	}

	// 准备运行
	r = ((psCContext*)ctx)->PrepareSpecial(-1, psFUNC_STRING);
	if( r < 0 )
	{
		return r;
	}

	// 运行脚本
	return ctx->Execute(0);
}

int psCScriptEngine::Discard(const psCHAR* module)
{
	if ( module == NULL )
		return psERROR;

	// 得到脚本模块
	ModuleNameMap_t::iterator it = m_ModuleNameMap.find( module );
	if(  it == m_ModuleNameMap.end() )
		return psNO_MODULE;

	psCModule* mod = m_ScriptModules[ it->second ];
	if ( mod == NULL )
		return psNO_MODULE;

	// NOTE: 要保证是线程安全的
	if ( !mod->CanReset() )
		return psMODULE_CANT_BE_RESET;

	// 通知调试器
	if ( m_pDebugger )
	{
		m_pDebugger->OnDeleteModule( mod );
	}

	// 消毁脚本模块, 
	mod->Destroy();

	if ( m_pLastModule == mod )
	{
		m_pLastModule = NULL;
	}

	for ( size_t i = 0; i < m_ScriptModules.size(); ++i )
	{
		if ( m_ScriptModules[i] == mod )
		{
			m_ScriptModules[i] = NULL;
		}
	}

	// Delete memory
	delete []( it->first );
	delete mod;

	// Erase it from module map
	m_ModuleNameMap.erase( it );

	return 0;
}

// Internal
int psCScriptEngine::CreateContext(psIScriptContext **ppContex)
{
	if ( ppContex )
	{
		// Hold engine reference
		*ppContex = new psCContext(this, true);
		return 0;
	}
	else
	{
		return psERROR;
	}
}

int psCScriptEngine::RegisterObjectProperty(const psCHAR* obj, const psCHAR* declaration, int byteOffset)
{
	psCDataType dt, type;
	psCString name;

	int r;
	psCBuilder bld(this, 0);

	// 检查数据类型
	r = bld.ParseDataType(obj, &dt);
	if( r < 0 )
		return ConfigError(r);

	if( (r = bld.VerifyProperty(&dt, declaration, name, type)) < 0 )
		return ConfigError(r);

	// 类成员属性性的类对象不能为空
	if( dt.GetObjectType() == NULL ) 
		return ConfigError(psINVALID_OBJECT);

	// 类成员属性不能为指针
	if ( type.IsPointer() )
		return ConfigError(psINVALID_DECLARATION);

	// 添加一个成员属性
	psSProperty *prop = new psSProperty;
	prop->name            = name;
	prop->type            = type;
	prop->byteOffset      = byteOffset;
	prop->base			  = PSBASE_UNKNOWN;
		
	dt.ObjectType()->m_Properties.push_back(prop);

	return psSUCCESS;
}

int psCScriptEngine::RegisterObjectType(const psCHAR* name, int byteSize, psDWORD flags)
{
	// 检查类对象标志
	if( flags >= psOBJ_CLASS_MAX )
		return ConfigError(psINVALID_ARG);

	// 检查类对象名不能为空
	if( name == NULL )
		return ConfigError(psINVALID_NAME);

	// 类的大小不能小于0
	if( byteSize < 0 )
		return ConfigError(psINVALID_ARG);

	// 分析类对象类型
	psCDataType dt;
	psCBuilder bld(this, 0);
	int r = bld.ParseDataType(name, &dt);

	// 如果成功, 那么这个类对象类型已经被注册了
	if ( r >= 0 )
		return (psCLASS_EXIST);

	// 确认其类对象名不是一个关键字
	psCTokenizer t;
	int tokenLen;
	int token = t.GetToken(name, int(ps_strlen(name)), &tokenLen);
	if( token != ttIdentifier || ps_strlen(name) != (unsigned)tokenLen )
		return ConfigError(psINVALID_NAME);

	// 检查名称冲突
	r = bld.CheckNameConflictObject(name, 0, 0);
	if( r < 0 ) 
		return ConfigError(psNAME_TAKEN);

	// 设置类对象类型
	psCObjectType *type = new psCObjectType;
	type->m_Name = name;
	type->m_TokenType = ttIdentifier;
	type->m_Size   = byteSize;
	type->m_Flags  = flags;
	if ( flags & psOBJ_INTERFACE )
	{
		type->m_Beh.isInterface = true;
	}else
	{
		type->m_Beh.isInterface = false;
	}

	m_ObjectTypes[type->m_Name.AddressOf()] = type;
	return psSUCCESS;
}

int psCScriptEngine::RegisterObjectBehaviour(const psCHAR* objname, psDWORD behaviour, const psCHAR* decl, psUPtr funcPointer, psDWORD callConv)
{
	if( objname == NULL )
		return ConfigError(psINVALID_ARG);
	
	// 检查调用约定
	psSSystemFunctionInterface internal;
	int r = DetectCallingConvention(true, funcPointer, callConv, &internal);
	if( r < 0 )
		return ConfigError(r);

	m_bIsPrepared = false;

	psCBuilder bld(this, 0);

	psCDataType type;

	// 分析出数据类型
	r = bld.ParseDataType(objname, &type);
	if( r < 0 ) 
		return ConfigError(r);

	// 检查数据类型的正确性
	if( type.IsReadOnly() || type.IsReference() )
		return ConfigError(psINVALID_TYPE);

	// 检查调用约定
	if( callConv != psCALL_THISCALL &&
		callConv != psCALL_STDCALL &&
		callConv != psCALL_CDECL_OBJFIRST )
	{
		return ConfigError(psNOT_SUPPORTED);
	}

	// 类型必需为类对象
	if( type.GetObjectType() == NULL )
		return ConfigError(psINVALID_TYPE);

	psSTypeBehaviour *beh = type.GetBehaviour();
	// 检查函数声明(不能为变参函数)
	psCScriptFunction func;

	bool bArgVariational;
	r = bld.ParseFunctionDeclaration(decl, &func, &bArgVariational);
	if( r < 0 || bArgVariational)
		return ConfigError(psINVALID_DECLARATION);

	// 检查函数的参数和返回值,其不能为数组类型
	for( size_t n = 0; n < func.m_ParamTypes.size(); n++ )
		if( func.m_ParamTypes[n].IsArrayType() )
			return ConfigError(psAPP_CANT_INTERFACE_DEFAULT_ARRAY);

	if( func.m_ReturnType.IsArrayType() )
		return ConfigError(psAPP_CANT_INTERFACE_DEFAULT_ARRAY);

	// 设置函数类对象类型
	func.m_ObjectType = type.ObjectType();

	if( behaviour == psBEHAVE_CONSTRUCT )
	{
		// 返回类型必需是空
		if( func.m_ReturnType != psCDataType(ttVoid, false, false) )
			return ConfigError(psINVALID_DECLARATION);

		// TODO: 检查相同的类型的构造函数是否已经存在

		// 设置函数名称
		func.m_Name = objname;

		// 设置类对象行为
		if( func.m_ParamTypes.size() == 0 )
		{
			if ( beh->construct )
				return ConfigError(psALREADY_REGISTERED);

			beh->construct = AddBehaviourFunction(func, internal);
			beh->constructors.push_back(beh->construct);
			beh->hasConstructor = true;
		}
		else if ( func.m_ParamTypes.size() == 1 && func.m_ParamTypes[0].IsEqualExceptConst(type) )
		{
			if( beh->copy )
				return ConfigError(psALREADY_REGISTERED);

			beh->copy = AddBehaviourFunction(func, internal );
			beh->constructors.push_back( beh->copy );
			beh->hasCopyConstructor = true;
		}else
		{
			beh->constructors.push_back(AddBehaviourFunction(func, internal));
		}
	}
	else if( behaviour == psBEHAVE_DESTRUCT )
	{
		if( beh->destruct != 0)
			return ConfigError(psALREADY_REGISTERED);

		// 返回类型必需是空
		if( func.m_ReturnType != psCDataType(ttVoid, false, false) )
			return ConfigError(psINVALID_DECLARATION);

		// 析构函数不能有参数
		if( func.m_ParamTypes.size() > 0 )
			return ConfigError(psINVALID_DECLARATION);

		// 设置函数名称
		func.m_Name = psCString(PS_T("~")) + objname;

		// 添加对象行为
		beh->destruct = AddBehaviourFunction(func, internal);
		beh->hasDestructor = true;
	}else if( behaviour == psBEHAVE_INDEX )
	{
		// 只能有一个参数
		if( func.m_ParamTypes.size() != 1 )
			return ConfigError(psINVALID_DECLARATION);

		// 返回类型不能是空
		if( func.m_ReturnType.TokenType() == ttVoid )
			return ConfigError(psINVALID_DECLARATION);

		// TODO:确认相同类型的函数是否已经存在

		// 设置函数名称
		func.m_Name = psCString(objname) + PS_T(" operator []");

		// 添加类对象行为属性
		beh->operators.push_back(ttOpenBracket);
		beh->operators.push_back(AddBehaviourFunction(func, internal));
	}else if( behaviour >= psBEHAVE_FIRST_ASSIGN && behaviour <= psBEHAVE_LAST_ASSIGN)
	{
		// 确认只能有一个参数
		if( func.m_ParamTypes.size() != 1 )
			return ConfigError(psINVALID_DECLARATION);

		// 返回类型必需和输入类型一致(必要不必要?)
		//if( func.m_ReturnType != type )
		//	return ConfigError(psINVALID_DECLARATION);

		// 设置函数名称
		func.m_Name = psCString(objname) + PS_T(" operator ") + behave_assign_token_name[behaviour-psBEHAVE_FIRST_ASSIGN];
		
		// TODO: 确认相同类型的函数是否已经存在

		// 添加类对象行为属性
		beh->operators.push_back(behave_assign_token[behaviour - psBEHAVE_FIRST_ASSIGN]); 
		beh->operators.push_back(AddBehaviourFunction(func, internal));
	}else if( behaviour >= psBEHAVE_FIRST_UNARY && behaviour <= psBEHAVE_LAST_UNARY )
	{
		// 确认没有参数
		if( func.m_ParamTypes.size() != 0 )
			return ConfigError(psINVALID_DECLARATION);

		// TODO: 确认相同类型的函数是否已经存在

		// 设置函数名称
		func.m_Name = psCString(objname) + PS_T(" operator ") + behave_unary_token_name[behaviour-psBEHAVE_FIRST_UNARY];

		// 添加类对象行为属性
		beh->operators.push_back( behave_unary_token[behaviour-psBEHAVE_FIRST_UNARY] );
		beh->operators.push_back(AddBehaviourFunction(func, internal));
	}
	else
	{
		assert(false);
		return ConfigError(psINVALID_ARG);
	}

	return psSUCCESS;
}

int psCScriptEngine::RegisterGlobalBehaviour(psDWORD behaviour, const psCHAR* decl, psUPtr funcPointer, psDWORD callConv)
{
	psSSystemFunctionInterface internal;
	// 检查调用约定
	int r = DetectCallingConvention(false, funcPointer, callConv, &internal);
	if( r < 0 )
		return ConfigError(r);

	m_bIsPrepared = false;

	psCBuilder bld(this, 0);

	if( callConv != psCALL_CDECL && callConv != psCALL_STDCALL )
		return ConfigError(psNOT_SUPPORTED);

	// 检查函数声明(并且不能为变参函数)
	psCScriptFunction func;

	bool bArgVariational;
	r = bld.ParseFunctionDeclaration(decl, &func, &bArgVariational);
	if( r < 0 || bArgVariational)
		return ConfigError(psINVALID_DECLARATION);

	// 检查函数的参数和返回值,其不能为数组类型
	for( size_t n = 0; n < func.m_ParamTypes.size(); n++ )
		if( func.m_ParamTypes[n].IsArrayType() )
			return ConfigError(psAPP_CANT_INTERFACE_DEFAULT_ARRAY);

	if( func.m_ReturnType.IsArrayType() )
		return ConfigError(psAPP_CANT_INTERFACE_DEFAULT_ARRAY);

	if( behaviour >= psBEHAVE_FIRST_DUAL && behaviour <= psBEHAVE_LAST_DUAL )
	{
		// 检查参数的大小只能为两个
		if( func.m_ParamTypes.size() != 2 )
			return ConfigError(psINVALID_DECLARATION);

		// 返回类型必需不能为空(必要不必要?)
		if( func.m_ReturnType.TokenType() == ttVoid )
			return ConfigError(psINVALID_DECLARATION);

		// 确认有一个参数是类对象类型
		if( !(func.m_ParamTypes[0].TokenType() == ttIdentifier) &&
			!(func.m_ParamTypes[1].TokenType() == ttIdentifier) )
			return ConfigError(psINVALID_DECLARATION);

		// TODO: 确认这个函数是否已经注册过了

		// 设置函数名称
		func.m_Name = psCString(PS_T("operator")) + behave_dual_token_name[behaviour-psBEHAVE_FIRST_DUAL];

		// 映射全局行为
		m_GlobalOperators.push_back(behave_dual_token[behaviour - psBEHAVE_FIRST_DUAL]); 
		m_GlobalOperators.push_back(AddBehaviourFunction(func, internal));
	}
	else
	{
		return ConfigError(psINVALID_ARG);
	}

	return psSUCCESS;
}

int psCScriptEngine::DeclareObjectHierarchy(const psCHAR* name, const psCHAR* parent, int baseOffset )
{
	assert( name && parent );
	assert( baseOffset >= 0 );

	psCObjectType* objType		 = _findObjectType( name );
	psCObjectType* parentObjType = _findObjectType( parent );

	if ( objType == NULL || parentObjType == NULL )
	{
		return psDONT_FIND_CLASS;
	}

	// 检查循环依赖
	psCObjectType* testType = parentObjType;
	while( testType )
	{
		if ( testType == objType )
		{
			return psCLASS_CICLE_DEPENDENCE;
		}
		testType = testType->m_pParent;
	}	

	objType->m_pParent    = parentObjType;
	objType->m_BaseOffset = baseOffset;
	return psSUCCESS;
}

int psCScriptEngine::AddBehaviourFunction(psCScriptFunction &func, psSSystemFunctionInterface &internal)
{
	psCScriptFunction *f = new psCScriptFunction();
	f->m_ReturnType = func.m_ReturnType;
	f->m_ObjectType = func.m_ObjectType;
	f->m_Name = func.m_Name;
	
	for( size_t n = 0; n < func.m_ParamTypes.size(); n++ )
	{
		f->m_ParamTypes.push_back(func.m_ParamTypes[n]);
	}

	AddSystemFunction(f, &internal);

	return f->m_ID;
}

int psCScriptEngine::RegisterGlobalProperty(const psCHAR* declaration, void* pointer)
{
	psCDataType type;
	psCString name;

	// 检查变量声明
	int r;
	psCBuilder bld(this, 0);
	if( (r = bld.VerifyProperty(0, declaration, name, type)) < 0 )
		return ConfigError(r);

	// 保存全局属性
	psSProperty *prop = new psSProperty();
	prop->name       = name;
	prop->type       = type;
	prop->index      = -1 - (int)m_GlobalPropAddresses.size();
	prop->base		 = PSBASE_UNKNOWN;

	std::pair<PropertyMap_t::iterator, bool> ir = m_GlobalProps.insert(std::make_pair(prop->name.AddressOf(), prop));
	if (ir.second == false)
	{
		delete prop;
		return ConfigError(psALREADY_REGISTERED);
	}

	m_GlobalPropAddresses.push_back(pointer);
	return psSUCCESS;
}

int psCScriptEngine::RegisterIntConstant(const psCHAR* name, int v)
{
	// 保存全局属性
	psSProperty *prop = new psSProperty;
	prop->name       = name;
	prop->type       = psCDataType(ttInt, true, false );
	prop->index      = v;
	prop->base		 = PSBASE_NONE;

	std::pair<PropertyMap_t::iterator, bool> r = m_GlobalProps.insert(std::make_pair(prop->name.AddressOf(), prop));
	if (r.second == false)
	{
		delete prop;
		return ConfigError(psALREADY_REGISTERED);
	}
	return psSUCCESS;
}

int psCScriptEngine::RegisterFloatConstant(const psCHAR* name, float v)
{
	// 保存全局属性
	psSProperty *prop = new psSProperty;
	prop->name       = name;
	prop->type       = psCDataType(ttFloat, true, false );
	prop->base		 = PSBASE_NONE;

	float* p		 = (float*)(&(prop->index));
	*p				 = v;

	std::pair<PropertyMap_t::iterator, bool> r = m_GlobalProps.insert(std::make_pair(prop->name.AddressOf(), prop));
	if (r.second == false)
	{
		delete prop;
		return ConfigError(psALREADY_REGISTERED);
	}
	return psSUCCESS;
}

int psCScriptEngine::RegisterObjectMethod(const psCHAR* obj, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv)
{
	psSSystemFunctionInterface internal;
	// 检查调用约定
	int r = DetectCallingConvention(true, funcPointer, callConv, &internal);
	if( r < 0 )
		return ConfigError(r);

	// 分析数据类型
	psCDataType dt;
	psCBuilder bld(this, 0);
	r = bld.ParseDataType(obj, &dt);
	if( r < 0 )
		return ConfigError(r);

	m_bIsPrepared = false;

	psCScriptFunction *func = new psCScriptFunction();

	// 设置函数的对象类型
	func->m_ObjectType = dt.ObjectType();
	
	bool bArgVariational = false;
	// 分析函数的声明
	r = bld.ParseFunctionDeclaration(declaration, func, &bArgVariational);
	if( r < 0) 
	{
		delete func;
		return ConfigError(psINVALID_DECLARATION);
	}

	// 设置变参标志
	func->SetArgVariational( bArgVariational );

	// 确认函数参数和返回值不能是数组类型
	for( size_t n = 0; n < func->m_ParamTypes.size(); n++ )
	{
		if( func->m_ParamTypes[n].IsArrayType() )
		{
			delete func;
			return ConfigError(psAPP_CANT_INTERFACE_DEFAULT_ARRAY);
		}
	}
	if( func->m_ReturnType.IsArrayType() )
	{
		delete func;
		return ConfigError(psAPP_CANT_INTERFACE_DEFAULT_ARRAY);
	}

	// 检查名称冲突
	r = bld.CheckNameConflictMember(dt, func->m_Name.AddressOf(), 0, 0);
	if( r < 0 )
	{
		delete func;
		return ConfigError(psNAME_TAKEN);
	}

	// 添回系统函数
	AddSystemFunction(func, &internal);

	// 添加类方法
	func->m_ObjectType->m_Methods.push_back(func->m_ID);

	return psSUCCESS;
}

int psCScriptEngine::RegisterGlobalFunction(const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv)
{
	psSSystemFunctionInterface internal;
	// 检查调用约定
	int r = DetectCallingConvention(false, funcPointer, callConv, &internal);
	if( r < 0 )
		return ConfigError(r);

	if( (callConv) != psCALL_CDECL && (callConv) != psCALL_STDCALL )
		return ConfigError(psNOT_SUPPORTED);

	m_bIsPrepared = false;

	psCScriptFunction *func = new psCScriptFunction();

	psCBuilder bld(this, 0);

	// 分析函数的声明
	bool bArgVariational;
	r = bld.ParseFunctionDeclaration(declaration, func, &bArgVariational);
	if( r < 0 ) 
	{
		delete func;
		return ConfigError(psINVALID_DECLARATION);
	}

	// 设置变参标志
	func->SetArgVariational( bArgVariational );

	// 确认函数参数和返回值不能是数组类型
	for( size_t n = 0; n < func->m_ParamTypes.size(); n++ )
		if( func->m_ParamTypes[n].IsArrayType() )
		{
			delete func;
			return ConfigError(psAPP_CANT_INTERFACE_DEFAULT_ARRAY);
		}

	if( func->m_ReturnType.IsArrayType() )
	{
		delete func;
		return ConfigError(psAPP_CANT_INTERFACE_DEFAULT_ARRAY);
	}

	// 添加系统函数
	AddSystemFunction(func, &internal);

	m_GlobalFunctions.push_back(func->m_ID);

	return psSUCCESS;
}

int psCScriptEngine::UnregisterObjectType( const psCHAR* name )
{
	assert( name );
	ObjectTypeMap_t::iterator ite = m_ObjectTypes.find( name );
	if ( ite == m_ObjectTypes.end() )
		return psERROR;

	psCObjectType* objType = ite->second;
	m_ObjectTypes.erase( ite );

	size_t i;
	// 删除所有的成员函数
	for ( i = 0; i < objType->m_Methods.size(); ++i )
	{
		DeleteSystemFunctionById( objType->m_Methods[i] );
	}

	// 删除所有的操作符重载函数
	for ( i = 0; i < objType->m_Beh.operators.size(); i += 2 )
	{
		DeleteSystemFunctionById( objType->m_Beh.operators[i + 1] );
	}

	// 删除所有的构造/析构函数
	for ( i = 0; i < objType->m_Beh.constructors.size(); ++i )
	{
		DeleteSystemFunctionById( objType->m_Beh.constructors[i] );
	}

	if ( objType->m_Beh.hasDestructor )
	{
		DeleteSystemFunctionById( objType->m_Beh.destruct );
	}

	// Delete memory
	delete objType;

	return psSUCCESS;
}

int psCScriptEngine::UnregisterGlobalProperty(void* ptr)
{
	for ( size_t i = 0; i < m_GlobalPropAddresses.size(); ++i )
	{
		if ( m_GlobalPropAddresses[i] == ptr )
		{
			m_GlobalPropAddresses[i] = NULL;
			// 删除全局属性
			{	
				int index = -1 - (int)i;
				PropertyMap_t::iterator ite = m_GlobalProps.begin();
				PropertyMap_t::iterator end = m_GlobalProps.end();
				for ( ; ite != end; ++ite )
				{
					if ( ite->second->index == index )
					{
						delete ite->second;
						m_GlobalProps.erase( ite );
						break;
					}
				}
			}
			return psSUCCESS;
		}
	}
	return psERROR;
}

int psCScriptEngine::UnregisterGlobalBehaviour( psUPtr funcPtr )
{
	for ( size_t i = 0; i < m_GlobalOperators.size(); i += 2 )
	{
		int funcId = m_GlobalOperators[i+1];
		psSSystemFunctionInterface* internal = m_SystemFunctionInterfaces[ -funcId - 1 ];
		
		if ( internal && internal->func == (void*)funcPtr.func)
		{
			DeleteSystemFunctionById( funcId );
			return psSUCCESS;
		}
	}
	return psERROR;
}

int psCScriptEngine::UnregisterGlobalFunction( psUPtr funcPtr )
{
	for (size_t i = 0; i < m_SystemFunctionInterfaces.size(); ++i )
	{
		psSSystemFunctionInterface* internal = m_SystemFunctionInterfaces[i];

		if ( internal && internal->func == (void*)funcPtr.func )
		{
			int funcId = -1 - (int)i;
			DeleteSystemFunctionById( funcId );

			for ( i = 0; i < m_GlobalFunctions.size(); ++i )
			{
				if ( m_GlobalFunctions[i] == funcId )
				{
					m_GlobalFunctions[i] = m_GlobalFunctions[m_GlobalFunctions.size()-1];
					m_GlobalFunctions.pop_back();
					break;
				}
			}
			return psSUCCESS;
		}
	}
	return psERROR;
}

void psCScriptEngine::AddSystemFunction(psCScriptFunction* &func, psSSystemFunctionInterface* internal)
{
	psSSystemFunctionInterface *newInterface = new psSSystemFunctionInterface();
	memcpy(newInterface, internal, sizeof(psSSystemFunctionInterface));

	if ( m_FreeFunctionId.size() > 0 )
	{
		func->m_ID = m_FreeFunctionId[m_FreeFunctionId.size() - 1];
		m_FreeFunctionId.pop_back();

		int index = -1 - func->m_ID;
		m_SystemFunctions[index] = func;
		m_SystemFunctionInterfaces[index] = newInterface;
	}else
	{
		func->m_ID = -1 - (int)m_SystemFunctions.size();
		m_SystemFunctions.push_back(func);	
		m_SystemFunctionInterfaces.push_back(newInterface);
	}
	// 构建函数名称映射表
	m_FuncNameMap[ func->m_Name ].push_back( func->m_ID );
}

void psCScriptEngine::DeleteSystemFunctionById( int funcId )
{
	int index = -1 - funcId;

	assert( index >= 0 && index < (int)m_SystemFunctions.size() );
	psCScriptFunction* func = m_SystemFunctions[index];
	psSSystemFunctionInterface* funcInterface = m_SystemFunctionInterfaces[index];

	m_SystemFunctions[index ] = NULL;
	m_SystemFunctionInterfaces[index] = NULL;

	delete func;
	delete funcInterface;

	m_FreeFunctionId.push_back( funcId );
}

const std::vector<int>& psCScriptEngine::GetSystemFuncIDByName( const psCString& name ) const
{
	FuncNameMap_t::const_iterator it = m_FuncNameMap.find( name );
	if ( it != m_FuncNameMap.end() )
	{
		return it->second;
	}else
	{
		static std::vector<int> s_nullvec;
		return s_nullvec;
	}
}

bool psCScriptEngine::IsFunctionEqual(psCScriptFunction* f0, psCScriptFunction* f1)
{
	assert(f0 && f1);
	if (f0->m_ReturnType != f1->m_ReturnType)
		return false;

	if (f0->m_Name != f1->m_Name)
		return false;

	if (f0->m_ObjectType != f1->m_ObjectType)
		return false;

	if (f0->m_funcFlag != f1->m_funcFlag)
		return false;

	if (f0->m_ParamTypes.size() != f1->m_ParamTypes.size())
		return false;

	for (size_t i = 0; i < f0->m_ParamTypes.size(); ++i)
		if (f0->m_ParamTypes[i] != f1->m_ParamTypes[i])
			return false;

	// The two function is equal
	return true;
}

psCObjectType* psCScriptEngine::GetObjectType(const psCHAR *type)
{
	ObjectTypeMap_t::iterator ite = m_ObjectTypes.find( type );
	if ( ite != m_ObjectTypes.end() )
		return ite->second;

	return 0;
}

void psCScriptEngine::PrepareEngine()
{
	if (m_bIsPrepared) return;
  
	for( size_t n = 0; n < m_SystemFunctions.size(); n++ )
	{
		if ( m_SystemFunctions[n] )
		{
			// Determine the host application interface
			PrepareSystemFunction(m_SystemFunctions[n], m_SystemFunctionInterfaces[n], this);
		}
	}

	m_bIsPrepared = true;
}

int psCScriptEngine::ConfigError(int err)
{
	m_bConfigFailed = true; 
	return err;
}

void* psCScriptEngine::GetGlobalAddress(size_t idx) 
{ 
	assert(idx>=0 && idx < m_GlobalPropAddresses.size());

	return m_GlobalPropAddresses[idx];
}

void psCScriptEngine::SetDefaultContextStackSize(psUINT initial, psUINT maximum)
{
	m_InitContexStackSize = initial;
	m_MaxContexStackSize = maximum;
}

void psCScriptEngine::FindGlobalOperators(int token, std::vector<int>& ops)
{
	ops.clear();
	for (size_t i = 0; i < m_GlobalOperators.size(); i += 2)
	{
		if (m_GlobalOperators[i] == token)
			ops.push_back(m_GlobalOperators[i+1]);
	}
}

bool psCScriptEngine::CallObjectMethod(void *obj, void *param, int modId, int func)
{
	if ( func < 0 )
	{
		psSSystemFunctionInterface *i = m_SystemFunctionInterfaces[-func-1];
		if ( i == NULL )
			return false;

		if( i->callConv == ICC_THISCALL )
		{
			union
			{
				psSIMPLEMETHOD_t mthd;
				psFUNCTION_t func;
			} p;

			p.func = (void (*)())(i->func);
			void (psCSimpleDummy::*f)(void *) = (void (psCSimpleDummy::*)(void *))(p.mthd);
			(((psCSimpleDummy*)obj)->*f)(param);
		}else if ( i->callConv == ICC_THIS_STDCALL)
		{
			union
			{
				psSIMPLEMETHOD_t     mthd;
				psSTDCALL_FUNCTION_t func;
			} p;

			typedef void (PS_STDCALL psCSimpleDummy::* ThisStdCall_t)(void*);

			p.func = psSTDCALL_FUNCTION_t(i->func);
			ThisStdCall_t f = ThisStdCall_t(p.mthd);

			(((psCSimpleDummy*)obj)->*f)(param);
		}
		else /*if( i->callConv == ICC_CDECL_OBJFIRST )*/
		{
			assert( i->callConv == ICC_CDECL_OBJFIRST );

			void (*f)(void *, void *) = (void (*)(void *, void *))(i->func);
			f(obj, param);			
		}
	}else
	{
		psCContext ctx(this, false);
		if ( ctx.Prepare( modId, func, obj ) != psSUCCESS )
		{
			return false;
		}	

		// Set argument0
		ctx.SetArgPointer( 0, param );

		// 运行函数	
		if ( ctx.Execute(0) != psEXECUTION_FINISHED )
		{
			return false;
		}
	}	
	return true;
}

bool psCScriptEngine::CallObjectMethod(void *obj, int modId, int func)
{
	if ( func < 0 )
	{
		psSSystemFunctionInterface *i = m_SystemFunctionInterfaces[-func-1];

		if( i->callConv == ICC_THISCALL )
		{
			union
			{
				psSIMPLEMETHOD_t mthd;
				psFUNCTION_t func;
			} p;
			p.func = (void (*)())(i->func);
			void (psCSimpleDummy::*f)() = p.mthd;
			(((psCSimpleDummy*)obj)->*f)();
		}else if ( i->callConv == ICC_THIS_STDCALL)
		{
			union
			{
				psSIMPLEMETHOD_t mthd;
				psSTDCALL_FUNCTION_t func;
			} p;
			p.func = psSTDCALL_FUNCTION_t(i->func);

			typedef void (PS_STDCALL psCSimpleDummy::* ThisStdCall_t)();

			ThisStdCall_t f = (ThisStdCall_t)(p.mthd);
			(((psCSimpleDummy*)obj)->*f)();
		}
		else /*if( i->callConv == ICC_CDECL_OBJFIRST )*/
		{
			assert( i->callConv == ICC_CDECL_OBJFIRST );

			void (*f)(void *) = (void (*)(void *))(i->func);
			f(obj);
		}
	}else
	{
		psCContext ctx(this, false);
		if ( ctx.Prepare( modId, func, obj ) != psSUCCESS )
		{
			return false;
		}	

		// 运行函数	
		if ( ctx.Execute(0) != psEXECUTION_FINISHED )
		{
			return false;
		}
	}		
	return true;
}

unsigned int psCScriptEngine::GetObjectTypeCount() const
{
	return (unsigned int )( m_ObjectTypes.size() );
}

void psCScriptEngine::GetAllObjectTypes(psIObjectType* objectTypes[]) const
{
	assert( objectTypes );
	ObjectTypeMap_t::const_iterator ite = m_ObjectTypes.begin();
	ObjectTypeMap_t::const_iterator end = m_ObjectTypes.end();
	int i = 0;
	for ( ; ite != end; ++ite )
	{
		objectTypes[i++] = ite->second;
	}
}


psCObjectType* psCScriptEngine::_findObjectType( const psCHAR* name ) const
{
	ObjectTypeMap_t::const_iterator ite = m_ObjectTypes.find( name );
	if ( ite != m_ObjectTypes.end() )
		return ite->second;
	return NULL;
}

const psIObjectType* psCScriptEngine::FindObjectType(const psCHAR* name) const
{
	return (const psIObjectType*)( _findObjectType( name) );
}

const psIScriptFunction* psCScriptEngine::GetGlobalFunction(unsigned int index) const
{
	int funcId = m_GlobalFunctions[index];
	if (funcId < 0)
		return m_SystemFunctions[-funcId - 1];
	else
		return NULL;
}

const psIScriptFunction* psCScriptEngine::GetSystemFunction(int nID) const
{
	int index = -nID - 1;
	if( index < (int)m_SystemFunctions.size() )
		return m_SystemFunctions[index];
	else
		return NULL;
}

void psCScriptEngine::GetAllGlobalProps( psIProperty* pAllGlobalProps[] ) const
{	
	PropertyMap_t::const_iterator ite = m_GlobalProps.begin();
	PropertyMap_t::const_iterator end = m_GlobalProps.end();
	
	int i = 0;
	for ( ; ite != end; ++ite )
	{
		pAllGlobalProps[i] = ite->second;
		i++;
	}
}

const psIProperty* psCScriptEngine::FindGlobalProp(const psCHAR* name) const
{
	PropertyMap_t::const_iterator ite = m_GlobalProps.find( name );
	if ( ite != m_GlobalProps.end() )
		return ite->second;
	return NULL;
}


psIModule* psCScriptEngine::GetModule(const psCHAR* name) const
{
	if ( m_pLastModule && m_pLastModule->Name() == name )
	{
		return m_pLastModule;
	}

	ModuleNameMap_t::const_iterator it = m_ModuleNameMap.find( name );
	if (it != m_ModuleNameMap.end() )
	{
		assert(it->second < (int)m_ScriptModules.size() );
		psCModule* mod = m_ScriptModules[it->second];
		if ( mod )
			m_pLastModule = mod;

		return mod;
	}else
	{
		return NULL;
	}
}

psIModule* psCScriptEngine::GetModule(unsigned int index) const
{
	assert( index < m_ScriptModules.size() );
	return m_ScriptModules[index];
}

int psCScriptEngine::CreateModule(const psCHAR* _pName, const psCHAR* _pFileName, psIModule** ppmod)
{
	if ( ppmod == NULL )
		return psERROR;
	else
		*ppmod = NULL;

	const psCHAR *name = PS_T("");
	if( _pName != NULL )
	{
		name = _pName;
	}
	const psCHAR* filename = PS_T("");
	if ( _pFileName != NULL )
	{
		filename = _pFileName;
	}

	// 根据名称得到脚本模块
	psIModule* module = GetModule( name );
	if (module )
	{
		*ppmod = module;
		return psMODULE_EXIST;
	}

	// 创建一个新的模块
	int index = (int)m_ScriptModules.size();
	psCModule* new_module = new psCModule(name, index, this);
	m_ScriptModules.push_back(new_module);

	// 建立名称映射表
	psCHAR* pNewName = new psCHAR[ ps_strlen(name) + 1 ];
	ps_strcpy( pNewName, name );
	m_ModuleNameMap[pNewName] = index;

	// 更新最后被访问的模块指针
	m_pLastModule = new_module;

	// 输出脚本模块指针
	*ppmod = new_module;

	// 设置源文件路径
	new_module->SetFileName( filename );

	if ( m_pDebugger )
	{
		m_pDebugger->OnCreateModule( new_module );
	}
	return psSUCCESS;
}


int psCScriptEngine::RegisterStringFactory(const psCHAR* datatype, PS_GET_STRING_VALUE_FUNC getFunc, PS_SET_STRING_VALUE_FUNC setFunc)
{
	// 分析数据类型
	psCBuilder bld(this, NULL);
	psCDataType dt;
	int r = bld.ParseDataType(datatype, &dt);
	if( r < 0 ) 
	{
		return ConfigError(psINVALID_TYPE);
	}

	// 必需是一个类对象类型
	if ( dt.GetObjectType() == NULL )
		return psINVALID_TYPE;

	// 注册字作串处理工厂
	STRING_FACTORY factory;
	factory.pGetValueFuncPointer = getFunc;
	factory.pSetValueFuncPointer = setFunc;
	m_StringFactory[ dt.ObjectType() ] = factory;

	return psSUCCESS;
}

bool psCScriptEngine::IsStringType(const psIObjectType* objType) const
{
	StringFactoryMap_t::const_iterator it = m_StringFactory.find( objType );
	if ( it != m_StringFactory.end() )
		return true;
	else
		return false;
}

bool psCScriptEngine::SetStringValue(const psIObjectType* objType, void* obj, const psCHAR* value) 
{
	StringFactoryMap_t::iterator it = m_StringFactory.find( objType );
	if ( it != m_StringFactory.end() )
	{
		PS_SET_STRING_VALUE_FUNC funcPtr = it->second.pSetValueFuncPointer;
		assert( funcPtr );
		(*funcPtr)(obj, value);
		return true;
	}else
	{
		return false;
	}
}

psAutoStringPtr psCScriptEngine::GetStringValue(const psIObjectType* objType, void* obj) const
{
	assert(obj);
	StringFactoryMap_t::const_iterator it = m_StringFactory.find( objType );
	if ( it != m_StringFactory.end() )
	{
		PS_GET_STRING_VALUE_FUNC funcPtr = it->second.pGetValueFuncPointer;
		assert( funcPtr );
		return (*funcPtr)( obj );
	}
	else
	{
		return NULL;
	}
}

void* psCScriptEngine::AllocObject( int moduleId, const psIObjectType* objType ) 
{
	assert( objType );

	// Call the object's default constructor
	char *obj = (char*)malloc(objType->GetSizeInMemoryBytes());
	assert( obj );

	if ( !CallObjectMethod( obj, moduleId, objType->GetTypeBehaviour()->GetConstructFuncId() ) )
	{
		free( obj );
		return NULL;
	}else
	{
		return obj;
	}
}


bool psCScriptEngine::FreeObject( void* obj, int moduleId, const psIObjectType* objType  )
{
	if ( objType && obj )
	{
		bool r = CallObjectMethod( obj, moduleId, objType->GetTypeBehaviour()->GetDestructFuncId() );
		free( obj );
		return r;
	}else
	{
		assert(false);
		return false;
	}
}


bool psCScriptEngine::SetMacroList(const psCHAR* macros, psCHAR sperator /* = ';' */)
{
	if (macros == NULL)
		macros = PS_T("");
	
	m_MacroList.clear();
	int length = (int)ps_strlen(macros);

	psCString strMacroList(PS_T(""));
	psCHAR* strMacro = new psCHAR[length+2];

	int pos, priorPos = 0;
	for (pos = 0; pos <= length; ++pos)
	{
		if (macros[pos] == sperator || macros[pos] == PS_T('\0'))
		{
			if (pos - priorPos > 0)
			{
				int count = 0;
				for (int i = priorPos; i < pos; ++i)
					if (macros[i] != PS_T('\t') && macros[i] != PS_T(' '))
					{
						strMacro[count++] = macros[i];
					}
					strMacro[count] = PS_T('\0');
					m_MacroList.push_back( psCString(strMacro) );

					strMacro[count] = sperator;
					strMacro[count+1] = PS_T('\0');
					strMacroList += strMacro;
					priorPos = pos + 1;
			}
		}
	}

	delete []strMacro;

	if (strMacroList == m_strMacros)
	{
		return false;
	}
	else
	{
		m_strMacros = strMacroList;
		return true;
	}
}

const psCHAR* psCScriptEngine::GetMacroList()
{
	return m_strMacros.AddressOf();
}
