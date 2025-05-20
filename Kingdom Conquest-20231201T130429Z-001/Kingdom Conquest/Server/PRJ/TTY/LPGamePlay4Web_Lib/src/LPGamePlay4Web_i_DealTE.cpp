#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

int		LPGamePlay4Web_i::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// 看看事件队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T*			pUnit = NULL;
	while (m_TQ.GetUnitBeforeTime(m_tickNow, (void**)&pUnit, &id) == 0)
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);
		m_TQ.Del(id);
	}

	return 0;
}
void	LPGamePlay4Web_i::TEDeal_NoLogin_TimeOut(TQUNIT_T* pTQUnit)
{
	// 判断用户是否存在,并且判断用户是否还没有LOGIN成功
	PlayerUnit*	pPlayer	= GetPlayerByID(pTQUnit->un.player.nID);
	if (pPlayer != NULL)
	{
		switch (pPlayer->GetStatus())
		{
		case PlayerUnit::STATUS_T::NOTHING:
		case PlayerUnit::STATUS_T::LOGINING:
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"TEDeal_NoLogin_TimeOut,NoLogin TimeOut,0x%X", pTQUnit->un.player.nID);
				TellLogicMainStructureToKickPlayer(pTQUnit->un.player.nID);
			}
			break;
		}
	}
}
void	LPGamePlay4Web_i::SetPlayerTE_NoLogin(PlayerUnit* pPlayer)
{
	TQUNIT_T*	pTQUnit	= NULL;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nNoLoginTimeOut, (void**)&pTQUnit, &pPlayer->teid) < 0)
	{
		assert(0);
		// 移除用户
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_NoLogin_TimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	LPGamePlay4Web_i::TEDeal_PlayerDropWait_TimeOut(TQUNIT_T* pTQUnit)
{
	// !!!得修改,这个应该是直接删除PlayerUnit并通知GameMngS删除Player了,因为这个状态下是忽略offline消息的
	TellLogicMainStructureToKickPlayer(pTQUnit->un.player.nID);
}
void	LPGamePlay4Web_i::SetPlayerTE_PlayerDropWait(PlayerUnit* pPlayer)
{
	TQUNIT_T*	pTQUnit	= NULL;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nPlayerDropWaitTimeOut, (void**)&pTQUnit, &pPlayer->teid) < 0)
	{
		assert(0);
		// 移除用户
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_PlayerDropWait_TimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	LPGamePlay4Web_i::TEDeal_PlayerCloseDelay_TimeOut(TQUNIT_T* pTQUnit)
{
	// !!!得修改,这个应该是直接删除PlayerUnit并通知GameMngS删除Player了,因为这个状态下是忽略offline消息的
	TellLogicMainStructureToKickPlayer(pTQUnit->un.player.nID);
}
void	LPGamePlay4Web_i::SetPlayerTE_PlayerCloseDelay(PlayerUnit* pPlayer, int nDelay)
{
	TQUNIT_T*	pTQUnit	= NULL;
	if (m_TQ.AddGetRef(m_tickNow+nDelay, (void**)&pTQUnit, &pPlayer->teid) < 0)
	{
		assert(0);
		// 移除用户
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_PlayerCloseDelay_TimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	LPGamePlay4Web_i::TEDeal_Deal_DB_TimeEvent_TimeOut(TQUNIT_T* pTQUnit)
{
	// 请求未处理的列表
	P_DBS4WEB_GET_DB_TIMEEVENT_LIST_T	Cmd;
	Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd				= CMDID_GET_TIMEEVENT_LIST_REQ;
	CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));

	// 重新注册时间事件
	SetTE_DealDBTimeEvent();
}
void	LPGamePlay4Web_i::SetTE_DealDBTimeEvent()
{
	TQUNIT_T*	pTQUnit	= NULL;
	whtimequeue::ID_T	teid;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nDealDBTimeEventTimeOut, (void**)&pTQUnit, &teid) < 0)
	{
		assert(0);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_Deal_DB_TimeEvent_TimeOut;
}
void	LPGamePlay4Web_i::TEDeal_Deal_Pay_TimeEvent_TimeOut(TQUNIT_T* pTQUnit)
{
	// 请求未处理的列表
	P_DBS4WEB_GET_PAY_LIST_T	Cmd;
	Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd			= CMDID_GET_PAY_LIST_REQ;
	SendCmdToDB(&Cmd, sizeof(Cmd));

	// 重新注册时间事件
	SetTE_DealPayTimeEvent();
}
void	LPGamePlay4Web_i::SetTE_DealPayTimeEvent()
{
	TQUNIT_T*	pTQUnit	= NULL;
	whtimequeue::ID_T	teid;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nDealPayTimeOut, (void**)&pTQUnit, &teid) < 0)
	{
		assert(0);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_Deal_Pay_TimeEvent_TimeOut;
}
void	LPGamePlay4Web_i::TEDeal_Deal_AdColony_TimeEvent_TimeOut(TQUNIT_T* pTQUnit)
{
	// 请求处理AdColony超时
	P_DBS4WEB_DEAL_ADCOLONY_TE_T	Cmd;
	Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd				= CMDID_DEAL_ADCOLONY_TE_REQ;
	SendCmdToDB(&Cmd, sizeof(Cmd));

	// 重新注册时间事件
	SetTE_DealAdColonyTimeEvent();
}
// 设置上面的超时
void	LPGamePlay4Web_i::SetTE_DealAdColonyTimeEvent()
{
	TQUNIT_T*	pTQUnit	= NULL;
	whtimequeue::ID_T	teid;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nDealPayTimeOut, (void**)&pTQUnit, &teid) < 0)
	{
		assert(0);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_Deal_AdColony_TimeEvent_TimeOut;
}
// 处理战斗结果的超时
void	LPGamePlay4Web_i::TEDeal_Deal_GetAlliInstanceCombatResultEvent(TQUNIT_T* pTQUnit)
{
	P_DBS4WEB_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T	Cmd;
	Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd				= CMDID_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_REQ;
	SendCmdToDB(&Cmd, sizeof(Cmd));

	SetTE_GetAlliInstanceCombatResultEvent();
}
// 设置上面的超时
void	LPGamePlay4Web_i::SetTE_GetAlliInstanceCombatResultEvent()
{
	TQUNIT_T*	pTQUnit	= NULL;
	whtimequeue::ID_T	teid;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nDealCombatResultTimeOut, (void**)&pTQUnit, &teid) < 0)
	{
		assert(0);
		return;
	}
	pTQUnit->tefunc		= &LPGamePlay4Web_i::TEDeal_Deal_GetAlliInstanceCombatResultEvent;
}
