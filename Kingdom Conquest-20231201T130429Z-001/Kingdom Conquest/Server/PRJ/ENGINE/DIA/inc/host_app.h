// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: host_app.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_HOST_APP_H__
#define __DIA_HOST_APP_H__

#include "dll.h"
#include "plugin.h"
#include "event_reactor.h"
#include "acceptor.h"

class dia_host_app
{
public:
	typedef dia_thread_func_return (*dha_plugin_creator)(bool);

public:
	dia_host_app()
		: _plugin_creator(NULL)
		, _plugin(NULL)
		, _shutdown(false)
		, _last_ticks(0)
		, _ah_last_ticks(0)
		,_rename_last_ticks(0)
		, _ah_freq(0)
		,_rename_freq(0)
		{}
	~dia_host_app() {}

public:
	int dha_init(DER_CFG *conf);
	void dha_fini();
	void dha_tick();

	int recv_dequeue(DMMQ_HEAD *head, char *raw, int real_raw_len) { return _reactor.recv_dequeue(head, raw, real_raw_len); }
	void send_enqueue(const DMMQ_HEAD *head, const void *raw, int raw_len) { _reactor.send_enqueue(head, raw, raw_len); }
	void send_enqueue_addon(const DMMQ_HEAD *head, const void *raw, int raw_len, const void *addon, int addon_len) { _reactor.send_enqueue_addon(head, raw, raw_len, addon, addon_len); }
	void send_enqueue_addon_xds(const DMMQ_HEAD *head, const void *raw, int raw_len, const void *xds, int xds_len, const void *addon, int addon_len) { _reactor.send_enqueue_addon_xds(head, raw, raw_len, xds, xds_len, addon, addon_len); }

public:
	dia_event_reactor* get_reactor() { return &_reactor; }

	bool shutdown() { return _shutdown; }
	void shutdown(bool shutdown) { _shutdown = shutdown; }

	bool recv_empty() { return _reactor.recv_empty(); }

public:
	static bool dha_thread_stop;
	static dia_thread_func_return dia_gf_call dha_thread_routine(void *thread_data);

private:
	dha_plugin_creator _plugin_creator;
	dia_event_reactor _reactor;
	dia_acceptor _acceptor;

	dia_plugin *_plugin;
	bool _shutdown;

	dia_uint32_t _last_ticks;
	dia_uint32_t _ah_last_ticks;
	dia_uint32_t _rename_last_ticks;
	int _ah_freq;
	int _rename_freq;
};


#endif

