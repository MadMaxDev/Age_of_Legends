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
	bool	m_bEnable;			// 是否启用，为假的时候不输出log
	bool	m_bShowCount;		// 是否显示记录的条数
	char	m_szLogName[256];	// Log的名称
	int		m_iCount;			// 记录的Log条数
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
	int				m_nID;		// 记录是哪个编号的NetLog
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
	map<string,CLogList*>		m_LogList;			// LogName 和 LogList的对应表(通过Log别名索引某个LogList)
	multimap<string,CLogList*>	m_LogClientMap;		// Log2Client的LogName 和 LogList的对应表(这个主要用来删除玩家监控LOG)
};

// 目的:	添加一个LogList到全局管理器
// 参数:	LogList指针
// 返回值:	无
void		LogMng_Add( CLogList* );

// 目的:	在全局管理器添加一个发往客户端的日志
// 参数:	ClientLog别名,添加过Log2Client后的LogList指针(应该在调用层生成Log2Client)
// 返回值:	无
void		LogMng_AddClientLog( LPCSTR szClientName ,CLogList* pLogList );

// 目的:	从全局管理器删除一个发往客户端的日志
// 参数:	ClientLog别名
// 返回值:	无
void		LogMng_DelClientLog( LPCSTR szClientName );

// 目的:	从全局管理器获得一个LogList
// 参数:	LogList别名
// 返回值:	无
CLogList*	LogMng_Get( LPCSTR szLogName );

// 目的:	Log全局管理器逻辑
// 参数:	无
// 返回值:	无
void		LogMng_Logic();

// 目的:	全局管理器释放
// 参数:	无
// 返回值:	无
void		LogMng_Release();

// 目的:	输出网络日志,主要是为了给应用层封装接口
// 参数:	Log的类型ID,Log内容
// 返回值:	无
void		OutputNetLog( int nID,LPCSTR szError );

// 目的:	输出错误日志,主要给GameUntility层输出错误日志
// 参数:	Log的类型ID,Log内容
// 返回值:	无
void		OutputErrorLog( LPCSTR szError );
void		OutputErrorLogF( LPCSTR szErrorFmt, ... );


// 目的:	输出道具日志
// 参数:	信息
// 返回值:	无
void		OutputItemLog( LPCSTR szInfo );

void		OutputFightLog( LPCSTR szInfo );
void		OutputNpcLog( LPCSTR szInfo );

void		OutputSurveyLog(LPCSTR szInfo);
// 目的:	输出客户端日志
// 参数:	信息
// 返回值:	无
void		OutputClientLog( LPCSTR szInfo );

// 目的:	输出系统日志
// 参数:	信息
// 返回值:	无
void		OutputSysLog( LPCSTR szInfo );

// 目的:	输出billing日志
// 参数:	信息
// 返回值:	无
void		OutputBillingLog( LPCSTR szInfo );

// 目的:	输出脚本日志
// 参数:	信息
// 返回值:	无
void		OutputScriptLog( LPCSTR szInfo );

// 目的:	输出错误日志,表达式，附带文件名，行数，Release版本也起作用
// 参数:	表达式，如果是false，则输出错误
#define		assertlog(exp) if( exp == false) { char szError[10240];sprintf( szError, GLGR_STD_HDR(740,ERR_CODELINE)"%s,%s,%d",#exp, __FILE__,__LINE__ );OutputErrorLog( szError ); }
// 参数:	表达式，如果是false，则输出错误,错误信息,str是字符串
#define		assertlogstr(exp,str) if( exp == false) { char szError[10240];sprintf( szError, GLGR_STD_HDR(740,ERR_CODELINE)"%s,%s,%s,%d",str,#exp, __FILE__,__LINE__ );OutputErrorLog( szError ); }
// 参数:	表达式，如果是false，则输出错误,错误信息,idata是整数
#define		assertlogint(exp,idata) if( exp == false) { char szError[10240];sprintf( szError, GLGR_STD_HDR(740,ERR_CODELINE)"%d,%s,%s,%d",idata,#exp, __FILE__,__LINE__ );OutputErrorLog( szError ); }


#endif
