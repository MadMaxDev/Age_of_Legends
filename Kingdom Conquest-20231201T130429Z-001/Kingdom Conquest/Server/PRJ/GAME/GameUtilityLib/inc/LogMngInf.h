/********************************************************
Copyright 1996-2006, Pixel Software Corporation, 
All rights reserved.

Module name:	Global Log Manger

Purpose:		Output Log for Net, File, Client
				and so on

Author:			Liuyi

Compiler:		Microsoft Visual C++ 7.1

History:		2006-3-15

Remarks:		

*********************************************************/

#ifndef __LogMngInf_H
#define __LogMngInf_H

#include "DMtype.h"
#include <list>
#include <string>
#include <map>
#include <WHCMN/inc/whlog.h>
#include <WHCMN/inc/whstring.h>
#include <WHNET/inc/whnetudpGLogger.h>

using namespace n_whcmn;
using namespace std;


enum
{
	LOG_CMN			= 0,
	LOG_ERR,
	LOG_STORY,
	LOG_HACK,
	LOG_FATAL,
	LOG_DATARECOVER,
	LOG_BILLING,
	LOG_PLAYER,
	LOG_FIGHT,
	LOG_ITEM,
	LOG_NPC,
	LOG_CLAN,
	LOG_TALK,
	LOG_GM,
	LOG_SURVEY,
	LOG_MAX,
};

extern const char*		g_szLogName[];


class CLogBase
{
public:
	virtual ~CLogBase() {}
	virtual void	Init( LPCSTR szLogName )		= 0;
	virtual void	Write( LPCSTR szText )			= 0;
	virtual void	WriteLine( LPCSTR szText )		= 0;
	virtual void	Logic()							= 0;

	virtual void	Release();

	CLogBase();
	void	ShowCount();

	void	SetShowCount( bool bEnable )
	{
		m_bShowCount = bEnable;
	}
	void	SetEnable( bool bEnable )
	{
		m_bEnable = bEnable;
	}
	bool	IsEnabled()
	{
		return m_bEnable;
	}
	void	SetName( LPCSTR szLogName )
	{
		WH_STRNCPY0(m_szLogName,szLogName);
	}
	LPCSTR	GetName()
	{
		return m_szLogName;
	}

protected:
	bool	m_bEnable;			// �Ƿ����ã�Ϊ�ٵ�ʱ�����log
	bool	m_bShowCount;		// �Ƿ���ʾ��¼������
	char	m_szLogName[256];	// Log������
	int		m_iCount;			// ��¼��Log����
};

class CLogNet : public CLogBase
{
public:
	CLogNet();

	void			Init( LPCSTR szLogName );
	
	virtual void	Write( LPCSTR szText );
	virtual void	WriteLine( LPCSTR szText );
	virtual void	Logic();
	virtual void	Release();

private:
	int				m_nID;		// ��¼���ĸ���ŵ�NetLog
};

class CLogNetFile : public CLogBase
{
public:
	CLogNetFile();

	void			Init( LPCSTR szFileName );

	virtual void	Write( LPCSTR szText );
	virtual void	WriteLine( LPCSTR szText );
	virtual void	Logic();	
	virtual void	Release();
};

class CLogLocaleFile : public CLogBase
{
public:
	CLogLocaleFile();
	virtual	~CLogLocaleFile();

	void			Init( LPCSTR szFileName );

	virtual void	Write( LPCSTR szText );
	virtual void	WriteLine( LPCSTR szText );
	virtual void	Logic();
	virtual void	Release();
protected:
	whlogwriter		m_GLogger;
};

typedef list<CLogBase*>::iterator log_iter;

class CLogList : public CLogBase
{
public:
	CLogList();
	virtual ~CLogList();

	void			Init( LPCSTR szLogName );

	void			AddLog( CLogBase* pLog );
	void			DelLog( LPCSTR szLogName );

	virtual void	Write( LPCSTR szText );
	virtual void	WriteLine( LPCSTR szText );
	virtual void	Logic();
	virtual void	Release();

private:
	list<CLogBase*>	m_LogList;
};

typedef map<string,CLogList*>::iterator loglist_iter;

class CLogMng
{
public:
	CLogMng();
	~CLogMng();

	void			AddLogList( LPCSTR szLogName, CLogList* pLogList );
	void			DelLogList( LPCSTR szLogName );

	CLogList*		GetLogList( LPCSTR szLogName );

	void			AddClientLog( LPCSTR szLogName, CLogList* pLogList );
	void			DelClientLog( LPCSTR szLogName );

	void			Logic();
	void			Release();

private:
	map<string,CLogList*>		m_LogList;			// LogName �� LogList�Ķ�Ӧ��(ͨ��Log��������ĳ��LogList)
	multimap<string,CLogList*>	m_LogClientMap;		// Log2Client��LogName �� LogList�Ķ�Ӧ��(�����Ҫ����ɾ����Ҽ��LOG)
};

// Ŀ��:	���һ��LogList��ȫ�ֹ�����
// ����:	LogListָ��
// ����ֵ:	��
void		LogMng_Add( CLogList* );

// Ŀ��:	��ȫ�ֹ��������һ�������ͻ��˵���־
// ����:	ClientLog����,��ӹ�Log2Client���LogListָ��(Ӧ���ڵ��ò�����Log2Client)
// ����ֵ:	��
void		LogMng_AddClientLog( LPCSTR szClientName ,CLogList* pLogList );

// Ŀ��:	��ȫ�ֹ�����ɾ��һ�������ͻ��˵���־
// ����:	ClientLog����
// ����ֵ:	��
void		LogMng_DelClientLog( LPCSTR szClientName );

// Ŀ��:	��ȫ�ֹ��������һ��LogList
// ����:	LogList����
// ����ֵ:	��
CLogList*	LogMng_Get( LPCSTR szLogName );

// Ŀ��:	Logȫ�ֹ������߼�
// ����:	��
// ����ֵ:	��
void		LogMng_Logic();

// Ŀ��:	ȫ�ֹ������ͷ�
// ����:	��
// ����ֵ:	��
void		LogMng_Release();

// Ŀ��:	���������־,��Ҫ��Ϊ�˸�Ӧ�ò��װ�ӿ�
// ����:	Log������ID,Log����
// ����ֵ:	��
void		OutputNetLog( int nID,LPCSTR szError );

// Ŀ��:	���������־,��Ҫ��GameUntility�����������־
// ����:	Log������ID,Log����
// ����ֵ:	��
void		OutputErrorLog( LPCSTR szError );
void		OutputErrorLogF( LPCSTR szErrorFmt, ... );


// Ŀ��:	���������־
// ����:	��Ϣ
// ����ֵ:	��
void		OutputItemLog( LPCSTR szInfo );

void		OutputFightLog( LPCSTR szInfo );
void		OutputNpcLog( LPCSTR szInfo );

void		OutputSurveyLog(LPCSTR szInfo);
// Ŀ��:	����ͻ�����־
// ����:	��Ϣ
// ����ֵ:	��
void		OutputClientLog( LPCSTR szInfo );

// Ŀ��:	���ϵͳ��־
// ����:	��Ϣ
// ����ֵ:	��
void		OutputSysLog( LPCSTR szInfo );

// Ŀ��:	���billing��־
// ����:	��Ϣ
// ����ֵ:	��
void		OutputBillingLog( LPCSTR szInfo );

// Ŀ��:	����ű���־
// ����:	��Ϣ
// ����ֵ:	��
void		OutputScriptLog( LPCSTR szInfo );

// Ŀ��:	���������־,���ʽ�������ļ�����������Release�汾Ҳ������
// ����:	���ʽ�������false�����������
#define		assertlog(exp) if( exp == false) { char szError[10240];sprintf( szError, GLGR_STD_HDR(740,ERR_CODELINE)"%s,%s,%d",#exp, __FILE__,__LINE__ );OutputErrorLog( szError ); }
// ����:	���ʽ�������false�����������,������Ϣ,str���ַ���
#define		assertlogstr(exp,str) if( exp == false) { char szError[10240];sprintf( szError, GLGR_STD_HDR(740,ERR_CODELINE)"%s,%s,%s,%d",str,#exp, __FILE__,__LINE__ );OutputErrorLog( szError ); }
// ����:	���ʽ�������false�����������,������Ϣ,idata������
#define		assertlogint(exp,idata) if( exp == false) { char szError[10240];sprintf( szError, GLGR_STD_HDR(740,ERR_CODELINE)"%d,%s,%s,%d",idata,#exp, __FILE__,__LINE__ );OutputErrorLog( szError ); }


#endif
