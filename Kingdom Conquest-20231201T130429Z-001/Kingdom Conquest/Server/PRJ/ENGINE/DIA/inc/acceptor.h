// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: acceptor.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_ACCEPTOR_H__
#define __DIA_ACCEPTOR_H__

#include "ipc_socket.h"
#include "event_handler.h"

class dia_acceptor
	: public dia_event_handler, public dia_ipc_socket
{
public:
	dia_acceptor() {}
	virtual ~dia_acceptor() {}

public:
	virtual int set_reactor(dia_event_reactor *reactor);

	virtual dia_handle_t get_handle() { return dia_ipc_socket::get_socket(); }

	virtual int handle_read();

	int open(const char *host, unsigned short port);

};


#endif

