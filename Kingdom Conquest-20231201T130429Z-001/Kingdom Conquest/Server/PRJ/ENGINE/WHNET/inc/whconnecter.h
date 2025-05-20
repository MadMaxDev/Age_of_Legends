// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File         : whconnecter.h
// Creator      : Wei Hua (魏华)
// Comment      : 通用的连接器
// CreationDate : 2003-05-21
// ChangeLOG    : 2005-09-02 增加TCPConnecterMsger<_TyMsger>::DoOneWork()中如果时间很长的统计(Debug版才有)
//              : 2005-10-24 加入了SureSend机制，确保一个tick内的立即发送
//              : 2007-03-08 把Connecter的SureSend名字改了，免得在多继承的时候跟别人的名字冲突。

#ifndef	__WHCONNECTER_H__
#define	__WHCONNECTER_H__

#include "whnetcmn.h"
#ifdef	_DEBUG
#include "whnetudpGLogger.h"
#endif
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtime.h>

namespace n_whnet
{

class	Connecter;

class	ConnecterMan
{
public:
	struct	CONNECTOR_INFO_T
	{
		Connecter *	pCntr;
		int			nExt;									// 一个附加数据（由上层设置），默认为0
		CONNECTOR_INFO_T()
			: pCntr(NULL)
			, nExt(0)
		{
		}
		void	clear()
		{
			WHMEMSET0THIS();
		}
	};
public:
	n_whcmn::whvector<SOCKET>	m_extrasock;				// 在select时附加的socket
protected:
	fd_set					m_readfds;						// 用于读选择
	fd_set					m_exceptfds;					// 用于出错选择
	n_whcmn::whunitallocatorFixed<CONNECTOR_INFO_T>	m_Connecters;
															// 存放Connecter指针的分配器
	n_whcmn::whvector<int>	m_tmpvectID;					// 用于临时存放需要删除的ID列表
protected:
	bool	m_bIsReleasing;									// 表明现在正在Release，这样在Relase中调用的函数(如：BeforeDeleteConnecter可能就要做不同处理)
	int		Release();
public:
	// 获得还已经加入的Connecter的数量
	inline int		GetSize() const
	{
		return	m_Connecters.size();
	}
	// 获得还可以加入的Connecter的数量
	inline int		GetAvailSize() const
	{
		return	m_Connecters.availsize();
	}
	inline Connecter *	GetConnecterPtr(int nID)
	{
		CONNECTOR_INFO_T	*pCInfo	= m_Connecters.getptr(nID);
		if( pCInfo )
		{
			return	pCInfo->pCntr;
		}
		return NULL;
	}
public:
	ConnecterMan();
	virtual	~ConnecterMan();
	int		Init(int nMaxNum);
	// 添加一个连接，返回连接的序号(同时pCntr中也被存放了序号)
	int		AddConnecter(Connecter * pCntr);
	// 根据序号移除一个连接(移除时回删除Connecter对象，而Connecter对象默认是自己关闭socket)
	// 里面会调用BeforeDeleteConnecter
	int		RemoveConnecter(int nID);
	// 根据对象指针移除一个连接
	int		RemoveConnecter(Connecter *pCntr);
	// 移除所有的连接（连接肯定也是要中断的啦）
	int		RemoveAllConnecter();
	// 在ms毫秒内做所有socket的selectrd，可以从m_readfds中获得select的结果
	// 返回和cmn_select_rd一致，>0表示有东西，0表示没东西，<0表示socket出错
	int		DoSelect(int ms);
	// 获得所有的socket并appent到vect后面
	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	// 各个Connecter的逻辑运行(应该包括Read/Logic/Write)
	// 如果bSureSend为真则代表需要直接Send
	int		DoWork(bool bSureSend);
	// 确保各个单元的Send会在这个tick内调用
	int		SureSend();
	// 注：一般是先DoSelect，然后再DoWork（当然如果外部有select了就不用了）
private:
	// 当一个连接断掉时需要先做的(比如清除和这个连接相关的信息)
	// 这个是在RemoveConnecter中调用的
	virtual void	BeforeDeleteConnecter(int nCntrID, CONNECTOR_INFO_T *pCntrInfo)
	{}
	// 添加完一个Connecter后一些通知性的东西
	virtual void	AfterAddConnecter(Connecter * pCntr)
	{
	}
};

class	Connecter
{
	friend	class ConnecterMan;
protected:
	bool			m_bDeleteByMan;							// 是否应该被管理器删除(有些可能需要加入它的人删除，比如一个Connecter就是这个Man)
	bool			m_bAutoCloseSocket;						// 是否在析构中自动删除socket
	bool			m_bSocketError;							// 表示网络出错了
	SOCKET			m_sock;
	ConnecterMan	*m_pMan;								// 对应管理器的指针
	int				m_nIDInMan;								// 在ConnecterMan中的序号
public:
	Connecter()
	: m_bDeleteByMan(true), m_bAutoCloseSocket(true), m_bSocketError(false)
	, m_sock(INVALID_SOCKET), m_pMan(NULL), m_nIDInMan(-1)
	{}
	virtual ~Connecter();
	// 主要是为了派生类可以附加自己的参数(这个应该不常用，所以就用字符串名字)
	// 比如：这个可以用来区分TCPConnecterServer的this和其他Connecter
	virtual void *	QueryInterface(const char *cszIName=NULL)
	{
		return	NULL;
	}

	// 获得相关的socket（主要用于select）
	inline SOCKET	GetSocket() const
	{
		return	m_sock;
	}
	inline void		SetSocket(SOCKET sock)
	{
		m_sock	= sock;
	}
	inline int	GetIDInMan() const
	{
		return	m_nIDInMan;
	}
	inline bool	GetSocketError() const
	{
		return	m_bSocketError;
	}
	inline void	SetSocketError()
	{
		m_bSocketError	= true;
	}
protected:
	virtual	int		Connecter_DoOneWork()	= 0;
	virtual	int		Connecter_SureSend()	= 0;
};

// 连接接受器
class	Connecter_TCPAccepter	: public Connecter
{
// 为父类实现的
protected:
	virtual	int		Connecter_DoOneWork();
	virtual	int		Connecter_SureSend();
// 让子类为自己实现
public:
	Connecter_TCPAccepter()
	: m_pallowdeny(NULL)
	{
	}
public:
	cmn_addr_allowdeny	*m_pallowdeny;
private:
	// 当发现一个新连接时的处理
	// 返回0成功，-1失败
	virtual int		DealNewSocket(SOCKET sock, struct sockaddr_in *paddr)
															= 0;
};

// 多连入的TCP服务器
// 从他继承的都不要实现QueryInterface，因为这样才能在遍历Connecter时区分出自己
class	TCPConnecterServer		: public ConnecterMan, public Connecter_TCPAccepter
{
public:
	struct	INFO_T
	{
		int		nMaxNum;		// 最多的连接数
		int		nPort;			// 监听的端口
		int		nPort1;			// 附加的端口(如果需要范围绑定的话)
		char	szIP[WHNET_MAXIPSTRLEN];
								// 绑定到这个IP
		char	szAllowDeny[WH_MAX_PATH];
								// 允许禁止地址
		INFO_T()
		: nMaxNum(16), nPort(0), nPort1(0)
		{
			szIP[0]			= 0;
			szAllowDeny[0]	= 0;
		}
	};
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
	enum
	{
		ERRNOTIFYCMD_AVAILSIZEIS0		= 0,	// 可用的连接数为0
	};
public:
	~TCPConnecterServer();
	int	Init(INFO_T *pInfo);
	int	Release();
	// 删除所有连接
	int	DeleteAllConnection();
	// 获得第一个可用的连接器ID
	int	GetFirstConnecterID();
public:
	// 获得还已经加入的连接的数量
	inline int		GetSize() const
	{
		return	ConnecterMan::GetSize()-1;
	}
	inline struct sockaddr_in &	GetBindAddr()
	{
		return	m_bindaddr;
	};
private:
	cmn_addr_allowdeny	m_AllowDeny4Connect;
	struct sockaddr_in	m_bindaddr;				// 自己绑定的地址
private:
	// 这个是为了Connecter_TCPAccepter实现的
	virtual int			DealNewSocket(SOCKET sock, struct sockaddr_in *paddr);
	// 子类创建合适的Connecter（这个是在DealNewSocket中调用的）
	virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
																	= 0;
	// 情况通告
	virtual void		ErrNotify(int nNotifyCmd, void *pExtData)	{}
};

// 可进行小于64K消息传输的Connecter
template<class _TyMsger>
class	TCPConnecterMsger	: public Connecter, public _TyMsger
{
public:
	struct	INFO_T
	{
		// 用于初始化tcpmsger
		typename _TyMsger::INFO_T	msgerINFO;
		// 自己用的信息
	};
public:
	int	Init(INFO_T *pInfo);
	inline SOCKET	GetSocket()
	{
		return	Connecter::GetSocket();
	}
protected:
	virtual	int		Connecter_DoOneWork();
	virtual	int		Connecter_SureSend();
private:
	// 是否有空处理消息
	virtual bool	CanDealMsg() const							= 0;
	// 子类中实现消息处理
	virtual int		DealMsg(const void *pData, size_t nSize)	= 0;
};
template<class _TyMsger>
int		TCPConnecterMsger<_TyMsger>::Init(INFO_T *pInfo)
{
	// 保证设置符合要求
	// 这里不能有多个线程
	pInfo->msgerINFO.bAutoStartThread	= false;
	// 设置我的sock
	m_sock	= pInfo->msgerINFO.sock;
	// 调用msger的初始化
	if( _TyMsger::Init(&pInfo->msgerINFO)<0 )
	{
		return	-1;
	}
	return	0;
}
template<class _TyMsger>
int		TCPConnecterMsger<_TyMsger>::Connecter_DoOneWork()
{
	// 手工收一次数据
	_TyMsger::ManualRecv(0);
	// 看看有无数据输入
	const void	*pData;
	size_t		nSize;
	// 上层可以控制CanDealMsg，决定是否可以处理新消息(比如向外部发送指令的通道阻塞了)
	// 但是这时输入指令的一方认为命令已经发出了，所以需要注意处理后事(比如输入方和服务器断线后对缓冲区内指令的处理)
	while( CanDealMsg() && (pData = _TyMsger::PeekMsg(&nSize))!=NULL )
	{
		// 处理之
		DealMsg(pData, nSize);
		// 将数据弹出
		_TyMsger::FreeMsg();
	}
	// 判断是否有错误
	switch( _TyMsger::GetStatus() )
	{
		case	_TyMsger::STATUS_ERROR:
			Connecter::SetSocketError();
		break;
	}
	return	0;
}
template<class _TyMsger>
int		TCPConnecterMsger<_TyMsger>::Connecter_SureSend()
{
	_TyMsger::ManualSend(0);
	return	0;
}

// 简单的消息返回
template<class _TyMsger>
class	TCPConnecterMsger_Echo	: public TCPConnecterMsger<_TyMsger>
{
private:
	typedef	TCPConnecterMsger<_TyMsger>	FATHERCLASS;
private:
	// 里面只是把消息原样返回(测试用)
	virtual int		DealMsg(const void *pData, size_t nSize)
	{
		return	FATHERCLASS::SendMsg(pData, nSize);
	}
	virtual bool	CanDealMsg() const
	{
		return	true;
	}
};

}		// EOF namespace n_whnet

#endif	// EOF __WHCONNECTER_H__
