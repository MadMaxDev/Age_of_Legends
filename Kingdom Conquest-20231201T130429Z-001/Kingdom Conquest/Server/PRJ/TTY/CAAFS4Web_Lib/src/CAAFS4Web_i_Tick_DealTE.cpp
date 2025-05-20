#include "../inc/CAAFS4Web_i.h"

using namespace n_pngs;

int		CAAFS4Web_i::Tick_DealTE()
{
	m_tickNow		= wh_gettickcount();
	// 看看时间队列中有什么东西
	whtimequeue::ID_T	id;
	TQUNIT_T*			pUnit;
	while (m_TQ.GetUnitBeforeTime(m_tickNow, (void**)&pUnit, &id) == 0)
	{
		assert(pUnit->tefunc != NULL);
		(this->*pUnit->tefunc)(pUnit);		// 这里面可以申请别的时间事件
		// 必须删除,不然会出问题
		m_TQ.Del(id);
	}
	return 0;
}
void	CAAFS4Web_i::TEDeal_Close_TimeOut(TQUNIT_T* pTQUnit)
{
	// 删除这个用户
	RemovePlayerUnit(pTQUnit->un.player.nID);
}
void	CAAFS4Web_i::TEDeal_QueueSeq(TQUNIT_T* pTQUnit)
{
	PlayerUnit*	pPlayer		= m_Players.getptr(pTQUnit->un.player.nID);
	assert(pPlayer != NULL);
	// 如果发生变化,才发送
	SendPlayerQueueInfo(pPlayer, false);
	// 注册下一个事件
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
		// 移除用户
		RemovePlayerUnit(pPlayer);
		return;
	}
	pNextTQUnit->tefunc			= &CAAFS4Web_i::TEDeal_QueueSeq;
	pNextTQUnit->un.player.nID	= pPlayer->nID;
}
