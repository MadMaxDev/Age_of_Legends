#include "../inc/LPMainStructure4Web_i.h"

using namespace n_pngs;

int		LPMainStructure4Web_i::RemovePlayerUnit(int nID, unsigned char nRemoveReason/* =CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING */)
{
	return RemovePlayerUnit( m_Players.getptr(nID), nRemoveReason );
}
int		LPMainStructure4Web_i::RemovePlayerUnit(PlayerUnit* pPlayer, unsigned char nRemoveReason/* =CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING */)
{
	if (pPlayer == NULL)
	{
		return -1;
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,0x%X,0x%X,%d,%d,%d", __FUNCTION__, pPlayer->nID, pPlayer->nClientIDInCAAFS4Web, pPlayer->nStatus, nRemoveReason, m_Players.size());

	// 判断用户是否已经GP在线了(如果用户还在CAAFS4Web中则没有GP在线)
	bool	bGPOnline	= true;
	switch (pPlayer->nStatus)
	{
	case PlayerUnit::STATUS_WANTCLS4Web:
	case PlayerUnit::STATUS_GOINGTOCLS4Web:
		{
			bGPOnline	= false;
		}
		break;
	default:	// 没有nothing状态的,一经分配就会是want cls
		break;
	}
	// 通过player找到cls再找caafs(cls id肯定存在的)
	CLS4WebUnit*	pCLS4Web	= m_CLS4Webs.getptr(pPlayer->nCLS4WebID);
	assert(pCLS4Web != NULL);
	CAAFS4WebGroup*	pCAAFS4Web	= GetCAAFS4WebByGroupID(pCLS4Web->nGroupID);
	if (pCAAFS4Web!=NULL && !bGPOnline)
	{
		LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T	ClientCanGo;
		ClientCanGo.nCmd					= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web;
		ClientCanGo.nClientIDInCAAFS4Web	= pPlayer->nClientIDInCAAFS4Web;
		ClientCanGo.nRst					= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T::RST_ERR_UNKNOWN;
		SendCmdToConnecter(pCAAFS4Web->nCntrID, &ClientCanGo, sizeof(ClientCanGo));
	}

	// 需要通知上层玩家离开了
	if (bGPOnline)
	{
		LPPACKET_2GP_PLAYEROFFLINE_T	CmdPlayerOffline;
		CmdPlayerOffline.nClientID		= pPlayer->nID;
		CmdPlayerOffline.nReason		= nRemoveReason;
		CmdOutToLogic_AUTO(m_pLogicGamePlay, LPPACKET_2GP_PLAYEROFFLINE, &CmdPlayerOffline, sizeof(CmdPlayerOffline));
	}
	// 清除数据
	pPlayer->clear();
	m_Players.FreeUnitByPtr(pPlayer);

	return 0;
}
