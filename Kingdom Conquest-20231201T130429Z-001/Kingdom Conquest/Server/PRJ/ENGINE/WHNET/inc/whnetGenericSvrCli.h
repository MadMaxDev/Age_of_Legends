// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetGenericSvrCli.h
// Creator      : Wei Hua (魏华)
// Comment      : 通用的服务器/客户端模板
//              : ClientID：实际上就是连接的ID
//              : 上层在SENDDATA/RECVDATA之上自己指定逻辑相关的通讯协议
// CreationDate : 2005-03-04
// Change LOG   :

#ifndef	__WHNETGENERICSVRCLI_H__
#define	__WHNETGENERICSVRCLI_H__

#include "whnetcmn.h"

namespace n_whnet
{

class	whnetGenericSvrCli
{
public:
	enum
	{
		TYPE_SMPTCPSVR				= 10,					// 简单TCP服务器
		TYPE_SMPTCPCLI				= 11,					// 简单TCP客户端
	};
public:
	static whnetGenericSvrCli *		Create(int nType);
	virtual ~whnetGenericSvrCli()	{}
public:
	virtual	int	Init(const char *cszCFG)					= 0;
	virtual	int	Release()									= 0;
	virtual	int	DoSelect(int nInterval)						= 0;
	virtual	int	Tick()										= 0;
	virtual	int	CmdIn(int nCmd, const void *pData, size_t nDSize)
															= 0;
public:
	class	CmdDeal
	{
	public:
		virtual ~CmdDeal()	{}
		virtual int	CallFunc(int nCmd, const void *pData, size_t nDSize)
															= 0;
	};
protected:
	CmdDeal							*m_pCmdDeal;			// 这个用来向调用者返回数据
public:
	// 设置指令回调对象(!!!!这个必须在Init之前设置，工作过程中最好不要换!!!!)
	inline void	SetCmdDeal(CmdDeal *pCmdDeal)
	{
		m_pCmdDeal	= pCmdDeal;
	}
	// (一般由子类调用，比如后面的whnetGenericSvrCli_SMTCPSVR、whnetGenericSvrCli_SMTCPCLI等等)调用回调
	inline int	RPLCmdCall(int nCmd, const void *pData, size_t nDSize)
	{
		return	m_pCmdDeal->CallFunc(nCmd, pData, nDSize);
	}

////////////////////////////////////////////////////////////
// 指令交互的各种结构和定义{
////////////////////////////////////////////////////////////
public:
	// 这些指令不能超过65535
	// REQ指令必须是连续的
	enum
	{
		// 服务器用到的指令
		SVRCMD_REQ_STARTSERVER		= 1,					// 开启服务器
															// 结构：SVRCMD_REQ_STARTSERVER_T
		SVRCMD_REQ_STOPSERVER		= 2,					// 开启服务器
															// 结构：无
		SVRCMD_REQ_SENDDATA			= 3,					// 向某个客户端发送数据
															// 结构：SVRCMD_REQ_SENDDATA_T
		SVRCMD_REQ_DISCONNECTCLIENT	= 4,					// 关闭客户端的连接
															// 结构：SVRCMD_REQ_DISCONNECTCLIENT_T
		SVRCMD_REQ_SETCLIENTTAG		= 5,					// 设置某个客户端的标记
															// 结构：SVRCMD_REQ_SETCLIENTTAG_T
		SVRCMD_RPL_STATUS			= 100,					// 服务器自身状态通知
															// 结构：SVRCMD_RPL_STATUS_T
		SVRCMD_RPL_CLIENTIN			= 101,					// 通知有客户端连入
															// 结构：SVRCMD_RPL_CLIENTIN_T
		SVRCMD_RPL_CLIENTOUT		= 102,					// 通知有客户端断线
															// 结构：SVRCMD_RPL_CLIENTOUT_T
		SVRCMD_RPL_RECVDATA			= 103,					// 某个客户端发来了数据
															// 结构：SVRCMD_RPL_RECVDATA_T
		// 客户端用到的指令
		CLICMD_REQ_CONNECT			= 1,					// 连接服务器
															// 结构：CLICMD_REQ_CONNECT_T
		CLICMD_REQ_DISCONNECT		= 2,					// 从服务器断开
															// 结构：无
		CLICMD_REQ_SENDDATA			= 3,					// 向服务器发送数据
															// 结构：char[1]
		CLICMD_RPL_STATUS			= 100,					// 客户端状态通知(比如连接成功、网络断线等等)
															// 结构：CLICMD_RPL_STATUS_T
		CLICMD_RPL_RECVDATA			= 101,					// 从服务器获得数据
															// 结构：char[1]
	};
	enum
	{
		// 服务器状态
		SVRSTATUS_NOTHING			= 0,
		SVRSTATUS_STARTING			= 1,					// 服务器开启中
		SVRSTATUS_WORKING			= 10,					// 正常工作状态
		SVRSTATUS_STOPING			= 20,					// 服务器关闭中
		SVRSTATUS_INTERNALERROR		= -1,					// 内部出错
		SVRSTATUS_SOCKETERROR		= -2,					// 网络出错
		// 客户端状态
		CLISTATUS_NOTHING			= 0,
		CLISTATUS_CONNECTING		= 1,					// 正在连接服务器
		CLISTATUS_WORKING			= 10,					// 连接成功，正常工作中
		CLISTATUS_DISCONNECTING		= 20,					// 正在从服务器断开
		CLISTATUS_CONNECTTIMEOUT	= -1,					// 连接超时
		CLISTATUS_DISCONNECTED		= -2,					// 网络断线了
		CLISTATUS_INTERNALERROR		= -3,					// 内部错误(比如内存、或者某些对象初始化出错)
	};
	#pragma pack(1)
	// 服务器用到的结构
	struct	SVRCMD_REQ_STARTSERVER_T
	{
		char	szIP[WHNET_MAXADDRSTRLEN];					// 监听的IP
		int		nPort;										// 监听的端口
	};
	struct	SVRCMD_REQ_SENDDATA_T
	{
		int		nClientID;									// 对应的ClientID，如果是-1表示对所有人(这时下面的TagXXX才有意义)
		int		nTagIdx;									// 如果>=0表示和某个位置Tag相等才能发送
		int		nTagValue;									// 表示对应的Tag应该等于的值
		char	szData[1];									// 数据部分自己解释
	};
	struct	SVRCMD_REQ_DISCONNECTCLIENT_T
	{
		int		nClientID;									// 对应的玩家ID，如果是-1表示对所有人(这是下面的TagXXX才有意义)
	};
	struct	SVRCMD_REQ_SETCLIENTTAG_T
	{
		int		nClientID;
		int		nTagIdx;									// 要设置的Tag索引
		int		nTagValue;									// 表示对应的Tag应该等于的值
	};
	struct	SVRCMD_RPL_STATUS_T
	{
		int		nStatus;									// 前面定义的SVRSTATUS_XXX
	};
	struct	SVRCMD_RPL_CLIENTIN_T
	{
		int		nClientID;									// 进入的玩家的ID
		struct sockaddr_in	addr;							// 玩家地址
	};
	struct	SVRCMD_RPL_CLIENTOUT_T
	{
		enum
		{
			REASON_NOTHING		= 0,
			REASON_CLOSE		= 1,						// 客户端主动断线
			REASON_TIMEOUT		= 2,						// 客户端超时
		};
		int		nClientID;									// 离开的玩家的ID
		int		nReason;									// 离开的原因
	};
	struct	SVRCMD_RPL_RECVDATA_T
	{
		int		nClientID;									// 对应的玩家ID
		char	szData[1];									// 数据部分
	};
	// 客户端用到的结构
	struct	CLICMD_REQ_CONNECT_T
	{
		char	szAddr[WHNET_MAXADDRSTRLEN];				// 目标地址
		int		nTimeOut;									// 连接超时时间(毫秒)
		CLICMD_REQ_CONNECT_T()
		: nTimeOut(5000)
		{
			szAddr[0]	= 0;
		}
	};
	struct	CLICMD_RPL_STATUS_T
	{
		int		nStatus;									// 前面定义的CLISTATUS_XXX
	};
	#pragma pack()
////////////////////////////////////////////////////////////
// 指令交互的各种结构和定义}
////////////////////////////////////////////////////////////
};

}

#endif	// EOF __WHNETGENERICSVRCLI_H__
