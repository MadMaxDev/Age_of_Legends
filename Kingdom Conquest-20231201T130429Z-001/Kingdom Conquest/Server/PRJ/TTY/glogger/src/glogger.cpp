// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pnldb
// File         : glogger.cpp
// Creator      : Wei Hua (魏华)
// Comment      : log服务器程序
// CreationDate : 2003-10-22
// ChangeLog    : 2004-07-16 确定为V1.0版本。
//                2004-08-31 升级为V1.01。将接收缓冲改为32768。
//                2004-09-01 LoggerUnit的nLastKATick改为只要收到消息就更新
//                2004-09-21 升级为V1.02。增加了来包长度判断。
//                2004-09-23 修正了whlogwriter::RawPrintf中对空文件指针的判断
//                2005-01-11 升级为V1.03。增加了日期切换时的记录。为了查找日期变换但是文件不变之谜。
//                           修改了日期切换的机制（原来是每次比较，这次直接用了一个crontab）
//                2005-01-11 升级为V1.04。修正了每天一个文件时在文件超长后每个tick都打开关闭的问题。
//                2005-03-08 升级为V1.05。增加了丢包检查的功能。
//                2005-08-08 升级为V1.06。重大调整：把whunitallocatorFixed的浏览方式修改为iterator。
//                2005-08-22 升级为V1.07。修正了因为使用iterator且循环中有删除操作而可能导致内存访问出错的bug。
//                2005-08-31 升级为V1.08。应sohu要求把时间长度缩短到只有日。
//                2006-03-27 升级为V1.09。在windows下把标题栏设置为exe+config文件，这样容易看清楚日志程序的功能。
//                2007-03-01 升级为V1.10。修改为只读一个文件了。
//                2007-04-17 升级为V1.11。增加了日志长度超过危险数值也强制。
//                2007-04-20 升级为V1.12。重新设计了生成新文件的方式。OFOD还是一天只有一个文件。否则文件超过一定限度就自动开启新文件，自动加时刻或者是文件的当天序号。
//                2007-05-30 升级为V1.13。增加了每隔整数时间都生成新文件的设置。（ps.股市大跌啊。哈哈）

static const char	*VERSTR	= "V1.13";

static const char	*HISVER[][2]	=
{
	{"V1.0"
	, "\r\n(2004-07-16)确定为V1.0版本。"
	},
	{"V1.01"
	, "\r\n(2004-08-31)将接收缓冲改为32768。"
	  "\r\n(2004-09-01)LoggerUnit的nLastKATick改为只要收到消息就更新"
	},
	{"V1.02"
	, "\r\n(2004-09-21)增加了来包长度判断。"
      "\r\n(2004-09-23)修正了whlogwriter::RawPrintf中对空文件指针的判断。"
	},
	{"V1.03"
	, "\r\n(2005-01-11)增加了日期切换时的记录。为了查找日期变换但是文件不变之谜。"
	  "\r\n修改了日期切换的机制（原来是每次比较，这次直接用了一个crontab）。"
	},
	{"V1.04"
	, "\r\n(2005-01-11)修正了每天一个文件时在文件超长后每个tick都打开关闭的问题。"
	},
	{"V1.05"
	, "\r\n(2005-03-08)增加了丢包检查的功能。"
	},
	{"V1.06"
	, "\r\n(2005-08-08)重大调整：把whunitallocatorFixed的浏览方式修改为iterator。"
	},
	{"V1.07"
	, "\r\n(2005-08-22)修正了因为使用iterator且循环中有删除操作而可能导致内存访问出错的bug。"
	},
	{"V1.08"
	, "\r\n(2005-08-31)应sohu要求把时间长度缩短到只有日。"
	},
	{"V1.09"
	, "\r\n(2006-03-27)在windows下把标题栏设置为exe+config文件，这样容易看清楚日志程序的功能。"
	},
	{"V1.10"
	, "\r\n(2007-03-01)修改为只读一个文件了。"
	},
	{"V1.11"
	, "\r\n(2007-04-17)增加了日志长度超过危险数值也强制。"
	},
	{"V1.12"
	, "\r\n(2007-04-20)重新设计了生成新文件的方式。OFOD还是一天只有一个文件。否则文件超过一定限度就自动开启新文件，自动加时刻或者是文件的当天序号。"
	},
	{"V1.13"
	, "\r\n(2007-05-30)增加了每隔整数时间都生成新文件的设置。（ps.股市大跌啊。哈哈）"
	},
};

#include <WHCMN/inc/whsignal.h>
#include <WHCMN/inc/whdaemon.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whdir.h>
#include <WHCMN/inc/whprocess.h>
#include <WHNET/inc/whnetudp.h>
#include <WHNET/inc/whnetudpGLogger.h>
#include <stdio.h>
#include <time.h>

#include <WHCMN/inc/wh_platform.h>
#ifdef	WIN32
#pragma comment(lib, "WS2_32")
#endif

using namespace n_whcmn;
using namespace n_whnet;

// EXE <cfgfile>
const char	*CSZEXENAME	= "glogger";

/////////////////////////////////////////
// 全局配置相关
/////////////////////////////////////////
WHDATAINI_STRUCT_DECLARE_BEGIN(GLOGSVR_INFO_T, GLogServerer::INFO_T)
	bool	bDaemon;
	bool	bReuseAddr;
	char	szPIDFile[WH_MAX_PATH];
	GLOGSVR_INFO_T()
	: bDaemon(true)
	, bReuseAddr(false)
	{
		szPIDFile[0]	= 0;
	}
WHDATAINI_STRUCT_DECLARE_END(GLOGSVR_INFO_T)
WHDATAINI_STRUCT_DECLARE(LOGWRITE_INFO_T, whlogwriter::INFO_T);

WHDATAPROP_MAP_BEGIN_AT_ROOT(GLOGSVR_INFO_T)
	WHDATAPROP_ON_SETVALUE_smp(bool, bDaemon, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSelectInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nKeepAliveTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxLogger, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szBindAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bAppendTagName, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bAppendSourceID, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bRawWrite, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bReuseAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szPIDFile, 0)
WHDATAPROP_MAP_END()

WHDATAPROP_MAP_BEGIN_AT_ROOT(LOGWRITE_INFO_T)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szLogFilePrefix, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szLogFileSuffix, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nOpenMode, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nProp, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCloseTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nFileStartIdx, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxPageSize, 0)
WHDATAPROP_MAP_END()

static bool	g_bStop = false;
static void	stopfunc(int sig)
{
	switch(sig)
	{
		case	SIGINT:
		case	SIGTERM:
			g_bStop	= true;
		break;
	}
}

int	main(int argc, char *argv[])
{
	whdir_SetCWDToExePath();

	GLOGSVR_INFO_T	ginfo;
	LOGWRITE_INFO_T	lwinfo;
	char			*szCFG		= "glogger_cfg.txt";

	if( argc>=2 )
	{
		if( argv[1][0] == '-' )
		{
			if( strcmp(argv[1], "-history")==0 )
			{
				if( argc==2 )
				{
					wh_strkeyprint(HISVER);
				}
				else
				{
					wh_strkeyprint(HISVER, argv[2]);
				}
				return	0;
			}
			else if( strcmp(argv[1], "-h")==0 )
			{
				printf("%s %s Built on %s %s%s"
					   "Usage: %s <cfgfile> [<cmncfg>]%s"
					, CSZEXENAME
					, VERSTR
					, __DATE__, __TIME__
					, WHLINEEND
					, CSZEXENAME
					, WHLINEEND
					);
			}
			else
			{
				printf("Unknown option: %s%s"
					, argv[1]
					, WHLINEEND
					);
			}
			return	0;
		}
		szCFG		= argv[1];
	}

	// 网络初始化
	cmn_ir_obj	cir;

	// 从配置文件中读取
	int				rst;
	WHDATAINI_CMN	ini;
	ini.addobj("GLOGSVR", &ginfo);
	ini.addobj("LOGWRITE", &lwinfo);

	// 分析配置
	rst	= ini.analyzefile(szCFG);
	if( rst<0 )
	{
		printf("cfgfile:%s is BAD! "
			   "Common values will be used!%s"
			, szCFG
			, WHLINEEND
			);
		ini.printerrreport(stdout);
		printf("%spress ENTER to end", WHLINEEND);
		puts(WHLINEEND);
		getchar();
		return	-1;
	}
	printf("CFG:%s is OK!%s"
		, szCFG
		, WHLINEEND
		);

	// 变成守护进程
	if( ginfo.bDaemon )
	{
		whdaemon_init();
	}

	if( ginfo.bReuseAddr )
	{
		cmn_set_reuseaddr(true);
	}

	if( ginfo.szPIDFile[0] )
	{
		// 尝试删除旧pid文件，如果失败则不能继续
		rst	= whprocess_trydelpidfile(ginfo.szPIDFile);
		if( rst<0 )
		{
			printf("Old server might be running. err code:%d%s", rst, WHLINEEND);
			return	rst;
		}
		else if( rst==1 )
		{
			printf("Old pid file %s deleted. There might be a previous crash.%s", ginfo.szPIDFile, WHLINEEND);
		}
	}
	// 创建pid文件（如果文件名为空则自动不会创建的）
	whprocess_pidfile	wpf(ginfo.szPIDFile);

	// 设置结束信号处理器
	whsingal_set_exitfunc(stopfunc);

	// 初始化
	GLogServerer	glogsvr;
	rst	= glogsvr.Init(&ginfo, &lwinfo);
	if( rst<0 )
	{
		// 到了这里就没有地方报错了
		return	-1;
	}

	// 写下开头信息
	glogsvr.GetLogWriter()->WriteFmtLine("****,Logger Started listening on address,%s,****"
		, ginfo.szBindAddr
		);

#ifdef	WIN32
	// 设置标题栏
	char	buf[1024];
	sprintf(buf, "%s %s", CSZEXENAME, szCFG);
	SetConsoleTitle(buf);
#endif

	while( !g_bStop )
	{
		glogsvr.Tick();
	}

	glogsvr.GetLogWriter()->WriteFmtLine("****,Logger Ended on addr,%s,****", ginfo.szBindAddr);

	glogsvr.Release();

	return	0;
}
