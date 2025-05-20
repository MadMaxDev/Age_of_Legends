// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whprocess.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 简单的进程相关函数。
//                这个库的用处是统一windows和linux（可能还有别的系统）下的进程相关使用方法
// CreationDate : 2003-08-07
// ChangeLOG    : 2007-07-12 修改了windows下的whprocess_exists，使用窗口枚举来判断是否pid存在。原来OpenProcess的方法可能会导致父进程中判断已经被关闭的子进程还存在。
//              : 2007-11-12 在whprocess_pidfile中增加了如果文件为空则不创建和删除文件

#include "../inc/whprocess.h"
#include "../inc/whvector.h"
#include "../inc/whfile.h"
#include <stdlib.h>
#ifdef	__GNUC__
#include <signal.h>
#include <sys/wait.h>
#include "../inc/whtime.h"
#include "../inc/whdir.h"
#endif

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
#if defined( WIN32 )			// Windows下的代码	{
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 从别人那里考过来的获取进程主窗口的代码
// http://www.vckbase.com/document/viewdoc/?id=404
////////////////////////////////////////////////////////////////////
// Iterate the top-level windows. Encapsulates ::EnumWindows.
//
class CWindowIterator {
protected:
	HWND	*m_hwnds;			// array of hwnds for this PID
	DWORD	m_nAlloc;			// size of array
	DWORD	m_count;			// number of HWNDs found
	DWORD	m_current;			// current HWND
	static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lp);

	// virtual enumerator
	virtual BOOL OnEnumProc(HWND hwnd);

	// override to filter different kinds of windows
	virtual BOOL OnWindow(HWND hwnd) {
		return TRUE;
	}

public:
	CWindowIterator(DWORD nAlloc=1024);
	~CWindowIterator();
	
	DWORD GetCount() { return m_count; }
	HWND First();
	HWND Next() {
		return m_hwnds && m_current < m_count ? m_hwnds[m_current++] : NULL;
	}
};

//////////////////
// Iterate the top-level windows in a process.
//
class CMainWindowIterator : public CWindowIterator  {
protected:
	DWORD m_pid;				// process id
	virtual BOOL OnWindow(HWND hwnd);
public:
	CMainWindowIterator(DWORD pid, DWORD nAlloc=1024);
	~CMainWindowIterator();
};
CWindowIterator::CWindowIterator(DWORD nAlloc)
{
	assert(nAlloc>0);
	m_current = m_count = 0;
	m_hwnds = new HWND [nAlloc];
	m_nAlloc = nAlloc;
}

CWindowIterator::~CWindowIterator()
{
	delete [] m_hwnds;
}

//////////////////
// Move to first top-level window.
// Stuff entire array and return the first HWND.
//
HWND CWindowIterator::First()
{
	::EnumWindows(EnumProc, (LPARAM)this);
	m_current = 0;
	return Next();
}

//////////////////
// Static enumerator passes to virtual fn.
//
BOOL CALLBACK CWindowIterator::EnumProc(HWND hwnd, LPARAM lp)
{
	return ((CWindowIterator*)lp)->OnEnumProc(hwnd);
}

//////////////////
// Virtual enumerator proc: add HWND to array if OnWindow is TRUE.
//
BOOL CWindowIterator::OnEnumProc(HWND hwnd)
{
	if (OnWindow(hwnd)) {
		if (m_count < m_nAlloc)
			m_hwnds[m_count++] = hwnd;
	}
	return TRUE; // keep looking
}

//////////////////
// Main window iterator: special case to iterate main windows of a process.
//
CMainWindowIterator::CMainWindowIterator(DWORD pid, DWORD nAlloc)
	: CWindowIterator(nAlloc)
{
	m_pid = pid;
}

CMainWindowIterator::~CMainWindowIterator()
{
}

//////////////////
// virtual override: is this window a main window of my process?
//
BOOL CMainWindowIterator::OnWindow(HWND hwnd)
{
	if (GetWindowLong(hwnd,GWL_STYLE) & WS_VISIBLE) {
		DWORD pidwin;
		GetWindowThreadProcessId(hwnd, &pidwin);
		if (pidwin==m_pid)
			return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////
// 其他我的函数
////////////////////////////////////////////////////////////////////

int		whprocess_create_with_arg(whpid_t *__pid, const char * __cmdline,const char * __workpath, char *  argv[])
{
	//windows 下没实现
	return -1;
}
int		whprocess_create(whpid_t *__pid, const char *__cmdline, const char *__workpath)
{
	STARTUPINFO			StartupInfo;
	char				*cmdline = NULL;
	PROCESS_INFORMATION	ProcessInfo;
	whvector<char>		vectCmdLine(strlen(__cmdline)+1);
	memcpy(vectCmdLine.getbuf(), __cmdline, vectCmdLine.size());

	*__pid	= INVALID_PID;

	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_SHOW;
	if(!(CreateProcess(	NULL,		//FileName,
						vectCmdLine.getbuf(),
						NULL,
						NULL,
						TRUE,
						CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS,
						NULL,
						__workpath,
						&StartupInfo,
						&ProcessInfo)))
	{
		return	-1;
	}

	CloseHandle(ProcessInfo.hThread);
	*__pid		= ProcessInfo.dwProcessId;

	return		0;
}
int		whprocess_terminate(whpid_t __pid, bool bForce)
{
	if( bForce )
	{
		HANDLE	hP	= OpenProcess(PROCESS_ALL_ACCESS, FALSE, __pid);
		if( !hP )
		{
			// 相应的进程应该不存在
			return	-1;
		}
		::TerminateProcess(hP, -1);
		::CloseHandle(hP);
		return	0;
	}
	else
	{
		CMainWindowIterator	itw(__pid);
		for(HWND hwnd = itw.First(); hwnd; hwnd=itw.Next())
		{
			// do something
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}
		return	0;
	}
}
bool	whprocess_waitend(whpid_t __pid, unsigned int __timeout)
{
	HANDLE	hP	= OpenProcess(PROCESS_ALL_ACCESS, FALSE, __pid);
	if( !hP )
	{
		// 相应的进程应该不存在
		return	true;
	}
	bool	bRst	= false;
	switch( WaitForSingleObject(hP, __timeout) )
	{
		case	WAIT_TIMEOUT:
		break;
		default:
			bRst	= true;
		break;
	}
	// 即使出错也认为是结束了
	CloseHandle(hP);
	return	bRst;
}
int		whprocess_getidentifier()
{
	return	GetCurrentProcessId();
}
bool	whprocess_exists(whpid_t __pid)
{
	CMainWindowIterator	itw(__pid);
	if( itw.First()==NULL )
	{
		return	false;
	}
	else
	{
		return	true;
	}
	/*
	// 只要能用OpenProcess打开就是说明进程存在
	HANDLE	hP	= OpenProcess(PROCESS_DUP_HANDLE, FALSE, __pid);
	if( hP )
	{
		CloseHandle(hP);
		return	true;
	}
	else
	{
		return	false;
	}
	*/
}

#endif							// EOF Windows下的代码	}

////////////////////////////////////////////////////////////////////
#if defined( __GNUC__ )			// Linux下的代码		{
////////////////////////////////////////////////////////////////////
int		whprocess_create_with_arg(whpid_t *__pid, const char * __cmdline,const char * __workpath, char *  argv[])
{
	if( ( (*__pid)=vfork() ) == -1 )
	{
		// 创建出错
		return	-1;
	}
	if( (*__pid)>0 )
	{
		// 这个在父进程中返回
		return	0;
	}
	// 这个在子进程中
	// 假定先不实现带参数的
	whdir_SetCWD(__workpath);
	//脱离主进程控制
	setsid();
	//已经在父进程返回了，这里肯定是子进程
	return   execve(__cmdline, argv, NULL);
	exit(0);
}
int		whprocess_create(whpid_t *__pid, const char *__cmdline, const char *__workpath)
{
	if( ( (*__pid)=vfork() ) == -1 )
	{
		// 创建出错
		return	-1;
	}
	if( (*__pid)>0 )
	{
		// 这个在父进程中返回
		return	0;
	}
	// 这个在子进程中
	// 假定先不实现带参数的
	char	*szArg[1];
	szArg[0]	= NULL;
	whdir_SetCWD(__workpath);
	return  execve(__cmdline, szArg, NULL);
	exit(0);
	
}
int		whprocess_terminate(whpid_t __pid, bool bForce)
{
	if( bForce )
	{
		// 强制杀死
		kill(__pid, SIGKILL);
	}
	else
	{
		// 用term会好一些，如果对方接受term信号就可以正常退出
		kill(__pid, SIGTERM);
	}
	return	0;
}
bool	whprocess_waitend(whpid_t __pid, unsigned int __timeout)
{
	int	nTLeft = __timeout;
	// 50毫秒看一次
	const int	CSTTOSLEEP = 50;
	while(1)
	{
		if( !whprocess_exists(__pid) )
		{
			// Over了
			return	true;
		}
		wh_sleep(CSTTOSLEEP);
		nTLeft	-= CSTTOSLEEP;
		if( nTLeft<=0 )
		{
			break;
		}
	}

	return	false;
}
int		whprocess_getidentifier()
{
	return	getpid();
}
bool	whprocess_exists(whpid_t __pid)
{
	// 只要/proc下面的进程号的目录存在就说明进程存在（进程目录应该是所有用户都可读的）
	char	szTmp[WH_MAX_PATH];
	sprintf(szTmp, "/proc/%u", __pid);
	return	whfile_ispathexisted(szTmp);
}

#endif							// EOF Linux下的代码	}

int		whprocess_createpidfile(const char *__file, whpid_t __pid)
{
	int	rst	= whprocess_trydelpidfile(__file);
	if( rst<0 )
	{
		return	-100 + rst;
	}
	if( __pid == INVALID_PID )
	{
		__pid	= whprocess_getidentifier();
	}
	FILE	*fp	= fopen(__file, "wt");
	if( !fp )
	{
		return	-1;
	}
	fprintf(fp, "%u", __pid);
	fclose(fp);
	return	0;
}
whpid_t	whprocess_getfrompidfile(const char *__file)
{
	whpid_t	pid	= INVALID_PID;
	FILE	*fp	= fopen(__file, "rt");
	if( fp )
	{
		fscanf(fp, "%u", &pid);
		fclose(fp);
	}
	return	pid;
}
int		whprocess_trydelpidfile(const char *__file)
{
	// 先从文件中读出pid
	whpid_t	pid	= whprocess_getfrompidfile(__file);
	if( pid == INVALID_PID )
	{
		// 说明文件不存在，如果存在只能说明把0写入文件了
		return	0;
	}
	// 判断进程是否存在
	if( whprocess_exists(pid) )
	{
		// 不能删除
		return	-1;
	}
	int	rst	= whfile_del(__file);
	if( rst<0 )
	{
		return	-100 + rst;
	}
	return	1;
}
int		whprocess_terminatebypidfile(const char *__file, bool bForce, int nWaitEnd)
{
	whpid_t	pid	= whprocess_getfrompidfile(__file);
	if( pid == INVALID_PID )
	{
		// 文件都不存在则进程应该也不在了
		return	0;
	}
	// 关闭进程
	int	rst	= whprocess_terminate(pid, bForce);
	if( rst<0 )
	{
		return	rst;
	}
	if( nWaitEnd>0 )
	{
		whprocess_waitend(pid, nWaitEnd);
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// whprocess_pidfile
////////////////////////////////////////////////////////////////////
whprocess_pidfile::whprocess_pidfile(const char *__file)
{
	strcpy(m_szFile, __file);
	if( m_szFile[0] )
	{
		whprocess_createpidfile(m_szFile);
	}
}
whprocess_pidfile::~whprocess_pidfile()
{
	if( m_szFile[0] )
	{
		whfile_del(m_szFile);
	}
}

}// EOF	namespace n_whcmn
