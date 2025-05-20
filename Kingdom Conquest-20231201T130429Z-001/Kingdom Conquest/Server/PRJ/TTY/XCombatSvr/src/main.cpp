#include "WHCMN/inc/whdir.h"
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whsignal.h"
#include "WHCMN/inc/whprocess.h"
#include "WHCMN/inc/whdaemon.h"
#include "WHCMN/inc/whtime.h"
#include "WHNET/inc/whnetcmn.h"
#include "WHNET/inc/whnetudpGLogger.h"
#include "DIA/inc/st_mysql_query.h"

#include "../inc/CombatLogic.h"

using namespace n_whcmn;
using namespace n_whnet;

bool	g_bStop = false;
void	stopfunc(int sig)
{
	switch(sig)
	{
	case	SIGINT:
	case	SIGTERM:
		g_bStop	= true;
		break;
	}
}

char	g_szCFG[WH_MAX_PATH]		= "combatsvr_cfg.txt";
struct	CMN_CFGINFO_T		: public whdataini::obj
{
	bool	bDaemon;
	bool	bSupportMT;
	char	szLoggerAddr[WHNET_MAXADDRSTRLEN];
	int		nKillWaitTimeOut;
	int		nSleepInterval;
	int		nCheckCFGInterval;
	char	szSvrName[64];
	char	szPIDFile[WH_MAX_PATH];									

	CMN_CFGINFO_T()
		: bDaemon(true)
		, nKillWaitTimeOut(10*60*1000)
		, nSleepInterval(200)
		, nCheckCFGInterval(10*1000)
		, bSupportMT(false)
	{
		szLoggerAddr[0]		= 0;
		szPIDFile[0]		= 0;
		strcpy(szSvrName, "XCombatSvr");
	}
	inline void	MakeDftPIDFIle()
	{
		WH_STRNCPY0(szPIDFile, "/tmp/XCombatSvr.pid");
	}
	WHDATAPROP_SETVALFUNC_DECLARE(ENV)								// 设置环境变量
	{
		char	szEnvKey[256]	= "";
		char	szEnvVal[1024]	= "";
		wh_strsplit("sa", cszVal, ",", szEnvKey, szEnvVal);
		wh_setenv(szEnvKey, szEnvVal);
		return	0;
	}

	// 定义为有成员映射的结构
	WHDATAPROP_DECLARE_MAP(CMN_CFGINFO_T)
}	g_cmn_cfginfo;
WHDATAPROP_MAP_BEGIN_AT_ROOT(CMN_CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nKillWaitTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCheckCFGInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bDaemon, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bSupportMT, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szLoggerAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szPIDFile, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szSvrName, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSleepInterval, 0)
	WHDATAPROP_ON_SETVALFUNC_reload1(ENV)
WHDATAPROP_MAP_END()

int main(int argc, char *argv[])
{
	whdir_SetCWDToExePath();

	enum
	{
		ACTION_RUN		= 0,
		ACTION_STOP		= 1,
	};
	int	nAction			= ACTION_RUN;
	if (argc >= 2)
	{
		if (strcmp(argv[1], "-stop") == 0)
		{
			nAction		= ACTION_STOP;
		}
	}

	// 配置文件分析
	WHDATAINI_CMN	ini;
	ini.addobj("CMN", &g_cmn_cfginfo);
	int	nRst		= ini.analyzefile(g_szCFG);
	if (nRst < 0)
	{
		printf("Can not analyze cfgfile:%s rst:%d %s%s", g_szCFG, nRst, ini.printerrreport(), WHLINEEND);
		return -1;
	}

	if(g_cmn_cfginfo.szPIDFile[0] == 0)
	{
		g_cmn_cfginfo.MakeDftPIDFIle();
	}
	switch( nAction )
	{
	case	ACTION_STOP:
		{
			// 尝试停止服务器
			return	whprocess_terminatebypidfile(g_cmn_cfginfo.szPIDFile, false, g_cmn_cfginfo.nKillWaitTimeOut);
		}
		break;
	}

#ifdef	WIN32
	SetConsoleTitle(g_cmn_cfginfo.szSvrName);
#endif
	// 设置结束信号处理器
	whsingal_set_exitfunc(stopfunc);

	// 尝试删除旧pid文件,如果失败则不继续
	nRst	= whprocess_trydelpidfile(g_cmn_cfginfo.szPIDFile);
	if (nRst < 0)
	{
		printf("Old server might be running. err code:%d%s", nRst, WHLINEEND);
		return -2;
	}
	else if (nRst == 1)
	{
		printf("Old pid file %s deleted. There might be a previous crash.%s", g_cmn_cfginfo.szPIDFile, WHLINEEND);
	}

	// 网络初始化
	cmn_ir_obj		cir;

	// 变成daemon
	if (g_cmn_cfginfo.bDaemon)
	{
		whdaemon_init();
	}

	// 创建pid文件
	whprocess_pidfile	wpf(g_cmn_cfginfo.szPIDFile);

	// 日志初始化
	GLOGGER2_INIT();
	GLOGGER2_INFO_T	info;
	strcpy(info.szExeTag, g_cmn_cfginfo.szSvrName);
	info.bSupportMT		= g_cmn_cfginfo.bSupportMT;
	nRst		= GLOGGER2_ADDMULTI(&info, g_cmn_cfginfo.szLoggerAddr);
	if (nRst < 0)
	{
		printf("GLOGGER2_ADDMULTI %s ERROR:%d%s", g_cmn_cfginfo.szLoggerAddr, nRst, WHLINEEND);
		GLOGGER2_RELEASE();
		return -3;
	}

	// 设置WHCMN的log函数
	WHCMN_Set_Log_WriteFmt(&GLOGGER2_WRITEFMT);

	// 配置文件检测
	whlooper	wlCheckCFG;
	wlCheckCFG.setinterval(g_cmn_cfginfo.nCheckCFGInterval);

	CCombatLogic	logic;
	if ((nRst=logic.Init(g_szCFG)) < 0)
	{
		printf("CCombatLogic.Init,%d%s", nRst, WHLINEEND);
		GLOGGER2_RELEASE();
		return -4;
	}

	while (!g_bStop)
	{
		if (wlCheckCFG.check())
		{
			wlCheckCFG.reset();
			ini.analyzefile(g_szCFG, false, 1);
			logic.Init_CFG_Reload(g_szCFG);
		}

		if (logic.Work() <= 0)
		{
			wh_sleep(g_cmn_cfginfo.nSleepInterval);
		}

		GLOGGER2_TICK();
	}

	GLOGGER2_RELEASE();
	return 0;
}
