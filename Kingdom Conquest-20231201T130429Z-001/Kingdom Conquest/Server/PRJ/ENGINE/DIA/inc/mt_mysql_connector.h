// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: mt_mysql_connector.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2008-05-09
// changelog	:
#ifndef __DIA_MT_MYSQL_CONNECTOR_H__
#define __DIA_MT_MYSQL_CONNECTOR_H__

#include "mt_lifo.h"
#include "memory_chunk.h"

#include <mysql/mysql.h>

class dia_mt_mysql_connector
{
public:
	struct dmmc_node
	{
		dia_mt_lifo<dmmc_node*>::lifo_node _ln;
		MYSQL _dbi;
	};
	typedef dia_mt_lifo<dmmc_node*>::lifo_node lifo_node;

public:
	dia_mt_mysql_connector(int count)
		: _alloc_lifo(count) {}
	~dia_mt_mysql_connector() {}

public:
	int dmmc_init(const char *mysql_host,
		const char *mysql_user,
		const char *mysql_passwd,
		const char *mysql_db,
		const char *mysql_charset,
		unsigned short mysql_port,
		const char *mysql_socket);
	void dmmc_fini();

	MYSQL* dmmc_open();
	void dmmc_close(MYSQL *dbi);

private:
	dia_memory_chunk< dia_mt_lifo<char*>, dmmc_node > _alloc_lifo;
	dia_mt_lifo<dmmc_node*> _dbi_free_list;

	char _mysql_host[dia_string_len];
	char _mysql_user[dia_string_len];
	char _mysql_passwd[dia_string_len];
	char _mysql_db[dia_string_len];
	char _mysql_charset[dia_string_len];
	unsigned short _mysql_port;
	char _mysql_socket[dia_string_len];
	int _mysql_flag;
};


#endif

