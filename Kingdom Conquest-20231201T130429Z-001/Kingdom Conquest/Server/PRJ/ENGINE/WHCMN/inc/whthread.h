// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whthread.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵��̺߳�����
//                �������ô���ͳһwindows��linux�����ܻ��б��ϵͳ���µ��߳�ʹ�÷���
// CreationDate : 2003-05-28

#ifndef	__WHTHREAD_H__
#define	__WHTHREAD_H__

#include "../inc/wh_platform.h"

namespace n_whcmn{

void *	WHCMN_THREAD_STATIC_INFO_Out();
void	WHCMN_THREAD_STATIC_INFO_In(void *pInfo);

// ��׼����Ϊ (��linux����һ��)
// void * whthread_func(void *);
typedef	void * (*whthread_func_t)(void *);
// �߳�id
#ifdef	WIN32
typedef HANDLE				whtid_t;
#else
typedef unsigned int		whtid_t;
#endif
// �Ƿ����߳�id
const whtid_t INVALID_TID	= (whtid_t)0;

// tid�з����ҵ��߳�id (��0��ʼ)
// __priorityΪ-1��ʾĬ�ϵ����ȼ�
enum
{
	THREAD_PRIORIY_IDLE		= 0,
	THREAD_PRIORIY_NORMAL	= 1,
	THREAD_PRIORIY_HIGH		= 2,
	THREAD_PRIORIY_REALTIME	= 3,
};
int		whthread_create(whtid_t *__tid, whthread_func_t __func, void *__param, int __priority = -1);
// ǿ�н���
int		whthread_terminate(whtid_t __tid);
// �ر�tid���(ע�⣬whthread_create�������߳̽�����
// �ϲ������whthread_closetid�رվ��������whthread_terminate����֮)
void	whthread_closetid(whtid_t __tid);
// �ж��߳��Ƿ����
bool	whthread_isend(whtid_t __tid);
// �ȴ��߳̽���������false��ʾ��ʱ
// timeoutΪ�ȴ�ʱ�䣬Ĭ��Ϊ0�������ȵȴ�
bool	whthread_waitend(whtid_t __tid, unsigned int __timeout = 0);
// ����ܵ��߳�����
int		whthread_num();
// �ȴ������������������term֮
// ���أ�	0	��ʾ��������
//			1	��ʾǿ�н���
//			-1	��ʾ����ʧ��
int	whthread_waitend_or_terminate(whtid_t tid, unsigned int __timeout);

}// EOF	namespace wh_genericfunc

#endif	// EOF __WHTHREAD_H__
