// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: st_mysql_query.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2008-05-22
// changelog	:
//					2008-05-22: 快撤～～
#ifndef __DIA_ST_MYSQL_QUERY_H__
#define __DIA_ST_MYSQL_QUERY_H__

#include <WHCMN/inc/whstring.h>

#include "misc.h"
#include "mt_mysql_connector.h"

//////////////////////////////////////////////////////////////////////////
enum
{
	MYSQL_QUERY_RES_BUSY									= -5,			// 结果句柄非空
	MYSQL_QUERY_SRV_GONE_AWAY								= -4,			// 服务器不可用
	MYSQL_QUERY_SQL_INVALID									= -3,			// SQL非法
	MYSQL_QUERY_NORMAL										= 0,			// 正常
};

//////////////////////////////////////////////////////////////////////////

class dia_mysql_query
{
public:
	dia_mysql_query(dia_mt_mysql_connector *dbi_pool, int len);
	dia_mysql_query(MYSQL* dbi, int len);
	~dia_mysql_query();

public:
	bool			Connected();					// 测试连接是否存在

	char*			BinaryToString(int len, const char *ptr);
													// 连接字符串src=>dst
	char*			StrMove(const char *src, ...);
													// 连接字符串src=>dst，有退格
	char*			StrMove_BackSpace(int count, const char *src, ...);
													// 结束字符串，有退格
	char*			StrEnd(int count);
													// 生成SQL语句
	char*			SpawnQuery(const char *param, ...);

	void			ExecuteInsertID(int &ret, bool bLog=true);		// 执行SQL，并记录所操作的Row-ID
	void			ExecuteBinary(int &ret, bool bLog=true);		// 执行SQL，含binary的语句
	MYSQL_RES*		ExecuteSPWithResult(int &ret, bool bLog=true);	// 执行SP，返回结果
	void			Execute(int &ret, bool bLog=true);				// 执行SQL，一般语句
	MYSQL_RES*		GetResult(int &ret, bool bLog=true);			// 执行SQL，返回结果到m_res

	MYSQL_ROW		FetchRow();						// 按行读取m_res
	void			FreeResult();					// 释放查询m_res
													// 清除SP返回的结果集合
	void			ClearSPResult() { FreeResult(); }

public:
	MYSQL*			GetMySQL() { return m_dbi; }	// 获得MYSQL连接句柄

													// 返回上次操作的行ID
	my_ulonglong	InsertID() { if (m_dbi) return mysql_insert_id(m_dbi); else return 0; }
													// 返回错误
	const char*		GetError() { return m_dbi? mysql_error(m_dbi): ""; }
	int				GetErrno() { return m_dbi? mysql_errno(m_dbi): 0; }
													// 结果行是否为空
	bool			IsNull(int n) { if (m_dbi && m_res && m_row) return m_row[n]? false: true; else return false; }
													// m_res的行数
	int				NumRows() { return (m_dbi && m_res)? (int)mysql_num_rows(m_res): 0; }
													// m_res当前行的字段数
	int				NumFields() { return (m_dbi && m_res)? mysql_num_fields(m_res): 0; }
													// m_res当前行各字段长度
	int*			FetchLengths() { return (m_dbi && m_res)? (int*)mysql_fetch_lengths(m_res): NULL; }
	void			SkipRow() { m_rowCount++; }		// 读结果时，跳过一行

													// 返回第n+1行
	const char*		GetStr(int n) { if (m_dbi && m_res && m_row) return m_row[n]? m_row[n]: ""; else return NULL; }
													// 返回m_rowCount行
	const char*		GetStr() { return GetStr(m_rowCount++); }
													// 返回m_rowCount行，保持偏移
	const char*		GetStr_Pre() { return GetStr(m_rowCount); }

													// 返回第n+1行
	int				GetVal_32(int n) { return (m_dbi && m_res && m_row && m_row[n])? n_whcmn::wh_atoi(m_row[n]): 0; }
	unsigned int	GetVal_32l(int n) { return (m_dbi && m_res && m_row && m_row[n])? n_whcmn::wh_atoi(m_row[n]): 0; }
													// 返回m_rowCount行
	int				GetVal_32() { return GetVal_32(m_rowCount++); }
	unsigned int	GetVal_32l() { return GetVal_32l(m_rowCount++); }

													// 结果类型：uint64
	dia_int64_t		GetVal_64(int n) { 	return (m_dbi && m_res && m_row && m_row[n])? dia_ato64(m_row[n]): 0; }
	dia_int64_t		GetVal_64() { return GetVal_64(m_rowCount++); }

													// 结果类型：float
	double			GetVal_float(int n) { return (m_dbi && m_res && m_row && m_row[n])? atof(m_row[n]): 0; }
	double			GetVal_float() { return GetVal_float(m_rowCount++); }

	dia_int64_t		GetInsertID();					// insertID

													// 返回字段集合
	MYSQL_FIELD&	GetField(int n) { return m_field[n]; }
	MYSQL_FIELD&	GetField() { return GetField(m_fieldCount++); }

	short			GetFieldCount() { return m_fieldCount; }

	my_ulonglong			GetAffectedRow() { return mysql_affected_rows(m_dbi);}

public:
	dia_mt_mysql_connector *m_dbi_pool;
	MYSQL			*m_dbi;
	MYSQL_RES		*m_res;
	MYSQL_ROW		m_row;
	MYSQL_FIELD		*m_field;

	short			m_rowCount;						// 结果行数
	short			m_fieldCount;					// 字段行数

public:
	char			*m_end;							// SQL：字符串末尾指针，用于binary处理
	char			*m_query;						// SQL：当前SQL
	char			m_spQuery[64];					// SP：	读取结果的SQL
	dia_int64_t		m_insertID;						// 上次操作的行ID
	int				m_len;
};


#define DMQ_IS_INT64(t)			(t == MYSQL_TYPE_LONGLONG)
#define DMQ_IS_INT32(t)			(t == MYSQL_TYPE_LONG)
#define DMQ_IS_FLOAT(t)			(t == MYSQL_TYPE_FLOAT)
#define DMQ_IS_STRING(t)		(t == MYSQL_TYPE_STRING)
#define DMQ_IS_DATETIME(t)		(t == MYSQL_TYPE_DATETIME)
#define DMQ_IS_DATE(t)			(t == MYSQL_TYPE_DATE)
#define DMQ_IS_BLOB(t)			(t == MYSQL_TYPE_BLOB)


//////////////////////////////////////////////////////////////////////////
#endif

