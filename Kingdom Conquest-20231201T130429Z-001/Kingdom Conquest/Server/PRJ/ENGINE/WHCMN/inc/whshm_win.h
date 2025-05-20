// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File     : whshm_win.h
// Creator  : Wei Hua (魏华)
// Comment  :
//          简单的共享内存功能的WIN32实现版本
//          create和destroy都需要通过pipe访问本地的shmserver
//          为了简便和可扩充性，全部使用字串作为命令和返回
//          指令和返回说明：
//          1. 创建
//          指令：create strname, size
//          返回：
//          成功：OK
//          失败：ERR code, codestr
//          说明：如果原来没有，且创建成功，则返回OK。
//                如果原来有，则返回ERR 名字已经存在
//          2. 销毁
//          指令：destroy strname
//          成功：OK
//          失败：ERR code, codestr
//          ///////////////////////
//          错误代码：
//          0     没有错误
//          1     系统错误(如内存不足无法分配)
//          11    名字已经存在
//          12    名字不存在
// CreationDate: 2004-01-20
// ChangeLOG   : 2005年底 修改为pipe版本

#ifdef	WIN32	// { WIN32 BEGIN

#ifndef	__WHSHM_WIN_H__
#define	__WHSHM_WIN_H__

#include "./whshm_cmn.h"

namespace	n_whcmn
{

// 内部使用的一些结构

// 内部使用的一些函数
// 通过nKey创建出一个名字
const char *	i_whshm_makename(int nKey, char *szName);
const char *	i_whshm_makename(int nKey);
// 创建的具体动作(whshm_raw_create内部通过shmserver做，shmserver通过这个做)
int		i_whshm_raw_create(const char *cszKeyName, size_t nSize, WHSHM_RAW_INFO_T *pInfo);
int		i_whshm_raw_open(const char *cszKeyName, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr);

}				// EOF namespace n_whcmn

#endif			// EOF __WHSHM_WIN_H__

#endif			// } EOF WIN32
