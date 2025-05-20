// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: thread.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_THREAD_H__
#define __DIA_THREAD_H__

#include "numeric_type.h"
#ifdef __GNUC__
#include <pthread.h>
#else
#include <process.h>
#endif

//////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__
typedef pthread_t dia_thread_t;
typedef pthread_cond_t dia_event_t;
typedef pthread_mutex_t dia_mutex_t;
#define dia_thread_func_return void*
typedef dia_thread_func_return (*dia_thread_func_t)(void*);
#define dia_invalid_thread 0
#else
typedef HANDLE dia_thread_t;
typedef HANDLE dia_event_t;
#define dia_thread_func_return unsigned int
typedef dia_thread_func_return (__stdcall *dia_thread_func_t)(void*);
#define dia_invalid_thread NULL
#endif

//////////////////////////////////////////////////////////////////////////
dia_thread_t dt_create_thread(dia_thread_func_t func, void *arg);

dia_event_t dt_event_init();

#ifdef __GNUC__
void dt_awake_single_thread(dia_event_t *event);
void dt_awake_multi_thread(dia_event_t *event);
void dt_event_fini(dia_event_t *event);
#else
void dt_awake_single_thread(dia_event_t event);
void dt_awake_multi_thread(dia_event_t event);
void dt_event_fini(dia_event_t event);
#endif

int dt_waitfor_thread_terminate(dia_thread_t thread);

#ifdef __GNUC__
int dt_waitfor_single_event(dia_event_t *event, dia_mutex_t *mutex, dia_uint32_t millisec);
int dt_waitfor_single_event_infinite(dia_event_t *event, dia_mutex_t *mutex);
#else
int dt_waitfor_single_event(dia_event_t event, dia_uint32_t millisec);
int dt_waitfor_single_event_infinite(dia_event_t event);
#endif


#endif

