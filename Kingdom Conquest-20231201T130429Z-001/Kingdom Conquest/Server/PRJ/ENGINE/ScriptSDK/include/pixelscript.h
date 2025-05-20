// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : 
// File         : pixelscript.h
// Creator      : 龚静
// Comment      : 如果要包含系统文件的话，就包含这个
// CreationDate : 2006-04-21
// ChangeLOG    : 2006-09-22 修改psAutoStringPtr为带有引用计数和拷贝构造函数的版本。

#ifndef __PIXELSCRIPT_H__
#define __PIXELSCRIPT_H__

#include <assert.h>
#include "types.h"

// 脚本引擎的版本信息
#define PIXELSCRIPT_VERSION        20000
#define PIXELSCRIPT_VERSION_MAJOR  2
#define PIXELSCRIPT_VERSION_MINOR  0
#define PIXELSCRIPT_VERSION_BUILD  0
#define PIXELSCRIPT_VERSION_STRING "2.0.0"

// 函数指针类型
typedef void (* psFUNCTION_t)();
typedef void (PS_STDCALL* psSTDCALL_FUNCTION_t)();

// 类的成员函数指针类型
class psUnknownClass;
typedef void (psUnknownClass::*psMETHOD_t)();

// 联合函数指针类型和类的成员函数指针类型
union psUPtr
{
	psMETHOD_t   mthd;
	psFUNCTION_t func; 
};

//-------------------------------------------------------------------------------------------------------------------//
// 解析函数地址值的宏定义
// 'f''表示函数名或重载操作符名(i.e. operator = )
// 'p' 表示函数的参数说明(i.e. int , int)
// 'r' 表示函数的返回值类型(i.e. int )
// 'c' 表示类名
// 'm' 表示成员函数名或重载操作符名
// 列如: 在C++中有全局函数: void func();					其地址值为: psFUNCTION(func)
//						    int func(int a, int b);			其地址值为: psFUNCTIONPR(func, (int a, int b), int)
//							int operator + (int a, int b);  其地址值为: psFUNCITONPR(operator +, (int a, int b), int)
//							
//		在C++中有类: class CTest
//					 {
//						void func();						其地址值为: psMETHOD(CTest, func)
//						int  func1(int a);					其地址值为: psMETHODPR(CTest, func1,(int a), int)
//						int operator += (int a);			其地址值为: psMETHODPR(CTest, operator+=, (int a), int)
//					 };
//-------------------------------------------------------------------------------------------------------------------//
#define psFUNCTION(f)		  psFunctionPtr((void (*)())(f))
#define psFUNCTIONPR(f,p,r)	  psFunctionPtr((void (*)())((r (*)p)(f)))
#define psMETHOD(c,m)		  psSMethodPtr<sizeof(void (c::*)())>::Convert((void (c::*)())(&c::m))
#define psMETHODPR(c,m,p,r)	  psSMethodPtr<sizeof(void (c::*)())>::Convert((r (c::*)p)(&c::m))
#define psBASEOFFSET(c, base) ( (int)( (base*)((c*)1) ) - 1 )  

#ifdef _DEBUG
	#define PS_VERIFY(e) assert(e >= 0)
#else
	#define PS_VERIFY(e) (e)
#endif

// 如果要把脚本引擎编译为动态链接库要定义宏: PIXELSCRIPT_EXPORT
// 如果是显示链接脚本引擎库,要定义宏: PIXELSCRIPT_DLL_LIBRARY_IMPORT 
// 如果是静态链接,PS_API被定义为空
#ifdef WIN32
  #ifdef PIXELSCRIPT_STATIC
	#define PS_API
  #else
	#ifdef PIXELSCRIPT_EXPORTS
		#define PS_API __declspec(dllexport)
	#else
		#define PS_API __declspec(dllimport)
	#endif
  #endif
#else
  #define PS_API
#endif

// 最多支持变参参数个数
const int PS_MAX_VAR_ARG_SIZE = 32;		

//---------------------------------------------------------------------//
// 基址方式
//----------------------------------------------------------------------//
const int PSBASE_NONE	  = 0;	// 操作数直接作为值来使用
const int PSBASE_CONST	  = 1;	// 操作数纪录常数在常数池中的索引
const int PSBASE_STACK	  = 2;	// 操作数相对于堆栈的索引
const int PSBASE_GLOBAL	  = 3;	// 操作数相对于全局对象的索引
const int PSBASE_HEAP	  = 4;  // 操作数相对于类对象的索引
const int PSBASE_UNKNOWN  = 5;  // 
//---------------------------------------------------------------

//----------------------------------------------------------------------//
// 代码生的优化选项
//----------------------------------------------------------------------//
struct psSOptimizeOptions
{
	psSOptimizeOptions()
	{
		bDelDebugInstr		= 1;
		bCombinedInstr	    = 1;
		bDelUnnessnaryInstr = 1;
		bDelUnusedInstr	    = 0;
		bCreateSymbolTable  = 0;
		bUnused				= 0;
	}
	psDWORD bCreateSymbolTable  : 1;		// 是否生成符号表(DEBUG版应该为真)
	psDWORD bDelDebugInstr	    : 1;		// 是否删除调试相关的指今(RELEASE版应该为真)
	psDWORD bCombinedInstr	    : 1;		// 是否合并指今(RELEASE版应该为真)
	psDWORD bDelUnnessnaryInstr : 1;		// 是否删除不必要的指今(RELEASE版应该为真)
	psDWORD bDelUnusedInstr		: 1;		// 是否删除没有用的指令(RELEASE版应该为真)
	psDWORD bUnused				: 27;
};

class psIDataType;
class psIScriptEngine;
class psIScriptContext;
class psIDebugger;

//-------------------------------------------------------------//
// 类名: psIBuffer
// 说明: 内存数据接口,可以在不同的模块间传递
//-------------------------------------------------------------//
class psIBuffer
{
public:
	virtual psBYTE* GetBufferPointer()  = 0;
	virtual void	Release()			= 0;
	virtual void	AddRef()			= 0;
protected:
	virtual ~psIBuffer() {}
};

//----------------------------------------------------------------------//
// 全局函数
//----------------------------------------------------------------------//
extern "C"
{
	// 创建脚本引擎接口
	psIScriptEngine * psCreateScriptEngine(psDWORD version);

	// 得到脚本引擎版本的字符串说明
	const char* psGetLibraryVersion();

	// 清除被管理的内存和全局数据(避免VC++报内存遗漏)
	void psCleanup();

	// 得到当前活动的脚本上下文
	psIScriptContext * psGetActiveContext();

	// 得到二进制码的反汇编代码
	// 参数: 'bc'[in]: 二进制代码
	//		 'size'[in] : 二进制代码的大小
	//		 'outLineNumber'[out]: 输出行号信息, 排列方式如下:[行号,位置,行号,位置...]
	//		'outNumLines':输出反汇编代码的行数
	// 返回值: 反汇编代码
	psIBuffer* psGetDisassembleCode( const psBYTE* bc, size_t size, psIBuffer** outLineNumbers, int* outNumLines );

	// 得到标记符的字符串定义
	const psCHAR* psGetTokenDefinition(int tokenType);
}

//-------------------------------------------------------------//
// 类名: psAutoStringPtr
// 说明: 能自动删除内存的字符串指针
//-------------------------------------------------------------//
class psAutoStringPtr
{
public:
	psAutoStringPtr( psIBuffer* p ) : m_pStringBuf(p)
	{
	}
	~psAutoStringPtr()
	{
		if ( m_pStringBuf )
		{
			m_pStringBuf->Release();
		}
	}
	psAutoStringPtr( const psAutoStringPtr& rhs )
	{
		m_pStringBuf = rhs.m_pStringBuf;
		m_pStringBuf->AddRef();
	}
	psAutoStringPtr& operator = (const psAutoStringPtr& rhs)
	{
		if ( this == &rhs )
			return *this;
		m_pStringBuf = rhs.m_pStringBuf;
		m_pStringBuf->AddRef();
		return *this;
	}
	operator const psCHAR* () 
	{
		return c_str();
	}
	const psCHAR* c_str() 
	{
		if ( m_pStringBuf )
			return (psCHAR*)(m_pStringBuf->GetBufferPointer());
		else
			return PS_T("");
	}
private:
	psIBuffer* m_pStringBuf;
};

//-------------------------------------------------------------------
// 类名: psIOutputStream
// 说明: 输出流的接口定义(用于输出警告和错误信息)
//-------------------------------------------------------------------
class psIOutputStream
{
public:
	virtual void Write(const psCHAR* text) = 0;
	virtual void WriteWarning(const psCHAR* text, const psCHAR* filename, int row, int col) = 0;
	virtual void WriteError(const psCHAR* text, const psCHAR* filename, int row, int col) = 0;
protected:
	virtual ~psIOutputStream() {}
};

//-------------------------------------------------------------------
// 类名: psITextStream
// 说明: 文本输入输出流
//-------------------------------------------------------------------
class psITextStream
{
public:
	virtual		bool	Open( const psCHAR* filename, const psCHAR* mode ) = 0;

	virtual		void    Close() = 0;

	virtual		long	GetLength() = 0;

	virtual		long    Read( psCHAR* buf, long length ) = 0;

	virtual		long    Write( psCHAR* buf, long length ) = 0;
protected:
	virtual ~psITextStream() {}
};

//-------------------------------------------------------------------
// 类名: psIBinaryStream
// 说明: 二进制流的接口定义(用于保存数据)
//-------------------------------------------------------------------
class psIBinaryStream
{
public:
	virtual void Read(void* ptr, unsigned int size) = 0;
	virtual void Write(const void* ptr, unsigned int size) = 0;
protected:
	virtual ~psIBinaryStream() {}
};


//-------------------------------------------------------------------
// 类名: psIProperty
// 说明: 属性接口(有可能是全局属性,也有可能为类成员的属性
//-------------------------------------------------------------------
class psIProperty
{
public:
	// 得到属性名
	virtual const psCHAR* GetName() const = 0;

	// 得到属性偏移
	virtual int GetOffset() const = 0;

	// 得到属性的数据类型
	virtual const psIDataType* GetDataType() const = 0;
protected:
	virtual ~psIProperty() {}
};

//-------------------------------------------------------------------
// 类名: psITypeBehaviour
// 说明: 类的行为描述接口
//-------------------------------------------------------------------
class psITypeBehaviour
{
public:
	// 得到构造函数的个数
	virtual unsigned int GetNumConstructFuncs() const = 0;

	// 得到构造函数的ID
	virtual int GetConstructFuncId(unsigned int index) const = 0;

	// 得到默认的构造函数ID		
	virtual bool HasConstructor() const	= 0;
	virtual int GetConstructFuncId() const = 0;

	// 得到默认的拷贝构造函数ID	
	virtual bool HasCopyConstructor() const	   = 0;
	virtual int GetCopyConstructFuncId() const = 0;

	// 得到析构函数的ID	
	virtual bool HasDestructor() const	  = 0;
	virtual int GetDestructFuncId() const = 0;

	// 得到初重载的操作符的个数
	virtual unsigned int GetNumOperators() const = 0;

	// 得到重载的操作符的标记符
	virtual int GetOperatorToken(unsigned int index) const = 0;

	// 得到重载操作符的函数ID
	virtual int GetOperatorFuncId(unsigned int index) const = 0;
protected:
	virtual ~psITypeBehaviour() {}
};

//-------------------------------------------------------------------
// 类名: psIObjectType
// 说明: 类对象类型接口 
//-------------------------------------------------------------------
class psIObjectType
{
public:
	// 得到类名
	virtual const psCHAR* GetName() const = 0;

	// 得到这个类在内存中的大小
	virtual int GetSizeInMemoryBytes() const = 0;

	// 得到类方法的个数
	virtual unsigned int GetNumMethods() const = 0;

	// 得到类方法的ID
	virtual int GetMethodFuncId(unsigned int index) const = 0;

	// 得到类成员属性的个数
	virtual unsigned int GetNumProps() const = 0;

	// 得到类成员属性
	virtual const psIProperty* GetProperty(unsigned int index) const = 0;

	// 得到类的行为描述接口
	virtual const psITypeBehaviour* GetTypeBehaviour() const = 0;

	// 是否为脚本的类对象
	virtual bool IsScriptObject() const = 0;

	// 得到父类类型
	virtual psIObjectType* GetParent() const = 0;

	// 父类到子类的偏移
	virtual int GetBaseOffset() const = 0;

	// 得到数据类型
	virtual const psIDataType* GetDataType() const = 0;
protected:
	virtual ~psIObjectType() {}
};

//-------------------------------------------------------------------
// 类名: psIDataType
// 说明: 数据类型接口
//-------------------------------------------------------------------
class psIDataType
{
public:
	// 得到这个数据类型的类对象类型接口
	virtual const psIObjectType* GetObjectType() const = 0;
	
	// 得到这个类型的字符串说明
	virtual psAutoStringPtr GetFormatString(const psCHAR* varname) const = 0;

	// 是否为指针类型
	virtual bool IsPointer()	 const = 0;

	// 是否需要解引用
	virtual bool IsReference() const = 0;

	// 是否为只读的
	virtual bool IsReadOnly() const = 0;

	// 是否为基础数据类型
	virtual bool IsPrimitive() const = 0;

	// 是否为类对象类型
	virtual bool IsObject() const = 0;

	// 是否为整型
	virtual bool IsIntegerType() const = 0;

	// 是否为浮点型
	virtual bool IsFloatType() const = 0;

	// 是否为浮点型(双精度)
	virtual bool IsDoubleType() const = 0;

	// 是否为'bool'型
	virtual bool IsBooleanType() const = 0;

	// 是否为数组类型
	virtual bool IsArrayType() const = 0;

	// 得到数据的维数
	virtual int  GetArrayDim() const = 0;

	// 得到某一维数组的大小
	virtual int  GetArraySize(int dim) const = 0;

	// 得到其在堆栈上所占用的双字节数
	virtual int  GetSizeOnStackDWords()  const = 0;

	// 得到其在内存中占用的字节数
	virtual int  GetSizeInMemoryBytes()  const = 0;
protected:
	virtual ~psIDataType() {}
};

//-------------------------------------------------------------------
// 类名: psIScriptFunction
// 说明: 脚本函数接口
//-------------------------------------------------------------------
class psIScriptFunction
{
public:
	// 得到函数名
	virtual const psCHAR* GetName() const = 0;

	// 得到源文件
	virtual const psCHAR* GetSourceFile() const = 0;

	// 得到二进制代码
	virtual psIBuffer* GetByteCode(int& outsize) const = 0;

	// 得到函数ID(NOTE: 函数ID不一定就是其索引值)
	virtual const int GetID() const = 0;

	// 得到函数的字符串说明
	virtual	psAutoStringPtr GetDeclarationString() const = 0;

	// 得到返回值的类型
	virtual const psIDataType* GetReturnType() const = 0;

	// 得到这个函数的类对象类型(如果是全局函数其返回值为空)
	virtual const psIObjectType* GetObjectType() const = 0;

	// 得到这个函数的参数个数
	virtual const unsigned int GetNumParams() const = 0;

	// 得到这个函数的参数类型
	virtual const psIDataType* GetParamType(int index) const = 0;

	// 从行号得到二进制码的位置
	virtual int GetPosFromLineNumber(int lineno, int& realLineno) const = 0;

	// 从二进制码的位置得到源代码的行号
	virtual	int GetLineNumber(int pos) const = 0;

	// 得到最小的行号
	virtual int GetMinLineNumber() const = 0;

	// 得到最大的行号
	virtual int GetMaxLineNumber() const = 0;
protected:
	virtual ~psIScriptFunction() {}
};

//-------------------------------------------------------------------
// 类名: psIModule
// 说明: 脚本模块接口
//-------------------------------------------------------------------
class psIModule
{
public:
	// 重置脚本模块
	// 相当于把这个模块所有内容清空
	virtual void Reset() = 0;

	// 是否可以重置
	virtual bool CanReset() const  = 0;

	// 得到模块名
	virtual const psCHAR* GetName() const = 0;

	// 得到文件名
	virtual const psCHAR* GetFileName() const = 0;

	// 得到模块ID
	virtual int GetID() const = 0;

	// 添加一个脚本代码段
	// 参数: 'code'[in]: 源代码
	//		 'lenght'[in]: 源码长度
	// 返回值: psSUCCESS: 成功
	//		   psERROR: 失败
	virtual int SetCode(const psCHAR* code, int length) = 0;

	// 获取源代码
	virtual const psCHAR* GetSourceCode() const = 0;

	// 得到模块中的函数总数
	virtual unsigned int GetFunctionCount() const = 0;

	// 根据函数ID. 得到脚本函数接口
	// 参数: 'nID'[in]:　脚本函数函数ID
	// 返回值: 脚本函数接口指针, 这个值可能为NULL
	virtual const psIScriptFunction* GetFunctionByID(int nID) const = 0;

	// 得到初始化和退出函数
	virtual const psIScriptFunction* GetInitFunction() const = 0;
	virtual const psIScriptFunction* GetExitFunction() const = 0;

	// 根据函数名称， 得到脚本函数的ID
	// 参数: 'name'[in]:　脚本函数函数的名称
	// 返回值: 脚本函数的ID(大于等于0成功)
	//		   psMULTIPLE_FUNCTIONS: 找到了多个匹配的函数
	//		   psNO_FUNCTION: 没有找到匹配的函数
	//		   psERROR: 编译这个脚本模块的时候失败
	virtual int GetFunctionIDByName(const psCHAR* name) const = 0;

	// 根据函数声明. 得到脚本函数的ID
	// 参数: 'decl'[in]:　脚本函数函数的声明
	// 返回值: 脚本函数的ID(大于等于0成功)
	//		   psINVALID_DECLARATION: 无效的声明
	//		   psMULTIPLE_FUNCTIONS: 找到了多个匹配的函数
	//		   psNO_FUNCTION: 没有找到匹配的函数
	//		   psERROR: 编译这个脚本模块的时候失败
	virtual int GetFunctionIDByDecl(const psCHAR* decl) const = 0;

	// 得到类对象的个数
	virtual unsigned int GetObjectTypeCount() const = 0;

	// 根据类对象索引得到类对象
	// 参数: 'index': 类对象类型在脚本模块中的索引
	// 返回值: 类对象类型接口(不可能为空)
	virtual const psIObjectType* GetObjectType(unsigned int index) const = 0;

	// 根据类对象名称得到类对象
	// 参数: 'name': 类对象的名称
	// 返回值: '类对象类型接口(其值有可能为空)
	virtual const psIObjectType* GetObjectType(const psCHAR* name) const = 0;

	// 得到全局变量的总数
	virtual unsigned int GetGlobalVarCount() const = 0;

	// 得到全局变量属性
	// 参数: 'index': 全局属性在脚本模块中的索引
	// 返回值: 全局变量属性接口(不可能为空)
	virtual const psIProperty* GetGlobalVar(int index) const = 0;

	// 保存二进制码
	// 参数: 'out': 输出二进制码的二进制流
	// 返回值: psSUCESS：  sdf
	virtual int SaveByteCode(psIBinaryStream* out) const = 0;

	// 装载二进制码
	// 参数: 'in': 输入二进制码的二进制流
	virtual int LoadByteCode(psIBinaryStream* in) = 0;

	// 切换断点
	// 参数: 'lineno':断点的行号
	//		 'hasBreakPoint'此时现在是否有断点
	// 返回值: true: 成功 false: 失败
	virtual bool ToggleBreakPoint( int lineno, bool& hasBreakPoint) = 0;

	// 插入用户断点
	// 参数: 'lineno':断点的行号
	// 返回值: true: 成功 false: 失败
	virtual bool InsertBreakPoint( int lineno) = 0;

	// 移除用户断点
	// 参数: 'lineno':断点的行号
	// 返回值: true: 成功 false: 失败
	virtual bool RemoveBreakPoint( int lineno) = 0;

	// 移除所有的用户断点
	// 参数: 'lineno':断点的行号
	// 返回值: true: 成功 false: 失败
	virtual void RemoveAllBreakPoint() = 0;

	// 得到所有的断点行号信息
	virtual psIBuffer* GetAllBreakPoint( int* numBreakPoints ) = 0;
protected:
	virtual ~psIModule() {}
};

//-------------------------------------------------------------------
// 脚本配置函数
//-------------------------------------------------------------------
typedef bool (PS_STDCALL * PS_CONFIG_ENGINE_FUNC)(psIScriptEngine* engine);
typedef bool (PS_STDCALL * PS_ARGTYPE_CHECK_FUNC)(const psIScriptFunction* func, const psIDataType** argTypes, const char** constArgs, size_t numArg);

typedef psAutoStringPtr (PS_STDCALL * PS_GET_STRING_VALUE_FUNC)(void* obj);
typedef void (PS_STDCALL * PS_SET_STRING_VALUE_FUNC)(void* obj, const psCHAR* value);

#define PS_REGISTER_ENGINE_FUNC_NAME    "psRegisterEngine"
#define PS_UNREGISTER_ENGINE_FUNC_NAME  "psUnregisterEngine"

//-------------------------------------------------------------------
// 类名: psIScriptEngine
// 说明: 脚本引擎接口
//-------------------------------------------------------------------
class psIScriptEngine
{
public:
	// 内存管理
	virtual long AddRef() = 0;
	virtual long Release() = 0;

	// 注册一个C++类
	// 参数: 'name':	 在脚本中使用的类名
	//		 'byteSize': 在C++中的真实大小
	//		 'flags'   : 类的行为标志.
	// 返回值: psINVALID_ARG: 传入此函数的参数无效.
	//		   psINVALID_NAME: 'name'的值为空, 或者无效
	//		   psCLASS_EXIST:  此类名已经被注册过
	//		   psNAME_TAKEN:  此类名与其它名称冲突
	//		   psSUCCESS:	  成功
	virtual int RegisterObjectType(const psCHAR* name, int byteSize, psDWORD flags) = 0;

	// 注册C++类的成员属性
	// 参数: 'objname': 在脚本中使用的类名
	//		 'declaration': 属性声明
	//		 'byteoffset' : 属性在C++类的偏移
	// 返回值: psINVALID_OBJECT: 'objname'类对象无效
	//		   psSUCCESS: 成功 
	//		   其它: 'declaration' 无效		
	virtual int RegisterObjectProperty(const psCHAR* objname, const psCHAR* declaration, int byteOffset) = 0;
 
	// 注册C++类的成员方法
	// 参数: 'objname': 在脚本中使用的类名
	//		 'declaration': 对象方法声明
	//		 'funcPointer': 函数地址值
	//		 'callConv':    调用方式
	// 返回值: psINVALID_OBJECT: 'objname'类对象无效
	//		   psNOT_SUPPORTED:  'callConv'指定的调用方式不被支持
	//		   psAPP_CANT_INTERFACE_DEFAULT_ARRAY: 'declaration'中的参数和返回值不能为数组类型
	//		   psNAME_TAKEN:    函数名与其它名称冲突
	//		   psINVALID_DECLARATION: 'declaration' 无效
	//		   psSUCCESS: 成功 
	virtual int RegisterObjectMethod(const psCHAR* objname, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv) = 0;

	// 注册C++类的行为
	// 参数: 'objname': 在脚本中使用的类名
	//		 'behaviour': 　对象行为属性的类型(e.g: psBEHAVE_CONSTRUCT)
	//		 'declaration': 对象方法声明
	//		 'funcPointer': 函数地址值
	//		 'callConv':    调用方式
	// 返回值: psINVALID_OBJECT: 'objname'类对象无效
	//		   psNOT_SUPPORTED:  'callConv'指定的调用方式不被支持
    //		   psAPP_CANT_INTERFACE_DEFAULT_ARRAY: 'declaration'中的参数和返回值不能为数组类型
	//		   psINVALID_DECLARATION: 'declaration' 无效
	//		   psALREADY_REGISTERED: 这个类行为属性已经被注册过
	//		   psSUCCESS: 成功
	virtual int RegisterObjectBehaviour(const psCHAR* objname, psDWORD behaviour, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv) = 0;

	// 注册一个全局变量
	// 参数: 'declaration' : 全局变量的声明
	//		 'pointer' :	 为其地址值
	// 返回值: psINVALID_TYPE: 无效的数据类型
	//		   psSUCCESS:      成功
	//		   其它:		   无效的声明	
	virtual int RegisterGlobalProperty(const psCHAR* declaration, void* pointer) = 0;

	// 注册一个整型常量
	virtual int RegisterIntConstant(const psCHAR* name, int v) = 0;

	// 注册一个浮点常量
	virtual int RegisterFloatConstant(const psCHAR* name, float v) = 0;

	// 注册一个全局函数
	// 参数: 'declaration': 函数声明
	//		 'funcPointer': 函数地址值
	//		 'callConv': 调用方式
	// 返回值: psINVALID_ARG: 传入此函数的参数无效.
	//		   psNOT_SUPPORTED: 'callConv'指定的调用方式不被支持
	//		   psINVALID_DECLARATION: 'declaration' 无效
	//		   psAPP_CANT_INTERFACE_DEFAULT_ARRAY: 'declaration'中的参数和返回值不能为数组类型
	//		   psNAME_TAKEN:    函数名与其它名称冲突
	//		   psSUCCESS: 成功
	virtual int RegisterGlobalFunction(const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv) = 0;

	// 注册一个全局操作符重载('behavior'为操作重载修饰, 'decl'为其声明, 'funcPointer'为其函数地址值, 'callConv'表示其调用方式)
	// 参数: 'behaviour': 全局行为属性类型(e.g:psBEHAVE_ADD)
	//		 'decl': 全局函数的声明
	//		 'funcPointer': 函数地址值
	//		 'callConv': 调用方式
	// 返回值: psNOT_SUPPORTED: 'callConv'指定的调用方式不被支持
	//		   psINVALID_DECLARATION: 'declaration' 无效
	//		   psAPP_CANT_INTERFACE_DEFAULT_ARRAY: 'declaration'中的参数和返回值不能为数组类型
	//		   psNAME_TAKEN:    函数名与其它名称冲突
	virtual int RegisterGlobalBehaviour(psDWORD behaviour, const psCHAR* decl, psUPtr funcPointer, psDWORD callConv) = 0;

	// 声明类的继承关系
	// 参数: 'name'类名
	//		 'parent'父类名
	//		 'baseOffset'C++中子类到父类的偏移
	virtual int DeclareObjectHierarchy(const psCHAR* name, const psCHAR* parent, int baseOffset ) = 0 ;

	// 注销一个C++类
	// 参数: 'name': 类名
	// 返回值: psSUCCESS: 成功
	//		   psERROR: 失败
	virtual int UnregisterObjectType( const psCHAR* name ) = 0;

	// 注销一个全局变量
	// 参数: 'ptr': 全局变量的指针
	// 返回值: psSUCCESS: 成功
	//		   psERROR: 失败
	virtual int UnregisterGlobalProperty(void* ptr)	= 0;

	// 注销一个全局函数
	// 参数: 'funcPtr': 全局函数的指针
	// 返回值: psSUCCESS: 成功
	//		   psERROR: 失败
	virtual int UnregisterGlobalFunction( psUPtr funcPtr ) = 0;

	// 注销一个全局操作符重载
	// 参数: 'ptr': 全局函数的指针
	// 返回值: psSUCCESS: 成功
	//		   psERROR: 失败
	virtual int UnregisterGlobalBehaviour( psUPtr funcPtr ) = 0;

	// 获取类对象类型的个数
	virtual unsigned int GetObjectTypeCount() const = 0;

	// 获取所有的类对象类型
	virtual void GetAllObjectTypes(psIObjectType* objectTypes[]) const = 0;

	// 根据名称得到注册的C++类的类型接口, 这个函数可能会失败
	// 参数: 'name': 类对象的名称
	// 返回值: 类对象接口, 其值可能为空
	virtual const psIObjectType* FindObjectType(const psCHAR* name) const = 0;

	// 根据函数ID,得到系统函数接口(NOTE:系统函数的ID都小于0)
	// 参数: 'nID': 函数ID
	// 返回值: 脚本函数接口, 其值可能为空
	virtual const psIScriptFunction* GetSystemFunction(int nID) const = 0;

	// 得到系统全局函数的个数
	virtual unsigned int GetGlobalFunctionCount() const = 0; 

	// 根据函数索引,得到系统全局函数接口
	// 参数: 'index': 全局函数在脚本引擎中的索引
	// 返回值: 脚本函数接口(其值不可能为空)
	virtual const psIScriptFunction* GetGlobalFunction(unsigned int index) const = 0;

	// 得到全局属性的个数
	virtual unsigned int GetGlobalPropCount() const = 0;

	// 获取所有的全局属性
	virtual void GetAllGlobalProps( psIProperty* pAllGlobalProps[] ) const = 0;

	//  根据名称得到全局变量属性接口
	virtual const psIProperty* FindGlobalProp(const psCHAR* name) const = 0;

	// 得到脚本模块的总数
	virtual unsigned int GetScriptModuleCount() const = 0; 

	// 根据索引得到脚本模块接口
	// 参数: 'index' : 脚本模块在脚本引擎中的索引
	// 返回值: 脚本模块接口(其不可能为空)
	virtual psIModule* GetModule(unsigned int index) const = 0;

	// 根据模块名称得到脚本模块接口
	// 参数: 'name' : 脚本模块的名称
	// 返回值: 脚本模块接口, 其值可能为空
	virtual psIModule* GetModule(const psCHAR* name) const = 0;

	// 创建一个脚本模块指针
	// 参数: 'name'[in] 脚本模块的名称
	//		 'filename'[in]: 源文件路径
	//		 'mod'[out]用于返回脚本模块的指针地址.
	// 返回值: psSUCCESS: 成功
	//		   psERROR: 失败
	// 		   psMODULE_EXIST : 模块已经存在, 但此时'ppMod'返回已有模块的指针.
	virtual int CreateModule( const psCHAR* name, const psCHAR* filename, psIModule** mod ) = 0;

	// 废弃所有的脚本模块
	virtual void Reset() = 0;

	// 丢弃一个脚本模块
	virtual int Discard(const psCHAR* module) = 0;

	// 设置代码优化选项
	virtual void SetOptimizeOptions( psSOptimizeOptions options ) = 0;

	// 得到代码优化选项
	virtual psSOptimizeOptions GetOptimizeOptions() const = 0;

	// 编译一个脚本模块
	// 参数: 'mod': 脚本模块接口.
	//		 'pTextStream': 用于分析文本文件的接口
	//		 'pOutStream': 用于输出错误信息的接口
	// 返回值: sSUCCESS: 成功
	//		   psINVALID_CONFIGURATION: 引擎配置无效
	//		   psMODULE_CANT_BE_RESET: 模块不能被重置: 
	virtual int Compile( psIModule* mod, psITextStream* pTextStream, psIOutputStream* pOutStream ) = 0;

	// 执行一个字符串
	// 参数: 'ctx': 用于执行脚本的动行上下文接口, 如果它为空,那么脚本使用脚本引擎的默认脚本上下文接口
	//		  要注意的是如果是多线程的, 要保证每个线程用不同的脚本上下文接口.
	//		 'script': 字符串源码
	//		 'pOutStream':  用于输出错误信息的接口
	virtual int ExecuteString(psIScriptContext* ctx, const psCHAR *script, psIOutputStream* pOutStream ) = 0;

	// 设轩默认的堆栈大小
	// 参数: 'initial': 初始堆栈大小
	//		 'maximum': 最大的堆栈大小, 如果为0表示没有限制
	// 返回值: 无
	virtual void SetDefaultContextStackSize(psUINT initial, psUINT maximum) = 0;

	// 创建脚本运行上下文接口
	// 参数: 'context':指向脚本运行上下文接口的指针地址
	// 返回值: psSUCCESS: 创建成功
	//		   psERROR : 创建失败			
	virtual int CreateContext(psIScriptContext **contex) = 0;

	// 设置调试器
	// 参数: 'debugger' 调试器接口
	// 返回值: 无
	virtual void SetDebugger( psIDebugger* debugger ) = 0;

	// 注册一个字符串处理工厂(  )
	// 参数: 'dataType': 字符串的类名
	//		 'getFunc'为如何从一个类对象得到字符串值的函数指针.
	//		 'setFunc'为如何从一个字符串指针赋值
	// 返回值: psSUCCESS: 成功
	//		   psINVALID_TYPE: 无效的类名
	virtual int  RegisterStringFactory(const psCHAR* dataType, PS_GET_STRING_VALUE_FUNC getFunc, PS_SET_STRING_VALUE_FUNC setFunc) = 0 ;

	// 判断一个对象类型是否为字符串类型
	// 参数: 'objType': 类对象类型接口
	virtual bool IsStringType(const psIObjectType* objType) const = 0;

	// 设置一个字符串类对象的值
	// 参数: 'obj': 字符串类对象指针.
	//		 'value': 字符串的值
	// 返回值: psSUCCESS: 成功
	//		   psERROR:   失败
	virtual bool  SetStringValue(const psIObjectType* objType, void* obj, const psCHAR* value) = 0;

	// 得到一个字符串类对象的值
	// 参数: 'obj': 字符串类对象指针.
	// 返回值: NULL: 失败, 否则为字符串的值.
	virtual psAutoStringPtr GetStringValue(const psIObjectType* objType, void* obj) const = 0;

	// 分配一个类对象
	// 参数: 'moduleId': 脚本模块ID(如果是系统类对象其值为-1)
	//		 'objType':  类对象类型接口
	// 返回值: NULL: 失败, 否则为类对象的指针
	virtual void* AllocObject( int moduleId, const psIObjectType* objType ) = 0;

	// 释放一个类对象
	// 参数: 'obj': 类对象的指针
	//		 'moduleId': 脚本模块ID(如果是系统类对象其值为-1)
	//		 'objType':  类对象类型接口
	// 返回值: true: 成功. false: 失败
	virtual bool FreeObject( void* obj, int moduleId, const psIObjectType* objType  ) = 0;

	// 设置宏定义列表字符串
	// 参数: 'macros':   所有的宏定义(e.g."_DEBGU; UNICODE" )
	//		 'sperator': 分隔字符
	// 返回值: true: 成功. false: 失败
	virtual bool SetMacroList(const psCHAR* macros, psCHAR sperator) = 0;

	// 得到宏定义列表字符串
	// 返回值: 为宏定义字符串指针(其不可能为空)
	virtual const psCHAR* GetMacroList() = 0;

	// 设置变参函数类型检查回调函数
	virtual void SetArgTypeCheckFunc( PS_ARGTYPE_CHECK_FUNC pFunc ) = 0;

	// 得到变参函数类型检查回调函数
	virtual PS_ARGTYPE_CHECK_FUNC GetArgTypeCheckFunc() = 0;
protected:
	virtual ~psIScriptEngine() {}
};

//-------------------------------------------------------------------
// 类名: psIScriptContext
// 说明: 脚本运行上下文接口
//-------------------------------------------------------------------
class psIScriptContext
{
public:
	// 内存管理
	virtual long AddRef() = 0;
	virtual long Release() = 0;

	// 得到引擎指针
	virtual psIScriptEngine *GetEngine() = 0;

	// 得到当前的上下文状态
	// 返回值: psEXECUTION_SUSPENDED: 被挂起
	//		   psEXECUTION_ACTIVE :   活动
	//         psEXECUTION_EXCEPTION: 异常
	//		   psEXECUTION_FINISHED:  动行结束
	//		   psEXECUTION_PREPARED:  准备动行
	//		   psEXECUTION_UNINITIALIZED: 未初始化
	//		   psERROR: 未知错误
	virtual int GetState() = 0;

	// 准备执行一个脚本函数
	// 参数: 'moduleID'[in] : 脚本模块ID(如果为-1,表示再次准备运行上一次的函数)
	//		 'funcID'[in]: 脚本函数ID(如果为-1,表示再次准备运行上一次的函数)
	//		 'obj'[in]:   类对象指针,如果这调用一个类的方法那么它为其对象指针
	//					  如果为一个全局函数,它应该为空.
	// 返回值: psCONTEXT_ACTIVE: 当前脚本上下文是已经是活动的.
	//		   psNO_FUNCTION: 获取函数指针失败
	//		   psNO_MODULE: 获取脚本模块失败
	//		   psIVALID_OBJECT_POINTER: 'obj'的值无效
	//		   psSUCCESS: 成功
	//		   psERROR: 失败
	virtual int Prepare(int moduleID, int funcID, void* obj) = 0;

	// 设置参数的值
	// 参数: 'arg[in]': 参数的索引(从0开始)
	//		 'value'[in]: 参数的值
	// 返回值: psCONTEXT_NOT_PREPARED: 脚本上下文没有准备运行函数
	//		   psINVALID_ARG: 无效的输入参数
	//		   psINVALID_TYPE: 数据类型不匹配
	//		   psSUCESS: 成功
	////////////////////////////////////////////////////////////////////
	//		   NOTE:如果这个函数调用失败,以后调用'Execute'函数时也会失败.
	virtual int SetArgDWord(psUINT arg, psDWORD value) = 0;
	virtual int SetArgFloat(psUINT arg, float value) = 0;
	virtual int SetArgDouble(psUINT arg, double value) = 0;
	virtual int SetArgPointer(psUINT arg, void* ptr) = 0;

	// 得到函数的返回值
	virtual psDWORD GetReturnDWord() = 0;
	virtual float   GetReturnFloat() = 0;
	virtual double  GetReturnDouble() = 0;
	virtual void*   GetReturnPointer() = 0;

	// 运行函数
	// exeFlag: psEXEC_DEBUG为调试动行(其会调用调试器的回调函数)
	//			psEXEC_STEP_INTO为单步跟进, 
	//			psEXEC_STEP_OVER为单步执行
	//			0为正常执行
	// 返回值: 为当前上下文的状态(psEXECUTION_FINISHED ... )
	virtual int Execute( int exeFlag ) = 0;

	// 运行函数的一条汇编指今
	// 返回值: 为当前上下文的状态(psEXECUTION_FINISHED ... )
	virtual int ExecuteInstr() = 0;

	// 退出函数
	virtual int Abort() = 0;

	// 释放对脚本模块的引用
	virtual void ReleaseModuleRef() = 0;

	// 设置初始函数
	virtual int SetInitFunction( int modId, int funcId ) = 0;

	// 得到异常发生的行号
	virtual int GetExceptionLineNumber() = 0;

	// 得到异常发生的函数ID
	virtual int GetExceptionFunctionId() = 0;

	// 得到发生异常的模块ID
	virtual int GetExceptionModuleId()   = 0;

	// 得到异常的字符串说明
	virtual const psCHAR* GetExceptionString() = 0;

	// 得到当前所在的行号
	virtual int GetCurrentLineNumber() = 0;

	// 得到当前所在的二进制码中的位置
	virtual int GetCurrentBytePos() = 0;

	// 查找调试符号
	virtual const psIDataType* FindDebugSymbol(const psCHAR* name, psDWORD* addr) const = 0;

	// 得到当前的脚本模块
	virtual psIModule* GetCurrentModule() = 0;

	// 得到当前的脚本函数接口
	virtual psIScriptFunction* GetCurrentFunction()  = 0;

	// 得到初始的脚本函数接口
	virtual psIScriptFunction* GetInitFunction()  = 0;

	// 得到当前调用栈的字符串说明
	virtual psAutoStringPtr GetCallStackString() const = 0;

	// 设置异常
	virtual int SetException(const psCHAR *descr) = 0;

protected:
	virtual ~psIScriptContext() {};
};

//-------------------------------------------------------------------
// 类名: psIDebugger
// 说明: 脚本调试器接口
//-------------------------------------------------------------------
class psIDebugger
{
public:
	// 调试器的回调事件
	virtual int OnDebugEvent(psIScriptContext* ctx) = 0;
	
	// 在创建一个脚本模块时通知调试器
	virtual void OnCreateModule( psIModule* mod ) = 0;

	// 在删除一个脚本模块时通知调试器
	virtual void OnDeleteModule( psIModule* mod ) = 0;

	// 在编译一个脚本模块时通知调试器
	virtual void OnCompileModule( psIModule* mod, int r ) = 0;
protected:
	virtual ~psIDebugger() {};
};

//----------------------------------------------------------------------//
// 常量定义
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// 调用方式约定
//----------------------------------------------------------------------//
const psDWORD psCALL_CDECL            = 0;
const psDWORD psCALL_STDCALL          = 1;
const psDWORD psCALL_THISCALL         = 2;
const psDWORD psCALL_CDECL_OBJFIRST   = 4;

//----------------------------------------------------------------------//
// 类标志
// 说明一个C++类的行为标志.
// i.e. 如果一个类只有构造函数,那么其标志为: psOBJ_CLASS_CONSTRUCTOR
// i.e. 如果一个类又有构造函数,又有析构函数,又有赋值函数,那么其标志为: psOBJ_CLASS_CDA
//----------------------------------------------------------------------//
const psDWORD psOBJ_CLASS             = 1;
const psDWORD psOBJ_INTERFACE         = 2;
const psDWORD psOBJ_CLASS_CONSTRUCTOR = 4;
const psDWORD psOBJ_CLASS_DESTRUCTOR  = 8;
const psDWORD psOBJ_CLASS_ASSIGNMENT  = 16;
const psDWORD psOBJ_PRIMITIVE		  = 32;
const psDWORD psOBJ_CLASS_MAX		  = 65;

const psDWORD psOBJ_CLASS_C           = (psOBJ_CLASS + psOBJ_CLASS_CONSTRUCTOR);
const psDWORD psOBJ_CLASS_CD          = (psOBJ_CLASS + psOBJ_CLASS_CONSTRUCTOR + psOBJ_CLASS_DESTRUCTOR);
const psDWORD psOBJ_CLASS_CA          = (psOBJ_CLASS + psOBJ_CLASS_CONSTRUCTOR + psOBJ_CLASS_ASSIGNMENT);
const psDWORD psOBJ_CLASS_CDA         = (psOBJ_CLASS + psOBJ_CLASS_CONSTRUCTOR + psOBJ_CLASS_DESTRUCTOR + psOBJ_CLASS_ASSIGNMENT);
const psDWORD psOBJ_CLASS_D           = (psOBJ_CLASS + psOBJ_CLASS_DESTRUCTOR);
const psDWORD psOBJ_CLASS_A           = (psOBJ_CLASS + psOBJ_CLASS_ASSIGNMENT);
const psDWORD psOBJ_CLASS_DA          = (psOBJ_CLASS + psOBJ_CLASS_DESTRUCTOR + psOBJ_CLASS_ASSIGNMENT);

//----------------------------------------------------------------------//
// 类的行为属性
//----------------------------------------------------------------------//
 const psDWORD psBEHAVE_CONSTRUCT     = 0;
 const psDWORD psBEHAVE_DESTRUCT      = 1;

 const psDWORD psBEHAVE_FIRST_ASSIGN  = 2;
 const psDWORD psBEHAVE_ASSIGNMENT    = 2;
 const psDWORD psBEHAVE_ADD_ASSIGN    = 3;
 const psDWORD psBEHAVE_SUB_ASSIGN    = 4;
 const psDWORD psBEHAVE_MUL_ASSIGN    = 5;
 const psDWORD psBEHAVE_DIV_ASSIGN    = 6;
 const psDWORD psBEHAVE_MOD_ASSIGN    = 7;
 const psDWORD psBEHAVE_OR_ASSIGN     = 8;
 const psDWORD psBEHAVE_AND_ASSIGN    = 9;
 const psDWORD psBEHAVE_XOR_ASSIGN    = 10;
 const psDWORD psBEHAVE_SLL_ASSIGN    = 11;
 const psDWORD psBEHAVE_SRL_ASSIGN    = 12;
 const psDWORD psBEHAVE_SRA_ASSIGN    = 13;
 const psDWORD psBEHAVE_LAST_ASSIGN   = 13;
 const psDWORD psBEHAVE_FIRST_DUAL    = 14;
 const psDWORD psBEHAVE_ADD           = 14;
 const psDWORD psBEHAVE_SUBTRACT      = 15;
 const psDWORD psBEHAVE_MULTIPLY      = 16;
 const psDWORD psBEHAVE_DIVIDE        = 17;
 const psDWORD psBEHAVE_MODULO        = 18;
 const psDWORD psBEHAVE_EQUAL         = 19;
 const psDWORD psBEHAVE_NOTEQUAL      = 20;
 const psDWORD psBEHAVE_LESSTHAN      = 21;
 const psDWORD psBEHAVE_GREATERTHAN   = 22;
 const psDWORD psBEHAVE_LEQUAL        = 23;
 const psDWORD psBEHAVE_GEQUAL        = 24;
 const psDWORD psBEHAVE_LOGIC_OR	  = 25;
 const psDWORD psBEHAVE_LOGIC_AND	  = 26;
 const psDWORD psBEHAVE_BIT_OR        = 27;
 const psDWORD psBEHAVE_BIT_AND       = 28;
 const psDWORD psBEHAVE_BIT_XOR       = 29;
 const psDWORD psBEHAVE_BIT_SLL       = 30;
 const psDWORD psBEHAVE_BIT_SRL       = 31;
 const psDWORD psBEHAVE_LAST_DUAL     = 31;

 const psDWORD psBEHAVE_FIRST_UNARY   = 32;
 const psDWORD psBEHAVE_UNARY_MINUS   = 32;
 const psDWORD psBEHAVE_UNARY_PLUS	  = 33;
 const psDWORD psBEHAVE_UNARY_NOT	  = 34;		  
 const psDWORD psBEHAVE_UNARY_BITNOT  = 35;
 const psDWORD psBEHAVE_INC			  = 36;
 const psDWORD psBEHAVE_DEC			  = 37;
 const psDWORD psBEHAVE_LAST_UNARY    = 37;

 const psDWORD psBEHAVE_INDEX         = 38;

 //----------------------------------------------------------------------//
 // 返回值
 //----------------------------------------------------------------------//
 const int psSUCCESS                              =  0;
 const int psERROR                                = -1;
 const int psCONTEXT_ACTIVE                       = -2;
 const int psCONTEXT_NOT_FINISHED                 = -3;
 const int psCONTEXT_NOT_PREPARED                 = -4;
 const int psINVALID_ARG                          = -5;
 const int psNO_FUNCTION                          = -6;
 const int psNOT_SUPPORTED                        = -7;
 const int psINVALID_NAME                         = -8;
 const int psNAME_TAKEN                           = -9;
 const int psINVALID_DECLARATION                  = -10;
 const int psINVALID_OBJECT                       = -11;
 const int psINVALID_TYPE                         = -12;
 const int psALREADY_REGISTERED                   = -13;
 const int psMULTIPLE_FUNCTIONS                   = -14;
 const int psNO_MODULE                            = -15; 
 const int psNO_GLOBAL_VAR                        = -16;
 const int psINVALID_CONFIGURATION                = -17;
 const int psINVALID_INTERFACE                    = -18;
 const int psAPP_CANT_INTERFACE_DEFAULT_ARRAY     = -19; 
 const int psMODULE_CANT_BE_RESET				  = -20;
 const int psNO_BYTECODE						  = -21;
 const int psCLASS_EXIST						  = -22;
 const int psMODULE_EXIST						  = -23;
 const int psIVALID_OBJECT_POINTER				  = -24;
 const int psDONT_FIND_CLASS					  = -25;
 const int psCLASS_CICLE_DEPENDENCE				  = -26;
 const int psCANT_INIT_MODULE					  = -27;
 //----------------------------------------------------------------------//
 // 脚本运行上下文状态
 //----------------------------------------------------------------------//

 const int psEXECUTION_FINISHED      = 0;
 const int psEXECUTION_SUSPENDED     = 1;
 const int psEXECUTION_ABORTED       = 2;
 const int psEXECUTION_EXCEPTION     = 3;
 const int psEXECUTION_PREPARED      = 4;
 const int psEXECUTION_UNINITIALIZED = 5;
 const int psEXECUTION_ACTIVE        = 6;
 const int psEXECUTION_ERROR         = 7;


 //----------------------------------------------------------------------//
 // 单步执行的标识
 //----------------------------------------------------------------------//
 const int psEXEC_STEP_INTO = 1;
 const int psEXEC_STEP_OVER = 2;
 const int psEXEC_DEBUG		= 4;

 // Prepare flags
 const int psPREPARE_PREVIOUS = -1;

//----------------------------------------------------------------------//
// 解析函数地址值的函数定义
//----------------------------------------------------------------------//
#include <memory.h>

 inline psUPtr psFunctionPtr(psFUNCTION_t func)
 {
	 psUPtr p;
	 memset(&p, 0, sizeof(p));
	 p.func = func;

	 return p;
 }

 // Method pointers

 // Declare a dummy class so that we can determine the size of a simple method pointer
 class psCSimpleDummy {};
 typedef void (psCSimpleDummy::*psSIMPLEMETHOD_t)();
 const int SINGLE_PTR_SIZE = sizeof(psSIMPLEMETHOD_t);


 // Define template
 template <int N>
 struct psSMethodPtr
 {
	 template<class M>
		 static psUPtr Convert(M Mthd)
	 {
		 // This version of the function should never be executed, nor compiled,
		 // as it would mean that the size of the method pointer cannot be determined.
		 // int ERROR_UnsupportedMethodPtr[-1];
		 return 0;
	 }
 };


 // Template specialization
 template <>
 struct psSMethodPtr<SINGLE_PTR_SIZE>
 {
	 template<class M> 
		 static psUPtr Convert(M Mthd)
	 {
		 psUPtr p;
		 memset(&p, 0, sizeof(p));

		 memcpy(&p, &Mthd, SINGLE_PTR_SIZE);

		 return p;
	 }
 };


#if defined(_MSC_VER) && !defined(__MWERKS__)

 // MSVC and Intel uses different sizes for different class method pointers
 template <>
 struct psSMethodPtr<SINGLE_PTR_SIZE+1*sizeof(int)>
 {
	 template <class M>
		 static psUPtr Convert(M Mthd)
	 {
		 psUPtr p;
		 memset(&p, 0, sizeof(p));

		 memcpy(&p, &Mthd, SINGLE_PTR_SIZE+sizeof(int));

		 return p;
	 }
 };

 template <>
 struct psSMethodPtr<SINGLE_PTR_SIZE+2*sizeof(int)>
 {
	 template <class M>
		 static psUPtr Convert(M Mthd)
	 {
		 // This is where a class with virtual inheritance falls

		 // Since method pointers of this type doesn't have all the 
		 // information we need we force a compile failure for this case.
		 int ERROR_VirtualInheritanceIsNotAllowedForMSVC[-1];

		 // The missing information is the location of the vbase table,
		 // which is only known at compile time.

		 // You can get around this by forward declaring the class and 
		 // storing the sizeof its method pointer in a constant. Example:

		 // class ClassWithVirtualInheritance;
		 // const int ClassWithVirtualInheritance_workaround = sizeof(void ClassWithVirtualInheritance::*());

		 // This will force the compiler to use the unknown type 
		 // for the class, which falls under the next case

		 psUPtr p;
		 return p;
	 }
 };

 template <>
 struct psSMethodPtr<SINGLE_PTR_SIZE+3*sizeof(int)>
 {
	 template <class M>
		 static psUPtr Convert(M Mthd)
	 {
		 psUPtr p;
		 memset(&p, 0, sizeof(p));

		 memcpy(&p, &Mthd, SINGLE_PTR_SIZE+3*sizeof(int));

		 return p;
	 }
 };

#endif

#endif // __PIXELSCRIPT_H__








