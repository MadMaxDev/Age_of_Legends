#include "../inc/GS4Web_i.h"

using namespace n_pngs;

int		GS4Web_i::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	return 0;
}
int		GS4Web_i::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch (nCmd)
	{
	case PNGSPACKET_2LOGIC_SETMOOD:
		{
			m_nMood = CMN::ILogic::MOOD_STOPPED;
		}
		break;
	case PNGS_TR2CD_NOTIFY:
		{
			PNGS_TR2CD_NOTIFY_T*	pCmd	= (PNGS_TR2CD_NOTIFY_T*)pData;
			switch (pCmd->nSubCmd)
			{
			case PNGS_TR2CD_NOTIFY_T::SUBCMD_DISCONNECT:
				{
					m_vectConnecters.delvalue(pCmd->nParam1);
					{
						// 服务器掉线就不输出在线信息了
						int	nGroupID	= GetGroupIDByCntrID(pCmd->nParam1);
						if (nGroupID>0 && nGroupID<TTY_MAX_GG_NUM)
						{
							m_aGGs[nGroupID].Clear();
						}
					}
					RemoveCntrIDnGroupIDMapByCntrID(pCmd->nParam1);		// 删除映射
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GS4Web_i_RT)"%s,disconnect,cntr id:0x%X", __FUNCTION__, pCmd->nParam1);
				}
				break;
			case PNGS_TR2CD_NOTIFY_T::SUBCMD_CONNECT:
				{
					m_vectConnecters.push_back(pCmd->nParam1);
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GS4Web_i_RT)"%s,connect,cntr id:0x%X", __FUNCTION__, pCmd->nParam1);
				}
				break;
			default:
				{

				}
				break;
			}
		}
		break;
	case PNGS_TR2CD_CMD:
		{
			return DealCmdIn_One_PNGS_TR2CD_CMD(pData, nDSize);
		}
		break;
	case PNGSPACKET_BD4WEB_CMD_REQ:
		{
			return DealCmdFromBD4Web((const PNGSPACKET_BD4WEB_CMD_REQ_T*)pData, nDSize);
		}
		break;
	case PNGSPACKET_BD4WEB_CMD_SET_PTR:
		{
			m_pLogicBD4Web		= (ILogicBase*)pData;
		}
		break;
	default:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_RT)"%s,%d,unknown,%d", __FUNCTION__, nCmd, nDSize);
		}
		break;
	}
	return 0;
}
int		GS4Web_i::DealCmdIn_One_PNGS_TR2CD_CMD(const void* pData, int nDSize)
{
	PNGS_TR2CD_CMD_T*	pCDCmd	= (PNGS_TR2CD_CMD_T*)pData;
	GS4WEB_CMD_BASE_T*		pCmd	= (GS4WEB_CMD_BASE_T*)pCDCmd->pData;
	switch (pCmd->nCmd)
	{
	case P_GS4WEB_DB_REQ_CMD:
		{
			m_msgerDB.SendMsg((const void*)pCDCmd->pData, pCDCmd->nDSize);
		}
		break;
	case P_GS4WEB_FIRSTIN:
		{
			GS4WEB_FIRSTIN_T*	pFirstIn	= (GS4WEB_FIRSTIN_T*)pCmd;
			int	nCntrID			= pCDCmd->nConnecterID;
			int	nGroupID		= pFirstIn->nGroupID;
			
			MakeCntrIDnGroupIDMap(nCntrID, nGroupID);

			// 把通知发给服务器
			if (!m_bLoadNotificationOK)
			{
				if (m_teidLoadNotification.IsValid())
				{
					map<int, int>::iterator	it	= m_map4NeedToSendLoadingNotify.find(nGroupID);
					if (it != m_map4NeedToSendLoadingNotify.end())
					{
						it->second	= nCntrID;
					}
					else
					{
						m_map4NeedToSendLoadingNotify.insert(map<int, int>::value_type(nGroupID, nCntrID));
					}
				}
				else
				{
					// 请求载入通知
					ReqLoadNotification();
					// 生成载入通知时间事件
					SetTE_LoadingNotification();
					// 插入请求队列
					m_map4NeedToSendLoadingNotify.insert(map<int, int>::value_type(nGroupID, nCntrID));
				}
			}
			else
			{
				SendNotificationToGroup(nGroupID);
			}
		}
		break;
	case P_GS4WEB_GROUP_RPL_CMD:
		{
			switch (pCmd->nSubCmd)
			{
			case GROUP_GET_GROUP_INFO_RPL:
				{
					GS4WEB_GROUP_GET_GROUP_INFO_RPL_T*	pRpl	= (GS4WEB_GROUP_GET_GROUP_INFO_RPL_T*)pCmd;
					int	nGroupID			= GetGroupIDByCntrID(pCDCmd->nConnecterID);
					if (nGroupID>0 && nGroupID<TTY_MAX_GG_NUM)
					{
						GGUnit_T& gg		= m_aGGs[nGroupID];
						gg.nCntrID			= pCDCmd->nConnecterID;
						gg.nPlayerTotalNum	= pRpl->nPlayerTotalNum;
						gg.nMaxPlayerNum	= pRpl->nMaxPlayerNum;
						gg.vectTermUnits.clear();
						gg.vectTermUnits.reserve(TERM_TYPE_NUM);
						for (int i=0; i<TERM_TYPE_NUM; i++)
						{
							GGUnit_T::TERM_UNIT_T	unit;
							unit.nPlayerNum	= pRpl->aPlayerNum[i];
							unit.nTermType	= i;
							gg.vectTermUnits.push_back(unit);
						}
					}
				}
				break;
			case GROUP_RECHARGE_DIAMOND_RPL:
				{
					// 发回给BD系统
					GS4WEB_GROUP_RECHARGE_DIAMOND_RPL_T*	pRplCmd	= (GS4WEB_GROUP_RECHARGE_DIAMOND_RPL_T*)pCmd;
					BD_CMD_RECHARGE_DIAMOND_RPL_T	Cmd;
					Cmd.nCmd		= BD_CMD_RECHARGE_DIAMOND_RPL;
					Cmd.nSize		= sizeof(Cmd);
					Cmd.nAccountID	= pRplCmd->nAccountID;
					Cmd.nAddedDiamond	= pRplCmd->nAddedDiamond;
					Cmd.nGroupID	= GetGroupIDByCntrID(pCDCmd->nConnecterID);
					Cmd.nVip		= pRplCmd->nVip;
					Cmd.nRst		= pRplCmd->nRst;
					_SendBDRplCmd(pRplCmd->nExt, (char*)&Cmd, sizeof(Cmd));
				}
				break;
			case GROUP_RELOAD_EXCEL_TABLE_RPL:
				{
					GS4WEB_GROUP_RELOAD_EXCEL_TABLE_RPL_T*	pRplCmd	= (GS4WEB_GROUP_RELOAD_EXCEL_TABLE_RPL_T*)pCmd;
					BD_CMD_RELOAD_EXCEL_TABLE_RPL_T	Cmd;
					Cmd.nCmd		= BD_CMD_RELOAD_EXCEL_TABLE_RPL;
					Cmd.nSize		= sizeof(Cmd);
					Cmd.nGroupID	= pRplCmd->nGroupID;
					Cmd.nRst		= pRplCmd->nRst;
					_SendBDRplCmd(pRplCmd->nExt, (char*)&Cmd, sizeof(Cmd));
				}
				break;
			case GROUP_KICK_CHAR_RPL:
				{
					GS4WEB_GROUP_KICK_CHAR_RPL_T*	pRplCmd	= (GS4WEB_GROUP_KICK_CHAR_RPL_T*)pCmd;
					BD_CMD_KICK_CHAR_RPL_T	Cmd;
					Cmd.nCmd		= BD_CMD_KICK_CHAR_RPL;
					Cmd.nSize		= sizeof(Cmd);
					Cmd.nGroupID	= pRplCmd->nGroupID;
					Cmd.nAccountID	= pRplCmd->nAccountID;
					Cmd.nRst		= pRplCmd->nRst;

					_SendBDRplCmd(pRplCmd->nExt, (char*)&Cmd, sizeof(Cmd));
				}
				break;
			case GROUP_BAN_CHAR_RPL:
				{
					GS4WEB_GROUP_BAN_CHAR_RPL_T*	pRplCmd	= (GS4WEB_GROUP_BAN_CHAR_RPL_T*)pCmd;
					BD_CMD_KICK_CHAR_RPL_T	Cmd;
					Cmd.nCmd		= BD_CMD_BAN_CHAR_RPL;
					Cmd.nSize		= sizeof(Cmd);
					Cmd.nGroupID	= pRplCmd->nGroupID;
					Cmd.nAccountID	= pRplCmd->nAccountID;
					Cmd.nRst		= pRplCmd->nRst;

					_SendBDRplCmd(pRplCmd->nExt, (char*)&Cmd, sizeof(Cmd));
				}
				break;
			case GROUP_NEW_ADCOLONY_RPL:
				{
					GS4WEB_GROUP_NEW_ADCOLONY_RPL_T*	pRplCmd	= (GS4WEB_GROUP_NEW_ADCOLONY_RPL_T*)pCmd;

					GS4WEB_DB_DEAL_ADCOLONY_REQ_T	Cmd;
					Cmd.nCmd		= P_GS4WEB_DB_REQ_CMD;
					Cmd.nSubCmd		= DB_DEAL_ADCOLONY_REQ;
					Cmd.nTransactionID	= pRplCmd->nTransactionID;
					m_msgerDB.SendMsg(&Cmd, sizeof(Cmd));
				}
				break;
			case GROUP_KICK_CLIENT_ALL_RPL:
				{
					GS4WEB_GROUP_KICK_CLIENT_ALL_RPL_T*	pRplCmd	= (GS4WEB_GROUP_KICK_CLIENT_ALL_RPL_T*)pCmd;
					BD_CMD_KICK_CLIENT_ALL_RPL_T	Cmd;
					Cmd.nCmd		= BD_CMD_KICK_CLIENT_ALL_RPL;
					Cmd.nSize		= sizeof(Cmd);
					Cmd.nGroupID	= pRplCmd->nGroupID;
					Cmd.nRst		= pRplCmd->nRst;

					_SendBDRplCmd(pRplCmd->nExt, (char*)&Cmd, sizeof(Cmd));
				}
				break;
			}
		}
		break;
	default:
		{

		}
		break;
	}
	return 0;
}
