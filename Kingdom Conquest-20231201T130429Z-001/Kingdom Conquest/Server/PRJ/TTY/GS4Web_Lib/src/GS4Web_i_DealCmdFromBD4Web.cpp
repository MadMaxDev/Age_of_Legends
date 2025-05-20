#include "../inc/GS4Web_i.h"

using namespace n_pngs;

void	GS4Web_i::_SendBDRplCmd(const int* pExt, char* pData, size_t nDSize)
{
	static PNGSPACKET_BD4WEB_CMD_RPL_T RplCmd;
	RplCmd.nDSize		= nDSize;
	RplCmd.pData		= (BD_CMD_BASE_T*)pData;
	memcpy(RplCmd.nExt, pExt, sizeof(RplCmd.nExt));

	CmdOutToLogic_AUTO(m_pLogicBD4Web, PNGSPACKET_BD4WEB_CMD_RPL, &RplCmd, sizeof(RplCmd));
}

int		GS4Web_i::DealCmdFromBD4Web(const PNGSPACKET_BD4WEB_CMD_REQ_T* pCmd, size_t nSize)
{
	const BD_CMD_BASE_T*	pBaseCmd	= pCmd->pData;
	if (pBaseCmd->nSize != pCmd->nDSize)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_RT)"%s,ERROR cmd size,%d,%d", __FUNCTION__, pBaseCmd->nSize, pCmd->nDSize);
		return -1;
	}
	switch (pBaseCmd->nCmd)
	{
	case BD_CMD_ADD_NOTIFY:
		{
			BD_CMD_ADD_NOTIFY_T*	pReq	= (BD_CMD_ADD_NOTIFY_T*)pBaseCmd;
			int*	pContentLen				= (int*)wh_getoffsetaddr(pReq, sizeof(BD_CMD_ADD_NOTIFY_T) + pReq->nTitleLen);
			m_vectrawbuf.resize(sizeof(GS4WEB_DB_ADD_NOTIFY_REQ_T) + pReq->nTitleLen + sizeof(int) + *pContentLen);
			GS4WEB_DB_ADD_NOTIFY_REQ_T*	pCmdReq	= (GS4WEB_DB_ADD_NOTIFY_REQ_T*)m_vectrawbuf.getbuf();
			pCmdReq->nCmd				= P_GS4WEB_DB_REQ_CMD;
			pCmdReq->nSubCmd			= DB_ADD_NOTIFY_REQ;
			memcpy(pCmdReq->nExt, pCmd->nExt, sizeof(pCmd->nExt));
			pCmdReq->nGroupID				= pReq->nGroupID;
			if (pReq->szTimeStr[0] == 0)
			{
				pCmdReq->nTime			= wh_time();
			}
			else
			{
				pCmdReq->nTime			= wh_gettimefromstr(pReq->szTimeStr);
			}
			pCmdReq->nTitleLen			= pReq->nTitleLen;
			char*	pDstTitle			= (char*)wh_getptrnexttoptr(pCmdReq);
			char*	pSrcTitle			= (char*)wh_getptrnexttoptr(pReq);
			memcpy(pDstTitle, pSrcTitle, pCmdReq->nTitleLen);
			int*	pDstContentLen		= (int*)wh_getoffsetaddr(pDstTitle, pCmdReq->nTitleLen);
			*pDstContentLen				= *pContentLen;
			memcpy(wh_getptrnexttoptr(pDstContentLen), wh_getptrnexttoptr(pContentLen), *pDstContentLen);

			m_msgerDB.SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case BD_CMD_DEL_NOTIFY_BY_ID:
		{
			BD_CMD_DEL_NOTIFY_BY_ID_T*	pReq	= (BD_CMD_DEL_NOTIFY_BY_ID_T*)pBaseCmd;
			GS4WEB_DB_DEL_NOTIFY_BY_ID_REQ_T	ReqCmd;
			ReqCmd.nCmd					= P_GS4WEB_DB_REQ_CMD;
			ReqCmd.nSubCmd				= DB_DEL_NOTIFY_BY_ID_REQ;
			ReqCmd.nNotificationID		= pReq->nNotificationID;
			memcpy(ReqCmd.nExt, pCmd->nExt, sizeof(ReqCmd.nExt));

			m_msgerDB.SendMsg(&ReqCmd, sizeof(ReqCmd));
		}
		break;
	case BD_CMD_DEL_NOTIFY_BEFORE_TIME:
		{
			BD_CMD_DEL_NOTIFY_BEFORE_TIME_T*	pReq	= (BD_CMD_DEL_NOTIFY_BEFORE_TIME_T*)pBaseCmd;
			GS4WEB_DB_DEL_NOTIFY_BT_REQ_T	ReqCmd;
			ReqCmd.nCmd					= P_GS4WEB_DB_REQ_CMD;
			ReqCmd.nSubCmd				= DB_DEL_NOTIFY_BT_REQ;
			ReqCmd.nTime				= wh_gettimefromstr(pReq->szTimeStr);
			memcpy(ReqCmd.nExt, pCmd->nExt, sizeof(ReqCmd.nExt));

			m_msgerDB.SendMsg(&ReqCmd, sizeof(ReqCmd));
		}
		break;
	case BD_CMD_DEL_NOTIFY_AFTER_TIME:
		{
			BD_CMD_DEL_NOTIFY_AFTER_TIME_T*	pReq		= (BD_CMD_DEL_NOTIFY_AFTER_TIME_T*)pBaseCmd;
			GS4WEB_DB_DEL_NOTIFY_AT_REQ_T	ReqCmd;
			ReqCmd.nCmd					= P_GS4WEB_DB_REQ_CMD;
			ReqCmd.nSubCmd				= DB_DEL_NOTIFY_AT_REQ;
			ReqCmd.nTime				= wh_gettimefromstr(pReq->szTimeStr);
			memcpy(ReqCmd.nExt, pCmd->nExt, sizeof(ReqCmd.nExt));

			m_msgerDB.SendMsg(&ReqCmd, sizeof(ReqCmd));
		}
		break;
	case BD_CMD_DEL_NOTIFY_ALL:
		{
			GS4WEB_DB_DEL_NOTIFY_ALL_REQ_T	ReqCmd;
			ReqCmd.nCmd					= P_GS4WEB_DB_REQ_CMD;
			ReqCmd.nSubCmd				= DB_DEL_NOTIFY_ALL_REQ;
			memcpy(ReqCmd.nExt, pCmd->nExt, sizeof(ReqCmd.nExt));

			m_msgerDB.SendMsg(&ReqCmd, sizeof(ReqCmd));
		}
		break;
	case BD_CMD_DEL_NOTIFY_BY_GROUP:
		{
			BD_CMD_DEL_NOTIFY_BY_GROUP_T*	pReq	= (BD_CMD_DEL_NOTIFY_BY_GROUP_T*)pBaseCmd;

			GS4WEB_DB_DEL_NOTIFY_BY_GROUP_REQ_T	ReqCmd;
			ReqCmd.nCmd					= P_GS4WEB_DB_REQ_CMD;
			ReqCmd.nSubCmd				= DB_DEL_NOTIFY_BY_GROUP_REQ;
			ReqCmd.nGroupID				= pReq->nGroupID;
			memcpy(ReqCmd.nExt, pCmd->nExt, sizeof(ReqCmd.nExt));

			m_msgerDB.SendMsg(&ReqCmd, sizeof(ReqCmd));
		}
		break;
	case BD_CMD_GET_NOTIFY_BY_ID:
		{
			BD_CMD_GET_NOTIFY_BY_ID_T*	pReq	= (BD_CMD_GET_NOTIFY_BY_ID_T*)pBaseCmd;

			m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_BY_ID_RPL_T));
			BD_CMD_GET_NOTIFY_BY_ID_RPL_T*	pRplCmd	= (BD_CMD_GET_NOTIFY_BY_ID_RPL_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd				= BD_CMD_GET_NOTIFY_BY_ID_RPL;
			pRplCmd->nSize				= sizeof(BD_CMD_GET_NOTIFY_BY_ID_RPL_T);
			pRplCmd->nRst				= BD_RST_OK;

			if (m_bLoadNotificationOK)
			{
				Web_Notification_T*	pNotification	= m_pNotificationMngS->GetNotificationByID(pReq->nNotificationID);
				if (pNotification == NULL)
				{
					pRplCmd->nRst		= BD_RST_NOTIFY_NOT_EXIST;
				}
				else
				{
					m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_BY_ID_RPL_T) + pNotification->GetTotalSize());
					// 原来的地址可能不对了,需要校准,大小也需要重新校准
					pRplCmd				= (BD_CMD_GET_NOTIFY_BY_ID_RPL_T*)m_vectrawbuf.getbuf();
					pRplCmd->nSize		= m_vectrawbuf.size();
					memcpy(wh_getptrnexttoptr(pRplCmd), pNotification, pNotification->GetTotalSize());
				}
			}
			else
			{
				ReqLoadNotification();
				pRplCmd->nRst			= BD_RST_NOTIFY_LOADING;
			}

			_SendBDRplCmd(pCmd->nExt, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case BD_CMD_GET_NOTIFY_BEFORE_TIME:
		{
			time_t	nTime	= 0;
			BD_CMD_GET_NOTIFY_BEFORE_TIME_T*	pReq		= (BD_CMD_GET_NOTIFY_BEFORE_TIME_T*)pBaseCmd;
			nTime			= wh_gettimefromstr(pReq->szTimeStr);
			
			m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL_T));
			BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL_T*	pRplCmd	= (BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd	= BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL;
			pRplCmd->nSize	= m_vectrawbuf.size();
			pRplCmd->nRst	= BD_RST_OK;
			pRplCmd->nNum	= 0;

			if (m_bLoadNotificationOK)
			{
				whvector<Web_Notification_T*>	vect;
				m_pNotificationMngS->GetNotificationBeforeTime(nTime, vect);
				int	nNotifyTotalSize	= 0;
				for (int i=0; i<vect.size(); i++)
				{
					nNotifyTotalSize	+= vect[i]->GetTotalSize();
				}
				m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL_T) + nNotifyTotalSize);
				// 原来的地址可能不对了,需要校准
				pRplCmd			= (BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL_T*)m_vectrawbuf.getbuf();
				pRplCmd->nSize	= m_vectrawbuf.size();
				pRplCmd->nNum	= vect.size();
				Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRplCmd);
				for (int i=0; i<vect.size(); i++)
				{
					memcpy(pNotification, vect[i], vect[i]->GetTotalSize());

					pNotification	= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
				}
			}
			else
			{
				ReqLoadNotification();
				pRplCmd->nRst	= BD_RST_NOTIFY_LOADING;
			}
			_SendBDRplCmd(pCmd->nExt, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case BD_CMD_GET_NOTIFY_AFTER_TIME:
		{
			time_t	nTime	= 0;
			BD_CMD_GET_NOTIFY_BEFORE_TIME_T*	pReq		= (BD_CMD_GET_NOTIFY_BEFORE_TIME_T*)pBaseCmd;
			nTime			= wh_gettimefromstr(pReq->szTimeStr);

			m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_AFTER_TIME_RPL_T));
			BD_CMD_GET_NOTIFY_AFTER_TIME_RPL_T*	pRplCmd	= (BD_CMD_GET_NOTIFY_AFTER_TIME_RPL_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd	= BD_CMD_GET_NOTIFY_AFTER_TIME_RPL;
			pRplCmd->nSize	= m_vectrawbuf.size();
			pRplCmd->nRst	= BD_RST_OK;
			pRplCmd->nNum	= 0;

			if (m_bLoadNotificationOK)
			{
				whvector<Web_Notification_T*>	vect;
				m_pNotificationMngS->GetNotificationAfterTime(nTime, vect);
				int	nNotifyTotalSize	= 0;
				for (int i=0; i<vect.size(); i++)
				{
					nNotifyTotalSize	+= vect[i]->GetTotalSize();
				}
				m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_AFTER_TIME_RPL_T) + nNotifyTotalSize);
				// 原来的地址可能不对了,需要校准
				pRplCmd			= (BD_CMD_GET_NOTIFY_AFTER_TIME_RPL_T*)m_vectrawbuf.getbuf();
				pRplCmd->nSize	= m_vectrawbuf.size();
				pRplCmd->nNum	= vect.size();
				Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRplCmd);
				for (int i=0; i<vect.size(); i++)
				{
					memcpy(pNotification, vect[i], vect[i]->GetTotalSize());

					pNotification	= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
				}
			}
			else
			{
				ReqLoadNotification();
				pRplCmd->nRst	= BD_RST_NOTIFY_LOADING;
			}
			_SendBDRplCmd(pCmd->nExt, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case BD_CMD_GET_NOTIFY_ALL:
		{
			m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_ALL_RPL_T));
			BD_CMD_GET_NOTIFY_ALL_RPL_T*	pRplCmd	= (BD_CMD_GET_NOTIFY_ALL_RPL_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd	= BD_CMD_GET_NOTIFY_ALL_RPL;
			pRplCmd->nSize	= m_vectrawbuf.size();
			pRplCmd->nRst	= BD_RST_OK;
			pRplCmd->nNum	= 0;

			if (m_bLoadNotificationOK)
			{
				whvector<Web_Notification_T*>	vect;
				m_pNotificationMngS->GetNotificationAll(vect);
				int	nNotifyTotalSize	= 0;
				for (int i=0; i<vect.size(); i++)
				{
					nNotifyTotalSize	+= vect[i]->GetTotalSize();
				}
				m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_ALL_RPL_T) + nNotifyTotalSize);
				// 原来的地址可能不对了,需要校准
				pRplCmd			= (BD_CMD_GET_NOTIFY_ALL_RPL_T*)m_vectrawbuf.getbuf();
				pRplCmd->nSize	= m_vectrawbuf.size();
				pRplCmd->nNum	= vect.size();
				Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRplCmd);
				for (int i=0; i<vect.size(); i++)
				{
					memcpy(pNotification, vect[i], vect[i]->GetTotalSize());

					pNotification	= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
				}
			}
			else
			{
				ReqLoadNotification();
				pRplCmd->nRst	= BD_RST_NOTIFY_LOADING;
			}
			_SendBDRplCmd(pCmd->nExt, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case BD_CMD_GET_NOTIFY_BY_GROUP:
		{
			BD_CMD_GET_NOTIFY_BY_GROUP_T*	pReq		= (BD_CMD_GET_NOTIFY_BY_GROUP_T*)pBaseCmd;

			m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_BY_GROUP_RPL_T));
			BD_CMD_GET_NOTIFY_BY_GROUP_RPL_T*	pRplCmd	= (BD_CMD_GET_NOTIFY_BY_GROUP_RPL_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd	= BD_CMD_GET_NOTIFY_BY_GROUP_RPL;
			pRplCmd->nSize	= m_vectrawbuf.size();
			pRplCmd->nRst	= BD_RST_OK;
			pRplCmd->nNum	= 0;

			if (m_bLoadNotificationOK)
			{
				whvector<Web_Notification_T*>	vect;
				m_pNotificationMngS->GetNotificationByGroup(pReq->nGroupID, vect);
				int	nNotifyTotalSize	= 0;
				for (int i=0; i<vect.size(); i++)
				{
					nNotifyTotalSize	+= vect[i]->GetTotalSize();
				}
				m_vectrawbuf.resize(sizeof(BD_CMD_GET_NOTIFY_BY_GROUP_RPL_T) + nNotifyTotalSize);
				// 原来的地址可能不对了,需要校准
				pRplCmd			= (BD_CMD_GET_NOTIFY_BY_GROUP_RPL_T*)m_vectrawbuf.getbuf();
				pRplCmd->nSize	= m_vectrawbuf.size();
				pRplCmd->nNum	= vect.size();
				Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRplCmd);
				for (int i=0; i<vect.size(); i++)
				{
					memcpy(pNotification, vect[i], vect[i]->GetTotalSize());

					pNotification	= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
				}
			}
			else
			{
				ReqLoadNotification();
				pRplCmd->nRst	= BD_RST_NOTIFY_LOADING;
			}
			_SendBDRplCmd(pCmd->nExt, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case BD_CMD_RECHARGE_DIAMOND:
		{
			BD_CMD_RECHARGE_DIAMOND_T*	pReq	= (BD_CMD_RECHARGE_DIAMOND_T*)pBaseCmd;

			GS4WEB_GROUP_RECHARGE_DIAMOND_REQ_T	Cmd;
			Cmd.nCmd			= P_GS4WEB_GROUP_REQ_CMD;
			Cmd.nSubCmd			= GROUP_RECHARGE_DIAMOND_REQ;
			Cmd.nAccountID		= pReq->nAccountID;
			Cmd.nAddedDiamond	= pReq->nAddedDiamond;
			memcpy(Cmd.nExt, pCmd->nExt, sizeof(Cmd.nExt));
			G_SendMsgToGroup(pReq->nGroupID, &Cmd, sizeof(Cmd), pCmd->nExt);
		}
		break;
	case BD_CMD_RELOAD_EXCEL_TABLE:
		{
			BD_CMD_RELOAD_EXCEL_TABLE_T*	pReq	= (BD_CMD_RELOAD_EXCEL_TABLE_T*)pBaseCmd;

			GS4WEB_GROUP_RELOAD_EXCEL_TABLE_REQ_T	Cmd;
			Cmd.nCmd			= P_GS4WEB_GROUP_REQ_CMD;
			Cmd.nSubCmd			= GROUP_RELOAD_EXCEL_TABLE_REQ;
			memcpy(Cmd.nExt, pCmd->nExt, sizeof(Cmd.nExt));
			G_SendMsgToGroup(pReq->nGroupID, &Cmd, sizeof(Cmd), pCmd->nExt);
		}
		break;
	case BD_CMD_KICK_CHAR:
		{
			BD_CMD_KICK_CHAR_T*	pReq	= (BD_CMD_KICK_CHAR_T*)pBaseCmd;

			GS4WEB_GROUP_KICK_CHAR_REQ_T	Cmd;
			Cmd.nCmd			= P_GS4WEB_GROUP_REQ_CMD;
			Cmd.nSubCmd			= GROUP_KICK_CHAR_REQ;
			Cmd.nAccountID		= pReq->nAccountID;
			memcpy(Cmd.nExt, pCmd->nExt, sizeof(Cmd.nExt));
			G_SendMsgToGroup(pReq->nGroupID, &Cmd, sizeof(Cmd), pCmd->nExt);
		}
		break;
	case BD_CMD_BAN_CHAR:
		{
			BD_CMD_BAN_CHAR_T*	pReq	= (BD_CMD_BAN_CHAR_T*)pBaseCmd;

			GS4WEB_GROUP_BAN_CHAR_REQ_T	Cmd;
			Cmd.nCmd			= P_GS4WEB_GROUP_REQ_CMD;
			Cmd.nSubCmd			= GROUP_BAN_CHAR_REQ;
			Cmd.nAccountID		= pReq->nAccountID;
			memcpy(Cmd.nExt, pCmd->nExt, sizeof(Cmd.nExt));
			G_SendMsgToGroup(pReq->nGroupID, &Cmd, sizeof(Cmd), pCmd->nExt);
		}
		break;
	case BD_CMD_BAN_ACCOUNT:
		{
			BD_CMD_BAN_ACCOUNT_T*	pReq	= (BD_CMD_BAN_ACCOUNT_T*)pBaseCmd;
			
			GS4WEB_DB_BAN_ACCOUNT_REQ_T	ReqCmd;
			ReqCmd.nCmd					= P_GS4WEB_DB_REQ_CMD;
			ReqCmd.nSubCmd				= DB_BAN_ACCOUNT_REQ;
			ReqCmd.nAccountID			= pReq->nAccountID;
			ReqCmd.bBanDevice			= pReq->bBanDevice;
			memcpy(ReqCmd.nExt, pCmd->nExt, sizeof(ReqCmd.nExt));

			m_msgerDB.SendMsg(&ReqCmd, sizeof(ReqCmd));
		}
		break;
	case BD_CMD_KICK_CLIENT_ALL:
		{
			BD_CMD_KICK_CLIENT_ALL_T*	pReq	= (BD_CMD_KICK_CLIENT_ALL_T*)pBaseCmd;

			m_vectrawbuf.resize(sizeof(GS4WEB_GROUP_KICK_CLIENT_ALL_REQ_T) + pReq->nLength);
			GS4WEB_GROUP_KICK_CLIENT_ALL_REQ_T*	pGroupCmd	= (GS4WEB_GROUP_KICK_CLIENT_ALL_REQ_T*)m_vectrawbuf.getbuf();
			pGroupCmd->nCmd			= P_GS4WEB_GROUP_REQ_CMD;
			pGroupCmd->nSubCmd		= GROUP_KICK_CLIENT_ALL_REQ;
			pGroupCmd->nLength		= pReq->nLength;
			memcpy(wh_getptrnexttoptr(pGroupCmd), wh_getptrnexttoptr(pReq), pReq->nLength);
			memcpy(pGroupCmd->nExt, pCmd->nExt, sizeof(pCmd->nExt));
			G_SendMsgToGroup(pReq->nGroupID, m_vectrawbuf.getbuf(), m_vectrawbuf.size(), pCmd->nExt);
		}
		break;
	}
	return 0;
}
