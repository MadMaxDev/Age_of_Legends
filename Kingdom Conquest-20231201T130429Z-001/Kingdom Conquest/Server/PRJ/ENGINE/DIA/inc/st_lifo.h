// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: st_lifo.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_ST_LIFO_H__
#define __DIA_ST_LIFO_H__

template<class _entity, class _pointer=_entity*>
class dia_st_lifo
{
public:
	typedef _entity entity_type;
	typedef _pointer pointer_type;
	struct dsl_node
	{
		dsl_node *_next;
		entity_type _value;
	};
	typedef dsl_node lifo_node;

	struct dsl_top
	{
		dsl_node *_ptr;
	};

public:
	dia_st_lifo()
	{
		_lifo_top._ptr = NULL;
	}
	~dia_st_lifo()
	{}

	void lifo_push(dsl_node *node)
	{
		node->_next = _lifo_top._ptr;
		_lifo_top._ptr = node;
	}
	dsl_node* lifo_pop()
	{
		dsl_node *head;
		head = _lifo_top._ptr;
		if (head == NULL)
			return NULL;

		_lifo_top._ptr = head->_next;
		if (head != NULL)
			head->_next = NULL;
		return head;
	}

private:
	dsl_top _lifo_top;
};


#endif

