#ifndef __PS_CONFIG_H__
#define __PS_CONFIG_H__

// 
// Compiler differences
//-----------------------------------------

// vsnprintf() 
// Some compilers use different names for this function. If your compiler
// doesn't use the name vsnprintf() then you need to write a macro to translate 
// the function into its real name.

// ASM_AT_N_T or ASM_INTEL
// You should choose what inline assembly syntax to use when compiling

// __int64
// If your compiler doesn't support the __int64 type you'll need to define
// a substitute for it that is 64 bits large.

// VALUE_OF_BOOLEAN_TRUE
// This flag allows to customize the exact value of boolean true


//
// How to identify different compilers
//-----------------------------------------

// MS Visual C++
//  _MSC_VER   is defined
//  __MWERKS__ is not defined

// GNU C based compilers
//  __GNUC__   is defined

//
// CPU differences
//---------------------------------------
//
// Calling conventions
//-----------------------------------------

// GNU_STYLE_VIRTUAL_METHOD
// This constant should be defined if method pointers store index for virtual 
// functions in the same location as the function pointer. In such cases the method 
// is identified as virtual if the least significant bit is set.

// MULTI_BASE_OFFSET(x)
// This macro is used to retrieve the offset added to the object pointer in order to
// implicitly cast the object to the base object. x is the method pointer received by
// the register function.

// HAVE_VIRTUAL_BASE_OFFSET
// Define this constant if the compiler stores the virtual base offset in the method
// pointers. If it is not stored in the pointers then AngelScript have no way of
// identifying a method as coming from a class with virtual inheritance.

// VIRTUAL_BASE_OFFSET(x)
// This macro is used to retrieve the offset added to the object pointer in order to
// find the virtual base object. x is the method pointer received by the register 
// function;

// COMPLEX_MASK
// This constant shows what attributes determines if an object is returned in memory 
// or in the registers as normal structures

// THISCALL_RETURN_SIMPLE_IN_MEMORY
// CDECL_RETURN_SIMPLE_IN_MEMORY
// STDCALL_RETURN_SIMPLE_IN_MEMORY
// When these constants are defined then the corresponding calling convention always 
// return classes/structs in memory regardless of size or complexity.

// CALLEE_POPS_HIDDEN_RETURN_POINTER
// This constant should be defined if the callee pops the hidden return pointer, 
// used when returning an object in memory.

// THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
// With this constant defined AngelScript will pass the object pointer on the stack

// THISCALL_CALLEE_POPS_ARGUMENTS
// If the callee pops arguments for class methods then define this constant

// COMPLEX_OBJS_PASSED_BY_REF
// Some compilers always pass certain objects by reference. GNUC for example does 
// this if the the class has a defined destructor.

// 
// Configurations 
//------------------------------------------------

#define VALUE_OF_BOOLEAN_TRUE  1
#define MAX_ARRAY_DIM		   4

//THREADS功能应该已经没用了
//#ifdef WIN32 
//	#define USE_THREADS
//#endif

#define THISCALL_CALLEE_POPS_HIDDEN_RETURN_POINTER

// Microsoft Visual C++
#if defined(_MSC_VER) && !defined(__MWERKS__)
	#define MULTI_BASE_OFFSET(x) (*((psDWORD*)(&x)+1))
	#define HAVE_VIRTUAL_BASE_OFFSET
	#define VIRTUAL_BASE_OFFSET(x) (*((psDWORD*)(&x)+3))
	#define THIS_STDCALL_RETURN_SIMPLE_IN_MEMORY
	#define THISCALL_PASS_OBJECT_POINTER_IN_ECX
	#define THISCALL_CALLEE_POPS_ARGUMENTS
	#define COMPLEX_MASK (psOBJ_CLASS_CONSTRUCTOR | psOBJ_CLASS_DESTRUCTOR | psOBJ_CLASS_ASSIGNMENT)
	#define ASM_INTEL
	#define LANGUAGE_CHINESE
#elif defined(__GNUC__)
// GNU C
	#define GNU_STYLE_VIRTUAL_METHOD
	#define MULTI_BASE_OFFSET(x) (*((psDWORD*)(&x)+1))
	#define CALLEE_POPS_HIDDEN_RETURN_POINTER
	#define COMPLEX_OBJS_PASSED_BY_REF
	#define __int64 long long
	#define ASM_AT_N_T
	#define COMPLEX_MASK (psOBJ_CLASS_DESTRUCTOR)
	//#ifdef __linux__
		#define THISCALL_RETURN_SIMPLE_IN_MEMORY
		#define CDECL_RETURN_SIMPLE_IN_MEMORY
		#define STDCALL_RETURN_SIMPLE_IN_MEMORY
	//#endif
#endif



//----------------------------------------------------------------------//
// 操作数相关的宏定义
//----------------------------------------------------------------------//
#define BC_OFFSET1(bc) (*((int*)bc + 1))
#define BC_OFFSET2(bc) (*((int*)bc + 2))
#define BC_OFFSET3(bc) (*((int*)bc + 3))

#define BC_BASE1(bc)   (bc[1])	
#define BC_BASE2(bc)   (bc[2])
#define BC_BASE3(bc)   (bc[3])

//----------------------------------------------------------------------
// 头文件
#include <malloc.h>
#include <assert.h>
#include <vector>
#include <map>
#include <stack>
#include <math.h>

#include "../include/pixelscript.h"

//----------------------------------------------------------------------//
// 类的预先声明
//----------------------------------------------------------------------//
class  psCScriptEngine;
class  psCTokenizer;
class  psCParser;
class  psCCompiler;
class  psCBuilder;
class  psCModule;
class  psCByteCode;
class  psCScriptNode;
class  psCScriptCode;
class  psCScriptFunction;
class  psCContext;
class  psCVariableScope;
class  psCDataType;
class  psCObjectType;
class  psSTypeBehaviour;
struct psSGlobalVariableDesc;

//----------------------------------------------------------------------//
// 实用函数
//----------------------------------------------------------------------//
template<class T>
inline T* MAlloc(size_t n)
{
	T* ptr = static_cast<T*>( ::malloc(sizeof(T)*n) );
	assert( ptr );
	return ptr;
}

//----------------------------------------------------------------------//
// 实用类
//----------------------------------------------------------------------//
class psBuffer: public psIBuffer
{
public:
	psBYTE* GetBufferPointer() 
	{ 
		return m_pBuf; 
	}
	void Release()  
	{
		if ( --m_RefCount == 0 )
		{
			delete this;
		}
	}
	void AddRef()
	{
		++m_RefCount;
	}
	psBuffer() : m_pBuf ( NULL )
			   , m_bNeedDelete( false )
			   , m_RefCount(1)
	{
	}
	~psBuffer()
	{	
		if ( m_bNeedDelete && m_pBuf )
		{
			::free( m_pBuf );
		}
	}
	void Alloc( size_t s )
	{
		m_pBuf = (psBYTE*)::malloc( s );
		m_bNeedDelete = true;
	}
	void Assign( psBYTE* buf , bool needDelete )
	{
		m_pBuf = buf;
		m_bNeedDelete = needDelete;
	}
private:
	psBYTE* m_pBuf;
	bool	m_bNeedDelete;
	psDWORD m_RefCount;
};

#endif // __PS_CONFIG_H__

