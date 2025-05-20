#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whdaemon.h"
#include "WHCMN/inc/whcmd.h"
#include "WHCMN/inc/whdbgmem.h"
#include "WHCMN/inc/whvector.h"
#include "WHCMN/inc/whdir.h"
#include "WHCMN/inc/whsignal.h"

#include "../../Common/inc/tty_common_def.h"
#include "../../Common/inc/tty_common_BD4Web.h"
#include "../../Common/inc/tty_def_gs4web.h"

#include "../inc/StrCmdDealer.h"
#include "../inc/MyMsger.h"

using namespace n_whcmn;
using namespace n_pngs;

struct XBDClient4Web_CFG_T : public whdataini::obj 
{
	char				szIP[WHNET_MAXIPSTRLEN];
	unsigned short		nPort;
	bool				bNBO;
	char				szCharSet[64];

	XBDClient4Web_CFG_T()
	: nPort(0)
	, bNBO(true)
	{
		szIP[0]			= 0;
		WH_STRNCPY0(szCharSet, "GBK");
	}

	WHDATAPROP_DECLARE_MAP(XBDClient4Web_CFG_T)
};
WHDATAPROP_MAP_BEGIN_AT_ROOT(XBDClient4Web_CFG_T)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szIP, 0)
	WHDATAPROP_ON_SETVALUE_smp(short, nPort, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bNBO, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szCharSet, 0)
WHDATAPROP_MAP_END()

bool		g_bStop		= false;
bool		g_bNBO		= true;
MyMsger*	g_pMyMsger	= NULL;
CStrCmdDealer*	g_pStrCmdDealer	= NULL;
XBDClient4Web_CFG_T	g_BDClientCfg;

int		dealnccmd(const char* cszBuf, size_t nSize)
{
	char	cmd[256]	= "";
	char	param[MAX_BD_TEXT_LEN*2]	= "";
	wh_strsplit("sa", cszBuf, "", cmd, param);
	g_pStrCmdDealer->m_srad.CallFunc(cmd, param);
	if (g_pStrCmdDealer->m_srad.m_bShouldHelp)
	{
		g_pStrCmdDealer->_i_srad_help(cmd);
	}
	return 0;
}

int		DealBDCmd(const BD_CMD_BASE_T* pBaseCmd, size_t nSize)
{
	switch (pBaseCmd->nCmd)
	{
	case BD_CMD_ADD_NOTIFY_RPL:
		{
			BD_CMD_ADD_NOTIFY_RPL_T*	pRst	= (BD_CMD_ADD_NOTIFY_RPL_T*)pBaseCmd;
			Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRst);
			printf("add_notify reply,rst:%d,id:%d,group:%d,time:%s%s"
				, pRst->nRst, pNotification->nNotificationID, pNotification->nGroupID, wh_gettimestr(pNotification->nTime), WHLINEEND);
		}
		break;
	case BD_CMD_DEL_NOTIFY_BY_ID_RPL:
		{
			BD_CMD_DEL_NOTIFY_BY_ID_RPL_T*	pRst	= (BD_CMD_DEL_NOTIFY_BY_ID_RPL_T*)pBaseCmd;
			printf("del_notify_by_id reply,rst:%d,id:%d%s", pRst->nRst, pRst->nNotificationID, WHLINEEND);
		}
		break;
	case BD_CMD_DEL_NOTIFY_BEFORE_TIME_RPL:
		{
			BD_CMD_DEL_NOTIFY_BEFORE_TIME_RPL_T*	pRst	= (BD_CMD_DEL_NOTIFY_BEFORE_TIME_RPL_T*)pBaseCmd;
			printf("del_notify_before_time reply,rst:%d,time:%s%s", pRst->nRst, pRst->szTimeStr, WHLINEEND);
		}
		break;
	case BD_CMD_DEL_NOTIFY_AFTER_TIME_RPL:
		{
			BD_CMD_DEL_NOTIFY_AFTER_TIME_RPL_T*	pRst	= (BD_CMD_DEL_NOTIFY_AFTER_TIME_RPL_T*)pBaseCmd;
			printf("del_notify_after_time reply,rst:%d,time:%s%s", pRst->nRst, pRst->szTimeStr, WHLINEEND);
		}
		break;
	case BD_CMD_DEL_NOTIFY_ALL_RPL:
		{
			BD_CMD_DEL_NOTIFY_ALL_RPL_T*	pRst	= (BD_CMD_DEL_NOTIFY_ALL_RPL_T*)pBaseCmd;
			printf("del_notify_all reply,rst:%d%s", pRst->nRst, WHLINEEND);
		}
		break;
	case BD_CMD_DEL_NOTIFY_BY_GROUP_RPL:
		{
			BD_CMD_DEL_NOTIFY_BY_GROUP_RPL_T*	pRst	= (BD_CMD_DEL_NOTIFY_BY_GROUP_RPL_T*)pBaseCmd;
			printf("del_notify_by_group reply,rst:%d,group_id:%d%s", pRst->nRst, pRst->nGroupID, WHLINEEND);
		}
		break;
	case BD_CMD_GET_NOTIFY_BY_ID_RPL:
		{
			BD_CMD_GET_NOTIFY_BY_ID_RPL_T*	pRst	= (BD_CMD_GET_NOTIFY_BY_ID_RPL_T*)pBaseCmd;
			printf("get_notify_by_id:rst:%d%s", pRst->nRst, WHLINEEND);
			if (pRst->nRst == BD_RST_OK)
			{
				Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRst);
				char*	pTitle		= (char*)wh_getptrnexttoptr(pNotification);
				int*	pContentLen	= (int*)wh_getoffsetaddr(pTitle, pNotification->nTitleLen);
				char*	pContent	= (char*)wh_getptrnexttoptr(pContentLen);
				printf("id:%d,group:%d,time:%s,title:%s,content:%s%s"
					, pNotification->nNotificationID, pNotification->nGroupID, wh_gettimestr(pNotification->nTime), pTitle, pContent, WHLINEEND);
			}
		}
		break;
	case BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL:
		{
			BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL_T*	pRst	= (BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL_T*)pBaseCmd;
			printf("get_notify_bt:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
			Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				char*	pTitle		= (char*)wh_getptrnexttoptr(pNotification);
				int*	pContentLen	= (int*)wh_getoffsetaddr(pTitle, pNotification->nTitleLen);
				char*	pContent	= (char*)wh_getptrnexttoptr(pContentLen);
				printf("id:%d,group:%d,time:%s,title:%s,content:%s%s"
					, pNotification->nNotificationID, pNotification->nGroupID, wh_gettimestr(pNotification->nTime), pTitle, pContent, WHLINEEND);

				pNotification		= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
			}
		}
		break;
	case BD_CMD_GET_NOTIFY_AFTER_TIME_RPL:
		{
			BD_CMD_GET_NOTIFY_AFTER_TIME_RPL_T*	pRst	= (BD_CMD_GET_NOTIFY_AFTER_TIME_RPL_T*)pBaseCmd;
			printf("get_notify_at:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
			Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				char*	pTitle		= (char*)wh_getptrnexttoptr(pNotification);
				int*	pContentLen	= (int*)wh_getoffsetaddr(pTitle, pNotification->nTitleLen);
				char*	pContent	= (char*)wh_getptrnexttoptr(pContentLen);
				printf("id:%d,group:%d,time:%s,title:%s,content:%s%s"
					, pNotification->nNotificationID, pNotification->nGroupID, wh_gettimestr(pNotification->nTime), pTitle, pContent, WHLINEEND);

				pNotification		= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
			}
		}
		break;
	case BD_CMD_GET_NOTIFY_ALL_RPL:
		{
			BD_CMD_GET_NOTIFY_ALL_RPL_T*	pRst	= (BD_CMD_GET_NOTIFY_ALL_RPL_T*)pBaseCmd;
			printf("get_notify_all:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
			Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				char*	pTitle		= (char*)wh_getptrnexttoptr(pNotification);
				int*	pContentLen	= (int*)wh_getoffsetaddr(pTitle, pNotification->nTitleLen);
				char*	pContent	= (char*)wh_getptrnexttoptr(pContentLen);
				printf("id:%d,group:%d,time:%s,title:%s,content:%s%s"
					, pNotification->nNotificationID, pNotification->nGroupID, wh_gettimestr(pNotification->nTime), pTitle, pContent, WHLINEEND);

				pNotification		= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
			}
		}
		break;
	case BD_CMD_GET_NOTIFY_BY_GROUP_RPL:
		{
			BD_CMD_GET_NOTIFY_BY_GROUP_RPL_T*	pRst	= (BD_CMD_GET_NOTIFY_BY_GROUP_RPL_T*)pBaseCmd;
			printf("get_notify_by_group:rst:%d,num:%d%s", pRst->nRst, pRst->nNum, WHLINEEND);
			Web_Notification_T*	pNotification	= (Web_Notification_T*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				char*	pTitle		= (char*)wh_getptrnexttoptr(pNotification);
				int*	pContentLen	= (int*)wh_getoffsetaddr(pTitle, pNotification->nTitleLen);
				char*	pContent	= (char*)wh_getptrnexttoptr(pContentLen);
				printf("id:%d,group:%d,time:%s,title:%s,content:%s%s"
					, pNotification->nNotificationID, pNotification->nGroupID, wh_gettimestr(pNotification->nTime), pTitle, pContent, WHLINEEND);

				pNotification		= (Web_Notification_T*)wh_getoffsetaddr(pNotification, pNotification->GetTotalSize());
			}
		}
		break;
	case BD_CMD_RECHARGE_DIAMOND_RPL:
		{
			BD_CMD_RECHARGE_DIAMOND_RPL_T*	pRst	= (BD_CMD_RECHARGE_DIAMOND_RPL_T*)pBaseCmd;
			printf("recharge_diamond:rst:%d,group:%d,account_id:0x%"WHINT64PRFX"X,added_diamond:%d,vip:%d%s", pRst->nRst, pRst->nGroupID, pRst->nAccountID, pRst->nAddedDiamond, pRst->nVip, WHLINEEND);
		}
		break;
	case BD_CMD_RELOAD_EXCEL_TABLE_RPL:
		{
			BD_CMD_RELOAD_EXCEL_TABLE_RPL_T*	pRst	= (BD_CMD_RELOAD_EXCEL_TABLE_RPL_T*)pBaseCmd;
			printf("reload_excel_table:rst:%d,group_id:%d%s", pRst->nRst, pRst->nGroupID, WHLINEEND);
		}
		break;
	case BD_CMD_BAN_ACCOUNT_RPL:
		{
			BD_CMD_BAN_ACCOUNT_RPL_T*	pRst	= (BD_CMD_BAN_ACCOUNT_RPL_T*)pBaseCmd;
			printf("ban_account:rst:%d,account_id:0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nAccountID, WHLINEEND);
		}
		break;
	case BD_CMD_KICK_CHAR_RPL:
		{
			BD_CMD_KICK_CHAR_RPL_T*	pRst	= (BD_CMD_KICK_CHAR_RPL_T*)pBaseCmd;
			printf("kick_char:rst:%d,group_id:%d,account_id:0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nGroupID, pRst->nAccountID, WHLINEEND);
		}
		break;
	case BD_CMD_BAN_CHAR_RPL:
		{
			BD_CMD_BAN_CHAR_RPL_T*	pRst	= (BD_CMD_BAN_CHAR_RPL_T*)pBaseCmd;
			printf("ban_char:rst:%d,group_id:%d,account_id:0x%"WHINT64PRFX"X%s", pRst->nRst, pRst->nGroupID, pRst->nAccountID, WHLINEEND);
		}
		break;
	case BD_CMD_KICK_CLIENT_ALL_RPL:
		{
			BD_CMD_KICK_CLIENT_ALL_RPL_T*	pRst	= (BD_CMD_KICK_CLIENT_ALL_RPL_T*)pBaseCmd;
			printf("kick_client_all:rst:%d,group_id:%d%s", pRst->nRst, pRst->nGroupID, WHLINEEND);
		}
		break;

	case BD_CMD_GROUP_NOT_EXIST_RPL:
		{
			BD_CMD_GROUP_NOT_EXIST_RPL_T*	pRst	= (BD_CMD_GROUP_NOT_EXIST_RPL_T*)pBaseCmd;
			printf("group %d not connect ok%s", pRst->nGroupID, WHLINEEND);
		}
		break;
	case BD_CMD_BD_NOT_OPEN_RPL:
		{
			printf("business dealer not open%s", WHLINEEND);
		}
		break;
	}
	return 0;
}

int		main(int argc, char* argv[])
{
	char	cszCFG[256]		= "bdclient4web_cfg.txt";
	whdir_SetCWDToExePath();
	if (argc == 2)
	{
		strcpy(cszCFG, argv[1]);
	}

	// 网络初始化
	cmn_ir_obj	netIR;

	// 配置文件分析
	WHDATAINI_CMN		ini;
	ini.addobj("XBDClient4Web_CFG", &g_BDClientCfg);

	int	nRst	= ini.analyzefile(cszCFG);
	if (nRst < 0)
	{
		printf("Can not analyze cfgfile:%s rst:%d %s%s", cszCFG, nRst, ini.printerrreport(), WHLINEEND);
		return -1;
	}

	// 设置字符集
	WHCMN_setcharset(g_BDClientCfg.szCharSet);

	// 命令处理器初始化
	g_pStrCmdDealer			= new CStrCmdDealer;

	// 通讯器生成及初始化
	g_pMyMsger				= new MyMsger;

	MyMsger::INFO_T			info;
	info.bAutoCloseSocket	= true;
	info.nRecvBufSize		= 10*1024*1024;
	info.nSendBufSize		= 10*1024*1024;
	info.sock				= tcp_create_connect_socket(g_BDClientCfg.szIP, g_BDClientCfg.nPort, 0, 10*1000);
	if (!cmn_is_validsocket(info.sock))
	{
		printf("Cannot connect to server %s:%d%s", g_BDClientCfg.szIP, (int)g_BDClientCfg.nPort, WHLINEEND);
		return -2;
	}
	if ((nRst=g_pMyMsger->Init(&info)) < 0)
	{
		printf("g_pMyMsger->Init %d%s", nRst, WHLINEEND);
		return -3;
	}

	// 控制台线程
	whcmn_nml_console	nc;
	nc.StartThread();

	whvector<char>		vectBuf;
	vectBuf.reserve(2*MAX_BD_CMD_SIZE);

	// 开始工作
	while (!g_bStop)
	{
		// 防止空转
		wh_sleep(10);

		// 指令处理
		//////////////////////////////////////////////////////////////////////////
		// console的指令
		//////////////////////////////////////////////////////////////////////////
		size_t	nSize	= vectBuf.capacity();
		while (nc.m_cmdqueue.Out(vectBuf.getbuf(), &nSize) == 0)
		{
			nc.m_cmdqueue.lock();
			// 处理指令
			if (dealnccmd(vectBuf.getbuf(), nSize) < 0)
			{
				printf("deal cmd error:%s%s", vectBuf.getbuf(), WHLINEEND);
				nc.m_cmdqueue.unlock();
				break;
			}
			nc.m_cmdqueue.unlock();
			// 下一条
			size_t	nSize	= vectBuf.capacity();
		}
		//////////////////////////////////////////////////////////////////////////
		// 网络来的
		//////////////////////////////////////////////////////////////////////////
		g_pMyMsger->ManualRecvAndSend();
		const BD_CMD_BASE_T*	pCmd	= NULL;
		while ((pCmd=(const BD_CMD_BASE_T*)g_pMyMsger->PeekMsg(&nSize)) != NULL)
		{
			DealBDCmd(pCmd, nSize);
			g_pMyMsger->FreeMsg();
		}
	}

	WHSafeDelete(g_pMyMsger);

	return 0;
}
