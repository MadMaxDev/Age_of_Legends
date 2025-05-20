// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whpipe.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵�PIPE���ܣ�����ɾ��pipe����ܵ����ر�
//              : ע�⣺���ܶ��߳�ʹ�ã�Ϊɶ������:(��
//              : 
// CreationDate : 2004-01-20

#ifndef	__WHPIPE_H__
#define	__WHPIPE_H__

namespace	n_whcmn
{

// Ĭ��ʵ�־���ʲôҲ����
class	whpipe
{
public:
	virtual ~whpipe()		{}
	virtual void	Disconnect()
	{
	}
	virtual int	Write(const void *pData, int nSize)
	{
		return	0;
	}
	virtual int	Read(void *pData, int *pnSize)
	{
		return	0;
	}
	// ����1�ʹ��������룬0��ʾû�����룬-1��ʾ��������(����Է�����)��
	virtual int		WaitSelect(int nMS)
	{
		return	0;
	}
	virtual bool	IsFailed()
	{
		return	false;
	}
};

// ����pipe�Ⱥ�����
whpipe *	whpipe_create(const char *cszPipeName, int nIOBufSize);
// ����һ���Ѿ������õ�pipe
whpipe *	whpipe_open(const char *cszPipeName, int nTimeOut, int nIOBufSize);

}		// EOF namespace n_whcmn

#endif	// EOF __WHPIPE_H__
