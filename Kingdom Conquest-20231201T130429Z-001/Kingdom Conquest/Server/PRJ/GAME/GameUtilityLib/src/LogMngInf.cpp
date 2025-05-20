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


#include "../inc/LogMngInf.h"

using namespace n_whcmn;
using namespace n_whnet;

#ifdef WIN32
	#include <Windows.h>
	#define LOG_MSG(text) ::MessageBox(NULL,text,"LogMng",MB_OK|MB_SYSTEMMODAL)
#else
	#define LOG_MSG(text) printf(text)
#endif

static const int		g_iMaxLogSize = 4096;

CLogList*		g_pLogList[LOG_MAX];
const char*		g_szLogName[]	= { "cnm", "err", "story","hack", "fatal", "datarecover", "billing", "player", "fight", "item", "npc", "clan", "talk", "gm", "survey" };
CLogMng			g_LogMng;

//////////////////////////////////////////////////////////////////////
CLogBase::CLogBase()
{
	m_bEnable		= true;
	m_bShowCount	= false;
	memset(m_szLogName,0,32);
	m_iCount		= 0;
}

void CLogBase::ShowCount()
{
	if(m_bShowCount && m_iCount > 0)
	{
		char buf[256];
		sprintf(buf,"%s Log Count: %d\r\n",m_szLogName,m_iCount);
		LOG_MSG(buf);
	}
}

void CLogBase::Release()
{
	ShowCount();
}

//////////////////////////////////////////////////////////////////////
CLogNet::CLogNet()
{

}

void CLogNet::Init( LPCSTR szLogName )
{
	if(szLogName == NULL)	return;

	SetName(szLogName);

	m_nID = 0;
	
	// 根据别名确定是哪个ID
	int iSize = sizeof(g_szLogName) / sizeof(g_szLogName[0]);
	for(int i=0;i<iSize;i++)
	{
		if(stricmp(g_szLogName[i],szLogName) == 0)
		{
			m_nID = i;
			break;
		}
	}
}

void CLogNet::Write( LPCSTR szText )
{
	GLOGGER2_WRITESTR(m_nID,szText);		// 必须保证 Server 或者 Client 已经运行了 GLOGGER_INIT 来初始化LOG
	m_iCount++;
}
void CLogNet::WriteLine( LPCSTR szText )
{
	char	szStr[g_iMaxLogSize];
	snprintf( szStr, g_iMaxLogSize, "%s\n", szText );
	szStr[g_iMaxLogSize-1] = '\0';			// Linux下可以不添0，windows似乎是应该添上
	GLOGGER2_WRITESTR(m_nID,szStr);
	m_iCount++;
}

void CLogNet::Logic()
{

}

void CLogNet::Release()
{
	
}

//////////////////////////////////////////////////////////////////////
void CLogNetFile::Init( LPCSTR szFileName )
{
	SetName(szFileName);
}

void CLogNetFile::Write( LPCSTR szText )
{
	m_iCount++;
}
void CLogNetFile::WriteLine( LPCSTR szText )
{
	m_iCount++;
}

void CLogNetFile::Logic()
{

}

void CLogNetFile::Release()
{
	CLogBase::Release();
}

//////////////////////////////////////////////////////////////////////
CLogLocaleFile::CLogLocaleFile()
{

}

CLogLocaleFile::~CLogLocaleFile()
{

}

void CLogLocaleFile::Init( LPCSTR szFileName )
{
	whlogwriter::INFO_T info;
	WH_STRNCPY0(info.szLogFilePrefix,szFileName);
	info.nProp = whlogwriter::INFO_T::PROP_SOHU_DEFAULT | whlogwriter::INFO_T::PROP_LINE_QUICKFLUSH;
	m_GLogger.Init(&info);
	SetName(szFileName);
}

void CLogLocaleFile::Write( LPCSTR szText )
{
	m_GLogger.WriteLine(szText);
	m_iCount++;
}
void CLogLocaleFile::WriteLine( LPCSTR szText )
{
	m_GLogger.WriteLine(szText);
	m_iCount++;
}

void CLogLocaleFile::Logic()
{
	m_GLogger.Tick();
}

void CLogLocaleFile::Release()
{
	m_GLogger.Release();
	CLogBase::Release();
}


//////////////////////////////////////////////////////////////////////
CLogList::CLogList()
{

}

CLogList::~CLogList()
{

}

void CLogList::Init( LPCSTR szLogName )
{
	if(szLogName == NULL)	return;

	SetName(szLogName);
}


void CLogList::AddLog( CLogBase* pLog )
{
	if(pLog == NULL)	return;
	m_LogList.push_back(pLog);
}

void CLogList::DelLog( LPCSTR szLogName )
{
	log_iter	it	= m_LogList.begin();
	log_iter	e	= m_LogList.end();
	CLogBase	*pLog	= NULL;
	for(;it!=e;)
	{
		pLog		= (*it);
		log_iter	tmpit	= it++;
		if(strcmp(pLog->GetName(),szLogName) == 0)
		{
			pLog->Release();
			delete pLog;
			pLog = NULL;

			m_LogList.erase(tmpit);
		}
	}
}

void CLogList::Write( LPCSTR szText )
{
	log_iter it = m_LogList.begin();
	log_iter e = m_LogList.end();
	CLogBase* pLog = NULL;
	for(;it!=e;++it)
	{
		pLog = (*it);
		pLog->Write(szText);
	}
}
void CLogList::WriteLine( LPCSTR szText )
{
	char	szStr[g_iMaxLogSize];
	snprintf( szStr, g_iMaxLogSize, "%s\n", szText );
	szStr[g_iMaxLogSize-1] = '\0';			// Linux下可以不添0，windows似乎是应该添上
	Write(szStr);
}

void CLogList::Logic()
{
	log_iter it = m_LogList.begin();
	log_iter e = m_LogList.end();
	CLogBase* pLog = NULL;
	for(;it!=e;++it)
	{
		pLog = (*it);
		pLog->Logic();
	}
}

void CLogList::Release()
{
	log_iter it = m_LogList.begin();
	log_iter e = m_LogList.end();
	CLogBase* pLog = NULL;
	for(;it!=e;++it)
	{
		pLog = (*it);
		pLog->Release();
		delete pLog;
		pLog = NULL;
	}
	m_LogList.clear();
}

//////////////////////////////////////////////////////////////////////
CLogMng::CLogMng()
{
	for( int i=0;i<LOG_MAX;i++ )
	{
		g_pLogList[i] = NULL;
	}
	
}
CLogMng::~CLogMng()
{

}

void CLogMng::AddLogList( LPCSTR szLogName, CLogList* pLogList )
{
	if(szLogName == NULL && pLogList == NULL)	return;
	m_LogList.insert(make_pair(szLogName,pLogList));
}

void CLogMng::DelLogList( LPCSTR szLogName )
{
	loglist_iter it = m_LogList.find(szLogName);
	CLogList* pLogList = NULL;
	if(it != m_LogList.end())
	{
		pLogList = (*it).second;
		pLogList->Release();
		delete pLogList;
		pLogList = NULL;
	}
}

CLogList* CLogMng::GetLogList( LPCSTR szLogName )
{
	loglist_iter it = m_LogList.find(szLogName);
	if(it != m_LogList.end())
	{
		return (*it).second;
	}
	return NULL;
}

void CLogMng::AddClientLog( LPCSTR szLogName, CLogList* pLogList )
{
	if(szLogName == NULL && pLogList == NULL)	return;
	m_LogClientMap.insert(make_pair(szLogName,pLogList));
}

void CLogMng::DelClientLog( LPCSTR szLogName )
{
	typedef	multimap<string,CLogList*>::iterator	IT_T;
	IT_T	it = m_LogClientMap.lower_bound(szLogName);
	IT_T	up = m_LogClientMap.upper_bound(szLogName);

	CLogList* pLogList = NULL;
	for(;it!=up;)
	{
		IT_T	tmpit	= it++;
		pLogList = (*tmpit).second;
		pLogList->DelLog(szLogName);
		m_LogClientMap.erase(tmpit);
	}
}

void CLogMng::Logic()
{
	loglist_iter it = m_LogList.begin();
	loglist_iter e = m_LogList.end();
	CLogList* pLogList = NULL;
	for(;it!=e;++it)
	{
		pLogList = (*it).second;
		pLogList->Logic();
	}
}

void CLogMng::Release()
{
	loglist_iter it = m_LogList.begin();
	loglist_iter e = m_LogList.end();
	CLogList* pLogList = NULL;
	for(;it!=e;++it)
	{
		pLogList = (*it).second;
		pLogList->Release();
		delete pLogList;
		pLogList = NULL;
	}
	m_LogList.clear();
}

// 目的:	添加一个LogList到全局管理器
// 参数:	LogList指针
// 返回值:	无
void		LogMng_Add( CLogList* pLogList )
{
	// 根据别名确定是哪个ID
	int iSize = sizeof(g_szLogName) / sizeof(g_szLogName[0]);
	for(int i=0;i<iSize;i++)
	{
		if(stricmp(g_szLogName[i],pLogList->GetName()) == 0)
		{
			g_pLogList[i] = pLogList;
			break;
		}
	}
	g_LogMng.AddLogList(pLogList->GetName(),pLogList);
}

// 目的:	在全局管理器添加一个发往客户端的日志
// 参数:	ClientLog别名,添加过Log2Client后的LogList指针(应该在调用层生成Log2Client)
// 返回值:	无
void		LogMng_AddClientLog( LPCSTR szClientName ,CLogList* pLogList )
{
	g_LogMng.AddClientLog(szClientName,pLogList);
}

// 目的:	从全局管理器删除一个发往客户端的日志
// 参数:	ClientLog别名
// 返回值:	无
void		LogMng_DelClientLog( LPCSTR szClientName )
{
	g_LogMng.DelClientLog(szClientName);
}

// 目的:	从全局管理器获得一个LogList
// 参数:	LogList别名
// 返回值:	无
CLogList*	LogMng_Get( LPCSTR szLogName )
{
	return g_LogMng.GetLogList(szLogName);
}

// 目的:	Log全局管理器逻辑
// 参数:	无
// 返回值:	无
void		LogMng_Logic()
{
	g_LogMng.Logic();
}

// 目的:	全局管理器释放
// 参数:	无
// 返回值:	无
void		LogMng_Release()
{
	g_LogMng.Release();
	// 清除所有NetLog指针，之后再调用log就没有任何意义了
	memset(g_pLogList,0,sizeof(g_pLogList));
}

// 目的:	输出网络日志,主要是为了给应用层封装接口
// 参数:	Log的类型ID,Log内容
// 返回值:	无
void		OutputNetLog( int nID,LPCSTR szError )
{
	if(nID >= LOG_CMN && nID < LOG_MAX )
	{
		if(g_pLogList[nID] != NULL)
		{
			g_pLogList[nID]->WriteLine(szError);
		}
	}
}

// 目的:	输出错误日志,主要给GameUntility层输出错误日志
// 参数:	Log的类型ID,Log内容
// 返回值:	无
void		OutputErrorLog( LPCSTR szError )
{
	if(g_pLogList[LOG_FATAL] != NULL)
	{
		g_pLogList[LOG_FATAL]->WriteLine(szError);
	}
}

void		OutputErrorLogF( LPCSTR szErrorFmt, ... )
{
    if(g_pLogList[LOG_FATAL] == NULL) return;

    va_list args;
    va_start(args, szErrorFmt);
	char szInfo[10240];
	vsprintf(szInfo, szErrorFmt, args);
	g_pLogList[LOG_FATAL]->WriteLine(szInfo);
    va_end(args);
}


// 目的:	输出道具日志
// 参数:	信息
// 返回值:	无
void		OutputItemLog( LPCSTR szInfo )
{
	if(g_pLogList[LOG_ITEM] != NULL)
	{
		g_pLogList[LOG_ITEM]->WriteLine(szInfo);
	}
}
//目的:		输出调查日志
//参数:		信息
//返回值:	无
void		OutputSurveyLog(LPCSTR szInfo)
{
	if(g_pLogList[LOG_SURVEY] != NULL)
	{
		g_pLogList[LOG_SURVEY]->WriteLine(szInfo);
	}
}
// 目的:	输出客户端日志
// 参数:	信息
// 返回值:	无
void		OutputClientLog( LPCSTR szInfo )
{
	if(g_pLogList[LOG_PLAYER] != NULL)
	{
		g_pLogList[LOG_PLAYER]->WriteLine(szInfo);
	}
}

// 目的:	输出系统日志
// 参数:	信息
// 返回值:	无
void		OutputSysLog( LPCSTR szInfo )
{
	if(g_pLogList[LOG_STORY] != NULL)
	{
		g_pLogList[LOG_STORY]->WriteLine(szInfo);
	}
}

void		OutputFightLog( LPCSTR szInfo )
{
	if(g_pLogList[LOG_FIGHT] != NULL)
	{
		g_pLogList[LOG_FIGHT]->WriteLine(szInfo);
	}
}

void		OutputNpcLog( LPCSTR szInfo )
{
	if(g_pLogList[LOG_NPC] != NULL)
	{
		g_pLogList[LOG_NPC]->WriteLine(szInfo);
	}
}

void		OutputBillingLog( LPCSTR szInfo )
{
	if(g_pLogList[LOG_BILLING] != NULL )
	{
		g_pLogList[LOG_BILLING]->WriteLine(szInfo);
	}
}
void		OutputScriptLog( LPCSTR szInfo )
{
	if(g_pLogList[LOG_PLAYER] != NULL)
	{
		g_pLogList[LOG_PLAYER]->WriteLine(szInfo);
	}
}
