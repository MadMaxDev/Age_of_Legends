// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: st_mysql_query.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2008-05-22
// changelog	:
//					2008-05-22: �쳷����
#ifndef __DIA_ST_MYSQL_QUERY_H__
#define __DIA_ST_MYSQL_QUERY_H__

#include <WHCMN/inc/whstring.h>

#include "misc.h"
#include "mt_mysql_connector.h"

//////////////////////////////////////////////////////////////////////////
enum
{
	MYSQL_QUERY_RES_BUSY									= -5,			// �������ǿ�
	MYSQL_QUERY_SRV_GONE_AWAY								= -4,			// ������������
	MYSQL_QUERY_SQL_INVALID									= -3,			// SQL�Ƿ�
	MYSQL_QUERY_NORMAL										= 0,			// ����
};

//////////////////////////////////////////////////////////////////////////

class dia_mysql_query
{
public:
	dia_mysql_query(dia_mt_mysql_connector *dbi_pool, int len);
	dia_mysql_query(MYSQL* dbi, int len);
	~dia_mysql_query();

public:
	bool			Connected();					// ���������Ƿ����

	char*			BinaryToString(int len, const char *ptr);
													// �����ַ���src=>dst
	char*			StrMove(const char *src, ...);
													// �����ַ���src=>dst�����˸�
	char*			StrMove_BackSpace(int count, const char *src, ...);
													// �����ַ��������˸�
	char*			StrEnd(int count);
													// ����SQL���
	char*			SpawnQuery(const char *param, ...);

	void			ExecuteInsertID(int &ret, bool bLog=true);		// ִ��SQL������¼��������Row-ID
	void			ExecuteBinary(int &ret, bool bLog=true);		// ִ��SQL����binary�����
	MYSQL_RES*		ExecuteSPWithResult(int &ret, bool bLog=true);	// ִ��SP�����ؽ��
	void			Execute(int &ret, bool bLog=true);				// ִ��SQL��һ�����
	MYSQL_RES*		GetResult(int &ret, bool bLog=true);			// ִ��SQL�����ؽ����m_res

	MYSQL_ROW		FetchRow();						// ���ж�ȡm_res
	void			FreeResult();					// �ͷŲ�ѯm_res
													// ���SP���صĽ������
	void			ClearSPResult() { FreeResult(); }

public:
	MYSQL*			GetMySQL() { return m_dbi; }	// ���MYSQL���Ӿ��

													// �����ϴβ�������ID
	my_ulonglong	InsertID() { if (m_dbi) return mysql_insert_id(m_dbi); else return 0; }
													// ���ش���
	const char*		GetError() { return m_dbi? mysql_error(m_dbi): ""; }
	int				GetErrno() { return m_dbi? mysql_errno(m_dbi): 0; }
													// ������Ƿ�Ϊ��
	bool			IsNull(int n) { if (m_dbi && m_res && m_row) return m_row[n]? false: true; else return false; }
													// m_res������
	int				NumRows() { return (m_dbi && m_res)? (int)mysql_num_rows(m_res): 0; }
													// m_res��ǰ�е��ֶ���
	int				NumFields() { return (m_dbi && m_res)? mysql_num_fields(m_res): 0; }
													// m_res��ǰ�и��ֶγ���
	int*			FetchLengths() { return (m_dbi && m_res)? (int*)mysql_fetch_lengths(m_res): NULL; }
	void			SkipRow() { m_rowCount++; }		// �����ʱ������һ��

													// ���ص�n+1��
	const char*		GetStr(int n) { if (m_dbi && m_res && m_row) return m_row[n]? m_row[n]: ""; else return NULL; }
													// ����m_rowCount��
	const char*		GetStr() { return GetStr(m_rowCount++); }
													// ����m_rowCount�У�����ƫ��
	const char*		GetStr_Pre() { return GetStr(m_rowCount); }

													// ���ص�n+1��
	int				GetVal_32(int n) { return (m_dbi && m_res && m_row && m_row[n])? n_whcmn::wh_atoi(m_row[n]): 0; }
	unsigned int	GetVal_32l(int n) { return (m_dbi && m_res && m_row && m_row[n])? n_whcmn::wh_atoi(m_row[n]): 0; }
													// ����m_rowCount��
	int				GetVal_32() { return GetVal_32(m_rowCount++); }
	unsigned int	GetVal_32l() { return GetVal_32l(m_rowCount++); }

													// ������ͣ�uint64
	dia_int64_t		GetVal_64(int n) { 	return (m_dbi && m_res && m_row && m_row[n])? dia_ato64(m_row[n]): 0; }
	dia_int64_t		GetVal_64() { return GetVal_64(m_rowCount++); }

													// ������ͣ�float
	double			GetVal_float(int n) { return (m_dbi && m_res && m_row && m_row[n])? atof(m_row[n]): 0; }
	double			GetVal_float() { return GetVal_float(m_rowCount++); }

	dia_int64_t		GetInsertID();					// insertID

													// �����ֶμ���
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

	short			m_rowCount;						// �������
	short			m_fieldCount;					// �ֶ�����

public:
	char			*m_end;							// SQL���ַ���ĩβָ�룬����binary����
	char			*m_query;						// SQL����ǰSQL
	char			m_spQuery[64];					// SP��	��ȡ�����SQL
	dia_int64_t		m_insertID;						// �ϴβ�������ID
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

