// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : wh_platform.h
// Creator      : Wei Hua (魏华)
// Comment      : 如果要包含系统文件的话，就包含这个
// CreationDate : 2005-09-22
// ChangeLOG    : 2006-04-30 修改FD_SETSIZE定义为256，windows默认的是64
//              : 2007-08-23 增加了NOTUSE宏，解决有些变量故意不用报warning的问题

#ifndef	__WH_PLATFORM_H__
#define	__WH_PLATFORM_H__

#if defined( WIN32 )					// Windows下的头		{
//#define	FD_SETSIZE	256				// 不用要这么多了
#include <winsock2.h>
#if !defined(_AFX)
#include <windows.h>
#endif
#endif									//						}

#if defined( __GNUC__ )					// Linux下的头			{
#endif									//						}

#define	NOTUSE(x)	(void)x;

#endif	// EOF __WH_PLATFORM_H__
