//////////////////////////////////////////////////////////////////////////
//web扩展连接负载服务器
//移动扩展也需要使用
//对应的CAAFS是指CAAFS4Web
//添加延迟发送机制(2012-01-06),实际上如果把epoll_server改成connecter统一使用一个buf做缓冲(所有connecter都从该buf申请缓冲),就不需要这个延迟发送机制了
//////////////////////////////////////////////////////////////////////////
#ifndef __CLS4Web_i_H__
#define __CLS4Web_i_H__

#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_packet_logic.h"

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whtimequeue.h"

#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnetepoll.h"
#include "WHNET/inc/whnettcp.h"

#include "../../Common/inc/pngs_packet_web_extension.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class CLS4Web_i : public CMN::ILogic
{
// ILogic接口
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char* cszCFGName);
	virtual	int		Init_CFG_Reload(const char* cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(whvector<SOCKET>& vect);
	virtual	int		SureSend();
private:
	virtual	int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		Tick_BeforeDealCmdIn();
	virtual	int		Tick_AfterDealCmdIn();
// 自己的东西
public:
	enum
	{
		STATUS_ERR_NETWORK			= -1,			// 工作中网络出错
		STATUS_NOTHING				= 0,
		STATUS_WORKING				= 1,			// 连接成功,工作中
		STATUS_KICK_ALL_PLAYER		= 2,			// 正在应LP4Web的要求踢帐号下线中
	};
public:
	CLS4Web_i();
	virtual	~CLS4Web_i();
public:
	// 初始化配置
	struct CFGINFO_T : public whdataini::obj 
	{
		char			szWelcomeAddr[WHNET_MAXADDRSTRLEN];				// 监听地址
		char			szLogicProxyAddr[WHNET_MAXADDRSTRLEN];			// gms的连接地址
		int				nGroupID;										// 和相应的	CAAFS对应
		int				nTQChunkSize;									// 时间队列每次分配块大小
		int				nConnectOtherSvrTimeOut;						// 连接其他服务器的超时
		int				nCloseFromLPTimeOut;							// 通知LP自己要退出后,等待LP关闭连接的超时
		int				nHelloTimeOut;									// 等待连接发来的hello的超时
		int				nClientConnectToMeTimeOut;						// 用户连接CLS的超时(这么长没连接成功就超时)
		int				nCntrSendRecvBuf;								// 每个连接的send和recvbuf的大小
		int				nCmdStatInterval;								// 进行指令统计的间隔(<=0表示不统计)
		char			szSelfInfoToClient[256];						// 用户连接会先看到这样一句话,可以做比较看连接是否正确
		int				nMaxRecvCount;									// 一段时间内最多可接收的包数
		int				nMaxRecvSize;									// 一段时间内最多可接收的数据量
		int				nCheckRecvInterval;								// 检查上面两个量的间隔
		int				nBigTickFilter;									// big tick log输出的阀值
	
		CFGINFO_T()
		: nGroupID(0)
		, nTQChunkSize(1024)
		, nConnectOtherSvrTimeOut(PNGS_DFT_SVRCONNECT_TIMEOUT)
		, nCloseFromLPTimeOut(10*1000)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nClientConnectToMeTimeOut(30*1000)
		, nCntrSendRecvBuf(64*1024)
		, nCmdStatInterval(20*1000)
		, nMaxRecvCount(5*60*3)											// 平均每秒3个包
		, nMaxRecvSize(5*60*4*1024)										// 平均每秒4k数据量
		, nCheckRecvInterval(5*60*1000)									// 默认五分钟统计一次
		, nBigTickFilter(150)
		{
			strcpy(szWelcomeAddr, "localhost:15101,15110");
			strcpy(szLogicProxyAddr, "localhost:4400");
			strcpy(szSelfInfoToClient, "CLS24Web");
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// 玩家
	struct	PlayerUnit;
	typedef	whDList<PlayerUnit*>		DLPLAYER_T;
	typedef	DLPLAYER_T::node			DLPLAYER_NODE_T;
	struct	PlayerUnit 
	{
		enum
		{
			MAX_TAG_NUM					= 16,							// 最多可以标记的tag数量
		};
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_CONNECTING			= 1,							// 正在连接过程中
			STATUS_WORKING				= 2,							// 连接成功,正常通讯中
		};
		int						nID;									// 在Players数组中的ID(这个是由LP中的ID决定的)
		char					szName[64];								// 名字标志(这个由上层设置,比如:帐号.角色,主要是为了关键性日志能够反映出用户名,便于将来查询)
		int						nStatus;								// 当前状态
		int						nCntrID;								// 对应的连接ID
		unsigned int			nPasswordToConnectCLS4Web;				// 连接CLS用的密码
		unsigned char			nRemoveReason;							// CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_XXX
		unsigned char			nProp;									// 属性,定义为PNGS_CLS_PLAYER_PROP_LOG_XXX,在pngs_def.h中
		unsigned char			nSvrIdx;								// 对应的GZS序号(这个可以用来做和真实GZS中的用户交互时使用,暂不使用)
		int						nRecvCount;								// 在一段时间内收到的包的数量
		int						nRecvSize;								// 在一段时间内收到的数据量
		whtimequeue::ID_T		teid;									// 目前用途:用户连入超时,每隔一段时间检测是否发送过多的包

		int						nTermType;								// 终端类型

		struct TAGINFO_T 
		{
			short				nValue;									// 对应的tag值(0表示不在任何队列中)
			DLPLAYER_NODE_T		nodeTag;								// 在相应的tag队列中的节点

			TAGINFO_T()
			: nValue(0)
			{
			}

			void clear()
			{
				nValue	= 0;
				nodeTag.leave();
			}
		};
		TAGINFO_T				taginfo[MAX_TAG_NUM];
		whhash<whuint64, DLPLAYER_NODE_T*>	map64Tag2Node;

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nCntrID(0)
		, nPasswordToConnectCLS4Web(0)
		, nRemoveReason(CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING)
		, nProp(0)
		, nSvrIdx(0)
		, nTermType(TERM_TYPE_NONE)
		{
			szName[0]			= 0;
			for (int i=0; i<MAX_TAG_NUM; i++)
			{
				taginfo[i].nodeTag.data		= this;
			}
		}

		void	clear(CLS4Web_i* pCLS)
		{
			for (int i=0; i<MAX_TAG_NUM; i++)
			{
				pCLS->DelPlayerFromTagList(this, i);
			}
			whhash<whuint64, DLPLAYER_NODE_T*>::kv_iterator	it	= map64Tag2Node.begin();
			while (it != map64Tag2Node.end())
			{
				whuint64	nTag	= it.getkey();
				++it;
				pCLS->DelPlayerFromTag64List(this, nTag);
			}

			teid.quit();

			nID					= 0;
			nSvrIdx				= 0;
			nStatus				= STATUS_NOTHING;
			nCntrID				= 0;
			nPasswordToConnectCLS4Web	= 0;
			nRemoveReason				= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING;
			nProp				= 0;
			szName[0]			= 0;
			nRecvCount			= 0;
			nRecvSize			= 0;
		}
		void	ReadFromOther(PlayerUnit* pOther)
		{
			//nID不需要
			memcpy(szName, pOther->szName, sizeof(szName));
			nStatus				= pOther->nStatus;
			nCntrID				= pOther->nCntrID;
			nPasswordToConnectCLS4Web	= pOther->nPasswordToConnectCLS4Web;
			nRemoveReason		= pOther->nRemoveReason;
			nProp				= pOther->nProp;
			nSvrIdx				= pOther->nSvrIdx;
			teid.quit();		// 这个不用读取别人的
			for (int i=0; i<MAX_TAG_NUM; i++)
			{
				if (pOther->taginfo[i].nodeTag.isinlist())
				{
					taginfo[i].nodeTag.insertbefore(&pOther->taginfo[i].nodeTag);
				}
			}
		}
	};
	struct	TQUNIT_T;
	typedef	void	(CLS4Web_i::*TEDEAL_T)(TQUNIT_T*);
	struct	TQUNIT_T 
	{
		TEDEAL_T	tefunc;
		// 数据部分
		union
		{
			struct PLAYER_T						// 玩家的定时事件
			{
				int	nID;						// 玩家ID
			}player;
			struct CNTR_T						// 连接ID
			{
				int	nID;
			}cntr;
		}un;
	};
	struct	DLPUNIT_T 
	{
		DLPLAYER_T*	pDLP;

		DLPUNIT_T()
		: pDLP(NULL)
		{
		}

		void	clear(CLS4Web_i* pCLS)
		{
			pCLS->SafeDeleteDLP(pDLP);
		}
	};
private:
	friend	struct	PlayerUnit;
	friend	struct	DLPUNIT_T;
	// 配置
	CFGINFO_T							m_cfginfo;						// CLS4Web的配置
	tcpmsger::DATA_INI_INFO_T			m_MSGER_INFO;					// 和LP连接的msger配置
	epoll_server::info_T				m_epollServerInfo;
	// 当前的工作状态
	int									m_nStatus;
	// 当前的时刻
	whtick_t							m_tickNow;
	// 自己在LP中的ID(在重连的时候有用)
	int									m_nCLS4WebID;
	// epoll连接管理
	epoll_server*						m_pEpollServer;
	// 时间队列
	whtimequeue							m_TQ;
	// 各个玩家
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// 连接LP的msger
	class	MYMSGER_T : public tcpretrymsger<tcpmsger>
	{
	public:
		CLS4Web_i*		m_pHost;
		whtick_t		m_tickWorkBegin;
	public:
		virtual	void	i_Worker_TRYCONNECT_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
		}
		virtual	void	i_Worker_WORKING_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	friend	class	MYMSGER_T;
	MYMSGER_T							m_msgerLP;
	// 用于删除出错的player
	whvector<int>						m_vectIDToDestroy;
	// tag到相应的DList的映射
	whhash<unsigned int, DLPUNIT_T>		m_mapTag2DL;
	// 64位的映射
	whhash<whuint64, DLPUNIT_T>			m_map64Tag2DL;
	// 临时组装指令
	whvector<char>						m_vectrawbuf;
	// 告诉上层应该退出
	bool								m_bShouldStop;
	// 用于统计指令发送频率
	struct CMDSTAT_T 
	{
		unsigned int		nByte;		// 累计字节数
		unsigned int		nCount;		// 累计条数

		inline	bool	operator < (const CMDSTAT_T& other) const
		{
			if (nByte != other.nByte)
			{
				return nByte > other.nByte;
			}
			return nCount > other.nCount;
		}
	};
	whvector<CMDSTAT_T>					m_vectCmdStat;
	// 用于定时检查输出log
	whlooper							m_loopCmdStat;
private:
	bool	ShouldStop() const;
	// 设置当前的状态
	void	SetStatus(int nStatus);
	// 获得当前状态
	int		GetStatus() const;
	// 移除player
	int		RemovePlayerUnit(int nPlayerID);
	int		RemovePlayerUnit(PlayerUnit* pPlayer);
	// 通过cntrID获得Player
	PlayerUnit*		GetPlayerByCntrID(int nCntrID);
	// 尝试获取玩家,如果不存在就向LP发送用户下线的通知
	PlayerUnit*		TryGetPlayerAndSendNotifyToLPIsNotExisted(int nPlayerID);
	// 向单个玩家发送数据(里面需要把数据合成CLS4Web_CLIENT_DATA_T的形式,然后再调用SendRawCmdToSinglePlayer)
	int		SendDataToSinglePlayer(PlayerUnit* pPlayer, void* pData, int nSize, pngs_cmd_t nSendCmd);
	// 向单个玩家发送纯指令
	int		SendRawCmdToSinglePlayer(int nPlayerID, void* pRawCmd, int nSize);
	int		SendRawCmdToSinglePlayer(PlayerUnit* pPlayer, void* pRawCmd, int nSize);
	// 向所有玩家发送纯指令
	int		SendRawCmdToAllPlayer(void* pRawCmd, int nSize);
	// 根据idx和val合成一个hash键值
	inline	unsigned int		CreateHashKey(unsigned char nTagIdx, short nTagVal)
	{
		return (((unsigned int)nTagIdx)<<16) | nTagVal;
	}
	// 从hash键值中得到idx和val
	inline	void	GetIdxAndVal(unsigned int nHashKey, unsigned char* pnTagIdx, short* pnTagVal)
	{
		*pnTagIdx		= (unsigned char)((nHashKey & 0xFF0000)>>16);
		*pnTagVal		= (short)(nHashKey&0xFFFF);
	}
	// 把玩家加入某个tag队列中
	int		AddPlayerToTagList(PlayerUnit* pPlayer, unsigned char nTagIdx, short nTagVal);
	// 把玩家从某个tag队列中移出来
	int		DelPlayerFromTagList(PlayerUnit* pPlayer, unsigned char nTagIdx);
	// 把玩家加入某个tag64队列中
	int		AddPlayerToTag64List(PlayerUnit* pPlayer, whuint64 nTag);
	// 把玩家从某个tag64队列中移出来
	int		DelPlayerFromTag64List(PlayerUnit* pPlayer, whuint64 nTag);

	// New一个DLPLAYER_T对象
	DLPLAYER_T*		NewDLP();
	// Delete一个DLPLAYER_T对象
	void	SafeDeleteDLP(DLPLAYER_T*& pDLP);
	// Init子函数
	int		Init_CFG(const char* cszCFG);
	// 各种tickwork
	int		Tick_DealLogicProxyMsg();
	int		Tick_DealLogicProxyMsg_LP_CLS4Web_CLIENTWANTCLS4Web(const char* pCmd, size_t nSize);
	
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_CTRL(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_CLIENT_DATA(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_MULTICLIENT_DATA(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_SET_TAG_TO_CLIENT(void* pCmd, int nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_TAGGED_CLIENT_DATA(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_ALL_CLIENT_DATA(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_SET_TAG64_TO_CLIENT(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_TAGGED64_CLIENT_DATA(void* pCmd, size_t nSize);
	
	int		Tick_DealClientMsg();
	int		Tick_DealClientMsg_CONTROL_OUT_CNTR_ACCEPTED(epoll_server::ctrl_out_T* pCOCmd, int nSize);
	int		Tick_DealClientMsg_CONTROL_OUT_CNTR_CLOSED(epoll_server::ctrl_out_T* pCOCmd, int nSize);
	int		Tick_DealClientMsg_CLIENT_DATA(pngs_cmd_t nCmd, PlayerUnit* pPlayer, pngs_cmd_t* pCmd, int nSize);
	int		Tick_DealTE();
	int		Tick_DoMyOwnWork();
	// 公共数据交换

	// 当重连成功时需要调用的函数
	void	Retry_Worker_WORKING_Begin();

	// 时间事件处理
	// 客户端连接超时
	void	TEDeal_ClientConnnectToMeTimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的
	void	SetTEDeal_ClientConnectToMeTimeOut(PlayerUnit* pPlayer);
	// 定时检查用户发来的包的频率
	void	TEDeal_CheckRecvInterval(TQUNIT_T* pTQUnit);
	// 设置上面的
	void	SetTEDeal_CheckRecvInterval(PlayerUnit* pPlayer);
};
}

#endif
