//////////////////////////////////////////////////////////////////////////
// web扩展连接分配服务器
// 移动扩展也需要使用
// 对应的CLS是指CLS4Web
// 1.prelogin暂时废弃
//////////////////////////////////////////////////////////////////////////
#ifndef __CAAFS4Web_i_H__
#define __CAAFS4Web_i_H__

#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_packet_logic.h"

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whgener.h"
#include "WHCMN/inc/whtimequeue.h"

#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnetepoll.h"
#include "WHNET/inc/whnettcp.h"

#include "../../Common/inc/pngs_packet_web_extension.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class CAAFS4Web_i : public CMN::ILogic
{
//ILogic接口
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		SureSend();
private:
	virtual	int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		Tick_BeforeDealCmdIn();
	virtual	int		Tick_AfterDealCmdIn();
//自己的东西
public:
	enum
	{
		STATUS_ERR_NETWORK		= -1,			// 工作中网络出错
		STATUS_NOTHING			= 0,
		STATUS_WORKING			= 1,			// 连接成功,工作中
	};
public:
	CAAFS4Web_i();
	virtual	~CAAFS4Web_i();
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		// 基本参数
		int				nVerCmpMode;							// 取值为PNGS_VERCMP_MODE_XXX,见pngs_cmn.h
		char			szVer[PNGS_VER_LEN];					// 版本
		char			szWelcomeAddr[WHNET_MAXADDRSTRLEN];		// 用户看到的服务器地址
		char			szLogicProxyAddr[WHNET_MAXADDRSTRLEN];	// 代理服务器地址
		int				nConnectLogicProxyTimeout;				// 连接逻辑代理服务器的超时
		int				nSelectInterval;						// tick中做select的间隔(ms)
		int				nGroupID;								// 组ID,和CLS对应
		int				nTQChunkSize;							// 时间队列每次性分配块大小
		int				nMaxPlayerRecvCmdNum;					// 每个玩家可以发来的最多的指令数量
		// 队列相关参数
		int				nClientWaitingNumMax;					// 处于等待状态的玩家的最多数量
		int				nClientConnectingNumMax;				// 处于连接过程中的玩家的最多数量
		int				nHelloCheckTimeout;						// 检查玩家是否发来hello的超时(ms)
		int				nCloseCheckTimeout;						// 检查玩家关闭和CAAFS连接的超时(ms)
		int				nQueueSeqSendInterval;					// 向玩家发送前面还有多少人的间隔(ms)
		float			fWaitTransToConnectRate;				// 从等待转化为连接的速率(个/s)
		bool			bAtLeastOnTransToConnectPerTick;		// 每个tick至少转换一个用户到连接中
		int				nQueueChannelNum;						// 队列数量
		// 不好分类的
		char			szSelfInfoToClient[256];				// 用户连接会先看到这样一句话,可以做比较看连接是否正确
		unsigned int	nSelfNotify;							// 32bit表示32个意思
		int				_nSelfInfoToClientLen;					// szSelfInfoToClient字串的长度
	
		CFGINFO_T()
		: nVerCmpMode(PNGS_VERCMP_MODE_EQUAL)
		, nConnectLogicProxyTimeout(PNGS_DFT_SVRCONNECT_TIMEOUT)
		, nSelectInterval(50)
		, nGroupID(0)
		, nTQChunkSize(100)
		, nMaxPlayerRecvCmdNum(4)
		, nClientWaitingNumMax(2000)
		, nClientConnectingNumMax(256)
		, nHelloCheckTimeout(2000)
		, nCloseCheckTimeout(1000)
		, nQueueSeqSendInterval(5000)
		, fWaitTransToConnectRate(32.f)
		, bAtLeastOnTransToConnectPerTick(false)
		, nQueueChannelNum(9)
		, nSelfNotify(0)
		{
			strcpy(szVer, "");
			strcpy(szWelcomeAddr, "localhost:4200");
			strcpy(szLogicProxyAddr, "localhost:4400");
			strcpy(szSelfInfoToClient, "caafs4web");
			_nSelfInfoToClientLen	= strlen(szSelfInfoToClient)+1;
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// 玩家
	struct PlayerUnit 
	{
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_WAITING				= 1,				// 排队等待连接的机会
			STATUS_CONNECTINGCLS4Web		= 2,				// CAAFS正在向CLS请求连接中,Client正在和CLS建立连接中
		};
		int					nID;							// 在Players数组中的ID
		int					nStatus;						// 当前状态
		int					nCntrID;						// 对应的通讯connecter号
		unsigned int		IP;								// 用户的IP地址
		unsigned short		nPort;							// 用户的端口
		unsigned short		nSeq0;							// 上次通知时候的Seq0序号
		unsigned short		nSeq;							// 玩家排队的序号
		unsigned int		nPasswordToConnectCLS4Web;			// 连接CLS用的密码
		whDList<PlayerUnit*>::node	nodeWC;					// 对应waiting和connecting队列
		whtimequeue::ID_T	teid;
		unsigned char		nRecvCmdNum;					// 目前收到的指令数量(不能超过指定值)
		unsigned char		nChannel;						// 自己所属的channel
		int					nTermType;						// 使用的终端类型

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nCntrID(0)
		, IP(0)
		, nSeq0(0)
		, nSeq(0)
		, nPasswordToConnectCLS4Web(0)
		, nRecvCmdNum(0)
		, nTermType(TERM_TYPE_NONE)
		{
			nodeWC.data		= this;
		}

		void	clear()
		{
			nID				= 0;
			nStatus			= STATUS_NOTHING;
			nCntrID			= 0;
			IP				= 0;
			nSeq0			= 0;
			nSeq			= 0;
			nPasswordToConnectCLS4Web		= 0;
			nodeWC.leave();
			teid.quit();
			nRecvCmdNum		= 0;
		}
	};
	struct TQUNIT_T 
	{
		typedef	void (CAAFS4Web_i::*TEDeal_T)(TQUNIT_T*);
		TEDeal_T	tefunc;										// 事件处理函数
		// 数据部分
		union
		{
			struct PLAYER_T										// 玩家定时事件
			{
				int		nID;
			}player;
		}un;
	};
private:
	// 配置结构
	CFGINFO_T								m_cfginfo;
	tcpmsger::DATA_INI_INFO_T				m_MsgerLogicProxy_info;
	epoll_server::info_T					m_epollServerInfo;
	int										m_nStatus;
	whtick_t								m_tickNow;
	unsigned int							m_nSelfNotify;
	epoll_server*							m_pEpollServer;
	whtimequeue								m_TQ;
	whunitallocatorFixed<PlayerUnit>		m_Players;
	// 排队中的玩家列表
	struct WAITCHANNEL_T 
	{
		whDList<PlayerUnit*>*				pdlWait;
		unsigned short						nWaitingSeq0;
		unsigned short						nWaitingSeqNew;

		void	Init()
		{
			WHMEMSET0THIS();
			pdlWait		= new whDList<PlayerUnit*>;
		}
		void	Release()
		{
			WHSafeDelete(pdlWait);
		}
	};
	// 这个是否为cpp没多大意义,因为WAITCHANNEL_T没有自定义构造和析构函数
	whvector<WAITCHANNEL_T, true>			m_vectWaitChannel;
	whDList<PlayerUnit*>					m_dlPlayerConnecting;
	int										m_nConnectingNum;
	class MYMSGER_T : public tcpretrymsger<tcpmsger>
	{
	public:
		CAAFS4Web_i*		m_pHost;
		whtick_t			m_tickWorkBegin;
	public:
		virtual	void	i_Worker_TRYCONNECT_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_TRYCONNECT_Begin();
		}
		virtual	void	i_Worker_WORKING_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	friend class	MYMSGER_T;
	MYMSGER_T								m_msgerLogicProxy;
	int										m_nLogicProxyAvailPlayerNum;
	whgener									m_generWaitTransToConnect;
	whinterval								m_itv;
	bool									m_bShouldStop;
	bool									m_bIsReconnect;
	whvector<char>							m_vectrawbuf;
private:
	// 设置当前状态
	void	SetStatus(int nStatus);
	// 获得当前状态
	int		GetStatus() const;
	// 移除player
	int		RemovePlayerUnit(int nPlayerID);
	int		RemovePlayerUnit(PlayerUnit* pPlayer);
	// 通过cntrid获得Player
	PlayerUnit*		GetPlayerByCntrID(int iCntrID);
	// 生成连接CLS的密码
	unsigned int	GenPasswordToConnectCLS4Web();
	// 把玩家从wait转移到connecting队列
	int		TransPlayerFromWaitToConnectQueue(PlayerUnit* pPlayer);
	// Init子函数
	int		Init_CFG(const char* cszCFG);
	// 各种tickwork
	int		Tick_DealLogicProxyMsg();
	int		Tick_DealLogicProxyMsg_LP_CAAFS4Web_CTRL(void* pCmd, int nSize);
	int		Tick_DealLogicProxyMsg_LP_CAAFS4Web_LPINFO(void* pCmd, int nSize);
	int		Tick_DealLogicProxyMsg_LP_CAAFS4Web_CLIENTCANGOTOCLS4Web(void* pCmd, int nSize);
	int		Tick_DealClientMsg();
	int		Tick_DealClientMsg_CONTROL_OUT_CNTR_ACCEPTED(epoll_server::ctrl_out_T* pCOCmd, int nSize);
	int		Tick_DealClientMsg_CONTROL_OUT_CNTR_CLOSED(epoll_server::ctrl_out_T* pCOCmd, int nSize);
	int		TIck_DealClientMsg_CLIENT_CLS4Web_DATA(PlayerUnit* pPlayer, pngs_cmd_t nCmd, pngs_cmd_t* pCmd, int nSize);
	int		Tick_DealQueue();
	int		Tick_DealTE();
	int		Tick_DoMyOwnWork();				// 就是tick的替代品,Tick不是virtual方法
	// 各种TEDeal函数
	// 等待用户say hello的超时
	void	TEDeal_HELLO_TimeOut(TQUNIT_T* pTQUnit);
	// 等待用户关闭连接的超时
	void	TEDeal_Close_TimeOut(TQUNIT_T* pTQUnit);
	// 发送前面队列人数的超时
	void	TEDeal_QueueSeq(TQUNIT_T* pTQUnit);
	// 设置上面的事件
	void	SetTE_QueueSeq(PlayerUnit* pPlayer, bool bRand=false);
	// 前往CLS过程中的超时
	void	TEDeal_ClientGoCLSTimeOut(TQUNIT_T* pTQUnit);
	// 重新开始时需要调用的函数
	void	Retry_Worker_TRYCONNECT_Begin();
	// 当重连成功时需要调用的函数
	void	Retry_Worker_WORKING_Begin();
	// 发送玩家当前的排队信息
	void	SendPlayerQueueInfo(PlayerUnit* pPlayer, bool bMustSend);
	// 把玩家放入一个channel
	bool	PutPlayerInChannel(PlayerUnit* pPlayer, unsigned char nChannel);
};
}

#endif
