#ifndef __PS_CONTEXT_H__
#define __PS_CONTEXT_H__

#include "config.h"
#include "string.h"
#include "objecttype.h"
#include "scriptfunction.h"
#include "threadmanager.h"

//-------------------------------------------------------------------
// ����: psEContextState
// ˵��: 
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
// ����: psCContext
// ˵��: 
//-------------------------------------------------------------------
class psCContext: public psIScriptContext
{
public:
	//----------------------------------------------------------------------//
	// ʵ�ֻ���ĺ�������
	//----------------------------------------------------------------------//
	// �ڴ����
	virtual long AddRef() ;
	virtual long Release() ;

	// �õ�����ָ��
	virtual psIScriptEngine *GetEngine() ;

	// �õ���ǰ��������״̬
	virtual int GetState();

	// ׼��ִ��һ���ű�����
	virtual int Prepare(int modID, int funcID, void* obj) ;

	// ���ò�����ֵ
	virtual int SetArgDWord(psUINT arg, psDWORD value);
	virtual int SetArgFloat(psUINT arg, float value);
	virtual int SetArgDouble(psUINT arg, double value);
	virtual int SetArgPointer(psUINT arg, void* ptr);

	// �õ������ķ���ֵ
	virtual psDWORD GetReturnDWord();
	virtual float   GetReturnFloat();
	virtual double  GetReturnDouble();
	virtual void*   GetReturnPointer();

	// ���к���
	virtual int Execute( int flag );
	
	// ���к�����һ�����ָ��
	virtual int ExecuteInstr();

	// �˳�����
	virtual int Abort();

	// �ͷŶԽű�ģ�������
	virtual void ReleaseModuleRef();

	// ���ó�ʼ����
	virtual int SetInitFunction( int modId, int funcId );

	// �õ��쳣�������к�
	virtual int GetExceptionLineNumber();

	// �õ��쳣�����ĺ���ID
	virtual int GetExceptionFunctionId();

	// �õ������쳣��ģ��ID
	virtual int GetExceptionModuleId();

	// �õ��쳣���ַ���˵��
	virtual const psCHAR* GetExceptionString();

	// �õ���ǰ���ڵ��к�
	virtual int GetCurrentLineNumber();

	// �õ���ǰ���ڵĶ��������е�λ��
	virtual int GetCurrentBytePos();

	// ���ҵ��Է���
	virtual const psIDataType* FindDebugSymbol(const psCHAR* name, psDWORD* addr) const;

	// �õ���ǰ�Ľű�ģ��
	virtual psIModule* GetCurrentModule();

	// �õ���ǰ�Ľű������ӿ�
	virtual psIScriptFunction* GetCurrentFunction() { return m_CurrentFunction; }

	// �õ���ʼ�Ľű������ӿ�
	virtual psIScriptFunction* GetInitFunction()	{ return m_InitialFunction; }

	// �õ���ǰ����ջ���ַ���˵��(NOTE: ��������ķ���ֵҪ��psDeleteString���ͷ�)
	virtual psAutoStringPtr GetCallStackString() const;

	// �����쳣
	virtual int SetException(const psCHAR *descr);
public:
	//----------------------------------------------------------------------//
	// �ڲ�����
	//----------------------------------------------------------------------//
	psCContext(psCScriptEngine* engine, bool holdEngineRef);
	virtual ~psCContext(void);

	inline void  SetReturnValue(psQWORD value)				{ m_ReturnVal = value;						 }

	int GetArgOffset(psUINT arg ); 
    
	// ��������
	void DetachEngine();

	// ׼������ĺ�������(i.e.ģ���ʼ��)
	int  PrepareSpecial(int modID, int funcID);

	// ����һ��ָ��
	void ExecuteNext();

	__inline void ExecuteInstr( psBYTE* l_bc, psDWORD* l_sp, psDWORD* l_fp, psBYTE* bases );

	// �����ջ
	void CleanStack();
	void CleanStackFrame();

	// ����ö�/�����ö�
	void PushCallState();
	bool PopCallState();

	// ���ýű�����
	void CallScriptFunction(psCModule *mod, psCScriptFunction *func);

	// �õ��ѱ�ָ��
	psDWORD* GetStackPointer() const { return m_StackPointer; }

private:
	//-------------------------------------------------------------------
	// ����: CALL_STACK
	// ˵��: �ڵ��ýű�����ʱҪ���������Ϣ
	//-------------------------------------------------------------------
	struct CALL_STACK
	{
		psDWORD				* stackFramePointer;		// ��ջ��ַ
		psDWORD			    * stackPointer;				// ��ջָ��
		psCScriptFunction	* currentFunction;			// ��ǰ����
		psBYTE				* binaryCode;				// �������λ��
		int					  stackIndex;				// �ѵ�����
		psCModule			* module;					// ģ��ָ��	 
		int					  isBreak;					// For debugging
	};
private:
	psCScriptEngine*   m_pEngine;						// ����ָ��
	psCModule*		   m_pModule;						// ģ��ָ��

	bool			   m_HoldEngineRef;
	bool			   m_InExceptionHandler;
	bool			   m_IsStackMemoryNotAllocated;
	bool			   m_unused;
	int				   m_Status;						// ��ǰ״̬

	psCScriptFunction *m_CurrentFunction;				// ��ǰ����ָ��
	psCScriptFunction *m_InitialFunction;				// ��ʼ���еĺ���ָ��
	psDWORD			  *m_StackFramePointer;				// ��ջ��ַ
	psDWORD			  *m_StackPointer;					// ��ջָ��
	psBYTE			  *m_BinaryCode;					// ���������λ��
	psQWORD			   m_ReturnVal;						// ����ֵ

	int				  m_StackBlockSize;				    // ��ǰ�ѵĴ�С
	int				  m_StackIndex;					    // ��ǰ�ѵ�����

	std::vector< CALL_STACK >   m_CallStack;			// ����ջ
	std::vector< psDWORD *>     m_StackBlocks;			// �����˵Ķ�

	psCString		   m_ExceptionString;				// ���ɵ��쳣�ַ���
	int				   m_ExceptionModuleId;				// �����쳣��ģ��ID
	int				   m_ExceptionFunctionId;			// �����쳣�ĺ���ID
	int				   m_ExceptionLine;					// �����쳣���к�

	psCAtomicCount	   m_RefCount;						// ���ü���
};

#endif // __PS_CONTEXT_H__

