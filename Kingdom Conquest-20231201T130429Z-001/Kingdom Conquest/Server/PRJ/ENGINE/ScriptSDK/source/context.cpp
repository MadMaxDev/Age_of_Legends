#include "bytecode.h"
#include "scriptengine.h"
#include "module.h"
#include "callfunc.h"
#include "texts.h"
#include "context.h"

#include <math.h>

const int RESERVE_STACK = 2;

void psPopActiveContext(psIScriptContext *ctx);
void psPushActiveContext(psIScriptContext *ctx);

#if defined(_MSC_VER)
	#pragma warning(disable: 4312)
	#pragma warning(disable: 4311)
#endif

using namespace pse;

psCContext::psCContext(psCScriptEngine* engine,  bool holdEngineRef)
{
	m_pEngine					  = engine;
	m_HoldEngineRef				  = holdEngineRef;
	m_Status					  = tsUninitialized;
	m_StackBlockSize			  = 0;
	m_pModule					  = NULL;
	m_InExceptionHandler		  = false;
	m_IsStackMemoryNotAllocated	  = false;

	m_CurrentFunction			  = NULL;
	m_InitialFunction			  = NULL;
	m_ReturnVal					  = 0;
	m_RefCount					  = 1;

	if ( m_HoldEngineRef && m_pEngine )
	{
		m_pEngine->AddRef();
	}
}


psCContext::~psCContext(void)
{
	// 脱离引擎
	DetachEngine();

	// 释放所有的堆
	size_t i;
	for( i = 0; i < m_StackBlocks.size(); i++ )
	{
		psDWORD* buf = m_StackBlocks[i];	
		m_StackBlocks[i] = NULL;
		if( buf  )
		{
			delete[] buf;
		}
	}
	m_StackBlocks.clear();
}

void psCContext::DetachEngine()
{
	if( m_pEngine == NULL )
	{
		return;
	}

	// 退出函数
	Abort();

	// 释放模块引用计数
	ReleaseModuleRef();

	if ( m_pEngine && m_HoldEngineRef )
	{
		m_pEngine->Release();
		m_HoldEngineRef = false;
	}		
	m_pEngine = NULL;
}

void psCContext::ReleaseModuleRef()
{
	// 释放模块引用计数
	if ( m_pModule )
	{
		m_pModule->ReleaseContextRef();
		m_pModule = NULL;
	}
	m_InitialFunction = NULL;
	m_CurrentFunction = NULL;
}

int psCContext::SetInitFunction(int modId, int funcId )
{
	if ( m_pEngine == NULL )
		return psERROR;

	// 检查当前状态
	if( m_Status == tsActive || m_Status == tsSuspended )
		return psCONTEXT_ACTIVE;

	if ( modId < 0 || funcId < 0 )
	{
		return psNO_FUNCTION;
	}

	if( m_pModule ) 
	{	
		// 移除先前的模块引用计数
		m_pModule->ReleaseContextRef();
	}

	// 重新获取新的模块
	m_pModule = (psCModule*)( m_pEngine->GetModule(modId) );
	if( m_pModule == NULL ) 
	{
		return psNO_MODULE;
	}
	else 
	{
		// 添加引用计数
		m_pModule->AddContextRef(); 
	}

	// 获取起动函数
	m_InitialFunction = m_pModule->GetScriptFunction(funcId);
	return psSUCCESS;
}

long psCContext::AddRef()
{
	return ++m_RefCount;
}

long psCContext::Release()
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

int psCContext::GetArgOffset( psUINT arg )
{
	if( m_Status != tsPrepared || m_InitialFunction == NULL )
		return psCONTEXT_NOT_PREPARED;

	if( arg >= (psUINT)m_InitialFunction->m_ParamTypes.size() )
	{
		m_Status = psEXECUTION_ERROR;
		return psINVALID_ARG;
	}

	// 计算参数的偏移	
	int offset = 0;
	if (  m_InitialFunction->m_ReturnType.IsObject() && 
		 !m_InitialFunction->m_ReturnType.IsReference() )
	{
		++offset;
	}	
	if ( m_InitialFunction->GetObjectType() )
	{
		++offset;
	}
	for( psUINT n = 0; n < arg; n++ )
	{
		offset += m_InitialFunction->m_ParamTypes[n].GetSizeOnStackDWords();
	}
	return offset;
}

int psCContext::SetArgDWord(psUINT arg, psDWORD value)
{	
	// 获取参数偏移
	int offset = GetArgOffset(arg);

	if (offset < 0) 
		return offset;

	// 检查类型
	psCDataType *dt = &m_InitialFunction->m_ParamTypes[arg];
	if ( ( !dt->IsIntegerType() && !dt->IsBooleanType()) || 
		   dt->IsReference() )
	{
		return psINVALID_TYPE;
	}

	// 设置其值
	m_StackFramePointer[offset] = value;
	return psSUCCESS;
}

int psCContext::SetArgFloat(psUINT arg, float value)
{	
	// 获取参数偏移
	int offset = GetArgOffset(arg);
	if (offset < 0)
		return offset;

	// 检查类型
	psCDataType *dt = &m_InitialFunction->m_ParamTypes[arg];
	if ( !dt->IsFloatType() || dt->IsReference() )
	{
		return psINVALID_TYPE;
	}

	// 设置其值
	*(float*)(&m_StackFramePointer[offset]) = value;

	return psSUCCESS;
}

int psCContext::SetArgDouble(psUINT arg, double value)
{
	// 获取参数偏移
	int offset = GetArgOffset(arg);
	if (offset < 0) 
		return offset;

	// 检查类型
	psCDataType *dt = &m_InitialFunction->m_ParamTypes[arg];
	if ( !dt->IsDoubleType() || dt->IsReference() )
	{
		return psINVALID_TYPE;
	}

	// 设置其值
	*(double*)(&m_StackFramePointer[offset]) = value;

	return psSUCCESS;
}


int psCContext::SetArgPointer(psUINT arg, void *obj)
{
	if( m_Status != tsPrepared || m_InitialFunction == NULL)
		return psCONTEXT_NOT_PREPARED;

	int offset = GetArgOffset( arg );
	if ( offset < 0 )
		return offset;

	psCDataType *dt = &m_InitialFunction->m_ParamTypes[arg];
    if ( !dt->IsReference() && dt->IsObject() )
	{
		void *mem = (void*)(m_StackFramePointer + offset);

		// 调用类的构造函数
		psSTypeBehaviour *beh = &dt->ObjectType()->m_Beh;

		// 调用对象的拷贝构造函数
		if( beh->hasCopyConstructor )
		{
			m_pEngine->CallObjectMethod( mem, obj, m_pModule->GetID(), beh->copy);
		}
		else
		{
			// 如果没有拷贝构造函数,做一个简单的内存拷贝
			memcpy(mem, obj, dt->GetSizeInMemoryBytes());
		}
	}else
	{
		// 检查类型
		if ( !dt->IsReference() )
		{
			return psINVALID_TYPE;
		}
		// 设置其值
		m_StackFramePointer[offset] = (psDWORD)obj;	
	}
	return psSUCCESS;
}

psDWORD psCContext::GetReturnDWord()
{
	if( m_Status != tsProgramFinished ) return 0;

	psCDataType *dt = &m_InitialFunction->m_ReturnType;

	if( dt->IsObject() ) return 0;
	if (dt->IsReference()) 
	{
		psDWORD addr = psDWORD(m_ReturnVal);
		return *((psDWORD*)addr);
	}else
	{
		return psDWORD( m_ReturnVal );
	}
}

float psCContext::GetReturnFloat()
{
	if( m_Status != tsProgramFinished ) return 0;

	psCDataType *dt = &m_InitialFunction->m_ReturnType;

	if( dt->IsObject() ) return 0;

	if (dt->IsReference())
	{
		psDWORD addr = psDWORD(m_ReturnVal);
		return *((float*)addr);
	}else
	{
		return *((float*)(&m_ReturnVal));
	}
}

double psCContext::GetReturnDouble()
{
	if( m_Status != tsProgramFinished ) return 0;

	psCDataType *dt = &m_InitialFunction->m_ReturnType;

	if( dt->IsObject() ) return 0;

	if (dt->IsReference())
	{
		psDWORD addr = psDWORD(m_ReturnVal);
		return *((double*)addr);
	}else
	{
		return *(double*)(&m_ReturnVal);
	}
}

void *psCContext::GetReturnPointer()
{
	if( m_Status != tsProgramFinished ) return 0;

	psCDataType *dt = &m_InitialFunction->m_ReturnType;

	if( !dt->IsReference() && !dt->IsObject() ) return 0;

	return (void*)(psDWORD(m_ReturnVal));
}

int psCContext::Abort()
{
	// TODO: Make thread safe
	if( m_pEngine == NULL ) return psERROR;

	if( m_Status == tsSuspended )
	{
		m_Status = tsProgramAborted;

		// 清空堆栈
		CleanStack();
	}
	m_CallStack.clear();

	return 0;
}


psIScriptEngine* psCContext::GetEngine() 
{
	return m_pEngine;
}

int psCContext::GetState()
{
	if( m_Status == tsSuspended )
		return psEXECUTION_SUSPENDED;

	if( m_Status == tsActive )
		return psEXECUTION_ACTIVE;

	if( m_Status == tsUnhandledException )
		return psEXECUTION_EXCEPTION;

	if( m_Status == tsProgramFinished )
		return psEXECUTION_FINISHED;

	if( m_Status == tsPrepared )
		return psEXECUTION_PREPARED;

	if( m_Status == tsUninitialized )
		return psEXECUTION_UNINITIALIZED;

	return psERROR;
}

int psCContext::Prepare(int modID, int funcId, void* obj)
{
	// 检查当前状态
	if( m_Status == tsActive || m_Status == tsSuspended )
		return psCONTEXT_ACTIVE;

	int ArgumentsSize = 0;
	if ( funcId < 0 || modID < 0 )
	{
		if ( m_InitialFunction == NULL )
			return psNO_FUNCTION;

		m_CurrentFunction = m_InitialFunction;
		ArgumentsSize	  = m_CurrentFunction->GetArgumentsSpaceOnStack();
	}else
	{
		// 检查引擎指针
		if ( m_pEngine == NULL )
			return psERROR;

		if( m_pModule ) 
		{	
			// 移除先前的模块引用计数
			m_pModule->ReleaseContextRef();
		}

		// 重新获取新的模块
		m_pModule = (psCModule*)( m_pEngine->GetModule(modID) );
		if( m_pModule == NULL ) 
		{
			return psNO_MODULE;
		}
		else 
		{
			// 添加引用计数
			m_pModule->AddContextRef(); 
		}

		// 获取起动函数
		m_InitialFunction = m_pModule->GetScriptFunction(funcId);
		m_CurrentFunction = m_InitialFunction;
		if (m_CurrentFunction == NULL)
			return psNO_FUNCTION;

		// 计算参数的大小
		ArgumentsSize		 = m_CurrentFunction->GetArgumentsSpaceOnStack();

		// 计算堆栈大小
		int stackSize			 = ArgumentsSize + m_CurrentFunction->m_StackNeeded + RESERVE_STACK;;
		stackSize = stackSize > m_pEngine->GetInitialContextStackSize() ? stackSize : m_pEngine->GetInitialContextStackSize();

		if( stackSize != m_StackBlockSize )
		{
			// 如果当前堆栈大小与需示的不相等,删除所有已分配的堆
			for( size_t n = 0; n < m_StackBlocks.size(); n++ )
			{
				if( m_StackBlocks[n] )
				{
					psDWORD* buf = m_StackBlocks[n];
					m_StackBlocks[n] = NULL;
					delete[] buf;
				}
			}
			m_StackBlocks.clear();

			// 设置堆的大小
			m_StackBlockSize = stackSize;

			// 生新分配一个新的堆
			psDWORD *pStack = new psDWORD[m_StackBlockSize];
			m_StackBlocks.push_back(pStack);
		}
	}
	if ( (m_CurrentFunction->GetObjectType() != NULL && obj == NULL ) || 
		 (m_CurrentFunction->GetObjectType() == NULL && obj != NULL ) )
	{
		return psIVALID_OBJECT_POINTER;
	}

	// 设置二进制代码的位置
	m_BinaryCode			  = m_CurrentFunction->m_BinaryCode;
	if ( m_BinaryCode == NULL )
		return psNO_BYTECODE;

	// 重设当前的状态
	m_ExceptionLine			  = -1;
	m_ExceptionFunctionId	  = -1;
	m_ExceptionModuleId		  = -1;
	m_Status				  = tsPrepared;
	m_ReturnVal				  = 0;

	// 设置堆栈基址和堆栈指针
	m_StackFramePointer		  = m_StackBlocks[0]  + m_StackBlockSize - ArgumentsSize - RESERVE_STACK;
	m_StackPointer			  = m_StackFramePointer;
	m_StackIndex			  = 0;

	// 把所有的参数都设为0
	memset(m_StackPointer,        0, sizeof(psDWORD) * ArgumentsSize );
	
	// 如果返回值是类对象并且不是引用类型
	if (  m_CurrentFunction->m_ReturnType.IsObject() && 
		 !m_CurrentFunction->m_ReturnType.IsReference())
	{
		void* retPointer = m_pEngine->AllocObject( m_pModule->GetID(), m_CurrentFunction->m_ReturnType.ObjectType() );
		if ( retPointer == NULL)
		{
			return psERROR;
		}

		// Set object pointer
		if ( obj )
			m_StackFramePointer[1] = (psDWORD)retPointer;
		else
			m_StackFramePointer[0] = (psDWORD)retPointer;
	}
	
	// 如果是类成员函数
	if ( obj )
	{
		// Set this pointer
		m_StackFramePointer[0] = (psDWORD)obj;
	}

	// Prepare engine
	m_pEngine->PrepareEngine();
	return psSUCCESS;
}

int psCContext::PrepareSpecial(int modID, int funcID)
{
	// 检查引擎指针
	if( m_pEngine == NULL ) return psERROR;

	if( m_Status == tsActive || m_Status == tsSuspended )
		return psCONTEXT_ACTIVE;

	m_ExceptionLine		  = -1;
	m_ExceptionModuleId   = -1;
	m_ExceptionFunctionId = -1;

	if( m_pModule )
	{	
		// 移除先前的模块引用计数
		m_pModule->ReleaseContextRef();
	}

	// 重新获取新的模块
	if ( funcID == psFUNC_STRING )
		m_pModule = m_pEngine->GetStringModule();
	else 
		m_pModule = (psCModule*)( m_pEngine->GetModule(modID) );

	if( m_pModule == NULL ) 
	{
		return psNO_MODULE;
	}
	else 
	{
		// 添加引用计数
		m_pModule->AddContextRef(); 
	}

	// 获取起动函数
	m_InitialFunction = m_pModule->GetSpecialFunction(funcID);
	m_CurrentFunction = m_InitialFunction;
	if( m_CurrentFunction == NULL )
	{
		return psERROR;
	}

	// 设置二进制代码的位置
	m_BinaryCode = m_CurrentFunction->m_BinaryCode;
	if ( m_BinaryCode == NULL )
		return psNO_BYTECODE;

	m_Status = tsPrepared;

	int ArgumentsSize = m_CurrentFunction->GetArgumentsSpaceOnStack();

	// 判数最不的堆栈大小
	int stackSize = ArgumentsSize + m_CurrentFunction->m_StackNeeded;

	stackSize = stackSize > m_pEngine->GetInitialContextStackSize() ? stackSize : m_pEngine->GetInitialContextStackSize();

	if( stackSize != m_StackBlockSize )
	{
		// 如果当前堆栈大小与需示的不相等,删除所有已分配的堆
		for( size_t n = 0; n < m_StackBlocks.size(); n++ )
		{
			if( m_StackBlocks[n] )
			{
				psDWORD* buf = m_StackBlocks[n];
				m_StackBlocks[n] = NULL;
				delete[] buf;
			}
		}
		m_StackBlocks.clear();

		// 设置堆的大小
		m_StackBlockSize = stackSize;

		// 生新分配一个新的堆
		psDWORD *pStack = new psDWORD[m_StackBlockSize];
		m_StackBlocks.push_back(pStack);
	}

	// 设置堆栈基址和堆栈指针
	m_StackFramePointer = m_StackBlocks[0] + m_StackBlockSize - ArgumentsSize - RESERVE_STACK;
	m_StackPointer		= m_StackFramePointer;
	m_StackIndex		= 0;

	// 把所有的参数都设为0
	memset(m_StackPointer, 0, sizeof(psDWORD)*ArgumentsSize);

	// Prepare engine
	m_pEngine->PrepareEngine();

	return psSUCCESS;
}

int psCContext::ExecuteInstr()
{
	// 检查安全性
	if( m_pEngine == NULL || m_pModule == NULL || m_BinaryCode == NULL)
		return psERROR;

	if ( m_Status != tsSuspended && m_Status != tsPrepared )
		return psERROR;

	psBYTE*			   curBC   = m_BinaryCode;

	// 基地址
	psBYTE* bases[] =
	{
		0,							   		// 立即数寻址
		m_pModule->GetConstantMemPtr(),		// 常数寻址
		(psBYTE*)m_StackFramePointer,		// 堆栈寻址
		m_pModule->GetGlobalMemPtr(), 		// 全局寻址
		(psBYTE*)(m_StackFramePointer[0]),	// 类对象指针
	};
	#define OP1(bc)		   (bases[BC_BASE1(bc)] + BC_OFFSET1(bc))
	#define V_INT(op)      ( *((int*)op) )
	#define V_INT1(bc)     V_INT(OP1(bc))

	psBYTE* dstBC = NULL;
	if ( *curBC == BC_JMP )
	{	
		// 如果当前指今是绝对跳转, 在跳转的目的地下断点
		dstBC = curBC + BCS_JMP + BC_OFFSET1(curBC);	
	}else if ( *curBC == BC_JZ )
	{
		// 如果当前指今是条件跳转, 在跳转的目的地下断点 
		if ( V_INT1( curBC ) == 0)
			dstBC = curBC + BCS_JZ + BC_OFFSET2(curBC);
		else
			dstBC = curBC + BCS_JZ;

	}else if ( *curBC == BC_JNZ )
	{
	   // 如果当前指今是条件跳转, 在跳转的目的地下断点
		if ( V_INT1( curBC ) != 0)
			dstBC = curBC + BCS_JNZ + BC_OFFSET2(curBC);
		else
			dstBC = curBC + BCS_JNZ;
	}else if ( *curBC == BC_CALL )
	{
		 int funcId = BC_OFFSET1( curBC );
		 psCScriptFunction* func = m_pModule->GetScriptFunction(funcId);
		 assert(func);
		 dstBC = func->m_BinaryCode;
	}else if ( *curBC == BC_RET )
	{
		if ( m_CallStack.size() > 0 )
		{
			CALL_STACK& cs = m_CallStack[m_CallStack.size()-1];
			dstBC = cs.binaryCode;
		}
	}else
	{
		// 移动到下一条指今, 把此条此今写为挂起指今
		dstBC = curBC + g_psBCSize[*curBC];
	}

	int  oldBC = BC_SUSPEND;
	if ( dstBC && *dstBC != BC_SUSPEND )
	{
		oldBC = *(int*)dstBC;
		*(int*)dstBC = BC_SUSPEND;
	}

	psPushActiveContext((psIScriptContext *)this);
	// 运行
	try 
	{		
		m_Status = tsActive;
		ExecuteNext();
	}
	catch (...)
	{
		SetException( TXT_SYSTEM_EXECEPTION );
	}
	psPopActiveContext((psIScriptContext *)this);

	if ( dstBC )
		*(int*)dstBC = oldBC;

	if ( m_Status == tsSuspended )
		 return psEXECUTION_SUSPENDED;

	if( m_Status == tsProgramFinished )
		return psEXECUTION_FINISHED;

	if( m_Status == tsUnhandledException )
		return psEXECUTION_EXCEPTION;

	return psERROR;
}

int psCContext::Execute( int flag )
{
	// 检查安全性
	if( m_pEngine == NULL || m_pModule == NULL || m_BinaryCode == NULL)
		return psERROR;

	psCScriptFunction* curFunc = m_CurrentFunction;
	psBYTE*			   curBC   = m_BinaryCode;
	psBYTE			   curOp   = *m_BinaryCode;
    int                callStack = (int)m_CallStack.size() - 1;

	if( m_Status == tsSuspended )
	{
		// 如果当前处在挂起状态, 要恢复原来的代码以便继续执行
		// NOTE: 只有处于用户断点时才执行此操作
		int curBreakPos = int( curBC - curFunc->m_BinaryCode );
		if  ( curFunc->RemoveBreakPointByPos( curBreakPos ) )
		{	
			// 更新当前操作数
			curOp = *curBC;

			// 执行这条汇编指令
			int r = ExecuteInstr();	
			
			// 恢复原来的断点
			curFunc->InsertBreakPointbyPos( curBreakPos );	

			bool bRet = (r != psEXECUTION_SUSPENDED);

			// 如果当前指今是返回指今，并且是单步运行要中断运行
			if ( curOp == BC_RET && (flag & (psEXEC_STEP_INTO|psEXEC_STEP_OVER) ) )
			{
				bRet = true;
			} 
		    if ( bRet )
			{
				if ( flag & psEXEC_DEBUG )
				{
					psIDebugger* debugger = m_pEngine->GetDebugger();
					if ( debugger )
						return debugger->OnDebugEvent( this );
				}
				return r;
			}
		}
	}else if ( m_Status != tsPrepared )
	{
		return psERROR;
	}

	//-------------------------------------------------------------//
	typedef psCScriptFunction::BREAK_POINT BREAK_POINT;
	std::vector< BREAK_POINT > tmpBreakPoints;
	#define INSERT_BREAK_POINT(bc)  if ( *bc != BC_SUSPEND ) { BREAK_POINT bp = {(int)(bc), *(int*)(bc)}; tmpBreakPoints.push_back(bp); *(int*)(bc) = BC_SUSPEND; }
	if ( flag & ( psEXEC_STEP_INTO | psEXEC_STEP_OVER ) )
	{
		bool bHasRet = false;

		int realLineno;
		// 得到下一行行号
		int nextpos    = -1;
		int curpos = int(curBC - curFunc->m_BinaryCode);
		int lineno = curFunc->GetLineNumber( curpos );	
		int prepos = curFunc->GetPosFromLineNumber( lineno, realLineno );
		lineno	   = curFunc->GetNextLineNumber(lineno, &nextpos );
			
		//-------------------------------------------------------------//
		// 设置临时的断点
		psBYTE* bc = curBC;
		do 
		{
			assert( curOp != BC_SUSPEND );
			if ( curOp == BC_JMP )
			{
				int offset = BC_OFFSET1(bc);
				psBYTE* dstBC = bc + BCS_JMP + offset;

				if ( dstBC - curFunc->m_BinaryCode >= curFunc->GetMaxLinePos() )
				{
					bHasRet = true;
				}
				// 在跳转目的地下断点
				if ( ( offset > 0 && ( dstBC - curFunc->m_BinaryCode ) >= nextpos ) ||
					 ( offset < 0 && ( dstBC - curFunc->m_BinaryCode ) <= prepos ) )
				{
					INSERT_BREAK_POINT( dstBC );
					break;
				}
			}else if ( curOp == BC_JZ || curOp == BC_JNZ ) 
			{
				// 在成功/不成功跳转的目的地下断点
				int offset = BC_OFFSET2(bc);
				psBYTE* dstBC = bc + BCS_JZ + offset;

				if (  ( offset > 0 && ( dstBC - curFunc->m_BinaryCode ) >= nextpos ) ||
					  ( offset < 0 && ( dstBC - curFunc->m_BinaryCode ) <= prepos ) )
				{
					INSERT_BREAK_POINT( dstBC );
				}
			}else if ( curOp == BC_CALL )
			{
			   if ( flag & psEXEC_STEP_INTO )
			   {
				   int funcId = BC_OFFSET1(bc);
				   psCScriptFunction* func = m_pModule->GetScriptFunction(funcId);
				   assert(func);

				   int minPos = func->GetMinPos();
				   // 如果是析构函数，或者没有行号信息就不要跟进
				   if ( minPos > 0 && (func->m_funcFlag & psCScriptFunction::DESTRUCTOR) == 0 )
				   {
					   psBYTE* dstBC = func->m_BinaryCode + minPos;
					   INSERT_BREAK_POINT( dstBC );
				   }
   			   }
			}

			bc += g_psBCSize[curOp];
			curOp = *bc;
		}while( bc < curFunc->m_BinaryCode + nextpos );

		// 在下一行下断点
		if ( !bHasRet && lineno <= curFunc->GetMaxLineNumber() && nextpos >= 0 )
		{
			psBYTE* dstBC = curFunc->m_BinaryCode + nextpos;
			INSERT_BREAK_POINT( dstBC );
		}else
		{
			bHasRet = true;
		}

		if ( bHasRet && callStack >= 0 )
		{
			CALL_STACK& cs = m_CallStack[callStack];
			cs.isBreak = 1;
		}
	}

   	psPushActiveContext((psIScriptContext *)this);
	// 运行
	try 
	{		
		m_Status = tsActive;
		ExecuteNext();
	}
	catch (...)
	{
		SetException( TXT_SYSTEM_EXECEPTION );
	}
	psPopActiveContext((psIScriptContext *)this);

	// 清除所有的临时断点	
	for (size_t i = 0; i < tmpBreakPoints.size(); ++i )
	{
		BREAK_POINT& bp = tmpBreakPoints[i];
		*(int*)(bp.pos) = bp.bc;
	}

	if ( flag & psEXEC_DEBUG )
	{
		psIDebugger* debugger = m_pEngine->GetDebugger();
		if ( debugger )
		{
			return debugger->OnDebugEvent( this );
		}
	}

	if( m_Status == tsSuspended )
		return psEXECUTION_SUSPENDED;

	if( m_Status == tsProgramFinished )
		return psEXECUTION_FINISHED;

	if( m_Status == tsUnhandledException )
		return psEXECUTION_EXCEPTION;

	return psERROR;
}

void psCContext::ExecuteNext()
{
	//----------------------------------------------------------------------//
	// 宏定义
	//----------------------------------------------------------------------//
	#define OP1(bc)		   (bases[BC_BASE1(bc)] + BC_OFFSET1(bc))
	#define OP2(bc)		   (bases[BC_BASE2(bc)] + BC_OFFSET2(bc))
	#define OP3(bc)		   (bases[BC_BASE3(bc)] + BC_OFFSET3(bc))

	#define V_INT(op)      ( *((int*)op) )
	#define V_UINT(op)      ( *((int*)op) )

	#define V_QWORD(op)    ( *((__int64*)op) )
	#define V_FLOAT(op)    ( *((float*)op) )
	#define V_DOUBLE(op)   ( *((double*)op) )

	#define V_INT1(bc)     V_INT(OP1(bc))
	#define V_INT2(bc)     V_INT(OP2(bc))
	#define V_INT3(bc)     V_INT(OP3(bc))
	#define V_UINT1(bc)    V_INT(OP1(bc))
	#define V_UINT2(bc)    V_INT(OP2(bc))
	#define V_UINT3(bc)    V_INT(OP3(bc))

	#define V_QWORD1(bc)   V_QWORD(OP1(bc))
	#define V_QWORD2(bc)   V_QWORD(OP2(bc))
	#define V_QWORD3(bc)   V_QWORD(OP3(bc))

	#define V_FLOAT1(bc)   V_FLOAT(OP1(bc))
	#define V_FLOAT2(bc)   V_FLOAT(OP2(bc))
	#define V_FLOAT3(bc)   V_FLOAT(OP3(bc))

	#define V_DOUBLE1(bc)  V_DOUBLE(OP1(bc))
	#define V_DOUBLE2(bc)  V_DOUBLE(OP2(bc))
	#define V_DOUBLE3(bc)  V_DOUBLE(OP3(bc))

	// 获取运行上下文
	psBYTE  *l_bc = m_BinaryCode;
	psDWORD *l_sp = m_StackPointer;
	psDWORD *l_fp = m_StackFramePointer;

	// 基地址
	psBYTE* bases[] =
	{
		0,									// 立即数寻址
		m_pModule->GetConstantMemPtr(),		// 常数寻址
		(psBYTE*)m_StackFramePointer,		// 堆栈寻址
		m_pModule->GetGlobalMemPtr(),		// 全局寻址
		(psBYTE*)(m_StackFramePointer[0]),	// 类对象指针
	};

	for (;;)
	{	
		switch(*l_bc)
		{
		case BC_NOP:
			l_bc += BCS_NOP;
			break;
		case BC_SUSPEND:
			{
				// 把当前的运行上下文保存起来
				m_BinaryCode = l_bc;
				m_StackPointer = l_sp;
				m_StackFramePointer = l_fp;
					
				m_Status = tsSuspended;
				return;
			}break;
			//--------------
			// memory access functions
		case BC_POP:
			l_sp += BC_OFFSET1(l_bc);
			l_bc += BCS_POP;
			break;
		case BC_PUSH:
			l_sp -= BC_OFFSET1(l_bc);
			l_bc += BCS_PUSH;
			break;
		case BC_PUSH4:
			--l_sp;
			*l_sp = V_INT1(l_bc);
			l_bc+= BCS_PUSH4;
			break;
		case BC_PUSH8:
			l_sp -= 2;
			*((__int64*)l_sp) = V_QWORD1(l_bc);
			l_bc += BCS_PUSH8;
			break;
		case BC_PUSHREF:
			--l_sp;
			*l_sp = (psDWORD)OP1(l_bc);
			
			l_bc += BCS_PUSHREF;
			break;
		case BC_MOV1:
			*( (char*)OP1(l_bc) ) = *( (char*)OP2(l_bc) );
			l_bc += BCS_MOV1;
			break;
		case BC_MOV2:
			*( (short*)OP1(l_bc) ) = *( (short*)OP2(l_bc) );
			l_bc += BCS_MOV2;
			break;
		case BC_MOV4:
			V_INT1(l_bc) = V_INT2(l_bc);
			l_bc += BCS_MOV4;
			break;
		case BC_MOV8:
			V_QWORD1(l_bc) = V_QWORD2(l_bc);
			l_bc += BCS_MOV8;
			break;
		//----------------------------------------------------------------------//
		case BC_DEFREF:
			V_INT1(l_bc) = (int)OP2(l_bc);
			l_bc += BCS_DEFREF;
			break;
		case BC_RDREF1:
			*( (char*)OP1(l_bc) ) = *(char*)(V_INT2(l_bc));
			l_bc += BCS_RDREF1;
			break;
		case BC_WTREF1:
			*(char*)(V_INT1(l_bc)) = *( (char*)OP2(l_bc) );
			l_bc += BCS_WTREF1;
			break;
		case BC_RDREF2:
			*( (short*)OP1(l_bc) ) = *(short*)(V_INT2(l_bc));
			l_bc += BCS_RDREF2;
			break;
		case BC_WTREF2:
			*(short*)(V_INT1(l_bc)) = *( (short*)OP2(l_bc) );
			l_bc += BCS_WTREF2;
			break;
		case BC_RDREF4:
			V_INT1(l_bc) = *(int*)(V_INT2(l_bc));
			l_bc += BCS_RDREF4;
			break;
		case BC_WTREF4:
			*(int*)(V_INT1(l_bc)) = V_INT2(l_bc);
			l_bc += BCS_WTREF4;
			break;
		case BC_RDREF8:
			V_QWORD1(l_bc) = *(__int64*)(V_INT2(l_bc));
			l_bc += BCS_RDREF8;
			break;
		case BC_WTREF8:
			*(__int64*)(V_INT1(l_bc)) = V_QWORD2(l_bc);
			l_bc += BCS_WTREF8;
			break;
		//----------------------------------------------------------------------//
		case BC_RRET1:
			*(char*)(OP1(l_bc)) = *((char*)(&m_ReturnVal));
			l_bc += BCS_RRET1;
			break;
		case BC_SRET1:
			*((char*)&m_ReturnVal) = *(char*)(OP1(l_bc));
			l_bc += BCS_SRET1;
			break;
		case BC_RRET2:
			*(short*)(OP1(l_bc)) = *((short*)(&m_ReturnVal));
			l_bc += BCS_RRET2;
			break;
		case BC_SRET2:
			*((short*)&m_ReturnVal) = *(short*)(OP1(l_bc));
			l_bc += BCS_SRET2;
			break;
		case BC_RRET4:
			V_INT1(l_bc) = *((int*)(&m_ReturnVal));
			l_bc += BCS_RRET4;
			break;
		case BC_SRET4:
			*((int*)(&m_ReturnVal)) = V_INT1(l_bc);
			l_bc += BCS_SRET4;
			break;
		case BC_RRET8:
			V_QWORD1(l_bc) = m_ReturnVal;
			l_bc += BCS_RRET8;
			break;
		case BC_SRET8:
			m_ReturnVal = V_QWORD1(l_bc);
			l_bc += BCS_SRET8;
			break;
		//-------------------------------------------------------------//
		case BC_PGA:
			{
				int idx = BC_OFFSET2(l_bc);
				assert( idx < 0 );
				V_INT1(l_bc) = psDWORD(m_pEngine->GetGlobalAddress(-idx - 1));
				
				l_bc += BCS_PGA;
			}break;
		case BC_PSP:
			{
				V_INT1(l_bc) = (psUINT)l_sp;
				l_bc += BCS_PSP;
			}break;
		case BC_MEMCPY:
			{
				int size = BC_OFFSET3(l_bc);
				int dest = V_INT1(l_bc);
				int src = V_INT2(l_bc);
				if (dest == 0 || src == 0)
				{
					SetException(TXT_NULL_POINTER_ACCESS);
					return;
				}
				memcpy((void*)dest, (void*)src, size);
				l_bc += BCS_MEMCPY;
			}break;
			//--------------------------------------------------------------
			// Path control functions
		case BC_CALL:
			{
				int funcId = BC_OFFSET1(l_bc);
				l_bc += BCS_CALL;
				assert( funcId >= 0 );
				
				// 把当前的运行上下文保存起来
				m_BinaryCode = l_bc;
				m_StackPointer = l_sp;
				m_StackFramePointer = l_fp;
				
				CallScriptFunction(m_pModule, m_pModule->GetScriptFunction(funcId) );
				
				//重新获取运行上下文
				l_bc = m_BinaryCode;
				l_sp = m_StackPointer;
				l_fp = m_StackFramePointer;
				
				// 需要重设堆栈基址, 类对象指针
				bases[PSBASE_STACK] = (psBYTE*)m_StackFramePointer;
				bases[PSBASE_HEAP]  = (psBYTE*)m_StackFramePointer[0];

				// 可能引发异常
				if( m_Status != tsActive )
				{			
					m_BinaryCode = l_bc;
					m_StackPointer = l_sp;
					m_StackFramePointer = l_fp;
					return;
				}
			}
			break;
		case BC_RET:
			{	
				if( m_CallStack.size() == 0 )
				{
					m_Status = tsProgramFinished;
					return;
				}
				int add = BC_OFFSET1(l_bc);		

				// 弹出调用栈
				bool isBreak = PopCallState();
				
				//重新获取运行上下文
				l_bc = m_BinaryCode;
				l_sp = m_StackPointer;
				l_fp = m_StackFramePointer;

				// 需要重设堆栈基址, 类对象指针
				bases[PSBASE_STACK] = (psBYTE*)m_StackFramePointer;
				bases[PSBASE_HEAP]  = (psBYTE*)m_StackFramePointer[0];
				
				// 弹出函数参数
				l_sp += add;
				if ( isBreak )
				{
					m_Status	= tsSuspended;
					m_BinaryCode = l_bc;
					m_StackPointer = l_sp;
					m_StackFramePointer = l_fp;
					return;
				}
			}
			break;
		case BC_CALLSYS:
			{
				int funcId  = BC_OFFSET1(l_bc);
				int argSize = BC_OFFSET2(l_bc);

				assert( funcId < 0 && argSize >= 0 );
				// NOTE: 注意要在保存当前动行上下文件之前就更新代码的位置
				//		 否则会死循环
				l_bc += BCS_CALLSYS;

				// 把当前的运行上下文保存起来
				m_BinaryCode = l_bc;
				m_StackPointer = l_sp;
				m_StackFramePointer = l_fp;

				l_sp += CallSystemFunction(funcId, argSize, this);

				// 可能引发异常
				if( m_Status != tsActive )
				{
					m_BinaryCode = l_bc;
					m_StackPointer = l_sp;
					m_StackFramePointer = l_fp;
					return;
				}
			}break;
		case BC_JMP:
			{
				l_bc += BCS_JMP + BC_OFFSET1(l_bc);
			}break;
		case BC_JMPP:
			{
				l_bc += BCS_JMPP + SIZEOFBC(1) * V_INT1(l_bc);
			}break;
		case BC_JNZ:
			{
				if (V_INT1(l_bc) != 0)
					l_bc += BCS_JNZ + BC_OFFSET2(l_bc);
				else
					l_bc += BCS_JNZ;
			}break;
		case BC_JZ:
			{	
				if (V_INT1(l_bc) == 0)
					l_bc += BCS_JZ + BC_OFFSET2(l_bc);
				else
					l_bc += BCS_JZ;
			}break;
			//-----------------------------------------------
			// Integer operations 
		case BC_ADDi:
			V_INT1(l_bc) = V_INT2(l_bc) + V_INT3(l_bc);
			l_bc += BCS_ADDi;
			break;
		case BC_SUBi:
			V_INT1(l_bc) = V_INT2(l_bc) - V_INT3(l_bc);
			l_bc += BCS_SUBi;
			break;
		case BC_MULi:
			V_INT1(l_bc) = V_INT2(l_bc) * V_INT3(l_bc);
			l_bc += BCS_MULi;
			break;
		case BC_DIVi:
			{
				int v = V_INT3(l_bc);
				if (v != 0)
				{
					V_INT1(l_bc) = (V_INT2(l_bc) / v);
					l_bc += BCS_MODi;
				}
				else
				{
					// 把当前的运行上下文保存起来
					m_BinaryCode = l_bc;
					m_StackPointer = l_sp;
					m_StackFramePointer = l_fp;

					// 产生了异常
					SetException(TXT_DIVIDE_BY_ZERO);
					return;
				}
			}break;
		case BC_MODi:
			{
				int v = V_INT3(l_bc);
				if (v != 0)
				{
					V_INT1(l_bc) = (V_INT2(l_bc) % v);
					l_bc += BCS_MODi;
				}
				else
				{	
					l_bc += BCS_MODi;

					// 把当前的运行上下文保存起来
					m_BinaryCode = l_bc;
					m_StackPointer = l_sp;
					m_StackFramePointer = l_fp;

					// 产生了异常
					SetException(TXT_DIVIDE_BY_ZERO);
					return;
				}
			}break;
		case BC_INCi:
			{
				++(V_INT1(l_bc));
				l_bc += BCS_INCi;
			}break;
		case BC_DECi:
			{
				--(V_INT1(l_bc));
				l_bc += BCS_DECi;
			}break;
		case BC_NEGi:
			{
				V_INT1(l_bc) = -(V_INT2(l_bc));
				l_bc += BCS_NEGi;
			}break;
			//------------------------------------------------------------------
			// floating point operations
		case BC_ADDf:
			{
				V_FLOAT1(l_bc) = V_FLOAT2(l_bc) + V_FLOAT3(l_bc);
				l_bc += BCS_ADDf;
			}break;
		case BC_SUBf:
			{
				V_FLOAT1(l_bc) = V_FLOAT2(l_bc) - V_FLOAT3(l_bc);
				l_bc += BCS_SUBf;
			}break;
		case BC_MULf:
			{
				V_FLOAT1(l_bc) = V_FLOAT2(l_bc) * V_FLOAT3(l_bc);
				l_bc += BCS_MULf;
			}break;
		case BC_DIVf:
			{
				V_FLOAT1(l_bc) = V_FLOAT2(l_bc) / V_FLOAT3(l_bc);
				l_bc += BCS_DIVf;
			}break;
		case BC_MODf:
			{
				V_FLOAT1(l_bc) = (float)fmod(V_FLOAT2(l_bc), V_FLOAT3(l_bc));
				l_bc += BCS_MODf;
			}break;
		case BC_INCf:
			{
				V_FLOAT1(l_bc) += 1.0f;
				l_bc += BCS_INCf;
			}break;
		case BC_DECf:
			{
				V_FLOAT1(l_bc) -= 1.0f;
				l_bc += BCS_DECf;
			}break;
		case BC_NEGf:
			{
				V_FLOAT1(l_bc) = -(V_FLOAT2(l_bc));
				l_bc += BCS_NEGf;
			}break;
			//------------------------------------------------------------------------
			// double operations
		case BC_ADDd:
			{
				V_DOUBLE1(l_bc) = V_DOUBLE2(l_bc) + V_DOUBLE3(l_bc);
				l_bc += BCS_ADDd;
			}break;
		case BC_SUBd:
			{
				V_DOUBLE1(l_bc) = V_DOUBLE2(l_bc) - V_DOUBLE3(l_bc);
				l_bc += BCS_SUBd;
			}break;
		case BC_MULd:
			{
				V_DOUBLE1(l_bc) = V_DOUBLE2(l_bc) * V_DOUBLE3(l_bc);
				l_bc += BCS_MULd;
			}break;
		case BC_DIVd:
			{
				V_DOUBLE1(l_bc) = V_DOUBLE2(l_bc) / V_DOUBLE3(l_bc);
				l_bc += BCS_DIVd;
			}break;
		case BC_MODd:
			{
				V_DOUBLE1(l_bc) = fmod(V_DOUBLE2(l_bc), V_DOUBLE3(l_bc));
				l_bc += BCS_MODd;
			}break;
		case BC_INCd:
			{
				V_DOUBLE1(l_bc) += 1.0;
				l_bc += BCS_INCd;
			}break;
		case BC_DECd:
			{
				V_DOUBLE1(l_bc) -= 1.0;
				l_bc += BCS_DECd;
			}break;
		case BC_NEGd:
			{
				V_DOUBLE1(l_bc) = -(V_DOUBLE2(l_bc));
				l_bc += BCS_NEGd;
			}break;
			
			//---------------------------------------------------------------
			// Bitwise operations
		case BC_NOT:
			{
				V_UINT1(l_bc) = ~V_UINT2(l_bc);
				l_bc += BCS_NOT;
			}break;
		case BC_AND:
			{
				V_UINT1(l_bc) = V_UINT2(l_bc) & V_UINT3(l_bc);
				l_bc += BCS_AND;
			}break;
		case BC_OR:
			{			
				V_UINT1(l_bc) = V_UINT2(l_bc) | V_UINT3(l_bc);
				l_bc += BCS_OR;
			}break;
		case BC_XOR:
			{
				V_UINT1(l_bc) = V_UINT2(l_bc) ^ V_UINT3(l_bc);
				l_bc += BCS_XOR;
			}break;
		case BC_SLL:
			{
				V_UINT1(l_bc) = V_UINT2(l_bc) << V_UINT3(l_bc);
				l_bc += BCS_SLL;
			}break;
		case BC_SRL:
			{
				V_UINT1(l_bc) = V_UINT2(l_bc) >> V_UINT3(l_bc);
				l_bc += BCS_SRL;
			}break;
		case BC_SLA:
			{
				V_UINT1(l_bc) = V_UINT2(l_bc) << V_UINT3(l_bc);
				l_bc += BCS_SLA;
			}break;
		case BC_SRA:
			{
				V_UINT1(l_bc) = V_UINT2(l_bc) >> V_UINT3(l_bc);
				l_bc += BCS_SRA;
			}break;
			
			// --------------------------------------------------------------------
			// Comparison operations (int)
		case BC_TEi:
			{
				V_INT1(l_bc) = (V_INT2(l_bc) == V_INT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TEi;
			}break;
		case BC_TNEi:
			{
				V_INT1(l_bc) = (V_INT2(l_bc) != V_INT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TNEi;
			}break;
		case BC_TLi:
			{
				V_INT1(l_bc) = (V_INT2(l_bc) < V_INT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TLi;
			}break;
		case BC_TGi:
			{
				V_INT1(l_bc) = (V_INT2(l_bc) > V_INT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TGi;
			}break;
		case BC_TLEi:
			{
				V_INT1(l_bc) = (V_INT2(l_bc) <= V_INT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TLEi;
			}break;
		case BC_TGEi:
			{
				V_INT1(l_bc) = (V_INT2(l_bc) >= V_INT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TGEi;
			}break;
			// ----------------------------------------------------------------------
			// Comparison operations (float)
		case BC_TEf:
			{
				V_INT1(l_bc) = (V_FLOAT2(l_bc) == V_FLOAT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TEf;
			}break;
		case BC_TNEf:
			{
				V_INT1(l_bc) = (V_FLOAT2(l_bc) != V_FLOAT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TNEf;
			}break;
		case BC_TLf:
			{
				V_INT1(l_bc) = (V_FLOAT2(l_bc) < V_FLOAT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TLf;
			}break;
		case BC_TGf:
			{
				V_INT1(l_bc) = (V_FLOAT2(l_bc) > V_FLOAT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TGf;
			}break;
		case BC_TLEf:
			{
				V_INT1(l_bc) = (V_FLOAT2(l_bc) <= V_FLOAT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TLEf;
			}break;
		case BC_TGEf:
			{
				V_INT1(l_bc) = (V_FLOAT2(l_bc) >= V_FLOAT3(l_bc)) ? 1 : 0;
				l_bc += BCS_TGEf;
			}break;
			
			// ----------------------------------------------------------------------
			// Comparison operations (double)
		case BC_TEd:
			{
				V_INT1(l_bc) = (V_DOUBLE2(l_bc) == V_DOUBLE3(l_bc)) ? 1 : 0;
				l_bc += BCS_TEf;
			}break;
		case BC_TNEd:
			{
				V_INT1(l_bc) = (V_DOUBLE2(l_bc) != V_DOUBLE3(l_bc)) ? 1 : 0;
				l_bc += BCS_TNEf;
			}break;
		case BC_TLd:
			{
				V_INT1(l_bc) = (V_DOUBLE2(l_bc) < V_DOUBLE3(l_bc)) ? 1 : 0;
				l_bc += BCS_TLd;
			}break;
		case BC_TGd:
			{
				V_INT1(l_bc) = (V_DOUBLE2(l_bc) > V_DOUBLE3(l_bc)) ? 1 : 0;
				l_bc += BCS_TGd;
			}break;
		case BC_TLEd:
			{
				V_INT1(l_bc) = (V_DOUBLE2(l_bc) <= V_DOUBLE3(l_bc)) ? 1 : 0;
				l_bc += BCS_TLEd;
			}break;
		case BC_TGEd:
			{
				V_INT1(l_bc) = (V_DOUBLE2(l_bc) >= V_DOUBLE3(l_bc)) ? 1 : 0;
				l_bc += BCS_TGEd;
			}break;
			
			//----------------------------------------------------------------------
			// type conversion
		case BC_I2F:
			{
				V_FLOAT1(l_bc) = (float)V_INT2(l_bc);
				l_bc += BCS_I2F;
			}break;
		case BC_F2I:
			{
				V_INT1(l_bc) = (int)V_FLOAT2(l_bc);
				l_bc += BCS_F2I;
			}break;
		case BC_D2I:
			{
				V_INT1(l_bc) = (int)V_DOUBLE2(l_bc);
				l_bc += BCS_D2I;
			}break;
		case BC_I2D:
			{
				V_DOUBLE1(l_bc) = (double)V_INT2(l_bc);
				l_bc += BCS_I2D;
			}break;
		case BC_D2F:
			{
				V_FLOAT1(l_bc) = (float)V_DOUBLE2(l_bc);
				l_bc += BCS_D2F;
			}break;
		case BC_F2D:
			{
				V_DOUBLE1(l_bc) = (double)V_FLOAT2(l_bc);
				l_bc += BCS_F2D;
			}break;
		case BC_I2B:
			{
				*( (unsigned char*)OP1(l_bc) ) = (unsigned char)V_INT2(l_bc);
				l_bc += BCS_I2B;
			}break;
		case BC_B2I:
			{
				V_INT1(l_bc) = *( (unsigned char*)OP2(l_bc) );
				l_bc += BCS_B2I;
			}break;
		case BC_I2W:
			{
				*( (unsigned short*)OP1(l_bc) ) = (unsigned short)V_INT2(l_bc);
				l_bc += BCS_I2W;
			}break;
		case BC_W2I:
			{
				V_INT1(l_bc) = *( (unsigned short*)OP2(l_bc) );
				l_bc += BCS_W2I;
			}break;
		case BC_I2C:
			{	
				*( (char*)OP1(l_bc) ) = (char)V_INT2(l_bc);
				l_bc += BCS_I2C;
			}break;
		case BC_C2I:
			{
				V_INT1(l_bc) = *( (char*)OP2(l_bc) );
				l_bc += BCS_C2I;
			}break;
		case BC_I2S:
			{	
				*( (short*)OP1(l_bc) ) = (short)V_INT2(l_bc);
				l_bc += BCS_I2S;
			}break;
		case BC_S2I:
			{
				V_INT1(l_bc) = *( (short*)OP2(l_bc) );
				l_bc += BCS_S2I;
			}break;
		default:
			assert(false);
			SetException(TXT_UNRECOGNIZED_BYTE_CODE);
			return;
		}

	}
}

void psCContext::CleanStack()
{
	m_InExceptionHandler = true;

	// Run the clean up code for each of the functions called
	CleanStackFrame();

	while( m_CallStack.size() > 0 )
	{
		PopCallState();

		CleanStackFrame();
	}
	m_InExceptionHandler = false;
}

void psCContext::CleanStackFrame()
{
	// 清除参数类对象(非引用传递)
	int offset = 0;
	for( size_t n = 0; n < m_CurrentFunction->m_ParamTypes.size(); ++n )
	{
		psCDataType& type = m_CurrentFunction->m_ParamTypes[n];
		if( type.IsObject() && !type.IsReference()  )
		{
			void* obj = (void*)( m_StackFramePointer + offset );

			m_pEngine->CallObjectMethod( obj, m_pModule->GetID(), type.ObjectType()->m_Beh.destruct );
		}
		offset += type.GetSizeOnStackDWords();
	}

	if ( m_CurrentFunction == m_InitialFunction )
	{
		// 清除返回值类对象
		if ( m_CurrentFunction->m_ReturnType.IsObject() && !m_CurrentFunction->m_ReturnType.IsReference() )
		{
			void* retPointer = NULL;
			if ( m_CurrentFunction->GetObjectType() == NULL )
				retPointer = (void*)m_StackFramePointer[0];
			else
				retPointer = (void*)m_StackFramePointer[1];

			// 释放内存
			free( retPointer );
		}
	}
}

void psCContext::PushCallState()
{
	CALL_STACK cs;
	cs.binaryCode		  = m_BinaryCode;
	cs.currentFunction	  = m_CurrentFunction;
	cs.module			  = m_pModule;
	cs.stackFramePointer  = m_StackFramePointer;
	cs.stackIndex		  = m_StackIndex;
	cs.stackPointer		  = m_StackPointer;
	cs.isBreak			  = 0;
	m_CallStack.push_back( cs );
}

bool psCContext::PopCallState()
{
	CALL_STACK& cs		  = m_CallStack[m_CallStack.size() - 1];

	m_StackFramePointer   = cs.stackFramePointer;
	m_CurrentFunction     = cs.currentFunction;
	m_BinaryCode          = cs.binaryCode;
	m_StackPointer        = cs.stackPointer;
	m_StackIndex          = cs.stackIndex;
	m_pModule             = cs.module;
	bool isBreak		  = cs.isBreak ? true : false;
	m_CallStack.pop_back();
	return isBreak;
}


void psCContext::CallScriptFunction(psCModule* module, psCScriptFunction* func )
{
	// 将脚本运行相关的数据入堆
	PushCallState();

	// 重新设置当前函数,脚本模块, 程序指针, 对象指针,
	m_CurrentFunction = func;
	m_pModule		  = module;
	m_BinaryCode	  = m_CurrentFunction->m_BinaryCode;

	// 检查是否有足够大上堆空间,如果没有分配一个新的
	while( m_StackPointer - (func->m_StackNeeded + RESERVE_STACK) < m_StackBlocks[m_StackIndex] )
	{
		psDWORD *oldStackPointer = m_StackPointer;

		// 每个堆的大小由以下的公式来计算:
		// size = stackBlockSize << index

		// 确认没有发生堆栈溢出
		if( m_pEngine->GetMaxContextStackSize() > 0 )
		{
			// 如果发生了堆栈溢出, 设置异常后返回
			if( m_StackBlockSize * ((1 << (m_StackIndex+1)) - 1) > m_pEngine->GetMaxContextStackSize() )
			{
				m_IsStackMemoryNotAllocated = true;

				// 重新设置堆栈基址指针
				m_StackFramePointer = m_StackPointer;

				SetException(TXT_STACK_OVERFLOW);
				return;
			}
		}
		
		// 更新分配堆的索引
		m_StackIndex++;
		if( int(m_StackBlocks.size()) == m_StackIndex )
		{
			psDWORD *stack = new psDWORD[m_StackBlockSize << m_StackIndex];
			m_StackBlocks.push_back(stack);
		}

		int ArgumentSize = func->GetArgumentsSpaceOnStack();

		// 重新设置堆栈指针
		m_StackPointer = m_StackBlocks[m_StackIndex] + (m_StackBlockSize << m_StackIndex) - ArgumentSize;

		// 拷贝参数到新的堆栈中
		memcpy(m_StackPointer, oldStackPointer, sizeof(psDWORD) * ArgumentSize );
	}

	// 更新堆栈基址指针
	m_StackFramePointer   = m_StackPointer;	
}

int psCContext::GetCurrentLineNumber()
{
	if (m_CurrentFunction == NULL) return -1;

	if( m_Status == tsSuspended || m_Status == tsActive )
	{
		return m_CurrentFunction->GetLineNumber(int(m_BinaryCode - &(m_CurrentFunction->m_BinaryCode[0])));
	}

	return -1;
}

int psCContext::GetCurrentBytePos()
{
	if (m_CurrentFunction == NULL) return -1;

	if( m_Status == tsSuspended || m_Status == tsActive )
	{
		return int(m_BinaryCode - &(m_CurrentFunction->m_BinaryCode[0]));
	}

	return -1;
}

int psCContext::GetExceptionFunctionId()
{
	if( GetState() != psEXECUTION_EXCEPTION ) return psERROR;

	return m_ExceptionFunctionId;
}

int psCContext::GetExceptionModuleId()
{
	if( GetState() != psEXECUTION_EXCEPTION ) return psERROR;

	return m_ExceptionModuleId;
}

int psCContext::GetExceptionLineNumber()
{
	if( GetState() != psEXECUTION_EXCEPTION ) return psERROR;

	return m_ExceptionLine;
}

const psCHAR* psCContext::GetExceptionString()
{
	if( GetState() != psEXECUTION_EXCEPTION ) return NULL;

	return m_ExceptionString.AddressOf();
}

int psCContext::SetException(const psCHAR* desc)
{
	if( m_InExceptionHandler )
	{
		assert(false); // Shouldn't happen
		return -1; // but if it does, at least this will not crash the application
	}

	m_ExceptionString     = desc;
	m_ExceptionModuleId	  = m_pModule->GetID();
	m_ExceptionFunctionId = m_CurrentFunction->m_ID;
	m_ExceptionLine		  = m_CurrentFunction->GetLineNumber(int(m_BinaryCode - &(m_CurrentFunction->m_BinaryCode[0])) );
	m_Status			  = tsUnhandledException;

	// 清空堆栈
	CleanStack();

	return 0;
}

const psIDataType* psCContext::FindDebugSymbol(const psCHAR* name, psDWORD* addr) const
{
	if (addr) *addr = 0;
	if (name == 0) return 0;

	if (m_CurrentFunction)
	{
		if (ps_strcmp(name, PS_T("this")) == 0)
		{
			// 如果为this指针
			if (m_CurrentFunction->m_ObjectType == NULL)
				return NULL;
			else 
			{
				if (addr)
					*addr = m_StackFramePointer[0];
				return &(m_CurrentFunction->m_ObjectType->m_DataType);
			}
		}
		// 首先从局部变量中寻找
		for (int i = 0; i < m_CurrentFunction->m_NumDebugSymbols; ++i)
		{
			psSDebugSymbol* symbol = &m_CurrentFunction->m_DebugSymbols[i];
			if (symbol->name == name)
			{
				if (symbol->offset >= 0)
				{
					if (addr)
						*addr = psDWORD( (psBYTE*)m_StackFramePointer + symbol->offset );
					return &symbol->type;
				}
				else 
				{
					int curPos = int(m_BinaryCode - m_CurrentFunction->m_BinaryCode);
					if (curPos > symbol->start && (symbol->end < 0 || curPos <= symbol->end))
					{
						if (addr)
							*addr = psDWORD( (psBYTE*)m_StackFramePointer + symbol->offset );
						return &symbol->type;
					}
				}
			}
		}
		
		// 如果是类的成员函数,再从类的属性中寻找
		psCObjectType* objType = m_CurrentFunction->m_ObjectType;
		if (objType && objType->m_bScriptObject)
		{
			psSProperty* prop = objType->FindProperty(name);
			if (prop)
			{
				if (addr)
				{
					psDWORD thisPointer = m_StackFramePointer[0];
	
					*addr = (psDWORD)( (psBYTE*)thisPointer + prop->byteOffset );
				}
				return &prop->type;
			}
		}
	}

	// 最后从全局变量中寻找
	if (m_pModule)
	{
		for (size_t i = 0; i < m_pModule->GetGlobalVarCount(); ++i)
		{
			psSProperty* prop = m_pModule->GlobalVar(i);
			if ( prop->name == name )
			{
				if (addr)
					*addr = psDWORD(m_pModule->GetGlobalMemPtr() + prop->byteOffset);
				return &prop->type;
			}
		}
	}
	return 0;
}


psAutoStringPtr psCContext::GetCallStackString() const
{
	psCString callStackString(PS_T(""));

	if (m_CurrentFunction != 0)
	{	
		callStackString = m_CurrentFunction->GetDeclaration();
		callStackString += PS_T("\n");

		psCString str;
		for (int i = (int)m_CallStack.size() - 1; i >= 0; --i)
		{
			const psCScriptFunction* func =  m_CallStack[i].currentFunction;
			str = func->GetDeclaration();
			callStackString += str;
			callStackString += PS_T("\n");
		}	
	}
	psBuffer* buf = new psBuffer;
	buf->Assign( (psBYTE*)callStackString.RemoveBuffer(), true );
	psAutoStringPtr str(buf);
	return str;
}

psIModule* psCContext::GetCurrentModule() 
{
	return m_pModule;
}


