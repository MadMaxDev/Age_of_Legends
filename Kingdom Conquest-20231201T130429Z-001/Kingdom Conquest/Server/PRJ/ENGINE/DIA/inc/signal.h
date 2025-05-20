// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: signal.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_SIGNAL_H__
#define __DIA_SIGNAL_H__

#include <signal.h>

#ifdef WIN32
BOOL WINAPI tty_close_handler(DWORD dwCtrlType)
{
	switch(dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		raise(SIGTERM);
		return true;
		break;
	default:
		return false;
		break;
	}
}
bool capture_signal()
{
	return SetConsoleCtrlHandler(tty_close_handler, true)? true: false;
}
#endif


#endif

