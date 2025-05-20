// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whshm.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵Ĺ����ڴ湦��(��windos��ʹ����Ҫ�����繦��)
// CreationDate : 2004-01-20
// ChangeLOG    : 2006-05-22 ȡ����whshm::GetMemHdr

#ifndef	__WHSHM_H__
#define	__WHSHM_H__

#include <stdlib.h>

namespace	n_whcmn
{

class	whshm
{
public:
	// create����open�õ����ڴ����ͨ��ֱ��deleteָ���ͷż���
	virtual ~whshm()					{}
	virtual void *	GetBuf() const		= 0;	// ��ÿ��õĹ����ڴ濪ʼ�ĵط�
	virtual size_t	GetSize() const		= 0;	// ��ÿ��õĹ����ڴ�ȫ���ߴ�
	virtual long	GetMemDiff() const	= 0;	// ����¾��ڴ�ָ���ƫ����(����ڼ̳о��ڴ��ʱ������)
	virtual void	SaveCurMemHdrPtr()	= 0;	// ���浱ǰ���ڴ�ͷָ�룬�����Ҫ�����ڼ̳оɵ��ڴ沢����֮��
	virtual void	SetStatus(unsigned char nStatus)	= 0;
												// ����״̬��Ĭ���ʼ��0
	virtual unsigned char	GetStatus() const			= 0;
												// ���״̬��Ĭ���ʼ��0
};

// ����key���������ڴ�
whshm *	whshm_create(int nKey, size_t nSize, void *pBaseAddr=NULL);
// ����key���Ѿ������Ĺ����ڴ棨pBaseAddr��ʾҪӳ�䵽�ı����̿ռ��еĵ�ַ��NULL��ʾ��ϵͳѡ��
whshm *	whshm_open(int nKey, void *pBaseAddr=NULL);

// ͨ������ɾ�������ڴ棬ע����һ�����������û�����ʹ����������²�����
// �Ƿ�ʹ�ú���Ը�!@#!@##@!
int		whshm_destroy(int nKey);

}		// EOF namespace n_whcmn

#endif	// EOF__WHSHM_H__
