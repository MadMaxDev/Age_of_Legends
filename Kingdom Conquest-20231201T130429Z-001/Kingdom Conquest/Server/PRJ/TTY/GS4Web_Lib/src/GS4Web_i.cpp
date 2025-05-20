#include "../inc/GS4Web_i.h"

using namespace n_pngs;

WHDATAPROP_MAP_BEGIN_AT_ROOT(GS4Web_i::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nCmdQueueSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nStatisticOnlineInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nLoadNotificationTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nNotificationNum, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nQueryGroupInfoTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nWriteGroupInfoTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szGroupInfoFileName, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szGDBAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nDealAdColonyTimeOut, 0)
WHDATAPROP_MAP_END()

int		GS4Web_i::Organize()
{
	// 查找TCPReceiver并设置自己为它的使用者
	m_pLogicTCPReceiver		= m_pCMN->GetLogic(PNGS_DLL_NAME_TCPRECEIVER, TCPRECEIVER_VER);
	if (m_pLogicTCPReceiver == NULL)
	{
		return -1;
	}
	PNGS_CD2TR_CONFIG_T	cfg;
	cfg.nSubCmd				= PNGS_CD2TR_CONFIG_T::SUBCMD_SET_CMDDEALER;
	if (m_pLogicTCPReceiver->CmdIn_Instant(this, PNGS_CD2TR_CONFIG, &cfg, sizeof(cfg)) < 0)
	{
		return -2;
	}

	return 0;
}
int		GS4Web_i::Detach(bool bQuick)
{
	return 0;
}
GS4Web_i::GS4Web_i()
: m_pLogicTCPReceiver(NULL)
, m_pLogicBD4Web(NULL)
, m_bLoadNotificationOK(false)
, m_pNotificationMngS(NULL)
{
	strcpy(FATHERCLASS::m_szLogicType, TTY_DLL_NAME_GS4Web);
	FATHERCLASS::m_nVer		= GS4Web_VER;
}
GS4Web_i::~GS4Web_i()
{

}
int		GS4Web_i::Init(const char *cszCFGName)
{
	m_nTimeNow		= wh_time();
	m_tickNow		= wh_gettickcount();

	m_vectConnecters.reserve(64);

	m_pNotificationMngS		= new CNotificationMngS();

	int		nRst	= 0;
	if ((nRst=Init_CFG(cszCFGName)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_INT)"%s,Init_CFG,%d", __FUNCTION__, nRst);
		return -1;
	}

	if ((nRst=ILogicBase::QueueInit(m_cfginfo.nCmdQueueSize)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_INT)"%s,%d,ILogicBase::QueueInit,%d", __FUNCTION__, nRst, m_cfginfo.nCmdQueueSize);
		return -4;
	}

	whtimequeue::INFO_T	info;
	info.nChunkSize		= m_cfginfo.nTQChunkSize;
	info.nUnitLen		= sizeof(TQUNIT_T);
	if ((nRst=m_TQ.Init(&info)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_INT)"%s,%d,m_TQ.Init,%d,%d", __FUNCTION__, nRst, info.nChunkSize, info.nUnitLen);
		return -100;
	}

	if ((nRst=m_msgerDB.Init(m_msgerDB_info.GetBase(), m_cfginfo.szGDBAddr)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_INT)"%s,%d,m_msgerDB.Init,%s", __FUNCTION__, nRst, m_cfginfo.szGDBAddr);
		return -101;
	}

	// 载入通知到内存
	ReqLoadNotification();

	// 大区信息相关
	SetTE_QueryGroupInfo();
	SetTE_WriteGroupInfo();

	SetTE_DealAdColonyTimeEvent();

	return 0;
}
int		GS4Web_i::Init_CFG(const char* cszCFGName)
{
	// 分析文件
	WHDATAINI_CMN	ini;
	ini.addobj("GS4Web", &m_cfginfo);
	ini.addobj("MSGER_GDB", &m_msgerDB_info);
	// 默认参数
	m_msgerDB_info.nSendBufSize			= 1*1024*1024;
	m_msgerDB_info.nRecvBufSize			= 1*1024*1024;
	m_msgerDB_info.nSockRcvBufSize		= 512*1024;
	m_msgerDB_info.nSockSndBufSize		= 512*1024;

	int		nRst	= ini.analyzefile(cszCFGName);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_INT)"%s,%d,ini.analyzefile,%s", __FUNCTION__, nRst, ini.printerrreport());
		return -1;
	}

	return 0;
}
int		GS4Web_i::Init_CFG_Reload(const char *cszCFGName)
{
	// 分析文件
	WHDATAINI_CMN	ini;
	ini.addobj("GS4Web", &m_cfginfo);
	int		nRst	= ini.analyzefile(cszCFGName, false, 1);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GS4Web_i_INT)"%s,%d,ini.analyzefile,%s", __FUNCTION__, nRst, ini.printerrreport());
		return -1;
	}
	return 0;
}
int		GS4Web_i::Release()
{
	m_msgerDB.Release();
	return 0;
}
int		GS4Web_i::SureSend()
{
	m_msgerDB.ManualSend();
	return 0;
}
int		GS4Web_i::Tick_BeforeDealCmdIn()
{
	if (m_nMood == CMN::ILogic::MOOD_STOPPED)
	{
		return 0;
	}

	// DB相关
	m_msgerDB.Tick();
	Tick_DealGDB4WebMsg();

	// 时间事件
	Tick_DealTE();

	return 0;
}
int		GS4Web_i::Tick_AfterDealCmdIn()
{
	return 0;
}
int		GS4Web_i::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	SOCKET sock = m_msgerDB.GetSocket();
	if (cmn_is_validsocket(sock))
	{
		vect.push_back(sock);
	}
	return vect.size();
}
int		GS4Web_i::SendNotificationToGroup(int nGroupID)
{
	whvector<Web_Notification_T*>		vect;
	m_pNotificationMngS->GetNotificationByGroup(nGroupID, vect);
	m_pNotificationMngS->GetNotificationByGroup(GROUP_ID_ALL_WITHOUT_GLOBAL, vect);
	m_pNotificationMngS->GetNotificationByGroup(GROUP_ID_ALL, vect);

	int		nTotalSize						= sizeof(GS4WEB_GROUP_LOAD_NOTIFY_REQ_T);
	for (unsigned int i=0; i<vect.size(); i++)
	{
		nTotalSize	+= vect[i]->GetTotalSize();
	}
	m_vectrawbuf.resize(nTotalSize);
	GS4WEB_GROUP_LOAD_NOTIFY_REQ_T*	pCmd	= (GS4WEB_GROUP_LOAD_NOTIFY_REQ_T*)m_vectrawbuf.getbuf();
	pCmd->nCmd		= P_GS4WEB_GROUP_REQ_CMD;
	pCmd->nSubCmd	= GROUP_LOAD_NOTIFY_REQ;
	pCmd->nNum		= vect.size();
	Web_Notification_T*	pNotify				= (Web_Notification_T*)wh_getptrnexttoptr(pCmd);
	for (int i=0; i<pCmd->nNum; i++)
	{
		int	nNotificationLen		= vect[i]->GetTotalSize();
		memcpy(pNotify, vect[i], nNotificationLen);
		pNotify						= (Web_Notification_T*)wh_getoffsetaddr(pNotify, nNotificationLen);
	}
	SendMsgToGroup(nGroupID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

	return 0;
}
int		GS4Web_i::ReqLoadNotification()
{
	m_bLoadNotificationOK			= false;

	GS4WEB_DB_LOAD_NOTIFY_REQ_T		Cmd;
	Cmd.nCmd						= P_GS4WEB_DB_REQ_CMD;
	Cmd.nSubCmd						= DB_LOAD_NOTIFY_REQ;
	m_msgerDB.SendMsg(&Cmd, sizeof(Cmd));
	return 0;
}
