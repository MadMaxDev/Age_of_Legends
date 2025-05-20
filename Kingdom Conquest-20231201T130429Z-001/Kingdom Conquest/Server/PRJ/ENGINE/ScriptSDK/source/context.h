#ifndef __PS_CONTEXT_H__
#define __PS_CONTEXT_H__

#include "config.h"
#include "string.h"
#include "objecttype.h"
#include "scriptfunction.h"
#include "threadmanager.h"

//-------------------------------------------------------------------
// 类名: psEContextState
// 说明: 
//-------------------------------------------------------------------
enum psEContextState
{
	tsUninitialized,
	tsPrepared,
	tsSuspended,
	tsActive,
	tsProgramFinished,
	tsProgramAborted,
	tsUnhandledException,
};

//-------------------------------------------------------------------
// 类名: psCContext
// 说明: 
//-------------------------------------------------------------------
class psCContext: public psIScriptContext
{
public:
	//----------------------------------------------------------------------//
	// 实现基类的函数定义
	//----------------------------------------------------------------------//
	// 内存管理
	virtual long AddRef() ;
	virtual long Release() ;

	// 得到引擎指针
	virtual psIScriptEngine *GetEngine() ;

	// 得到当前的上下文状态
	virtual int GetState();

	// 准备执行一个脚本函数
	virtual int Prepare(int modID, int funcID, void* obj) ;

	// 设置参数的值
	virtual int SetArgDWord(psUINT arg, psDWORD value);
	virtual int SetArgFloat(psUINT arg, float value);
	virtual int SetArgDouble(psUINT arg, double value);
	virtual int SetArgPointer(psUINT arg, void* ptr);

	// 得到函数的返回值
	virtual psDWORD GetReturnDWord();
	virtual float   GetReturnFloat();
	virtual double  GetReturnDouble();
	virtual void*   GetReturnPointer();

	// 运行函数
	virtual int Execute( int flag );
	
	// 运行函数的一条汇编指今
	virtual int ExecuteInstr();

	// 退出函数
	virtual int Abort();

	// 释放对脚本模块的引用
	virtual void ReleaseModuleRef();

	// 设置初始函数
	virtual int SetInitFunction( int modId, int funcId );

	// 得到异常发生的行号
	virtual int GetExceptionLineNumber();

	// 得到异常发生的函数ID
	virtual int GetExceptionFunctionId();

	// 得到发生异常的模块ID
	virtual int GetExceptionModuleId();

	// 得到异常的字符串说明
	virtual const psCHAR* GetExceptionString();

	// 得到当前所在的行号
	virtual int GetCurrentLineNumber();

	// 得到当前所在的二进制码中的位置
	virtual int GetCurrentBytePos();

	// 查找调试符号
	virtual const psIDataType* FindDebugSymbol(const psCHAR* name, psDWORD* addr) const;

	// 得到当前的脚本模块
	virtual psIModule* GetCurrentModule();

	// 得到当前的脚本函数接口
	virtual psIScriptFunction* GetCurrentFunction() { return m_CurrentFunction; }

	// 得到初始的脚本函数接口
	virtual psIScriptFunction* GetInitFunction()	{ return m_InitialFunction; }

	// 得到当前调用栈的字符串说明(NOTE: 这个函数的返回值要用psDeleteString来释放)
	virtual psAutoStringPtr GetCallStackString() const;

	// 设置异常
	virtual int SetException(const psCHAR *descr);
public:
	//----------------------------------------------------------------------//
	// 内部函数
	//----------------------------------------------------------------------//
	psCContext(psCScriptEngine* engine, bool holdEngineRef);
	virtual ~psCContext(void);

	inline void  SetReturnValue(psQWORD value)				{ m_ReturnVal = value;						 }

	int GetArgOffset(psUINT arg ); 
    
	// 脱离引擎
	void DetachEngine();

	// 准备特殊的函数调用(i.e.模块初始化)
	int  PrepareSpecial(int modID, int funcID);

	// 运行一条指令
	void ExecuteNext();

	__inline void ExecuteInstr( psBYTE* l_bc, psDWORD* l_sp, psDWORD* l_fp, psBYTE* bases );

	// 清除堆栈
	void CleanStack();
	void CleanStackFrame();

	// 入调用堆/出调用堆
	void PushCallState();
	bool PopCallState();

	// 调用脚本函数
	void CallScriptFunction(psCModule *mod, psCScriptFunction *func);

	// 得到堆本指针
	psDWORD* GetStackPointer() const { return m_StackPointer; }

private:
	//-------------------------------------------------------------------
	// 类名: CALL_STACK
	// 说明: 在调用脚本函数时要保存相关信息
	//-------------------------------------------------------------------
	struct CALL_STACK
	{
		psDWORD				* stackFramePointer;		// 堆栈基址
		psDWORD			    * stackPointer;				// 堆栈指针
		psCScriptFunction	* currentFunction;			// 当前函数
		psBYTE				* binaryCode;				// 二进码的位置
		int					  stackIndex;				// 堆的索引
		psCModule			* module;					// 模块指针	 
		int					  isBreak;					// For debugging
	};
private:
	psCScriptEngine*   m_pEngine;						// 引擎指针
	psCModule*		   m_pModule;						// 模块指针

	bool			   m_HoldEngineRef;
	bool			   m_InExceptionHandler;
	bool			   m_IsStackMemoryNotAllocated;
	bool			   m_unused;
	int				   m_Status;						// 当前状态

	psCScriptFunction *m_CurrentFunction;				// 当前函数指针
	psCScriptFunction *m_InitialFunction;				// 开始运行的函数指针
	psDWORD			  *m_StackFramePointer;				// 堆栈基址
	psDWORD			  *m_StackPointer;					// 堆栈指针
	psBYTE			  *m_BinaryCode;					// 二进制码的位置
	psQWORD			   m_ReturnVal;						// 返回值

	int				  m_StackBlockSize;				    // 当前堆的大小
	int				  m_StackIndex;					    // 当前堆的索引

	std::vector< CALL_STACK >   m_CallStack;			// 调用栈
	std::vector< psDWORD *>     m_StackBlocks;			// 分配了的堆

	psCString		   m_ExceptionString;				// 生成的异常字符串
	int				   m_ExceptionModuleId;				// 产生异常的模块ID
	int				   m_ExceptionFunctionId;			// 产生异常的函数ID
	int				   m_ExceptionLine;					// 产生异常的行号

	psCAtomicCount	   m_RefCount;						// 引用计数
};

#endif // __PS_CONTEXT_H__

