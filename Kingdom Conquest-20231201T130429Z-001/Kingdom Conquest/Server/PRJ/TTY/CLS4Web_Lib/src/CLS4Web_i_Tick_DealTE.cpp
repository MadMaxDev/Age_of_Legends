#include "../inc/CLS4Web_i.h"

using namespace n_pngs;

int		CLS4Web_i::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// 看看时间队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T*			pUnit;
	while (m_TQ.GetUnitBeforeTime(m_tickNow, (void**)&pUnit, &id) == 0)
	{
		assert(pUnit->tefunc != NULL);
		(this->*pUnit->tefunc)(pUnit);
		m_TQ.Del(id);
	}
	return 0;
}
void	CLS4Web_i::TEDeal_ClientConnnectToMeTimeOut(TQUNIT_T* pTQUnit)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"TEDeal_ClientConnnectToMeTimeOut,0x%X", pTQUnit->un.player.nID);
	RemovePlayerUnit(pTQUnit->un.player.nID);
}
void	CLS4Web_i::SetTEDeal_ClientConnectToMeTimeOut(PlayerUnit* pPlayer)
{
	TQUNIT_T*	pTQUnit	= NULL;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nClientConnectToMeTimeOut, (void**)&pTQUnit, &pPlayer->teid) < 0)
	{
		assert(0);
		// 移除用户
		RemovePlayerUnit(pPlayer);
		return;
	}
	pTQUnit->tefunc			= &CLS4Web_i::TEDeal_ClientConnnectToMeTimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	CLS4Web_i::TEDeal_CheckRecvInterval(TQUNIT_T* pTQUnit)
{
	// 获取player指针
	PlayerUnit*	pPlayer		= m_Players.getptr(pTQUnit->un.player.nID);
	if (pPlayer != NULL)
	{
		if (pPlayer->nRecvCount>m_cfginfo.nMaxRecvCount || pPlayer->nRecvSize>m_cfginfo.nMaxRecvSize)
		{
			// 踢用户下线
			GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(1394,CLS4Web_RT)"%s,recv too much,0x%X,%s,%d,%d", __FUNCTION__, pPlayer->nID, pPlayer->szName, pPlayer->nRecvCount, pPlayer->nRecvSize);
			pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
			RemovePlayerUnit(pPlayer);
		}
		else
		{
			// 清空,重新统计
			pPlayer->nRecvCount	= 0;
			pPlayer->nRecvSize	= 0;
			SetTEDeal_CheckRecvInterval(pPlayer);
		}
	}
}
void	CLS4Web_i::SetTEDeal_CheckRecvInterval(PlayerUnit* pPlayer)
{
	TQUNIT_T*	pTQUnit	= NULL;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCheckRecvInterval, (void**)&pTQUnit, &pPlayer->teid) < 0)
	{
		assert(0);
		// 移除用户
		RemovePlayerUnit(pPlayer);
		return;
	}
	pTQUnit->tefunc			= &CLS4Web_i::TEDeal_CheckRecvInterval;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
