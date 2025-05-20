// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace : n_whcmn
// File      : whdaemon_linux.cpp
// Creator   : Wei Hua (κ��)
// Comment   : �ػ����̸���������������Linux�����ã�

#ifdef	__GNUC__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include "../inc/whstring.h"

namespace n_whcmn
{

// �����ļ��������36
#define	MAXFD	64

#include "../inc/whdaemon.h"

int		whdaemon_init(bool bAutoCloseFD)
{
	printf("Transform to daemon ... ");

	pid_t	pid;

	if( (pid=fork()) < 0 )
	{
		printf("Failed!%s", WHLINEEND);
		return	-1;
	}
	else if( pid!=0 )
	{
		// parent goes bye-bye
		printf("SUCCESS, pid might be:%d!%s", pid+1, WHLINEEND);
		exit(0);
	}

	// �ر����о��
	if( bAutoCloseFD )
	{
		for(int i=0;i<MAXFD;i++)
		{
			close(i);
		}
	}
	else
	{
		// �������ٹرձ�׼���������
		close(0);
		close(1);
		close(2);
	}

	// ���session leader
	setsid();

	signal(SIGHUP, SIG_IGN);

	// ������һ���ӽ���
	if( (pid=fork()) != 0 )
	{
		exit(0);
	}

	umask(0);

	return	0;
}

}	// EOF namespace n_whcmn

#endif

#ifdef	WIN32
#include <stdio.h>

namespace n_whcmn
{

int		whdaemon_init(bool bAutoCloseFD)
{
	// WIN32��û��daemon�ĸ������ȥ����
	printf("Transform to daemon ...\n");
	return	0;
}

}	// EOF namespace n_whcmn

#endif
