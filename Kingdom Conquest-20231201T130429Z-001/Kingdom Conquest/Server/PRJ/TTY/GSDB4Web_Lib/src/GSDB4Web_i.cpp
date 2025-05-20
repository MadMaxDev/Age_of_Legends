#include "../inc/GSDB4Web_i.h"

using namespace n_pngs;

WHDATAPROP_MAP_BEGIN_AT_ROOT(GSDB4Web_i::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueCmdInSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szDBS4WebAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSendRecvBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nConnectDBS4WebTimeout, 0)
WHDATAPROP_MAP_END()

int		GSDB4Web_i::Organize()
{
	return 0;
}

int		GSDB4Web_i::Detach(bool bQuick)
{
	return 0;
}

GSDB4Web_i::GSDB4Web_i()
: m_nTimeNow(0)
, m_tickNow(0)
, m_nAppType(APPTYPE_NOTHING)
, m_pLogicDBUser(NULL)
{
	strcpy(FATHERCLASS::m_szLogicType, TTY_DLL_NAME_GS_DB4Web);
	FATHERCLASS::m_nVer	= GSDB4Web_VER;
}

GSDB4Web_i::~GSDB4Web_i()
{
}

int		GSDB4Web_i::Init(const char *cszCFGName)
{
	m_nTimeNow		= wh_time();
	m_tickNow		= wh_gettickcount();

	int		iRst	= 0;
	if ((iRst=Init_CFG(cszCFGName)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1360,GSDB4Web_INT_RLS)"GSDB4Web_i::Init,Init_CFG,%d", iRst);
		return -1;
	}

	if ((iRst=ILogicBase::QueueInit(m_cfgInfo.nQueueCmdInSize)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1360,GSDB4Web_INT_RLS)"GSDB4Web_i::Init,QueueInit,%d,%d", iRst, m_cfgInfo.nQueueCmdInSize);
		return -2;
	}

	tcpmsger::INFO_T		msgerInfo;
	msgerInfo.nRecvBufSize		= m_cfgInfo.nSendRecvBufSize;
	msgerInfo.nSendBufSize		= m_cfgInfo.nSendRecvBufSize;
	msgerInfo.nInterval			= 0;
	m_retrymsgerDBS4Web.m_pHost	= this;
	if ((iRst=m_retrymsgerDBS4Web.Init(&msgerInfo, m_cfgInfo.szDBS4WebAddr)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1360,GSDB4Web_INT_RLS)"GSDB4Web_i::Init,m_retrymsgerDBS4Web.Init,nRst:%d,ip:%s", iRst, m_cfgInfo.szDBS4WebAddr);
		return -3;
	}

	return 0;
}

int		GSDB4Web_i::Init_CFG(const char* cszCFGName)
{
	//分析文件
	WHDATAINI_CMN		ini;
	ini.addobj("GSDB4Web", &m_cfgInfo);
	int		iRst		= ini.analyzefile(cszCFGName);
	if (iRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1360,GSDB4Web_INT_RLS)"GSDB4Web_i::Init_CFG,%d,ini.analyzefile,%s", iRst, ini.printerrreport());
		return -1;
	}
	return 0;
}

int		GSDB4Web_i::Init_CFG_Reload(const char *cszCFGName)
{
	//分析文件
	WHDATAINI_CMN		ini;
	ini.addobj("GSDB4Web", &m_cfgInfo);
	int		iRst		= ini.analyzefile(cszCFGName, false, 1);
	if (iRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1360,GSDB4Web_INT_RLS)"GSDB4Web_i::Init_CFG_Reload,%d,ini.analyzefile,%s", iRst, ini.printerrreport());
		return -1;
	}
	return 0;
}

int		GSDB4Web_i::Release()
{
	return 0;
}

int		GSDB4Web_i::SureSend()
{
	return 0;
}

int		GSDB4Web_i::Tick_BeforeDealCmdIn()
{
	m_tickNow		= wh_gettickcount();

	m_retrymsgerDBS4Web.Tick();
	const void*		pData;
	size_t			nDSize;
	while ((pData=m_retrymsgerDBS4Web.PeekMsg(&nDSize)) != NULL)
	{
		DealCmdFromDBS4Web(pData, nDSize);
		m_retrymsgerDBS4Web.FreeMsg();
	}
	return 0;
}

int		GSDB4Web_i::Tick_AfterDealCmdIn()
{
	return 0;
}

void	GSDB4Web_i::Retry_Worker_WORKING_Begin()
{
// 	P_DBS4WEB_FIRSTIN_T	cmd;
// 	cmd.nCmd		= P_DBS4WEB_FIRSTIN;
// 	cmd.ucAppType	= m_nAppType;
// 	m_retrymsgerDBS4Web.SendMsg(&cmd, sizeof(cmd));
}

int		GSDB4Web_i::ConnectToDBS4Web()
{
	whtick_t	t1	= m_tickNow;
	whtick_t	t2	= 0;
	while (m_retrymsgerDBS4Web.GetStatus() != tcpretrymsger<tcpmsger>::STATUS_WORKING)
	{
		m_retrymsgerDBS4Web.Tick();
		t2			= wh_gettickcount();
		if (wh_tickcount_diff(t2, t1) > m_cfgInfo.nConnectDBS4WebTimeout)
		{
			break;
		}
	}
	if (m_retrymsgerDBS4Web.GetStatus() != tcpretrymsger<tcpmsger>::STATUS_WORKING)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1360,GSDB4Web_INT_RLS)"GSDB4Web_i::Init,connect dbs4web timeout,t1:%d,t2:%d,ip:%s", t1, t2, m_cfgInfo.szDBS4WebAddr);
		return -1;
	}
	
// 	P_DBS4WEB_TEST_T	cmd;
// 	cmd.nCmd			= P_DBS4WEB_REQ_CMD;
// 	cmd.nSubCmd		= CMDID_TEST_REQ;
// 	cmd.timeNow			= m_nTimeNow;
// 	cmd.nParam			= 0;
// 	m_retrymsgerDBS4Web.SendMsg(&cmd, sizeof(cmd));

	CmdOutToLogic_AUTO(m_pLogicDBUser, PNGSPACKET_2DB4WEBUSER_READY4WORK, NULL, 0);

	return 0;
}
