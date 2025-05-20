// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whthread.cpp
// Creator: Wei Hua (魏华)
// Comment: 简单的线程函数。
//          这个库的用处是统一windows和linux（可能还有别的系统）下的线程使用方法
// CreationDate: 2003-05-28

#include "../inc/whthread.h"

#if defined( __GNUC__ )			// Linux下的头文件			{
#include "../inc/whhash.h"		// 哈西表用于记录tid到信息结构的映射
#include "../inc/whlock.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#endif							// EOF Linux下的头文件		}

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
#if defined( WIN32 )			// Windows下的代码	{
////////////////////////////////////////////////////////////////////
void *	WHCMN_THREAD_STATIC_INFO_Out()
{
	return	NULL;
}
void	WHCMN_THREAD_STATIC_INFO_In(void *pInfo)
{
}
int		whthread_create(whtid_t *__tid, whthread_func_t __func, void *__param, int __priority)
{
	HANDLE	handle;
	DWORD	threadid;
	// 创建线程
	handle	= ::CreateThread(NULL, 0
		, (LPTHREAD_START_ROUTINE)__func
		, __param, 0, &threadid);
	if( !handle )
	{
		return	-1;
	}

	// 设置优先级
	if( __priority>=THREAD_PRIORIY_IDLE
	&&  __priority<=THREAD_PRIORIY_REALTIME )
	{
		static const DWORD	cstPriority[] =
		{
			IDLE_PRIORITY_CLASS		,
			NORMAL_PRIORITY_CLASS	,
			HIGH_PRIORITY_CLASS		,
			REALTIME_PRIORITY_CLASS	,
		};
		SetPriorityClass(handle, cstPriority[__priority]);
	}

	*__tid	= handle;

	return	0;
}
// 判断线程是否结束
bool	whthread_isend(whtid_t __tid)
{
	return	WaitForSingleObject(__tid, 0) == WAIT_OBJECT_0;
}
bool	whthread_waitend(whtid_t __tid, unsigned int __timeout)
{
	switch(WaitForSingleObject(__tid, __timeout))
	{
		case	WAIT_TIMEOUT:
			return	false;
		break;
	}
	return	true;
}
// 强行结束
int		whthread_terminate(whtid_t __tid)
{
	::TerminateThread(__tid, -1);
	::CloseHandle(__tid);

	return	0;
}
void	whthread_closetid(whtid_t __tid)
{
	::CloseHandle(__tid);
}
int		whthread_num()
{
	return	0;
}
#endif							// EOF Windows下的代码	}

////////////////////////////////////////////////////////////////////
#if defined( __GNUC__ )			// Linux下的代码		{
////////////////////////////////////////////////////////////////////
class	whthread_man;
struct	whthread_info_t
{
	bool			bstopped;	// 线程已经结束
	pthread_t		linuxtid;	// linux的threadid
	pthread_cond_t	cond;
	pthread_mutex_t	mutex;
	whthread_func_t	func;		// 线程函数体
	void *			param;		// 线程函数参数
};
whthread_info_t *	whthread_info_alloc()
{
	whthread_info_t	*info = new whthread_info_t;
	if( !info )
	{
		return	NULL;
	}

	int     rst;
	::pthread_mutexattr_t   attr;
	// 初始化cond和mutex
	rst		= ::pthread_cond_init(&info->cond, NULL);
	if( rst!=0 )
	{
		delete	info;
		return	NULL;
	}

	::pthread_mutexattr_init(&attr);
	::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	rst     = ::pthread_mutex_init(&info->mutex, &attr);
	::pthread_mutexattr_destroy(&attr);
	if( rst!=0 )
	{
		::pthread_cond_destroy(&info->cond);
		delete	info;
		return	NULL;
	}

	return	info;
}
void	whthread_info_free( whthread_info_t * info)
{
	if( info )
	{
		::pthread_cond_destroy(&info->cond);
		::pthread_mutex_destroy(&info->mutex);
		delete	info;
	}
}
struct	_whtid_hashfunc
{
	_whtid_hashfunc() {}
	_whtid_hashfunc(const _whtid_hashfunc & hf) {}
	inline size_t	operator()(whtid_t data)	const	{return data;}
};

class	whthread_man
{
private:
	n_whcmn::whhash<whtid_t, whthread_info_t *, whcmnallocationobj, _whtid_hashfunc>	m_mapper;
	n_whcmn::whlock	m_mylk;
public:
	inline void	lock()
	{
		m_mylk.lock();
	}
	inline void	unlock()
	{
		m_mylk.unlock();
	}
	inline whthread_info_t *	getinfo(whtid_t __tid)
	{
		whthread_info_t	*info=NULL;
		if( !m_mapper.get(__tid, info) )
		{
			return	NULL;
		}
		return		info;
	}
public:
	whthread_man();
	~whthread_man();
	int		create(whtid_t *__tid, whthread_func_t __func, void *__param, int __priority);
	int		terminate(whtid_t __tid);
	bool	add(whtid_t __tid, whthread_info_t * __info);
	bool	removeandfree(whtid_t __tid);
	bool	waitend(whtid_t __tid, unsigned int timeout);
	bool	isend(whtid_t __tid);
	int		getnum();
};
whthread_man::whthread_man()
{
}
whthread_man::~whthread_man()
{
}
static void *	whthread_proc(void *ptr);
static whtid_t	g_tidcount = 0;
int	whthread_man::create(whtid_t *__tid, whthread_func_t __func, void *__param, int __priority)
{
	whthread_info_t	*info = whthread_info_alloc();
	if( !info )
	{
		return	-1;
	}
	// 填写参数
	info->func	= __func;
	info->param	= __param;

	while(1)
	{
		// 这个会死循环的前提是生成了0xFFFFFFFF个线程没有释放
		g_tidcount	++;
		if( g_tidcount == 0 )
		{
			continue;
		}
		if( m_mapper.get(g_tidcount) )
		{
			continue;
		}
		break;
	}

	//
	int	rst;

	// 属性结构
	::pthread_attr_t	attr;
	rst		= ::pthread_attr_init(&attr);
	if( rst!=0 )
	{
		//
		whthread_info_free(info);
		return	-2;
	}

	lock();
	// 创建线程
	pthread_t	tid;

#ifdef	__SIZEOF_PTHREAD_ATTR_T
	// GCC 4以后的attr就变了
	// 用法不清楚，所以就用NULL使用默认attr了
	rst		= ::pthread_create(&tid, NULL, whthread_proc, (void *)g_tidcount);
#else
	// detach状态
	attr.__detachstate	= PTHREAD_CREATE_DETACHED;
	// 设置优先级
	if( __priority>=THREAD_PRIORIY_IDLE
	&&  __priority<=THREAD_PRIORIY_REALTIME )
	{
		// 预定义的优先级
		struct	PRIORITY_T
		{
			int		policy;
			int		priority;
		};
		static const PRIORITY_T	cstPriority[] =
		{
			{ SCHED_OTHER	, 0 }	,
			{ SCHED_RR		, 1 }	,
			{ SCHED_RR		, 2 }	,
			{ SCHED_RR		, 3 }	,
		};
		attr.__schedpolicy					= cstPriority[__priority].policy;
		attr.__schedparam.__sched_priority	= cstPriority[__priority].priority;
	}

	rst		= ::pthread_create(&tid, &attr, whthread_proc, (void *)g_tidcount);
#endif
	if( rst!=0 )
	{
		::pthread_attr_destroy(&attr);
		whthread_info_free(info);
		unlock();
		return	-3;
	}
	info->linuxtid	= tid;

	// 如果后边没有错误就返回这个0
	rst	= 0;
	if( !add(g_tidcount, info)!=0 )
	{
		// 终结线程
		::pthread_cancel(tid);
		// 删除信息结构
		whthread_info_free(info);
		// 表示出错
		rst	= -4;
	}
	unlock();
	::pthread_attr_destroy(&attr);

	*__tid	= g_tidcount;

	// 把管理器交给子线程
	::pthread_detach(tid);

	return	rst;
}
int	whthread_man::terminate(whtid_t __tid)
{
	whthread_info_t	*info;
	// 查找该tid是否存在
	lock();
	info	= getinfo(__tid);
	if( info )
	{
		::pthread_cancel(info->linuxtid);
		// 删除结构
		removeandfree(__tid);
	}
	unlock();

	return	0;
}
bool	whthread_man::isend(whtid_t __tid)
{
	whthread_info_t	*info;

	// 获得信息结构
	lock();
	info	= getinfo(__tid);
	unlock();
	if( !info )
	{
		// 线程结构不存在说明应该已经退出了
		return	true;
	}
	return	false;
}
bool	whthread_man::waitend(whtid_t __tid, unsigned int timeout)
{
	int				rst;
	whthread_info_t	*info;

	// 获得信息结构
	lock();
	info	= getinfo(__tid);
	unlock();
	if( !info )
	{
		// 线程结构不存在说明应该已经退出了
		return	true;
	}
	::pthread_mutex_lock(&info->mutex);
	if( info->bstopped )
	{
		// 应该刚刚成功退出了
		::pthread_mutex_unlock(&info->mutex);
		return	true;
	}
	// 等待
	struct timeval  now;
	struct timespec tw;

	gettimeofday(&now, NULL);
	tw.tv_nsec	= now.tv_usec + timeout*1000;		// 现在是微秒
	tw.tv_sec	= now.tv_sec + tw.tv_nsec/1000000;	// 超出1秒部分加到这里
	tw.tv_nsec	= (tw.tv_nsec % 1000000) * 1000;	// 现在剩下的部分变成纳秒

	rst	= ::pthread_cond_timedwait(&info->cond, &info->mutex, &tw);
	::pthread_mutex_unlock(&info->mutex);

	bool	bval = true;
	switch(rst)
	{
		case	ETIMEDOUT:
			bval	= false;
		break;
		case	EINTR:
			// 线程被别人中断了，所以我要帮助它释放信息结构
			lock();
			removeandfree(__tid);
			unlock();
		break;
		default:
			// 线程自己结束了
		break;
	}

	return	bval;
}
int		whthread_man::getnum()
{
	return	m_mapper.size();
}

bool	whthread_man::add(whtid_t __tid, whthread_info_t * __info)
{
	if( !m_mapper.put(__tid, __info) )
	{
		return	false;
	}
	return	true;
}
bool	whthread_man::removeandfree(whtid_t __tid)
{
	whthread_info_t	*info = NULL;
	if( !m_mapper.geterase(__tid, info) )
	{
		// 没有这个东东
		return	false;
	}
	whthread_info_free(info);
	return	true;
}

// 唯一的管理器对象
static whthread_man	g_tman_dummy;
static whthread_man	*g_tman	= &g_tman_dummy;
void *	WHCMN_THREAD_STATIC_INFO_Out()
{
	return	g_tman;
}
void	WHCMN_THREAD_STATIC_INFO_In(void *pInfo)
{
#ifdef	_DEBUG
	printf("WHCMN_THREAD_STATIC_INFO_In %p %p%s", g_tman, pInfo, WHLINEEND);
#endif
	g_tman	= (whthread_man *)pInfo;
}

// 名义上的线程体
static void *	whthread_proc(void *ptr)
{
	whtid_t	tid = (whtid_t)ptr;
	whthread_info_t	*info;
	whthread_func_t func;
	void	* param, * vrst;


	// 获得线程的信息结构
	g_tman->lock();
	info	= g_tman->getinfo(tid);
	assert(info);
	info->bstopped	= false;
	func			= info->func;
	param			= info->param;
	g_tman->unlock();

	// 线程内容
	vrst	= (*func)(param);

	// 结束通知
	g_tman->lock();
	info	= g_tman->getinfo(tid);
	if( info )	// 如果info为空说明该线程已经被强行释放了
	{
		::pthread_mutex_lock(&info->mutex);
		info->bstopped	= true;
		::pthread_cond_signal(&info->cond);
		::pthread_mutex_unlock(&info->mutex);

		// 让等待的线程有机会成功返回
		usleep(10000);
		// 释放该结构
		g_tman->removeandfree(tid);
	}
	g_tman->unlock();

	return	vrst;
}

int		whthread_create(whtid_t *__tid, whthread_func_t __func, void *__param, int __priority)
{
	return	g_tman->create(__tid, __func, __param, __priority);
}
int		whthread_terminate(whtid_t __tid)
{
	return	g_tman->terminate(__tid);
}
void	whthread_closetid(whtid_t __tid)
{
// detach要在create之后用来把控制权完全交给子线程，不是用来close的。而且也要用linux的tid，不是我自己的这个！
//	pthread_detach(__tid);
}
bool	whthread_isend(whtid_t __tid)
{
	return	g_tman->isend(__tid);
}
bool	whthread_waitend(whtid_t __tid, unsigned int timeout)
{
	return	g_tman->waitend(__tid, timeout);
}
int		whthread_num()
{
	return	g_tman->getnum();
}

#endif							// EOF Linux下的代码	}

int	whthread_waitend_or_terminate(whtid_t tid, unsigned int __timeout)
{
	if( whthread_waitend(tid, __timeout) )
	{
		whthread_closetid(tid);
		return	0;
	}
	if( whthread_terminate(tid)<0 )
	{
		return	-1;
	}
	// 这个表示是强行结束的
	return		1;
}


}// EOF	namespace n_whcmn
