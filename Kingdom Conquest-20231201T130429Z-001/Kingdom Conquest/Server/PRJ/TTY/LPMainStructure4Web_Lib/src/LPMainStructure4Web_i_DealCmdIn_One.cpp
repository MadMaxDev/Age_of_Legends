//////////////////////////////////////////////////////////////////////////
// 1.CAAFS4Web_LP_CLIENTWANTCLS4Web
// �ڴ��������Ϣ�����CLS4Web�������쳣(���ǵ���,�һ�δ����״̬ΪSTATUS_NOTHING),
// LP����Ϊ���Ը�CLS4Web���Լ����������,������������
// ���:�ͻ����޷�������ȥ,ֻ�����µ�¼;�������Ѵ��ڵ�Player��Ԫ�޷�����
// ��ʱ�Ȳ�������
//////////////////////////////////////////////////////////////////////////
#include "../inc/LPMainStructure4Web_i.h"

using namespace n_pngs;

int		LPMainStructure4Web_i::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	return 0;
}
int		LPMainStructure4Web_i::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch (nCmd)
	{
	case PNGSPACKET_2LOGIC_SETMOOD:
		{
			PNGSPACKET_2LOGIC_SETMOOD_T*	pCmdSetMood	= (PNGSPACKET_2LOGIC_SETMOOD_T*)pData;
			switch (pCmdSetMood->nMood)
			{
			case CMN::ILogic::MOOD_STOPPING:
				{
					m_nMood	= CMN::ILogic::MOOD_STOPPING;	// stopping״̬�µȴ����е�cls4web��caafs4web�ر�
					TellAllConnecterToQuit();
				}
				break;
			}
		}
		break;
	case PNGS_TR2CD_NOTIFY:
		{
			PNGS_TR2CD_NOTIFY_T*	pNotify	= (PNGS_TR2CD_NOTIFY_T*)pData;
			switch (pNotify->nSubCmd)
			{
			case PNGS_TR2CD_NOTIFY_T::SUBCMD_DISCONNECT:
				{
					map<int, AppInfo_T>::iterator it	= m_mapCntrID2AppInfo.find(pNotify->nParam1);
					if (it == m_mapCntrID2AppInfo.end())
					{
						// �����û�з���hi(����hi��Ϣ��Ӧ����)�Ͷϵ���
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"cannot find appinfo about cntr(no HI or HI error),%s,0x%X", __FUNCTION__, pNotify->nParam1);
					}
					else
					{
						AppInfo_T*	pAppInfo			= &it->second;
						switch (pAppInfo->nAppType)
						{
						case APPTYPE_CLS4WEB:
							{
								//DealCLS4WebDisconnection((CLS4WebUnit*)pAppInfo->pAppUnit);
								CLS4WebUnit*	pCLS4Web		= (CLS4WebUnit*)pAppInfo->pAppUnit;
								pCLS4Web->nCntrID	= 0;
								pCLS4Web->nStatus	= CLS4WebUnit::STATUS_NOTHING;
								// ע�������ʱ
								SetTE_CLS4Web_DropWait_TimeOut(pCLS4Web);
							}
							break;
						case APPTYPE_CAAFS4WEB:
							{
								((CAAFS4WebGroup*)pAppInfo->pAppUnit)->nCntrID	= 0;
							}
							break;
						default:
							{
								
							}
							break;
						}
						// ��Ҫɾ����Ӧ��������Ϣ
						m_mapCntrID2AppInfo.erase(it);
					}
				}
				break;
			case PNGS_TR2CD_NOTIFY_T::SUBCMD_CONNECT:
				{
					SetTE_Hello_TimeOut(pNotify->nParam1);
				}
				break;
			default:
				break;
			}
		}
		break;
	case PNGS_TR2CD_CMD:
		{
			return DealCmdIn_One_PNGS_TR2CD_CMD(pData, nDSize);
		}
		break;
	case LPPACKET_2MS_CMD2ONEPLAYER:
		{
			LPPACKET_2MS_CMD2ONEPLAYER_T*	pCmd	= (LPPACKET_2MS_CMD2ONEPLAYER_T*)pData;
			SendCmdToPlayer(pCmd->nClientID, pCmd->pData, pCmd->nDSize);
		}
		break;
	case LPPACKET_2MS_CMD2MULTIPLAYER:
		{
			LPPACKET_2MS_CMD2MULTIPLAYER_T*	pCmd	= (LPPACKET_2MS_CMD2MULTIPLAYER_T*)pData;
			SendCmdToMultiPlayer(pCmd->paClientID, pCmd->nClientNum, pCmd->pData, pCmd->nDSize);
		}
		break;
	case LPPACKET_2MS_CMD2PLAYERWITHTAG:
		{
			LPPACKET_2MS_CMD2PLAYERWITHTAG_T*	pCmd	= (LPPACKET_2MS_CMD2PLAYERWITHTAG_T*)pData;
			SendCmdToAllPlayerByTag(pCmd->nTagIdx, pCmd->nTagVal, pCmd->pData, pCmd->nDSize);
		}
		break;
	case LPPACKET_2MS_CMD2ALLPLAYER:
		{
			LPPACKET_2MS_CMD2ALLPLAYER_T*	pCmd	= (LPPACKET_2MS_CMD2ALLPLAYER_T*)pData;
			SendCmdToAllPlayer(pCmd->pData, pCmd->nDSize);
		}
		break;
	case LPPACKET_2MS_SETPLAYERTAG:
		{
			LPPACKET_2MS_SETPLAYERTAG_T*	pCmd	= (LPPACKET_2MS_SETPLAYERTAG_T*)pData;
			SetPlayerTag(pCmd->nClientID, pCmd->nTagIdx, pCmd->nTagVal);
		}
		break;
	case LPPACKET_2MS_CMD2PLAYERWITHTAG64:
		{
			LPPACKET_2MS_CMD2PLAYERWITHTAG64_T*	pCmd	= (LPPACKET_2MS_CMD2PLAYERWITHTAG64_T*)pData;
			SendCmdToAllPlayerByTag64(pCmd->nTag, pCmd->pData, pCmd->nDSize);
		}
		break;
	case LPPACKET_2MS_SETPLAYERTAG64:
		{
			LPPACKET_2MS_SETPLAYERTAG64_T*	pCmd	= (LPPACKET_2MS_SETPLAYERTAG64_T*)pData;
			SetPlayerTag64(pCmd->nClientID, pCmd->nTag, pCmd->bDel);
		}
		break;
	case LPPACKET_2MS_KICKPLAYER:
		{
			LPPACKET_2MS_KICKPLAYER_T*	pCmd		= (LPPACKET_2MS_KICKPLAYER_T*)pData;
			TellCLS4WebToKickPlayer(pCmd->nClientID, pCmd->nKickSubCmd);
		}
		break;
	case LPPACKET_2MS_CMD2CLS4Web:
		{
			LPPACKET_2MS_CMD2CLS4Web_T*	pCmd	= (LPPACKET_2MS_CMD2CLS4Web_T*)pData;
			SendCmdToPlayerCLS4Web(pCmd->nClientID, pCmd->pData, pCmd->nDSize);
		}
		break;
	case LPPACKET_2MS_CAAFS4WebCTRL:
		{
			// ֱ�ӷ��͸�CAAFS4Web
			LP_CAAFS4Web_CTRL_T*	pCmd	= (LP_CAAFS4Web_CTRL_T*)pData;
			int	nCntrID	= m_aCAAFS4WebGroup[pCmd->nCAAFS4WebIdx].nCntrID;
			if (nCntrID != 0)
			{
				SendCmdToConnecter(nCntrID, pCmd, sizeof(*pCmd));
			}
		}
		break;
	default:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,%d,unknown,%d", __FUNCTION__, nCmd, nDSize);
		}
		break;
	}
	return 0;
}
int		LPMainStructure4Web_i::DealCmdIn_One_PNGS_TR2CD_CMD(const void *pData, int nDSize)
{
	PNGS_TR2CD_CMD_T*	pCDCmd	= (PNGS_TR2CD_CMD_T*)pData;
	pngs_cmd_t*			pCmd	= (pngs_cmd_t*)pCDCmd->pData;
	switch (*pCmd)
	{
	case CAAFS4Web_LP_HI:
		{
			CAAFS4Web_LP_HI_T*	pHI	= (CAAFS4Web_LP_HI_T*)pCmd;
			LP_CAAFS4Web_HI_T	HI;
			HI.nCmd				= LP_CAAFS4Web_HI;
			HI.nRst				= LP_CAAFS4Web_HI_T::RST_OK;
			HI.nLPVer			= LPMainStructure_VER;
			if (LPMainStructure_VER != pHI->nCAAFS4WebVer)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CAAFS4Web_LP_HI_T,,bad ver,%d,%d,%d", __FUNCTION__, pHI->nGroupID, pHI->nCAAFS4WebVer, CAAFS4Web_VER);
				// ����ʧ��HI
				HI.nRst			= LP_CAAFS4Web_HI_T::RST_ERR_BAD_VER;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			// ע:CAAFS4Web����յ�ʧ�ܵ�HI���ر�����,��������ر�,���˳�ʱLP��ص�
			// �ж�group����Ч��
			if (pHI->nGroupID<0 || pHI->nGroupID>=LP_MAX_CAAFS4Web_NUM)
			{
				// group��Ҫ������Χ
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CAAFS4Web_LP_HI_T,,group id out of range,%d,%d", __FUNCTION__, pHI->nGroupID, LP_MAX_CAAFS4Web_NUM);
				// ����ʧ��HI
				HI.nRst			= LP_CAAFS4Web_HI_T::RST_ERR_BAD_GROUPID;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			CAAFS4WebGroup*	pGroup	= &m_aCAAFS4WebGroup[pHI->nGroupID];
			if (pGroup->nCntrID != 0)
			{
				// �Ѿ������group��caafs4web��,����������
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CAAFS4Web_LP_HI_T,,group id dup,%d,%s", __FUNCTION__, pHI->nGroupID, cmn_get_IP(pCDCmd->IP));
				// ����ʧ��HI
				HI.nRst				= LP_CAAFS4Web_HI_T::RST_ERR_BAD_GROUPID;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			else
			{
				// ��������������
				pGroup->nCntrID		= pCDCmd->nConnecterID;
				// ����������Ϣ
				AppInfo_T	appInfo;
				appInfo.nAppType	= APPTYPE_CAAFS4WEB;
				appInfo.pAppUnit	= pGroup;
				m_mapCntrID2AppInfo.insert(map<int, AppInfo_T>::value_type(pCDCmd->nConnecterID, appInfo));
				char	szReConnect[64]	= "";
				if (pHI->bReconnect)
				{
					strcpy(szReConnect, ",RECONNECT");
					// �����Ĳ��÷���HI��
				}
				else
				{
					SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				}
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1386,LPMS_RT)"HI from CAAFS4Web,0x%X,%s%s", pCDCmd->nConnecterID, cmn_get_IP(pCDCmd->IP), szReConnect);
			}
		}
		break;
	case CAAFS4Web_LP_CLIENTWANTCLS4Web:
		{
			map<int, AppInfo_T>::iterator	it	= m_mapCntrID2AppInfo.find(pCDCmd->nConnecterID);
			if (it == m_mapCntrID2AppInfo.end())
			{
				return 0;
			}
			
			CAAFS4WebGroup*		pGroup			= (CAAFS4WebGroup*)it->second.pAppUnit;
			int					nGroupID		= GetCAAFS4WebGroupID(pGroup);

			CAAFS4Web_LP_CLIENTWANTCLS4Web_T*	pClientWantCLS4Web	= (CAAFS4Web_LP_CLIENTWANTCLS4Web_T*)pCmd;
			LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T	ClientCanGo;
			ClientCanGo.nCmd					= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web;
			ClientCanGo.nClientIDInCAAFS4Web	= pClientWantCLS4Web->nClientIDInCAAFS4Web;
			
			// �������CAAFS4Web��group�е��ĸ�CLS4Web��λ�����
			CLS4WebUnit*	pCLS4Web			= NULL;
			int				nMinPlayer			= m_cfginfo.anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup[nGroupID];
			for (whDList<CLS4WebUnit*>::node* pNode=pGroup->dlCLS4Web.begin(); pNode!=pGroup->dlCLS4Web.end(); pNode=pNode->next)
			{
				CLS4WebUnit*	pTmpCLS4Web		= pNode->data;
				if (!pTmpCLS4Web->CanAcceptPlayer()
					|| pTmpCLS4Web->GetCurPlayerNum()>=pTmpCLS4Web->nMaxPlayer
					|| pTmpCLS4Web->nGroupID != nGroupID)
				{
					continue;
				}
				if (pTmpCLS4Web->GetCurPlayerNum()<nMinPlayer)
				{
					nMinPlayer	= pTmpCLS4Web->GetCurPlayerNum();
					pCLS4Web	= pTmpCLS4Web;
				}
			}
			if (pCLS4Web == NULL)
			{
				// û��λ����
				ClientCanGo.nRst	= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T::RST_ERR_NOCLS4WebCANACCEPT;
				SendCmdToConnecter(pCDCmd->nConnecterID, &ClientCanGo, sizeof(ClientCanGo));
				return 0;
			}

			// �������
			PlayerUnit*	pPlayer	= NULL;
			int			nID		= m_Players.AllocUnit(pPlayer);
			if (nID < 0)
			{
				// ֪ͨCAAFS4Web�޷����з���
				ClientCanGo.nRst	= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T::RST_ERR_MEM;
				SendCmdToConnecter(pCDCmd->nConnecterID, &ClientCanGo, sizeof(ClientCanGo));
				return 0;
			}
			// �����������
			pPlayer->clear();
			pPlayer->nID		= nID;
			pPlayer->nStatus	= PlayerUnit::STATUS_WANTCLS4Web;
			pPlayer->nCLS4WebID	= pCLS4Web->nID;
			pPlayer->nClientIDInCAAFS4Web	= pClientWantCLS4Web->nClientIDInCAAFS4Web;
			pPlayer->IP			= pClientWantCLS4Web->IP;
			pPlayer->nTermType	= pClientWantCLS4Web->nTermType;
			pCLS4Web->dlPlayer.AddToTail(&pPlayer->dlnodeInCLS4Web);
			// ����������ݸ�CLS4Web
			// ֻҪCLS4Web����������һ���з���,�������,����LPȥ����
			LP_CLS4Web_CLIENTWANTCLS4Web_T	ToCLS4WebClientWantCLS4Web;
			ToCLS4WebClientWantCLS4Web.nCmd	= LP_CLS4Web_CLIENTWANTCLS4Web;
			ToCLS4WebClientWantCLS4Web.nClientID	= nID;
			ToCLS4WebClientWantCLS4Web.nPassword	= pClientWantCLS4Web->nPassword;
			ToCLS4WebClientWantCLS4Web.nTermType	= pClientWantCLS4Web->nTermType;
			SendCmdToConnecter(pCLS4Web->nCntrID, &ToCLS4WebClientWantCLS4Web, sizeof(ToCLS4WebClientWantCLS4Web));

			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"CAAFS4Web_LP_CLIENTWANTCLS4Web,0x%X,0x%X,%u", pPlayer->nID, pPlayer->nClientIDInCAAFS4Web, pClientWantCLS4Web->nPassword);
		}
		break;
	case CAAFS4Web_LP_CLIENT_CMD:
		{
			map<int, AppInfo_T>::iterator it	= m_mapCntrID2AppInfo.find(pCDCmd->nConnecterID);
			if (it != m_mapCntrID2AppInfo.end())
			{
				CAAFS4WebGroup* pGroup			= (CAAFS4WebGroup*)it->second.pAppUnit;
				assert(pGroup != NULL);
				assert(pGroup->nCntrID != 0);
				int	nGroupID	= GetCAAFS4WebGroupID(pGroup);
				// �����ϲ�
				CAAFS4Web_LP_CLIENT_CMD_T*	pClientCmd	= (CAAFS4Web_LP_CLIENT_CMD_T*)pCmd;
				LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web_T	Cmd2GP;
				Cmd2GP.nCAAFS4WebIdx					= nGroupID;
				Cmd2GP.nClientIDInCAAFS4Web				= pClientCmd->nClientIDInCAAFS4Web;
				Cmd2GP.IP								= pClientCmd->IP;
				Cmd2GP.pData							= wh_getptrnexttoptr(pClientCmd);
				Cmd2GP.nDSize							= nDSize - sizeof(*pClientCmd);
				CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web, Cmd2GP);
			}
		}
		break;
	case CLS4Web_LP_HI:
		{
			// ע:CLS4Web����յ�ʧ�ܵ�HI���ر�����,��������ر�,���˳�ʱLP������ر�
			CLS4Web_LP_HI_T*	pHI	= (CLS4Web_LP_HI_T*)pCmd;
			LP_CLS4Web_HI_T		HI;
			HI.nCmd				= LP_CLS4Web_HI;
			HI.nRst				= LP_CLS4Web_HI_T::RST_OK;
			HI.nLPVer			= LPMainStructure_VER;
			if (CLS4Web_VER != pHI->nCLS4WebVer)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CLS4Web_LP_HI_T,,bad ver,%d,%d,%d", __FUNCTION__, pHI->nGroupID, pHI->nCLS4WebVer, CLS4Web_VER);
				// ����ʧ��HI
				HI.nRst			= LP_CLS4Web_HI_T::RST_ERR_BAD_VER;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			// �ж�group����Ч��
			if (pHI->nGroupID<0 || pHI->nGroupID>=LP_MAX_CAAFS4Web_NUM)
			{
				// group��Ҫ������Χ
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CLS4Web_LP_HI_T,,group id out of range,%d,%d", __FUNCTION__, pHI->nGroupID, LP_MAX_CAAFS4Web_NUM);
				// ����ʧ��HI
				HI.nRst			= LP_CLS4Web_HI_T::RST_ERR_BAD_GROUPID;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			CLS4WebUnit*	pCLS4WebUnit	= NULL;
			char		szReConnect[64]	= "";
			if (pHI->nOldID > 0)
			{
				// ˵����������
				// ����һ�¾ɵ�
				pCLS4WebUnit	= m_CLS4Webs.getptr(pHI->nOldID);
				// Ӧ���ǿ����ҵ���
				if (pCLS4WebUnit == NULL)	// �����������Ϊ(1)LP������,CLS4Webû�е���(2)���ӶϿ�,�����ڶ�����ʱ��û�������ɹ�
				{
					// ������CLS4Web�˳�
					SVR_CLS4Web_CTRL_T	ctrl;
					ctrl.nCmd			= SVR_CLS4Web_CTRL;
					
					// ��Ϊ��һ���µļ���,��ΪLP�����������CLS4Web����Ϣ,�����Ǹ���û��CLS4Web����Ϣ
					// ���ǲ�������ʹ��(���ܼ���CAAFS4WebGroup),��Ҫ�ȵ���CLS4Web�Ѿ����������Ӷ������˲���ʹ��
					
					// ����Ӧ��group�е�CLS4Web�����Ƿ��Ѿ�����
					if (m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size() >= m_cfginfo.anMaxCLS4WebNumInCAAFS4WebGroup[pHI->nGroupID])
					{
						// ���group��CLS�Ѿ�̫����
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"CLS4Web_LP_HI_T,,too many CLS4Web,%d,%d", pHI->nGroupID, m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size());
						// �����CLS4Web�رհ�
						ctrl.nSubCmd		= SVR_CLS4Web_CTRL_T::SUBCMD_EXIT;
						SendCmdToConnecter(pCDCmd->nConnecterID, &ctrl, sizeof(ctrl));
						return 0;
					}
					// ����CLS4Web����
					int		nID				= m_CLS4Webs.AllocUnit(pCLS4WebUnit);
					if (nID < 0)
					{
						//assert(0);
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"CLS4Web_LP_HI_T,,cannot alloc CLS4Web,%d,%d", pHI->nGroupID, m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size());
						// �����CLS4Web�رհ�
						ctrl.nSubCmd		= SVR_CLS4Web_CTRL_T::SUBCMD_EXIT;
						SendCmdToConnecter(pCDCmd->nConnecterID, &ctrl, sizeof(ctrl));
						return 0;
					}
					// ���
					pCLS4WebUnit->clear();
					pCLS4WebUnit->nCntrID	= pCDCmd->nConnecterID;
					pCLS4WebUnit->nID		= nID;
					pCLS4WebUnit->nGroupID	= pHI->nGroupID;
					pCLS4WebUnit->nMaxPlayer	= m_cfginfo.anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup[pCLS4WebUnit->nGroupID];
					// �������Թ���
					pCLS4WebUnit->nStatus	= CLS4WebUnit::STATUS_NOTHING;

					// �ߵ��������Ӻ���ܹ���
					ctrl.nSubCmd		= SVR_CLS4Web_CTRL_T::SUBCMD_KICK_ALL_PLAYER;
					SendCmdToConnecter(pCDCmd->nConnecterID, &ctrl, sizeof(ctrl));
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1386,LPMS_RT)"CLS4Web_LP_HI_T,,old id not found,kick all player,0x%X", pHI->nOldID);
				}
				else
				{
					// ˵����������ʱ�ж϶���
					strcpy(szReConnect, ",RECONNECT");
				}
			}
			else
			{
				// ����Ӧ��group�е�CLS4Web�����Ƿ��Ѿ�����
				if (m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size() >= m_cfginfo.anMaxCLS4WebNumInCAAFS4WebGroup[pHI->nGroupID])
				{
					// ���group��CLS�Ѿ�̫����
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"CLS4Web_LP_HI_T,,too many CLS4Web,%d,%d", pHI->nGroupID, m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size());
					// ����ʧ��HI
					HI.nRst				= LP_CLS4Web_HI_T::RST_ERR_TOOMANY_CLS4Web;
					SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
					return 0;
				}
				// ����CLS4Web����
				int		nID				= m_CLS4Webs.AllocUnit(pCLS4WebUnit);
				if (nID < 0)
				{
					//assert(0);
					HI.nRst		= LP_CLS4Web_HI_T::RST_ERR_MEMERR;
					SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
					return 0;
				}
				// ���
				pCLS4WebUnit->clear();
				pCLS4WebUnit->nCntrID	= pCDCmd->nConnecterID;
				pCLS4WebUnit->nID		= nID;
				pCLS4WebUnit->nGroupID	= pHI->nGroupID;
				pCLS4WebUnit->nMaxPlayer	= m_cfginfo.anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup[pCLS4WebUnit->nGroupID];
				// ����CAAFS4Web/CLS4Web��group
				m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.AddToTail(&pCLS4WebUnit->dlnode);
				// �ȼٶ�������ʽ��ʼ������
				pCLS4WebUnit->nStatus	= CLS4WebUnit::STATUS_WORKING;
				// ����HI
				HI.nCLS4WebID			= nID;
				HI.nLPMaxPlayer			= m_cfginfo.nMaxPlayer;
				HI.nCLS4WebMaxConnection= pCLS4WebUnit->nMaxPlayer;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
			}
			// ����appinfo
			AppInfo_T	appinfo;
			appinfo.nAppType		= APPTYPE_CLS4WEB;
			appinfo.pAppUnit		= pCLS4WebUnit;
			m_mapCntrID2AppInfo.insert(map<int, AppInfo_T>::value_type(pCDCmd->nConnecterID, appinfo));
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1386,LPMS_RT)"HI from CLS4Web,0x%X,%s%s", pCDCmd->nConnecterID, cmn_get_IP(pCDCmd->IP), szReConnect);
			if (szReConnect[0])
			{
				// ��CLS4Web�����Լ��Ƿ������û�����
				TellCLS4WebToCheckPlayerOffline(pCLS4WebUnit);
			}
		}
		break;
	case CLS4Web_LP_HI1:
		{
			map<int, AppInfo_T>::iterator	it	= m_mapCntrID2AppInfo.find(pCDCmd->nConnecterID);
			if (it == m_mapCntrID2AppInfo.end())
			{
				return 0;
			}
			CLS4WebUnit*	pCLS4Web	= (CLS4WebUnit*)it->second.pAppUnit;
			CLS4Web_LP_HI1_T*	pHI1	= (CLS4Web_LP_HI1_T*)pCmd;
			pCLS4Web->IP				= pHI1->IP;
			pCLS4Web->nPort				= pHI1->nPort;

			char	szAfterKick[64]		= "";
			if (pCLS4Web->nStatus == CLS4WebUnit::STATUS_NOTHING)
			{
				pCLS4Web->nStatus		= CLS4WebUnit::STATUS_WORKING;
				// ����CAAFS4WebGroup
				m_aCAAFS4WebGroup[pCLS4Web->nGroupID].dlCLS4Web.AddToTail(&pCLS4Web->dlnode);
				strcpy(szAfterKick, ",after kick");
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1386,LPMS_RT)"HI1 from CLS4Web,0x%X,%s,%d%s", pCLS4Web->nID, cmn_get_IP(pCLS4Web->IP), pCLS4Web->nPort, szAfterKick);
		}
		break;
	case CLS4Web_LP_CLIENTCANGOTOCLS4Web:
		{
			map<int, AppInfo_T>::iterator	it	= m_mapCntrID2AppInfo.find(pCDCmd->nConnecterID);
			if (it == m_mapCntrID2AppInfo.end())
			{
				return 0;
			}
			CLS4WebUnit*	pCLS4Web			= (CLS4WebUnit*)it->second.pAppUnit;

			CLS4Web_LP_CLIENTCANGOTOCLS4Web_T*	pClientCanGo	= (CLS4Web_LP_CLIENTCANGOTOCLS4Web_T*)pCmd;
			PlayerUnit*	pPlayer		= m_Players.getptr(pClientCanGo->nClientID);
			assert(pPlayer != NULL);	// �����
			// �յ������Ϣ��˵��һ���ɹ���
			// ���״̬�ı�
			pPlayer->nStatus		= PlayerUnit::STATUS_GOINGTOCLS4Web;
			// ��Ҷ�Ӧ��CLS4WebӦ��һ���Ƿ������ָ���CLS4Web
			assert(pCLS4Web->nID == pPlayer->nCLS4WebID);
			// �ҵ�CAAFS4Web����
			CAAFS4WebGroup*	pGroup	= GetCAAFS4WebByGroupID(pCLS4Web->nGroupID);
			if (pGroup!=NULL && pGroup->nCntrID!=0)
			{
				// ����CAAFS4Web
				LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T	ClientCanGo;
				ClientCanGo.nCmd	= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web;
				ClientCanGo.nRst	= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T::RST_OK;
				ClientCanGo.nPort	= pCLS4Web->nPort;
				if (m_cfginfo.bCLSUseCAAFSIP)
				{
					ClientCanGo.IP	= 0;
				}
				else
				{
					ClientCanGo.IP	= pCLS4Web->IP;
				}
				
				ClientCanGo.nClientIDInCAAFS4Web	= pPlayer->nClientIDInCAAFS4Web;
				ClientCanGo.nClientID				= pPlayer->nID;
				SendCmdToConnecter(pGroup->nCntrID, &ClientCanGo, sizeof(ClientCanGo));
			}
			else
			{
				// CAAFS4Web������
				//RemovePlayerUnit(pPlayer);//CLS4Web�ᷢ���Ƴ�,��߾Ͳ�����(connect to me timeout)
				GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"CAAFS4Web Lost,0x%X,0x%X,%d", pPlayer->nID, pPlayer->nClientIDInCAAFS4Web, pCLS4Web->nGroupID);
			}
		}
		break;
	case CLS4Web_LP_CLIENTTOCLS4Web_RST:
		{
			CLS4Web_LP_CLIENTTOCLS4Web_RST_T*	pClientToCLS4WebRst	= (CLS4Web_LP_CLIENTTOCLS4Web_RST_T*)pCmd;
			if (pClientToCLS4WebRst->nRst == CLS4Web_LP_CLIENTTOCLS4Web_RST_T::RST_OK)
			{
				// �ɹ�
				PlayerUnit*	pPlayer		= m_Players.getptr(pClientToCLS4WebRst->nClientID);
				assert(pPlayer != NULL);
				// ��Ҹı�״̬
				pPlayer->nStatus		= PlayerUnit::STATUS_GAMING;
				// ��������Ѿ���CAAFS4Web�޹���
				// ֪ͨGamePlay
				LPPACKET_2GP_PLAYERONLINE_T	CmdPlayerOnline;
				CmdPlayerOnline.nClientID	= pPlayer->nID;
				CmdPlayerOnline.IP			= pPlayer->IP;
				CmdPlayerOnline.nTermType	= pPlayer->nTermType;
				CmdOutToLogic_AUTO(m_pLogicGamePlay, LPPACKET_2GP_PLAYERONLINE, &CmdPlayerOnline, sizeof(CmdPlayerOnline));
			}
			else
			{
				// ʧ��,ɾ�����
				RemovePlayerUnit(pClientToCLS4WebRst->nClientID);
			}
		}
		break;
	case CLS4Web_LP_CLIENT_DROP:
		{
			CLS4Web_LP_CLIENT_DROP_T*	pDrop	= (CLS4Web_LP_CLIENT_DROP_T*)pCmd;
			RemovePlayerUnit(pDrop->nClientID, pDrop->nRemoveReason);
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"CLS4Web_LP_CLIENT_DROP_T,%d,0x%X", pDrop->nRemoveReason, pDrop->nClientID);
		}
		break;
	case CLS4Web_SVR_CLIENT_DATA:
		{
			InnerRouteClientGameCmd(LPPACKET_2GP_PLAYERCMD, (CLS4Web_SVR_CLIENT_DATA_T*)pCmd, pCDCmd->nDSize);
		}
		break;
	default:
		break;
	}
	return 0;
}
