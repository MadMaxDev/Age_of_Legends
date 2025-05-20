// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_TCPReceiver.h
// Creator      : Wei Hua (魏华)
// Comment      : TCP指令接受模块
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2007-02-25
// Change LOG   : 2008-05-20 给PNGS_TR2CD_CMD_T中增加了IP

#ifndef	__PNGS_TCPRECEIVER_H__
#define	__PNGS_TCPRECEIVER_H__

#include "pngs_cmn.h"
#include <WHNET/inc/whconnecter.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whgener.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

////////////////////////////////////////////////////////////////////
// PNGS_TCPRECEIVER
////////////////////////////////////////////////////////////////////
class	PNGS_TCPRECEIVER	: public CMN::ILogic, public TCPConnecterServer
{
///////////////////////////////////////////
// 为接口实现的
///////////////////////////////////////////
protected:
	// CMN::ILogic的
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	PNGS_TCPRECEIVER();
	~PNGS_TCPRECEIVER();
	// CMN::ILogic的
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		SureSend();

private:
	// CMN::ILogic的
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		Tick_BeforeDealCmdIn();
	virtual int		Tick_AfterDealCmdIn();
	// TCPConnecterServer的
	virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	virtual void		BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo);
	virtual void		AfterAddConnecter(Connecter * pCntr);
// 自己用的
private:
	struct	CFGINFO_T		: public whdataini::obj
	{
		int										nQueueCmdInSize;	// 指令输入队列的长度
		int										nMaxCmdPerSec;		// 每秒中最多可以处理的发来的指令数量
		TCPConnecterServer::DATA_INI_INFO_T		CNTRSVR;			// TCPConnecterServer的通用配置
		tcpmsger::DATA_INI_INFO_T				MSGER;				// 每个TCP连接的通讯msger的配置

		CFGINFO_T()
			: nQueueCmdInSize(2000000)
			, nMaxCmdPerSec(2000)
		{
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	class	MYCNTR	: public	TCPConnecterMsger<tcpmsger>
	{
	public:
		PNGS_TCPRECEIVER	*m_pHost;
		struct sockaddr_in	m_addr;								// 来源地址
	public:
		MYCNTR(PNGS_TCPRECEIVER *pHost)
			: m_pHost(pHost)
		{
		}
		// 我原来竟然把这个给去掉了，晕
		virtual void *	QueryInterface(const char *cszIName)
		{
			// 重载了这个就可以和Accepter区分了（Accepter返回NULL）
			return	this;
		}
		// 是否有空处理消息
		virtual bool	CanDealMsg() const
		{
			return	m_pHost->CNTR_CanDealMsg();
		}
		// 子类中实现消息处理
		virtual int		DealMsg(const void *pData, size_t nSize)
		{
			return	(m_pHost->CNTR_DealMsg)(this, pData, nSize);
		}
	};

	CFGINFO_T	m_cfginfo;
	ILogicBase	*m_pLogicDftCmdDealer;	// 默认的指令处理模块
	typedef		whhash<pngs_cmd_t, whlist<ILogicBase *> * >	MAPCMD2LOGIC_T;
	typedef		MAPCMD2LOGIC_T::kv_iterator					MAPCMD2LOGIC_IT_T;
	MAPCMD2LOGIC_T	m_mapCmdReg2Logic;						// 用于指令分拣，把特定的指令分拣给特定的逻辑模块去执行

	// 用于生成单位时间可以处理发来的指令数量
	whgener		m_generDealGMSCmdNum;
	// 用于获得每个tick的时间间隔
	whinterval	m_itv;
	// 当前tick剩余可处理的指令数量
	int			m_nCmdNumThisTick;

private:
	friend	class	MYCNTR;
	// 配置文件读取
	int		Init_CFG(const char *cszCFGName);
	// 针对一个Connecter做一次工作
	bool	CNTR_CanDealMsg() const;
	// 针对一个Connecter做一次工作
	int		CNTR_DealMsg(MYCNTR *pCntr, const void *pCmd, size_t nSize);

public:
	// 创建自己类型的对象
	static CMN::ILogic *	Create()
	{
		return	new	PNGS_TCPRECEIVER;
	}
};

// 仅用于测试返回的CmdDealer（把传来的指令原样返回）
class	PNGS_TCPRECEIVER_DUMMYCMDDEALER	: public CMN::ILogic
{
///////////////////////////////////////////
// 为接口实现的
///////////////////////////////////////////
protected:
	// CMN::ILogic的
	virtual	int		Organize();
public:
	PNGS_TCPRECEIVER_DUMMYCMDDEALER();
	~PNGS_TCPRECEIVER_DUMMYCMDDEALER();
	// CMN::ILogic的
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
private:
	// CMN::ILogic的
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		Tick_BeforeDealCmdIn();
	virtual int		Tick_AfterDealCmdIn();
///////////////////////////////////////////
// 自用的
///////////////////////////////////////////
private:
	ILogicBase	*m_pLogicTCPReceiver;

public:
	// 创建自己类型的对象
	static CMN::ILogic *	Create()
	{
		return	new	PNGS_TCPRECEIVER_DUMMYCMDDEALER;
	}
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_TCPRECEIVER_H__
