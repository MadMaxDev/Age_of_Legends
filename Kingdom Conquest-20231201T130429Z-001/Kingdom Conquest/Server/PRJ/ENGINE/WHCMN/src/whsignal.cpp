// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whsignal.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 对signal的包装，为了在2000和linux下都用。
//                对我来说，signal一般都是给控制台程序用的，windows程序就直接用消息吧。
// CreationDate : 2004-02-20

#include "../inc/whsignal.h"
#include "../inc/wh_platform.h"

namespace	n_whcmn
{

#ifdef	WIN32
// 把windows的消息转换为signal
static BOOL WINAPI	WHConsoleHandler(DWORD dwCtrlType)
{
	switch(dwCtrlType)
	{
	case	CTRL_C_EVENT:	// 如果已经用single设置了ctrl-c的处理函数了，这个会在第二次ctrl-c时才被调用
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
	// 设置我的控制台消息处理器
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
// Linux下本来就有TERM信号
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
