// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whsignal.h
// Creator      : Wei Hua (魏华)
// Comment      : 对signal的包装，为了在2000和linux下都用。
//                只要调用了whsignal_init，就可以使用signal来设置特定信号的处理函数
//                对我来说，signal一般都是给控制台程序用的，windows程序就直接用消息吧。
//                需要注意：如果在线程中进行输入，则按了ctrl-c后会使输入无法继续进行!!!! 不过也好，反正按下ctrl-c了就是不希望再继续了。
// CreationDate : 2004-02-20
// ChangeLOG    : 2006-08-16 增加了whsingal_set_exitfunc函数，这样在windows下连续按ctrl-c或者关闭钮就可以被截获了

#ifndef	__WHSIGNAL_H__
#define	__WHSIGNAL_H__

#include <signal.h>

namespace	n_whcmn
{

// 对使用signal的初始化（主要为了在windows下可以产生TERM消息）
int		whsignal_init();

// 如果希望简单一些，可以直接使用下面的函数
// 这样就可以在ctrl-c或关闭应用程序的时候调用指定的函数
int		whsingal_set_exitfunc(void (*sigfunc)(int sig));

}				// EOF namespace n_whcmn

#endif	// EOF __WHSIGNAL_H__
