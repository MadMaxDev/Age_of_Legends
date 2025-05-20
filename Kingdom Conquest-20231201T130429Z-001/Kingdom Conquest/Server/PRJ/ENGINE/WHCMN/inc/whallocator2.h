// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whallocator2.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于多个大块内存的有限尺寸个数的分配器
//              : 目前按照4字节对齐，且地址为32位，的标准来做
// CreationDate : 2005-12-30
// ChangeLOG    : 2006-01-11 取消FIRSTHDR_T，原来这个只是为了可以继承旧共享内存做的，现在继承在更上层做，而且继承的内存在使用完毕后就可以初始化了，旧数据无需保存。

#ifndef	__WHALLOCATOR2_H__
#define	__WHALLOCATOR2_H__

#include "whallocator.h"
#include "whhash2.h"
#include "whdbg.h"

namespace	n_whcmn
{

// 只在一块大内存中分配的分配器
template<class _HashFunc=_whcmn_hashfunc>
class	whallocationobjwithonemem
{
private:
	enum
	{
		UNITSTEP	= 0x10,
		UNITMASK	= UNITSTEP-1,
	};
	// 一个分配单元的头部
	struct	UNITHDR_T
	{
		UNITHDR_T	*pPrev;
		UNITHDR_T	*pNext;
		short		nSizeUnitIdx;				// 对应的尺寸列表对象的序号
		bool		bAllocated:1;
		unsigned short		nReserved:11;
		unsigned short		nMinusSize:4;		// 实际尺寸减去这个就是逻辑尺寸（这个要配合前面的UNITSTEP）
		UNITHDR_T()
		: pPrev(NULL), pNext(NULL), nSizeUnitIdx(0), nReserved(0)
		{
		}
		void	clear()
		{
			pPrev	= NULL;
			pNext	= NULL;
			nSizeUnitIdx	= 0;
#ifdef	_DEBUG
			bAllocated		= false;
#endif
			nMinusSize		= 0;
			nReserved		= 0;
		}
		// 下面两个Insert要注意：
		// InsertAfter不能用于tail
		void	InsertAfter(UNITHDR_T *pOther)
		{
			pPrev	= pOther;
			pNext	= pOther->pNext;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// InsertBefore不能用于head
		void	InsertBefore(UNITHDR_T *pOther)
		{
			pPrev	= pOther->pPrev;
			pNext	= pOther;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// 从队列中退出
		void	Quit()
		{
			pPrev->pNext	= pNext;
			pNext->pPrev	= pPrev;
		}
		// 和一个尾(tail)对象连接
		void	InitHeadTail(UNITHDR_T *pTail, short nIdx)
		{
			nSizeUnitIdx	= nIdx;
			pPrev			= NULL;
			pNext			= pTail;
			pTail->pPrev	= this;
			pTail->pNext	= NULL;
		}
	};
	// 一个尺寸列表
	struct	SIZELIST_T
	{
		size_t		nSize;						// 对应的分配尺寸
		UNITHDR_T	avail_head, avail_tail;
		UNITHDR_T	alloc_head, alloc_tail;
		void	clear(short nIdx, size_t nS)
		{
			nSize	= nS;
			avail_head.InitHeadTail(&avail_tail, nIdx);
			alloc_head.InitHeadTail(&alloc_tail, nIdx);
		}
	};
	typedef	whhash2<size_t, SIZELIST_T *, _HashFunc>	HASH2;
	typedef	typename HASH2::iterator					HASH2IT;
	HASH2			m_mapSize2List;
	char			*m_pszChunk;				// 对应的内存指针
	size_t			m_nChunkSize;				// 大内存块的大小
	int				m_nOffset;					// 当前已经用到的位置
	SIZELIST_T		*m_sl;						// 各个尺寸的列表对象数组
	int				m_MAXSIZENUM;				// 最多的尺寸数量
	int				m_nSizeNum;					// 现在已经分配过的尺寸的个数
private:
	inline void *	GetCurPtr()
	{
		return	m_pszChunk + m_nOffset;
	}
public:
	struct	STAT_T
	{
		int			nSizeNum;					// 最多的尺寸数量
		int			nOffset;					// 当前已经用到的位置
		STAT_T()
			: nSizeNum(0)
			, nOffset(0)
		{
		}
	};
	inline void		GetStat(STAT_T *pStat) const
	{
		pStat->nSizeNum	= m_nSizeNum;
		pStat->nOffset	= m_nOffset;
	}
	inline size_t	GetChunkSize() const
	{
		return	m_nChunkSize;
	}
public:
	whallocationobjwithonemem()
	: 
	  m_pszChunk(NULL), m_nChunkSize(0), m_nOffset(0)
	, m_sl(NULL), m_MAXSIZENUM(0), m_nSizeNum(0)
	{
	}
	~whallocationobjwithonemem()
	{
		Release();
	}
	// _HASHSTEP1, _HASHSTEP2必须是质数，取值请参考whhash.h中的g_prime_list
	int		Init(void *pChunk, size_t nChunkSize, int _MAXSIZENUM=200, int _HASHSTEP1=1543, int _HASHSTEP2=97)
	{
		//
		m_pszChunk		= (char *)pChunk;
		m_nChunkSize	= nChunkSize;
		// 用内存的前边部分存放尺寸表
		int	nTotalSize	= sizeof(SIZELIST_T)*_MAXSIZENUM;
		m_sl			= (SIZELIST_T *)GetCurPtr();
		m_nOffset		+= nTotalSize;
		m_MAXSIZENUM	= _MAXSIZENUM;
		m_nSizeNum		= 0;
		// 接着存放尺寸对应尺寸对象的hash表
		if( m_mapSize2List.Init(GetCurPtr(), _HASHSTEP1, _HASHSTEP2, _MAXSIZENUM)<0 )
		{
			return	-11;
		}
		m_nOffset		+= m_mapSize2List.GetUnitMemSize(_HASHSTEP1);
		return	0;
	}
	int		Release()
	{
		if( m_pszChunk==NULL )
		{
			// 说明已经释放过了
			return	0;
		}
		// 释放尺寸对对象列表
		m_mapSize2List.Release();
		// 指针清空
		m_pszChunk		= NULL;
		return	0;
	}
	void *	Realloc(void *pBuf, size_t nSize)
	{
		size_t	nOldSize	= GetRawMemSize(pBuf);
		if( nOldSize>=nSize )
		{
			// 这个就不用申请了
			return	pBuf;
		}
		void	*ptr	= Alloc(nSize);
		if( nOldSize>0 )
		{
			memcpy(ptr, pBuf, nOldSize);
		}
		// 释放旧的ptr
		Free(pBuf);
		// 返回新指针
		return	ptr;
	}
	void *	Alloc(size_t nSize)
	{
		// 这个4是我自己定的，一个chunk如果不能至少分配4个unit就太没劲了吧
		assert( nSize < m_nChunkSize/4 );
		// 先把尺寸变成UNITSTEP的倍数
		short	nExtSize	= nSize%UNITSTEP;
		if( (nSize&UNITMASK) != 0 )
		{
			nSize	&= 0xFFFFFFFF^UNITMASK;
			nSize	+= UNITSTEP;
		}
		SIZELIST_T	*pSL	= NULL;
		short		nIdx	= 0;
		// 查找这个尺寸是否已经分配过了
		HASH2IT	it	= m_mapSize2List.find(nSize);
		if( it == m_mapSize2List.end() )
		{
			// 原来没有，创建新的
			if( m_nSizeNum >= m_MAXSIZENUM )
			{
				// 没有记录新尺寸的地方了
				assert(0);
				return	NULL;
			}
			pSL		= &m_sl[m_nSizeNum];
			pSL->clear(m_nSizeNum, nSize);
			it		= m_mapSize2List.put(nSize, pSL);
			nIdx	= (short)m_nSizeNum;
			m_nSizeNum	++;
		}
		else
		{
			// 原来有，直接用
			pSL	= (*it).GetVal();
			nIdx	= (short)(pSL-m_sl);
		}
		// 从这个列表中分配
		UNITHDR_T	*pUnit	= NULL;
		// 检查列表中是否还有可用的
		if( pSL->avail_head.pNext != &pSL->avail_tail )
		{
			// 可用就直接用
			pUnit	= pSL->avail_head.pNext;
			// 从avail列表中删除
			pUnit->Quit();
			// 加入分配列表
			pUnit->InsertAfter(&pSL->alloc_head);
		}
		else
		{
			// 否则就分配新的
			size_t	nTotalSize	= sizeof(UNITHDR_T) + nSize;
			if( m_nOffset+nTotalSize > m_nChunkSize )
			{
				// 不能再分配了
				return	NULL;
			}
			// 从当前chunk的后部开始分配
			pUnit	= (UNITHDR_T *)GetCurPtr();
			pUnit->clear();
			pUnit->nSizeUnitIdx		= nIdx;
			// 加入分配列表
			pUnit->InsertAfter(&pSL->alloc_head);
			// 偏移向后走
			m_nOffset	+= nTotalSize;
		}
#ifdef	_DEBUG
		pUnit->bAllocated	= true;
#endif
		if( nExtSize==0 )
		{
			pUnit->nMinusSize	= 0;
		}
		else
		{
			pUnit->nMinusSize	= UNITSTEP - nExtSize;
		}
		// 返回数据部分的指针
		return	pUnit+1;
	}
	void	Free(void *pBuf)
	{
		if( pBuf == NULL )
		{
			return;
		}
		// 通过指针找到UNIT头部
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)pBuf) - sizeof(UNITHDR_T));
#ifdef	_DEBUG
		if( !pUnit->bAllocated )
		{
			// 不应该被释放的指针（很可能是释放了两次）
			assert(0);
			return;
		}
		pUnit->bAllocated	= false;
#endif
		// 从alloc队列中退出
		pUnit->Quit();
		// 找到对应的尺寸列表。进入avail队列。
		assert(pUnit->nSizeUnitIdx>=0 && pUnit->nSizeUnitIdx<m_nSizeNum);
		pUnit->InsertAfter(&m_sl[pUnit->nSizeUnitIdx].avail_head);
	}
	// 获得指针对应内存的大小
	size_t	GetRawMemSize(void *ptr)
	{
		if( ptr==NULL )
		{
			return	0;
		}
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)ptr) - sizeof(UNITHDR_T));
#ifdef	_DEBUG
		assert( pUnit->bAllocated );
#endif
		return	m_sl[pUnit->nSizeUnitIdx].nSize;
	}
	// 获得指针对应的逻辑内存大小
	size_t	GetLogicMemSize(void *ptr)
	{
		if( ptr==NULL )
		{
			return	0;
		}
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)ptr) - sizeof(UNITHDR_T));
#ifdef	_DEBUG
		assert( pUnit->bAllocated );
#endif
		return	m_sl[pUnit->nSizeUnitIdx].nSize - pUnit->nMinusSize;
	}
	// 输出leak的统计
	void	LeakReport(FILE *perrout)
	{
		// 浏览所有的sizeunit，输出没有被释放的单元
		for(int i=0;i<m_nSizeNum;i++)
		{
			bool		bFirst	= true;
			SIZELIST_T	*pSL	= &m_sl[i];
			UNITHDR_T	*pUnit	= pSL->alloc_head.pNext;
			while( pUnit != &pSL->alloc_tail )
			{
				if( bFirst )
				{
					bFirst		= false;
					fprintf(perrout, "Dumping SIZE:%d\n", pSL->nSize);
				}
				fprintf(perrout, "%p:\n", pUnit+1);
				dbg_printmem(pUnit+1, pSL->nSize, perrout);
				pUnit	= pUnit->pNext;
			}
		}
	}
	// 内部指针调整
	void	AdjustInnerPtr(int nOffset)
	{
		wh_adjustaddrbyoffset(m_pszChunk, nOffset);
		wh_adjustaddrbyoffset(m_sl, nOffset);
		m_mapSize2List.AdjustInnerPtr(nOffset);
		HASH2IT	it	= m_mapSize2List.begin();
		while( it!=m_mapSize2List.end() )
		{
			SIZELIST_T	*&pSL	= (*it).GetVal();
			wh_adjustaddrbyoffset(pSL, nOffset);
			++it;
		}
		// 调整所有尺寸的已分配单元和可用单元链表的内部指针
		for(int i=0;i<m_nSizeNum;i++)
		{
			SIZELIST_T	*pSL	= &m_sl[i];
			wh_adjustaddrbyoffset(pSL->avail_head.pNext, nOffset);
			wh_adjustaddrbyoffset(pSL->avail_tail.pPrev, nOffset);
			wh_adjustaddrbyoffset(pSL->alloc_head.pNext, nOffset);
			wh_adjustaddrbyoffset(pSL->alloc_tail.pPrev, nOffset);

			UNITHDR_T	*pUnit	= pSL->alloc_head.pNext;
			while( pUnit != &pSL->alloc_tail )
			{
				wh_adjustaddrbyoffset(pUnit->pNext, nOffset);
				wh_adjustaddrbyoffset(pUnit->pPrev, nOffset);
				pUnit	= pUnit->pNext;
			}
			pUnit	= pSL->avail_head.pNext;
			while( pUnit != &pSL->avail_tail )
			{
				wh_adjustaddrbyoffset(pUnit->pNext, nOffset);
				wh_adjustaddrbyoffset(pUnit->pPrev, nOffset);
				pUnit	= pUnit->pNext;
			}
		}
	}
};

// 每次分配一定大小的快，所有小尺寸都在大块中分配，释放的指针都放在链表中，下次使用相同尺寸的直接使用
template<class _AllocationObj, class _HashFunc=_whcmn_hashfunc>
class	whchunkallocationobj
{
private:
	enum
	{
		UNITSTEP	= 0x10,
		UNITMASK	= UNITSTEP-1,
	};
	// 一个分配单元的头部
	struct	UNITHDR_T
	{
		UNITHDR_T	*pPrev;
		UNITHDR_T	*pNext;
		short		nSizeUnitIdx;				// 对应的尺寸列表对象的序号
		bool		bAllocated:1;
		short		nReserved:12;
		short		nMinusSize:3;				// 实际尺寸减去这个就是逻辑尺寸
		UNITHDR_T()
		: pPrev(NULL), pNext(NULL), nSizeUnitIdx(0), nReserved(0)
		{
		}
		void	clear()
		{
			pPrev	= NULL;
			pNext	= NULL;
			nSizeUnitIdx	= 0;
#ifdef	_DEBUG
			bAllocated		= false;
#endif
			nMinusSize		= 0;
			nReserved		= 0;
		}
		// 下面两个Insert要注意：
		// InsertAfter不能用于tail
		void	InsertAfter(UNITHDR_T *pOther)
		{
			pPrev	= pOther;
			pNext	= pOther->pNext;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// InsertBefore不能用于head
		void	InsertBefore(UNITHDR_T *pOther)
		{
			pPrev	= pOther->pPrev;
			pNext	= pOther;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// 从队列中退出
		void	Quit()
		{
			pPrev->pNext	= pNext;
			pNext->pPrev	= pPrev;
		}
		// 和一个尾(tail)对象连接
		void	InitHeadTail(UNITHDR_T *pTail, short nIdx)
		{
			nSizeUnitIdx	= nIdx;
			pPrev			= NULL;
			pNext			= pTail;
			pTail->pPrev	= this;
			pTail->pNext	= NULL;
		}
	};
	// 一个尺寸列表
	struct	SIZELIST_T
	{
		size_t		nSize;						// 对应的分配尺寸
		UNITHDR_T	avail_head, avail_tail;
		UNITHDR_T	alloc_head, alloc_tail;
		void	clear(short nIdx, size_t nS)
		{
			nSize	= nS;
			avail_head.InitHeadTail(&avail_tail, nIdx);
			alloc_head.InitHeadTail(&alloc_tail, nIdx);
		}
	};
	// 普通大块的头部
	struct	CHUNKHDR_T
	{
		CHUNKHDR_T	*pNextChunk;				// 下一块的地址
		size_t		nChunkSize;					// 块的总大小
		int			nOffset;					// 可分配的开始位置
		void	clear()
		{
			pNextChunk	= NULL;
			nChunkSize	= 0;
			nOffset		= sizeof(CHUNKHDR_T);
		}
		inline int	GetAvailSize() const		// 获得本块chunk的可用尺寸
		{
			return	(int)(nChunkSize - nOffset);
		}
	};
	typedef	whhash2<size_t, SIZELIST_T *, _HashFunc>	HASH2;
	typedef	typename HASH2::iterator					HASH2IT;
	HASH2			m_mapSize2List;
	_AllocationObj	*m_pAllocationObj;
	size_t			m_nChunkSize;
	CHUNKHDR_T		*m_pFirstChunk;
	CHUNKHDR_T		*m_pCurChunk;
	int				m_nSizeNum;					// 现在已经分配过的尺寸的个数
	SIZELIST_T		*m_sl;						// 各个尺寸的列表对象数组
	int				m_MAXSIZENUM;				// 最多的尺寸数量
private:
	void *	GetCurPtr()
	{
		return	((char *)m_pCurChunk)+m_pCurChunk->nOffset;
	}
public:
	whchunkallocationobj()
	: 
	  m_pAllocationObj(NULL), m_nChunkSize(0)
	, m_pFirstChunk(NULL), m_pCurChunk(NULL)
	, m_nSizeNum(0), m_sl(NULL), m_MAXSIZENUM(0)
	{
	}
	~whchunkallocationobj()
	{
		Release();
	}
	// _HASHSTEP1, _HASHSTEP2必须是质数，取值请参考whhash.h中的g_prime_list
	int		Init(_AllocationObj *pAllocationObj, size_t nChunkSize, int _MAXSIZENUM=200, int _HASHSTEP1=1543, int _HASHSTEP2=97)
	{
		if( m_pFirstChunk )
		{
			assert(0);
			return	-3;
		}
		// 保证_MAXSIZENUM不能超过65535
		if( _MAXSIZENUM>0xFFFF )
		{
			assert(0);
			return	-4;
		}
		//
		m_pAllocationObj	= pAllocationObj;
		m_nChunkSize	= nChunkSize;
		// 申请第一块，然后做一些初始化工作
		m_pFirstChunk	= (CHUNKHDR_T *)m_pAllocationObj->Alloc(m_nChunkSize);
		m_pFirstChunk->clear();
		m_pFirstChunk->nChunkSize	= nChunkSize;
		m_pCurChunk		= m_pFirstChunk;
		//
		int	nTotalSize	= sizeof(SIZELIST_T)*_MAXSIZENUM;
		m_sl			= (SIZELIST_T *)GetCurPtr();
		m_pCurChunk->nOffset	+= nTotalSize;
		m_MAXSIZENUM	= _MAXSIZENUM;
		m_nSizeNum		= 0;
		//
		if( m_mapSize2List.Init(GetCurPtr(), _HASHSTEP1, _HASHSTEP2, _MAXSIZENUM)<0 )
		{
			return	-11;
		}
		m_pCurChunk->nOffset	+= m_mapSize2List.GetUnitMemSize(_HASHSTEP1);
		return	0;
	}
	int		Release()
	{
		if( m_pFirstChunk == NULL )
		{
			// 已经被release过了，或者根本没有初始化
			return	0;
		}
		// 先释放除FirstChunk以外的所有从m_pAllocationObj申请的块
		CHUNKHDR_T	*pChunk	= m_pFirstChunk->pNextChunk;
		while( pChunk )
		{
			CHUNKHDR_T	*pChunkToRelease	= pChunk;
			pChunk	= pChunk->pNextChunk;
			m_pAllocationObj->Free(pChunkToRelease);
		}
		// 再释放FirstChunk
		m_pAllocationObj->Free(m_pFirstChunk);
		m_pFirstChunk	= NULL;
		m_pCurChunk		= NULL;
		// 释放尺寸对对象列表
		m_mapSize2List.Release();
		return	0;
	}
	void *	Realloc(void *pBuf, size_t nSize)
	{
		size_t	nOldSize	= GetRawMemSize(pBuf);
		if( nOldSize>=nSize )
		{
			// 这个就不用申请了
			return	pBuf;
		}
		void	*ptr	= Alloc(nSize);
		if( nOldSize>0 )
		{
			memcpy(ptr, pBuf, nOldSize);
		}
		// 释放旧的ptr
		Free(pBuf);
		// 返回新指针
		return	ptr;
	}
	void *	Alloc(size_t nSize)
	{
		// 这个4是我自己定的，一个chunk如果不能至少分配4个unit就太没劲了吧
		assert( nSize < m_nChunkSize/4 );
		// 先把尺寸变成4的倍数
		short	nExtSize	= nSize%UNITSTEP;
		if( (nSize&UNITMASK) != 0 )
		{
			nSize	&= 0xFFFFFFFF^UNITMASK;
			nSize	+= UNITSTEP;
		}
		SIZELIST_T	*pSL	= NULL;
		short		nIdx	= 0;
		// 查找这个尺寸是否已经分配过了
		HASH2IT	it	= m_mapSize2List.find(nSize);
		if( it == m_mapSize2List.end() )
		{
			// 原来没有，创建新的
			if( m_nSizeNum >= m_MAXSIZENUM )
			{
				// 没有记录新尺寸的地方了
				assert(0);
				return	NULL;
			}
			pSL		= &m_sl[m_nSizeNum];
			pSL->clear(m_nSizeNum, nSize);
			it		= m_mapSize2List.put(nSize, pSL);
			nIdx	= (short)m_nSizeNum;
			m_nSizeNum	++;
		}
		else
		{
			// 原来有，直接用
			pSL	= (*it).GetVal();
			nIdx	= (short)(pSL-m_sl);
		}
		// 从这个列表中分配
		UNITHDR_T	*pUnit	= NULL;
		// 检查列表中是否还有可用的
		if( pSL->avail_head.pNext != &pSL->avail_tail )
		{
			// 可用就直接用
			pUnit	= pSL->avail_head.pNext;
			// 从avail列表中删除
			pUnit->Quit();
			// 加入分配列表
			pUnit->InsertAfter(&pSL->alloc_head);
		}
		else
		{
			// 否则就分配新的
			size_t	nTotalSize	= sizeof(UNITHDR_T) + nSize;
			if( m_pCurChunk->nOffset+nTotalSize > m_pCurChunk->nChunkSize )
			{
				// 应该再分配一块新的chunk了
				CHUNKHDR_T	*pNewChunk	= (CHUNKHDR_T *)m_pAllocationObj->Alloc(m_nChunkSize);
				if( pNewChunk == NULL )
				{
					assert(0);
					return	NULL;
				}
				pNewChunk->clear();
				pNewChunk->nChunkSize	= m_nChunkSize;
				pNewChunk->pNextChunk	= m_pFirstChunk->pNextChunk;
				m_pFirstChunk->pNextChunk	= pNewChunk;
				m_pCurChunk				= pNewChunk;
			}
			// 从当前chunk的后部开始分配
			pUnit	= (UNITHDR_T *)GetCurPtr();
			pUnit->clear();
			pUnit->nSizeUnitIdx		= nIdx;
			// 加入分配列表
			pUnit->InsertAfter(&pSL->alloc_head);
			// 偏移向后走
			m_pCurChunk->nOffset	+= nTotalSize;
		}
#ifdef	_DEBUG
		pUnit->bAllocated	= true;
#endif
		if( nExtSize==0 )
		{
			pUnit->nMinusSize	= 0;
		}
		else
		{
			pUnit->nMinusSize	= UNITSTEP - nExtSize;
		}
		// 返回数据部分的指针
		return	pUnit+1;
	}
	void	Free(void *pBuf)
	{
		if( pBuf == NULL )
		{
			return;
		}
		// 通过指针找到UNIT头部
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)pBuf) - sizeof(UNITHDR_T));
#ifdef	_DEBUG
		if( !pUnit->bAllocated )
		{
			// 不应该被释放的指针（很可能是释放了两次）
			assert(0);
			return;
		}
		pUnit->bAllocated	= false;
#endif
		// 从alloc队列中退出
		pUnit->Quit();
		// 找到对应的尺寸列表。进入avail队列。
		assert(pUnit->nSizeUnitIdx>=0 && pUnit->nSizeUnitIdx<m_nSizeNum);
		pUnit->InsertAfter(&m_sl[pUnit->nSizeUnitIdx].avail_head);
	}
	// 获得指针对应内存的大小
	size_t	GetRawMemSize(void *ptr)
	{
		if( ptr==NULL )
		{
			return	0;
		}
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)ptr) - sizeof(UNITHDR_T));
#ifdef	_DEBUG
		assert( pUnit->bAllocated );
#endif
		return	m_sl[pUnit->nSizeUnitIdx].nSize;
	}
	// 获得指针对应的逻辑内存大小
	size_t	GetLogicMemSize(void *ptr)
	{
		if( ptr==NULL )
		{
			return	0;
		}
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)ptr) - sizeof(UNITHDR_T));
#ifdef	_DEBUG
		assert( pUnit->bAllocated );
#endif
		return	m_sl[pUnit->nSizeUnitIdx].nSize - pUnit->nMinusSize;
	}
	// 获得当前块所剩余的可分配空间
	inline int		GetAvailSizeOfCurChunk() const
	{
		return	m_pCurChunk->GetAvailSize();
	}
	// 输出leak的统计
	void	LeakReport(FILE *perrout)
	{
		// 浏览所有的sizeunit，输出没有被释放的单元
		for(int i=0;i<m_nSizeNum;i++)
		{
			bool		bFirst	= true;
			SIZELIST_T	*pSL	= &m_sl[i];
			UNITHDR_T	*pUnit	= pSL->alloc_head.pNext;
			while( pUnit != &pSL->alloc_tail )
			{
				if( bFirst )
				{
					bFirst		= false;
					fprintf(perrout, "Dumping SIZE:%d\n", pSL->nSize);
				}
				fprintf(perrout, "%p:\n", pUnit+1);
				dbg_printmem(pUnit+1, pSL->nSize, perrout);
				pUnit	= pUnit->pNext;
			}
		}
	}
};

// 只针对一种尺寸的分配器（默认CHUNK尺寸4M）
template<class _AllocationObj>
class	whchunkallocationobjonesize
{
private:
	// 一个分配单元的头部
	struct	UNITHDR_T
	{
		UNITHDR_T	*pPrev;
		UNITHDR_T	*pNext;
		UNITHDR_T()
		: pPrev(NULL), pNext(NULL)
		{
		}
		void	clear()
		{
			pPrev	= NULL;
			pNext	= NULL;
		}
		// 下面两个Insert要注意：
		// InsertAfter不能用于tail
		void	InsertAfter(UNITHDR_T *pOther)
		{
			pPrev	= pOther;
			pNext	= pOther->pNext;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// InsertBefore不能用于head
		void	InsertBefore(UNITHDR_T *pOther)
		{
			pPrev	= pOther->pPrev;
			pNext	= pOther;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// 从队列中退出
		void	Quit()
		{
			pPrev->pNext	= pNext;
			pNext->pPrev	= pPrev;
		}
		// 和一个尾(tail)对象连接
		void	InitHeadTail(UNITHDR_T *pTail)
		{
			pPrev			= NULL;
			pNext			= pTail;
			pTail->pPrev	= this;
			pTail->pNext	= NULL;
		}
	};
	// 普通大块的头部
	struct	CHUNKHDR_T
	{
		CHUNKHDR_T	*pNextChunk;				// 下一块的地址
		void	clear()
		{
			pNextChunk	= NULL;
		}
	};
	_AllocationObj	*m_pAllocationObj;
	CHUNKHDR_T		*m_pFirstChunk;
	CHUNKHDR_T		*m_pCurChunk;
	char			*m_pCurUnitToAlloc;			// 当前可分配的对象指针（用char*是为了好增加偏移量）
	int				m_nCurUnitIdxToAlloc;		// 当前可以分配的对象的序号
	UNITHDR_T		avail_head, avail_tail;		// 可用列表
	UNITHDR_T		alloc_head, alloc_tail;		// 已分配列表
	// 数据部分尺寸
	int				m_DATASIZE;
	// 一整个分配单元尺寸
	int				m_UNITSIZE;
	// 一个chunk的尺寸
	int				m_CHUNKSIZE;
	// 一块chunk中的最多单元个数
	int				m_NUMINCHUNK;
public:
	whchunkallocationobjonesize()
	: m_pAllocationObj(NULL)
	, m_pFirstChunk(NULL), m_pCurChunk(NULL)
	, m_pCurUnitToAlloc(NULL), m_nCurUnitIdxToAlloc(0)
	, m_DATASIZE(0), m_UNITSIZE(0), m_CHUNKSIZE(0), m_NUMINCHUNK(0)
	{
	}
	~whchunkallocationobjonesize()
	{
		Release();
	}
	int		Init(_AllocationObj *pAllocationObj, int _SIZE, int _CHUNKSIZE=4*1024*1024)
	{
		if( m_pFirstChunk )
		{
			assert(0);
			return	-1;
		}

		m_CHUNKSIZE		= _CHUNKSIZE;
		m_DATASIZE		= (_SIZE/4)*4 + ((_SIZE%4)==0 ? 0 : 4);
		m_UNITSIZE		= sizeof(UNITHDR_T)+m_DATASIZE;
		m_NUMINCHUNK	= (_CHUNKSIZE - sizeof(CHUNKHDR_T)) / m_UNITSIZE,

		m_pAllocationObj	= pAllocationObj;
		// 初始化两个列表
		avail_head.InitHeadTail(&avail_tail);
		alloc_head.InitHeadTail(&alloc_tail);
		// 申请第一块，然后做一些初始化工作
		m_pFirstChunk	= (CHUNKHDR_T *)m_pAllocationObj->Alloc(_CHUNKSIZE);
		m_pFirstChunk->clear();
		m_pCurChunk		= m_pFirstChunk;
		// 初始化客分配的指针地址
		m_nCurUnitIdxToAlloc	= 0;
		m_pCurUnitToAlloc		= (char *)(m_pCurChunk+1);
		return	0;
	}
	int		Release()
	{
		if( m_pFirstChunk == NULL )
		{
			// 已经被release过了，或者根本没有初始化
			return	0;
		}
		// 先释放除FirstChunk以外的所有从m_pAllocationObj申请的块
		while( m_pFirstChunk )
		{
			CHUNKHDR_T	*pChunkToRelease	= m_pFirstChunk;
			m_pFirstChunk	= m_pFirstChunk->pNextChunk;
			m_pAllocationObj->Free(pChunkToRelease);
		}
		m_pFirstChunk	= NULL;
		return	0;
	}
	void *	Alloc()
	{
		// 从这个列表中分配
		UNITHDR_T	*pUnit	= NULL;
		// 检查列表中是否还有可用的
		if( avail_head.pNext != &avail_tail )
		{
			// 可用就直接用
			pUnit	= avail_head.pNext;
			// 从avail列表中删除
			pUnit->Quit();
			// 加入分配列表
			pUnit->InsertAfter(&alloc_head);
		}
		else
		{
			// 否则就分配新的
			if( m_nCurUnitIdxToAlloc >= m_NUMINCHUNK )
			{
				// 应该再分配一块新的chunk了
				CHUNKHDR_T	*pNewChunk	= (CHUNKHDR_T *)m_pAllocationObj->Alloc(m_CHUNKSIZE);
				if( pNewChunk == NULL )
				{
					assert(0);
					return	NULL;
				}
				pNewChunk->clear();
				pNewChunk->pNextChunk	= m_pFirstChunk->pNextChunk;
				m_pFirstChunk->pNextChunk	= pNewChunk;
				m_pCurChunk				= pNewChunk;
				m_nCurUnitIdxToAlloc	= 0;
				m_pCurUnitToAlloc		= (char *)(m_pCurChunk+1);
			}
			// 从当前chunk的后部开始分配
			pUnit	= (UNITHDR_T *)m_pCurUnitToAlloc;
			pUnit->clear();
			// 加入分配列表
			pUnit->InsertAfter(&alloc_head);
			// 偏移向后走
			m_pCurUnitToAlloc		+= m_UNITSIZE;
			m_nCurUnitIdxToAlloc	++;
		}
		// 返回数据部分的指针
		return	pUnit+1;
	}
	void	Free(void *pBuf)
	{
		if( pBuf == NULL )
		{
			return;
		}
		// 通过指针找到UNIT头部
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)pBuf) - sizeof(UNITHDR_T));
		// 从alloc队列中退出
		pUnit->Quit();
		pUnit->InsertAfter(&avail_head);
	}
	// 输出leak的统计
	void	LeakReport(FILE *perrout)
	{
		// 浏览所有的sizeunit，输出没有被释放的单元
		bool		bFirst	= true;
		UNITHDR_T	*pUnit	= alloc_head.pNext;
		while( pUnit != &alloc_tail )
		{
			if( bFirst )
			{
				bFirst		= false;
				fprintf(perrout, "Dumping SIZE:%d\n", m_DATASIZE);
			}
			fprintf(perrout, "%p:\n", pUnit+1);
			dbg_printmem(pUnit+1, m_DATASIZE, perrout);
			pUnit	= pUnit->pNext;
		}
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHALLOCATOR2_H__

