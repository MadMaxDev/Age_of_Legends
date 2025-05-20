#include "../inc/GS4Web_i.h"

using namespace n_pngs;

int		GS4Web_i::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// 看看事件队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T*			pUnit	= NULL;
	while (m_TQ.GetUnitBeforeTime(m_tickNow, (void**)&pUnit, &id) == 0)
	{
		assert(pUnit->tefunc != NULL);
		(this->*pUnit->tefunc)(pUnit);
		m_TQ.Del(id);
	}

	return 0;
}
int		GS4Web_i::TEDeal_LoadingNotification_TimeOut(TQUNIT_T* pTQUnit)
{
	ReqLoadNotification();
	SetTE_LoadingNotification();
	return 0;
}
int		GS4Web_i::SetTE_LoadingNotification()
{
	TQUNIT_T*	pTQUnit	= NULL;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nLoadNotificationTimeOut, (void**)&pTQUnit, &m_teidLoadNotification) < 0)
	{
		return -1;
	}
	pTQUnit->tefunc		= &GS4Web_i::TEDeal_LoadingNotification_TimeOut;
	return 0;
}
int		GS4Web_i::TEDeal_QueryGroupInfo_TimeOut(TQUNIT_T* pTQUnit)
{
	GS4WEB_GROUP_GET_GROUP_INFO_REQ_T	Cmd;
	Cmd.nCmd			= P_GS4WEB_GROUP_REQ_CMD;
	Cmd.nSubCmd			= GROUP_GET_GROUP_INFO_REQ;
	SendMsgToAllGroup(&Cmd, sizeof(Cmd));

	SetTE_QueryGroupInfo();
	return 0;
}
int		GS4Web_i::SetTE_QueryGroupInfo()
{
	TQUNIT_T*	pTQUnit	= NULL;
	whtimequeue::ID_T	id;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nQueryGroupInfoTimeOut, (void**)&pTQUnit, &id) < 0)
	{
		return -1;
	}
	pTQUnit->tefunc		= &GS4Web_i::TEDeal_QueryGroupInfo_TimeOut;
	return 0;
}
int		GS4Web_i::TEDeal_WriteGroupInfo_TimeOut(TQUNIT_T* pTQUnit)
{
	FILE*	fp			= fopen(m_cfginfo.szGroupInfoFileName, "wt");
	if (fp != NULL)
	{
		for (int i=0; i<TTY_MAX_GG_NUM; i++)
		{
			if (m_aGGs[i].nCntrID > 0)
			{
				GGUnit_T&	gg	= m_aGGs[i];
				fprintf(fp, "%d,%d,%d", i, gg.nPlayerTotalNum, gg.nMaxPlayerNum);
				for (int j=0; j<gg.vectTermUnits.size(); j++)
				{
					if (gg.vectTermUnits[j].nPlayerNum != 0)
					{
						fprintf(fp, " ,%d,%d", gg.vectTermUnits[j].nTermType, gg.vectTermUnits[j].nPlayerNum);
					}
				}
				fprintf(fp, "%s", WHLINEEND);
			}
		}
		fclose(fp);
	}
	SetTE_WriteGroupInfo();
	return 0;
}
int		GS4Web_i::SetTE_WriteGroupInfo()
{
	TQUNIT_T*	pTQUnit	= NULL;
	whtimequeue::ID_T	id;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nQueryGroupInfoTimeOut, (void**)&pTQUnit, &id) < 0)
	{
		return -1;
	}
	pTQUnit->tefunc		= &GS4Web_i::TEDeal_WriteGroupInfo_TimeOut;
	return 0;
}
int	GS4Web_i::TEDeal_Deal_AdColony_TimeEvent_TimeOut(TQUNIT_T* pTQUnit)
{
	// 请求处理AdColony超时
	GS4WEB_DB_GET_ADCOLONY_REQ_T	ReqCmd;
	ReqCmd.nCmd					= P_GS4WEB_DB_REQ_CMD;
	ReqCmd.nSubCmd				= DB_GET_ADCOLONY_REQ;

	m_msgerDB.SendMsg(&ReqCmd, sizeof(ReqCmd));

	// 重新注册时间事件
	SetTE_DealAdColonyTimeEvent();
	return 0;
}
// 设置上面的超时
int	GS4Web_i::SetTE_DealAdColonyTimeEvent()
{
	TQUNIT_T*	pTQUnit	= NULL;
	whtimequeue::ID_T	teid;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nDealAdColonyTimeOut, (void**)&pTQUnit, &teid) < 0)
	{
		assert(0);
		return 0;
	}
	pTQUnit->tefunc			= &GS4Web_i::TEDeal_Deal_AdColony_TimeEvent_TimeOut;
	return 0;
}

