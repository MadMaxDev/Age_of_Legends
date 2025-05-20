#ifndef __PS_SCRIPTENGINE_H__
#define __PS_SCRIPTENGINE_H__

#include "config.h"
#include "objecttype.h"
#include "callfunc.h"
#include "context.h"
#include "macro.h"

class psCScriptEngine: public psIScriptEngine
{
	//----------------------------------------------------------------------//
	// 实现基类的方法
	//----------------------------------------------------------------------//
public:
	// 内存管理
	virtual long AddRef();
	virtual long Release();

	// 注册一个C++类('name'为其在脚本中使用的类名, 'byteSize'为其大小, 'flags'为其类的行为标志) 
	virtual int RegisterObjectType(const psCHAR* name, int byteSize, psDWORD flags);

	// 注册C++类的成员属性('objname'为其在脚本中使用的类名, 'declaration'为其成员变量的声明, 'byteOffset'为其在C++类中的偏移)
	virtual int RegisterObjectProperty(const psCHAR* objname, const psCHAR* declaration, int byteOffset);

	// 注册C++类的成员方法('objname'为其在脚本中使用的类名, 'declaration'为其成员函数的声明, funcPointer为其函数地址值, 'callConv'表示其调用方式)
	virtual int RegisterObjectMethod(const psCHAR* objname, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv);

	// 注册C++类的行为('objname'为其在脚本中使用的类名, 'behavior'为类行为修饰, 'decaration'为其函数声明, funcPointer为其函数地址值, 'callConv'表示其调用方式)
	virtual int RegisterObjectBehaviour(const psCHAR* objname, psDWORD behaviour, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv);

	// 注册一个全局变量('declaration'为其声明, 'pointer'为其地址值
	virtual int RegisterGlobalProperty(const psCHAR* declaration, void* pointer);

	// 注册一个整型常量
	virtual int RegisterIntConstant(const psCHAR* name, int v);

	// 注册一个浮点常量
	virtual int RegisterFloatConstant(const psCHAR* name, float v);

	// 注册一个全局函数('declaration'为其声明, 'funcPointer'为其函数地址值, 'callConv'表示其调用方式)
	virtual int RegisterGlobalFunction(const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv);

	//  注册一个全局操作符重载('behavior'为操作重载修饰, 'decl'为其声明, 'funcPointer'为其函数地址值, 'callConv'表示其调用方式)
	virtual int RegisterGlobalBehaviour(psDWORD behaviour, const psCHAR* decl, psUPtr funcPointer, psDWORD callConv);

	// 声明类的继承关系
	// 参数: 'name'类名
	//		 'parent'父类名
	//		 'baseOffset'C++中子类到父类的偏移
	virtual int DeclareObjectHierarchy(const psCHAR* name, const psCHAR* parent, int baseOffset );
		
	// 注销一个C++类
	virtual int UnregisterObjectType( const psCHAR* name );

	// 注销一个全局变量
	virtual int UnregisterGlobalProperty(void* ptr);

	// 注销一个全局函数
	virtual int UnregisterGlobalFunction( psUPtr funcPtr );

	// 注销一个全局操作符重载
	virtual int UnregisterGlobalBehaviour( psUPtr funcPtr );

	// 获取类对象类型的个数
	virtual unsigned int GetObjectTypeCount() const;

	// 获取所有的类对象类型
	virtual void GetAllObjectTypes(psIObjectType* objectTypes[]) const;

	// 根据名称得到注册的C++类的类型接口, 这个函数可能会失败
	virtual const psIObjectType* FindObjectType(const psCHAR* name) const;

	// 根据函数ID,得到系统函数接口
	virtual const psIScriptFunction* GetSystemFunction(int nID) const;

	// 得到系统全局函数的个数
	virtual unsigned int GetGlobalFunctionCount() const { return (unsigned int)m_GlobalFunctions.size(); }

	// 根据函数索引,得到系统全局函数接口
	virtual const psIScriptFunction* GetGlobalFunction(unsigned int index) const;

	// 得到全局属性的个数
	virtual unsigned int GetGlobalPropCount() const { return (unsigned int)m_GlobalProps.size();	}

	// 获取所有的全局属性
	virtual void GetAllGlobalProps( psIProperty* pAllGlobalProps[] ) const;

	//  根据名称得到全局变量属性接口
	virtual const psIProperty* FindGlobalProp(const psCHAR* name) const;

	// 得到脚本模块的总数
	virtual unsigned int GetScriptModuleCount() const { return (unsigned int)m_ScriptModules.size(); }

	// 根据索引得到脚本模块接口
	virtual psIModule* GetModule(unsigned int index) const;

	// 根据模块名称得到脚本模块接口
	virtual psIModule* GetModule(const psCHAR* name) const;

	virtual int CreateModule(const psCHAR* name, const psCHAR* filename, psIModule** mod);

	// 废弃所有的脚本模块
	virtual void Reset();

	// 丢弃一个脚本模块
	virtual int Discard(const psCHAR* module);

	// 设置代码优化选项
	virtual void SetOptimizeOptions( psSOptimizeOptions options ) { m_OptimizeOptions = options; }

	// 得到代码优化选项
	virtual psSOptimizeOptions GetOptimizeOptions() const		  { return m_OptimizeOptions; }

	// 编译一个脚本模块
	virtual int Compile(  psIModule* mod, psITextStream* pTextStream, psIOutputStream* pOutStream );

	// 执行一个字符串
	virtual int ExecuteString( psIScriptContext* ctx, const psCHAR *script, psIOutputStream* pOutStream );

	// 设轩默认的堆栈大小
	virtual void SetDefaultContextStackSize(psUINT initial, psUINT maximum);

	// 创建脚本运行上下文接口
	virtual int CreateContext(psIScriptContext **contex);

	virtual void SetDebugger( psIDebugger* debugger )  { m_pDebugger = debugger; }

	// 注册一个字符串处理工厂('dataType'为类名,'getFunc'为如何从一个类对象得到字符串值的函数指针, 'setFunc'为如何从一个字符串指针赋值 )
	virtual int  RegisterStringFactory(const psCHAR* dataType, PS_GET_STRING_VALUE_FUNC getFunc, PS_SET_STRING_VALUE_FUNC setFunc);

	// 判断一个对象类型是否为字符串类型
	virtual bool IsStringType(const psIObjectType* objType) const;

	// 设置一个字符串类对象的值
	virtual bool  SetStringValue(const psIObjectType* objType, void* obj, const psCHAR* value);

	// 得到一个字符串类对象的值
	virtual psAutoStringPtr GetStringValue(const psIObjectType* objType, void* obj) const;

	// 分配一个类对象
	virtual void* AllocObject( int moduleId, const psIObjectType* objType );

	// 释放一个类对象
	virtual bool FreeObject( void* obj, int moduleId, const psIObjectType* objType  );

	// 设置宏定义列表字符串
	virtual bool SetMacroList(const psCHAR* macros, psCHAR sperator);

	// 得到宏定义列表字符串
	virtual const psCHAR* GetMacroList();

	// 设置变参函数类型检查回调函数
	virtual void SetArgTypeCheckFunc( PS_ARGTYPE_CHECK_FUNC pFunc ) { m_pArgTypeCheckFunc = pFunc; }

	// 得到变参函数类型检查回调函数
	virtual PS_ARGTYPE_CHECK_FUNC GetArgTypeCheckFunc()				{ return m_pArgTypeCheckFunc; }
public:	
	//----------------------------------------------------------------------//
	// 内部函数
	//----------------------------------------------------------------------//
	psCScriptEngine(void);
	virtual ~psCScriptEngine(void);

	psCObjectType* _findObjectType( const psCHAR* name ) const;

	void AddSystemFunction(psCScriptFunction* &func, psSSystemFunctionInterface* internal);
	void DeleteSystemFunctionById( int funcId );

	const std::vector<int>& GetSystemFuncIDByName( const psCString& name ) const;

	bool IsFunctionEqual(psCScriptFunction* f0, psCScriptFunction* f1);

	psSSystemFunctionInterface* GetSystemFunctionInterface(int index) { return m_SystemFunctionInterfaces[index]; }

	unsigned int GetSystemFunctionCount() const			  { return (unsigned int)m_SystemFunctions.size();  }
	inline psCScriptFunction* GetSystemFunctionByIndex(int index) 
	{ 
		assert( index < (int)m_SystemFunctions.size() && index >= 0 );	
		return m_SystemFunctions[index];
	}
	inline psCScriptFunction* GetSystemFunctionByID( int nID )
	{
		return GetSystemFunctionByIndex( -1 - nID );
	}
	
	void* GetGlobalAddress(size_t idx);

	void FindGlobalOperators(int op, std::vector<int>& ops);

	bool CallObjectMethod(void *obj, int modId, int func);
	bool CallObjectMethod(void *obj, void *param, int modId, int func);
	bool CallArrayObjMethod( void* obj, int modId, int func );

	size_t GetNumMacros() const						{ return m_MacroList.size(); }
	const psCString& GetMacroName(size_t idx) const { return m_MacroList[idx];   }

	int GetInitialContextStackSize() const			{ return m_InitContexStackSize;  }
	int GetMaxContextStackSize()     const			{ return m_MaxContexStackSize;   }

	void PrepareEngine();

	psCObjectType *GetObjectType(const psCHAR *type);

	int AddBehaviourFunction(psCScriptFunction &func, psSSystemFunctionInterface &internal);

	int ConfigError(int err);

	// 判断操作符能否可以被重载
	static bool IsOpAllowOverload(int op, bool object);

	// 脚本模块
	psCModule* GetLastModule() const		{ return m_pLastModule;			 }
	psCModule* GetStringModule() const		{ return m_pStringModule;		 }

	void SetLastModule(psCModule* module)   { m_pLastModule = module;}
	void ClearModule(int index)				{ m_ScriptModules[index] = NULL; }

	psIDebugger* GetDebugger() const		{ return m_pDebugger; }

	typedef std::vector<psCString>			 MacroList_t;
private:
	struct STRING_FACTORY
	{
		PS_GET_STRING_VALUE_FUNC	pGetValueFuncPointer;
		PS_SET_STRING_VALUE_FUNC	pSetValueFuncPointer;
	};
	struct stringptr_lesser
	{
		bool operator()(const psCHAR* lhs, const psCHAR* rhs ) const
		{
			return (ps_strcmp(lhs, rhs) < 0);
		}
	};
	typedef std::map< const psCHAR*, int, stringptr_lesser >			 ModuleNameMap_t;
	typedef	std::map< const psIObjectType*, STRING_FACTORY>				 StringFactoryMap_t;	
	typedef std::map< psCString, std::vector< int > >					 FuncNameMap_t;
	typedef std::map< const psCHAR*, psCObjectType*, stringptr_lesser >  ObjectTypeMap_t;
	typedef std::map< const psCHAR*, psSProperty*, stringptr_lesser >	 PropertyMap_t;

	int										    m_InitContexStackSize;					// 初始堆栈的大小
	int											m_MaxContexStackSize;					// 最大的堆栈大小

	// 脚本注册的信息
	StringFactoryMap_t							m_StringFactory;						// 字符串工厂
	std::vector< int >							m_GlobalOperators;						// 全局操作符的行为属性
	ObjectTypeMap_t								m_ObjectTypes;							// 所有的类对象类型
	PropertyMap_t								m_GlobalProps;							// 全局变量属性
	std::vector< void* >						m_GlobalPropAddresses;					// 全局变量地址
	std::vector< psCScriptFunction* >			m_SystemFunctions;						// 系统函数
	std::vector< psSSystemFunctionInterface* >  m_SystemFunctionInterfaces;				// 系统函数接口
	std::vector< int >							m_FreeFunctionId;						// 空余的函数ID
	std::vector< int >							m_GlobalFunctions;						// 全局函数ID
	FuncNameMap_t								m_FuncNameMap;							 
	psIDebugger								   *m_pDebugger;

	bool										m_bConfigFailed;						// 脚本配置是否有错
	bool										m_bIsPrepared;							// 引擎是否已经准备好了

	psCAtomicCount								m_RefCount;								// 引用计数
	std::vector<psCModule *>					m_ScriptModules;						// 所有的脚本模块
	ModuleNameMap_t								m_ModuleNameMap;						//
	mutable psCModule							*m_pLastModule;							// 最近访问的脚本模块
	psCModule									*m_pStringModule;						// 动行字符串模块
	psCContext									*m_pDefaultCtx;							// 默认的运行上下文

	psSOptimizeOptions							 m_OptimizeOptions;

	// 宏定义
	MacroList_t m_MacroList;
	psCString m_strMacros;

	PS_ARGTYPE_CHECK_FUNC						 m_pArgTypeCheckFunc;					// 变参函数编译类型检查的回调
	friend class psCRestore;
};

#endif // __PS_SCRIPTENGINE_H__


