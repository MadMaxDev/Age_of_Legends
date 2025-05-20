// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: st_fifo.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_ST_QUEUE_H__
#define __DIA_ST_QUEUE_H__

template<class _entity, class _pointer=_entity*>
class dia_st_fifo
{
public:
	typedef _entity entity_type;
	typedef _pointer pointer_type;
	struct dsf_node
	{
		dsf_node *_next;
		entity_type _value;
	};
	typedef dsf_node fifo_node;

	struct dsf_head
	{
		dsf_node *_ptr;
	};
	struct dsf_tail
	{
		dsf_node *_ptr;
	};

public:
	dia_st_fifo(dsf_node *fifo_dummy)
		: _fifo_dummy(fifo_dummy)
	{
		_endfifo = (dsf_node*)this;
		_fifo_dummy->_next = _endfifo;
		_fifo_head._ptr = _fifo_dummy;
		_fifo_tail._ptr = _fifo_dummy;
	}
	~dia_st_fifo()
	{}

	dsf_node* ENDFIFO()
	{
		return _endfifo;
	}
	void fifo_push(dsf_node *node)
	{
		dsf_node *tail;

		node->_next = ENDFIFO();
		tail = (dsf_node*)_fifo_tail._ptr;
		tail->_next = node;
		_fifo_tail._ptr = node;
	}
	void fifo_push_reverse(dsf_node *node)
	{
		dsf_node *head, *next;

		head = (dsf_node*)_fifo_head._ptr;
		next = (dsf_node*)head->_next;

		node->_next = next;
		head->_next = node;
	}
	dsf_node* fifo_pop()
	{
		dsf_node *head, *next;
		entity_type value;

		head = (dsf_node*)_fifo_head._ptr;
		next = (dsf_node*)head->_next;

		if (head == _fifo_tail._ptr)
			return NULL;

		if (next != ENDFIFO())
		{
			value = next->_value;
			_fifo_head._ptr = next;
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
	dsf_head _fifo_head;
	dsf_tail _fifo_tail;
	dsf_node *_fifo_dummy;
	dsf_node *_endfifo;
};


#endif

