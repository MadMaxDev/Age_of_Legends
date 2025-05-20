// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : ��
// File         : TCPReceiver.cpp
// Creator      : Wei Hua (κ��)
// Comment      : TCPReceiverģ��
// CreationDate : 2007-02-28
// ChangeLog    :

#ifdef	WIN32
#include <WHCMN/inc/wh_platform.h>
// ֻ����windows�������Ҫ�����
#pragma comment(lib, "WS2_32")
#endif

#include <PNGS/inc/pngs_TCPReceiver.h>
#include <WHCMN/inc/wh_dllstaticmodule.h>
#include <WHCMN/inc/whdbgmem.h>

using namespace n_pngs;

#if	defined(WIN32) && !defined(_AFX)
// ֻ����windows������û��MFC����Ҫʵ��DllMain����
BOOL APIENTRY DllMain(
					  HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

extern "C" WH_DLL_API	n_pngs::CMN::ILogic *	TCPReceiver_Create()
{
	whdbg_SetBreakAlloc();
	return	new PNGS_TCPRECEIVER;
}
extern "C" WH_DLL_API	n_pngs::CMN::ILogic *	TCPReceiver_DummyCmdDealer_Create()
{
	whdbg_SetBreakAlloc();
	return	new PNGS_TCPRECEIVER_DUMMYCMDDEALER;
}
