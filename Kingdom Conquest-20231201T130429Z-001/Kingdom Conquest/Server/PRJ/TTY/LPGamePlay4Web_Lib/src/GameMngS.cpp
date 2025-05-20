#include "../inc/GameMngS.h"
#include <math.h>

using namespace n_pngs;

WHDATAPROP_MAP_BEGIN_AT_ROOT(CGameMngS::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nNotificationSendToClientNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nGroupPlayerTotalNumExpect, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nWorldXMax, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nWorldYMax, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nAllianceNumExpect, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nStoredChatEntryNum, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSendChatEntryNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMBDir, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szRankFileDir, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nRefreshMailTime, 0)
WHDATAPROP_MAP_END()

CGameMngS::CGameMngS()
: m_pPlayerMng(NULL)
, m_pNotificationMng(NULL)
, m_pPlayerCardMng(NULL)
, m_pWorldMng(NULL)
, m_pAllianceMng(NULL)
, m_pChatMng(NULL)
, m_pLotteryMng(NULL)
, m_pRankMng(NULL)
, m_pInstanceMng(NULL)
{
}

CGameMngS::~CGameMngS()
{
	WHSafeDelete(m_pPlayerMng);
	WHSafeDelete(m_pNotificationMng);
	WHSafeDelete(m_pPlayerCardMng);
	WHSafeDelete(m_pWorldMng);
	WHSafeDelete(m_pAllianceMng);
	WHSafeDelete(m_pChatMng);
	WHSafeDelete(m_pLotteryMng);
	WHSafeDelete(m_pRankMng);
	WHSafeDelete(m_pInstanceMng);
}

int		CGameMngS::Init(CFGINFO_T* pInfo)
{
	m_pInfo			= pInfo;

	int		nRst	= 0;

	// 这个需要最早初始化
	if (!g_ExcelTableMng.LoadTables(m_pInfo->szMBDir, 1))
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"g_ExcelTableMng.LoadTables,%s,%d", m_pInfo->szMBDir, nRst);
		return -8;
	}

	m_pPlayerMng	= new CPlayerMngS;
	if ((nRst=m_pPlayerMng->Init(pInfo->nMaxPlayer,this)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pPlayerMng init,%d", nRst);
		return -1;
	}

	m_pNotificationMng	= new CNotificationMngS(this);
	if( m_pNotificationMng == NULL )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pNotificationMng is NULL");
		return -2;
	}

	m_pPlayerCardMng	= new CPlayerCardMngS();
	if ((nRst=m_pPlayerCardMng->Init(this)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pPlayerCardMng init,%d", nRst);
		return -3;
	}

	m_pWorldMng			= new CWorldMngS();
	if ((nRst=m_pWorldMng->Init(this)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pWorldMng init,%d", nRst);
		return -4;
	}

	m_pAllianceMng		= new CAllianceMngS();
	if ((nRst=m_pAllianceMng->Init(this)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pAllianceMng init,%d", nRst);
		return -5;
	}

	whtimequeue::INFO_T	tqInfo;
	tqInfo.nChunkSize	= m_pInfo->nTQChunkSize;
	tqInfo.nUnitLen		= sizeof(TQUNIT_T);
	if ((nRst=m_TQ.Init(&tqInfo)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_TQ init,%d", nRst);
		return -6;
	}

	m_pExcelTextMng		= new CExcelTextMngS();

	m_pChatMng			= new CChatMngS();
	if ((nRst=m_pChatMng->Init(this)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pChatMng init,%d", nRst);
		return -7;
	}

	m_pLotteryMng		= new CLotteryMngS();
	if ((nRst=m_pLotteryMng->Init(this)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pLotteryMng->Init,%d", nRst);
		return -9;
	}

	m_pRankMng			= new CRankMngS();
	if ((nRst=m_pRankMng->Init(this)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pRankMng->Init,%d", nRst);
		return -10;
	}

	m_pInstanceMng		= new CInstanceMngS();
	if ((nRst=m_pInstanceMng->Init(this)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS_TMP_LOG)"m_pInstanceMng->Init,%d", nRst);
		return -11;
	}

	m_vectrawbuf.reserve(10*1024);
	m_vectrawbuf_city.reserve(10*1024);
	m_vectrawbuf_terrain.reserve(10*1024);
	m_vectrawbuf_worldres.reserve(10*1024);

	m_tickNow		= wh_gettickcount();

	return 0;
}
void	CGameMngS::Logic()
{
	m_tickNow		= wh_gettickcount();

	m_pPlayerMng->Tick_DealTE();
}
int		CGameMngS::ReloadExcelTable()
{
	m_pLotteryMng->ReloadExcelTable();
	m_pWorldMng->ReloadExcelTable();
	return 0;
}
void	CGameMngS::Receive(int nCmd, const void* pData, size_t nDSize)
{
	switch (nCmd)
	{
	case LPPACKET_2GP_PLAYERCMD:
		{
			LPPACKET_2GP_PLAYERCMD_T*	pPlayerCmd	= (LPPACKET_2GP_PLAYERCMD_T*)pData;
			PlayerData*					pPlayer		= m_pPlayerMng->GetPlayerByGID(pPlayerCmd->nClientID);
			if (pPlayer == NULL)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GAMECMD)"%s,player not exist,0x%X", __FUNCTION__, pPlayerCmd->nClientID);
				return;
			}
			// 处理PLAYERCMD
			TTY_CLIENT_LPGAMEPLAY_GAME_CMD_T*	pGameCmd	= (TTY_CLIENT_LPGAMEPLAY_GAME_CMD_T*)pPlayerCmd->pData;
			if (pGameCmd->nCmd != TTY_CLIENT_LPGAMEPLAY_GAME_CMD)
			{
				// 记录错误日志
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GAMECMD)"not gamecmd,%s,%d", __FUNCTION__, pGameCmd->nCmd);
				return;
			}
			else
			{
				switch (pGameCmd->nGameCmd)
				{
				case CTS_GAMECMD_CHAT_PRIVATE:
					{
						CTS_GAMECMD_CHAT_PRIVATE_T*	pChat	= (CTS_GAMECMD_CHAT_PRIVATE_T*)pGameCmd;
// 						if (pChat->nPeerAccountID == pPlayer->m_CharAtb.nAccountID)
// 						{
// 							// 不能和自己聊天
// 							break;
// 						}
						PlayerData*	pPeer	= g_pLPNet->GetPlayerData(pChat->nPeerAccountID);
						if (pPeer != NULL)
						{
							m_vectrawbuf.resize(wh_offsetof(STC_GAMECMD_CHAT_PRIVATE_T, szText) + pChat->nTextLen);
							STC_GAMECMD_CHAT_PRIVATE_T*	pChatTo	= (STC_GAMECMD_CHAT_PRIVATE_T*)m_vectrawbuf.getbuf();
							pChatTo->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							pChatTo->nGameCmd	= STC_GAMECMD_CHAT_PRIVATE;
							pChatTo->nChatID	= m_pChatMng->NewChatID();
							pChatTo->nFromAccountID	= pPlayer->m_CharAtb.nAccountID;
							WH_STRNCPY0(pChatTo->szFrom, pPlayer->m_CharAtb.szName);
							pChatTo->nToAccountID	= pPeer->m_CharAtb.nAccountID;
							WH_STRNCPY0(pChatTo->szTo, pPeer->m_CharAtb.szName);
							pChatTo->nTime		= wh_time();
							pChatTo->nVip		= pPlayer->GetVip();
							pChatTo->nTextLen	= pChat->nTextLen;
							memcpy(pChatTo->szText, pChat->szText, pChatTo->nTextLen);
							g_pLPNet->SendCmdToClient(pPeer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
						else
						{
							// 通知该玩家不在线
							STC_GAMECMD_CHAR_IS_ONLINE_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_CHAR_IS_ONLINE;
							Cmd.nAccountID		= pChat->nPeerAccountID;
							Cmd.bOnline			= false;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_CHAT_ALLIANCE:
					{
						if (pPlayer->m_CharAtb.nAllianceID != 0)
						{
							CTS_GAMECMD_CHAT_ALLIANCE_T*	pChat	= (CTS_GAMECMD_CHAT_ALLIANCE_T*)pGameCmd;
							m_vectrawbuf.resize(wh_offsetof(STC_GAMECMD_CHAT_ALLIANCE_T, szText) + pChat->nTextLen);
							STC_GAMECMD_CHAT_ALLIANCE_T*	pChatTo	= (STC_GAMECMD_CHAT_ALLIANCE_T*)m_vectrawbuf.getbuf();
							pChatTo->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							pChatTo->nGameCmd	= STC_GAMECMD_CHAT_ALLIANCE;
							pChatTo->nChatID	= m_pChatMng->NewChatID();
							pChatTo->nAccountID	= pPlayer->m_CharAtb.nAccountID;
							pChatTo->nTime		= wh_time();
							pChatTo->nVip		= pPlayer->GetVip();
							pChatTo->nTextLen	= pChat->nTextLen;
							WH_STRNCPY0(pChatTo->szFrom, pPlayer->m_CharAtb.szName);
							memcpy(pChatTo->szText, pChat->szText, pChatTo->nTextLen);
							g_pLPNet->SendCmdToClientWithTag64(pPlayer->m_CharAtb.nAllianceID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
							m_pChatMng->AddAllianceChatEntry(pPlayer->m_CharAtb.nAllianceID, m_vectrawbuf.size(), m_vectrawbuf.getbuf(), pChatTo->nTime);
						}
					}
					break;
				case CTS_GAMECMD_CHAT_WORLD:
					{
						CTS_GAMECMD_CHAT_WORLD_T*	pChat	= (CTS_GAMECMD_CHAT_WORLD_T*)pGameCmd;
						m_vectrawbuf.resize(wh_offsetof(STC_GAMECMD_CHAT_WORLD_T, szText) + pChat->nTextLen);
						STC_GAMECMD_CHAT_WORLD_T*	pChatTo	= (STC_GAMECMD_CHAT_WORLD_T*)m_vectrawbuf.getbuf();
						pChatTo->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pChatTo->nGameCmd	= STC_GAMECMD_CHAT_WORLD;
						pChatTo->nChatID	= m_pChatMng->NewChatID();
						pChatTo->nAccountID	= pPlayer->m_CharAtb.nAccountID;
						pChatTo->nTime		= wh_time();
						pChatTo->nVip		= pPlayer->GetVip();
						pChatTo->nTextLen	= pChat->nTextLen;
						WH_STRNCPY0(pChatTo->szFrom, pPlayer->m_CharAtb.szName);
						memcpy(pChatTo->szText, pChat->szText, pChatTo->nTextLen);
						g_pLPNet->SendCmdToAllClient(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						m_pChatMng->AddWorldChatEntry(m_vectrawbuf.size(), m_vectrawbuf.getbuf(), pChatTo->nTime);
					}
					break;
				case CTS_GAMECMD_CHAT_GET_RECENT_CHAT:
					{
						m_pChatMng->GetRecentChat(pPlayer);
					}
					break;
				case CTS_GAMECMD_CHAT_GROUP:
					{
						if (pPlayer->m_nGroupID == 0)
						{
							return;
						}
						CTS_GAMECMD_CHAT_GROUP_T*	pChat	= (CTS_GAMECMD_CHAT_GROUP_T*)pGameCmd;
						m_vectrawbuf.resize(wh_offsetof(STC_GAMECMD_CHAT_GROUP_T, szText) + pChat->nTextLen);
						STC_GAMECMD_CHAT_GROUP_T*	pChatTo	= (STC_GAMECMD_CHAT_GROUP_T*)m_vectrawbuf.getbuf();
						pChatTo->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pChatTo->nGameCmd	= STC_GAMECMD_CHAT_GROUP;
						pChatTo->nChatID	= m_pChatMng->NewChatID();
						pChatTo->nAccountID	= pPlayer->m_CharAtb.nAccountID;
						pChatTo->nTime		= wh_time();
						pChatTo->nVip		= pPlayer->GetVip();
						pChatTo->nTextLen	= pChat->nTextLen;
						WH_STRNCPY0(pChatTo->szFrom, pPlayer->m_CharAtb.szName);
						memcpy(pChatTo->szText, pChat->szText, pChatTo->nTextLen);
						g_pLPNet->SendCmdToClientWithTag64(pPlayer->m_nGroupID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}
					break;
				case CTS_GAMECMD_GET_NOTIFICATION:
					{
						m_pNotificationMng->SendNotificationToPlayer(pPlayer);
					}
					break;
				case CTS_GAMECMD_GET_PLAYERCARD:
					{
						CTS_GAMECMD_GET_PLAYERCARD_T*	pGetCard	= (CTS_GAMECMD_GET_PLAYERCARD_T*)pGameCmd;
						STC_GAMECMD_GET_PLAYERCARD_T	RplCmd;
						RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						RplCmd.nGameCmd		= STC_GAMECMD_GET_PLAYERCARD;
						RplCmd.nRst			= STC_GAMECMD_GET_PLAYERCARD_T::RST_OK;
						PlayerCard*	pCard	= m_pPlayerCardMng->GetCardByAccountID(pGetCard->nAccountID);
						if (pCard == NULL)
						{
							RplCmd.nRst		= STC_GAMECMD_GET_PLAYERCARD_T::RST_NOTEXIST;
						}
						else
						{
							memcpy(&RplCmd.card, pCard, sizeof(PlayerCard));
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
					}
					break;
				case CTS_GAMECMD_GET_PLAYERCARD_POS:
					{
						CTS_GAMECMD_GET_PLAYERCARD_POS_T*	pGetCard	= (CTS_GAMECMD_GET_PLAYERCARD_POS_T*)pGameCmd;
						STC_GAMECMD_GET_PLAYERCARD_T	RplCmd;
						RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						RplCmd.nGameCmd		= STC_GAMECMD_GET_PLAYERCARD;
						RplCmd.nRst			= STC_GAMECMD_GET_PLAYERCARD_T::RST_OK;
						PlayerCard*	pCard	= m_pPlayerCardMng->GetCardByPosition(pGetCard->nPosX, pGetCard->nPosY);
						if (pCard == NULL)
						{
							RplCmd.nRst		= STC_GAMECMD_GET_PLAYERCARD_T::RST_NOTEXIST;
						}
						else
						{
							memcpy(&RplCmd.card, pCard, sizeof(PlayerCard));
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
					}
					break;
				case CTS_GAMECMD_GET_TILE_INFO:
					{
						CTS_GAMECMD_GET_TILE_INFO_T*	pGetTile	= (CTS_GAMECMD_GET_TILE_INFO_T*)pGameCmd;
						PlayerCard*	pCard		= m_pPlayerCardMng->GetCardByPosition(pGetTile->nPosX, pGetTile->nPosY);
						if (pCard == NULL)
						{
							STC_GAMECMD_GET_TILE_NONE_T	RplCmd;
							RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							RplCmd.nGameCmd		= STC_GAMECMD_GET_TILE_NONE;
							RplCmd.nPosX		= pGetTile->nPosX;
							RplCmd.nPosY		= pGetTile->nPosY;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
						}
						else
						{
							STC_GAMECMD_GET_TILE_INFO_T	RplCmd;
							RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							RplCmd.nGameCmd		= STC_GAMECMD_GET_TILE_INFO;
							RplCmd.nPosX		= pGetTile->nPosX;
							RplCmd.nPosY		= pGetTile->nPosY;
							RplCmd.nAccountID	= pCard->nAccountID;
							WH_STRNCPY0(RplCmd.szName, pCard->szName);
							RplCmd.nAllianceID	= pCard->nAllianceID;
							WH_STRNCPY0(RplCmd.szAllianceName, pCard->szAllianceName);
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
						}
					}
					break;
				case CTS_GAMECMD_GET_BUILDING_LIST:
					{
						P_DBS4WEB_GET_BUILDING_LIST_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_BUILDING_LIST_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_BUILDING_TE_LIST:
					{
						P_DBS4WEB_GET_BUILDING_TE_LIST_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_BUILDING_TE_LIST_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_PRODUCTION_EVENT:
					{
						CTS_GAMECMD_GET_PRODUCTION_EVENT_T*	pGet	= (CTS_GAMECMD_GET_PRODUCTION_EVENT_T*)pGameCmd;
						P_DBS4WEB_GET_PRODUCTION_EVENT_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_PRODUCTION_EVENT_REQ;
						Cmd.nAccountID	= pGet->nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_PRODUCTION_TE_LIST:
					{
						CTS_GAMECMD_GET_PRODUCTION_TE_LIST_T*	pGet	= (CTS_GAMECMD_GET_PRODUCTION_TE_LIST_T*)pGameCmd;
						P_DBS4WEB_GET_PRODUCTION_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_PRODUCTION_TE_REQ;
						Cmd.nAccountID	= pGet->nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_TECHNOLOGY:
					{
						P_DBS4WEB_GET_TECHNOLOGY_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_TECHNOLOGY_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_RESEARCH_TE:
					{
						P_DBS4WEB_GET_RESEARCH_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_RESEARCH_TE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_SOLDIER:
					{
						P_DBS4WEB_GET_SOLDIER_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_SOLDIER_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_ALLIANCE_INFO:
					{
// 						P_DBS4WEB_GET_ALLIANCE_INFO_T	Cmd;
// 						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
// 						Cmd.nSubCmd		= CMDID_GET_ALLIANCE_INFO_REQ;
// 						Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
// 						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
// 
// 						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

						STC_GAMECMD_GET_ALLIANCE_INFO_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_INFO;
						Cmd.nRst		= STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_OK;
						Cmd.nMemberNum	= 0;
						Cmd.nMemberMaxNum	= 0;
						Cmd.nMyPosition		= alliance_position_member;
						Cmd.nDevelopment	= 0;
						Cmd.nTotalDevelopment	= 0;
						if (pPlayer->m_CharAtb.nAllianceID == 0)
						{
							CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T*	pGet	= (CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T*)pGameCmd;
							P_DBS4WEB_GET_MY_ALLIANCE_JOIN_T	Cmd;
							Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd			= CMDID_GET_MY_ALLIANCE_JOIN_REQ;
							Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
							Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
							return;
						}
						else
						{
							AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(pPlayer->m_CharAtb.nAllianceID);
							if (pAlliance == NULL)
							{
								Cmd.nRst= STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_LOADING;
								LoadAllianceFromDB(pPlayer->m_CharAtb.nAllianceID);
							}
							else
							{
								memcpy(&Cmd.alliance, &pAlliance->basicInfo, sizeof(pAlliance->basicInfo));
								Cmd.nMemberNum			= pAlliance->members.size();
								Cmd.nLevel				= pAlliance->GetLevel();
								Cmd.nMemberMaxNum		= m_pAllianceMng->GetMaxMemberByAllianceLevel(Cmd.nLevel);
								Cmd.nMyPosition			= pAlliance->GetMemberPosition(pPlayer->m_CharAtb.nAccountID);
								Cmd.nDevelopment		= 0;
								Cmd.nTotalDevelopment	= 0;
								for (int i=0; i<pAlliance->members.size(); i++)
								{
									if (pAlliance->members[i].nAccountID == pPlayer->m_CharAtb.nAccountID)
									{
										Cmd.nDevelopment		= pAlliance->members[i].nDevelopment;
										Cmd.nTotalDevelopment	= pAlliance->members[i].nTotalDevelopment;
									}
								}
							}
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_GET_ALLIANCE_MEMBER:
					{
// 						P_DBS4WEB_GET_ALLIANCE_MEMBER_T	Cmd;
// 						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
// 						Cmd.nSubCmd		= CMDID_GET_ALLIANCE_MEMBER_REQ;
// 						Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
// 						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
// 
// 						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

						STC_GAMECMD_GET_ALLIANCE_MEMBER_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_MEMBER;
						Cmd.nNum		= 0;
						Cmd.nRst		= STC_GAMECMD_GET_ALLIANCE_MEMBER_T::RST_OK;
						if (pPlayer->m_CharAtb.nAllianceID == 0)
						{
							Cmd.nRst	= STC_GAMECMD_GET_ALLIANCE_MEMBER_T::RST_NO_ALLI;
						}
						else
						{
							AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(pPlayer->m_CharAtb.nAllianceID);
							if (pAlliance == NULL)
							{
								Cmd.nRst= STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_LOADING;
								LoadAllianceFromDB(pPlayer->m_CharAtb.nAllianceID);
							}
							else
							{
								Cmd.nNum= pAlliance->members.size();
								if (Cmd.nNum > 0)
								{
									m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_MEMBER_T) + Cmd.nNum*sizeof(AllianceMemberUnit));
									STC_GAMECMD_GET_ALLIANCE_MEMBER_T*	pRplCmd	= (STC_GAMECMD_GET_ALLIANCE_MEMBER_T*)m_vectrawbuf.getbuf();
									memcpy(pRplCmd, &Cmd, sizeof(Cmd));
									memcpy(wh_getptrnexttoptr(pRplCmd), pAlliance->members.getbuf(), Cmd.nNum*sizeof(AllianceMemberUnit));
									g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
									return;
								}
							}
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_ALLIANCE_BUILDING_TE:
					{
// 						P_DBS4WEB_GET_ALLIANCE_BUILDING_TE_T	Cmd;
// 						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
// 						Cmd.nSubCmd		= CMDID_GET_ALLIANCE_BUILDING_TE_REQ;
// 						Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
// 						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
// 
// 						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

						STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_BUILDING_TE;
						Cmd.nNum		= 0;
						Cmd.nRst		= STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T::RST_OK;
						if (pPlayer->m_CharAtb.nAllianceID == 0)
						{
							Cmd.nRst	= STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T::RST_NO_ALLI;
						}
						else
						{
							AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(pPlayer->m_CharAtb.nAllianceID);
							if (pAlliance == NULL)
							{
								Cmd.nRst= STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T::RST_LOADING;
								LoadAllianceFromDB(pPlayer->m_CharAtb.nAllianceID);
							}
							else
							{
								Cmd.nNum= pAlliance->buildingTEs.size();
								if (Cmd.nNum > 0)
								{
									m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T) + Cmd.nNum*sizeof(AllianceBuildingTimeEvent));
									STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T*	pRplCmd	= (STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T*)m_vectrawbuf.getbuf();
									memcpy(pRplCmd, &Cmd, sizeof(Cmd));
									memcpy(wh_getptrnexttoptr(pRplCmd), pAlliance->buildingTEs.getbuf(), Cmd.nNum*sizeof(AllianceBuildingTimeEvent));
									g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
									return;
								}
							}
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_ALLIANCE_BUILDING:
					{
// 						P_DBS4WEB_GET_ALLIANCE_BUILDING_T	Cmd;
// 						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
// 						Cmd.nSubCmd		= CMDID_GET_ALLIANCE_BUILDING_REQ;
// 						Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
// 						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
// 
// 						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

						STC_GAMECMD_GET_ALLIANCE_BUILDING_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_BUILDING;
						Cmd.nNum		= 0;
						Cmd.nRst		= STC_GAMECMD_GET_ALLIANCE_BUILDING_T::RST_OK;
						if (pPlayer->m_CharAtb.nAllianceID == 0)
						{
							Cmd.nRst	= STC_GAMECMD_GET_ALLIANCE_BUILDING_T::RST_NO_ALLI;
						}
						else
						{
							AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(pPlayer->m_CharAtb.nAllianceID);
							if (pAlliance == NULL)
							{
								Cmd.nRst= STC_GAMECMD_GET_ALLIANCE_BUILDING_T::RST_LOADING;
								LoadAllianceFromDB(pPlayer->m_CharAtb.nAllianceID);
							}
							else
							{
								Cmd.nNum= pAlliance->buildings.size();
								if (Cmd.nNum > 0)
								{
									m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_BUILDING_T) + Cmd.nNum*sizeof(AllianceBuildingUnit));
									STC_GAMECMD_GET_ALLIANCE_BUILDING_T*	pRplCmd	= (STC_GAMECMD_GET_ALLIANCE_BUILDING_T*)m_vectrawbuf.getbuf();
									memcpy(pRplCmd, &Cmd, sizeof(Cmd));
									memcpy(wh_getptrnexttoptr(pRplCmd), pAlliance->buildings.getbuf(), Cmd.nNum*sizeof(AllianceBuildingUnit));
									g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
									return;
								}
							}
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_ALLIANCE_JOIN_EVENT:
					{
// 						P_DBS4WEB_GET_ALLIANCE_JOIN_EVENT_T	Cmd;
// 						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
// 						Cmd.nSubCmd		= CMDID_GET_ALLIANCE_JOIN_EVENT_REQ;
// 						Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
// 						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
// 
// 						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

						STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT;
						Cmd.nNum		= 0;
						Cmd.nRst		= STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T::RST_OK;
						if (pPlayer->m_CharAtb.nAllianceID == 0)
						{
							Cmd.nRst	= STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T::RST_NO_ALLI;
						}
						else
						{
							AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(pPlayer->m_CharAtb.nAllianceID);
							if (pAlliance == NULL)
							{
								Cmd.nRst= STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T::RST_LOADING;
								LoadAllianceFromDB(pPlayer->m_CharAtb.nAllianceID);
							}
							else
							{
								Cmd.nNum= pAlliance->joinEvents.size();
								if (Cmd.nNum > 0)
								{
									m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T) + Cmd.nNum*sizeof(AllianceJoinEvent));
									STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T*	pRplCmd	= (STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T*)m_vectrawbuf.getbuf();
									memcpy(pRplCmd, &Cmd, sizeof(Cmd));
									memcpy(wh_getptrnexttoptr(pRplCmd), pAlliance->joinEvents.getbuf(), Cmd.nNum*sizeof(AllianceJoinEvent));
									g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
									return;
								}
							}
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_CHAR_ATB:
					{
						STC_GAMECMD_GET_CHAR_ATB_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_GET_CHAR_ATB;
						unsigned int nTimeNow			= wh_time();
						pPlayer->m_CharAtb.nProtectTime	= nTimeNow>pPlayer->m_CharAtb.nProtectTimeEndTick?0:pPlayer->m_CharAtb.nProtectTimeEndTick-nTimeNow;
						pPlayer->m_CharAtb.nAddBuildNumTime	= nTimeNow>pPlayer->m_CharAtb.nAddBuildNumTimeEndTick?0:pPlayer->m_CharAtb.nAddBuildNumTimeEndTick-nTimeNow;
						memcpy(&Cmd.charAtb, &pPlayer->m_CharAtb, sizeof(Cmd.charAtb));

						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_MY_FRIEND_APPLY:
					{
						P_DBS4WEB_GET_MY_FRIEND_APPLY_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_MY_FRIEND_APPLY_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_OTHERS_FRIEND_APPLY:
					{
						P_DBS4WEB_GET_OTHERS_FRIEND_APPLY_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_OTHERS_FRIEND_APPLY_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_FRIEND_LIST:
					{
						P_DBS4WEB_GET_FRIEND_LIST_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_FRIEND_LIST_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_WORLD_AREA_INFO:
					{
						CTS_GAMECMD_GET_WORLD_AREA_INFO_T*	pGet	= (CTS_GAMECMD_GET_WORLD_AREA_INFO_T*)pGameCmd;
						unsigned int	nFromX	= pGet->nFromX;
						unsigned int	nFromY	= pGet->nFromY;
						unsigned int	nToX	= pGet->nToX;
						unsigned int	nToY	= pGet->nToY;
						if ((int)nFromX>=m_pInfo->nWorldXMax || (int)nFromY>=m_pInfo->nWorldYMax
							|| nFromX>nToX || nFromY>nToY)
						{
							// 发回空结果
							STC_GAMECMD_GET_WORLD_AREA_INFO_CITY_T		CityCmd;
							CityCmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							CityCmd.nGameCmd	= STC_GAMECMD_GET_WORLD_AREA_INFO_CITY;
							CityCmd.nFromX		= pGet->nFromX;
							CityCmd.nFromY		= pGet->nFromY;
							CityCmd.nToX		= pGet->nToX;
							CityCmd.nToY		= pGet->nToY;
							CityCmd.nNum		= 0;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &CityCmd, sizeof(CityCmd));

							STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN_T	TerrainCmd;
							TerrainCmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							TerrainCmd.nGameCmd	= STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN;
							TerrainCmd.nFromX	= pGet->nFromX;
							TerrainCmd.nFromY	= pGet->nFromY;
							TerrainCmd.nToX		= pGet->nToX;
							TerrainCmd.nToY		= pGet->nToY;
							TerrainCmd.nNum		= 0;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &TerrainCmd, sizeof(TerrainCmd));
							return;
						}
						else if ((int)pGet->nToX>=m_pInfo->nWorldXMax || (int)pGet->nToY>=m_pInfo->nWorldYMax)
						{
							nToX		= m_pInfo->nWorldXMax-1;
							nToY		= m_pInfo->nWorldYMax-1;
						}
						TileUnit**	pUnits		= m_pWorldMng->GetAllTile();
						int			nMaxNum		= (nToX-nFromX+1)*(nToY-nFromY+1);
						m_vectrawbuf_terrain.reserve(sizeof(STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN_T) + nMaxNum*sizeof(TerrainUnit));
						m_vectrawbuf_city.reserve(sizeof(STC_GAMECMD_GET_WORLD_AREA_INFO_CITY_T) + nMaxNum*sizeof(CityCard));
						m_vectrawbuf_worldres.reserve(sizeof(STC_GAMECMD_GET_WORLD_RES_T) + nMaxNum*sizeof(WorldResSimpleInfo));
						STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN_T*	pGetTerrain	= (STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN_T*)m_vectrawbuf_terrain.getbuf();
						pGetTerrain->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pGetTerrain->nGameCmd	= STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN;
						pGetTerrain->nFromX		= pGet->nFromX;
						pGetTerrain->nFromY		= pGet->nFromY;
						pGetTerrain->nToX		= pGet->nToX;
						pGetTerrain->nToY		= pGet->nToY;
						pGetTerrain->nNum		= 0;
						TerrainUnit*	pTerrain= (TerrainUnit*)wh_getptrnexttoptr(pGetTerrain);
						STC_GAMECMD_GET_WORLD_AREA_INFO_CITY_T*	pGetCity	= (STC_GAMECMD_GET_WORLD_AREA_INFO_CITY_T*)m_vectrawbuf_city.getbuf();
						pGetCity->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pGetCity->nGameCmd		= STC_GAMECMD_GET_WORLD_AREA_INFO_CITY;
						pGetCity->nFromX		= pGet->nFromX;
						pGetCity->nFromY		= pGet->nFromY;
						pGetCity->nToX			= pGet->nToX;
						pGetCity->nToY			= pGet->nToY;
						pGetCity->nNum			= 0;
						CityCard*	pCard		= (CityCard*)wh_getptrnexttoptr(pGetCity);
						STC_GAMECMD_GET_WORLD_RES_T*	pGetRes	= (STC_GAMECMD_GET_WORLD_RES_T*)m_vectrawbuf_worldres.getbuf();
						pGetRes->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pGetRes->nGameCmd		= STC_GAMECMD_GET_WORLD_RES;
						pGetRes->nFromX			= pGet->nFromX;
						pGetRes->nFromY			= pGet->nFromY;
						pGetRes->nToX			= pGet->nToX;
						pGetRes->nToY			= pGet->nToY;
						pGetRes->nNum			= 0;
						WorldResSimpleInfo*	pRes		= (WorldResSimpleInfo*)wh_getptrnexttoptr(pGetRes);
						for (unsigned int i=nFromX; i<=nToX; i++)
						{
							for (unsigned int j=nFromY; j<=nToY; j++)
							{
								if (pUnits[i][j].nType == TileUnit::TILETYPE_TERRAIN)
								{
									if (pUnits[i][j].terrainInfo.IsTerrain())
									{
										pTerrain->nPosX	= i;
										pTerrain->nPosY	= j;
										pTerrain->info	= pUnits[i][j].terrainInfo;

										pGetTerrain->nNum++;
										pTerrain++;
									}
									
									if (pUnits[i][j].nAccountID != 0)
									{
										PlayerCard*	pCardTmp	= m_pPlayerCardMng->GetCardByAccountID(pUnits[i][j].nAccountID);
										if (pCardTmp != NULL)
										{
											pCard->nAccountID	= pCardTmp->nAccountID;
											WH_STRNCPY0(pCard->szName, pCardTmp->szName);
											pCard->nAllianceID	= pCardTmp->nAllianceID;
											WH_STRNCPY0(pCard->szAllianceName, pCardTmp->szAllianceName);
											pCard->nLevel		= pCardTmp->nLevel;
											pCard->nPosX		= pCardTmp->nPosX;
											pCard->nPosY		= pCardTmp->nPosY;
											pCard->nProtectTime	= pCardTmp->nProtectTime;
											pCard->nVip			= pCardTmp->nVip;
											pCard->bDisplayVip	= pCardTmp->bVipDisplay;

											pGetCity->nNum++;
											pCard++;
										}
										else
										{
											// 错误日志
											GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,WorldInfo)"Card To Position Error,%d,%d,0x%"WHINT64PRFX"X", i, j, pUnits[i][j].nAccountID);
										}
									}

									// 世界资源
									if (pUnits[i][j].terrainInfo.nFloor2 >= wr_floor2_begin)
									{
										const WorldRes*	pResTmp		= m_pWorldMng->GetWorldRes(i,j);
										if (pResTmp != NULL)
										{
											pRes->nID				= pResTmp->nID;
											pRes->nType				= pResTmp->nType;
											pRes->nLevel			= pResTmp->nLevel;
											pRes->nPosX				= pResTmp->nPosX;
											pRes->nPosY				= pResTmp->nPosY;

											pGetRes->nNum++;
											pRes++;
										}
										else
										{
											// 错误日志
											GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,WorldInfo)"WorldRes Position Error,%d,%d", i, j);
										}
									}
								}
							}
						}
						m_vectrawbuf_terrain.resize(sizeof(STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN_T) + pGetTerrain->nNum*sizeof(TerrainUnit));
						m_vectrawbuf_city.resize(sizeof(STC_GAMECMD_GET_WORLD_AREA_INFO_CITY_T) + pGetCity->nNum*sizeof(PlayerCard));
						m_vectrawbuf_worldres.resize(sizeof(STC_GAMECMD_GET_WORLD_RES_T) + pGetRes->nNum*sizeof(WorldResSimpleInfo));
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf_terrain.getbuf(), m_vectrawbuf_terrain.size());
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf_city.getbuf(), m_vectrawbuf_city.size());
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf_worldres.getbuf(), m_vectrawbuf_worldres.size());
					}
					break;
// 				case CTS_GAMECMD_GET_ALLIANCE_MAIL:
// 					{
// 						if (pPlayer->m_CharAtb.nAllianceID == 0)
// 						{
// 							STC_GAMECMD_GET_ALLIANCE_LOG_T	Cmd;
// 							Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 							Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_MAIL;
// 							Cmd.nNum		= 0;
// 							Cmd.nRst		= STC_GAMECMD_GET_ALLIANCE_LOG_T::RST_OK;
// 
// 							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
// 						}
// 						else
// 						{
// // 							P_DBS4WEB_GET_ALLIANCE_MAIL_T	Cmd;
// // 							Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
// // 							Cmd.nSubCmd		= CMDID_GET_ALLIANCE_MAIL_REQ;
// // 							Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
// // 							Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
// // 
// // 							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
// 							CTS_GAMECMD_GET_ALLIANCE_MAIL_T*	pReq	= (CTS_GAMECMD_GET_ALLIANCE_MAIL_T*)pGameCmd;
// 							unsigned int	nReqMailSize		= pReq->nToNum-pReq->nFromNum;
// 							AllianceData_T*	pAlliance			= m_pAllianceMng->GetAlliance(pPlayer->m_CharAtb.nAllianceID);
// 							if (pAlliance != NULL)
// 							{
// 								if (pAlliance->mailList.size() < nReqMailSize)
// 								{
// 									nReqMailSize	= pAlliance->mailList.size();
// 								}
// 								m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_MAIL_T) + nReqMailSize*(wh_offsetof(AllianceMailUnit, szText)+TTY_MAIL_TEXT_LEN));
// 								STC_GAMECMD_GET_ALLIANCE_MAIL_T*		pRst	= (STC_GAMECMD_GET_ALLIANCE_MAIL_T*)m_vectrawbuf.getbuf();
// 								pRst->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 								pRst->nGameCmd		= STC_GAMECMD_GET_ALLIANCE_MAIL;
// 								pRst->nTotalMailNum	= pAlliance->mailList.size();
// 								pRst->nNum			= 0;
// 								pRst->nRst			= STC_GAMECMD_GET_ALLIANCE_MAIL_T::RST_OK;
// 								if (pReq->nFromNum > pReq->nToNum)
// 								{
// 									m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_MAIL_T));
// 									g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
// 								}
// 								else
// 								{
// 									int		nRealSize	= sizeof(STC_GAMECMD_GET_ALLIANCE_MAIL_T);
// 									AllianceMailUnit*	pNewMail	= (AllianceMailUnit*)wh_getptrnexttoptr(pRst);
// 									int		nCurNum		= 0;
// 									whlist<AllianceMailUnit*>::iterator	it	= pAlliance->mailList.begin();
// 									for (int i=0; i<pRst->nTotalMailNum; i++)
// 									{
// 										if (i>=pReq->nFromNum && i<=pReq->nToNum)
// 										{
// 											// 需要的
// 											AllianceMailUnit*	pMailUnit	= *it;
// 											int	nMailSize	= wh_offsetof(AllianceMailUnit, szText) + pMailUnit->nTextLen;
// 											memcpy(pNewMail, pMailUnit, nMailSize);
// 											pNewMail		= (AllianceMailUnit*)wh_getoffsetaddr(pNewMail, nMailSize);
// 											nRealSize		+= nMailSize;
// 											pRst->nNum++;
// 										}
// 										else if (i > pReq->nToNum)
// 										{
// 											break;
// 										}
// 										++it;
// 									}
// 									m_vectrawbuf.resize(nRealSize);
// 									g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
// 								}
// 							}
// 						}
// 					}
// 					break;
				case CTS_GAMECMD_GET_ALLIANCE_LOG:
					{
						if (pPlayer->m_CharAtb.nAllianceID == 0)
						{
							STC_GAMECMD_GET_ALLIANCE_LOG_T	Cmd;
							Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_LOG;
							Cmd.nNum		= 0;
							Cmd.nRst		= STC_GAMECMD_GET_ALLIANCE_LOG_T::RST_OK;

							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							P_DBS4WEB_GET_ALLIANCE_LOG_T	Cmd;
							Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd		= CMDID_GET_ALLIANCE_LOG_REQ;
							Cmd.nAllianceID	= pPlayer->m_CharAtb.nAccountID;
							Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_GET_PRIVATE_LOG:
					{
						P_DBS4WEB_GET_PRIVATE_LOG_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_PRIVATE_LOG_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_TRAINING_TE:
					{
						P_DBS4WEB_GET_TRAINING_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_TRAINING_TE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_PLAYERCARD_NAME:
					{
						CTS_GAMECMD_GET_PLAYERCARD_NAME_T*	pGet	= (CTS_GAMECMD_GET_PLAYERCARD_NAME_T*)pGameCmd;
						STC_GAMECMD_GET_PLAYERCARD_T	RplCmd;
						RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						RplCmd.nGameCmd		= STC_GAMECMD_GET_PLAYERCARD;
						RplCmd.nRst			= STC_GAMECMD_GET_PLAYERCARD_T::RST_OK;
						PlayerCard*	pCard	= m_pPlayerCardMng->GetCardByName(pGet->szCharName);
						if (pCard == NULL)
						{
							RplCmd.nRst		= STC_GAMECMD_GET_PLAYERCARD_T::RST_NOTEXIST;
						}
						else
						{
							memcpy(&RplCmd.card, pCard, sizeof(PlayerCard));
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
					}
					break;
				case CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT:
					{
						CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T*	pGet	= (CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T*)pGameCmd;
						P_DBS4WEB_GET_MY_ALLIANCE_JOIN_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_GET_MY_ALLIANCE_JOIN_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID:
					{
						CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T*	pGet		= (CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T*)pGameCmd;
						AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(pGet->nAllianceID);
						m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T) + wh_offsetof(AllianceCard, szIntroduction) + TTY_INTRODUCTION_LEN);
						STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T*	pCmd	= (STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pCmd->nGameCmd				= STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID;
						pCmd->nRst					= STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T::RST_OK;
						AllianceCard*	pCard		= (AllianceCard*)wh_getptrnexttoptr(pCmd);
						if (pAlliance != NULL)
						{
							pCard->nAllianceID		= pAlliance->basicInfo.nAllianceID;
							WH_STRNCPY0(pCard->szAllianceName, pAlliance->basicInfo.szAllianceName);
							pCard->nLeaderID		= pAlliance->basicInfo.nLeaderID;
							WH_STRNCPY0(pCard->szLeaderName, pAlliance->basicInfo.szLeaderName);
							pCard->nDevelopment		= pAlliance->basicInfo.nDevelopment;
							pCard->nTotalDevelopment= pAlliance->basicInfo.nTotalDevelopment;
							pCard->nIntroductionLen	= strlen(pAlliance->basicInfo.szIntroduction) + 1;
							memcpy(pCard->szIntroduction, pAlliance->basicInfo.szIntroduction, pCard->nIntroductionLen);
							pCard->nRank			= pAlliance->basicInfo.nRank;
							pCard->nLevel			= pAlliance->GetLevel();
							pCard->nMemberNum		= pAlliance->members.size();
							pCard->nMaxMemberNum	= m_pAllianceMng->GetMaxMemberByAllianceLevel(pCard->nLevel);
							m_vectrawbuf.resize(sizeof(CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T) + wh_offsetof(AllianceCard, szIntroduction) + pCard->nIntroductionLen);
						}
						else
						{
							pCmd->nRst					= STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T::RST_NOT_EXIST;
							m_vectrawbuf.resize(sizeof(CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T));
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}
					break;
				case CTS_GAMECMD_GET_ALLIANCE_CARD_BY_RANK:
					{
						CTS_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T*	pGet	= (CTS_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T*)pGameCmd;
						whvector<tty_id_t> vectAllianceRank	= m_pAllianceMng->m_vectAllianceRank;
						STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T		Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK;
						Cmd.nNum		= 0;
						Cmd.nTotalNum	= vectAllianceRank.size();
	
						if (Cmd.nTotalNum==0 || pGet->nFromRank>pGet->nToRank || pGet->nFromRank>Cmd.nTotalNum)
						{
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
							return;
						}
						if (pGet->nToRank >= Cmd.nTotalNum)
						{
							pGet->nToRank = Cmd.nTotalNum - 1;
						}
						m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T) + (pGet->nToRank-pGet->nFromRank+1)*(wh_offsetof(AllianceCard, szIntroduction) + TTY_INTRODUCTION_LEN));
						STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T*	pRstCmd	= (STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T*)m_vectrawbuf.getbuf();
						memcpy(pRstCmd, &Cmd, sizeof(Cmd));
						AllianceCard*	pCard	= (AllianceCard*)wh_getptrnexttoptr(pRstCmd);
						unsigned int nTotalLength	= sizeof(STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T);
						for (int i=pGet->nFromRank; i<=pGet->nToRank; i++)
						{
							tty_id_t	nAllianceID		= vectAllianceRank[i];
							AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(nAllianceID);
							if (pAlliance != NULL)
							{
								pCard->nAllianceID		= pAlliance->basicInfo.nAllianceID;
								WH_STRNCPY0(pCard->szAllianceName, pAlliance->basicInfo.szAllianceName);
								pCard->nLeaderID		= pAlliance->basicInfo.nLeaderID;
								WH_STRNCPY0(pCard->szLeaderName, pAlliance->basicInfo.szLeaderName);
								pCard->nDevelopment		= pAlliance->basicInfo.nDevelopment;
								pCard->nTotalDevelopment= pAlliance->basicInfo.nTotalDevelopment;
								pCard->nRank			= pAlliance->basicInfo.nRank;
								pCard->nLevel			= pAlliance->GetLevel();
								pCard->nMemberNum		= pAlliance->members.size();
								pCard->nMaxMemberNum	= m_pAllianceMng->GetMaxMemberByAllianceLevel(pCard->nLevel);
								pCard->nIntroductionLen	= strlen(pAlliance->basicInfo.szIntroduction) + 1;
								memcpy(pCard->szIntroduction, pAlliance->basicInfo.szIntroduction, pCard->nIntroductionLen);
								int	nLength				= wh_offsetof(AllianceCard, szIntroduction) + pCard->nIntroductionLen;
								nTotalLength			+= nLength;
								
								pCard					= (AllianceCard*)wh_getoffsetaddr(pCard, nLength);
								pRstCmd->nNum++;
							}
						}
						m_vectrawbuf.resize(nTotalLength);
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}
					break;
				case CTS_GAMECMD_GET_OTHER_GOLDORE_INFO:
					{
						CTS_GAMECMD_GET_OTHER_GOLDORE_INFO_T*	pGet	= (CTS_GAMECMD_GET_OTHER_GOLDORE_INFO_T*)pGameCmd;
						if (pGet->nAccountID == pPlayer->m_CharAtb.nAccountID)
						{
							return;
						}
						P_DBS4WEB_GET_OTHER_GOLDORE_INFO_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_GET_OTHER_GOLDORE_INFO_REQ;
						Cmd.nAccountID			= pGet->nAccountID;
						Cmd.nMyAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						/*
						// 1.获取别人的金矿信息
						{
							P_DBS4WEB_GET_OTHER_GOLDORE_POSITION_T	Cmd;
							Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd			= CMDID_GET_OTHER_GOLDORE_POSITION_REQ;
							Cmd.nAccountID		= pGet->nAccountID;
							Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
						// 2.获取金矿成熟事件
						{
							P_DBS4WEB_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T	Cmd;
							Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd			= CMDID_GET_OTHER_GOLDORE_PRODUCTION_EVENT_REQ;
							Cmd.nAccountID		= pGet->nAccountID;
							Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
						// 3.获取金矿时间事件
						{
							P_DBS4WEB_GET_OTHER_GOLDORE_PRODUCTION_TE_T	Cmd;
							Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd			= CMDID_GET_OTHER_GOLDORE_PRODUCTION_TE_REQ;
							Cmd.nAccountID		= pGet->nAccountID;
							Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
						// 4.获取金矿的摘取信息
						{
							P_DBS4WEB_GET_OTHER_GOLDORE_FETCH_INFO_T	Cmd;
							Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd			= CMDID_GET_OTHER_GOLDORE_FETCH_INFO_REQ;
							Cmd.nAccountID		= pGet->nAccountID;
							Cmd.nMyAccountID	= pPlayer->m_CharAtb.nAccountID;
							Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
						// 5.获取圣诞树信息
						{
							P_DBS4WEB_GET_OTHER_CHRISTMAS_TREE_INFO_T	Cmd;
							Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd			= CMDID_GET_OTHER_CHRISTMAS_TREE_INFO_REQ;
							Cmd.nAccountID		= pGet->nAccountID;
							Cmd.nMyAccountID	= pPlayer->m_CharAtb.nAccountID;
							Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
						*/
					}
					break;
				case CTS_GAMECMD_GET_CHRISTMAS_TREE_INFO:
					{
						CTS_GAMECMD_GET_CHRISTMAS_TREE_INFO_T*	pGet	= (CTS_GAMECMD_GET_CHRISTMAS_TREE_INFO_T*)pGameCmd;
						P_DBS4WEB_GET_CHRISTMAS_TREE_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_GET_CHRISTMAS_TREE_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_GOLDORE_SMP_INFO_ALL:
					{
						CTS_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T*	pGet	= (CTS_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T*)pGameCmd;
						if (pGet->nNum <=0)
						{
							STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T	Cmd;
							Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd			= STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL;
							Cmd.nNum				= 0;
							Cmd.nRst				= STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T::RST_NEGTIVE_0_NUM;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else if (pGet->nNum > CTS_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T::max_num)
						{
							STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T	Cmd;
							Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd			= STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL;
							Cmd.nNum				= 0;
							Cmd.nRst				= STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T::RST_TOO_MANY;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							int	nIDsSize			= pGet->nNum*sizeof(tty_id_t);
							m_vectrawbuf.resize(sizeof(P_DBS4WEB_GET_GOLDORE_SMP_INFO_ALL_T) + nIDsSize);
							P_DBS4WEB_GET_GOLDORE_SMP_INFO_ALL_T*	pCmd	= (P_DBS4WEB_GET_GOLDORE_SMP_INFO_ALL_T*)m_vectrawbuf.getbuf();
							pCmd->nCmd				= P_DBS4WEB_REQ_CMD;
							pCmd->nSubCmd			= CMDID_GET_GOLDORE_SMP_INFO_ALL_REQ;
							pCmd->nAccountID		= pPlayer->m_CharAtb.nAccountID;
							pCmd->nNum				= pGet->nNum;
							pCmd->nExt[0]			= pPlayer->m_nPlayerGID;
							memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pGet), nIDsSize);
							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
					}
					break;
				case CTS_GAMECMD_GET_ALLIANCE_TRADE_INFO:
					{
						CTS_GAMECMD_GET_ALLIANCE_TRADE_INFO_T*	pGet	= (CTS_GAMECMD_GET_ALLIANCE_TRADE_INFO_T*)pGameCmd;
						P_DBS4WEB_GET_ALLIANCE_TRADE_INFO_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_GET_ALLIANCE_TRADE_INFO_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_ENEMY_LIST:
					{
						P_DBS4WEB_GET_ENEMY_LIST_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_GET_ENEMY_LIST_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_RANK_LIST:
					{
						CTS_GAMECMD_GET_RANK_LIST_T*	pGet		= (CTS_GAMECMD_GET_RANK_LIST_T*)pGameCmd;
						if (pGet->nNum > max_rank_query_num)
						{
							pGet->nNum	= max_rank_query_num;
						}
						CRankMngS::rankdata_vector_T*	pRankList	= m_pRankMng->GetRankDataVector(pGet->nType);
						STC_GAMECMD_GET_RANK_LIST_T		Cmd;
						Cmd.nCmd						= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd					= STC_GAMECMD_GET_RANK_LIST;
						Cmd.nNum						= 0;
						Cmd.nTotalNum					= 0;
						Cmd.nType						= pGet->nType;
						Cmd.nMyRank						= m_pRankMng->GetRank(pGet->nType, pPlayer->m_CharAtb.nAccountID);
						if (pRankList != NULL)
						{
							Cmd.nTotalNum				= pRankList->size();
							if (pRankList->size() > pGet->nFrom)
							{
								Cmd.nNum				= pRankList->size()>(pGet->nFrom+pGet->nNum)?pGet->nNum:pRankList->size()-pGet->nFrom;

								switch (pGet->nType)
								{
								case rank_type_char_level:
								case rank_type_char_gold:
								case rank_type_char_diamond:
								case rank_type_instance_wangzhe:
									{
										m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_RANK_LIST_T) + Cmd.nNum*sizeof(CharRankInfo));
										STC_GAMECMD_GET_RANK_LIST_T*	pRplCmd	= (STC_GAMECMD_GET_RANK_LIST_T*)m_vectrawbuf.getbuf();
										memcpy(pRplCmd, &Cmd, sizeof(Cmd));
										CharRankInfo*	pRankInfo				= (CharRankInfo*)wh_getptrnexttoptr(pRplCmd);
										for (int i=pGet->nFrom; i<(pGet->nFrom+Cmd.nNum); i++)
										{
											RankData*	pRankData				= pRankList->getptr(i);
											pRankInfo->nAccountID				= pRankData->nOnlyID;
											pRankInfo->nRank					= i+1;
											pRankInfo->nData					= pRankData->nData;
											pRankInfo->nHeadID					= 0;
											pRankInfo->nLevel					= 0;
											pRankInfo->szName[0]				= 0;

											PlayerCard*	pCard					= m_pPlayerCardMng->GetCardByAccountID(pRankInfo->nAccountID);
											if (pCard != NULL)
											{
												pRankInfo->nHeadID				= pCard->nHeadID;
												pRankInfo->nLevel				= pCard->nLevel;
												WH_STRNCPY0(pRankInfo->szName, pCard->szName);
											}

											pRankInfo++;
										}
									}
									break;
								}
								g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
								return;
							}
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_MY_RANK:
					{
						CTS_GAMECMD_GET_MY_RANK_T*	pGet	= (CTS_GAMECMD_GET_MY_RANK_T*)pGameCmd;
						STC_GAMECMD_GET_MY_RANK_T	Cmd;
						Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd				= STC_GAMECMD_GET_MY_RANK;
						Cmd.nType					= pGet->nType;
						Cmd.nRank					= 0;
						Cmd.nTotalNum				= 0;
						CRankMngS::rankdata_vector_T*	pVect	= m_pRankMng->GetRankDataVector(pGet->nType);
						if (pVect != NULL)
						{
							Cmd.nRank				= m_pRankMng->GetRank(pGet->nType, pPlayer->m_CharAtb.nAccountID);
							Cmd.nTotalNum			= pVect->size();
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_IMPORTANT_CHAR_ATB:
					{
						g_pLPNet->SendPlayerCharAtbToClient(pPlayer);
					}
					break;
				case CTS_GAMECMD_GET_PAY_SERIAL:
					{
						P_DBS4WEB_GET_PAY_SERIAL_T	Cmd;
						Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd					= CMDID_GET_PAY_SERIAL_REQ;
						Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST:
					{
						const whvector<WorldFamousCity>& vectFamousCitys = m_pWorldMng->GetWorldFamousCity();
						m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T) + vectFamousCitys.size()*sizeof(STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T::FamousCityUnit));
						STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T*	pRplCmd	= (STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T*)m_vectrawbuf.getbuf();
						pRplCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pRplCmd->nGameCmd			= STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST;
						pRplCmd->nNum				= vectFamousCitys.size();
						STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T::FamousCityUnit*	pUnit	= (STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T::FamousCityUnit*)wh_getptrnexttoptr(pRplCmd);
						for (int i=0; i<pRplCmd->nNum; i++)
						{
							pUnit->nType			= vectFamousCitys[i].nType;
							pUnit->nPosX			= vectFamousCitys[i].nPosX;
							pUnit->nPosY			= vectFamousCitys[i].nPosY;

							pUnit++;
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}
					break;
				case CTS_GAMECMD_GET_SERVER_TIME:
					{
						STC_GAMECMD_GET_SERVER_TIME_T	Cmd;
						Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd				= STC_GAMECMD_GET_SERVER_TIME;
						Cmd.nTimeNow				= (unsigned int)wh_time();
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_DONATE_SOLDIER_QUEUE:
					{
						P_DBS4WEB_GET_DONATE_SOLDIER_QUEUE_T	Cmd;
						Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd					= CMDID_GET_DONATE_SOLDIER_QUEUE_REQ;
						Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_LEFT_DONATE_SOLDIER:
					{
						P_DBS4WEB_GET_LEFT_DONATE_SOLDIER_T	Cmd;
						Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd					= CMDID_GET_LEFT_DONATE_SOLDIER_REQ;
						Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_GOLD_DEAL:
					{
						CTS_GAMECMD_GET_GOLD_DEAL_T*	pReq	= (CTS_GAMECMD_GET_GOLD_DEAL_T*)pGameCmd;
						P_DBS4WEB_GET_GOLD_DEAL_T	Cmd;
						Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd					= CMDID_GET_GOLD_DEAL_REQ;
						Cmd.nPage					= pReq->nPage;
						Cmd.nPageSize				= pReq->nPageSize;
						Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_SELF_GOLD_DEAL:
					{
						CTS_GAMECMD_GET_SELF_GOLD_DEAL_T*	pReq	= (CTS_GAMECMD_GET_SELF_GOLD_DEAL_T*)pGameCmd;
						P_DBS4WEB_GET_SELF_GOLD_DEAL_T	Cmd;
						Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd					= CMDID_GET_SELF_GOLD_DEAL_REQ;
						Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_WORLD_RES_CARD:
					{
						CTS_GAMECMD_GET_WORLD_RES_CARD_T*	pReq	= (CTS_GAMECMD_GET_WORLD_RES_CARD_T*)pGameCmd;
						STC_GAMECMD_GET_WORLD_RES_CARD_T	Cmd;
						Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd				= STC_GAMECMD_GET_WORLD_RES_CARD;
						Cmd.nRst					= STC_GAMECMD_GET_WORLD_RES_CARD_T::RST_OK;
						const WorldRes*	pRes		= m_pWorldMng->GetWorldRes(pReq->nID);
						if (pRes == NULL)
						{
							Cmd.nRst				= STC_GAMECMD_GET_WORLD_RES_CARD_T::RST_NOT_EXIST;
						}
						else
						{
							WorldRes2Client&	res	= Cmd.res;
							res.nID					= pRes->nID;
							res.nType				= pRes->nType;
							res.nLevel				= pRes->nLevel;
							res.nPosX				= pRes->nPosX;
							res.nPosY				= pRes->nPosY;
							res.nGold				= pRes->nGold;
							res.nPop				= pRes->nPop;
							res.nCrystal			= pRes->nCrystal;
							res.nForce				= pRes->nForce;
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_KICK_CLIENT_ALL:
					{
						CTS_GAMECMD_GET_KICK_CLIENT_ALL_T*	pReq	= (CTS_GAMECMD_GET_KICK_CLIENT_ALL_T*)pGameCmd;
						P_DBS4WEB_GET_KICK_CLIENT_ALL_T		Cmd;
						Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd					= CMDID_GET_KICK_CLIENT_ALL_REQ;
						Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ADD_MAIL:
					{
						CTS_GAMECMD_ADD_MAIL_T*	pAdd	= (CTS_GAMECMD_ADD_MAIL_T*)pGameCmd;
						if (pAdd->nTextLen > TTY_MAIL_TEXT_LEN || pAdd->nTextLen <= 0)
						{
							STC_GAMECMD_ADD_MAIL_T	Cmd;
							Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd	= STC_GAMECMD_ADD_MAIL;
							Cmd.nRst		= STC_GAMECMD_ADD_MAIL_T::RST_TEXT_TOO_LONG;

							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else if (pAdd->nToCharID == pPlayer->m_CharAtb.nAccountID)
						{
							STC_GAMECMD_ADD_MAIL_T	Cmd;
							Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd	= STC_GAMECMD_ADD_MAIL;
							Cmd.nRst		= STC_GAMECMD_ADD_MAIL_T::RST_CANNOT_SEND_SELF;

							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							int		nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
							nExt[0]			= pPlayer->m_nPlayerGID;
							g_pLPNet->AddMail(pPlayer->m_CharAtb.nAccountID, pPlayer->m_CharAtb.szName, pAdd->nToCharID, true, MAIL_TYPE_PRIVATE, MAIL_FLAG_NORMAL, true, pAdd->nTextLen, pAdd->szText, 0, NULL, nExt);
						}
					}
					break;
				case CTS_GAMECMD_GET_MAIL:
					{
						CTS_GAMECMD_GET_MAIL_T*		pReq	= (CTS_GAMECMD_GET_MAIL_T*)pGameCmd;
						unsigned int	nReqMailSize		= pReq->nToNum-pReq->nFromNum;
						if (pPlayer->m_MailList.size() < nReqMailSize)
						{
							nReqMailSize	= pPlayer->m_MailList.size();
						}
						m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_MAIL_T));
						STC_GAMECMD_GET_MAIL_T*		pRst	= (STC_GAMECMD_GET_MAIL_T*)m_vectrawbuf.getbuf();
						pRst->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pRst->nGameCmd		= STC_GAMECMD_GET_MAIL;
						pRst->nTotalMailNum	= 0;
						pRst->nUnreadMailNum= 0;
						pRst->nNum			= 0;
						pRst->nRst			= STC_GAMECMD_GET_MAIL_T::RST_OK;
						if (pReq->nFromNum > pReq->nToNum)
						{
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
						else
						{
							PrivateMailUnit*	pNewMail	= NULL;
							whlist<PrivateMailUnit*>::iterator	it	= pPlayer->m_MailList.begin();
							for (; it!=pPlayer->m_MailList.end(); ++it)
							{
								PrivateMailUnit*	pMailUnit	= *it;
								if ((pMailUnit->nType&pReq->nType) != 0)
								{
									if ((unsigned int)pRst->nTotalMailNum>=pReq->nFromNum && (unsigned int)pRst->nTotalMailNum<=pReq->nToNum)
									{
										// 需要的
										int	nMailSize	= wh_offsetof(PrivateMailUnit, szText) + pMailUnit->nTextLen;
										int*	pExtDataLen	= (int*)wh_getoffsetaddr(pMailUnit, nMailSize);
										nMailSize		+= (sizeof(int) + *pExtDataLen);
										pNewMail		= (PrivateMailUnit*)m_vectrawbuf.pushn_back(nMailSize);
										memcpy(pNewMail, pMailUnit, nMailSize);
										pRst->nNum++;
									}
									pRst->nTotalMailNum++;
									if (!pMailUnit->bReaded)
									{
										pRst->nUnreadMailNum++;
									}
								}
							}
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
					}
					break;
				case CTS_GAMECMD_READ_MAIL:
					{
						CTS_GAMECMD_READ_MAIL_T*	pRead	= (CTS_GAMECMD_READ_MAIL_T*)pGameCmd;
						for (whlist<PrivateMailUnit*>::iterator it=pPlayer->m_MailList.begin(); it!=pPlayer->m_MailList.end(); ++it)
						{
							PrivateMailUnit*	pMail		= *it;
							if (pMail->nMailID == pRead->nMailID)
							{
								if (pMail->bReaded)
								{
									return;
								}
								pMail->bReaded	= true;
							}
						}

						P_DBS4WEB_READ_PRIVATE_MAIL_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_READ_PRIVATE_MAIL_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nMailID		= pRead->nMailID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_DELETE_MAIL:
					{
						CTS_GAMECMD_DELETE_MAIL_T*	pDelete	= (CTS_GAMECMD_DELETE_MAIL_T*)pGameCmd;

						P_DBS4WEB_READ_PRIVATE_MAIL_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEL_PRIVATE_MAIL_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nMailID		= pDelete->nMailID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
// 				case CTS_GAMECMD_GET_MAIL_1:
// 					{
// 						CTS_GAMECMD_GET_MAIL_1_T*	pGet	= (CTS_GAMECMD_GET_MAIL_1_T*)pGameCmd;
// 
// 						P_DBS4WEB_READ_PRIVATE_MAIL_T	Cmd;
// 						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
// 						Cmd.nSubCmd		= CMDID_GET_PRIVATE_MAIL_1_REQ;
// 						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
// 						Cmd.nMailID		= pGet->nMailID;
// 						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
// 
// 						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
// 					}
// 					break;
				case CTS_GAMECMD_READ_MAIL_ALL:
					{
						CTS_GAMECMD_READ_MAIL_ALL_T*	pReq	= (CTS_GAMECMD_READ_MAIL_ALL_T*)pGameCmd;

						char		szTmpBuf[256];
						char*		pPos	= szTmpBuf;
						int			nLength	= 0;
						if (pReq->nMailType&MAIL_TYPE_PRIVATE)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_PRIVATE);
						}
						if (pReq->nMailType&MAIL_TYPE_ALLIANCE)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_ALLIANCE);
						}
						if (pReq->nMailType&MAIL_TYPE_SYSTEM)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_SYSTEM);
						}
						if (pReq->nMailType&MAIL_TYPE_ALLIANCE_EVENT)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_ALLIANCE_EVENT);
						}
						if (pReq->nMailType&MAIL_TYPE_GOLDORE_EVENT)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_GOLDORE_EVENT);
						}
						if (pPos != szTmpBuf)
						{
							*(--pPos)		= 0;
							nLength			= (pPos-szTmpBuf)+1;
						}
						else
						{
							*pPos			= 0;
						}

						if (nLength > 0)
						{
							m_vectrawbuf.resize(wh_offsetof(P_DBS4WEB_READ_MAIL_ALL_T, szTypeStr) + nLength);
							P_DBS4WEB_READ_MAIL_ALL_T*	pDBCmd	= (P_DBS4WEB_READ_MAIL_ALL_T*)m_vectrawbuf.getbuf();
							pDBCmd->nCmd		= P_DBS4WEB_REQ_CMD;
							pDBCmd->nSubCmd		= CMDID_READ_MAIL_ALL_REQ;
							pDBCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
							pDBCmd->nMailType	= pReq->nMailType;
							memcpy(pDBCmd->szTypeStr, szTmpBuf, nLength);
							pDBCmd->nExt[0]		= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
						else
						{
							STC_GAMECMD_READ_MAIL_ALL_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_READ_MAIL_ALL;
							Cmd.nMailType		= pReq->nMailType;
							Cmd.nRst			= STC_GAMECMD_READ_MAIL_ALL_T::RST_ERR_TYPE;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_DELETE_MAIL_ALL:
					{
						CTS_GAMECMD_DELETE_MAIL_ALL_T*	pReq	= (CTS_GAMECMD_DELETE_MAIL_ALL_T*)pGameCmd;

						char		szTmpBuf[256];
						char*		pPos	= szTmpBuf;
						int			nLength	= 0;
						if (pReq->nMailType&MAIL_TYPE_PRIVATE)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_PRIVATE);
						}
						if (pReq->nMailType&MAIL_TYPE_ALLIANCE)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_ALLIANCE);
						}
						if (pReq->nMailType&MAIL_TYPE_SYSTEM)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_SYSTEM);
						}
						if (pReq->nMailType&MAIL_TYPE_ALLIANCE_EVENT)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_ALLIANCE_EVENT);
						}
						if (pReq->nMailType&MAIL_TYPE_GOLDORE_EVENT)
						{
							pPos			+= sprintf(pPos, "%d,", MAIL_TYPE_GOLDORE_EVENT);
						}
						if (pPos != szTmpBuf)
						{
							*(--pPos)		= 0;
							nLength			= (pPos-szTmpBuf)+1;
						}
						else
						{
							*pPos			= 0;
						}

						if (nLength > 0)
						{
							m_vectrawbuf.resize(wh_offsetof(P_DBS4WEB_DELETE_MAIL_ALL_T, szTypeStr) + nLength);
							P_DBS4WEB_DELETE_MAIL_ALL_T*	pDBCmd	= (P_DBS4WEB_DELETE_MAIL_ALL_T*)m_vectrawbuf.getbuf();
							pDBCmd->nCmd		= P_DBS4WEB_REQ_CMD;
							pDBCmd->nSubCmd		= CMDID_DELETE_MAIL_ALL_REQ;
							pDBCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
							pDBCmd->nMailType	= pReq->nMailType;
							memcpy(pDBCmd->szTypeStr, szTmpBuf, nLength);
							pDBCmd->nExt[0]		= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
						else
						{
							STC_GAMECMD_DELETE_MAIL_ALL_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_DELETE_MAIL_ALL;
							Cmd.nMailType		= pReq->nMailType;
							Cmd.nRst			= STC_GAMECMD_DELETE_MAIL_ALL_T::RST_ERR_TYPE;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_GET_RELATION_LOG:
					{
						CTS_GAMECMD_GET_RELATION_LOG_T*	pGet	= (CTS_GAMECMD_GET_RELATION_LOG_T*)pGameCmd;

						P_DBS4WEB_GET_RELATION_LOG_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_RELATION_LOG_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_BUILD_BUILDING:
					{
						CTS_GAMECMD_OPERATE_BUILD_BUILDING_T*	pBuildBuilding	= (CTS_GAMECMD_OPERATE_BUILD_BUILDING_T*)pGameCmd;

						P_DBS4WEB_BUILD_BUILDING_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_BUILD_BUILDING_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pBuildBuilding->nExcelID;
						Cmd.nAutoID		= pBuildBuilding->nAutoID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_UPGRADE_BUILDING:
					{
						CTS_GAMECMD_OPERATE_UPGRADE_BUILDING_T*	pUpgradeBuilding	= (CTS_GAMECMD_OPERATE_UPGRADE_BUILDING_T*)pGameCmd;

						P_DBS4WEB_UPGRADE_BUILDING_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_UPGRADE_BUILDING_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nAutoID		= pUpgradeBuilding->nAutoID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_PRODUCE_GOLD:
					{
						CTS_GAMECMD_OPERATE_PRODUCE_GOLD_T*	pProduce	= (CTS_GAMECMD_OPERATE_PRODUCE_GOLD_T*)pGameCmd;

						P_DBS4WEB_ADD_PRODUCTION_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ADD_PRODUCTION_TE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nAutoID		= pProduce->nAutoID;
						Cmd.nProductionChoice	= pProduce->nProductionChoice;
						Cmd.nType		= te_subtype_production_gold;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_FETCH_GOLD:
					{
						CTS_GAMECMD_OPERATE_FETCH_GOLD_T*	pFetch	= (CTS_GAMECMD_OPERATE_FETCH_GOLD_T*)pGameCmd;
						
						P_DBS4WEB_FETCH_PRODUCTION_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_FETCH_PRODUCTION_REQ;
						Cmd.nAutoID		= pFetch->nAutoID;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nType		= te_subtype_production_gold;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ACCE_BUILDING:
					{
						CTS_GAMECMD_OPERATE_ACCE_BUILDING_T*	pAcce	= (CTS_GAMECMD_OPERATE_ACCE_BUILDING_T*)pGameCmd;

						P_DBS4WEB_ACCELERATE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ACCELERATE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nAutoID		= pAcce->nAutoID;
						Cmd.nExcelID	= 0;
						Cmd.nTime		= pAcce->nTime;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nType		= te_type_building;
						Cmd.nSubType	= pAcce->nType;
						Cmd.nMoneyType	= pAcce->nMoneyType;
						
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE:
					{
						CTS_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE_T*	pAcce	= (CTS_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE_T*)pGameCmd;

						P_DBS4WEB_ACCELERATE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ACCELERATE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nAutoID		= pAcce->nAutoID;
						Cmd.nExcelID	= 0;
						Cmd.nTime		= pAcce->nTime;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nType		= te_type_production;
						Cmd.nSubType	= te_subtype_production_gold;
						Cmd.nMoneyType	= pAcce->nMoneyType;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_RESEARCH:
					{
						CTS_GAMECMD_OPERATE_RESEARCH_T*	pResearch	= (CTS_GAMECMD_OPERATE_RESEARCH_T*)pGameCmd;
						P_DBS4WEB_ADD_RESEARCH_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ADD_RESEARCH_TE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pResearch->nExcelID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CONSCRIPT_SOLDIER:
					{
						CTS_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T*	pConscript	= (CTS_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T*)pGameCmd;
						P_DBS4WEB_CONSCRIPT_SOLDIER_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CONSCRIPT_SOLDIER_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pConscript->nExcelID;
						Cmd.nLevel		= pConscript->nLevel;
						Cmd.nNum		= pConscript->nNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_UPGRADE_SOLDIER:
					{
						CTS_GAMECMD_OPERATE_UPGRADE_SOLDIER_T*	pUpgrade	= (CTS_GAMECMD_OPERATE_UPGRADE_SOLDIER_T*)pGameCmd;
						P_DBS4WEB_UPGRADE_SOLDIER_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_UPGRADE_SOLDIER_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pUpgrade->nExcelID;
						Cmd.nFromLevel	= pUpgrade->nFromLevel;
						Cmd.nToLevel	= pUpgrade->nToLevel;
						Cmd.nNum		= pUpgrade->nNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ACCE_RESEARCH:
					{
						CTS_GAMECMD_OPERATE_ACCE_RESEARCH_T*	pAcce	= (CTS_GAMECMD_OPERATE_ACCE_RESEARCH_T*)pGameCmd;
						P_DBS4WEB_ACCELERATE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ACCELERATE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pAcce->nExcelID;
						Cmd.nAutoID		= 0;
						Cmd.nType		= te_type_research;
						Cmd.nSubType	= 0;
						Cmd.nTime		= pAcce->nTime;
						Cmd.nMoneyType	= pAcce->nMoneyType;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER:
					{
						CTS_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T*	pAcce	= (CTS_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T*)pGameCmd;
						P_DBS4WEB_ACCEPT_ALLIANCE_MEMBER_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ACCEPT_ALLIANCE_MEMBER_REQ;
						Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
						Cmd.nManagerID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nNewMemberID= pAcce->nMemberID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_BUILD_ALLI_BUILDING:
					{
						CTS_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T*	pBuild	= (CTS_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T*)pGameCmd;
						P_DBS4WEB_BUILD_ALLIANCE_BUILDING_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_BUILD_ALLIANCE_BUILDING_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pBuild->nExcelID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CANCEL_JOIN_ALLI:
					{
						CTS_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T*	pJoin	= (CTS_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T*)pGameCmd;
						P_DBS4WEB_CANCEL_JOIN_ALLIANCE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CANCEL_JOIN_ALLIANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CREATE_ALLI:
					{
						CTS_GAMECMD_OPERATE_CREATE_ALLI_T*	pCreate	= (CTS_GAMECMD_OPERATE_CREATE_ALLI_T*)pGameCmd;
						if (!g_pLPNet->IsCharNameValid(pCreate->szAllianceName))
						{
							STC_GAMECMD_OPERATE_CREATE_ALLI_T	Cmd;
							Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd	= STC_GAMECMD_OPERATE_CREATE_ALLI;
							Cmd.nAllianceID	= 0;
							Cmd.nRst		= STC_GAMECMD_OPERATE_CREATE_ALLI_T::RST_NAME_INVALID;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							P_DBS4WEB_CREATE_ALLIANCE_T	Cmd;
							Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd		= CMDID_CREATE_ALLIANCE_REQ;
							Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
							WH_STRNCPY0(Cmd.szAllianceName, pCreate->szAllianceName);
							Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_DISMISS_ALLI:
					{
						P_DBS4WEB_DISMISS_ALLIANCE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DISMISS_ALLIANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_EXIT_ALLI:
					{
						P_DBS4WEB_EXIT_ALLIANCE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_EXIT_ALLIANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER:
					{
						CTS_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T*	pExpel	= (CTS_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T*)pGameCmd;
						P_DBS4WEB_EXPEL_ALLIANCE_MEMBER_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_EXPEL_ALLIANCE_MEMBER_REQ;
						Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
						Cmd.nManagerID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nMemberID	= pExpel->nMemberID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_JOIN_ALLI:
					{
						CTS_GAMECMD_OPERATE_JOIN_ALLI_T*	pJoin	= (CTS_GAMECMD_OPERATE_JOIN_ALLI_T*)pGameCmd;
						P_DBS4WEB_JOIN_ALLIANCE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_JOIN_ALLIANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nAllianceID	= pJoin->nAllianceID;
						Cmd.nLevel		= pPlayer->m_CharAtb.nLevel;
						Cmd.nHeadID		= pPlayer->m_CharAtb.nHeadID;
						WH_STRNCPY0(Cmd.szName, pPlayer->m_CharAtb.szName);
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_SET_ALLI_POSITION:
					{
						CTS_GAMECMD_OPERATE_SET_ALLI_POSITION_T*	pSet	= (CTS_GAMECMD_OPERATE_SET_ALLI_POSITION_T*)pGameCmd;
						P_DBS4WEB_SET_ALLIANCE_POSITION_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_SET_ALLIANCE_POSITION_REQ;
						Cmd.nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
						Cmd.nManagerID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nMemberID	= pSet->nMemberID;
						Cmd.nPosition	= pSet->nPosition;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_REFUSE_JOIN_ALLI:
					{
						CTS_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T*	pRefuse	= (CTS_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T*)pGameCmd;
						P_DBS4WEB_REFUSE_JOIN_ALLIANCE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_REFUSE_JOIN_ALLIANCE_REQ;
						Cmd.nManagerID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nApplicantID= pRefuse->nApplicantID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ABDICATE_ALLI:
					{
						CTS_GAMECMD_OPERATE_ABDICATE_ALLI_T*	pAbdicate	= (CTS_GAMECMD_OPERATE_ABDICATE_ALLI_T*)pGameCmd;
						P_DBS4WEB_ABDICATE_ALLIANCE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ABDICATE_ALLIANCE_REQ;
						Cmd.nLeaderID	= pPlayer->m_CharAtb.nAccountID;
						WH_STRNCPY0(Cmd.szMemberName, pAbdicate->szMemberName);
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_APPROVE_FRIEND:
					{
						CTS_GAMECMD_OPERATE_APPROVE_FRIEND_T*	pApprove	= (CTS_GAMECMD_OPERATE_APPROVE_FRIEND_T*)pGameCmd;
						P_DBS4WEB_APPROVE_FRIEND_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_APPROVE_FRIEND_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nPeerAccountID	= pApprove->nPeerAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_APPLY_FRIEND:
					{
						CTS_GAMECMD_OPERATE_APPLY_FRIEND_T*	pApply	= (CTS_GAMECMD_OPERATE_APPLY_FRIEND_T*)pGameCmd;
						P_DBS4WEB_APPLY_FRIEND_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_APPLY_FRIEND_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nPeerAccountID	= pApply->nPeerAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY:
					{
						CTS_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T*	pRefuse	= (CTS_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T*)pGameCmd;
						P_DBS4WEB_REFUSE_FRIEND_APPLY_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_REFUSE_FRIEND_APPLY_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nPeerAccountID	= pRefuse->nPeerAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY:
					{
						CTS_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T*	pCancel	= (CTS_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T*)pGameCmd;
						P_DBS4WEB_CANCEL_FRIEND_APPLY_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CANCEL_FRIEND_APPLY_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nPeerAccountID	= pCancel->nPeerAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_DELETE_FRIEND:
					{
						CTS_GAMECMD_OPERATE_DELETE_FRIEND_T*	pDelete	= (CTS_GAMECMD_OPERATE_DELETE_FRIEND_T*)pGameCmd;
						P_DBS4WEB_DELETE_FRIEND_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DELETE_FRIEND_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nPeerAccountID	= pDelete->nFriendID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL:
					{
						CTS_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T*	pAdd	= (CTS_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T*)pGameCmd;
						STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL;
						Cmd.nRst		= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_OK;
						if (pAdd->nLen > TTY_ALLIANCE_MAIL_TEXT_LEN)
						{
							Cmd.nRst		= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_TEXT_TOO_LONG;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							// 作废,联盟邮件改为私人邮件
// 							m_vectrawbuf.resize(wh_offsetof(P_DBS4WEB_ADD_ALLIANCE_MAIL_T, szText) + pAdd->nLen);
// 							P_DBS4WEB_ADD_ALLIANCE_MAIL_T*	pCmd	= (P_DBS4WEB_ADD_ALLIANCE_MAIL_T*)m_vectrawbuf.getbuf();
// 							pCmd->nCmd		= P_DBS4WEB_REQ_CMD;
// 							pCmd->nSubCmd	= CMDID_ADD_ALLIANCE_MAIL_REQ;
// 							pCmd->nAccountID= pPlayer->m_CharAtb.nAccountID;
// 							WH_STRNCPY0(pCmd->szSender, pPlayer->m_CharAtb.szName);
// 							pCmd->nTextLen	= pAdd->nLen;
// 							memcpy(pCmd->szText, pAdd->szText, pAdd->nLen);
// 							pCmd->szText[pCmd->nTextLen-1]	= 0;
// 							pCmd->nExt[0]	= pPlayer->m_nPlayerGID;

							if (pPlayer->m_CharAtb.nAllianceID == 0)
							{
								Cmd.nRst	= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_NOT_IN_ALLIANCE;
								g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
							}
							else
							{
								AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(pPlayer->m_CharAtb.nAllianceID);
								if (pAlliance == NULL)
								{
									LoadAllianceFromDB(pPlayer->m_CharAtb.nAllianceID);

									Cmd.nRst	= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_ALLIANCE_NOT_IN_MEM;
									g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
								}
								else
								{
									// 先通知客户端服务器收到了
									g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));

									// 遍历联盟列表，发送私人邮件
									for (int i=0; i<pAlliance->members.size(); i++)
									{
										g_pLPNet->AddMail(pPlayer->m_CharAtb.nAccountID, pPlayer->m_CharAtb.szName, pAlliance->members[i].nAccountID, false, MAIL_TYPE_ALLIANCE, MAIL_FLAG_NORMAL, false, pAdd->nLen, pAdd->szText, 0, NULL, NULL);
									}
								}
							}
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_BUY_ITEM:
					{
						CTS_GAMECMD_OPERATE_BUY_ITEM_T*	pReq	= (CTS_GAMECMD_OPERATE_BUY_ITEM_T*)pGameCmd;
						if (pReq->nNum > TTY_BUY_ITEM_MAX_NUM)
						{
							STC_GAMECMD_OPERATE_BUY_ITEM_T	Cmd;
							Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd	= STC_GAMECMD_OPERATE_BUY_ITEM;
							Cmd.nMoneyNum	= 0;
							Cmd.nMoneyType	= pReq->nMoneyType;
							Cmd.nExcelID	= pReq->nExcelID;
							Cmd.nItemIDNum	= 0;
							Cmd.nNum		= pReq->nNum;
							Cmd.nRst		= STC_GAMECMD_OPERATE_BUY_ITEM_T::RST_TOO_MANY;

							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							P_DBS4WEB_BUY_ITEM_T	Cmd;
							Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd		= CMDID_BUY_ITEM_REQ;
							Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
							Cmd.nExcelID	= pReq->nExcelID;
							Cmd.nNum		= pReq->nNum;
							Cmd.nMoneyType	= pReq->nMoneyType;
							Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
							
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_SELL_ITEM:
					{
						CTS_GAMECMD_OPERATE_SELL_ITEM_T*	pReq	= (CTS_GAMECMD_OPERATE_SELL_ITEM_T*)pGameCmd;
						P_DBS4WEB_SELL_ITEM_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_SELL_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nItemID		= pReq->nItemID;
						Cmd.nNum		= pReq->nNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE:
					{
						CTS_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T*	pReq	= (CTS_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T*)pGameCmd;
						P_DBS4WEB_FETCH_CHRISTMAS_TREE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_FETCH_CHRISTMAS_TREE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE:
					{
						CTS_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T*	pReq	= (CTS_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T*)pGameCmd;
						P_DBS4WEB_WATERING_CHRISTMAS_TREE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_WATERING_CHRISTMAS_TREE_REQ;
						Cmd.nAccountID	= pReq->nPeerAccountID;
						Cmd.nWateringAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_STEAL_GOLD:
					{
						CTS_GAMECMD_OPERATE_STEAL_GOLD_T*	pReq	= (CTS_GAMECMD_OPERATE_STEAL_GOLD_T*)pGameCmd;
						P_DBS4WEB_STEAL_GOLD_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_STEAL_GOLD_REQ;
						Cmd.nAccountID	= pReq->nPeerAccountID;
						Cmd.nAutoID		= pReq->nAutoID;
						Cmd.nThiefAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ADD_TRAINING:
					{
						CTS_GAMECMD_OPERATE_ADD_TRAINING_T*	pReq	= (CTS_GAMECMD_OPERATE_ADD_TRAINING_T*)pGameCmd;
						if (pReq->nNum>40 || pReq->nNum<=0)	// 不能太多
						{
							return;
						}
						m_vectrawbuf.resize(sizeof(P_DBS4WEB_ADD_TRAINING_T) + pReq->nNum*sizeof(tty_id_t));
						P_DBS4WEB_ADD_TRAINING_T*	pCmd	= (P_DBS4WEB_ADD_TRAINING_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_ADD_TRAINING_REQ;
						pCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
						pCmd->nTimeUnitNum	= pReq->nTimeUnitNum;
						pCmd->nNum			= pReq->nNum;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;
						memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pReq), pCmd->nNum*sizeof(tty_id_t));

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}
					break;
				case CTS_GAMECMD_OPERATE_EXIT_TRAINING:
					{
						CTS_GAMECMD_OPERATE_EXIT_TRAINING_T*	pReq	= (CTS_GAMECMD_OPERATE_EXIT_TRAINING_T*)pGameCmd;
						P_DBS4WEB_EXIT_TRAINING_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_EXIT_TRAINING_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE:
					{
						CTS_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T*	pReq	= (CTS_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T*)pGameCmd;
						P_DBS4WEB_CONTRIBUTE_ALLIANCE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CONTRIBUTE_ALLIANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nGold		= pReq->nGold;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD:
					{
						CTS_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T*	pReq	= (CTS_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T*)pGameCmd;
						P_DBS4WEB_CONVERT_DIAMOND_GOLD_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CONVERT_DIAMOND_GOLD_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nDiamond	= pReq->nDiamond;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_JOIN_ALLI_NAME:
					{
						CTS_GAMECMD_OPERATE_JOIN_ALLI_NAME_T*	pReq		= (CTS_GAMECMD_OPERATE_JOIN_ALLI_NAME_T*)pGameCmd;
						P_DBS4WEB_JOIN_ALLIANCE_NAME_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_JOIN_ALLIANCE_NAME_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						WH_STRNCPY0(Cmd.szAllianceName, pReq->szAllianceName);
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_SET_ALLI_INTRO:
					{
						CTS_GAMECMD_OPERATE_SET_ALLI_INTRO_T*	pReq	= (CTS_GAMECMD_OPERATE_SET_ALLI_INTRO_T*)pGameCmd;
						if (pReq->nLen >= TTY_INTRODUCTION_LEN)
						{
							STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_OPERATE_SET_ALLI_INTRO;
							Cmd.nRst			= STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T::RST_INTRO_TOO_LONG;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else if (pPlayer->m_CharAtb.nAllianceID == 0)
						{
							STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_OPERATE_SET_ALLI_INTRO;
							Cmd.nRst			= STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T::RST_NO_ALLI;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							P_DBS4WEB_SET_ALLI_INTRO_T	Cmd;
							Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd			= CMDID_SET_ALLI_INTRO_REQ;
							Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
							Cmd.nAllianceID		= pPlayer->m_CharAtb.nAllianceID;
							memcpy(Cmd.szIntroduction, pReq->szIntroduction, pReq->nLen);
							Cmd.szIntroduction[pReq->nLen]	= 0;
							Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
// 							if (g_pLPNet->AntiSQLInjectionAttack(sizeof(Cmd.szIntroduction), Cmd.szIntroduction) != 0)
// 							{
// 								STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T	Cmd;
// 								Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 								Cmd.nGameCmd		= STC_GAMECMD_OPERATE_SET_ALLI_INTRO;
// 								Cmd.nRst			= STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T::RST_INTRO_TOO_LONG;
// 								g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
// 							}
// 							else
 							{
								g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
							}
						}
					}
					break;
// 				case CTS_GAMECMD_OPERATE_DRAW_LOTTERY:
// 					{
// 						m_pLotteryMng->DrawOnceLottery();
// 						m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_DRAW_LOTTERY_T) + lottery_probability_max_choice*sizeof(LotteryEntry));
// 						STC_GAMECMD_OPERATE_DRAW_LOTTERY_T*	pRst	= (STC_GAMECMD_OPERATE_DRAW_LOTTERY_T*)m_vectrawbuf.getbuf();
// 						pRst->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 						pRst->nGameCmd			= STC_GAMECMD_OPERATE_DRAW_LOTTERY;
// 						pRst->nRst				= STC_GAMECMD_OPERATE_DRAW_LOTTERY_T::RST_OK;
// 						pRst->nChoice			= m_pLotteryMng->GetCurChoice();
// 						pRst->nNum				= lottery_probability_max_choice;
// 						memcpy(wh_getptrnexttoptr(pRst), m_pLotteryMng->GetCurLotteryEntries().getbuf(), lottery_probability_max_choice*sizeof(LotteryEntry));
// 
// 						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
// 					}
// 					break;
				case CTS_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY:
					{
						CTS_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T*	pReq	= (CTS_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T*)pGameCmd;
						P_DBS4WEB_CHECK_DRAW_LOTTERY_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_CHECK_DRAW_LOTTERY_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nAllianceID			= pPlayer->m_CharAtb.nAllianceID;
						Cmd.nType				= pReq->nType;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_FETCH_LOTTERY:
					{
						CTS_GAMECMD_OPERATE_FETCH_LOTTERY_T*	pReq	= (CTS_GAMECMD_OPERATE_FETCH_LOTTERY_T*)pGameCmd;
						if (pPlayer->m_lottery.nData != 0)
						{
							P_DBS4WEB_FETCH_LOTTERY_T	Cmd;
							Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd				= CMDID_FETCH_LOTTERY_REQ;
							Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
							Cmd.nType				= pPlayer->m_lottery.nType;
							Cmd.nData				= pPlayer->m_lottery.nData;
							Cmd.bTopReward			= pPlayer->m_lottery.bTopReward;
							Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
							Cmd.nExt[1]				= pReq->nMoneyType;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
						else
						{
							STC_GAMECMD_OPERATE_FETCH_LOTTERY_T	Cmd;
							Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd			= STC_GAMECMD_OPERATE_FETCH_LOTTERY;
							Cmd.nType				= lottery_type_none;
							Cmd.nData				= 0;
							Cmd.nItemID				= 0;
							Cmd.nRst				= STC_GAMECMD_OPERATE_FETCH_LOTTERY_T::RST_NO_LOTTERY;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_ALLIANCE_TRADE:
					{
						P_DBS4WEB_ALLIANCE_TRADE_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ALLIANCE_TRADE_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_DELETE_ENEMY:
					{
						CTS_GAMECMD_OPERATE_DELETE_ENEMY_T*	pReq	= (CTS_GAMECMD_OPERATE_DELETE_ENEMY_T*)pGameCmd;
						P_DBS4WEB_DELETE_ENEMY_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_DELETE_ENEMY_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nEnemyID			= pReq->nEnemyID;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_WRITE_SIGNATURE:
					{
						CTS_GAMECMD_OPERATE_WRITE_SIGNATURE_T*	pReq	= (CTS_GAMECMD_OPERATE_WRITE_SIGNATURE_T*)pGameCmd;
						P_DBS4WEB_WRITE_CHAR_SIGNATURE_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_WRITE_CHAR_SIGNATURE_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						WH_STRNCPY0(Cmd.szSignature, pReq->szSignature);
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
// 						if (g_pLPNet->AntiSQLInjectionAttack(sizeof(Cmd.szSignature), Cmd.szSignature) != 0)
// 						{
// 							STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T	RplCmd;
// 							RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 							RplCmd.nGameCmd		= STC_GAMECMD_OPERATE_WRITE_SIGNATURE;
// 							RplCmd.nRst			= STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T::RST_TOO_LONG;
// 							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
// 						}
// 						else
						{
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_CHANGE_NAME:
					{
						CTS_GAMECMD_OPERATE_CHANGE_NAME_T*	pReq	= (CTS_GAMECMD_OPERATE_CHANGE_NAME_T*)pGameCmd;
						if (!g_pLPNet->IsCharNameValid(pReq->szNewName))
						{
							STC_GAMECMD_OPERATE_CHANGE_NAME_T	Cmd;
							Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd			= STC_GAMECMD_OPERATE_CHANGE_NAME;
							Cmd.nDiamond			= 0;
							Cmd.nRst				= STC_GAMECMD_OPERATE_CHANGE_NAME_T::RST_NAME_INVALID;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							P_DBS4WEB_CHANGE_NAME_T	Cmd;
							Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd				= CMDID_CHANGE_NAME_REQ;
							Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
							WH_STRNCPY0(Cmd.szNewName, pReq->szNewName);
							Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_MOVE_CITY:
					{
						CTS_GAMECMD_OPERATE_MOVE_CITY_T*	pReq	= (CTS_GAMECMD_OPERATE_MOVE_CITY_T*)pGameCmd;
						if (!m_pWorldMng->CanMovePosition(pPlayer->m_CharAtb.nPosX, pPlayer->m_CharAtb.nPosY, pReq->nPosX, pReq->nPosY))
						{
							STC_GAMECMD_OPERATE_MOVE_CITY_T		RplCmd;
							RplCmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							RplCmd.nGameCmd			= STC_GAMECMD_OPERATE_MOVE_CITY;
							RplCmd.nRst				= STC_GAMECMD_OPERATE_MOVE_CITY_T::RST_CANNOT_MOVE_CITY;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
							return;
						}
						else
						{
							m_pWorldMng->BookPosition(pReq->nPosX, pReq->nPosY);
							P_DBS4WEB_MOVE_POSITION_T	Cmd;
							Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd				= CMDID_MOVE_POSITION_REQ;
							Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
							Cmd.nSrcPosX			= pPlayer->m_CharAtb.nPosX;
							Cmd.nSrcPosY			= pPlayer->m_CharAtb.nPosY;
							Cmd.nDstPosX			= pReq->nPosX;
							Cmd.nDstPosY			= pReq->nPosY;
							Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
							return;
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_READ_NOTIFICATION:
					{
						CTS_GAMECMD_OPERATE_READ_NOTIFICATION_T*	pReq	= (CTS_GAMECMD_OPERATE_READ_NOTIFICATION_T*)pGameCmd;
						if (pReq->nNotificationID > pPlayer->m_CharAtb.nNotificationID)
						{
							P_DBS4WEB_READ_NOTIFICATION_T	Cmd;
							Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd				= CMDID_READ_NOTIFICATION_REQ;
							Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
							Cmd.nNotificationID		= pReq->nNotificationID;
							Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
						else
						{
							STC_GAMECMD_OPERATE_READ_NOTIFICATION_T	Cmd;
							Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd			= STC_GAMECMD_OPERATE_READ_NOTIFICATION;
							Cmd.nRst				= STC_GAMECMD_OPERATE_READ_NOTIFICATION_T::RST_OK;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_CHANGE_HERO_NAME:
					{
						CTS_GAMECMD_OPERATE_CHANGE_HERO_NAME_T*	pReq	= (CTS_GAMECMD_OPERATE_CHANGE_HERO_NAME_T*)pGameCmd;
						if (!g_pLPNet->IsCharNameValid(pReq->szNewName))
						{
							STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T	Cmd;
							Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd			= STC_GAMECMD_OPERATE_CHANGE_HERO_NAME;
							Cmd.nDiamond			= 0;
							Cmd.nRst				= STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T::RST_NAME_INVALID;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							P_DBS4WEB_CHANGE_HERO_NAME_T	Cmd;
							Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd					= CMDID_CHANGE_HERO_NAME_REQ;
							Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
							Cmd.nHeroID					= pReq->nHeroID;
							WH_STRNCPY0(Cmd.szName, pReq->szNewName);
							Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_ADD_GM_MAIL:
					{
						CTS_GAMECMD_OPERATE_ADD_GM_MAIL_T*	pReq	= (CTS_GAMECMD_OPERATE_ADD_GM_MAIL_T*)pGameCmd;
						if (pReq->nLength > TTY_MAIL_TEXT_LEN)
						{
							STC_GAMECMD_OPERATE_ADD_GM_MAIL_T	Cmd;
							Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd			= STC_GAMECMD_OPERATE_ADD_GM_MAIL;
							Cmd.nRst				= STC_GAMECMD_OPERATE_ADD_GM_MAIL_T::RST_TEXT_TOO_LONG;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							m_vectrawbuf.resize(sizeof(P_DBS4WEB_ADD_GM_MAIL_T) + pReq->nLength + 1);
							P_DBS4WEB_ADD_GM_MAIL_T*	pCmd	= (P_DBS4WEB_ADD_GM_MAIL_T*)m_vectrawbuf.getbuf();
							pCmd->nCmd				= P_DBS4WEB_REQ_CMD;
							pCmd->nSubCmd			= CMDID_ADD_GM_MAIL_REQ;
							pCmd->nAccountID		= pPlayer->m_CharAtb.nAccountID;
							pCmd->nLength			= pReq->nLength+1;
							char*	pText			= (char*)wh_getptrnexttoptr(pCmd);
							memcpy(pText, wh_getptrnexttoptr(pReq), pReq->nLength);
							pText[pReq->nLength]	= 0;
							pCmd->nExt[0]			= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_SET_ALLI_NAME:
					{
						CTS_GAMECMD_OPERATE_SET_ALLI_NAME_T*	pReq	= (CTS_GAMECMD_OPERATE_SET_ALLI_NAME_T*)pGameCmd;
						P_DBS4WEB_CHANGE_ALLIANCE_NAME_T	Cmd;
						Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd					= CMDID_CHANGE_ALLIANCE_NAME_REQ;
						Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
						WH_STRNCPY0(Cmd.szAllianceName, pReq->szName);
						Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_FETCH_MAIL_REWARD:
					{
						CTS_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T*	pReq	= (CTS_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T*)pGameCmd;
						STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T		RplCmd;
						RplCmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						RplCmd.nGameCmd				= STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD;
						RplCmd.nMailID				= pReq->nMailID;
						RplCmd.nNum					= 0;
						RplCmd.nRst					= STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RST_MAIL_ERR;
						whlist<PrivateMailUnit*>::iterator	it	= pPlayer->m_MailList.begin();
						for (; it != pPlayer->m_MailList.end(); ++it)
						{
							PrivateMailUnit*	pMail	= *it;
							if (pMail->nMailID == pReq->nMailID)
							{
								if (pMail->nFlag == MAIL_FLAG_REWARD)
								{
									P_DBS4WEB_FETCH_MAIL_REWARD_T	Cmd;
									Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
									Cmd.nSubCmd					= CMDID_FETCH_MAIL_REWARD_REQ;
									Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
									Cmd.nMailID					= pReq->nMailID;
									Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
									g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
									return;
								}
							}
						}
						g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_UPLOAD_BILL:
					{
						CTS_GAMECMD_OPERATE_UPLOAD_BILL_T*	pReq		= (CTS_GAMECMD_OPERATE_UPLOAD_BILL_T*)pGameCmd;
						if (pReq->nTextLen > max_epoll_msg_data_size)
						{
							STC_GAMECMD_OPERATE_UPLOAD_BILL_T	Cmd;
							Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd			= STC_GAMECMD_OPERATE_UPLOAD_BILL;
							Cmd.nRst				= STC_GAMECMD_OPERATE_UPLOAD_BILL_T::RST_TEXT_TOO_LONG;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));

							// 账单数据太长(开头的-1表示验证开始阶段失败,)
							GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1100,BILL)"-1,0x%"WHINT64PRFX"X,%u,%u,%d,%s,0"
								, pPlayer->m_CharAtb.nAccountID, pReq->nClientTime, wh_time(), pReq->nTextLen, pReq->szText);
						}
						else
						{
							m_vectrawbuf.resize(wh_offsetof(P_DBS4WEB_UPLOAD_BILL_T, szText) + pReq->nTextLen+1);
							P_DBS4WEB_UPLOAD_BILL_T*	pDBCmd	= (P_DBS4WEB_UPLOAD_BILL_T*)m_vectrawbuf.getbuf();
							pDBCmd->nCmd			= P_DBS4WEB_REQ_CMD;
							pDBCmd->nSubCmd			= CMDID_UPLOAD_BILL_REQ;
							pDBCmd->nAccountID		= pPlayer->m_CharAtb.nAccountID;
							pDBCmd->nClientTime		= pReq->nClientTime;
							pDBCmd->nServerTime		= wh_time();
							pDBCmd->nAppID			= pPlayer->m_nAppID;
							pDBCmd->nTextLen		= pReq->nTextLen+1;
							memcpy(pDBCmd->szText, pReq->szText, pReq->nTextLen);
							pDBCmd->szText[pReq->nTextLen]	= 0;
							pDBCmd->nExt[0]			= pPlayer->m_nPlayerGID;
							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());

							GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1100,BILL)"0,0x%"WHINT64PRFX"X,%u,%u,%d,%s,0"
								, pPlayer->m_CharAtb.nAccountID, pReq->nClientTime, pDBCmd->nServerTime, pReq->nTextLen, pDBCmd->szText);
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_REGISTER_ACCOUNT:
					{
						CTS_GAMECMD_OPERATE_REGISTER_ACCOUNT_T*	pReq	= (CTS_GAMECMD_OPERATE_REGISTER_ACCOUNT_T*)pGameCmd;
						char*	pSrcPass			= (char*)wh_getptrnexttoptr(pReq);
						int*	pSrcPassOldLen		= (int*)wh_getoffsetaddr(pSrcPass, pReq->nPassLen);

						STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T	Cmd;
						Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd				= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT;
						Cmd.nRst					= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_OK;

						if (!g_pLPNet->IsAccountNameValid(pReq->szAccount))
						{
							Cmd.nRst				= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_INVALID_NAME;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else if (pReq->nPassLen<0 || *pSrcPassOldLen<0)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,REGISTER)"password invalid,0x%"WHINT64PRFX"X,%d,%d"
								, pPlayer->m_CharAtb.nAccountID, pReq->nPassLen, *pSrcPassOldLen);
							Cmd.nRst				= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_PASS_INVALID_LEN;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else if (pReq->nPassLen>TTY_ACCOUNTPASSWORD_LEN || *pSrcPassOldLen>TTY_ACCOUNTPASSWORD_LEN)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,REGISTER)"password too long,0x%"WHINT64PRFX"X,%d,%d"
								, pPlayer->m_CharAtb.nAccountID, pReq->nPassLen, *pSrcPassOldLen);
							Cmd.nRst				= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_PASS_TOO_LONG;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							m_vectrawbuf.resize(sizeof(GS4WEB_DB_REGISTER_REQ_T) + (pReq->nPassLen+1) + sizeof(int) + (*pSrcPassOldLen+1));
							GS4WEB_DB_REGISTER_REQ_T*	pDBReq	= (GS4WEB_DB_REGISTER_REQ_T*)m_vectrawbuf.getbuf();
							pDBReq->nCmd				= P_GS4WEB_DB_REQ_CMD;
							pDBReq->nSubCmd				= DB_REGISTER_REQ;
							pDBReq->nAccountID			= pPlayer->m_CharAtb.nAccountID;
							WH_STRNCPY0(pDBReq->szAccount, pReq->szAccount);
							pDBReq->nGroupIdx			= g_pLPNet->GetSvrGroupIndex();
							pDBReq->nPassLen			= pReq->nPassLen+1;
							char*	pPass				= (char*)wh_getptrnexttoptr(pDBReq);
							memcpy(pPass, pSrcPass, pReq->nPassLen);
							pPass[pReq->nPassLen]		= 0;
							int*	pPassOldLen			= (int*)wh_getoffsetaddr(pPass, pDBReq->nPassLen);
							*pPassOldLen				= *pSrcPassOldLen;
							char*	pPassOld			= (char*)wh_getptrnexttoptr(pPassOldLen);
							memcpy(pPassOld, wh_getptrnexttoptr(pSrcPassOldLen), *pSrcPassOldLen);
							pPassOld[*pSrcPassOldLen]	= 0;
							pDBReq->nExt[0]				= pPlayer->m_nPlayerGID;
							if (g_pLPNet->SendCmdToGS(m_vectrawbuf.getbuf(), m_vectrawbuf.size()) < 0)
							{
								GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,REGISTER)"SendCmdToGS FAIL,0x%"WHINT64PRFX"X", pPlayer->m_CharAtb.nAccountID);
								Cmd.nRst				= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_CONNECT_GS_FAIL;
								g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
							}
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_BIND_DEVICE:
					{
						CTS_GAMECMD_OPERATE_BIND_DEVICE_T*	pReq	= (CTS_GAMECMD_OPERATE_BIND_DEVICE_T*)pGameCmd;
						
						GS4WEB_DB_BIND_DEVICE_REQ_T	DBCmd;
						DBCmd.nCmd					= P_GS4WEB_DB_REQ_CMD;
						DBCmd.nSubCmd				= DB_BIND_DEVICE_REQ;
						DBCmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						DBCmd.nGroupIdx				= g_pLPNet->GetSvrGroupIndex();
						WH_STRNCPY0(DBCmd.szDeviceID, pReq->szDeviceID);
						DBCmd.nExt[0]				= pPlayer->m_nPlayerGID;
						if (g_pLPNet->SendCmdToGS(&DBCmd, sizeof(DBCmd)) < 0)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,BIND_DEVICE)"SendCmdToGS FAIL,0x%"WHINT64PRFX"X", pPlayer->m_CharAtb.nAccountID);

							STC_GAMECMD_OPERATE_BIND_DEVICE_T	Cmd;
							Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd				= STC_GAMECMD_OPERATE_BIND_DEVICE;
							Cmd.nRst					= STC_GAMECMD_OPERATE_BIND_DEVICE_T::RST_CONNECT_GS_FAIL;
							
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
					}
					break;
				case CTS_GAMECMD_OPERATE_DISMISS_SOLDIER:
					{
						CTS_GAMECMD_OPERATE_DISMISS_SOLDIER_T*	pReq	= (CTS_GAMECMD_OPERATE_DISMISS_SOLDIER_T*)pGameCmd;

						P_DBS4WEB_DISMISS_SOLDIER_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_DISMISS_SOLDIER_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID		= pReq->nExcelID;
						Cmd.nLevel			= pReq->nLevel;
						Cmd.nNum			= pReq->nNum;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER:
					{
						CTS_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T*	pReq	= (CTS_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T*)pGameCmd;

						P_DBS4WEB_ALLI_DONATE_SOLDIER_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_DONATE_SOLDIER_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID		= pReq->nExcelID;
						Cmd.nLevel			= pReq->nLevel;
						Cmd.nNum			= pReq->nNum;
						Cmd.nObjID			= pReq->nObjID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER:
					{
						CTS_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T*	pReq	= (CTS_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T*)pGameCmd;

						P_DBS4WEB_ALLI_RECALL_SOLDIER_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_RECALL_SOLDIER_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nEventID		= pReq->nEventID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER:
					{
						CTS_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T*	pReq	= (CTS_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T*)pGameCmd;

						P_DBS4WEB_ALLI_ACCE_SOLDIER_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_ACCE_SOLDIER_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nEventID		= pReq->nEventID;
						Cmd.nMoneyType		= pReq->nMoneyType;
						Cmd.nTime			= pReq->nTime;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ADD_GOLD_DEAL:
					{
						CTS_GAMECMD_OPERATE_ADD_GOLD_DEAL_T*	pReq	= (CTS_GAMECMD_OPERATE_ADD_GOLD_DEAL_T*)pGameCmd;

						P_DBS4WEB_ADD_GOLD_DEAL_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ADD_GOLD_DEAL_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nGold			= pReq->nGold;
						Cmd.nPrice			= pReq->nPrice;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET:
					{
						CTS_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T*	pReq	= (CTS_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T*)pGameCmd;

						P_DBS4WEB_BUY_GOLD_IN_MARKET_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_BUY_GOLD_IN_MARKET_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nMoneyType		= pReq->nMoneyType;
						Cmd.nTransactionID	= pReq->nTransactionID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CANCEL_GOLD_DEAL:
					{
						CTS_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T*	pReq	= (CTS_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T*)pGameCmd;

						P_DBS4WEB_CANCEL_GOLD_DEAL_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_CANCEL_GOLD_DEAL_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nTransactionID	= pReq->nTransactionID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;

				case CTS_GAMECMD_OPERATE_TAVERN_REFRESH:
					{
						CTS_GAMECMD_OPERATE_TAVERN_REFRESH_T* pRefresh = (CTS_GAMECMD_OPERATE_TAVERN_REFRESH_T*)pGameCmd;
						P_DBS4WEB_TAVERN_REFRESH_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_TAVERN_REFRESH_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nUseItem	= pRefresh->nUseItem;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_HIRE_HERO:
					{
						CTS_GAMECMD_OPERATE_HIRE_HERO_T* pHire = (CTS_GAMECMD_OPERATE_HIRE_HERO_T*)pGameCmd;
						P_DBS4WEB_HIRE_HERO_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_HIRE_HERO_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nSlotID		= pHire->nSlotID;
						Cmd.nHeroID		= pHire->nHeroID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_FIRE_HERO:
					{
						CTS_GAMECMD_OPERATE_FIRE_HERO_T* pFire = (CTS_GAMECMD_OPERATE_FIRE_HERO_T*)pGameCmd;
						P_DBS4WEB_FIRE_HERO_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_FIRE_HERO_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pFire->nHeroID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_LEVELUP_HERO:
					{
						CTS_GAMECMD_OPERATE_LEVELUP_HERO_T* pLevelup = (CTS_GAMECMD_OPERATE_LEVELUP_HERO_T*)pGameCmd;
						P_DBS4WEB_LEVELUP_HERO_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_LEVELUP_HERO_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pLevelup->nHeroID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_ADD_GROW:
					{
						CTS_GAMECMD_OPERATE_ADD_GROW_T* pGrow = (CTS_GAMECMD_OPERATE_ADD_GROW_T*)pGameCmd;
						P_DBS4WEB_ADD_GROW_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ADD_GROW_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pGrow->nHeroID;
						Cmd.fGrow		= pGrow->fGrow;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CONFIG_HERO:
					{
						CTS_GAMECMD_OPERATE_CONFIG_HERO_T* pConfig = (CTS_GAMECMD_OPERATE_CONFIG_HERO_T*)pGameCmd;
						P_DBS4WEB_CONFIG_HERO_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CONFIG_HERO_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pConfig->nHeroID;
						Cmd.nArmyType	= pConfig->nArmyType;
						Cmd.nArmyLevel	= pConfig->nArmyLevel;
						Cmd.nArmyNum	= pConfig->nArmyNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_GET_HERO_REFRESH:
					{
						CTS_GAMECMD_OPERATE_GET_HERO_REFRESH_T* pRefresh = (CTS_GAMECMD_OPERATE_GET_HERO_REFRESH_T*)pGameCmd;
						P_DBS4WEB_GET_HERO_REFRESH_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_HERO_REFRESH_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_GET_HERO_HIRE:
					{
						CTS_GAMECMD_OPERATE_GET_HERO_HIRE_T* pRefresh = (CTS_GAMECMD_OPERATE_GET_HERO_HIRE_T*)pGameCmd;
						P_DBS4WEB_GET_HERO_HIRE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_HERO_HIRE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_START_COMBAT:
					{
						CTS_GAMECMD_OPERATE_START_COMBAT_T* pSC = (CTS_GAMECMD_OPERATE_START_COMBAT_T*)pGameCmd;
						P_DBS4WEB_START_COMBAT_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_START_COMBAT_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nObjID		= pSC->nObjID;
						Cmd.nCombatType	= pSC->nCombatType;
						Cmd.nAutoCombat	= pSC->nAutoCombat;
						Cmd.nAutoSupply	= pSC->nAutoSupply;
						Cmd.n1Hero		= pSC->n1Hero;
						Cmd.n2Hero		= pSC->n2Hero;
						Cmd.n3Hero		= pSC->n3Hero;
						Cmd.n4Hero		= pSC->n4Hero;
						Cmd.n5Hero		= pSC->n5Hero;
						Cmd.nStopLevel	= pSC->nStopLevel;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_STOP_COMBAT:
					{
						CTS_GAMECMD_OPERATE_STOP_COMBAT_T* pSC = (CTS_GAMECMD_OPERATE_STOP_COMBAT_T*)pGameCmd;
						P_DBS4WEB_STOP_COMBAT_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_STOP_COMBAT_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nCombatID	= pSC->nCombatID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_GET_COMBAT:
					{
						CTS_GAMECMD_OPERATE_GET_COMBAT_T* pSC = (CTS_GAMECMD_OPERATE_GET_COMBAT_T*)pGameCmd;
						P_DBS4WEB_GET_COMBAT_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_COMBAT_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nCombatID	= pSC->nCombatID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						// 收到回执后直接发给client
						Cmd.nExt[1]		= 0;
						Cmd.nExt[2]		= 0;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_SAVE_COMBAT:
					{
						CTS_GAMECMD_OPERATE_SAVE_COMBAT_T* pSC = (CTS_GAMECMD_OPERATE_SAVE_COMBAT_T*)pGameCmd;
						P_DBS4WEB_GET_COMBAT_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_COMBAT_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nCombatID	= pSC->nCombatID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						// 收到回执后会计算战斗并存盘
						Cmd.nExt[1]		= 1;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_LIST_COMBAT:
					{
						CTS_GAMECMD_OPERATE_LIST_COMBAT_T* pCombat = (CTS_GAMECMD_OPERATE_LIST_COMBAT_T*)pGameCmd;
						P_DBS4WEB_LIST_COMBAT_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_LIST_COMBAT_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_OPERATE_CONFIG_CITYDEFENSE:
					{
						CTS_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T* pSC = (CTS_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T*)pGameCmd;
						P_DBS4WEB_CONFIG_CITYDEFENSE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CONFIG_CITYDEFENSE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.n1Hero		= pSC->n1Hero;
						Cmd.n2Hero		= pSC->n2Hero;
						Cmd.n3Hero		= pSC->n3Hero;
						Cmd.n4Hero		= pSC->n4Hero;
						Cmd.n5Hero		= pSC->n5Hero;
						Cmd.nAutoSupply	= pSC->nAutoSupply;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_ITEM:
					{
						CTS_GAMECMD_GET_ITEM_T* pReq = (CTS_GAMECMD_GET_ITEM_T*)pGameCmd;
						P_DBS4WEB_GET_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ADD_ITEM:
					{
						CTS_GAMECMD_ADD_ITEM_T* pReq = (CTS_GAMECMD_ADD_ITEM_T*)pGameCmd;
						P_DBS4WEB_ADD_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ADD_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nNum		= pReq->nNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_DEL_ITEM:
					{
						CTS_GAMECMD_DEL_ITEM_T* pReq = (CTS_GAMECMD_DEL_ITEM_T*)pGameCmd;
						P_DBS4WEB_DEL_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEL_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nItemID		= pReq->nItemID;
						Cmd.nNum		= pReq->nNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_EQUIP_ITEM:
					{
						CTS_GAMECMD_EQUIP_ITEM_T* pReq = (CTS_GAMECMD_EQUIP_ITEM_T*)pGameCmd;
						P_DBS4WEB_EQUIP_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_EQUIP_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nItemID		= pReq->nItemID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_DISEQUIP_ITEM:
					{
						CTS_GAMECMD_DISEQUIP_ITEM_T* pReq = (CTS_GAMECMD_DISEQUIP_ITEM_T*)pGameCmd;
						P_DBS4WEB_DISEQUIP_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DISEQUIP_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nItemID		= pReq->nItemID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_MOUNT_ITEM:
					{
						CTS_GAMECMD_MOUNT_ITEM_T* pReq = (CTS_GAMECMD_MOUNT_ITEM_T*)pGameCmd;
						P_DBS4WEB_MOUNT_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_MOUNT_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nEquipID	= pReq->nEquipID;
						Cmd.nSlotIdx	= pReq->nSlotIdx;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_UNMOUNT_ITEM:
					{
						CTS_GAMECMD_UNMOUNT_ITEM_T* pReq = (CTS_GAMECMD_UNMOUNT_ITEM_T*)pGameCmd;
						P_DBS4WEB_UNMOUNT_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_UNMOUNT_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nEquipID	= pReq->nEquipID;
						Cmd.nSlotIdx	= pReq->nSlotIdx;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_COMPOS_ITEM:
					{
						CTS_GAMECMD_COMPOS_ITEM_T* pReq = (CTS_GAMECMD_COMPOS_ITEM_T*)pGameCmd;
						P_DBS4WEB_COMPOS_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_COMPOS_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nNum		= pReq->nNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_DISCOMPOS_ITEM:
					{
						CTS_GAMECMD_DISCOMPOS_ITEM_T* pReq = (CTS_GAMECMD_DISCOMPOS_ITEM_T*)pGameCmd;
						P_DBS4WEB_DISCOMPOS_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DISCOMPOS_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nNum		= pReq->nNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_EQUIP_ITEM_ALL:
					{
						CTS_GAMECMD_EQUIP_ITEM_ALL_T* pReq = (CTS_GAMECMD_EQUIP_ITEM_ALL_T*)pGameCmd;
						P_DBS4WEB_EQUIP_ITEM_ALL_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_EQUIP_ITEM_ALL_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.n1HeadID	= pReq->n1HeadID;
						Cmd.n2ChestID	= pReq->n2ChestID;
						Cmd.n3ShoeID	= pReq->n3ShoeID;
						Cmd.n4WeaponID	= pReq->n4WeaponID;
						Cmd.n5TrinketID	= pReq->n5TrinketID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_MOUNT_ITEM_ALL:
					{
						CTS_GAMECMD_MOUNT_ITEM_ALL_T* pReq = (CTS_GAMECMD_MOUNT_ITEM_ALL_T*)pGameCmd;
						P_DBS4WEB_MOUNT_ITEM_ALL_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_MOUNT_ITEM_ALL_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nEquipID	= pReq->nEquipID;
						Cmd.n1ExcelID	= pReq->n1ExcelID;
						Cmd.n2ExcelID	= pReq->n2ExcelID;
						Cmd.n3ExcelID	= pReq->n3ExcelID;
						Cmd.n4ExcelID	= pReq->n4ExcelID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_QUEST:
					{
						CTS_GAMECMD_GET_QUEST_T* pReq = (CTS_GAMECMD_GET_QUEST_T*)pGameCmd;
						P_DBS4WEB_GET_QUEST_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_QUEST_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_DONE_QUEST:
					{
						CTS_GAMECMD_DONE_QUEST_T* pReq = (CTS_GAMECMD_DONE_QUEST_T*)pGameCmd;
						P_DBS4WEB_DONE_QUEST_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DONE_QUEST_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_INSTANCESTATUS:
					{
						CTS_GAMECMD_GET_INSTANCESTATUS_T* pReq = (CTS_GAMECMD_GET_INSTANCESTATUS_T*)pGameCmd;
						P_DBS4WEB_GET_INSTANCESTATUS_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_INSTANCESTATUS_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_INSTANCEDESC:
					{
						CTS_GAMECMD_GET_INSTANCEDESC_T* pReq = (CTS_GAMECMD_GET_INSTANCEDESC_T*)pGameCmd;
						P_DBS4WEB_GET_INSTANCEDESC_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_INSTANCEDESC_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nClassID	= pReq->nClassID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_CREATE_INSTANCE:
					{
						CTS_GAMECMD_CREATE_INSTANCE_T* pReq = (CTS_GAMECMD_CREATE_INSTANCE_T*)pGameCmd;
						P_DBS4WEB_CREATE_INSTANCE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CREATE_INSTANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nClassID	= pReq->nClassID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_JOIN_INSTANCE:
					{
						CTS_GAMECMD_JOIN_INSTANCE_T* pReq = (CTS_GAMECMD_JOIN_INSTANCE_T*)pGameCmd;
						P_DBS4WEB_JOIN_INSTANCE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_JOIN_INSTANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_QUIT_INSTANCE:
					{
						CTS_GAMECMD_QUIT_INSTANCE_T* pReq = (CTS_GAMECMD_QUIT_INSTANCE_T*)pGameCmd;
						P_DBS4WEB_QUIT_INSTANCE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_QUIT_INSTANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_DESTROY_INSTANCE:
					{
						CTS_GAMECMD_DESTROY_INSTANCE_T* pReq = (CTS_GAMECMD_DESTROY_INSTANCE_T*)pGameCmd;
						P_DBS4WEB_DESTROY_INSTANCE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DESTROY_INSTANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_INSTANCEDATA:
					{
						CTS_GAMECMD_GET_INSTANCEDATA_T* pReq = (CTS_GAMECMD_GET_INSTANCEDATA_T*)pGameCmd;
						P_DBS4WEB_GET_INSTANCEDATA_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_INSTANCEDATA_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nExt[1]		= 0;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_CONFIG_INSTANCEHERO:
					{
						CTS_GAMECMD_CONFIG_INSTANCEHERO_T* pSC = (CTS_GAMECMD_CONFIG_INSTANCEHERO_T*)pGameCmd;
						P_DBS4WEB_CONFIG_INSTANCEHERO_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CONFIG_INSTANCEHERO_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pSC->nExcelID;
						Cmd.nInstanceID	= pSC->nInstanceID;
						Cmd.n1Hero		= pSC->n1Hero;
						Cmd.n2Hero		= pSC->n2Hero;
						Cmd.n3Hero		= pSC->n3Hero;
						Cmd.n4Hero		= pSC->n4Hero;
						Cmd.n5Hero		= pSC->n5Hero;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_INSTANCELOOT:
					{
						CTS_GAMECMD_GET_INSTANCELOOT_T* pReq = (CTS_GAMECMD_GET_INSTANCELOOT_T*)pGameCmd;
						P_DBS4WEB_GET_INSTANCELOOT_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_INSTANCELOOT_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_AUTO_COMBAT:
					{
						CTS_GAMECMD_AUTO_COMBAT_T* pCombat = (CTS_GAMECMD_AUTO_COMBAT_T*)pGameCmd;
						P_DBS4WEB_AUTO_COMBAT_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_AUTO_COMBAT_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID	= pCombat->nInstanceID;
						Cmd.nVal		= pCombat->nVal;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_USE_ITEM:
					{
						CTS_GAMECMD_USE_ITEM_T *pReq	= (CTS_GAMECMD_USE_ITEM_T*)pGameCmd;
						P_DBS4WEB_USE_ITEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_USE_ITEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nNum		= pReq->nNum;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_SYNC_CHAR:
					{
						CTS_GAMECMD_SYNC_CHAR_T *pReq	= (CTS_GAMECMD_SYNC_CHAR_T*)pGameCmd;
						P_DBS4WEB_SYNC_CHAR_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_SYNC_CHAR_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_SYNC_HERO:
					{
						CTS_GAMECMD_SYNC_HERO_T *pReq	= (CTS_GAMECMD_SYNC_HERO_T*)pGameCmd;
						P_DBS4WEB_SYNC_HERO_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_SYNC_HERO_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_AUTO_SUPPLY:
					{
						CTS_GAMECMD_AUTO_SUPPLY_T* pReq = (CTS_GAMECMD_AUTO_SUPPLY_T*)pGameCmd;
						P_DBS4WEB_AUTO_SUPPLY_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_AUTO_SUPPLY_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nVal		= pReq->nVal;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_EQUIP:
					{
						CTS_GAMECMD_GET_EQUIP_T* pReq = (CTS_GAMECMD_GET_EQUIP_T*)pGameCmd;
						P_DBS4WEB_GET_EQUIP_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_EQUIP_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_GEM:
					{
						CTS_GAMECMD_GET_GEM_T* pReq = (CTS_GAMECMD_GET_GEM_T*)pGameCmd;
						P_DBS4WEB_GET_GEM_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_GEM_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_MANUAL_SUPPLY:
					{
						CTS_GAMECMD_MANUAL_SUPPLY_T* pReq = (CTS_GAMECMD_MANUAL_SUPPLY_T*)pGameCmd;
						P_DBS4WEB_MANUAL_SUPPLY_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_MANUAL_SUPPLY_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nCombatType	= pReq->nCombatType;
						Cmd.n1Hero		= pReq->n1Hero;
						Cmd.n2Hero		= pReq->n2Hero;
						Cmd.n3Hero		= pReq->n3Hero;
						Cmd.n4Hero		= pReq->n4Hero;
						Cmd.n5Hero		= pReq->n5Hero;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_REPLACE_EQUIP:
					{
						CTS_GAMECMD_REPLACE_EQUIP_T* pReq = (CTS_GAMECMD_REPLACE_EQUIP_T*)pGameCmd;
						P_DBS4WEB_REPLACE_EQUIP_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_REPLACE_EQUIP_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nHeroID		= pReq->nHeroID;
						Cmd.nEquipType	= pReq->nEquipType;
						Cmd.nItemID		= pReq->nItemID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_WORLDGOLDMINE:
					{
						CTS_GAMECMD_GET_WORLDGOLDMINE_T* pReq = (CTS_GAMECMD_GET_WORLDGOLDMINE_T*)pGameCmd;
						P_DBS4WEB_GET_WORLDGOLDMINE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_WORLDGOLDMINE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nArea		= pReq->nArea;
						Cmd.nClass		= pReq->nClass;
						Cmd.nIdx		= pReq->nIdx;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_CONFIG_WORLDGOLDMINE_HERO:
					{
						CTS_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T* pReq = (CTS_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T*)pGameCmd;
						P_DBS4WEB_CONFIG_WORLDGOLDMINE_HERO_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_CONFIG_WORLDGOLDMINE_HERO_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.n1Hero		= pReq->n1Hero;
						Cmd.n2Hero		= pReq->n2Hero;
						Cmd.n3Hero		= pReq->n3Hero;
						Cmd.n4Hero		= pReq->n4Hero;
						Cmd.n5Hero		= pReq->n5Hero;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ROB_WORLDGOLDMINE:
					{
						CTS_GAMECMD_ROB_WORLDGOLDMINE_T* pReq = (CTS_GAMECMD_ROB_WORLDGOLDMINE_T*)pGameCmd;
						P_DBS4WEB_ROB_WORLDGOLDMINE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ROB_WORLDGOLDMINE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nArea		= pReq->nArea;
						Cmd.nClass		= pReq->nClass;
						Cmd.nIdx		= pReq->nIdx;
						Cmd.n1Hero		= pReq->n1Hero;
						Cmd.n2Hero		= pReq->n2Hero;
						Cmd.n3Hero		= pReq->n3Hero;
						Cmd.n4Hero		= pReq->n4Hero;
						Cmd.n5Hero		= pReq->n5Hero;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_DROP_WORLDGOLDMINE:
					{
						CTS_GAMECMD_DROP_WORLDGOLDMINE_T* pReq = (CTS_GAMECMD_DROP_WORLDGOLDMINE_T*)pGameCmd;
						P_DBS4WEB_DROP_WORLDGOLDMINE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DROP_WORLDGOLDMINE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_MY_WORLDGOLDMINE:
					{
						CTS_GAMECMD_MY_WORLDGOLDMINE_T* pReq = (CTS_GAMECMD_MY_WORLDGOLDMINE_T*)pGameCmd;
						P_DBS4WEB_MY_WORLDGOLDMINE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_MY_WORLDGOLDMINE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_COMBAT_PROF:
					{
						CTS_GAMECMD_COMBAT_PROF_T* pReq = (CTS_GAMECMD_COMBAT_PROF_T*)pGameCmd;
						P_DBS4WEB_COMBAT_PROF_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_COMBAT_PROF_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nClassID	= pReq->nClassID;
						Cmd.nObjID		= pReq->nObjID;
						Cmd.nCombatType	= pReq->nCombatType;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_KICK_INSTANCE:
					{
						CTS_GAMECMD_KICK_INSTANCE_T* pReq = (CTS_GAMECMD_KICK_INSTANCE_T*)pGameCmd;
						P_DBS4WEB_KICK_INSTANCE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_KICK_INSTANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nObjID		= pReq->nObjID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_START_INSTANCE:
					{
						CTS_GAMECMD_START_INSTANCE_T* pReq = (CTS_GAMECMD_START_INSTANCE_T*)pGameCmd;
						P_DBS4WEB_START_INSTANCE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_START_INSTANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_PREPARE_INSTANCE:
					{
						CTS_GAMECMD_PREPARE_INSTANCE_T* pReq = (CTS_GAMECMD_PREPARE_INSTANCE_T*)pGameCmd;
						P_DBS4WEB_PREPARE_INSTANCE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_PREPARE_INSTANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;
						Cmd.nVal		= pReq->nVal;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_SUPPLY_INSTANCE:
					{
						CTS_GAMECMD_SUPPLY_INSTANCE_T* pReq = (CTS_GAMECMD_SUPPLY_INSTANCE_T*)pGameCmd;
						P_DBS4WEB_SUPPLY_INSTANCE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_SUPPLY_INSTANCE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;
						Cmd.nExcelID	= pReq->nExcelID;
						Cmd.nInstanceID	= pReq->nInstanceID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_GET_CITYDEFENSE:
					{
						CTS_GAMECMD_GET_CITYDEFENSE_T* pReq = (CTS_GAMECMD_GET_CITYDEFENSE_T*)pGameCmd;
						P_DBS4WEB_GET_CITYDEFENSE_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_CITYDEFENSE_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_USE_DRUG:
					{
						CTS_GAMECMD_USE_DRUG_T* pReq = (CTS_GAMECMD_USE_DRUG_T*)pGameCmd;
						P_DBS4WEB_USE_DRUG_T *pCmd	= (P_DBS4WEB_USE_DRUG_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_USE_DRUG_REQ;
						pCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;
						pCmd->nNum			= pReq->nNum;
						memcpy((char*)wh_getptrnexttoptr(pCmd), (char*)wh_getptrnexttoptr(pReq), sizeof(tty_id_t)*pReq->nNum);

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd)+sizeof(tty_id_t)*pReq->nNum);
					}
					break;
				case CTS_GAMECMD_ARMY_ACCELERATE:
					{
						CTS_GAMECMD_ARMY_ACCELERATE_T* pReq = (CTS_GAMECMD_ARMY_ACCELERATE_T*)pGameCmd;
						P_DBS4WEB_ARMY_ACCELERATE_T *pCmd	= (P_DBS4WEB_ARMY_ACCELERATE_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_ARMY_ACCELERATE_REQ;
						pCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;
						pCmd->nCombatID		= pReq->nCombatID;
						pCmd->nTime			= pReq->nTime;
						pCmd->byType		= pReq->byType;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd));
					}
					break;
				case CTS_GAMECMD_ARMY_BACK:
					{
						CTS_GAMECMD_ARMY_BACK_T* pReq = (CTS_GAMECMD_ARMY_BACK_T*)pGameCmd;
						P_DBS4WEB_ARMY_BACK_T *pCmd	= (P_DBS4WEB_ARMY_BACK_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_ARMY_BACK_REQ;
						pCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;
						pCmd->nCombatID		= pReq->nCombatID;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd));
					}
					break;
				case CTS_GAMECMD_WORLDCITY_GET:
					{
						CTS_GAMECMD_WORLDCITY_GET_T* pReq = (CTS_GAMECMD_WORLDCITY_GET_T*)pGameCmd;
						P_DBS4WEB_WORLDCITY_GET_T *pCmd	= (P_DBS4WEB_WORLDCITY_GET_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_WORLDCITY_GET_REQ;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd));
					}
					break;
				case CTS_GAMECMD_WORLDCITY_GETLOG:
					{
						CTS_GAMECMD_WORLDCITY_GETLOG_T* pReq = (CTS_GAMECMD_WORLDCITY_GETLOG_T*)pGameCmd;
						P_DBS4WEB_WORLDCITY_GETLOG_T *pCmd	= (P_DBS4WEB_WORLDCITY_GETLOG_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_WORLDCITY_GETLOG_REQ;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;
						pCmd->nPageNum		= pReq->nPageNum;
						pCmd->nPageIdx		= pReq->nPageIdx;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd));
					}
					break;
				case CTS_GAMECMD_WORLDCITY_GETLOG_ALLIANCE:
					{
						CTS_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T* pReq = (CTS_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T*)pGameCmd;
						P_DBS4WEB_WORLDCITY_GETLOG_ALLIANCE_T *pCmd	= (P_DBS4WEB_WORLDCITY_GETLOG_ALLIANCE_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_WORLDCITY_GETLOG_ALLIANCE_REQ;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;
						pCmd->nPageNum		= pReq->nPageNum;
						pCmd->nPageIdx		= pReq->nPageIdx;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd));
					}
					break;
				case CTS_GAMECMD_WORLDCITY_RANK_MAN:
					{
						CTS_GAMECMD_WORLDCITY_RANK_MAN_T* pReq = (CTS_GAMECMD_WORLDCITY_RANK_MAN_T*)pGameCmd;
						P_DBS4WEB_WORLDCITY_RANK_MAN_T *pCmd	= (P_DBS4WEB_WORLDCITY_RANK_MAN_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_WORLDCITY_RANK_MAN_REQ;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;
						pCmd->nPageNum		= pReq->nPageNum;
						pCmd->nPageIdx		= pReq->nPageIdx;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd));
					}
					break;
				case CTS_GAMECMD_WORLDCITY_RANK_ALLIANCE:
					{
						CTS_GAMECMD_WORLDCITY_RANK_ALLIANCE_T* pReq = (CTS_GAMECMD_WORLDCITY_RANK_ALLIANCE_T*)pGameCmd;
						P_DBS4WEB_WORLDCITY_RANK_ALLIANCE_T *pCmd	= (P_DBS4WEB_WORLDCITY_RANK_ALLIANCE_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_WORLDCITY_RANK_ALLIANCE_REQ;
						pCmd->nExt[0]		= pPlayer->m_nPlayerGID;
						pCmd->nPageNum		= pReq->nPageNum;
						pCmd->nPageIdx		= pReq->nPageIdx;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd));
					}
					break;
				case CTS_GAMECMD_RECHARGE_TRY:
					{
						CTS_GAMECMD_RECHARGE_TRY_T* pReq = (CTS_GAMECMD_RECHARGE_TRY_T*)pGameCmd;
						P_DBS4WEB_RECHARGE_TRY_T *pCmd	= (P_DBS4WEB_RECHARGE_TRY_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pCmd->nSubCmd		= CMDID_RECHARGE_TRY_REQ;
						pCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
						pCmd->nDiamond		= pReq->nDiamond;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), sizeof(*pCmd));
					}
					break;

				case CTS_GAMECMD_ALLI_INSTANCE_CREATE:
					{
						CTS_GAMECMD_ALLI_INSTANCE_CREATE_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_CREATE_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_CREATE_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_CREATE_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_JOIN:
					{
						CTS_GAMECMD_ALLI_INSTANCE_JOIN_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_JOIN_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_JOIN_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_JOIN_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_EXIT:
					{
						CTS_GAMECMD_ALLI_INSTANCE_EXIT_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_EXIT_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_EXIT_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_EXIT_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_KICK:
					{
						CTS_GAMECMD_ALLI_INSTANCE_KICK_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_KICK_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_KICK_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_KICK_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.nObjID			= pReq->nObjID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_DESTROY:
					{
						CTS_GAMECMD_ALLI_INSTANCE_DESTROY_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_DESTROY_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_DESTROY_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_DESTROY_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_CONFIG_HERO:
					{
						CTS_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_CONFIG_HERO_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_CONFIG_HERO_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.n1HeroID		= pReq->n1HeroID;
						Cmd.n2HeroID		= pReq->n2HeroID;
						Cmd.n3HeroID		= pReq->n3HeroID;
						Cmd.n4HeroID		= pReq->n4HeroID;
						Cmd.n5HeroID		= pReq->n5HeroID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_START:
					{
						CTS_GAMECMD_ALLI_INSTANCE_START_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_START_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_START_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_START_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_READY:
					{
						CTS_GAMECMD_ALLI_INSTANCE_READY_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_READY_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_READY_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_READY_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.bReady			= pReq->bReady;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_GET_LIST:
					{
						CTS_GAMECMD_ALLI_INSTANCE_GET_LIST_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_GET_LIST_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_GET_LIST_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_GET_LIST_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA:
					{
						CTS_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_GET_CHAR_DATA_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_GET_CHAR_DATA_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA:
					{
						CTS_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_GET_HERO_DATA_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_GET_HERO_DATA_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_GET_STATUS:
					{
						CTS_GAMECMD_ALLI_INSTANCE_GET_STATUS_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_GET_STATUS_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_GET_STATUS_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_GET_STATUS_REQ;
						Cmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
						Cmd.nCombatType		= pReq->nCombatType;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_GET_LOOT:
					{
						CTS_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_GET_LOOT_T		Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_GET_LOOT_REQ;
						Cmd.nInstanceID		= pReq->nInstanceID;
						Cmd.nExt[0]			= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY:
					{
						CTS_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*)pGameCmd;
						unsigned int	nNum		= pReq->nSize/sizeof(HeroDeploy);
						if (nNum==0 || nNum>TTY_MAX_DEPLOY_HERO_NUM)
						{
							STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY;
							Cmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T::RST_ERR_DEPLOY;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
							return;
						}
						HeroDeploy*	pUnit		= (HeroDeploy*)wh_getptrnexttoptr(pReq);
						char	szBuf[1024]		= ",";
						int		nLength			= 1;
						for (int i=0; i<nNum; i++)
						{
							nLength				+= sprintf(szBuf+nLength, "*%d*%d*%"WHINT64PRFX"d", pUnit->nRow, pUnit->nCol, pUnit->nHeroID);
							pUnit++;
						}
						szBuf[nLength]			= 0;

						m_vectrawbuf.resize(sizeof(P_DBS4WEB_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T) + nLength+1);
						P_DBS4WEB_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*	pDBCmd	= (P_DBS4WEB_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*)m_vectrawbuf.getbuf();
						pDBCmd->nCmd			= P_DBS4WEB_REQ_CMD;
						pDBCmd->nSubCmd			= CMDID_ALLI_INSTANCE_SAVE_HERO_DEPLOY_REQ;
						pDBCmd->nAccountID		= pPlayer->m_CharAtb.nAccountID;
						pDBCmd->nDeployDataLen	= nLength+1;
						memcpy(wh_getptrnexttoptr(pDBCmd), szBuf, pDBCmd->nDeployDataLen);
						pDBCmd->nExt[0]			= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_START_COMBAT:
					{
						CTS_GAMECMD_ALLI_INSTANCE_START_COMBAT_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_START_COMBAT_T*)pGameCmd;
						unsigned int	nNum	= pReq->nSize/sizeof(HeroDeploy);
						if (nNum>TTY_MAX_DEPLOY_HERO_NUM)
						{
							STC_GAMECMD_ALLI_INSTANCE_START_COMBAT_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_ALLI_INSTANCE_START_COMBAT;
							Cmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_START_COMBAT_T::RST_NO_HERO;
							Cmd.nMarchTime		= 0;
							
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							HeroDeploy*	pUnit	= (HeroDeploy*)wh_getptrnexttoptr(pReq);
							char	szBuf[1024]	= "";
							int		nLength		= 0;
							if (nNum > 0)
							{
								szBuf[0]		= ',';
								nLength			= 1;
								for (int i=0; i<nNum; i++)
								{
									nLength		+= sprintf(szBuf+nLength, "*%d*%d*%"WHINT64PRFX"d", pUnit->nRow, pUnit->nCol, pUnit->nHeroID);
									pUnit++;
								}
								szBuf[nLength]	= 0;
							}
							
							m_vectrawbuf.resize(sizeof(P_DBS4WEB_ALLI_INSTANCE_START_COMBAT_T) + nLength+1);
							P_DBS4WEB_ALLI_INSTANCE_START_COMBAT_T*	pDBCmd	= (P_DBS4WEB_ALLI_INSTANCE_START_COMBAT_T*)m_vectrawbuf.getbuf();
							pDBCmd->nCmd			= P_DBS4WEB_REQ_CMD;
							pDBCmd->nSubCmd			= CMDID_ALLI_INSTANCE_START_COMBAT_REQ;
							pDBCmd->nObjID			= pReq->nObjID;
							pDBCmd->nCombatType		= pReq->nCombatType;
							pDBCmd->bAutoCombat		= pReq->bAutoCombat;
							pDBCmd->bAutoSupply		= pReq->bAutoSupply;
							pDBCmd->nAccountID		= pPlayer->m_CharAtb.nAccountID;
							pDBCmd->nStopLevel		= pReq->nStopLevel;
							pDBCmd->nLength			= nLength+1;
							memcpy(wh_getptrnexttoptr(pDBCmd), szBuf, pDBCmd->nLength);
							pDBCmd->nExt[0]			= pPlayer->m_nPlayerGID;

							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG:
					{
						CTS_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_GET_COMBAT_LOG_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ALLI_INSTANCE_GET_COMBAT_LOG_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nCombatType			= pReq->nCombatType;
						Cmd.nID					= pReq->nID;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG:
					{
						CTS_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nCombatType			= pReq->nCombatType;
						Cmd.nInstanceID			= pReq->nInstanceID;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_STOP_COMBAT:
					{
						CTS_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_STOP_COMBAT_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ALLI_INSTANCE_STOP_COMBAT_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID			= pReq->nInstanceID;
						Cmd.nCombatType			= pReq->nCombatType;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
						
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_SUPPLY:
					{
						CTS_GAMECMD_ALLI_INSTANCE_SUPPLY_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_SUPPLY_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_SUPPLY_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ALLI_INSTANCE_SUPPLY_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nInstanceID			= pReq->nInstanceID;
						Cmd.nCombatType			= pReq->nCombatType;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY:
					{
						CTS_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*	pReq	= (CTS_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*)pGameCmd;
						P_DBS4WEB_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_REQ;
						Cmd.nExcelID			= pReq->nExcelID;
						Cmd.nLevel				= pReq->nLevel;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
						
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;

				case CTS_GAMECMD_ARENA_UPLOAD_DATA:
					{
						CTS_GAMECMD_ARENA_UPLOAD_DATA_T*	pReq	= (CTS_GAMECMD_ARENA_UPLOAD_DATA_T*)pGameCmd;
						unsigned int	nNum	= pReq->nSize/sizeof(HeroDeploy);
						if (nNum==0 || nNum>TTY_MAX_DEPLOY_HERO_NUM)
						{
							STC_GAMECMD_ARENA_UPLOAD_DATA_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_ARENA_UPLOAD_DATA;
							Cmd.nRst			= STC_GAMECMD_ARENA_UPLOAD_DATA_T::RST_FORCE_0;
							Cmd.nRank			= 0;
							Cmd.bFirstUpload	= false;

							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							HeroDeploy*	pUnit		= (HeroDeploy*)wh_getptrnexttoptr(pReq);
							char	szBuf[1024]		= ",";
							int		nLength			= 1;
							for (int i=0; i<nNum; i++)
							{
								nLength				+= sprintf(szBuf+nLength, "*%d*%d*%"WHINT64PRFX"d", pUnit->nRow, pUnit->nCol, pUnit->nHeroID);
								pUnit++;
							}
							szBuf[nLength]			= 0;
							m_vectrawbuf.resize(sizeof(P_DBS4WEB_ARENA_UPLOAD_DATA_T) + nLength+1);
							P_DBS4WEB_ARENA_UPLOAD_DATA_T*		pDBCmd	= (P_DBS4WEB_ARENA_UPLOAD_DATA_T*)m_vectrawbuf.getbuf();
							pDBCmd->nCmd			= P_DBS4WEB_REQ_CMD;
							pDBCmd->nSubCmd			= CMDID_ARENA_UPLOAD_DATA_REQ;
							pDBCmd->nAccountID		= pPlayer->m_CharAtb.nAccountID;
							pDBCmd->nLength			= nLength+1;
							memcpy(wh_getptrnexttoptr(pDBCmd), szBuf, pDBCmd->nLength);
							pDBCmd->nExt[0]			= pPlayer->m_nPlayerGID;

							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
					}
					break;
				case CTS_GAMECMD_ARENA_GET_RANK_LIST:
					{
						CTS_GAMECMD_ARENA_GET_RANK_LIST_T*	pReq	= (CTS_GAMECMD_ARENA_GET_RANK_LIST_T*)pGameCmd;
						P_DBS4WEB_ARENA_GET_RANK_LIST_T		Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ARENA_GET_RANK_LIST_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nPage				= pReq->nPage;
						Cmd.nPageSize			= pReq->nPageSize;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ARENA_GET_STATUS:
					{
						CTS_GAMECMD_ARENA_GET_STATUS_T*	pReq	= (CTS_GAMECMD_ARENA_GET_STATUS_T*)pGameCmd;
						P_DBS4WEB_ARENA_GET_STATUS_T		Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ARENA_GET_STATUS_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nNum				= pReq->nNum;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ARENA_PAY:
					{
						CTS_GAMECMD_ARENA_PAY_T*	pReq	= (CTS_GAMECMD_ARENA_PAY_T*)pGameCmd;
						P_DBS4WEB_ARENA_PAY_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ARENA_PAY_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nMoneyType			= pReq->nMoneyType;
						Cmd.nNum				= pReq->nNum;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_ARENA_GET_DEPLOY:
					{
						P_DBS4WEB_ARENA_GET_DEPLOY_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_ARENA_GET_DEPLOY_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_HERO_SUPPLY:
					{
						CTS_GAMECMD_HERO_SUPPLY_T*	pReq	= (CTS_GAMECMD_HERO_SUPPLY_T*)pGameCmd;
						if (pReq->nNum > TTY_MAX_DEPLOY_HERO_NUM)
						{
							STC_GAMECMD_HERO_SUPPLY_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_HERO_SUPPLY;
							Cmd.nRst			= STC_GAMECMD_HERO_SUPPLY_T::RST_TOO_MANY_HERO;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							char	szBuf[1024]		= "";
							unsigned int	nLength	= 0;
							tty_id_t*	pID			= (tty_id_t*)wh_getptrnexttoptr(pReq);
							for (int i=0; i<pReq->nNum; i++)
							{
								nLength				+= sprintf(szBuf+nLength, ",%"WHINT64PRFX"d", *pID);
								pID++;
							}
							szBuf[nLength]			= 0;
						
							m_vectrawbuf.resize(sizeof(P_DBS4WEB_HERO_SUPPLY_T) + nLength+1);
							P_DBS4WEB_HERO_SUPPLY_T*	pDBCmd	= (P_DBS4WEB_HERO_SUPPLY_T*)m_vectrawbuf.getbuf();
							pDBCmd->nCmd			= P_DBS4WEB_REQ_CMD;
							pDBCmd->nSubCmd			= CMDID_HERO_SUPPLY_REQ;
							pDBCmd->nCombatType		= pReq->nCombatType;
							pDBCmd->nAccountID		= pPlayer->m_CharAtb.nAccountID;
							pDBCmd->nExt[0]			= pPlayer->m_nPlayerGID;
							memcpy(wh_getptrnexttoptr(pDBCmd), szBuf, nLength+1);
							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
					}
					break;

				case CTS_GAMECMD_OPERATE_SET_VIP_DISPLAY:
					{
						CTS_GAMECMD_OPERATE_SET_VIP_DISPLAY_T*	pReq	= (CTS_GAMECMD_OPERATE_SET_VIP_DISPLAY_T*)pGameCmd;
						P_DBS4WEB_SET_VIP_DISPLAY_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_SET_VIP_DISPLAY_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.bVipDisplay			= pReq->bVipDisplay;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;

				case CTS_GAMECMD_POSITION_MARK_ADD_RECORD:
					{
						CTS_GAMECMD_POSITION_MARK_ADD_RECORD_T*	pReq	= (CTS_GAMECMD_POSITION_MARK_ADD_RECORD_T*)pGameCmd;
						PositionMark*	pUnit	= (PositionMark*)wh_getptrnexttoptr(pReq);
						if (pUnit->nLength > TTY_POSITION_MARK_PROMPT_LEN)
						{
							STC_GAMECMD_POSITION_MARK_ADD_RECORD_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_POSITION_MARK_ADD_RECORD;
							Cmd.nRst			= STC_GAMECMD_POSITION_MARK_ADD_RECORD_T::RST_PROMPT_TOO_LONG;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							m_vectrawbuf.resize(sizeof(P_DBS4WEB_POSITION_MARK_ADD_RECORD_T) + pUnit->Size());
							P_DBS4WEB_POSITION_MARK_ADD_RECORD_T*	pDBCmd	= (P_DBS4WEB_POSITION_MARK_ADD_RECORD_T*)m_vectrawbuf.getbuf();
							pDBCmd->nCmd		= P_DBS4WEB_REQ_CMD;
							pDBCmd->nSubCmd		= CMDID_POSITION_MARK_ADD_RECORD_REQ;
							pDBCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
							pDBCmd->nExt[0]		= pPlayer->m_nPlayerGID;
							memcpy(wh_getptrnexttoptr(pDBCmd), pUnit, pUnit->Size());
							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
					}
					break;
				case CTS_GAMECMD_POSITION_MARK_CHG_RECORD:
					{
						CTS_GAMECMD_POSITION_MARK_CHG_RECORD_T*	pReq	= (CTS_GAMECMD_POSITION_MARK_CHG_RECORD_T*)pGameCmd;
						PositionMark*	pUnit	= (PositionMark*)wh_getptrnexttoptr(pReq);
						if (pUnit->nLength > TTY_POSITION_MARK_PROMPT_LEN)
						{
							STC_GAMECMD_POSITION_MARK_CHG_RECORD_T	Cmd;
							Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd		= STC_GAMECMD_POSITION_MARK_CHG_RECORD;
							Cmd.nRst			= STC_GAMECMD_POSITION_MARK_CHG_RECORD_T::RST_PROMPT_TOO_LONG;
							g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
						}
						else
						{
							m_vectrawbuf.resize(sizeof(P_DBS4WEB_POSITION_MARK_CHG_RECORD_T) + pUnit->Size());
							P_DBS4WEB_POSITION_MARK_CHG_RECORD_T*	pDBCmd	= (P_DBS4WEB_POSITION_MARK_CHG_RECORD_T*)m_vectrawbuf.getbuf();
							pDBCmd->nCmd		= P_DBS4WEB_REQ_CMD;
							pDBCmd->nSubCmd		= CMDID_POSITION_MARK_CHG_RECORD_REQ;
							pDBCmd->nAccountID	= pPlayer->m_CharAtb.nAccountID;
							pDBCmd->nExt[0]		= pPlayer->m_nPlayerGID;
							memcpy(wh_getptrnexttoptr(pDBCmd), pUnit, pUnit->Size());
							g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
					}
					break;
				case CTS_GAMECMD_POSITION_MARK_DEL_RECORD:
					{
						CTS_GAMECMD_POSITION_MARK_DEL_RECORD_T*	pReq	= (CTS_GAMECMD_POSITION_MARK_DEL_RECORD_T*)pGameCmd;
						P_DBS4WEB_POSITION_MARK_DEL_RECORD_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_POSITION_MARK_DEL_RECORD_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nPosX				= pReq->nPosX;
						Cmd.nPosY				= pReq->nPosY;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				case CTS_GAMECMD_POSITION_MARK_GET_RECORD:
					{
						CTS_GAMECMD_POSITION_MARK_GET_RECORD_T*	pReq	= (CTS_GAMECMD_POSITION_MARK_GET_RECORD_T*)pGameCmd;
						P_DBS4WEB_POSITION_MARK_GET_RECORD_T	Cmd;
						Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd				= CMDID_POSITION_MARK_GET_RECORD_REQ;
						Cmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]				= pPlayer->m_nPlayerGID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				}
			}
		}
		break;
	case LPPACKET_2GP_PLAYEROFFLINE:
		{
			LPPACKET_2GP_PLAYEROFFLINE_T*	pOffline	= (LPPACKET_2GP_PLAYEROFFLINE_T*)pData;
			PlayerOffline(pOffline->nClientID);
		}
		break;
	case PNGSPACKET_2DB4WEBUSER_RPL:
		{
			P_DBS4WEB_CMD_T*	pBaseCmd	= (P_DBS4WEB_CMD_T*)pData;
			switch (pBaseCmd->nSubCmd)
			{
			case CMDID_LOAD_CHAR_RPL:
				{
					P_DBS4WEBUSER_LOAD_CHAR_T*	pRst	= (P_DBS4WEBUSER_LOAD_CHAR_T*)pBaseCmd;
					PlayerData*	pPlayer		= m_pPlayerMng->AddPlayer(pRst->nExt[1]);
					if (pPlayer == NULL)
					{
						// 踢下线
						g_pLPNet->KickPlayerOffline(pRst->nExt[1]);
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,FATAL_ERR)"CANNOT AddPlayer,0x%X", pRst->nExt[1]);
						return;
					}
					else
					{
						pPlayer->m_nPlayerGID	= pRst->nExt[1];
						pPlayer->m_nTermType	= pRst->nExt[2];
						pPlayer->m_nAppID		= g_pLPNet->GetPlayerAppID(pPlayer->m_nPlayerGID);
						pPlayer->LoadFromDBPacket(wh_getptrnexttoptr(pRst));
						if (pPlayer->m_CharAtb.nAllianceID != 0)
						{
							g_pLPNet->SetPlayerTag64(pPlayer->m_nPlayerGID, pPlayer->m_CharAtb.nAllianceID, false);

							// 修改联盟列表中的成员信息
							AllianceData_T*	pAlliance	= m_pAllianceMng->GetAlliance(pPlayer->m_CharAtb.nAllianceID);
							if (pAlliance == NULL)
							{
								LoadAllianceFromDB(pPlayer->m_CharAtb.nAllianceID);
							}
							else
							{
								int	nPos	= pAlliance->members.findvalue(AllianceMemberUnit(pPlayer->m_CharAtb.nAccountID));
								if (nPos < 0)
								{
									// 有错误了,需要纠正
									LoadAllianceFromDB(pPlayer->m_CharAtb.nAllianceID);
									GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(,GameMngS)"Alliance Member Not IN Memory,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
										, pPlayer->m_CharAtb.nAllianceID, pPlayer->m_CharAtb.nAccountID);
								}
								else
								{
									// 重新设置一下名字就行
									WH_STRNCPY0(pAlliance->members[nPos].szName, pPlayer->m_CharAtb.szName);
								}
							}
						}
					}

					m_pWorldMng->SetTileType(pPlayer->m_CharAtb.nPosX, pPlayer->m_CharAtb.nPosY, TileUnit::TILETYPE_TERRAIN, pPlayer->m_CharAtb.nAccountID);
					PlayerCard* pPlayerCard		= m_pPlayerCardMng->GetCardByAccountID(pPlayer->m_CharAtb.nAccountID, false);
					bool	bNewPlayer			= false;
					if (pPlayerCard == NULL)
					{
						pPlayerCard				= m_pPlayerCardMng->AddCard(pPlayer->m_CharAtb.nAccountID);
						bNewPlayer				= true;
					}
					if (pPlayerCard == NULL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,FATAL_ERR)"CANNOT AddPlayerCard,0x%X,0x%"WHINT64PRFX"X", pRst->nExt[1], pPlayer->m_CharAtb.nAccountID);
					}
					else
					{
						pPlayerCard->nAccountID		= pPlayer->m_CharAtb.nAccountID;
						WH_STRNCPY0(pPlayerCard->szName, pPlayer->m_CharAtb.szName);
						pPlayerCard->nLevel			= pPlayer->m_CharAtb.nLevel;
						pPlayerCard->nVip			= pPlayer->m_CharAtb.nVip;
						pPlayerCard->nSex			= pPlayer->m_CharAtb.nSex;
						pPlayerCard->nHeadID		= pPlayer->m_CharAtb.nHeadID;
						pPlayerCard->nPosX			= pPlayer->m_CharAtb.nPosX;
						pPlayerCard->nPosY			= pPlayer->m_CharAtb.nPosY;
						pPlayerCard->nAllianceID	= pPlayer->m_CharAtb.nAllianceID;
						pPlayerCard->nDevelopment	= pPlayer->m_CharAtb.nDevelopment;
						WH_STRNCPY0(pPlayerCard->szAllianceName, pPlayer->m_CharAtb.szAllianceName);
						pPlayerCard->nProtectTime	= pPlayer->m_CharAtb.nProtectTime;
						pPlayerCard->nProtectTimeEndTick	= pPlayer->m_CharAtb.nProtectTimeEndTick;
						WH_STRNCPY0(pPlayerCard->szSignature, pPlayer->m_CharAtb.szSignature);
						pPlayerCard->nInstanceWangzhe	= pPlayer->m_CharAtb.nInstanceWangzhe;
						pPlayerCard->nInstanceZhengzhan	= pPlayer->m_CharAtb.nInstanceZhengzhan;
						pPlayerCard->nCup				= pPlayer->m_CharAtb.nCup;
						pPlayerCard->bVipDisplay		= pPlayer->m_CharAtb.bVipDisplay;
						// city_level在building_list载入时初始化,level_rank在每次重载排名表时初始化

						if (bNewPlayer)
						{
							pPlayerCard->nLevelRank	= m_pRankMng->GetRank(rank_type_char_level, pPlayer->m_CharAtb.nAccountID);
							pPlayerCard->nCityLevel	= 1;
							m_pPlayerCardMng->AddCharName2AccountNameMap(pPlayer->m_CharAtb.szName, pPlayer->m_CharAtb.nAccountID);
						}
					}

					// 载入邮件列表
					{
						P_DBS4WEB_GET_PRIVATE_MAIL_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_PRIVATE_MAIL_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExt[0]		= pPlayer->m_nPlayerGID;

						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					//////////////////////////////////////////////////////////////////////////
					// 注册人口生产时间事件
					//////////////////////////////////////////////////////////////////////////
					AddPopulationProductionTE(pPlayer->m_CharAtb.nAccountID);

					// 看看有没有队伍
					{
						P_DBS4WEB_GET_INSTANCE_ID_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_GET_INSTANCE_ID_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						Cmd.nExcelID	= 101;	// 南征北战副本excel ID
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}

					// 看看有没有联盟副本
					{
						P_DBS4WEB_ALLI_INSTANCE_GET_INSTANCE_ID_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_INSTANCE_ID_REQ;
						Cmd.nAccountID	= pPlayer->m_CharAtb.nAccountID;
						g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
					}
				}
				break;
			}
		}
		break;
	}
}
int		CGameMngS::PlayerOffline(int nClientID)
{
	PlayerData*	pPlayer	= m_pPlayerMng->GetPlayerByGID(nClientID);
	if (pPlayer!=NULL && pPlayer->m_CharAtb.nAllianceID!=0)
	{
		g_pLPNet->SetPlayerTag64(pPlayer->m_nPlayerGID, pPlayer->m_CharAtb.nAllianceID, true);
	}
	m_pPlayerMng->DeletePlayer(nClientID);
	return 0;
}
int		CGameMngS::AddPopulationProductionTE(tty_id_t nAccountID)
{
	P_DBS4WEB_ADD_ONCE_PERPERSON_TE_T	Cmd;
	Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd		= CMDID_ADD_ONCE_PERPERSON_TE_REQ;
	Cmd.nAccountID	= nAccountID;
	Cmd.nType		= te_subtype_once_population;
	g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

	return 0;
}
int		CGameMngS::AddEnemy(tty_id_t nAccountID, tty_id_t nEnemyID)
{
	P_DBS4WEB_ADD_ENEMY_T	Cmd;
	Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd		= CMDID_ADD_ENEMY_REQ;
	Cmd.nAccountID	= nAccountID;
	Cmd.nEnemyID	= nEnemyID;
	g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

	return 0;
}
int		CGameMngS::AddAllianceDevelopment(tty_id_t nAllianceID, int nDeltaDevelopment)
{
	if (nAllianceID == 0)
	{
		return -1;
	}
	if (nDeltaDevelopment == 0)
	{
		return -2;
	}

	P_DBS4WEB_ADD_ALLIANCE_DEV_T	Cmd;
	Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd		= CMDID_ADD_ALLIANCE_DEV_REQ;
	Cmd.nAllianceID	= nAllianceID;
	Cmd.nDeltaDevelopment	= nDeltaDevelopment;
	g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

	return 0;
}

int		CGameMngS::CombatCalc_Raid(CombatUnit *aA, CombatUnit *aD, P_DBS4WEBUSER_GET_COMBAT_T *pData)
{
	int nProfResA = 0;
	int nProfResD = 0;
	// 读出数据
	CombatUnit *p = (CombatUnit*)wh_getptrnexttoptr(pData);
	// 攻方
	for (int i=0; i<pData->nNumA; ++i)
	{
		memcpy(&aA[p->nSlot-1], p, sizeof(*p));
		++ p;
	}
	// 守方
	for (int i=0; i<pData->nNumD; ++i)
	{
		memcpy(&aD[p->nSlot-1], p, sizeof(*p));
		++ p;
	}
	// 战斗开始
	// 通道上的部队按兵种速度朝对方大本营行进，与敌方相遇即开战
	// 由于部队不会攻击到其他通道，因此距离已经没什么意义，服务器可以直接进入攻防计算阶段
	int i;
	for (i=0; i<COMBATFIELD_CHANNEL; ++i)
	{
		int nArmyNumA = aA[i].nArmyNum;
		int nArmyNumD = aD[i].nArmyNum;
		int nProfA = 0;
		int nProfD = 0;
		CombatCalc_Raid_Round(&aA[i], &aD[i], nProfA, nProfD);
		nProfResA += nProfA;
		nProfResD += nProfD;
		if (aA[i].nSlot>0)
		{
		//	UpdateHeroToDB(aA[i].nHeroID, aA[i].nArmyNum-nArmyNumA);
		}
		// 对手是玩家的话，也需要更新
		if (aD[i].nSlot>0 && 
			(pData->nCombatType==COMBAT_PVP_ATTACK ||
			pData->nCombatType==COMBAT_PVP_WORLDGOLDMINE_ATTACK)
			)
		{
		//	UpdateHeroToDB(aD[i].nHeroID, aD[i].nArmyNum-nArmyNumD);
		}
	}
	// 遭遇战后，以剩余部队战力多少决定战斗胜负
/*
	// 为了简单，目前就只考虑攻击力
	int nAttackA = 0;
	int nAttackD = 0;
	for (i=0; i<COMBATFIELD_CHANNEL; ++i)
	{
		nAttackA += aA[i].nAttack;
		nAttackD += aD[i].nAttack;
	}
	if (nAttackA > nAttackD)
	{
		// 攻方胜利
		return COMBAT_RST_VICTORY;
	}
	// 守方胜利
	return COMBAT_RST_LOSE;
*/
	// 2012-11-22 根据剩余战力定胜负
	if (nProfResA >= nProfResD)
	{
		// 攻方胜利
		return COMBAT_RST_VICTORY;
	}
	// 守方胜利
	return COMBAT_RST_LOSE;
}
int		CGameMngS::CombatCalc_Raid_Round(CombatUnit *pA, CombatUnit *pD, int &nProfResA, int &nProfResD)
{
//	int nHealthA = pA->nHealth;
//	int nHealthD = pD->nHealth;
	
	// 有兵种克制吗
	float nMA = 1;
	float nMD = 1;
	if (pD->nArmyType==m_army[pA->nArmyType].aCrush[0] || pD->nArmyType==m_army[pA->nArmyType].aCrush[1])
	{
		// 攻击加成
		nMA += m_army[pA->nArmyType].fBonus;
	}
	if (pA->nArmyType==m_army[pD->nArmyType].aCrush[0] || pA->nArmyType==m_army[pD->nArmyType].aCrush[1])
	{
		nMD += m_army[pD->nArmyType].fBonus;
	}
/*
	// 这里的攻击/防御/生命都是经过数量加成的了，可以直接用
	while (pA->nHealth>0 && pD->nHealth>0)
	{
		// 攻方回合
		int nDamageA = pA->nAttack*nMA - pD->nDefense;
		if (nDamageA > 0)
		{
			pD->nHealth -= nDamageA;
		}
		else
		{
			// 如果双方攻防正好抵消，那么还是要算上一些伤害的
			if (nDamageA==0)
			{
				pD->nHealth -= 10;
			}
		}
		// 守方回合
		int nDamageD = pD->nAttack*nMD - pA->nDefense;
		if (nDamageD > 0)
		{
			pA->nHealth -= nDamageD;
		}
		else
		{
			// 如果双方攻防正好抵消，那么还是要算上一些伤害的
			if (nDamageD==0)
			{
				pA->nHealth -= 10;
			}
		}
		if (nDamageA<0 && nDamageD<0)
		{
			// 双方攻击都不能造成伤害时，还是要比较一下
			if (nDamageA>=nDamageD)
			{
				pD->nHealth -= 10;
			}
			else
			{
				pA->nHealth -= 10;
			}
		}
		// 可以退出了吗
		if (pD->nHealth <= 0)
		{
			pD->nHealth = 0;
			if (pA->nHealth <= 0)
			{
				pA->nHealth += nDamageD;
			}
			goto combatcalc_round_end;
		}
		if (pA->nHealth <= 0)
		{
			pA->nHealth = 0;
			if (pD->nHealth <= 0)
			{
				pD->nHealth += nDamageA;
			}
			goto combatcalc_round_end;
		}
	}
combatcalc_round_end:
	// 计算部队剩余数量
	float fA = (nHealthA>0)? (float)pA->nHealth/(float)nHealthA: 0;
	float fD = (nHealthD>0)? (float)pD->nHealth/(float)nHealthD: 0;
	pA->nArmyNum = (int)(fA*pA->nArmyNum);
	pD->nArmyNum = (int)(fD*pD->nArmyNum);
	pA->nAttack = (int)(fA*pA->nAttack);
	pD->nAttack = (int)(fD*pD->nAttack);
	pA->nDefense = (int)(fA*pA->nDefense);
	pD->nDefense = (int)(fD*pD->nDefense);
	pA->nHealth = (int)(fA*pA->nHealth);
	pD->nHealth = (int)(fD*pD->nHealth);
	// 计算武将受伤程度
*/

	int nProfA	= (int)((pA->nAttack*nMA+pA->nDefense*0.9+pA->nHealth*0.8)/3);
	if (nProfA==0 && pA->nArmyNum>0) nProfA=1;
	int nProfD	= (int)((pD->nAttack*nMD+pD->nDefense*0.9+pD->nHealth*0.8)/3);
	if (nProfD==0 && pD->nArmyNum>0) nProfD=1;
	int nProfMin	= nProfA>nProfD? nProfD: nProfA;
	float fA = (nProfA>0)? (float)nProfMin/(float)nProfA: 0;
	float fD = (nProfD>0)? (float)nProfMin/(float)nProfD: 0;
	fA = (int)(fA*1000+0.5)/1000.0;
	fD = (int)(fD*1000+0.5)/1000.0;
	fA = 1-fA;
	fD = 1-fD;
	pA->nArmyNum = (int)(fA*pA->nArmyNum);
	pD->nArmyNum = (int)(fD*pD->nArmyNum);
	pA->nAttack = (int)(fA*pA->nAttack);
	pD->nAttack = (int)(fD*pD->nAttack);
	pA->nDefense = (int)(fA*pA->nDefense);
	pD->nDefense = (int)(fD*pD->nDefense);
	pA->nHealth = (int)(fA*pA->nHealth);
	pD->nHealth = (int)(fD*pD->nHealth);

	nProfResA = (int)(fA*nProfA);
	nProfResD = (int)(fD*nProfD);
	return 0;
}
int		CGameMngS::UpdateHeroToDB(tty_id_t nHeroID, int nArmyNum)
{
	// 通知db存盘
	P_DBS4WEB_UPDATE_HERO_T Cmd;
	Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd		= CMDID_UPDATE_HERO_REQ;
	Cmd.nHeroID		= nHeroID;
	Cmd.nArmyNum	= nArmyNum;
	g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
	return 0;
}
