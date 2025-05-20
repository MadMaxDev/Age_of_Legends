// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pnldb
// File         : glogger.cpp
// Creator      : Wei Hua (κ��)
// Comment      : log����������
// CreationDate : 2003-10-22
// ChangeLog    : 2004-07-16 ȷ��ΪV1.0�汾��
//                2004-08-31 ����ΪV1.01�������ջ����Ϊ32768��
//                2004-09-01 LoggerUnit��nLastKATick��ΪֻҪ�յ���Ϣ�͸���
//                2004-09-21 ����ΪV1.02�����������������жϡ�
//                2004-09-23 ������whlogwriter::RawPrintf�жԿ��ļ�ָ����ж�
//                2005-01-11 ����ΪV1.03�������������л�ʱ�ļ�¼��Ϊ�˲������ڱ任�����ļ�����֮�ա�
//                           �޸��������л��Ļ��ƣ�ԭ����ÿ�αȽϣ����ֱ������һ��crontab��
//                2005-01-11 ����ΪV1.04��������ÿ��һ���ļ�ʱ���ļ�������ÿ��tick���򿪹رյ����⡣
//                2005-03-08 ����ΪV1.05�������˶������Ĺ��ܡ�
//                2005-08-08 ����ΪV1.06���ش��������whunitallocatorFixed�������ʽ�޸�Ϊiterator��
//                2005-08-22 ����ΪV1.07����������Ϊʹ��iterator��ѭ������ɾ�����������ܵ����ڴ���ʳ����bug��
//                2005-08-31 ����ΪV1.08��ӦsohuҪ���ʱ�䳤�����̵�ֻ���ա�
//                2006-03-27 ����ΪV1.09����windows�°ѱ���������Ϊexe+config�ļ����������׿������־����Ĺ��ܡ�
//                2007-03-01 ����ΪV1.10���޸�Ϊֻ��һ���ļ��ˡ�
//                2007-04-17 ����ΪV1.11����������־���ȳ���Σ����ֵҲǿ�ơ�
//                2007-04-20 ����ΪV1.12������������������ļ��ķ�ʽ��OFOD����һ��ֻ��һ���ļ��������ļ�����һ���޶Ⱦ��Զ��������ļ����Զ���ʱ�̻������ļ��ĵ�����š�
//                2007-05-30 ����ΪV1.13��������ÿ������ʱ�䶼�������ļ������á���ps.���д������������

static const char	*VERSTR	= "V1.13";

static const char	*HISVER[][2]	=
{
	{"V1.0"
	, "\r\n(2004-07-16)ȷ��ΪV1.0�汾��"
	},
	{"V1.01"
	, "\r\n(2004-08-31)�����ջ����Ϊ32768��"
	  "\r\n(2004-09-01)LoggerUnit��nLastKATick��ΪֻҪ�յ���Ϣ�͸���"
	},
	{"V1.02"
	, "\r\n(2004-09-21)���������������жϡ�"
      "\r\n(2004-09-23)������whlogwriter::RawPrintf�жԿ��ļ�ָ����жϡ�"
	},
	{"V1.03"
	, "\r\n(2005-01-11)�����������л�ʱ�ļ�¼��Ϊ�˲������ڱ任�����ļ�����֮�ա�"
	  "\r\n�޸��������л��Ļ��ƣ�ԭ����ÿ�αȽϣ����ֱ������һ��crontab����"
	},
	{"V1.04"
	, "\r\n(2005-01-11)������ÿ��һ���ļ�ʱ���ļ�������ÿ��tick���򿪹رյ����⡣"
	},
	{"V1.05"
	, "\r\n(2005-03-08)�����˶������Ĺ��ܡ�"
	},
	{"V1.06"
	, "\r\n(2005-08-08)�ش��������whunitallocatorFixed�������ʽ�޸�Ϊiterator��"
	},
	{"V1.07"
	, "\r\n(2005-08-22)��������Ϊʹ��iterator��ѭ������ɾ�����������ܵ����ڴ���ʳ����bug��"
	},
	{"V1.08"
	, "\r\n(2005-08-31)ӦsohuҪ���ʱ�䳤�����̵�ֻ���ա�"
	},
	{"V1.09"
	, "\r\n(2006-03-27)��windows�°ѱ���������Ϊexe+config�ļ����������׿������־����Ĺ��ܡ�"
	},
	{"V1.10"
	, "\r\n(2007-03-01)�޸�Ϊֻ��һ���ļ��ˡ�"
	},
	{"V1.11"
	, "\r\n(2007-04-17)��������־���ȳ���Σ����ֵҲǿ�ơ�"
	},
	{"V1.12"
	, "\r\n(2007-04-20)����������������ļ��ķ�ʽ��OFOD����һ��ֻ��һ���ļ��������ļ�����һ���޶Ⱦ��Զ��������ļ����Զ���ʱ�̻������ļ��ĵ�����š�"
	},
	{"V1.13"
	, "\r\n(2007-05-30)������ÿ������ʱ�䶼�������ļ������á���ps.���д������������"
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
// ȫ���������
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

	// �����ʼ��
	cmn_ir_obj	cir;

	// �������ļ��ж�ȡ
	int				rst;
	WHDATAINI_CMN	ini;
	ini.addobj("GLOGSVR", &ginfo);
	ini.addobj("LOGWRITE", &lwinfo);

	// ��������
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

	// ����ػ�����
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
		// ����ɾ����pid�ļ������ʧ�����ܼ���
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
	// ����pid�ļ�������ļ���Ϊ�����Զ����ᴴ���ģ�
	whprocess_pidfile	wpf(ginfo.szPIDFile);

	// ���ý����źŴ�����
	whsingal_set_exitfunc(stopfunc);

	// ��ʼ��
	GLogServerer	glogsvr;
	rst	= glogsvr.Init(&ginfo, &lwinfo);
	if( rst<0 )
	{
		// ���������û�еط�������
		return	-1;
	}

	// д�¿�ͷ��Ϣ
	glogsvr.GetLogWriter()->WriteFmtLine("****,Logger Started listening on address,%s,****"
		, ginfo.szBindAddr
		);

#ifdef	WIN32
	// ���ñ�����
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
