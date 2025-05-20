// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: st_order_list.h
// creator		: yanghongyu
// comment		: 
// creationdate	: 2008-06-12
// changelog	:
#ifndef __DIA_ST_ORDER_LIST_H__
#define __DIA_ST_ORDER_LIST_H__

#include <stdio.h>

template<class _entity, class _pointer=_entity*>
class dia_st_order_list
{
public:
	typedef _entity entity_type;
	typedef _pointer pointer_type;
	struct dsol_node
	{
		unsigned int _key;
		dsol_node *_prev;
		dsol_node *_next;
		entity_type _value;
	};
	typedef dsol_node list_node;

public:
	dia_st_order_list()
	{
		tail()->_prev = tail();
		tail()->_next = tail();
		_head = tail();
	}
	~dia_st_order_list()
	{}

	dsol_node* tail()
	{
		return &_tail;
	}
	int dsol_pushback(dsol_node *node)
	{
		if (_head == tail())
		{
			tail()->_prev = node;
			node->_next = tail();
			node->_prev = NULL;
			_head = node;
			return 0;
		}
		else
		{
			dsol_node *cur, *next;
			cur = tail()->_prev;
			while (cur)
			{
				next = cur->_next;
				if (cur->_key <= node->_key)
				{
					cur->_next = node;
					next->_prev = node;
					node->_prev = cur;
					node->_next = next;
					return 0;
				}
				next = cur;
				cur = cur->_prev;
			}
			next->_prev = node;
			node->_prev = NULL;
			node->_next = next;
			_head = node;
			return 0;
		}
	}
	int dsol_pushfront(dsol_node *node)
	{
		if (_head == tail())
		{
			tail()->_prev = node;
			node->_next = tail();
			node->_prev = NULL;
			_head = node;
			return 0;
		}
		else
		{
			dsol_node *cur, *prev;
			cur = _head;
			while (cur != tail())
			{
				prev = cur->_prev;
				if (cur->_key >= node->_key)
				{
					cur->_prev = node;
					if (prev)
						prev->_next = node;

					node->_prev = prev;
					node->_next = cur;
					_head = node;
					return 0;
				}
				prev = cur;
				cur = cur->_next;
			}
			tail()->_prev = node;
			prev->_next = node;
			node->_prev = prev;
			node->_next = tail();
			return 0;
		}
	}
	dsol_node* dsol_popback()
	{
		if (_head == tail())
			return NULL;

		dsol_node *node;
		node = tail()->_prev;
		if (node == _head)
		{
			tail()->_prev = tail();
			_head = tail();
		}
		else
		{
			node->_prev->_next = tail();
			tail()->_prev = node->_prev;
		}

		node->_prev = node->_next = NULL;
		return node;
	}
	dsol_node* dsol_popfront()
	{
		if (_head == tail())
			return NULL;

		dsol_node *node;
		node = _head;
		if (node->_next == tail())
		{
			tail()->_prev = tail();
			_head = tail();
		}
		else
		{
			node->_next->_prev = NULL;
			_head = node->_next;
		}

		node->_next = NULL;
		return node;
	}
	dsol_node* dsol_remove(unsigned int key)
	{
		dsol_node *cur, *prev;
		cur = _head;
		while (cur != tail())
		{
			prev = cur->_prev;
			if (cur->_key > key)
				return NULL;

			if (cur->_key == key)
			{
				cur->_next->_prev = prev;
				if (prev)
					prev->_next = cur->_next;
				else
					_head = cur->_next;

				cur->_prev = cur->_next = NULL;
				return cur;
			}
			cur = cur->_next;
		}
		return NULL;
	}
	dsol_node* dsol_remove_le(unsigned int key)
	{
		dsol_node *cur, *prev;
		cur = _head;
		while (cur != tail())
		{
			prev = cur->_prev;
			if (cur->_key > key)
				return NULL;

			if (cur->_key < key)
			{
				cur->_next->_prev = prev;
				if (prev)
					prev->_next = cur->_next;
				else
					_head = cur->_next;

				cur->_prev = cur->_next = NULL;
				return cur;
			}
			cur = cur->_next;
		}
		return NULL;
	}
	bool dsol_find(unsigned int key)
	{
		dsol_node *cur;
		cur = _head;
		while (cur != tail())
		{
			if (cur->_key > key)
				return false;

			if (cur->_key == key)
				return true;

			cur = cur->_next;
		}
		return false;
	}
	bool dsol_get_value(unsigned int key, entity_type &value)
	{
		dsol_node *cur;
		cur = _head;
		while (cur != tail())
		{
			if (cur->_key > key)
				return false;

			if (cur->_key == key)
			{
				value = cur->_value;
				return true;
			}

			cur = cur->_next;
		}
		return false;
	}
	int dsol_getmaxvalue(entity_type &value)
	{
		if (_head == tail())
			return -1;

		value = tail()->_prev->_value;
		return 0;
	}
	int dsol_getminvalue(entity_type &value)
	{
		if (_head == tail())
			return -1;

		value = _head->_value;
		return 0;
	}
	unsigned int dsol_getmaxkey()
	{
		if (_head == tail())
			return 0;
		return tail()->_prev->_key;
	}
	unsigned int dsol_getminkey()
	{
		if (_head == tail())
			return 0;
		return _head->_key;
	}

private:
	dsol_node *_head;
	dsol_node _tail;
};



#endif

