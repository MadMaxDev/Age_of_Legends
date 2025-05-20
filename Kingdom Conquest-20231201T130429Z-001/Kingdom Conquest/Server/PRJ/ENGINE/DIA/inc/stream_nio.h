// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: stream_nio.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_STREAM_NIO_H__
#define __DIA_STREAM_NIO_H__

#include "ipc_socket.h"
#include "event_handler.h"
#include "event_reactor_timer.h"
#include "protocol_type.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////////
enum
{
	DSN_STATE_NONE				= 0,
	DSN_STATE_ACTIVE_CONNECTING,
	DSN_STATE_ACTIVE_CONNECTED,
	DSN_STATE_ACTIVE_CLOSED,

	DSN_STATE_PASSIVE_CONNECTING,
	DSN_STATE_PASSIVE_CONNECTED,
	DSN_STATE_PASSIVE_CLOSED,
};
class dia_stream_nio
	: public dia_event_handler, public dia_ipc_socket
{
public:
	dia_stream_nio();
	~dia_stream_nio();

public:
	virtual int set_reactor(dia_event_reactor *reactor);

	virtual dia_handle_t get_handle() { return dia_ipc_socket::get_socket(); }
	virtual void set_handle(dia_handle_t handle) { dia_ipc_socket::set_socket(handle); }

	virtual int handle_timeout();
	virtual int handle_read();
	virtual int handle_write();
	virtual void handle_close();

	virtual int send(const DMMQ_HEAD *head, const char *raw, int raw_len);

	virtual void clear_self();
	virtual void disconnect();

public:
	void handshaking();
	void keepalive();

	int reconnect_linger(dia_event_reactor *reactor);
	int reconnect_nolinger(dia_event_reactor *reactor);

public:
	void set_connect_info(const char *host, unsigned short port, dia_uint32_t interval) { strcpy(_peer_host, host); _peer_port = port; _connect_interval = interval; }
	void set_keepalive_idle(dia_uint32_t idle) { _keepalive_idle = idle; }

protected:
	bool recvable();
	bool sendable(dia_uint32_t raw_len);

public:
	dia_uint8_t state() { return _state; }
	void state(dia_uint8_t state) { _state = state; }
	dert_node* timer() { return _dert_timer; }
	void timer(dert_node *dert_timer) { _dert_timer = dert_timer; }
	dert_node* keepalive_timer() { return _keepalive_timer; }
	void keepalive_timer(dert_node *keepalive_timer) { _keepalive_timer = keepalive_timer; }

public:
	bool _handshaked;
	dert_node *_dert_timer;
	dert_node *_keepalive_timer;

	dia_uint8_t *_recv_buf;
	dia_uint8_t *_send_buf;

	dia_uint8_t *_recv_wt;
	dia_uint8_t *_send_rd;
	dia_uint8_t *_send_wt;
	dia_uint8_t *_send_end;

	dia_uint32_t _buf_len;
	bool _in_cycle;
	dia_uint8_t _state;

	char _peer_host[dia_string_len];
	unsigned short _peer_port;
	dia_uint32_t _connect_interval;

	dia_uint32_t _keepalive_idle;
};


#endif

