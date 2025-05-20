// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: mt_message_queue.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include "../inc/mt_message_queue.h"

int dia_mt_message_queue::dmmq_enqueue(const DMMQ_HEAD *head, const char *raw, int raw_len)
{
	pointer_type ptr;
	if ((ptr = _alloc_chunk.dmca_new(raw_len+sizeof(DMMQ_HEAD))) == NULL)
		return -1;

	lifo_node *ln = (lifo_node*)((lifo_node*)ptr - 1);
	memcpy((char*)ln->_value, (char*)head, sizeof(DMMQ_HEAD));

	bool first = true;
	int idx = 0;
	while (idx < raw_len)
	{
		int frag;
		if (first)
		{
			frag = (dia_uint32_t)(raw_len-idx)>(_meta_size-sizeof(DMMQ_HEAD))? (_meta_size-sizeof(DMMQ_HEAD)): (raw_len-idx);
			memcpy((char*)ln->_value+sizeof(DMMQ_HEAD), raw+idx, frag);
			first = false;
		}
		else
		{
			frag = (dia_uint32_t)(raw_len-idx)>(_meta_size)? (_meta_size): (raw_len-idx);
			memcpy((char*)ln->_value, raw+idx, frag);
		}
		idx += frag;
		ln = ln->_next;
	}

	fifo_node *fn;
	if ((fn = _alloc_fifo.dmc_new()) == NULL)
	{
		_alloc_chunk.dmca_delete(ptr);
		return -1;
	}

	dmmq_node_data dnd;
	dnd._ptr = ptr;
	dnd._size = raw_len+sizeof(DMMQ_HEAD);
	fn->_value = dnd;
	_msg_list.fifo_push(fn);

	++_msg_stat_enqs;
	return 0;
}

int dia_mt_message_queue::dmmq_dequeue(DMMQ_HEAD *head, char *raw, int real_raw_len)
{
	fifo_node *fn;
	if ((fn = _msg_list.fifo_pop()) == NULL)
		return -1;

	dmmq_node_data dnd;
	dnd = fn->_value;
	int total_len = dnd._size;
	int raw_len = total_len-sizeof(DMMQ_HEAD);
	if (real_raw_len < raw_len)
	{
		return -2;
	}

	pointer_type ptr = dnd._ptr;
	lifo_node *ln = (lifo_node*)((lifo_node*)ptr - 1);
	memcpy((char*)head, (char*)ln->_value, sizeof(DMMQ_HEAD));

	bool first = true;
	int idx = 0;
	while (idx < raw_len)
	{
		int frag;
		if (first)
		{
			frag = (dia_uint32_t)(raw_len-idx)>(_meta_size-sizeof(DMMQ_HEAD))? (_meta_size-sizeof(DMMQ_HEAD)): (raw_len-idx);
			memcpy(raw+idx, (char*)ln->_value+sizeof(DMMQ_HEAD), frag);
			first = false;
		}
		else
		{
			frag = (dia_uint32_t)(raw_len-idx)>(_meta_size)? (_meta_size): (raw_len-idx);
			memcpy(raw+idx, (char*)ln->_value, frag);
		}
		idx += frag;
		ln = ln->_next;
	}

	_alloc_chunk.dmca_delete(ptr);
	_alloc_fifo.dmc_delete(fn);

	++_msg_stat_deqs;
	return raw_len;
}

