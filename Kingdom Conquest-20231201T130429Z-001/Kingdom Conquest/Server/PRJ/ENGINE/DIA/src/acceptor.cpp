// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: acceptor.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetudpGLogger.h>

#include "../inc/acceptor.h"
#include "../inc/event_reactor.h"

int dia_acceptor::set_reactor(dia_event_reactor *reactor)
{
	if (_reactor != NULL)
	{
		_reactor = NULL;
	}
	if (reactor != NULL)
	{
		if (reactor->register_handler(this, DIA_EVENT_READ) == -1)
			return -1;
		_reactor = reactor;
	}
	return 0;
}

int dia_acceptor::handle_read()
{
	dia_stream_nio *peer;
	if ((peer = get_reactor()->_alloc_peers->dmc_new()) == NULL)
		return -1;

	peer = new (peer) dia_stream_nio();
	dia_socket_t socket;
	if (dia_ipc_socket::accept(socket) == -1)
	{
		get_reactor()->_alloc_peers->dmc_delete(peer);
		return -1;
	}

	peer->set_handle(socket);

	int ret;
	if ((ret = peer->set_keepalive(get_reactor()->get_keepalive_idle(), get_reactor()->get_keepalive_interval())) < 0)
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(109,DIA_ACCEPTOR)"%s,set_keepalive,%d", __FUNCTION__, peer->get_handle(), ret);

	if ((ret = peer->set_nonblock(true)) < 0)
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(109,DIA_ACCEPTOR)"%s,set_nonblock,%d", __FUNCTION__, peer->get_handle(), ret);

	peer->set_reactor(get_reactor());
	peer->set_keepalive_idle(get_reactor()->_manual_keepalive_idle);
	peer->state(DSN_STATE_PASSIVE_CONNECTED);
	peer->handshaking();

	struct sockaddr_in addr; int len = sizeof(addr);
	peer->get_peer_addr((sockaddr*)&addr, &len);
	GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_CMN, GLGR_STD_HDR(109,DIA_ACCEPTOR)"%s,connected,%d,%s:%d", __FUNCTION__, peer->get_handle(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	strcpy(peer->_peer_host, inet_ntoa(addr.sin_addr));
	peer->_peer_port = ntohs(addr.sin_port);

	// keepalive
	if (peer->keepalive_timer() != NULL) { peer->keepalive_timer()->_removable = true; peer->keepalive_timer(NULL); }
	peer->keepalive_timer(get_reactor()->schedule_handler(peer, get_reactor()->_manual_keepalive_interval));
	return 0;
}

int dia_acceptor::open(const char *host, unsigned short port)
{
	return dia_ipc_socket::open_stream_accept(host, port);
}



