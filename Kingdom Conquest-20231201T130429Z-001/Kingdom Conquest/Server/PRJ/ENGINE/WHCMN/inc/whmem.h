// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whmem.h
// Creator: Wei Hua (魏华)
// Comment: 内存管理相关的功能
//          __FILE__, __LINE__ 两个宏在linux下面也可以用的
//          
// CreationDate: 2003-05-09

// 使用方法：
// 1.  将"你的根路径/WHCMN/inc"目录加入头文件搜索路径
// 2.  在需要监控的文件最顶部包含whmem.h文件，如：#include <whmem.h>
// 3.  将"你的根路径/WHCMN/lib"目录加入头库搜索路径，并让工程包含WHCMN.lib；或者让你的工程直接依赖于WHCMN工程
// 4.  要打开检测开关：在工程中加入宏定义WHMEM_ACTIVATED，如果需要多线程则还要添加WHMEM_MT
// 
// 参数设置：
// 1.  setmemopt(MMOPT_LOGFILE, 日志名字串, 0)可以更改日志文件名。默认为WHMEM.log
// 2.  setmemoptint(MMOPT_OUTPUTBIT, MM_OUTPUTBIT_DBGWIN 和 MM_OUTPUTBIT_LOGFILE 组合)
//     可以设置日志输出到debug窗口或文件

#pragma message("this file "__FILE__" should not be included!")

/*

#ifndef	__WHMEM_H__
#define	__WHMEM_H__

////////////////////////////////////////////////////////////////////
// 打开开关，必须把这个定义放在工程中!!!!
////////////////////////////////////////////////////////////////////
// #define	WHMEM_ACTIVATED
// 是否是多线程
// #define	WHMEM_MT

////////////////////////////////////////////////////////////////////
// 定义
////////////////////////////////////////////////////////////////////
//
#ifndef	WHMEM_ACTIVATED
#define	WHMEM_SETMEMOPT	0
#else
#define	WHMEM_SETMEMOPT	setmemopt
#endif

//
#ifdef	WHMEM_ACTIVATED

#include <stdlib.h>
#include "whmem_newoff.h"
#include "whmem_def.h"

inline void* operator new( size_t size, const char *file, int line ) 
{
	return n_whcmn::allocmem( file, line, size, n_whcmn::MM_NEW ); 
}
inline void* operator new[]( size_t size, const char *file, int line )
{
	return n_whcmn::allocmem( file, line, size, n_whcmn::MM_NEW_ARRAY ); 
}
inline void operator delete( void *address )
{
	n_whcmn::freemem( address, n_whcmn::MM_DELETE );
}
inline void operator delete[]( void *address )
{
	n_whcmn::freemem( address, n_whcmn::MM_DELETE_ARRAY );
}

// These 2 are defined to make VC Happy, avoiding C4291 warning
inline void operator delete( void *address, const char *file, int line )   { free( address ); }
inline void operator delete[]( void *address, const char *file, int line ) { free( address ); }

////////////////////////////////////////////////////////////////////
// 最后，用于程序中的定义才是这样的
////////////////////////////////////////////////////////////////////
#define new              new( __FILE__, __LINE__ )
#define delete           (n_whcmn::setmemowner( __FILE__, __LINE__ ), false) ? 0 : delete
#define malloc(sz)       n_whcmn::allocmem( __FILE__, __LINE__, sz,     n_whcmn::MM_MALLOC       )
#define calloc(num, sz)  n_whcmn::allocmem( __FILE__, __LINE__, sz*num, n_whcmn::MM_CALLOC       )
#define realloc(ptr, sz) n_whcmn::allocmem( __FILE__, __LINE__, sz,     n_whcmn::MM_REALLOC, ptr )
#define free(sz)         n_whcmn::freemem ( sz, n_whcmn::MM_FREE )

#endif	// EOF WHMEM_ACTIVATED

#endif	// EOF __WHMEM_H__

*/