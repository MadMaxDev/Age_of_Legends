// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的CLS模块
//                PNGS是Pixel Network Game Structure的缩写
//                CLS是Connection Load Server的缩写，是连接分配功能中的负载服务器
//                本文件是CLS的内部实现
//                CLS和客户端通讯使用cnl2。和服务器间通讯使用TCP。
//                用户Tag使用idx+val的hash表实现，同tag+val的检索速度比旧的CLS要快。
// CreationDate : 2005-07-22
// ChangeLOG    : 2006-12-19 增加了szFakeWelcomeAddr

#ifndef	__PNGS_CLS_I_H__
#define	__PNGS_CLS_I_H__
#define GZSTICKFUNC 15
#include "./pngs_cls.h"
#include "./pngs_packet.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whnetcnl2.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whlist.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whdbg.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

class	CLS2_I		: public CLS2
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
// 自己使用的
public:
	CLS2_I();
	~CLS2_I();
private:
	// 各种结构定义
	// 初始化参数
	struct	CFGINFO_T		: public whdataini::obj
	{
		char				szWelcomeAddr[WHNET_MAXADDRSTRLEN];		// 给用户来连接的服务器地址
		char				szFakeWelcomeAddr[WHNET_MAXADDRSTRLEN];	// 虚假的欢迎地址（如果非空则把这个发给用户。一般用于转发测试或者代理）
																	// 地址的格式为“IP:deltaport”，
																	// 比如“IP2:0”就表示得到自己确切绑定的地址后，修改IP为IP2，端口不变
																	// 比如“:1000”就表示得到自己确切绑定的地址后，IP不变，端口都加上1000
		char				szGMSAddr[WHNET_MAXADDRSTRLEN];			// GMS等待其他服务器连接地址(所有服务器连接GMS的地址都是这个)
		unsigned char		nNetworkType;							// 一个组内也可以是对应若干不同的网络的（比如，一些CLS对应网通、一些CLS对应电信）
		bool				bCanAcceptPlayerWhenCLSConnectingGZS;	// 在CLS连接GZS的过程中允许用户连接
		bool				bLogSvrCmd;								// 是否记录服务器发来的指令
		int					nGroupID;								// 和相应的CAAFS对应
		int					nTQChunkSize;							// 时间队列的每次次性分配块大小
		int					nConnectOtherSvrTimeOut;				// 连接GMS、GZS等其他Server的超时
		int					nCloseFromGMSTimeOut;					// 通知GMS自己要退出后，等待GMS关闭连接的超时
		int					nHelloTimeOut;							// 等待连接发来Hello或者Hi的超时
		int					nClientConnectToMeTimeOut;				// 用户连接CLS的超时(这么长时间没有连接成功就超时)
		int					nSlotInOutNum;							// 各个slot的In/Out数量是一样的
		int					nCmdStatInterval;						// 进行指令统计的间隔（如果为0则表示不用统计）
		//
		char				szSelfInfoToClient[256];				// 用户连接会先看到这样一句话，可以做比较看连接是否正确
		// 反外挂
		int					nMaxRecvCount;							// 一段时间内最多可接收的包数
		int					nMaxRecvSize;							// 一段时间内最多可接收的数据量
		int					nCheckRecvInterval;						// 检查上面两个量的间隔
		int					nBigTickFilter;							// BIGtick log输出的阀值
		CFGINFO_T()
		: nNetworkType(NETWORKTYPE_UNKNOWN)
		, bCanAcceptPlayerWhenCLSConnectingGZS(false)
		, bLogSvrCmd(false)
		, nGroupID(0)
		, nTQChunkSize(1024)
		, nConnectOtherSvrTimeOut(PNGS_DFT_SVRCONNECT_TIMEOUT)		// 这个长一些是为了怕服务器初始化可能会浪费一些时间
		, nCloseFromGMSTimeOut(10*1000)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nClientConnectToMeTimeOut(30000)
		, nSlotInOutNum(PNGS_SLOTINOUTNUM)
		, nCmdStatInterval(20*1000)
		, nMaxRecvCount(5*60*3)										// 平均每秒不应该超过3个包
		, nMaxRecvSize(5*60*2*1024)									// 假定最多每秒2k的数据吧
		, nCheckRecvInterval(5*60*1000)								// 默认5分钟统计一次
		, nBigTickFilter(150)
		{
			strcpy(szWelcomeAddr, "localhost:15901,15910");
			szFakeWelcomeAddr[0]	= 0;
			sprintf(szGMSAddr, "localhost:%d", GMS_DFT_PORT);
			strcpy(szSelfInfoToClient, "BO2 CLS2");
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// 对应一个GZS连接
	struct	GZSUnit
	{
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_CONNECTING			= 1,						// 正在连接该服务器的过程中（如果长时间没有连接上则DROP该用户）
			STATUS_SHAKEHAND			= 2,						// Hello/Hi中
			STATUS_WORKING				= 3,						// 连接成功，正常通讯中
		};
		int					nStatus;								// 当前状态
		unsigned char		nSvrIdx;								// GZS服务器序号(1~GZS_MAX_NUM-1)
		SOCKET				sockGZS;								// 连接GZS的socket
		whtick_t			tickConnectGZSTimeOut;					// 连接GZS超时的时刻
																	// 因为DoAllGZSRecv中反正也要轮询一遍GZS，所以就索性在那里面判断超时了
		tcpmsger			msgerGZS;								// 连接GZS的msger

		GZSUnit()
		: nStatus(STATUS_NOTHING)
		, nSvrIdx(0)												// 0则表示还没有对应的真正GZS呢
		, sockGZS(INVALID_SOCKET)
		, tickConnectGZSTimeOut(0)
		{
		}

		void	clear()
		{
			nStatus		= STATUS_NOTHING;
			nSvrIdx		= 0;
			cmn_safeclosesocket(sockGZS);							// 为了保险关闭一下
			tickConnectGZSTimeOut	= 0;
			msgerGZS.Release();										// 为了保险释放一下
		}
	};
	// 玩家
	struct	PlayerUnit;
	typedef	whDList<PlayerUnit *>		DLPLAYER_T;
	typedef	DLPLAYER_T::node			DLPLAYER_NODE_T;
	struct	PlayerUnit
	{
		enum
		{
			MAX_TAG_NUM					= 16,						// 最多可以标记的tag数量
		};
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_CONNECTING			= 1,						// 正在连接过程中（如果长时间没有连接上则DROP该用户）
			STATUS_WORKING				= 2,						// 连接成功，正常通讯中
		};
		int					nID;									// 在Players数组中的ID（这个是根据GMS中的ID决定的）
		char				szName[64];								// 名字标志（这个由上层设置，比如：账号.角色，主要是为了关键性日志能够反应出用户名，便于将来查询）
		int					nStatus;								// 当前状态
		int					nSlot;									// 对应的通讯slot号
		unsigned int		nPasswordToConnectCLS;					// 连接CLS用的密码（0表示没有密码）
		unsigned char		nRemoveReason;							// CLS_GMS_CLIENT_DROP_T::REMOVEREASON_XXX
		unsigned char		nProp;									// 属性。定义为PNGS_CLS_PLAYER_PROP_LOG_XXX，在pngs_def.h中
		unsigned char		nSvrIdx;								// 对应的GZS序号
																	// 如果是0则表示还没有进入GZS（即还在GMS中）
																	// 在某个GZS和CLS断线的时候，这个数据可以用来踢出所有和GZS相关的用户
																	// !!!!这个是必须的，因为不需要让用户知道自己现在处于那个GZS
		int					nRecvCount;								// 在一段时间内收到的包的数量（用于反外挂）
		int					nRecvSize;								// 在一段时间内收到的数据量（用于反外挂）
		whtimequeue::ID_T	teid;									// 目前用途：用户连入超时；每隔一段时间检查一下是否发送过多的包

		struct	TAGINFO_T
		{
			short			nValue;									// 对应的tag值(0表示不在任何队列中)
			DLPLAYER_NODE_T	nodeTag;								// 在相应的tag队列中的节点

			TAGINFO_T()
			: nValue(0)
			{
			}

			void	clear()
			{
				nValue	= 0;
				nodeTag.leave();
			}
		};
		TAGINFO_T			taginfo[MAX_TAG_NUM];
		whhash<whuint64, DLPLAYER_NODE_T *>	map64Tag2Node;

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nSlot(0)
		, nPasswordToConnectCLS(0)
		, nRemoveReason(CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTHING)
		, nProp(0)
		, nSvrIdx(0)
		{
			szName[0]	= 0;
			for(int i=0;i<MAX_TAG_NUM;i++)
			{
				taginfo[i].nodeTag.data	= this;
			}
		}
		void	clear(CLS2_I *pCLS)
		{
			int	i;
			for(i=0;i<MAX_TAG_NUM;i++)
			{
				pCLS->DelPlayerFromTagList(this, i);
			}
			whhash<whuint64, DLPLAYER_NODE_T *>::kv_iterator it=map64Tag2Node.begin();
			while(it!=map64Tag2Node.end())
			{
				whuint64	nTag	= it.getkey();
				++it;
				pCLS->DelPlayerFrom64TagList(this, nTag);
			}

			teid.quit();

			nID			= 0;
			nSvrIdx		= 0;
			nStatus		= STATUS_NOTHING;
			nSlot		= 0;
			nPasswordToConnectCLS	= 0;
			nRemoveReason			= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTHING;
			nProp		= 0;
			szName[0]	= 0;
			nRecvCount	= 0;
			nRecvSize	= 0;
		}
		void	ReadFromOther(PlayerUnit *pOther)
		{
			//nID				= pOther->nID;	这个不用
			memcpy(szName, pOther->szName, sizeof(szName));
			nStatus			= pOther->nStatus;
			nSlot			= pOther->nSlot;
			nPasswordToConnectCLS	= pOther->nPasswordToConnectCLS;
			nRemoveReason	= pOther->nRemoveReason;
			nProp			= pOther->nProp;
			nSvrIdx			= pOther->nSvrIdx;
			teid.quit();	// 这个不用读取别人的
			for(int i=0;i<MAX_TAG_NUM;i++)
			{
				if( pOther->taginfo[i].nodeTag.isinlist() )
				{
					taginfo[i].nodeTag.insertbefore(&pOther->taginfo[i].nodeTag);
				}
			}
		}
	};
	struct	TQUNIT_T;
	typedef	void (CLS2_I::*TEDEAL_T)(TQUNIT_T *);
	struct	TQUNIT_T
	{
		TEDEAL_T	tefunc;											// 处理该事件的函数
		// 数据部分
		union
		{
			struct	PLAYER_T										// 玩家定时事件
			{
				int	nID;											// 玩家ID
			}player;
			struct	GZS_T											// GZS相关的定时时间（如：）
			{
				int	nID;											// 
			}gzs;
		}un;
	};
	struct	DLPUNIT_T
	{
		DLPLAYER_T	*pDLP;

		DLPUNIT_T()
		: pDLP(NULL)
		{
		}

		void	clear(CLS2_I *pCLS)
		{
			// SafeDeleteDLP里面会自动判断pDLP是否为空
			pCLS->SafeDeleteDLP(pDLP);
		}
	};
private:
	friend	struct	PlayerUnit;
	friend	struct	DLPUNIT_T;
	// 配置结构
	CFGINFO_T							m_cfginfo;					// CLS的配置
	tcpmsger::DATA_INI_INFO_T			m_MSGER_INFO;				// 和GMS以及GZS连接的msger的配置
	CNL2SlotMan::DATA_INI_INFO_T		m_CLIENT_SLOTMAN_INFO;		// 等待客户端连接的slotman的配置
	// 当前工作状态
	int									m_nStatus;					// CLS2::STATUS_XXX
	// 当前的时刻
	whtick_t							m_tickNow;
	// 自己在GMS中的ID（在重连的时候有用）
	int									m_nCLSID;
	// 加密工厂
	ICryptFactory						*m_pICryptFactory;
	// 对Client的连接管理
	CNL2SlotMan							*m_pSlotMan4Client;
	// 时间队列
	// 比如：处于连接状态的用户多长时间内如果没有连接成功则断线
	whtimequeue							m_TQ;
	// 各个服务器
	GZSUnit								m_GZSs[GZS_MAX_NUM];
	// 各个玩家
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// 连接GMS的msger
	class	MYMSGER_T	: public tcpretrymsger<tcpmsger>
	{
	public:
		CLS2_I			*m_pHost;
		whtick_t		m_tickWorkBegin;
	public:
		virtual void	i_Worker_TRYCONNECT_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
		}
		virtual void	i_Worker_WORKING_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	friend class MYMSGER_T;
	MYMSGER_T							m_msgerGMS;
	// 用于删除出错的GZS或Player
	whvector<int>						m_vectIDToDestroy;
	// tag到相应的DList的映射
	whhash<unsigned int, DLPUNIT_T>		m_mapTag2DL;
	// 64位的映射
	whhash<whuint64, DLPUNIT_T>			m_map64Tag2DL;
	// 临时组装指令（给GMS、GZS、给Client）
	whvector<char>						m_vectrawbuf;
	// 告诉上层应该退出
	bool								m_bShouldStop;
	// 当前发来指令的GZS或者其他服务器的序号
	unsigned char						m_nCurCmdFromSvrIdx;
	// 用于统计指令发送频率
	struct	CMDSTAT_T
	{
		unsigned int	nByte;			// 累计字节数
		unsigned int	nCount;			// 累计条数
		inline bool	operator < (const CMDSTAT_T &other) const
		{
			if( nByte != other.nByte )	return	nByte > other.nByte;
			return	nCount>other.nCount;
		}
	};
	whvector<CMDSTAT_T>					m_vectCmdStat;
	// 用于定时检查输出log
	whlooper							m_loopCmdStat;

	// 为了测试CLS为什么慢的原因
//	dbgtimeuse							m_dtu;
	unsigned int									m_nGZScmdCount;
	unsigned int									m_GZStickFuncCount[GZSTICKFUNC];
	unsigned int									m_GZStickFuncTime[GZSTICKFUNC];
private:
	// 设置当前状态
	void	SetStatus(int nStatus);
	// 移除Player
	int		RemovePlayerUnit(int nPlayerID);
	int		RemovePlayerUnit(PlayerUnit *pPlayer);
	void	RemoveAllPlayerOfGZS(unsigned char nSvrIdx);
	// 通过Slot获得Player
	PlayerUnit *	GetPlayerBySlot(int nSlot);
	// 尝试获取玩家，如果不存在就向GMS发送用户下线的通知
	PlayerUnit *	TryGetPlayerAndSendNotifyToGMSIsNotExisted(int nPlayerID);
	// 移除GZS
	int		RemoveGZSUnit(int nSvrIdx);
	int		RemoveGZSUnit(GZSUnit *pGZS);
	// 向单个玩家发送数据(里面需要把数据合成为CLS_CLIENT_DATA_T的形式，然后调用SendRawCmdToSinglePlayer)
	int		SendDataToSinglePlayer(PlayerUnit *pPlayer, unsigned char nChannel, void *pData, int nSize, pngs_cmd_t nSendCmd);
	// 向单个玩家发送不经修饰合成的纯指令
	int		SendRawCmdToSinglePlayer(int nPlayerID, unsigned char nChannel, void *pRawCmd, int nSize);
	int		SendRawCmdToSinglePlayer(PlayerUnit *pPlayer, unsigned char nChannel, void *pRawCmd, int nSize);
	// 向所有玩家发送不经修饰合成的纯指令
	int		SendRawCmdToAllPlayer(unsigned char nChannel, void *pRawCmd, int nSize);
	// 向所有和某个GZS相关的玩家发送不经修饰合成的纯指令
	int		SendRawCmdToAllPlayerOfGZS(unsigned char nSvrIdx, unsigned char nChannel, void *pRawCmd, int nSize);
	// 根据idx和val合成一个hash键值
	inline	unsigned int	CreateHashKey(unsigned char nTagIdx, short nTagVal)
	{
		return	(((unsigned int)nTagIdx) << 16) | nTagVal;
	}
	// 从hash键值中得到idx和val
	inline	void	GetIdxAndVal(unsigned int nHashKey, unsigned char *pnTagIdx, short *pnTagVal)
	{
		*pnTagIdx	= (unsigned char)((nHashKey & 0xFF0000) >> 16);
		*pnTagVal	= (short)(nHashKey & 0xFFFF);
	}
	// 把玩家加入某个tag队列中
	int		AddPlayerToTagList(PlayerUnit *pPlayer, unsigned char nTagIdx, short nTagVal);
	// 把玩家从某个tag队列中移出来
	int		DelPlayerFromTagList(PlayerUnit *pPlayer, unsigned char nTagIdx);
	// 把玩家加入某个tag64队列中
	int		AddPlayerToTag64List(PlayerUnit *pPlayer, whuint64 nTag);
	// 把玩家从某个tag64队列中移出来
	int		DelPlayerFrom64TagList(PlayerUnit *pPlayer, whuint64 nTag);

	// New一个DLPLAYER_T对象
	DLPLAYER_T *	NewDLP();
	// Delete一个DLPLAYER_T对象
	void	SafeDeleteDLP(DLPLAYER_T *&pDLP);
	// Init子函数
	int		Init_CFG(const char *cszCFG);
	// 获得GZS的所有socket
	int		GetGZSSockets(whvector<SOCKET> &vect);
	// 向GZS发送指令
	int		DoAllGZSSend();
	// 各种TickWork
	int		Tick_DealGMSMsg();
	int		Tick_DealGMSMsg_GMS_CLS_GZSINFO(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CLS_CLIENTWANTCLS(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CLS_ROUTECLIENTDATATOGZS(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CLS_PLAYER_CHANGEID(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_SVR_CLS_CLIENT_DATA(void *pCmd, int nSize);
	int		Tick_DealGZSMsg();										// 接收并处理GZS发来的消息(同时判断与GZS的连接的状况)
	int		Tick_DealClientMsg();
	int		Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize);
	int		Tick_DealClientMsg_CONTROL_OUT_SLOT_CLOSED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize);
	int		Tick_DealClientMsg_CLIENT_CLS_DATA(pngs_cmd_t nCmd, PlayerUnit *pPlayer, int nChannel, pngs_cmd_t *pCmd, int nSize);
	int		Tick_DealTE();
	// 公共的数据数据交换
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_CTRL(GZSUnit *pGZS, void *pCmd, int nSize);	// 如果pGZS为空则代表是GMS
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG_TO_CLIENT(void *pCmd, int nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_SET_PROP_TO_CLIENT(void *pCmd, int nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_ALL_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG64_TO_CLIENT(void *pCmd, int nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED64_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);

	// 当重连成功时需要调用的函数
	void	Retry_Worker_WORKING_Begin();

	// 时间事件处理
	// 客户端连接超时
	void	TEDeal_nClientConnectToMeTimeOut(TQUNIT_T *pTQUnit);
	// 设置上面的
	void	SetTEDeal_nClientConnectToMeTimeOut(PlayerUnit *pPlayer);
	// 定时检查用户发来的包的频率
	void	TEDeal_nCheckRecvInterval(TQUNIT_T *pTQUnit);
	// 设置上面的
	void	SetTEDeal_nCheckRecvInterval(PlayerUnit *pPlayer);

};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CLS_I_H__
