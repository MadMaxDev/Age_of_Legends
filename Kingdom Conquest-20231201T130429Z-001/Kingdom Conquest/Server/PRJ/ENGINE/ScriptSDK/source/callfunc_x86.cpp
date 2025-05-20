#include "config.h"
#include "callfunc.h"
#include "scriptengine.h"
#include "texts.h"
#include "tokendef.h"
#include <assert.h>
#include <malloc.h>

#if defined(_MSC_VER)
	#pragma warning (disable : 4312)
	#pragma warning (disable : 4311)
#endif

namespace pse 
{
	int DetectCallingConvention(bool bObject, psUPtr &ptr, psDWORD callConv, psSSystemFunctionInterface *internal)
	{
		memset(internal, 0, sizeof(psSSystemFunctionInterface));

		internal->func = (void*)ptr.func;

		psDWORD base = callConv;
		if( !bObject )
		{
			if( base == psCALL_CDECL )
				internal->callConv = ICC_CDECL;
			else if( base == psCALL_STDCALL )
				internal->callConv = ICC_STDCALL;
			else
				return psNOT_SUPPORTED;
		}
		else
		{
			if( base == psCALL_THISCALL || 
				base == psCALL_STDCALL  )
			{
				
				if (base == psCALL_THISCALL)
					internal->callConv = ICC_THISCALL;
				else
					internal->callConv = ICC_THIS_STDCALL;
#ifdef GNU_STYLE_VIRTUAL_METHOD
				if ( (psDWORD)(ptr.func) & 1 )
				{
					if ( base == psCALL_THISCALL )
						internal->callConv = ICC_VIRTUAL_THISCALL;
				}
#endif
				internal->baseOffset = MULTI_BASE_OFFSET(ptr);

#ifdef HAVE_VIRTUAL_BASE_OFFSET
				// We don't support virtual inheritance
				if( VIRTUAL_BASE_OFFSET(ptr) != 0 )
					return psNOT_SUPPORTED;
#endif
			}
			else if( base == psCALL_CDECL_OBJFIRST ||
					 base == psCALL_CDECL )
			{
				internal->callConv = ICC_CDECL_OBJFIRST;
			}
			else
			{
				return psNOT_SUPPORTED;
			}
		}

		return 0;
	}

	// This function should prepare system functions so that it will be faster to call them
	int PrepareSystemFunction(psCScriptFunction *func, psSSystemFunctionInterface *internal, psCScriptEngine *engine)
	{
		// References are always returned as primitive data
		if( func->m_ReturnType.IsReference() )
		{
			internal->hostReturnInMemory = false;
			internal->hostReturnType	 = HOST_RETURN_DWORD;
		}
		// Registered types have special flags that determine how they are returned
		else if( func->m_ReturnType.IsObject() )
		{
			psDWORD objType = func->m_ReturnType.ObjectType()->m_Flags;
			if( objType & psOBJ_CLASS )
			{
				if( objType & COMPLEX_MASK )
				{
					internal->hostReturnInMemory = true;
					internal->hostReturnType     = HOST_RETURN_DWORD;
				}
				else
				{
					int s = func->m_ReturnType.GetSizeInMemoryBytes();
					if( s > 8 )
					{
						internal->hostReturnInMemory = true;
						internal->hostReturnType	 = HOST_RETURN_DWORD;
					}
					else
					{
						internal->hostReturnInMemory = false;
						if ( s == 1 )
							internal->hostReturnType = HOST_RETURN_BYTE;
						else if ( s == 2 )
							internal->hostReturnType = HOST_RETURN_WORD;
						else if ( s <= 4)
							internal->hostReturnType = HOST_RETURN_DWORD;
						else
							internal->hostReturnType = HOST_RETURN_QWORD;
					}
					if ( (objType & psOBJ_PRIMITIVE) == 0 )
					{
#ifdef THIS_STDCALL_RETURN_SIMPLE_IN_MEMORY
						if( internal->callConv == ICC_THIS_STDCALL  )
						{
							internal->hostReturnInMemory = true;
							internal->hostReturnType = HOST_RETURN_DWORD;
						}
#endif // THIS_STDCALL_RETURN_SIMPLE_IN_MEMORY

#ifdef THISCALL_RETURN_SIMPLE_IN_MEMORY
						if( internal->callConv == ICC_THISCALL ||
							internal->callConv == ICC_VIRTUAL_THISCALL )
						{
							internal->hostReturnInMemory = true;
							internal->hostReturnType = HOST_RETURN_DWORD;
						}
#endif // THISCALL_RETURN_SIMPLE_IN_MEMORY

#ifdef CDECL_RETURN_SIMPLE_IN_MEMORY
						if( internal->callConv == ICC_CDECL || 
							internal->callConv == ICC_CDECL_OBJFIRST )
						{
							internal->hostReturnInMemory = true;
							internal->hostReturnType = HOST_RETURN_DWORD;
						}
#endif // CDECL_RETURN_SIMPLE_IN_MEMORY

#ifdef STDCALL_RETURN_SIMPLE_IN_MEMORY
						if( internal->callConv == ICC_STDCALL )
						{
							internal->hostReturnInMemory = true;
							internal->hostReturnType = HOST_RETURN_DWORD;
						}
#endif // STDCALL_RETURN_SIMPLE_IN_MEMORY
					}
				}
			}else
			{
				int s = func->m_ReturnType.GetSizeInMemoryBytes();
				if( s > 8 )
				{
					internal->hostReturnInMemory = true;
					internal->hostReturnType	 = HOST_RETURN_DWORD;
				}
				else
				{
					internal->hostReturnInMemory = false;
					if ( s == 1 )
						internal->hostReturnType = HOST_RETURN_BYTE;
					else if ( s == 2 )
						internal->hostReturnType = HOST_RETURN_WORD;
					else if ( s <= 4)
						internal->hostReturnType = HOST_RETURN_DWORD;
					else
						internal->hostReturnType = HOST_RETURN_QWORD;
				}
			}
		}
		else 
		{
			// 基本类型
			int s = func->m_ReturnType.GetSizeInMemoryBytes();
			if( func->m_ReturnType.IsBooleanType() || s == 1 )
			{
				internal->hostReturnInMemory = false;
				internal->hostReturnType	 = HOST_RETURN_BYTE; 
			}else if( s == 2 )
			{
				internal->hostReturnInMemory = false;
				internal->hostReturnType	 = HOST_RETURN_WORD; 
			}else if ( s <= 4 )
			{
				internal->hostReturnInMemory = false;
				if ( func->m_ReturnType.IsFloatType() )
				{
					internal->hostReturnType = HOST_RETURN_FLOAT;
				}else
				{
					internal->hostReturnType = HOST_RETURN_DWORD;
				}
			}else if ( s <= 8 )
			{
				internal->hostReturnInMemory = false;
				if ( func->m_ReturnType.IsDoubleType() )
				{
					internal->hostReturnType = HOST_RETURN_DOUBLE;
				}else
				{
					internal->hostReturnType = HOST_RETURN_QWORD;
				}
			}else
			{
				// Shouldn't be possible to get here
				assert(false);

				internal->hostReturnInMemory = true;
				internal->hostReturnType	 = HOST_RETURN_DWORD; 
			}
		}
		// Calculate the size needed for the parameters
		internal->paramSize = func->GetSpaceNeededForArguments();

		// Verify if the function takes any objects by value
		internal->takesObjByVal = false;
		for( size_t n = 0; n < func->m_ParamTypes.size(); n++ )
		{
			psCDataType& dt = func->m_ParamTypes[n];
			if( dt.IsObject() && !dt.IsReference() )
			{
				internal->takesObjByVal = true;
				break;
			}
		}
		return 0;
	}


	typedef psQWORD (*t_CallCDeclQW)(const psDWORD *, int, size_t);
	typedef psQWORD (*t_CallCDeclQWObj)(void *obj, const psDWORD *, int, size_t);
	typedef psDWORD (*t_CallCDeclRetByRef)(const psDWORD *, int, size_t, void *);
	typedef psDWORD (*t_CallCDeclObjRetByRef)(void *obj, const psDWORD *, int, size_t, void *);
	typedef psQWORD (*t_CallSTDCallQW)(const psDWORD *, int, size_t);
	typedef psQWORD (*t_CallThisCallQW)(const void *, const psDWORD *, int, size_t);
	typedef psDWORD (*t_CallThisCallRetByRef)(const void *, const psDWORD *, int, size_t, void *);

	// Prototypes
	void CallCDeclFunction(const psDWORD *args, int paramSize, size_t func);
	void CallCDeclFunctionObjLast(const void *obj, const psDWORD *args, int paramSize, size_t func);
	void CallCDeclFunctionObjFirst(const void *obj, const psDWORD *args, int paramSize, size_t func);
	void CallCDeclFunctionRetByRef_impl(const psDWORD *args, int paramSize, size_t func, void *retPtr);
	void CallCDeclFunctionRetByRefObjLast_impl(const void *obj, const psDWORD *args, int paramSize, size_t func, void *retPtr);
	void CallCDeclFunctionRetByRefObjFirst_impl(const void *obj, const psDWORD *args, int paramSize, size_t func, void *retPtr);
	void CallSTDCallFunction(const psDWORD *args, int paramSize, size_t func);
	void CallThisCallFunction(const void *obj, const psDWORD *args, int paramSize, size_t func);
	void CallThisCallFunctionRetByRef_impl(const void *, const psDWORD *, int, size_t, void *retPtr);
	void CallThisStdCallFunction(const void *obj, const psDWORD *args, int paramSize, size_t func);
	void CallThisStdCallFunctionRetByRef_impl(const void *, const psDWORD *, int, size_t, void *retPtr); 

	// Initialize function pointers
	const t_CallCDeclQW CallCDeclFunctionQWord = (t_CallCDeclQW)CallCDeclFunction;
	const t_CallCDeclQWObj CallCDeclFunctionQWordObjLast = (t_CallCDeclQWObj)CallCDeclFunctionObjLast;
	const t_CallCDeclQWObj CallCDeclFunctionQWordObjFirst = (t_CallCDeclQWObj)CallCDeclFunctionObjFirst;
	const t_CallCDeclRetByRef CallCDeclFunctionRetByRef = (t_CallCDeclRetByRef)CallCDeclFunctionRetByRef_impl;
	const t_CallCDeclObjRetByRef CallCDeclFunctionRetByRefObjLast = (t_CallCDeclObjRetByRef)CallCDeclFunctionRetByRefObjLast_impl;
	const t_CallCDeclObjRetByRef CallCDeclFunctionRetByRefObjFirst = (t_CallCDeclObjRetByRef)CallCDeclFunctionRetByRefObjFirst_impl;
	const t_CallSTDCallQW CallSTDCallFunctionQWord = (t_CallSTDCallQW)CallSTDCallFunction;
	const t_CallThisCallQW CallThisCallFunctionQWord = (t_CallThisCallQW)CallThisCallFunction;
	const t_CallThisCallRetByRef CallThisCallFunctionRetByRef = (t_CallThisCallRetByRef)CallThisCallFunctionRetByRef_impl;
	const t_CallThisCallQW CallThisStdCallFunctionQWord = (t_CallThisCallQW)CallThisStdCallFunction;
	const t_CallThisCallRetByRef CallThisStdCallFunctionRetByRef = (t_CallThisCallRetByRef)CallThisStdCallFunctionRetByRef_impl;

	psDWORD GetReturnedFloat();
	psQWORD GetReturnedDouble();
	psDWORD GetReturnedFloat();
	psQWORD GetReturnedDouble();

	int CallSystemFunction(int id, int paramSize, psCContext *context)
	{
		// 得到系统函数的索引
		int index = -id - 1;

		psCScriptEngine *engine				 = (psCScriptEngine*)context->GetEngine();
		psSSystemFunctionInterface *sysFunc  = engine->GetSystemFunctionInterface(index);
		psCScriptFunction *desc			     = engine->GetSystemFunctionByIndex(index);

		// 检查系统函数是否有效
		if (desc == NULL || sysFunc == NULL)
		{
			assert(false);
			context->SetException(TXT_SYSTEM_FUNCTION_NOTFIND);
			return 0;
		}

		psQWORD retQW = 0;
		int     callConv           = sysFunc->callConv;
		void   *func               = (void*)sysFunc->func;
		psDWORD *args              = (psDWORD*)context->GetStackPointer();
		int     popSize            = paramSize;
		void    *retPointer		   = NULL;
		void    *obj			   = NULL;
		psDWORD *vftable		   = NULL;

		if( callConv >= ICC_THISCALL )
		{
			// 对象指针必需从堆栈中弹出
			++popSize;

			// 检查是否为空指针访问
			obj = (void*)args[0];
			if( obj == NULL )
			{	
				context->SetException(TXT_NULL_POINTER_ACCESS);
				return 0;
			}

			// 如果是多继承,加上其基类偏移
			obj = (void*)((char*)(obj) + sysFunc->baseOffset);
			++args;
		}

		if( desc->m_ReturnType.IsObject() && !desc->m_ReturnType.IsReference() )
		{
			// 如果返回值不是引用,得到返回类对象的指针
			retPointer = (void*)args[0];

			assert(paramSize > 0);
			// 参数大小应该减一
			paramSize--;
			++args;

			if( sysFunc->hostReturnInMemory )
			{
				// 如果类对象返回值是在堆上, 需要改变其调用方式
				// NOTE: 如果一个C++类为构造函数,析构函数,赋值函数,或者其大小大于8个字节,
				// 那么这个类对象返回值是在堆上做的, 否则其返回值是通过EAX:EDX来得到的
				callConv++;
			}
		}

#ifdef COMPLEX_OBJS_PASSED_BY_REF
		if ( sysFunc->takesObjByVal )
		{
			psDWORD* param_buf = (psDWORD*)alloca(sizeof(psDWORD)*(paramSize+1));
			int spos = 0;
			int dpos = 1;
			for( size_t n = 0; n < desc->m_ParamTypes.size(); n++ )
			{
				psCDataType& dt = desc->m_ParamTypes[n];
				if (   (dt.IsObject() && !dt.IsReference())
					&& (dt.ObjectType()->m_Flags & COMPLEX_MASK))
				{ 
					int s = dt.GetSizeOnStackDWords();
					param_buf[dpos++] = (psDWORD)( args + spos );
					spos += s;
				}else
				{
					int s = dt.GetSizeOnStackDWords();
					memcpy(param_buf+dpos, args+spos, s*sizeof(psDWORD));
					dpos += s;
					spos += s;
				}
			}
			while ( spos < paramSize )
			{
				// 可能是变参函数调用, 拷贝其它的参数
				param_buf[dpos++] = args[spos++];
			}	
			args	  = &param_buf[1];
			paramSize = dpos-1;
		}
#endif
		switch( callConv )
		{
		case ICC_CDECL:
			retQW = CallCDeclFunctionQWord(args, paramSize<<2, (size_t)func);
			break;

		case ICC_CDECL_RETURNINMEM:
			retQW = CallCDeclFunctionRetByRef(args, paramSize<<2, (size_t)func, retPointer);
			break;

		case ICC_STDCALL:
			retQW = CallSTDCallFunctionQWord(args, paramSize<<2, (size_t)func);
			break;

		case ICC_STDCALL_RETURNINMEM:
			// Push the return pointer on the stack
			paramSize++;
			args--;
			*(size_t*)args = (size_t)retPointer;

			retQW = CallSTDCallFunctionQWord(args, paramSize<<2, (size_t)func);
			break;

		case ICC_THISCALL:
			retQW = CallThisCallFunctionQWord(obj, args, paramSize<<2, (size_t)func);
			break;

		case ICC_THISCALL_RETURNINMEM:
			retQW = CallThisCallFunctionRetByRef(obj, args, paramSize<<2, (size_t)func, retPointer);
			break;

		case ICC_THIS_STDCALL:
			retQW = CallThisStdCallFunctionQWord(obj, args, paramSize<<2, (size_t)func);
			break;

		case ICC_THIS_STDCALL_RETURNINEME:
			retQW = CallThisStdCallFunctionRetByRef(obj, args, paramSize<<2, (size_t)func, retPointer);
			break;

		case ICC_VIRTUAL_THISCALL:
			// Get virtual function table from the object pointer
			vftable = *(psDWORD**)obj;

			retQW = CallThisCallFunctionQWord(obj, args, paramSize<<2, vftable[size_t(func)>>2]);
			break;

		case ICC_VIRTUAL_THISCALL_RETURNINMEM:
			// Get virtual function table from the object pointer
			vftable = *(psDWORD**)obj;

			retQW = CallThisCallFunctionRetByRef(obj, args, paramSize<<2, vftable[size_t(func)>>2], retPointer);
			break;

		case ICC_CDECL_OBJFIRST:
			// Call the system object method as a cdecl with the obj ref as the first parameter
			retQW = CallCDeclFunctionQWordObjFirst(obj, args, paramSize<<2, (size_t)func);
			break;

		case ICC_CDECL_OBJFIRST_RETURNINMEM:
			// Call the system object method as a cdecl with the obj ref as the first parameter
			retQW = CallCDeclFunctionRetByRefObjFirst(obj, args, paramSize<<2, (size_t)func, retPointer);
			break;

		default:
			context->SetException(TXT_INVALID_CALLING_CONVENTION);
		}

		// 保存返回值
		if( desc->m_ReturnType.IsObject() && !desc->m_ReturnType.IsReference() )
		{
			if( !sysFunc->hostReturnInMemory )
			{
				// Copy the returned value to the pointer sent by the script engine
				if( sysFunc->hostReturnType == HOST_RETURN_QWORD )
					*(psQWORD*)retPointer = retQW;
				else		
					*(psDWORD*)retPointer = (psDWORD)retQW;
			}

			// 设置返回值
			context->SetReturnValue( (psDWORD)retPointer );
		}
		else
		{
			// 设置返回值
			switch ( sysFunc->hostReturnType )
			{
			case HOST_RETURN_BYTE:   context->SetReturnValue( (psBYTE)retQW );				  break;
			case HOST_RETURN_WORD:   context->SetReturnValue( (psWORD)retQW );				  break;
			case HOST_RETURN_DWORD:  context->SetReturnValue( (psDWORD)retQW );				  break;
			case HOST_RETURN_QWORD:  context->SetReturnValue( (psQWORD)retQW );				  break;
			case HOST_RETURN_FLOAT:  context->SetReturnValue( (psQWORD)GetReturnedFloat() );  break;
			case HOST_RETURN_DOUBLE: context->SetReturnValue( (psQWORD)GetReturnedDouble() ); break;
			default:																		  break;
			}
		}
		return popSize;
	}


// On GCC we need to prevent the compiler from inlining these assembler routines when
// optimizing for speed (-O3), as the loop labels get duplicated which cause compile errors.

#ifdef __GNUC__
    #define NOINLINE __attribute ((__noinline__))
#else
    #define NOINLINE
#endif

#define UNUSED_VAR(x) (x)=(x)

void NOINLINE CallCDeclFunction(const psDWORD *args, int paramSize, size_t func)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize

		// Restore registers
		pop  ecx

		// return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  12(%ebp), %eax \n" // paramSize
		"addl  $4, %eax       \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"movl  12(%ebp), %ecx \n" // paramSize
		"movl  8(%ebp), %eax  \n" // args
		"addl  %ecx, %eax     \n" // push arguments on the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy        \n"
		"copyloop:            \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop       \n"
		"endcopy:             \n"
		"call  *16(%ebp)      \n"
		"addl  12(%ebp), %esp \n" // pop arguments

		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}

void NOINLINE CallCDeclFunctionObjLast(const void *obj, const psDWORD *args, int paramSize, size_t func)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		// Push the object pointer as the last argument to the function
		push obj

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize
		add  esp, 4

		// Restore registers
		pop  ecx

		// return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(obj);
    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  16(%ebp), %eax \n" // paramSize
		"addl  $8, %eax       \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"pushl 8(%ebp)        \n"
		"movl  16(%ebp), %ecx \n" // paramSize
		"movl  12(%ebp), %eax \n" // args
		"addl  %ecx, %eax     \n" // push arguments on the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy8       \n"
		"copyloop8:           \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop8      \n"
		"endcopy8:            \n"
		"call  *20(%ebp)      \n"
		"addl  16(%ebp), %esp \n" // pop arguments
		"addl  $4, %esp       \n"

		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}

void NOINLINE CallCDeclFunctionObjFirst(const void *obj, const psDWORD *args, int paramSize, size_t func)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// push object as first parameter
        push obj

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize
        add  esp, 4

		// Restore registers
		pop  ecx

		// return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(obj);
    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  16(%ebp), %eax \n" // paramSize
		"addl  $8, %eax       \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"movl  16(%ebp), %ecx \n" // paramSize
		"movl  12(%ebp), %eax \n" // args
		"addl  %ecx, %eax     \n" // push arguments on the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy6       \n"
		"copyloop6:           \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop6      \n"
		"endcopy6:            \n"
		"pushl 8(%ebp)        \n" // push obj
		"call  *20(%ebp)      \n"
		"addl  16(%ebp), %esp \n" // pop arguments
        "addl  $4, %esp       \n"

		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}

void NOINLINE CallCDeclFunctionRetByRefObjFirst_impl(const void *obj, const psDWORD *args, int paramSize, size_t func, void *retPtr)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Push the object pointer
        push obj

		// Push the return pointer
		push retPtr;

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize

#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		// Pop the return pointer
		add  esp, 8
#else
        add  esp, 4
#endif
		// Restore registers
		pop  ecx

		// return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(obj);
    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);
    UNUSED_VAR(retPtr);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  16(%ebp), %eax \n" // paramSize
		"addl  $12, %eax      \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"movl  16(%ebp), %ecx \n" // paramSize
		"movl  12(%ebp), %eax \n" // args
		"addl  %ecx, %eax     \n" // push arguments on the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy5       \n"
		"copyloop5:           \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop5      \n"
		"endcopy5:            \n"
        "pushl 8(%ebp)        \n" // push object first
		"pushl 24(%ebp)       \n" // retPtr
		"call  *20(%ebp)      \n" // func
		"addl  16(%ebp), %esp \n" // pop arguments
#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		"addl  $8, %esp       \n" // Pop the return pointer and object pointer
#else
		"addl  $4, %esp       \n" // Pop the object pointer
#endif
		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}

void NOINLINE CallCDeclFunctionRetByRef_impl(const psDWORD *args, int paramSize, size_t func, void *retPtr)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Push the return pointer
		push retPtr;

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize

#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		// Pop the return pointer
		add  esp, 4
#endif
		// Restore registers
		pop  ecx

		// return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);
    UNUSED_VAR(retPtr);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  12(%ebp), %eax \n" // paramSize
		"addl  $8, %eax       \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"movl  12(%ebp), %ecx \n" // paramSize
		"movl  8(%ebp), %eax  \n" // args
		"addl  %ecx, %eax     \n" // push arguments on the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy7       \n"
		"copyloop7:           \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop7      \n"
		"endcopy7:            \n"
		"pushl 20(%ebp)       \n" // retPtr
		"call  *16(%ebp)      \n" // func
		"addl  12(%ebp), %esp \n" // pop arguments
#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		"addl  $4, %esp       \n" // Pop the return pointer
#endif
		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}

void NOINLINE CallCDeclFunctionRetByRefObjLast_impl(const void *obj, const psDWORD *args, int paramSize, size_t func, void *retPtr)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		push obj

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Push the return pointer
		push retPtr;

		// Call function
		call [func]

		// Pop arguments from stack
		add  esp, paramSize
		add  esp, 4

#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		// Pop the return pointer
		add  esp, 4
#endif
		// Restore registers
		pop  ecx

		// return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(obj);
    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);
    UNUSED_VAR(retPtr);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  16(%ebp), %eax \n" // paramSize
		"addl  $12, %eax      \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"pushl 8(%ebp)        \n"
		"movl  16(%ebp), %ecx \n" // paramSize
		"movl  12(%ebp), %eax \n" // args
		"addl  %ecx, %eax     \n" // push arguments on the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy4       \n"
		"copyloop4:           \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop4      \n"
		"endcopy4:            \n"
		"pushl 24(%ebp)       \n" // retPtr
		"call  *20(%ebp)      \n" // func
		"addl  16(%ebp), %esp \n" // pop arguments
#ifndef CALLEE_POPS_HIDDEN_RETURN_POINTER
		"addl  $8, %esp       \n" // Pop the return pointer
#else
		"addl  $4, %esp       \n" // Pop the return pointer
#endif
		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}

void NOINLINE CallSTDCallFunction(const psDWORD *args, int paramSize, size_t func)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

		// Call function
		call [func]

		// The callee already removed parameters from the stack

		// Restore registers
		pop  ecx

		// return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  12(%ebp), %eax \n" // paramSize
		"addl  $4, %eax       \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"movl  12(%ebp), %ecx \n" // paramSize
		"movl  8(%ebp), %eax  \n" // args
		"addl  %ecx, %eax     \n" // push arguments on the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy2       \n"
		"copyloop2:           \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop2      \n"
		"endcopy2:            \n"
		"call  *16(%ebp)      \n" // callee pops the arguments

		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}


void NOINLINE CallThisCallFunction(const void *obj, const psDWORD *args, int paramSize, size_t func)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		// Push the object pointer on the stack
		push obj
#else
		// Move object pointer to ECX
		mov  ecx, obj
#endif

		// Call function
		call [func]

#ifndef THISCALL_CALLEE_POPS_ARGUMENTS
		// Pop arguments
		add  esp, paramSize
#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		// Pop object pointer
		add esp, 4
#endif
#endif

		// Restore registers
		pop  ecx

		// Return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(obj);
    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  16(%ebp), %eax \n" // paramSize
		"addl  $8, %eax       \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"movl  16(%ebp), %ecx \n" // paramSize
		"movl  12(%ebp), %eax \n" // args
		"addl  %ecx, %eax     \n" // push all arguments on the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy1       \n"
		"copyloop1:           \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop1      \n"
		"endcopy1:            \n"
		"movl  8(%ebp), %ecx  \n" // move obj into ECX
		"pushl 8(%ebp)        \n" // push obj on the stack
		"call  *20(%ebp)      \n"
		"addl  16(%ebp), %esp \n" // pop arguments
		"addl  $4, %esp       \n" // pop obj

		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}

void NOINLINE CallThisCallFunctionRetByRef_impl(const void *obj, const psDWORD *args, int paramSize, size_t func, void *retPtr)
{
#if defined ASM_INTEL

	// Copy the data to the real stack. If we fail to do
	// this we may run into trouble in case of exceptions.
	__asm
	{
		// We must save registers that are used
	    push ecx

		// Clear the FPU stack, in case the called function doesn't do it by itself
		fninit

		// Copy arguments from script
		// stack to application stack
        mov  ecx, paramSize
		mov  eax, args
		add  eax, ecx
		cmp  ecx, 0
		je   endcopy
copyloop:
		sub  eax, 4
		push dword ptr [eax]
		sub  ecx, 4
		jne  copyloop
endcopy:

#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		// Push the object pointer on the stack
		push obj
#else
		// Move object pointer to ECX
		mov  ecx, obj
#endif

		// Push the return pointer
		push retPtr

		// Call function
		call [func]

#ifndef THISCALL_CALLEE_POPS_HIDDEN_RETURN_POINTER
		// Pop the return pointer
		add  esp, 4
#endif

#ifndef THISCALL_CALLEE_POPS_ARGUMENTS
		// Pop arguments
		add  esp, paramSize
#ifdef THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
		// Pop object pointer
		add esp, 4
#endif
#endif

		// Restore registers
		pop  ecx

		// Return value in EAX or EAX:EDX
	}

#elif defined ASM_AT_N_T

    UNUSED_VAR(obj);
    UNUSED_VAR(args);
    UNUSED_VAR(paramSize);
    UNUSED_VAR(func);
    UNUSED_VAR(retPtr);

	asm("pushl %ecx           \n"
	    "fninit               \n"

		// Need to align the stack pointer so that it is aligned to 16 bytes when making the function call.
		// It is assumed that when entering this function, the stack pointer is already aligned, so we need
		// to calculate how much we will put on the stack during this call.
		"movl  16(%ebp), %eax \n" // paramSize
		"addl  $12, %eax      \n" // counting esp that we will push on the stack
		"movl  %esp, %ecx     \n"
		"subl  %eax, %ecx     \n"
		"andl  $15, %ecx      \n"
		"movl  %esp, %eax     \n"
		"subl  %ecx, %esp     \n"
		"pushl %eax           \n" // Store the original stack pointer

		"movl  16(%ebp), %ecx \n" // paramSize
		"movl  12(%ebp), %eax \n" // args
		"addl  %ecx, %eax     \n" // push all arguments to the stack
		"cmp   $0, %ecx       \n"
		"je    endcopy3       \n"
		"copyloop3:           \n"
		"subl  $4, %eax       \n"
		"pushl (%eax)         \n"
		"subl  $4, %ecx       \n"
		"jne   copyloop3      \n"
		"endcopy3:            \n"
		"movl  8(%ebp), %ecx  \n" // move obj into ECX
		"pushl 8(%ebp)        \n" // push obj on the stack
		"pushl 24(%ebp)       \n" // push retPtr on the stack
		"call  *20(%ebp)      \n"
#ifndef THISCALL_CALLEE_POPS_HIDDEN_RETURN_POINTER
		"addl  $4, %esp       \n" // pop return pointer
#endif
		"addl  16(%ebp), %esp \n" // pop arguments
		"addl  $4, %esp       \n" // pop the object pointer
		                          // the return pointer was popped by the callee
		// Pop the alignment bytes
		"popl  %esp           \n"

		"popl  %ecx           \n");

#endif
}

	void CallThisStdCallFunction(const void *obj, const psDWORD *args, int paramSize, size_t func)
	{
#if defined ASM_INTEL

		// Copy the data to the real stack. If we fail to do
		// this we may run into trouble in case of exceptions.
		__asm
		{
			// We must save registers that are used
			push ecx			

				// Copy arguments from script  
				// stack to application stack
				mov  ecx, paramSize
				mov  eax, args
				add  eax, ecx
				cmp  ecx, 0
				je   endcopy
copyloop:
			sub  eax, 4
				push dword ptr [eax]
				sub  ecx, 4
					jne  copyloop
endcopy:

				// Push the object pointer on the stack
				push obj

				// Call function
				call [func]         

				// Pop arguments
				add  esp, paramSize

				// Restore registers
				pop  ecx

				// Return value in EAX or EAX:EDX
		}

#elif defined ASM_AT_N_T

		asm("pushl %ecx           \n"
			"movl  16(%ebp), %ecx \n" // paramSize
			"movl  12(%ebp), %eax \n" // args
			"addl  %ecx, %eax     \n" // push all arguments on the stack
			"cmp   $0, %ecx       \n"
			"je    endcopy11       \n"
			"copyloop11:           \n"
			"subl  $4, %eax       \n"
			"pushl (%eax)         \n"
			"subl  $4, %ecx       \n"
			"jne   copyloop11      \n"
			"endcopy11:            \n"
			"movl  8(%ebp), %ecx  \n" // move obj into ECX
			"pushl 8(%ebp)        \n" // push obj on the stack
			"call  *20(%ebp)      \n"
			"addl  16(%ebp), %esp \n" // pop arguments
			"addl  $4, %esp       \n" // pop obj
			"popl  %ecx           \n");

#endif
	}

	void CallThisStdCallFunctionRetByRef_impl(const void *obj, const psDWORD *args, int paramSize, size_t func, void *retPtr)
	{
#if defined ASM_INTEL
		// Copy the data to the real stack. If we fail to do
		// this we may run into trouble in case of exceptions.
		__asm
		{
			// We must save registers that are used
			push ecx			

				// Copy arguments from script  
				// stack to application stack
				mov  ecx, paramSize
				mov  eax, args
				add  eax, ecx
				cmp  ecx, 0
				je   endcopy
copyloop:
			sub  eax, 4
				push dword ptr [eax]
				sub  ecx, 4
					jne  copyloop
endcopy:
				// Push the return pointer
				push retPtr
					
				// push object pointer
				push obj

				// Call function
				call [func]         

				// Pop arguments
				add  esp, paramSize

				// Restore registers
				pop  ecx

				// Return value in EAX or EAX:EDX
		}


#elif defined ASM_AT_N_T

asm("pushl %ecx           \n"
	"movl  16(%ebp), %ecx \n" // paramSize
	"movl  12(%ebp), %eax \n" // args
	"addl  %ecx, %eax     \n" // push all arguments to the stack
	"cmp   $0, %ecx       \n"
	"je    endcopy13       \n"
	"copyloop13:           \n"
	"subl  $4, %eax       \n"
	"pushl (%eax)         \n"
	"subl  $4, %ecx       \n"
	"jne   copyloop13      \n"
	"endcopy13:            \n"
	"movl  8(%ebp), %ecx  \n" // move obj into ECX
	"pushl 8(%ebp)        \n" // push obj on the stack
	"pushl 24(%ebp)       \n" // push retPtr on the stack
	"call  *20(%ebp)      \n"
	"addl  16(%ebp), %esp \n" // pop arguments
	"addl  $4, %esp       \n" // pop the object pointer
	// the return pointer was popped by the callee
	"popl  %ecx           \n");

#endif

}

psDWORD GetReturnedFloat()
{
    psDWORD f;

#if defined ASM_INTEL

	// Get the float value from ST0
    __asm fstp dword ptr [f]

#elif defined ASM_AT_N_T

	asm("fstps %0 \n" : "=m" (f));

#endif

	return f;
}

psQWORD GetReturnedDouble()
{
    psQWORD d;

#if defined ASM_INTEL

	// Get the double value from ST0
    __asm fstp qword ptr [d]

#elif defined ASM_AT_N_T

	asm("fstpl %0 \n" : "=m" (d));

#endif

	return d;
}

} // End namespace