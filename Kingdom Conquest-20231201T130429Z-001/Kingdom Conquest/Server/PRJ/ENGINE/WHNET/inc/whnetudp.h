// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File: whnetudp.h
// Creator: Wei Hua (魏华)
// Comment: 简单的UDP网络功能
// CreationDate: 2003-05-15

#ifndef	__WHNETUDP_H__
#define	__WHNETUDP_H__

#include "whnetcmn.h"
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whunitallocator.h>

namespace n_whnet
{

////////////////////////////////////////////////////////////////////
// 普通函数
////////////////////////////////////////////////////////////////////
// 只是创建一个和本地端口绑定的socket
// 如果__addr非空则返回绑定到的地址
SOCKET	udp_create_socket(port_t __localport, struct sockaddr_in *__addr = 0);
// 创建和本地某个ip和端口绑定的socket(__ip为空表示不特定绑定在某个网卡地址上)
SOCKET	udp_create_socket(const char *__ip, port_t __localport, struct sockaddr_in *__addr = 0);
// 创建socket随机绑定一定范围内的端口
SOCKET	udp_create_rand_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr = 0);
SOCKET	udp_create_rand_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr = 0);
// 通过简单地址结构创建udp
inline SOCKET	udp_create_socket_byeasyaddr(const EASYADDR *__eaddr, struct sockaddr_in *__addr = 0)
{
	return	udp_create_rand_socket(__eaddr->szAddrStr, __eaddr->port, __eaddr->port2, __addr);
}
SOCKET	udp_create_socket_by_ipnportstr(const char *__ipnportstr, struct sockaddr_in *__addr = 0);

// 创建用于本地通讯的socket管道
// 在不同线程中都可以通过send向其中发送，用recv从其中接收数据
// nRecvBufSize表示接收缓冲的大小，0表示使用默认的(Windows是9000字节，Linux是64k)
SOCKET	udp_create_lo(int __recvbufsize = 0);

// 把SOCKET LO当作一个事件来用
// 设置事件
bool	udp_socketevent_set(SOCKET __sock);
// 清除事件
bool	udp_socketevent_clr(SOCKET __sock);
// 检查是否有事件
bool	udp_socketevent_chk(SOCKET __sock);

// 不用管来源地址的recvfrom，返回::recvfrom的返回
int		udp_recv(SOCKET __sock, void *__buf, int __len);
int		udp_recv(SOCKET __sock, void *__buf, int __len, struct sockaddr_in *__addr);

// 简单的sendto
int		udp_sendto(SOCKET __sock, const void *__buf, int __len, const struct sockaddr_in *__addr);

// 为了给上两个设置丢包率
// 单位％（比如15.8，表示15.8%的包要丢）
void	udp_set_sendto_lostratio(float fLost);
float	udp_get_sendto_lostratio();
void	udp_set_recv_lostratio(float fLost);
float	udp_get_recv_lostratio();

////////////////////////////////////////////////////////////////////
// UDP，简单的udp socket创建和销毁器
////////////////////////////////////////////////////////////////////
class	UDP
{
protected:
	SOCKET	m_sock;
public:
	UDP(port_t nPort=0);
	UDP(const char *szIP, port_t nPort=0);
	~UDP();
	inline SOCKET	GetSock() const
	{
		return	m_sock;
	}
	inline int		SendTo(const char *pBuf, int nSize, const struct sockaddr_in *pAddr)
	{
		return	::sendto(m_sock, pBuf, nSize, 0, (struct sockaddr *)pAddr, sizeof(*pAddr));
	}
	inline int		RecvFrom(char *pBuf, int nSize, struct sockaddr_in *pAddr)
	{
		socklen_t	nLen = sizeof(*pAddr);
		return	::recvfrom(m_sock, pBuf, nSize, 0, (struct sockaddr *)pAddr, &nLen);
	}
	inline int		Select_RD(int nTimeOut)
	{
		return	cmn_select_rd(m_sock, nTimeOut);
	}
	inline int		Select_WR(int nTimeOut)
	{
		return	cmn_select_wr(m_sock, nTimeOut);
	}
};

// 基于UDP的通知器（利用localhost的UDP来模拟事件，类似于windows的Event的功能）
// 和前面的socketevent类似
class	UDPEvent
{
protected:
	SOCKET	m_sock;
public:
	inline SOCKET	GetSocket() const
	{
		return	m_sock;
	}
	UDPEvent();
	~UDPEvent();
	int	Init();
	int	Release();
	int	AddEvent(const void *pData="e", size_t nSize=1);
	int	GetEvent(void *pData, size_t *pnSize);
	int	ClrAllEvent();
};

////////////////////////////////////////////////////////////////////
// 日志记录器
////////////////////////////////////////////////////////////////////
// 旧日志记录器，为了让原来的程序都可以用，这里还保留它
class	UDPLogger
{
private:
	SOCKET	m_sock;
public:
	UDPLogger();
	~UDPLogger();
	// szParam格式
	// ip:port	比如："127.0.0.1:1976"
	int	Init(const char *szParam);
	int	Release();
	int	WriteBin(const char *szMsg, int nLen);
	int	Write(const char *szMsg);
	int	WriteFmt(const char *szFmt, ...);
};

////////////////////////////////////////////////////////////////////
// 转发器(用于模拟网络上的延时和丢包)
////////////////////////////////////////////////////////////////////
// 第一个连接自己的包带来的是源地址，以后就在目标地址和源地址间传递数据
// 上层使用方法
// ...
// select_rd(router->GetSocket(), 10);
// router->Tick();
// ...
class	UDPRouter
{
public:
	enum
	{
		MAX_PACKET_SIZE		= 1024,
		MAX_BUF_SIZE		= MAX_PACKET_SIZE * 2,
	};
	// 初始化参数结构
	struct	INFO_T
	{
		char	szDstIP[WHNET_MAXADDRSTRLEN];				// 目标地址
		port_t	nDstPort;									// 目标端口
		port_t	nLocalPort;									// 本地端口
		int		nMaxPacketNum;								// 可以累积的数据数量
		int		nMaxPacketSize;								// 单个包的最大尺寸
		int		nMinDelay, nMaxDelay;						// 延迟时间范围(毫秒)
		int		nLoss;										// 丢包率(千分之m_nLoss)
	};
	// 数据缓冲单元结构
	struct	PACKETUNIT_T
	{
		char	*szData;									// 指向申请的缓冲的指针
		int		nSize;										// 数据大小
		unsigned int	nTimeToSend;						// 超过这个时刻就发送
		int		nAddr;										// 将要发向的地址(就是下面enum的两个数)
	};
	enum
	{
		DSTADDR		= 0,
		SRCADDR		= 1,
	};
	enum
	{
		LOSSRANGE	= 1000,									// 说明nLoss的单位是千分之一
	};
private:
	SOCKET	m_sock;											// socket
	struct sockaddr_in	m_addr[2];							// 目标地址和来源地址
	bool	m_bVirgin;										// 还没有包来
	int		m_nMinDelay, m_nMaxDelay;						// 延迟时间范围(毫秒)
	int		m_nLoss;										// 丢包率(千分之m_nLoss)
	n_whcmn::whunitallocatorFixed<PACKETUNIT_T>	m_Units;	// 所有等待延时发送的包
	int		*m_pSendList;									// 在DoSend中使用的本次应该发送的包的序号表
	char	*m_tmpbuf;										// 用于临时获取收到数据的缓冲，给recvfrom用的
	int		m_tmpsize;										// m_tmpbuf所能容纳的最大数据长度
public:
	inline SOCKET	GetSocket() const
	{
		return	m_sock;
	}
	// 设置参数
	// 延时。在[nMin,nMax]毫秒之间
	inline void		SetDelay(int nMin, int nMax)
	{
		m_nMinDelay	= nMin;
		m_nMaxDelay	= nMax;
	}
	// 获得延时
	inline void		GetDelay(int *pnMin, int *pnMax)
	{
		*pnMin	= m_nMinDelay;
		*pnMax	= m_nMaxDelay;
	}
	// 丢包率。千分之nLoss。
	inline void		SetLoss(int nLoss)
	{
		m_nLoss	= nLoss;
	}
	// 获得丢包率
	inline int		GetLoss()
	{
		return	m_nLoss;
	}
public:
	UDPRouter();
	~UDPRouter();
	int		Init(INFO_T *pInfo);
	int		Release();
	// 一次运动，读入所有输入，转发给对方(已经延时或者丢弃了)
	int		Tick();
private:
	// 内部功能
	// 读入一个输入，判断要丢还是要延时。
	int		DoOneRecv();
	// 发送要延时的包。
	int		DoSend();
	// 随机，是否要丢包
	bool	ShouldItLost();
	// 生成一个随机延迟毫秒
	int		GenDelay();
};

////////////////////////////////////////////////////////////////////
// 基于地址的登记列表（比如用于游戏列表）
// 注意，NOOP一定要发的足够使网关的地址映射不改变
////////////////////////////////////////////////////////////////////
class	UDPRegLister
{
public:
	// 命令
	enum
	{
		CMD_NOOP					= 0,					// 保持连接
		CMD_REGIST					= 1,					// 申请注册
		CMD_UNREGIST				= 2,					// 申请注销
		CMD_REGIST_AGREE			= 11,					// 同意注册
		CMD_REGIST_REFUSE			= 12,					// 拒绝注册
		CMD_UNREGIST_OK				= 20,					// 注销完成
		CMD_DATA					= 80,					// 数据通知，由虚函数解释
	};
	// 用到的结构
	#pragma pack(1)
	struct	CMD_T
	{
		short	cmd;
		char	data[1];
	};
	#pragma pack()
	// 检查是否允许注册
	enum
	{
		CHECK_REG_RST_ERR			= -1,
		CHECK_REG_RST_OK			= 0,
	};
	// 初始化数据
	struct	INFO_T
	{
		SOCKET		sock;			// 监听socket
		int			nMaxRegNum;		// 最多可注册个数
		int			nMaxRegData;	// 注册数据的最大长度
		int			nMaxUnRegData;	// 注消数据的最大长度
		int			nMaxWorkData;	// 工作中通知数据的最大长度
		int			nDropTimeOut;	// 如果这么长时间没收到NOOP包就认为断线了(毫秒)
		INFO_T()
		: sock(INVALID_SOCKET)
		, nMaxRegNum(100), nMaxRegData(128), nMaxUnRegData(128), nMaxWorkData(128)
		, nDropTimeOut(60000)		// (默认60秒)
		{ }
	};
	// 单元结构
	struct	UNIT_T
	{
		enum
		{
			STATUS_DROPPED		= -1,
			STATUS_NOTHING		= 0,
			STATUS_WORKING		= 1,
		};
		int						nStatus;
		unsigned int			nCreateTime;
		unsigned int			nLastRecvTime;
		struct sockaddr_in		addr;
		n_whcmn::whvector<char>	regData;
		n_whcmn::whvector<char>	unregData;
		n_whcmn::whvector<char>	workData;		// 工作中的信息
		UNIT_T()
		{
			clear();
		}
		void	clear()
		{
			nStatus			= 0;
			nCreateTime		= 0;
			nLastRecvTime	= 0;
			// destroy可以节约内存
			regData.destroy();
			unregData.destroy();
			workData.destroy();
		}
	};
private:
	// 检查注册数据是否正确
	virtual int	CheckRegist(char *szData, int nSize);
	// 在删除一个单元前应该作的事情
	virtual int	BeforeDeleteUnit(int nID);
	// 在每次NOOP需要做的事
	virtual int	NoopJob(int nID);
	// 发来的数据
	virtual int	DataJob(int nID);
protected:
	INFO_T	m_info;
	n_whcmn::whunitallocatorFixed<UNIT_T>							m_Units;
	n_whcmn::whhash<struct sockaddr_in, int, n_whcmn::whcmnallocationobj, _whnet_addr_hashfunc>	m_mapAddr2ID;
	n_whcmn::whvector<int>											m_vectToRemove;
protected:
	inline int	RecvFrom(char * buf, int len, struct sockaddr_in *from, socklen_t *fromlen)
	{
		*fromlen	= sizeof(*from);
		return		::recvfrom(m_info.sock, buf, len, 0, (struct sockaddr *)from, fromlen);
	}
	inline int	SendTo(char * buf, int len, struct sockaddr_in *to)
	{
		return		::sendto(m_info.sock, buf, len, 0, (struct sockaddr *)to, sizeof(*to));
	}
	inline int	SendCmdTo(short cmd, struct sockaddr_in *to)
	{
		CMD_T	Cmd;
		Cmd.cmd	= cmd;
		return	SendTo((char *)&Cmd, sizeof(Cmd), to);
	}
	int		RemoveUnit(int nID);
public:
	inline SOCKET	GetSocket() const
	{
		return	m_info.sock;
	}
public:
	UDPRegLister();
	virtual ~UDPRegLister();
	int		Init(INFO_T *pInfo);
	int		Release();
	int		Tick();					// 在外面select正常就可以调用这里面的
};
////////////////////////////////////////////////////////////////////
// 配合上面的登记申请人
////////////////////////////////////////////////////////////////////
class	UDPReger
{
public:
	enum
	{
		STATUS_DROPPED				= -2,					// 断线了
		STATUS_ERROR				= -1,					// 出错了
		STATUS_NOTHING				= 0,
		STATUS_REGING				= 1,
		STATUS_UNREGING				= 2,
		STATUS_WORKING				= 10,					//
		STATUS_WORKEND				= 20,					// UNREG也完毕了，所有工作成功结束
	};
	struct	INFO_T
	{
		SOCKET						sock;					// 通讯socket (UDP，传进来就应该已经和对方connect了)
		n_whcmn::WHRANGE_T<int>		rgNoopInterval;			// 发送NOOP的间隔
		int							nReqInterval;			// 发送Reg或UnReg请求的间隔
		int							nReqTimeOut;			// 请求超时
		INFO_T()
		: sock(INVALID_SOCKET)
		, rgNoopInterval(18000, 22000)						// 平均20秒（用随即是为了让服务器的负载更平均一些）
		, nReqInterval(1000)
		, nReqTimeOut(5000)
		{ }
	};
protected:
	int		m_nStatus;
	unsigned int	m_nLastNoopTime;						// 上次发送NOOP的时刻
	int		m_nNoopInterval;								// NOOP本次的发送间隔
	unsigned int	m_nStartReqTime;						// 最初发送命令请求的时刻
	unsigned int	m_nLastReqTime;							// 上次发送命令请求的时刻
	INFO_T	m_info;
	n_whcmn::whvector<char>			m_RegData;
	n_whcmn::whvector<char>			m_UnRegData;
	n_whcmn::whvector<char>			m_WorkData;
protected:
	void	SetStatus(int nStatus);
	int		Tick_Reging();
	int		Tick_UnReging();
	int		Tick_Working();
public:
	inline SOCKET	GetSocket() const
	{
		return	m_info.sock;
	}
	inline int		GetStatus() const
	{
		return	m_nStatus;
	}
public:
	UDPReger();
	~UDPReger();
	int		Init(INFO_T *pInfo);
	int		Release();
	int		Reg(char *pData, int nSize);
	int		UnReg(char *pData, int nSize);
	int		SendData(char *pData, int nSize);
	int		Tick();
};

// 用于udp日志
int		gUDPLogger_Dft_Init(const char *szParam);
int		gUDPLogger_Dft_Release();
int		gUDPLogger_Dft_WriteFmt(const char *szFmt, ...);
void	gUDPLogger_Set_PrintInScreenOnly(bool bSet);
void	gUDPLogger_Set_PrintInScreenToo(bool bSet);

}		// EOF namespace n_whnet

// 如果只有一个就用这个，省得创建了，不过必须在程序开始的时候用Init初始化这个g_UDPLogger
// 2004-06-29，需要与glogger合起来，所以这里先不用了
//#define	SYSLOGGER_INIT			n_whnet::gUDPLogger_Dft_Init
//#define	SYSLOGGER_RELEASE		n_whnet::gUDPLogger_Dft_Release
//#define	SYSLOGGER_WRITEFMT		n_whnet::gUDPLogger_Dft_WriteFmt
//#define	SYSLOGGER_SET_PRINTINSCREENONLY		n_whnet::gUDPLogger_Set_PrintInScreenOnly
//#define	SYSLOGGER_SET_PRINTINSCREENTOO		n_whnet::gUDPLogger_Set_PrintInScreenToo

//#define	SYSLOGGER_WRITEFMT		printf

#endif	// EOF __WHNETUDP_H__
