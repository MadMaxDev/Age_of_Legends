//////////////////////////////////////////////////////////////////////////
// 1.CAAFS4Web_LP_CLIENTWANTCLS4Web
// 在处理这个消息中如果CLS4Web的连接异常(不是当掉,且还未设置状态为STATUS_NOTHING),
// LP误认为可以该CLS4Web可以继续接收玩家,则会造成死连接
// 后果:客户端无法继续下去,只能重新登录;服务器已存在的Player单元无法回收
// 暂时先不处理了
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
					m_nMood	= CMN::ILogic::MOOD_STOPPING;	// stopping状态下等待所有的cls4web和caafs4web关闭
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
						// 这个是没有发来hi(或者hi信息对应不上)就断掉的
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
								// 注册断连超时
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
						// 需要删除对应的连接信息
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
			// 直接发送给CAAFS4Web
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
				// 发送失败HI
				HI.nRst			= LP_CAAFS4Web_HI_T::RST_ERR_BAD_VER;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			// 注:CAAFS4Web如果收到失败的HI则会关闭连接,如果它不关闭,则到了超时LP会关掉
			// 判断group的有效性
			if (pHI->nGroupID<0 || pHI->nGroupID>=LP_MAX_CAAFS4Web_NUM)
			{
				// group需要超出范围
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CAAFS4Web_LP_HI_T,,group id out of range,%d,%d", __FUNCTION__, pHI->nGroupID, LP_MAX_CAAFS4Web_NUM);
				// 发送失败HI
				HI.nRst			= LP_CAAFS4Web_HI_T::RST_ERR_BAD_GROUPID;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			CAAFS4WebGroup*	pGroup	= &m_aCAAFS4WebGroup[pHI->nGroupID];
			if (pGroup->nCntrID != 0)
			{
				// 已经有这个group的caafs4web了,不能再连入
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CAAFS4Web_LP_HI_T,,group id dup,%d,%s", __FUNCTION__, pHI->nGroupID, cmn_get_IP(pCDCmd->IP));
				// 发送失败HI
				HI.nRst				= LP_CAAFS4Web_HI_T::RST_ERR_BAD_GROUPID;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			else
			{
				// 关联连接器对象
				pGroup->nCntrID		= pCDCmd->nConnecterID;
				// 设置连接信息
				AppInfo_T	appInfo;
				appInfo.nAppType	= APPTYPE_CAAFS4WEB;
				appInfo.pAppUnit	= pGroup;
				m_mapCntrID2AppInfo.insert(map<int, AppInfo_T>::value_type(pCDCmd->nConnecterID, appInfo));
				char	szReConnect[64]	= "";
				if (pHI->bReconnect)
				{
					strcpy(szReConnect, ",RECONNECT");
					// 重连的不用发送HI了
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
			
			// 看看这个CAAFS4Web的group中的哪个CLS4Web空位置最多
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
				// 没有位置了
				ClientCanGo.nRst	= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T::RST_ERR_NOCLS4WebCANACCEPT;
				SendCmdToConnecter(pCDCmd->nConnecterID, &ClientCanGo, sizeof(ClientCanGo));
				return 0;
			}

			// 创建玩家
			PlayerUnit*	pPlayer	= NULL;
			int			nID		= m_Players.AllocUnit(pPlayer);
			if (nID < 0)
			{
				// 通知CAAFS4Web无法进行分配
				ClientCanGo.nRst	= LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T::RST_ERR_MEM;
				SendCmdToConnecter(pCDCmd->nConnecterID, &ClientCanGo, sizeof(ClientCanGo));
				return 0;
			}
			// 数据设置相关
			pPlayer->clear();
			pPlayer->nID		= nID;
			pPlayer->nStatus	= PlayerUnit::STATUS_WANTCLS4Web;
			pPlayer->nCLS4WebID	= pCLS4Web->nID;
			pPlayer->nClientIDInCAAFS4Web	= pClientWantCLS4Web->nClientIDInCAAFS4Web;
			pPlayer->IP			= pClientWantCLS4Web->IP;
			pPlayer->nTermType	= pClientWantCLS4Web->nTermType;
			pCLS4Web->dlPlayer.AddToTail(&pPlayer->dlnodeInCLS4Web);
			// 发送玩家数据给CLS4Web
			// 只要CLS4Web连接正常就一定有返回,如果当了,则由LP去清理
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
				// 传给上层
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
			// 注:CLS4Web如果收到失败的HI则会关闭连接,如果它不关闭,则到了超时LP会把它关闭
			CLS4Web_LP_HI_T*	pHI	= (CLS4Web_LP_HI_T*)pCmd;
			LP_CLS4Web_HI_T		HI;
			HI.nCmd				= LP_CLS4Web_HI;
			HI.nRst				= LP_CLS4Web_HI_T::RST_OK;
			HI.nLPVer			= LPMainStructure_VER;
			if (CLS4Web_VER != pHI->nCLS4WebVer)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CLS4Web_LP_HI_T,,bad ver,%d,%d,%d", __FUNCTION__, pHI->nGroupID, pHI->nCLS4WebVer, CLS4Web_VER);
				// 发送失败HI
				HI.nRst			= LP_CLS4Web_HI_T::RST_ERR_BAD_VER;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			// 判断group的有效性
			if (pHI->nGroupID<0 || pHI->nGroupID>=LP_MAX_CAAFS4Web_NUM)
			{
				// group需要超出范围
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"%s,CLS4Web_LP_HI_T,,group id out of range,%d,%d", __FUNCTION__, pHI->nGroupID, LP_MAX_CAAFS4Web_NUM);
				// 发送失败HI
				HI.nRst			= LP_CLS4Web_HI_T::RST_ERR_BAD_GROUPID;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
				return 0;
			}
			CLS4WebUnit*	pCLS4WebUnit	= NULL;
			char		szReConnect[64]	= "";
			if (pHI->nOldID > 0)
			{
				// 说明是重连的
				// 查找一下旧的
				pCLS4WebUnit	= m_CLS4Webs.getptr(pHI->nOldID);
				// 应该是可以找到的
				if (pCLS4WebUnit == NULL)	// 这种情况是因为(1)LP当掉了,CLS4Web没有当掉(2)连接断开,但是在断连超时中没有重连成功
				{
					// 发送让CLS4Web退出
					SVR_CLS4Web_CTRL_T	ctrl;
					ctrl.nCmd			= SVR_CLS4Web_CTRL;
					
					// 认为是一个新的加入,因为LP或者清理掉了CLS4Web的信息,或者是根本没有CLS4Web的信息
					// 但是不能立即使用(不能加入CAAFS4WebGroup),需要等到该CLS4Web已经将现有连接都踢完了才能使用
					
					// 看对应的group中的CLS4Web数量是否已经够多
					if (m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size() >= m_cfginfo.anMaxCLS4WebNumInCAAFS4WebGroup[pHI->nGroupID])
					{
						// 这个group的CLS已经太多了
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"CLS4Web_LP_HI_T,,too many CLS4Web,%d,%d", pHI->nGroupID, m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size());
						// 让这个CLS4Web关闭吧
						ctrl.nSubCmd		= SVR_CLS4Web_CTRL_T::SUBCMD_EXIT;
						SendCmdToConnecter(pCDCmd->nConnecterID, &ctrl, sizeof(ctrl));
						return 0;
					}
					// 创建CLS4Web对象
					int		nID				= m_CLS4Webs.AllocUnit(pCLS4WebUnit);
					if (nID < 0)
					{
						//assert(0);
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"CLS4Web_LP_HI_T,,cannot alloc CLS4Web,%d,%d", pHI->nGroupID, m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size());
						// 让这个CLS4Web关闭吧
						ctrl.nSubCmd		= SVR_CLS4Web_CTRL_T::SUBCMD_EXIT;
						SendCmdToConnecter(pCDCmd->nConnecterID, &ctrl, sizeof(ctrl));
						return 0;
					}
					// 清空
					pCLS4WebUnit->clear();
					pCLS4WebUnit->nCntrID	= pCDCmd->nConnecterID;
					pCLS4WebUnit->nID		= nID;
					pCLS4WebUnit->nGroupID	= pHI->nGroupID;
					pCLS4WebUnit->nMaxPlayer	= m_cfginfo.anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup[pCLS4WebUnit->nGroupID];
					// 还不可以工作
					pCLS4WebUnit->nStatus	= CLS4WebUnit::STATUS_NOTHING;

					// 踢掉已有连接后才能工作
					ctrl.nSubCmd		= SVR_CLS4Web_CTRL_T::SUBCMD_KICK_ALL_PLAYER;
					SendCmdToConnecter(pCDCmd->nConnecterID, &ctrl, sizeof(ctrl));
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1386,LPMS_RT)"CLS4Web_LP_HI_T,,old id not found,kick all player,0x%X", pHI->nOldID);
				}
				else
				{
					// 说明是网络临时中断而已
					strcpy(szReConnect, ",RECONNECT");
				}
			}
			else
			{
				// 看对应的group中的CLS4Web数量是否已经够多
				if (m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size() >= m_cfginfo.anMaxCLS4WebNumInCAAFS4WebGroup[pHI->nGroupID])
				{
					// 这个group的CLS已经太多了
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1382,LPMS_RT)"CLS4Web_LP_HI_T,,too many CLS4Web,%d,%d", pHI->nGroupID, m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.size());
					// 发送失败HI
					HI.nRst				= LP_CLS4Web_HI_T::RST_ERR_TOOMANY_CLS4Web;
					SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
					return 0;
				}
				// 创建CLS4Web对象
				int		nID				= m_CLS4Webs.AllocUnit(pCLS4WebUnit);
				if (nID < 0)
				{
					//assert(0);
					HI.nRst		= LP_CLS4Web_HI_T::RST_ERR_MEMERR;
					SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
					return 0;
				}
				// 清空
				pCLS4WebUnit->clear();
				pCLS4WebUnit->nCntrID	= pCDCmd->nConnecterID;
				pCLS4WebUnit->nID		= nID;
				pCLS4WebUnit->nGroupID	= pHI->nGroupID;
				pCLS4WebUnit->nMaxPlayer	= m_cfginfo.anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup[pCLS4WebUnit->nGroupID];
				// 加入CAAFS4Web/CLS4Web的group
				m_aCAAFS4WebGroup[pHI->nGroupID].dlCLS4Web.AddToTail(&pCLS4WebUnit->dlnode);
				// 先假定可以正式开始工作了
				pCLS4WebUnit->nStatus	= CLS4WebUnit::STATUS_WORKING;
				// 发送HI
				HI.nCLS4WebID			= nID;
				HI.nLPMaxPlayer			= m_cfginfo.nMaxPlayer;
				HI.nCLS4WebMaxConnection= pCLS4WebUnit->nMaxPlayer;
				SendCmdToConnecter(pCDCmd->nConnecterID, &HI, sizeof(HI));
			}
			// 设置appinfo
			AppInfo_T	appinfo;
			appinfo.nAppType		= APPTYPE_CLS4WEB;
			appinfo.pAppUnit		= pCLS4WebUnit;
			m_mapCntrID2AppInfo.insert(map<int, AppInfo_T>::value_type(pCDCmd->nConnecterID, appinfo));
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1386,LPMS_RT)"HI from CLS4Web,0x%X,%s%s", pCDCmd->nConnecterID, cmn_get_IP(pCDCmd->IP), szReConnect);
			if (szReConnect[0])
			{
				// 让CLS4Web告诉自己是否所有用户离线
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
				// 加入CAAFS4WebGroup
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
			assert(pPlayer != NULL);	// 必须的
			// 收到这个消息就说明一定成功了
			// 玩家状态改变
			pPlayer->nStatus		= PlayerUnit::STATUS_GOINGTOCLS4Web;
			// 玩家对应的CLS4Web应该一定是发出这个指令的CLS4Web
			assert(pCLS4Web->nID == pPlayer->nCLS4WebID);
			// 找到CAAFS4Web对象
			CAAFS4WebGroup*	pGroup	= GetCAAFS4WebByGroupID(pCLS4Web->nGroupID);
			if (pGroup!=NULL && pGroup->nCntrID!=0)
			{
				// 发给CAAFS4Web
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
				// CAAFS4Web不行了
				//RemovePlayerUnit(pPlayer);//CLS4Web会发起移除,这边就不用了(connect to me timeout)
				GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"CAAFS4Web Lost,0x%X,0x%X,%d", pPlayer->nID, pPlayer->nClientIDInCAAFS4Web, pCLS4Web->nGroupID);
			}
		}
		break;
	case CLS4Web_LP_CLIENTTOCLS4Web_RST:
		{
			CLS4Web_LP_CLIENTTOCLS4Web_RST_T*	pClientToCLS4WebRst	= (CLS4Web_LP_CLIENTTOCLS4Web_RST_T*)pCmd;
			if (pClientToCLS4WebRst->nRst == CLS4Web_LP_CLIENTTOCLS4Web_RST_T::RST_OK)
			{
				// 成功
				PlayerUnit*	pPlayer		= m_Players.getptr(pClientToCLS4WebRst->nClientID);
				assert(pPlayer != NULL);
				// 玩家改变状态
				pPlayer->nStatus		= PlayerUnit::STATUS_GAMING;
				// 现在玩家已经和CAAFS4Web无关了
				// 通知GamePlay
				LPPACKET_2GP_PLAYERONLINE_T	CmdPlayerOnline;
				CmdPlayerOnline.nClientID	= pPlayer->nID;
				CmdPlayerOnline.IP			= pPlayer->IP;
				CmdPlayerOnline.nTermType	= pPlayer->nTermType;
				CmdOutToLogic_AUTO(m_pLogicGamePlay, LPPACKET_2GP_PLAYERONLINE, &CmdPlayerOnline, sizeof(CmdPlayerOnline));
			}
			else
			{
				// 失败,删除玩家
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
