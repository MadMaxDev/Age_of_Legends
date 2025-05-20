// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whsignal.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ��signal�İ�װ��Ϊ����2000��linux�¶��á�
//                ������˵��signalһ�㶼�Ǹ�����̨�����õģ�windows�����ֱ������Ϣ�ɡ�
// CreationDate : 2004-02-20

#include "../inc/whsignal.h"
#include "../inc/wh_platform.h"

namespace	n_whcmn
{

#ifdef	WIN32
// ��windows����Ϣת��Ϊsignal
static BOOL WINAPI	WHConsoleHandler(DWORD dwCtrlType)
{
	switch(dwCtrlType)
	{
	case	CTRL_C_EVENT:	// ����Ѿ���single������ctrl-c�Ĵ������ˣ�������ڵڶ���ctrl-cʱ�ű�����
		{
			raise(SIGINT);
			return	TRUE;
		}
		break;
	case	CTRL_CLOSE_EVENT:
	case	CTRL_LOGOFF_EVENT:
	case	CTRL_SHUTDOWN_EVENT:
		{
			raise(SIGTERM);
			return	TRUE;
		}
		break;
	}
	return	FALSE;
}
int		whsignal_init()
{
	// �����ҵĿ���̨��Ϣ������
	return	SetConsoleCtrlHandler(WHConsoleHandler, TRUE) ? 0 : -1;
}
static void (*g_sigfunc)(int sig)	= NULL;
static BOOL WINAPI	WHConsoleHandler_On_ExitFunc(DWORD dwCtrlType)
{
	switch(dwCtrlType)
	{
	case	CTRL_C_EVENT:
		{
			g_sigfunc(SIGINT);
			return	TRUE;
		}
		break;
	case	CTRL_CLOSE_EVENT:
	case	CTRL_LOGOFF_EVENT:
	case	CTRL_SHUTDOWN_EVENT:
		{
			g_sigfunc(SIGTERM);
			return	TRUE;
		}
		break;
	}
	return	FALSE;
}
int		whsingal_set_exitfunc(void (*sigfunc)(int sig))
{
	g_sigfunc	= sigfunc;
	return	SetConsoleCtrlHandler(WHConsoleHandler_On_ExitFunc, TRUE) ? 0 : -1;
}
#endif

#ifdef	__GNUC__
// Linux�±�������TERM�ź�
int		whsignal_init()
{
	return	0;
}
int		whsingal_set_exitfunc(void (*sigfunc)(int sig))
{
	if( signal(SIGINT, sigfunc)==SIG_ERR )
	{
		return	-1;
	}
	if( signal(SIGTERM, sigfunc)==SIG_ERR )
	{
		return	-2;
	}
	return		0;
}
#endif

}				// EOF namespace n_whcmn
