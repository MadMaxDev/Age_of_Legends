// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdbgmem.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 内存相关的调试
//              : 参考蒋黎的实现
// CreationDate : 2005-12-15

#include "../inc/whdbgmem.h"

#if defined( WIN32 )					// Windows下的实现		{
#include <crtdbg.h>
namespace n_whcmn
{

void	whdbg_check_leak(bool check)
{
#ifdef _DEBUG
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	if(check)
	{
		tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	}
	else
	{
		tmpFlag &= ~_CRTDBG_LEAK_CHECK_DF;
	}
	_CrtSetDbgFlag(tmpFlag);
#endif
}
void	whdbg_SetBreakAlloc(const char *cszBreakPointFile)
{
#ifdef _DEBUG
	// 从cszBreakPointFile中读出各点数值进行设置
	// 下面的用于实时的调试，手动设置，最后设0结束
	int	aid=0;
	if( aid )
	{
		_CrtSetBreakAlloc(aid);
	}
#endif
}

}		// EOF namespace n_whcmn
										// EOF Windows下的实现	}
#else									// 其他系统				{
namespace n_whcmn
{

void	whdbg_check_leak(bool check)
{
}
void	whdbg_SetBreakAlloc(const char *cszBreakPointFile)
{
}

}		// EOF namespace n_whcmn
#endif									// EOF其他系统			}
