// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: mt_fifo.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_MT_QUEUE_H__
#define __DIA_MT_QUEUE_H__
#include <stdio.h>
#include "atomic.h"

template<class _entity, class _pointer=_entity*>
class dia_mt_fifo
{
public:
	typedef _entity entity_type;
	typedef _pointer pointer_type;
	struct dmf_node
	{
		dmf_node *_next;
		entity_type _value;
	};
	typedef dmf_node fifo_node;

	struct dmf_head
	{
		volatile dmf_node *_ptr;
		volatile int _deq_count;
	};
	struct dmf_tail
	{
		volatile dmf_node *_ptr;
		volatile int _enq_count;	
	};

public:
	dia_mt_fifo(dmf_node *fifo_dummy)
		: _fifo_dummy(fifo_dummy)
	{
		_fifo_head._deq_count = 0;
		_fifo_tail._enq_count = 0;
		_endfifo = (dmf_node*)this;
		_fifo_dummy->_next = _endfifo;
		_fifo_head._ptr = _fifo_dummy;
		_fifo_tail._ptr = _fifo_dummy;
	}
	~dia_mt_fifo()
	{}

	dmf_node* ENDFIFO()
	{
		return _endfifo;
	}
	void fifo_push(dmf_node *node)
	{
		dmf_node *tail;
		int count;

		node->_next = ENDFIFO();
		while (1)
		{
			tail = (dmf_node*)_fifo_tail._ptr;
			count = (int)_fifo_tail._enq_count;
			if (dia_cas(&tail->_next, (dia_uint32_t)ENDFIFO(), (dia_uint32_t)node))
				break;
			else
				dia_cas2(&_fifo_tail, (dia_uint32_t)tail, (dia_uint32_t)count, (dia_uint32_t)tail->_next, (dia_uint32_t)(count+1));
		}
		dia_cas2(&_fifo_tail, (dia_uint32_t)tail, (dia_uint32_t)count, (dia_uint32_t)node, (dia_uint32_t)(count+1));
	}
	dmf_node* fifo_pop()
	{
		dmf_node *head, *next;
		int deq_count, enq_count;
		entity_type value;

		while (1)
		{
			deq_count = (int)_fifo_head._deq_count;
			enq_count = (int)_fifo_tail._enq_count;
			head = (dmf_node*)_fifo_head._ptr;
			next = (dmf_node*)head->_next;

			if (deq_count == (int)_fifo_head._deq_count)
			{
				if (head == _fifo_tail._ptr)
				{
					if (next == ENDFIFO())
						return NULL;
					else
						dia_cas2(&_fifo_tail, (dia_uint32_t)head, (dia_uint32_t)enq_count, (dia_uint32_t)next, (dia_uint32_t)(enq_count+1));
				}
				else if (next != ENDFIFO())
				{
					value = next->_value;
					if (dia_cas2(&_fifo_head, (dia_uint32_t)head, (dia_uint32_t)deq_count, (dia_uint32_t)next, (dia_uint32_t)(deq_count+1)))
						break;
				}
			}
		}
		head->_value = value;
		head->_next = NULL;
		return head;
	}
	bool fifo_empty()
	{
		return (_fifo_head._ptr==_fifo_tail._ptr)? true: false;
	}

private:
	volatile dmf_head _fifo_head;
	volatile dmf_tail _fifo_tail;
	dmf_node *_fifo_dummy;
	dmf_node *_endfifo;
};


#endif

