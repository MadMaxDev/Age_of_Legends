// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpGLogger.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 利用UDP的通用日志记录器
// CreationDate : 2003-05-21
// ChangeLOG    : 2007-05-31 修改了所有提示性的日志

#include "../inc/whnetudpGLogger.h"
#include "../inc/whnetudp.h"
#include <WHCMN/inc/whstring.h>

using namespace n_whnet;
using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// GLogServerer
////////////////////////////////////////////////////////////////////
GLogServerer::GLogServerer()
: m_sock(INVALID_SOCKET)
, m_nKATimeOut(60000)
{
}
GLogServerer::~GLogServerer()
{
	Release();
}
int		GLogServerer::Init(INFO_T *pInfo, whlogwriter::INFO_T	*pLogWRInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	m_nKATimeOut	= m_info.nKeepAliveTimeOut * 1000;

	int	rst = 0;

	rst		= m_logwriter.Init(pLogWRInfo);
	if( rst<0 )
	{
		return	-1;
	}

	rst		= m_Loggers.Init(m_info.nMaxLogger);
	if( rst<0 )
	{
		return	-2;
	}

	// 初始化socket
	m_sock	= udp_create_socket_by_ipnportstr(m_info.szBindAddr);
	if( m_sock == INVALID_SOCKET )
	{
		return	-3;
	}
	// 设置缓冲区大一些(linux下最多是128k好像，当然应该可以通过编译内核增加)
	// 这个可以比较好的应付峰值的LOG，比如每秒1000多条的log。
	cmn_setsockopt_int(m_sock, SOL_SOCKET, SO_RCVBUF, 1000000);

	return		0;
}
int		GLogServerer::Release()
{
	m_logwriter.Release();

	m_Loggers.Release();
	if( m_sock != INVALID_SOCKET )
	{
		closesocket(m_sock);
		m_sock	= INVALID_SOCKET;
	}

	return	0;
}
int		GLogServerer::Tick()
{
	// 网络有错
	if( m_sock == INVALID_SOCKET )
	{
		return	0;
	}

	// 判断是否有断线的程序
	Tick_KeepAlive();

	// 对文件的tick
	m_logwriter.Tick();

	// 检测是否有输入
	int	rst;
	rst	= cmn_select_rd(m_sock, m_info.nSelectInterval);
	if( rst<=0 )
	{
		// 没有数据或网络出错
		return	0;
	}

	char				szBuf[whlogwriter::MAXMSGLEN];
	GLOG_CMD_T			*pCmd	= (GLOG_CMD_T*)szBuf;
	socklen_t			len = sizeof(m_fromaddr);
	while( cmn_select_rd(m_sock, 0)>0 )
	{
		int	nSize = ::recvfrom(m_sock, szBuf, sizeof(szBuf)-1, 0, (struct sockaddr*)&m_fromaddr, &len);
		if( nSize<=0 )
		{
			// 网络出问题了，或者被关闭了。
			continue;
		}
		// 判断指令
		switch(pCmd->cmd)
		{
			case	GLOG_CMD_REQ_KEEPALIVE:
				Tick_REQ_KEEPALIVE( (GLOG_CMD_INT_T*)pCmd );
			break;
			case	GLOG_CMD_REQ_CONNECT:
				Tick_REQ_CONNECT( (GLOG_CMD_REQ_CONNECT_T*)pCmd );
			break;
			case	GLOG_CMD_REQ_DISCONNECT:
				Tick_REQ_DISCONNECT( (GLOG_CMD_INT_T*)pCmd );
			break;
			case	GLOG_CMD_REQ_MSG:
				Tick_REQ_MSG( (GLOG_CMD_REQ_MSG_T*)pCmd, nSize );
			break;
			case	GLOG_CMD_REQ_COUNT:
				Tick_GLOG_CMD_REQ_COUNT( (GLOG_CMD_REQ_COUNT_T*)pCmd, nSize );
			break;
		}
	}

	return	0;
}
int		GLogServerer::Tick_KeepAlive()
{
	// 判断各个Logger是否有失去联系的
	m_Loggers.BeginGet();
	LoggerUnit	*pLoggerUnit;
	whtick_t	nNow = wh_gettickcount();
	while( (pLoggerUnit=m_Loggers.GetNext()) != NULL )
	{
		// 对于已经标记的就不用检查了
		if( !pLoggerUnit->bCanBeRemoved )
		{
			if( wh_tickcount_diff(nNow, pLoggerUnit->nLastKATick) >= m_nKATimeOut )
			{
				// 显示移除提示
				m_logwriter.WriteFmtLine(",****,[%s],is timeout and can be removed,%s,****"
					, pLoggerUnit->szExeTag
					, cmn_get_ipnportstr_by_saaddr(&pLoggerUnit->addr)
					);
				// 标记为可移除，如果在被人顶替他前他又恢复了，就清掉这个变量
				pLoggerUnit->bCanBeRemoved	= true;
			}
		}
	}
	return	0;
}
int		GLogServerer::Tick_REQ_KEEPALIVE(GLOG_CMD_INT_T *pCmd)
{
	// 找到对应的单元
	LoggerUnit	*pLoggerUnit = m_Loggers.getptr(pCmd->nParam);
	if( !pLoggerUnit )
	{
		// 已经被删除了
		return	0;
	}
	whtick_t	nNow = wh_gettickcount();
	if( pLoggerUnit->bCanBeRemoved )
	{
		pLoggerUnit->bCanBeRemoved	= false;
		m_logwriter.WriteFmtLine(",****,[%s],is resumed after %d seconds,%s,****"
			, pLoggerUnit->szExeTag
			, wh_tickcount_diff(nNow, pLoggerUnit->nLastKATick)/1000
			, cmn_get_ipnportstr_by_saaddr(&m_fromaddr)
			);
	}
	pLoggerUnit->nLastKATick	= nNow;
	return	0;
}
int		GLogServerer::Tick_REQ_CONNECT(GLOG_CMD_REQ_CONNECT_T *pCmd)
{
	GLOG_CMD_RPL_CONNECT_T	rpl_connect;
	rpl_connect.cmd			= GLOG_CMD_RPL_CONNECT;
	rpl_connect.rst			= GLOG_CMD_RPL_CONNECT_T::RST_OK;
	rpl_connect.nID			= -1;

	// 申请新的LoggerUnit
	LoggerUnit	*pLoggerUnit;
	int			nID	= m_Loggers.AllocUnit(pLoggerUnit);
	if( nID<0 )
	{
		// 看看是否可以清除一些已经掉线的logger
		whvector<int>	vectToDestroy;
		m_Loggers.BeginGet();
		while( (pLoggerUnit=m_Loggers.GetNext()) != NULL )
		{
			if( pLoggerUnit->bCanBeRemoved )
			{
				m_logwriter.WriteFmtLine(",****,[%s],is really removed now! (save space for newcomer),****"
					, pLoggerUnit->szExeTag
					);
				vectToDestroy.push_back(pLoggerUnit->nID);
			}
		}
		for(size_t i=0;i<vectToDestroy.size();i++)
		{
			RemoveLoggerUnit(vectToDestroy[i]);
		}
		nID	= m_Loggers.AllocUnit(pLoggerUnit);
	}
	if( nID>0 )
	{
		pLoggerUnit->clear();
		rpl_connect.nID				= nID;
		pLoggerUnit->nID			= nID;
		pLoggerUnit->nLastKATick	= wh_gettickcount();
		pLoggerUnit->addr			= m_fromaddr;
		WH_STRSURELAST0(pCmd->szExeTag);
		// 给tag加上数据号，便于区别同名不同来源
		if( m_info.bAppendSourceID )
		{
			sprintf(pLoggerUnit->szExeTag, "%s-%08x", pCmd->szExeTag, nID);
		}
		else
		{
			sprintf(pLoggerUnit->szExeTag, "%s", pCmd->szExeTag);
		}

		// 显示开始提示
		m_logwriter.WriteFmtLine(",****,[%s],Started,%s,****"
			, pLoggerUnit->szExeTag
			, cmn_get_ipnportstr_by_saaddr(&m_fromaddr)
			);
	}
	else
	{
		rpl_connect.rst		= GLOG_CMD_RPL_CONNECT_T::RST_ERR_NOROOM;
		// 显示出错提示
		m_logwriter.WriteFmtLine(",****,[%s],Can not be started for there is no room!!!!,%s,****"
			, pCmd->szExeTag
			, cmn_get_ipnportstr_by_saaddr(&m_fromaddr)
			);
	}
	// 将结果返回
	SendToCurAddr(&rpl_connect, sizeof(rpl_connect));
	return	0;
}
int		GLogServerer::Tick_REQ_DISCONNECT(GLOG_CMD_INT_T *pCmd)
{
	LoggerUnit	*pLoggerUnit = m_Loggers.getptr(pCmd->nParam);
	if( !pLoggerUnit )
	{
		return	0;
	}
	// 显示结束提示
	m_logwriter.WriteFmtLine(",****,[%s],Finished!,%s,****"
		, pLoggerUnit->szExeTag
		, cmn_get_ipnportstr_by_saaddr(&m_fromaddr)
		);
	// 删除对应的单元
	RemoveLoggerUnit(pCmd->nParam);
	return	0;
}
int		GLogServerer::Tick_REQ_MSG(GLOG_CMD_REQ_MSG_T *pCmd, int nSize)
{
	// 找到对应的单元
	LoggerUnit	*pLoggerUnit = m_Loggers.getptr(pCmd->nID);
	char	szTmpTag[64];
	char	*pszExeTag = szTmpTag;
	if( !pLoggerUnit )
	{
		// 已经被删除了
		sprintf(szTmpTag, "%08X", pCmd->nID);
	}
	else
	{
		pszExeTag	= pLoggerUnit->szExeTag;
		pLoggerUnit->nLastKATick	= wh_gettickcount();
		// 增加计数
		pLoggerUnit->nCount			++;
	}

	// 获得数据部分的长度
	nSize	-= (sizeof(*pCmd)-1);
	if( nSize<=0 || nSize>whlogwriter::MAXMSGLEN )
	{
		printf("%s, !!!!SELFERROR!!!! Bad size:%d from %s%s"
			, pszExeTag, nSize, cmn_get_ipnportstr_by_saaddr(&m_fromaddr)
			, WHLINEEND
			);
		m_logwriter.WriteFmtLine(",****,[%s],!!!!SELFERROR!!!! Bad size,%d,%s,****"
			, pszExeTag, nSize, cmn_get_ipnportstr_by_saaddr(&m_fromaddr)
			);
		return	-1;
	}
	// 写日志到文件
	if( m_info.bRawWrite )
	{
		m_logwriter.WriteBin(pCmd->szMsg, nSize);
	}
	else
	{
		if( m_info.bAppendTagName )
		{
			// 在前面写上标记
			m_logwriter.RawPrintf("%s,", pszExeTag);
		}
		// 写本行(发来的消息保证后面有0结尾)
		pCmd->szMsg[nSize-1]	= 0;
		m_logwriter.WriteLine(pCmd->szMsg);
	}

	return	0;
}
int		GLogServerer::Tick_GLOG_CMD_REQ_COUNT(GLOG_CMD_REQ_COUNT_T *pCmd, int nSize)
{
	// 找到对应的log单元
	LoggerUnit	*pLoggerUnit = m_Loggers.getptr(pCmd->nID);
	if( !pLoggerUnit )
	{
		// 已经被删除了，不用进行后面的了
		return	0;
	}

	pLoggerUnit->nLastKATick	= wh_gettickcount();
	
	if( pLoggerUnit->nCount != pCmd->nCount )
	{
		// 记录日志
		m_logwriter.RawPrintf("%s,", pLoggerUnit->szExeTag);
		m_logwriter.WriteFmtLine(",****,[%s],!!!!SELFERROR!!!! count diff incoming,%lu,%lu,%lu,****"
			, pLoggerUnit->szExeTag
			, pCmd->nCount
			, pLoggerUnit->nCount
			, (int)(pCmd->nCount - pLoggerUnit->nCount)
			);
		// 记录完了就同步一下
		pLoggerUnit->nCount	= pCmd->nCount;
	}

	return	0;
}
int		GLogServerer::RemoveLoggerUnit(int nID)
{
	LoggerUnit	*pLoggerUnit = m_Loggers.getptr(nID);
	if( pLoggerUnit )
	{
		return	RemoveLoggerUnit(pLoggerUnit);
	}
	return	-1;
}
int		GLogServerer::RemoveLoggerUnit(LoggerUnit *pLoggerUnit)
{
	m_Loggers.FreeUnit(pLoggerUnit->nID);
	return	0;
}
int		GLogServerer::SendToCurAddr(const void *pCmd, size_t nSize)
{
	udp_sendto(m_sock, pCmd, nSize, &m_fromaddr);
	return	0;
}


////////////////////////////////////////////////////////////////////
// GLogger
////////////////////////////////////////////////////////////////////
GLogger::GLogger()
: m_sock(INVALID_SOCKET)
, m_nID(0)
, m_nCount(0)
, m_pLock(NULL)
{
}
GLogger::~GLogger()
{
	Release();
}
int		GLogger::Init(INFO_T *pInfo)
{
	Release();

	m_nCount	= 0;

	memcpy(&m_info, pInfo, sizeof(m_info));

	// nKeepAliveInterval的单位是秒
	m_keepaliveloop.setinterval(m_info.nKeepAliveInterval*1000);
	m_keepaliveloop.reset();

	m_checkcountloop.setinterval(m_info.nCountCheckInterval);
	m_checkcountloop.reset();

	// 创建socket，并固定指向地址
	struct sockaddr_in	addr;
	if( !cmn_get_saaddr_by_ipnportstr(&addr, m_info.szDstAddr) )
	{
		// 无法解析地址
		return	INITRST_ERR_ADDR;
	}
	m_sock	= udp_create_socket(0);
	if( m_sock == INVALID_SOCKET )
	{
		return	INITRST_ERR_SOCK;
	}
	connect(m_sock, (struct sockaddr *)&addr, sizeof(addr));

	// 看是否需要支持MT
	if( m_info.bSupportMT )
	{
		m_pLock			= new whlock;
	}

	// 发送起始包
	GLOG_CMD_REQ_CONNECT_T	req_connect;
	req_connect.cmd		= GLOG_CMD_REQ_CONNECT;
	WH_STRNCPY0(req_connect.szExeTag, m_info.szExeTag);
	RawWrite(&req_connect, sizeof(req_connect));

	// 等待着获取ID
	int	rst = cmn_select_rd(m_sock, m_info.nConnectTimeOut);
	if( rst<=0 )
	{
		return	INITRST_ERR_SVR;
	}
	GLOG_CMD_RPL_CONNECT_T	rpl_connect;
	rst		= ::recv(m_sock, (char *)&rpl_connect, sizeof(rpl_connect), 0);
	if( rst != sizeof(rpl_connect)
	||  rpl_connect.cmd != GLOG_CMD_RPL_CONNECT
	||  rpl_connect.rst != GLOG_CMD_RPL_CONNECT_T::RST_OK
	)
	{
		return	INITRST_ERR_SVR;
	}
	// 设置ID
	m_nID				= rpl_connect.nID;

	return		INITRST_OK;
}
int		GLogger::Release()
{
	// 释放socket
	if( m_sock != INVALID_SOCKET )
	{
		// 发送断开连接包
		GLOG_CMD_INT_T			req_disconnect;
		req_disconnect.cmd		= GLOG_CMD_REQ_DISCONNECT;
		req_disconnect.nParam	= m_nID;

		RawWrite(&req_disconnect, sizeof(req_disconnect));
		// 延时一下，以保证数据能送到server
		wh_sleep(111);

		// 关闭socket
		closesocket(m_sock);
		m_sock	= INVALID_SOCKET;
	}
	if( m_pLock )
	{
		delete	m_pLock;
		m_pLock	= NULL;
	}
	return	0;
}
int		GLogger::Tick()
{
	// 检查keepalive
	if( m_keepaliveloop.check() )
	{
		// 发送一个KeepAlive包
		GLOG_CMD_INT_T			req_keepalive;
		req_keepalive.cmd		= GLOG_CMD_REQ_KEEPALIVE;
		req_keepalive.nParam	= m_nID;
		RawWrite(&req_keepalive, sizeof(req_keepalive));
		// 重新开始
		m_keepaliveloop.reset();
	}
	if( m_checkcountloop.check() )
	{
		// 发送检查包
		GLOG_CMD_REQ_COUNT_T	req_count;
		req_count.cmd			= GLOG_CMD_REQ_COUNT;
		req_count.nID			= m_nID;
		req_count.nCount		= m_nCount;
		RawWrite(&req_count, sizeof(req_count));
		// 重新开始
		m_checkcountloop.reset();
	}
	return	0;
}
int		GLogger::WriteBin(const void *pData, size_t nSize)
{
	// 这个函数是所有日志写操作的最终实现，并且这里需要对m_szMsgBuf变量进行改动，所以在这里实现线程间互锁
	// RawWrite里面只是一个send所以不用考虑互锁了
	if( m_pLock )
	{
		m_pLock->lock();
	}

	GLOG_CMD_REQ_MSG_T	*p_req_msg = (GLOG_CMD_REQ_MSG_T *)m_szMsgBuf;
	p_req_msg->cmd		= GLOG_CMD_REQ_MSG;
	p_req_msg->nID		= m_nID;
	memcpy(p_req_msg->szMsg, pData, nSize);
	nSize				+= sizeof(*p_req_msg)-1;
	RawWrite(p_req_msg, nSize);
	// 增加消息计数
	m_nCount			++;

	if( m_pLock )
	{
		m_pLock->unlock();
	}

	return	0;
}
int		GLogger::WriteLine(const char *szStr)
{
	return	WriteBin(szStr, strlen(szStr)+1);
}
int		GLogger::WriteFmtLine(const char *szFmt, ...)
{
	va_list	arglist;
	va_start( arglist, szFmt );
	vsprintf( m_szFmtStrBuf, szFmt, arglist );
	va_end( arglist );
	return	WriteLine(m_szFmtStrBuf);
}
int		GLogger::WriteFmtLine0(const char *szFmt, va_list arglist)
{
	vsprintf( m_szFmtStrBuf, szFmt, arglist );
	return	WriteLine(m_szFmtStrBuf);
}
int		GLogger::RawWrite(const void *pData, size_t nSize)
{
	return	::send(m_sock, (const char *)pData, nSize, 0);
}

////////////////////////////////////////////////////////////////////
// GLoggerMan
////////////////////////////////////////////////////////////////////
GLoggerMan::GLoggerMan(int nNum)
: m_vectUnits(nNum)
, m_nDftLoggerIdx(0)
{
}
GLoggerMan::~GLoggerMan()
{
	RemoveAllLogger();
}
int		GLoggerMan::AddLogger(int nID, GLogger::INFO_T *pInfo)
{
	if( nID<0 || (size_t)nID>=m_vectUnits.size() )
	{
		assert(0);
		return	-1;
	}
	if( m_vectUnits[nID].pLogger )
	{
		// 该ID原来已经加入
		assert(0);
		return	-2;
	}
	GLogger	*pLogger	= new GLogger;
	assert(pLogger);
	if( pLogger->Init(pInfo)<0 )
	{
		delete	pLogger;
		return	-3;
	}
	GLOGGERUNIT_T		*pUnit	= m_vectUnits.getptr(nID);
	pUnit->pLogger		= pLogger;
	memcpy(&pUnit->info, pInfo, sizeof(pUnit->info));
	return	0;
}
int		GLoggerMan::RemoveAllLogger()
{
	for(size_t i=0;i<m_vectUnits.size();i++)
	{
		GLOGGERUNIT_T	*pUnit	= m_vectUnits.getptr(i);
		if( pUnit->pLogger )
		{
			delete		pUnit->pLogger;
			pUnit->pLogger	= NULL;
		}
	}
	return	0;
}
int		GLoggerMan::WriteStr(int nID, const char *szStr)
{
	GLogger *pLogger	= GetLogger(nID);
	if( !pLogger )
	{
		return	-1;
	}
	return	pLogger->WriteLine(szStr);
}
int		GLoggerMan::WriteFmt(int nID, const char *szFmt, ...)
{
	int		rst;
	va_list	arglist;
	va_start( arglist, szFmt );
	rst		= WriteFmt0(nID, szFmt, arglist);
	va_end( arglist );
	return	rst;
}
int		GLoggerMan::WriteFmt0(int nID, const char *szFmt, va_list arglist)
{
	GLogger *pLogger	= GetLogger(nID);
	if( !pLogger )
	{
		return	-1;
	}
	return	pLogger->WriteFmtLine0(szFmt, arglist);
}
int		GLoggerMan::Tick()
{
	for(size_t i=0;i<m_vectUnits.size();i++)
	{
		GLOGGERUNIT_T	*pUnit	= m_vectUnits.getptr(i);
		if( pUnit->pLogger )
		{
			pUnit->pLogger->Tick();
		}
	}
	return	0;
}
GLogger *	GLoggerMan::GetLogger(int nID)
{
	if( nID<0 || (size_t)nID>=m_vectUnits.size() )
	{
		assert(0);
		return	NULL;
	}
	GLOGGERUNIT_T	*pUnit	= m_vectUnits.getptr(nID);
	if( !pUnit->pLogger )
	{
		pUnit	= m_vectUnits.getptr(m_nDftLoggerIdx);
	}
	return	pUnit->pLogger;
}

// 全局调用的LOG函数
namespace n_whnet
{

struct	UDPGLOGGER_STATIC_INFO_T
{
	GLoggerMan		*g_pGLoggerMan;
	bool			g_bPrintInScreenOnly;
	bool			g_bPrintInScreenToo;
	UDPGLOGGER_STATIC_INFO_T()
	: g_pGLoggerMan(NULL)
	, g_bPrintInScreenOnly(false)
	, g_bPrintInScreenToo(false)
	{
	}
};
static UDPGLOGGER_STATIC_INFO_T		l_si;
static UDPGLOGGER_STATIC_INFO_T		*l_psi	= &l_si;
#define	G_PGLOGGERMAN				l_psi->g_pGLoggerMan
#define	G_BPRINTINSCREENONLY		l_psi->g_bPrintInScreenOnly
#define	G_BPRINTINSCREENTOO			l_psi->g_bPrintInScreenToo
void *	WHNET_UDPGLOGGER_STATIC_INFO_Out()
{
	return	&n_whnet::l_si;
}
void	WHNET_UDPGLOGGER_STATIC_INFO_In(void *pInfo)
{
#ifdef	_DEBUG
	printf("WHNET_UDPGLOGGER_STATIC_INFO_In %p %p%s", l_psi, pInfo, WHLINEEND);
#endif
	l_psi	= (UDPGLOGGER_STATIC_INFO_T *)pInfo;
}

int		GLogger_Init(int nNum)
{
	if( G_PGLOGGERMAN )
	{
		// 已经初始化过了
		assert(0);
		return	-1;
	}
	G_PGLOGGERMAN	= new GLoggerMan(nNum);
	assert(G_PGLOGGERMAN);
	return		0;
}
int		GLogger_Release()
{
	if( !G_PGLOGGERMAN )
	{
		// 还没有初始化过
		return	-1;
	}
	delete			G_PGLOGGERMAN;
	G_PGLOGGERMAN	= NULL;
	return		0;
}
int		GLogger_Add(int nID, GLogger::INFO_T *pInfo)
{
	assert(G_PGLOGGERMAN);
	return	G_PGLOGGERMAN->AddLogger(nID, pInfo);
}
int		GLogger_AddMulti(GLogger::INFO_T *pCmnInfo, const char *cszAddrs)
{
	assert(G_PGLOGGERMAN);
	int		nOffset	= 0;
	int		nID		= 0;
	while( wh_strsplit(&nOffset, "ds", cszAddrs, ",", &nID, pCmnInfo->szDstAddr)==2 )
	{
		if( G_PGLOGGERMAN->AddLogger(nID, pCmnInfo)<0 )
		{
			return	-100 - nID;
		}
	}
	return	0;
}
int		GLogger_WriteFmt(int nID, const char *szFmt, ...)
{
	// 为了快就不检查了。
	if( !G_PGLOGGERMAN )
	{
		return	0;
	}
	int		rst	= 0;
	va_list	arglist;
	va_start( arglist, szFmt );
	if( !G_BPRINTINSCREENONLY )
	{
		rst		= G_PGLOGGERMAN->WriteFmt0(nID, szFmt, arglist);
	}
	if( G_BPRINTINSCREENONLY || G_BPRINTINSCREENTOO )
	{
		vprintf(szFmt, arglist);
		// 为了保险打印一个空行结尾(2005-09-07加)
		printf(WHLINEEND);
	}
	va_end( arglist );
	return	rst;
}
int		GLogger_WriteFmtStd(int nID, int nLOGID, const char *cszLOGKW, const char *szFmt, ...)
{
	// 为了快就不检查了。
	if( !G_PGLOGGERMAN )
	{
		return	0;
	}
	int		rst	= 0;
	va_list	arglist;
	va_start( arglist, szFmt );
	if( !G_BPRINTINSCREENONLY )
	{
		rst		= G_PGLOGGERMAN->WriteFmt0(nID, szFmt, arglist);
	}
	if( G_BPRINTINSCREENONLY || G_BPRINTINSCREENTOO )
	{
		vprintf(szFmt, arglist);
		// 为了保险打印一个空行结尾(2005-09-07加)
		printf(WHLINEEND);
	}
	va_end( arglist );
	return	rst;
}
int		GLogger_WriteStr(int nID, const char *szStr)
{
	int		rst	= 0;
	if( !G_BPRINTINSCREENONLY )
	{
		rst		= G_PGLOGGERMAN->WriteStr(nID, szStr);
	}
	if( G_BPRINTINSCREENONLY || G_BPRINTINSCREENTOO )
	{
		puts(szStr);
	}
	return	rst;
}
int		GLogger_Tick()
{
	if( !G_PGLOGGERMAN )
	{
		return	0;
	}

	return	G_PGLOGGERMAN->Tick();
}
void	GLogger_Set_PrintInScreenOnly(bool bSet)
{
	G_BPRINTINSCREENONLY	= bSet;
}
void	GLogger_Set_PrintInScreenToo(bool bSet)
{
	G_BPRINTINSCREENTOO		= bSet;
}

}		// EOF namespace n_whnet
