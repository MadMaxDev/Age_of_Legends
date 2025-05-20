// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: plugin.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_PLUGIN_H__
#define __DIA_PLUGIN_H__

#include "thread.h"

class dia_host_app;
class dia_export dia_plugin
{
public:
	dia_plugin(bool multi_thread)
		: _multi_thread(multi_thread)
		, _host_app(NULL)
	{
		if (_multi_thread)
		{
#ifdef __GNUC__
			pthread_cond_init(&_event, NULL);
			pthread_mutex_init(&_trivial_mutex, NULL);
#else
			_event = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
		}
	}
	virtual ~dia_plugin()
	{
		if (_multi_thread)
		{
#ifdef __GNUC__
			pthread_cond_destroy(&_event);
			pthread_mutex_destroy(&_trivial_mutex);
#else
			CloseHandle(_event);
#endif
		}
	}

public:
	virtual int dp_init(dia_host_app *host_app, void *cfg) = 0;
	virtual int dp_fini() = 0;
	virtual void dp_tick() = 0;
	virtual void dp_tick_B() = 0;
	virtual void dp_tick_rename() = 0;

public:
	void set_host_app(dia_host_app *host_app) { _host_app = host_app; }
	dia_host_app* get_host_app() { return _host_app; }

	void set_dll_handle(dia_dll_handle handle) { _dll_handle = handle; }
	dia_dll_handle get_dll_handle() { return _dll_handle; }

	void set_thread_handle(dia_thread_t handle) { _thread_handle = handle; }
	dia_thread_t get_thread_handle() { return _thread_handle; }

#ifdef __GNUC__
	dia_mutex_t* get_mutex() { return &_trivial_mutex; }
	dia_event_t* get_event() { return &_event; }
#else
	dia_event_t get_event() { return _event; }
#endif
	bool is_multi_thread() { return _multi_thread; }

private:
	bool _multi_thread;
	dia_host_app *_host_app;
	dia_dll_handle _dll_handle;
	dia_thread_t _thread_handle;
	dia_event_t _event;

#ifdef __GNUC__
	dia_mutex_t _trivial_mutex;
#endif
};


#endif


