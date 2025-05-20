// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whunitallocator.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ���ڼ򵥵Ķ�̬���������ֻ����Ԫ������
//                ����ʹ�ñ�׼stl��Ϊ�˿���ʹ�ö����Ӧ�Ĺ��캯������
// CreationDate : 2003-10-11
// ChangeLog    : 

#include "../inc/whunitallocator.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whbufarray
////////////////////////////////////////////////////////////////////
whbufarray::whbufarray()
: m_pBuf(NULL)
, m_nUnitSize(0)
, m_nTotal(0)
{
}
whbufarray::~whbufarray()
{
	Release();
}
int		whbufarray::Init(int nTotal, int nUnitSize)
{
	m_nUnitSize	= nUnitSize;
	m_nTotal	= nTotal;
	m_pBuf		= new char[nUnitSize*nTotal];
	if( !m_pBuf )
	{
		return	-1;
	}
	return	0;
}
int		whbufarray::Release()
{
	if( m_pBuf )
	{
		delete []	m_pBuf;
		m_pBuf		= NULL;
	}
	return	0;
}
char *	whbufarray::GetPtr(int nIdx) const
{
	if( nIdx<0 || nIdx>=m_nTotal )
	{
		return	NULL;
	}
	return		m_pBuf + m_nUnitSize * nIdx;
}

////////////////////////////////////////////////////////////////////
// whsmpunitallocator
////////////////////////////////////////////////////////////////////
int		whsmpunitallocator::AllocUnit(char *&pBuf)
{
	int	nIdx;
	if( m_listAvail.size()>0 )
	{
		nIdx	= *(m_listAvail.begin());
		m_listAvail.pop_front();
	}
	else
	{
		nIdx	= m_chunkUnit.size();
		m_chunkUnit.alloc();
	}
	UNIT_T	*pUnit		= getunit(nIdx);
	pUnit->bUsed		= true;
	pBuf				= pUnit->data;
	return	nIdx;
}
int		whsmpunitallocator::FreeUnit(int nIdx)
{
	UNIT_T	*pUnit	= (UNIT_T *)m_chunkUnit[nIdx];
	if( !pUnit->bUsed )
	{
		// �Ѿ��ͷ���(��ų����Ҳ���)
		return	-1;
	}
	pUnit->bUsed	= false;
	// ��ϣ�����������ָո��ͷžͱ�ʹ�õ�����
	m_listAvail.push_back(nIdx);
	// m_listAvail.push_front(nIdx);
	return		0;
}
void	whsmpunitallocator::clear()
{
	m_chunkUnit.destroy();
	m_listAvail.clear();
}
