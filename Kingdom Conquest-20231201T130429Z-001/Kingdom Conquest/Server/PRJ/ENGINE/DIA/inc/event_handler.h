// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: event_handler.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_EVENT_HANDLER_H__
#define __DIA_EVENT_HANDLER_H__

#include "numeric_type.h"

//////////////////////////////////////////////////////////////////////////
typedef dia_int8_t dia_event_mask;
enum
{
	DIA_EVENT_NONE			= 0x0000,
	DIA_EVENT_TIMEOUT		= 0x0001,
	DIA_EVENT_READ			= 0x0002,
	DIA_EVENT_WRITE			= 0x0004,
};
struct DMMQ_HEAD;
class dia_event_reactor;
class dia_event_handler
{
public:
	dia_event_handler()
		: _reactor(NULL) {}
	virtual ~dia_event_handler() {}

public:
	dia_event_reactor* get_reactor() { return _reactor; }
	virtual int set_reactor(dia_event_reactor *reactor) { return -1; }

	virtual dia_handle_t get_handle() {  return -1; }
	virtual void set_handle(dia_handle_t handle) {}

	virtual int handle_timeout() { return -1; }
	virtual int handle_read() { return -1; }
	virtual int handle_write() { return -1; }
	virtual void handle_close() {}

	virtual int send(const DMMQ_HEAD *head, const char *raw, int raw_len) { return -1; }

	virtual void clear_self() {}
	virtual void disconnect() {}

protected:
	dia_event_reactor *_reactor;
};


#endif

