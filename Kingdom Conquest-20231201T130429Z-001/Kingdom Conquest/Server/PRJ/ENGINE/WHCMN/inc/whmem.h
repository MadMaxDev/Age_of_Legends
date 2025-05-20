// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whmem.h
// Creator: Wei Hua (κ��)
// Comment: �ڴ������صĹ���
//          __FILE__, __LINE__ ��������linux����Ҳ�����õ�
//          
// CreationDate: 2003-05-09

// ʹ�÷�����
// 1.  ��"��ĸ�·��/WHCMN/inc"Ŀ¼����ͷ�ļ�����·��
// 2.  ����Ҫ��ص��ļ��������whmem.h�ļ����磺#include <whmem.h>
// 3.  ��"��ĸ�·��/WHCMN/lib"Ŀ¼����ͷ������·�������ù��̰���WHCMN.lib����������Ĺ���ֱ��������WHCMN����
// 4.  Ҫ�򿪼�⿪�أ��ڹ����м���궨��WHMEM_ACTIVATED�������Ҫ���߳���Ҫ���WHMEM_MT
// 
// �������ã�
// 1.  setmemopt(MMOPT_LOGFILE, ��־���ִ�, 0)���Ը�����־�ļ�����Ĭ��ΪWHMEM.log
// 2.  setmemoptint(MMOPT_OUTPUTBIT, MM_OUTPUTBIT_DBGWIN �� MM_OUTPUTBIT_LOGFILE ���)
//     ����������־�����debug���ڻ��ļ�

#pragma message("this file "__FILE__" should not be included!")

/*

#ifndef	__WHMEM_H__
#define	__WHMEM_H__

////////////////////////////////////////////////////////////////////
// �򿪿��أ���������������ڹ�����!!!!
////////////////////////////////////////////////////////////////////
// #define	WHMEM_ACTIVATED
// �Ƿ��Ƕ��߳�
// #define	WHMEM_MT

////////////////////////////////////////////////////////////////////
// ����
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
// ������ڳ����еĶ������������
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