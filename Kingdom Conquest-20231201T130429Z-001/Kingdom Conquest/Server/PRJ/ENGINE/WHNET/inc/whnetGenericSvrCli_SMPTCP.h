// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetGenericSvrCli_SMPTCP.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于简单的TCP传输的whnetGenericSvrCli的实现
// CreationDate : 2005-03-04
// Change LOG   :

#ifndef	__WHNETGENERICSVRCLI_SMPTCP_H__
#define	__WHNETGENERICSVRCLI_SMPTCP_H__

#include "whnetGenericSvrCli.h"
#include "whconnecter.h"
#include "whnettcp.h"
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whcmd.h>

using namespace n_whcmn;

namespace n_whnet
{

class	whnetGenericSvrCli_SMTCPSVR	: public whnetGenericSvrCli, public TCPConnecterServer
{
public:
	WHDATAINI_STRUCT_DECLARE(CNTRSVRINFO_T, TCPConnecterServer::INFO_T);
	WHDATAINI_STRUCT_DECLARE(MSGERINFO_T, tcpmsger::INFO_T)
	struct	CFGINFO_T		: public whdataini::obj
	{
		CNTRSVRINFO_T	CNTRSVRINFO;
		MSGERINFO_T		MSGERINFO;
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
public:
	whnetGenericSvrCli_SMTCPSVR();
	virtual ~whnetGenericSvrCli_SMTCPSVR();
public:
	// 为whnetGenericSvrCli实现
	virtual	int	Init(const char *cszCFG);
	virtual	int	Release();
	virtual	int	DoSelect(int nInterval);
	virtual	int	Tick();
	virtual int	CmdIn(int nCmd, const void *pData, size_t nDSize);
private:
	// 为TCPConnecterServer实现
	virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	virtual void	AfterAddConnecter(Connecter * pCntr);
	virtual void	BeforeDeleteConnecter(int nCntrID, CONNECTOR_INFO_T *pCntrInfo);
private:
	// 自己用的成员变量
	// 配置信息
	CFGINFO_T		m_cfginfo;
	// 用于临时存放指令的缓冲区
	whvector<char>	m_vectTmpBuf;
	// 用于存放需要Remove的Connect的ID
	whvector<int>	m_vectConnecterToRemove;
	// 状态
	int				m_nStatus;
	// 指令映射器
	WHCMN_CMD_REGANDDEAL_DECLARE_m_rad(whnetGenericSvrCli_SMTCPSVR);
private:
	// 各个指令对应的函数
	int	Cmd_SVRCMD_REQ_STARTSERVER(const void *pData, size_t nDSize);
	int	Cmd_SVRCMD_REQ_STOPSERVER(const void *pData, size_t nDSize);
	int	Cmd_SVRCMD_REQ_SENDDATA(const void *pData, size_t nDSize);
	int	Cmd_SVRCMD_REQ_DISCONNECTCLIENT(const void *pData, size_t nDSize);
	int	Cmd_SVRCMD_REQ_SETCLIENTTAG(const void *pData, size_t nDSize);
	// 通知上层自己的状态
	void	StatusNotify();
	// 设置状态同时通知上层(同时里面会根据不同的状态做一些操作)
	void	SetStatusAndNofity(int nStatus);
public:
	// 给Connecter调用的函数
	bool	CanDealMsg() const;
	int		DealMsg(int nIDInMan, const void *pData, size_t nSize);
private:
	// 自己用到的Connecter结构
	class	I_Connecter	: public TCPConnecterMsger<tcpmsger>
	{
	public:
		enum
		{
			MAXTAGNUM					= 32,
		};
		whnetGenericSvrCli_SMTCPSVR		*m_pHost;
		int								m_aTag[MAXTAGNUM];
		struct sockaddr_in				m_addr;						// 用户地址
	public:
		I_Connecter(whnetGenericSvrCli_SMTCPSVR *pHost)
		: m_pHost(pHost)
		{
			memset(m_aTag, 0, sizeof(m_aTag));
			memset(&m_addr, 0, sizeof(m_addr));
		}
		// 其父类都是虚析构
		virtual ~I_Connecter()
		{
		}
		virtual void *	QueryInterface(const char *cszIName)
		{
			// 简单起见
			return	this;
		}
	public:
		// 设置标记
		int	SetTag(int nIdx, int nValue)
		{
			if( nIdx<0 || nIdx>= MAXTAGNUM )
			{
				// 超界
				return	-1;
			}
			m_aTag[nIdx]	= nValue;
			return	0;
		}
		inline int	GetTag(int nIdx) const
		{
			return	m_aTag[nIdx];
		}
	private:
		// 为TCPConnecterMsger实现的
		bool	CanDealMsg() const
		{
			return	m_pHost->CanDealMsg();
		}
		int		DealMsg(const void *pData, size_t nSize)
		{
			return	m_pHost->DealMsg(m_nIDInMan, pData, nSize);
		}
	};
};

class	whnetGenericSvrCli_SMTCPCLI	: public whnetGenericSvrCli
{
public:
	WHDATAINI_STRUCT_DECLARE(MSGERINFO_T, tcpmsger::INFO_T)
	struct	CFGINFO_T		: public whdataini::obj
	{
		MSGERINFO_T		MSGERINFO;
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
public:
	whnetGenericSvrCli_SMTCPCLI();
	virtual ~whnetGenericSvrCli_SMTCPCLI();
public:
	virtual	int	Init(const char *cszCFG);
	virtual	int	Release();
	virtual	int	DoSelect(int nInterval);
	virtual	int	Tick();
	virtual int	CmdIn(int nCmd, const void *pData, size_t nDSize);
private:
	// 自己用的成员变量
	// 状态
	int				m_nStatus;
	// 超时的时刻
	whtick_t		m_nTimeOutTime;
	// 配置信息
	CFGINFO_T		m_cfginfo;
	// 通讯器
	tcpmsger		m_msger;
	// 用于临时存放指令的缓冲区
	whvector<char>	m_vectTmpBuf;
	// 指令映射器
	WHCMN_CMD_REGANDDEAL_DECLARE_m_rad(whnetGenericSvrCli_SMTCPCLI);
private:
	// 各个指令对应的函数
	int	Cmd_CLICMD_REQ_CONNECT(const void *pData, size_t nDSize);
	int	Cmd_CLICMD_REQ_DISCONNECT(const void *pData, size_t nDSize);
	int	Cmd_CLICMD_REQ_SENDDATA(const void *pData, size_t nDSize);
	// 通知上层自己的状态
	void	StatusNotify();
	// 设置状态同时通知上层(同时里面会根据不同的状态做一些操作)
	void	SetStatusAndNofity(int nStatus);
};

}

#endif	// EOF __WHNETGENERICSVRCLI_SMPTCP_H__
