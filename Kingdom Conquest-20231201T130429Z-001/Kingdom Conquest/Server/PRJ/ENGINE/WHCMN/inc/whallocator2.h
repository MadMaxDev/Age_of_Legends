// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whallocator2.h
// Creator      : Wei Hua (κ��)
// Comment      : ���ڶ������ڴ�����޳ߴ�����ķ�����
//              : Ŀǰ����4�ֽڶ��룬�ҵ�ַΪ32λ���ı�׼����
// CreationDate : 2005-12-30
// ChangeLOG    : 2006-01-11 ȡ��FIRSTHDR_T��ԭ�����ֻ��Ϊ�˿��Լ̳оɹ����ڴ����ģ����ڼ̳��ڸ��ϲ��������Ҽ̳е��ڴ���ʹ����Ϻ�Ϳ��Գ�ʼ���ˣ����������豣�档

#ifndef	__WHALLOCATOR2_H__
#define	__WHALLOCATOR2_H__

#include "whallocator.h"
#include "whhash2.h"
#include "whdbg.h"

namespace	n_whcmn
{

// ֻ��һ����ڴ��з���ķ�����
template<class _HashFunc=_whcmn_hashfunc>
class	whallocationobjwithonemem
{
private:
	enum
	{
		UNITSTEP	= 0x10,
		UNITMASK	= UNITSTEP-1,
	};
	// һ�����䵥Ԫ��ͷ��
	struct	UNITHDR_T
	{
		UNITHDR_T	*pPrev;
		UNITHDR_T	*pNext;
		short		nSizeUnitIdx;				// ��Ӧ�ĳߴ��б��������
		bool		bAllocated:1;
		unsigned short		nReserved:11;
		unsigned short		nMinusSize:4;		// ʵ�ʳߴ��ȥ��������߼��ߴ磨���Ҫ���ǰ���UNITSTEP��
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
		// ��������InsertҪע�⣺
		// InsertAfter��������tail
		void	InsertAfter(UNITHDR_T *pOther)
		{
			pPrev	= pOther;
			pNext	= pOther->pNext;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// InsertBefore��������head
		void	InsertBefore(UNITHDR_T *pOther)
		{
			pPrev	= pOther->pPrev;
			pNext	= pOther;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// �Ӷ������˳�
		void	Quit()
		{
			pPrev->pNext	= pNext;
			pNext->pPrev	= pPrev;
		}
		// ��һ��β(tail)��������
		void	InitHeadTail(UNITHDR_T *pTail, short nIdx)
		{
			nSizeUnitIdx	= nIdx;
			pPrev			= NULL;
			pNext			= pTail;
			pTail->pPrev	= this;
			pTail->pNext	= NULL;
		}
	};
	// һ���ߴ��б�
	struct	SIZELIST_T
	{
		size_t		nSize;						// ��Ӧ�ķ���ߴ�
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
	char			*m_pszChunk;				// ��Ӧ���ڴ�ָ��
	size_t			m_nChunkSize;				// ���ڴ��Ĵ�С
	int				m_nOffset;					// ��ǰ�Ѿ��õ���λ��
	SIZELIST_T		*m_sl;						// �����ߴ���б��������
	int				m_MAXSIZENUM;				// ���ĳߴ�����
	int				m_nSizeNum;					// �����Ѿ�������ĳߴ�ĸ���
private:
	inline void *	GetCurPtr()
	{
		return	m_pszChunk + m_nOffset;
	}
public:
	struct	STAT_T
	{
		int			nSizeNum;					// ���ĳߴ�����
		int			nOffset;					// ��ǰ�Ѿ��õ���λ��
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
	// _HASHSTEP1, _HASHSTEP2������������ȡֵ��ο�whhash.h�е�g_prime_list
	int		Init(void *pChunk, size_t nChunkSize, int _MAXSIZENUM=200, int _HASHSTEP1=1543, int _HASHSTEP2=97)
	{
		//
		m_pszChunk		= (char *)pChunk;
		m_nChunkSize	= nChunkSize;
		// ���ڴ��ǰ�߲��ִ�ųߴ��
		int	nTotalSize	= sizeof(SIZELIST_T)*_MAXSIZENUM;
		m_sl			= (SIZELIST_T *)GetCurPtr();
		m_nOffset		+= nTotalSize;
		m_MAXSIZENUM	= _MAXSIZENUM;
		m_nSizeNum		= 0;
		// ���Ŵ�ųߴ��Ӧ�ߴ�����hash��
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
			// ˵���Ѿ��ͷŹ���
			return	0;
		}
		// �ͷųߴ�Զ����б�
		m_mapSize2List.Release();
		// ָ�����
		m_pszChunk		= NULL;
		return	0;
	}
	void *	Realloc(void *pBuf, size_t nSize)
	{
		size_t	nOldSize	= GetRawMemSize(pBuf);
		if( nOldSize>=nSize )
		{
			// ����Ͳ���������
			return	pBuf;
		}
		void	*ptr	= Alloc(nSize);
		if( nOldSize>0 )
		{
			memcpy(ptr, pBuf, nOldSize);
		}
		// �ͷžɵ�ptr
		Free(pBuf);
		// ������ָ��
		return	ptr;
	}
	void *	Alloc(size_t nSize)
	{
		// ���4�����Լ����ģ�һ��chunk����������ٷ���4��unit��̫û���˰�
		assert( nSize < m_nChunkSize/4 );
		// �Ȱѳߴ���UNITSTEP�ı���
		short	nExtSize	= nSize%UNITSTEP;
		if( (nSize&UNITMASK) != 0 )
		{
			nSize	&= 0xFFFFFFFF^UNITMASK;
			nSize	+= UNITSTEP;
		}
		SIZELIST_T	*pSL	= NULL;
		short		nIdx	= 0;
		// ��������ߴ��Ƿ��Ѿ��������
		HASH2IT	it	= m_mapSize2List.find(nSize);
		if( it == m_mapSize2List.end() )
		{
			// ԭ��û�У������µ�
			if( m_nSizeNum >= m_MAXSIZENUM )
			{
				// û�м�¼�³ߴ�ĵط���
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
			// ԭ���У�ֱ����
			pSL	= (*it).GetVal();
			nIdx	= (short)(pSL-m_sl);
		}
		// ������б��з���
		UNITHDR_T	*pUnit	= NULL;
		// ����б����Ƿ��п��õ�
		if( pSL->avail_head.pNext != &pSL->avail_tail )
		{
			// ���þ�ֱ����
			pUnit	= pSL->avail_head.pNext;
			// ��avail�б���ɾ��
			pUnit->Quit();
			// ��������б�
			pUnit->InsertAfter(&pSL->alloc_head);
		}
		else
		{
			// ����ͷ����µ�
			size_t	nTotalSize	= sizeof(UNITHDR_T) + nSize;
			if( m_nOffset+nTotalSize > m_nChunkSize )
			{
				// �����ٷ�����
				return	NULL;
			}
			// �ӵ�ǰchunk�ĺ󲿿�ʼ����
			pUnit	= (UNITHDR_T *)GetCurPtr();
			pUnit->clear();
			pUnit->nSizeUnitIdx		= nIdx;
			// ��������б�
			pUnit->InsertAfter(&pSL->alloc_head);
			// ƫ�������
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
		// �������ݲ��ֵ�ָ��
		return	pUnit+1;
	}
	void	Free(void *pBuf)
	{
		if( pBuf == NULL )
		{
			return;
		}
		// ͨ��ָ���ҵ�UNITͷ��
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)pBuf) - sizeof(UNITHDR_T));
#ifdef	_DEBUG
		if( !pUnit->bAllocated )
		{
			// ��Ӧ�ñ��ͷŵ�ָ�루�ܿ������ͷ������Σ�
			assert(0);
			return;
		}
		pUnit->bAllocated	= false;
#endif
		// ��alloc�������˳�
		pUnit->Quit();
		// �ҵ���Ӧ�ĳߴ��б�����avail���С�
		assert(pUnit->nSizeUnitIdx>=0 && pUnit->nSizeUnitIdx<m_nSizeNum);
		pUnit->InsertAfter(&m_sl[pUnit->nSizeUnitIdx].avail_head);
	}
	// ���ָ���Ӧ�ڴ�Ĵ�С
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
	// ���ָ���Ӧ���߼��ڴ��С
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
	// ���leak��ͳ��
	void	LeakReport(FILE *perrout)
	{
		// ������е�sizeunit�����û�б��ͷŵĵ�Ԫ
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
	// �ڲ�ָ�����
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
		// �������гߴ���ѷ��䵥Ԫ�Ϳ��õ�Ԫ������ڲ�ָ��
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

// ÿ�η���һ����С�Ŀ죬����С�ߴ綼�ڴ���з��䣬�ͷŵ�ָ�붼���������У��´�ʹ����ͬ�ߴ��ֱ��ʹ��
template<class _AllocationObj, class _HashFunc=_whcmn_hashfunc>
class	whchunkallocationobj
{
private:
	enum
	{
		UNITSTEP	= 0x10,
		UNITMASK	= UNITSTEP-1,
	};
	// һ�����䵥Ԫ��ͷ��
	struct	UNITHDR_T
	{
		UNITHDR_T	*pPrev;
		UNITHDR_T	*pNext;
		short		nSizeUnitIdx;				// ��Ӧ�ĳߴ��б��������
		bool		bAllocated:1;
		short		nReserved:12;
		short		nMinusSize:3;				// ʵ�ʳߴ��ȥ��������߼��ߴ�
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
		// ��������InsertҪע�⣺
		// InsertAfter��������tail
		void	InsertAfter(UNITHDR_T *pOther)
		{
			pPrev	= pOther;
			pNext	= pOther->pNext;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// InsertBefore��������head
		void	InsertBefore(UNITHDR_T *pOther)
		{
			pPrev	= pOther->pPrev;
			pNext	= pOther;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// �Ӷ������˳�
		void	Quit()
		{
			pPrev->pNext	= pNext;
			pNext->pPrev	= pPrev;
		}
		// ��һ��β(tail)��������
		void	InitHeadTail(UNITHDR_T *pTail, short nIdx)
		{
			nSizeUnitIdx	= nIdx;
			pPrev			= NULL;
			pNext			= pTail;
			pTail->pPrev	= this;
			pTail->pNext	= NULL;
		}
	};
	// һ���ߴ��б�
	struct	SIZELIST_T
	{
		size_t		nSize;						// ��Ӧ�ķ���ߴ�
		UNITHDR_T	avail_head, avail_tail;
		UNITHDR_T	alloc_head, alloc_tail;
		void	clear(short nIdx, size_t nS)
		{
			nSize	= nS;
			avail_head.InitHeadTail(&avail_tail, nIdx);
			alloc_head.InitHeadTail(&alloc_tail, nIdx);
		}
	};
	// ��ͨ����ͷ��
	struct	CHUNKHDR_T
	{
		CHUNKHDR_T	*pNextChunk;				// ��һ��ĵ�ַ
		size_t		nChunkSize;					// ����ܴ�С
		int			nOffset;					// �ɷ���Ŀ�ʼλ��
		void	clear()
		{
			pNextChunk	= NULL;
			nChunkSize	= 0;
			nOffset		= sizeof(CHUNKHDR_T);
		}
		inline int	GetAvailSize() const		// ��ñ���chunk�Ŀ��óߴ�
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
	int				m_nSizeNum;					// �����Ѿ�������ĳߴ�ĸ���
	SIZELIST_T		*m_sl;						// �����ߴ���б��������
	int				m_MAXSIZENUM;				// ���ĳߴ�����
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
	// _HASHSTEP1, _HASHSTEP2������������ȡֵ��ο�whhash.h�е�g_prime_list
	int		Init(_AllocationObj *pAllocationObj, size_t nChunkSize, int _MAXSIZENUM=200, int _HASHSTEP1=1543, int _HASHSTEP2=97)
	{
		if( m_pFirstChunk )
		{
			assert(0);
			return	-3;
		}
		// ��֤_MAXSIZENUM���ܳ���65535
		if( _MAXSIZENUM>0xFFFF )
		{
			assert(0);
			return	-4;
		}
		//
		m_pAllocationObj	= pAllocationObj;
		m_nChunkSize	= nChunkSize;
		// �����һ�飬Ȼ����һЩ��ʼ������
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
			// �Ѿ���release���ˣ����߸���û�г�ʼ��
			return	0;
		}
		// ���ͷų�FirstChunk��������д�m_pAllocationObj����Ŀ�
		CHUNKHDR_T	*pChunk	= m_pFirstChunk->pNextChunk;
		while( pChunk )
		{
			CHUNKHDR_T	*pChunkToRelease	= pChunk;
			pChunk	= pChunk->pNextChunk;
			m_pAllocationObj->Free(pChunkToRelease);
		}
		// ���ͷ�FirstChunk
		m_pAllocationObj->Free(m_pFirstChunk);
		m_pFirstChunk	= NULL;
		m_pCurChunk		= NULL;
		// �ͷųߴ�Զ����б�
		m_mapSize2List.Release();
		return	0;
	}
	void *	Realloc(void *pBuf, size_t nSize)
	{
		size_t	nOldSize	= GetRawMemSize(pBuf);
		if( nOldSize>=nSize )
		{
			// ����Ͳ���������
			return	pBuf;
		}
		void	*ptr	= Alloc(nSize);
		if( nOldSize>0 )
		{
			memcpy(ptr, pBuf, nOldSize);
		}
		// �ͷžɵ�ptr
		Free(pBuf);
		// ������ָ��
		return	ptr;
	}
	void *	Alloc(size_t nSize)
	{
		// ���4�����Լ����ģ�һ��chunk����������ٷ���4��unit��̫û���˰�
		assert( nSize < m_nChunkSize/4 );
		// �Ȱѳߴ���4�ı���
		short	nExtSize	= nSize%UNITSTEP;
		if( (nSize&UNITMASK) != 0 )
		{
			nSize	&= 0xFFFFFFFF^UNITMASK;
			nSize	+= UNITSTEP;
		}
		SIZELIST_T	*pSL	= NULL;
		short		nIdx	= 0;
		// ��������ߴ��Ƿ��Ѿ��������
		HASH2IT	it	= m_mapSize2List.find(nSize);
		if( it == m_mapSize2List.end() )
		{
			// ԭ��û�У������µ�
			if( m_nSizeNum >= m_MAXSIZENUM )
			{
				// û�м�¼�³ߴ�ĵط���
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
			// ԭ���У�ֱ����
			pSL	= (*it).GetVal();
			nIdx	= (short)(pSL-m_sl);
		}
		// ������б��з���
		UNITHDR_T	*pUnit	= NULL;
		// ����б����Ƿ��п��õ�
		if( pSL->avail_head.pNext != &pSL->avail_tail )
		{
			// ���þ�ֱ����
			pUnit	= pSL->avail_head.pNext;
			// ��avail�б���ɾ��
			pUnit->Quit();
			// ��������б�
			pUnit->InsertAfter(&pSL->alloc_head);
		}
		else
		{
			// ����ͷ����µ�
			size_t	nTotalSize	= sizeof(UNITHDR_T) + nSize;
			if( m_pCurChunk->nOffset+nTotalSize > m_pCurChunk->nChunkSize )
			{
				// Ӧ���ٷ���һ���µ�chunk��
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
			// �ӵ�ǰchunk�ĺ󲿿�ʼ����
			pUnit	= (UNITHDR_T *)GetCurPtr();
			pUnit->clear();
			pUnit->nSizeUnitIdx		= nIdx;
			// ��������б�
			pUnit->InsertAfter(&pSL->alloc_head);
			// ƫ�������
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
		// �������ݲ��ֵ�ָ��
		return	pUnit+1;
	}
	void	Free(void *pBuf)
	{
		if( pBuf == NULL )
		{
			return;
		}
		// ͨ��ָ���ҵ�UNITͷ��
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)pBuf) - sizeof(UNITHDR_T));
#ifdef	_DEBUG
		if( !pUnit->bAllocated )
		{
			// ��Ӧ�ñ��ͷŵ�ָ�루�ܿ������ͷ������Σ�
			assert(0);
			return;
		}
		pUnit->bAllocated	= false;
#endif
		// ��alloc�������˳�
		pUnit->Quit();
		// �ҵ���Ӧ�ĳߴ��б�����avail���С�
		assert(pUnit->nSizeUnitIdx>=0 && pUnit->nSizeUnitIdx<m_nSizeNum);
		pUnit->InsertAfter(&m_sl[pUnit->nSizeUnitIdx].avail_head);
	}
	// ���ָ���Ӧ�ڴ�Ĵ�С
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
	// ���ָ���Ӧ���߼��ڴ��С
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
	// ��õ�ǰ����ʣ��Ŀɷ���ռ�
	inline int		GetAvailSizeOfCurChunk() const
	{
		return	m_pCurChunk->GetAvailSize();
	}
	// ���leak��ͳ��
	void	LeakReport(FILE *perrout)
	{
		// ������е�sizeunit�����û�б��ͷŵĵ�Ԫ
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

// ֻ���һ�ֳߴ�ķ�������Ĭ��CHUNK�ߴ�4M��
template<class _AllocationObj>
class	whchunkallocationobjonesize
{
private:
	// һ�����䵥Ԫ��ͷ��
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
		// ��������InsertҪע�⣺
		// InsertAfter��������tail
		void	InsertAfter(UNITHDR_T *pOther)
		{
			pPrev	= pOther;
			pNext	= pOther->pNext;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// InsertBefore��������head
		void	InsertBefore(UNITHDR_T *pOther)
		{
			pPrev	= pOther->pPrev;
			pNext	= pOther;
			pPrev->pNext	= this;
			pNext->pPrev	= this;
		}
		// �Ӷ������˳�
		void	Quit()
		{
			pPrev->pNext	= pNext;
			pNext->pPrev	= pPrev;
		}
		// ��һ��β(tail)��������
		void	InitHeadTail(UNITHDR_T *pTail)
		{
			pPrev			= NULL;
			pNext			= pTail;
			pTail->pPrev	= this;
			pTail->pNext	= NULL;
		}
	};
	// ��ͨ����ͷ��
	struct	CHUNKHDR_T
	{
		CHUNKHDR_T	*pNextChunk;				// ��һ��ĵ�ַ
		void	clear()
		{
			pNextChunk	= NULL;
		}
	};
	_AllocationObj	*m_pAllocationObj;
	CHUNKHDR_T		*m_pFirstChunk;
	CHUNKHDR_T		*m_pCurChunk;
	char			*m_pCurUnitToAlloc;			// ��ǰ�ɷ���Ķ���ָ�루��char*��Ϊ�˺�����ƫ������
	int				m_nCurUnitIdxToAlloc;		// ��ǰ���Է���Ķ�������
	UNITHDR_T		avail_head, avail_tail;		// �����б�
	UNITHDR_T		alloc_head, alloc_tail;		// �ѷ����б�
	// ���ݲ��ֳߴ�
	int				m_DATASIZE;
	// һ�������䵥Ԫ�ߴ�
	int				m_UNITSIZE;
	// һ��chunk�ĳߴ�
	int				m_CHUNKSIZE;
	// һ��chunk�е���൥Ԫ����
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
		// ��ʼ�������б�
		avail_head.InitHeadTail(&avail_tail);
		alloc_head.InitHeadTail(&alloc_tail);
		// �����һ�飬Ȼ����һЩ��ʼ������
		m_pFirstChunk	= (CHUNKHDR_T *)m_pAllocationObj->Alloc(_CHUNKSIZE);
		m_pFirstChunk->clear();
		m_pCurChunk		= m_pFirstChunk;
		// ��ʼ���ͷ����ָ���ַ
		m_nCurUnitIdxToAlloc	= 0;
		m_pCurUnitToAlloc		= (char *)(m_pCurChunk+1);
		return	0;
	}
	int		Release()
	{
		if( m_pFirstChunk == NULL )
		{
			// �Ѿ���release���ˣ����߸���û�г�ʼ��
			return	0;
		}
		// ���ͷų�FirstChunk��������д�m_pAllocationObj����Ŀ�
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
		// ������б��з���
		UNITHDR_T	*pUnit	= NULL;
		// ����б����Ƿ��п��õ�
		if( avail_head.pNext != &avail_tail )
		{
			// ���þ�ֱ����
			pUnit	= avail_head.pNext;
			// ��avail�б���ɾ��
			pUnit->Quit();
			// ��������б�
			pUnit->InsertAfter(&alloc_head);
		}
		else
		{
			// ����ͷ����µ�
			if( m_nCurUnitIdxToAlloc >= m_NUMINCHUNK )
			{
				// Ӧ���ٷ���һ���µ�chunk��
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
			// �ӵ�ǰchunk�ĺ󲿿�ʼ����
			pUnit	= (UNITHDR_T *)m_pCurUnitToAlloc;
			pUnit->clear();
			// ��������б�
			pUnit->InsertAfter(&alloc_head);
			// ƫ�������
			m_pCurUnitToAlloc		+= m_UNITSIZE;
			m_nCurUnitIdxToAlloc	++;
		}
		// �������ݲ��ֵ�ָ��
		return	pUnit+1;
	}
	void	Free(void *pBuf)
	{
		if( pBuf == NULL )
		{
			return;
		}
		// ͨ��ָ���ҵ�UNITͷ��
		UNITHDR_T	*pUnit	= (UNITHDR_T *)(((char *)pBuf) - sizeof(UNITHDR_T));
		// ��alloc�������˳�
		pUnit->Quit();
		pUnit->InsertAfter(&avail_head);
	}
	// ���leak��ͳ��
	void	LeakReport(FILE *perrout)
	{
		// ������е�sizeunit�����û�б��ͷŵĵ�Ԫ
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

