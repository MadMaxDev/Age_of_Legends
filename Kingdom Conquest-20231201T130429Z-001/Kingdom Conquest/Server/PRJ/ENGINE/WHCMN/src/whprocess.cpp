// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whprocess.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵Ľ�����غ�����
//                �������ô���ͳһwindows��linux�����ܻ��б��ϵͳ���µĽ������ʹ�÷���
// CreationDate : 2003-08-07
// ChangeLOG    : 2007-07-12 �޸���windows�µ�whprocess_exists��ʹ�ô���ö�����ж��Ƿ�pid���ڡ�ԭ��OpenProcess�ķ������ܻᵼ�¸��������ж��Ѿ����رյ��ӽ��̻����ڡ�
//              : 2007-11-12 ��whprocess_pidfile������������ļ�Ϊ���򲻴�����ɾ���ļ�

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
#if defined( WIN32 )			// Windows�µĴ���	{
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// �ӱ������￼�����Ļ�ȡ���������ڵĴ���
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
// �����ҵĺ���
////////////////////////////////////////////////////////////////////

int		whprocess_create_with_arg(whpid_t *__pid, const char * __cmdline,const char * __workpath, char *  argv[])
{
	//windows ��ûʵ��
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
			// ��Ӧ�Ľ���Ӧ�ò�����
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
		// ��Ӧ�Ľ���Ӧ�ò�����
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
	// ��ʹ����Ҳ��Ϊ�ǽ�����
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
	// ֻҪ����OpenProcess�򿪾���˵�����̴���
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

#endif							// EOF Windows�µĴ���	}

////////////////////////////////////////////////////////////////////
#if defined( __GNUC__ )			// Linux�µĴ���		{
////////////////////////////////////////////////////////////////////
int		whprocess_create_with_arg(whpid_t *__pid, const char * __cmdline,const char * __workpath, char *  argv[])
{
	if( ( (*__pid)=vfork() ) == -1 )
	{
		// ��������
		return	-1;
	}
	if( (*__pid)>0 )
	{
		// ����ڸ������з���
		return	0;
	}
	// ������ӽ�����
	// �ٶ��Ȳ�ʵ�ִ�������
	whdir_SetCWD(__workpath);
	//���������̿���
	setsid();
	//�Ѿ��ڸ����̷����ˣ�����϶����ӽ���
	return   execve(__cmdline, argv, NULL);
	exit(0);
}
int		whprocess_create(whpid_t *__pid, const char *__cmdline, const char *__workpath)
{
	if( ( (*__pid)=vfork() ) == -1 )
	{
		// ��������
		return	-1;
	}
	if( (*__pid)>0 )
	{
		// ����ڸ������з���
		return	0;
	}
	// ������ӽ�����
	// �ٶ��Ȳ�ʵ�ִ�������
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
		// ǿ��ɱ��
		kill(__pid, SIGKILL);
	}
	else
	{
		// ��term���һЩ������Է�����term�źžͿ��������˳�
		kill(__pid, SIGTERM);
	}
	return	0;
}
bool	whprocess_waitend(whpid_t __pid, unsigned int __timeout)
{
	int	nTLeft = __timeout;
	// 50���뿴һ��
	const int	CSTTOSLEEP = 50;
	while(1)
	{
		if( !whprocess_exists(__pid) )
		{
			// Over��
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
	// ֻҪ/proc����Ľ��̺ŵ�Ŀ¼���ھ�˵�����̴��ڣ�����Ŀ¼Ӧ���������û����ɶ��ģ�
	char	szTmp[WH_MAX_PATH];
	sprintf(szTmp, "/proc/%u", __pid);
	return	whfile_ispathexisted(szTmp);
}

#endif							// EOF Linux�µĴ���	}

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
	// �ȴ��ļ��ж���pid
	whpid_t	pid	= whprocess_getfrompidfile(__file);
	if( pid == INVALID_PID )
	{
		// ˵���ļ������ڣ��������ֻ��˵����0д���ļ���
		return	0;
	}
	// �жϽ����Ƿ����
	if( whprocess_exists(pid) )
	{
		// ����ɾ��
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
		// �ļ��������������Ӧ��Ҳ������
		return	0;
	}
	// �رս���
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
