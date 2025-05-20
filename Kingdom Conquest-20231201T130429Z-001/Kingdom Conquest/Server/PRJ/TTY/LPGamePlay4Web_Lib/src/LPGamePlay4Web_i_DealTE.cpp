#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

int		LPGamePlay4Web_i::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// �����¼��������Ƿ��ж���
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
	// �ж��û��Ƿ����,�����ж��û��Ƿ�û��LOGIN�ɹ�
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
		// �Ƴ��û�
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_NoLogin_TimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	LPGamePlay4Web_i::TEDeal_PlayerDropWait_TimeOut(TQUNIT_T* pTQUnit)
{
	// !!!���޸�,���Ӧ����ֱ��ɾ��PlayerUnit��֪ͨGameMngSɾ��Player��,��Ϊ���״̬���Ǻ���offline��Ϣ��
	TellLogicMainStructureToKickPlayer(pTQUnit->un.player.nID);
}
void	LPGamePlay4Web_i::SetPlayerTE_PlayerDropWait(PlayerUnit* pPlayer)
{
	TQUNIT_T*	pTQUnit	= NULL;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nPlayerDropWaitTimeOut, (void**)&pTQUnit, &pPlayer->teid) < 0)
	{
		assert(0);
		// �Ƴ��û�
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_PlayerDropWait_TimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	LPGamePlay4Web_i::TEDeal_PlayerCloseDelay_TimeOut(TQUNIT_T* pTQUnit)
{
	// !!!���޸�,���Ӧ����ֱ��ɾ��PlayerUnit��֪ͨGameMngSɾ��Player��,��Ϊ���״̬���Ǻ���offline��Ϣ��
	TellLogicMainStructureToKickPlayer(pTQUnit->un.player.nID);
}
void	LPGamePlay4Web_i::SetPlayerTE_PlayerCloseDelay(PlayerUnit* pPlayer, int nDelay)
{
	TQUNIT_T*	pTQUnit	= NULL;
	if (m_TQ.AddGetRef(m_tickNow+nDelay, (void**)&pTQUnit, &pPlayer->teid) < 0)
	{
		assert(0);
		// �Ƴ��û�
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}
	pTQUnit->tefunc			= &LPGamePlay4Web_i::TEDeal_PlayerCloseDelay_TimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	LPGamePlay4Web_i::TEDeal_Deal_DB_TimeEvent_TimeOut(TQUNIT_T* pTQUnit)
{
	// ����δ������б�
	P_DBS4WEB_GET_DB_TIMEEVENT_LIST_T	Cmd;
	Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd				= CMDID_GET_TIMEEVENT_LIST_REQ;
	CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));

	// ����ע��ʱ���¼�
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
	// ����δ������б�
	P_DBS4WEB_GET_PAY_LIST_T	Cmd;
	Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd			= CMDID_GET_PAY_LIST_REQ;
	SendCmdToDB(&Cmd, sizeof(Cmd));

	// ����ע��ʱ���¼�
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
	// ������AdColony��ʱ
	P_DBS4WEB_DEAL_ADCOLONY_TE_T	Cmd;
	Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd				= CMDID_DEAL_ADCOLONY_TE_REQ;
	SendCmdToDB(&Cmd, sizeof(Cmd));

	// ����ע��ʱ���¼�
	SetTE_DealAdColonyTimeEvent();
}
// ��������ĳ�ʱ
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
// ����ս������ĳ�ʱ
void	LPGamePlay4Web_i::TEDeal_Deal_GetAlliInstanceCombatResultEvent(TQUNIT_T* pTQUnit)
{
	P_DBS4WEB_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T	Cmd;
	Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd				= CMDID_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_REQ;
	SendCmdToDB(&Cmd, sizeof(Cmd));

	SetTE_GetAlliInstanceCombatResultEvent();
}
// ��������ĳ�ʱ
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
