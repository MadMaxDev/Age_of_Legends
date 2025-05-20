// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : XCMNSVR_Lib.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 通用服务器框架
// CreationDate : 2007-12-06
// ChangeLog    :
//                2005-09-20 定版为V0.0001。从XGMS2.cpp继承过来。
//                2005-11-15 升级为V0.0002。增加UDPEvent以迅速知道程序被关闭了。
//                2005-12-02 升级为V0.0003。转移到VC2003.net。
//                2006-04-07 升级为V0.0004。在打不开配置文件的时候报错，而不是打印帮助。
//                2006-08-22 升级为V0.0005。增加了文件管理器。
//                2007-03-02 升级为V0.0006。增加了windows下的调用栈打印。
//                2007-05-18 升级为V0.0007。把epoll编入主程序，因为我现在编译的so不能自动吧epoll代码连入。
//                2007-06-12 升级为V0.0008。增了了pid文件的相关设置。并可以通过-stop停止服务器、-status获得服务器运行状态。
//                2007-06-13 修正了任何时候正常启动都报Old pid file deleted的问题。
//                2007-07-19 升级为V0.0009。增加在-stop的时候等待进程结束再退出。
//                2007-08-21 升级为V0.0010。把文件查找顺序顺序改为优先读目录再读取包中数据。
//                2007-08-31 增加了-kill参数。
//                2007-12-06 升级为V0.0011。从XCMNSVR.cpp中摘出来。

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
#include <WHNET/inc/whnetepoll.h>				//added by yuezhongyue 2011-11-15,添加epoll支持
#include "../inc/pngs_cmn.h"
#include "../inc/pngs_def.h"
#include "../inc/pngs_gms.h"

//added by yuezhongyue 2011-08-05,添加本地日志备份处理
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
// 最多载入64个PlugIn
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
	bool	bDisplayEveryTick;										// 是否需要每个tick都显示一个提示
	unsigned char	nDbgLvl;										// 调试级别（这个牵扯到一些assert是否会报）
	int		nThreadTag;
	int		nSelectInterval;
	int		nKillWaitTimeOut;										// 如果要停止服务器，等待进程结束的时间（毫秒）
	int		nStopStatInterval;										// 如果要停止服务器，等待进程结束的时间（毫秒）
	int		nCheckCFGInterval;										// 定期检查配置文件是否改变的间隔
	char	szLoggerAddr[WHNET_MAXADDRSTRLEN*16];
	char	szSvrName[64];											// 这个主要用于制作提示和日志
	char	szPackagePass[64];										// 包密码
	char	szPIDFile[WH_MAX_PATH];									// pid文件（用于一个程序只能启动一次，以及通过-stop停止进程）
	char	szCharSet[16];											// 本地字符集（默认为"GBK"）
	//added by yuezhongyue 2011-08-05,添加本地日志备份处理,默认不记录本地日志
	char	szLocalLogFilePrefix[WH_MAX_PATH];						//本地日志前缀(主要是目录信息)
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
		szPIDFile[0]		= 0;									// 只要这个设置为空则代表用/tmp/szSvrName.pid作为pid文件名
		strcpy(szCharSet, "GBK");
		//added by yuezhongyue 2011-08-05,添加本地日志备份处理,默认不记录本地日志
		szLocalLogFilePrefix[0]	= 0;
	}
	WHDATAPROP_SETVALFUNC_DECLARE(HOST)
	{
		return	n_whnet::cmn_AddETCHostsLine(cszVal);
	}
	WHDATAPROP_SETVALFUNC_DECLARE(ENV)								// 设置环境变量
	{
		char	szEnvKey[256]	= "";
		char	szEnvVal[1024]	= "";
		wh_strsplit("sa", cszVal, ",", szEnvKey, szEnvVal);
		wh_setenv(szEnvKey, szEnvVal);
		return	0;
	}
	inline void	MakeDftPIDFIle()									// 用szSvrName制造pid文件名
	{
		sprintf(szPIDFile, "/tmp/%s.pid", szSvrName);
	}

	// 定义为有成员映射的结构
	WHDATAPROP_DECLARE_MAP(CMN_CFGINFO_T)
};

int		AddPlugInInfo(const char *cszInfoStr);
struct	PLUGIN_CFGINFO_T	: public whdataini::obj
{
	WHDATAPROP_SETVALFUNC_DECLARE(DLL)
	{
		return	AddPlugInInfo(cszVal);
	}
	// 定义为有成员映射的结构
	WHDATAPROP_DECLARE_MAP(PLUGIN_CFGINFO_T)
};

PLUGIN_CFGINFO_T	g_plugin_cfginfo;
UDPEvent		g_loevent;

// 最多载入64个PlugIn
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
	WHDATAPROP_ON_SETVALUE_smp(charptr, szLocalLogFilePrefix, 0)	//added by yuezhongyue 20110805,添加本地日志备份处理
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
		// 如果是DummyDLL则创建Dummy
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

		// 主线程和DLL之间的网络基本数据同步
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
// 释放所有DLL
void	DL_CMN::DelAllDLLLoaders()
{
	// 卸载所有DLL
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
// 按配置文件行加入一个DLL的信息
int		AddPlugInInfo(const char *cszInfoStr)
{
	// 格式：
	// threadtag, sonanme, createfunc[, cfgfile]
	int	i	= 0;
	while( i<MAXDLLNUM-1 )
	{
		if( aDLLInfo[i].szDLLFile[0] == 0 )
		{
			// 默认使用总的cfg文件
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
	, "\r\n(2005-09-20)创建。从XGMS2.cpp继承过来。"
	},
	{"V0.0002"
	, "\r\n(2005-11-15)增加UDPEvent以迅速知道程序被关闭了。"
	},
	{"V0.0003"
	, "\r\n(2005-12-02)转移到VC2003.net。"
	},
	{"V0.0004"
	, "\r\n(2006-04-07)在打不开配置文件的时候报错，而不是打印帮助。"
	},
	{"V0.0005"
	, "\r\n(2006-08-22)增加了文件管理器。"
	},
	{"V0.0006"
	, "\r\n(2007-03-02)增加了windows下的调用栈打印。"
	},
	{"V0.0007"
	, "\r\n(2007-05-18)把epoll编入主程序，因为我现在编译的so不能自动吧epoll代码连入。"
	},
	{"V0.0008"
	, "\r\n(2007-06-12)增了了pid文件的相关设置。并可以通过-stop停止服务器、-status获得服务器运行状态。"
	  "\r\n修正了任何时候正常启动都报Old pid file deleted的问题。"
	},
	{"V0.0009"
	, "\r\n(2007-07-19)增加在-stop的时候等待进程结束再退出。"
	},
	{"V0.0010"
	, "\r\n(2007-08-21)把文件查找顺序顺序改为优先读目录再读取包中数据。"
	  "\r\n(2007-08-31)增加了-kill参数，直接杀死进程。"
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
// 打印调用栈的机制
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
	s_stack_buf[2048]	= 0;	// 限制MessageBox消息的长度不能太长（原来4096好像也太多了）
	LRESULT  r = ::MessageBox(0, s_stack_buf, "Error", MB_OKCANCEL|MB_SYSTEMMODAL);
	if(r == IDOK)
	{
		// 把整个文件都打出来吧
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
	// 写这个的目的是为了能够引用epoll，让epoll相关的东西能被连入
	int	eRst	= epoll_create(1);
	close(eRst);
#endif
	//added by yuezhongyue 2011-11-15,添加epoll支持
	whnet_epoll_ir_obj	epollIR;

	whdbg_check_leak(true);
	whdbg_SetBreakAlloc();

	whdir_SetCWDToExePath();

	// 如果当前目录下面有gmon.out这个文件就自动改名
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

	// 配置文件分析
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
	// 如果没有设置pid名则创建默认的pid文件名
	if( g_cmn_cfginfo.szPIDFile[0]==0 )
	{
		g_cmn_cfginfo.MakeDftPIDFIle();
	}

	switch( nAction )
	{
	case	ACTION_KILL:
		{
			// 创建一个标记文件
			whfile_writefile(CSZ_KILL_TAG_FILE, "", 1);
		}
	case	ACTION_STOP:
		{
			// 尝试停止服务器
			return	whprocess_terminatebypidfile(g_cmn_cfginfo.szPIDFile, false, g_cmn_cfginfo.nKillWaitTimeOut);
		}
		break;
	case	ACTION_STATUS:
		{
			// 获得进程状态（是否在运行）
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

	// 修改EXE title
	//strcpy(argv[0], CSZEXE);
#ifdef	WIN32
	SetConsoleTitle(CSZEXE);
#endif
	// 设置结束信号处理器
	whsingal_set_exitfunc(stopfunc);

	// 尝试删除旧pid文件，如果失败则不能继续
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

	// 网络初始化
	cmn_ir_obj		cir;

	// 变成daemon
	if( g_cmn_cfginfo.bDaemon )
	{
		whdaemon_init();
	}

	// 设置全局本地字符集
	WHCMN_setcharset(g_cmn_cfginfo.szCharSet);

	// 创建pid文件
	whprocess_pidfile	wpf(g_cmn_cfginfo.szPIDFile);

	//added by yuezhongyue 2011-08-05,添加本地日志备份处理
	//本地日志初始化
	LLOGGER_INIT(g_cmn_cfginfo.szLocalLogFilePrefix);

	// 日志初始化
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

	// 创建GMS对象
	CMNBody			svr;
	// 设置LOG函数
	svr.m_fn_GLogger_WriteFmt	= &GLOGGER2_WRITEFMT;
	// 设置主框架的线程标记（一般默认是0即可，所有0的模块就都和主框架在一个线程）
	svr.SetThreadTag(g_cmn_cfginfo.nThreadTag);
	// 设置WHCMN部分的log函数
	WHCMN_Set_Log_WriteFmt(&GLOGGER2_WRITEFMT);

	// 创建文件管理器（对于服务器需要优先读取目录中的数据，因为可能需要临时修改，这样就不用每次都打包了）
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
	// 设置文件管理器到全局变量
	WHCMN_FILEMAN_SET(pFM);

	if( g_loevent.Init()<0 )
	{
		return	-4;
	}

	rst	= LoadDLL(pFM, svr, oc);
	if( rst<0 )
	{
		// 脱离所有Logic
		svr.DetachAllLogic(true, true);
		// 卸载所有DLL
		oc.DelAllDLLLoaders();
		// 销毁文件管理器
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
		vectSOCKET.push_back( g_loevent.GetSocket() );	// 这个是为了能够尽快发现停止动作

		cmn_select_rd_array(vectSOCKET.getbuf(), vectSOCKET.size(), g_cmn_cfginfo.nSelectInterval);

		if( wlCheckCFG.check() )
		{
			wlCheckCFG.reset();
			whvector<whmultifilechangedetector::RST_T> *pRst	= ini.m_mfcd.CheckChanged();
			if( pRst )
			{
				// 打印日志
				char	buf[1024];
				char	*pbuf	= buf;
				for(size_t i=0;i<pRst->size();++i)
				{
					int	rst	= sprintf(pbuf, ",%s", pRst->get(i).pszFileName);
					pbuf	+= rst;
				}
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1102,PNGS_RT)"cfg reload,%s", buf);
				// 重新载入配置
				ini.analyzefile(cszCFG, false, 1);
				svr.ReloadAllCFG(cszCFG);
			}
		}

		svr.AllTick();
		svr.AllSureSend();
		GLOGGER2_TICK();

		//added by yuezhongyue 2011-08-05,添加本地日志备份处理
		LLOGGER_TICK();
	}
	if( g_cmn_cfginfo.bSupportMT )
	{
		svr.StopThreads();
	}

	// 判断是否有kill标记文件
	if( whfile_ispathexisted(CSZ_KILL_TAG_FILE) )
	{
		whfile_del(CSZ_KILL_TAG_FILE);
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(866,PNGS_INT_RLS)"CMNSVR KILLED! Nothing will be released.");
		exit(0);
	}

	// 清除所有事件
	g_loevent.ClrAllEvent();

	// 服务器进入停止状态
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
			// 输出日志，看那些logic没有停
		}
		vectSOCKET.clear();
		svr.GetSockets(vectSOCKET);
		if( cmn_select_rd_array(vectSOCKET.getbuf(), vectSOCKET.size(), 50)<0 )	// 结束就快些吧。
		{
			// 如果在这期间出现了socket错误，则很可能导致select立即返回，而造成CPU占用过高。
			wh_sleep(50);
		}
		svr.AllTick();
		svr.AllSureSend();
		GLOGGER2_TICK();

		//added by yuezhongyue 2011-08-05,添加本地日志备份处理
		LLOGGER_TICK();
	}

	// 为了保险再调用一些tick
	for(int i=0;i<10;i++)
	{
		svr.AllTick();
		GLOGGER2_TICK();

		//added by yuezhongyue 2011-08-05,添加本地日志备份处理
		LLOGGER_TICK();

		wh_sleep(20);
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(584,PNGS_INT_RLS)"WORK END,Releasing");

	svr.DetachAllLogic(true);

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(585,PNGS_INT_RLS)"Good End!");

	oc.DelAllDLLLoaders();

	// 销毁文件管理器
	WHSafeSelfDestroy(pFM);
	// 日志结束
	GLOGGER2_RELEASE();

	//added by yuezhongyue 2011-08-05,添加本地日志备份处理
	//本地日志结束
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
	// 载入各个DLL模块，并挂接相应的Logic
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
			// 如果Attach失败则需要立刻删除这个，因为svr内部并没有记录它，所以后面的DetachAllLogic(true)就无法删除
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(580,PNGS_INT_RLS)"LoadDLL,%d,svr.AttachLogic,%s", rst, pLogic->GetLogicType());
			pLogic->SelfDestroy();
			return	-25;
		}
		// 设置文件管理器
		pLogic->SetFileMan(pFM);
		// 设置线程标识
		if( g_cmn_cfginfo.bSupportMT )
		{
			pLogic->SetThreadTag(aDLLInfo[i].nThreadTag);
		}
		if( (rst=pLogic->Init(aDLLInfo[i].szCFGFile)) < 0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(581,PNGS_INT_RLS)"LoadDLL,%d,pLogic->Init,%s,%s", rst, aDLLInfo[i].szDLLFile, aDLLInfo[i].szCFGFile);
			return	-26;
		}
		// 如果Init过程还没有完，则调用其tick
		if( !pLogic->IsInitOver() )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(806,PNGS_INT_RLS)"Continue init,BEGIN,%s,%s", aDLLInfo[i].szDLLFile, pLogic->GetLogicType());
			while( !pLogic->IsInitOver() )
			{
				pLogic->Tick();
				pLogic->SureSend();
				GLOGGER2_TICK();

				//added by yuezhongyue 2011-08-05,添加本地日志备份处理
				LLOGGER_TICK();

				wh_sleep(5);
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(806,PNGS_INT_RLS)"Continue init,END,%s,%s", aDLLInfo[i].szDLLFile, pLogic->GetLogicType());
		}
		// 如果上面的过程出错了就停止
		if( svr.ShouldStop() )
		{
			return	-27;
		}
		// 下一个
		i++;
	}

	// 整理所有logic（之间的相互关系）
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
