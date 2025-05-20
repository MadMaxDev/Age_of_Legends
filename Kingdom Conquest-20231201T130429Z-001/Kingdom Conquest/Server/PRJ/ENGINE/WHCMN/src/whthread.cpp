// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whthread.cpp
// Creator: Wei Hua (κ��)
// Comment: �򵥵��̺߳�����
//          �������ô���ͳһwindows��linux�����ܻ��б��ϵͳ���µ��߳�ʹ�÷���
// CreationDate: 2003-05-28

#include "../inc/whthread.h"

#if defined( __GNUC__ )			// Linux�µ�ͷ�ļ�			{
#include "../inc/whhash.h"		// ���������ڼ�¼tid����Ϣ�ṹ��ӳ��
#include "../inc/whlock.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#endif							// EOF Linux�µ�ͷ�ļ�		}

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
#if defined( WIN32 )			// Windows�µĴ���	{
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
	// �����߳�
	handle	= ::CreateThread(NULL, 0
		, (LPTHREAD_START_ROUTINE)__func
		, __param, 0, &threadid);
	if( !handle )
	{
		return	-1;
	}

	// �������ȼ�
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
// �ж��߳��Ƿ����
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
// ǿ�н���
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
#endif							// EOF Windows�µĴ���	}

////////////////////////////////////////////////////////////////////
#if defined( __GNUC__ )			// Linux�µĴ���		{
////////////////////////////////////////////////////////////////////
class	whthread_man;
struct	whthread_info_t
{
	bool			bstopped;	// �߳��Ѿ�����
	pthread_t		linuxtid;	// linux��threadid
	pthread_cond_t	cond;
	pthread_mutex_t	mutex;
	whthread_func_t	func;		// �̺߳�����
	void *			param;		// �̺߳�������
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
	// ��ʼ��cond��mutex
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
	// ��д����
	info->func	= __func;
	info->param	= __param;

	while(1)
	{
		// �������ѭ����ǰ����������0xFFFFFFFF���߳�û���ͷ�
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

	// ���Խṹ
	::pthread_attr_t	attr;
	rst		= ::pthread_attr_init(&attr);
	if( rst!=0 )
	{
		//
		whthread_info_free(info);
		return	-2;
	}

	lock();
	// �����߳�
	pthread_t	tid;

#ifdef	__SIZEOF_PTHREAD_ATTR_T
	// GCC 4�Ժ��attr�ͱ���
	// �÷�����������Ծ���NULLʹ��Ĭ��attr��
	rst		= ::pthread_create(&tid, NULL, whthread_proc, (void *)g_tidcount);
#else
	// detach״̬
	attr.__detachstate	= PTHREAD_CREATE_DETACHED;
	// �������ȼ�
	if( __priority>=THREAD_PRIORIY_IDLE
	&&  __priority<=THREAD_PRIORIY_REALTIME )
	{
		// Ԥ��������ȼ�
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

	// ������û�д���ͷ������0
	rst	= 0;
	if( !add(g_tidcount, info)!=0 )
	{
		// �ս��߳�
		::pthread_cancel(tid);
		// ɾ����Ϣ�ṹ
		whthread_info_free(info);
		// ��ʾ����
		rst	= -4;
	}
	unlock();
	::pthread_attr_destroy(&attr);

	*__tid	= g_tidcount;

	// �ѹ������������߳�
	::pthread_detach(tid);

	return	rst;
}
int	whthread_man::terminate(whtid_t __tid)
{
	whthread_info_t	*info;
	// ���Ҹ�tid�Ƿ����
	lock();
	info	= getinfo(__tid);
	if( info )
	{
		::pthread_cancel(info->linuxtid);
		// ɾ���ṹ
		removeandfree(__tid);
	}
	unlock();

	return	0;
}
bool	whthread_man::isend(whtid_t __tid)
{
	whthread_info_t	*info;

	// �����Ϣ�ṹ
	lock();
	info	= getinfo(__tid);
	unlock();
	if( !info )
	{
		// �߳̽ṹ������˵��Ӧ���Ѿ��˳���
		return	true;
	}
	return	false;
}
bool	whthread_man::waitend(whtid_t __tid, unsigned int timeout)
{
	int				rst;
	whthread_info_t	*info;

	// �����Ϣ�ṹ
	lock();
	info	= getinfo(__tid);
	unlock();
	if( !info )
	{
		// �߳̽ṹ������˵��Ӧ���Ѿ��˳���
		return	true;
	}
	::pthread_mutex_lock(&info->mutex);
	if( info->bstopped )
	{
		// Ӧ�øոճɹ��˳���
		::pthread_mutex_unlock(&info->mutex);
		return	true;
	}
	// �ȴ�
	struct timeval  now;
	struct timespec tw;

	gettimeofday(&now, NULL);
	tw.tv_nsec	= now.tv_usec + timeout*1000;		// ������΢��
	tw.tv_sec	= now.tv_sec + tw.tv_nsec/1000000;	// ����1�벿�ּӵ�����
	tw.tv_nsec	= (tw.tv_nsec % 1000000) * 1000;	// ����ʣ�µĲ��ֱ������

	rst	= ::pthread_cond_timedwait(&info->cond, &info->mutex, &tw);
	::pthread_mutex_unlock(&info->mutex);

	bool	bval = true;
	switch(rst)
	{
		case	ETIMEDOUT:
			bval	= false;
		break;
		case	EINTR:
			// �̱߳������ж��ˣ�������Ҫ�������ͷ���Ϣ�ṹ
			lock();
			removeandfree(__tid);
			unlock();
		break;
		default:
			// �߳��Լ�������
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
		// û���������
		return	false;
	}
	whthread_info_free(info);
	return	true;
}

// Ψһ�Ĺ���������
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

// �����ϵ��߳���
static void *	whthread_proc(void *ptr)
{
	whtid_t	tid = (whtid_t)ptr;
	whthread_info_t	*info;
	whthread_func_t func;
	void	* param, * vrst;


	// ����̵߳���Ϣ�ṹ
	g_tman->lock();
	info	= g_tman->getinfo(tid);
	assert(info);
	info->bstopped	= false;
	func			= info->func;
	param			= info->param;
	g_tman->unlock();

	// �߳�����
	vrst	= (*func)(param);

	// ����֪ͨ
	g_tman->lock();
	info	= g_tman->getinfo(tid);
	if( info )	// ���infoΪ��˵�����߳��Ѿ���ǿ���ͷ���
	{
		::pthread_mutex_lock(&info->mutex);
		info->bstopped	= true;
		::pthread_cond_signal(&info->cond);
		::pthread_mutex_unlock(&info->mutex);

		// �õȴ����߳��л���ɹ�����
		usleep(10000);
		// �ͷŸýṹ
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
// detachҪ��create֮�������ѿ���Ȩ��ȫ�������̣߳���������close�ġ�����ҲҪ��linux��tid���������Լ��������
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

#endif							// EOF Linux�µĴ���	}

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
	// �����ʾ��ǿ�н�����
	return		1;
}


}// EOF	namespace n_whcmn
