// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace : n_whcmn
// File      : whdaemon_linux.cpp
// Creator   : Wei Hua (魏华)
// Comment   : 守护进程辅助函数集（仅在Linux下有用）

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

// 最多的文件句柄数量36
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

	// 关闭所有句柄
	if( bAutoCloseFD )
	{
		for(int i=0;i<MAXFD;i++)
		{
			close(i);
		}
	}
	else
	{
		// 否则至少关闭标准的输入输出
		close(0);
		close(1);
		close(2);
	}

	// 变成session leader
	setsid();

	signal(SIGHUP, SIG_IGN);

	// 结束第一个子进程
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
	// WIN32下没有daemon的概念，跳过去即可
	printf("Transform to daemon ...\n");
	return	0;
}

}	// EOF namespace n_whcmn

#endif
