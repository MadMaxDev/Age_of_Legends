#include "../inc/GS4Web_i.h"

using namespace n_pngs;

int		GS4Web_i::Tick_DealGDB4WebMsg()
{
	GS4WEB_CMD_BASE_T*	pCmd	= NULL;
	size_t				nSize	= 0;
	while ((pCmd=(GS4WEB_CMD_BASE_T*)m_msgerDB.PeekMsg(&nSize)) != NULL)
	{
		if (pCmd->nCmd != P_GS4WEB_DB_RPL_CMD)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_RT)"%s,ERROR CMD,%d,%d", __FUNCTION__, (int)pCmd->nCmd, pCmd->nSubCmd);
			m_msgerDB.FreeMsg();	// �����������ѭ����
			continue;
		}
		switch (pCmd->nSubCmd)
		{
		case DB_ACCOUNT_LOGIN_RPL:
			{
				GS4WEB_DB_ACCOUNT_LOGIN_RPL_T*	pRpl	= (GS4WEB_DB_ACCOUNT_LOGIN_RPL_T*)pCmd;
				SendMsgToGroup(pRpl->nFromGroupIdx, (void*)pRpl, nSize);
			}
			break;
		case DB_ADD_NOTIFY_RPL:
			{
				GS4WEB_DB_ADD_NOTIFY_RPL_T*	pAddNotifyRpl	= (GS4WEB_DB_ADD_NOTIFY_RPL_T*)pCmd;
				if (pAddNotifyRpl->nRst == BD_RST_OK)
				{
					Web_Notification_T*	pSrcNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pAddNotifyRpl);
					// 1.��֪ͨ��ӵ��ڴ���
					m_pNotificationMngS->AddNotification(pSrcNotification);
					
					// 2.��֪ͨ���͸�����
					m_vectrawbuf.resize(sizeof(GS4WEB_GROUP_ADD_NOTIFY_REQ_T) + pSrcNotification->GetTotalSize());
					GS4WEB_GROUP_ADD_NOTIFY_REQ_T*	pReq	= (GS4WEB_GROUP_ADD_NOTIFY_REQ_T*)m_vectrawbuf.getbuf();
					pReq->nCmd			= P_GS4WEB_GROUP_REQ_CMD;
					pReq->nSubCmd		= GROUP_ADD_NOTIFY_REQ;
					Web_Notification_T*	pNotify				= (Web_Notification_T*)wh_getptrnexttoptr(pReq);
					memcpy(pNotify, pSrcNotification, pSrcNotification->GetTotalSize());
					switch (pNotify->nGroupID)
					{
					case GROUP_ID_ALL_WITHOUT_GLOBAL:
					case GROUP_ID_ALL:
						{
							SendMsgToAllGroup(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
						break;
					default:
						{
							SendMsgToGroup(pNotify->nGroupID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}
						break;
					}
				}
				// ��������ظ�BDģ��
				{
					int	nNotifySize		= nSize - sizeof(BD_CMD_ADD_NOTIFY_RPL_T);
					m_vectrawbuf.resize(sizeof(BD_CMD_ADD_NOTIFY_RPL_T) + nNotifySize);
					BD_CMD_ADD_NOTIFY_RPL_T*	pRplCmd	= (BD_CMD_ADD_NOTIFY_RPL_T*)m_vectrawbuf.getbuf();
					pRplCmd->nCmd		= BD_CMD_ADD_NOTIFY_RPL;
					pRplCmd->nSize		= m_vectrawbuf.size();
					pRplCmd->nRst		= pAddNotifyRpl->nRst;
					memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pAddNotifyRpl), nNotifySize);
					_SendBDRplCmd(pAddNotifyRpl->nExt, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
			}
			break;
		case DB_LOAD_NOTIFY_RPL:
			{
				GS4WEB_DB_LOAD_NOTIFY_RPL_T*	pRplCmd	= (GS4WEB_DB_LOAD_NOTIFY_RPL_T*)pCmd;
				if (pRplCmd->nRst == BD_RST_OK)
				{
					// �����һ��
					m_pNotificationMngS->DelNotificationAll();
					// ��������֪ͨ���ڴ�
					Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRplCmd);
					for (int i=0; i<pRplCmd->nNum; i++)
					{
						m_pNotificationMngS->AddNotification(pNotification);
						pNotification					= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
					}
					// ��֪ͨ���͸���Щ����
					map<int, int>::iterator		it		= m_map4NeedToSendLoadingNotify.begin();
					for (; it != m_map4NeedToSendLoadingNotify.end(); ++it)
					{
						if (IsGroupExist(it->first))
						{
							SendNotificationToGroup(it->first);
						}
					}
					// �����ӳٷ����б�
					m_map4NeedToSendLoadingNotify.clear();
					// ʱ���¼��˳�
					m_teidLoadNotification.quit();
					// ��������ɹ�
					m_bLoadNotificationOK	= true;
				}
			}
			break;
		case DB_DEL_NOTIFY_BY_ID_RPL:
			{
				GS4WEB_DB_DEL_NOTIFY_BY_ID_RPL_T*	pDelNotifyRpl	= (GS4WEB_DB_DEL_NOTIFY_BY_ID_RPL_T*)pCmd;
				if (pDelNotifyRpl->nRst == BD_RST_OK)
				{
					Web_Notification_T*	pNotify	= m_pNotificationMngS->GetNotificationByID(pDelNotifyRpl->nNotificationID);
					if (pNotify != NULL)
					{
						// 1.�ô���Ҳɾһ��֪ͨ
						GS4WEB_GROUP_DEL_NOTIFY_BY_ID_REQ_T	ReqCmd;
						ReqCmd.nCmd				= P_GS4WEB_GROUP_REQ_CMD;
						ReqCmd.nSubCmd			= GROUP_DEL_NOTIFY_BY_ID_REQ;
						ReqCmd.nNotificationID	= pDelNotifyRpl->nNotificationID;
						switch (pNotify->nGroupID)
						{
						case GROUP_ID_ALL_WITHOUT_GLOBAL:
						case GROUP_ID_ALL:
							{
								SendMsgToAllGroup(&ReqCmd, sizeof(ReqCmd));
							}
							break;
						default:
							{
								SendMsgToGroup(pNotify->nGroupID, &ReqCmd, sizeof(ReqCmd));
							}
							break;
						}

						// 2.�ڴ���Ҳ��һ��
						m_pNotificationMngS->DelNotificationByID(pDelNotifyRpl->nNotificationID);
					}
				}
				// ��������ظ�BDģ��
				{
					BD_CMD_DEL_NOTIFY_BY_ID_RPL_T	RplCmd;
					RplCmd.nCmd				= BD_CMD_DEL_NOTIFY_BY_ID_RPL;
					RplCmd.nSize			= sizeof(BD_CMD_DEL_NOTIFY_BY_ID_RPL_T);
					RplCmd.nNotificationID	= pDelNotifyRpl->nNotificationID;
					RplCmd.nRst				= pDelNotifyRpl->nRst;

					_SendBDRplCmd(pDelNotifyRpl->nExt, (char*)&RplCmd, sizeof(RplCmd));
				}
			}
			break;
		case DB_DEL_NOTIFY_BT_RPL:
			{
				GS4WEB_DB_DEL_NOTIFY_BT_RPL_T*		pDelNotifyRpl	= (GS4WEB_DB_DEL_NOTIFY_BT_RPL_T*)pCmd;
				if (pDelNotifyRpl->nRst == BD_RST_OK)
				{
					// 1.��ȫ������ɾһ��
					GS4WEB_GROUP_DEL_NOTIFY_BT_REQ_T	ReqCmd;
					ReqCmd.nCmd				= P_GS4WEB_GROUP_REQ_CMD;
					ReqCmd.nSubCmd			= GROUP_DEL_NOTIFY_BT_REQ;
					ReqCmd.nTime			= pDelNotifyRpl->nTime;
					SendMsgToAllGroup(&ReqCmd, sizeof(ReqCmd));
					// 2.�ڴ�����һ��
					m_pNotificationMngS->DelNotificationBeforeTime(pDelNotifyRpl->nTime);
				}
				// ��������ظ�BDģ��
				{
					BD_CMD_DEL_NOTIFY_BEFORE_TIME_RPL_T	RplCmd;
					RplCmd.nCmd				= BD_CMD_DEL_NOTIFY_BEFORE_TIME_RPL;
					RplCmd.nSize			= sizeof(RplCmd);
					RplCmd.nRst				= pDelNotifyRpl->nRst;
					wh_gettimestr(pDelNotifyRpl->nTime, RplCmd.szTimeStr);
					_SendBDRplCmd(pDelNotifyRpl->nExt, (char*)&RplCmd, sizeof(RplCmd));
				}
			}
			break;
		case DB_DEL_NOTIFY_AT_RPL:
			{
				GS4WEB_DB_DEL_NOTIFY_AT_RPL_T*		pDelNotifyRpl	= (GS4WEB_DB_DEL_NOTIFY_AT_RPL_T*)pCmd;
				if (pDelNotifyRpl->nRst == BD_RST_OK)
				{
					// 1.��ȫ������ɾһ��
					GS4WEB_GROUP_DEL_NOTIFY_AT_REQ_T	ReqCmd;
					ReqCmd.nCmd				= P_GS4WEB_GROUP_REQ_CMD;
					ReqCmd.nSubCmd			= GROUP_DEL_NOTIFY_AT_REQ;
					ReqCmd.nTime			= pDelNotifyRpl->nTime;
					SendMsgToAllGroup(&ReqCmd, sizeof(ReqCmd));
					// 2.�ڴ�����һ��
					m_pNotificationMngS->DelNotificationAfterTime(pDelNotifyRpl->nTime);
				}
				// ��������ظ�BDģ��
				{
					BD_CMD_DEL_NOTIFY_AFTER_TIME_RPL_T	RplCmd;
					RplCmd.nCmd				= BD_CMD_DEL_NOTIFY_AFTER_TIME_RPL;
					RplCmd.nSize			= sizeof(RplCmd);
					RplCmd.nRst				= pDelNotifyRpl->nRst;
					wh_gettimestr(pDelNotifyRpl->nTime, RplCmd.szTimeStr);
					_SendBDRplCmd(pDelNotifyRpl->nExt, (char*)&RplCmd, sizeof(RplCmd));
				}
			}
			break;
		case DB_DEL_NOTIFY_ALL_RPL:
			{
				GS4WEB_DB_DEL_NOTIFY_ALL_RPL_T*		pDelNotifyRpl	= (GS4WEB_DB_DEL_NOTIFY_ALL_RPL_T*)pCmd;
				if (pDelNotifyRpl->nRst == BD_RST_OK)
				{
					// 1.��ȫ������ɾһ��
					GS4WEB_GROUP_DEL_NOTIFY_ALL_REQ_T	ReqCmd;
					ReqCmd.nCmd				= P_GS4WEB_GROUP_REQ_CMD;
					ReqCmd.nSubCmd			= GROUP_DEL_NOTIFY_ALL_REQ;
					SendMsgToAllGroup(&ReqCmd, sizeof(ReqCmd));
					// 2.�ڴ�����һ��
					m_pNotificationMngS->DelNotificationAll();
				}
				// ��������ظ�BDģ��
				{
					BD_CMD_DEL_NOTIFY_ALL_RPL_T	RplCmd;
					RplCmd.nCmd				= BD_CMD_DEL_NOTIFY_ALL_RPL;
					RplCmd.nSize			= sizeof(RplCmd);
					RplCmd.nRst				= pDelNotifyRpl->nRst;
					_SendBDRplCmd(pDelNotifyRpl->nExt, (char*)&RplCmd, sizeof(RplCmd));
				}
			}
			break;
		case DB_DEL_NOTIFY_BY_GROUP_RPL:
			{
				GS4WEB_DB_DEL_NOTIFY_BY_GROUP_RPL_T*	pDelNotifyRpl	= (GS4WEB_DB_DEL_NOTIFY_BY_GROUP_RPL_T*)pCmd;
				if (pDelNotifyRpl->nRst == BD_RST_OK)
				{
					// 1.��ȫ������ɾһ��
					GS4WEB_GROUP_DEL_NOTIFY_BY_GROUP_REQ_T	ReqCmd;
					ReqCmd.nCmd				= P_GS4WEB_GROUP_REQ_CMD;
					ReqCmd.nSubCmd			= GROUP_DEL_NOTIFY_BY_GROUP_REQ;
					ReqCmd.nGroupID			= pDelNotifyRpl->nGroupID;
					switch (ReqCmd.nGroupID)
					{
					case GROUP_ID_ALL_WITHOUT_GLOBAL:
					case GROUP_ID_ALL:
						{
							SendMsgToAllGroup(&ReqCmd, sizeof(ReqCmd));
						}
						break;
					default:
						{
							SendMsgToGroup(ReqCmd.nGroupID, &ReqCmd, sizeof(ReqCmd));
						}
						break;
					}
					// 2.�ڴ�����һ��
					m_pNotificationMngS->DelNotificationByGroup(pDelNotifyRpl->nGroupID);
				}
				// ��������ظ�BDģ��
				{
					BD_CMD_DEL_NOTIFY_BY_GROUP_RPL_T	RplCmd;
					RplCmd.nCmd			= BD_CMD_DEL_NOTIFY_BY_GROUP_RPL;
					RplCmd.nSize		= sizeof(RplCmd);
					RplCmd.nGroupID		= pDelNotifyRpl->nGroupID;
					RplCmd.nRst			= pDelNotifyRpl->nRst;
					_SendBDRplCmd(pDelNotifyRpl->nExt, (char*)&RplCmd, sizeof(RplCmd));
				}
			}
			break;
		case DB_ACCOUNT_CREATE_RPL:
			{
				GS4WEB_DB_ACCOUNT_CREATE_RPL_T*	pRpl	= (GS4WEB_DB_ACCOUNT_CREATE_RPL_T*)pCmd;
				SendMsgToGroup(pRpl->nFromGroupIdx, (void*)pRpl, nSize);
			}
			break;
		case DB_REGISTER_RPL:
			{
				GS4WEB_DB_REGISTER_RPL_T*	pRpl	= (GS4WEB_DB_REGISTER_RPL_T*)pCmd;
				SendMsgToGroup(pRpl->nFromGroupIdx, (void*)pRpl, nSize);
			}
			break;
		case DB_BIND_DEVICE_RPL:
			{
				GS4WEB_DB_BIND_DEVICE_RPL_T*	pRpl	= (GS4WEB_DB_BIND_DEVICE_RPL_T*)pCmd;
				SendMsgToGroup(pRpl->nFromGroupIdx, (void*)pRpl, nSize);
			}
			break;
		case DB_BAN_ACCOUNT_RPL:
			{
				GS4WEB_DB_BAN_ACCOUNT_RPL_T*	pRpl	= (GS4WEB_DB_BAN_ACCOUNT_RPL_T*)pCmd;

				BD_CMD_BAN_ACCOUNT_RPL_T	RplCmd;
				RplCmd.nCmd			= BD_CMD_BAN_ACCOUNT_RPL;
				RplCmd.nSize		= sizeof(RplCmd);
				RplCmd.nAccountID	= pRpl->nAccountID;
				RplCmd.nRst			= pRpl->nRst;
				RplCmd.bBanDevice	= pRpl->bBanDevice;
				WH_STRNCPY0(RplCmd.szDeviceID, pRpl->szDeviceID);

				_SendBDRplCmd(pRpl->nExt, (char*)&RplCmd, sizeof(RplCmd));
			}
			break;
		case DB_GET_ADCOLONY_RPL:
			{
				GS4WEB_DB_GET_ADCOLONY_RPL_T*	pRpl	= (GS4WEB_DB_GET_ADCOLONY_RPL_T*)pCmd;

				GS4WEB_GROUP_NEW_ADCOLONY_REQ_T	Cmd;
				Cmd.nCmd					= P_GS4WEB_GROUP_REQ_CMD;
				Cmd.nSubCmd					= GROUP_NEW_ADCOLONY_REQ;
				GS4WEB_DB_GET_ADCOLONY_RPL_T::AdColony_T*	pAdColony	= (GS4WEB_DB_GET_ADCOLONY_RPL_T::AdColony_T*)wh_getptrnexttoptr(pRpl);
				for (int i=0; i<pRpl->nNum; i++)
				{
					Cmd.nAccountID			= pAdColony->nAccountID;
					Cmd.nAmount				= pAdColony->nAmount;
					Cmd.nTime				= pAdColony->nTime;
					Cmd.nTransactionID		= pAdColony->nTransactionID;
					WH_STRNCPY0(Cmd.szName, pAdColony->szName);

					SendMsgToGroup(pAdColony->nGroupID, &Cmd, sizeof(Cmd));
				}
			}
			break;
		}
		m_msgerDB.FreeMsg();
	}

	return 0;
}
