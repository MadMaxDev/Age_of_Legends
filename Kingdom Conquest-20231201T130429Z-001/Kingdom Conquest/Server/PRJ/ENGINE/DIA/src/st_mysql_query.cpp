// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: st_mysql_query.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2008-05-09
// changelog	:
#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHCMN/inc/whstring.h>

#include "../inc/st_mysql_query.h"

dia_mysql_query::dia_mysql_query(dia_mt_mysql_connector *dbi_pool, int len)
: m_dbi_pool(dbi_pool)
, m_dbi(dbi_pool->dmmc_open())
, m_res(NULL)
, m_row(NULL)
, m_field(NULL)
, m_end(NULL)
, m_query(NULL)
{
	if (len > 0)
	{
		m_query = new char[len];
		m_len = len;
	}
}
dia_mysql_query::dia_mysql_query(MYSQL* dbi, int len)
: m_dbi_pool(NULL)
, m_dbi(dbi)
, m_res(NULL)
, m_row(NULL)
, m_field(NULL)
, m_end(NULL)
, m_query(NULL)
{
	if (len > 0)
	{
		m_query = new char[len];
		m_len = len;
	}
}
dia_mysql_query::~dia_mysql_query()
{
	if (m_res) { FreeResult(); }
	if (m_dbi) 
	{ 
		if (m_dbi_pool != NULL)
		{
			m_dbi_pool->dmmc_close(m_dbi);
		}
	}
	if (m_query != NULL) { delete[] m_query; m_query = NULL; }
}


char* dia_mysql_query::SpawnQuery(const char *param, ...)
{
	va_list arglist;
	va_start(arglist, param);
	vsprintf(m_query, param, arglist);
	va_end(arglist);

	m_end = m_query+strlen(m_query);
	return m_end;
}
char* dia_mysql_query::StrMove(const char *src, ...)
{
	va_list arglist;
	va_start(arglist, src);
	vsprintf(m_end, src, arglist);
	va_end(arglist);

	m_end = m_query+strlen(m_query);
	return m_end;
}
char* dia_mysql_query::StrMove_BackSpace(int count, const char *src, ...)
{
	va_list arglist;
	va_start(arglist, src);
	vsprintf(m_end-count, src, arglist);
	va_end(arglist);

	m_end = m_query+strlen(m_query);
	return m_end;
}
char* dia_mysql_query::StrEnd(int count)
{
	m_query[strlen(m_query)-count] = 0;
	return m_query;
}
char* dia_mysql_query::BinaryToString(int len, const char *ptr)
{
	if( m_dbi)
	{
	m_end += mysql_real_escape_string(m_dbi, m_end, ptr, len);
	}
	return m_end;
}


void dia_mysql_query::Execute(int &ret, bool bLog)
{
	if (m_dbi && m_res)
	{
		ret = MYSQL_QUERY_RES_BUSY;
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,,mysql_res busy", __FUNCTION__);
	}
	if (m_dbi && !m_res)
	{
		n_whcmn::wh_strstrreplace(m_query, m_len, ";", "\\;");
		n_whcmn::wh_strstrreplace(m_query, m_len, "\\G", "\\\\G");

		char tmp = m_query[100];
		m_query[100] = 0;
		if (bLog)
		{
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(1158,SQL)"%s,%s", __FUNCTION__, m_query);
		}
		m_query[100] = tmp;
		if (mysql_query(m_dbi, m_query))
		{
			if (mysql_errno(m_dbi) == 2006)
				ret = MYSQL_QUERY_SRV_GONE_AWAY;
			else
				ret = MYSQL_QUERY_SQL_INVALID;
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,mysql_query,%s,%s", __FUNCTION__, m_query, mysql_error(m_dbi));
		}
		else
		{
			ret = MYSQL_QUERY_NORMAL;
		}
	}
}
void dia_mysql_query::ExecuteBinary(int &ret, bool bLog)
{
	if (m_dbi && m_res)
	{
		ret = MYSQL_QUERY_RES_BUSY;
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,,mysql_res busy", __FUNCTION__);
	}
	if (m_dbi && !m_res)
	{
		n_whcmn::wh_strstrreplace(m_query, m_len, ";", "\\;");
		n_whcmn::wh_strstrreplace(m_query, m_len, "\\G", "\\\\G");

		char tmp = m_query[100];
		m_query[100] = 0;
		if (bLog)
		{
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(1158,SQL)"%s,%s", __FUNCTION__, m_query);
		}
		m_query[100] = tmp;
		if (mysql_real_query(m_dbi, m_query, strlen(m_query)))
		{
			if (mysql_errno(m_dbi) == 2006)
				ret = MYSQL_QUERY_SRV_GONE_AWAY;
			else
				ret = MYSQL_QUERY_SQL_INVALID;
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,mysql_real_query,,%s", __FUNCTION__, mysql_error(m_dbi));
		}
		else
		{
			ret = MYSQL_QUERY_NORMAL;
		}
	}
}
void dia_mysql_query::ExecuteInsertID(int &ret, bool bLog)
{
	if (m_dbi && m_res)
	{
		ret = MYSQL_QUERY_RES_BUSY;
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,,mysql_res busy", __FUNCTION__);
	}
	if (m_dbi && !m_res)
	{
		n_whcmn::wh_strstrreplace(m_query, m_len, ";", "\\;");
		n_whcmn::wh_strstrreplace(m_query, m_len, "\\G", "\\\\G");

		char tmp = m_query[100];
		m_query[100] = 0;
		if (bLog)
		{
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(1158,SQL)"%s,%s", __FUNCTION__, m_query);
		}
		m_query[100] = tmp;
		if (mysql_query(m_dbi, m_query))
		{
			if (mysql_errno(m_dbi) == 2006)
				ret = MYSQL_QUERY_SRV_GONE_AWAY;
			else
				ret = MYSQL_QUERY_SQL_INVALID;
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,mysql_query,%s,%s", __FUNCTION__, m_query, mysql_error(m_dbi));
		}
		else	
		{
			m_insertID = mysql_insert_id(m_dbi);
			ret = MYSQL_QUERY_NORMAL;
		}
	}
}
MYSQL_RES* dia_mysql_query::ExecuteSPWithResult(int &ret, bool bLog)
{
	if (m_dbi && m_res)
	{
		ret = MYSQL_QUERY_RES_BUSY;
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,,mysql_res busy", __FUNCTION__);
	}
	if (m_dbi && !m_res)
	{
		n_whcmn::wh_strstrreplace(m_query, m_len, ";", "\\;");
		n_whcmn::wh_strstrreplace(m_query, m_len, "\\G", "\\\\G");

		char tmp = m_query[100];
		m_query[100] = 0;
		if (bLog)
		{
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(1158,SQL)"%s,%s", __FUNCTION__, m_query);
		}
		m_query[100] = tmp;
		if (mysql_real_query(m_dbi, m_query, strlen(m_query)))
		{
			if (mysql_errno(m_dbi) == 2006)
				ret = MYSQL_QUERY_SRV_GONE_AWAY;
			else
				ret = MYSQL_QUERY_SQL_INVALID;
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,mysql_query,%s,%s", __FUNCTION__, m_query, mysql_error(m_dbi));
			return NULL;
		}
		else
		{
			ret = MYSQL_QUERY_NORMAL;
			m_res = mysql_store_result(m_dbi);
			if (m_res != NULL)
			{
				m_fieldCount = mysql_num_fields(m_res);
				m_field = mysql_fetch_fields(m_res);
			}
		}
	}
	return m_res;
}
MYSQL_RES* dia_mysql_query::GetResult(int &ret, bool bLog)
{
	if (m_dbi && m_res)
	{
		ret = MYSQL_QUERY_RES_BUSY;
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(106,DIA_MYSQL)"%s,,,mysql_res busy", __FUNCTION__);
	}
	if (m_dbi && !m_res)
	{
		n_whcmn::wh_strstrreplace(m_query, m_len, ";", "\\;");
		n_whcmn::wh_strstrreplace(m_query, m_len, "\\G", "\\\\G");

		char tmp = m_query[100];
		m_query[100] = 0;
//		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(1158,SQL)"%s,%s", __FUNCTION__, m_query);
		m_query[100] = tmp;
		Execute(ret, bLog);
		if (ret == MYSQL_QUERY_NORMAL)
		{
			m_res = mysql_store_result(m_dbi);
			if (m_res != NULL)
			{
				m_fieldCount = mysql_num_fields(m_res);
				m_field = mysql_fetch_fields(m_res);
			}
		}
		else
			return NULL;
	}
	return m_res;
}

void dia_mysql_query::FreeResult()
{
	if (m_dbi && m_res)
	{
		mysql_free_result(m_res);
		while (mysql_next_result(m_dbi) == 0)
		{
			m_res = mysql_store_result(m_dbi);
			mysql_free_result(m_res);
		}

		m_res = NULL;
		m_row = NULL;
	}
}

MYSQL_ROW dia_mysql_query::FetchRow()
{
	m_rowCount = 0;
	if (m_dbi && m_res)
	{	
		m_row = mysql_fetch_row(m_res);
		if (m_row != NULL)
		{
			return m_row;
		}
		else
		{
			FreeResult();
			return NULL;
		}
	}
	else
		return NULL;
}


bool dia_mysql_query::Connected()
{
	if (m_dbi)
	{
		if (mysql_ping(m_dbi))
		{
			GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(107,DIA_MYSQL)"%s,mysql_ping,,failed", __FUNCTION__);
			return false;
		}
	}
	return m_dbi? true: false;
}

