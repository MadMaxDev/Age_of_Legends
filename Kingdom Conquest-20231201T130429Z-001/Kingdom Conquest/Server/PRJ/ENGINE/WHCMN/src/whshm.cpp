// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whshm.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 简单的共享内存功能的通用功能实现
// CreationDate : 2004-01-20
// ChangeLOG    : 2006-05-22 取消了whshm::GetMemHdr

#include "../inc/whshm.h"
#include "../inc/whshm_win.h"
#include "../inc/whshm_linux.h"
#include "../inc/whcmn_def.h"

namespace	n_whcmn
{

class	whshm_i	: public whshm
{
public:
	WHSHM_RAW_INFO_T	m_info;
public:
	// create或者open得到的内存最后通过直接delete指针释放即可
	whshm_i()
	{
		m_info.clear();
	}
	virtual ~whshm_i()
	{
		whshm_raw_close(&m_info);
	}
	virtual void *	GetBuf() const
	{
		return	wh_getptrnexttoptr(m_info.pInfoHdr);
	}
	virtual size_t	GetSize() const
	{
		return	m_info.pInfoHdr->nSize;
	}
	virtual long	GetMemDiff() const
	{
		return	long(m_info.pInfoHdr) - long(m_info.pInfoHdr->pOldMem);
	}
	virtual void	SaveCurMemHdrPtr()
	{
		m_info.pInfoHdr->pOldMem	= m_info.pInfoHdr;
	}
	virtual void	SetStatus(unsigned char nStatus)
	{
		m_info.pInfoHdr->nStatus	= nStatus;
	}
	virtual unsigned char	GetStatus() const
	{
		return	m_info.pInfoHdr->nStatus;
	}
};

whshm *	whshm_create(int nKey, size_t nSize, void *pBaseAddr)
{
	whshm_i	*pSHM = new whshm_i;
	pSHM->m_info.pInfoHdr	= (WHSHM_RAW_INFO_T::INFOHDR_T *)pBaseAddr;
	if( whshm_raw_create(nKey, nSize+sizeof(WHSHM_RAW_INFO_T::INFOHDR_T), &pSHM->m_info)==0 )
	{
		pSHM->SaveCurMemHdrPtr();
		pSHM->m_info.pInfoHdr->nSize	= nSize;
		return	pSHM;
	}
	delete	pSHM;
	return	NULL;
}
whshm *	whshm_open(int nKey, void *pBaseAddr)
{
	whshm_i	*pSHM = new whshm_i;
	if( whshm_raw_open(nKey, &pSHM->m_info, pBaseAddr)==0 )
	{
		return	pSHM;
	}
	delete	pSHM;
	return	NULL;
}

int		whshm_destroy(int nKey)
{
	return	whshm_raw_destroy(nKey);
}

}				// EOF namespace n_whcmn
