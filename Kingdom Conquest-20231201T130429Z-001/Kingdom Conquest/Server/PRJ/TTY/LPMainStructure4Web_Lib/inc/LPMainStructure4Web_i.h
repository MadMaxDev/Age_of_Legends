//////////////////////////////////////////////////////////////////////////
//web扩展逻辑代理服务器
//移动扩展也需要使用
//暂时应该相当于web的GMS,并且具备简单的逻辑处理功能
//可以给所有的CLS、CLS4Web发消息,但是只能管理CLS4Web,不能管理CLS
//和CLS的消息流向是单向的(CLS<-LP),暂时
//尽量开辟新的消息通道,不然有很多地方可能不能通过校验
//!!!怎么发现断线重连而不是宕机？给每个连接加上个链接密码，两边对不上就是有一个是宕机的，直接踢掉即可(下一步修改)
//以下情况暂不处理:LP没有宕机,CLS4Web全部当掉(且每个CAAFS4Web的CLS4Web连接到上限),在LP发现断连超时之前,CLS4Web可能得不断重启(收到HI却发现没位置了,就通知CLS4Web退出)
//以后改进:其实CLS4Web也用数组存,每个连接带一个连接密码,用CLS4Web的索引(现在是ID)和连接密码来判断是否重连的,就能避免上述情况
//////////////////////////////////////////////////////////////////////////

#ifndef __LPMainStructure4Web_i_H__
#define __LPMainStructure4Web_i_H__

#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_def.h"
#include "PNGS/inc/pngs_TCPReceiver_def.h"
#include "PNGS/inc/pngs_packet_mem_logic.h"
#include "PNGS/inc/pngs_packet_logic.h"
#include "PNGS/inc/pngs_packet.h"
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whunitallocator3.h"
#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnettcp.h"
#include "../../Common/inc/pngs_packet_web_extension.h"
#include "../../Common/inc/tty_common_def.h"
#include "../../Common/inc/pngs_packet_lp_logic.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class LPMainStructure4Web_i : public CMN::ILogic
{
// ILogic接口
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
	int				Tick_DealTE();
// 自己的东西
public:
	LPMainStructure4Web_i();
	~LPMainStructure4Web_i();
	int				Init_CFG(const char* cszCFGName);
	int				DealCmdIn_One_PNGS_TR2CD_CMD(const void *pData, int nDSize);
private:
	// 时间队列中的单元
	struct TQUNIT_T 
	{
		typedef	void	(LPMainStructure4Web_i::*TEDEAL_T)(TQUNIT_T*);
		TEDEAL_T		tefunc;
		// 数据部分
		union
		{
			struct SVR_T 
			{
				int	nID;
			}svr;
			void*	ptr;
		}un;
	};
public:
	struct	CLS4WebUnit;
	struct	PlayerUnit;
	struct	CAAFS4WebGroup
	{
		int						nCntrID;		// 对应的连接器对象指针,如果为0表示CAAFS4Web还没有连接进来
		whDList<CLS4WebUnit*>	dlCLS4Web;		// 统领的一票CLS4Web列表
	
		CAAFS4WebGroup()
		: nCntrID(0)
		{}

		void	clear()
		{
			nCntrID		= 0;
			dlCLS4Web.reset();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			nCntrID		= 0;
			dlCLS4Web.AdjustInnerPtr(nOffset);
			// 每个单元的指针内容就是CLS4Webs里面的内容,它自己调整过了,所以就不用调整了
		}
	};
	struct	CLS4WebUnit 
	{
		enum
		{
			STATUS_NOTHING		= 0,
			STATUS_WORKING		= 1,					// 正常工作
		};
		enum
		{
			PROP_GOODEXIT		= 0x01,					// 表明自己是正常退出的
		};
		int				nID;
		int				nStatus;						// 状态
		int				nCntrID;						// 对应的连接ID
		int				nProp;							// 附加属性
		port_t			nPort;							// 对Client的端口
		unsigned int	IP;								// 对Client的IP
		int				nGroupID;						// 一个CAAFS4Web和若干CLS4Web为一组
		int				nMaxPlayer;						// 该CLS4Web能容纳的最多用户数量(由LP设置)
		whDList<CLS4WebUnit*>::node		dlnode;			// 在CAAFS4WebGroup中的列表中的节点
		whDList<PlayerUnit*>			dlPlayer;		// 属于这个CLS4Web的Player列表
		whtimequeue::ID_T				teid;			// 时间事件
		whvector<int>*	pVectClientID;					// 向多个客户端发送数据时关联这个CLS4Web的客户端ID列表(整理时需要重新分配)
		whDList<CLS4WebUnit*>::node		dlnodeInSCTMP;	// 在给SendCmdToMultiPlayer中使用的临时节点
	
		CLS4WebUnit()
		: nID(0), nStatus(STATUS_NOTHING)
		, nCntrID(0), nProp(0)
		, nPort(0), IP(0)
		, nGroupID(-1), nMaxPlayer(0)
		{
			dlnode.data			= this;
			dlnodeInSCTMP.data	= this;
			pVectClientID		= new whvector<int>;
			pVectClientID->reserve(64);
		}
		~CLS4WebUnit()
		{
			WHSafeDelete(pVectClientID);
		}
		void	clear()
		{
			nID			= 0;
			nStatus		= STATUS_NOTHING;
			nCntrID		= 0;
			nProp		= 0;
			nPort		= 0;
			IP			= 0;
			nGroupID	= -1;
			nMaxPlayer	= 0;
			dlnode.leave();
			dlPlayer.clear();
			teid.quit();
			dlnodeInSCTMP.leave();
		}
		// 获得已经拥有的玩家数量
		inline int	GetCurPlayerNum() const
		{
			return dlPlayer.size();
		}
		// 获得还可以接纳的玩家数量
		inline int	GetAcceptablePlayerNum() const
		{
			return nMaxPlayer - GetCurPlayerNum();
		}
		// 返回自己当前的状态是否允许接纳玩家
		inline bool	CanAcceptPlayer() const
		{
			return nStatus == STATUS_WORKING;
		}
		void	AdjustInnerPtr(int nOffset)
		{
			nCntrID		= 0;
			teid.clear();
			// dlnode已经调整过了
			dlPlayer.AdjustInnerPtr(nOffset);
			whDList<PlayerUnit*>::node*	pNode	= dlPlayer.begin();
			for (; pNode!=dlPlayer.end(); pNode=pNode->next)
			{
				wh_adjustaddrbyoffset(pNode->data, nOffset);
			}
			dlnode.data		= this;
			pVectClientID	= new whvector<int>;
			pVectClientID->reserve(64);

			dlnodeInSCTMP.data	= this;
			dlnodeInSCTMP.reset();
		}
	};
	struct	PlayerUnit
	{
		enum
		{
			STATUS_NOTHING			= 0,
			STATUS_WANTCLS4Web		= 1,				// LP正在向CLS4Web提交请求中
			STATUS_GOINGTOCLS4Web	= 2,				// 玩家正在去往CLS4Web中
			STATUS_GAMING			= 3,				// 正常的游戏通讯中
		};
		int		nID;
		int		nStatus;
		int		nCLS4WebID;
		int		nClientIDInCAAFS4Web;
		unsigned int	IP;
		whDList<PlayerUnit*>::node	dlnodeInCLS4Web;
		int		nTermType;

		PlayerUnit()
		: nID(0), nStatus(STATUS_NOTHING)
		, nCLS4WebID(0), nClientIDInCAAFS4Web(0)
		, IP(0)
		, nTermType(TERM_TYPE_NONE)
		{
			dlnodeInCLS4Web.data	= this;
		}
		void	clear()
		{
			nID						= 0;
			nStatus					= STATUS_NOTHING;
			nCLS4WebID				= 0;
			nClientIDInCAAFS4Web	= 0;
			dlnodeInCLS4Web.leave();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			// 目前没有什么可调整的(dlnodeInCLS4Web在dlPlayer中调整好了)
		}
		void	ReadFromOther(PlayerUnit* pOther)
		{
			nStatus			= pOther->nStatus;
			nCLS4WebID		= pOther->nCLS4WebID;
			IP				= pOther->IP;
			if (pOther->dlnodeInCLS4Web.isinlist())
			{
				dlnodeInCLS4Web.insertbefore(&pOther->dlnodeInCLS4Web);
			}
		}
	};
public:
	struct	CFGINFO_T : public whdataini::obj
	{
		// 基本参数
		int				nMaxPlayer;									// 最多可以承载的玩家数量
		int				nSafeReservePlayerNum;						// 为了缓解由于人数统计不准错误把用户放进来而导致用户连接失败的缓冲阀值
		int				nMaxCLS4Web;								// 最多可以承载的CLS4Web数量
		int				nTQChunkSize;								// 时间队列的每次次性分配块大小
		int				nQueueCmdInSize;							// 指令输入队列的长度
		int				nHelloTimeOut;								// 等待连接发来Hello的超时
		int				nSvrDropWaitTimeOut;						// 如果有服务器断线,可以等待一段时间后再删除,因为可能是临时的网络原因,不是对方真的当了
		int				nCalcAndSendCAAFS4WebAvailPlayerTimeOut;	// 定时计算各个CAAFS4Web组的可进入人数的间隔
		int				anMaxCLS4WebNumInCAAFS4WebGroup[LP_MAX_CAAFS4Web_NUM];				// 各个CAAFS4Web组中最多的CLS4Web数量
		int				anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup[LP_MAX_CAAFS4Web_NUM];		// 各个CAAFS4Web组中每个CLS4Web最多的玩家数量
		bool			bCLSUseCAAFSIP;								// 为true的时候，CLS和CAAFS使用同一个地址(为了内网和外网通用)
		
		CFGINFO_T()
		: nMaxPlayer(256)
		, nSafeReservePlayerNum(128)
		, nMaxCLS4Web(0)
		, nTQChunkSize(100)
		, nQueueCmdInSize(2*1024*1024)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nSvrDropWaitTimeOut(20*1000)
		, nCalcAndSendCAAFS4WebAvailPlayerTimeOut(10*1000)
		, bCLSUseCAAFSIP(true)
		{
			memset(anMaxCLS4WebNumInCAAFS4WebGroup, 0, sizeof(anMaxCLS4WebNumInCAAFS4WebGroup));
			memset(anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup, 0, sizeof(anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup));
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
private:
	typedef	CMN::ILogic			FATHERCLASS;
	// 配置结构
	CFGINFO_T		m_cfginfo;
	// 底层tcp通讯框架
	ILogicBase*		m_pLogicTCPReceiver;
	// gameplay模块
	ILogicBase*		m_pLogicGamePlay;
	whtick_t		m_tickNow;
	// 时间事件处理器
	whtimequeue		m_TQ;
	// 用于临时合成指令(所有不用二次整理的指令应该都可以通过他发送)
	// 现在使用他的有:Send系列需要指令内容的函数,即所有最终目标是Player的函数
	// Tick中可以用他来生成给CLS4Web的指令
	whvector<char>	m_vectrawbuf;
	// 定时统计各CAAFS4Web组的可进入人数
	whtimequeue::ID_T	m_teid_CalcAndSend_CAAFS4Web_AvailPlayerNum;
	whunitallocatorFixed<CLS4WebUnit>	m_CLS4Webs;
	whunitallocatorFixed<PlayerUnit>	m_Players;
	CAAFS4WebGroup	m_aCAAFS4WebGroup[LP_MAX_CAAFS4Web_NUM];
	// cntr id到类型的映射
	struct AppInfo_T 
	{
		int			nAppType;		// 应用类型
		void*		pAppUnit;		// 单元指针,指向

		AppInfo_T()
		: nAppType(APPTYPE_NOTHING)
		, pAppUnit(NULL)
		{
		}
	};
	map<int, AppInfo_T>	m_mapCntrID2AppInfo;
private:
	// 通过GroupID找到CAAFS4WebGroup
	CAAFS4WebGroup*	GetCAAFS4WebByGroupID(int nGroupID);
	// 让所有链接的服务器自动退出(这样LP的端口就不会timewait了)
	void	TellAllConnecterToQuit();
	// 让所有CAAFS4Web退出
	void	TellAllCAAFS4WebQuit();
	// 让所有CLS4Web退出
	void	TellAllCLS4WebQuit();
	// 让CLS4Web踢出用户(这样写是为了强调踢出用户都是从CLS开始的)
	void	TellCLS4WebToKickPlayer(int nClientID, pngs_cmd_t nSubCmd=SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID);
	//
	void	TellCLS4WebToCheckPlayerOffline(CLS4WebUnit* pCLS4Web);
	//
	int		RemovePlayerUnit(int nID, unsigned char nRemoveReason=CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING);
	int		RemovePlayerUnit(PlayerUnit* pPlayer, unsigned char nRemoveReason=CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING);
	// 给所有的CLS4Web发一条消息
	int		SendCmdToAllCLS4Web(const void* pCmd, size_t nSize);
	// 给所有的CLS发送一条消息
	//int		SendCmdToAllCLS(const void* pCmd, size_t nSize);
	// 给Player所在的CLS4Web发送一条指令(内部直接把指令转给CLS4Web)
	int		SendCmdToPlayerCLS4Web(int nClientID, const void* pCmd, size_t nSize);
	int		SendCmdToPlayerCLS4Web(PlayerUnit* pPlayer, const void* pCmd, size_t nSize);
	// 给Player发送指令(内部会包装成发给CLS4Web的指令)
	int		SendCmdToPlayer(int nPlayerID, const void* pCmd, size_t nSize);
	int		SendCmdToPlayer(PlayerUnit* pPlayer, const void* pCmd, size_t nSize);
	int		SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS4Web(int nCLS4WebID, int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS4Web(CLS4WebUnit* pCLS4Web, int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize);
	int		SendCmdToAllPlayer(const void* pData, size_t nSize);
	int		SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, const void* pCmd, size_t nSize);
	int		SendCmdToAllPlayerByTag64(whuint64 nTag, const void* pCmd, size_t nSize);
	// 设置player的tag值
	int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal);
	int		SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel);
	
	// Tick中的方法
	// 时间事件处理
	// 其他服务器连入没有发送hello/hi的超时
	void	TEDeal_Hello_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的事件
	void	SetTE_Hello_TimeOut(int nCntrID);
	// CLS4Web断线等待超时
	void	TEDeal_CLS4Web_DropWait_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的事件
	void	SetTE_CLS4Web_DropWait_TimeOut(CLS4WebUnit* pCLS4Web);
	// 定期计算各组CAAFS4Web还可进入的用户数量的间隔
	void	TEDeal_CalcAndSend_CAAFS4Web_AvailPlayerNum(TQUNIT_T* pTQUnit);
	// 处理用户发来的指令(内部应该是转发给相应的逻辑处理单元)
	void	InnerRouteClientGameCmd(CMN::cmd_t nCmd, CLS4Web_SVR_CLIENT_DATA_T* pCmd, size_t nSize);
	// 设置CLS4Web断连超时

	// 处理CLS4Web断线的一些事情
	int		DealCLS4WebDisconnection(CLS4WebUnit* pCLS4Web);
	int		SendCmdToConnecter(int nCntrID, const void* pData, int nDSize);
	int		RemoveConnecter(int nCntrID);

	// 根据指针获取CAAFS4Web的序号
	inline	int		GetCAAFS4WebGroupID(CAAFS4WebGroup* pCAAFS)
	{
		return wh_getidxinarray(m_aCAAFS4WebGroup, pCAAFS);
	}
};
}

#endif

