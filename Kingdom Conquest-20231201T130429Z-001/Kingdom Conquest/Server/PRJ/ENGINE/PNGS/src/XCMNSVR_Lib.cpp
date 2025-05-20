// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : XCMNSVR_Lib.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ͨ�÷��������
// CreationDate : 2007-12-06
// ChangeLog    :
//                2005-09-20 ����ΪV0.0001����XGMS2.cpp�̳й�����
//                2005-11-15 ����ΪV0.0002������UDPEvent��Ѹ��֪�����򱻹ر��ˡ�
//                2005-12-02 ����ΪV0.0003��ת�Ƶ�VC2003.net��
//                2006-04-07 ����ΪV0.0004���ڴ򲻿������ļ���ʱ�򱨴������Ǵ�ӡ������
//                2006-08-22 ����ΪV0.0005���������ļ���������
//                2007-03-02 ����ΪV0.0006��������windows�µĵ���ջ��ӡ��
//                2007-05-18 ����ΪV0.0007����epoll������������Ϊ�����ڱ����so�����Զ���epoll�������롣
//                2007-06-12 ����ΪV0.0008��������pid�ļ���������á�������ͨ��-stopֹͣ��������-status��÷���������״̬��
//                2007-06-13 �������κ�ʱ��������������Old pid file deleted�����⡣
//                2007-07-19 ����ΪV0.0009��������-stop��ʱ��ȴ����̽������˳���
//                2007-08-21 ����ΪV0.0010�����ļ�����˳��˳���Ϊ���ȶ�Ŀ¼�ٶ�ȡ�������ݡ�
//                2007-08-31 ������-kill������
//                2007-12-06 ����ΪV0.0011����XCMNSVR.cpp��ժ������

#include "../inc/XCMNSVR_Lib.h"
#ifdef	WIN32
#define	USER_STACKTRACE
#endif

#ifdef	__GNUC__
#include <sys/epoll.h>
#endif
#ifdef	WIN32
#include <WHCMN/inc/wh_platform.h>
#pragma comment(lib, "WS2_32")
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

#include <WHCMN/inc/whdll.h>
#include <WHCMN/inc/whvector.h>
#include <WHCMN/inc/whfile_util.h>
#include <WHCMN/inc/whprocess.h>
#include <WHCMN/inc/whdbgmem.h>
#include <WHCMN/inc/whstring.h>
#include <WHCMN/inc/whdir.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whsignal.h>
#include <WHCMN/inc/whdaemon.h>
#include <WHCMN/inc/wh_package_fileman.h>
#include <WHNET/inc/whnetudp.h>
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whnetepoll.h>				//added by yuezhongyue 2011-11-15,���epoll֧��
#include "../inc/pngs_cmn.h"
#include "../inc/pngs_def.h"
#include "../inc/pngs_gms.h"

//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���
#include "WHCMN/inc/whcmn_locallogger.h"

using namespace n_whcmn;
using namespace n_whnet;

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{

typedef	void					(*fn_WHNET_STATIC_INFO_In)(void *);
typedef	void					(*fn_WHCMN_STATIC_INFO_In)(void *);
struct	ILOGICDLLINFO_T
{
	char					szDLLFile[WH_MAX_PATH];
	char					szFuncName[128];
	char					szCFGFile[WH_MAX_PATH];
	int						nThreadTag;
	n_pngs::CMN::fn_ILOGIC_CREATE_T		CreateFunc;
	n_whcmn::WHDLL_Loader	*pLoader;
	ILOGICDLLINFO_T()
	{
		WHMEMSET0THIS();
	}
};
// �������64��PlugIn
enum
{
	MAXDLLNUM			= 64
};

const char	*VERSTR				= "V0.0010";
const char	*CSZEXE				= "XCMNSVR";
const char	*CSZ_KILL_TAG_FILE	= "~killtag";
char			cszCFG[WH_MAX_PATH] = "cfg.txt";
bool	g_bStop = false;

struct	CMN_CFGINFO_T		: public whdataini::obj
{
	bool	bDaemon;
	bool	bSupportMT;
	bool	bDisplayEveryTick;										// �Ƿ���Ҫÿ��tick����ʾһ����ʾ
	unsigned char	nDbgLvl;										// ���Լ������ǣ����һЩassert�Ƿ�ᱨ��
	int		nThreadTag;
	int		nSelectInterval;
	int		nKillWaitTimeOut;										// ���Ҫֹͣ���������ȴ����̽�����ʱ�䣨���룩
	int		nStopStatInterval;										// ���Ҫֹͣ���������ȴ����̽�����ʱ�䣨���룩
	int		nCheckCFGInterval;										// ���ڼ�������ļ��Ƿ�ı�ļ��
	char	szLoggerAddr[WHNET_MAXADDRSTRLEN*16];
	char	szSvrName[64];											// �����Ҫ����������ʾ����־
	char	szPackagePass[64];										// ������
	char	szPIDFile[WH_MAX_PATH];									// pid�ļ�������һ������ֻ������һ�Σ��Լ�ͨ��-stopֹͣ���̣�
	char	szCharSet[16];											// �����ַ�����Ĭ��Ϊ"GBK"��
	//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���,Ĭ�ϲ���¼������־
	char	szLocalLogFilePrefix[WH_MAX_PATH];						//������־ǰ׺(��Ҫ��Ŀ¼��Ϣ)
	CMN_CFGINFO_T()
	: bDaemon(true)
	, bSupportMT(false)
	, bDisplayEveryTick(false)
	, nDbgLvl(10)
	, nThreadTag(0)
	, nSelectInterval(10)
	, nKillWaitTimeOut(10*60*1000)
	, nStopStatInterval(10*1000)
	, nCheckCFGInterval(10*1000)
	{
		szLoggerAddr[0]		= 0;
		strcpy(szSvrName, "XCMNSVR");
		strcpy(szPackagePass, "tianxiawudi");
		szPIDFile[0]		= 0;									// ֻҪ�������Ϊ���������/tmp/szSvrName.pid��Ϊpid�ļ���
		strcpy(szCharSet, "GBK");
		//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���,Ĭ�ϲ���¼������־
		szLocalLogFilePrefix[0]	= 0;
	}
	WHDATAPROP_SETVALFUNC_DECLARE(HOST)
	{
		return	n_whnet::cmn_AddETCHostsLine(cszVal);
	}
	WHDATAPROP_SETVALFUNC_DECLARE(ENV)								// ���û�������
	{
		char	szEnvKey[256]	= "";
		char	szEnvVal[1024]	= "";
		wh_strsplit("sa", cszVal, ",", szEnvKey, szEnvVal);
		wh_setenv(szEnvKey, szEnvVal);
		return	0;
	}
	inline void	MakeDftPIDFIle()									// ��szSvrName����pid�ļ���
	{
		sprintf(szPIDFile, "/tmp/%s.pid", szSvrName);
	}

	// ����Ϊ�г�Աӳ��Ľṹ
	WHDATAPROP_DECLARE_MAP(CMN_CFGINFO_T)
};

int		AddPlugInInfo(const char *cszInfoStr);
struct	PLUGIN_CFGINFO_T	: public whdataini::obj
{
	WHDATAPROP_SETVALFUNC_DECLARE(DLL)
	{
		return	AddPlugInInfo(cszVal);
	}
	// ����Ϊ�г�Աӳ��Ľṹ
	WHDATAPROP_DECLARE_MAP(PLUGIN_CFGINFO_T)
};

PLUGIN_CFGINFO_T	g_plugin_cfginfo;
UDPEvent		g_loevent;

// �������64��PlugIn
ILOGICDLLINFO_T		aDLLInfo[MAXDLLNUM];
CMN_CFGINFO_T	g_cmn_cfginfo;
WHDATAPROP_MAP_BEGIN_AT_ROOT(CMN_CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(bool, bDaemon, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bSupportMT, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bDisplayEveryTick, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(whbyte, nDbgLvl, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nThreadTag, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSelectInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nKillWaitTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nStopStatInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCheckCFGInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szLoggerAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szSvrName, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szPIDFile, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szCharSet, 0)
	WHDATAPROP_ON_SETVALFUNC_reload1(HOST)
	WHDATAPROP_ON_SETVALFUNC_reload1(ENV)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szLocalLogFilePrefix, 0)	//added by yuezhongyue 20110805,��ӱ�����־���ݴ���
WHDATAPROP_MAP_END()
WHDATAPROP_MAP_BEGIN_AT_ROOT(PLUGIN_CFGINFO_T)
	WHDATAPROP_ON_SETVALFUNC(DLL)
WHDATAPROP_MAP_END()

void	stopfunc(int sig)
{
	switch(sig)
	{
		case	SIGINT:
		case	SIGTERM:
			g_bStop	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1092,PNGS_INT_RLS)"I WILL STOP ^__^");
			g_loevent.AddEvent();
		break;
	}
}

////////////////////////////////////////////////////////////////////
// DL_CMN
////////////////////////////////////////////////////////////////////
struct	DL_CMN
{
	int		Create(int i, CMN::ILogic *&pLogic);
	void	DelAllDLLLoaders();
	bool	NeedPlugInFile() const;
	int		load_logic_dll_and_query_history(const char *cszDLLFile, const char *cszCreateFuncName, const char *cszVer);
};
int		DL_CMN::Create(int i, CMN::ILogic *&pLogic)
{
	int	rst	= 0;
	if( stricmp(aDLLInfo[i].szFuncName, "DUMMY")==0 )
	{
		// �����DummyDLL�򴴽�Dummy
		pLogic	= CMN::CreateDummyLogic(aDLLInfo[i].szDLLFile);
	}
	else
	{
		WHDLL_LOAD_UNIT_T	aUnits[]	=
		{
			WHDLL_LOAD_UNIT_DECLARE0(aDLLInfo[i].szFuncName, aDLLInfo[i].CreateFunc, NULL)
				WHDLL_LOAD_UNIT_DECLARE_LAST()
		};

		aDLLInfo[i].pLoader	= WHDLL_Loader::Create();
		rst	= aDLLInfo[i].pLoader->Load(aDLLInfo[i].szDLLFile, aUnits);
		if( rst<0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(578,PNGS_INT_RLS)"LoadDLL,%d,%s,%s", rst, aDLLInfo[i].pLoader->m_szLastError, aDLLInfo[i].szDLLFile);
			return	-21;
		}

		// ���̺߳�DLL֮��������������ͬ��
		fn_WHCMN_STATIC_INFO_In		WHCMN_STATIC_INFO_In	= (fn_WHCMN_STATIC_INFO_In)aDLLInfo[i].pLoader->GetFunc("WHCMN_STATIC_INFO_In");
		if( WHCMN_STATIC_INFO_In != NULL )
		{
			(*WHCMN_STATIC_INFO_In)(WHCMN_STATIC_INFO_Out());
		}

		fn_WHNET_STATIC_INFO_In		WHNET_STATIC_INFO_In	= (fn_WHNET_STATIC_INFO_In)aDLLInfo[i].pLoader->GetFunc("WHNET_STATIC_INFO_In");
		if( WHNET_STATIC_INFO_In != NULL )
		{
			(*WHNET_STATIC_INFO_In)(WHNET_STATIC_INFO_Out());
		}

		pLogic	= aDLLInfo[i].CreateFunc();
		if( !pLogic )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(579,PNGS_INT_RLS)"LoadDLL,,CreateFunc,%s", aDLLInfo[i].szFuncName);
			return	-24;
		}
	}
	return	0;
}
// �ͷ�����DLL
void	DL_CMN::DelAllDLLLoaders()
{
	// ж������DLL
	for(int i=0;i<MAXDLLNUM;i++)
	{
		if( aDLLInfo[i].pLoader != NULL )
		{
			delete	aDLLInfo[i].pLoader;
			aDLLInfo[i].pLoader	= NULL;
		}
	}
}
bool	DL_CMN::NeedPlugInFile() const
{
	return	true;
}
int		DL_CMN::load_logic_dll_and_query_history(const char *cszDLLFile, const char *cszCreateFuncName, const char *cszVer)
{
	return	pngs_load_logic_dll_and_query_history(cszDLLFile, cszCreateFuncName, cszVer);
}

////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////
// �������ļ��м���һ��DLL����Ϣ
int		AddPlugInInfo(const char *cszInfoStr)
{
	// ��ʽ��
	// threadtag, sonanme, createfunc[, cfgfile]
	int	i	= 0;
	while( i<MAXDLLNUM-1 )
	{
		if( aDLLInfo[i].szDLLFile[0] == 0 )
		{
			// Ĭ��ʹ���ܵ�cfg�ļ�
			strcpy(aDLLInfo[i].szCFGFile, cszCFG);
			wh_strsplit("dsss", cszInfoStr, ",", &aDLLInfo[i].nThreadTag, aDLLInfo[i].szDLLFile, aDLLInfo[i].szFuncName, aDLLInfo[i].szCFGFile);
			return	0;
		}
		i	++;
	}
	fprintf(stderr, "Can not add PlugIn: %s%s", cszInfoStr, WHLINEEND);
	return	-1;
}

static const char	*HISVER[][2]	=
{
	{"V0.0001"
	, "\r\n(2005-09-20)��������XGMS2.cpp�̳й�����"
	},
	{"V0.0002"
	, "\r\n(2005-11-15)����UDPEvent��Ѹ��֪�����򱻹ر��ˡ�"
	},
	{"V0.0003"
	, "\r\n(2005-12-02)ת�Ƶ�VC2003.net��"
	},
	{"V0.0004"
	, "\r\n(2006-04-07)�ڴ򲻿������ļ���ʱ�򱨴������Ǵ�ӡ������"
	},
	{"V0.0005"
	, "\r\n(2006-08-22)�������ļ���������"
	},
	{"V0.0006"
	, "\r\n(2007-03-02)������windows�µĵ���ջ��ӡ��"
	},
	{"V0.0007"
	, "\r\n(2007-05-18)��epoll������������Ϊ�����ڱ����so�����Զ���epoll�������롣"
	},
	{"V0.0008"
	, "\r\n(2007-06-12)������pid�ļ���������á�������ͨ��-stopֹͣ��������-status��÷���������״̬��"
	  "\r\n�������κ�ʱ��������������Old pid file deleted�����⡣"
	},
	{"V0.0009"
	, "\r\n(2007-07-19)������-stop��ʱ��ȴ����̽������˳���"
	},
	{"V0.0010"
	, "\r\n(2007-08-21)���ļ�����˳��˳���Ϊ���ȶ�Ŀ¼�ٶ�ȡ�������ݡ�"
	  "\r\n(2007-08-31)������-kill������ֱ��ɱ�����̡�"
	},
	{NULL, NULL}
};

static void	usage()
{
	printf("%s %s Built on %s %s%s"
		, CSZEXE
		, VERSTR
		, __DATE__, __TIME__
		, WHLINEEND
		);
	printf("usage: %s [cfgfile=cfg.txt]            // run server%s"
		, CSZEXE
		, WHLINEEND
		);
	printf("usage: %s -stop [cfgfile=cfg.txt]      // stop server%s"
		, CSZEXE
		, WHLINEEND
		);
	printf("usage: %s -status [cfgfile=cfg.txt]    // show server status%s"
		, CSZEXE
		, WHLINEEND
		);
	printf("usage: %s -h|--help                   // print this screen%s"
		, CSZEXE
		, WHLINEEND
		);
}

#ifdef	USER_STACKTRACE
// ��ӡ����ջ�Ļ���
namespace
{

	struct LocalBuffer
	{
		char*   m_pos;
		char*   m_end;
		char*   m_fmt;
		char*   m_mod;
		IMAGEHLP_SYMBOL*    m_sym;

		LocalBuffer(char* p, char* e) : m_pos(p), m_end(e)
		{
			m_mod = (char*)::HeapAlloc(::GetProcessHeap(), 0, MAX_PATH);
			m_fmt = (char*)::HeapAlloc(::GetProcessHeap(), 0, 1000);
			m_sym = (IMAGEHLP_SYMBOL*)::HeapAlloc(::GetProcessHeap(), 0, 1000);
			m_sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
			m_sym->Address = 0;
			m_sym->Size = 0;
			m_sym->MaxNameLength = 1000-sizeof(IMAGEHLP_SYMBOL);
		}
		~LocalBuffer()
		{
			if(m_mod)
			{
				::HeapFree(::GetProcessHeap(), 0, m_mod);
			}
			if(m_fmt)
			{
				::HeapFree(::GetProcessHeap(), 0, m_fmt);
			}
			if(m_sym)
			{
				::HeapFree(::GetProcessHeap(), 0, m_sym);
			}
		}

		void Append(char* p)
		{
			size_t c = strlen(p);
			if(c > size_t(m_end-m_pos))
			{
				c = m_end-m_pos;
			}
			memcpy(m_pos, p, c);
			m_pos += c;
		}

		char* GetModule(HANDLE hProcess, size_t addr, size_t& rel_addr)
		{
			BYTE* base = (BYTE*)(size_t)SymGetModuleBase(hProcess, (DWORD)addr);
			if(base == 0)
			{
				return 0;
			}
			IMAGE_DOS_HEADER* dosHeader    = (IMAGE_DOS_HEADER*)base;
			if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			{
				return 0;
			}
			if(dosHeader->e_lfanew <= 0)
			{
				return 0;
			}
			IMAGE_NT_HEADERS* headers   = (IMAGE_NT_HEADERS*)((BYTE*)base+dosHeader->e_lfanew);
			rel_addr = addr - (size_t(base)-headers->OptionalHeader.ImageBase);
			GetModuleFileName((HMODULE)base, m_mod, MAX_PATH);

			return m_mod;
		}
	};

	char s_stack_buf[16384];

} // namespace
__declspec(noinline) static int G_GetStackTrace(char* s, size_t c, LPEXCEPTION_POINTERS exp_ptr)
{
	if(c <= 0)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
	{
		memset(s, 0, c);
		char* e = s+c-1;
		LocalBuffer lb(s, e);

		if(!SymInitialize(GetCurrentProcess(), NULL, TRUE))
		{
			printf("faileed");
			return EXCEPTION_CONTINUE_SEARCH;
		}

		HANDLE hThread = GetCurrentThread();
		HANDLE hProcess = GetCurrentProcess();
		BOOL bResult;
		DWORD_PTR Disp;
		IMAGEHLP_LINE Line;

		CONTEXT* ctx = exp_ptr->ContextRecord;
		STACKFRAME sfm;
		memset(&sfm, 0, sizeof(sfm));
		sfm.AddrPC.Offset = ctx->Eip;
		sfm.AddrStack.Offset = ctx->Esp;
		sfm.AddrFrame.Offset = ctx->Ebp;
		sfm.AddrPC.Mode = AddrModeFlat;
		sfm.AddrStack.Mode = AddrModeFlat;
		sfm.AddrFrame.Mode = AddrModeFlat;

		for( ; ; )
		{
			bResult = StackWalk(
				IMAGE_FILE_MACHINE_I386,
				hProcess,
				hThread,
				&sfm,
				0,
				NULL,
				SymFunctionTableAccess,
				SymGetModuleBase,
				NULL);
			if(!bResult || sfm.AddrFrame.Offset == 0)
			{
				break;
			}

			sprintf(lb.m_fmt, "* EIP<%.8X> RET<%.8X> PARAM<%.8X,%.8X,%.8X,%.8X>\n",
				(size_t)sfm.AddrPC.Offset, (size_t)sfm.AddrReturn.Offset,
				(size_t)sfm.Params[0],(size_t)sfm.Params[1],(size_t)sfm.Params[2],(size_t)sfm.Params[3]);
			lb.Append(lb.m_fmt);

			bResult = SymGetLineFromAddr(hProcess, sfm.AddrPC.Offset, &Disp, &Line);
			if(bResult)
			{
				sprintf(lb.m_fmt, "  source<%s> line<%d>\n", Line.FileName, Line.LineNumber);
				lb.Append(lb.m_fmt);
			}

			size_t rel = 0;
			char* mod = lb.GetModule(hProcess, sfm.AddrPC.Offset, rel);
			if(mod)
			{
				sprintf(lb.m_fmt, "  module<%s> offset<%.8X>\n", mod, rel);
				lb.Append(lb.m_fmt);
			}

			bResult = SymGetSymFromAddr(hProcess, sfm.AddrPC.Offset, &Disp, lb.m_sym);
			if(bResult)
			{
				sprintf(lb.m_fmt, "  symbol<%s> address<%.8X>\n", lb.m_sym->Name, lb.m_sym->Address);
				lb.Append(lb.m_fmt);
			}
			lb.Append("\n");
		}
	}
	char szPath[256];
	sprintf( szPath,"%s/svr-ps-%s.txt",whdir_GetExeFullpath(), wh_getsmptimestr_for_file());
	FILE* fp = fopen(szPath,"wt");
	if(fp != NULL)
	{
		fprintf(fp,s_stack_buf);
		fclose(fp);
	}
	s_stack_buf[2048]	= 0;	// ����MessageBox��Ϣ�ĳ��Ȳ���̫����ԭ��4096����Ҳ̫���ˣ�
	LRESULT  r = ::MessageBox(0, s_stack_buf, "Error", MB_OKCANCEL|MB_SYSTEMMODAL);
	if(r == IDOK)
	{
		// �������ļ����������
		system(szPath);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

template<class _ObjCreator>
int	_mymain(int argc, char *argv[], _ObjCreator &oc)
{
#ifdef	__GNUC__
	// д�����Ŀ����Ϊ���ܹ�����epoll����epoll��صĶ����ܱ�����
	int	eRst	= epoll_create(1);
	close(eRst);
#endif
	//added by yuezhongyue 2011-11-15,���epoll֧��
	whnet_epoll_ir_obj	epollIR;

	whdbg_check_leak(true);
	whdbg_SetBreakAlloc();

	whdir_SetCWDToExePath();

	// �����ǰĿ¼������gmon.out����ļ����Զ�����
	if( whfile_ispathexisted("gmon.out") )
	{
		char	szNewPath[WH_MAX_PATH];
		sprintf(szNewPath, "gmon.out-%s", wh_getsmptimestr_for_file());
		whfile_ren("gmon.out", szNewPath);
	}

	enum
	{
		ACTION_RUN		= 0,
		ACTION_STOP		= 1,
		ACTION_KILL		= 2,
		ACTION_STATUS	= 3,
	};
	int	nAction			= ACTION_RUN;

	if( argc>=2 )
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
		if( strcmp(argv[1], "-sohistory")==0 )
		{
			if( argc>=4 )
			{
				// -sohistory DLLName CreateFunc [ver]
				const char	*cszVer	= NULL;
				if( argc >= 5 )
				{
					cszVer	= argv[4];
				}
				int	rst	= oc.load_logic_dll_and_query_history(argv[2], argv[3], cszVer);
				if( rst<0 )
				{
					printf("sohistory err:%d%s", rst, WHLINEEND);
				}
				return	rst;
			}
			else
			{
				printf("Usage: %s -sohistory DLLName CreateFunc [ver]%s", CSZEXE, WHLINEEND);
				return	0;
			}
		}
		if( (strcmp(argv[1], "-h")==0 || strcmp(argv[1], "--help")==0) )
		{
			usage();
			return	0;
		}
		if( strcmp(argv[1], "-stop")==0 )
		{
			nAction	= ACTION_STOP;
		}
		else if( strcmp(argv[1], "-kill")==0 )
		{
			nAction	= ACTION_KILL;
		}
		else if( strcmp(argv[1], "-status")==0 )
		{
			nAction	= ACTION_STATUS;
		}
		else if( argv[1][0] == '-' )
		{
			printf("Unknown option '%s'%s", argv[1], WHLINEEND);
			return	-1;
		}
		else if( !whfile_ispathexisted(argv[1]) )
		{
			printf("cfgfile:'%s' NOT EXIST!%s", argv[1], WHLINEEND);
			return	-2;
		}
		if( nAction == ACTION_RUN )
		{
			strcpy(cszCFG, argv[1]);
		}
		else
		{
			if( argc>=3 )
			{
				strcpy(cszCFG, argv[2]);
			}
		}
	}

	// �����ļ�����
	WHDATAINI_CMN	ini;
	ini.addobj("CMN", &g_cmn_cfginfo);
	if( oc.NeedPlugInFile() )
	{
		ini.addobj("PlugIn", &g_plugin_cfginfo);
	}
	int	rst = ini.analyzefile(cszCFG);
	if( rst<0 )
	{
		printf("Can not analyze cfgfile:%s rst:%d %s%s", cszCFG, rst, ini.printerrreport(), WHLINEEND);
		return	-1;
	}
#ifdef	_DEBUG
	if( g_cmn_cfginfo.nDbgLvl>0 )
	{
		WHCMN_SET_DBGLVL(g_cmn_cfginfo.nDbgLvl);
	}
#endif
	CSZEXE	= g_cmn_cfginfo.szSvrName;
	// ���û������pid���򴴽�Ĭ�ϵ�pid�ļ���
	if( g_cmn_cfginfo.szPIDFile[0]==0 )
	{
		g_cmn_cfginfo.MakeDftPIDFIle();
	}

	switch( nAction )
	{
	case	ACTION_KILL:
		{
			// ����һ������ļ�
			whfile_writefile(CSZ_KILL_TAG_FILE, "", 1);
		}
	case	ACTION_STOP:
		{
			// ����ֹͣ������
			return	whprocess_terminatebypidfile(g_cmn_cfginfo.szPIDFile, false, g_cmn_cfginfo.nKillWaitTimeOut);
		}
		break;
	case	ACTION_STATUS:
		{
			// ��ý���״̬���Ƿ������У�
			whpid_t	pid	= whprocess_getfrompidfile(g_cmn_cfginfo.szPIDFile);
			if( whprocess_exists(pid) )
			{
				printf("%s pid:%u running%s", g_cmn_cfginfo.szSvrName, pid, WHLINEEND);
			}
			else
			{
				printf("%s pid:%u NOT running%s", g_cmn_cfginfo.szSvrName, pid, WHLINEEND);
			}
			return	0;
		}
		break;
	default:
		break;
	}

	// �޸�EXE title
	//strcpy(argv[0], CSZEXE);
#ifdef	WIN32
	SetConsoleTitle(CSZEXE);
#endif
	// ���ý����źŴ�����
	whsingal_set_exitfunc(stopfunc);

	// ����ɾ����pid�ļ������ʧ�����ܼ���
	rst	= whprocess_trydelpidfile(g_cmn_cfginfo.szPIDFile);
	if( rst<0 )
	{
		printf("Old server might be running. err code:%d%s", rst, WHLINEEND);
		return	rst;
	}
	else if( rst==1 )
	{
		printf("Old pid file %s deleted. There might be a previous crash.%s", g_cmn_cfginfo.szPIDFile, WHLINEEND);
	}

	// �����ʼ��
	cmn_ir_obj		cir;

	// ���daemon
	if( g_cmn_cfginfo.bDaemon )
	{
		whdaemon_init();
	}

	// ����ȫ�ֱ����ַ���
	WHCMN_setcharset(g_cmn_cfginfo.szCharSet);

	// ����pid�ļ�
	whprocess_pidfile	wpf(g_cmn_cfginfo.szPIDFile);

	//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���
	//������־��ʼ��
	LLOGGER_INIT(g_cmn_cfginfo.szLocalLogFilePrefix);

	// ��־��ʼ��
	GLOGGER2_INIT();
	GLOGGER2_INFO_T	info;
	strcpy(info.szExeTag, CSZEXE);
	info.bSupportMT		= g_cmn_cfginfo.bSupportMT;
	rst	= GLOGGER2_ADDMULTI(&info, g_cmn_cfginfo.szLoggerAddr);
	if( rst<0 )
	{
		printf("GLOGGER2_ADDMULTI %s ERROR:%d", g_cmn_cfginfo.szLoggerAddr, rst);
		GLOGGER2_RELEASE();
		return	-2;
	}

	// ����GMS����
	CMNBody			svr;
	// ����LOG����
	svr.m_fn_GLogger_WriteFmt	= &GLOGGER2_WRITEFMT;
	// ��������ܵ��̱߳�ǣ�һ��Ĭ����0���ɣ�����0��ģ��Ͷ����������һ���̣߳�
	svr.SetThreadTag(g_cmn_cfginfo.nThreadTag);
	// ����WHCMN���ֵ�log����
	WHCMN_Set_Log_WriteFmt(&GLOGGER2_WRITEFMT);

	// �����ļ������������ڷ�������Ҫ���ȶ�ȡĿ¼�е����ݣ���Ϊ������Ҫ��ʱ�޸ģ������Ͳ���ÿ�ζ�����ˣ�
	whfileman_package_INFO_Easy_T	infoeasy;
	infoeasy.nSearchOrder			= whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2;
	infoeasy.nOPMode				= whfileman::OP_MODE_BIN_READONLY;
	infoeasy.nPassLen				= strlen(g_cmn_cfginfo.szPackagePass);
	memcpy(infoeasy.szPass, g_cmn_cfginfo.szPackagePass, infoeasy.nPassLen);
	whfileman	*pFM				= whfileman_package_Create_Easy(&infoeasy);
	if( !pFM )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(586,PNGS_INT_RLS)"mymain,whfileman_package_Create_Easy failed");
		return	-5;
	}
	// �����ļ���������ȫ�ֱ���
	WHCMN_FILEMAN_SET(pFM);

	if( g_loevent.Init()<0 )
	{
		return	-4;
	}

	rst	= LoadDLL(pFM, svr, oc);
	if( rst<0 )
	{
		// ��������Logic
		svr.DetachAllLogic(true, true);
		// ж������DLL
		oc.DelAllDLLLoaders();
		// �����ļ�������
		WHSafeSelfDestroy(pFM);
		//
		GLOGGER2_RELEASE();
		return	rst;
	}

	whvector<SOCKET>	vectSOCKET;
	vectSOCKET.SetNoResize(true);
	vectSOCKET.reserve(PNGS_MAX_SOCKETTOSELECT);
	int	nTickCount	= 0;

	if( g_cmn_cfginfo.bSupportMT )
	{
		svr.StartThreads();
	}

	whlooper	wlCheckCFG;
	wlCheckCFG.setinterval(g_cmn_cfginfo.nCheckCFGInterval);

	while( !g_bStop && !svr.ShouldStop() )
	{
		if( WHCMN_GET_NOTIFY()>0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1049,PNGS_RT)"notify,%d", WHCMN_GET_NOTIFY());
			break;
		}
		if( g_cmn_cfginfo.bDisplayEveryTick )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,TEMP)"tick:%d time:%u", ++nTickCount, wh_gettickcount());
		}
		vectSOCKET.clear();
		svr.GetSockets(vectSOCKET);
		vectSOCKET.push_back( g_loevent.GetSocket() );	// �����Ϊ���ܹ����췢��ֹͣ����

		cmn_select_rd_array(vectSOCKET.getbuf(), vectSOCKET.size(), g_cmn_cfginfo.nSelectInterval);

		if( wlCheckCFG.check() )
		{
			wlCheckCFG.reset();
			whvector<whmultifilechangedetector::RST_T> *pRst	= ini.m_mfcd.CheckChanged();
			if( pRst )
			{
				// ��ӡ��־
				char	buf[1024];
				char	*pbuf	= buf;
				for(size_t i=0;i<pRst->size();++i)
				{
					int	rst	= sprintf(pbuf, ",%s", pRst->get(i).pszFileName);
					pbuf	+= rst;
				}
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1102,PNGS_RT)"cfg reload,%s", buf);
				// ������������
				ini.analyzefile(cszCFG, false, 1);
				svr.ReloadAllCFG(cszCFG);
			}
		}

		svr.AllTick();
		svr.AllSureSend();
		GLOGGER2_TICK();

		//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���
		LLOGGER_TICK();
	}
	if( g_cmn_cfginfo.bSupportMT )
	{
		svr.StopThreads();
	}

	// �ж��Ƿ���kill����ļ�
	if( whfile_ispathexisted(CSZ_KILL_TAG_FILE) )
	{
		whfile_del(CSZ_KILL_TAG_FILE);
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(866,PNGS_INT_RLS)"CMNSVR KILLED! Nothing will be released.");
		exit(0);
	}

	// ��������¼�
	g_loevent.ClrAllEvent();

	// ����������ֹͣ״̬
	svr.SetAllLogicToStopMood();
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(583,PNGS_INT_RLS)"All Logics are in STOP MOOD now");
#ifdef	WIN32
	{
		char	buf[256];
		sprintf(buf, "%s stopping ...", g_cmn_cfginfo.szSvrName);
		SetConsoleTitle(buf);
	}
#endif
	whlooper	whl;
	whl.setinterval(g_cmn_cfginfo.nStopStatInterval);
	whl.reset();
	while( !svr.AreAllLogicsReallyStopped() )
	{
		if( whl.check() )
		{
			whl.reset();
			// �����־������Щlogicû��ͣ
		}
		vectSOCKET.clear();
		svr.GetSockets(vectSOCKET);
		if( cmn_select_rd_array(vectSOCKET.getbuf(), vectSOCKET.size(), 50)<0 )	// �����Ϳ�Щ�ɡ�
		{
			// ��������ڼ������socket������ܿ��ܵ���select�������أ������CPUռ�ù��ߡ�
			wh_sleep(50);
		}
		svr.AllTick();
		svr.AllSureSend();
		GLOGGER2_TICK();

		//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���
		LLOGGER_TICK();
	}

	// Ϊ�˱����ٵ���һЩtick
	for(int i=0;i<10;i++)
	{
		svr.AllTick();
		GLOGGER2_TICK();

		//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���
		LLOGGER_TICK();

		wh_sleep(20);
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(584,PNGS_INT_RLS)"WORK END,Releasing");

	svr.DetachAllLogic(true);

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(585,PNGS_INT_RLS)"Good End!");

	oc.DelAllDLLLoaders();

	// �����ļ�������
	WHSafeSelfDestroy(pFM);
	// ��־����
	GLOGGER2_RELEASE();

	//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���
	//������־����
	LLOGGER_RELEASE();

	return		0;
}

template<class _ObjCreator>
int	mymain(int argc, char *argv[], _ObjCreator &oc)
{
#ifdef	USER_STACKTRACE
	__try
	{
#endif
		return	_mymain<_ObjCreator>(argc, argv, oc);
#ifdef	USER_STACKTRACE
	}
	__except(G_GetStackTrace(s_stack_buf, sizeof(s_stack_buf), GetExceptionInformation()))
	{
		TerminateProcess(GetCurrentProcess(), 0);
	}
#endif
	return	0;
}

template<class _ObjCreator>
int	LoadDLL(whfileman *pFM, CMNBody &svr, _ObjCreator &oc)
{
	int		rst=0;
	int		i=0;
	// �������DLLģ�飬���ҽ���Ӧ��Logic
	i	= 0;
	while( aDLLInfo[i].szDLLFile[0] != 0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(577,PNGS_INT_RLS)"Load plugin,%s", aDLLInfo[i].szDLLFile);

		if( aDLLInfo[i].szCFGFile[0]==0 )
		{
			strcpy(aDLLInfo[i].szCFGFile, cszCFG);
		}

		CMN::ILogic	*pLogic	= NULL;
		rst	= oc.Create(i, pLogic);
		if( rst<0 )
		{
			return	rst;
		}
		if( (rst=svr.AttachLogic(pLogic)) < 0 )
		{
			// ���Attachʧ������Ҫ����ɾ���������Ϊsvr�ڲ���û�м�¼�������Ժ����DetachAllLogic(true)���޷�ɾ��
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(580,PNGS_INT_RLS)"LoadDLL,%d,svr.AttachLogic,%s", rst, pLogic->GetLogicType());
			pLogic->SelfDestroy();
			return	-25;
		}
		// �����ļ�������
		pLogic->SetFileMan(pFM);
		// �����̱߳�ʶ
		if( g_cmn_cfginfo.bSupportMT )
		{
			pLogic->SetThreadTag(aDLLInfo[i].nThreadTag);
		}
		if( (rst=pLogic->Init(aDLLInfo[i].szCFGFile)) < 0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(581,PNGS_INT_RLS)"LoadDLL,%d,pLogic->Init,%s,%s", rst, aDLLInfo[i].szDLLFile, aDLLInfo[i].szCFGFile);
			return	-26;
		}
		// ���Init���̻�û���꣬�������tick
		if( !pLogic->IsInitOver() )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(806,PNGS_INT_RLS)"Continue init,BEGIN,%s,%s", aDLLInfo[i].szDLLFile, pLogic->GetLogicType());
			while( !pLogic->IsInitOver() )
			{
				pLogic->Tick();
				pLogic->SureSend();
				GLOGGER2_TICK();

				//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���
				LLOGGER_TICK();

				wh_sleep(5);
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(806,PNGS_INT_RLS)"Continue init,END,%s,%s", aDLLInfo[i].szDLLFile, pLogic->GetLogicType());
		}
		// �������Ĺ��̳����˾�ֹͣ
		if( svr.ShouldStop() )
		{
			return	-27;
		}
		// ��һ��
		i++;
	}

	// ��������logic��֮����໥��ϵ��
	if( (rst=svr.OrganizeLogic()) < 0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(582,PNGS_INT_RLS)"LoadDLL,%d,svr.OrganizeLogic", rst);
		return	-31;
	}
	return	0;
}

int	main_dll(int argc, char *argv[])
{
	DL_CMN	dl;
	return	mymain<DL_CMN>(argc, argv, dl);
}

}	// EOF namespace n_pngs
