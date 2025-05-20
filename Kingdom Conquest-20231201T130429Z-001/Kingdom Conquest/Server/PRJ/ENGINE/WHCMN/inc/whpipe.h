// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whpipe.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的PIPE功能，用完删除pipe对象管道即关闭
//              : 注意：不能多线程使用（为啥捏？忘了:(）
//              : 
// CreationDate : 2004-01-20

#ifndef	__WHPIPE_H__
#define	__WHPIPE_H__

namespace	n_whcmn
{

// 默认实现就是什么也不做
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
	// 返回1就代表有输入，0表示没有输入，-1表示出问题了(比如对方断线)。
	virtual int		WaitSelect(int nMS)
	{
		return	0;
	}
	virtual bool	IsFailed()
	{
		return	false;
	}
};

// 创建pipe等候连接
whpipe *	whpipe_create(const char *cszPipeName, int nIOBufSize);
// 连接一个已经建立好的pipe
whpipe *	whpipe_open(const char *cszPipeName, int nTimeOut, int nIOBufSize);

}		// EOF namespace n_whcmn

#endif	// EOF __WHPIPE_H__
