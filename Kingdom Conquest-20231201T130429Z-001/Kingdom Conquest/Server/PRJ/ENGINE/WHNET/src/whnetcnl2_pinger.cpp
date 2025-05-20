// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_pinger.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 用于测试基于CNL2的服务器的ping值的对象

#include "../inc/whnetcnl2_pinger.h"
#include "../inc/whnetcnl2_i.h"

using namespace n_whnet;
using namespace n_whcmn;

CNL_PINGER::CNL_PINGER(int nMaxDst)
: sock(INVALID_SOCKET)
, nInterval(1000)
, nTimeOut(4000)
, nTotal(10)
{
	sock	= udp_create_socket(0);
	assert(sock!=INVALID_SOCKET);	// 有可能事没有初始化网络造成的
	m_Units.Init(nMaxDst);
}
CNL_PINGER::~CNL_PINGER()
{
	if( sock!=INVALID_SOCKET )
	{
		closesocket(sock);
	}
	m_Units.Release();
}
int		CNL_PINGER::AddUnit(const struct sockaddr_in &addr)
{
	if( sock==INVALID_SOCKET )
	{
		return	-1;
	}

	DSTUNIT_T	*pUnit	= NULL;
	int			nID		= m_Units.AllocUnit(pUnit);
	if( nID<0 )
	{
		return	-2;
	}
	pUnit->clear();
	pUnit->dstaddr		= addr;

	return	nID;
}
int		CNL_PINGER::AddUnit(const char *cszIPPort)
{
	struct sockaddr_in	addr;
	if( !cmn_get_saaddr_by_ipnportstr(&addr, cszIPPort) )
	{
		return	-1;
	}
	return	AddUnit(addr);
}
int		CNL_PINGER::DelUnit(int nID)
{
	return	m_Units.FreeUnit(nID);
}
int		CNL_PINGER::ClearAllUnit()
{
	m_Units.clear();
	return	0;
}
int		CNL_PINGER::Tick()
{
	whtick_t	nNow	= wh_gettickcount();

	// 看收到哪些地址的返回
	while( cmn_select_rd(sock)>0 )
	{
		char				buf[1024];
		struct sockaddr_in	addr;
		if( udp_recv(sock, buf, sizeof(buf), &addr)<=0 )
		{
			// 可能只是由于对方地址不存在导致的返回
			continue;
		}
		CNL2_CMD_EXTEND_T	*pEcho	= (CNL2_CMD_EXTEND_T *)buf;
		if( pEcho->cmd != CNL2_CMD_EXTEND
		||  pEcho->subcmd != CNL2_CMD_EXTEND_T::CMD_ECHO_RST
		)
		{
			// 不是需要的返回
			continue;
		}
		ECHODATA_T		*pEchoData	= (ECHODATA_T *)pEcho->data;
		DSTUNIT_T			*pUnit	= m_Units.getptr(pEchoData->nID);
		if( !pUnit )
		{
			// 对应的pinger已经不存在了（这延迟也太大了）
			// 说明这个是前面的ping留下的，不能算在我头上了
			continue;
		}
		// 延时
		int	nDelay			= nNow - pEchoData->nSendTime;
		pUnit->nTotalDelay	+= nDelay;
		pUnit->nRecvCount	++;
	}

	// 继续发
	for(whunitallocatorFixed<DSTUNIT_T>::iterator it=m_Units.begin();it!=m_Units.end();++it)
	{
		DSTUNIT_T	*pUnit	= &(*it);
		if( pUnit->IsCanSend(nTotal)
		&&  pUnit->IsShouldSend(nNow, nInterval)
		)
		{
			// 发送
			SendEchoReq(pUnit);
		}
	}
	return	0;
}
int		CNL_PINGER::ResetUnit(int nID)
{
	DSTUNIT_T	*pUnit	= m_Units.getptr(nID);
	if( pUnit )
	{
		pUnit->Reset();
		return	0;
	}
	return		-1;
}
int		CNL_PINGER::ResetAllUnit()
{
	for(whunitallocatorFixed<DSTUNIT_T>::iterator it=m_Units.begin();it!=m_Units.end();++it)
	{
		(*it).Reset();
	}
	return		0;
}
bool	CNL_PINGER::IsAllOver(int nID)
{
	DSTUNIT_T	*pUnit	= m_Units.getptr(nID);
	if( pUnit )
	{
		return	pUnit->IsAllOver(nTotal, n_whcmn::wh_gettickcount(), nTimeOut);
	}
	return		false;
}
void	CNL_PINGER::GetStatistic(n_whcmn::whvector<PINGSTAT_T> &vectStat)
{
	vectStat.clear();
	whtick_t	nNow		= wh_gettickcount();
	for(whunitallocatorFixed<DSTUNIT_T>::iterator it=m_Units.begin();it!=m_Units.end();++it)
	{
		PINGSTAT_T	*pStat	= vectStat.push_back();
		pStat->nID			= it.getid();
		pStat->addr			= (*it).dstaddr;
		pStat->bIsAllOver	= (*it).IsAllOver(nTotal, nNow, nTimeOut);
		(*it).GetStatistic(&pStat->fLossRate, &pStat->nDelay, &pStat->nWeightedDelay, nTotal);
	}
}
int		CNL_PINGER::GetStatistic(int nID, PINGSTAT_T *pStat)
{
	DSTUNIT_T	*pUnit	= m_Units.getptr(nID);
	if( !pUnit )
	{
		return	-1;
	}
	pStat->nID			= nID;
	pStat->addr			= pUnit->dstaddr;
	pStat->bIsAllOver	= pUnit->IsAllOver(nTotal, wh_gettickcount(), nTimeOut);
	pUnit->GetStatistic(&pStat->fLossRate, &pStat->nDelay, &pStat->nWeightedDelay, nTotal);
	return	0;
}
int		CNL_PINGER::SendEchoReq(DSTUNIT_T *pUnit)
{
	// 发送计数
	pUnit->nSendCount		++;
	pUnit->nLastSendTime	= wh_gettickcount();

	char	buf[wh_offsetof(CNL2_CMD_EXTEND_T, data)+sizeof(ECHODATA_T)];
	CNL2_CMD_EXTEND_T	*pEcho	= (CNL2_CMD_EXTEND_T *)buf;
	pEcho->cmd			= CNL2_CMD_EXTEND;
	pEcho->subcmd		= CNL2_CMD_EXTEND_T::CMD_ECHO;
	ECHODATA_T		*pEchoData	= (ECHODATA_T *)pEcho->data;
	pEchoData->nSendTime		= wh_gettickcount();
	pEchoData->nID				= m_Units.GetIDByPtr(pUnit);
	pEcho->calcsetcrc(sizeof(buf));
	return	udp_sendto(sock, buf, sizeof(buf), &pUnit->dstaddr);
}

void	CNL_PINGER::DSTUNIT_T::Reset()
{
	nLastSendTime	= 0;
	nSendCount		= 0;
	nRecvCount		= 0;
	nTotalDelay		= 0;
}
void	CNL_PINGER::DSTUNIT_T::GetStatistic(float *pfLossRate, int *pnDelay, int *pnWeightedDelay, int nTotalCountRef)
{
	*pfLossRate			= 0;
	*pnDelay			= -1;
	*pnWeightedDelay	= -1;
	float	fGotRate	= 1;
	if( nSendCount>0 )
	{
		fGotRate		= (float)nRecvCount / (float)nSendCount;
		*pfLossRate		= 1 - fGotRate;
	}
	if( nRecvCount>0 )
	{
		*pnDelay		= nTotalDelay / nRecvCount;
		*pnWeightedDelay= (*pnDelay) * nTotalCountRef/(nTotalCountRef-nSendCount+nRecvCount);
	}
}

static void * cnl_pinger_thread_func(void *ptr)
{
	CNL_PINGER_MAN	*pMan	= (CNL_PINGER_MAN *)ptr;
	while( pMan->Tick()==0 );
	return	0;
}

CNL_PINGER_MAN::CNL_PINGER_MAN(int nMaxDst)
: m_pinger(nMaxDst)
, m_tid(INVALID_TID), m_stopthread(false)
, m_interval(50)
{
}
CNL_PINGER_MAN::~CNL_PINGER_MAN()
{
	EndThread();
}
int		CNL_PINGER_MAN::StartThread(int nInterval)
{
	if( m_tid!=INVALID_TID )
	{
		// 线程已经开启了
		assert(0);
		return	-1;
	}
	m_stopthread	= false;
	if( nInterval>0 )
	{
		m_interval	= nInterval;
	}
	if( whthread_create(&m_tid, cnl_pinger_thread_func, this)<0 )
	{
		return	-1;
	}
	return	0;
}
int		CNL_PINGER_MAN::EndThread()
{
	if( m_tid==INVALID_TID )
	{
		// 已经结束过了
		return	0;
	}
	m_stopthread	= true;
//	m_lock.lock();
	whthread_waitend_or_terminate(m_tid, 2000);
//	m_lock.unlock();
	m_tid	= INVALID_TID;
	return	0;
}
int		CNL_PINGER_MAN::Tick()
{
	if( m_stopthread )
	{
		return	-1;
	}
	cmn_select_rd(m_pinger.GetSocket(), m_interval);
	m_lock.lock();
	m_pinger.Tick();
	m_lock.unlock();
	return	0;
}
