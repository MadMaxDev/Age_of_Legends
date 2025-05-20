// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdbgmem.h
// Creator      : Wei Hua (魏华)
// Comment      : 内存相关的调试
//              : 参考蒋黎的实现
// CreationDate : 2005-12-15

#ifndef __WHDBGMEM_H__
#define	__WHDBGMEM_H__

namespace n_whcmn
{

// 这两个函数只在_DEBUG定义的情况下才有内容
// 设置是否在程序结束的时候Dump内存泄漏
void	whdbg_check_leak(bool check);
// 调试时设置break断点
// cszBreakPointFile中是一个文本文件，记录了需要中断的地方
void	whdbg_SetBreakAlloc(const char *cszBreakPointFile=0);

}		// EOF namespace n_whcmn

#endif
