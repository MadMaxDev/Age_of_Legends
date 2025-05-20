// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace : n_whcmn
// File      : whshm_linux.cpp
// Creator   : Wei Hua (κ��)
// Comment   : �򵥵Ĺ����ڴ湦�ܵ�Linuxʵ�ְ汾
// ChangeLOG : 2007-01-12 �޸���whshm_raw_open��shmat�Ĵ�������shmat�����Ƿ���-1�������Ƿ���NULL

#ifdef	__GNUC__	// { Linux BEGIN

#include <sys/ipc.h>
#include <sys/shm.h>
#include "../inc/whshm_linux.h"

namespace	n_whcmn
{

int		whshm_raw_create(int nKey, size_t nSize, WHSHM_RAW_INFO_T *pInfo)
{
	// ����(��֤ԭ��������)
	pInfo->hSHM	= shmget((key_t)nKey, nSize, IPC_CREAT | IPC_EXCL | 0600);
	if( pInfo->hSHM<0)
	{
		return	-1;
	}
	// attach
	pInfo->pInfoHdr	= (WHSHM_RAW_INFO_T::INFOHDR_T *)shmat(pInfo->hSHM, pInfo->pInfoHdr, 0);
	if( !pInfo->pInfoHdr )
	{
		// ���ͷ����
		shmctl(pInfo->hSHM, IPC_RMID, NULL);
		// ���ش���
		return	-2;
	}
	// ���ڴ�����
	memset(pInfo->pInfoHdr, 0, nSize);

	return		0;
}
int		whshm_raw_open(int nKey, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr)
{
	// ��֤ԭ������
	pInfo->hSHM	= shmget((key_t)nKey, 0, 0);
	if( pInfo->hSHM<0)
	{
		return	-1;
	}
	// attach
	pInfo->pInfoHdr	= (WHSHM_RAW_INFO_T::INFOHDR_T *)shmat(pInfo->hSHM, pBaseAddr, 0);
	if( pInfo->pInfoHdr==(WHSHM_RAW_INFO_T::INFOHDR_T *)(-1) )
	{
		// ���ͷ����
		shmctl(pInfo->hSHM, IPC_RMID, NULL);
		// ���ش���
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
	// ��֤ԭ������
	int		hSHM	= shmget((key_t)nKey, 0, 0);
	if( hSHM<0 )
	{
		return	-1;
	}
	// ����Ƴ�
	shmctl(hSHM, IPC_RMID, NULL);
	return	0;
}

}				// EOF namespace n_whcmn

#endif			// } EOF Linux
