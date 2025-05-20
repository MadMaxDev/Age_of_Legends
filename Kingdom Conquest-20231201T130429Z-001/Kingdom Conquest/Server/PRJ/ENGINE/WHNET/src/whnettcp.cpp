// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnettcp.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 简单的TCP网络功能
// CreationDate : 2003-05-14
// ChangeLOG    :
// 2004-02-20   : 添加了tcp_create_socket绑定到特定IP的能力
// 2004-07-04     原来tcpmsger::Release中忘了清状态为NOTHING，结果导致被Release了还能调用SendMsg导致assert
// 2004-07-11     原来的tcpretryer::BeginRetry()中发现如果socket创建出来就是错误的，就是直接Sleep，其实是错误的。比如直接连接一个近距离的端口，就会直接报错的。
// 2005-01-19     增加了各个SendMsg的错误返回
// 2005-06-22     重大修正!!!!修改了tcpbigmsger::RecvNextSize中while(nRecvSize>0)为while(nRecvSize>MIN_TCPMSG_WHOLESIZE)
//                原来因为这个导致在输入缓冲中只剩下一个字节的时候数据头不匹配导致数据被清，或者小于4字节的时候尺寸读入错误。
// 2005-07-05     所有的tcpsocket都取消linger了
// 2005-09-08     给所有listen的socket的创建走增加了范围随机绑定功能
// 2007-05-29     如果随机绑定的第二端口为0，则采用resueaddr的方式
// 2007-09-20     给tcpmsger增加了如果超过一定限度就立即发送的机制，避免一个过长的tick内累计到太多的指令知道缓冲满

#include "../inc/whnettcp.h"
#include "../inc/whnetudp.h"
#include <WHCMN/inc/whstring.h>
#include <WHCMN/inc/whdbg.h>
#include <WHCMN/inc/whbits.h>
#include <WHNET/inc/whnetudpGLogger.h>

using namespace n_whcmn;

namespace n_whnet
{

SOCKET	tcp_create_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr)
{
	return	tcp_create_socket(NULL, __port1, __port2, __addr);
}
SOCKET	tcp_create_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr)
{
	SOCKET	sock;
	// 如果__port2为0，则说明绑定单独端口，则reuse之
	if( __port2==0 )
	{
		cmn_push_reuseaddr_val(true);
		sock	= cmn_create_socket(SOCK_STREAM, IPPROTO_TCP, __ip, __port1, __port2, __addr);
		cmn_pop_reuseaddr_val();
	}
	else
	{
		sock	= cmn_create_socket(SOCK_STREAM, IPPROTO_TCP, __ip, __port1, __port2, __addr);
	}
	if( sock != INVALID_SOCKET )
	{
		cmn_setsock_linger(sock, 0, 0);
	}
	return	sock;
}

SOCKET	tcp_create_listen_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr, int __maxconn)
{
	int		rst;
	SOCKET	sock	= tcp_create_socket(__ip, __port1, __port2, __addr);

	if( sock == INVALID_SOCKET )
	{
		return	sock;
	}

	if( __maxconn > TCP_SOMAXCONN )
	{
		__maxconn	= TCP_SOMAXCONN;
	}

	// make it listen
	rst	= listen(sock, __maxconn);
	if( SOCKET_ERROR == rst )
	{
		goto	ErrEnd;
	}

	goto GoodEnd;
ErrEnd:
	// 出错了，如果已经生成了socket则要关闭之
	closesocket(sock);
	sock	= INVALID_SOCKET;
GoodEnd:
	return	sock;
}
SOCKET	tcp_create_listen_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr, int __maxconn)
{
	return	tcp_create_listen_socket(NULL, __port1, __port2, __addr, __maxconn);
}
SOCKET	tcp_create_listen_socket_by_ipnportstr(const char *__ipnport, struct sockaddr_in *__addr, int __maxconn)
{
	char	szIP[WHNET_MAXADDRSTRLEN]="";
	char	szPorts[WHNET_MAXADDRSTRLEN]="";
	int		nPort1	= 0;
	int		nPort2	= 0;
	wh_strsplit("ss", __ipnport, ":", szIP, szPorts);
	wh_strsplit("dd", szPorts, ",", &nPort1, &nPort2);
	return	tcp_create_listen_socket(szIP, nPort1, nPort2, __addr, __maxconn);
}

SOCKET	tcp_create_connect_socket(struct sockaddr_in *__dstaddr, port_t __localport, int __timeout)
{
	bool	bErr;
	int		rst;
	SOCKET	sock;

	sock	= tcp_create_socket(__localport, 0);
	if( sock == INVALID_SOCKET )
	{
		return	sock;
	}

	bErr	= false;

	cmn_setsock_nonblock(sock, true);		// {	SOCKET begin noblock
	// 为了cmn_is_nonblock_error()能够获得正确的错误号
	cmn_clear_error();
	// connect to server
	rst		= connect(sock, (const struct sockaddr *)__dstaddr, sizeof(*__dstaddr));
	// rst must be SOCKET_ERROR
	if( rst == SOCKET_ERROR )
	{
		if( cmn_is_nonblock_error() )
		{
			// 等待超时
			rst	= cmn_select_wr(sock, __timeout);
			if( rst<=0
			||  cmn_getsockerror(sock)!=0 )
			{
				// 超时或者socket错误
				bErr	= true;
			}
		}
		else
		{
			// 错误
			bErr		= true;
		}
	}
	cmn_setsock_nonblock(sock, false);		// }	SOCKET block again

	if( bErr )
	{
		// 出错了，如果已经生成了socket则要关闭之
		closesocket(sock);
		sock	= INVALID_SOCKET;
	}
	return	sock;
}
SOCKET	tcp_create_connect_socket(const char *__ip, port_t __port, port_t __localport, int __timeout)
{
	struct sockaddr_in	addr;
	if( !cmn_get_saaddr_by_ipnport(&addr, __ip, __port) )
	{
		return	INVALID_SOCKET;
	}
	return	tcp_create_connect_socket(&addr, __localport, __timeout);
}
SOCKET	tcp_create_connect_socket_by_ipnportstr(const char *__ipnport, port_t __localport, int __timeout)
{
	char	szIP[WHNET_MAXADDRSTRLEN];
	int		nPort;
	if( wh_strsplit("sd", __ipnport, ":", szIP, &nPort)<2 )
	{
		// 地址格式错误
		return	INVALID_SOCKET;
	}
	return	tcp_create_connect_socket(szIP, nPort, __localport, __timeout);
}
SOCKET	tcp_create_connecting_socket(const char *__ip, port_t __port, port_t __localport)
{
	struct sockaddr_in	addr;
	// resolve the server sa_addr
	if( !cmn_get_saaddr_by_ipnport(&addr, __ip, __port) )
	{
		return	INVALID_SOCKET;
	}

	return	tcp_create_connecting_socket(&addr, __localport);
}
SOCKET	tcp_create_connecting_socket(const char *__ipportstr, port_t __localport)
{
	struct sockaddr_in	addr;
	// resolve the server sa_addr
	if( !cmn_get_saaddr_by_ipnportstr(&addr, __ipportstr) )
	{
		return	INVALID_SOCKET;
	}

	return	tcp_create_connecting_socket(&addr, __localport);
}
SOCKET	tcp_create_connecting_socket(const struct sockaddr_in *__addr, port_t __localport)
{
	int		rst;
	SOCKET	sock;

	sock	= tcp_create_socket(__localport, 0);
	if( sock == INVALID_SOCKET )
	{
		return	sock;
	}

	cmn_setsock_nonblock(sock, true);		// {	SOCKET begin noblock
	// 为了cmn_is_nonblock_error()能够获得正确的错误号
	cmn_clear_error();
	// connect to server
	rst		= connect(sock, (const struct sockaddr *)__addr, sizeof(*__addr));
	// rst must be SOCKET_ERROR
	if( rst == SOCKET_ERROR )
	{
		if( cmn_is_nonblock_error() )
		{
			if( cmn_select_wr(sock, 10)>0 )
			{
				if( cmn_getsockerror(sock)!=0 )	// 这个可能是在Linux下连接被对方拒绝
				{
					goto	ErrEnd;
				}
			}
			return	sock;
		}
	}
ErrEnd:
	closesocket(sock);
	return	INVALID_SOCKET;
}
int		tcp_check_connecting_socket(SOCKET __sock, int __interval)
{
	// 应该是创建的时候就没有出来
	if( __sock == INVALID_SOCKET )
	{
		return	CONNECTING_SOCKET_CHKRST_ERR;
	}

	// 等待超时
	int rst	= cmn_select_wr(__sock, __interval);
	if( rst<0 )
	{
		// socket错误
		return	CONNECTING_SOCKET_CHKRST_ERR;
	}
	if( rst==0 )
	{
		// 还在Connect中
		return	CONNECTING_SOCKET_CHKRST_CONNTECTING;
	}
	if( cmn_getsockerror(__sock)!=0 )
	{
		// socket错误
		// 这个可能是在Linux下连接被对方拒绝
		return	CONNECTING_SOCKET_CHKRST_ERR;
	}
	// 连接成功了(将其重新置为block模式，理论上上层应该不会再调用这个函数了)
	cmn_setsock_nonblock(__sock, false);
	return		CONNECTING_SOCKET_CHKRST_OK;
}

SOCKET	tcp_create_connect_socket_block(const char *__ip, port_t __port, port_t __localport)
{
	bool	bErr;
	int		rst;
	SOCKET	sock;
	struct sockaddr_in	addr;

	sock	= tcp_create_socket(__localport, 0);
	if( sock == INVALID_SOCKET )
	{
		return	sock;
	}

	bErr	= false;

	// resolve the server sa_addr
	if( cmn_get_saaddr_by_ipnport(&addr, __ip, __port) )
	{
		return	INVALID_SOCKET;
	}

	// connect to server
	rst		= connect(sock, (const struct sockaddr *)&addr, sizeof(addr));

	if( rst<0 )
	{
		closesocket(sock);
		sock	= INVALID_SOCKET;
	}

	return	sock;
}
int		tcp_accept_socket(SOCKET __listensock, SOCKET *__insock, struct sockaddr *__inaddr, socklen_t *__addrlen, int __timeout)
{
	int		rst;

	*__insock	= INVALID_SOCKET;
	rst			= cmn_select_rd(__listensock, __timeout);
	if( rst>0 )
	{
		*__insock	= accept(__listensock, __inaddr, __addrlen);
	}

	return	rst;
}
int		tcp_accept_socket(SOCKET __listensock, SOCKET *__insock, struct sockaddr_in *__inaddr, int __timeout)
{
	socklen_t	len		= sizeof(*__inaddr);
	return		tcp_accept_socket(__listensock, __insock, (struct sockaddr *)__inaddr, &len, __timeout);
}
int		tcp_set_socket_nodelay(SOCKET __sock, int __val)
{
	return	cmn_setsockopt_int(__sock, IPPROTO_TCP, TCP_NODELAY, __val);
}

int	tcp_verysafeclose(SOCKET &__sock)
{
	if( __sock == INVALID_SOCKET )
	{
		return	0;
	}

	int	rst	= shutdown(__sock, SHUT_WR);

	if( rst==0 )
	{
		// 读直到返回0
		whtick_t	t	= wh_gettickcount();
		while( wh_tickcount_diff(wh_gettickcount(), t)<100 )
		{
			if( cmn_select_rd(__sock, 100)>0 )
			{
				char	buf[1024];
				int		nSize	= ::recv(__sock, buf, sizeof(buf), 0);
				if( nSize<=0 )
				{
					break;
				}
			}
		}
	}

	closesocket(__sock);
	__sock	= INVALID_SOCKET;
	return	0;
}

}		// EOF namespace n_whnet


using namespace n_whnet;
using namespace n_whcmn;

static void * tcpmsgdealer_workthread(void *ptr)
{
	tcpmsger	*pSnder = (tcpmsger *)ptr;

	pSnder->WorkThread();

	return	NULL;
}

////////////////////////////////////////////////////////////////////
// tcpmsger
////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(tcpmsger::DATA_INI_INFO_T)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nTrySendSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSendBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nRecvBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSockSndBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSockRcvBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(short, nBegin, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(short, nEnd, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bLogSendRecv, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bNonBlockSocket, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, keepalive_idle, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, keepalive_interval, 0)
WHDATAPROP_MAP_END()
tcpmsger::tcpmsger()
: m_tid(INVALID_TID), m_nStatus(STATUS_NOTHING)
, m_lastrecvtime(0)
, m_bStop(false), m_bUseLock(true)
, m_bNBO(false)
{
}
tcpmsger::~tcpmsger()
{
	// 为了保险调用一下
	Release();
}
int	tcpmsger::Init(INFO_T *pInfo)
{
	int	rst;
	rst	= m_bufSend.Init(pInfo->nSendBufSize, 0, pInfo->nSendBufSize/4);
	if( rst<0 )
	{
		return	-2;
	}
	rst	= m_bufRecv.Init(pInfo->nRecvBufSize, 0, pInfo->nRecvBufSize/4);
	if( rst<0 )
	{
		return	-3;
	}

	memcpy(&m_info, pInfo, sizeof(m_info));

	// 这个主要是怕手动Release后又再次Init
	m_lkSend.reinit();
	m_lkRecv.reinit();

	if( pInfo->bAutoStartThread )
	{
		rst	= StartWorkThread();
		m_bUseLock	= true;
	}
	else
	{
		m_bUseLock	= false;
	}

	if( rst<0 )
	{
		return	-4;
	}

	whtimequeue::INFO_T			tqinfo;
	tqinfo.nUnitLen				= sizeof(TQUNIT_T);
	tqinfo.nChunkSize			= 64;
	m_TQ.Init(&tqinfo);

	whtick_t		ticknow		= wh_gettickcount();

	if( m_info.keepalive_idle>0 && m_info.keepalive_interval )
	{
		int	rst	= cmn_setkeepalive(m_info.sock, m_info.keepalive_idle, m_info.keepalive_interval);
		if( rst<0 )
		{
			return	-100+rst;
		}
	}

	if( m_info.bNonBlockSocket )
	{
		cmn_setsock_nonblock(m_info.sock, true);
	}

	if( m_info.nSockSndBufSize )
	{
		cmn_setsockopt_int(m_info.sock, SOL_SOCKET, SO_SNDBUF, m_info.nSockSndBufSize);
	}
	if( m_info.nSockRcvBufSize )
	{
		cmn_setsockopt_int(m_info.sock, SOL_SOCKET, SO_RCVBUF, m_info.nSockRcvBufSize);
	}

	SetStatus(STATUS_WORKING);
	m_lastrecvtime	= ticknow;
	m_lastsendtime	= ticknow;

	return	0;
}
int	tcpmsger::Release()
{
	// 如果有就终结线程
	StopWorkThread();

	// 关闭socket，虽然外面也可能关闭，不过多关闭一次应该也无所
	cmn_safeclosesocket(m_info.sock);

	// 其他终结
	m_bufSend.Release();
	m_bufRecv.Release();

	m_TQ.Release();

	SetStatus(STATUS_NOTHING);

	return	0;
}
int	tcpmsger::StartWorkThread()
{
	// 开启线程
	m_bStop		= false;
	if( whthread_create(&m_tid, tcpmsgdealer_workthread, this)<0 )
	{
		return	-1;
	}
	return		0;
}
int	tcpmsger::StopWorkThread()
{
	// 停止线程
	m_bStop	= true;
	if( m_tid!=INVALID_TID )
	{
		whthread_waitend_or_terminate(m_tid, 4000);
		m_tid	= INVALID_TID;
	}

	return		0;
}

int	tcpmsger::_SendMsgFillData(tcpmsg_head_t *pHead, const void *pData, size_t nSize)
{
	// 添加头尾
	pHead->nBegin	= m_info.nBegin;
	memcpy(pHead->Data, pData, nSize);
	// 注意：pHead->nSize在tcpmsg_gettail之前变为网络序
	if( m_bNBO )
	{
		// 转换为网络字节序
		pHead->nSize	= htons(nSize);
	}
	else
	{
		pHead->nSize	= nSize;
	}
	tcpmsg_tail_t	*pTail	= tcpmsg_gettail(pHead, m_bNBO);
	pTail->nEnd		= m_info.nEnd;

	return	0;
}
int	tcpmsger::SendMsg(const void *pData, size_t nSize)
{
	if( m_nStatus != STATUS_WORKING )
	{
		// 不处在工作状态就不能继续发送东西了
		return	-1;
	}
	// 奇怪的是有时会出现WORKING状态的invalidsocket
	// 我猜应该是init传过来的就是错误的socket
	if( m_info.sock == INVALID_SOCKET )
	{
		assert(0);
		return	-2;
	}

	assert(nSize<MAX_TCPMSG_SIZE);
	// 返回值
	int		rst = 0;
	// 锁住
	sendlock();
	// 获得总长
	size_t	nTotalSize		= tcpmsg_getwholesize(nSize);
	tcpmsg_head_t	*pHead	= (tcpmsg_head_t *)m_bufSend.InAlloc(nTotalSize);
	if( pHead )
	{
		_SendMsgFillData(pHead, pData, nSize);
	}
	else
	{
		// 缓冲已满，尝试立即发送
		sendunlock();
		ManualSend(0);
		sendlock();
		// 尝试重新分配空间
		pHead	= (tcpmsg_head_t *)m_bufSend.InAlloc(nTotalSize);
		if( pHead )
		{
			_SendMsgFillData(pHead, pData, nSize);
		}
		else
		{
			// 不能发送!! 说明连TCP底层都没有发送的可能了
			rst	= -3;
			// assert(0); 这个在现实中还的确出现了，所以就不用assert了。让用户的消息都丢了吧。
		}
	}
	// 解锁
	sendunlock();
	// 如果超出了指定长度，则立即发送一次
	if( m_bufSend.GetSize()>m_info.nTrySendSize )
	{
		ManualSend(0);
	}

	return	rst;
}
int	tcpmsger::RecvMsg(void *pData, size_t *pnSize)
{
	const tcpmsg_head_t	*pHead	= GetAvailMsgHead();
	if( !pHead )
	{
		return	0;
	}

	msgsize_t	nMsgSize	= pHead->GetSize(m_bNBO);

	// 上面提供的空间尺寸不够
	if( nMsgSize>(*pnSize) )
	{
		return		-1;
	}

	// 拷贝数据
	memcpy(pData, pHead->Data, nMsgSize);
	(*pnSize)	= nMsgSize;

	// 锁住
	recvlock();

	m_bufRecv.Out( tcpmsg_getwholesize(nMsgSize) );

	// 解锁
	recvunlock();
	return	1;
}
const void *	tcpmsger::PeekMsg(size_t *pnSize)
{
	*pnSize	= 0;
	tcpmsg_head_t	*pHead	= GetAvailMsgHead();
	if( !pHead )
	{
		return	NULL;
	}
	*pnSize	= pHead->GetSize(m_bNBO);
	return	pHead->Data;
}
void	tcpmsger::FreeMsg()
{
	tcpmsg_head_t	*pHead	= GetAvailMsgHead();
	if( !pHead )
	{
		return;
	}
	m_bufRecv.Out( tcpmsg_getwholesize(pHead, m_bNBO) );
}
size_t	tcpmsger::GetNextSizeToRecv()
{
	// 这个只读，而且数据递增，所以不用锁定
	tcpmsg_head_t	*pHead	= GetAvailMsgHead();
	if( pHead )
	{
		return	pHead->GetSize(m_bNBO);
	}
	return	0;
}

int	tcpmsger::CleanRecvData()
{
	// 找到下一个开始符号或数据结尾
	int		i, num = m_bufRecv.GetSize()-sizeof(httag_t)+1;
	char	*pData = m_bufRecv.GetBuf();
	bool	bCleanAll = true;

	// 不从第一个开始
	for(i=1;i<num;i++)
	{
		if( (*(httag_t*)(pData+i)) == m_info.nBegin )
		{
			bCleanAll	= false;
			break;
		}
	}

	recvlock();
	if( bCleanAll )
	{
		m_bufRecv.Clean();
	}
	else
	{
		m_bufRecv.Out(i);
	}
	recvunlock();

	return	0;
}

tcpmsg_head_t *	tcpmsger::GetAvailMsgHead()
{
	if( m_bufRecv.GetSize()<=MIN_TCPMSG_WHOLESIZE )
	{
		return	NULL;
	}

	tcpmsg_head_t	*pHead = (tcpmsg_head_t *)m_bufRecv.GetBuf();
	if( pHead->nBegin != m_info.nBegin )
	{
		// 清除错误数据
		CleanRecvData();
		return	NULL;
	}

	if( tcpmsg_getwholesize(pHead, m_bNBO)<=m_bufRecv.GetSize() )
	{
		if( tcpmsg_gettail(pHead, m_bNBO)->nEnd == m_info.nEnd )
		{
			return	pHead;
		}
		else
		{
			// 清除错误数据
			CleanRecvData();
			return	NULL;
		}
	}

	return	NULL;
}

int	tcpmsger::WorkThread()
{
	while(!m_bStop)
	{
		// 检查输入/进行输出
		if( ManualRecvAndSend(m_info.nInterval)<0 )
		{
			// 出错，里面已经设置错误状态了
			return	-1;
		}
	}

	SetStatus(STATUS_NOTHING);
	return	0;
}
int	tcpmsger::ManualRecv(int nInterval)
{
	whtick_t	tickNow	= wh_gettickcount();
	// 看看时间队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_TQ.GetUnitBeforeTime(tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// 这里面可以申请别的时间事件，反正和这个不干扰的。
		// 这个get出来的只能通过Del删除，不能自己删
		m_TQ.Del(id);
	}

	int	rst = 0;
	// 这里面加锁是为了WorkThread可以直接调用
	if( m_bufRecv.GetSizeLeft()>0 )
	{
		// 缓冲没满，尝试接收
		if( (rst=cmn_select_rd(m_info.sock, nInterval)) > 0 )
		{
			// 记录最后一次收到数据的时间(用来判断是否还在保持连接)
			m_lastrecvtime	= tickNow;
			// 锁住
			recvlock();
			// 按最大量收取数据
			int	size = tcp_saferecv(m_info.sock, m_bufRecv.GetTail(), m_bufRecv.GetSizeLeft());
			if( size>0 )
			{
				m_bufRecv.InAlloc(size);
			}
			// 解锁
			recvunlock();

			if( size<=0 )
			{
				// socket出错或者被关闭了
				SetStatus(STATUS_ERROR);
				return	-1;
			}
		}
		else if( rst<0 )
		{
			SetStatus(STATUS_ERROR);
			return	-1;
		}
	}
	else if( m_bufSend.GetSize()==0 && nInterval)
	{
		// 如果没有发出的，并且收取缓冲已满，则睡一会(免得总是占用CPU)
		wh_sleep(nInterval);
	}
	return	0;
}
int	tcpmsger::ManualSend(int nInterval)
{
	// 这里面加锁是为了WorkThread可以直接调用
	if( m_bufSend.GetSize()>0 )
	{
		whtick_t	t1	= wh_gettickcount();
		int	rst = cmn_select_wr(m_info.sock, nInterval);
		int	nDiff	= wh_tickcount_diff(wh_gettickcount(), t1);
		if( nDiff>100 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(872,BIGTICK)"tcpmsger::ManualSend,cmn_select_wr,%d", nDiff);
		}
		if( rst>0 )
		{
			// 记录上次发送的时间
			m_lastsendtime	= wh_gettickcount();
			// 锁住
			sendlock();
			whtick_t	t1	= wh_gettickcount();
			int	size = ::send(m_info.sock, m_bufSend.GetBuf(), m_bufSend.GetSize(), 0);
			int	nDiff	= wh_tickcount_diff(wh_gettickcount(), t1);
			if( nDiff>100 )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(873,BIGTICK)"tcpmsger::ManualSend,send,%d,%d,%d", nDiff, m_bufSend.GetSize(),size);
			}
			if( size>0 )
			{
				m_bufSend.Out(size);
			}
			// 解锁
			sendunlock();
			if( size==0 )
			{
				// 这个是比较奇怪的状况(原来只是在同时使用线程和ManualXXX的时候才会发生)
			}
			else if( size<0 )
			{
				// socket出错或者被关闭了
				SetStatus(STATUS_ERROR);
				return	-1;
			}
		}
		else if( rst<0 )
		{
			SetStatus(STATUS_ERROR);
			return	-1;
		}
	}
	return	0;
}
int	tcpmsger::ManualRecvAndSend(int nInterval)
{
	if( ManualRecv(nInterval)<0 )	return -1;
	if( ManualSend(nInterval)<0 )	return -1;
	return	0;
}

void	tcpmsger::SetStatus(int nStatus)
{
	m_nStatus	= nStatus;
}


////////////////////////////////////////////////////////////////////
// tcpretryer
////////////////////////////////////////////////////////////////////
// 记住啊，改了这里就改了默认的参数值
tcpretryer::tcpretryer()
: m_nStatus(STATUS_NOTHING), m_sock(INVALID_SOCKET)
, m_nTotalRetry(-1), m_nRetryCount(0)
, m_nRetryTimeOut(6000), m_nRetryStartTime(0)
, m_nSleepTime(2000), m_nSleepStartTime(0)
, m_pCurDstAddr(NULL), m_nDstAddrNum(0), m_nCurDstAdrrIdx(0), m_nCurTryCount(0)
{
}
tcpretryer::~tcpretryer()
{
	if( m_sock != INVALID_SOCKET )
	{
		closesocket(m_sock);
		m_sock	= INVALID_SOCKET;
	}
}
void	tcpretryer::ClrAllDstAddr()
{
	m_nDstAddrNum	= 0;
}
int		tcpretryer::AddDstAddr(const char *szDstAddr, int nCanTryCount)
{
	if( m_nDstAddrNum>=MAX_DSTADDRNUM )
	{
		return	-1;
	}
	m_pCurDstAddr				= m_DstAddrs+m_nDstAddrNum;
	strcpy(m_pCurDstAddr->szAddr, szDstAddr);
	m_pCurDstAddr->nCanTryCount	= nCanTryCount;
	m_nDstAddrNum				++;
	return		0;
}
void	tcpretryer::StartWork(bool bSleepFirst)
{
	m_nRetryCount		= 0;
	m_nRetryStartTime	= wh_gettickcount();
	m_nSleepStartTime	= wh_gettickcount();
	m_nStatus			= STATUS_NOTHING;
	m_nCurDstAdrrIdx	= 0;
	m_nCurTryCount		= 0;
	m_pCurDstAddr		= &m_DstAddrs[m_nCurDstAdrrIdx];
	if( !bSleepFirst )
	{
		// 应该立即开始Retry
		BeginRetry();
	}
	else
	{
		// 睡先
		BeginSleep();
	}
}
int		tcpretryer::DoSelect(int nInterval)
{
	// 看是否有连通
	if( m_sock!=INVALID_SOCKET )
	{
		return	cmn_select_wr(m_sock, nInterval);
	}
	return		0;
}
void	tcpretryer::Work()
{
	switch(m_nStatus)
	{
		case	STATUS_NOTHING:
			if( ShouldRetry() )
			{
				BeginRetry();
			}
		break;
		case	STATUS_TRYCONNECT:
			DoRetry();
		break;
		case	STATUS_WORK:
			// 什么也不用做
		break;
		case	STATUS_SLEEP:
			DoSleep();
		break;
	}
}
bool	tcpretryer::ShouldRetry() const
{
	if( m_nTotalRetry<=0 || m_nRetryCount<m_nTotalRetry )
	{
		return	true;
	}
	return		false;
}
void	tcpretryer::BeginSleep()
{
	m_nStatus			= STATUS_SLEEP;
	m_nSleepStartTime	= wh_gettickcount();
}
void	tcpretryer::DoSleep()
{
	if( wh_tickcount_diff(wh_gettickcount(), m_nSleepStartTime) >= m_nSleepTime )
	{
		m_nStatus		= STATUS_NOTHING;
	}
}
void	tcpretryer::BeginRetry()
{
	if( GetCurDstAddr()[0]==0 )
	{
		m_nStatus		= STATUS_NEVERTRY;
		// 不用retry了，根本就没有设置retry的目标
		return;
	}

	// 开始连接
	m_sock			= tcp_create_connecting_socket(GetCurDstAddr(), 0);
	// 重试次数++
	m_nRetryCount	++;
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(8,TCPRETRY)"tcpretryer::BeginRetry,%s,%d", GetCurDstAddr(), m_sock);

	// 开始try的过程
	m_nStatus			= STATUS_TRYCONNECT;
	m_nRetryStartTime	= wh_gettickcount();
}
void	tcpretryer::DoRetry()
{
	bool				bFail = false;
	switch( tcp_check_connecting_socket(m_sock, 0) )
	{
		case	CONNECTING_SOCKET_CHKRST_OK:
			// OK了
			m_nStatus	= STATUS_WORK;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(9,TCPRETRY)"DoRetry OK,%s,%d", GetCurDstAddr(), m_sock);
		break;
		case	CONNECTING_SOCKET_CHKRST_ERR:
			// 出错了
			bFail		= true;
		break;
		default:
			// 超时了
			if( wh_tickcount_diff(wh_gettickcount(), m_nRetryStartTime) >= m_nRetryTimeOut )
			{
				bFail	= true;
			}
		break;
	}
	if( bFail )
	{
		if( m_sock!=INVALID_SOCKET )
		{
			closesocket(m_sock);
		}
		m_sock			= INVALID_SOCKET;
		// 看是否需要换下一个地址
		SwitchNextDst();
		// 开始等待
		BeginSleep();
	}
}
void	tcpretryer::SwitchNextDst()
{
	if( (++m_nCurTryCount) >= m_pCurDstAddr->nCanTryCount )
	{
		m_nCurDstAdrrIdx		++;
		if( m_nCurDstAdrrIdx>=m_nDstAddrNum )
		{
			m_nCurDstAdrrIdx	= 0;
		}
		m_nCurTryCount			= 0;
		m_pCurDstAddr			= &m_DstAddrs[m_nCurDstAdrrIdx];
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(10,TCPRETRY)"tcpretryer::SwitchNextDst,%s", GetCurDstAddr());
	}
}

////////////////////////////////////////////////////////////////////
// tcpmsgerbase
////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(tcpmsgerbase::DATA_INI_INFO_T)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSendBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nRecvBufSize, 0)
WHDATAPROP_MAP_END()
tcpmsgerbase::tcpmsgerbase()
: m_nStatus(STATUS_NOTHING)
, m_nCheckedLen(0)
{
}
tcpmsgerbase::~tcpmsgerbase()
{
	// 为了保险调用一下
	Release();
}
int	tcpmsgerbase::Init(INFO_T *pInfo)
{
	int	rst;
	rst	= m_bufSend.Init(pInfo->nSendBufSize, 0, pInfo->nSendBufSize/4);
	assert(rst==0);

	rst	= m_bufRecv.Init(pInfo->nRecvBufSize, 0, pInfo->nRecvBufSize/4);
	assert(rst==0);

	memcpy(&m_info, pInfo, sizeof(m_info));

	SetStatus(STATUS_WORKING);
	m_nCheckedLen	= 0;
	m_lastrecvtime	= wh_gettickcount();

	return	0;
}
int	tcpmsgerbase::Release()
{
	if( m_info.bAutoCloseSocket )
	{
		if( m_info.sock != INVALID_SOCKET )
		{
			closesocket(m_info.sock);
			m_info.sock	= INVALID_SOCKET;
		}
	}

	// 其他终结
	m_bufSend.Release();
	m_bufRecv.Release();

	SetStatus(STATUS_NOTHING);
	return	0;
}
int	tcpmsgerbase::SendMsg(const void *pData, size_t nSize)
{
	if( m_nStatus != STATUS_WORKING )
	{
		// 不处在工作状态就不能继续发送东西了
		return	-1;
	}
	if( m_info.sock == INVALID_SOCKET )
	{
		assert(0);
		return	-2;
	}

	// 根据长度获得pack后的长度
	int	nTotalSize	= GetPackUnitSize(nSize);
	// 看是否还能分配这么长
	void	*pBuf = m_bufSend.InAlloc(nTotalSize);
	if( !pBuf )
	{
		return	-3;
	}
	// 然后装配之(一般应该是不会出错的)
	PackUnit(pData, nSize, pBuf);
	return	0;
}
int	tcpmsgerbase::RecvMsg(void *pData, size_t *pnSize)
{
	size_t		nSize;
	const void	*pDataPeek	= PeekMsg(&nSize);
	if( !pDataPeek )
	{
		return	0;
	}
	if( *pnSize>0 && *pnSize<nSize )
	{
		// 无法拷贝数据
		return	-1;
	}
	memcpy(pData, pDataPeek, nSize);
	*pnSize	= nSize;
	// 读完了就释放消息啦
	FreeMsg();
	return	1;
}
const void *	tcpmsgerbase::PeekMsg(size_t *pnSize)
{
	if( m_queueLen.size()==0 )
	{
		return	NULL;
	}
	if (m_queueLen[0] < 0)//说明下面一段数据是错误的 应该跳过释放掉
	{
		m_nCheckedLen += m_queueLen[0];
		m_bufRecv.Out(-m_queueLen[0]);
		m_queueLen.pop_front();
		return NULL;
	}
	
	return	GetDataPtrInPackUnit(m_bufRecv.GetBuf(), m_queueLen[0], pnSize);
}
void	tcpmsgerbase::FreeMsg()
{
	if( m_queueLen.size()== 0)
	{
		// 没有东西可释放
		return;
	}
	int	nLen		= m_queueLen[0];
	m_nCheckedLen	-= nLen;
	m_bufRecv.Out(nLen);
	m_queueLen.pop_front();
}
int	tcpmsgerbase::ManualRecv(int nInterval)
{
	int	rst;
	if( m_bufRecv.GetSizeLeft()>0 )
	{
		// 缓冲没满，尝试接收
		if( (rst=cmn_select_rd(m_info.sock, nInterval)) > 0 )
		{
			// 记录最后一次收到数据的时间(用来判断是否还在保持连接)
			m_lastrecvtime	= wh_gettickcount();
			// 按最大量收取数据
			int	size = tcp_saferecv(m_info.sock, m_bufRecv.GetTail(), m_bufRecv.GetSizeLeft());
			if( size>0 )
			{
				m_bufRecv.InAlloc(size);
				// 分析这里有多少完整的unit
				ManualRecv_AnalyzeRecvData();
			}
			else
			{
				// socket出错或者被关闭了
				SetStatus(STATUS_ERROR);
				return	-1;
			}
		}
		else if( rst<0 )
		{
			SetStatus(STATUS_ERROR);
			return	-1;
		}
	}
	else if( m_bufSend.GetSize()==0 && nInterval )
	{
		// 如果没有发出的，并且收取缓冲已满，则睡一会(免得总是占用CPU)
		wh_sleep(nInterval);
	}
	return	0;
}
int	tcpmsgerbase::ManualRecv_AnalyzeRecvData()
{
	for(;;)
	{
		void	*pBegin		= m_bufRecv.GetBuf() + m_nCheckedLen;
		size_t	nUnitLen;
		int		nLeftLen	= m_bufRecv.GetSize()-m_nCheckedLen;
		if( nLeftLen==0 )
		{
			// 结束
			break;
		}
		else if( nLeftLen<0 )
		{
			assert(0);
			break;
		}
		int		rst		= CheckUnit(pBegin, nLeftLen, &nUnitLen);
		if( rst==CHECKUNIT_RST_NA )
		{
			break;
		}
		else if(rst==CHECKUNIT_RST_ERR)
		{
			m_queueLen.push_back(-nUnitLen);
			m_nCheckedLen	+= nUnitLen;
		}
		else if(rst== CHECKUNIT_RST_OK)
		{
			m_queueLen.push_back(nUnitLen);
			m_nCheckedLen	+= nUnitLen;
		}
	}
	return	0;
}
int	tcpmsgerbase::ManualSend(int nInterval)
{
	// 这里面加锁是为了WorkThread可以直接调用
	if( m_bufSend.GetSize()>0 )
	{
		int	rst = cmn_select_wr(m_info.sock, nInterval);
		if( rst>0 )
		{
			int	size = ::send(m_info.sock, m_bufSend.GetBuf(), m_bufSend.GetSize(), 0);
			if( size>0 )
			{
				m_bufSend.Out(size);
			}
			else
			{
				// socket出错或者被关闭了
				SetStatus(STATUS_ERROR);
				return	-1;
			}
		}
		else if( rst<0 )
		{
			SetStatus(STATUS_ERROR);
			return	-1;
		}
	}
	return	0;
}
int	tcpmsgerbase::ManualRecvAndSend(int nInterval)
{
	if( ManualRecv(nInterval)<0 )	return -1;
	if( ManualSend(nInterval)<0 )	return -1;
	return	0;
}
void	tcpmsgerbase::SetStatus(int nStatus)
{
	m_nStatus	= nStatus;
}

// tcpmsger_cmnline
static const char	*CENDSTR		= "\r\n";
static const int CENDSTRLEN	= 2;
static const char	CENDCHAR		= '\n';
int	tcpmsger_cmnline::CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen)
{
	// 从pBegin找直到有结束符
	const char	*pBuf	= (char *)pBegin;
	size_t		i		= 0;
	while( i<nSize )
	{
		if( pBuf[i] == CENDCHAR )
		{
			// 包括最后一个结束符
			*pnUnitLen	= i+1;
			return		CHECKUNIT_RST_OK;
		}
		i	++;
	}
	return	CHECKUNIT_RST_NA;
}
size_t	tcpmsger_cmnline::GetPackUnitSize(size_t nDSize)
{
	return	nDSize+CENDSTRLEN;
}
int	tcpmsger_cmnline::PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf)
{
	memcpy(pDstBuf, pRawData, nDSize);

	memcpy((((char*)pDstBuf)+nDSize), CENDSTR, CENDSTRLEN);
	return	0;
}
const void *	tcpmsger_cmnline::GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize)
{
	*pnDSize	= nTotalSize-CENDSTRLEN;
	return	pPackedData;
}

// tcpmsger_smallpacket
int		tcpmsger_smallpacket::CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen)
{
	mysize_t	*pnSize	= (mysize_t *)pBegin;
	if( nSize<sizeof(*pnSize)
	||  *pnSize > nSize-sizeof(mysize_t) )
	{
		// 还没有收完
		return	CHECKUNIT_RST_NA;
	}
	*pnUnitLen			= GetPackUnitSize(*pnSize);
	return		CHECKUNIT_RST_OK;
}
size_t	tcpmsger_smallpacket::GetPackUnitSize(size_t nDSize)
{
	return		nDSize + sizeof(mysize_t);
}
int		tcpmsger_smallpacket::PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf)
{
	if( nDSize>=0xFFFF )
	{
		assert(0);
		return	-1;
	}
	mysize_t	*pnSize	= (mysize_t *)pDstBuf;
	*pnSize		= nDSize;	// + sizeof(mysize_t);
	pnSize		++;
	memcpy(pnSize, pRawData, nDSize);
	return		0;
}
const void *	tcpmsger_smallpacket::GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize)
{
	*pnDSize	= nTotalSize - sizeof(mysize_t);
	return	((mysize_t *)pPackedData) + 1;
}

// tcpmsger_vn2_packet
int		tcpmsger_vn2_packet::CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen)
{
	// 判断长度
	unsigned int	nDataSize;
	int				nHdrSize	= whbit_vn2_get(pBegin, nSize, &nDataSize);
	if( nHdrSize == 0
	||  nDataSize > nSize-nHdrSize
	)
	{
		// 还没有收完
		return	CHECKUNIT_RST_NA;
	}
	*pnUnitLen		= nHdrSize + nDataSize;
	return		CHECKUNIT_RST_OK;
}
size_t	tcpmsger_vn2_packet::GetPackUnitSize(size_t nDSize)
{
	return		nDSize + whbit_vn2_calclen(nDSize);
}
int		tcpmsger_vn2_packet::PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf)
{
	// 超过16M的传输实在有些大了，因为我内部的缓冲区都不一定能定成这么大
	if( nDSize>=0x1000000 )
	{
		assert(0);
		return	-1;
	}
	// 填写长度头
	int			nHdrSize		= whbit_vn2_set(pDstBuf, nDSize);
	// 拷贝数据
	pDstBuf		= wh_getoffsetaddr(pDstBuf, nHdrSize);
	memcpy(pDstBuf, pRawData, nDSize);
	return		0;
}
const void *	tcpmsger_vn2_packet::GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize)
{
	int				nHdrSize	= whbit_vn2_get(pPackedData, (unsigned int *)pnDSize);
	return			wh_getoffsetaddr(pPackedData, nHdrSize);
}

// tcpmsger_fixedsize
int	tcpmsger_fixedsize::CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen)
{
	if( (int)nSize>=m_nPacketSize )
	{
		*pnUnitLen	= m_nPacketSize;
		return	CHECKUNIT_RST_OK;
	}
	return	CHECKUNIT_RST_NA;
}
size_t	tcpmsger_fixedsize::GetPackUnitSize(size_t nDSize)
{
	assert( (int)nDSize == m_nPacketSize );
	return	nDSize;
}
int	tcpmsger_fixedsize::PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf)
{
	assert( (int)nDSize == m_nPacketSize );
	memcpy(pDstBuf, pRawData, nDSize);
	return	0;
}
const void *	tcpmsger_fixedsize::GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize)
{
	*pnDSize	= m_nPacketSize;
	return	pPackedData;
}

////////////////////////////////////////////////////////////////////
// tcpproxy
////////////////////////////////////////////////////////////////////
tcpproxy::tcpproxy()
: m_sockListen(INVALID_SOCKET)
{
	memset(&m_dstaddr, 0, sizeof(m_dstaddr));
}
tcpproxy::~tcpproxy()
{
	Release();
}
int		tcpproxy::Init(INFO_T *pInfo, struct sockaddr_in *pListenAddr)
{
	m_tickNow		= wh_gettickcount();

	assert( m_sockListen == INVALID_SOCKET );

	m_info			= *pInfo;

	m_sockListen	= tcp_create_listen_socket_by_ipnportstr(pInfo->cszListenAddr, pListenAddr, pInfo->nMaxConnection);
	if( m_sockListen == INVALID_SOCKET )
	{
		return		-1;
	}
	SetDst(pInfo->cszDstAddr);

	if( m_Units.Init(pInfo->nMaxConnection)<0 )
	{
		return		-2;
	}

	// 延迟缓冲
	m_queueDelayUp.Init(pInfo->nDelayQueueSize);
	m_queueDelayDown.Init(pInfo->nDelayQueueSize);

	m_vectBuf.resize(pInfo->nMaxBufLen);

    return			0;
}
int		tcpproxy::Release()
{
	// 删除所有socket
	if( m_Units.size() > 0 )
	{
		for(whunitallocatorFixed<UNIT_T>::iterator it=m_Units.begin(); it!=m_Units.end(); ++it)
		{
			UNIT_T	*pUnit	= &(*it);
			closesocket(pUnit->sockFrom);
			closesocket(pUnit->sockTo);
			pUnit->clear();
		}
		m_Units.clear();
	}
	m_vectBuf.clear();
	m_queueDelayUp.Release();
	m_queueDelayDown.Release();
	return			0;
}
void	tcpproxy::SetDst(const char *cszDstAddr)
{
	cmn_get_saaddr_by_ipnportstr(&m_dstaddr, cszDstAddr);
}
void	tcpproxy::SetDelay(int nUpMin, int nUpMax, int nDownMin, int nDownMax)
{
	m_rgDelayUp.Set(nUpMin, nUpMax);
	m_rgDelayDown.Set(nDownMin, nDownMax);
}
void	tcpproxy::SetByteRate(int nUp, int nDown)
{
	m_info.nBRateUp		= nUp;
	m_info.nBRateDown	= nDown;
}
int		tcpproxy::DoSelect(int nInterval)
{
	m_tickNow	= wh_gettickcount();
	m_selector.clear();
	m_selector.addsocket(m_sockListen);

	for(whunitallocatorFixed<UNIT_T>::iterator it=m_Units.begin(); it!=m_Units.end(); ++it)
	{
		UNIT_T	*pUnit	= &(*it);
		if( pUnit->IsTimeToReadFrom(m_tickNow) )
		{
			m_selector.addsocket(pUnit->sockFrom);
		}
		if( pUnit->IsTimeToReadTo(m_tickNow) )
		{
			m_selector.addsocket(pUnit->sockTo);
		}
	}

	return	m_selector.select_rd(nInterval);
}
int		tcpproxy::DoTick()
{
	m_tickNow	= wh_gettickcount();
	// 看看是否有新对
	SOCKET		insock;
	struct sockaddr_in	inaddr;
	UNIT_T		*pUnit;
	int			i;
	while( tcp_accept_socket(m_sockListen, &insock, &inaddr, 0)>0 )
	{
		// 申请单元
		int	nID	= m_Units.AllocUnit(pUnit);
		if( nID<0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(11,TCPPROXY)"tcpproxy::DoTick,%d,m_Units.AllocUnit,%s", nID, cmn_get_ipnportstr_by_saaddr(&inaddr));
			closesocket(insock);
			continue;
		}
		// 初始化
		pUnit->clear();
		// 设置来源
		pUnit->sockFrom	= insock;
		// 尝试连接目标，为了简单就写成阻塞的了
		pUnit->sockTo	= tcp_create_connect_socket(&m_dstaddr, 0, m_info.nConnectTimeout);
		if( pUnit->sockTo == INVALID_SOCKET )
		{
			closesocket(insock);
			m_Units.FreeUnit(nID);
			char	bufdst[WHNET_MAXIPSTRLEN], bufsrc[WHNET_MAXIPSTRLEN];
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(12,TCPPROXY)"DoTick,tcp_create_connect_socket,%s,%s"
				, cmn_get_ipnportstr_by_saaddr(&inaddr, bufsrc)
				, cmn_get_ipnportstr_by_saaddr(&m_dstaddr, bufdst)
				);
			continue;
		}
		// 设置下次时刻
		pUnit->nFromReadNextTime	= m_tickNow;
		pUnit->nToReadNextTime		= m_tickNow;
		if( m_stat.nMaxConcurrentConnection<m_Units.size() )
		{
			m_stat.nMaxConcurrentConnection	= m_Units.size();
		}
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(13,TCPPROXY)"incoming,,0x%X,%d %s %d %d/%d"
			, nID
			, pUnit->sockFrom, cmn_get_ipnportstr_by_saaddr(&inaddr)
			, pUnit->sockTo
			, m_Units.size(), m_stat.nMaxConcurrentConnection
			);
	}

	// 检查已经连接的对
	// 这里重新获取一次是为了避免在CONNECT的时候阻塞了太多的时间
	m_tickNow	= wh_gettickcount();
	m_vectUnitIDToDel.clear();

	for(whunitallocatorFixed<UNIT_T>::iterator it=m_Units.begin(); it!=m_Units.end(); ++it)
	{
		pUnit	= &(*it);
		int	rst	= 0;
		if( pUnit->sockTo != INVALID_SOCKET )
		{
			if( pUnit->IsTimeToReadTo(m_tickNow) )
			{
				rst	= CheckRecvThenSend(it.getid(), pUnit->sockTo, pUnit->sockFrom, m_info.nBRateDown, pUnit->nToReadNextTime, m_rgDelayDown, m_queueDelayDown, pUnit->nDownCount);
				if( rst<0 )
				{
					// 读取出错，很可能是上游关闭了socket，所以也关闭这个socket
					cmn_safeclosesocket(pUnit->sockTo);
					// 看看下行数据是否已经都发送完毕了
					if( pUnit->nDownCount<=0 )
					{
						// 删除这个对象吧
						m_vectUnitIDToDel.push_back(it.getid());
					}
					else
					{
						// 所以后面也不用检查来源是否有新数据了
						// 但是不用立即关闭这个unit，还是应该等到所有延迟的包都发出了才好
						continue;
					}
				}
			}
		}
		if( pUnit->sockFrom != INVALID_SOCKET )
		{
			if( pUnit->IsTimeToReadFrom(m_tickNow) )
			{
				rst	= CheckRecvThenSend(it.getid(), pUnit->sockFrom, pUnit->sockTo, m_info.nBRateUp, pUnit->nFromReadNextTime, m_rgDelayUp, m_queueDelayUp, pUnit->nUpCount);
				if( rst<0 )
				{
					// 读取出错，很可能是来源关闭了socket，所以也关闭这个socket
					cmn_safeclosesocket(pUnit->sockFrom);
					// 看看上行数据是否已经都发送完毕了
					if( pUnit->nUpCount<=0 )
					{
						// 删除这个对象吧
						m_vectUnitIDToDel.push_back(it.getid());
					}
				}
			}
		}
	}

	// 发送延迟的包
	// 上行的
	for(;;)
	{
		size_t	nSize;
		DELAY_T	*pDelay	= (DELAY_T *)m_queueDelayUp.OutPeek(&nSize);
		if( !pDelay )
		{
			break;
		}
		if( wh_tickcount_diff(m_tickNow, pDelay->nSendTime)<0 )
		{
			// 不可以发送。并且后面的也都不可以。
			break;
		}
		// 删除这条数据（但是前面的指针应该还可以用的）
		m_queueDelayUp.FreeN(1);
		UNIT_T	*pUnit	= m_Units.getptr(pDelay->nUnitID);
		if( !pUnit )
		{
			continue;
		}
		// 发送给目标
		send(pUnit->sockTo, (const char *)(pDelay+1), nSize-sizeof(*pDelay), MSG_NOSIGNAL);
		pUnit->nUpCount	--;
		if( pUnit->nUpCount<=0 && pUnit->sockFrom==INVALID_SOCKET )
		{
			m_vectUnitIDToDel.push_back(pDelay->nUnitID);
		}
	}
	// 下行的
	for(;;)
	{
		size_t	nSize;
		DELAY_T	*pDelay	= (DELAY_T *)m_queueDelayDown.OutPeek(&nSize);
		if( !pDelay )
		{
			break;
		}
		if( wh_tickcount_diff(m_tickNow, pDelay->nSendTime)<0 )
		{
			// 不可以发送。并且后面的也都不可以。
			break;
		}
		// 删除这条数据（但是前面的指针应该还可以用的）
		m_queueDelayDown.FreeN(1);
		//
		UNIT_T	*pUnit	= m_Units.getptr(pDelay->nUnitID);
		if( !pUnit )
		{
			continue;
		}
		// 发送给来源
		send(pUnit->sockFrom, (const char *)(pDelay+1), nSize-sizeof(*pDelay), MSG_NOSIGNAL);
		pUnit->nDownCount	--;
		if( pUnit->nDownCount<=0 && pUnit->sockTo==INVALID_SOCKET )
		{
			m_vectUnitIDToDel.push_back(pDelay->nUnitID);
		}
	}

	// 移除出错的socket对
	for(i=0;i<(int)m_vectUnitIDToDel.size();i++)
	{
		int	nID	= m_vectUnitIDToDel[i];
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(14,TCPPROXY)"remove connection unit,0x%X", nID);
		pUnit	= m_Units.getptr(nID);
		// 关闭两个socket
		cmn_safeclosesocket(pUnit->sockFrom);
		cmn_safeclosesocket(pUnit->sockTo);
		// 删除单元
		m_Units.FreeUnit(nID);
	}

	return	0;
}
int		tcpproxy::CheckRecvThenSend(int nUnitID, SOCKET sockIn, SOCKET sockOut, int nBRate, whtick_t &nNowAndThen, n_whcmn::WHRANGE_T<int> &WR, n_whcmn::whsmpqueue &queueDelay, int &nCount)
{
	const int	MINWAIT	= 10;
	// 先看目标是否可写
	if( cmn_select_wr(sockOut, 0)<=0 )
	{
		nNowAndThen	+= MINWAIT;
		return	0;
	}
	// 尝试读取
	if( cmn_select_rd(sockIn, 0)<=0 )
	{
		// 没有东西可读
		nNowAndThen	+= MINWAIT;
		return	0;
	}
	int	nSize	= recv(sockIn, m_vectBuf.getbuf(), m_vectBuf.size(), 0);
	if( nSize<=0 )
	{
		// 接收出错，外面应该关闭了
		return	-1;
	}
	// 判断是否需要延迟
	int	nDelay	= WR.GetRand();
	if( nDelay==0 )
	{
		// 不用延迟
		// 直接发送
		int	nSendSize		= send(sockOut, m_vectBuf.getbuf(), nSize, MSG_NOSIGNAL);
		if( nSize!=nSendSize )
		{
			// 发送出错，外面应该关闭了
			return	-2;
		}
	}
	else
	{
		// 放入延迟队列
		DELAY_T	*pDelay		= (DELAY_T *)queueDelay.InAlloc(sizeof(DELAY_T) + nSize);
		if( pDelay )
		{
			pDelay->nUnitID		= nUnitID;
			pDelay->nSendTime	= m_tickNow + nDelay;
			void	*pData		= pDelay+1;
			memcpy(pData, m_vectBuf.getbuf(), nSize);
			nCount				++;
		}
		else
		{
			// 这个不应该发生的（只能说明对来包的能力估计不准）
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(15,TCPPROXY)"tcpproxy::CheckRecvThenSend,,queueDelay overflow,0x%X %d", nUnitID, nSize);
			// 删除相应的
			m_vectUnitIDToDel.push_back(nUnitID);
		}
	}
	// 计算下次可以读取的时间
	if( nBRate>0 )
	{
		nNowAndThen	+= nSize*1000 / nBRate;
	}

	return	0;
}
const char *	tcpproxy::ShowInfo(char *pszBuf)
{
	sprintf(pszBuf, "param: BRate:%d,%d Delay:%d,%d,%d,%d Connection:%d/%d"
		, m_info.nBRateUp, m_info.nBRateDown
		, m_rgDelayUp.GetFrom(), m_rgDelayUp.GetTo(), m_rgDelayDown.GetFrom(), m_rgDelayDown.GetTo()
		, m_Units.size(), m_stat.nMaxConcurrentConnection
		);
	return	pszBuf;
}

tcpproxyWT::tcpproxyWT()
: m_tid(INVALID_TID)
, m_bStop(false)
{
}
tcpproxyWT::~tcpproxyWT()
{
	// 保险起见
	StopThread();
}
static	void *	tcpproxyWT_ThreadProc(void *ptr)
{
	tcpproxyWT	*pProxy	= (tcpproxyWT *)ptr;
	while( !pProxy->m_bStop )
	{
		pProxy->DoSelect(10);
		pProxy->DoTick();
	}
	return	0;
}
int		tcpproxyWT::StartThread(int nPriority)
{
	if( m_tid != INVALID_TID )
	{
		assert(0);
		return	-1;
	}
	if( whthread_create(&m_tid, tcpproxyWT_ThreadProc, this, nPriority)<0 )
	{
		return	-2;
	}
	m_bStop	= false;
	return	0;
}
int		tcpproxyWT::StopThread()
{
	if( m_tid != INVALID_TID )
	{
		m_bStop	= true;
		whthread_waitend_or_terminate(m_tid, 5000);
		m_tid	= INVALID_TID;
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// whtcpproxy
////////////////////////////////////////////////////////////////////
whtcpproxy::whtcpproxy()
{
	for(int i=0;i<2;i++)
	{
		m_sock[i]	= INVALID_SOCKET;
		m_vect[i].reserve(4096);
	}
}
whtcpproxy::~whtcpproxy()
{
	for(int i=0;i<2;i++)
	{
		cmn_safeclosesocket(m_sock[i]);
	}
}
void	whtcpproxy::GetSockets(whvector<SOCKET> &vect)
{
	for(int i=0;i<2;i++)
	{
		if( cmn_is_validsocket(m_sock[i]) )
		{
			vect.push_back(m_sock[i]);
		}
	}
}
void	whtcpproxy::SetSocket(int nIdx, SOCKET sock)
{
	assert(nIdx>=0 && nIdx<2);
	m_sock[nIdx]	= sock;
}
int		whtcpproxy::DoTick()
{
	// 从一个收，加入另一个的缓冲
	bool	bAllOver	= true;
	for(int i=0;i<2;i++)
	{
		SOCKET	&sock	= m_sock[i];
		if( !cmn_is_validsocket(sock) )
		{
			continue;
		}
		bAllOver		= false;
		whvector<char>	&vect	= m_vect[1-i];
		// 获得有多长
		int	rst	= cmn_select_rd(sock, 0);
		if( rst>0 )
		{
			int	nSize	= cmn_getsizetorecv(sock);
			if( nSize>0 )
			{
				int	nVSize	= vect.size();
				vect.resize(nVSize + nSize);
				int	rst	= ::recv(sock, vect.getptr(nVSize), nSize, 0);
				if(rst<0)
				{
					printf("recv fail,%d,%d%s", nSize, rst, WHLINEEND);
					// 关闭这个socket
					FailSocket(i);
				}
				else if( rst<nSize )
				{
					vect.resize(vect.size()+rst-nSize);
				}
			}
			else if( nSize<=0 )
			{
				// 关闭这个socket
				printf("getsizetorecv,%d%s", nSize, WHLINEEND);
				FailSocket(i);
			}
		}
		else if( rst<0 )
		{
			// 关闭这个socket
			FailSocket(i);
		}
	}
	// 如果一个可写，则写入
	for(int i=0;i<2;i++)
	{
		SOCKET	&sock	= m_sock[i];
		if( !cmn_is_validsocket(sock) )
		{
			continue;
		}
		// 看socket是否可写
		int	rst	= cmn_select_wr(sock, 0);
		if( rst<0 )
		{
			// 关闭这个socket
			FailSocket(i);
			continue;
		}
		whvector<char>	&vect	= m_vect[i];
		if( vect.size()==0 )
		{
			continue;
		}
		// 发送并移动
		rst	= ::send(sock, vect.getbuf(), vect.size(), 0);
		printf("->%d,%d,%d%s", i, vect.size(), rst, WHLINEEND);
		if( rst<0 )
		{
			// 关闭这个socket
			FailSocket(i);
			continue;
		}
		int	nNewSize	= vect.size() - rst;
		if( nNewSize>0 )
		{
			memmove(vect.getbuf(), vect.getptr(rst), nNewSize);
		}
		vect.resize(nNewSize);
	}
	if( bAllOver )
	{
		return	-1;
	}
	return	0;
}
void	whtcpproxy::FailSocket(int nIdx)
{
	printf("Fail socket %d%s", nIdx, WHLINEEND);
	cmn_safeclosesocket(m_sock[nIdx]);
	OnSocketFail(nIdx);
}

whtcpproxy_Director::whtcpproxy_Director()
{
	memset(m_aD, 0, sizeof(m_aD));
}
whtcpproxy_Director::~whtcpproxy_Director()
{
	for(int i=0;i<2;i++)
	{
		if( m_aD[i] )
		{
			delete	m_aD[i];
			m_aD[i]	= NULL;
		}
	}
}
int		whtcpproxy_Director::SetListenDirection(int nIdx, const char *cszAddr)
{
	class	Director_Listen	: public Director
	{
	protected:
		SOCKET	m_sockListen;
		SOCKET	m_sockAccepted;
	public:
		Director_Listen(const char *cszAddr)
			: m_sockListen(INVALID_SOCKET)
			, m_sockAccepted(INVALID_SOCKET)
		{
			m_sockListen	= tcp_create_listen_socket_by_ipnportstr(cszAddr);
		}
		~Director_Listen()
		{
			cmn_safeclosesocket( m_sockListen );
		}
		virtual	int	WorkTick()
		{
			struct sockaddr_in	addr;
			int	rst	= tcp_accept_socket(m_sockListen, &m_sockAccepted, &addr, 0);
			if( rst<0 )
			{
				return	WORKRST_ERR;
			}
			if( rst>0 )
			{
				// 收到这个之后上层就可以GetSocket了
				return	WORKRST_OK;
			}
			return	WORKRST_WAIT;
		}
		virtual	SOCKET	GetSocket()
		{
			return	m_sockAccepted;
		}
	};
	Director	*&pD	= m_aD[nIdx];
	assert(pD==NULL);
	pD	= new Director_Listen(cszAddr);

	return	0;
}
inline SOCKET my_tcp_create_connecting_socket(const char *__ipportstr, port_t __localport)
{
	return	tcp_create_connecting_socket(__ipportstr, __localport);
}
int		whtcpproxy_Director::SetConnectDirection(int nIdx, const char *cszAddr)
{
	class	Director_Connect	: public whtcpproxy_Director::Director
	{
	protected:
		SOCKET	m_sock;
	public:
		Director_Connect(const char *cszAddr)
		{
			m_sock	= my_tcp_create_connecting_socket(cszAddr, 0);	// 直接调用tcp_create_connecting_socket就不行，会报（error C2326: 'n_whnet::whtcpproxy_Director::SetConnectDirection::Director_Connect::SetConnectDirection::Director_Connect(const char *)' : function cannot access 'n_whnet::tcp_create_connecting_socket'）
		}
		~Director_Connect()
		{
			//cmn_safeclosesocket(m_sock);	这个不用关闭了，因为要给上层用
		}
		virtual	int	WorkTick()
		{
			switch( tcp_check_connecting_socket(m_sock) )
			{
			case	CONNECTING_SOCKET_CHKRST_CONNTECTING:
				return	WORKRST_WAIT;
			case	CONNECTING_SOCKET_CHKRST_OK:
				return	WORKRST_OK;
			default:
				return	WORKRST_ERR;
			}
		}
		virtual	SOCKET	GetSocket()
		{
			return	m_sock;
		}
	};
	Director	*&pD	= m_aD[nIdx];
	assert(pD==NULL);
	pD	= new Director_Connect(cszAddr);
	return	0;
}
int		whtcpproxy_Director::DoTick()
{
	bool	bAllOver	= true;
	int		i;
	for(i=0;i<2;i++)
	{
		Director	*&pD	= m_aD[i];
		if( pD )
		{
			bAllOver	= false;
			int	rst	= pD->WorkTick();
			if( rst==Director::WORKRST_OK )
			{
				whtcpproxy::SetSocket(i, pD->GetSocket());
				delete	pD;
				pD		= NULL;
			}
		}
	}
	int	rst	= whtcpproxy::DoTick();
	if( bAllOver )
	{
		return	rst;
	}
	return	0;
}
