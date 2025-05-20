#include "../inc/CAAFS4Web_i.h"

using namespace n_pngs;

int		CAAFS4Web_i::Tick_DealTE()
{
	m_tickNow		= wh_gettickcount();
	// ����ʱ���������ʲô����
	whtimequeue::ID_T	id;
	TQUNIT_T*			pUnit;
	while (m_TQ.GetUnitBeforeTime(m_tickNow, (void**)&pUnit, &id) == 0)
	{
		assert(pUnit->tefunc != NULL);
		(this->*pUnit->tefunc)(pUnit);		// ���������������ʱ���¼�
		// ����ɾ��,��Ȼ�������
		m_TQ.Del(id);
	}
	return 0;
}
void	CAAFS4Web_i::TEDeal_Close_TimeOut(TQUNIT_T* pTQUnit)
{
	// ɾ������û�
	RemovePlayerUnit(pTQUnit->un.player.nID);
}
void	CAAFS4Web_i::TEDeal_QueueSeq(TQUNIT_T* pTQUnit)
{
	PlayerUnit*	pPlayer		= m_Players.getptr(pTQUnit->un.player.nID);
	assert(pPlayer != NULL);
	// ��������仯,�ŷ���
	SendPlayerQueueInfo(pPlayer, false);
	// ע����һ���¼�
	SetTE_QueueSeq(pPlayer);
}
void	CAAFS4Web_i::SetTE_QueueSeq(PlayerUnit* pPlayer, bool bRand/* =false */)
{
	TQUNIT_T*	pNextTQUnit;
	whtick_t	t	= 0;
	if (bRand)
	{
		t		= m_tickNow+(rand()%(m_cfginfo.nQueueSeqSendInterval/100+1))*100;
	}
	else
	{
		t		= m_tickNow+m_cfginfo.nQueueSeqSendInterval;
	}
	if (m_TQ.AddGetRef(t, (void**)&pNextTQUnit, &pPlayer->teid) < 0)
	{
		assert(0);
		// �Ƴ��û�
		RemovePlayerUnit(pPlayer);
		return;
	}
	pNextTQUnit->tefunc			= &CAAFS4Web_i::TEDeal_QueueSeq;
	pNextTQUnit->un.player.nID	= pPlayer->nID;
}
