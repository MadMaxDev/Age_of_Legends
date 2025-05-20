// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstreambuffer.h
// Creator      : Wei Hua (魏华)
// Comment      : 纯数据流缓冲。在Out函数中回导致GetBuf获得指针失效
// CreationDate : 2003-10-06
// 

#ifndef	__WHSTREAMBUFFER_H__
#define	__WHSTREAMBUFFER_H__

#include <stdlib.h>
#include "whlock.h"

namespace n_whcmn
{

// whstreambuffer在做in操作的时候，是不会导致指针变化的
class	whstreambuffer
{
protected:
	char	*m_pBuf;								// 用于申请数据缓冲区
	size_t	m_nMaxSize;								// 可以存放的最大数据长度
	size_t	m_nSize;								// 已经存放的数据总长度
	size_t	m_nBegin;								// 数据开始的位置
	size_t	m_nScrollSize;							// 当取出数据导致m_nBegin超过m_nScrollSize，就将所有数据滚到缓冲头部
public:
	inline char *	GetBuf() const					// 目前数据的开始指针，在下一个变更操作(Free)前有效
	{
		return	m_pBuf + m_nBegin;
	}
	inline size_t	GetMaxSize() const
	{
		return	m_nMaxSize;
	}
	inline size_t	GetSize() const					// 获得已经存在的数据长度
	{
		return	m_nSize;
	}
	inline char *	GetTail() const					// 获得尾指针
	{
		return	m_pBuf + m_nBegin + m_nSize;
	}
	inline size_t	GetSizeLeft() const				// 获得还可以添加的数据长度
	{
		return	m_nMaxSize - m_nBegin - m_nSize;
	}
public:
	whstreambuffer();
	~whstreambuffer();
	int		Init(size_t nMaxSize, size_t nTailPadding, size_t nScrollSize);
													// 初始化缓冲。nTailPadding表明在结尾处添加若干字节不用。
	int		Release();								// 终结(会在析构中自动调用)
	void	Clean();								// 清除缓冲中的所有数据
	void *	InAlloc(size_t nSize);					// 申请加入缓冲内存，自行填写内容，避免多申请拷贝一次
	int		In(const void *pData, size_t nSize);	// 加入长度为nSize的数据，返回0表示成功，-1表明不能加入，不会出现加入部分的情况
	int		Out(void *pData, size_t *pnSize);		// 取出数据，*pnSize里面存放取得的数据长度，返回0表示成功获得数据，-1表示没有数据
	void	Out(size_t nSize);						// 释放数据
};

class	whstreambufferWL :	public	whstreambuffer
{
private:
	whlock	m_lock;
public:
	inline void	ReInitLock()
	{
		m_lock.reinit();
	}
	inline void	Clean()
	{
		m_lock.lock();
		whstreambuffer::Clean();
		m_lock.unlock();
	}
	inline void *	InAlloc(size_t nSize)
	{
		m_lock.lock();
		void *	ptr = whstreambuffer::InAlloc(nSize);
		m_lock.unlock();
		return	ptr;
	}
	int		In(const void *pData, size_t nSize)
	{
		m_lock.lock();
		int		rst = whstreambuffer::In(pData, nSize);
		m_lock.unlock();
		return	rst;
	}
	inline int		Out(void *pData, size_t *pnSize)
	{
		m_lock.lock();
		int		rst = whstreambuffer::Out(pData, pnSize);
		m_lock.unlock();
		return	rst;
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHSTREAMBUFFER_H__
