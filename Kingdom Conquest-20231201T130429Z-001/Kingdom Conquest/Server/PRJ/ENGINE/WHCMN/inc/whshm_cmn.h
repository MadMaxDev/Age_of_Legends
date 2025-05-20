// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whshm_cmn.h
// Creator      : Wei Hua (κ��)
// Comment      : ��whshm_win.h��whshm_linux.h��ͬ������ͷ�ļ�
//              : ע�����Ĵ�����ָ��Ӧ�ò�ʹ�õĽӿ��ṩ�ߡ�
// CreationDate : 2006-05-23
// ChangeLOG    : 

#ifndef	__WHSHM_CMN_H__
#define	__WHSHM_CMN_H__

#include "../inc/wh_platform.h"
#include "../inc/whcmn_def.h"

namespace	n_whcmn
{

struct	WHSHM_RAW_INFO_T
{
#ifdef	WIN32
	HANDLE	hSHM;
#endif
#ifdef	__GNUC__
	int		hSHM;
#endif
	// pMem����ͷ��Ӧ��������ṹ������������Ĵ��������õ�
	struct	INFOHDR_T
	{
		size_t			nSize;					// ���nSize�������ڴ���ܴ�С�������ų���ͷ��֮��Ĵ�С�������Ĵ��������õ�
		void			*pOldMem;				// �ϴε��ڴ�ָ��
		unsigned char	nStatus;				// ����״̬
		unsigned char	junk[3];				// ����4�����
	};
	INFOHDR_T	*pInfoHdr;						// ����Ƿ�������Ĺ����ڴ�ͷ���ģ���������ΪNULL���Դ���������ط���
	inline void	clear()
	{
		WHMEMSET0THIS();
	}
};

int		whshm_raw_create(int nKey, size_t nSize, WHSHM_RAW_INFO_T *pInfo);	// ����ɹ�����дpInfo�е�����
int		whshm_raw_open(int nKey, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr);
int		whshm_raw_close(WHSHM_RAW_INFO_T *pInfo);
int		whshm_raw_destroy(int nKey);

}				// EOF namespace n_whcmn

#endif	// EOF __WHSHM_CMN_H__

