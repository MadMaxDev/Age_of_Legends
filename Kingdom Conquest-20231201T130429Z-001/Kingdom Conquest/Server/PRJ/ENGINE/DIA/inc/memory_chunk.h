// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: memory_chunk.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
//					2008-04-14: 原有内存耗尽后，按照初始块数进行自动扩容；但是不能自动收缩；
#ifndef __DIA_MEMORY_CHUNK_H__
#define __DIA_MEMORY_CHUNK_H__

#include "numeric_type.h"

template<class _lifo, class _entity=char, class _pointer=_entity*>
class dia_memory_chunk
{
public:
	typedef _lifo chunklist;
	typedef _lifo freelist;
	typedef _entity entity_type;
	typedef _pointer pointer_type;
	typedef typename _lifo::lifo_node lifo_node;

public:
	dia_memory_chunk(dia_uint32_t count, dia_uint32_t weight=1)
		: _meta_size_node(sizeof(lifo_node))
		, _meta_size(sizeof(entity_type)*weight)
		, _meta_count(count)
	{
		char *initchunk = (char*)malloc(_meta_size_node + _meta_count*(_meta_size+_meta_size_node));
		for (dia_uint32_t i = 0; i < _meta_count; ++i)
		{
			lifo_node *node = (lifo_node*)(initchunk + _meta_size_node + i*(_meta_size+_meta_size_node));
			node->_value = (char*)(node + 1);
			_freelist.lifo_push(node);
		}
		_chunklist.lifo_push((lifo_node*)initchunk);
	}
	~dia_memory_chunk()
	{
		char *chunk;
		while ((chunk = (char*)_chunklist.lifo_pop()) != NULL) free(chunk);
	}

public:
	typename dia_memory_chunk<_lifo, _entity>::pointer_type dmc_new()
	{
		lifo_node *ptr;
		while ((ptr = _freelist.lifo_pop()) == NULL)
		{
			char *chunk = (char*)malloc(_meta_size_node + _meta_count*(_meta_size+_meta_size_node));
			for (dia_uint32_t i = 0; i < _meta_count; ++i)
			{
				lifo_node *node = (lifo_node*)(chunk + _meta_size_node + i*(_meta_size+_meta_size_node));
				node->_value = (char*)(node + 1);
				_freelist.lifo_push(node);
			}
			_chunklist.lifo_push((lifo_node*)chunk);
		}
		return (pointer_type)(ptr->_value);
	}
	void dmc_delete(pointer_type ptr)
	{
		_freelist.lifo_push((lifo_node*)((lifo_node*)ptr - 1));
	}

private:
	dia_uint32_t _meta_size_node;
	dia_uint32_t _meta_size;
	dia_uint32_t _meta_count;
	chunklist _chunklist;
	freelist _freelist;
};


#endif

