#include "WHCMN/inc/whprocess.h"
#include "WHCMN/inc/whsignal.h"
#include "WHCMN/inc/whvector.h"
#include "WHCMN/inc/whtime.h"
#include "WHCMN/inc/whfile.h"

using namespace n_whcmn;

static bool	g_bStop	= false;
static void	stop_func(int sig)
{
	switch (sig)
	{
	case SIGINT:
	case SIGTERM:
		{
			g_bStop	= true;
		}
		break;
	}
}

static	whvector<whpid_t>	g_vectPids;
#define cmd_line_length		256

int		main(int argc, char* argv[])
{
	if (argc < 2)
	{
		return 0;
	}
	whfile*	pFile	= whfile_OpenCmnFile(argv[1], "rt");
	if (pFile == NULL)
	{
		printf("file not exist:%s%s", argv[1], WHLINEEND);
		return 0;
	}

	char		szBuf[cmd_line_length];
	char		szExePath[WH_MAX_PATH];
	char		szCfgFile[WH_MAX_PATH];
	char		szWorkDir[WH_MAX_PATH];
	int			nReadSize	= 0;
	whpid_t		pid			= INVALID_PID;
	int			nRet		= 0;

	while (!pFile->IsEOF())
	{
		nReadSize	= pFile->ReadLine(szBuf, cmd_line_length);
		if (nReadSize <= 0)
		{
			break;
		}
		wh_strsplit("ss", szBuf, "", szExePath, szCfgFile);
		sprintf(szBuf, "%s %s", szExePath, szCfgFile);		// 这个是为了去掉换行
		if (whfile_getfilepath(szExePath, szWorkDir))
		{
			nRet	= whprocess_create(&pid, szBuf, szWorkDir);
		}
		else
		{
			nRet	= whprocess_create(&pid, szBuf, ".");
		}
		if (nRet == 0)
		{
			g_vectPids.push_back(pid);
		}
	}

	whsingal_set_exitfunc(stop_func);
	while (!g_bStop)
	{
		wh_sleep(1000);
	}

	int		nSize	= g_vectPids.size();
	for (int i=0; i<nSize; i++)
	{
		whprocess_terminate(g_vectPids[i], false);//true的话需要获取的权限太高,容易失败(你情我愿才是王道嘛)
	}

	whfile_CloseCmnFile(pFile);

	return 0;
}
