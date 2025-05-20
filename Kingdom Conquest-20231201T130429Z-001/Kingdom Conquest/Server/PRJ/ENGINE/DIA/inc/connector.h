// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: connector.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_CONNECTOR_H__
#define __DIA_CONNECTOR_H__

#include "event_handler.h"

class dia_connector
	: public dia_event_handler
{
public:
	dia_connector() {}
	virtual ~dia_connector() {}

public:
	virtual int set_reactor(dia_event_reactor *reactor);

	int open(const char *host, unsigned short port, dia_uint32_t timeout);

};


#endif

