// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace : n_whcmn
// File      : whshm_linux.cpp
// Creator   : Wei Hua (魏华)
// Comment   : 简单的共享内存功能的Linux实现版本
// ChangeLOG : 2007-01-12 修改了whshm_raw_open中shmat的处理不当，shmat出错是返回-1，而不是返回NULL

#ifdef	__GNUC__	// { Linux BEGIN

#include <sys/ipc.h>
#include <sys/shm.h>
#include "../inc/whshm_linux.h"

namespace	n_whcmn
{

int		whshm_raw_create(int nKey, size_t nSize, WHSHM_RAW_INFO_T *pInfo)
{
	// 创建(保证原来不存在)
	pInfo->hSHM	= shmget((key_t)nKey, nSize, IPC_CREAT | IPC_EXCL | 0600);
	if( pInfo->hSHM<0)
	{
		return	-1;
	}
	// attach
	pInfo->pInfoHdr	= (WHSHM_RAW_INFO_T::INFOHDR_T *)shmat(pInfo->hSHM, pInfo->pInfoHdr, 0);
	if( !pInfo->pInfoHdr )
	{
		// 先释放这个
		shmctl(pInfo->hSHM, IPC_RMID, NULL);
		// 返回错误
		return	-2;
	}
	// 将内存清零
	memset(pInfo->pInfoHdr, 0, nSize);

	return		0;
}
int		whshm_raw_open(int nKey, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr)
{
	// 保证原来存在
	pInfo->hSHM	= shmget((key_t)nKey, 0, 0);
	if( pInfo->hSHM<0)
	{
		return	-1;
	}
	// attach
	pInfo->pInfoHdr	= (WHSHM_RAW_INFO_T::INFOHDR_T *)shmat(pInfo->hSHM, pBaseAddr, 0);
	if( pInfo->pInfoHdr==(WHSHM_RAW_INFO_T::INFOHDR_T *)(-1) )
	{
		// 先释放这个
		shmctl(pInfo->hSHM, IPC_RMID, NULL);
		// 返回错误
		return	-2;
	}
	return		0;
}
int		whshm_raw_close(WHSHM_RAW_INFO_T *pInfo)
{
	shmdt(pInfo->pInfoHdr);
	pInfo->hSHM		= -1;
	pInfo->pInfoHdr	= NULL;
	return	0;
}
int		whshm_raw_destroy(int nKey)
{
	// 保证原来存在
	int		hSHM	= shmget((key_t)nKey, 0, 0);
	if( hSHM<0 )
	{
		return	-1;
	}
	// 标记移除
	shmctl(hSHM, IPC_RMID, NULL);
	return	0;
}

}				// EOF namespace n_whcmn

#endif			// } EOF Linux
