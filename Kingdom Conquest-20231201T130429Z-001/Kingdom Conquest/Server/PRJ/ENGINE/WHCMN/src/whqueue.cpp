// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whqueue.cpp
// Creator      : Wei Hua (魏华)
// Comment      : whsmpqueue简单的变长元素数据队列，从whsmpqueue改编而来
//                可插入的单个单位尺寸在.cpp中定义为unitsize_t，目前是u_short
// 注意(!!!!)   : 插入队列的元素加上单元头的最大长度不能超过nTotalSize的一半
//                否则赶上Head和Tail刚好在中间的情况就在也不能插入了
// CreationDate : 2003-05-21
// ChangeLOG    : 2003-06-27
//                加入QickQueue。并添加在Out释放完毕后自动将队列指针清到0点的功能。
//                2005-02-24 unitsize_t从u_short改为u_long
//                2005-07-01 whsmpqueue::Release中取消了Clean.觉得这里应该没有必要这么做吧。m_pBuf都删除了那些数据还有啥用腻。
//                2005-07-15 whsmpqueue::OutFree中增加了如果尺寸太小则跳过数据的措施。这样可以有人利用这个bug造成上层缓冲区溢出进行攻击。

// 设计说明：
// 所有单元在队列内首位相接，连续排放，FIFO。
// 最多会有m_nTotalSize-1的内存被使用，一个byte的浪费可以使m_nHead和m_nTail的关系简单化

#include "../inc/whqueue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

using namespace n_whcmn;

typedef	unsigned int	unitsize_t;
#pragma pack(1)
struct	SQ_UNIT
{
	unitsize_t	size;
	char		data[1];
};
#pragma pack()
#define	SQ_UNIT_HDR_SIZE	(sizeof(SQ_UNIT)-1)
#define	QQ_UNIT_LEN(dsize)	(sizeof(int) + dsize)

////////////////////////////////////////////////////////////////////
// 简单队列（每个元素可以不等长）
////////////////////////////////////////////////////////////////////
whsmpqueue::whsmpqueue()
: m_nHead			(0)
, m_nTail			(0)
, m_nLEnd			(0)
, m_pBuf			(NULL)
, m_nTotalSize		(0)
, m_nLogicMaxSize	(0)
, m_nSize			(0)
, m_nUnitNum		(0)
, m_bAcceptZeroLen	(false)
{
}
whsmpqueue::~whsmpqueue()
{
	Release();
}
int		whsmpqueue::Init(size_t nTotalSize)
{
	assert( !m_pBuf );

	m_pBuf	= new char[nTotalSize];
	if( !m_pBuf )
	{
		return	-1;
	}

	m_nTotalSize	= nTotalSize;
	m_nLogicMaxSize	= nTotalSize;
	Clean();

	return	0;
}
int		whsmpqueue::Release()
{
	// 有时候会忘了调用Clean();
	// Clean(); 2005-07-01 觉得这里应该没有必要这么做吧。m_pBuf都删除了那些数据还有啥用腻。
	if( m_pBuf )
	{
		delete []	m_pBuf;
		m_pBuf		= NULL;
	}
	return	0;
}
void	whsmpqueue::Clean()
{
	m_nUnitNum	= 0;
	m_nSize		= 0;
	m_nHead		= 0;
	m_nTail		= 0;
	m_nLEnd		= 0;
	Remember();
}
size_t	whsmpqueue::GetMaxInSizse() const
{
	if( m_nTail>=m_nHead )
	{
		return	m_nTotalSize - 1 - m_nTail - SQ_UNIT_HDR_SIZE;
	}
	else
	{
		return	m_nHead - 1 - m_nTail - SQ_UNIT_HDR_SIZE;
	}
}
void *	whsmpqueue::InAlloc(size_t nSize)
{
	size_t	nUnitSize;
	nUnitSize	= SQ_UNIT_HDR_SIZE + nSize;

	if(nUnitSize >= m_nTotalSize/2-1)
	{
		assert(0);
		return	NULL;
	}
	if( nUnitSize+m_nSize >= m_nLogicMaxSize )
	{
		// 超过了逻辑最大值
		return	NULL;
	}

	if( m_nTail>=m_nHead )
	{
		if( m_nTail+nUnitSize < m_nTotalSize )
		{
			goto	GoodEnd;
		}
		else if( nUnitSize<m_nHead )
		{
			m_nLEnd	= m_nTail;
			m_nTail	= 0;
			// 当队列内容为空时，Head需要和Tail一起回绕
			if( m_nHead==m_nLEnd )
			{
				m_nHead	= 0;
			}
			goto	GoodEnd;
		}
	}
	else
	{
		if( m_nTail+nUnitSize < m_nHead )
		{
			goto	GoodEnd;
		}
	}

	return	NULL;

GoodEnd:
	SQ_UNIT	*pRst;
	pRst		= (SQ_UNIT *)(m_pBuf + m_nTail);
	pRst->size	= (unitsize_t)nSize;
	m_nTail		+= nUnitSize;
	m_nSize		+= nUnitSize;
	m_nUnitNum	++;

	return	pRst->data;
}
int		whsmpqueue::In(const void *pData, size_t nSize)
{
	// 空插入就什么也不作
	if( nSize==0 )
	{
		if( !m_bAcceptZeroLen )
		{
			return	0;
		}
	}

	void	*pRst;

	pRst	= InAlloc(nSize);
	if( !pRst )
	{
		return	-1;
	}
	// 也就是说pData可以为空
	if( nSize > 0 )
	{
		memcpy(pRst, pData, nSize);
	}

	return	0;
}
size_t	whsmpqueue::PeekSize() const
{
	if( m_nHead == m_nTail )
	{
		return	0;
	}

	SQ_UNIT	*pRst;
	pRst	= (SQ_UNIT *)(m_pBuf + m_nHead);
	return	pRst->size;
}
void *	whsmpqueue::OutPeek(size_t *pnSize) const
{
	if( m_nHead == m_nTail )
	{
		*pnSize	= 0;
		return	NULL;
	}

	SQ_UNIT	*pRst;
	pRst	= (SQ_UNIT *)(m_pBuf + m_nHead);
	if( pnSize )
	{
		*pnSize	= pRst->size;
	}

	return	pRst->data;
}
void *	whsmpqueue::OutFree(size_t *pnSize)
{
	if( m_nUnitNum == 0 )
	{
		// 不能用m_head == m_tail，因为队列满的时候会有错
		if( pnSize )	*pnSize	= 0;
		return	NULL;
	}

	SQ_UNIT	*pRst;
	size_t		nUnitSize;
	bool		bSkip	= false;
	pRst		= (SQ_UNIT *)(m_pBuf + m_nHead);
	if( (*pnSize)>0 && (*pnSize)<pRst->size )
	{
		// 2005-07-15 加
		assert(0);
		// 跳过这个数据
		bSkip	= true;
	}
	*pnSize		= pRst->size;
	nUnitSize	= SQ_UNIT_HDR_SIZE + (*pnSize);

	m_nHead		+= nUnitSize;
	if( m_nTail<m_nHead )
	{
		if( m_nHead==m_nLEnd )
		{
			m_nHead	= 0;
		}
		else
		{
			assert(m_nHead<m_nLEnd);
		}
		// m_nHead>m_nLEnd的情况在正常操作下是不会出现的，
		// 除非有人改变了队列中的元素头
	}
	// m_nTail>=m_nHead的情况就不用别的操作了，等待m_nHead慢慢接近m_nTail

	m_nUnitNum	--;
	m_nSize		-= nUnitSize;
	assert(m_nSize>=0);
	// 如果全没了就回零点啦，这样下次可以放大一些的数据
	if( m_nUnitNum == 0 )
	{
		Clean();
	}

	if( bSkip )
	{
		return	NULL;
	}
	return	pRst->data;
}
int		whsmpqueue::Out(void *pData, size_t *pnSize)
{
	void	*pRst;

	pRst	= OutFree(pnSize);
	if( !pRst )
	{
		return	-1;
	}
	memcpy(pData, pRst, *pnSize);

	return	0;
}
int		whsmpqueue::FreeN(int nNum)
{
	if( m_nUnitNum <= (size_t)nNum )
	{
		// 这样就全部释放了啦
		nNum	= m_nUnitNum;
		Clean();
		return	nNum;
	}
	int			nCount = 0;
	size_t		nUnitSize;
	
	while( nCount<nNum )
	{
		SQ_UNIT	*pRst	= (SQ_UNIT *)(m_pBuf + m_nHead);
		nUnitSize	= SQ_UNIT_HDR_SIZE + pRst->size;
		m_nHead		+= nUnitSize;
		if( m_nTail<m_nHead )
		{
			if( m_nHead==m_nLEnd )
			{
				m_nHead	= 0;
			}
			else
			{
				assert(m_nHead<m_nLEnd);
			}
		}
		m_nUnitNum	--;
		m_nSize		-= nUnitSize;
		nCount		++;
	}

	return	nCount;
}
int		whsmpqueue::In2(const void *pData0, size_t nSize0, const void *pData, size_t nSize)
{
	char *ptr;

	ptr	= (char *)InAlloc(nSize0 + nSize);
	if( !ptr )
	{
		return	-1;
	}

	if( nSize0 > 0 )
	{
		memcpy(ptr, pData0, nSize0);
	}
	if( nSize > 0 )
	{
		memcpy(ptr+nSize0, pData, nSize);
	}
	return	0;
}
int		whsmpqueue::Out2(void *pData0, size_t nSize0, void *pData, size_t *pnSize)
{
	char *ptr;

	ptr	= (char *)OutFree(pnSize);
	if( !ptr )
	{
		return	-1;
	}
	(*pnSize)	-= nSize0;
	if( (*pnSize)<0 )
	{
		return	-1;
	}
	memcpy(pData0, ptr, nSize0);
	memcpy(pData, ptr+nSize0, (*pnSize));

	return	0;
}
void	whsmpqueue::Remember()
{
	m_remember.nUnitNum		= m_nUnitNum;
	m_remember.nSize		= m_nSize;
	m_remember.nHead		= m_nHead;
	m_remember.nTail		= m_nTail;
	m_remember.nLEnd		= m_nLEnd;
}
void	whsmpqueue::RollBack()
{
	m_nUnitNum		= m_remember.nUnitNum;
	m_nSize			= m_remember.nSize;
	m_nHead			= m_remember.nHead;
	m_nTail			= m_remember.nTail;
	m_nLEnd			= m_remember.nLEnd;
}

////////////////////////////////////////////////////////////////////
// 快速队列（每个元素必须不大于一定长度，但是会按照最大长度存储）
// 而且内部默认长度是4的倍数
////////////////////////////////////////////////////////////////////
whquickqueue::whquickqueue()
: m_nHead			(0)				// 指向第一个已分配的位置
, m_nTail			(0)				// 指向第一个可分配的位置
, m_nCurPos			(0)				// 当前检索访问的位置
, m_nCount			(0)				// 还需要访问的个数
, m_nUnitNum		(0)				// 当前单元总数
, m_pBuf			(NULL)			// 数据缓冲区
, m_nUnitSize		(0)				// 一个单元含头部(整形的长度)的长度
, m_nTotalSize		(0)				// 缓冲区的总长度 (m_nUnitSize * m_nMaxUnit
, m_nUnitDataSize	(0)				// 一个单元数据部分的长度
, m_nMaxUnit		(0)				// 最多存储的单元数量
, m_nLogicMaxUnit	(0)				// 逻辑上最多存储的单元数量
{
}
whquickqueue::~whquickqueue()
{
	Release();
}
int		whquickqueue::Init(int nMaxUnit, int nUnitDataSize)
{
	assert( !m_pBuf );

	m_nUnitDataSize	= nUnitDataSize;
	m_nMaxUnit		= nMaxUnit;
	m_nLogicMaxUnit	= nMaxUnit;
	m_nUnitSize		= QQ_UNIT_LEN(nUnitDataSize);
	m_nTotalSize	= m_nUnitSize * m_nMaxUnit;

	m_pBuf	= new char[m_nTotalSize];
	if( !m_pBuf )
	{
		return	-1;
	}

	Clean();

	return	0;
}
int		whquickqueue::Release()
{
	if( m_pBuf )
	{
		delete []	m_pBuf;
		m_pBuf	= NULL;
	}
	return	0;
}
void	whquickqueue::Clean()
{
	m_nHead		= 0;
	m_nTail		= 0;
	m_nCurPos	= 0;
	m_nCount	= 0;
	m_nUnitNum	= 0;
}
void *	whquickqueue::InAlloc(int nSize)
{
	if( nSize > m_nUnitDataSize )
	{
		// 无法申请过大的内存
		return	NULL;
	}
	if( m_nUnitNum >= m_nLogicMaxUnit )
	{
		// 超过了逻辑限制
		return	NULL;
	}
	if( m_nUnitNum == m_nMaxUnit )
	{
		// 分配光了
		// 此时m_nTail==m_nHead
		return	NULL;
	}

	if( m_nTail >= m_nHead )
	{
		if( m_nTail < m_nTotalSize )
		{
			goto	GoodEnd;
		}
		else if( 0 < m_nHead )
		{
			m_nTail	= 0;
			goto	GoodEnd;
		}
	}
	else
	{
		// m_nTail在前面一定有空间
		goto	GoodEnd;
	}

	return	NULL;

GoodEnd:
	char	*pRst;
	pRst		= m_pBuf + m_nTail;
	m_nTail		+= m_nUnitSize;
	m_nUnitNum	++;

	*(int *)pRst	= nSize;
	pRst		+= sizeof(int);

	return	pRst;
}
int		whquickqueue::In(const void *pData, int nSize)
{
	void	*pRst;

	pRst	= InAlloc(nSize);
	if( !pRst )
	{
		return	-1;
	}
	if( nSize>0 )
	{
		memcpy(pRst, pData, nSize);
	}

	return	0;
}
void *	whquickqueue::OutPeek(int *pnSize) const
{
	if( m_nUnitNum == 0 )
	{
		*pnSize	= 0;
		return	NULL;
	}

	char	*pRst;
	pRst	= m_pBuf + m_nHead;
	if( pnSize )
	{
		*pnSize	= *(int *)pRst;
	}

	return	pRst + sizeof(int);
}
void *	whquickqueue::OutFree(int *pnSize)
{
	if( m_nUnitNum == 0 )
	{
		if( pnSize )	*pnSize	= 0;
		return	NULL;
	}

	char	*pRst;
	pRst	= m_pBuf + m_nHead;
	if( pnSize )
	{
		assert((*pnSize)==0 || (*pnSize)>=(*(int *)pRst));
		*pnSize	= *(int *)pRst;
	}
	pRst	+= sizeof(int);

	m_nHead	+= m_nUnitSize;
	if( m_nHead == m_nTotalSize )
	{
		// 回绕
		m_nHead	= 0;
		if( m_nTail == m_nTotalSize )
		{
			// 一起回绕
			m_nTail	= 0;
		}
	}
	m_nUnitNum	--;
	// 如果全没了就回零点啦，这样下次可以放大一些的数据
	if( m_nUnitNum == 0 )
	{
		Clean();
	}

	return	pRst;
}
int		whquickqueue::Out(void *pData, int *pnSize)
{
	const void	*pRst;

	pRst	= OutFree(pnSize);
	if( !pRst )
	{
		return	-1;
	}
	memcpy(pData, pRst, *pnSize);

	return	0;
}
int		whquickqueue::FreeN(int nNum)
{
	if( m_nUnitNum <= nNum )
	{
		// 这样就全部释放了啦
		nNum	= m_nUnitNum;
		Clean();
		return	nNum;
	}

	m_nHead		+= nNum * m_nUnitSize;
	// 注意：m_nHead和m_nTail都在m_nTotalSize的情况不会出现
	//       因为那属于全部释放的情况，在前面处理了
	if( m_nHead>=m_nTotalSize )
	{
		// 折回头
		m_nHead	-= m_nTotalSize;
	}

	m_nUnitNum	-= nNum;

	return	nNum;
}
int		whquickqueue::FreeLastN(int nNum)
{
	if( m_nUnitNum <= nNum )
	{
		// 这样就全部释放了啦
		nNum	= m_nUnitNum;
		Clean();
		return	nNum;
	}

	m_nTail		-= nNum * m_nUnitSize;
	if( m_nTail<=0 )
	{
		// 折回头
		m_nTail	+= m_nTotalSize;
	}
	m_nUnitNum	-= nNum;

	return	nNum;
}
int		whquickqueue::BeginGet()
{
	m_nCurPos	= m_nHead;
	m_nCount	= m_nUnitNum;
	return	0;
}
void *	whquickqueue::GetNext(int *pnSize)
{
	if( m_nCount == 0 )
	{
		// 到了尽头
		return	NULL;
	}
	m_nCount	--;

	// 获得本位置的数据
	char	*pRst = m_pBuf + m_nCurPos;
	if( pnSize )
	{
		*pnSize	= *(int *)pRst;
	}
	pRst	+= sizeof(int);

	// 获得下一个位置
	m_nCurPos	+= m_nUnitSize;
	if( m_nCurPos == m_nTotalSize )
	{
		if( m_nTail != m_nTotalSize )
		{
			// 只要m_nTail不指向总缓冲区的下一个字节(即回绕位置)，m_nCurPos就要回绕
			m_nCurPos	= 0;
		}
	}

	return	pRst;
}

////////////////////////////////////////////////////////////////////
// 高级队列
////////////////////////////////////////////////////////////////////
whadvqueue::whadvqueue()
: m_paIdx(NULL)
, m_pData(NULL)
, m_nMaxNum(0)
, m_nMaxSize(0)
, m_nHead(0)
, m_nToRead(0)
, m_nTail(0)
, m_nToWrite(0)
, m_nTotalNum(0)
{
	// 2005-07-05 还没有想清楚。暂时搁置吧。:(
	assert(0);
}
whadvqueue::~whadvqueue()
{
	Release();
}
int		whadvqueue::Init(int nMaxNum, int nMaxSize)
{
	assert( m_paIdx == NULL );
	assert( m_pData == NULL );

	m_paIdx	= new IDX_T[nMaxNum];
	if( !m_paIdx )
	{
		assert(0);
		return	-1;
	}
	m_pData	= new char[nMaxSize];
	if( !m_pData )
	{
		assert(0);
		return	-2;
	}

	m_nMaxNum	= nMaxNum;
	m_nMaxSize	= nMaxSize;

	Clean();

	return	0;
}
int		whadvqueue::Release()
{
	if( m_paIdx )
	{
		delete []	m_paIdx;
		m_paIdx	= NULL;
	}
	if( m_pData )
	{
		delete []	m_pData;
		m_pData	= NULL;
	}
	return	0;
}
void	whadvqueue::Clean()
{
	m_nHead		= 0;
	m_nToRead	= 0;
	m_nTail		= 0;
	m_nToWrite	= 0;
	m_nTotalNum	= 0;
}
void *	whadvqueue::InAlloc(int nSize)
{
	// 判断是否可以插入
	// 个数是否超过
	int	nIdx	= idxpp(m_nToWrite);
	if( nIdx == m_nHead )
	{
		return	NULL;
	}
	// 是否还有空间插入余地

	return	NULL;
}
