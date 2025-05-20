// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: mysql_connector.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2008-05-09
// changelog	:
#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetudpGLogger.h>

#include "../inc/mt_mysql_connector.h"
#if MYSQL_VERSION_ID>50100
#include <mysql/my_global.h>
#include <mysql/my_sys.h>
#endif

int dia_mt_mysql_connector::dmmc_init(const char *mysql_host,
									  const char *mysql_user,
									  const char *mysql_passwd,
									  const char *mysql_db,
									  const char *mysql_charset,
									  unsigned short mysql_port,
									  const char *mysql_socket)
{
	strcpy(_mysql_host, mysql_host);
	strcpy(_mysql_user, mysql_user);
	strcpy(_mysql_passwd, mysql_passwd);
	strcpy(_mysql_db, mysql_db);
	strcpy(_mysql_charset, mysql_charset);
	_mysql_port = mysql_port;
	strcpy(_mysql_socket, mysql_socket);
	_mysql_flag = 0;
	_mysql_flag |= CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS;

	my_init();
	return 0;
}
void dia_mt_mysql_connector::dmmc_fini()
{
	lifo_node *ln;
	while ((ln = _dbi_free_list.lifo_pop()) != NULL)
	{
		dmmc_node *dn = ln->_value;
		mysql_thread_end();
		mysql_close(&dn->_dbi);
	}
}


MYSQL* dia_mt_mysql_connector::dmmc_open()
{
	if (mysql_thread_init() != 0)
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,mysql_thread_init", __FUNCTION__);
	}

	lifo_node *ln;
	while ((ln = _dbi_free_list.lifo_pop()) != NULL)
	{
		dmmc_node *dn = ln->_value;
		if (!mysql_ping(&dn->_dbi))
		{
			return &dn->_dbi;
		}

		_alloc_lifo.dmc_delete(dn);
	}

	dmmc_node *dn;
	if ((dn = _alloc_lifo.dmc_new()) == NULL)
	{
		return NULL;
	}

	dn->_ln._value = dn;
	if (!mysql_init(&dn->_dbi))
	{
		_alloc_lifo.dmc_delete(dn);
		return NULL;
	}

	// mysql_real_connect timeout
	int tout = 1;
	mysql_options(&dn->_dbi, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&tout);
	if (!mysql_real_connect(&dn->_dbi, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, _mysql_socket, _mysql_flag))
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,mysql_real_connect,%s,%s,%s,%s,%d,%s,%d,%s",
			__FUNCTION__, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, _mysql_socket, _mysql_flag, mysql_error(&dn->_dbi));
		_alloc_lifo.dmc_delete(dn);
		return NULL;
	}

	GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_CMN, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,mysql_real_connect,%s,%s,%s,%s,%d,%s,%d",
		__FUNCTION__, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, _mysql_socket, _mysql_flag);

	if (mysql_set_character_set(&dn->_dbi, _mysql_charset))
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,mysql_set_character_set,%s,%s",
			__FUNCTION__, _mysql_charset, mysql_error(&dn->_dbi));
		_alloc_lifo.dmc_delete(dn);
		return NULL;
	}
	
	GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_CMN, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,mysql_set_character_set,%s",
		__FUNCTION__, _mysql_charset);

	return &dn->_dbi;
}
void dia_mt_mysql_connector::dmmc_close(MYSQL *dbi)
{
	lifo_node *ln = (lifo_node*)((lifo_node*)dbi - 1);
	_dbi_free_list.lifo_push(ln);
}

