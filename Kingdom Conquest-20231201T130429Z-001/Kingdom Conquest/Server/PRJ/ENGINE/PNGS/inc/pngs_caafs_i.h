// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的CAAFS模块的内部实现
//                PNGS是Pixel Network Game Structure的缩写
//                CAAFS是Connection Allocation and Address Filter Server的缩写，是连接分配功能中的连接分配和地址过滤服务器
//                CAAFS上面只有两种用户，等待连接的（排队的）和正在连接中的，连接好的用户就离开了CAAFS。所以CAAFS是可以宕掉再重启的。
//                CAAFS中不记录各个CLS的信息，这些信息都是在GMS中记录的。CAAFS所做的只是提交用户希望连入的请求给GMS。
// CreationDate : 2005-07-26
// Change LOG   :

#ifndef	__PNGS_CAAFS_I_H__
#define	__PNGS_CAAFS_I_H__

#include "./pngs_caafs.h"
#include "./pngs_packet.h"
#include "./pngs_cmn.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whnetcnl2.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whDList.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whgener.h>
#include <WHCMN/inc/whcmd.h>
#include <set>
#include <map>
#include <string>

using namespace n_whnet;
using namespace n_whcmn;
using namespace std;

#define TTY_ACCOUNTNAME_LONG_LEN 64  //不想include太多东西了，就在这里定义了
//#define UNITTEST

namespace n_pngs
{

class	CAAFS2_I	: public CAAFS2
{
public:
	virtual	void	SelfDestroy();
	virtual	int		Init(const char *cszCFG);
	virtual	int		Init_CFG_Reload(const char *cszCFG);
	virtual	int		Release();
	virtual	int		SetICryptFactory(ICryptFactory *pFactory);
	virtual size_t	GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		Tick();
	virtual	int		GetStatus() const;
	virtual	bool	ShouldStop() const;
public:
	CAAFS2_I();
	~CAAFS2_I();
private:
	// 各种结构定义
	// 初始化参数
	struct	CFGINFO_T		: public whdataini::obj
	{
		// 基本参数
		int					nVerCmpMode;							// 取值为(PNGS_VERCMP_MODE_XXX，单间pngs_cmn.h)
		char				szVer[PNGS_VER_LEN];					// 版本（客户端必须和这个版本对比一样了才行）
		char				szWelcomeAddr[WHNET_MAXADDRSTRLEN];		// 用户看到的服务器地址
		char				szGMSAddr[WHNET_MAXADDRSTRLEN];			// GMS等待其他服务器连接地址(所有服务器连接GMS的地址都是这个)
		int					nConnectGMSTimeOut;						// 连接GMS的超时
		int					nSelectInterval;						// Tick中做select的间隔(毫秒)
		int					nGroupID;								// 和相应的CLS对应
		int					nTQChunkSize;							// 时间队列的每次次性分配块大小
		int					nMaxPlayerRecvCmdNum;					// 每个玩家可以发来的最多的指令数量
		// 队列相关的参数
		int					nClientWaitingNumMax;					// 处于等待状态的玩家的最多数量
		int					nClientConnectingNumMax;				// 处于连接过程中的玩家的最多数量
		int					nHelloCheckTimeOut;						// 检查玩家是否发来Hello的超时(毫秒)
		int					nCloseCheckTimeOut;						// 检查玩家关闭和CAAFS连接的超时(毫秒)。这个是在通知用户连接CLS之后启动的。
		int					nQueueSeqSendInterval;					// 向玩家发送前面还有多少人的间隔(毫秒)
		float				fWaitTransToConnectRate;				// 从等待转化为连接的速率(个/秒)
		bool				bAtLeastOneTransToConnectPerTick;		// 每个tick至少转换一个用户到连接中
		int					nQueueChannelNum;						// 队列数量
		// 不好分类的
		char				szSelfInfoToClient[256];				// 用户连接会先看到这样一句话，可以做比较看连接是否正确
		unsigned int		nSelfNotify;							// 32bit表示32个意思
		int					_nSelfInfoToClientLen;					// szSelfInfoToClient字串的长度(这个用户不能设置的)
		int					nQueuePosKeepTime;						// 队列位置保留时间(秒)，掉线多久后连接还能回到原来的排队位置
		
		CFGINFO_T()
		: nVerCmpMode(PNGS_VERCMP_MODE_EQUAL)
		, nConnectGMSTimeOut(PNGS_DFT_SVRCONNECT_TIMEOUT)
		, nSelectInterval(50)
		, nGroupID(0)
		, nTQChunkSize(100)
		, nMaxPlayerRecvCmdNum(4)
		, nClientWaitingNumMax(2000)
		, nClientConnectingNumMax(32)
		, nHelloCheckTimeOut(2000)
		, nCloseCheckTimeOut(4000)
		, nQueueSeqSendInterval(5000)
		, fWaitTransToConnectRate(16.f)
		, bAtLeastOneTransToConnectPerTick(false)
		, nQueueChannelNum(9)
		, nSelfNotify(0)
		, _nSelfInfoToClientLen(0)
		, nQueuePosKeepTime(10*60)
		{
			strcpy(szVer, "");
			strcpy(szWelcomeAddr, "localhost:3100");
			sprintf(szGMSAddr, "localhost:%d", GMS_DFT_PORT);
			strcpy(szSelfInfoToClient, PNGS_DFT_CAAFS_SELFINFO_STRING);
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// 玩家
	struct	PlayerUnit
	{
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_WAITING				= 1,						// 排队等待连接的机会
			STATUS_CONNECTINGCLS		= 2,						// CAAFS正在向CLS请求连接中
																	// Client正在和CLS建立连接中
		};
		int					nID;									// 在Players数组中的ID
		int					nStatus;								// 当前状态
		int					nSlot;									// 对应的通讯slot号
		unsigned int		IP;										// 用户的IP地址
		unsigned short		nSeq0;									// 上次通知时候的Seq0序号
		unsigned short		nSeq;									// 玩家排队的序号
		unsigned int		nPasswordToConnectCLS;					// 连接CLS用的密码
		whDList<PlayerUnit *>::node		nodeWC;						// 对应Waiting和Connecting队列，因为他只可能在这两个队列中的一个中，所以用一个node即可
		whtimequeue::ID_T	teid;
		unsigned char		nNetworkType;							// 对应的网络类型
		unsigned char		nRecvCmdNum;							// 目前收到的指令数量（不能超过指定值，否则就是外挂）
		unsigned char		nVIPChannel;							// 自己属于那个vipchannel

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nSlot(0)
		, IP(0)
		, nSeq0(0)
		, nSeq(0)
		, nPasswordToConnectCLS(0)
		, nNetworkType(NETWORKTYPE_UNKNOWN)
		, nRecvCmdNum(0)
		, nVIPChannel(0)
		{
			nodeWC.data	= this;
		}

		void	clear()
		{
			nID				= 0;
			nStatus			= STATUS_NOTHING;
			nSlot			= 0;
			IP				= 0;
			nSeq0			= 0;
			nSeq			= 0;
			nPasswordToConnectCLS		= 0;
			nodeWC.leave();											// 为了保险才这么做的。上层应该保证使用完了就leave的
			teid.quit();
			nNetworkType	= NETWORKTYPE_UNKNOWN;
			nRecvCmdNum		= 0;
			nVIPChannel		= 0;
		}
	};
	struct	TQUNIT_T
	{
		typedef	void (CAAFS2_I::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// 处理该事件的函数
		// 数据部分
		union
		{
			struct	PLAYER_T										// 玩家定时事件
			{
				int	nID;
			}player;
		}un;
	};
private:
	// 配置结构
	CFGINFO_T							m_cfginfo;					// CAAFS的配置
	tcpmsger::DATA_INI_INFO_T			m_MSGERGMS_INFO;			// 和GMS连接的msger的配置
	CNL2SlotMan::DATA_INI_INFO_T		m_CLIENT_SLOTMAN_INFO;		// 等待客户端连接的slotman的配置
	int									m_nStatus;					// CAAFS2::STATUS_XXX
	// 当前的tick时间
	whtick_t							m_tickNow;
	// 用于给进来的client进行第一次通知
	unsigned int						m_nSelfNotify;
	// 加密工厂
	ICryptFactory						*m_pICryptFactory;
	// 对Client的连接管理
	CNL2SlotMan							*m_pSlotMan4Client;
	// 时间队列
	// 比如：给各个用户多长时间通知一次前面还有多少等待的人
	// 比如：多长时间统计一次连接状态有多少人，把新的等待用户放入连接队列中
	whtimequeue							m_TQ;
	// 玩家数组
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// 排队中的玩家列表（多通道）
	struct	WAITCHANNEL_T
	{
		whDList<PlayerUnit *>			*pdlWait;
		// Waiting队列中的第一个人的序号
		unsigned short					nWaitingSeq0;
		// Waiting队列中的准备给新人的序号(每来一个新人就++)
		unsigned short					nWaitingSeqNew;

		void	Init()
		{
			WHMEMSET0THIS();
			pdlWait	= new whDList<PlayerUnit *>;
			nWaitingSeq0 = 1;
			nWaitingSeqNew = 1;
		}
		void	Release()
		{
			if( pdlWait )
			{
				delete	pdlWait;
				pdlWait	= NULL;
			}
		}
	};
	whvector<  WAITCHANNEL_T, true >	m_vectWaitChannel;
	// 连接过程中的玩家列表
	whDList<PlayerUnit *>				m_dlPlayerConnecting;
	// 处于连接状态的人数
	int									m_nConnectingNum;
	// 连接GMS的msger
	class	MYMSGER_T	: public tcpretrymsger<tcpmsger>
	{
	public:
		CAAFS2_I		*m_pHost;
		whtick_t		m_tickWorkBegin;
	public:
		virtual void	i_Worker_TRYCONNECT_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_TRYCONNECT_Begin();
		}
		virtual void	i_Worker_WORKING_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	friend class MYMSGER_T;
	MYMSGER_T							m_msgerGMS;
	// GMS还能够承受的玩家数量
	int									m_nGMSAvailPlayerNum;
	// 用于生成每个tick转换到connecting队列的个数
	whgener								m_generWaitTransToConnect;
	// 用于获得每个tick的时间间隔
	whinterval							m_itv;
	// 字串指令处理相关的
	whcmn_strcmd_reganddeal_DECLARE_m_srad(CAAFS2_I);
	// DealStrCmd处理后的结果会放在这里
	char								m_szRstStr[GZS_STRCMD_RST_LEN_MAX];
	// 告诉上层应该退出
	bool								m_bShouldStop;
	// 是否是重连的
	bool								m_bIsReconnect;
	// 临时生成指令
	whvector<char>						m_vectrawbuf;

	/* 
	排队掉线后恢复队列位置用的
	*/
	struct ResumedPlayerUnit{
		unsigned short nSeq;
		unsigned short nSeq0;
		unsigned char nVIPChannel;
		unsigned int  nLeaveTime;
		ResumedPlayerUnit():nSeq(0),nSeq0(0),nVIPChannel(0),nLeaveTime(0){};

	};
	map<string,ResumedPlayerUnit> m_wait2resume_map;
	map<int,string> m_clientID2Account_map;
	whtimequeue::ID_T m_resume_map_tid;
	
private:
	// 把玩家加入Waiting队列(玩家连入之后应该马上被放入Waiting队列)
	int		AddPlayerToWaitingList(PlayerUnit *pPlayer);
	// 把玩家转移到Connecting队列
	int		AddPlayerToConnectingList(PlayerUnit *pPlayer);
	// 把玩家移出当前队列
	int		DelPlayerFromList(PlayerUnit *pPlayer);
	// 设置当前状态
	void	SetStatus(int nStatus);
	// 移除Player
	int		RemovePlayerUnit(int nPlayerID);
	int		RemovePlayerUnit(PlayerUnit *pPlayer);
	// 通过Slot获得Player
	PlayerUnit *	GetPlayerBySlot(int nSlot);
	// 生成连接CLS的密码
	unsigned int	GenPasswordToConnectCLS();
	// 把玩家从Wait转移到Connecting队列
	int		TransPlayerFromWaitToConnectQueue(PlayerUnit *pPlayer);
	// Init子函数
	int		Init_CFG(const char *cszCFG);
	// 各种TickWork
	int		Tick_DealGMSMsg();
	int		Tick_DealGMSMsg_GMS_CAAFS_CTRL(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CAAFS_GMSINFO(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CAAFS_CLIENTCANGOTOCLS(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CAAFS_KEEP_QUEUE_POS(void *pCmd,	int nSize);
	int		Tick_DealClientMsg();
	int		Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize);
	int		Tick_DealClientMsg_CONTROL_OUT_SLOT_CLOSED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize);
	int		Tick_DealQueue();
	int		Tick_DealTE();
	// 各种TEDeal函数
	// 等待用户say hello的超时
	void	TEDeal_HELLO_TimeOut(TQUNIT_T * pTQUnit);
	// 等待用户关闭连接的超时
	void	TEDeal_Close_TimeOut(TQUNIT_T * pTQUnit);
	// 发送前面队列人数的超时
	void	TEDeal_QueueSeq(TQUNIT_T * pTQUnit);
	// 设置上面的事件
	void	SetTE_QueueSeq(PlayerUnit *pPlayer, bool bRand=false);
	// 前往CLS的过程中超时
	void	TEDeal_ClientGoCLSTimeOut(TQUNIT_T * pTQUnit);
	////////////////////////////////////////////////////////////////
	// 字串指令处理
	////////////////////////////////////////////////////////////////
	// 载入指令和函数对应关系
	int		InitStrCmdMap();
	// 处理字串指令，返回在m_szRstStr中
	int		DealStrCmd(const char *cszStrCmd);
	// 具体的字串指令处理函数
	int		_i_srad_NULL(const char *param);
	int		_i_srad_setver(const char *param);

	// 重连开始时需要调用的函数
	void	Retry_Worker_TRYCONNECT_Begin();
	// 当重连成功时需要调用的函数
	void	Retry_Worker_WORKING_Begin();

	// 把玩家放入一个vipchannel
	bool	PutPlayerInVIPChannel(PlayerUnit *pPlayer, unsigned char nVIPChannel);
	// 发送玩家当前的排队信息
	void	SendPlayerQueueInfo(PlayerUnit *pPlayer, bool bMustSend);

	// 处理带有帐号的排队请求
	void	DealKeepQueuePosReq(PlayerUnit	*pPlayer,const char * szAccount); 
	// 恢复玩家在队列里的位置，应该是排队的时候掉线了
	bool    ResumePlayerPosInVIPChannel(PlayerUnit *pPlayer, unsigned char nVIPChannel);
	// 打印全部队列信息（仅供测试）
	void    DisplayQueueInfo();
	// 处理排队掉线的超时
	void	TEDeal_ResumeMapTimeOut(TQUNIT_T * pTQUnit);
	// 设置排队掉线的超时
	void	SetTE_ResumeMapTimeOut();
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CAAFS_I_H__
