// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: stream_nio.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetudpGLogger.h>

#include "../inc/event_reactor.h"
#include "../inc/stream_nio.h"

dia_stream_nio::dia_stream_nio()
: dia_ipc_socket()
, _handshaked(false)
, _dert_timer(NULL)
, _keepalive_timer(NULL)
, _recv_buf(NULL)
, _send_buf(NULL)
, _recv_wt(NULL)
, _send_rd(NULL)
, _send_wt(NULL)
, _send_end(NULL)
, _buf_len(0)
, _in_cycle(true)
, _state(DSN_STATE_NONE)
{}
dia_stream_nio::~dia_stream_nio()
{
	clear_self();
}

void dia_stream_nio::clear_self()
{
	if (_send_buf != NULL) { delete [] _send_buf; _send_buf = NULL; }
	if (_recv_buf != NULL) { delete [] _recv_buf; _recv_buf = NULL; }
}


int dia_stream_nio::set_reactor(dia_event_reactor *reactor)
{
	assert (reactor!=_reactor);
	if (_reactor != NULL)
	{
		_reactor = NULL;
		clear_self();
	}
	if (reactor != NULL)
	{
		if (reactor->register_handler(this, DIA_EVENT_WRITE) == -1)
			return -1;

		_buf_len = reactor->get_nio_buf_len();
		_reactor = reactor;

		if (_recv_buf == NULL) _recv_buf = new dia_uint8_t[_buf_len*2];
		if (_send_buf == NULL) _send_buf = new dia_uint8_t[_buf_len*2];

		_recv_wt = _recv_buf;
		_send_rd = _send_wt = _send_buf;
		_in_cycle = true;
		_handshaked = false;
		_keepalive_idle = _reactor->_manual_keepalive_idle;
	}
	return 0;
}


int dia_stream_nio::handle_timeout()
{
	switch (state())
	{
	case DSN_STATE_ACTIVE_CONNECTING:
		{
			get_reactor()->remove_handler_by_timer(this);
			dia_event_reactor *old_reactor = get_reactor();
			dia_socket_t old_socket = dia_ipc_socket::get_socket();
			set_reactor(NULL);
			dia_ipc_socket::close_all();

			_recv_wt = _recv_buf;
			_send_rd = _send_wt = _recv_buf;
			_in_cycle = true;
			_handshaked = false;

			if (timer() != NULL) { timer()->_removable = true; timer(NULL); }
			if (keepalive_timer() != NULL) { keepalive_timer()->_removable = true; keepalive_timer(NULL); }
			state(DSN_STATE_ACTIVE_CLOSED);

			DMMQ_HEAD head;
			head._id = old_socket;
			head._opcode = DIA_NIO_HANDLE_TIMEOUT;

			STREAM_NIO_CLOSED msg;
			msg._peer = this;
			old_reactor->recv_enqueue(&head, (const char*)&msg, sizeof(msg));
			return 0;
		}break;
	case DSN_STATE_ACTIVE_CLOSED:
		{
			dia_event_reactor *reactor = _reactor;
			_reactor = NULL;
			if (timer() != NULL) { timer()->_removable = true; timer(NULL); }
			if (keepalive_timer() != NULL) { keepalive_timer()->_removable = true; keepalive_timer(NULL); }
			reconnect_nolinger(reactor);
			return 0;
		}break;
	case DSN_STATE_ACTIVE_CONNECTED:
	case DSN_STATE_PASSIVE_CONNECTED:
		{
			// 可以断开了
			if (_keepalive_idle <= 0)
			{
				get_reactor()->remove_handler_by_timer(this);
				dia_event_reactor *old_reactor = get_reactor();
				dia_socket_t old_socket = dia_ipc_socket::get_socket();
				set_reactor(NULL);
				dia_ipc_socket::close_all();

				_recv_wt = _recv_buf;
				_send_rd = _send_wt = _recv_buf;
				_in_cycle = true;
				_handshaked = false;

				if (timer() != NULL) { timer()->_removable = true; timer(NULL); }
				if (keepalive_timer() != NULL) { keepalive_timer()->_removable = true; keepalive_timer(NULL); }

				if (state() == DSN_STATE_ACTIVE_CONNECTED)
					state(DSN_STATE_ACTIVE_CLOSED);
				else
					state(DSN_STATE_PASSIVE_CLOSED);

				DMMQ_HEAD head;
				head._id = old_socket;
				head._opcode = DIA_NIO_HANDLE_CLOSED;

				STREAM_NIO_CLOSED msg;
				msg._peer = this;
				old_reactor->recv_enqueue(&head, (const char*)&msg, sizeof(msg));
			}
			else
			{
				_keepalive_idle--;
				keepalive();
			}
			return 0;
		}break;
	default:
		assert (this==NULL);
		return 0;
	}
}

int dia_stream_nio::handle_read()
{
	if (!recvable())
		return 0;

	int n = dia_ipc_socket::recv((char*)_recv_wt, _buf_len*2-(_recv_wt-_recv_buf));
	if (n == 0)
	{
		// gracefully closed
		return -1;
	}
	if (n < 0)
	{
		// error
		if (dia_ipc_socket::get_last_error() == dia_socket_wouldblock)
			return 0;
		return -1;
	}

	if (!_handshaked)
	{
		dia_uint16_t msg_type = *(dia_uint16_t*)(_recv_wt);
		dia_uint32_t msg_len = *(dia_uint32_t*)(_recv_wt+2);

		if (state() == DSN_STATE_ACTIVE_CONNECTED &&
			msg_type == DIA_NIO_HANDSHAKE_PASSIVE &&
			msg_len == 0)
		{
			_handshaked = true;
			_recv_wt = _recv_buf;
			handshaking();
			return 0;
		}
		else if (state() == DSN_STATE_PASSIVE_CONNECTED &&
			msg_type == DIA_NIO_HANDSHAKE_ACTIVE &&
			msg_len == 0)
		{
			_handshaked = true;
			_recv_wt = _recv_buf;

			// handshake completed
			DMMQ_HEAD head;
			head._id = dia_ipc_socket::get_socket();
			head._opcode = DIA_NIO_HANDSHAKE_COMPLETED;
			get_reactor()->recv_enqueue(&head, NULL, 0);
			return 0;
		}
		else
		{
			return -1;
		}
	}

	_recv_wt += n;
	dia_uint8_t *idx = _recv_buf;
	while (1)
	{
		dia_uint32_t res_len = (dia_uint32_t)(_recv_wt - idx);
		if (res_len == 0)
		{
			_recv_wt = _recv_buf;
			return (idx-_recv_buf);
		}

		if (res_len < 6)
		{
			if (_recv_buf != idx)
				memmove(_recv_buf, idx, res_len);

			_recv_wt = _recv_buf + res_len;
			return (idx-_recv_buf);
		}

		dia_uint16_t msg_type = *(dia_uint16_t*)(idx);
		dia_uint32_t msg_len = *(dia_uint32_t*)(idx+2);
		if (res_len < (6 + msg_len))
		{
			if (_recv_buf != idx)
				memmove(_recv_buf, idx, res_len);

			_recv_wt = _recv_buf + res_len;
			return (idx-_recv_buf);
		}

		// keepalive
		if (msg_type == DIA_NIO_HANDLE_KEEPALIVE)
		{
			_keepalive_idle = get_reactor()->_manual_keepalive_idle;
		}
		else
		{
			DMMQ_HEAD head;
			head._id = dia_ipc_socket::get_socket();
			head._opcode = msg_type;
			head._len = msg_len;
			get_reactor()->recv_enqueue(&head, (const char*)idx+6, msg_len);
		}
		idx += (6+msg_len);
	}
}

int dia_stream_nio::handle_write()
{
	if (state() == DSN_STATE_ACTIVE_CONNECTING)
	{
		int optval = 0;
		int optlen = sizeof(optval);
		int ret = dia_ipc_socket::get_socket_opt(SOL_SOCKET, SO_ERROR, &optval, &optlen);
		if (ret < 0 || optval != 0)
			return 0;

		struct sockaddr_in self_addr, peer_addr; int len;
		len = sizeof(peer_addr);
		dia_ipc_socket::get_peer_addr ((sockaddr*)&peer_addr, &len);
		len = sizeof(self_addr);
		dia_ipc_socket::get_local_addr((sockaddr*)&self_addr, &len);
		if (!memcmp(&self_addr.sin_port, &peer_addr.sin_port, sizeof(self_addr.sin_port)) &&
			!memcmp(&self_addr.sin_addr, &peer_addr.sin_addr, sizeof(self_addr.sin_addr)))
			return 0;

		if (timer() != NULL) { timer()->_removable = true; timer(NULL); }
		// keepalive
		if (keepalive_timer() != NULL) { keepalive_timer()->_removable = true; keepalive_timer(NULL); }
		keepalive_timer(get_reactor()->schedule_handler(this, get_reactor()->_manual_keepalive_interval));
		state(DSN_STATE_ACTIVE_CONNECTED);
		get_reactor()->update_handler(this, DIA_EVENT_READ);
		return 0;
	}

	int len;
	if (_in_cycle)
		len = _send_wt - _send_rd;
	else
		len = _send_end - _send_rd;

	if (len == 0)
	{
		get_reactor()->update_handler(this, DIA_EVENT_READ);
		return 0;
	}

	int n = dia_ipc_socket::send((const char*)_send_rd, len);
	if (n == 0)
	{
		// gracefully closed
		return -1;
	}
	if (n < 0)
	{
		// error
		if (dia_ipc_socket::get_last_error() == dia_socket_wouldblock)
			return 0;
		return -1;
	}

	if (len == n)
	{
		if (_in_cycle)
		{
			_send_rd = _send_wt = _send_buf;
			get_reactor()->update_handler(this, DIA_EVENT_READ);
		}
		else
		{
			_send_rd = _send_buf;
			_in_cycle = true;

			int len1 = _send_wt - _send_rd;
			if (len1 == 0)
			{
				get_reactor()->update_handler(this, DIA_EVENT_READ);
				return n;
			}

			int n1 = dia_ipc_socket::send((const char*)_send_rd, len1);
			if (n1 == 0)
			{
				// gracefully closed
				return -1;
			}
			if (n1 < 0)
			{
				// error
				if (dia_ipc_socket::get_last_error() == dia_socket_wouldblock)
					return n;
				return -1;
			}

			if (len1 == n1)
			{
				_send_wt = _send_buf;
				get_reactor()->update_handler(this, DIA_EVENT_READ);
			}
			else
			{
				_send_rd += n1;
			}
			return n+n1;
		}
	}
	else
	{
		_send_rd += n;
	}
	return n;
}

void dia_stream_nio::handle_close()
{
	dia_event_reactor *old_reactor = get_reactor();
	dia_socket_t old_socket = dia_ipc_socket::get_socket();
	set_reactor(NULL);
	dia_ipc_socket::close_all();

	_recv_wt = _recv_buf;
	_send_rd = _send_wt = _recv_buf;
	_in_cycle = true;
	_handshaked = false;

	if (timer() != NULL) { timer()->_removable = true; timer(NULL); }
	if (keepalive_timer() != NULL) { keepalive_timer()->_removable = true; keepalive_timer(NULL); }
	if (state() == DSN_STATE_ACTIVE_CONNECTING ||
		state() == DSN_STATE_ACTIVE_CONNECTED)
		state(DSN_STATE_ACTIVE_CLOSED);
	else if (state() == DSN_STATE_PASSIVE_CONNECTING ||
		state() == DSN_STATE_PASSIVE_CONNECTED)
		state(DSN_STATE_PASSIVE_CLOSED);

	DMMQ_HEAD head;
	head._id = old_socket;
	head._opcode = DIA_NIO_HANDLE_CLOSED;

	STREAM_NIO_CLOSED msg;
	msg._peer = this;
	old_reactor->recv_enqueue(&head, (const char*)&msg, sizeof(msg));
}


int dia_stream_nio::send(const DMMQ_HEAD *head, const char *raw, int raw_len)
{
	if (!sendable(raw_len+6))
		return -1;

	*(dia_uint16_t*)(_send_wt) = head->_opcode;
	*(dia_uint32_t*)(_send_wt+2) = raw_len;

	memcpy(_send_wt+6, raw, raw_len);
	if ((!_in_cycle) || ((dia_uint32_t)(_send_wt-_send_buf+raw_len+6) < _buf_len))
	{
		_send_wt += raw_len+6;
	}
	else
	{
		_send_end = _send_wt + raw_len+6;
		_send_wt = _send_buf;
		_in_cycle = false;
	}

	get_reactor()->update_handler(this, DIA_EVENT_READ|DIA_EVENT_WRITE);
	return raw_len+6;
}


int dia_stream_nio::reconnect_linger(dia_event_reactor *reactor)
{
	_reactor = reactor;
	timer(get_reactor()->schedule_handler(this, _connect_interval));
	return 0;
}
int dia_stream_nio::reconnect_nolinger(dia_event_reactor *reactor)
{
	if (open_stream() == -1)
		return -1;

	if (dia_ipc_socket::connect(_peer_host, _peer_port) == -1)
		return -2;

	state(DSN_STATE_ACTIVE_CONNECTING);
	set_reactor(reactor);

	int ret;
	if ((ret = set_keepalive(get_reactor()->get_keepalive_idle(), get_reactor()->get_keepalive_interval())) < 0)
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(109,DIA_ACCEPTOR)"%s,set_keepalive,%d", __FUNCTION__, get_handle(), ret);

	timer(get_reactor()->schedule_handler(this, _connect_interval));
	GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_CMN, GLGR_STD_HDR(101,DIA_STREAM_NIO)"%s,connecting,%s:%d", __FUNCTION__, _peer_host, _peer_port);
	return 0;
}


void dia_stream_nio::disconnect()
{
	get_reactor()->remove_handler_by_timer(this);
	dia_event_reactor *old_reactor = get_reactor();
	dia_socket_t old_socket = dia_ipc_socket::get_socket();
	set_reactor(NULL);
	dia_ipc_socket::close_all();

	_recv_wt = _recv_buf;
	_send_rd = _send_wt = _recv_buf;
	_in_cycle = true;
	_handshaked = false;

	if (timer() != NULL) { timer()->_removable = true; timer(NULL); }
	if (keepalive_timer() != NULL) { keepalive_timer()->_removable = true; keepalive_timer(NULL); }
	if (state() == DSN_STATE_ACTIVE_CONNECTING ||
		state() == DSN_STATE_ACTIVE_CONNECTED)
		state(DSN_STATE_ACTIVE_CLOSED);
	else if (state() == DSN_STATE_PASSIVE_CONNECTING ||
		state() == DSN_STATE_PASSIVE_CONNECTED)
		state(DSN_STATE_PASSIVE_CLOSED);

	DMMQ_HEAD head;
	head._id = old_socket;
	head._opcode = DIA_NIO_HANDLE_CLOSED;

	STREAM_NIO_CLOSED msg;
	msg._peer = this;
	old_reactor->recv_enqueue(&head, (const char*)&msg, sizeof(msg));
}


bool dia_stream_nio::recvable()
{
	return (dia_uint32_t)(_recv_wt-_recv_buf) < _buf_len;
}
bool dia_stream_nio::sendable(dia_uint32_t raw_len)
{
	if (_in_cycle && ((_send_wt-_send_buf+raw_len) > _buf_len*2))
		return false;
	if (!_in_cycle && ((dia_uint32_t)(_send_rd-_send_wt) < raw_len))
		return false;
	return true;
}


void dia_stream_nio::handshaking()
{
	DMMQ_HEAD head;
	head._id = dia_ipc_socket::get_socket();

	if (state() == DSN_STATE_ACTIVE_CONNECTED)
		head._opcode = DIA_NIO_HANDSHAKE_ACTIVE;
	else if (state() == DSN_STATE_PASSIVE_CONNECTED)
		head._opcode = DIA_NIO_HANDSHAKE_PASSIVE;
	else
		return;

	get_reactor()->send_enqueue(&head, NULL, 0);
}
void dia_stream_nio::keepalive()
{
	DMMQ_HEAD head;
	head._id = dia_ipc_socket::get_socket();
	head._opcode = DIA_NIO_HANDLE_KEEPALIVE;

	get_reactor()->send_enqueue(&head, NULL, 0);
}

