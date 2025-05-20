// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace : n_whcmn
// File      : whdaemon.h
// Creator   : Wei Hua (魏华)
// Comment   : 守护进程辅助函数集（仅在Linux下使用）
// ChangeLOG :

#ifndef	__WHDAEMON_H__
#define	__WHDAEMON_H__

namespace n_whcmn
{

// bAutoCloseFD表示是否需要在子进程中关闭所有父进程创建的文件句柄
// 注意：如果bAutoCloseFD为真，则在使用whdaemon_init之前不要建立任何socket，否则会被关闭!!!!
int		whdaemon_init(bool bAutoCloseFD=false);

}

#endif	// EOF __WHDAEMON_H__
