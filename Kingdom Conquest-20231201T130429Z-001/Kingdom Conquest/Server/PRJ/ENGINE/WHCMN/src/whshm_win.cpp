// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whshm_win.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 简单的共享内存功能的WIN32实现版本
// CreationDate : 2004-01-20
// ChangeLOG    : 2005-06-28 把和服务器的UDP通讯改为PIPE，这样就把WHCMN和WHNET彻底分开了

// 不使用UDP，改使用pipe了
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
	// 先置默认值
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
		// 原来存在就应改通过open打开，而不是create
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
		// 关闭句柄
		CloseHandle(pInfo->hSHM);
		pInfo->hSHM	= NULL;
		// 返回错误
		return	-1;
	}

	// 将内存清零
	memset(pInfo->pInfoHdr, 0, nSize);

	return	0;
}

static const char	*cszPipeSvrName	= "\\\\.\\pipe\\wh_xshmsvr";
static int		i_whshm_raw_destroy(whpipe *pPipe, int nKey)
{

	// 通过udp通知本机的shmserver建立nKey的共享内存
	// 生成字串指令
	int		rst;
	char	szCmd[256];
	sprintf(szCmd, "destroy %s", i_whshm_makename(nKey));

	// 通知本机的shmserver建立nKey的共享内存
	if( pPipe->Write(szCmd, strlen(szCmd))<0 )
	{
		return	-2;
	}

	// 当等待返回，超时返回错误
	whtick_t	t	= wh_gettickcount();
	while( 1 )
	{
		if( wh_tickcount_diff(wh_gettickcount(), t) >= CNSELECTTIMEOUT )
		{
			// 超时
			// 没有收到返回(但可能对方反应太慢造成的)
			return	-3;
		}
		pPipe->WaitSelect(50);
		// 看看有没有返回
		int	nSize	= sizeof(szCmd)-1;
		if( pPipe->Read(szCmd, &nSize)<0 )
		{
			// 没有收到返回
			continue;
		}
		// 判断返回是什么
		szCmd[nSize]	= 0;
		char	szRst[64]="", szData[128]="";
		rst		= wh_strsplit("sa", szCmd, " ", szRst, szData);
		if( strcmp("OK", szRst)==0 )
		{
			// 成功了
			return	0;
		}
		else
		{
			// 出错，不管是什么错了，都直接返回，由上层作下一步检查
			return	-4;
		}
	}
	return	0;
}
int		whshm_raw_create(int nKey, size_t nSize, WHSHM_RAW_INFO_T *pInfo)
{
	// 连接pipe服务器
	whsafeptr<whpipe>	pPipe(whpipe_open(cszPipeSvrName, CNCONNECTTIMEOUT, PIPEIOBUFSIZE));
	if( pPipe == NULL )
	{
		return	-1;
	}

	// 生成字串指令
	int		rst;
	char	szCmd[256];
	sprintf(szCmd, "create %s, %d", i_whshm_makename(nKey), nSize);

	// 通知本机的shmserver建立nKey的共享内存
	if( pPipe->Write(szCmd, strlen(szCmd))<0 )
	{
		return	-2;
	}
	// 当等待返回，超时返回错误
	whtick_t	t	= wh_gettickcount();
	while( 1 )
	{
		if( wh_tickcount_diff(wh_gettickcount(), t) >= CNSELECTTIMEOUT )
		{
			// 超时
			// 没有收到返回(但可能对方反应太慢造成的)，为了保险，应该主动销毁一下同名的内存
			i_whshm_raw_destroy(pPipe, nKey);
			return	-3;
		}
		pPipe->WaitSelect(50);
		// 看看有没有返回
		int	nSize	= sizeof(szCmd)-1;
		if( pPipe->Read(szCmd, &nSize)<0 )
		{
			// 没有收到返回
			continue;
		}
		// 判断返回是什么
		szCmd[nSize]	= 0;
		char	szRst[64]="", szData[128]="";
		rst		= wh_strsplit("sa", szCmd, " ", szRst, szData);
		if( strcmp("OK", szRst)==0 )
		{
			// 成功了，然后通过Open打开它
			rst	= whshm_raw_open(nKey, pInfo, pInfo->pInfoHdr);
			if( rst<0 )
			{
				// 删除
				pPipe.selfdelete();
				whshm_raw_destroy(nKey);
				return	rst;
			}
			return	0;
		}
		else
		{
			// 出错，不管是什么错了，都直接返回，由上层作下一步检查
			return	-4;
		}
	}
	return	0;
}
int		whshm_raw_destroy(int nKey)
{
	// 连接pipe服务器
	whsafeptr<whpipe>	pPipe(whpipe_open(cszPipeSvrName, CNCONNECTTIMEOUT, PIPEIOBUFSIZE));
	if( pPipe == NULL )
	{
		return	-1;
	}

	return	i_whshm_raw_destroy(pPipe, nKey);
}

int		i_whshm_raw_open(const char *cszKeyName, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr)
{
	// 先置默认值
	pInfo->hSHM		= NULL;
	pInfo->pInfoHdr	= NULL;

	pInfo->hSHM	= OpenFileMapping(
				FILE_MAP_ALL_ACCESS
				, FALSE
				, cszKeyName
				);
	if( !pInfo->hSHM )
	{
		// 可能不存在
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
		// 关闭句柄
		CloseHandle(pInfo->hSHM);
		pInfo->hSHM	= NULL;
		// 返回错误
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
