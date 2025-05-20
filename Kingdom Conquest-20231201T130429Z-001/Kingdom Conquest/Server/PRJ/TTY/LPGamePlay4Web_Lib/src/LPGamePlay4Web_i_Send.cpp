#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

int		LPGamePlay4Web_i::SendCmdToClient(int nClientID, const void* pCmd, size_t nSize)
{
	LPPACKET_2MS_CMD2ONEPLAYER_T	Cmd2MS;
	Cmd2MS.nClientID				= nClientID;
	Cmd2MS.nDSize					= nSize;
	Cmd2MS.pData					= (void*)pCmd;
	return CMN_LOGIC_CMDIN_AUTO(this, m_pLogicMainStructure, LPPACKET_2MS_CMD2ONEPLAYER, Cmd2MS);
}
int		LPGamePlay4Web_i::SendCmdToMultiClient(int* aClientID, int nClientNum, const void* pCmd, size_t nSize)
{
	LPPACKET_2MS_CMD2MULTIPLAYER_T	Cmd2MS;
	Cmd2MS.nClientNum				= nClientNum;
	Cmd2MS.paClientID				= aClientID;
	Cmd2MS.nDSize					= nSize;
	Cmd2MS.pData					= (void*)pCmd;
	return CMN_LOGIC_CMDIN_AUTO(this, m_pLogicMainStructure, LPPACKET_2MS_CMD2MULTIPLAYER, Cmd2MS);
}
int		LPGamePlay4Web_i::SendCmdToClientWithTag(unsigned char nTagIdx, short nTagVal, const void* pCmd, size_t nSize)
{
	LPPACKET_2MS_CMD2PLAYERWITHTAG_T	Cmd2MS;
	Cmd2MS.nTagIdx					= nTagIdx;
	Cmd2MS.nTagVal					= nTagVal;
	Cmd2MS.nDSize					= nSize;
	Cmd2MS.pData					= (void*)pCmd;
	return CMN_LOGIC_CMDIN_AUTO(this, m_pLogicMainStructure, LPPACKET_2MS_CMD2PLAYERWITHTAG, Cmd2MS);
}
int		LPGamePlay4Web_i::SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal)
{
	LPPACKET_2MS_SETPLAYERTAG_T		Cmd2MS;
	Cmd2MS.nClientID		= nClientID;
	Cmd2MS.nTagIdx			= nTagIdx;
	Cmd2MS.nTagVal			= nTagVal;
	return CmdOutToLogic_AUTO(m_pLogicMainStructure, LPPACKET_2MS_SETPLAYERTAG, &Cmd2MS, sizeof(Cmd2MS));
}
int		LPGamePlay4Web_i::SendCmdToClientWithTag64(whuint64 nTag, const void* pCmd, size_t nSize)
{
	LPPACKET_2MS_CMD2PLAYERWITHTAG64_T	Cmd2MS;
	Cmd2MS.nTag				= nTag;
	Cmd2MS.nDSize			= nSize;
	Cmd2MS.pData			= (void*)pCmd;
	return CMN_LOGIC_CMDIN_AUTO(this, m_pLogicMainStructure, LPPACKET_2MS_CMD2PLAYERWITHTAG64, Cmd2MS);
}
int		LPGamePlay4Web_i::SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel)
{
	LPPACKET_2MS_SETPLAYERTAG64_T	Cmd2MS;
	Cmd2MS.nClientID		= nClientID;
	Cmd2MS.nTag				= nTag;
	Cmd2MS.bDel				= bDel;
	return CmdOutToLogic_AUTO(m_pLogicMainStructure, LPPACKET_2MS_SETPLAYERTAG64, &Cmd2MS, sizeof(Cmd2MS));
}
int		LPGamePlay4Web_i::SendCmdToAllClient(const void* pCmd, size_t nSize)
{
	LPPACKET_2MS_CMD2ALLPLAYER_T	Cmd2MS;
	Cmd2MS.nDSize			= nSize;
	Cmd2MS.pData			= (void*)pCmd;
	return CMN_LOGIC_CMDIN_AUTO(this, m_pLogicMainStructure, LPPACKET_2MS_CMD2ALLPLAYER, Cmd2MS);
}
int		LPGamePlay4Web_i::SendCmdToDB(const void* pData, size_t nSize)
{
	return CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, pData, nSize);
}
int		LPGamePlay4Web_i::SendReplaceWarningToPlayer(int nClientID, unsigned int nFromIP)
{
	TTY_LPGAMEPLAY_CLIENT_NOTIFY_OTHERTRYREPLACE_T	Cmd;
	Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_NOTIFY;
	Cmd.nWhat		= WEB_NOTIFY_WHAT_OTHERTRYREPLACE;
	Cmd.nFromIP		= nFromIP;

	SendCmdToClient(nClientID, &Cmd, sizeof(Cmd));

	return 0;
}
void	LPGamePlay4Web_i::SendPlayerCharAtbToClient(PlayerData* pPlayer, bool bSimple/* =true */)
{
// 	STC_GAMECMD_GET_CHAR_ATB_T	Cmd;
// 	Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 	Cmd.nGameCmd	= STC_GAMECMD_GET_CHAR_ATB;
// 	memcpy(&Cmd.charAtb, &pPlayer->m_CharAtb, sizeof(Cmd.charAtb));

	if (bSimple)
	{
		STC_GAMECMD_GET_IMPORTANT_CHAR_ATB_T	Cmd;
		Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
		Cmd.nGameCmd				= STC_GAMECMD_GET_IMPORTANT_CHAR_ATB;
		Cmd.nCrystal				= pPlayer->m_CharAtb.nCrystal;
		Cmd.nDevelopment			= pPlayer->m_CharAtb.nDevelopment;
		Cmd.nDiamond				= pPlayer->m_CharAtb.nDiamond;
		Cmd.nExp					= pPlayer->m_CharAtb.nExp;
		Cmd.nGold					= pPlayer->m_CharAtb.nGold;
		Cmd.nPopulation				= pPlayer->m_CharAtb.nPopulation;
		Cmd.nLevel					= pPlayer->m_CharAtb.nLevel;
		unsigned int nTimeNow		= wh_time();
		Cmd.nProtectTime			= nTimeNow>pPlayer->m_CharAtb.nProtectTimeEndTick?0:pPlayer->m_CharAtb.nProtectTimeEndTick-nTimeNow;
		Cmd.nAddBuildNumTime		= nTimeNow>pPlayer->m_CharAtb.nAddBuildNumTimeEndTick?0:pPlayer->m_CharAtb.nAddBuildNumTimeEndTick-nTimeNow;
		Cmd.nTotalBuildNum			= pPlayer->m_CharAtb.nTotalBuildNum;
		Cmd.nCup					= pPlayer->m_CharAtb.nCup;
		SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
	}
	else
	{
		STC_GAMECMD_GET_CHAR_ATB_T	Cmd;
		Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
		Cmd.nGameCmd	= STC_GAMECMD_GET_CHAR_ATB;
		unsigned int nTimeNow			= wh_time();
		pPlayer->m_CharAtb.nProtectTime		= nTimeNow>pPlayer->m_CharAtb.nProtectTimeEndTick?0:pPlayer->m_CharAtb.nProtectTimeEndTick-nTimeNow;
		pPlayer->m_CharAtb.nAddBuildNumTime	= nTimeNow>pPlayer->m_CharAtb.nAddBuildNumTimeEndTick?0:pPlayer->m_CharAtb.nAddBuildNumTimeEndTick-nTimeNow;
		memcpy(&Cmd.charAtb, &pPlayer->m_CharAtb, sizeof(Cmd.charAtb));
		SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
	}
}
void	LPGamePlay4Web_i::RefreshPlayerCharAtbFromDB(const char* cszAccountIDs, bool bSimple, bool bCheckOnline)
{
	whvector<tty_id_t>	vectAccountIDs;
	tty_id_t	nAccountID	= 0;
	int			nOffset	= 0;
	while (wh_strsplit(&nOffset, "I", cszAccountIDs, ",", &nAccountID) == 1)
	{
		if (nAccountID != 0)
		{
			vectAccountIDs.push_back(nAccountID);
		}
	}
	RefreshPlayerCharAtbFromDB(vectAccountIDs.size(), vectAccountIDs.getbuf(), bSimple, bCheckOnline);
}
void	LPGamePlay4Web_i::RefreshPlayerCharAtbFromDB(unsigned int nNum, tty_id_t* pAccountIDs, bool bSimple, bool bCheckOnline)
{
	for (int i=0; i<nNum; i++)
	{
		RefreshPlayerCharAtbFromDB(pAccountIDs[i], bSimple, bCheckOnline);
	}
}
void	LPGamePlay4Web_i::RefreshPlayerCharAtbFromDB(tty_id_t nAccountID, bool bSimple, bool bCheckOnline)
{
	if (nAccountID == 0)
	{
		return;
	}

	if (bCheckOnline)
	{
		PlayerUnit*	pPlayer	= GetPlayerByAccountID(nAccountID);
		if (pPlayer == NULL)
		{
			return;
		}
	}

	P_DBS4WEB_REFRESH_CHAR_ATB_T	Cmd;
	Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd		= CMDID_REFRESH_CHAR_ATB_REQ;
	Cmd.nAccountID	= nAccountID;
	Cmd.bSimple		= bSimple;

	SendCmdToDB(&Cmd, sizeof(Cmd));
}
void	LPGamePlay4Web_i::GetHeroSimpleData(unsigned int nCombatType, tty_id_t nExtID, const char* cszHeroIDs, unsigned int nReason)
{
	whvector<tty_id_t>	vectHeroIDs;
	tty_id_t	nHeroID	= 0;
	int			nOffset	= 0;
	while (wh_strsplit(&nOffset, "I", cszHeroIDs, ",", &nHeroID) == 1)
	{
		if (nHeroID != 0)
		{
			vectHeroIDs.push_back(nHeroID);
		}
	}
	GetHeroSimpleData(nCombatType, nExtID, vectHeroIDs.size(), vectHeroIDs.getbuf(), nReason);
}
void	LPGamePlay4Web_i::GetHeroSimpleData(unsigned int nCombatType, tty_id_t nExtID, unsigned int nNum, tty_id_t*	pHeroIDs, unsigned int nReason)
{
	if (nNum == 0)
	{
		STC_GAMECMD_HERO_SIMPLE_DATA_T	Cmd;
		Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
		Cmd.nGameCmd		= STC_GAMECMD_HERO_SIMPLE_DATA;
		Cmd.nReason			= nReason;
		Cmd.nCombatType		= nCombatType;
		Cmd.nNum			= 0;
		switch (nCombatType)
		{
		case combat_type_alli_instance:
			{
				SendCmdToClientWithTag64(nExtID, &Cmd, sizeof(Cmd));
			}
			break;
		case combat_type_arena:
			{
				PlayerUnit*	pPlayer	= GetPlayerByAccountID(nExtID);
				SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
			}
			break;
		}
		return;
	}

	m_vectrawbuf.resize(sizeof(P_DBS4WEB_GET_HERO_SIMPLE_DATA_T) + nNum*sizeof(tty_id_t));
	P_DBS4WEB_GET_HERO_SIMPLE_DATA_T*	pReq	= (P_DBS4WEB_GET_HERO_SIMPLE_DATA_T*)m_vectrawbuf.getbuf();
	pReq->nCmd		= P_DBS4WEB_REQ_CMD;
	pReq->nSubCmd	= CMDID_GET_HERO_SIMPLE_DATA_REQ;
	pReq->nReason	= nReason;
	pReq->nCombatType		= nCombatType;
	pReq->nID		= nExtID;
	pReq->nNum		= nNum;
	tty_id_t*	pID	= (tty_id_t*)wh_getptrnexttoptr(pReq);
	for (int i=0; i<nNum; i++)
	{
		*pID		= pHeroIDs[i];
		pID++;
	}
	SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}
void	LPGamePlay4Web_i::RefreshDrug(const char* cszAccountIDs)
{
	whvector<tty_id_t>	vectAccountIDs;
	tty_id_t	nAccountID	= 0;
	int			nOffset	= 0;
	while (wh_strsplit(&nOffset, "I", cszAccountIDs, ",", &nAccountID) == 1)
	{
		if (nAccountID != 0)
		{
			vectAccountIDs.push_back(nAccountID);
		}
	}
	RefreshDrug(vectAccountIDs.size(), vectAccountIDs.getbuf());
}
void	LPGamePlay4Web_i::RefreshDrug(unsigned int nNum, tty_id_t* pAccountIDs)
{
	if (nNum == 0)
	{
		return;
	}

	char	szBuf[4096]	= "";
	int		nLength		= sprintf(szBuf, "0x%"WHINT64PRFX"X", pAccountIDs[0]);
	for (int i=1; i<nNum; i++)
	{
		nLength			+= sprintf(szBuf+nLength, ",0x%"WHINT64PRFX"X", pAccountIDs[i]);
	}
	szBuf[nLength]		= 0;
	
	m_vectrawbuf.resize(sizeof(P_DBS4WEB_REFRESH_DRUG_T) + nLength+1);
	P_DBS4WEB_REFRESH_DRUG_T*	pReq	= (P_DBS4WEB_REFRESH_DRUG_T*)m_vectrawbuf.getbuf();
	pReq->nCmd			= P_DBS4WEB_REQ_CMD;
	pReq->nSubCmd		= CMDID_REFRESH_DRUG_REQ;
	memcpy(wh_getptrnexttoptr(pReq), szBuf, nLength+1);
	SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}
int		LPGamePlay4Web_i::AddMail(tty_id_t nSenderID, const char*	cszSenderName, tty_id_t nReceiverID, bool bNeedRst, unsigned char nType, unsigned char nFlag, bool bSendLimit, int nTextLen, const char* cszText, int nExtDataLen, void* pExtData, int pAddedExtData[PNGSPACKET_2DB4WEB_EXT_NUM])
{
	m_vectrawbuf.resize(wh_offsetof(P_DBS4WEB_ADD_PRIVATE_MAIL_T, szText) + (nTextLen+1) + sizeof(int) + nExtDataLen);
	P_DBS4WEB_ADD_PRIVATE_MAIL_T* pPrvMail	= (P_DBS4WEB_ADD_PRIVATE_MAIL_T*)m_vectrawbuf.getbuf();
	pPrvMail->nCmd		= P_DBS4WEB_REQ_CMD;
	pPrvMail->nSubCmd	= CMDID_ADD_PRIVATE_MAIL_REQ;
	pPrvMail->bNeedRst	= bNeedRst;
	pPrvMail->nSenderID	= nSenderID;
	WH_STRNCPY0(pPrvMail->szSender, cszSenderName);
	pPrvMail->nReceiverID	= nReceiverID;
	pPrvMail->nType		= nType;
	pPrvMail->nFlag		= nFlag;
	pPrvMail->bSendLimit= bSendLimit;
	pPrvMail->nTextLen	= nTextLen+1;
	memcpy(pPrvMail->szText, cszText, nTextLen);
	pPrvMail->szText[nTextLen]	= 0;
	int*	pExtDataLen	= (int*)wh_getoffsetaddr(pPrvMail, wh_offsetof(P_DBS4WEB_ADD_PRIVATE_MAIL_T, szText) + nTextLen+1);
	*pExtDataLen		= nExtDataLen;
	if (nExtDataLen > 0)
	{
		void*	pExtDataNew	= wh_getptrnexttoptr(pExtDataLen);
		memcpy(pExtDataNew, pExtData, nExtDataLen);
	}
	if (pAddedExtData == NULL)
	{
		memset(pPrvMail->nExt, 0, sizeof(pPrvMail->nExt));
	}
	else
	{
		memcpy(pPrvMail->nExt, pAddedExtData, sizeof(pPrvMail->nExt));
	}
	SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());

	return 0;
}
int		LPGamePlay4Web_i::RechargeDiamond(tty_id_t nAccountID, unsigned int nAddedDiamond, bool bFromGS/* =false */, const int* pExt/* =NULL */)
{
	P_DBS4WEB_RECHARGE_DIAMOND_T	Cmd;
	Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd			= CMDID_RECHARGE_DIAMOND_REQ;
	Cmd.nAccountID		= nAccountID;
	Cmd.nAddedDiamond	= nAddedDiamond;
	Cmd.bFromGS			= bFromGS;
	memcpy(Cmd.nExt, pExt, sizeof(Cmd.nExt));

	SendCmdToDB(&Cmd, sizeof(Cmd));
	return 0;
}
int		LPGamePlay4Web_i::SendAlliInstanceNotify(tty_id_t nInstanceID, tty_id_t nAccountID, int nAction)
{
	STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T	Cmd;
	Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
	Cmd.nGameCmd		= STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY;
	Cmd.nAccountID		= nAccountID;
	Cmd.nInstanceID		= nInstanceID;
	Cmd.nAction			= nAction;
	SendCmdToClientWithTag64(nInstanceID, &Cmd, sizeof(Cmd));

	return 0;
}
