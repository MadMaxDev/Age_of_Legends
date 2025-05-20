// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whmem_def.h
// Creator: Wei Hua (κ��)
// Comment: �ڴ������ص����ⶼ�õ����ͺͷ�������
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
	// �������͵ķ�Χ
	MM_BEGIN          = 1,
	MM_END            = 5,
};

void *	allocmem( const char *file, int line, size_t size, ALLOC_TYPE type, void *oldaddress = 0 );
void	freemem( void *address, ALLOC_TYPE type );
void	setmemowner( const char *file, int line );

// �����ı����ò�����ʹ���������
void	setmemopt( int opt, void *dat, size_t size );
void	setmemoptint( int opt, int dat );
// opt ����
enum
{
	MMOPT_LOGFILE			= 1,				// dat : ��־�ļ�����size : ����
	MMOPT_OUTPUTBIT			= 2,				// dat : �������Щ�ط�������uint��size : ����
};
// OUTPUTBIT ����
enum
{
	MM_OUTPUTBIT_DBGWIN		= 1,				// ���Դ���
	MM_OUTPUTBIT_LOGFILE	= 2,				// ��־�ļ�
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHMEM_DEF_H__
*/