// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: event_reactor.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_EVENT_REACTOR_H__
#define __DIA_EVENT_REACTOR_H__

#include "event_handler.h"
#include "event_reactor_timer.h"
#include "stream_nio.h"
#include "mt_message_queue.h"
#include "protocol_type.h"

class dia_event_reactor
{
public:
	struct der_event
	{
		dia_event_mask _mask;
		union der_event_data
		{
			dia_socket_t handle;
			void *ptr;
		};
		der_event_data _data;
		der_event(): _mask(DIA_EVENT_NONE) {}
	};

public:
	dia_event_reactor()
		: _alloc_peers(NULL)
		, _handler_max(0)
		, _nio_buf_len(0)
		, _alloc_events(NULL)
		, _msg_recv(NULL)
		, _msg_send(NULL)
		, _send_buf(NULL)
	{}
	~dia_event_reactor()
	{ fini(); }

public:
	int init(dia_uint32_t keepalive_idle, dia_uint32_t keepalive_interval, dia_uint32_t handler_max, dia_uint32_t nio_buf_len);
	void fini();
	void tick_timer();
	int tick_recv(dia_uint32_t millisec);
	int tick_send(dia_uint32_t millisec);

	int register_handler(dia_event_handler *handler, dia_event_mask mask);
	dert_node* schedule_handler(dia_event_handler *handler, dia_uint32_t millisec_interval);
	int update_handler(dia_event_handler *handler, dia_event_mask mask);
	int remove_handler_by_timer(dia_event_handler *handler);
	int remove_handler_by_socket(dia_socket_t socket);

public:
	dia_uint32_t get_keepalive_idle() { return _keepalive_idle; }
	dia_uint32_t get_keepalive_interval() { return _keepalive_interval; }
	dia_uint32_t get_handler_max() { return _handler_max; }
	dia_uint32_t get_nio_buf_len() { return _nio_buf_len; }
	dia_memory_chunk<dia_mt_lifo<char*>, dia_stream_nio> *_alloc_peers;

	int recv_enqueue(const DMMQ_HEAD *head, const char *raw, int raw_len) { return _msg_recv->dmmq_enqueue(head, raw, raw_len); }
	int recv_dequeue(DMMQ_HEAD *head, char *raw, int real_raw_len) { return _msg_recv->dmmq_dequeue(head, raw, real_raw_len); }

	void set_keepalive(dia_uint32_t idle, dia_uint32_t interval) { _manual_keepalive_idle = idle; _manual_keepalive_interval = interval; }

public:
	bool recv_empty()
	{
		return _msg_recv->dmmq_empty();
	}
	void send_enqueue(const DMMQ_HEAD *head, const void *raw, int raw_len)
	{
		_msg_send->dmmq_enqueue(head, (const char*)raw, raw_len);
	}
	void send_enqueue_addon(const DMMQ_HEAD *head, const void *raw, int raw_len, const void *addon, int addon_len)
	{
		char *send_msg = new char[raw_len+addon_len];
		memcpy(send_msg, (const char*)raw, raw_len);
		memcpy(send_msg+raw_len, (const char*)addon, addon_len);
		send_enqueue(head, send_msg, raw_len+addon_len);
		delete[] send_msg;
	}
	void send_enqueue_addon_xds(const DMMQ_HEAD *head, const void *raw, int raw_len, const void *xds, int xds_len, const void *addon, int addon_len)
	{
		char *send_msg = new char[raw_len+xds_len+addon_len];
		memcpy(send_msg, (const char*)raw, raw_len);
		memcpy(send_msg+raw_len, (const char*)xds, xds_len);
		memcpy(send_msg+raw_len+xds_len, (const char*)addon, addon_len);
		send_enqueue(head, send_msg, raw_len+xds_len+addon_len);
		delete[] send_msg;
	}

public:
	dia_uint32_t _keepalive_idle;
	dia_uint32_t _keepalive_interval;
	dia_uint32_t _handler_max;
	dia_uint32_t _nio_buf_len;

	dia_memory_chunk<dia_st_lifo<char*>, der_event> *_alloc_events;
	dia_event_reactor_timer _reactor_timer;

	fd_set _read_set;
	fd_set _write_set;

	typedef std::map<dia_socket_t, der_event*> der_event_map;
	der_event_map _event_map;

	dia_mt_message_queue *_msg_recv;
	dia_mt_message_queue *_msg_send;

	dia_uint8_t *_send_buf;

	dia_uint32_t _manual_keepalive_idle;
	dia_uint32_t _manual_keepalive_interval;
};


#endif

