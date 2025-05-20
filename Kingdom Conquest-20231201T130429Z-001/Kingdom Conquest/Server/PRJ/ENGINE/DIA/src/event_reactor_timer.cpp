// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: event_reactor_timer.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:

#include "../inc/event_reactor_timer.h"
#include "../inc/event_handler.h"
#include "../inc/misc.h"

int dia_event_reactor_timer::init(dia_uint32_t event_max)
{
	if ((_event_max = event_max) == 0)
		return -1;
	if ((_freq_jiffies = get_cpu_freq()) == 0)
		return -2;
	if ((_alloc = new dia_memory_chunk<dia_st_lifo<char*>, dert_node>(_event_max)) == NULL)
		return -3;
	return 0;
}
void dia_event_reactor_timer::fini()
{
	_event_map.clear();
	if (_alloc != NULL) { delete _alloc; _alloc = NULL; }
}

void dia_event_reactor_timer::tick()
{
	dert_event_map::iterator start, stop;
	start = stop = _event_map.begin();
	for (; stop != _event_map.end(); ++stop)
	{
		if (stop->second->_removable)
		{
			_alloc->dmc_delete(stop->second);
			continue;
		}

		if (get_jiffies_fromboot() < stop->first)
			break;

		stop->second->_handler->handle_timeout();

		if (stop->second->_removable)
		{
			_alloc->dmc_delete(stop->second);
			continue;
		}

		_event_map.insert(std::make_pair(get_jiffies_fromboot()+stop->second->_interval, stop->second));
	}
	_event_map.erase(start, stop);
}

dert_node* dia_event_reactor_timer::register_handler(dia_event_handler *handler, dia_uint32_t millisec_interval)
{
	dert_node *event;
	if ((event = _alloc->dmc_new()) == NULL)
		return NULL;

	event = new (event) dert_node();
	event->_handler = handler;
	event->_interval = _freq_jiffies*millisec_interval/1000;
	_event_map.insert(std::make_pair(get_jiffies_fromboot()+event->_interval, event));
	return event;
}

