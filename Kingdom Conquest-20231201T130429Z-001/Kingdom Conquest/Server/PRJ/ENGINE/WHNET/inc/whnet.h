// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File: whnet.h
// Creator: Wei Hua (魏华)
// Comment: 简单的网络功能的全包含头文件。要使用whnet只需要包含这个就可以了。
//          在windows下需要现实包含下面的库，只要用到和网络相关的功能就要这样做
//          #pragma comment(lib, "wsock32.lib")
//          或者 #pragma comment(lib, "WS2_32.lib") （其实用前面就可以了）
// CreationDate: 2003-05-14

#ifndef	__WHNET_H__
#define	__WHNET_H__

#include "whnet_def.h"
#include "whnetcmn.h"
#include "whnettcp.h"
#include "whnetudp.h"
#include "whnetraw.h"
// #include "whnetcnl.h" 这个是比较特殊的，不用放在对外公共头中

// 一般在主cpp中应该有，以保证在win32下连接通过
// #ifdef	WIN32
// #pragma comment(lib, "WS2_32")
// #endif

#endif	// EOF __WHNET_H__
