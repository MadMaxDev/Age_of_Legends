// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtimequeue.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ʱ�����(���Է��ü����49.7����¼�)
// CreationDate : 2005-04-11
// ChangeLOG    : 2005-09-13 whtimequeue::GetUnitBeforeTime�ж�δ��ʱ���жϴ�ԭ����>=��Ϊ>�������ڱ�tick������Ϊ��ǰtick��һ���ᱻ����
//              : 2008-07-14 ������whtimequeue::Del�еı�����ʩ���Ա���set���ݱ��ƻ�ʱҲ���Ծ���ɾ����

#include "../inc/whtimequeue.h"

using namespace n_whcmn;

whtimequeue::whtimequeue()
: m_punits(NULL)
, m_nRotate(0)
{
}
whtimequeue::~whtimequeue()
{
	Release();
}
int		whtimequeue::Init(INFO_T *pInfo)
{
	if(m_punits!=NULL)
	{
		assert(0);
		return	-1;
	}
	m_punits	= new whsmpunitallocator(pInfo->nUnitLen, pInfo->nChunkSize);
	assert(m_punits);
	memcpy(&m_info, pInfo, sizeof(m_info));
	m_nRotate	= 0;
	return	0;
}
int		whtimequeue::Release()
{
	m_idset.clear();
	if( m_punits )
	{
		delete		m_punits;
		m_punits	= NULL;
	}
	return	0;
}
void	whtimequeue::Clear()
{
	m_punits->clear();
	m_idset.clear();
}
int 	whtimequeue::Add(whtick_t t, void *pUnit, ID_T *pID)
{
	void	*pUnitRef;
	int	rst	= AddGetRef(t, &pUnitRef, pID);
	if( rst<0 )
	{
		return	rst;
	}
	memcpy(pUnitRef, pUnit, m_info.nUnitLen);
	return	0;
}
int 	whtimequeue::AddGetRef(whtick_t t, void **ppUnit, ID_T *pID)
{
	assert(pID);
	ID_T	&id	= *pID;
	id.quit();	// Ϊ�˱��գ�quitһ��
	char	*pszBuf;
	// ����idx
	id.t	= t;
	id.idx	= m_punits->AllocUnit(pszBuf);
	id.pHost= NULL;
	if( id.idx<0 )
	{
		return	-1;
	}
	*ppUnit	= pszBuf;
	// ����set
	idset_t::iterator	it	= m_idset.find(id);
	if( it != m_idset.end() )
	{
		m_punits->FreeUnit(id.idx);
		assert(0);
		return	-2;
	}
	id.nRotate	= m_nRotate++;
	// �Ȳ��룬pHostҪ�ں������ã�����ͨ������������id��û��pHost��Ҳ�Ͳ����Լ��˳���
	m_idset.insert(id);
	// �������Ҫ�������
	id.pHost	= this;

	return	0;
}
int		whtimequeue::TimeMove(whtimequeue::ID_T *pID, whtick_t newt)
{
	assert(pID->pHost);
	assert(pID->idx >= 0);
	// ����
	idset_t::iterator	it	= m_idset.find(*pID);
	if( it == m_idset.end() )
	{
		// û�����
		return		-1;
	}
	// ɾ���ɵ�
	m_idset.erase(it);
	// �����µ�
	pID->t		= newt;
	pID->pHost	= NULL;
	m_idset.insert(*pID);
	pID->pHost	= this;
	return	0;
}
int		whtimequeue::Del(ID_T &id)
{
	if( id.idx<0 )
	{
		// û�����
		// Ϊ�˱��ջ���Ҫ���host���������Լ�����quit���ᵽ������
		id.pHost	= NULL;
		return		-1;
	}
	idset_t::iterator	it	= m_idset.find(id);
	if( it == m_idset.end() )
	{
		// û�����
		if( id.pHost )
		{
			// Ϊ�˱��ջ���Ҫ���host���������Լ�����quit���ᵽ�������������������Ϊ��ĵط���ɾ���ˣ�
			id.pHost	= NULL;
		}
		// �ж��Ƿ���begin
		it	= m_idset.begin();
		if( m_idset.size()>0 && (*it)==id )
		{
			// ������ֱ�ӵ�����ȥɾ���ˣ��������Ի�������set���ݱ��ƻ����µ���ѭ��
			// ��¼һ����־
			WHCMN_LOG_WRITEFMT(WHCMN_LOG_ID_FATAL, WHCMN_LOG_STD_HDR(1019, TQ_ERR)"whtimequeue::Del Fail,%d,%d,%d", id.t, id.nRotate, id.idx);
		}
		else
		{
			return		-1;
		}
	}

	m_idset.erase(it);
	m_punits->FreeUnit(id.idx);
	// Ϊ�˱���ֱ�����
	id.clear();
	return	0;
}
int		whtimequeue::GetUnitBeforeTime(whtick_t t, void **ppUnit, ID_T *pID)
{
	idset_t::iterator	it	= m_idset.begin();
	if( it == m_idset.end() )
	{
		// û�ж�����
		return	-1;
	}
	if( wh_tickcount_diff( (*it).t, t) > 0 )
	{
		// ʱ��û��(��������ʱ������ڵ�ʱ����)(ֻҪ��Ⱦ���Ϊ��ʱ����)
		return	-2;
	}
	if( pID )
	{
		*pID	= (*it);
		// (2005-07-12)���ҳ�����pID->pHostӦ��һ���ǿյ�
		assert(pID->pHost == NULL);
	}
	*ppUnit	= GetUnitPtr((*it).idx);
	assert( (*ppUnit)!=NULL );
	return	0;
}
