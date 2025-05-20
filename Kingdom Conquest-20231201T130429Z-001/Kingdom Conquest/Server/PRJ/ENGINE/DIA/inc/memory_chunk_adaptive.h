// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: memory_chunk_adaptive.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_MEMORY_CHUNK_ADAPTIVE_H__
#define __DIA_MEMORY_CHUNK_ADAPTIVE_H__

#include "memory_chunk.h"

//////////////////////////////////////////////////////////////////////////
// alignment macros
// REFERENCES: align.h (Linux Kernel)
// align = 2^n
#define DIA_ALIGN(n, align) ((n + (align-1)) & ~(align-1))

template<class _lifo, class _entity=char, class _pointer=_entity*>
class dia_memory_chunk_adaptive
{
public:
	typedef typename dia_memory_chunk<_lifo, _entity>::lifo_node lifo_node;
	typedef typename dia_memory_chunk<_lifo, _entity>::pointer_type pointer_type;

public:
	dia_memory_chunk_adaptive(dia_uint32_t count, dia_uint32_t weight=1)
		: _meta_size(sizeof(_entity)*weight)
		, _dmc(count, weight) {}
	~dia_memory_chunk_adaptive() {}

public:
	typename dia_memory_chunk_adaptive<_lifo, _entity>::pointer_type dmca_new(dia_uint32_t raw_size)
	{
		dia_uint32_t align_size = DIA_ALIGN(raw_size, _meta_size);
		pointer_type head, prev, tmp;

		head = _dmc.dmc_new();
		if (head == NULL)
			return NULL;

		prev = head;
		while ((align_size -= _meta_size) > 0)
		{
			tmp = _dmc.dmc_new();
			if (tmp == NULL)
			{
				dmca_delete(head);
				return NULL;
			}

			lifo_node *t = (lifo_node*)((lifo_node*)prev - 1);
			t->_next = (lifo_node*)((lifo_node*)tmp - 1);
			prev = tmp;
		}
		return head;
	}
	void dmca_delete(pointer_type ptr)
	{
		lifo_node *next, *tmp;
		next = (lifo_node*)((lifo_node*)ptr - 1);
		do
		{
			tmp = next;
			next = tmp->_next;
			_dmc.dmc_delete((pointer_type)tmp->_value);
		}while (next != NULL);
	}
	dia_uint32_t dmca_meta_size()
	{
		return _meta_size;
	}

private:
	dia_uint32_t _meta_size;
	dia_memory_chunk<_lifo, _entity> _dmc;
};


#endif

