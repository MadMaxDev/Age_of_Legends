// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: host_app.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include <WHCMN/inc/whtime.h>

#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetudpGLogger.h>

#include "../inc/host_app.h"
#include "../inc/misc.h"

typedef	void	(*fn_WHNET_STATIC_INFO_In)(void *);
typedef	void	(*fn_WHCMN_STATIC_INFO_In)(void *);

bool dia_host_app::dha_thread_stop = false;
dia_thread_func_return dia_gf_call dia_host_app::dha_thread_routine(void *thread_data)
{
	dia_plugin *plugin = (dia_plugin*)thread_data;
	while (!dha_thread_stop)
	{
#ifdef __GNUC__
		if (dt_waitfor_single_event(plugin->get_event(), plugin->get_mutex(), 1) == 0)
#else
		if (dt_waitfor_single_event(plugin->get_event(), 1) == 0)
#endif
			plugin->dp_tick();
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

int dia_host_app::dha_init(DER_CFG *conf)
{
	_ah_freq = conf->ah_freq;
	_rename_freq = conf->rename_freq;
	if (_reactor.init(conf->keepalive_idle, conf->keepalive_interval, conf->handler_max, conf->nio_buf_len) < 0)
		return -1;
	_reactor.set_keepalive(conf->manual_keepalive_idle, conf->manual_keepalive_interval);
	if (_acceptor.open(conf->tcp_listen_host, conf->tcp_listen_port) < 0)
		return -2;
	_acceptor.set_reactor(&_reactor);

	dia_dll_handle handle = dd_open(conf->dll_name);
	_plugin_creator = (dha_plugin_creator)dd_get_func(handle, conf->dll_create_func);
	if (_plugin_creator == NULL)
	{
		dd_close(handle);
		return -3;
	}

	// 主线程和DLL之间的网络基本数据同步
	fn_WHCMN_STATIC_INFO_In		WHCMN_STATIC_INFO_In	= (fn_WHCMN_STATIC_INFO_In)dd_get_func(handle, "WHCMN_STATIC_INFO_In");
	if( WHCMN_STATIC_INFO_In != NULL )
	{
		(*WHCMN_STATIC_INFO_In)(WHCMN_STATIC_INFO_Out());
	}

	fn_WHNET_STATIC_INFO_In		WHNET_STATIC_INFO_In	= (fn_WHNET_STATIC_INFO_In)dd_get_func(handle, "WHNET_STATIC_INFO_In");
	if( WHNET_STATIC_INFO_In != NULL )
	{
		(*WHNET_STATIC_INFO_In)(WHNET_STATIC_INFO_Out());
	}

	_plugin = (dia_plugin*)_plugin_creator(conf->dll_multi_thread);
	_plugin->set_dll_handle(handle);
	if (_plugin->dp_init(this, conf->plugin_cfg) < 0)
		return -4;

	if (_plugin->is_multi_thread())
		_plugin->set_thread_handle(dt_create_thread(dha_thread_routine, _plugin));

	_ah_last_ticks = _last_ticks = n_whcmn::wh_gettickcount();
	return 0;
}
void dia_host_app::dha_fini()
{
	dha_thread_stop = true;
	if (!_plugin)
		return ;

	if (_plugin->is_multi_thread())
	{
		dia_thread_t thread_handle = _plugin->get_thread_handle();
		dt_waitfor_thread_terminate(thread_handle);
	}

	dia_dll_handle dll_handle = _plugin->get_dll_handle();
	_plugin->dp_fini();
	dd_close(dll_handle);
}

void dia_host_app::dha_tick()
{
	// log
	dia_uint32_t cur_ticks = n_whcmn::wh_gettickcount();
	if (get_millisec_diff(cur_ticks, _last_ticks) >= 1000*5)
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_CMN, GLGR_STD_HDR(1085,REACTOR_MSG_QUEUE)"%s,%d,%d,%d,%d", __FUNCTION__, 
			_reactor._msg_recv->_msg_stat_enqs, _reactor._msg_recv->_msg_stat_deqs,
			_reactor._msg_send->_msg_stat_enqs, _reactor._msg_send->_msg_stat_deqs);

		_reactor._msg_recv->_msg_stat_enqs = _reactor._msg_recv->_msg_stat_deqs = 0;
		_reactor._msg_send->_msg_stat_enqs = _reactor._msg_send->_msg_stat_deqs = 0;
		_last_ticks = cur_ticks;
	}
	if (get_millisec_diff(cur_ticks, _ah_last_ticks) >= _ah_freq)
	{
		_plugin->dp_tick_B();
		_ah_last_ticks = cur_ticks;
	}
	if (get_millisec_diff(cur_ticks,_rename_last_ticks) >= _rename_freq )
	{
		_plugin->dp_tick_rename();
		_rename_last_ticks = cur_ticks;
	}
	
	_reactor.tick_timer();
	_reactor.tick_recv(1);

	if (!_reactor.recv_empty())
	{
		if (_plugin->is_multi_thread())
		{
			dt_awake_single_thread(_plugin->get_event());
		}
		else
		{
			_plugin->dp_tick();
		}
	}

	_reactor.tick_send(0);
}


