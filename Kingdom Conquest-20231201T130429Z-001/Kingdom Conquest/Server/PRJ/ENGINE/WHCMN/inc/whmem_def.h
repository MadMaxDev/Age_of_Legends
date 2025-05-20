// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whmem_def.h
// Creator: Wei Hua (魏华)
// Comment: 内存管理相关的内外都用的类型和方法定义
// CreationDate: 2003-05-10

#pragma message("this file "__FILE__" should not be included!")

/*
#ifndef	__WHMEM_DEF_H__
#define	__WHMEM_DEF_H__

#include <stdlib.h>

namespace n_whcmn
{

typedef char ALLOC_TYPE;
enum
{
	MM_UNKNOWN        = 0,
	MM_NEW            = 1,
	MM_NEW_ARRAY      = 2,
	MM_MALLOC         = 3,
	MM_CALLOC         = 4,
	MM_REALLOC        = 5,
	MM_DELETE         = 6,
	MM_DELETE_ARRAY   = 7,
	MM_FREE           = 8,
	// 申请类型的范围
	MM_BEGIN          = 1,
	MM_END            = 5,
};

void *	allocmem( const char *file, int line, size_t size, ALLOC_TYPE type, void *oldaddress = 0 );
void	freemem( void *address, ALLOC_TYPE type );
void	setmemowner( const char *file, int line );

// 如果想改变配置参数，使用这个函数
void	setmemopt( int opt, void *dat, size_t size );
void	setmemoptint( int opt, int dat );
// opt 参数
enum
{
	MMOPT_LOGFILE			= 1,				// dat : 日志文件名，size : 不用
	MMOPT_OUTPUTBIT			= 2,				// dat : 输出到哪些地方的掩码uint，size : 不用
};
// OUTPUTBIT 定义
enum
{
	MM_OUTPUTBIT_DBGWIN		= 1,				// 调试窗口
	MM_OUTPUTBIT_LOGFILE	= 2,				// 日志文件
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHMEM_DEF_H__
*/