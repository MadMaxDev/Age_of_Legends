// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whshm_cmn.h
// Creator      : Wei Hua (魏华)
// Comment      : 有whshm_win.h和whshm_linux.h共同包含的头文件
//              : 注：最后的创建者指给应用层使用的接口提供者。
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
	// pMem的最头里应该是这个结构，这个是由最后的创建者设置的
	struct	INFOHDR_T
	{
		size_t			nSize;					// 这个nSize并不是内存的总大小，而是排除了头部之后的大小，由最后的创建者设置的
		void			*pOldMem;				// 上次的内存指针
		unsigned char	nStatus;				// 工作状态
		unsigned char	junk[3];				// 凑足4对齐的
	};
	INFOHDR_T	*pInfoHdr;						// 这个是放在申请的共享内存头部的（如果这个不为NULL则尝试创建到这个地方）
	inline void	clear()
	{
		WHMEMSET0THIS();
	}
};

int		whshm_raw_create(int nKey, size_t nSize, WHSHM_RAW_INFO_T *pInfo);	// 如果成功会填写pInfo中的内容
int		whshm_raw_open(int nKey, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr);
int		whshm_raw_close(WHSHM_RAW_INFO_T *pInfo);
int		whshm_raw_destroy(int nKey);

}				// EOF namespace n_whcmn

#endif	// EOF __WHSHM_CMN_H__

