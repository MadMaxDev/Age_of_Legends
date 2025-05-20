// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: atomic.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:

#include "../inc/atomic.h"

//////////////////////////////////////////////////////////////////////////
// compare and swap
bool dia_cas(volatile void *ptr, 
			 dia_uint32_t old1, 
			 dia_uint32_t new1)
{
#ifdef __GNUC__
	bool ret;
	__asm__ __volatile__(
		"lock cmpxchgl %1,%2; sete %%al"
		: "=a"(ret)
		: "r"(new1), "m"(*(dia_uint32_t*)ptr), "0"(old1)
		: "memory");
	return ret;
#else
	return (InterlockedCompareExchange((LONG*)ptr, new1, old1) == old1);
#endif
}


// from jiangli
#ifdef WIN32
__declspec(naked) long long __stdcall PG_InterlockedCompareExchange64(
	long long volatile * Destination, long long Exchange, long long Comperand)
{
	Destination;Exchange;Comperand;
	__asm
	{
		push        ebx  
		push        ebp  
		mov         ebp,dword ptr [esp+0Ch] 
		mov         ebx,dword ptr [esp+10h] 
		mov         ecx,dword ptr [esp+14h] 
		mov         eax,dword ptr [esp+18h] 
		mov         edx,dword ptr [esp+1Ch] 
		lock cmpxchg8b qword ptr [ebp] 
		pop         ebp  
		pop         ebx  
		ret         14h
	}
}
#endif


//////////////////////////////////////////////////////////////////////////
// compare and swap with 2 contiguous [u]int32
bool dia_cas2(volatile void *ptr, 
			  dia_uint32_t old1, dia_uint32_t old2, 
			  dia_uint32_t new1, dia_uint32_t new2)
{
#ifdef __GNUC__
	bool ret;
	__asm__ __volatile__(
		"pushl %%ebx;"
		"movl %4,%%ebx;"
		"lock cmpxchg8b (%1); sete %%al;"
		"pop %%ebx"
		: "=a"(ret)
		: "D"((dia_uint64_t*)ptr), "a"(old1), "d"(old2), "m"(new1), "c"(new2)
		: "memory");
	return ret;
#else
	LONGLONG _old, _new;
	*(LONG*)(&_old) = old1;
	*((LONG*)(&_old)+1) = old2;
	*(LONG*)(&_new) = new1;
	*((LONG*)(&_new)+1) = new2;
//	return (InterlockedCompareExchange64 ((LONGLONG*)ptr, _new, _old) == _old);
	return (PG_InterlockedCompareExchange64 ((LONGLONG*)ptr, _new, _old) == _old);
#endif
}



