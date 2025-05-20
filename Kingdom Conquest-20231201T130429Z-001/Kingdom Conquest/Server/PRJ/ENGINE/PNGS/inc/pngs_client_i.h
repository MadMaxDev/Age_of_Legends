// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_client_i.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的Client模块内部实现
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-08-25
// Change LOG   :

#ifndef	__PNGS_CLIENT_I_H__
#define	__PNGS_CLIENT_I_H__

#include "./pngs_client.h"
#include "./pngs_packet.h"
#include "./pngs_def.h"
#include "./pngs_packet_client_logic.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whnetcnl2.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whdll.h>
#include <WHCMN/inc/whthread.h>

//#define CLIENT_STATIC_ANTIHACK	// 把内部变量定义成静态的，防止外挂创建出多个对象

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

class	PNGSClient_I	: public PNGSClient, public CMNBody
{
public:
	// PNGSClient用的
	virtual	void	SelfDestroy();
	virtual	int		Init(const char *cszCFG);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		SetICryptFactory(n_whcmn::ICryptFactory *pFactory);
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		TickRecv();
	virtual	int		TickSend();
	virtual	void	SetVer(const char *szVer);
	virtual	const char *	GetVer();
	virtual	int		Connect(const char *cszAddr, unsigned char nNetworkType);
	virtual	int		Disconnect();
	virtual	int		ControlOut(CONTROL_T **ppCmd, size_t *pnSize);
	virtual	int		Send(unsigned char nSvrIdx, unsigned char nChannel, const void *pCmd, size_t nSize);
	virtual	int		SendCAAFS(const void *pCmd, size_t nSize);
	virtual	int		Recv(unsigned char *pnChannel, void **ppCmd, size_t *pnSize);
	virtual	int		GetLastError() const;
	virtual	void	SetLastError(int nErrCode);
	virtual void	SetFileMan(n_whcmn::whfileman *pFileMan);
	virtual	ILogicBase *	GetPlugIn(const char *cszPlugInName, int nVer);
	virtual int		GetPing() const;
	virtual int		StartThread();
	virtual int		StopThread();
private:
	// CMNBody扩展的
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
// 自己使用的
public:
	PNGSClient_I();
	~PNGSClient_I();
private:
	// 各种结构定义
	// 初始化参数
	struct	CFGINFO_T		: public whdataini::obj
	{
		// 基本参数(和连接相关的都在DATA_INI_INFO_T中定义了)
		char				szCAAFSSelfInfoToClient[256];			// 用户连接CAAFS会先看到这样一句话，可以做比较看连接是否正确
		int					nSlotInOutNum;							// 各个slot的In/Out数量是一样的
		int					nTQChunkSize;							// 时间队列的每次次性分配块大小
		int					nControlOutQueueSize;					// 用于通知上层的指令队列长度
		int					nDataQueueSize;							// 用于通知上层数据的队列长度
		int					nQueueCmdInSize;
		int					nPlayStatInteravl;						// 设置为<=0表示不打开统计

		CFGINFO_T()
		: nSlotInOutNum(PNGS_SLOTINOUTNUM)
		, nTQChunkSize(100)
		, nControlOutQueueSize(10000)
		, nDataQueueSize(PNGS_RAWBUF_SIZE)							// 默认大一点吧。否则累计很多指令就丢了。
		, nQueueCmdInSize(512*1024)
		, nPlayStatInteravl(66*1000)
		{
			strcpy(szCAAFSSelfInfoToClient, PNGS_DFT_CAAFS_SELFINFO_STRING);
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// 时间事件结构
	struct	TQUNIT_T
	{
		typedef	void (PNGSClient_I::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// 处理该事件的函数
		// 数据部分
		union
		{
		}un;
	};
	// 给m_queueData输入的数据格式(内部用，所以不用pack修饰)
	struct	DATA_T
	{
		size_t			nSize;
		unsigned char	nChannel;
		char			data[1];
	};
	// 为了PlugIn准备的
	// 用于装载DLL中的函数的指针
	typedef	n_pngs::CMN::ILogic *	(*fn_ILOGIC_CREATE_T)();
	typedef	void					(*fn_WHNET_STATIC_INFO_In)(void *);
	typedef	void					(*fn_WHCMN_STATIC_INFO_In)(void *);
	struct	PLUGIN_CFGINFO_T	: public whdataini::obj
	{
		PNGSClient_I			*pHost;

		WHDATAPROP_SETVALFUNC_DECLARE(DLL)
		{
			return	pHost->AddPlugIn(cszVal);
		}
		// 定义为有成员映射的结构
		WHDATAPROP_DECLARE_MAP(PLUGIN_CFGINFO_T)
	};
private:
	// 数据部分
	CFGINFO_T		m_cfginfo;										// 配置
	CNL2SlotMan::DATA_INI_INFO_T		m_CLIENT_SLOTMAN_INFO;		// 客户端连接服务器的slotman的配置
	int				m_nStatus;										// 当前状态
	int				m_nLastError;									// 最后设置的错误码
	whtick_t		m_tickNow;
	int				m_nClientID;									// 在服务器中的ID
	char			m_szVer[PNGS_VER_LEN];							// 版本串

#ifdef CLIENT_STATIC_ANTIHACK
	// 加密工厂
	static ICryptFactory	*m_pICryptFactory;
	// 连接管理
	static CNL2SlotMan		*m_pSlotMan;
#else
	// 加密工厂
	ICryptFactory	*m_pICryptFactory;
	// 连接管理
	CNL2SlotMan		*m_pSlotMan;
#endif

	// slot号
	int				m_nSlotCAAFS;									// 和CAAFS连接的
	int				m_nSlotCLS;										// 和CLS连接的
	// 记录上次CAAFS_CLIENT_GOTOCLS_T的内容
	CAAFS_CLIENT_GOTOCLS_T	m_GoToCLSInfo;
	// 时间队列。比如：连接超时
	whtimequeue		m_TQ;
	// ControlOut的队列
	whsmpqueue		m_queueControlOut;
	// 临时存放m_queueControlOut给上层的指令
	whvector<char>	m_vectControlOut;
	// 收到数据的队列
	whsmpqueue		m_queueData;
	// 临时存放m_queueData给上层的数据
	whvector<char>	m_vectData;
	// 用于临时合成指令的
	whvector<char>	m_vectrawbuf;

	// 状态处理函数指针
	typedef	int	(PNGSClient_I::*TICK_CONTROLOUT_STATUS_FUNC_T)(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	TICK_CONTROLOUT_STATUS_FUNC_T	m_pTickControlOut;
	typedef	int	(PNGSClient_I::*TICK_DEALRECV_STATUS_FUNC_T)(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	TICK_DEALRECV_STATUS_FUNC_T		m_pTickDealRecv;

	// 和PlugIn相关的
	PLUGIN_CFGINFO_T				m_PlugInCfg;
	const char						*m_pcszCFG;						// PlugIn初始化时需要默认的配置文件名
	whvector<WHDLL_Loader *>		m_vectDLLLoader;				// 各个DLL载入器
	bool							m_bQuickDetachLogic;			// 如果在Init过程中出错了。在Release时这个就需要置为true。
	whhash<pngs_cmd_t, ILogicBase *>	m_mapCmdReg2Logic;			// 用于指令分拣，把特定的指令分拣给特定的逻辑模块去执行

	// 内部线程
	whtid_t							m_tid;
	bool							m_bStopThread;					// 标记是否停止线程

	// stat相关
	whtimequeue::ID_T				m_teidStat;
private:
	// Init子函数
	int		Init_CFG(const char *cszCFG);
	// 设置状态（同时设置相关的属性，如：各个处理函数）
	void	SetStatus(int nStatus);
	// 发送ControlOut给上层
	void *	ControlOutAlloc(size_t nSize);
	template<typename _Ty>
	inline void	ControlOutAlloc(_Ty *&ptr)
	{
		ptr	= (_Ty *)ControlOutAlloc(sizeof(_Ty));
	}
	// Tick子函数
	int		Tick_ControlOut_STATUS_NOTHING(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_ControlOut_STATUS_CONNECTINGCAAFS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_ControlOut_STATUS_WAITINGINCAAFS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_ControlOut_STATUS_CONNECTINGCLS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_ControlOut_STATUS_WORKING(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_NOTHING(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_CONNECTINGCAAFS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_WAITINGINCAAFS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_CONNECTINGCLS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_WORKING(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	// 处理一个CLS_CLIENT_DATA数据包（首先内部分拣，然后压入给上层的队列）
	int		Deal_CLS_CLIENT_DATA_One(int nChannel, const void *pData, size_t nDSize);

	// 定时相关的
	void	TEDeal_PlayStat(TQUNIT_T * pTQUnit);
	void	SetTE_PlayStat();
private:
	// 内部使用的给外界提供函数的真是部分（主要是为了给线程做的）
	int		_GetSockets(n_whcmn::whvector<SOCKET> &vect);
	int		_TickRecv();
	int		_TickSend();
public:
	int		AddPlugIn(const char *cszVal);
	// 线程函数
	static void *	thread_func(void *ptr);
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CLIENT_I_H__
