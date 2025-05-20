// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnettcp.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的TCP网络功能
//                备注tcp_set_socket_nodelay可以设置tcp为立即发送
// CreationDate : 2003-05-14
// ChangeLog    :
//                2004-02-04 MIN_TCPMSG_WHOLESIZE原来忘了减1了，幸亏刘毅发现了，呵呵。1字节的发送会出错。
//                2005-08-31 tcpmsger::INFO_T::bAutoStartThread的默认值改为false，即默认不打开线程
//                2007-06-08 把tcpmsger的数据长度表示单元改为int，同时把默认的发送和接收缓冲区长度改为512K。取消了tcpbigmsger。
//                2007-08-10 把tcpmsger的bKeepAlive默认改为false
//                2008-05-23 取消tcpmsger的手动KeepAlive。就直接使用系统的tcp的KeepAlive就可以了

#ifndef	__WHNETTCP_H__
#define	__WHNETTCP_H__

#include "whnetcmn.h"
#include <WHCMN/inc/whcmn_def.h>
#include <WHCMN/inc/whstreambuffer.h>
#include <WHCMN/inc/whthread.h>
#include <WHCMN/inc/whstatusworker.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <assert.h>

using n_whcmn::whstatusholder;
using n_whcmn::whstatusworker;
using n_whcmn::whvector;

namespace n_whnet
{

enum
{
	TCP_SOMAXCONN		= WHNET_MAX_SOCKTOSELECT	,
};

// 只是创建一个和端口绑定的socket (bind to ADDRESSANY)
SOCKET	tcp_create_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr=NULL);
// (bind to fixed address)
SOCKET	tcp_create_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr=NULL);
// 监听本机端口的连接，最多同时处理的连接数为__maxconn
SOCKET	tcp_create_listen_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr=NULL, int __maxconn = TCP_SOMAXCONN);
SOCKET	tcp_create_listen_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr=NULL, int __maxconn = TCP_SOMAXCONN);
SOCKET	tcp_create_listen_socket_by_ipnportstr(const char *__ipnport, struct sockaddr_in *__addr=NULL, int __maxconn = TCP_SOMAXCONN);
// (阻塞式)连接服务器，本地端口为__localport
// __timeout为连接超时(单位毫秒)，默认10秒钟超时
SOCKET	tcp_create_connect_socket(struct sockaddr_in *__dstaddr, port_t __localport, int __timeout);
SOCKET	tcp_create_connect_socket(const char *__ip, port_t __port, port_t __localport, int __timeout);
SOCKET	tcp_create_connect_socket_by_ipnportstr(const char *__ipnport, port_t __localport, int __timeout);
// 没有超时判断的连接(阻塞到连接成功或系统超时)
SOCKET	tcp_create_connect_socket_block(const char *__ip, port_t __port, port_t __localport);
// 创建一个noblock连接
SOCKET	tcp_create_connecting_socket(const char *__ip, port_t __port, port_t __localport);
SOCKET	tcp_create_connecting_socket(const char *__ipportstr, port_t __localport);
SOCKET	tcp_create_connecting_socket(const struct sockaddr_in *__addr, port_t __localport);
// 检查noblock连接是否已经连接上了(当连接成功，里面会自动把socket设置回block模式，调用者应该记住不能再次check了，否则不知会是什么结果，反正我没试过)
enum
{
	CONNECTING_SOCKET_CHKRST_ERR			= -1,		// 连接出错
	CONNECTING_SOCKET_CHKRST_OK				= 0,		// 连接上了
	CONNECTING_SOCKET_CHKRST_CONNTECTING	= 1,		// 连接过程中
};
int		tcp_check_connecting_socket(SOCKET __sock, int __interval = 0);
// 试图接纳一个连接
// __addrlen	(In/Out)表示地址缓冲区的传入大小和最后获得的大小
// 返回(和select_rd一致)
//      >0				表示接收了连接
//      0				表示没有接收连接
//      SOCKET_ERROR	表示socket错误
int		tcp_accept_socket(SOCKET __listensock, SOCKET *__insock, struct sockaddr *__inaddr, socklen_t *__addrlen, int __timeout);
int		tcp_accept_socket(SOCKET __listensock, SOCKET *__insock, struct sockaddr_in *__inaddr, int __timeout);
// 设置socket为TCP_NODELAY
int		tcp_set_socket_nodelay(SOCKET __sock, int __val);

// 由于微软的recv有bug，__size过大时会报错，所以需要这样替代一下
#ifdef	WIN32
inline int	tcp_saferecv(SOCKET __sock, char *__data, int __size)
{
	// 2004-09-11 改为1M，原来10M可能都有些大
	const int	CNMAXRECV	= 1000000;
	if( __size>CNMAXRECV )
	{
		__size	= CNMAXRECV;
	}
	return	::recv(__sock, __data, __size, 0);
}
#endif
// Linux则不用，直接recv即可
#ifdef	__GNUC__
inline int	tcp_saferecv(SOCKET __sock, char *__data, int __size)
{
	return	::recv(__sock, __data, __size, 0);
}
#endif

// 非常保险的tcp关闭(这样基本上应该可以没有TIMEWAIT)
int	tcp_verysafeclose(SOCKET &__sock);

////////////////////////////////////////////////
// 基于tcp的消息发送和接收
////////////////////////////////////////////////
typedef unsigned short		httag_t;			// head/tail tag
typedef unsigned long		msgsize_t;

#pragma pack(1)
struct	tcpmsg_head_t
{
	httag_t		nBegin;
	msgsize_t	nSize;
	char		Data[1];
	inline msgsize_t	GetSize(bool bNBO) const
	{
		return	bNBO ? ntohl(nSize) : nSize;
	}
};
struct	tcpmsg_tail_t
{
	httag_t		nEnd;
};
#pragma pack()
enum
{
	MAX_TCPMSG_SIZE			= 100000000,				// 最大的消息尺寸
	MIN_TCPMSG_WHOLESIZE	= sizeof(tcpmsg_head_t)-1 + sizeof(tcpmsg_tail_t),
													// 消息包的最小尺寸
};
inline size_t	tcpmsg_getwholesize(size_t nSize)
{
	return	sizeof(tcpmsg_head_t) - 1 + nSize + sizeof(tcpmsg_tail_t);
}
inline size_t	tcpmsg_getwholesize(tcpmsg_head_t *pMsg, bool bNBO)
{
	return	tcpmsg_getwholesize(pMsg->GetSize(bNBO));
}
inline tcpmsg_tail_t *	tcpmsg_gettail(tcpmsg_head_t *pMsg, bool bNBO)
{
	return	(tcpmsg_tail_t *)(pMsg->Data + pMsg->GetSize(bNBO));
}

class	tcpmsger
{
public:
	enum
	{
		STATUS_ERROR			= -1,		// 一般是SOCKET出错
		STATUS_NOTHING			= 0,
		STATUS_WORKING			= 1
	};
	struct	INFO_T
	{
		SOCKET	sock;						// 这个由外界创建，切记不要忘了设置!!!!
		int		nInterval;					// 每轮检查的最长时间(这个只有在使用线程时才需要!)
		size_t	nTrySendSize;				// 当发送缓冲超过这个值的时候就尝试发送一次数据
		size_t	nSendBufSize;				// 发送缓冲（逻辑上的）
		size_t	nRecvBufSize;				// 接收缓冲（逻辑上的）
		size_t	nSockSndBufSize;			// 发送缓冲（TCP底层的）
		size_t	nSockRcvBufSize;			// 接收缓冲（TCP底层的）
		httag_t	nBegin, nEnd;				// 包的前后标志数据
		bool	bAutoStartThread;			// 自动打开工作线程
											// (默认false。在多个socket处理的情况下一般为false)
		bool	bLogSendRecv;				// 记录发送日志
		bool	bNonBlockSocket;			// 是否设置为非阻塞socket
		int		keepalive_idle;				// 网络底层的keepalive起始时间
		int		keepalive_interval;			// 网络底层的keepalive间隔
		INFO_T()
		: sock(INVALID_SOCKET), nInterval(50)
		, nTrySendSize(256*1024)
		, nSendBufSize(512*1024), nRecvBufSize(512*1024)
		, nSockSndBufSize(0), nSockRcvBufSize(0)	// 写0就代表直接使用系统默认的缓冲大小
		, nBegin(0x55AA), nEnd(0xAA55)
		, bAutoStartThread(false)
		, bLogSendRecv(false)
		, bNonBlockSocket(true)
		, keepalive_idle(30000), keepalive_interval(5000)
		{ }	
	};
	// 这个为了可以通过配置文件自动填充
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
protected:
	INFO_T	m_info;
	n_whcmn::whstreambuffer	m_bufSend;			// 输出的缓冲
	n_whcmn::whstreambuffer	m_bufRecv;			// 输入的缓冲
	n_whcmn::whlock			m_lkSend, m_lkRecv;	// 对于输入输出队列的锁
	n_whcmn::whtid_t		m_tid;				// 工作线程的id
	int		m_nStatus;							// 当前状态
	n_whcmn::whtick_t		m_lastrecvtime;		// 最后一次收到数据的时刻
	n_whcmn::whtick_t		m_lastsendtime;		// 最后一次发送数据的时刻
	bool	m_bStop;							// 是否停止线程
	bool	m_bUseLock;							// 是否使用锁互斥
	bool	m_bNBO;								// 是否需转换网络字节序(NetByteOrder)，这个起始只是tcpmsg_head_t中的nSize需要转换
	n_whcmn::whtimequeue	m_TQ;				// 时间队列
	struct	TQUNIT_T
	{
		typedef	void (tcpmsger::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T			tefunc;
	};
protected:
	tcpmsg_head_t *	GetAvailMsgHead();
												// 如后有完整的消息可以获得，获得其头。里面没有锁，需要外面加锁。
												// *pnReason中返回没有得到的原因
	int	CleanRecvData();						// 清理出错的数据。清理到下一个头标志。
												// 这个在收到错误数据的情况下用，将数据清理到下一个起始标志
	int	_SendMsgFillData(tcpmsg_head_t *pHead, const void *pData, size_t nSize);
												// 在SendMsg中使用，把数据填写到pHead后的缓冲区中去
public:
	inline INFO_T *	GetInfo()
	{
		return	&m_info;
	}
	inline int	GetStatus() const
	{
		return	m_nStatus;
	}
	inline size_t	GetRecvedSize() const		// 获得已经收到的数据长度
	{
		return	m_bufRecv.GetSize();
	}
	inline n_whcmn::whtick_t	GetLastRecvTime() const
	{
		return	m_lastrecvtime;
	}
	inline void	SetLastRecvTime(n_whcmn::whtick_t t)
	{
		m_lastrecvtime	= t;;
	}
	inline void		SetNBO(bool bSet)
	{
		m_bNBO	= bSet;
	}
	// 下面带"_"的lock和unlock是可以外界手动调用的，一般给PeekMsg和FreeMsg用
	inline void	_sendlock()
	{
		m_lkSend.lock();
	}
	inline void	sendlock()
	{
		if( m_bUseLock )
		{
			_sendlock();
		}
	}
	inline void	_sendunlock()
	{
		m_lkSend.unlock();
	}
	inline void	sendunlock()
	{
		if( m_bUseLock )
		{
			_sendunlock();
		}
	}
	inline void	_recvlock()
	{
		m_lkRecv.lock();
	}
	inline void	recvlock()
	{
		if( m_bUseLock )
		{
			_recvlock();
		}
	}
	inline void	_recvunlock()
	{
		m_lkRecv.unlock();
	}
	inline void	recvunlock()
	{
		if( m_bUseLock )
		{
			_recvunlock();
		}
	}
	inline n_whcmn::whstreambuffer &	getbufSend()
	{
		return	m_bufSend;
	}
	inline n_whcmn::whstreambuffer &	getbufRecv()
	{
		return	m_bufRecv;
	}
public:
	tcpmsger();
	virtual	~tcpmsger();
	int	Init(INFO_T *pInfo);					// 初始化并
	int	Release();								// 终结并结束工作线程(并且会关闭通讯用的socket)
	int	StartWorkThread();						// 开始工作线程
	int	StopWorkThread();						// 结束工作线程
	int	SendMsg(const void *pData, size_t nSize);
												// 返回0表示成功，-1表示出错
	int	RecvMsg(void *pData, size_t *pnSize);	// 返回1表示有数据，0表示没数据，-1表示出错
	// !!!! 注意：如果m_bNBO为真，则PeekMsg获取的数据内部可能还是没有转换为本机字节序的数据，慎用 !!!!
	const void *	PeekMsg(size_t *pnSize);	// 获取数据指针，但是不释放
												// 注意Peek和Free内部都没有加锁定，所以用于多线程时需要外部加锁
	void	FreeMsg();							// 释放最新的一条消息
	size_t	GetNextSizeToRecv();				// 获得可以收取的下一个消息的大小
												// 返回正数表示有数据
												// 返回0表示没有
	int	WorkThread();							// 线程调用的
	// 如果不使用线程则需要手动调用下面的函数
	// 比如
	// ManualRecv
	// RecvMsg直到收完
	// ...
	// SendMsg
	// ManualSend
	int	ManualRecv(int nInterval = 0);
	int	ManualSend(int nInterval = 0);
	// 上面两个可以合并为
	int	ManualRecvAndSend(int nInterval = 0);

	// 设置状态
	void	SetStatus(int nStatus);
};

// 基于tcp的消息接收/发送
// 只能单线程使用!!!!
class	tcpmsgerbase
{
public:
	// 状态
	enum
	{
		STATUS_ERROR		= -1,				// 一般是SOCKET出错
		STATUS_NOTHING		= 0,
		STATUS_WORKING		= 1
	};
	struct	INFO_T
	{
		SOCKET	sock;							// 这个由外界创建，切记不要忘了设置!!!!
		int		nInterval;						// 每轮检查的最长时间(这个只有在使用线程时才需要!)
		int		nSendBufSize;					// 发送缓冲
		int		nRecvBufSize;					// 接收缓冲
		bool	bAutoCloseSocket;				// 在Release时自动关闭socket
		bool	bAutoStartThread;				// 自动打开工作线程(这个是为了兼容性考虑的)
		INFO_T()
		: sock(INVALID_SOCKET), nInterval(50)
		, nSendBufSize(65536), nRecvBufSize(65536)
		, bAutoCloseSocket(true)
		, bAutoStartThread(false)
		{ }
	};
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
protected:
	int						m_nStatus;
	INFO_T					m_info;
	n_whcmn::whstreambuffer	m_bufSend;			// 输出的缓冲
	n_whcmn::whstreambuffer	m_bufRecv;			// 输入的缓冲
	n_whcmn::whtick_t		m_lastrecvtime;		// 最后一次收到数据的时刻
	whvector<int>			m_queueLen;			// 每段Unit的长度（能放到这里就说明长度是足够正确的）
	int						m_nCheckedLen;		// 已经检查过OK的长度
public:
	inline const INFO_T *	GetInfo() const
	{
		return	&m_info;
	}
	inline int	GetStatus() const
	{
		return	m_nStatus;
	}
	inline int	GetRecvedSize() const			// 获得已经收到的数据长度
	{
		return	m_bufRecv.GetSize();
	}
	inline n_whcmn::whtick_t	GetLastRecvTime() const
	{
		return	m_lastrecvtime;
	}
	inline void	SetLastRecvTime(n_whcmn::whtick_t t)
	{
		m_lastrecvtime	= t;;
	}
public:
	tcpmsgerbase();
	virtual	~tcpmsgerbase();
	// 注意：必须手动调用Release，因为这里用虚析构，所以为了清楚不要指望它自动调用Release
	int	Init(INFO_T *pInfo);					// 初始化
	int	Release();								// 终结(并且会关闭通讯用的socket)
	int	SendMsg(const void *pData, size_t nSize);
												// 返回0表示成功，-1表示出错
	int	RecvMsg(void *pData, size_t *pnSize);	// 返回1表示有数据，0表示没数据，-1表示出错
	const void *	PeekMsg(size_t *pnSize);	// 获取数据指针，但是不释放
												// 注意Peek和Free内部都没有加锁定，所以用于多线程时需要外部加锁
	void	FreeMsg();							// 释放最新的一条消息
	int	ManualRecv(int nInterval = 0);
	int	ManualSend(int nInterval = 0);
	// 上面两个可以合并为
	int	ManualRecvAndSend(int nInterval = 0);
	// 设置状态
	void	SetStatus(int nStatus);
private:
	int	ManualRecv_AnalyzeRecvData();
protected:
	enum
	{
		CHECKUNIT_RST_ERR	= -1,				// 发现数据错误
		CHECKUNIT_RST_NA	= 0,				// 没有发现单元
		CHECKUNIT_RST_OK	= 1,				// 已经找到了一个单元
	};
private:
	// 检查从pBegin开始的数据是否含有完整的数据单元
	// 注意，一定要判断数据长度是否符合最小单元标准（比如开头一个字节为长度，那就要判断nSize是否不小于1，然后才能取第一个字节，判断长度）
	// pBegin		是数据开始
	// nSize		是数据总长
	// *pnUnitLen	如果找到一个单元则这里面是单元的总长度(从pBegin开始)
	//				如果发现数据错误，则这里面是没错的开始点位置(也就是出错数据的总长度)
	// 返回值：		CHECKUNIT_RST_XXX
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen)
												= 0;
	// 获得打包后的新长度
	virtual size_t	GetPackUnitSize(size_t nDSize)
												= 0;
	// 将一段数据打包成适合通讯的格式(pDstBuf应该实现已经分配好了，尺寸至少是GetPackUnitSize获得的长度)
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf)
												= 0;
	// 从打包数据中找到数据指针，并返回数据长度（注nTotalSize是这个unit打包后的总长度，不多不少的）
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize)
												= 0;
};
// 以"\n"为行结尾，即整个包打包后增加一个字节
class	tcpmsger_cmnline		:	public tcpmsgerbase
{
private:
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen);
	virtual size_t	GetPackUnitSize(size_t nDSize);
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf);
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize);
};
// 以双字节开头代表包总长度(也就是说包长度不能超过65536)
class	tcpmsger_smallpacket	:	public tcpmsgerbase
{
public:
	// 这个是包最开头表示整个包长度的
	typedef	unsigned short		mysize_t;
private:
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen);
	virtual size_t	GetPackUnitSize(size_t nDSize);
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf);
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize);
};
// 用变长的长度vn2表示长度
class	tcpmsger_vn2_packet	:	public tcpmsgerbase
{
private:
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen);
	virtual size_t	GetPackUnitSize(size_t nDSize);
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf);
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize);
};
// 发送接收都是固定长度的（kr竟然喜欢用这样的通讯方式:(）
class	tcpmsger_fixedsize	:	public tcpmsgerbase
{
private:
	int		m_nPacketSize;
public:
	tcpmsger_fixedsize()
		: m_nPacketSize(0)
	{
	}
	inline	void	SetPacketSize(int nPacketSize)
	{
		m_nPacketSize	= nPacketSize;
	}
private:
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen);
	virtual size_t	GetPackUnitSize(size_t nDSize);
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf);
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize);
};

// 重连尝试器
// 注意：这里面的sock不能用select_rd来判断是否可用，而需要用select_wr来判断，所以不能那出去一起做select_rd!!!!
class	tcpretryer
{
public:
	enum
	{
		MAX_DSTADDRNUM					= 16,	// 最多可以尝试连接的地址个数
	};
	enum
	{
		STATUS_NOTHING					= 0,	// 什么也不作，可能刚刚开始工作，或者决定不再工作了
		STATUS_TRYCONNECT				= 1,	// 尝试连接目标
		STATUS_WORK						= 2,	// 正常的工作
		STATUS_SLEEP					= 3,	// 休息
		STATUS_NEVERTRY					= 10,	// 永远不用try了
	};
	struct	DSTADDR_T
	{
		int		nCanTryCount;					// 可以被尝试的次数
		char	szAddr[WHNET_MAXADDRSTRLEN];	// 地址字符串
	};
protected:
	int		m_nStatus;							// 状态
	SOCKET	m_sock;								// 如果连接成功，socket在这里
	int		m_nTotalRetry;						// 设置总共重试的次数(<=0表示无限次重试)
	int		m_nRetryCount;						// 目前已经重试的次数
	int		m_nRetryTimeOut;					// 重连的超时(单位：毫秒，默认值见tcpretryer::tcpretryer()中的赋值)
	n_whcmn::whtick_t	m_nRetryStartTime;		// 重连开始的时刻时间
	int		m_nSleepTime;						// 休息的时间(单位：毫秒，默认值见tcpretryer::tcpretryer()中的赋值)
	n_whcmn::whtick_t	m_nSleepStartTime;		// 休息开始的时间
	// 备用地址相关
	DSTADDR_T	m_DstAddrs[MAX_DSTADDRNUM];		// 目标地址单元数组
	DSTADDR_T	*m_pCurDstAddr;					// 当前的地址单元
	int		m_nDstAddrNum;						// 目前的备用地址总数
	int		m_nCurDstAdrrIdx;					// 当前地址使用的地址索引
	int		m_nCurTryCount;						// 当前地址被尝试的次数
public:
	inline bool		Failed() const				// 多次连接都失败了，并且不能再连接了
	{
		return	(m_nStatus == STATUS_NOTHING) && (!ShouldRetry());
	}
	inline SOCKET	PickSocketOut()				// 获得连接好的socket，所以需要在STATUS_WORK的状态下调用
	{
		SOCKET	sock = m_sock;
		m_sock	= INVALID_SOCKET;
		return	sock;
	}
	inline int		GetStatus() const			// 获得当前状态
	{
		return	m_nStatus;
	}
	// 获得当前需要连接的地址
	inline const char *	GetCurDstAddr() const
	{
		assert(m_pCurDstAddr);
		return	m_pCurDstAddr->szAddr;
	}
	inline void		SetTotalRetry(int nTotal)
	{
		m_nTotalRetry	= nTotal;
	}
	inline void		SetRetryTimeOut(int nTimeOut)
	{
		m_nRetryTimeOut	= nTimeOut;				// 单位：毫秒
	}
	inline void		SetSleepTime(int nSleepTime)
	{
		m_nSleepTime	= nSleepTime;			// 单位：毫秒
	}
	inline int		GetRetryCount() const
	{
		return	m_nRetryCount;
	}
public:
	tcpretryer();
	~tcpretryer();
	void	ClrAllDstAddr();					// 清除已经设置的所有目标地址
	int		AddDstAddr(const char *szDstAddr, int nCanTryCount);
												// 添加目标地址。nCanTryCount表示这个地址可以连续尝试的次数(这个在有多个目标地址的时候才有用)。
	void	StartWork(bool bSleepFirst=false);	// 开始工作(这之后就开始重连，直到连接上或者超过次数为止)
												// 如果bSleepFirst为真则先开始睡
	int		DoSelect(int nInterval);			// 查看socket是否有数据
	void	Work();								// 做一次工作(这里面会通过tcp_check_connecting_socket来判断是否连接成功)
private:
	bool	ShouldRetry() const;				// 判断是否应该重试
	void	BeginSleep();						// 开始等待一段时间
	void	DoSleep();							// 等待的内容
	void	BeginRetry();						// 开始尝试连接
	void	DoRetry();							// 尝试的内容
	void	SwitchNextDst();					// 转换为下一个尝试地址
};

// 具有retry功能的msger
// 注意，这个只能和控制在一个线程中，即不能使用bAutoStartThread
// 所以，也不需要使用select功能，需要外界控制调用tick的频率
// 可用的方法除了_TyMsger，tcpretryer的方法也可以随便用
// 如：SetRetryTimeOut、SetSleepTime、SetTotalRetry等，不过记住：它们必须在Init之前调用，因为Init中就已经有连接动作了
// 还有：SendMsg、RecvMsg、PeekMsg、FreeMsg等等都可以在同一线程使用
template<class _TyMsger>
class	tcpretrymsger	: public _TyMsger, public tcpretryer, whstatusholder
{
public:
	enum
	{
		STATUS_NOTHING		= 0,
		STATUS_TRYCONNECT	= 1,
		STATUS_WORKING		= 2,
	};
public:
	bool				m_bSleepFirst;
	// 设为public的原因是为了可以外界也直接使用SetStatus
	whstatusworker		m_statusTRYCONNECT;
	whstatusworker		m_statusWORKING;
private:
	typename _TyMsger::INFO_T	m_msgerinfo;			// 存储初始化时的
	// 为下面用
	WHSH_DECLARETHISCLASS(tcpretrymsger<_TyMsger>);
private:
	void	Worker_TRYCONNECT_Begin()
	{
		// 关闭旧连接
		_TyMsger::Release();
		// 开始重试
		tcpretryer::StartWork(m_bSleepFirst);
		// 附加工作
		i_Worker_TRYCONNECT_Begin();
	}
	void	Worker_TRYCONNECT_Work()
	{
		tcpretryer::Work();
		if( tcpretryer::GetStatus()==tcpretryer::STATUS_WORK )
		{
			m_msgerinfo.sock				= tcpretryer::PickSocketOut();
			m_msgerinfo.bAutoStartThread	= false;
			if( _TyMsger::Init(&m_msgerinfo)<0 )
			{
				// 这个没办法了
				assert(0);
			}
			else
			{
				whstatusholder::SetStatus(m_statusWORKING);
			}
		}
		// 附加工作
		i_Worker_TRYCONNECT_Work();
	}
	void	Worker_WORKING_Begin()
	{
		i_Worker_WORKING_Begin();
	}
	void	Worker_WORKING_Work()
	{
		//
		_TyMsger::ManualRecv();
		// 判断是否断连了
		if( _TyMsger::GetStatus() != _TyMsger::STATUS_WORKING )
		{
			whstatusholder::SetStatus(m_statusTRYCONNECT);
			return;
		}
		// 如果有必要里面i_Worker_WORKING_Work可以多掉几次ManualSend
		i_Worker_WORKING_Work();
		//
		_TyMsger::ManualSend();
	}
public:
	tcpretrymsger()
	: m_bSleepFirst(false)
	, m_statusTRYCONNECT(STATUS_TRYCONNECT)
	, m_statusWORKING(STATUS_WORKING)
	{
		m_statusTRYCONNECT.SetBeginFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_TRYCONNECT_Begin));
		m_statusTRYCONNECT.SetWorkFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_TRYCONNECT_Work));
		m_statusWORKING.SetBeginFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_WORKING_Begin));
		m_statusWORKING.SetWorkFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_WORKING_Work));
	}
	// 这个只需要一个目标地址
	int		Init(typename _TyMsger::INFO_T *pInfo, const char *pcszDstAddrs)
	{
		// 这里仅仅是一个拷贝过程
		memcpy(&m_msgerinfo, pInfo, sizeof(m_msgerinfo));
		tcpretryer::ClrAllDstAddr();
		tcpretryer::AddDstAddr(pcszDstAddrs, 0);
		// 开始连接
		whstatusholder::SetStatus(m_statusTRYCONNECT);
		return	0;
	}

	// Tick中完成状态转换(重连/数据交换)
	int		Tick()
	{
		whstatusholder::DoWork();
		return	0;
	}
	// 状态就是上面定义的STATUS_XXX
	inline int	GetStatus() const
	{
		return	whstatusholder::GetStatus();
	}
	inline void	SetStatus(whstatusworker &worker)
	{
		whstatusholder::SetStatus(worker);
	}
	// 获取当前正在使用的socket，如果正在重连则返回INVALID_SOCKET
	SOCKET	GetSocket()
	{
		if( _TyMsger::GetStatus() == _TyMsger::STATUS_WORKING )
		{
			return	_TyMsger::GetInfo()->sock;
		}
		return		INVALID_SOCKET;
	}
	// 应该调用子类的这些(子类可以通过这些附加新功能)
	virtual void	i_Worker_TRYCONNECT_Begin()	{}
	virtual void	i_Worker_TRYCONNECT_Work()	{}
	virtual void	i_Worker_WORKING_Begin()	{}
	virtual void	i_Worker_WORKING_Work()		{}
};

// 简单的tcpproxy，在两个IP间进行数据交换，可以起多个连接
// 使用流程
//	tcpproxy				tp;
//	tcpproxy::INFO_T		tpinfo;
//	tpinfo.cszDstAddr		= pszDstAddr;
//	tpinfo.cszListenAddr	= szListenAddr;
//	tp.Init(&tpinfo);
//	while(!g_bStop)
//	{
//		tp.DoSelect(10);
//		tp.DoTick();
//		... ...
//		这时在外面可以进行tp.SetByteRate和tp.SetDst的操作
//	}
//	tp.Release();
class	tcpproxy
{
public:
	struct	INFO_T
	{
		const char	*cszListenAddr;				// 监听地址
		const char	*cszDstAddr;				// 目标地址
		int			nMaxConnection;				// 最多可以承受的连接数
		int			nMaxBufLen;					// 每次读取最多读入的数据长度
		int			nConnectTimeout;			// 连接目标地址的超时
		int			nBRateUp, nBRateDown;		// 上下行的传输率(字节/秒)
												// 0表示不限制
		int			nDelayQueueSize;

		INFO_T()
		: cszListenAddr(NULL)
		, cszDstAddr(NULL)
		, nMaxConnection(20)					// 如果同时并发的连接数比较大(比如浏览器)，则这个需要设置高一些
		, nMaxBufLen(4096)
		, nConnectTimeout(5000)
		, nBRateUp(0), nBRateDown(0)
		, nDelayQueueSize(4*1024*1024)			// 默认个4M的吧
		{
		}
	};
	struct	UNIT_T
	{
		SOCKET		sockFrom;					// 连接发起方(连入)
		SOCKET		sockTo;						// 连接目标(连出)
		// 延迟发送的计数
		int			nUpCount;					// 来自发起方的
		int			nDownCount;					// 来自目标方的
		// 下面几个用作流量控制
		n_whcmn::whtick_t	nFromReadNextTime;	// 下次可以从From读出数据的时间
		n_whcmn::whtick_t	nToReadNextTime;	// 下次可以从To读出数据的时间
		void	clear()
		{
			memset(this, 0, sizeof(*this));
			sockFrom	= INVALID_SOCKET;
			sockTo		= INVALID_SOCKET;
		}
		inline bool	IsTimeToReadFrom(n_whcmn::whtick_t nNow)
		{
			return	n_whcmn::wh_tickcount_diff(nNow, nFromReadNextTime) >= 0;
		}
		inline bool	IsTimeToReadTo(n_whcmn::whtick_t nNow)
		{
			return	n_whcmn::wh_tickcount_diff(nNow, nToReadNextTime) >= 0;
		}
	};
	struct	DELAY_T
	{
		int						nUnitID;		// 对应的转发UNIT的ID
		n_whcmn::whtick_t		nSendTime;		// 需要发送的时刻
	};	// 这个结构的后面就跟着数据部分
protected:
	INFO_T			m_info;
	struct sockaddr_in	m_dstaddr;				// 目标地址
	n_whcmn::WHRANGE_T<int>		m_rgDelayUp;	// 延迟范围(对于TCP来说其实应该是两个相邻包的间隔)
	n_whcmn::WHRANGE_T<int>		m_rgDelayDown;	// 
	cmn_selector	m_selector;
	SOCKET			m_sockListen;				// 用于监听的socket
	n_whcmn::whunitallocatorFixed<UNIT_T>		m_Units;
	whvector<int>								m_vectUnitIDToDel;
	whvector<char>								m_vectBuf;
	n_whcmn::whsmpqueue							m_queueDelayUp;		// 用于上行延迟（从来源发向目标）发送的队列
	n_whcmn::whsmpqueue							m_queueDelayDown;	// 用于下行延迟（从目标返回来源）发送的队列
	n_whcmn::whtick_t			m_tickNow;
	struct	STAT_T
	{
		int			nMaxConcurrentConnection;	// 同时并发的最大连接数
		STAT_T()
		{
			WHMEMSET0THIS();
		}
	}				m_stat;
public:
	tcpproxy();
	~tcpproxy();
	int		Init(INFO_T *pInfo, struct sockaddr_in *pListenAddr=NULL);
	int		Release();
	void	SetDst(const char *cszDstAddr);		// 设置目标地址（可以在运行中改变目标地址）
	void	SetDelay(int nUpMin, int nUpMax, int nDownMin, int nDownMax);
												// 设置延迟的时间范围（毫秒），如果都设0则表示不延迟。
	void	SetByteRate(int nUp, int nDown);	// 多少字节/秒，如果设0则表示不限制。（上下行速率都会设置）
	int		DoSelect(int nInterval);			// select所有的socket
	int		DoTick();							// 进行一次发送接收动作（以及内部逻辑）
	const char *	ShowInfo(char *pszBuf);		// 把信息打印到Buffer中
private:
	int		CheckRecvThenSend(int nUnitID, SOCKET sockIn, SOCKET sockOut, int nBRate, n_whcmn::whtick_t &nNowAndThen, n_whcmn::WHRANGE_T<int> &WR, n_whcmn::whsmpqueue &queueDelay, int &nCount);
};

// 上面的封装了线程的对象(With Thread)
// 使用流程
//	tcpproxyWT				tp;
//	tcpproxyWT::INFO_T		tpinfo;
//	tpinfo.cszDstAddr		= pszDstAddr;
//	tpinfo.cszListenAddr	= szListenAddr;
//	tp.Init(&tpinfo);
//	tp.StartThread();
//
//	这时在外面可以进行tp.SetByteRate和tp.SetDst的操作
//
//	tp.StopThread();
//	tp.Release();

class	tcpproxyWT	: public tcpproxy
{
public:
	n_whcmn::whtid_t	m_tid;
	bool				m_bStop;
public:
	tcpproxyWT();
	~tcpproxyWT();
	// nPriority的定义参见：whthread.h中的THREAD_PRIORIY_XXX，-1表示默认优先级
	int		StartThread(int nPriority=-1);
	int		StopThread();
};

// 更通用一些的代理的结构
class	whtcpproxy
{
protected:
	SOCKET				m_sock[2];
	whvector<char>		m_vect[2];
public:
	whtcpproxy();
	virtual ~whtcpproxy();
	// 设置某个socket，如果另外一个还没有设置（即另一个是INVALID_SOCKET），则这个收到的东西都会先累积起来
	void	SetSocket(int nIdx, SOCKET sock);
	// 获得当前可用的socket加入vect中
	void	GetSockets(whvector<SOCKET> &vect);
	int		DoTick();							// 进行一次发送接收动作（以及内部逻辑）
private:
	void	FailSocket(int nIdx);
	virtual void	OnSocketFail(int nIdx)	{}
};
class	whtcpproxy_Director	: public whtcpproxy
{
public:
	class	Director
	{
	public:
		virtual	~Director()					{}
		enum
		{
			WORKRST_OK			= 0,
			WORKRST_WAIT		= 1,
			WORKRST_ERR			= -1,
		};
		virtual	int	WorkTick()				= 0;
		virtual	SOCKET	GetSocket()			= 0;
	};
	Director	*m_aD[2];
public:
	whtcpproxy_Director();
	~whtcpproxy_Director();
	int		SetListenDirection(int nIdx, const char *cszAddr);
	int		SetConnectDirection(int nIdx, const char *cszAddr);
	int		DoTick();
};

template<class _Ty>
int	SafeSendMsg(_Ty *pMsger, const void *pData, size_t nSize)
{
	if( pMsger )
	{
		return	pMsger->SendMsg(pData, nSize);
	}
	return	-1;
}


}		// EOF namespace n_whnet

#endif	// EOF __WHNETTCP_H__
