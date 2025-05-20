// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whlist.cpp
// Creator: Wei Hua (κ��)
// Comment: �򵥵�����
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

	m_pUnit					= (char *)(m_pHdr+1);	// �൱�ڴ�ͷ����һ��HDR_T�ĳ���
	m_nUnitSize				= wh_offsetof(UNIT_T,Data) + nDataUnitSize;

	// ��ʼ��available���� (����д��Ȼ��Щ�ѣ�������Ϊ���õ��٣����Ҽ򵥣�����ôд��)
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
	m_pUnit					= (char *)(m_pHdr+1);	// �൱�ڴ�ͷ����һ��HDR_T�ĳ���
	m_nUnitSize				= wh_offsetof(UNIT_T,Data) + m_pHdr->nDataUnitSize;
	return	0;
}
int		whalist::InheritEnlarge(void *pBuf, size_t nNewMaxNum, size_t nNewDataUnitSize)
{
	Inherit(pBuf);
	if( m_pHdr->nMaxNum < nNewMaxNum )	// �������С�Ͳ�������
	{
		// ��չavail����
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
		// �ӳ�����unit
		// �����һ����ʼ�ƶ���һֱ����һ��
		int	nNewUnitSize		= wh_offsetof(UNIT_T,Data) + nNewDataUnitSize;
		for(int i=(int)m_pHdr->nMaxNum-1; i>=0; --i)
		{
			memmove(m_pUnit + i*nNewUnitSize, m_pUnit + i*m_nUnitSize, m_nUnitSize);
		}
		// �ı����
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
		// AvailableHdrָ����һ��
		m_pHdr->AvailableHdr	= pUnit->Next;
		// ���Ϊ�Ѿ�������
		pUnit->bAllocated		= true;
		// Next��ʼֵ
		pUnit->Next				= INVALIDIDX;
		// �Ѿ����������
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

	// ����Available��
	pUnit->Next					= m_pHdr->AvailableHdr;
	m_pHdr->AvailableHdr		= nIdx;
	// �Ѿ����������
	m_pHdr->nCurNum				--;
	return	0;
}
int		whalist::FreeChain(int nHeadIdx)
{
	if( nHeadIdx == whalist::INVALIDIDX )
	{
		// �����Ķ�����ô���ܱ��ͷ��أ����Ұ�!@#!@#$
		return	-1;
	}

	int		nIdx			= nHeadIdx;
	int		nTailIdx		= nHeadIdx;
	int		nNum			= 0;
	UNIT_T	*pUnit			= NULL;

	// �����е�Allocated״̬��Ϊfalse
	// ��ͳ������
	while( nIdx != whalist::INVALIDIDX )
	{
		pUnit				= GetUnitPtr(nIdx);
		// ����ԭ��һ���Ǳ������
		assert(pUnit->bAllocated);
		pUnit->bAllocated	= false;
		// nTailIdx��ָ�����һ��
		nTailIdx			= nIdx;
		// ��һ��
		nIdx				= pUnit->Next;
		nNum				++;
	}

	// �ܵ�һ��pUnitһ������Ϊ�յģ��������һ����Ԫ��ָ��
	// �ý�β����һ��ָ�����ڵ�AvailableHdr
	// Ȼ����AvailableHdrָ��nHeadIdx
	assert( pUnit );
	pUnit->Next				= m_pHdr->AvailableHdr;
	m_pHdr->AvailableHdr	= nHeadIdx;

	// �Ѿ��������������
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
