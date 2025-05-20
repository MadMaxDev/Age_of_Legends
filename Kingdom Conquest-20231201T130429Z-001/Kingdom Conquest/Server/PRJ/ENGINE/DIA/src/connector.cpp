// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: connector.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetudpGLogger.h>

#include "../inc/connector.h"
#include "../inc/event_reactor.h"

int dia_connector::set_reactor(dia_event_reactor *reactor)
{
	_reactor = reactor;
	return 0;
}

int dia_connector::open(const char *host, unsigned short port, dia_uint32_t timeout)
{
	dia_stream_nio *peer;
	if ((peer = get_reactor()->_alloc_peers->dmc_new()) == NULL)
		return -1;

	peer = new (peer) dia_stream_nio();
	if (peer->open_stream() == -1)
	{
		get_reactor()->_alloc_peers->dmc_delete(peer);
		return -1;
	}
	if (peer->connect(host, port) == -1)
	{
		get_reactor()->_alloc_peers->dmc_delete(peer);
		return -1;
	}

	int ret;
	if ((ret = peer->set_keepalive(get_reactor()->get_keepalive_idle(), get_reactor()->get_keepalive_interval())) < 0)
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(109,DIA_ACCEPTOR)"%s,set_keepalive,%d", __FUNCTION__, peer->get_handle(), ret);

	peer->set_connect_info(host, port, timeout);
	peer->set_keepalive_idle(get_reactor()->_manual_keepalive_idle);
	peer->set_reactor(get_reactor());
	peer->state(DSN_STATE_ACTIVE_CONNECTING);
	peer->timer(get_reactor()->schedule_handler(peer, timeout));

	GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_CMN, GLGR_STD_HDR(101,DIA_CONNECTOR)"%s,connecting,%s:%d", __FUNCTION__, host, port);
	return peer->get_handle();
}



