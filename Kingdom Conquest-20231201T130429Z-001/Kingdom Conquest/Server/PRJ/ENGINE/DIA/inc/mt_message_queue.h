// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: mt_message_queue.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_MT_MESSAGE_QUEUE_H__
#define __DIA_MT_MESSAGE_QUEUE_H__

#include <string.h>

#include "mt_lifo.h"
#include "mt_fifo.h"
#include "memory_chunk_adaptive.h"
#include "protocol_type.h"

class dia_mt_message_queue
{
public:
	struct dmmq_node_data
	{
		char *_ptr;
		dia_uint32_t _size;
	};
	typedef dia_mt_fifo<dmmq_node_data>::fifo_node fifo_node;
	typedef dia_memory_chunk_adaptive< dia_mt_lifo<char*> >::lifo_node lifo_node;
	typedef dia_memory_chunk_adaptive< dia_mt_lifo<char*> >::pointer_type pointer_type;

public:
	dia_mt_message_queue(int count, int weight=1)
		: _alloc_fifo(count)
		, _alloc_chunk(count, weight)
		, _msg_list((fifo_node*)_alloc_fifo.dmc_new())
		, _meta_size(_alloc_chunk.dmca_meta_size())
		, _msg_stat_enqs(0)
		, _msg_stat_deqs(0) { assert ((unsigned int)weight > sizeof(DMMQ_HEAD)); }
	~dia_mt_message_queue() {}

public:
	int dmmq_enqueue(const DMMQ_HEAD *head, const char *raw, int raw_len);
	int dmmq_dequeue(DMMQ_HEAD *head, char *raw, int real_raw_len);

public:
	bool dmmq_empty() { return _msg_list.fifo_empty(); }

public:
	dia_memory_chunk< dia_mt_lifo<char*>, fifo_node > _alloc_fifo;
	dia_memory_chunk_adaptive< dia_mt_lifo<char*> > _alloc_chunk;
	dia_mt_fifo<dmmq_node_data> _msg_list;
	dia_uint32_t _meta_size;

	dia_uint32_t _msg_stat_enqs;
	dia_uint32_t _msg_stat_deqs;
};



#endif

