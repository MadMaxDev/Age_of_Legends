#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

int		LPGamePlay4Web_i::Tick_DealGS4WebMsg()
{
	m_tickNow						= wh_gettickcount();

	if (m_msgerGS4Web.GetStatus() != tcpretrymsger<tcpmsger>::STATUS_WORKING)
	{
		return 0;
	}

	// 处理从GS4Web发来的消息
	GS4WEB_CMD_BASE_T*		pBaseCmd	= NULL;	
	size_t				nSize		= 0;

	while ((pBaseCmd=(GS4WEB_CMD_BASE_T*)m_msgerGS4Web.PeekMsg(&nSize)) != NULL)
	{
		switch (pBaseCmd->nCmd)
		{
		case P_GS4WEB_DB_RPL_CMD:
			{
				_Tick_DealGS4WebMsg_DB_PRL(pBaseCmd, nSize);
			}
			break;
		case P_GS4WEB_ASSIST_RPL_CMD:
			{
				_Tick_DealGS4WebMsg_Assist_PRL(pBaseCmd, nSize);
			}
			break;
		case P_GS4WEB_GROUP_REQ_CMD:
			{
				_Tick_DealGS4WebMsg_Group_REQ(pBaseCmd, nSize);
			}
			break;
		default:
			{

			}
			break;
		}

		// 释放刚刚处理的消息
		m_msgerGS4Web.FreeMsg();
	}

	return 0;
}
int		LPGamePlay4Web_i::_Tick_DealGS4WebMsg_Assist_PRL(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize)
{
	return 0;
}
int		LPGamePlay4Web_i::_Tick_DealGS4WebMsg_DB_PRL(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize)
{
	switch (pBaseCmd->nSubCmd)
	{
	case DB_ACCOUNT_LOGIN_RPL:
		{
			GS4WEB_DB_ACCOUNT_LOGIN_RPL_T*	pCmd	= (GS4WEB_DB_ACCOUNT_LOGIN_RPL_T*)pBaseCmd;
			whtransactionbase*	pT	= m_pTMan->GetTransaction(pCmd->nExt[0]);
			if (pT != NULL)
			{
				pT->DoNext(DB_ACCOUNT_LOGIN_RPL, pCmd, nSize);
			}
		}
		break;
	case DB_ACCOUNT_LOGOUT_RPL:
		{
			GS4WEB_DB_ACCOUNT_LOGOUT_RPL_T*	pCmd	= (GS4WEB_DB_ACCOUNT_LOGOUT_RPL_T*)pBaseCmd;
			whtransactionbase*	pT	= m_pTMan->GetTransaction(pCmd->nExt[0]);
			if (pT != NULL)
			{
				pT->DoNext(DB_ACCOUNT_LOGOUT_RPL, pCmd, nSize);
			}
		}
		break;
	case DB_ACCOUNT_CREATE_RPL:
		{
			GS4WEB_DB_ACCOUNT_CREATE_RPL_T*	pCmd	= (GS4WEB_DB_ACCOUNT_CREATE_RPL_T*)pBaseCmd;
			whtransactionbase*	pT	= m_pTMan->GetTransaction(pCmd->nExt[0]);
			if (pT != NULL)
			{
				pT->DoNext(DB_ACCOUNT_CREATE_RPL, pCmd, nSize);
			}
		}
		break;
	case DB_REGISTER_RPL:
		{
			GS4WEB_DB_REGISTER_RPL_T*	pRst	= (GS4WEB_DB_REGISTER_RPL_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst==STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_OK && pRst->bIsRegister)
			{
				// 需要更新common_characters的binded=1
				P_DBS4WEB_SET_BINDED_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_SET_BINDED_REQ;
				Cmd.nAccountID			= pRst->nAccountID;
				SendCmdToDB(&Cmd, sizeof(Cmd));

				// 不需要等数据库的返回了
				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.bBinded		= true;
				}
			}
		}
		break;
	case DB_BIND_DEVICE_RPL:
		{
			GS4WEB_DB_BIND_DEVICE_RPL_T*	pRst	= (GS4WEB_DB_BIND_DEVICE_RPL_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_BIND_DEVICE_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_BIND_DEVICE;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	default:
		{

		}
		break;
	}
	return 0;
}
int		LPGamePlay4Web_i::_Tick_DealGS4WebMsg_Group_REQ(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize)
{
	switch (pBaseCmd->nSubCmd)
	{
	case GROUP_ADD_NOTIFY_REQ:
		{
			GS4WEB_GROUP_ADD_NOTIFY_REQ_T*	pReq	= (GS4WEB_GROUP_ADD_NOTIFY_REQ_T*)pBaseCmd;
			Web_Notification_T*		pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pReq);
			m_pGameMngS->GetNotificactionMng()->AddNotification(pNotification);
			
			// 这个需要向所有的客户端推送一下
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_NEW_NOTIFICATION_T) + pNotification->GetTotalSize());
			STC_GAMECMD_NEW_NOTIFICATION_T*	pNew	= (STC_GAMECMD_NEW_NOTIFICATION_T*)m_vectrawbuf.getbuf();
			pNew->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pNew->nGameCmd			= STC_GAMECMD_NEW_NOTIFICATION;
			memcpy(wh_getptrnexttoptr(pNew), pNotification, pNotification->GetTotalSize());
			SendCmdToAllClient(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case GROUP_LOAD_NOTIFY_REQ:
		{
			GS4WEB_GROUP_LOAD_NOTIFY_REQ_T*	pReq	= (GS4WEB_GROUP_LOAD_NOTIFY_REQ_T*)pBaseCmd;
			// 先清空一下
			m_pGameMngS->GetNotificactionMng()->DelNotificationAll();
			// 载入所有通知到内存
			Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pReq);
			for (int i=0; i<pReq->nNum; i++)
			{
				m_pGameMngS->GetNotificactionMng()->AddNotification(pNotification);
				pNotification					= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
			}
		}
		break;
	case GROUP_DEL_NOTIFY_BY_ID_REQ:
		{
			GS4WEB_GROUP_DEL_NOTIFY_BY_ID_REQ_T*	pReq	= (GS4WEB_GROUP_DEL_NOTIFY_BY_ID_REQ_T*)pBaseCmd;
			m_pGameMngS->GetNotificactionMng()->DelNotificationByID(pReq->nNotificationID);
		}
		break;
	case GROUP_DEL_NOTIFY_BT_REQ:
		{
			GS4WEB_GROUP_DEL_NOTIFY_BT_REQ_T*	pReq	= (GS4WEB_GROUP_DEL_NOTIFY_BT_REQ_T*)pBaseCmd;
			m_pGameMngS->GetNotificactionMng()->DelNotificationBeforeTime(pReq->nTime);
		}
		break;
	case GROUP_DEL_NOTIFY_AT_REQ:
		{
			GS4WEB_GROUP_DEL_NOTIFY_AT_REQ_T*	pReq	= (GS4WEB_GROUP_DEL_NOTIFY_AT_REQ_T*)pBaseCmd;
			m_pGameMngS->GetNotificactionMng()->DelNotificationAfterTime(pReq->nTime);
		}
		break;
	case GROUP_DEL_NOTIFY_ALL_REQ:
		{
			m_pGameMngS->GetNotificactionMng()->DelNotificationAll();
		}
		break;
	case GROUP_DEL_NOTIFY_BY_GROUP_REQ:
		{
			GS4WEB_GROUP_DEL_NOTIFY_BY_GROUP_REQ_T*	pReq	= (GS4WEB_GROUP_DEL_NOTIFY_BY_GROUP_REQ_T*)pBaseCmd;
			m_pGameMngS->GetNotificactionMng()->DelNotificationByGroup(pReq->nGroupID);
		}
		break;
	case GROUP_GET_GROUP_INFO_REQ:
		{
			GS4WEB_GROUP_GET_GROUP_INFO_RPL_T	RplCmd;
			RplCmd.nCmd				= P_GS4WEB_GROUP_RPL_CMD;
			RplCmd.nSubCmd			= GROUP_GET_GROUP_INFO_RPL;
			RplCmd.nMaxPlayerNum	= m_cfginfo.nMaxPlayer;
			RplCmd.nPlayerTotalNum	= 0;
			m_msgerGS4Web.SendMsg(&RplCmd, sizeof(RplCmd));
		}
		break;
	case GROUP_RECHARGE_DIAMOND_REQ:
		{
			GS4WEB_GROUP_RECHARGE_DIAMOND_REQ_T*	pReq	= (GS4WEB_GROUP_RECHARGE_DIAMOND_REQ_T*)pBaseCmd;
			RechargeDiamond(pReq->nAccountID, pReq->nAddedDiamond, true, pReq->nExt);
		}
		break;
	case GROUP_RELOAD_EXCEL_TABLE_REQ:
		{
			GS4WEB_GROUP_RELOAD_EXCEL_TABLE_REQ_T*	pReq	= (GS4WEB_GROUP_RELOAD_EXCEL_TABLE_REQ_T*)pBaseCmd;

			m_pGameMngS->ReloadExcelTable();
			SafeInitBadNameFilter();

			GS4WEB_GROUP_RELOAD_EXCEL_TABLE_RPL_T	RplCmd;
			RplCmd.nCmd				= P_GS4WEB_GROUP_RPL_CMD;
			RplCmd.nSubCmd			= GROUP_RELOAD_EXCEL_TABLE_RPL;
			RplCmd.nGroupID			= m_cfginfo.nSvrGrpID;
			RplCmd.nRst				= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			m_msgerGS4Web.SendMsg(&RplCmd, sizeof(RplCmd));
		}
		break;
	case GROUP_KICK_CHAR_REQ:
		{
			GS4WEB_GROUP_KICK_CHAR_REQ_T*	pReq	= (GS4WEB_GROUP_KICK_CHAR_REQ_T*)pBaseCmd;
			PlayerUnit*	pPlayer		= GetPlayerByAccountID(pReq->nAccountID);
			if (pPlayer != NULL)
			{
				KickPlayerOffline(pPlayer->nID, SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID);
			}

			GS4WEB_GROUP_KICK_CHAR_RPL_T	RplCmd;
			RplCmd.nCmd				= P_GS4WEB_GROUP_RPL_CMD;
			RplCmd.nSubCmd			= GROUP_KICK_CHAR_RPL;
			RplCmd.nGroupID			= m_cfginfo.nSvrGrpID;
			RplCmd.nAccountID		= pReq->nAccountID;
			RplCmd.nRst				= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			m_msgerGS4Web.SendMsg(&RplCmd, sizeof(RplCmd));
		}
		break;
	case GROUP_BAN_CHAR_REQ:
		{
			GS4WEB_GROUP_BAN_CHAR_REQ_T*	pReq	= (GS4WEB_GROUP_BAN_CHAR_REQ_T*)pBaseCmd;
			P_DBS4WEB_BAN_CHAR_T	Cmd;
			Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd				= CMDID_BAN_CHAR_REQ;
			Cmd.nAccountID			= pReq->nAccountID;
			memcpy(Cmd.nExt, pReq->nExt, sizeof(pReq->nExt));
			SendCmdToDB(&Cmd, sizeof(Cmd));
		}
		break;
	case GROUP_KICK_CLIENT_ALL_REQ:
		{
			GS4WEB_GROUP_KICK_CLIENT_ALL_REQ_T*	pReq	= (GS4WEB_GROUP_KICK_CLIENT_ALL_REQ_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(P_DBS4WEBUSER_KICK_CLIENT_ALL_T) + pReq->nLength);
			P_DBS4WEB_KICK_CLIENT_ALL_T*	pCmd	= (P_DBS4WEB_KICK_CLIENT_ALL_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd				= P_DBS4WEB_REQ_CMD;
			pCmd->nSubCmd			= CMDID_KICK_CLIENT_ALL_REQ;
			pCmd->nLength			= pReq->nLength;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pReq), pReq->nLength);
			memcpy(pCmd->nExt, pReq->nExt, sizeof(pCmd->nExt));
			SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			// 通知所有用户
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_KICK_CLIENT_ALL_T) + pReq->nLength);
			STC_GAMECMD_GET_KICK_CLIENT_ALL_T*	pNotify	= (STC_GAMECMD_GET_KICK_CLIENT_ALL_T*)m_vectrawbuf.getbuf();
			pNotify->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pNotify->nGameCmd		= STC_GAMECMD_GET_KICK_CLIENT_ALL;
			pNotify->nRst			= STC_GAMECMD_GET_KICK_CLIENT_ALL_T::RST_OK;
			pNotify->nLength		= pReq->nLength;
			memcpy(wh_getptrnexttoptr(pNotify), wh_getptrnexttoptr(pReq), pReq->nLength);
			SendCmdToAllClient(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case GROUP_NEW_ADCOLONY_REQ:
		{
			GS4WEB_GROUP_NEW_ADCOLONY_REQ_T*	pReq		= (GS4WEB_GROUP_NEW_ADCOLONY_REQ_T*)pBaseCmd;
			
			P_DBS4WEB_NEW_ADCOLONY_T	Cmd;
			Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd				= CMDID_NEW_ADCOLONY_REQ;
			Cmd.nAccountID			= pReq->nAccountID;
			Cmd.nAmount				= pReq->nAmount;
			Cmd.nTime				= pReq->nTime;
			Cmd.nTransactionID		= pReq->nTransactionID;
			WH_STRNCPY0(Cmd.szName, pReq->szName);
			SendCmdToDB(&Cmd, sizeof(Cmd));
		}
		break;
	}
	return 0;
}
