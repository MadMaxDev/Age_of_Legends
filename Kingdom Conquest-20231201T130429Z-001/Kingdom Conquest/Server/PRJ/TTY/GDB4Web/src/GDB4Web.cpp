#ifdef WIN32
#include "WHCMN/inc/wh_platform.h"
#pragma comment(lib, "WS2_32")
#endif

#include "PNGS/inc/pngs_cmn.h"
#include "WHCMN/inc/wh_dllstaticmodule.h"
#include "WHCMN/inc/whdbgmem.h"
#include "../../GDB4Web_Lib/inc/GDB4Web_i.h"

using namespace n_pngs;

#if defined(WIN32) && !defined(_AFX)
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
extern "C" WH_DLL_API	n_pngs::CMN::ILogic *	GDB4Web_Create()
{
	whdbg_SetBreakAlloc();
	return	new GDB4Web_i;
}
