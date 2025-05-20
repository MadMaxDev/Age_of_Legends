// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: event_reactor.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:

#include "../inc/event_reactor.h"
#include "../inc/event_reactor_timer.h"

int dia_event_reactor::init(dia_uint32_t keepalive_idle, dia_uint32_t keepalive_interval, dia_uint32_t handler_max, dia_uint32_t nio_buf_len)
{
	_keepalive_idle = keepalive_idle;
	_keepalive_interval = keepalive_interval;
	if ((_handler_max = handler_max) == 0)
		return -1;
	if ((_nio_buf_len = nio_buf_len) == 0)
		return -2;
	if (_reactor_timer.init(handler_max) == -1)
		return -3;
	if ((_alloc_events = new dia_memory_chunk<dia_st_lifo<char*>, der_event>(handler_max)) == NULL)
		return -4;
	if ((_alloc_peers = new dia_memory_chunk<dia_mt_lifo<char*>, dia_stream_nio>(handler_max)) == NULL)
		return -5;
	if ((_msg_recv = new dia_mt_message_queue(dia_msg_chunk_count, dia_msg_chunk_size)) == NULL)
		return -6;
	if ((_msg_send = new dia_mt_message_queue(dia_msg_chunk_count, dia_msg_chunk_size)) == NULL)
		return -7;
	if ((_send_buf = new dia_uint8_t[_nio_buf_len]) == NULL)
		return -8;
	return 0;
}
void dia_event_reactor::fini()
{
	der_event_map::iterator iter;
	for (iter = _event_map.begin(); iter != _event_map.end(); ++iter)
	{
		dia_event_handler *handler = (dia_event_handler*)iter->second->_data.ptr;
		handler->clear_self();
	}
	_event_map.clear();

	_reactor_timer.fini();
	if (_alloc_events != NULL) { delete _alloc_events; _alloc_events = NULL; }
	if (_alloc_peers != NULL) { delete _alloc_peers; _alloc_peers = NULL; }
	if (_msg_recv != NULL) { delete _msg_recv; _msg_recv = NULL; }
	if (_msg_send != NULL) { delete _msg_send; _msg_send = NULL; }
	if (_send_buf != NULL) { delete[] _send_buf; _send_buf = NULL; }
}


void dia_event_reactor::tick_timer()
{
	_reactor_timer.tick();
}
int dia_event_reactor::tick_recv(dia_uint32_t millisec)
{
	FD_ZERO(&_read_set);
	der_event_map::iterator iter;
	for (iter = _event_map.begin(); iter != _event_map.end(); ++iter)
	{
		if (iter->second->_mask & DIA_EVENT_READ)
			FD_SET(iter->first, &_read_set);
	}

	int handles = 0;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = millisec*1000;
	handles = ::select(FD_SETSIZE, &_read_set, NULL, NULL, &tv);
	if (handles > 0)
	{
		for (iter = _event_map.begin(); iter != _event_map.end();)
		{
			if (FD_ISSET(iter->first, &_read_set))
			{
				dia_event_handler *handler = (dia_event_handler*)iter->second->_data.ptr;
				if (handler->handle_read() == -1)
				{
					handler->handle_close();
					_alloc_events->dmc_delete(iter->second);
					_event_map.erase(iter++);
					continue;
				}
			}
			++iter;
		}
	}
	return handles;
}
int dia_event_reactor::tick_send(dia_uint32_t millisec)
{
	int raw_len;
	int total_len = 0;
	while ((raw_len = _msg_send->dmmq_dequeue((DMMQ_HEAD*)_send_buf, (char*)_send_buf+6, _nio_buf_len-6)) >= 0)
	{
		der_event_map::iterator iter;
		if ((iter = _event_map.find(((DMMQ_HEAD*)_send_buf)->_id)) != _event_map.end())
		{
			dia_event_handler *handler = (dia_event_handler*)iter->second->_data.ptr;
			if (handler->send((const DMMQ_HEAD*)_send_buf, (const char*)_send_buf+6, raw_len) < 0)
			{
				// unsequence
				_msg_send->dmmq_enqueue((const DMMQ_HEAD*)_send_buf, (const char*)_send_buf+6, raw_len);
				break;
			}

			total_len += raw_len+6;
			if ((dia_uint32_t)total_len >= _nio_buf_len)
				break;
		}
	}

	bool empty = true;
	FD_ZERO(&_write_set);
	der_event_map::iterator iter;
	for (iter = _event_map.begin(); iter != _event_map.end(); ++iter)
	{
		if (iter->second->_mask & DIA_EVENT_WRITE)
		{
			FD_SET(iter->first, &_write_set);
			empty = false;
		}
	}

	int handles = 0;
	if (!empty)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = millisec*1000;
		handles = ::select(FD_SETSIZE, NULL, &_write_set, NULL, &tv);
	}
	if (handles > 0)
	{
		for (iter = _event_map.begin(); iter != _event_map.end();)
		{
			if (FD_ISSET(iter->first, &_write_set))
			{
				dia_event_handler *handler = (dia_event_handler*)iter->second->_data.ptr;
				if (handler->handle_write() == -1)
				{
					handler->handle_close();
					_alloc_events->dmc_delete(iter->second);
					_event_map.erase(iter++);
					continue;
				}
			}
			++iter;
		}
	}
	return handles;
}


int dia_event_reactor::register_handler(dia_event_handler *handler, dia_event_mask mask)
{
	der_event *event;
	if ((event = _alloc_events->dmc_new()) == NULL)
		return -1;

	event = new (event) der_event();
	event->_mask = mask;
	event->_data.ptr = handler;
	_event_map.insert(std::make_pair(handler->get_handle(), event));
	return 0;
}
dert_node* dia_event_reactor::schedule_handler(dia_event_handler *handler, dia_uint32_t millisec_interval)
{
	return _reactor_timer.register_handler(handler, millisec_interval);
}
int dia_event_reactor::update_handler(dia_event_handler *handler, dia_event_mask mask)
{
	der_event_map::iterator iter;
	if ((iter = _event_map.find(handler->get_handle())) != _event_map.end())
	{
		iter->second->_mask = mask;
		return 0;
	}
	return -1;
}
int dia_event_reactor::remove_handler_by_timer(dia_event_handler *handler)
{
	der_event_map::iterator iter;
	if ((iter = _event_map.find(handler->get_handle())) == _event_map.end())
		return -1;

	_alloc_events->dmc_delete(iter->second);
	_event_map.erase(iter);
	return 0;
}
int dia_event_reactor::remove_handler_by_socket(dia_socket_t socket)
{
	der_event_map::iterator iter;
	if ((iter = _event_map.find(socket)) == _event_map.end())
		return -1;

	dia_event_handler *handler = (dia_event_handler*)iter->second->_data.ptr;
	handler->disconnect();
	return 0;
}

