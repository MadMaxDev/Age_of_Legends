// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudp.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 简单的UDP网络功能
// CreationDate : 2003-05-15
// ChangeLOG    : 2005-09-08 udp_create_socket_by_ipnportstr中增加了对随机绑定的支持

#include "../inc/whnetudp.h"
#include <WHCMN/inc/whstring.h>
#include <WHCMN/inc/whvector.h>
#include <WHCMN/inc/whdir.h>
#include <WHCMN/inc/whdbg.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

// 这几个用完立即注释掉(免得被正常的程序使用)
#ifdef	_DEBUG
//#define	__WHNET_UDP_DEBUG_LOST_TEST
//#define	__WHNET_UDP_DEBUG_SENDRECV_LOG
#endif

using namespace n_whcmn;

namespace n_whnet
{

#define	UDPLOGGER_MAXMSGLEN		2048
#define	UDPREGER_MAXMSGLEN		2048

////////////////////////////////////////////////////////////////////
// 函数
////////////////////////////////////////////////////////////////////
SOCKET	udp_create_socket(port_t __localport, struct sockaddr_in *__addr)
{
	return	udp_create_socket(NULL, __localport, __addr);
}
SOCKET	udp_create_socket(const char *__ip, port_t __localport, struct sockaddr_in *__addr)
{
	return	udp_create_rand_socket(__ip, __localport, 0, __addr);
}
SOCKET	udp_create_rand_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr)
{
	return	udp_create_rand_socket(NULL, __port1, __port2, __addr);
}
SOCKET	udp_create_rand_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr)
{
	return	cmn_create_socket(SOCK_DGRAM, IPPROTO_UDP, __ip, __port1, __port2, __addr);
}

SOCKET	udp_create_socket_by_ipnportstr(const char *__ipnportstr, struct sockaddr_in *__addr)
{
	char	szIP[WHNET_MAXADDRSTRLEN]="";
	char	szPorts[WHNET_MAXADDRSTRLEN]="";
	int		nPort1	= 0;
	int		nPort2	= 0;
	wh_strsplit("ss", __ipnportstr, ":", szIP, szPorts);
	wh_strsplit("dd", szPorts, ",", &nPort1, &nPort2);
	return	udp_create_rand_socket(szIP, nPort1, nPort2, __addr);
}
SOCKET	udp_create_lo(int __recvbufsize)
{
	struct sockaddr_in	addr;
	SOCKET				socklo = udp_create_socket("localhost", 0, &addr);
	int					rst;
	if( socklo!=INVALID_SOCKET )
	{
		rst	= connect(socklo, (struct sockaddr *)&addr, sizeof(addr));
		if( rst<0 )
		{
			closesocket(socklo);
			socklo	= INVALID_SOCKET;
		}
		else
		{
			if( __recvbufsize>0 )
			{
				cmn_setsockopt_int(socklo, SOL_SOCKET, SO_RCVBUF, __recvbufsize);
			}
		}
	}
	return	socklo;
}
bool	udp_socketevent_set(SOCKET __sock)
{
	int	rst = cmn_getsizetorecv(__sock);
	if( rst<0 )
	{
		return	false;
	}
	if( rst>0 )
	{
		return	true;
	}
	::send(__sock, "1", 1, 0);
	return	true;
}
bool	udp_socketevent_clr(SOCKET __sock)
{
	char	buf[16];
	while(cmn_select_rd(__sock)>0)
	{
		::recv(__sock, buf, sizeof(buf), 0);
	}
	return	true;
}
bool	udp_socketevent_chk(SOCKET __sock)
{
	return	cmn_getsizetorecv(__sock)>0;
}

#ifdef	__WHNET_UDP_DEBUG_LOST_TEST
static	float	g_udp_sendto_lostratio	= 0;
static	float	g_udp_recv_lostratio	= 0;
void	udp_set_sendto_lostratio(float fLost)
{
	g_udp_sendto_lostratio	= fLost;
}
float	udp_get_sendto_lostratio()
{
	return	g_udp_sendto_lostratio;
}
void	udp_set_recv_lostratio(float fLost)
{
	g_udp_recv_lostratio	= fLost;
}
float	udp_get_recv_lostratio()
{
	return	g_udp_recv_lostratio;
}
#else
void	udp_set_sendto_lostratio(float fLost)
{
}
float	udp_get_sendto_lostratio()
{
	return	0;
}
void	udp_set_recv_lostratio(float fLost)
{
}
float	udp_get_recv_lostratio()
{
	return	0;
}
#endif

int		udp_recv(SOCKET __sock, void *__buf, int __len)
{
	struct sockaddr_in	addr;
	return	udp_recv(__sock, __buf, __len, &addr);
}
int		udp_recv(SOCKET __sock, void *__buf, int __len, struct sockaddr_in *__addr)
{
	socklen_t			len = sizeof(*__addr);
	int		rst			= ::recvfrom(__sock, (char *)__buf, __len, 0, (struct sockaddr*)__addr, &len);
#ifdef	__WHNET_UDP_DEBUG_LOST_TEST
	if( g_udp_recv_lostratio>0 )
	{
		WHRANGE_T<float>	r(0,100.f);
		float	fVal		= r.GetRand();
		if( fVal<g_udp_recv_lostratio )
		{
#ifdef	__WHNET_UDP_DEBUG_LOST_TEST
			WHTMPPRINTF("recv loss size:%d%s", rst, WHLINEEND);
#endif
			return	0;
		}
	}
#endif
#ifdef	__WHNET_UDP_DEBUG_SENDRECV_LOG
	int		nMax=rst<64 ? rst:64;
	char	buf[1024];
	WHTMPPRINTF("%s %lu recv:[%s] size:%d%s%s%s"
		, wh_gettimestr(), wh_gettickcount()
		, cmn_get_ipnportstr_by_saaddr(__addr), rst, WHLINEEND
		, dbg_printmem(__buf, nMax, buf), WHLINEEND
		);
#endif
	return	rst;
}
int		udp_sendto(SOCKET __sock, const void *__buf, int __len, const struct sockaddr_in *__addr)
{
#ifdef	__WHNET_UDP_DEBUG_LOST_TEST
	if( g_udp_sendto_lostratio>0 )
	{
		WHRANGE_T<float>	r(0,100.f);
		float	fVal		= r.GetRand();
		if( fVal<g_udp_sendto_lostratio )
		{
			// 模拟成功了
#ifdef	__WHNET_UDP_DEBUG_LOST_TEST
			WHTMPPRINTF("send loss size:%d%s", __len, WHLINEEND);
#endif
			return	__len;
		}
	}
#endif
#ifdef	__WHNET_UDP_DEBUG_SENDRECV_LOG
	int		nMax=__len<64 ? __len:64;
	char	buf[1024];
	WHTMPPRINTF("%s %lu send:[%s] size:%d%s%s%s"
		, wh_gettimestr(), wh_gettickcount()
		, cmn_get_ipnportstr_by_saaddr(__addr), __len, WHLINEEND
		, dbg_printmem(__buf, nMax, buf), WHLINEEND
		);
#endif
	return	::sendto(__sock, (const char *)__buf, __len, 0, (const struct sockaddr *)__addr, sizeof(*__addr));
}

// 给外部简便用的
static UDPLogger		*g_pUDPLogger			= NULL;
static bool				g_bPrintInScreenOnly	= false;
static bool				g_bPrintInScreenToo		= false;
int		gUDPLogger_Dft_Init(const char *szParam)
{
	if( !szParam || !szParam[0] )
	{
		// 什么也不用初始化
		return	0;
	}
	// 保险起见
	assert(!g_pUDPLogger);

	g_pUDPLogger	= new UDPLogger;
	if( !g_pUDPLogger )
	{
		return	-1;
	}
	return	g_pUDPLogger->Init(szParam);
}
int		gUDPLogger_Dft_Release()
{
	if( g_pUDPLogger )
	{
		g_pUDPLogger->Release();
		delete	g_pUDPLogger;
		g_pUDPLogger	= NULL;
	}
	return	0;
}
int		gUDPLogger_Dft_WriteFmt(const char *szFmt, ...)
{
	char	szMsg[UDPLOGGER_MAXMSGLEN];
	va_list	arglist;
	va_start( arglist, szFmt );
	vsprintf( szMsg, szFmt, arglist );
	va_end( arglist );
	if( g_pUDPLogger )
	{
		if( g_bPrintInScreenToo )
		{
			printf("%s%s", szMsg, WHLINEEND);
		}
		return	g_pUDPLogger->Write(szMsg);
	}
	else
	{
		if( g_bPrintInScreenOnly )
		{
			printf("%s%s", szMsg, WHLINEEND);
		}
		else
		{
			FILE	*fp;
			fp		= fopen("~tmplogger.log", "a");
			fprintf(fp, "%s%s", szMsg, WHLINEEND);
			fclose(fp);
		}
	}
	return	0;
}
void	gUDPLogger_Set_PrintInScreenOnly(bool bSet)
{
	g_bPrintInScreenOnly			= bSet;
}
void	gUDPLogger_Set_PrintInScreenToo(bool bSet)
{
	g_bPrintInScreenToo				= bSet;
}

}		// EOF namespace n_whnet

////////////////////////////////////////////////////////////////////
using namespace n_whnet;
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// UDP，简单的udp socket创建和销毁器
////////////////////////////////////////////////////////////////////
UDP::UDP(port_t nPort)
{
	m_sock	= udp_create_socket(nPort);
}
UDP::UDP(const char *szIP, port_t nPort)
{
	m_sock	= udp_create_socket(szIP, nPort);
}
UDP::~UDP()
{
	if( m_sock != INVALID_SOCKET )
	{
		closesocket(m_sock);
	}
}

////////////////////////////////////////////////////////////////////
// UDPEvent
////////////////////////////////////////////////////////////////////
UDPEvent::UDPEvent()
: m_sock(INVALID_SOCKET)
{
}
UDPEvent::~UDPEvent()
{
	Release();
}
int	UDPEvent::Init()
{
	assert(m_sock == INVALID_SOCKET && "Can not init twice!");
	m_sock	= udp_create_lo();
	if( m_sock == INVALID_SOCKET )
	{
		return	-1;
	}
	return	0;
}
int	UDPEvent::Release()
{
	cmn_safeclosesocket(m_sock);
	return	0;
}
int	UDPEvent::AddEvent(const void *pData, size_t nSize)
{
	assert(m_sock != INVALID_SOCKET);
	::send(m_sock, (const char *)pData, nSize, 0);
	return	0;
}
int	UDPEvent::GetEvent(void *pData, size_t *pnSize)
{
	assert(m_sock != INVALID_SOCKET);
	if( cmn_select_rd(m_sock, 0)<=0 )
	{
		// 没有东西可读
		return	-1;
	}
	*pnSize	= udp_recv(m_sock, pData, *pnSize);
	return	0;
}
int	UDPEvent::ClrAllEvent()
{
	char	buf[4096];
	while(cmn_select_rd(m_sock)>0)
	{
		::recv(m_sock, buf, sizeof(buf), 0);
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// 日志记录器
// UDPLogger
////////////////////////////////////////////////////////////////////

UDPLogger::UDPLogger()
: m_sock(INVALID_SOCKET)
{
}
UDPLogger::~UDPLogger()
{
	Release();
}
int	UDPLogger::Init(const char *szParam)
{
	// 保险起见
	Release();

	char	szIP[WHNET_MAXADDRSTRLEN]	= "127.0.0.1";
	int		nPort						= 1976;
	struct sockaddr_in	dstaddr;

	wh_strsplit("sd", szParam, ":", szIP, &nPort);

	if( !cmn_get_saaddr_by_ipnport(&dstaddr, szIP, nPort) )
	{
		return	-2;
	}

	m_sock	= udp_create_socket(0);
	if( m_sock == INVALID_SOCKET )
	{
		return	-1;
	}
	connect(m_sock, (struct sockaddr *)&dstaddr, sizeof(dstaddr));
	WriteFmt("[start] %s%s", whdir_GetExeModuleFileName(), WHLINEEND);

	return	0;
}
int	UDPLogger::Release()
{
	WriteFmt("[end] %s%s", whdir_GetExeModuleFileName(), WHLINEEND);
	wh_sleep(50);	// 这样保证这个包可以被发出去
	cmn_safeclosesocket(m_sock);

	return	0;
}
int	UDPLogger::WriteBin(const char *szMsg, int nLen)
{
	if( ::send(m_sock, szMsg, nLen, 0) < 0 )
		return	-1;
	return	0;
}
int	UDPLogger::Write(const char *szMsg)
{
	return	WriteBin(szMsg, strlen(szMsg));
}
int	UDPLogger::WriteFmt(const char *szFmt, ...)
{
	char	szMsg[UDPLOGGER_MAXMSGLEN];
	va_list	arglist;
	va_start( arglist, szFmt );
	vsprintf( szMsg, szFmt, arglist );
	va_end( arglist );
	return	Write(szMsg);
}

////////////////////////////////////////////////////////////////////
// 转发器(用于模拟网络上的延时和丢包)
// UDPRouter
////////////////////////////////////////////////////////////////////
UDPRouter::UDPRouter()
: m_sock(INVALID_SOCKET)
, m_bVirgin(true)
, m_nMinDelay(0), m_nMaxDelay(0)
, m_nLoss(0)
, m_pSendList(NULL)
, m_tmpbuf(NULL), m_tmpsize(0)
{
}
UDPRouter::~UDPRouter()
{
	Release();
}
int		UDPRouter::Init(INFO_T *pInfo)
{
	// 初始化随机种子
	srand(wh_time());

	// 创建socket
	m_sock	= udp_create_socket(pInfo->nLocalPort);
	if( m_sock == INVALID_SOCKET )
	{
		return	-1;
	}

	// 目的地址
	if( !cmn_get_saaddr_by_ipnport(&m_addr[DSTADDR], pInfo->szDstIP, pInfo->nDstPort) )
	{
		return	-2;
	}

	// 初始化缓冲分配器
	if( m_Units.Init(pInfo->nMaxPacketNum)<0 )
	{
		return	-3;
	}

	m_pSendList	= new int[pInfo->nMaxPacketNum];
	if( !m_pSendList )
	{
		return	-4;
	}

	m_tmpsize	= pInfo->nMaxPacketSize;
	m_tmpbuf	= new char[m_tmpsize];
	if( !m_tmpbuf )
	{
		return	-5;
	}

	// 其他参数
	m_bVirgin	= true;
	m_nMinDelay	= pInfo->nMinDelay;
	m_nMaxDelay	= pInfo->nMaxDelay;
	m_nLoss		= pInfo->nLoss;

	return		0;
}
int		UDPRouter::Release()
{
	cmn_safeclosesocket(m_sock);

	m_Units.Release();

	if( m_pSendList )
	{
		delete []	m_pSendList;
		m_pSendList	= NULL;
	}

	if( !m_tmpbuf )
	{
		delete []	m_tmpbuf;
		m_tmpbuf	= NULL;
	}

	return		0;
}
int		UDPRouter::Tick()
{
	while( cmn_havedatatorecv(m_sock) )
	{
		DoOneRecv();
	}

	DoSend();

	return	0;
}
int		UDPRouter::DoOneRecv()
{
	// 获得数据
	int					size;
	int					toaddridx;		// 该包需要发向的地址索引
	socklen_t			len;
	struct sockaddr_in	addr;
	len		= sizeof(addr);
	size	= ::recvfrom(m_sock, m_tmpbuf, m_tmpsize, 0, (struct sockaddr *)&addr, &len);
	if( size<=0 || len!=sizeof(addr) )
	{
		return	0;
	}

	toaddridx	= DSTADDR;
	if( m_bVirgin )
	{
		// 终于来了第一个包了，就嫁给他了
		memcpy(&m_addr[SRCADDR], &addr, sizeof(addr));
		// 然后就，...，嘿嘿黑
		m_bVirgin	= false;
	}
	else
	{
		// 检查是不是俺家里人来的
		if( cmn_addr_cmp(&m_addr[SRCADDR], &addr)==0 )
		{
			toaddridx	= DSTADDR;
		}
		else if( cmn_addr_cmp(&m_addr[DSTADDR], &addr)==0 )
		{
			toaddridx	= SRCADDR;
		}
		else
		{
			// 不是俺认识的包，俺什么也不做
			return	0;
		}
	}

	// 判断是否丢
	if( ShouldItLost() )
	{
		// 什么也不做，就丢了
		return	0;
	}
	// 还是延时
	int		nDelay = GenDelay();
	if( nDelay == 0 )
	{
		// 立即发送
		udp_sendto(m_sock, m_tmpbuf, size, &m_addr[toaddridx]);
		return	0;
	}
	// 添加到延时缓冲中
	int		nIdx;
	PACKETUNIT_T	*pUnit;
	nIdx	= m_Units.AllocUnit(pUnit);
	if( nIdx<0 )
	{
		// 缓冲中的包太多了，这个就丢了吧
		return	0;
	}
	pUnit->nAddr	= toaddridx;
	pUnit->nTimeToSend	= wh_gettickcount() + nDelay;
	pUnit->nSize	= size;
	pUnit->szData	= new char[size];
	if( !pUnit->szData )
	{
		// 释放不该有的单元
		m_Units.FreeUnit(nIdx);
		// 这就有问题了
		return	-1;
	}
	memcpy(pUnit->szData, m_tmpbuf, size);

	return	0;
}
int		UDPRouter::DoSend()
{
	int				nIdx, nCount;
	PACKETUNIT_T	*pUnit;
	unsigned int	nCurTick = wh_gettickcount();
	// 判断哪些包到了该发送的时间
	m_Units.BeginGet();

	nCount	= 0;
	while( (pUnit=m_Units.GetNext(&nIdx)) != NULL )
	{
		if( wh_tickcount_diff(nCurTick, pUnit->nTimeToSend)>=0 )
		{
			// 发送数据
			udp_sendto(m_sock, pUnit->szData, pUnit->nSize, &m_addr[pUnit->nAddr]);
			// 删除分配的内存
			delete []	pUnit->szData;
			// 记录，以便后面删除
			m_pSendList[nCount++]	= nIdx;
		}
	}
	// 将所有刚刚发送的数据从缓冲中删除
	while(--nCount>=0)
	{
		m_Units.FreeUnit( m_pSendList[nCount] );
	}
	return	0;
}
bool	UDPRouter::ShouldItLost()
{
	if( m_nLoss==0 )
	{
		return	false;
	}
	if( rand()%LOSSRANGE < m_nLoss )
	{
		// 随机在丢，丢之
		return	true;
	}
	return		false;
}
int		UDPRouter::GenDelay()
{
	return	rand() % (m_nMaxDelay + 1 - m_nMinDelay) + m_nMinDelay;
}

////////////////////////////////////////////////////////////////////
// 基于地址的登记列表（比如用于游戏列表）
////////////////////////////////////////////////////////////////////
UDPRegLister::UDPRegLister()
{
}
UDPRegLister::~UDPRegLister()
{
	Release();
}
int		UDPRegLister::Init(INFO_T *pInfo)
{
	int		rst;

	memcpy(&m_info, pInfo, sizeof(m_info));

	rst		= m_Units.Init(m_info.nMaxRegNum);
	if( rst<0 )
	{
		return	-1;
	}

	return	0;
}
int		UDPRegLister::Release()
{
	m_Units.Release();
	m_mapAddr2ID.clear();
	return	0;
}
int		UDPRegLister::RemoveUnit(int nID)
{
	UNIT_T	*pUnit = m_Units.getptr(nID);
	if( !pUnit )
	{
		return	-1;
	}

	// 扩充操作
	BeforeDeleteUnit(nID);

	// 从地址map中删除
	m_mapAddr2ID.erase(pUnit->addr);
	//
	pUnit->clear();
	m_Units.FreeUnit(nID);

	return	0;
}
int		UDPRegLister::Tick()
{
	struct sockaddr_in	addr;
	socklen_t			len;
	char				buf[UDPREGER_MAXMSGLEN];
	int					size;
	CMD_T				*pCmd = (CMD_T *)buf;
	int					nUnitID;
	UNIT_T				*pUnit;
	unsigned int		nThisTick = wh_gettickcount();

	m_vectToRemove.clear();

	// 检查输入
	while( cmn_select_rd(m_info.sock, 0)>0 )
	{
		size	= RecvFrom(buf, sizeof(buf), &addr, &len);
		if( size>0 )
		{
			// 获得数据部分的长度
			size	-= ( sizeof(CMD_T) - 1 );

			// 不管什么状态只要是UNREG命令就发回应
			switch( pCmd->cmd )
			{
				case	CMD_UNREGIST:
					SendCmdTo(CMD_UNREGIST_OK, &addr);
				break;
			}

			// 判断地址是否已经存在
			if( m_mapAddr2ID.get(addr, nUnitID) )
			{
				// 存在
				// 获得相应的单元指针
				pUnit	= m_Units.getptr(nUnitID);
				if( !pUnit )
				{
					//
					m_mapAddr2ID.erase(addr);
					// 不应该的
					assert(0);
				}
				else
				{
					switch(pCmd->cmd)
					{
						case	CMD_UNREGIST:
							// 拷贝数据
							if( size>m_info.nMaxUnRegData )
							{
								// 不处理这样的数据
							}
							else
							{
								pUnit->unregData.resize(m_info.nMaxUnRegData);
								pUnit->unregData.resize(size);
								memcpy(pUnit->unregData.getbuf(), pCmd->data, size);
							}
							// 删除单元
							RemoveUnit(nUnitID);
						break;
						case	CMD_NOOP:
							pUnit->nLastRecvTime	= nThisTick;
							NoopJob(nUnitID);
						break;
						case	CMD_DATA:
							pUnit->nLastRecvTime	= nThisTick;
							// 拷贝数据
							if( size>m_info.nMaxWorkData )
							{
								// 不处理这样的数据
							}
							else
							{
								pUnit->workData.resize(size>m_info.nMaxWorkData);
								pUnit->workData.resize(size);
								memcpy(pUnit->workData.getbuf(), pCmd->data, size);
								DataJob(nUnitID);
							}
						break;
						case	CMD_REGIST:
							// 再收到可能是丢包了，重发同意包
							SendCmdTo(CMD_REGIST_AGREE, &addr);
						break;
					}
				}
			}
			else
			{
				// 不存在
				switch(pCmd->cmd)
				{
					case	CMD_REGIST:
						// 判断大小是否正确
						if( size>m_info.nMaxRegData )
						{
							// 什么也不用做了，好阴险的数据!!!!
						}
						else
						{
							// 检查数据的合法性
							switch( CheckRegist(pCmd->data, size) )
							{
								case	CHECK_REG_RST_OK:
									// 创建一个新的单元
									nUnitID	= m_Units.AllocUnit(pUnit);
									if( nUnitID<0 )
									{
										// 不能注册了
										SendCmdTo(CMD_REGIST_REFUSE, &addr);
									}
									else
									{
										pUnit->regData.resize(m_info.nMaxRegData);
										pUnit->regData.resize(size);
										// 填写
										pUnit->nCreateTime	= nThisTick;
										pUnit->nLastRecvTime= pUnit->nCreateTime;
										memcpy(&pUnit->addr, &addr, sizeof(pUnit->addr));
										memcpy(pUnit->regData.getbuf(), pCmd->data, size);
										pUnit->nStatus	= UNIT_T::STATUS_WORKING;
										SendCmdTo(CMD_REGIST_AGREE, &addr);
										// 注册地址
										m_mapAddr2ID.put(addr, nUnitID);
									}
								break;
								case	CHECK_REG_RST_ERR:
									// 返回一个错误
									SendCmdTo(CMD_REGIST_REFUSE, &addr);
								break;
							}
						}
					break;
				}
			}
		}
	}

	// 检查是否应该有unit状态变化（比如超时）
	m_Units.BeginGet();
	while( (pUnit=m_Units.GetNext(&nUnitID))!=NULL )
	{
		if( wh_tickcount_diff(nThisTick, pUnit->nLastRecvTime) >= m_info.nDropTimeOut )
		{
			pUnit->nStatus	= UNIT_T::STATUS_DROPPED;
			m_vectToRemove.push_back(nUnitID);
		}
	}
	if( m_vectToRemove.size() )
	{
		for(unsigned int i=0;i<m_vectToRemove.size();i++)
		{
			RemoveUnit(m_vectToRemove[i]);
		}
	}

	return	0;
}
int		UDPRegLister::CheckRegist(char *szData, int nSize)
{
	return	0;
}
int		UDPRegLister::BeforeDeleteUnit(int nID)
{
	return	0;
}
int		UDPRegLister::NoopJob(int nID)
{
	return	0;
}
int		UDPRegLister::DataJob(int nID)
{
	return	0;
}

////////////////////////////////////////////////////////////////////
// 配合上面的登记申请人
////////////////////////////////////////////////////////////////////
UDPReger::UDPReger()
: m_nStatus(0)
, m_nLastNoopTime(0), m_nNoopInterval(0)
, m_nStartReqTime(0), m_nLastReqTime(0)
{
}
UDPReger::~UDPReger()
{
	Release();
}
int		UDPReger::Init(INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));

	return	0;
}
int		UDPReger::Release()
{
	return	0;
}
int		UDPReger::Reg(char *pData, int nSize)
{
	UDPRegLister::CMD_T	*pCmd;
	int	nTotalSize = nSize + sizeof(UDPRegLister::CMD_T) - 1;
	m_RegData.resize(nTotalSize);

	pCmd		= (UDPRegLister::CMD_T *)m_RegData.getbuf();
	pCmd->cmd	= UDPRegLister::CMD_REGIST;
	memcpy(pCmd->data, pData, nSize);

	SetStatus(STATUS_REGING);
	m_nLastReqTime	= wh_gettickcount() - m_info.nReqInterval;
	m_nStartReqTime	= m_nLastReqTime;

	return	0;
}
int		UDPReger::UnReg(char *pData, int nSize)
{
	switch(m_nStatus)
	{
		case	STATUS_WORKING:
			// 这个状态才能UnReg
		break;
		default:
			// 其他状态直接返回
			SetStatus(STATUS_WORKEND);
			return	0;
		break;
	}

	UDPRegLister::CMD_T	*pCmd;
	int	nTotalSize = nSize + sizeof(UDPRegLister::CMD_T) - 1;
	m_UnRegData.resize(nTotalSize);

	pCmd		= (UDPRegLister::CMD_T *)m_UnRegData.getbuf();
	pCmd->cmd	= UDPRegLister::CMD_UNREGIST;
	memcpy(pCmd->data, pData, nSize);

	SetStatus(STATUS_UNREGING);
	m_nLastReqTime	= wh_gettickcount() - m_info.nReqInterval;
	m_nStartReqTime	= m_nLastReqTime;

	return	0;
}
int		UDPReger::SendData(char *pData, int nSize)
{
	switch(m_nStatus)
	{
		case	STATUS_WORKING:
			// 这个状态才能SendData
		break;
		default:
			// 其他状态直接返回
			return	0;
		break;
	}

	UDPRegLister::CMD_T	*pCmd;
	int	nTotalSize = nSize + sizeof(UDPRegLister::CMD_T) - 1;
	m_WorkData.resize(nTotalSize);

	pCmd		= (UDPRegLister::CMD_T *)m_WorkData.getbuf();
	pCmd->cmd	= UDPRegLister::CMD_DATA;
	memcpy(pCmd->data, pData, nSize);

	::send(m_info.sock, m_WorkData.getbuf(), m_WorkData.size(), 0);

	return	0;
}
int		UDPReger::Tick_Reging()
{
	unsigned int	nThisTick = wh_gettickcount();
	// 全部超时了就结束
	if( wh_tickcount_diff(nThisTick, m_nStartReqTime) >= m_info.nReqTimeOut )
	{
		SetStatus(STATUS_DROPPED);
		return	-1;
	}
	// 看是否收到了回应
	while( cmn_select_rd(m_info.sock, 0)>0 )
	{
		int		rst;
		char	buf[UDPREGER_MAXMSGLEN];
		UDPRegLister::CMD_T	*pCmd = (UDPRegLister::CMD_T *)buf;
		rst		= ::recv(m_info.sock, buf, sizeof(buf), 0);
		if( rst>0 )
		{
			switch(pCmd->cmd)
			{
				case	UDPRegLister::CMD_REGIST_AGREE:
					SetStatus(STATUS_WORKING);
					return	0;
				break;
				case	UDPRegLister::CMD_REGIST_REFUSE:
					SetStatus(STATUS_ERROR);
					return	0;
				break;
			}
		}
	}
	// 看超过了时间就发送REQ
	if( wh_tickcount_diff(nThisTick, m_nLastReqTime) >= m_info.nReqInterval )
	{
		::send(m_info.sock, m_RegData.getbuf(), m_RegData.size(), 0);
		m_nLastReqTime	= nThisTick;
	}

	return	0;
}
int		UDPReger::Tick_UnReging()
{
	unsigned int	nThisTick = wh_gettickcount();
	// 全部超时了就结束
	if( wh_tickcount_diff(nThisTick, m_nStartReqTime) >= m_info.nReqTimeOut )
	{
		SetStatus(STATUS_DROPPED);
		return	-1;
	}
	// 看是否收到了回应
	while( cmn_select_rd(m_info.sock, 0)>0 )
	{
		int		rst;
		char	buf[UDPREGER_MAXMSGLEN];
		UDPRegLister::CMD_T	*pCmd = (UDPRegLister::CMD_T *)buf;
		rst		= ::recv(m_info.sock, buf, sizeof(buf), 0);
		if( rst>0 )
		{
			switch(pCmd->cmd)
			{
				case	UDPRegLister::CMD_UNREGIST_OK:
					SetStatus(STATUS_WORKEND);
					return	0;
				break;
			}
		}
	}
	// 看超过了时间就发送REQ
	if( wh_tickcount_diff(nThisTick, m_nLastReqTime) >= m_info.nReqInterval )
	{
		::send(m_info.sock, m_UnRegData.getbuf(), m_UnRegData.size(), 0);
		m_nLastReqTime	= nThisTick;
	}

	return	0;
}
int		UDPReger::Tick_Working()
{
	unsigned int	nThisTick = wh_gettickcount();

	// 看超过了时间就发送NOOP
	if( wh_tickcount_diff(nThisTick, m_nLastNoopTime) >= m_nNoopInterval )
	{
		UDPRegLister::CMD_T	Cmd;
		Cmd.cmd	= UDPRegLister::CMD_NOOP;
		::send(m_info.sock, (char *)&Cmd, sizeof(Cmd), 0);
		m_nLastNoopTime	= nThisTick;
		m_nNoopInterval	= m_info.rgNoopInterval.GetRand();
	}

	return	0;
}
int		UDPReger::Tick()
{
	int		rst = 0;
	switch(m_nStatus)
	{
		case	STATUS_REGING:
			rst	= Tick_Reging();
		break;
		case	STATUS_UNREGING:
			rst	= Tick_UnReging();
		break;
		case	STATUS_WORKING:
			rst	= Tick_Working();
		break;
	}
	return	rst;
}
void	UDPReger::SetStatus(int nStatus)
{
	m_nStatus	= nStatus;
	switch( m_nStatus )
	{
		case	STATUS_WORKING:
			m_nLastNoopTime	= wh_gettickcount();
			m_nNoopInterval	= 0;
		break;
	}
}
