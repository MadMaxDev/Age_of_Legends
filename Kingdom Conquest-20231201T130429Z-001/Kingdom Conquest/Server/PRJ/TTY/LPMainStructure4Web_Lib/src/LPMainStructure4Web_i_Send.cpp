#include "../inc/LPMainStructure4Web_i.h"
using namespace n_pngs;

int		LPMainStructure4Web_i::SendCmdToPlayerCLS4Web(int nClientID, const void* pCmd, size_t nSize)
{
	return SendCmdToPlayerCLS4Web(m_Players.getptr(nClientID), pCmd, nSize);
}
int		LPMainStructure4Web_i::SendCmdToPlayerCLS4Web(PlayerUnit* pPlayer, const void* pCmd, size_t nSize)
{
	if (pPlayer == NULL)
	{
		return -1;
	}
	CLS4WebUnit*	pCLS4Web	= m_CLS4Webs.getptr(pPlayer->nCLS4WebID);
	if (pCLS4Web == NULL)
	{
		return -2;
	}
	if (pCLS4Web->nCntrID == 0)
	{
		return -3;
	}
	return SendCmdToConnecter(pCLS4Web->nCntrID, pCmd, nSize);
}
int		LPMainStructure4Web_i::SendCmdToPlayer(int nPlayerID, const void* pCmd, size_t nSize)
{
	return SendCmdToPlayer(m_Players.getptr(nPlayerID), pCmd, nSize);
}
int		LPMainStructure4Web_i::SendCmdToPlayer(PlayerUnit* pPlayer, const void* pCmd, size_t nSize)
{
	if (pPlayer == NULL)
	{
		return -1;
	}
	// 找到对应的CLS
	CLS4WebUnit*	pCLS4Web	= m_CLS4Webs.getptr(pPlayer->nCLS4WebID);
	// CLS4Web对象不会在玩家之前消失
	assert(pCLS4Web != NULL);
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS4Web_CLIENT_DATA_T, data)+nSize);
	SVR_CLS4Web_CLIENT_DATA_T*	pClientData	= (SVR_CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pClientData->nCmd		= SVR_CLS4Web_CLIENT_DATA;
	pClientData->nClientID		= pPlayer->nID;
	memcpy(pClientData->data, pCmd, nSize);
	if (SendCmdToConnecter(pCLS4Web->nCntrID, m_vectrawbuf.getbuf(), m_vectrawbuf.size()) < 0)
	{
		return -2;
	}
	return 0;
}
int		LPMainStructure4Web_i::SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize)
{
	whDList<CLS4WebUnit*>	dlCLS4Web;
	for (int i=0; i<nPlayerNum; i++)
	{
		PlayerUnit*	pPlayer	= m_Players.getptr(paPlayerID[i]);
		if (pPlayer != NULL)
		{
			CLS4WebUnit*	pCLS4WebUnit	= m_CLS4Webs.getptr(pPlayer->nCLS4WebID);
			if (pCLS4WebUnit != NULL)
			{
				pCLS4WebUnit->pVectClientID->push_back(paPlayerID[i]);
				if (!pCLS4WebUnit->dlnodeInSCTMP.isinlist())
				{
					dlCLS4Web.AddToTail(&pCLS4WebUnit->dlnodeInSCTMP);
				}
			}
		}
	}

	whDList<CLS4WebUnit*>::node*	pNode	= dlCLS4Web.begin();
	while (pNode != dlCLS4Web.end())
	{
		CLS4WebUnit*	pCLS4Web	= pNode->data;
		SendCmdToMultiPlayerInOneCLS4Web(pCLS4Web, pCLS4Web->pVectClientID->getbuf(), pCLS4Web->pVectClientID->size(), pCmd, nSize);
		pCLS4Web->pVectClientID->clear();
		pNode->leave();
		pNode	= dlCLS4Web.begin();
	}
	return 0;
}
int		LPMainStructure4Web_i::SendCmdToMultiPlayerInOneCLS4Web(int nCLS4WebID, int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize)
{
	return SendCmdToMultiPlayerInOneCLS4Web(m_CLS4Webs.getptr(nCLS4WebID), paPlayerID, nPlayerNum, pCmd, nSize);
}
int		LPMainStructure4Web_i::SendCmdToMultiPlayerInOneCLS4Web(CLS4WebUnit* pCLS4Web, int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize)
{
	if (pCLS4Web == NULL)
	{
		return -1;
	}

	m_vectrawbuf.resize(SVR_CLS4Web_MULTICLIENT_DATA_T::GetTotalSize(nPlayerNum, nSize));
	SVR_CLS4Web_MULTICLIENT_DATA_T&	Cmd	= *(SVR_CLS4Web_MULTICLIENT_DATA_T*)m_vectrawbuf.getbuf();
	Cmd.nCmd		= SVR_CLS4Web_MULTICLIENT_DATA;
	Cmd.nClientNum	= nPlayerNum;
	memcpy(Cmd.anClientID, paPlayerID, sizeof(int)*nPlayerNum);
	memcpy(Cmd.GetDataPtr(), pCmd, nSize);

	if (SendCmdToConnecter(pCLS4Web->nCntrID, m_vectrawbuf.getbuf(), m_vectrawbuf.size()) < 0)
	{
		return -2;
	}

	return 0;
}
int		LPMainStructure4Web_i::SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal)
{
	SVR_CLS4Web_SET_TAG_TO_CLIENT_T		Cmd;
	Cmd.nCmd		= SVR_CLS4Web_SET_TAG_TO_CLIENT;
	Cmd.nTagIdx		= nTagIdx;
	Cmd.nTagVal		= nTagVal;
	Cmd.nClientID	= nClientID;
	return SendCmdToPlayerCLS4Web(nClientID, &Cmd, sizeof(Cmd));
}
int		LPMainStructure4Web_i::SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, const void* pCmd, size_t nSize)
{
	// 直接发送给所有CLS4Web告诉他们按tag发送
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS4Web_TAGGED_CLIENT_DATA_T, data) + nSize);
	SVR_CLS4Web_TAGGED_CLIENT_DATA_T&	Cmd	= *(SVR_CLS4Web_TAGGED_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	Cmd.nCmd		= SVR_CLS4Web_TAGGED_CLIENT_DATA;
	Cmd.nTagIdx		= nTagIdx;
	Cmd.nTagVal		= nTagVal;
	memcpy(Cmd.data, pCmd, nSize);
	whunitallocatorFixed<CLS4WebUnit>::iterator	it	= m_CLS4Webs.begin();
	for (; it != m_CLS4Webs.end(); ++it)
	{
		SendCmdToConnecter((*it).nCntrID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return 0;
}
int		LPMainStructure4Web_i::SendCmdToAllPlayer(const void* pData, size_t nSize)
{
	// 直接发给所有CLS4Web,告诉他们转播
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS4Web_ALL_CLIENT_DATA_T, data) + nSize);
	SVR_CLS4Web_ALL_CLIENT_DATA_T&		Cmd	= *(SVR_CLS4Web_ALL_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	Cmd.nCmd		= SVR_CLS4Web_ALL_CLIENT_DATA;
	memcpy(Cmd.data, pData, nSize);
	whunitallocatorFixed<CLS4WebUnit>::iterator	it	= m_CLS4Webs.begin();
	for (; it != m_CLS4Webs.end(); ++it)
	{
		SendCmdToConnecter((*it).nCntrID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return 0;
}
int		LPMainStructure4Web_i::SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel)
{
	SVR_CLS4Web_SET_TAG64_TO_CLIENT_T	Cmd;
	Cmd.nCmd		= SVR_CLS4Web_SET_TAG64_TO_CLIENT;
	Cmd.nTag		= nTag;
	Cmd.nClientID	= nClientID;
	Cmd.bDel		= bDel;
	return SendCmdToPlayerCLS4Web(nClientID, &Cmd, sizeof(Cmd));
}
int		LPMainStructure4Web_i::SendCmdToAllPlayerByTag64(whuint64 nTag, const void* pCmd, size_t nSize)
{
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS4Web_TAGGED64_CLIENT_DATA_T, data) + nSize);
	SVR_CLS4Web_TAGGED64_CLIENT_DATA_T&	Cmd	= *(SVR_CLS4Web_TAGGED64_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	Cmd.nCmd		= SVR_CLS4Web_TAGGED64_CLIENT_DATA;
	Cmd.nTag		= nTag;
	memcpy(Cmd.data, pCmd, nSize);
	whunitallocatorFixed<CLS4WebUnit>::iterator	it	= m_CLS4Webs.begin();
	for (; it != m_CLS4Webs.end(); ++it)
	{
		SendCmdToConnecter((*it).nCntrID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return 0;
}
int		LPMainStructure4Web_i::SendCmdToConnecter(int nCntrID, const void* pData, int nDSize)
{
	PNGS_CD2TR_CMD_T	cmd;
	cmd.nConnecterID	= nCntrID;
	cmd.nDSize			= nDSize;
	cmd.pData			= (void*)pData;
	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicTCPReceiver, PNGS_CD2TR_CMD, cmd);
	return 0;
}
