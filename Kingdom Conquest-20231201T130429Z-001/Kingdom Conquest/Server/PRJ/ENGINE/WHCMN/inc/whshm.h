// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whshm.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的共享内存功能(在windos下使用需要开网络功能)
// CreationDate : 2004-01-20
// ChangeLOG    : 2006-05-22 取消了whshm::GetMemHdr

#ifndef	__WHSHM_H__
#define	__WHSHM_H__

#include <stdlib.h>

namespace	n_whcmn
{

class	whshm
{
public:
	// create或者open得到的内存最后通过直接delete指针释放即可
	virtual ~whshm()					{}
	virtual void *	GetBuf() const		= 0;	// 获得可用的共享内存开始的地方
	virtual size_t	GetSize() const		= 0;	// 获得可用的共享内存全部尺寸
	virtual long	GetMemDiff() const	= 0;	// 获得新旧内存指针的偏移量(这个在继承旧内存的时候有用)
	virtual void	SaveCurMemHdrPtr()	= 0;	// 保存当前的内存头指针，这个主要是用于继承旧的内存并整理之后
	virtual void	SetStatus(unsigned char nStatus)	= 0;
												// 设置状态，默认最开始是0
	virtual unsigned char	GetStatus() const			= 0;
												// 获得状态，默认最开始是0
};

// 根据key创建共享内存
whshm *	whshm_create(int nKey, size_t nSize, void *pBaseAddr=NULL);
// 根据key打开已经创建的共享内存（pBaseAddr表示要映射到的本进程空间中的地址，NULL表示由系统选择）
whshm *	whshm_open(int nKey, void *pBaseAddr=NULL);

// 通过名字删除共享内存，注意这一定是在所有用户都不使用它的情况下才能用
// 非法使用后果自负!@#!@##@!
int		whshm_destroy(int nKey);

}		// EOF namespace n_whcmn

#endif	// EOF__WHSHM_H__
