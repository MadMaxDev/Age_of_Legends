// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: thread.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include "../inc/thread.h"

#ifdef __GNUC__

dia_thread_t dt_create_thread(dia_thread_func_t func, void *arg)
{
	dia_thread_t thread_id;
	if (pthread_create(&thread_id, NULL, func, arg) != 0)
		return 0;
	return thread_id;
}

dia_event_t dt_event_init()
{
	dia_event_t event;
	pthread_cond_init(&event, NULL);
	return event;
}
void dt_event_fini(dia_event_t *event)
{
	pthread_cond_destroy(event);
}

void dt_awake_single_thread(dia_event_t *event)
{
	pthread_cond_signal(event);
}
void dt_awake_multi_thread(dia_event_t *event)
{
	pthread_cond_broadcast(event);
}


int dt_waitfor_thread_terminate(dia_thread_t thread)
{
	return pthread_join(thread, NULL);
}

int dt_waitfor_single_event(dia_event_t *event, dia_mutex_t *mutex, dia_uint32_t millisec)
{
//	pthread_mutex_lock(mutex);
	struct timespec timeout;
	struct timeval timenow;
	gettimeofday(&timenow, NULL);
	timeout.tv_sec  = timenow.tv_sec;
	timeout.tv_nsec = timenow.tv_usec*1000 + millisec*1000000LL;
	int ret = pthread_cond_timedwait(event, mutex, &timeout)==0? 0: -1;
//	pthread_mutex_unlock(mutex);
	return ret;
}
int dt_waitfor_single_event_infinite(dia_event_t *event, dia_mutex_t *mutex)
{
//	pthread_mutex_lock(mutex);
	int ret = pthread_cond_wait(event, mutex)==0? 0: -1;
//	pthread_mutex_unlock(mutex);
	return ret;
}

#else

dia_thread_t dt_create_thread(dia_thread_func_t func, void *arg)
{
	return (dia_thread_t)_beginthreadex(NULL, 0, func, arg, 0, NULL);
}

dia_thread_t dt_event_init()
{
	return CreateEvent(NULL, FALSE, FALSE, NULL);
}
void dt_event_fini(dia_event_t event)
{
	CloseHandle(event);
}

void dt_awake_single_thread(dia_event_t event)
{
	SetEvent(event);
}
void dt_awake_multi_thread(dia_event_t event)
{
	PulseEvent(event);
}

int dt_waitfor_thread_terminate(dia_thread_t thread)
{
	if (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0)
		return -1;
	CloseHandle(thread);
	return 0;
}

int dt_waitfor_single_event(dia_event_t event, dia_uint32_t millisec)
{
	return WaitForSingleObject(event, millisec)==WAIT_OBJECT_0? 0: -1;
}
int dt_waitfor_single_event_infinite(dia_event_t event)
{
	return WaitForSingleObject(event, INFINITE)==WAIT_OBJECT_0? 0: -1;
}


#endif

