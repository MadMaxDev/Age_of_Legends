// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdbgmem.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �ڴ���صĵ���
//              : �ο������ʵ��
// CreationDate : 2005-12-15

#include "../inc/whdbgmem.h"

#if defined( WIN32 )					// Windows�µ�ʵ��		{
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
	// ��cszBreakPointFile�ж���������ֵ��������
	// ���������ʵʱ�ĵ��ԣ��ֶ����ã������0����
	int	aid=0;
	if( aid )
	{
		_CrtSetBreakAlloc(aid);
	}
#endif
}

}		// EOF namespace n_whcmn
										// EOF Windows�µ�ʵ��	}
#else									// ����ϵͳ				{
namespace n_whcmn
{

void	whdbg_check_leak(bool check)
{
}
void	whdbg_SetBreakAlloc(const char *cszBreakPointFile)
{
}

}		// EOF namespace n_whcmn
#endif									// EOF����ϵͳ			}
