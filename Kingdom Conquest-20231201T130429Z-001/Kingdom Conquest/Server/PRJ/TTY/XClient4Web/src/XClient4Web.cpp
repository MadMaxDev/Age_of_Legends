//////////////////////////////////////////////////////////////////////////
// 指令流程
// 1.connect
// 2.login
// 3.character_select
// 4.startgame
// ...
// n.exit(暂时不要使用logout,disconnect指令,有问题)
//////////////////////////////////////////////////////////////////////////
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whdaemon.h"
#include "WHCMN/inc/whcmd.h"
#include "WHCMN/inc/whdbgmem.h"
#include "WHCMN/inc/whvector.h"
#include "WHCMN/inc/whdir.h"
#include "WHCMN/inc/whsignal.h"

#include "WHNET/inc/whnetepoll_client.h"

#include "PNGS/inc/pngs_def.h"

#include "../../Common/inc/pngs_packet_web_extension.h"
#include "../../Common/inc/tty_lp_client_packet.h"
#include "../../Common/inc/tty_lp_client_gamecmd_packet.h"
#include "../../Common/inc/tty_common_def.h"
#include "../../Common/inc/tty_common_PlayerData4Web.h"
#include "../../Common/inc/tty_game_def.h"
#include "../../Common/inc/tty_def_gs4web.h"

#include "../inc/StrCmdDealer.h"

using namespace n_whcmn;
using namespace n_whnet;
using namespace n_pngs;

struct XClient_CFG_INFO : public whdataini::obj
{
	char			szCharSet[64];

	XClient_CFG_INFO()
	{
		WH_STRNCPY0(szCharSet, "GBK");
	}

	WHDATAPROP_DECLARE_MAP(XClient_CFG_INFO)
};
WHDATAPROP_MAP_BEGIN_AT_ROOT(XClient_CFG_INFO)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szCharSet, 0)
WHDATAPROP_MAP_END()

// 连接器连接的是谁
enum
{
	peertype_caafs4web	= 1,
	peertype_cls4web	= 2,
};
// 全局变量
bool	g_bStop		= false;
int		g_nPeerType	= peertype_caafs4web;
epoll_connecter_client::info_T	g_epollClientInfo;
epoll_connecter_client*	g_pEpollConnecterClient	= NULL;
CStrCmdDealer*			g_pStrCmdDealer			= NULL;
const char*	g_cszFileDir	= "file_cache";
XClient_CFG_INFO		g_clientCfgInfo;
int		g_nTermType	= TERM_TYPE_NONE;

// 连接数据,缓存用
struct connect2cls_info_T 
{
	char	szIP[WHNET_MAXIPSTRLEN];
	port_t	nPort;
	int		nClientID;
	int		nPassword;

	connect2cls_info_T()
	{
		WHMEMSET0THIS();
	}
}		g_connect2clsInfo;

// 连接数据,缓存用
struct connect2caafs_info_T 
{
	char	szIP[WHNET_MAXIPSTRLEN];
	port_t	nPort;

	connect2caafs_info_T()
	{
		WHMEMSET0THIS();
	}
}		g_connect2caafsInfo;

// 全局函数
inline	const char*	GetPeer()
{
	const char*	pPeer	= NULL;
	switch (g_nPeerType)
	{
	case peertype_caafs4web:
		{
			pPeer		= "caafs4web";
		}
		break;
	case peertype_cls4web:
		{
			pPeer		= "cls4web";
		}
		break;
	default:
		{
			pPeer		= "none";
		}
		break;
	}
	return pPeer;
}
int		dealnccmd(const char* cszBuf, size_t nSize)
{
	char	cmd[256]	= "";
	char	param[1024]	= "";
	wh_strsplit("sa", cszBuf, "", cmd, param);
	g_pStrCmdDealer->m_srad.CallFunc(cmd, param);
	if (g_pStrCmdDealer->m_srad.m_bShouldHelp)
	{
		g_pStrCmdDealer->_i_srad_help(cmd);
	}
	return 0;
}
int		deal_cmd_CLS4Web_CLIENT_DATA(pngs_cmd_t* pCmd, size_t nSize);
int		write_file(const char* cszFileName, const char* pData, int nSize)
{
	if (nSize <= 0)
	{
		return -1;
	}

	FILE*	fp	= fopen(cszFileName, "wb");
	if (fp == NULL)
	{
		return -2;
	}
	fwrite(pData, 1, nSize, fp);
	fclose(fp);
	return 0;
}

// void	stopfunc(int sig)
// {
// 	switch(sig)
// 	{
// 	case	SIGINT:
// 	case	SIGTERM:
// 		printf("use \"exit\" cmd to stop!!!%s", WHLINEEND);
// 		break;
// 	}
// }

// 战斗计算
// 群战单次攻防计算
int		GC_CombatCalc_Raid_Round(CombatUnit *pA, CombatUnit *pD)
{
	int nHealthA = pA->nHealth;
	int nHealthD = pD->nHealth;

	// 有兵种克制吗
	float nMA = 1;
	float nMD = 1;

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
	return 0;
}
// 群战
int		GC_CombatCalc_Raid(STC_GAMECMD_OPERATE_GET_COMBAT_T *pData)
{
	CombatUnit aA[COMBATFIELD_CHANNEL], aD[COMBATFIELD_CHANNEL];
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
		GC_CombatCalc_Raid_Round(&aA[i], &aD[i]);
	}
	// 遭遇战后，以剩余部队战力多少决定战斗胜负
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
		// 攻防胜利
		return 1;
	}
	// 守方胜利
	return 2;
}

int		main(int argc, char* argv[])
{
	char	cszCFG[256]		= "client4web_cfg.txt";
	whdir_SetCWDToExePath();
	if (argc == 2)
	{
		strcpy(cszCFG, argv[1]);
	}
	// 设置结束信号处理器
//	whsingal_set_exitfunc(stopfunc);

	whdir_SureMakeDir(g_cszFileDir);

	// 网络初始化
	cmn_ir_obj	netIR;

	// epoll连接器生成
	g_pEpollConnecterClient		= new epoll_connecter_client();

	// 配置文件分析
	WHDATAINI_CMN	ini;
	ini.addobj("EPOLL_CLIENT", g_pEpollConnecterClient->GetInfo());
	ini.addobj("XClient_CFG_INFO", &g_clientCfgInfo);
	
	int	nRst	= ini.analyzefile(cszCFG);
	if (nRst < 0)
	{
		printf("Can not analyze cfgfile:%s rst:%d %s%s", cszCFG, nRst, ini.printerrreport(), WHLINEEND);
		return -1;
	}

	WH_STRNCPY0(g_connect2caafsInfo.szIP, g_pEpollConnecterClient->GetInfo()->szIP);
	g_connect2caafsInfo.nPort	= g_pEpollConnecterClient->GetInfo()->iPort;

	// 设置字符集
	WHCMN_setcharset(g_clientCfgInfo.szCharSet);

	// 命令处理器初始化
	g_pStrCmdDealer		= new CStrCmdDealer;

	// 控制台线程
	whcmn_nml_console	nc;
	nc.StartThread();

	// 开始工作
	whvector<SOCKET>	vectSock;
	while (!g_bStop)
	{
		vectSock.clear();
		g_pEpollConnecterClient->GetSockets(vectSock);
		cmn_select_rd_array(vectSock.getbuf(), vectSock.size(), 10);

		// 指令处理
		//////////////////////////////////////////////////////////////////////////
		// console的指令
		//////////////////////////////////////////////////////////////////////////
		char	szBuf[1024];
		size_t	nSize	= sizeof(szBuf)-1;
		while (nc.m_cmdqueue.Out(szBuf, &nSize) == 0)
		{
			nc.m_cmdqueue.lock();
			// 处理指令
			if (dealnccmd(szBuf, nSize) < 0)
			{
				printf("deal cmd error:%s%s", szBuf, WHLINEEND);
				nc.m_cmdqueue.unlock();
				break;
			}
			nc.m_cmdqueue.unlock();
			// 下一条
			nSize	= sizeof(szBuf)-1;
		}
		//////////////////////////////////////////////////////////////////////////
		// 网络来的
		//////////////////////////////////////////////////////////////////////////
		g_pEpollConnecterClient->Work();
		// 1.控制指令
		epoll_connecter_client::ctrl_out_T*	pCtrlOut	= NULL;
		while (g_pEpollConnecterClient->ControlOut(&pCtrlOut, &nSize) == 0)
		{
			switch (pCtrlOut->iCmd)
			{
			case epoll_connecter_client::ctrlout_working:
				{
					printf("connect %s ok%s", GetPeer(), WHLINEEND);
				}
				break;
			case epoll_connecter_client::ctrlout_closed:
				{
					printf("connect closed,param:%d,%s%s", pCtrlOut->iParam, GetPeer(), WHLINEEND);
					if (g_nPeerType == peertype_caafs4web)
					{
						// 现在对方是cls4web了
						g_nPeerType	= peertype_cls4web;
						// 设置连接附加数据
						CLIENT_CLS4Web_EXTINFO_T	extInfo;
						extInfo.nClientID			= g_connect2clsInfo.nClientID;
						extInfo.nPassword			= g_connect2clsInfo.nPassword;
						g_pEpollConnecterClient->SetExtData(&extInfo, sizeof(extInfo));
						g_pEpollConnecterClient->m_bNeedCode	= true;
						// 连接cls4web
						epoll_connecter_client::info_T* pInfo	= g_pEpollConnecterClient->GetInfo();
						WH_STRNCPY0(pInfo->szIP, g_connect2clsInfo.szIP);
						pInfo->iPort	= g_connect2clsInfo.nPort;
						if ((nRst=g_pEpollConnecterClient->Init(pInfo)) < 0)
						{
							printf("epoll_connecter_client::Init,%d,%s:%d%s", nRst, pInfo->szIP, pInfo->iPort, WHLINEEND);
							continue;
						}
						printf("connect to cls4web connectID:0x%X,password:%d%s", g_connect2clsInfo.nClientID, g_connect2clsInfo.nPassword, WHLINEEND);
					}
					else if (g_nPeerType == peertype_cls4web)
					{
						g_nPeerType	= peertype_caafs4web;
						g_pEpollConnecterClient->SetExtData(NULL ,0);
						epoll_connecter_client::info_T* pInfo	= g_pEpollConnecterClient->GetInfo();
						WH_STRNCPY0(pInfo->szIP, g_connect2caafsInfo.szIP);
						pInfo->iPort	= g_connect2caafsInfo.nPort;
						g_pEpollConnecterClient->m_bNeedCode	= false;
					}
				}
				break;
			case epoll_connecter_client::ctrlout_error:
				{
					printf("connect closed,error,param:%d,%s%s", pCtrlOut->iParam, GetPeer(), WHLINEEND);
				}
				break;
			}
		}
		// 2.消息处理
		pngs_cmd_t*	pCmdRecv	= NULL;
		while ((pCmdRecv=(pngs_cmd_t*)g_pEpollConnecterClient->PeekMsg(&nSize)) != NULL)
		{
			switch (*pCmdRecv)
			{
			case CAAFS4Web_CLIENT_CAAFS4WebINFO:
				{
					CAAFS4Web_CLIENT_CAAFS4WebINFO_T*	pInfo	= (CAAFS4Web_CLIENT_CAAFS4WebINFO_T*)pCmdRecv;
					printf("caafs4web info:version:%s,", pInfo->szVer);
					printf("there is %d players before you%s", pInfo->nQueueSize, WHLINEEND);
				}
				break;
			case CAAFS4Web_CLIENT_GOTOCLS4Web:
				{
					CAAFS4Web_CLIENT_GOTOCLS4Web_T*	pGTC		= (CAAFS4Web_CLIENT_GOTOCLS4Web_T*)pCmdRecv;
					if (pGTC->IP == 0)
					{
						pGTC->IP	= cmn_get_S_addr(g_connect2caafsInfo.szIP);
					}
					cmn_get_IP(pGTC->IP, g_connect2clsInfo.szIP);
					g_connect2clsInfo.nPort			= pGTC->nPort;
					g_connect2clsInfo.nClientID		= pGTC->nClientID;
					g_connect2clsInfo.nPassword		= pGTC->nPassword;
					g_pEpollConnecterClient->Close();
				}
				break;
			case CLS4Web_CLIENT_DATA:
				{
					CLS4Web_CLIENT_DATA_T*	pCCD	= (CLS4Web_CLIENT_DATA_T*)pCmdRecv;
					deal_cmd_CLS4Web_CLIENT_DATA((pngs_cmd_t*)pCCD->data, nSize-sizeof(pngs_cmd_t));
				}
				break;
			}
			// 释放已经处理过的消息
			g_pEpollConnecterClient->FreeMsg();
		}
	}
	
	// 网络结束
	g_pEpollConnecterClient->Close();
	epoll_connecter_client::ctrl_out_T*	pCtrlOut	= NULL;
	size_t	nSize	= 0;
	bool	bClosed	= false;
	switch (g_pEpollConnecterClient->GetStatus())
	{
	case epoll_connecter_client::status_nothing:
	case epoll_connecter_client::status_error:
		{
			printf("close ok:already closed%s", WHLINEEND);
			bClosed		= true;
		}
		break;
	}
	whtick_t	t	= wh_gettickcount();
	while (!bClosed)
	{
		g_pEpollConnecterClient->Work();
		// 看看是否关闭OK
		while (g_pEpollConnecterClient->ControlOut(&pCtrlOut, &nSize) == 0)
		{
			switch (pCtrlOut->iCmd)
			{
			case epoll_connecter_client::ctrlout_closed:
				{
					printf("close ok:%d,%s%s", pCtrlOut->iParam, GetPeer(), WHLINEEND);
				}
				break;
			case epoll_connecter_client::ctrlout_error:
				{
					printf("close error:%d,%s%s", pCtrlOut->iParam, GetPeer(), WHLINEEND);
				}
				break;
			}
			bClosed	= true;
		}
		if (wh_tickcount_diff(wh_gettickcount(), t) > 10*1000)
		{
			printf("epoll_client_connecter close timeout%s", WHLINEEND);
			bClosed	= true;
		}
	}
	g_pEpollConnecterClient->Release();
	WHSafeDelete(g_pEpollConnecterClient);

	// 等待console线程结束
	nc.WaitThreadEnd(2*1000);

	return 0;
}
int		deal_cmd_CLS4Web_CLIENT_DATA(pngs_cmd_t* pCmd, size_t nSize)
{
	switch (*pCmd)
	{
	case TTY_LPGAMEPLAY_CLIENT_LOGIN_RST:
		{
			TTY_LPGAMEPLAY_CLIENT_LOGIN_RST_T*	pRst	= (TTY_LPGAMEPLAY_CLIENT_LOGIN_RST_T*)pCmd;
			if (pRst->nRst!=TTY_LOGIN_RST_SUCCEED && pRst->nRst!=TTY_LOGIN_RST_NOCHAR)
			{
				printf("login error!rst:%d,name:%s,pass:%s,device_id:%s%s", pRst->nRst, pRst->szAccount, pRst->szPass, pRst->szDeviceID, WHLINEEND);
			}
			else
			{
				printf("login ok!rst:%d,name:%s,pass:%s,device_id:%s%s", pRst->nRst, pRst->szAccount, pRst->szPass, pRst->szDeviceID, WHLINEEND);
			}
		}
		break;
	case TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST:
		{
			TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST_T*	pRst	= (TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST_T*)pCmd;
			if (pRst->nRst != TTY_ACCOUNT_CREATE_RST_OK)
			{
				printf("create account error:%d!%s", pRst->nRst, WHLINEEND);
			}
			else
			{
				printf("create account ok!%s%s", pRst->szAccount, WHLINEEND);
			}
		}
		break;
	case TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST:
		{
			TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST_T*	pRst	= (TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST_T*)pCmd;
			if (pRst->nRst != TTY_CHAR_CREATE_RST_OK)
			{
				printf("create char error:%d!%s", pRst->nRst, WHLINEEND);
			}
			else
			{
				printf("create char ok!%s", WHLINEEND);
			}
		}
		break;
	case TTY_LPGAMEPLAY_CLIENT_GAME_CMD:
		{
			TTY_LPGAMEPLAY_CLIENT_GAME_CMD_T*	pGameCmd	= (TTY_LPGAMEPLAY_CLIENT_GAME_CMD_T*)pCmd;
			switch (pGameCmd->nGameCmd)
			{
			case STC_GAMECMD_GET_NOTIFICATION:
				{
					STC_GAMECMD_GET_NOTIFICATION_T*	pRst	= (STC_GAMECMD_GET_NOTIFICATION_T*)pGameCmd;
					printf("notify_get:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					Web_Notification_T*	pNotification		= (Web_Notification_T*)wh_getptrnexttoptr(pRst);
					char*	pTitle							= NULL;
					int*	pContentLen						= NULL;
					char*	pContent						= NULL;
					for (int i=0; i<pRst->nNum; i++)
					{
						pTitle				= (char*)wh_getptrnexttoptr(pNotification);
						pContentLen			= (int*)wh_getoffsetaddr(pTitle, pNotification->nTitleLen);
						pContent			= (char*)wh_getptrnexttoptr(pContentLen);
						printf("id:%d,group_id:%d,time:%s,title:%s,content:%s%s", pNotification->nNotificationID, pNotification->nGroupID, wh_gettimestr(pNotification->nTime), pTitle, pContent, WHLINEEND);
						pNotification		= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
					}
				}
				break;
			case STC_GAMECMD_GET_PLAYERCARD:
				{
					STC_GAMECMD_GET_PLAYERCARD_T*	pCard	= (STC_GAMECMD_GET_PLAYERCARD_T*)pGameCmd;
					if (pCard->nRst != STC_GAMECMD_GET_PLAYERCARD_T::RST_OK)
					{
						printf("get_playercard:ERROR,%d%s", (int)pCard->nRst, WHLINEEND);
					}
					else
					{
						char	szBuf[1024]	= "";
						printf("get_playercard:OK%s", WHLINEEND);
						printf("-------------------------------%s", WHLINEEND);
						printf("account_id:0x%"WHINT64PRFX"X%s", pCard->card.nAccountID, WHLINEEND);
						wh_utf82char(pCard->card.szName, strlen(pCard->card.szName), szBuf, sizeof(szBuf));
						printf("name:%s%s", szBuf, WHLINEEND);
						printf("level:%u%s", pCard->card.nLevel, WHLINEEND);
						printf("vip:%u%s", pCard->card.nVip, WHLINEEND);
						printf("sex:%u%s", pCard->card.nSex, WHLINEEND);
						printf("head_id:%u%s", pCard->card.nHeadID, WHLINEEND);
						printf("position:%u,%u%s", pCard->card.nPosX, pCard->card.nPosY, WHLINEEND);
						printf("alliance_id:0x%"WHINT64PRFX"X%s", pCard->card.nAllianceID, WHLINEEND);
						wh_utf82char(pCard->card.szAllianceName, strlen(pCard->card.szAllianceName), szBuf, sizeof(szBuf));
						printf("alliance_name:%s%s", szBuf, WHLINEEND);
						printf("signature:%s%s", pCard->card.szSignature, WHLINEEND);
						printf("level_rank:%d%s", pCard->card.nLevelRank, WHLINEEND);
						printf("city_level:%d%s", pCard->card.nCityLevel, WHLINEEND);
						printf("protect_time:%d%s", pCard->card.nProtectTime, WHLINEEND);
						printf("instance_wangzhe:%d%s", pCard->card.nInstanceWangzhe, WHLINEEND);
						printf("instance_zhengzhan:%d%s", pCard->card.nInstanceZhengzhan, WHLINEEND);
						printf("-------------------------------%s", WHLINEEND);
					}
				}
				break;
			case STC_GAMECMD_GET_TILE_INFO:
				{
					STC_GAMECMD_GET_TILE_INFO_T*	pTile	= (STC_GAMECMD_GET_TILE_INFO_T*)pGameCmd;
					printf("tile:%s,position:(%u,%u),account_id:0x%"WHINT64PRFX"X,alliance_name:%s,alliance_id:0x%"WHINT64PRFX"X%s", 
						pTile->szName, pTile->nPosX, pTile->nPosY, pTile->nAccountID, pTile->szAllianceName, pTile->nAllianceID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_TILE_NONE:
				{
					STC_GAMECMD_GET_TILE_NONE_T*	pTile	= (STC_GAMECMD_GET_TILE_NONE_T*)pGameCmd;
					printf("tile:none,position:(%u,%u)%s", pTile->nPosX, pTile->nPosY, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_BUILDING_LIST:
				{
					STC_GAMECMD_GET_BUILDING_LIST_T*	pGetBuilding	= (STC_GAMECMD_GET_BUILDING_LIST_T*)pGameCmd;
					printf("get_building_list:%d,%d%s", (int)pGetBuilding->nRst, pGetBuilding->nNum, WHLINEEND);
					BuildingUnit*	pUnit	= (BuildingUnit*)wh_getptrnexttoptr(pGetBuilding);
					for (int i=0; i<pGetBuilding->nNum; i++)
					{
						printf("auto_id:%d,excel_id:%d,level:%d%s", pUnit->nAutoID, pUnit->nExcelID, pUnit->nLevel, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_BUILDING_TE_LIST:
				{
					STC_GAMECMD_GET_BUILDING_TE_LIST_T*	pGetBuildingTE	= (STC_GAMECMD_GET_BUILDING_TE_LIST_T*)pGameCmd;
					printf("get_building_te_list:%d,num:%d,time:%s%s", (int)pGetBuildingTE->nRst, pGetBuildingTE->nNum, wh_gettimestr(pGetBuildingTE->nTimeNow), WHLINEEND);
					BuildingTEUnit*	pUnit	= (BuildingTEUnit*)wh_getptrnexttoptr(pGetBuildingTE);
					for (int i=0; i<pGetBuildingTE->nNum; i++)
					{
						char	szBuf[256];
						char	szBuf1[256];
						printf("auto_id:%d,excel_id:%d,type:%d,begin_time:%s,end_time:%s%s", pUnit->nAutoID, pUnit->nExcelID, pUnit->nType, wh_gettimestr(pUnit->nBeginTime, szBuf), wh_gettimestr(pUnit->nEndTime, szBuf1), WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_BUILD_BUILDING:
				{
					STC_GAMECMD_OPERATE_BUILD_BUILDING_T*	pBuildBuilding	= (STC_GAMECMD_OPERATE_BUILD_BUILDING_T*)pGameCmd;
					printf("build_building:%d,auto_id:%d,excel_id:%d,added_char_exp:%d,added_char_dev:%d,cur_char_exp:%d,cur_char_level:%d%s"
						, (int)pBuildBuilding->nRst, pBuildBuilding->nAutoID, pBuildBuilding->nExcelID, pBuildBuilding->nAddedCharExp, pBuildBuilding->nAddedCharDev
						, pBuildBuilding->nCurCharExp, pBuildBuilding->nCurCharLevel, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_UPGRADE_BUILDING:
				{
					STC_GAMECMD_OPERATE_UPGRADE_BUILDING_T*	pUpgradeBuilding	= (STC_GAMECMD_OPERATE_UPGRADE_BUILDING_T*)pGameCmd;
					printf("upgrade_building:%d,auto_id:%d,excel_id:%d,added_char_exp:%d,added_char_dev:%d,cur_char_exp:%d,cur_char_level:%d%s"
						, (int)pUpgradeBuilding->nRst, pUpgradeBuilding->nAutoID, pUpgradeBuilding->nExcelID, pUpgradeBuilding->nAddedCharExp, pUpgradeBuilding->nAddedCharDev
						, pUpgradeBuilding->nCurCharExp, pUpgradeBuilding->nCurCharLevel, WHLINEEND);
				}
				break;
			case STC_GAMECMD_POPULATION_ADD:
				{
					STC_GAMECMD_POPULATION_ADD_T*	pPop	= (STC_GAMECMD_POPULATION_ADD_T*)pGameCmd;
					printf("add_population:%d%s", pPop->nProduction, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_PRODUCE_GOLD:
				{
					STC_GAMECMD_OPERATE_PRODUCE_GOLD_T*	pProduce	= (STC_GAMECMD_OPERATE_PRODUCE_GOLD_T*)pGameCmd;
					printf("produce_gold:%d,auto_id:%d,production:%d%s", (int)pProduce->nRst, pProduce->nAutoID, pProduce->nProduction, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_FETCH_GOLD:
				{
					STC_GAMECMD_OPERATE_FETCH_GOLD_T*	pFetch	= (STC_GAMECMD_OPERATE_FETCH_GOLD_T*)pGameCmd;
					printf("fetch_gold:%d,auto_id:%d,production:%d%s", (int)pFetch->nRst, pFetch->nAutoID, pFetch->nProduction, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GOLD_CAN_FETCH:
				{
					STC_GAMECMD_GOLD_CAN_FETCH_T*	pFetch	= (STC_GAMECMD_GOLD_CAN_FETCH_T*)pGameCmd;
					printf("gold_can_fetch:auto_id:%d,production:%d%s", pFetch->nAutoID, pFetch->nProduction, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_PRODUCTION_EVENT:
				{
					STC_GAMECMD_GET_PRODUCTION_EVENT_T*	pGet	= (STC_GAMECMD_GET_PRODUCTION_EVENT_T*)pGameCmd;
					printf("get_production:%d,num:%d,account_id:0x%"WHINT64PRFX"X%s", (int)pGet->nRst, pGet->nNum, pGet->nAccountID, WHLINEEND);
					ProductionEvent*	pEvent	= (ProductionEvent*)wh_getptrnexttoptr(pGet);
					for (int i=0; i<pGet->nNum; i++)
					{
						printf("event:auto_id:%d,production:%d,type:%d%s", pEvent->nAutoID, pEvent->nProduction, pEvent->nType, WHLINEEND);
						pEvent++;
					}
				}
				break;
			case STC_GAMECMD_GET_PRODUCTION_TE_LIST:
				{
					STC_GAMECMD_GET_PRODUCTION_TE_LIST_T*	pGet	= (STC_GAMECMD_GET_PRODUCTION_TE_LIST_T*)pGameCmd;
					printf("get_production_te:%d,%d,0x%"WHINT64PRFX"X,time_now:%s%s", (int)pGet->nRst, pGet->nNum, pGet->nAccountID, wh_gettimestr(pGet->nTimeNow), WHLINEEND);
					ProductionTimeEvent*	pEvent	= (ProductionTimeEvent*)wh_getptrnexttoptr(pGet);
					char	szBuf0[256];
					char	szBuf1[256];
					for (int i=0; i<pGet->nNum; i++)
					{
						printf("event:auto_id:%d,production:%d,type:%d,time_begin:%s,time_end:%s%s", pEvent->nAutoID, pEvent->nProduction, pEvent->nType, wh_gettimestr(pEvent->nBeginTime, szBuf0), wh_gettimestr(pEvent->nEndTime, szBuf1), WHLINEEND);
						pEvent++;
					}
				}
				break;
			case STC_GAMECMD_BUILDING_TE:
				{
					STC_GAMECMD_BUILDING_TE_T*	pTE	= (STC_GAMECMD_BUILDING_TE_T*)pGameCmd;
					printf("building_te_ok:auto_id:%d,excel_id:%d,type:%d%s", pTE->nAutoID, pTE->nExcelID, pTE->nType, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE:
				{
					STC_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE_T*	pAcce	= (STC_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE_T*)pGameCmd;
					printf("accelerate_gold:%d,excel_id:%d,auto_id:%d,money_num:%d,money_type:%d,time:%d%s", (int)pAcce->nRst, pAcce->nExcelID, pAcce->nAutoID, pAcce->nMoneyNum, pAcce->nMoneyType, pAcce->nTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ACCE_BUILDING:
				{
					STC_GAMECMD_OPERATE_ACCE_BUILDING_T*	pAcce	= (STC_GAMECMD_OPERATE_ACCE_BUILDING_T*)pGameCmd;
					printf("accelerate_building:%d,excel_id:%d,auto_id:%d,money_num:%d,money_type:%d,time:%d,type:%d%s", (int)pAcce->nRst,  pAcce->nExcelID, pAcce->nAutoID, pAcce->nMoneyNum, pAcce->nMoneyType, pAcce->nTime, pAcce->nType, WHLINEEND);
				}
				break;
			case STC_GAMECMD_RESEARCH_TE:
				{
					STC_GAMECMD_RESEARCH_TE_T*	pRes	= (STC_GAMECMD_RESEARCH_TE_T*)pGameCmd;
					printf("research_notify:%d%s", pRes->nExcelID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ACCE_RESEARCH:
				{
					STC_GAMECMD_OPERATE_ACCE_RESEARCH_T*	pAcce	= (STC_GAMECMD_OPERATE_ACCE_RESEARCH_T*)pGameCmd;
					printf("accelerate_research:%d,excel_id:%d,money_num:%d,money_type:%d,time:%d%s", (int)pAcce->nRst, pAcce->nExcelID, pAcce->nMoneyNum, pAcce->nMoneyType, pAcce->nTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_TECHNOLOGY:
				{
					STC_GAMECMD_GET_TECHNOLOGY_T*	pGet	= (STC_GAMECMD_GET_TECHNOLOGY_T*)pGameCmd;
					printf("get_technology:%d,%d%s", (int)pGet->nRst, pGet->nNum, WHLINEEND);
					TechnologyUnit*	pUnit	= (TechnologyUnit*)wh_getptrnexttoptr(pGet);
					for (int i=0; i<pGet->nNum; i++)
					{
						printf("unit:excel_id:%d,level:%d%s", pUnit->nExcelID, pUnit->nLevel, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_RESEARCH_TE:
				{
					STC_GAMECMD_GET_RESEARCH_TE_T*	pGet	= (STC_GAMECMD_GET_RESEARCH_TE_T*)pGameCmd;
					printf("get_research_te:%d,%d,time_now:%s%s", (int)pGet->nRst, pGet->nNum, wh_gettimestr(pGet->nTimeNow), WHLINEEND);
					ResearchTimeEvent*	pEvent	= (ResearchTimeEvent*)wh_getptrnexttoptr(pGet);
					char	szBuf0[256];
					char	szBuf1[256];
					for (int i=0; i<pGet->nNum; i++)
					{
						printf("event:excel_id:%d,time_begin:%s,time_end:%s%s", pEvent->nExcelID, wh_gettimestr(pEvent->nBeginTime, szBuf0), wh_gettimestr(pEvent->nEndTime, szBuf1), WHLINEEND);
						pEvent++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_RESEARCH:
				{
					STC_GAMECMD_OPERATE_RESEARCH_T*	pRes	= (STC_GAMECMD_OPERATE_RESEARCH_T*)pGameCmd;
					printf("research:%d,excel_id:%d,gold:%d,level:%d,time:%d%s", (int)pRes->nRst, pRes->nExcelID, pRes->nGold, pRes->nLevel, pRes->nTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER:
				{
					STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T*	pConscript	= (STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T*)pGameCmd;
					printf("conscript_soldier:%d,excel_id:%d,level:%d,num:%d,gold:%d%s", (int)pConscript->nRst, pConscript->nExcelID, pConscript->nLevel, pConscript->nNum, pConscript->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_UPGRADE_SOLDIER:
				{
					STC_GAMECMD_OPERATE_UPGRADE_SOLDIER_T*	pUpgrade	= (STC_GAMECMD_OPERATE_UPGRADE_SOLDIER_T*)pGameCmd;
					printf("conscript_soldier:%d,excel_id:%d,from_level:%d,to_level:%d,num:%d,gold:%d%s", (int)pUpgrade->nRst, pUpgrade->nExcelID, pUpgrade->nFromLevel, pUpgrade->nToLevel, pUpgrade->nNum, pUpgrade->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_SOLDIER:
				{
					STC_GAMECMD_GET_SOLDIER_T*	pGet	= (STC_GAMECMD_GET_SOLDIER_T*)pGameCmd;
					printf("get_soldier:%d,%d%s", (int)pGet->nRst, pGet->nNum, WHLINEEND);
					SoldierUnit*	pUnit	= (SoldierUnit*)wh_getptrnexttoptr(pGet);
					for (int i=0; i<pGet->nNum; i++)
					{
						printf("unit:excel_id:%d,level:%d,num:%d%s", pUnit->nExcelID, pUnit->nLevel, pUnit->nNum, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER:
				{
					STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T*	pRst	= (STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T*)pGameCmd;
					printf("accept_alliance_member:rst:%d,member_id:0x%"WHINT64PRFX"X%s", (int)pRst->nRst, pRst->nMemberID, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING:
				{
					STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T*	pRst	= (STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T*)pGameCmd;
					printf("build_alli_building:rst:%d,excel_id:%d,type:%d%s", (int)pRst->nRst, pRst->nExcelID, pRst->nType, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI:
				{
					STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T*	pRst	= (STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T*)pGameCmd;
					printf("cancel_join_alli:rst:%d%s", (int)pRst->nRst, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_OPERATE_CREATE_ALLI:
				{
					STC_GAMECMD_OPERATE_CREATE_ALLI_T*	pRst	= (STC_GAMECMD_OPERATE_CREATE_ALLI_T*)pGameCmd;
					printf("create_alli:rst:%d,alli_id:0x%"WHINT64PRFX"X%s", (int)pRst->nRst, pRst->nAllianceID, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_OPERATE_DISMISS_ALLI:
				{
					STC_GAMECMD_OPERATE_DISMISS_ALLI_T*	pRst	= (STC_GAMECMD_OPERATE_DISMISS_ALLI_T*)pGameCmd;
					printf("dismiss_alli:rst:%d%s", (int)pRst->nRst, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_OPERATE_EXIT_ALLI:
				{
					STC_GAMECMD_OPERATE_EXIT_ALLI_T*	pRst	= (STC_GAMECMD_OPERATE_EXIT_ALLI_T*)pGameCmd;
					printf("exit_alli:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER:
				{
					STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T*	pRst	= (STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T*)pGameCmd;
					printf("expel_alli_member:rst:%d,member_id:0x%"WHINT64PRFX"X%s", (int)pRst->nRst, pRst->nMemberID, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_OPERATE_JOIN_ALLI:
				{
					STC_GAMECMD_OPERATE_JOIN_ALLI_T*	pRst	= (STC_GAMECMD_OPERATE_JOIN_ALLI_T*)pGameCmd;
					printf("join_alli:rst:%d,alli_id:0x%"WHINT64PRFX"X%s", (int)pRst->nRst, pRst->nAllianceID, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_OPERATE_SET_ALLI_POSITION:
				{
					STC_GAMECMD_OPERATE_SET_ALLI_POSITION_T*	pRst	= (STC_GAMECMD_OPERATE_SET_ALLI_POSITION_T*)pGameCmd;
					printf("set_alli_position:rst:%d,member_id:0x%"WHINT64PRFX"X,position:%d%s", (int)pRst->nRst, pRst->nMemberID, pRst->nPosition, WHLINEEND);
				}
				break;
			case	STC_GAMECMD_GET_ALLIANCE_INFO:
				{
					STC_GAMECMD_GET_ALLIANCE_INFO_T*	pRst	= (STC_GAMECMD_GET_ALLIANCE_INFO_T*)pGameCmd;
					printf("get_alli:rst:%d%s", (int)pRst->nRst, WHLINEEND);
					if (pRst->nRst == STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_OK)
					{
						char	szBuf1[TTY_ALLIANCENAME_LEN];
						wh_utf82char(pRst->alliance.szAllianceName, strlen(pRst->alliance.szAllianceName), szBuf1, sizeof(szBuf1));
						char	szBuf2[TTY_CHARACTERNAME_LEN];
						wh_utf82char(pRst->alliance.szLeaderName, strlen(pRst->alliance.szLeaderName), szBuf2, sizeof(szBuf2));
						printf("alli_name:%s,alli_id:0x%"WHINT64PRFX"X,leader:%s"
							",leader_id:0x%"WHINT64PRFX"X,development:%d"
							",total_development:%d,introduction:%s,rank:%d"
							",level:%d,member_num:%d,max_member_num:%d,position:%d"
							",development:%d,total_development:%d%s"
							, szBuf1, pRst->alliance.nAllianceID, szBuf2, pRst->alliance.nLeaderID
							, pRst->alliance.nDevelopment, pRst->alliance.nTotalDevelopment, pRst->alliance.szIntroduction, pRst->alliance.nRank 
							, pRst->nLevel, pRst->nMemberNum, pRst->nMemberMaxNum, pRst->nMyPosition
							, pRst->nDevelopment, pRst->nTotalDevelopment, WHLINEEND);
					}
				}
				break;
			case	STC_GAMECMD_GET_ALLIANCE_MEMBER:
				{
					STC_GAMECMD_GET_ALLIANCE_MEMBER_T*	pRst	= (STC_GAMECMD_GET_ALLIANCE_MEMBER_T*)pGameCmd;
					printf("get_alli_member:rst:%d,num:%d%s", (int)pRst->nRst, pRst->nNum, WHLINEEND);
					AllianceMemberUnit*	pUnit	= (AllianceMemberUnit*)wh_getptrnexttoptr(pRst);
					for (unsigned int i=0; i<pRst->nNum; i++)
					{
						char	szBuf[TTY_CHARACTERNAME_LEN];
						wh_utf82char(pUnit->szName, strlen(pUnit->szName), szBuf, sizeof(szBuf));
						printf("name:%s,id:0x%"WHINT64PRFX"X,position:%d,dev:%d,total_dev:%d,level:%d,cup:%d%s"
							, szBuf, pUnit->nAccountID, pUnit->nPosition, pUnit->nDevelopment, pUnit->nTotalDevelopment, pUnit->nLevel, pUnit->nCup, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case	STC_GAMECMD_GET_ALLIANCE_BUILDING_TE:
				{
					STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T*	pRst	= (STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T*)pGameCmd;
					printf("get_alli_building_te:rst:%d,num:%d%s", (int)pRst->nRst, pRst->nNum, WHLINEEND);
					AllianceBuildingTimeEvent*	pUnit	= (AllianceBuildingTimeEvent*)wh_getptrnexttoptr(pRst);
					for (unsigned int i=0; i<pRst->nNum; i++)
					{
						char	szBuf1[64];
						char	szBuf2[64];
						printf("excel_id:%d,type:%d,begin_time:%s,end_time:%s%s", pUnit->nExcelID, pUnit->nType
							, wh_gettimestr(pUnit->nBeginTime, szBuf1), wh_gettimestr(pUnit->nEndTime, szBuf2), WHLINEEND);
						pUnit++;
					}
				}
				break;
			case	STC_GAMECMD_GET_ALLIANCE_BUILDING:
				{
					STC_GAMECMD_GET_ALLIANCE_BUILDING_T*	pRst	= (STC_GAMECMD_GET_ALLIANCE_BUILDING_T*)pGameCmd;
					printf("get_alli_building:rst:%d,num:%d%s", (int)pRst->nRst, pRst->nNum, WHLINEEND);
					AllianceBuildingUnit*	pUnit	= (AllianceBuildingUnit*)wh_getptrnexttoptr(pRst);
					for (unsigned int i=0; i<pRst->nNum; i++)
					{
						printf("excel_id:%d,level:%d%s", pUnit->nExcelID, pUnit->nLevel, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case	STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT:
				{
					STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T*	pRst	= (STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T*)pGameCmd;
					printf("get_alli_join_event:rst:%d,num:%d%s", (int)pRst->nRst, pRst->nNum, WHLINEEND);
					AllianceJoinEvent*	pUnit	= (AllianceJoinEvent*)wh_getptrnexttoptr(pRst);
					for (unsigned int i=0; i<pRst->nNum; i++)
					{
						char	szBuf[TTY_CHARACTERNAME_LEN];
						wh_utf82char(pUnit->szName, strlen(pUnit->szName), szBuf, sizeof(szBuf));
						printf("name:%s,id:0x%"WHINT64PRFX"X,level:%d,head_id:%d%s", szBuf, pUnit->nAccountID, pUnit->nLevel, pUnit->nHeadID, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_CHAR_ATB:
				{
					STC_GAMECMD_GET_CHAR_ATB_T*	pRst	= (STC_GAMECMD_GET_CHAR_ATB_T*)pGameCmd;
					CharAtb&	atb	= pRst->charAtb;
					printf("get_char_atb:%s", WHLINEEND);
					char	szBuf[1024];
					wh_utf82char(atb.szName, strlen(atb.szName), szBuf, sizeof(szBuf));
					printf("name:%s,id:0x%"WHINT64PRFX"X%s", szBuf, atb.nAccountID, WHLINEEND);
					wh_utf82char(atb.szAllianceName, strlen(atb.szAllianceName), szBuf, sizeof(szBuf));
					printf("alliance_name:%s,alliance_id:0x%"WHINT64PRFX"X%s", szBuf, atb.nAllianceID, WHLINEEND);
					printf("gold:%d,diamond:%d,crystal:%d,development:%d%s", atb.nGold, atb.nDiamond, atb.nCrystal, atb.nDevelopment, WHLINEEND);
					printf("pos_x:%d,pos_y:%d%s", atb.nPosX, atb.nPosY, WHLINEEND);
					printf("exp:%d,level:%d,head:%d,sex:%d,vip:%d%s", atb.nExp, atb.nLevel, atb.nHeadID, atb.nSex, atb.nVip, WHLINEEND);
					printf("population:%d,build_num:%d,added_build_num:%d%s", atb.nPopulation, atb.nBuildNum, atb.nAddedBuildNum, WHLINEEND);
					printf("free_draw_lottery_num:%d%s", atb.nFreeDrawLotteryNum, WHLINEEND);
					printf("signature:%s,change_name_num:%d%s", atb.szSignature, atb.nChangeNameNum, WHLINEEND);
					printf("protect_time:%d,binded:%d%s", atb.nProtectTime, (int)atb.bBinded, WHLINEEND);
					char	szBuf1[64];
					char	szBuf2[64];
					printf("last_login_time:%s,last_logout_time:%s%s", wh_gettimestr(atb.nLastLoginTime, szBuf1), wh_gettimestr(atb.nLastLogoutTime, szBuf2), WHLINEEND);
				}
				break;
			case STC_GAMECMD_CHAT_PRIVATE:
				{
					STC_GAMECMD_CHAT_PRIVATE_T*	pChat	= (STC_GAMECMD_CHAT_PRIVATE_T*)pGameCmd;
					char	szBuf[1024];
					wh_utf82char(pChat->szText, pChat->nTextLen, szBuf, sizeof(szBuf));
					printf("chat_private:from_name:%s,from_id:0x%"WHINT64PRFX"X,to_name:%s,to_id:0x%"WHINT64PRFX"X,text:%s%s"
						, pChat->szFrom, pChat->nFromAccountID, pChat->szTo, pChat->nToAccountID, szBuf, WHLINEEND);
				}
				break;
			case STC_GAMECMD_CHAT_ALLIANCE:
				{
					STC_GAMECMD_CHAT_ALLIANCE_T*	pChat	= (STC_GAMECMD_CHAT_ALLIANCE_T*)pGameCmd;
					char	szBuf[1024];
					wh_utf82char(pChat->szText, pChat->nTextLen, szBuf, sizeof(szBuf));
					printf("chat_alliance:name:%s,id:0x%"WHINT64PRFX"X,text:%s%s", pChat->szFrom, pChat->nAccountID, szBuf, WHLINEEND);
				}
				break;
			case STC_GAMECMD_CHAT_WORLD:
				{
					STC_GAMECMD_CHAT_WORLD_T*	pChat	= (STC_GAMECMD_CHAT_WORLD_T*)pGameCmd;
					char	szBuf[1024];
					wh_utf82char(pChat->szText, pChat->nTextLen, szBuf, sizeof(szBuf));
					printf("chat_world:name:%s,id:0x%"WHINT64PRFX"X,text:%s%s", pChat->szFrom, pChat->nAccountID, szBuf, WHLINEEND);
				}
				break;
			case STC_GAMECMD_WORLD_NOTIFY:
				{
					STC_GAMECMD_WORLD_NOTIFY_T*	pChat	= (STC_GAMECMD_WORLD_NOTIFY_T*)pGameCmd;
					char	szBuf[1024];
					wh_utf82char(pChat->szText, pChat->nTextLen, szBuf, sizeof(szBuf));
					printf("battle_report:id:0x%"WHINT64PRFX"X,text:%s%s", pChat->nAccountID, szBuf, WHLINEEND);
				}
				break;
			case STC_GAMECMD_CHAT_GROUP:
				{
					STC_GAMECMD_CHAT_GROUP_T*	pChat	= (STC_GAMECMD_CHAT_GROUP_T*)pGameCmd;
					char	szBuf[1024];
					wh_utf82char(pChat->szText, pChat->nTextLen, szBuf, sizeof(szBuf));
					printf("chat_group:name:%s,id:0x%"WHINT64PRFX"X,text:%s%s", pChat->szFrom, pChat->nAccountID, szBuf, WHLINEEND);
				}
				break;
			case STC_GAMECMD_CHAR_IS_ONLINE:
				{
					STC_GAMECMD_CHAR_IS_ONLINE_T*	pOnline	= (STC_GAMECMD_CHAR_IS_ONLINE_T*)pGameCmd;
					printf("char_online:0x%"WHINT64PRFX"X,online:%d%s", pOnline->nAccountID, (int)pOnline->bOnline, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ADD_MAIL:
				{
					STC_GAMECMD_ADD_MAIL_T*	pAdd	= (STC_GAMECMD_ADD_MAIL_T*)pGameCmd;
					printf("add_mail:rst:%d%s", pAdd->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_MAIL:
				{
					STC_GAMECMD_GET_MAIL_T*	pGet	= (STC_GAMECMD_GET_MAIL_T*)pGameCmd;
					printf("get_mail:rst:%d,num:%d,total_num:%d,unreaded_num:%d%s", pGet->nRst, pGet->nNum, pGet->nTotalMailNum, pGet->nUnreadMailNum, WHLINEEND);
					PrivateMailUnit*	pUnit		= (PrivateMailUnit*)wh_getptrnexttoptr(pGet);
					for (int i=0; i<pGet->nNum; i++)
					{
						char	szText[TTY_MAIL_TEXT_LEN];
						wh_utf82char(pUnit->szText, pUnit->nTextLen, szText, sizeof(szText));
						char	szBuf[TTY_CHARACTERNAME_LEN];
						wh_utf82char(pUnit->szSender, strlen(pUnit->szSender), szBuf, sizeof(szBuf));
						int		nMailSize			= wh_offsetof(PrivateMailUnit, szText)+pUnit->nTextLen;
						int*	pExtDataLen			= (int*)wh_getoffsetaddr(pUnit, nMailSize);
						unsigned char*	pExtData	= (unsigned char*)wh_getptrnexttoptr(pExtDataLen);
						nMailSize					+= (sizeof(int) + *pExtDataLen);
						printf("type:%d,flag:%d,text:%s,mail_id:%d,sender:%s,sender_id:0x%"WHINT64PRFX"X,time:%s,readed:%d,ext_data_len:%d,ext_data:%s%s"
							, (int)pUnit->nType, (int)pUnit->nFlag, szText, pUnit->nMailID, szBuf, pUnit->nSenderID, wh_gettimestr(pUnit->nTime)
							, (int)pUnit->bReaded, *pExtDataLen, wh_hex2str(pExtData, *pExtDataLen), WHLINEEND);

						pUnit		= (PrivateMailUnit*)wh_getoffsetaddr(pUnit, nMailSize);
					}
				}
				break;
			case STC_GAMECMD_READ_MAIL:
				{
					STC_GAMECMD_READ_MAIL_T*	pGet	= (STC_GAMECMD_READ_MAIL_T*)pGameCmd;
					printf("read_mail:rst:%d,mail_id:%d%s", pGet->nRst, pGet->nMailID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_DELETE_MAIL:
				{
					STC_GAMECMD_DELETE_MAIL_T*	pDelete	= (STC_GAMECMD_DELETE_MAIL_T*)pGameCmd;
					printf("delete_mail:rst:%d,mail_id:%d,total_mail_num:%d%s", pDelete->nRst, pDelete->nMailID, pDelete->nTotalMailNum, WHLINEEND);
				}
				break;
// 			case STC_GAMECMD_GET_MAIL_1:
// 				{
// 					STC_GAMECMD_GET_MAIL_1_T*	pGet	= (STC_GAMECMD_GET_MAIL_1_T*)pGameCmd;
// 					PrivateMailUnit*	pUnit		= (PrivateMailUnit*)wh_getptrnexttoptr(pGet);
// 					char	szText[TTY_MAIL_TEXT_LEN];
// 					wh_utf82char(pUnit->szText, pUnit->nTextLen, szText, sizeof(szText));
// 					char	szBuf[TTY_CHARACTERNAME_LEN];
// 					wh_utf82char(pUnit->szSender, strlen(pUnit->szSender), szBuf, sizeof(szBuf));
// 					printf("get_mail_1:rst:%d,type:%d,text:%s,mail_id:%d,sender:%s,sender_id:0x%"WHINT64PRFX"X,time:%s,readed:%d%s"
// 						, pGet->nRst, pUnit->nType, szText, pUnit->nMailID, szBuf, pUnit->nSenderID, wh_gettimestr(pUnit->nTime), (int)pUnit->bReaded, WHLINEEND);
// 				}
// 				break;
			case STC_GAMECMD_NEW_PRIVATE_MAIL:
				{
					STC_GAMECMD_NEW_PRIVATE_MAIL_T*	pNew	= (STC_GAMECMD_NEW_PRIVATE_MAIL_T*)pGameCmd;
					PrivateMailUnit*	pUnit				= (PrivateMailUnit*)wh_getptrnexttoptr(pNew);
					char	szText[TTY_MAIL_TEXT_LEN];
					wh_utf82char(pUnit->szText, pUnit->nTextLen, szText, sizeof(szText));
					char	szBuf[TTY_CHARACTERNAME_LEN];
					wh_utf82char(pUnit->szSender, strlen(pUnit->szSender), szBuf, sizeof(szBuf));
					int		nMailSize			= wh_offsetof(PrivateMailUnit, szText)+pUnit->nTextLen;
					int*	pExtDataLen			= (int*)wh_getoffsetaddr(pUnit, nMailSize);
					unsigned char*	pExtData			= (unsigned char*)wh_getptrnexttoptr(pExtDataLen);
					printf("new_private_mail:type:%d,flag:%d,text:%s,mail_id:%d,sender:%s,sender_id:0x%"WHINT64PRFX"X,time:%s,readed:%d,ext_data_len:%d,ext_data:%s%s"
						, (int)pUnit->nType, (int)pUnit->nFlag, szText, pUnit->nMailID, szBuf, pUnit->nSenderID, wh_gettimestr(pUnit->nTime)
						, (int)pUnit->bReaded, *pExtDataLen, ""/*wh_hex2str(pExtData, *pExtDataLen)*/, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI:
				{
					STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T*	pRefuse	= (STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T*)pGameCmd;
					printf("refuse_join_alliance:rst:%d,applicant_id:0x%"WHINT64PRFX"X%s", pRefuse->nRst, pRefuse->nApplicantID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ABDICATE_ALLI:
				{
					STC_GAMECMD_OPERATE_ABDICATE_ALLI_T*	pAbdicate	= (STC_GAMECMD_OPERATE_ABDICATE_ALLI_T*)pGameCmd;
					printf("abdicate_alliance:rst:%d%s", pAbdicate->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_MY_FRIEND_APPLY:
				{
					STC_GAMECMD_GET_MY_FRIEND_APPLY_T*	pApply	= (STC_GAMECMD_GET_MY_FRIEND_APPLY_T*)pGameCmd;
					printf("get_my_friend_apply:rst:%d,num:%d%s", pApply->nRst, pApply->nNum, WHLINEEND);
					FriendApplyUnit*	pUnit	= (FriendApplyUnit*)wh_getptrnexttoptr(pApply);
					for (unsigned int i=0; i<pApply->nNum; i++)
					{
						char	szBuf[TTY_CHARACTERNAME_LEN];
						wh_utf82char(pUnit->szName, strlen(pUnit->szName), szBuf, sizeof(szBuf));
						printf("id:0x%"WHINT64PRFX"X,name:%s,head_id:%d,level:%d%s", pUnit->nAccountID, szBuf, pUnit->nHeadID, pUnit->nLevel, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_OTHERS_FRIEND_APPLY:
				{
					STC_GAMECMD_GET_OTHERS_FRIEND_APPLY_T*	pApply	= (STC_GAMECMD_GET_OTHERS_FRIEND_APPLY_T*)pGameCmd;
					printf("get_others_friend_apply:rst:%d,num:%d%s", pApply->nRst, pApply->nNum, WHLINEEND);
					FriendApplyUnit*	pUnit	= (FriendApplyUnit*)wh_getptrnexttoptr(pApply);
					for (unsigned int i=0; i<pApply->nNum; i++)
					{
						char	szBuf[TTY_CHARACTERNAME_LEN];
						wh_utf82char(pUnit->szName, strlen(pUnit->szName), szBuf, sizeof(szBuf));
						printf("id:0x%"WHINT64PRFX"X,name:%s,head_id:%d,level:%d%s", pUnit->nAccountID, szBuf, pUnit->nHeadID, pUnit->nLevel, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_FRIEND_LIST:
				{
					STC_GAMECMD_GET_FRIEND_LIST_T*	pGet	= (STC_GAMECMD_GET_FRIEND_LIST_T*)pGameCmd;
					printf("get_friend_list:rst:%d,num:%d%s", pGet->nRst, pGet->nNum, WHLINEEND);
					FriendUnit*	pUnit	= (FriendUnit*)wh_getptrnexttoptr(pGet);
					for (unsigned int i=0; i<pGet->nNum; i++)
					{
						char	szBuf[TTY_CHARACTERNAME_LEN];
						wh_utf82char(pUnit->szName, strlen(pUnit->szName), szBuf, sizeof(szBuf));
						printf("id:0x%"WHINT64PRFX"X,name:%s,head_id:%d,level:%d%s", pUnit->nAccountID, szBuf, pUnit->nHeadID, pUnit->nLevel, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_APPROVE_FRIEND:
				{
					STC_GAMECMD_OPERATE_APPROVE_FRIEND_T*	pApprove	= (STC_GAMECMD_OPERATE_APPROVE_FRIEND_T*)pGameCmd;
					printf("approve_friend:rst:%d,peer_id:0x%"WHINT64PRFX"X,name:%s%s", pApprove->nRst, pApprove->friendUnit.nAccountID, pApprove->friendUnit.szName, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_APPLY_FRIEND:
				{
					STC_GAMECMD_OPERATE_APPLY_FRIEND_T*	pApply	= (STC_GAMECMD_OPERATE_APPLY_FRIEND_T*)pGameCmd;
					printf("apply_friend:rst:%d,peer_id:0x%"WHINT64PRFX"X%s", pApply->nRst, pApply->nPeerAccountID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY:
				{
					STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T*	pRefuse	= (STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T*)pGameCmd;
					printf("refuse_friend_apply:rst:%d,peer_id:0x%"WHINT64PRFX"X%s", pRefuse->nRst, pRefuse->nPeerAccountID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY:
				{
					STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T*	pCancel	= (STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T*)pGameCmd;
					printf("cancel_friend_apply:rst:%d,peer_id:0x%"WHINT64PRFX"X%s", pCancel->nRst, pCancel->nPeerAccountID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_DELETE_FRIEND:
				{
					STC_GAMECMD_OPERATE_DELETE_FRIEND_T*	pDelete	= (STC_GAMECMD_OPERATE_DELETE_FRIEND_T*)pGameCmd;
					printf("delete_friend:rst:%d,peer_id:0x%"WHINT64PRFX"X%s", pDelete->nRst, pDelete->nPeerAccountID, WHLINEEND);
				}
				break;

			case STC_GAMECMD_OPERATE_TAVERN_REFRESH:
				{
					STC_GAMECMD_OPERATE_TAVERN_REFRESH_T *pRst = (STC_GAMECMD_OPERATE_TAVERN_REFRESH_T*)pGameCmd;
					printf("tavern_refresh,%d,%d%s", pRst->nRst, pRst->nCD, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_HIRE_HERO:
				{
					STC_GAMECMD_OPERATE_HIRE_HERO_T *pRst = (STC_GAMECMD_OPERATE_HIRE_HERO_T*)pGameCmd;
					printf("hire_hero,%d,0x%"WHINT64PRFX"X,%d%s", pRst->nRst, pRst->nHeroID, pRst->nSlotID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_FIRE_HERO:
				{
					STC_GAMECMD_OPERATE_FIRE_HERO_T *pRst = (STC_GAMECMD_OPERATE_FIRE_HERO_T*)pGameCmd;
					printf("fire_hero,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_LEVELUP_HERO:
				{
					STC_GAMECMD_OPERATE_LEVELUP_HERO_T *pRst = (STC_GAMECMD_OPERATE_LEVELUP_HERO_T*)pGameCmd;
					printf("levelup_hero,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ADD_GROW:
				{
					STC_GAMECMD_OPERATE_ADD_GROW_T *pRst = (STC_GAMECMD_OPERATE_ADD_GROW_T*)pGameCmd;
					printf("add_grow,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_CONFIG_HERO:
				{
					STC_GAMECMD_OPERATE_CONFIG_HERO_T *pRst = (STC_GAMECMD_OPERATE_CONFIG_HERO_T*)pGameCmd;
					printf("config_hero,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_GET_HERO_REFRESH:
				{
					STC_GAMECMD_OPERATE_GET_HERO_REFRESH_T *pRst = (STC_GAMECMD_OPERATE_GET_HERO_REFRESH_T*)pGameCmd;
					printf("get_hero_refresh,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					RefreshHero*	pHero	= (RefreshHero*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("%d,0x%"WHINT64PRFX"X,%s,%d,%d,%d,%d,%d,%.2f%s", 
							pHero->nSlotID, pHero->nHeroID, pHero->szName, pHero->nProfession, pHero->nModel, pHero->nAttack, pHero->nDefense, pHero->nHealth, pHero->fGrow, WHLINEEND);
						pHero++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_GET_HERO_HIRE:
				{
					STC_GAMECMD_OPERATE_GET_HERO_HIRE_T *pRst = (STC_GAMECMD_OPERATE_GET_HERO_HIRE_T*)pGameCmd;
					printf("get_hero_hire,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					HireHero*	pHero	= (HireHero*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%s,%d,%d,%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d,%d,%d,%d,%d,%d,%d,%d%s", 
							pHero->nHeroID, pHero->szName, pHero->nProfession, pHero->nModel, pHero->nAttack, pHero->nAttackAdd, pHero->nDefense, pHero->nDefenseAdd, pHero->nHealth, pHero->nHealthAdd, pHero->nLeader, pHero->nLeaderAdd, pHero->fGrow, 
							pHero->nArmyType, pHero->nArmyLevel, pHero->nArmyNum, pHero->nStatus, pHero->nLevel, pHero->nProf, pHero->nHealthState, pHero->nExp, WHLINEEND);
						pHero++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_START_COMBAT:
				{
					STC_GAMECMD_OPERATE_START_COMBAT_T *pRst = (STC_GAMECMD_OPERATE_START_COMBAT_T*)pGameCmd;
					printf("start_combat,%d,0x%"WHINT64PRFX"X,%d,%d,%d%s", pRst->nRst, pRst->nCombatID, pRst->nCombatType, pRst->nGoTime, pRst->nBackTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_STOP_COMBAT:
				{
					STC_GAMECMD_OPERATE_STOP_COMBAT_T *pRst = (STC_GAMECMD_OPERATE_STOP_COMBAT_T*)pGameCmd;
					printf("stop_combat,%d,%d,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d%s", 
						pRst->nRst, pRst->nCombatRst, pRst->nCombatID, pRst->nCombatType, pRst->n1Hero, pRst->n2Hero, pRst->n3Hero, pRst->n4Hero, pRst->n5Hero, pRst->nDrug, WHLINEEND);
					switch (pRst->nRst)
					{
					// 出错
					case 0:
						{
						}break;
					// 战斗结束
					case 1:
						{
						}break;
					// 战斗结束，（由于打开了自动战斗）新战斗已经开始
					case 2:
						{
							printf("0x%"WHINT64PRFX"X,%d,%d,%d%s", pRst->nNewCombatID, pRst->nGoTime, pRst->nBackTime, pRst->nNum, WHLINEEND);
							InstanceHeroDesc*	pHero	= (InstanceHeroDesc*)wh_getptrnexttoptr(pRst);
							for (int i=0; i<pRst->nNum; i++)
							{
								printf("0x%"WHINT64PRFX"X,%d,%d,%d%s", pHero->nHeroID, pHero->nArmyNum, pHero->nArmyProf, pHero->nHealthState, WHLINEEND);
								pHero++;
							}
						}break;
					// 战斗结束，副本通关
					case 3:
						{
						}break;
					// 战斗结束，副本重试次数用完
					case 4:
						{
						}break;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_GET_COMBAT:
				{
					STC_GAMECMD_OPERATE_GET_COMBAT_T *pRst = (STC_GAMECMD_OPERATE_GET_COMBAT_T*)pGameCmd;
					printf("get_combat,%d%s", pRst->nRst, WHLINEEND);
					GC_CombatCalc_Raid(pRst);
				}
				break;
			case STC_GAMECMD_OPERATE_LIST_COMBAT:
				{
					STC_GAMECMD_OPERATE_LIST_COMBAT_T *pRst = (STC_GAMECMD_OPERATE_LIST_COMBAT_T*)pGameCmd;
					printf("list_combat,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					CombatDesc*	pCombat	= (CombatDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%s,%d,%d%s", pCombat->nCombatID, pCombat->nCombatType, pCombat->nObjID, pCombat->szName, pCombat->nResTime, pCombat->nStatus, WHLINEEND);
						pCombat++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE:
				{
					STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T *pRst = (STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T*)pGameCmd;
					printf("config_citydefense,%d%s", pRst->nRst, WHLINEEND);
				}
				break;

			case STC_GAMECMD_GET_ITEM:
				{
					STC_GAMECMD_GET_ITEM_T *pRst = (STC_GAMECMD_GET_ITEM_T*)pGameCmd;
					printf("get_item,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					ItemDesc* pItem = (ItemDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%d,%d%s", pItem->nItemID, pItem->nExcelID, pItem->nNum, WHLINEEND);
						pItem++;
					}
				}
				break;
			case STC_GAMECMD_ADD_ITEM:
				{
					STC_GAMECMD_ADD_ITEM_T *pRst = (STC_GAMECMD_ADD_ITEM_T*)pGameCmd;
					printf("add_item,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_DEL_ITEM:
				{
					STC_GAMECMD_DEL_ITEM_T *pRst = (STC_GAMECMD_DEL_ITEM_T*)pGameCmd;
					printf("del_item,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_EQUIP_ITEM:
				{
					STC_GAMECMD_EQUIP_ITEM_T *pRst = (STC_GAMECMD_EQUIP_ITEM_T*)pGameCmd;
					printf("equip_item,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_DISEQUIP_ITEM:
				{
					STC_GAMECMD_DISEQUIP_ITEM_T *pRst = (STC_GAMECMD_DISEQUIP_ITEM_T*)pGameCmd;
					printf("disequip_item,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_MOUNT_ITEM:
				{
					STC_GAMECMD_MOUNT_ITEM_T *pRst = (STC_GAMECMD_MOUNT_ITEM_T*)pGameCmd;
					HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRst);
					printf("mount_item,%d,%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d,%d,%d,%d%s", pRst->nRst, pHero->nAttackBase, pHero->nAttackAdd, pHero->nDefenseBase, pHero->nDefenseAdd, pHero->nHealthBase, pHero->nHealthAdd, pHero->nLeaderBase, pHero->nLeaderAdd, pHero->fGrow, pHero->nHealthState, pHero->nArmyType, pHero->nArmyLevel, pHero->nArmyNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_UNMOUNT_ITEM:
				{
					STC_GAMECMD_UNMOUNT_ITEM_T *pRst = (STC_GAMECMD_UNMOUNT_ITEM_T*)pGameCmd;
					HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRst);
					printf("unmount_item,%d,%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d,%d,%d,%d%s", pRst->nRst, pHero->nAttackBase, pHero->nAttackAdd, pHero->nDefenseBase, pHero->nDefenseAdd, pHero->nHealthBase, pHero->nHealthAdd, pHero->nLeaderBase, pHero->nLeaderAdd, pHero->fGrow, pHero->nHealthState, pHero->nArmyType, pHero->nArmyLevel, pHero->nArmyNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_COMPOS_ITEM:
				{
					STC_GAMECMD_COMPOS_ITEM_T *pRst = (STC_GAMECMD_COMPOS_ITEM_T*)pGameCmd;
					printf("compos_item,%d,0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nItemID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_DISCOMPOS_ITEM:
				{
					STC_GAMECMD_DISCOMPOS_ITEM_T *pRst = (STC_GAMECMD_DISCOMPOS_ITEM_T*)pGameCmd;
					printf("discompos_item,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_QUEST:
				{
					STC_GAMECMD_GET_QUEST_T *pRst = (STC_GAMECMD_GET_QUEST_T*)pGameCmd;
					printf("get_quest,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					QuestDesc* pQuest = (QuestDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("%d,%d%s", pQuest->nExcelID, pQuest->nDone, WHLINEEND);
						pQuest++;
					}
				}
				break;
			case STC_GAMECMD_DONE_QUEST:
				{
					STC_GAMECMD_DONE_QUEST_T *pRst = (STC_GAMECMD_DONE_QUEST_T*)pGameCmd;
					printf("done_quest,%d%s", pRst->nRst, WHLINEEND);
					UseItemDesc* pDesc = (UseItemDesc*)wh_getptrnexttoptr(pRst);
					printf("%d%s", pDesc->nNum, WHLINEEND);
					AddItemDesc* pItem = (AddItemDesc*)wh_getptrnexttoptr(pDesc);
					for (int i=0; i<pDesc->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%d,%d%s", pItem->nItemID, pItem->nExcelID, pItem->nNum, WHLINEEND);
						pItem++;
					}
				}
				break;
			case STC_GAMECMD_GET_INSTANCESTATUS:
				{
					STC_GAMECMD_GET_INSTANCESTATUS_T *pRst = (STC_GAMECMD_GET_INSTANCESTATUS_T*)pGameCmd;
					printf("get_instancestatus,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					InstanceStatus* pInstance = (InstanceStatus*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("%d,%d/%d,%d/%d,0x%"WHINT64PRFX"X,%d,%d,0x%"WHINT64PRFX"X%s", pInstance->nExcelID, pInstance->nResDayTimesFree, pInstance->nDayTimesFree, pInstance->nResDayTimesFee, pInstance->nDayTimesFee, pInstance->nInstanceID, pInstance->nStatus, pInstance->nClassID, pInstance->nCreatorID, WHLINEEND);
						pInstance++;
					}
				}
				break;
			case STC_GAMECMD_GET_INSTANCEDESC:
				{
					STC_GAMECMD_GET_INSTANCEDESC_T *pRst = (STC_GAMECMD_GET_INSTANCEDESC_T*)pGameCmd;
					printf("get_instancedesc,%d,%d,%d%s", pRst->nRst, pRst->nClassID, pRst->nNum, WHLINEEND);
					InstanceDesc* pInstance = (InstanceDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%s,%d,%d,%d,0x%"WHINT64PRFX"X%s", pInstance->nInstanceID, pInstance->szName, pInstance->nCurLevel, pInstance->nLeaderLevel, pInstance->nCurPlayer, pInstance->nCreatorID, WHLINEEND);
						pInstance++;
					}
				}
				break;
			case STC_GAMECMD_CREATE_INSTANCE:
				{
					STC_GAMECMD_CREATE_INSTANCE_T *pRst = (STC_GAMECMD_CREATE_INSTANCE_T*)pGameCmd;
					printf("create_instance,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_JOIN_INSTANCE:
				{
					STC_GAMECMD_JOIN_INSTANCE_T *pRst = (STC_GAMECMD_JOIN_INSTANCE_T*)pGameCmd;
					printf("join_instance,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_QUIT_INSTANCE:
				{
					STC_GAMECMD_QUIT_INSTANCE_T *pRst = (STC_GAMECMD_QUIT_INSTANCE_T*)pGameCmd;
					printf("quit_instance,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_DESTROY_INSTANCE:
				{
					STC_GAMECMD_DESTROY_INSTANCE_T *pRst = (STC_GAMECMD_DESTROY_INSTANCE_T*)pGameCmd;
					printf("destroy_instance,%d,0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nInstanceID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_INSTANCEDATA:
				{
					STC_GAMECMD_GET_INSTANCEDATA_T *pRst = (STC_GAMECMD_GET_INSTANCEDATA_T*)pGameCmd;
					printf("get_instancedata,%d,%d,%d,%d,%d,%d,%d%s", pRst->nRst, pRst->nCurLevel, pRst->nAutoCombat, pRst->nAutoSupply, pRst->nRetryTimes, pRst->nNum, pRst->nNumHero, WHLINEEND);
					InstanceData* pInstance = (InstanceData*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%s,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X%s", 
							pInstance->nAccountID, pInstance->szName, pInstance->nStatus, pInstance->n1Hero, pInstance->n2Hero, pInstance->n3Hero, pInstance->n4Hero, pInstance->n5Hero, WHLINEEND);
						pInstance++;
					}
					CombatUnit* pUnit = (CombatUnit*)pInstance;
					for (int i=0; i<pRst->nNumHero; i++)
					{
						printf("0x%"WHINT64PRFX"X,%s,%d,%d,%d,%d,%d,%d%s", 
							pUnit->nHeroID, pUnit->szName, pUnit->nArmyType, pUnit->nArmyLevel, pUnit->nArmyNum, pUnit->nAttack, pUnit->nDefense, pUnit->nHealth, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_INSTANCELOOT:
				{
					STC_GAMECMD_GET_INSTANCELOOT_T *pRst = (STC_GAMECMD_GET_INSTANCELOOT_T*)pGameCmd;
					printf("get_instanceloot,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					LootDesc* pLoot = (LootDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("%d%s", pLoot->nExcelID, WHLINEEND);
						pLoot++;
					}
				}
				break;
			case STC_GAMECMD_GET_INSTANCELOOT_ADDITEM:
				{
					STC_GAMECMD_GET_INSTANCELOOT_ADDITEM_T *pRst = (STC_GAMECMD_GET_INSTANCELOOT_ADDITEM_T*)pGameCmd;
					printf("get_instanceloot_additem,%d%s", pRst->nNum, WHLINEEND);
					InstanceLootDesc* pLoot = (InstanceLootDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%d,%d%s", pLoot->nItemID, pLoot->nExcelID, pLoot->nNum, WHLINEEND);
						pLoot++;
					}
				}
				break;
			case STC_GAMECMD_CONFIG_INSTANCEHERO:
				{
					STC_GAMECMD_CONFIG_INSTANCEHERO_T *pRst = (STC_GAMECMD_CONFIG_INSTANCEHERO_T*)pGameCmd;
					printf("config_instancehero,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_AUTO_COMBAT:
				{
					STC_GAMECMD_AUTO_COMBAT_T *pRst = (STC_GAMECMD_AUTO_COMBAT_T*)pGameCmd;
					printf("auto_combat,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_COMBAT_RST:
				{
					STC_GAMECMD_COMBAT_RST_T *pRst	= (STC_GAMECMD_COMBAT_RST_T*)pGameCmd;
					printf("combat_rst,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d%s", pRst->nCombatID, pRst->nCombatType, pRst->nObjID, pRst->nRst, pRst->nNum, WHLINEEND);
					CombatRst *p = (CombatRst*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; ++i)
					{
						printf("0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d,%d%s", p->nHeroID, p->nArmyNum, p->nArmyAttack, p->nArmyDefense, p->nArmyHealth, p->nHealthState, p->nArmyProf, p->nArmyNumRecover, WHLINEEND);
						++ p;
					}
				}
				break;
			case STC_GAMECMD_KICK_INSTANCE:
				{
					STC_GAMECMD_KICK_INSTANCE_T *pRst = (STC_GAMECMD_KICK_INSTANCE_T*)pGameCmd;
					printf("kick_instance,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_START_INSTANCE:
				{
					STC_GAMECMD_START_INSTANCE_T *pRst = (STC_GAMECMD_START_INSTANCE_T*)pGameCmd;
					printf("start_instance,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->n1Hero, pRst->n2Hero, pRst->n3Hero, pRst->n4Hero, pRst->n5Hero, WHLINEEND);
				}
				break;
			case STC_GAMECMD_PREPARE_INSTANCE:
				{
					STC_GAMECMD_PREPARE_INSTANCE_T *pRst = (STC_GAMECMD_PREPARE_INSTANCE_T*)pGameCmd;
					printf("prepare_instance,%d,%d%s", pRst->nRst, pRst->nStatus, WHLINEEND);
				}
				break;
			case STC_GAMECMD_SUPPLY_INSTANCE:
				{
					STC_GAMECMD_SUPPLY_INSTANCE_T *pRst = (STC_GAMECMD_SUPPLY_INSTANCE_T*)pGameCmd;
					printf("supply_instance,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_EQUIP_ITEM_ALL:
				{
					STC_GAMECMD_EQUIP_ITEM_ALL_T *pRst	= (STC_GAMECMD_EQUIP_ITEM_ALL_T*)pGameCmd;
					HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRst);
					printf("equip_item_all,%d,%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d%s", pRst->nRst, pHero->nAttackBase, pHero->nAttackAdd, pHero->nDefenseBase, pHero->nDefenseAdd, pHero->nHealthBase, pHero->nHealthAdd, pHero->nLeaderBase, pHero->nLeaderAdd, pHero->fGrow, pHero->nHealthState, WHLINEEND);
				}
				break;
			case STC_GAMECMD_MOUNT_ITEM_ALL:
				{
					STC_GAMECMD_MOUNT_ITEM_ALL_T *pRst	= (STC_GAMECMD_MOUNT_ITEM_ALL_T*)pGameCmd;
					HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRst);
					printf("mount_item_all,%d,%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d%s", pRst->nRst, pHero->nAttackBase, pHero->nAttackAdd, pHero->nDefenseBase, pHero->nDefenseAdd, pHero->nHealthBase, pHero->nHealthAdd, pHero->nLeaderBase, pHero->nLeaderAdd, pHero->fGrow, pHero->nHealthState, WHLINEEND);
				}
				break;
			case STC_GAMECMD_USE_ITEM:
				{
					STC_GAMECMD_USE_ITEM_T *pRst	= (STC_GAMECMD_USE_ITEM_T*)pGameCmd;
					printf("use_item,%d,%d%s", pRst->nRst, pRst->nType, WHLINEEND);
					if (pRst->nType==SYNC_TYPE_HERO)
					{
						HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRst);
						printf("%d,%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d,%d,%d%s", pRst->nRst, pHero->nAttackBase, pHero->nAttackAdd, pHero->nDefenseBase, pHero->nDefenseAdd, pHero->nHealthBase, pHero->nHealthAdd, pHero->nLeaderBase, pHero->nLeaderAdd, pHero->fGrow, pHero->nHealthState, pHero->nExp, pHero->nLevel, WHLINEEND);
					}
					else if (pRst->nType==SYNC_TYPE_CHAR)
					{
						CharDesc *pChar	= (CharDesc*)wh_getptrnexttoptr(pRst);
						printf("%d,%d,%d,%d,%d,%d,%d,%d,%d%s", pRst->nRst, pChar->nLevel, pChar->nExp, pChar->nDiamond, pChar->nCrystal, pChar->nGold, pChar->nVIP, pChar->nPopulation, pChar->nProtectTime, WHLINEEND);
					}
					else if (pRst->nType==SYNC_TYPE_ITEM)
					{
						UseItemDesc* pDesc = (UseItemDesc*)wh_getptrnexttoptr(pRst);
						printf("%d%s", pDesc->nNum, WHLINEEND);
						AddItemDesc* pItem = (AddItemDesc*)wh_getptrnexttoptr(pDesc);
						for (int i=0; i<pDesc->nNum; i++)
						{
							printf("0x%"WHINT64PRFX"X,%d,%d%s", pItem->nItemID, pItem->nExcelID, pItem->nNum, WHLINEEND);
							pItem++;
						}
					}
					else if (pRst->nType==SYNC_TYPE_DRUG)
					{
						DrugDesc *pDrug	= (DrugDesc*)wh_getptrnexttoptr(pRst);
						printf("%d,%d%s", pRst->nRst, pDrug->nNum, WHLINEEND);
					}
				}
				break;
			case STC_GAMECMD_SYNC_CHAR:
				{
					STC_GAMECMD_SYNC_CHAR_T *pRst	= (STC_GAMECMD_SYNC_CHAR_T*)pGameCmd;
					CharDesc *pChar	= (CharDesc*)wh_getptrnexttoptr(pRst);
					printf("sync_char,%d,%d,%d,%d,%d,%d%s", pRst->nRst, pChar->nExp, pChar->nDiamond, pChar->nCrystal, pChar->nGold, pChar->nVIP, WHLINEEND);
				}
				break;
			case STC_GAMECMD_SYNC_HERO:
				{
					STC_GAMECMD_SYNC_HERO_T *pRst	= (STC_GAMECMD_SYNC_HERO_T*)pGameCmd;
					HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRst);
					printf("sync_hero,%d,%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d%s", pRst->nRst, pHero->nAttackBase, pHero->nAttackAdd, pHero->nDefenseBase, pHero->nDefenseAdd, pHero->nHealthBase, pHero->nHealthAdd, pHero->nLeaderBase, pHero->nLeaderAdd, pHero->fGrow, pHero->nHealthState, WHLINEEND);
				}
				break;
			case STC_GAMECMD_AUTO_SUPPLY:
				{
					STC_GAMECMD_AUTO_SUPPLY_T *pRst = (STC_GAMECMD_AUTO_SUPPLY_T*)pGameCmd;
					printf("auto_supply,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_EQUIP:
				{
					STC_GAMECMD_GET_EQUIP_T *pRst = (STC_GAMECMD_GET_EQUIP_T*)pGameCmd;
					printf("get_equip,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					EquipDesc* pEquip = (EquipDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%d,%d,0x%"WHINT64PRFX"X%s", pEquip->nEquipID, pEquip->nEquipType, pEquip->nExcelID, pEquip->nHeroID, WHLINEEND);
						pEquip++;
					}
				}
				break;
			case STC_GAMECMD_GET_GEM:
				{
					STC_GAMECMD_GET_GEM_T *pRst = (STC_GAMECMD_GET_GEM_T*)pGameCmd;
					printf("get_gem,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					GemDesc* pGem = (GemDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X%s", pGem->nEquipID, pGem->nSlotIdx, pGem->nSlotType, pGem->nExcelID, pGem->nGemID, WHLINEEND);
						pGem++;
					}
				}
				break;
			case STC_GAMECMD_MANUAL_SUPPLY:
				{
					STC_GAMECMD_MANUAL_SUPPLY_T *pRst = (STC_GAMECMD_MANUAL_SUPPLY_T*)pGameCmd;
					printf("manual_supply,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					InstanceHeroDesc* pHero = (InstanceHeroDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("0x%"WHINT64PRFX"X,%d,%d,%d%s", pHero->nHeroID, pHero->nArmyNum, pHero->nArmyProf, pHero->nHealthState, WHLINEEND);
						pHero++;
					}
				}
				break;
			case STC_GAMECMD_REPLACE_EQUIP:
				{
					STC_GAMECMD_REPLACE_EQUIP_T *pRst	= (STC_GAMECMD_REPLACE_EQUIP_T*)pGameCmd;
					HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRst);
					printf("replace_equip,%d,%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d,%d,%d,%d%s", pRst->nRst, pHero->nAttackBase, pHero->nAttackAdd, pHero->nDefenseBase, pHero->nDefenseAdd, pHero->nHealthBase, pHero->nHealthAdd, pHero->nLeaderBase, pHero->nLeaderAdd, pHero->fGrow, pHero->nHealthState, pHero->nArmyType, pHero->nArmyLevel, pHero->nArmyNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_WORLDGOLDMINE:
				{
					STC_GAMECMD_GET_WORLDGOLDMINE_T *pRst = (STC_GAMECMD_GET_WORLDGOLDMINE_T*)pGameCmd;
					printf("get_worldgoldmine,%d,%d,%d%s", pRst->nRst, pRst->nArea, pRst->nNum, WHLINEEND);
					WorldGoldMineDesc* pMine = (WorldGoldMineDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("%d,%d,0x%"WHINT64PRFX"X,%s,%d%s", pMine->nClass, pMine->nIdx, pMine->nAccountID, pMine->szName, pMine->nSafeTime, WHLINEEND);
						pMine++;
					}
				}
				break;
			case STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO:
				{
					STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T *pRst = (STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T*)pGameCmd;
					printf("config_worldgoldmine_hero,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ROB_WORLDGOLDMINE:
				{
					STC_GAMECMD_ROB_WORLDGOLDMINE_T *pRst = (STC_GAMECMD_ROB_WORLDGOLDMINE_T*)pGameCmd;
					printf("rob_worldgoldmine,%d,0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nCombatID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_DROP_WORLDGOLDMINE:
				{
					STC_GAMECMD_DROP_WORLDGOLDMINE_T *pRst = (STC_GAMECMD_DROP_WORLDGOLDMINE_T*)pGameCmd;
					printf("drop_worldgoldmine,%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GAIN_WORLDGOLDMINE:
				{
					STC_GAMECMD_GAIN_WORLDGOLDMINE_T *pRst	= (STC_GAMECMD_GAIN_WORLDGOLDMINE_T*)pGameCmd;
					printf("gain_worldgoldmine,%d,%d,%d,%d,%d%s", pRst->nRst, pRst->nArea, pRst->nClass, pRst->nIdx, pRst->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_MY_WORLDGOLDMINE:
				{
					STC_GAMECMD_MY_WORLDGOLDMINE_T *pRst	= (STC_GAMECMD_MY_WORLDGOLDMINE_T*)pGameCmd;
					printf("my_worldgoldmine,%d,%d,%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X%s", 
						pRst->nRst, pRst->nArea, pRst->nClass, pRst->nIdx, pRst->nSafeTime, pRst->n1Hero, pRst->n2Hero, pRst->n3Hero, pRst->n4Hero, pRst->n5Hero, WHLINEEND);
				}
				break;
			case STC_GAMECMD_COMBAT_PROF:
				{
					STC_GAMECMD_COMBAT_PROF_T *pRst	= (STC_GAMECMD_COMBAT_PROF_T*)pGameCmd;
					printf("combat_prof,%d%s", pRst->nRst, WHLINEEND);
					CombatProf* pProf = (CombatProf*)wh_getptrnexttoptr(pRst);
					printf("%d,%d,%d,%d,%d,%d%s", pProf->nProf, pProf->n1ArmyType, pProf->n2ArmyType, pProf->n3ArmyType, pProf->n4ArmyType, pProf->n5ArmyType, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_CITYDEFENSE:
				{
					STC_GAMECMD_GET_CITYDEFENSE_T *pRst	= (STC_GAMECMD_GET_CITYDEFENSE_T*)pGameCmd;
					printf("get_citydefense,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->n1Hero, pRst->n2Hero, pRst->n3Hero, pRst->n4Hero, pRst->n5Hero, WHLINEEND);
				}
				break;
			case STC_GAMECMD_USE_DRUG:
				{
					STC_GAMECMD_USE_DRUG_T *pRst	= (STC_GAMECMD_USE_DRUG_T*)pGameCmd;
					printf("use_drug,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; ++i)
					{
						printf("%d/%d,%d/%d,%d/%d,%d/%d,%.2f,%d%s", pHero->nAttackBase, pHero->nAttackAdd, pHero->nDefenseBase, pHero->nDefenseAdd, pHero->nHealthBase, pHero->nHealthAdd, pHero->nLeaderBase, pHero->nLeaderAdd, pHero->fGrow, pHero->nHealthState, WHLINEEND);
						++ pHero;
					}
				}
				break;
			case STC_GAMECMD_SUPPLY_DRUG:
				{
					STC_GAMECMD_SUPPLY_DRUG_T *pRst	= (STC_GAMECMD_SUPPLY_DRUG_T*)pGameCmd;
					printf("supply_drug,%d%s", pRst->nNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_PVP_RST:
				{
					STC_GAMECMD_PVP_RST_T *pRst	= (STC_GAMECMD_PVP_RST_T*)pGameCmd;
					printf("pvp_rst,0x%"WHINT64PRFX"X,%d,%d,%d%s", pRst->nCombatID, pRst->nCombatRst, pRst->nGold, pRst->nMan, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ARMY_ACCELERATE:
				{
					STC_GAMECMD_ARMY_ACCELERATE_T *pRst	= (STC_GAMECMD_ARMY_ACCELERATE_T*)pGameCmd;
					printf("army_accelerate,0x%"WHINT64PRFX"X,%d,%d%s", pRst->nCombatID, pRst->nRst, pRst->nCostCrystal, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ARMY_BACK:
				{
					STC_GAMECMD_ARMY_BACK_T *pRst	= (STC_GAMECMD_ARMY_BACK_T*)pGameCmd;
					printf("army_back,0x%"WHINT64PRFX"X,%d,%d%s", pRst->nCombatID, pRst->nRst, pRst->nBackTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_WORLDCITY_GET:
				{
					STC_GAMECMD_WORLDCITY_GET_T *pRst	= (STC_GAMECMD_WORLDCITY_GET_T*)pGameCmd;
					printf("worldcity,%d,%d,%d,%d%s", pRst->nRst, pRst->nNum, pRst->nRefreshTime, pRst->nBonusTime, WHLINEEND);
					WorldCity *p	= (WorldCity*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; ++i)
					{
						printf("0x%"WHINT64PRFX"X,%d,%d,%d,%d%s", p->nID, p->nClass, p->nPosX, p->nPosY, p->nRecover, WHLINEEND);
						++ p;
					}
				}
				break;
			case STC_GAMECMD_WORLDCITY_GETLOG:
				{
					STC_GAMECMD_WORLDCITY_GETLOG_T *pRst	= (STC_GAMECMD_WORLDCITY_GETLOG_T*)pGameCmd;
					printf("worldcity_log,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					WorldCityLog *p	= (WorldCityLog*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; ++i)
					{
						printf("%s,%d%s", p->szName, p->nDiamond, WHLINEEND);
						++ p;
					}
				}
				break;
			case STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE:
				{
					STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T *pRst	= (STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T*)pGameCmd;
					printf("worldcity_log,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					WorldCityLogAlliance *p	= (WorldCityLogAlliance*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; ++i)
					{
						printf("%s,%d%s", p->szName, p->nDiamond, WHLINEEND);
						++ p;
					}
				}
				break;
			case STC_GAMECMD_WORLDCITY_RANK_MAN:
				{
					STC_GAMECMD_WORLDCITY_RANK_MAN_T *pRst	= (STC_GAMECMD_WORLDCITY_RANK_MAN_T*)pGameCmd;
					printf("worldcity_rank_man,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					WorldCityRankMan *p	= (WorldCityRankMan*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; ++i)
					{
						printf("0x%"WHINT64PRFX"X,%s,%d,%d,%d%s", p->nAccountID, p->szName, p->nHeadID, p->nLevel, p->nCup, WHLINEEND);
						++ p;
					}
				}
				break;
			case STC_GAMECMD_WORLDCITY_RANK_ALLIANCE:
				{
					STC_GAMECMD_WORLDCITY_RANK_ALLIANCE_T *pRst	= (STC_GAMECMD_WORLDCITY_RANK_ALLIANCE_T*)pGameCmd;
					printf("worldcity_rank_alliance,%d,%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					WorldCityRankAlliance *p	= (WorldCityRankAlliance*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; ++i)
					{
						printf("0x%"WHINT64PRFX"X,%s,%d,%d%s", p->nAllianceID, p->szName, p->nLevel, p->nCup, WHLINEEND);
						++ p;
					}
				}
				break;


			case STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN:
				{
					STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN_T*	pRst	= (STC_GAMECMD_GET_WORLD_AREA_INFO_TERRAIN_T*)pGameCmd;
					printf("get_terrain_area:(%d,%d) to (%d,%d),num:%d%s", pRst->nFromX, pRst->nFromY, pRst->nToX, pRst->nToY, pRst->nNum, WHLINEEND);
					TerrainUnit*	pUnit	= (TerrainUnit*)wh_getptrnexttoptr(pRst);
					for (unsigned int i=0; i<pRst->nNum; i++)
					{
						printf("terrain:(%d,%d),can_build:%d,f1:%d,f2:%d%s", pUnit->nPosX, pUnit->nPosY, (int)pUnit->info.bCanBuild, (int)pUnit->info.nFloor1, (int)pUnit->info.nFloor2, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_WORLD_AREA_INFO_CITY:
				{
					STC_GAMECMD_GET_WORLD_AREA_INFO_CITY_T*	pRst	= (STC_GAMECMD_GET_WORLD_AREA_INFO_CITY_T*)pGameCmd;
					printf("get_city_area:(%d,%d) to (%d,%d),num:%d%s", pRst->nFromX, pRst->nFromY, pRst->nToX, pRst->nToY, pRst->nNum, WHLINEEND);
					char	szBuf[1024]	= "";
					CityCard*	pCard	= (CityCard*)wh_getptrnexttoptr(pRst);
					for (unsigned int i=0; i<pRst->nNum; i++)
					{
						printf("-------------------------------%s", WHLINEEND);
						printf("account_id:0x%"WHINT64PRFX"X%s", pCard->nAccountID, WHLINEEND);
						wh_utf82char(pCard->szName, strlen(pCard->szName), szBuf, sizeof(szBuf));
						printf("name:%s%s", szBuf, WHLINEEND);
						printf("level:%u%s", pCard->nLevel, WHLINEEND);
						printf("vip:%u%s", pCard->nVip, WHLINEEND);
						printf("position:%u,%u%s", pCard->nPosX, pCard->nPosY, WHLINEEND);
						printf("alliance_id:0x%"WHINT64PRFX"X%s", pCard->nAllianceID, WHLINEEND);
						wh_utf82char(pCard->szName, strlen(pCard->szAllianceName), szBuf, sizeof(szBuf));
						printf("alliance_name:%s%s", szBuf, WHLINEEND);
						printf("protect_time:%d%s", pCard->nProtectTime, WHLINEEND);
						printf("-------------------------------%s", WHLINEEND);

						pCard++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL:
				{
					STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T*	pRst	= (STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T*)pGameCmd;
					printf("add_alliance_mail:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_ALLIANCE_MAIL:
				{
					STC_GAMECMD_GET_ALLIANCE_MAIL_T*	pGet	= (STC_GAMECMD_GET_ALLIANCE_MAIL_T*)pGameCmd;
					printf("get_alliance_mail:rst:%d,num:%d%s", pGet->nRst, pGet->nNum, WHLINEEND);
					AllianceMailUnit*	pMail	= (AllianceMailUnit*)wh_getptrnexttoptr(pGet);
					for (int i=0; i<pGet->nNum; i++)
					{
						char	szBuf1[TTY_CHARACTERNAME_LEN];
						char	szBuf2[TTY_ALLIANCE_MAIL_TEXT_LEN];
						wh_utf82char(pMail->szName, strlen(pMail->szName), szBuf1, sizeof(szBuf1));
						wh_utf82char(pMail->szText, pMail->nTextLen, szBuf2, sizeof(szBuf2));
						printf("mail_id:%d,account_id:0x%"WHINT64PRFX"X,name:%s,text:%s,time:%s%s", pMail->nMailID, pMail->nAccountID, szBuf1, szBuf2, wh_gettimestr(pMail->nTime), WHLINEEND);
						
						pMail				= (AllianceMailUnit*)wh_getoffsetaddr(pMail, wh_offsetof(AllianceMailUnit, szText)+pMail->nTextLen);
					}
				}
				break;
			case STC_GAMECMD_GET_ALLIANCE_LOG:
				{
					STC_GAMECMD_GET_ALLIANCE_LOG_T*	pGet	= (STC_GAMECMD_GET_ALLIANCE_LOG_T*)pGameCmd;
					printf("get_alliance_log:rst:%d,num:%d%s", pGet->nRst, pGet->nNum, WHLINEEND);
					AllianceLogUnit*	pLog	= (AllianceLogUnit*)wh_getptrnexttoptr(pGet);
					for (int i=0; i<pGet->nNum; i++)
					{
						char	szBuf1[TTY_ALLIANCE_MAIL_TEXT_LEN];
						wh_utf82char(pLog->szText, pLog->nTextLen, szBuf1, sizeof(szBuf1));
						printf("log_id:%d,text:%s,time:%s%s", pLog->nLogID, szBuf1, wh_gettimestr(pLog->nTime), WHLINEEND);

						pLog				= (AllianceLogUnit*)wh_getoffsetaddr(pLog, wh_offsetof(AllianceLogUnit, szText)+pLog->nTextLen);
					}
				}
				break;
			case STC_GAMECMD_GET_PRIVATE_LOG:
				{
					STC_GAMECMD_GET_PRIVATE_LOG_T*	pGet	= (STC_GAMECMD_GET_PRIVATE_LOG_T*)pGameCmd;
					printf("get_private_log:rst:%d,num:%d%s", pGet->nRst, pGet->nNum, WHLINEEND);
					PrivateLogUnit*	pLog	= (PrivateLogUnit*)wh_getptrnexttoptr(pGet);
					for (int i=0; i<pGet->nNum; i++)
					{
						char	szBuf1[TTY_ALLIANCE_MAIL_TEXT_LEN];
						wh_utf82char(pLog->szText, pLog->nTextLen, szBuf1, sizeof(szBuf1));
						printf("log_id:%d,text:%s,time:%s%s", pLog->nLogID, szBuf1, wh_gettimestr(pLog->nTime), WHLINEEND);

						pLog				= (PrivateLogUnit*)wh_getoffsetaddr(pLog, wh_offsetof(PrivateLogUnit, szText)+pLog->nTextLen);
					}
				}
				break;
			case STC_GAMECMD_OPERATE_BUY_ITEM:
				{
					STC_GAMECMD_OPERATE_BUY_ITEM_T*	pBuy	= (STC_GAMECMD_OPERATE_BUY_ITEM_T*)pGameCmd;
					printf("buy_item:rst:%d,excel_id:%d,num:%d,money_num:%d,money_type:%d,item_id_num:%d%s", pBuy->nRst, pBuy->nExcelID, pBuy->nNum, pBuy->nMoneyNum, pBuy->nMoneyType, pBuy->nItemIDNum, WHLINEEND);
					tty_id_t*	pIDs	= (tty_id_t*)wh_getptrnexttoptr(pBuy);
					for (unsigned int i=0; i<pBuy->nItemIDNum; i++)
					{
						printf("item_id:0x%"WHINT64PRFX"X%s", pIDs[i], WHLINEEND);
					}
				}
				break;
			case STC_GAMECMD_OPERATE_SELL_ITEM:
				{
					STC_GAMECMD_OPERATE_SELL_ITEM_T*	pSell	= (STC_GAMECMD_OPERATE_SELL_ITEM_T*)pGameCmd;
					printf("sell_item:rst:%d,id:0x%"WHINT64PRFX"X,excel_id:%d,num:%d%s", pSell->nRst, pSell->nItemID, pSell->nExcelID, pSell->nNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_RELATION_LOG:
				{
					STC_GAMECMD_GET_RELATION_LOG_T*	pRst	= (STC_GAMECMD_GET_RELATION_LOG_T*)pGameCmd;
					printf("get_relation_log:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					RelationLogUnit*	pUnit	= (RelationLogUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						char	szBuf[TTY_RELATION_LOG_TEXT_LEN];
						wh_utf82char(pUnit->szText, pUnit->nTextLen, szBuf, sizeof(szBuf));
						printf("log_id:%d,p_account_id:0x%"WHINT64PRFX"X,type:%d,time:%s,text:%s%s", pUnit->nLogID, pUnit->nAccountID, pUnit->nType, wh_gettimestr(pUnit->nTime), szBuf, WHLINEEND);
						
						pUnit		= (RelationLogUnit*)wh_getoffsetaddr(pUnit, wh_offsetof(RelationLogUnit, szText) + pUnit->nTextLen);
					}
				}
				break;
			case STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE:
				{
					STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T*	pRst	= (STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T*)pGameCmd;
					printf("fetch_christmas_tree:rst:%d,type:%d,crystal:%d,excel_id:%d,item_id:0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nType, pRst->nCrystal, pRst->nExcelID, pRst->nItemID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE:
				{
					STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T*	pRst	= (STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T*)pGameCmd;
					printf("watering_christmas_tree:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_STEAL_GOLD:
				{
					STC_GAMECMD_OPERATE_STEAL_GOLD_T*	pRst	= (STC_GAMECMD_OPERATE_STEAL_GOLD_T*)pGameCmd;
					printf("steal_gold:rst:%d,gold:%d%s", pRst->nRst, pRst->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_CHRISTMAS_TREE_READY_RIPE:
				{
					printf("christmas_tree_ready_ripe%s", WHLINEEND);
				}
				break;
			case STC_GAMECMD_CHRISTMAS_TREE_RIPE:
				{
					printf("christmas_tree_ripe%s", WHLINEEND);
				}
				break;
			case STC_GAMECMD_NEW_ALLIANCE_MAIL:
				{
					STC_GAMECMD_NEW_ALLIANCE_MAIL_T*	pRst	= (STC_GAMECMD_NEW_ALLIANCE_MAIL_T*)pGameCmd;
					AllianceMailUnit*	pMail	= (AllianceMailUnit*)wh_getptrnexttoptr(pRst);
					char	szBuf1[TTY_CHARACTERNAME_LEN];
					char	szBuf2[TTY_ALLIANCE_MAIL_TEXT_LEN];
					wh_utf82char(pMail->szName, strlen(pMail->szName), szBuf1, sizeof(szBuf1));
					wh_utf82char(pMail->szText, pMail->nTextLen, szBuf2, sizeof(szBuf2));
					printf("new_alliance_mail:mail_id:%d,account_id:0x%"WHINT64PRFX"X,name:%s,text:%s,time:%s%s", pMail->nMailID, pMail->nAccountID, szBuf1, szBuf2, wh_gettimestr(pMail->nTime), WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_TRAINING_TE:
				{
					STC_GAMECMD_GET_TRAINING_TE_T*	pRst	= (STC_GAMECMD_GET_TRAINING_TE_T*)pGameCmd;
					printf("get_training_te:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					TrainingTimeEvent*	pUnit	= (TrainingTimeEvent*)wh_getptrnexttoptr(pRst);
					for (unsigned int i=0; i<pRst->nNum; i++)
					{
						char	szBeginTime[32];
						char	szEndTime[32];
						wh_gettimestr(pUnit->nBeginTime, szBeginTime);
						wh_gettimestr(pUnit->nEndTime, szEndTime);
						printf("hero_id:0x%"WHINT64PRFX"X,exp:%d,begin_time:%s,end_time:%s%s", pUnit->nHeroID, pUnit->nExp, szBeginTime, szEndTime, WHLINEEND);
					}
				}
				break;
			case STC_GAMECMD_OPERATE_ADD_TRAINING:
				{
					STC_GAMECMD_OPERATE_ADD_TRAINING_T*	pRst	= (STC_GAMECMD_OPERATE_ADD_TRAINING_T*)pGameCmd;
					printf("add_training:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_EXIT_TRAINING:
				{
					STC_GAMECMD_OPERATE_EXIT_TRAINING_T*	pRst	= (STC_GAMECMD_OPERATE_EXIT_TRAINING_T*)pGameCmd;
					printf("exit_training:rst:%d,hero_id:0x%"WHINT64PRFX"X,exp:%d%s", pRst->nRst, pRst->hero.nHeroID, pRst->hero.nExp, WHLINEEND);
				}
				break;
			case STC_GAMECMD_TRAINING_OVER:
				{
					STC_GAMECMD_TRAINING_OVER_T*	pRst	= (STC_GAMECMD_TRAINING_OVER_T*)pGameCmd;
					printf("training_over:hero_id:0x%"WHINT64PRFX"X%s", pRst->hero.nHeroID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_NEW_FRIEND_APPLY:
				{
					STC_GAMECMD_NEW_FRIEND_APPLY_T*	pRst	= (STC_GAMECMD_NEW_FRIEND_APPLY_T*)pGameCmd;
					FriendApplyUnit*	pUnit				= &pRst->unit;
					char	szBuf[TTY_CHARACTERNAME_LEN];
					wh_utf82char(pUnit->szName, strlen(pUnit->szName), szBuf, sizeof(szBuf));
					printf("new_friend_apply:id:0x%"WHINT64PRFX"X,name:%s,level:%d%s", pUnit->nAccountID, szBuf, pUnit->nLevel, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE:
				{
					STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T*	pRst	= (STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T*)pGameCmd;
					printf("contribute_alliance:rst:%d,dev:%d,gold:%d%s", pRst->nRst, pRst->nDevelopment, pRst->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD:
				{
					STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T*	pRst	= (STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T*)pGameCmd;
					printf("convert_diamond_gold:rst:%d,diamond:%d,gold:%d%s", pRst->nRst, pRst->nDiamond, pRst->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_JOIN_ALLI_NAME:
				{
					STC_GAMECMD_OPERATE_JOIN_ALLI_NAME_T*	pRst	= (STC_GAMECMD_OPERATE_JOIN_ALLI_NAME_T*)pGameCmd;
					printf("join_alliance_name:rst:%d,alliance_id:0x%"WHINT64PRFX"X,alliance_name:%s%s", pRst->nRst, pRst->nAllianceID, pRst->szAllianceName, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT:
				{
					STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T*	pRst	= (STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T*)pGameCmd;
					printf("get_my_alliance_join:rst:%d,alliance_id:0x%"WHINT64PRFX"X,alliance_name:%s%s", pRst->nRst, pRst->nAllianceID, pRst->szAllianceName, WHLINEEND);
				}
				break;
			case STC_GAMECMD_IN_ALLIANCE:
				{
					STC_GAMECMD_IN_ALLIANCE_T*	pRst	= (STC_GAMECMD_IN_ALLIANCE_T*)pGameCmd;
					printf("in_alliance_notify:alliance_id:0x%"WHINT64PRFX"X%s", pRst->nAllianceID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID:
				{
					STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T*	pRst	= (STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T*)pGameCmd;
					AllianceCard*	pCard	= (AllianceCard*)wh_getptrnexttoptr(pRst);
					if (pRst->nRst == STC_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T::RST_OK)
					{
						printf("get_alliance_card_by_id:rst:%d,alliance_id:0x%"WHINT64PRFX"X,alliance_name:%s"
							",account_id:0x%"WHINT64PRFX"X,account_name:%s,member_num:%d,max_member_num:%d,rank:%d,level:%d%s"
							, pRst->nRst, pCard->nAllianceID, pCard->szAllianceName, pCard->nLeaderID, pCard->szLeaderName
							, pCard->nMemberNum, pCard->nMaxMemberNum, pCard->nRank, pCard->nLevel, WHLINEEND);
					}
					else
					{
						printf("get_alliance_card_by_id:rst:%d%s", pRst->nRst, WHLINEEND);
					}
					
				}
				break;
			case STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK:
				{
					STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T*	pRst	= (STC_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T*)pGameCmd;
					printf("get_alliance_card_by_rank:total_num:%d%s", pRst->nTotalNum, WHLINEEND);
					AllianceCard*	pCard	= (AllianceCard*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						
						printf("get_alliance_card_by_id:alliance_id:0x%"WHINT64PRFX"X,alliance_name:%s"
							",account_id:0x%"WHINT64PRFX"X,account_name:%s,member_num:%d,max_member_num:%d,rank:%d,level:%d%s"
							, pCard->nAllianceID, pCard->szAllianceName, pCard->nLeaderID, pCard->szLeaderName
							, pCard->nMemberNum, pCard->nMaxMemberNum, pCard->nRank, pCard->nLevel, WHLINEEND);
						pCard		= (AllianceCard*)wh_getoffsetaddr(pCard, wh_offsetof(AllianceCard, szIntroduction) + pCard->nIntroductionLen);
					}
				}
				break;
			case STC_GAMECMD_NEW_ALLIANCE_JOIN:
				{
					printf("new_alliance_join%s", WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_SET_ALLI_INTRO:
				{
					STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T*	pRst	= (STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T*)pGameCmd;
					printf("set_alli_intro:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_DRAW_LOTTERY:
				{
					STC_GAMECMD_OPERATE_DRAW_LOTTERY_T*	pRst	= (STC_GAMECMD_OPERATE_DRAW_LOTTERY_T*)pGameCmd;
					printf("draw_lottery:rst:%d,choice:%d,num:%d,type:%d,price:%d%s", pRst->nRst, pRst->nChoice, pRst->nNum, pRst->nType, pRst->nPrice, WHLINEEND);
					LotteryEntry*	pEntry	= (LotteryEntry*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("entry:type:%d,data:%d%s", pEntry->nType, pEntry->nData, WHLINEEND);
						pEntry++;
					}
				}
				break;
				/*
			case STC_GAMECMD_GET_OTHER_GOLDORE_POSITION:
				{
					STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T*	pRst	= (STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T*)pGameCmd;
					printf("get_other_goldore_position:rst:%d,account_id:0x%"WHINT64PRFX"X,num:%d%s", pRst->nRst, pRst->nAccountID, pRst->nNum, WHLINEEND);
					STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T::GoldorePositionUnit*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T::GoldorePositionUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("unit:auto_id:%d,level:%d%s", pUnit->nAutoID, pUnit->nLevel, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT:
				{
					STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*	pRst	= (STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*)pGameCmd;
					printf("get_other_goldore_production_event:rst:%d,account_id:0x%"WHINT64PRFX"X,num:%d%s", pRst->nRst, pRst->nAccountID, pRst->nNum, WHLINEEND);
					STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T::GoldoreProductionEvent*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T::GoldoreProductionEvent*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("unit:auto_id:%d,production:%d%s", pUnit->nAutoID, pUnit->nProduction, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE:
				{
					STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T*	pRst	= (STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T*)pGameCmd;
					printf("get_other_goldore_production_te:rst:%d,account_id:0x%"WHINT64PRFX"X,num:%d%s", pRst->nRst, pRst->nAccountID, pRst->nNum, WHLINEEND);
					STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T::GoldoreProductionTimeEvent*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T::GoldoreProductionTimeEvent*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("unit:auto_id:%d,begin_time:%d,end_time:%d%s", pUnit->nAutoID, pUnit->nBeginTime, pUnit->nEndTime, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO:
				{
					STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T*	pRst	= (STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T*)pGameCmd;
					printf("get_other_goldore_fetch_info:rst:%d,account_id:0x%"WHINT64PRFX"X,num:%d%s", pRst->nRst, pRst->nAccountID, pRst->nNum, WHLINEEND);
					STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T::FetchUnit*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T::FetchUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("unit:auto_id:%d,can_fetch:%d%s", pUnit->nAutoID, (int)pUnit->bCanFetch, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO:
				{
					STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T*	pRst	= (STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T*)pGameCmd;
					printf("get_other_christmas_tree_info:rst:%d,account_id:0x%"WHINT64PRFX"X,i_can_water:%d,count_down_time:%d,state:%d,watered_num:%d%s"
						, pRst->nRst, pRst->nAccountID, (int)pRst->bICanWater, pRst->nCountDownTime, pRst->nState, pRst->nWateredNum, WHLINEEND);
				}
				break;
				*/
			case STC_GAMECMD_GET_OTHER_GOLDORE_INFO:
				{
					STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T*	pRst	= (STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T*)pGameCmd;
					printf("get_other_goldore_info:rst:%d,account_id:0x%"WHINT64PRFX"X,num:%d%s"
						"CT_can_water:%d,CT_count_down_time:%d,CT_state:%d,CT_watered_num:%d%s"
						, pRst->nRst, pRst->nAccountID, pRst->nNum, WHLINEEND
						, (int)pRst->christmasTreeInfo.bICanWater, pRst->christmasTreeInfo.nCountDownTime
						, pRst->christmasTreeInfo.nState, pRst->christmasTreeInfo.nWateredNum, WHLINEEND);
					STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::GoldoreInfo*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::GoldoreInfo*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("goldore:auto_id:%d,count_down_time:%d,level:%d,state:%d%s"
							, pUnit->nAutoID, pUnit->nCountDownTime, pUnit->nLevel, pUnit->nState, WHLINEEND);

						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY:
				{
					STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T*	pRst	= (STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T*)pGameCmd;
					printf("check_draw_lottery:rst:%d,price:%d,type:%d%s", pRst->nRst, pRst->nPrice, pRst->nType, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_FETCH_LOTTERY:
				{
					STC_GAMECMD_OPERATE_FETCH_LOTTERY_T*	pRst	= (STC_GAMECMD_OPERATE_FETCH_LOTTERY_T*)pGameCmd;
					printf("fetch_lottery:rst:%d,type:%d,data:%d,item_id:0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nType, pRst->nData, pRst->nItemID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_CHRISTMAS_TREE_INFO:
				{
					STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T*	pRst	= (STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T*)pGameCmd;
					printf("get_christmas_tree:rst:%d,i_can_water:%d,count_down_time:%d,water_count_down_time:%d,state:%d,watered_num:%d%s"
						, pRst->nRst, (int)pRst->bICanWater, pRst->nCountDownTime, pRst->nCanWaterCountDownTime, pRst->nState, pRst->nWateredNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL:
				{
					STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T*	pRst	= (STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T*)pGameCmd;
					printf("get_goldore_smp_info_all:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					GoldoreSmpInfo*	pUnit	= (GoldoreSmpInfo*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("goldore_smp_info:account_id:0x%"WHINT64PRFX"X,fetch_gold:%d,can_water_CT:%d%s"
							, pUnit->nAccountID, (int)pUnit->bCanFetchGold, (int)pUnit->bCanWaterChristmasTree, WHLINEEND);

						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_ALLIANCE_TRADE_INFO:
				{
					STC_GAMECMD_GET_ALLIANCE_TRADE_INFO_T*	pRst	= (STC_GAMECMD_GET_ALLIANCE_TRADE_INFO_T*)pGameCmd;
					printf("get_alliance_trade_info:rst:%d,count_down_time:%d,gold:%d,trade_num:%d,excel_trade_gold:%d,excel_trade_num:%d,excel_trade_time:%d%s"
						, pRst->nRst, pRst->nCountDownTime, pRst->nGold, pRst->nTradeNum, pRst->nExcelTradeGold, pRst->nExcelTradeNum, pRst->nExcelTradeTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ALLIANCE_TRADE:
				{
					STC_GAMECMD_OPERATE_ALLIANCE_TRADE_T*	pRst	= (STC_GAMECMD_OPERATE_ALLIANCE_TRADE_T*)pGameCmd;
					printf("alliance_trade:rst:%d,count_down_time:%d,gold:%d%s", pRst->nRst, pRst->nCountDownTime, pRst->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_ENEMY_LIST:
				{
					STC_GAMECMD_GET_ENEMY_LIST_T*	pRst	= (STC_GAMECMD_GET_ENEMY_LIST_T*)pGameCmd;
					printf("get_enemy_list:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					EnemyUnit*	pUnit	= (EnemyUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("enemy:name:%s,account_id:0x%"WHINT64PRFX"X,head_id:%d,level:%d%s", pUnit->szName, pUnit->nAccountID, pUnit->nHeadID, pUnit->nLevel, WHLINEEND);

						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_DELETE_ENEMY:
				{
					STC_GAMECMD_OPERATE_DELETE_ENEMY_T*	pRst	= (STC_GAMECMD_OPERATE_DELETE_ENEMY_T*)pGameCmd;
					printf("delete_enemy:rst:%d,enemy_id:0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nEnemyID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_NEW_ENEMY:
				{
					STC_GAMECMD_NEW_ENEMY_T*	pRst	= (STC_GAMECMD_NEW_ENEMY_T*)pGameCmd;
					printf("new_enemy:name:%s,account_id:0x%"WHINT64PRFX"X,head_id:%d,level:%d%s"
						, pRst->enemy.szName, pRst->enemy.nAccountID, pRst->enemy.nHeadID, pRst->enemy.nLevel, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_RANK_LIST:
				{
					STC_GAMECMD_GET_RANK_LIST_T*	pGet	= (STC_GAMECMD_GET_RANK_LIST_T*)pGameCmd;
					printf("get_rank_list:type:%d,num:%d,total_num:%d%s", pGet->nType, pGet->nNum, pGet->nTotalNum, WHLINEEND);
					CharRankInfo*	pUnit	= (CharRankInfo*)wh_getptrnexttoptr(pGet);
					for (int i=0; i<pGet->nNum; i++)
					{
						printf("rank_info:rank:%d,data:%d,account_id:0x%"WHINT64PRFX"X,name:%s,head_id:%d,level:%d%s"
							, pUnit->nRank, pUnit->nData, pUnit->nAccountID, pUnit->szName, pUnit->nHeadID, pUnit->nLevel, WHLINEEND);
					
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_MY_RANK:
				{
					STC_GAMECMD_GET_MY_RANK_T*		pGet	= (STC_GAMECMD_GET_MY_RANK_T*)pGameCmd;
					printf("get_my_rank:type:%d,rank:%d,total_num:%d%s", pGet->nType, pGet->nRank, pGet->nTotalNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_NEW_NOTIFICATION:
				{
					STC_GAMECMD_NEW_NOTIFICATION_T*	pNew	= (STC_GAMECMD_NEW_NOTIFICATION_T*)pGameCmd;
					Web_Notification_T*	pNotification		= (Web_Notification_T*)wh_getptrnexttoptr(pNew);
					char*	pTitle				= (char*)wh_getptrnexttoptr(pNotification);
					int*	pContentLen			= (int*)wh_getoffsetaddr(pTitle, pNotification->nTitleLen);
					char*	pContent			= (char*)wh_getptrnexttoptr(pContentLen);
					printf("new_notification:id:%d,group_id:%d,time:%s,title:%s,content:%s%s", pNotification->nNotificationID, pNotification->nGroupID, wh_gettimestr(pNotification->nTime), pTitle, pContent, WHLINEEND);
				}
				break;
			case STC_GAMECMD_RECHARGE_DIAMOND:
				{
					STC_GAMECMD_RECHARGE_DIAMOND_T*	pRst	= (STC_GAMECMD_RECHARGE_DIAMOND_T*)pGameCmd;
					printf("recharge_diamond:added_diamond:%d,gift_diamond:%d,vip:%d%s", pRst->nAddedDiamond, pRst->nGiftDiamond, pRst->nVip, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_WRITE_SIGNATURE:
				{
					STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T*	pRst	= (STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T*)pGameCmd;
					printf("write_signature:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_IMPORTANT_CHAR_ATB:
				{
					STC_GAMECMD_GET_IMPORTANT_CHAR_ATB_T*	pRst	= (STC_GAMECMD_GET_IMPORTANT_CHAR_ATB_T*)pGameCmd;
					printf("get_important_char_atb:crystal:%d,development:%d,diamond:%d,exp:%d,gold:%d,population:%d,build_num_time:%d,protect_time:%d%s"
						, pRst->nCrystal, pRst->nDevelopment, pRst->nDiamond, pRst->nExp, pRst->nGold, pRst->nPopulation
						, pRst->nAddBuildNumTime, pRst->nProtectTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_CHANGE_NAME:
				{
					STC_GAMECMD_OPERATE_CHANGE_NAME_T*	pRst	= (STC_GAMECMD_OPERATE_CHANGE_NAME_T*)pGameCmd;
					printf("change_name:rst:%d,diamond:%d%s", pRst->nRst, pRst->nDiamond, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_MOVE_CITY:
				{
					STC_GAMECMD_OPERATE_MOVE_CITY_T*	pRst	= (STC_GAMECMD_OPERATE_MOVE_CITY_T*)pGameCmd;
					printf("move_position:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_BUILD_ITEM_USE_OUT:
				{
					STC_GAMECMD_BUILD_ITEM_USE_OUT_T*	pRst	= (STC_GAMECMD_BUILD_ITEM_USE_OUT_T*)pGameCmd;
					printf("build_item_use_out:current_build_num:%d%s", pRst->nCurrentBuildNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_PAY_SERIAL:
				{
					STC_GAMECMD_GET_PAY_SERIAL_T*	pRst	= (STC_GAMECMD_GET_PAY_SERIAL_T*)pGameCmd;
					char	szPaySerial[TTY_PAY_SERIAL_LEN + 1];
					memcpy(szPaySerial, pRst->szPaySerial, TTY_PAY_SERIAL_LEN);
					szPaySerial[TTY_PAY_SERIAL_LEN]			= 0;
					printf("get_payer_serial:%s%s", szPaySerial, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_READ_NOTIFICATION:
				{
					STC_GAMECMD_OPERATE_READ_NOTIFICATION_T*	pRst	= (STC_GAMECMD_OPERATE_READ_NOTIFICATION_T*)pGameCmd;
					printf("read_notification:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_CHANGE_HERO_NAME:
				{
					STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T*	pRst	= (STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T*)pGameCmd;
					printf("change_hero_name:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_READ_MAIL_ALL:
				{
					STC_GAMECMD_READ_MAIL_ALL_T*	pRst	= (STC_GAMECMD_READ_MAIL_ALL_T*)pGameCmd;
					printf("read_mail_all:rst:%d,mail_type:%d%s", pRst->nRst, pRst->nMailType, WHLINEEND);
				}
				break;
			case STC_GAMECMD_DELETE_MAIL_ALL:
				{
					STC_GAMECMD_DELETE_MAIL_ALL_T*	pRst	= (STC_GAMECMD_DELETE_MAIL_ALL_T*)pGameCmd;
					printf("delete_mail_all:rst:%d,mail_type:%d%s", pRst->nRst, pRst->nMailType, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ADD_GM_MAIL:
				{
					STC_GAMECMD_OPERATE_ADD_GM_MAIL_T*	pRst	= (STC_GAMECMD_OPERATE_ADD_GM_MAIL_T*)pGameCmd;
					printf("add_gm_mail:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_EXPELED_ALLI:
				{
					printf("expeled_alliance%s", WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_SET_ALLI_NAME:
				{
					STC_GAMECMD_OPERATE_SET_ALLI_NAME_T*	pRst	= (STC_GAMECMD_OPERATE_SET_ALLI_NAME_T*)pGameCmd;
					printf("set_alli_name:rst:%d,diamond:%d%s", pRst->nRst, pRst->nDiamond, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST:
				{
					STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T*	pRst	= (STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T*)pGameCmd;
					STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T::FamousCityUnit*	pUnit	= (STC_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T::FamousCityUnit*)wh_getptrnexttoptr(pRst);
					printf("get_world_famous_city_list:num:%d%s", pRst->nNum, WHLINEEND);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("type:%d,pos_x:%d,pos_y:%d%s", pUnit->nType, pUnit->nPosX, pUnit->nPosY, WHLINEEND);

						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD:
				{
					STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T*	pRst	= (STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T*)pGameCmd;
					printf("fetch_mail_reward:rst:%d,mail_id:%d,num:%d%s", pRst->nRst, pRst->nMailID, pRst->nNum, WHLINEEND);
					STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit*	pUnit	= (STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("reward:type:%d,num:%d,data:%d,id:0x%"WHINT64PRFX"X%s", pUnit->nType, pUnit->nNum, pUnit->nData, pUnit->nID, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_OPERATE_REGISTER_ACCOUNT:
				{
					STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T*	pRst	= (STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T*)pGameCmd;
					printf("register:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_BIND_DEVICE:
				{
					STC_GAMECMD_OPERATE_BIND_DEVICE_T*	pRst	= (STC_GAMECMD_OPERATE_BIND_DEVICE_T*)pGameCmd;
					printf("bind_device:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_SERVER_TIME:
				{
					STC_GAMECMD_GET_SERVER_TIME_T*	pRst	= (STC_GAMECMD_GET_SERVER_TIME_T*)pGameCmd;
					printf("ServerTimeNow:%u,%s%s", pRst->nTimeNow, wh_gettimestr(pRst->nTimeNow), WHLINEEND);
				}
				break;
			case STC_GAMECMD_ADCOLONY_AWARD:
				{
					STC_GAMECMD_ADCOLONY_AWARD_T*	pRst	= (STC_GAMECMD_ADCOLONY_AWARD_T*)pGameCmd;
					printf("type:%d,num:%d%s", pRst->nMoneyType, pRst->nMoneyNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_DISMISS_SOLDIER:
				{
					STC_GAMECMD_OPERATE_DISMISS_SOLDIER_T*	pRst	= (STC_GAMECMD_OPERATE_DISMISS_SOLDIER_T*)pGameCmd;
					printf("dismiss_soldier:%u,%u,%u%s", pRst->nExcelID, pRst->nLevel, pRst->nNum, WHLINEEND);
				}
				break;

			case STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE:
				{
					STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T*	pRst	= (STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T*)pGameCmd;
					printf("get_donate_soldier_queue:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T::UNIT_T*	pUnit	= (STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T::UNIT_T*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("%d,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X,%d,%d,%s%s"
							, pUnit->nDstType, pUnit->nEventID, pUnit->nExcelID
							, pUnit->nLevel, pUnit->nNum, pUnit->nPeerID
							, pUnit->nTime, pUnit->nType, pUnit->szCharName, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_LEFT_DONATE_SOLDIER:
				{
					STC_GAMECMD_GET_LEFT_DONATE_SOLDIER_T*	pRst	= (STC_GAMECMD_GET_LEFT_DONATE_SOLDIER_T*)pGameCmd;
					printf("get_left_donate_soldier:rst:%d,left_num:%d,max_num:%d%s", pRst->nRst, pRst->nLeftNum, pRst->nMaxNum, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER:
				{
					STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T*	pRst	= (STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T*)pGameCmd;
					printf("alli_donate_soldier:rst:%d,event_id:0x%"WHINT64PRFX"X,excel_id:%d,level:%d,num:%d,obj_id:0x%"WHINT64PRFX"X,time:%d%s"
						, pRst->nRst, pRst->nEventID, pRst->nExcelID, pRst->nLevel
						, pRst->nNum, pRst->nObjID, pRst->nTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER:
				{
					STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T*	pRst	= (STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T*)pGameCmd;
					printf("alli_recall_soldier:rst:%d,new_event:0x%"WHINT64PRFX"X,old_event:0x%"WHINT64PRFX"X,time:%d%s"
						, pRst->nRst, pRst->nNewEventID, pRst->nOldEventID, pRst->nTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER:
				{
					STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T*	pRst	= (STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T*)pGameCmd;
					printf("alli_acce_soldier:rst:%d,event_id:0x%"WHINT64PRFX"X,money:%d,money_type:%d,time:%d%s"
						, pRst->nRst, pRst->nEventID, pRst->nMoney, pRst->nMoneyType, pRst->nTime, WHLINEEND);
				}
				break;
			case STC_GAMECMD_DONATE_SOLDIER_TE:
				{
					STC_GAMECMD_DONATE_SOLDIER_TE_T*	pRst	= (STC_GAMECMD_DONATE_SOLDIER_TE_T*)pGameCmd;
					printf("donate_soldier_te:event_id:0x%"WHINT64PRFX"X,excel_id:%d,level:%d,num:%d,type:%d%s"
						, pRst->nEventID, pRst->nExcelID, pRst->nLevel, pRst->nNum, pRst->nType, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_ADD_GOLD_DEAL:
				{
					STC_GAMECMD_OPERATE_ADD_GOLD_DEAL_T*	pRst	= (STC_GAMECMD_OPERATE_ADD_GOLD_DEAL_T*)pGameCmd;
					printf("add_gold_deal:rst:%d,transaction_id:0x%"WHINT64PRFX"X,gold:%d%s"
						, pRst->nRst, pRst->nTransactionID, pRst->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET:
				{
					STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T*	pRst	= (STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T*)pGameCmd;
					printf("buy_gold_in_market:rst:%d,money_type:%d,total_price:%d,gold:%d%s"
						, pRst->nRst, pRst->nMoneyType, pRst->nTotalPrice, pRst->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL:
				{
					STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T*	pRst	= (STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T*)pGameCmd;
					printf("cancel_gold_deal:rst:%d,gold:%d%s", pRst->nRst, pRst->nGold, WHLINEEND);
				}
				break;
			case STC_GAMECMD_GET_GOLD_DEAL:
				{
					STC_GAMECMD_GET_GOLD_DEAL_T*	pRst	= (STC_GAMECMD_GET_GOLD_DEAL_T*)pGameCmd;
					printf("get_gold_deal:rst:%d,num:%d,total_num:%d,page:%d%s"
						, pRst->nRst, pRst->nNum, pRst->nTotalNum, pRst->nPage, WHLINEEND);
					GoldMarketDeal_T*	pUnit				= (GoldMarketDeal_T*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("transaction_id:0x%"WHINT64PRFX"X,gold:%d,price:%d,total_price:%d,account_id:0x%"WHINT64PRFX"X%s"
							, pUnit->nTransactionID, pUnit->nGold, pUnit->nPrice, pUnit->nTotalPrice, pUnit->nAccountID, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_SELF_GOLD_DEAL:
				{
					STC_GAMECMD_GET_SELF_GOLD_DEAL_T*	pRst	= (STC_GAMECMD_GET_SELF_GOLD_DEAL_T*)pGameCmd;
					printf("get_self_gold_deal:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					GoldMarketDeal_T*	pUnit				= (GoldMarketDeal_T*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("transaction_id:0x%"WHINT64PRFX"X,gold:%d,price:%d,total_price:%d,account_id:0x%"WHINT64PRFX"X%s"
							, pUnit->nTransactionID, pUnit->nGold, pUnit->nPrice, pUnit->nTotalPrice, pUnit->nAccountID, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_GET_WORLD_RES:
				{
					STC_GAMECMD_GET_WORLD_RES_T*	pRst	= (STC_GAMECMD_GET_WORLD_RES_T*)pGameCmd;
					printf("get_world_res:num:%d,from_x:%d,from_y:%d,to_x:%d,to_y:%d%s"
						, pRst->nNum, pRst->nFromX, pRst->nFromY, pRst->nToX, pRst->nToY, WHLINEEND);
					WorldResSimpleInfo*	pRes	= (WorldResSimpleInfo*)wh_getptrnexttoptr(pRst);
// 					for (int i=0; i<pRst->nNum; i++)
// 					{
// 						printf("can_build:%d,crystal:%d,floor2:%d,gold:%d,ID:0x%"WHINT64PRFX"X,level:%d,pop:%d,pos_x:%d,pos_y:%d"
// 							",src_terrain_type:%d,type:%d,army_data:%s,army_deploy:%s%s"
// 							, (int)pRes->bCanBuild, pRes->nCrystal, (unsigned int)pRes->nFloor2, pRes->nGold, pRes->nID
// 							, pRes->nLevel, pRes->nPop, pRes->nPosX, pRes->nPosY, pRes->nSrcTerrainType
// 							, pRes->nType, pRes->szArmyData, pRes->szArmyDeploy, WHLINEEND);
// 					}
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("id:0x%"WHINT64PRFX"X,type:%d,level:%d,pos_x:%d,pos_y:%d%s"
							, pRes->nID, pRes->nType, pRes->nLevel, pRes->nPosX, pRes->nPosY, WHLINEEND);
					}
				}
				break;
			case STC_GAMECMD_GET_WORLD_RES_CARD:
				{
					STC_GAMECMD_GET_WORLD_RES_CARD_T*	pRst	= (STC_GAMECMD_GET_WORLD_RES_CARD_T*)pGameCmd;
					if (pRst->nRst == STC_GAMECMD_GET_WORLD_RES_CARD_T::RST_OK)
					{
						WorldRes2Client*	pRes	= &pRst->res;
						printf("get_world_res_card:rst:%d,crystal:%d,gold:%d,ID:0x%"WHINT64PRFX"X,level:%d,pop:%d,pos_x:%d,pos_y:%d"
							",type:%d,force:%d%s", pRst->nRst
							, pRes->nCrystal, pRes->nGold, pRes->nID
							, pRes->nLevel, pRes->nPop, pRes->nPosX, pRes->nPosY
							, pRes->nType, pRes->nForce, WHLINEEND);
					}
					else
					{
						printf("get_world_res_card:rst:%d%s", pRst->nRst, WHLINEEND);
					}
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_GET_LIST:
				{
					STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T*)pGameCmd;
					printf("alli_instance_get_list:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T::ROOM_T*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T::ROOM_T*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("root:creator_id:0x%"WHINT64PRFX"X,head_id:%d,instance_id:0x%"WHINT64PRFX"X,level:%d,player_num:%d,name:%s%s"
							, pUnit->nCreatorID, pUnit->nHeadID, pUnit->nInstanceID, pUnit->nLevel, pUnit->nPlayerNum, pUnit->szCreatorName, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_CREATE:
				{
					STC_GAMECMD_ALLI_INSTANCE_CREATE_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_CREATE_T*)pGameCmd;
					printf("alli_instance_create:rst%d,instance_id:0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nInstanceID, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_JOIN:
				{
					STC_GAMECMD_ALLI_INSTANCE_JOIN_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_JOIN_T*)pGameCmd;
					printf("alli_instance_join:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_EXIT:
				{
					STC_GAMECMD_ALLI_INSTANCE_EXIT_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_EXIT_T*)pGameCmd;
					printf("alli_instance_exit:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_KICK:
				{
					STC_GAMECMD_ALLI_INSTANCE_KICK_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_KICK_T*)pGameCmd;
					printf("alli_instance_kick:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_DESTROY:
				{
					STC_GAMECMD_ALLI_INSTANCE_DESTROY_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_DESTROY_T*)pGameCmd;
					printf("alli_instance_destroy:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO:
				{
					STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T*)pGameCmd;
					printf("alli_instance_config_hero:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_START:
				{
					STC_GAMECMD_ALLI_INSTANCE_START_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_START_T*)pGameCmd;
					printf("alli_instance_start:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_READY:
				{
					STC_GAMECMD_ALLI_INSTANCE_READY_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_READY_T*)pGameCmd;
					printf("alli_instance_ready:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA:
				{
					STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T*)pGameCmd;
					printf("alli_instance_get_char_data:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T::CharData*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T::CharData*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("char_data:account_id:0x%"WHINT64PRFX"X,head_id:%d,level:%d,status:%d,name:%s%s"
							, pUnit->nAccountID, pUnit->nHeadID, pUnit->nLevel, pUnit->nStatus, pUnit->szName, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA:
				{
					STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T*)pGameCmd;
					printf("alli_instance_get_hero_data:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T::HeroData*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T::HeroData*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("char_data:account_id:0x%"WHINT64PRFX"X,hero_id:0x%"WHINT64PRFX"X,name:%s,level:%d,"
							"army_level:%d,army_num:%d,army_type:%d,col:%d,row:%d,head_id:%d,health:%d,prof:%d%s"
							, pUnit->nAccountID, pUnit->nHeroID, pUnit->szHeroName, pUnit->nLevel, pUnit->nArmyLevel, pUnit->nArmyNum, pUnit->nArmyType
							, pUnit->nCol, pUnit->nRow, pUnit->nHeadID, pUnit->nHealth, pUnit->nProf, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_GET_STATUS:
				{
					STC_GAMECMD_ALLI_INSTANCE_GET_STATUS_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_GET_STATUS_T*)pGameCmd;
					printf("alli_instance_get_status:rst:%d,auto_combat:%d,auto_supply:%d,char_status:%d"
						",cur_level:%d,instance_id:0x%"WHINT64PRFX"X,instance_status:%d,retry_times:%d,total_free_times:%d"
						",used_free_times:%d%s"
						, pRst->nRst, (int)pRst->bAutoCombat, (int)pRst->bAutoSupply, pRst->nCharStatus
						, pRst->nCurLevel, pRst->nInstanceID, pRst->nInstanceStatus, pRst->nRetryTimes
						, pRst->nTotalFreeTimes, pRst->nUsedFreeTimes, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY:
				{
					STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*	pRst		= (STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*)pGameCmd;
					printf("alli_instance_save_hero_deploy:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_START_COMBAT:
				{

				}
				break;
			case STC_GAMECMD_GET_KICK_CLIENT_ALL:
				{
					STC_GAMECMD_GET_KICK_CLIENT_ALL_T*	pRst	= (STC_GAMECMD_GET_KICK_CLIENT_ALL_T*)pGameCmd;
					printf("get_kick_client_all:rst:%d,length:%d,str:[%s]%s", pRst->nRst, pRst->nLength, (char*)wh_getptrnexttoptr(pRst), WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY:
				{
					STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T*)pGameCmd;
					printf("action_notify:account_id:0x%"WHINT64PRFX"X,instance_id:0x%"WHINT64PRFX"X,action:%d%s"
						, pRst->nAccountID, pRst->nInstanceID, pRst->nAction, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG:
				{
					STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T*)pGameCmd;
					if (pRst->nRst != STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T::RST_OK)
					{
						printf("alli_instance_combat_log:rst:%d%s", pRst->nRst, WHLINEEND);
						return 0;
					}
					AlliInstanceCombatLog*	pLog	= (AlliInstanceCombatLog*)wh_getptrnexttoptr(pRst);
					printf("alli_instance_combat_log:combat_type:%d,result:%d,account_id:0x%"WHINT64PRFX"X,obj_id:0x%"WHINT64PRFX"X"
						",randseed:%d,attack_hero_num:%d,defense_hero_num:%d,loot_num:%d%s"
						, pLog->nCombatType, pLog->nCombatResult, pLog->nAccountID, pLog->nObjID, pLog->nRandSeed
						, pLog->nAttackHeroNum, pLog->nDefenseHeroNum, pLog->nLootNum, WHLINEEND);
					void*	pPos		= wh_getptrnexttoptr(pLog);
					AlliInstanceCombatLog::HeroUnit*	pHero	= (AlliInstanceCombatLog::HeroUnit*)pPos;
					for (int i=0; i<pLog->nAttackHeroNum; ++i)
					{
						printf("ATTACK_HERO:row:%d,col:%d,char_name:%s,hero_name:%s,hero_id:0x%"WHINT64PRFX"X,level:%d,prof:%d,army_type:%d"
							",army_level:%d,army_num:%d,army_death_num:%d,army_restore_num:%d%s"
							, pHero->nRow, pHero->nCol, pHero->szCharName, pHero->szHeroName, pHero->nHeroID, pHero->nHeroLevel, pHero->nProf
							, pHero->nArmyType, pHero->nArmyLevel, pHero->nArmyNum, pHero->nArmyDeathNum, pHero->nArmyRestoreNum, WHLINEEND);
						pHero++;
						pPos			= (void*)pHero;
					}
					pHero				= (AlliInstanceCombatLog::HeroUnit*)pHero;
					for (int i=0; i<pLog->nDefenseHeroNum; ++i)
					{
						printf("DEFENSE_HERO:row:%d,col:%d,char_name:%s,hero_name:%s,hero_id:0x%"WHINT64PRFX"X,level:%d,prof:%d,army_type:%d"
							",army_level:%d,army_num:%d,army_death_num:%d,army_restore_num:%d%s"
							, pHero->nRow, pHero->nCol, pHero->szCharName, pHero->szHeroName, pHero->nHeroID, pHero->nHeroLevel, pHero->nProf
							, pHero->nArmyType, pHero->nArmyLevel, pHero->nArmyNum, pHero->nArmyDeathNum, pHero->nArmyRestoreNum, WHLINEEND);
						pHero++;
						pPos			= (void*)pHero;
					}
					AlliInstanceCombatLog::LootUnit*	pLoot	= (AlliInstanceCombatLog::LootUnit*)pPos;
					for (int i=0; i<pLog->nLootNum; ++i)
					{
						printf("LOOT:type:%d,num:%d,ext_data:%d%s", pLoot->nType, pLoot->nNum, pLoot->nExtData, WHLINEEND);
						pLoot++;
					}
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_BACK:
				{
					STC_GAMECMD_ALLI_INSTANCE_BACK_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_BACK_T*)pGameCmd;
					printf("alli_instance_back:combat_type:%d,obj_id:0x%"WHINT64PRFX"X,instance_destroyed:%d,instance_level:%d%s"
						, pRst->nCombatType, pRst->nObjID, (int)pRst->bInstanceDestroyed, pRst->nInstanceLevel, WHLINEEND);
				}
				break;
			case STC_GAMECMD_HERO_SIMPLE_DATA:
				{
					STC_GAMECMD_HERO_SIMPLE_DATA_T*	pRst	= (STC_GAMECMD_HERO_SIMPLE_DATA_T*)pGameCmd;
					
					printf("simple_hero_data:combat_type:%d,reason:%d,num:%d%s", pRst->nCombatType, pRst->nReason, pRst->nNum, WHLINEEND);
					HeroSimpleData*		pHeros	= (HeroSimpleData*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						HeroSimpleData&	hero	= pHeros[i];
						printf("hero_id:0x%"WHINT64PRFX"X,level:%d,status:%d,prof:%d,army_type:%d,army_level:%d,army_num:%d,exp:%d,health:%d%s"
							, hero.nHeroID, hero.nLevel, hero.nStatus, hero.nProf
							, hero.nArmyType, hero.nArmyLevel, hero.nArmyNum, hero.nExp
							, hero.nHealthState, WHLINEEND);
					}
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG:
				{
					STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*)pGameCmd;
					printf("simple_combat_log:combat_type:%d,combat_result:%d,instance_id:0x%"WHINT64PRFX"X,back_left_time:%d,instance_level:%d%s"
						, pRst->nCombatType, pRst->nCombatResult, pRst->nInstanceID, pRst->nBackLeftTime, pRst->nInstanceLevel, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT:
				{
					STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T*)pGameCmd;
					printf("stop_combat:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_SUPPLY:
				{
					STC_GAMECMD_ALLI_INSTANCE_SUPPLY_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_SUPPLY_T*)pGameCmd;
					printf("supply:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_GET_LOOT:
				{
					STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*)pGameCmd;
					printf("alli_instance_get_loot:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::LootUnit*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::LootUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("excel_id:%d,num:%d%s", pUnit->nExcelID, pUnit->nNum, WHLINEEND);

						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY:
				{
					STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*	pRst	= (STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*)pGameCmd;
					printf("instance_get_defender_deploy:rst:%d,excel_id:%d,level:%d,num:%d%s", pRst->nRst, pRst->nExcelID, pRst->nLevel, pRst->nNum, WHLINEEND);
					STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T::ArmyUnit*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T::ArmyUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("row:%d,col:%d,type:%d,force:%d%s", pUnit->nRow, pUnit->nCol, pUnit->nArmyType, pUnit->nForce, WHLINEEND);
						pUnit++;
					}
				}
				break;

			case STC_GAMECMD_ARENA_UPLOAD_DATA:
				{
					STC_GAMECMD_ARENA_UPLOAD_DATA_T*	pRst	= (STC_GAMECMD_ARENA_UPLOAD_DATA_T*)pGameCmd;
					printf("arena_upload_data:rst:%d,rank:%d,first_upload:%d%s"
						, pRst->nRst, pRst->nRank, (int)pRst->bFirstUpload, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ARENA_GET_STATUS:
				{
					STC_GAMECMD_ARENA_GET_STATUS_T*		pRst	= (STC_GAMECMD_ARENA_GET_STATUS_T*)pGameCmd;
					printf("arena_get_status:rst:%d,num:%d,up_cd:%d,challenge_left:%d,pay_times_left:%d,rank:%d%s"
						,pRst->nRst, pRst->nNum, pRst->nUploadTimeLeft, pRst->nChallengeTimesLeft, pRst->nPayTimesLeft
						,pRst->nRank, WHLINEEND);
					STC_GAMECMD_ARENA_GET_STATUS_T::ChallengeListUnit*	pUnit	= (STC_GAMECMD_ARENA_GET_STATUS_T::ChallengeListUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("account_id:0x%"WHINT64PRFX"X,name:%s,head_id:%d,level:%d,total_force:%d,rank:%d%s"
							, pUnit->nAccountID, pUnit->szName, pUnit->nHeadID, pUnit->nLevel, pUnit->nTotalForce, pUnit->nRank, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_ARENA_PAY:
				{
					STC_GAMECMD_ARENA_PAY_T*	pRst	= (STC_GAMECMD_ARENA_PAY_T*)pGameCmd;
					printf("arena_pay:rst:%d,money_type:%d,price:%d%s"
						, pRst->nRst, pRst->nMoneyType, pRst->nPrice, WHLINEEND);
				}
				break;
			case STC_GAMECMD_ARENA_GET_RANK_LIST:
				{
					STC_GAMECMD_ARENA_GET_RANK_LIST_T*		pRst	= (STC_GAMECMD_ARENA_GET_RANK_LIST_T*)pGameCmd;
					printf("arena_get_rank_list:rst:%d,num:%d,page:%d,total_num:%d%s"
						,pRst->nRst, pRst->nNum, pRst->nPage, pRst->nTotalNum, WHLINEEND);
					STC_GAMECMD_ARENA_GET_RANK_LIST_T::RankListUnit*	pUnit	= (STC_GAMECMD_ARENA_GET_RANK_LIST_T::RankListUnit*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("account_id:0x%"WHINT64PRFX"X,name:%s,head_id:%d,level:%d,total_force:%d,rank:%d,guild_name:%s%s"
							, pUnit->nAccountID, pUnit->szName, pUnit->nHeadID, pUnit->nLevel, pUnit->nTotalForce, pUnit->nRank, pUnit->szAllianceName, WHLINEEND);
						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_ARENA_GET_DEPLOY:
				{
					STC_GAMECMD_ARENA_GET_DEPLOY_T*	pRst	= (STC_GAMECMD_ARENA_GET_DEPLOY_T*)pGameCmd;
					printf("arena_get_deploy:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					ArenaHeroDeploy*	pUnit	= (ArenaHeroDeploy*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("row:%d,col:%d,hero_id:0x%"WHINT64PRFX"X,prof:%d,army_type:%d,army_level:%d,army_num:%d%s"
							, pUnit->nRow, pUnit->nCol, pUnit->nHeroID, pUnit->nProf
							, pUnit->nArmyType, pUnit->nArmyLevel, pUnit->nArmyNum, WHLINEEND);

						pUnit++;
					}
				}
				break;

			case STC_GAMECMD_POSITION_MARK_ADD_RECORD:
				{
					STC_GAMECMD_POSITION_MARK_ADD_RECORD_T*	pRst	= (STC_GAMECMD_POSITION_MARK_ADD_RECORD_T*)pGameCmd;
					printf("position_record_add_record:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_POSITION_MARK_CHG_RECORD:
				{
					STC_GAMECMD_POSITION_MARK_CHG_RECORD_T*	pRst	= (STC_GAMECMD_POSITION_MARK_CHG_RECORD_T*)pGameCmd;
					printf("position_record_chg_record:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			case STC_GAMECMD_POSITION_MARK_GET_RECORD:
				{
					STC_GAMECMD_POSITION_MARK_GET_RECORD_T*	pRst	= (STC_GAMECMD_POSITION_MARK_GET_RECORD_T*)pGameCmd;
					printf("position_record_get_record:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
					PositionMark*	pUnit	= (PositionMark*)wh_getptrnexttoptr(pRst);
					for (int i=0; i<pRst->nNum; i++)
					{
						printf("pos_x:%d,pos_y:%d,head_id:%d,prompt:%s%s"
							, pUnit->nPosX, pUnit->nPosY, pUnit->nHeadID, wh_getptrnexttoptr(pUnit), WHLINEEND);

						pUnit++;
					}
				}
				break;
			case STC_GAMECMD_POSITION_MARK_DEL_RECORD:
				{
					STC_GAMECMD_POSITION_MARK_DEL_RECORD_T*	pRst	= (STC_GAMECMD_POSITION_MARK_DEL_RECORD_T*)pGameCmd;
					printf("position_record_del_record:rst:%d%s", pRst->nRst, WHLINEEND);
				}
				break;
			}
		}
		break;
	case TTY_LPGAMEPLAY_CLIENT_NOTIFY:
		{
			TTY_LPGAMEPLAY_CLIENT_NOTIFY_T*	pNotify	= (TTY_LPGAMEPLAY_CLIENT_NOTIFY_T*)pCmd;
			switch (pNotify->nWhat)
			{
			case WEB_NOTIFY_WHAT_OTHERTRYREPLACE:
				{
					TTY_LPGAMEPLAY_CLIENT_NOTIFY_OTHERTRYREPLACE_T*	pReplaceNotify	= (TTY_LPGAMEPLAY_CLIENT_NOTIFY_OTHERTRYREPLACE_T*)pNotify;
					printf("account login in other place:%s%s", cmn_get_IP(pReplaceNotify->nFromIP), WHLINEEND);
				}
				break;
			}
		}
		break;
	}

	return 0;
}

