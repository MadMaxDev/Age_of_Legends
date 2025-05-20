// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick_DealTE.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CAAFS��Tick_DealTE���ֵľ���ʵ�֣��͸���TEDeal_XXX����
// CreationDate : 2005-07-28
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// ����ʱ��������Ƿ��ж���
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_TQ.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// ���������������ʱ���¼�����������������ŵġ�
		// ���get������ֻ��ͨ��Delɾ���������Լ�ɾ
		m_TQ.Del(id);
	}
	return	0;
}

void	CAAFS2_I::TEDeal_Close_TimeOut(TQUNIT_T * pTQUnit)
{
	// ɾ������û�
	RemovePlayerUnit(pTQUnit->un.player.nID);
}
void	CAAFS2_I::TEDeal_QueueSeq(TQUNIT_T * pTQUnit)
{
	PlayerUnit	*pPlayer	= m_Players.getptr(pTQUnit->un.player.nID);
	assert(pPlayer);
	// ���ǰ���Ƿ������仯������仯�˾ͷ����µ�Queue��Ϣ���ͻ���
	SendPlayerQueueInfo(pPlayer, false);
	WAITCHANNEL_T		*pC	= m_vectWaitChannel.getptr(pPlayer->nVIPChannel);
	if( pC->nWaitingSeq0 != pPlayer->nSeq0 )
	{
		// ��Ҫ���·���
		CAAFS_CLIENT_QUEUEINFO_T	QueueInfo;
		QueueInfo.nCmd				= CAAFS_CLIENT_QUEUEINFO;
		pPlayer->nSeq0				= pC->nWaitingSeq0;
		QueueInfo.nQueueSize		= pPlayer->nSeq - pPlayer->nSeq0;
		m_pSlotMan4Client->Send(pPlayer->nSlot, &QueueInfo, sizeof(QueueInfo), 0);
	}
	// ע����һ���¼�
	SetTE_QueueSeq(pPlayer);
}
void	CAAFS2_I::SetTE_QueueSeq(PlayerUnit *pPlayer, bool bRand)
{
	TQUNIT_T	*pNextTQUnit;
	whtick_t	t	= 0;
	if( bRand )
	{
		t	= m_tickNow+(rand()%(m_cfginfo.nQueueSeqSendInterval/100+1))*100;
	}
	else
	{
		t	= m_tickNow+m_cfginfo.nQueueSeqSendInterval;
	}
	if( m_TQ.AddGetRef(t, (void **)&pNextTQUnit, &pPlayer->teid)<0 )
	{
		// ��ô���޷�����ʱ���¼�����
		assert(0);
		// �Ƴ��û�
		RemovePlayerUnit(pPlayer);
		return;
	}
	pNextTQUnit->tefunc				= &CAAFS2_I::TEDeal_QueueSeq;
	pNextTQUnit->un.player.nID		= pPlayer->nID;
}

void	CAAFS2_I::TEDeal_ResumeMapTimeOut(TQUNIT_T * pTQUnit)
{
	for(map<string,ResumedPlayerUnit>::iterator iter=m_wait2resume_map.begin();iter!=m_wait2resume_map.end();iter++)
	{
		if(iter->second.nLeaveTime < m_tickNow - m_cfginfo.nQueuePosKeepTime*1000)
		{
#ifdef UNITTEST
			printf("ɾ����ʱ�û� :%s\n",iter->first.c_str());
#endif
			m_wait2resume_map.erase(iter++);
		}
	}
	SetTE_ResumeMapTimeOut();
}

void	CAAFS2_I::SetTE_ResumeMapTimeOut()
{
	TQUNIT_T	*pNextTQUnit;
	whtick_t	t	= 0;
	t	= m_tickNow+ 60*1000;
	if( m_TQ.AddGetRef(t, (void **)&pNextTQUnit, &m_resume_map_tid)<0 )
	{
		// ��ô���޷�����ʱ���¼�����
		assert(0);
		return;
	}
	pNextTQUnit->tefunc				= &CAAFS2_I::TEDeal_ResumeMapTimeOut;
}
