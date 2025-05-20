// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: mt_lifo.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_MT_LIFO_H__
#define __DIA_MT_LIFO_H__

#include "atomic.h"

template<class _entity, class _pointer=_entity*>
class dia_mt_lifo
{
public:
	typedef _entity entity_type;
	typedef _pointer pointer_type;
	struct dml_node
	{
		dml_node *_next;
		entity_type _value;
	};
	typedef dml_node lifo_node;

	struct dml_top
	{
		volatile dml_node *_ptr;
		volatile int _pop_count;
	};

public:
	dia_mt_lifo()
	{
		_lifo_top._ptr = NULL;
		_lifo_top._pop_count = 0;
	}
	~dia_mt_lifo()
	{}

	void lifo_push(dml_node *node)
	{
		do
		{
			node->_next = (dml_node*)_lifo_top._ptr;
		} while(!dia_cas(&_lifo_top, (dia_uint32_t)node->_next, (dia_uint32_t)node));
	}
	dml_node* lifo_pop()
	{
		dml_node *head, *next;
		int count;
		do
		{
			head = (dml_node*)_lifo_top._ptr;
			count = (int)_lifo_top._pop_count;
			if (head == NULL)
				break;
			next = head->_next;
		} while(!dia_cas2(&_lifo_top, (dia_uint32_t)head, (dia_uint32_t)count, (dia_uint32_t)next, (dia_uint32_t)(count+1)));

		if (head != NULL)
			head->_next = NULL;
		return head;
	}

private:
	volatile dml_top _lifo_top;
};


#endif

