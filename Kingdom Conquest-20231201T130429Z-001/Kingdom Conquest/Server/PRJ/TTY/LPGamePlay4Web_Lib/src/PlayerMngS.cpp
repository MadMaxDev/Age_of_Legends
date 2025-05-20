#include "../inc/PlayerMngS.h"
#include "../inc/GameMngS.h"

#include "WHNET/inc/whnetudpGLogger.h"

#include <vector>
using namespace std;

using namespace n_pngs;

int		CPlayerMngS::Init(int nMaxPlayer,CGameMngS *pHost)
{
	m_pHost	= pHost;
	if (m_Players.Init(nMaxPlayer) < 0)
	{
		return -1;
	}
	// 初始化时间队列
	whtimequeue::INFO_T		tqinfo;
	tqinfo.nUnitLen			= sizeof(TQUNIT_T);
	tqinfo.nChunkSize		= m_pHost->GetCfgInfo()->nTQChunkSize;
	if (m_TQ.Init(&tqinfo) < 0)
	{
		return -2;
	}

	m_nTickNow			= wh_gettickcount();
	return 0;
}
PlayerData*	CPlayerMngS::AddPlayer(int nPlayerGID)
{
	PlayerData*	pPlayer	= m_Players.AllocUnitByID(nPlayerGID);
	if (pPlayer != NULL)
	{
		SetTE_RefreshMail(nPlayerGID);
	}
	return pPlayer;
}
void	CPlayerMngS::DeletePlayer(int nPlayerGID)
{
	map_playergid2teid::iterator	it	= m_mapPlayerGID2TEID.find(nPlayerGID);
	if (it != m_mapPlayerGID2TEID.end())
	{
		delete it->second;
		m_mapPlayerGID2TEID.erase(it);
	}
	PlayerData*	pPlayer	= m_Players.getptr(nPlayerGID);
	if (pPlayer != NULL)
	{
		pPlayer->Clear();
		m_Players.FreeUnitByPtr(pPlayer);
	}
}
PlayerData*	CPlayerMngS::GetPlayerByGID(int nPlayerGID)
{
	return m_Players.getptr(nPlayerGID);
}
int			CPlayerMngS::Tick_DealTE()
{
	m_nTickNow	= wh_gettickcount();
	// 看看事件队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T*			pUnit = NULL;
	while (m_TQ.GetUnitBeforeTime(m_nTickNow, (void**)&pUnit, &id) == 0)
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);
		m_TQ.Del(id);
	}

	return 0;
}
void		CPlayerMngS::TEDeal_RefreshMail_TimeOut(CPlayerMngS::TQUNIT_T* pUnit)
{
	PlayerData*	pPlayer	= GetPlayerByGID(pUnit->nPlayerGID);
	if (pPlayer != NULL)
	{
		// 向数据库要一次数据
		P_DBS4WEB_REFRESH_MAIL_T	Cmd;
		Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
		Cmd.nSubCmd		= CMDID_REFRESH_MAIL_REQ;
		Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
		Cmd.nMailID		= pPlayer->GetMaxMailID();
		Cmd.nExt[0]		= pUnit->nPlayerGID;
		g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

		SetTE_RefreshMail(pUnit->nPlayerGID);
	}
}
void		CPlayerMngS::SetTE_RefreshMail(unsigned int nPlayerGID)
{
	TQUNIT_T*	pTQUnit	= NULL;
	unsigned int	nTimeOut	= m_pHost->GetCfgInfo()->nRefreshMailTime;
	whtimequeue::ID_T*	pTEID	= NULL;
	map_playergid2teid::iterator	it	= m_mapPlayerGID2TEID.find(nPlayerGID);
	if (it == m_mapPlayerGID2TEID.end())
	{
		pTEID					= new whtimequeue::ID_T;
		m_mapPlayerGID2TEID.insert(map_playergid2teid::value_type(nPlayerGID, pTEID));
	}
	else
	{
		pTEID					= it->second;
	}
	if (m_TQ.AddGetRef(m_nTickNow+nTimeOut, (void**)&pTQUnit, pTEID) < 0)
	{
		return;
	}
	pTQUnit->tefunc		= &CPlayerMngS::TEDeal_RefreshMail_TimeOut;
	pTQUnit->nPlayerGID	= nPlayerGID;
}

