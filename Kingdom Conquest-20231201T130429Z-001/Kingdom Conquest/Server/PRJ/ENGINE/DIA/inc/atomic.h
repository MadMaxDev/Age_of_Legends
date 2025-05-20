// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: atomic.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_ATOMIC_H__
#define __DIA_ATOMIC_H__

#include "numeric_type.h"

//////////////////////////////////////////////////////////////////////////
// atomic function

// compare and swap
bool dia_cas(volatile void *ptr, 
			 dia_uint32_t old1, 
			 dia_uint32_t new1);

// compare and swap 2 contiguous [u]int32
bool dia_cas2(volatile void *ptr, 
			  dia_uint32_t old1, dia_uint32_t old2, 
			  dia_uint32_t new1, dia_uint32_t new2);

// double compare and swap
bool dia_dcas(volatile void *ptr1, volatile void *ptr2,
			  dia_uint32_t old1, dia_uint32_t old2,
			  dia_uint32_t new1, dia_uint32_t new2);


//////////////////////////////////////////////////////////////////////////
#endif

