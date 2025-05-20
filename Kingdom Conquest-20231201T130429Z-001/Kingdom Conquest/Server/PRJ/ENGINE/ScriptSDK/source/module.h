#ifndef __PS_MODULE_H__
#define __PS_MODULE_H__

#if defined(_MSC_VER)
	#pragma warning(disable : 4786)
#endif

#include "config.h"
#include "string.h"
#include "datatype.h"
#include "objecttype.h"
#include "threadmanager.h"
#include "scriptfunction.h"

const int psFUNC_INIT		 = -1;
const int psFUNC_EXIT		 = -2;
const int psFUNC_STRING		 = -4;

class psCModule: public psIModule
{
public:
	psCModule(const psCHAR* name, int id, psCScriptEngine* engine);
	virtual ~psCModule(void);
	
public:
	//----------------------------------------------------------------------//
	// 实现基类接口的函数
	//----------------------------------------------------------------------//
	// 得到模块名
	virtual const psCHAR* GetName() const { return m_Name.AddressOf(); }

	// 得到文件名
	virtual const psCHAR* GetFileName() const { return m_FileName.AddressOf(); }

	// 得到模块ID
	virtual int GetID() const			{ return m_ModuleId;		  }

	// 添加一个脚本代码段
	virtual int SetCode(const psCHAR* code, int length);

	// 获取源代码
	virtual const psCHAR* GetSourceCode() const;

	// 得到模块中的函数总数
	virtual unsigned int GetFunctionCount() const { return (unsigned int)m_ScriptFunctions.size(); }

	// 根据函数ID. 得到脚本函数接口
	virtual const psIScriptFunction* GetFunctionByID(int nID) const;

	virtual const psIScriptFunction* GetInitFunction() const { return m_pInitFunction; }
	virtual const psIScriptFunction* GetExitFunction() const { return m_pExitFunction; }

	// 根据函数名称， 得到脚本函数ID
	virtual int GetFunctionIDByName(const psCHAR* name) const;

	// 根据函数声明. 得到脚本函数的ID
	virtual int GetFunctionIDByDecl(const psCHAR* decl) const;

	// 得到类对象的个数
	virtual unsigned int GetObjectTypeCount() const { return (unsigned int)m_ObjectTypes.size(); }

	// 根据类对象索引得到类对象
	virtual const psIObjectType* GetObjectType(unsigned int i) const {assert(i<m_ObjectTypes.size()); return m_ObjectTypes[i]; }

	// 根据类对象名称得到类对象
	virtual const psIObjectType* GetObjectType(const psCHAR* name) const { return FindObjectType(name); }

	// 得到全局变量的总数
	virtual unsigned int GetGlobalVarCount() const;

	// 得到全局变量属性
	virtual const psIProperty* GetGlobalVar(int index) const;

	// 保存二进制码
	virtual int SaveByteCode(psIBinaryStream* out) const;

	// 装载二进制码
	virtual int LoadByteCode(psIBinaryStream* in);

	// 切换断点
	virtual bool ToggleBreakPoint( int lineno, bool& isBreakPoint );

	// 插入用户断点
	virtual bool InsertBreakPoint( int lineno);

	// 移除用户断点
	virtual bool RemoveBreakPoint( int lineno);

	// 移除所有的用户断点
	virtual void RemoveAllBreakPoint();

	// 得到所有的断点信息
	virtual psIBuffer* GetAllBreakPoint( int* numBreakPoints );
public:
	//----------------------------------------------------------------------//
	// 内部使用的函数
	//----------------------------------------------------------------------//
	void Reset();
	void Destroy();
	int  Build(psITextStream* pTextStream, psIOutputStream* pOutStream);

	// 内存管理
	int AddContextRef()		 { return ++m_ContexRefCount;		}
	int ReleaseContextRef()  { return --m_ContexRefCount;		}
	bool CanReset() const     { return (m_ContexRefCount == 0 ); }

	// 初始化/析构全局变量
	bool CallInit();
	void CallExit();

	// 获取函数相关信息
	int GetNextFunctionId()			const	{ return int(m_ScriptFunctions.size());						}

	psCScriptFunction* GetScriptFunction(unsigned int idx) const;
	psCScriptFunction* GetSpecialFunction(int funcID);

	// 获取全局变量信息
	int  GetGlobalVarIDByName(const psCHAR *name);
	int  GetGlobalVarIDByDecl(const psCHAR *decl);

	// 添加脚本函数
	psCScriptFunction* AddScriptFunction(int id, const psCHAR* name, psCDataType returnType, std::vector<psCDataType>& paramTypes);
	int	AddScriptFunction(psCScriptFunction* func) { m_ScriptFunctions.push_back( func ); return (int)m_ScriptFunctions.size() - 1; }

	// 添加常量
	int AddConstantDouble( double value);
	int AddConstantInt( int value);
	int AddConstantFloat(float value);
	int AddConstantStrA( const char* str );
	int AddConstantStrW( const wchar_t* str );
	
	// 得到常数池和全局数据池
	psBYTE* GetGlobalMemPtr() const   { return m_GlobalMem; }
	psBYTE* GetConstantMemPtr() const { return m_ConstantMem; }
	int GetConstantMemSize() const    { return m_ConstantMemSize;}
	int GetGloablMemSize() const	  { return m_GlobalMemSize; }

	int  AddConstMemSize( int s );

	// 分配全局空间
	int	 AllocGlobalMemory(const psCDataType& type);
	void DeallocGlobalMemory(const psCDataType& type);

	void AddObjectType(psCObjectType* pObjType)		 { m_ObjectTypes.push_back( pObjType );  }
	void AddGlobalVar(psSProperty* prop)			 { m_ScriptGlobals.push_back(prop);		 }

	bool IsGlobalVarInitialized() const				 { return m_IsGlobalVarInitialized;		 }

	const psCString& Name() const					 { return m_Name;			   }
	void SetFileName( const psCHAR* filename )		 { m_FileName = filename;	   }
	psSProperty* GlobalVar(size_t id) const			 { return m_ScriptGlobals[id]; }

	// 得到脚本的对象类型
	psCObjectType* FindObjectType(const psCHAR* name) const;
	int GetObjectTypeIdx(const psCObjectType* objType);

	// 查找全局操作符重载
	void FindGlobalOperators(int op, std::vector<int>& funcs);
	void AddGlobalOperator(int op, int func);

	// 得到脚本源码
	psCScriptCode* GetCode()						 { return m_pCode; }

	// 给Restore（直接载入ByteCode文件）功能的接口
	void SetModuleRestoreSuccess()						{m_IsBuildWithoutErrors=true;}
private:	
    std::vector<psCScriptFunction*>			 m_ScriptFunctions;			// 脚本函数
	std::vector<psSProperty*>				 m_ScriptGlobals;			// 脚本全局部量

	psBYTE*									 m_GlobalMem;				// 全局变量空间
	psBYTE*									 m_ConstantMem;				// 常数池	
	int										 m_ConstantMemSize;
	int										 m_ConstantMemCapacity;
	int										 m_GlobalMemSize;
	std::map< int, int >					 m_IntConstants;			// 分配了的整型常数池	
	std::map< float, int >					 m_FloatConstants;			// 分配了的单精度浮点型常数池
	std::map< double, int >					 m_DoubleConstants;			// 分配了的双精度浮点型常数池
	std::map< psCStringA, int >				 m_StrAConstants;			// Ansi文字常量列表 
	std::map< psCStringW, int >				 m_StrWConstants;			// Unicode文字常量列表 

	std::vector<psCObjectType*>				 m_ObjectTypes;				// 脚本的对象类型
	std::vector<int>						 m_GlobalOperators;			// 脚本里的全局操作符重载

	psCScriptFunction						*m_pInitFunction;			// 初始化全局部量的函数
	psCScriptFunction						*m_pExitFunction;			// 
	psCScriptFunction						*m_pStringFunction;			// 字符串运行函数

	psCString								 m_Name;					// 模块名称
	psCString								 m_FileName;				// 文件名
	psCScriptCode*							 m_pCode;					// 脚本源代码
	int										 m_ModuleId;				// 模块ID
	bool									 m_IsBuildWithoutErrors;	// 是否构建成功
	bool									 m_IsGlobalVarInitialized;  // 全局变量是否已经初始化
	psCScriptEngine*						 m_pEngine;					// Engine

	psCAtomicCount							 m_ContexRefCount;			// 引用计数

	friend class psCRestore;
};

#endif // __PS_MODULE_H__
