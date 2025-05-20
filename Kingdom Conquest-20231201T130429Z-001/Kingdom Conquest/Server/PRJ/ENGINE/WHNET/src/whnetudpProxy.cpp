// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpProxy.cpp
// Creator      : Wei Hua (魏华)
// Comment      : UDP端口转发对象的实现部分
// CreationDate : 2006-12-19
// ChangeLog    :

#include "../inc/whnetudpProxy.h"
#include "../inc/whnetudp.h"
#include "../inc/whnetudpGLogger.h"
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whhash.h>

using namespace n_whcmn;
using namespace n_whnet;

namespace n_whnet
{

class	whnetudpProxy_I	: public whnetudpProxy
{
// 为接口实现的方法
public:
	whnetudpProxy_I();
	virtual	~whnetudpProxy_I();
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const INFO_T *pInfo);
	virtual	int		ResetInfo(const INFO_T *pInfo);
	virtual	int		Release();
	virtual	int		Tick(int nMS);
// 自用的部分
protected:
	// 常量和结构定义
	struct	FORWARD_T
	{
		SOCKET						sockDest;						// 向目标发送数据和接收数据的socket
		struct sockaddr_in			addrSrc;						// 用户来源地址（从sockDest收到的数据就转发回addrSrc去）
		whtick_t					tickLastRecv;					// 最后一次收到来包（双方的都可以）
		FORWARD_T()
			: sockDest(INVALID_SOCKET)
			, tickLastRecv(0)
		{
			memset(&addrSrc, 0, sizeof(addrSrc));
		}
		void	clear()
		{
			cmn_safeclosesocket(sockDest);
		}
	};
	struct	TQUNIT_T
	{
		typedef		void (whnetudpProxy_I::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// 处理该事件的函数
		void		*pData;											// 附加数据
		size_t		nDSize;											// 附加数据尺寸
		union
		{
			int		nForwardID;										// 本事件关联的Forward单元的ID（用于）
		}			un;

		void		clear()
		{
			WHMEMSET0THIS();
		}
		void		SetData(const void *__pData, size_t __nDSize)
		{
			assert(pData == NULL);
			pData	= malloc(__nDSize);
			nDSize	= __nDSize;
			memcpy(pData, __pData, nDSize);
		}
		void		FreeData()
		{
			if( pData )
			{
				free(pData);
				pData	= NULL;
			}
		}
	};
	// 成员变量
	INFO_T							m_info;							// 配置信息
	SOCKET							m_sockListen;					// 监听用户来包的socket
	struct sockaddr_in				m_addrDest;						// 目标地址
	whunitallocatorFixed<FORWARD_T>	m_Forwards;						// 所有Forward操作的数组
	whhash<struct sockaddr_in, int, whcmnallocationobj, _whnet_addr_hashfunc>			m_mapAddrSrc2Forward;
																	// 来源地址到Forward单元ID的映射
	whtimequeue						m_tq;							// 时间队列
	whtick_t						m_tickNow;
	whvector<char>					m_vectBuf;						// 用于临时存储来包的缓冲
	WHRANGE_T<int>					m_wr100;						// 用于生成100以内的数，来模拟丢包的百分数
	WHRANGE_T<int>					m_wrUpDelay;					// 上行的延迟时间
	WHRANGE_T<int>					m_wrDownDelay;					// 下行的延迟时间
	whvector<SOCKET>				m_vectSocket;					// 用于临时存储用于select的socket
private:
	int		Tick_DoSelect(int nMS);
	int		Tick_FORWARD();
	int		Tick_TQ();
	void	Reg_TEDeal_DropCheck(int nForwardID);
	void	TEDeal_DropCheck(TQUNIT_T *pTQUnit);
	void	TEDeal_Up_Send(TQUNIT_T *pTQUnit);
	void	TEDeal_Down_Send(TQUNIT_T *pTQUnit);
};

}	// EOF namespace n_whnet

whnetudpProxy *	whnetudpProxy::Create()
{
	return	new whnetudpProxy_I;
}

whnetudpProxy_I::whnetudpProxy_I()
: m_sockListen(INVALID_SOCKET)
, m_tickNow(0)
, m_wr100(0,100)
{
	m_vectSocket.reserve(1024);
}
whnetudpProxy_I::~whnetudpProxy_I()
{
}
int		whnetudpProxy_I::Init(const INFO_T *pInfo)
{
	ResetInfo(pInfo);
	// 判断参数是否合法

	// 创建监听socket
	m_sockListen	= udp_create_socket_by_ipnportstr(m_info.szLocalAddr);
	if( m_sockListen == INVALID_SOCKET )
	{
		return	-11;
	}
	// 获得远端地址
	if( !cmn_get_saaddr_by_ipnportstr(&m_addrDest, m_info.szDestAddr) )
	{
		return	-12;
	}
	// 初始化数组
	if( m_Forwards.Init(m_info.nMaxFORWARD)<0 )
	{
		return	-13;
	}
	// 初始化时间队列
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_info.nTQChunkSize;
	if( m_tq.Init(&tqinfo)<0 )
	{
		return	-14;
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(16,UDPPROXY)"whnetudpProxy_I::Init over,%s,%s", m_info.szLocalAddr, m_info.szDestAddr);

	return	0;
}
int		whnetudpProxy_I::ResetInfo(const INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	m_vectBuf.resize(m_info.nPacketSize);
	m_wrUpDelay.Set(m_info.nUpDelay0, m_info.nUpDelay1);
	m_wrDownDelay.Set(m_info.nDownDelay0, m_info.nDownDelay1);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(17,UDPPROXY)"whnetudpProxy_I::ResetInfo,%d,%d,%d,%d,%d,%d"
		, m_info.nUpLostRate, m_info.nDownLostRate
		, m_info.nUpDelay0, m_info.nUpDelay1
		, m_info.nDownDelay0, m_info.nDownDelay1
		);
	return	0;
}
int		whnetudpProxy_I::Release()
{
	cmn_safeclosesocket(m_sockListen);
	m_Forwards.Release();
	m_tq.Release();
	return	0;
}
int		whnetudpProxy_I::Tick(int nMS)
{
	m_tickNow	= wh_gettickcount();
	// 先进行select
	Tick_DoSelect(nMS);
	// 然后检查看看是否有收到东西（同时进行转发）
	Tick_FORWARD();
	// 时间队列的逻辑
	Tick_TQ();
	return	0;
}
int		whnetudpProxy_I::Tick_DoSelect(int nMS)
{
	m_vectSocket.clear();
	m_vectSocket.push_back(m_sockListen);
	// 然后所有的和dest相连的socket放在后面
	for(whunitallocatorFixed<FORWARD_T>::iterator it=m_Forwards.begin(); it!=m_Forwards.end(); ++it)
	{
		m_vectSocket.push_back((*it).sockDest);
	}
	return	cmn_select_rd_array_big(m_vectSocket.getbuf(), m_vectSocket.size(), nMS);
}
int		whnetudpProxy_I::Tick_FORWARD()
{
	// 先看看Incoming
	while( cmn_select_rd(m_sockListen)>0 )
	{
		// 接收数据，过滤，发送给目标
		struct sockaddr_in	addr;
		int	nRecvSize	= udp_recv(m_sockListen, m_vectBuf.getbuf(), m_vectBuf.size(), &addr);
		if( nRecvSize<=0 )
		{
			// 可能是
			continue;
		}
		// 看是否丢包
		if( m_info.nUpLostRate>0
		&&  m_wr100.GetRand()<=m_info.nUpLostRate
		)
		{
			// 可以丢弃，那就不用继续了
			continue;
		}
		// 查找该来源地址对应那个Forward对象
		int			nID;
		FORWARD_T	*pForward;
		// 看是否是第一次收到这样的包
		if( !m_mapAddrSrc2Forward.get(addr, nID) )
		{
			// 如果原来没有就创建新的
			nID	= m_Forwards.AllocUnit(pForward);
			if( nID<0 )
			{
				// 没有空间继续创建了（这个包就被忽略）
				continue;
			}
			// 加入hash表
			if( !m_mapAddrSrc2Forward.put(addr, nID) )
			{
				m_Forwards.FreeUnit(nID);
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(18,UDPPROXY)"whnetudpProxy_I::Tick_FORWARD,,m_mapAddrSrc2Forward.put");
				assert(0);
				continue;
			}
			// 地址赋值
			pForward->addrSrc	= addr;
			// 创建socket
			pForward->sockDest	= udp_create_socket(0);
			if( pForward->sockDest==INVALID_SOCKET )
			{
				m_mapAddrSrc2Forward.erase(addr);
				pForward->clear();
				m_Forwards.FreeUnit(nID);
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(19,UDPPROXY)"whnetudpProxy_I::Tick_FORWARD,,upd_create_socket sockDest");
				continue;
			}
			// 设置超时检查事件
			Reg_TEDeal_DropCheck(nID);
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(20,tcpproxy)"incoming,0x%X,%s,%d", nID, cmn_get_ipnportstr_by_saaddr(&pForward->addrSrc), m_Forwards.size());
		}
		else
		{
			pForward					= m_Forwards.getptr(nID);
		}
		// 更新收包时间
		pForward->tickLastRecv			= m_tickNow;
		// 到了这里nID和pForward就一定是有效的值了
		// 然后看是否延迟
		int	nDelay	= m_wrUpDelay.GetRand();
		if( nDelay>0 )
		{
			// 加入时间队列
			TQUNIT_T	*pTQUnit;
			whtimequeue::ID_T	tqid;
			if( m_tq.AddGetRef(m_tickNow + nDelay, (void **)&pTQUnit, &tqid)<0 )
			{
				// 加入时间队列失败
				assert(0);
				continue;
			}
			pTQUnit->clear();
			pTQUnit->un.nForwardID	= nID;
			pTQUnit->tefunc			= &whnetudpProxy_I::TEDeal_Up_Send;
			pTQUnit->SetData(m_vectBuf.getbuf(), nRecvSize);
			continue;
		}
		// 到这里的就可以直接发送了
		udp_sendto(pForward->sockDest, m_vectBuf.getbuf(), nRecvSize, &m_addrDest);
	}

	// 再看看FromDest
	for(whunitallocatorFixed<FORWARD_T>::iterator it=m_Forwards.begin(); it!=m_Forwards.end(); ++it)
	{
		FORWARD_T	*pForward	= &(*it);
		while( cmn_select_rd(pForward->sockDest)>0 )
		{
			// 接收数据，过滤，返回给来源
			struct sockaddr_in	addr;
			int	nRecvSize	= udp_recv(pForward->sockDest, m_vectBuf.getbuf(), m_vectBuf.size(), &addr);
			if( nRecvSize<=0 )
			{
				continue;
			}
			// 更新收包时间
			pForward->tickLastRecv	= m_tickNow;
			// 先看是否丢包
			if( m_info.nDownLostRate>0
			&&  m_wr100.GetRand()<=m_info.nDownLostRate
			)
			{
				// 可以丢弃，那就不用继续了
				continue;
			}
			// 然后看是否延迟
			int	nDelay	= m_wrDownDelay.GetRand();
			if( nDelay>0 )
			{
				// 加入时间队列
				TQUNIT_T	*pTQUnit;
				whtimequeue::ID_T	tqid;
				if( m_tq.AddGetRef(m_tickNow + nDelay, (void **)&pTQUnit, &tqid)<0 )
				{
					// 加入时间队列失败
					assert(0);
					continue;
				}
				pTQUnit->clear();
				pTQUnit->un.nForwardID	= m_Forwards.GetIDByPtr(pForward);
				pTQUnit->tefunc			= &whnetudpProxy_I::TEDeal_Down_Send;
				pTQUnit->SetData(m_vectBuf.getbuf(), nRecvSize);
				continue;
			}
			// 到这里的就可以直接发送了
			udp_sendto(m_sockListen, m_vectBuf.getbuf(), nRecvSize, &pForward->addrSrc);
		}
	}
	return	0;
}
int		whnetudpProxy_I::Tick_TQ()
{
	m_tickNow	= wh_gettickcount();
	// 看看时间队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_tq.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// 这里面可以申请别的时间事件，反正和这个不干扰的。
		// 这个get出来的只能通过Del删除，不能自己删
		m_tq.Del(id);
	}
	return	0;
}
////////////////////////////////////////////////////////////////////
// 时间事件处理函数
////////////////////////////////////////////////////////////////////
void	whnetudpProxy_I::Reg_TEDeal_DropCheck(int nForwardID)
{
	TQUNIT_T			*pTQUnit;
	whtimequeue::ID_T	tqid;
	if( m_tq.AddGetRef(m_tickNow + m_info.nDropTimeOut/2, (void **)&pTQUnit, &tqid)==0 )
	{
		pTQUnit->clear();
		pTQUnit->tefunc			= &whnetudpProxy_I::TEDeal_DropCheck;
		pTQUnit->un.nForwardID	= nForwardID;
	}
	else
	{
		assert(0);
	}
}
void	whnetudpProxy_I::TEDeal_DropCheck(TQUNIT_T *pTQUnit)
{
	// 找到相应的Forwar单元
	FORWARD_T	*pForward	= m_Forwards.getptr(pTQUnit->un.nForwardID);
	if( !pForward )
	{
		return;
	}
	// 看看是否上次的接收时间已经超过了允许的值
	if( wh_tickcount_diff(m_tickNow, pForward->tickLastRecv)>=m_info.nDropTimeOut )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(21,UDPPROXY)"remove by timeout,0x%X,%s,%d", pTQUnit->un.nForwardID, cmn_get_ipnportstr_by_saaddr(&pForward->addrSrc), m_Forwards.size()-1);
		// 删除Forward对象
		// 从此不再有这个Forward规则了
		m_mapAddrSrc2Forward.erase(pForward->addrSrc);
		m_Forwards.FreeUnit(pTQUnit->un.nForwardID);
		return;
	}
	// 继续设置下一个超时检查事件
	Reg_TEDeal_DropCheck(pTQUnit->un.nForwardID);
}
void	whnetudpProxy_I::TEDeal_Up_Send(TQUNIT_T *pTQUnit)
{
	// 找到相应的Forwar单元
	FORWARD_T	*pForward	= m_Forwards.getptr(pTQUnit->un.nForwardID);
	if( !pForward )
	{
		return;
	}
	// 发送
	udp_sendto(pForward->sockDest, pTQUnit->pData, pTQUnit->nDSize, &m_addrDest);
	// 释放内存
	pTQUnit->FreeData();
}
void	whnetudpProxy_I::TEDeal_Down_Send(TQUNIT_T *pTQUnit)
{
	// 找到相应的Forwar单元
	FORWARD_T	*pForward	= m_Forwards.getptr(pTQUnit->un.nForwardID);
	if( !pForward )
	{
		return;
	}
	// 发送
	udp_sendto(m_sockListen, pTQUnit->pData, pTQUnit->nDSize, &pForward->addrSrc);
	// 释放内存
	pTQUnit->FreeData();
}
