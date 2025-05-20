// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whlock.h
// Creator: Wei Hua (κ��)
// Comment: �߳���
// CreationDate: 2003-05-19

#ifndef	__WHLOCK_H__
#define	__WHLOCK_H__

#include "../inc/wh_platform.h"

#if defined(__GNUC__)
#include <pthread.h>
#endif

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// windows�Ĵ���
////////////////////////////////////////////////////////////////////
#if defined(WIN32)
typedef	CRITICAL_SECTION	lksect;
inline int	whlk_init(lksect *plk)
{
	InitializeCriticalSection(plk);
	return	0;
}
inline int	whlk_release(lksect *plk)
{
	DeleteCriticalSection(plk);
	return	0;
}
inline void	whlk_lock(lksect *plk)
{
	EnterCriticalSection(plk);
}
inline void	whlk_unlock(lksect *plk)
{
	LeaveCriticalSection(plk);
}
#endif				// EOF WIN32


////////////////////////////////////////////////////////////////////
// linux�Ĵ���
////////////////////////////////////////////////////////////////////
#if defined(__GNUC__)

typedef	::pthread_mutex_t	lksect;

inline int	whlk_init(lksect *plk)
{
	::pthread_mutexattr_t	attr;
	int		rst;
	::pthread_mutexattr_init(&attr);
	::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	rst		= ::pthread_mutex_init(plk, &attr);
	::pthread_mutexattr_destroy(&attr);
	return	rst;
}
inline int	whlk_release(lksect *plk)
{
	::pthread_mutex_destroy(plk);
	return	0;
}
inline void	whlk_lock(lksect *plk)
{
	::pthread_mutex_lock(plk);
}
inline void	whlk_unlock(lksect *plk)
{
	::pthread_mutex_unlock(plk);
}

#endif				// EOF Linux

////////////////////////////////////////////////////////////////////
// ��װ��
////////////////////////////////////////////////////////////////////
class	whlock
{
private:
	lksect	m_lk;
public:
	whlock()
	{
		whlk_init(&m_lk);
	}
	~whlock()
	{
		whlk_release(&m_lk);
	}
	inline void	reinit()
	{
		// ��������һ����
		whlk_release(&m_lk);
		whlk_init(&m_lk);
	}
	inline void	lock()
	{
		whlk_lock(&m_lk);
	}
	inline void	unlock()
	{
		whlk_unlock(&m_lk);
	}
};

////////////////////////////////////////////////////////////////////
// һ���Ժ�����(һ�����ں�������Ļ���)
////////////////////////////////////////////////////////////////////
class	whlockmutex
{
private:
	whlock	*m_pLock;
public:
	whlockmutex(whlock *pLock)			// pLock���ձ�ʾ��ʹ������
	: m_pLock(pLock)
	{
		if( m_pLock )
		{
			m_pLock->lock();
		}
	}
	~whlockmutex()
	{
		if( m_pLock )
		{
			m_pLock->unlock();
		}
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHLOCK_H__
