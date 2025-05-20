// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whthread.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的线程函数。
//                这个库的用处是统一windows和linux（可能还有别的系统）下的线程使用方法
// CreationDate : 2003-05-28

#ifndef	__WHTHREAD_H__
#define	__WHTHREAD_H__

#include "../inc/wh_platform.h"

namespace n_whcmn{

void *	WHCMN_THREAD_STATIC_INFO_Out();
void	WHCMN_THREAD_STATIC_INFO_In(void *pInfo);

// 标准定义为 (和linux保持一致)
// void * whthread_func(void *);
typedef	void * (*whthread_func_t)(void *);
// 线程id
#ifdef	WIN32
typedef HANDLE				whtid_t;
#else
typedef unsigned int		whtid_t;
#endif
// 非法的线程id
const whtid_t INVALID_TID	= (whtid_t)0;

// tid中返回我的线程id (从0开始)
// __priority为-1表示默认的优先级
enum
{
	THREAD_PRIORIY_IDLE		= 0,
	THREAD_PRIORIY_NORMAL	= 1,
	THREAD_PRIORIY_HIGH		= 2,
	THREAD_PRIORIY_REALTIME	= 3,
};
int		whthread_create(whtid_t *__tid, whthread_func_t __func, void *__param, int __priority = -1);
// 强行结束
int		whthread_terminate(whtid_t __tid);
// 关闭tid句柄(注意，whthread_create出来的线程结束后，
// 上层必须用whthread_closetid关闭句柄，或者whthread_terminate结束之)
void	whthread_closetid(whtid_t __tid);
// 判断线程是否结束
bool	whthread_isend(whtid_t __tid);
// 等待线程结束，返回false表示超时
// timeout为等待时间，默认为0，即不等等待
bool	whthread_waitend(whtid_t __tid, unsigned int __timeout = 0);
// 获得总的线程数量
int		whthread_num();
// 等待结束，如果不结束就term之
// 返回：	0	表示正常结束
//			1	表示强行结束
//			-1	表示结束失败
int	whthread_waitend_or_terminate(whtid_t tid, unsigned int __timeout);

}// EOF	namespace wh_genericfunc

#endif	// EOF __WHTHREAD_H__
