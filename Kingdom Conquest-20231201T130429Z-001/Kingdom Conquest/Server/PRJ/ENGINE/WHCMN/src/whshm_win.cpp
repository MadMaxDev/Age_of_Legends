// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whshm_win.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵Ĺ����ڴ湦�ܵ�WIN32ʵ�ְ汾
// CreationDate : 2004-01-20
// ChangeLOG    : 2005-06-28 �Ѻͷ�������UDPͨѶ��ΪPIPE�������Ͱ�WHCMN��WHNET���׷ֿ���

// ��ʹ��UDP����ʹ��pipe��
//#define	USEUDP

#ifdef	WIN32	// { WIN32 BEGIN

#include "../inc/whpipe.h"
#include "../inc/whshm_win.h"
#include "../inc/whstring.h"
#include "../inc/whtime.h"
#include "../inc/wh_platform.h"
#include <stdio.h>

namespace	n_whcmn
{

static const int	CNCONNECTTIMEOUT	= 20000;
static const int	PIPEIOBUFSIZE		= 4096;
static const int	CNSELECTTIMEOUT		= 10000;

const char *	i_whshm_makename(int nKey, char *szName)
{
	sprintf(szName, "whshm:%08x", nKey);
	return	szName;
}
const char *	i_whshm_makename(int nKey)
{
	static char	szName[32];
	return	i_whshm_makename(nKey, szName);
}
int		i_whshm_raw_create(const char *cszKeyName, size_t nSize, WHSHM_RAW_INFO_T *pInfo)
{
	// ����Ĭ��ֵ
	pInfo->hSHM		= NULL;
	pInfo->pInfoHdr	= NULL;

	pInfo->hSHM	= CreateFileMapping(
		INVALID_HANDLE_VALUE
		, NULL
		, PAGE_READWRITE
		, 0, nSize
		, cszKeyName
		);
	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		// ԭ�����ھ�Ӧ��ͨ��open�򿪣�������create
		CloseHandle(pInfo->hSHM);
		pInfo->hSHM	= NULL;
		return	-1;
	}
	if( !pInfo->hSHM )
	{
		return	-1;
	}

	pInfo->pInfoHdr	= (WHSHM_RAW_INFO_T::INFOHDR_T *)MapViewOfFileEx(
		pInfo->hSHM, FILE_MAP_ALL_ACCESS
		, 0, 0, 0
		, pInfo->pInfoHdr
		);
	if( !pInfo->pInfoHdr )
	{
		// �رվ��
		CloseHandle(pInfo->hSHM);
		pInfo->hSHM	= NULL;
		// ���ش���
		return	-1;
	}

	// ���ڴ�����
	memset(pInfo->pInfoHdr, 0, nSize);

	return	0;
}

static const char	*cszPipeSvrName	= "\\\\.\\pipe\\wh_xshmsvr";
static int		i_whshm_raw_destroy(whpipe *pPipe, int nKey)
{

	// ͨ��udp֪ͨ������shmserver����nKey�Ĺ����ڴ�
	// �����ִ�ָ��
	int		rst;
	char	szCmd[256];
	sprintf(szCmd, "destroy %s", i_whshm_makename(nKey));

	// ֪ͨ������shmserver����nKey�Ĺ����ڴ�
	if( pPipe->Write(szCmd, strlen(szCmd))<0 )
	{
		return	-2;
	}

	// ���ȴ����أ���ʱ���ش���
	whtick_t	t	= wh_gettickcount();
	while( 1 )
	{
		if( wh_tickcount_diff(wh_gettickcount(), t) >= CNSELECTTIMEOUT )
		{
			// ��ʱ
			// û���յ�����(�����ܶԷ���Ӧ̫����ɵ�)
			return	-3;
		}
		pPipe->WaitSelect(50);
		// ������û�з���
		int	nSize	= sizeof(szCmd)-1;
		if( pPipe->Read(szCmd, &nSize)<0 )
		{
			// û���յ�����
			continue;
		}
		// �жϷ�����ʲô
		szCmd[nSize]	= 0;
		char	szRst[64]="", szData[128]="";
		rst		= wh_strsplit("sa", szCmd, " ", szRst, szData);
		if( strcmp("OK", szRst)==0 )
		{
			// �ɹ���
			return	0;
		}
		else
		{
			// ����������ʲô���ˣ���ֱ�ӷ��أ����ϲ�����һ�����
			return	-4;
		}
	}
	return	0;
}
int		whshm_raw_create(int nKey, size_t nSize, WHSHM_RAW_INFO_T *pInfo)
{
	// ����pipe������
	whsafeptr<whpipe>	pPipe(whpipe_open(cszPipeSvrName, CNCONNECTTIMEOUT, PIPEIOBUFSIZE));
	if( pPipe == NULL )
	{
		return	-1;
	}

	// �����ִ�ָ��
	int		rst;
	char	szCmd[256];
	sprintf(szCmd, "create %s, %d", i_whshm_makename(nKey), nSize);

	// ֪ͨ������shmserver����nKey�Ĺ����ڴ�
	if( pPipe->Write(szCmd, strlen(szCmd))<0 )
	{
		return	-2;
	}
	// ���ȴ����أ���ʱ���ش���
	whtick_t	t	= wh_gettickcount();
	while( 1 )
	{
		if( wh_tickcount_diff(wh_gettickcount(), t) >= CNSELECTTIMEOUT )
		{
			// ��ʱ
			// û���յ�����(�����ܶԷ���Ӧ̫����ɵ�)��Ϊ�˱��գ�Ӧ����������һ��ͬ�����ڴ�
			i_whshm_raw_destroy(pPipe, nKey);
			return	-3;
		}
		pPipe->WaitSelect(50);
		// ������û�з���
		int	nSize	= sizeof(szCmd)-1;
		if( pPipe->Read(szCmd, &nSize)<0 )
		{
			// û���յ�����
			continue;
		}
		// �жϷ�����ʲô
		szCmd[nSize]	= 0;
		char	szRst[64]="", szData[128]="";
		rst		= wh_strsplit("sa", szCmd, " ", szRst, szData);
		if( strcmp("OK", szRst)==0 )
		{
			// �ɹ��ˣ�Ȼ��ͨ��Open����
			rst	= whshm_raw_open(nKey, pInfo, pInfo->pInfoHdr);
			if( rst<0 )
			{
				// ɾ��
				pPipe.selfdelete();
				whshm_raw_destroy(nKey);
				return	rst;
			}
			return	0;
		}
		else
		{
			// ����������ʲô���ˣ���ֱ�ӷ��أ����ϲ�����һ�����
			return	-4;
		}
	}
	return	0;
}
int		whshm_raw_destroy(int nKey)
{
	// ����pipe������
	whsafeptr<whpipe>	pPipe(whpipe_open(cszPipeSvrName, CNCONNECTTIMEOUT, PIPEIOBUFSIZE));
	if( pPipe == NULL )
	{
		return	-1;
	}

	return	i_whshm_raw_destroy(pPipe, nKey);
}

int		i_whshm_raw_open(const char *cszKeyName, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr)
{
	// ����Ĭ��ֵ
	pInfo->hSHM		= NULL;
	pInfo->pInfoHdr	= NULL;

	pInfo->hSHM	= OpenFileMapping(
				FILE_MAP_ALL_ACCESS
				, FALSE
				, cszKeyName
				);
	if( !pInfo->hSHM )
	{
		// ���ܲ�����
		return	-1;
	}
	pInfo->pInfoHdr	= (WHSHM_RAW_INFO_T::INFOHDR_T *)MapViewOfFileEx(
		pInfo->hSHM, FILE_MAP_ALL_ACCESS
		, 0, 0, 0
		, pBaseAddr
		);
	if( !pInfo->pInfoHdr )
	{
		DWORD	nErr	= GetLastError();
		// �رվ��
		CloseHandle(pInfo->hSHM);
		pInfo->hSHM	= NULL;
		// ���ش���
		return	-1;
	}

	return	0;
}
int		whshm_raw_open(int nKey, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr)
{
	return	i_whshm_raw_open(i_whshm_makename(nKey), pInfo, pBaseAddr);
}
int		whshm_raw_close(WHSHM_RAW_INFO_T *pInfo)
{
	BOOL	bVal = TRUE;
	if( pInfo->pInfoHdr )
	{
		bVal			= UnmapViewOfFile(pInfo->pInfoHdr);
		pInfo->pInfoHdr	= NULL;
	}
	if( pInfo->hSHM )
	{
		bVal			= CloseHandle(pInfo->hSHM);
		pInfo->hSHM		= NULL;
	}
	return	0;
}

}				// EOF namespace n_whcmn

#endif			// } EOF WIN32
