#include "../inc/BD4Web_i.h"

using namespace n_pngs;

// 配置CFGINFO_T
WHDATAPROP_MAP_BEGIN_AT_ROOT(n_pngs::BD4Web_i::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(unknowntype, CNTRSVR, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bNBO, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bBDSupport, 0)
WHDATAPROP_MAP_END()

int		BD4Web_i::Organize()
{
	m_pLogicGS4Web	= m_pCMN->GetLogic(TTY_DLL_NAME_GS4Web, GS4Web_VER);
	if (m_pLogicGS4Web == NULL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BD4Web_i_INT)"%s,m_pLogicGS4Web is NULL", __FUNCTION__);
		return -1;
	}
	CmdOutToLogic_AUTO(m_pLogicGS4Web, PNGSPACKET_BD4WEB_CMD_SET_PTR, (const void*)this, sizeof(this));
	return 0;
}
int		BD4Web_i::Detach(bool bQuick)
{
	return 0;
}
BD4Web_i::BD4Web_i()
: m_tickNow(0)
, m_pLogicGS4Web(NULL)
{
	strcpy(FATHERCLASS::m_szLogicType, TTY_DLL_NAME_BD4Web);
	FATHERCLASS::m_nVer		= BD4Web_VER;
	m_CntrSvr.m_pHost	= this;
}
BD4Web_i::~BD4Web_i()
{

}
int		BD4Web_i::Init(const char *cszCFGName)
{
	m_tickNow	= wh_gettickcount();

	if (Init_CFG(cszCFGName) < 0)
	{
		return -1;
	}

	int	nRst	= 0;

	// 初始化TCPServer
	if ((nRst=m_CntrSvr.Init(m_cfginfo.CNTRSVR.GetBase())) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BD4Web_i_INIT)"%s,m_CntrSvr.Init,%d", __FUNCTION__, nRst);
		return -2;
	}

	m_vectrawbuf.reserve(MAX_BD_CMD_SIZE);

	return 0;
}
int		BD4Web_i::Init_CFG(const char* cszCFGName)
{
	// 分析文件
	WHDATAINI_CMN	ini;
	ini.addobj("BD4Web", &m_cfginfo);
	ini.addobj("MSGER_BD", &m_MSGER_INFO);
	// 设置一下默认值
	m_MSGER_INFO.nSendBufSize		= 10*1024*1024;
	m_MSGER_INFO.nRecvBufSize		= 10*1024*1024;
	//
	int	nRst	= ini.analyzefile(cszCFGName);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BD4Web_i_INT)"%s,ini.analyzefile,%d,%s", __FUNCTION__, nRst, ini.printerrreport());
		return -1;
	}

	return 0;
}
int		BD4Web_i::Init_CFG_Reload(const char* cszCFGName)
{
	// 分析文件
	WHDATAINI_CMN	ini;
	ini.addobj("BD4Web", &m_cfginfo);
	ini.addobj("MSGER_BD", &m_MSGER_INFO);
	// 设置一下默认值
	m_MSGER_INFO.nSendBufSize		= 10*1024*1024;
	m_MSGER_INFO.nRecvBufSize		= 10*1024*1024;
	//
	int	nRst	= ini.analyzefile(cszCFGName, false, 1);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BD4Web_i_INT)"%s,ini.analyzefile,%d,%s", __FUNCTION__, nRst, ini.printerrreport());
		return -1;
	}

	return 0;
}
int		BD4Web_i::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	return m_CntrSvr.GetSockets(vect);
}
int		BD4Web_i::SureSend()
{
	return m_CntrSvr.SureSend();
}
int		BD4Web_i::Tick_BeforeDealCmdIn()
{
	m_CntrSvr.DoWork(true);
	return 0;
}
int		BD4Web_i::Tick_AfterDealCmdIn()
{
	return 0;
}
int		BD4Web_i::Release()
{
	return 0;
}
