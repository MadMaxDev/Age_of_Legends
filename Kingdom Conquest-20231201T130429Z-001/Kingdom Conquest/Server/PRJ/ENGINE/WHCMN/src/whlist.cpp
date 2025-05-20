// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whlist.cpp
// Creator: Wei Hua (魏华)
// Comment: 简单的链表
// CreationDate: 2004-01-18
// ChangeLog   :

#include "../inc/whlist.h"
#include "../inc/whcmn_def.h"

using namespace n_whcmn;

whalist::whalist()
: m_pHdr(NULL)
{
}
int		whalist::Init(void *pBuf, size_t nDataUnitSize, size_t nMaxNum)
{
	assert(nMaxNum>0);
	m_pHdr					= (HDR_T *)pBuf;
	m_pHdr->nDataUnitSize	= nDataUnitSize;
	m_pHdr->nMaxNum			= nMaxNum;
	m_pHdr->nCurNum			= 0;
	m_pHdr->AvailableHdr	= INVALIDIDX;

	m_pUnit					= (char *)(m_pHdr+1);	// 相当于从头跳过一个HDR_T的长度
	m_nUnitSize				= wh_offsetof(UNIT_T,Data) + nDataUnitSize;

	// 初始化available队列 (这样写虽然有些费，但是因为调用的少，而且简单，就这么写啦)
	for(int i=nMaxNum-1; i>=0; --i)
	{
		UNIT_T	*pUnit		= GetUnitPtr(i);
		pUnit->bAllocated	= false;
		pUnit->Next			= m_pHdr->AvailableHdr;
		m_pHdr->AvailableHdr= i;
	}

	return	0;
}
int		whalist::Inherit(void *pBuf)
{
	m_pHdr					= (HDR_T *)pBuf;
	m_pUnit					= (char *)(m_pHdr+1);	// 相当于从头跳过一个HDR_T的长度
	m_nUnitSize				= wh_offsetof(UNIT_T,Data) + m_pHdr->nDataUnitSize;
	return	0;
}
int		whalist::InheritEnlarge(void *pBuf, size_t nNewMaxNum, size_t nNewDataUnitSize)
{
	Inherit(pBuf);
	if( m_pHdr->nMaxNum < nNewMaxNum )	// 如果是缩小就不考虑了
	{
		// 扩展avail队列
		size_t	nOldMax			= m_pHdr->nMaxNum;
		m_pHdr->nMaxNum			= nNewMaxNum;
		for(int i=(int)nNewMaxNum-1; i>=(int)nOldMax; --i)
		{
			UNIT_T	*pUnit		= GetUnitPtr(i);
			pUnit->bAllocated	= false;
			pUnit->Next			= m_pHdr->AvailableHdr;
			m_pHdr->AvailableHdr= i;
		}
	}
	if( m_pHdr->nDataUnitSize < nNewDataUnitSize )
	{
		// 加长各个unit
		// 从最后一个开始移动，一直到第一个
		int	nNewUnitSize		= wh_offsetof(UNIT_T,Data) + nNewDataUnitSize;
		for(int i=(int)m_pHdr->nMaxNum-1; i>=0; --i)
		{
			memmove(m_pUnit + i*nNewUnitSize, m_pUnit + i*m_nUnitSize, m_nUnitSize);
		}
		// 改变参数
		m_pHdr->nDataUnitSize	= nNewDataUnitSize;
		m_nUnitSize				= nNewUnitSize;
	}
	return	0;
}
int		whalist::Reset()
{
	if( m_pHdr )
		return	Init(m_pHdr, m_pHdr->nDataUnitSize, m_pHdr->nMaxNum);
	else
		return	0;
}
int		whalist::Alloc()
{
	int		nIdx				= m_pHdr->AvailableHdr;
	UNIT_T	*pUnit				= GetUnitPtr(m_pHdr->AvailableHdr);
	if( pUnit )
	{
		// AvailableHdr指向下一个
		m_pHdr->AvailableHdr	= pUnit->Next;
		// 标记为已经被申请
		pUnit->bAllocated		= true;
		// Next初始值
		pUnit->Next				= INVALIDIDX;
		// 已经申请的数量
		m_pHdr->nCurNum			++;
	}
	return	nIdx;
}
int		whalist::Free(int nIdx)
{
	UNIT_T	*pUnit				= GetUnitPtr(nIdx);
	if( !pUnit || !pUnit->bAllocated )
	{
		return	-1;
	}
	pUnit->bAllocated			= false;

	// 放入Available中
	pUnit->Next					= m_pHdr->AvailableHdr;
	m_pHdr->AvailableHdr		= nIdx;
	// 已经申请的数量
	m_pHdr->nCurNum				--;
	return	0;
}
int		whalist::FreeChain(int nHeadIdx)
{
	if( nHeadIdx == whalist::INVALIDIDX )
	{
		// 这样的东东怎么可能被释放呢？玩我啊!@#!@#$
		return	-1;
	}

	int		nIdx			= nHeadIdx;
	int		nTailIdx		= nHeadIdx;
	int		nNum			= 0;
	UNIT_T	*pUnit			= NULL;

	// 将所有的Allocated状态置为false
	// 并统计总数
	while( nIdx != whalist::INVALIDIDX )
	{
		pUnit				= GetUnitPtr(nIdx);
		// 断言原来一定是被分配的
		assert(pUnit->bAllocated);
		pUnit->bAllocated	= false;
		// nTailIdx将指向最后一个
		nTailIdx			= nIdx;
		// 下一个
		nIdx				= pUnit->Next;
		nNum				++;
	}

	// 能到一步pUnit一定不会为空的，它是最后一个单元的指针
	// 让结尾的下一个指向现在的AvailableHdr
	// 然后让AvailableHdr指向nHeadIdx
	assert( pUnit );
	pUnit->Next				= m_pHdr->AvailableHdr;
	m_pHdr->AvailableHdr	= nHeadIdx;

	// 已经申请的数量减少
	m_pHdr->nCurNum			-= nNum;

	return	0;
}
void*	whalist::GetDataUnitPtr(int nIdx) const
{
	UNIT_T	*pUnit = GetUnitPtr(nIdx);
	if( !pUnit || !pUnit->bAllocated )
	{
		return	NULL;
	}
	return	pUnit->Data;
}
bool	whalist::IsUnitAllocated(int nIdx) const
{
	UNIT_T	*pUnit = GetUnitPtr(nIdx);
	if( !pUnit || !pUnit->bAllocated )
	{
		return	false;
	}
	return	true;
}
size_t	whalist::GetNum() const
{
	if( m_pHdr )
	{
		return	m_pHdr->nCurNum;
	}
	else
	{
		return	0;
	}
}
size_t	whalist::GetAvailNum() const
{
	if( m_pHdr )
	{
		return	m_pHdr->nMaxNum - m_pHdr->nCurNum;
	}
	else
	{
		return	0;
	}
}
whalist::UNIT_T*	whalist::GetUnitPtr(int nIdx) const
{
	if( nIdx<0 || nIdx>=(int)m_pHdr->nMaxNum )
	{
		return	NULL;
	}
	return	(UNIT_T *)(m_pUnit + nIdx*m_nUnitSize);
}

int		whalist::GetNextOf(int nIdx) const
{
	UNIT_T	*pUnit = (UNIT_T *)GetUnitPtr(nIdx);
	if( !pUnit || !pUnit->bAllocated )
	{
		return	INVALIDIDX;
	}
	return	pUnit->Next;
}
bool	whalist::SetNextOf(int nIdx, int nNext)
{
	UNIT_T	*pUnit = (UNIT_T *)GetUnitPtr(nIdx);
	if( !pUnit || !pUnit->bAllocated )
	{
		return	false;
	}
	pUnit->Next	= nNext;
	return	true;
}
