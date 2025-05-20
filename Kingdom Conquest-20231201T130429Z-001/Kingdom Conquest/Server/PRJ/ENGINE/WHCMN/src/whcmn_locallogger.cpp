#include "../inc/whcmn_locallogger.h"

namespace n_whcmn
{
static whlogwriter*	g_pLocalLogWriter	= NULL;

int		LLogger_Init(const char* cszLocalLogPrefix)
{
	if (cszLocalLogPrefix==NULL || cszLocalLogPrefix[0]==0)
	{
		return 0;
	}
	whlogwriter::INFO_T	info;
	sprintf(info.szLogFilePrefix, cszLocalLogPrefix);
	g_pLocalLogWriter				= new whlogwriter();
	if (g_pLocalLogWriter->Init(&info) < 0)
	{
		WHSafeDelete(g_pLocalLogWriter);
		return -1;
	}
	return 0;
}
int		LLogger_Release()
{
	WHSafeDelete(g_pLocalLogWriter);
	return 0;
}
int		LLogger_Tick()
{
	if (g_pLocalLogWriter)
	{
		return g_pLocalLogWriter->Tick();
	}
	return 0;
}
int		LLogger_WriteFmt(const char* cszFmt, ...)
{
	if (g_pLocalLogWriter == NULL)
	{
		return 0;
	}
	char		szLogBuf[1024];
	va_list		argList;
	va_start(argList, cszFmt);
	vsprintf(szLogBuf, cszFmt, argList);
	va_end(argList);
	return g_pLocalLogWriter->WriteLine(szLogBuf);
}

//added by yuezhongyue 2011-08-10,dllº‰g_pLocalLogWriter÷∏’Îπ≤œÌ
whlogwriter*	WHCMN_Get_LocalLogWriter()
{
	return g_pLocalLogWriter;
}
void			WHCMN_Set_LocalLogWriter(whlogwriter* pLogWriter)
{
	g_pLocalLogWriter	= pLogWriter;
}
}
