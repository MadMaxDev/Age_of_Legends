// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : wh_platform.h
// Creator      : Wei Hua (κ��)
// Comment      : ���Ҫ����ϵͳ�ļ��Ļ����Ͱ������
// CreationDate : 2005-09-22
// ChangeLOG    : 2006-04-30 �޸�FD_SETSIZE����Ϊ256��windowsĬ�ϵ���64
//              : 2007-08-23 ������NOTUSE�꣬�����Щ�������ⲻ�ñ�warning������

#ifndef	__WH_PLATFORM_H__
#define	__WH_PLATFORM_H__

#if defined( WIN32 )					// Windows�µ�ͷ		{
//#define	FD_SETSIZE	256				// ����Ҫ��ô����
#include <winsock2.h>
#if !defined(_AFX)
#include <windows.h>
#endif
#endif									//						}

#if defined( __GNUC__ )					// Linux�µ�ͷ			{
#endif									//						}

#define	NOTUSE(x)	(void)x;

#endif	// EOF __WH_PLATFORM_H__
