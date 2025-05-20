// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: event_reactor_timer.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_EVENT_REACTOR_TIMER_H__
#define __DIA_EVENT_REACTOR_TIMER_H__

#include <map>
#include <iterator>
#include "st_lifo.h"
#include "memory_chunk.h"

class dia_event_handler;
struct dert_node
{
	dia_uint64_t _interval;
	bool _removable;
	dia_event_handler *_handler;
	dert_node(): _interval(0), _removable(false), _handler(NULL) {}
};
class dia_event_reactor_timer
{
public:
	dia_event_reactor_timer()
		: _event_max(0)
		, _freq_jiffies(0)
		, _alloc(NULL)
	{}
	~dia_event_reactor_timer()
	{ fini(); }

public:
	int init(dia_uint32_t event_max);
	void fini();
	void tick();

	dert_node* register_handler(dia_event_handler *handler, dia_uint32_t millisec_interval);

private:
	dia_uint32_t _event_max;
	dia_uint64_t _freq_jiffies;
	dia_memory_chunk<dia_st_lifo<char*>, dert_node> *_alloc;

	typedef std::map<dia_uint64_t, dert_node*> dert_event_map;
	dert_event_map _event_map;
};


#endif

