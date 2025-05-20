// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS模块内部的GZS_MAINSTRUCTURE这个Logic模块的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GZS是Game Zone Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-09-20
// Change LOG   :

#ifndef	__PNGS_GZS_I_GZS_MAINSTRUCTURE_H__
#define	__PNGS_GZS_I_GZS_MAINSTRUCTURE_H__

#include "./pngs_cmn.h"
#include "./pngs_packet.h"
#include "./pngs_packet_gzs_logic.h"
#include "./pngs_packet_logic.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whconnecter.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whcmd.h>
#include <WHCMN/inc/whhash.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

////////////////////////////////////////////////////////////////////
// GZS_MAINSTRUCTURE
////////////////////////////////////////////////////////////////////
class	GZS_MAINSTRUCTURE	: public CMN::ILogic
{
// 为接口实现的
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	GZS_MAINSTRUCTURE();
	~GZS_MAINSTRUCTURE();
	virtual	void	SelfDestroy();
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		SureSend();
private:
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		Tick_BeforeDealCmdIn();
	virtual int		Tick_AfterDealCmdIn();
// 自己用的
public:
	class	MYCNTRSVR	: public	TCPConnecterServer
	{
	public:
		GZS_MAINSTRUCTURE	*m_pHost;
	public:
		class	MYCNTR	: public	TCPConnecterMsger<tcpmsger>
		{
		public:
			GZS_MAINSTRUCTURE	*m_pHost;
			struct sockaddr_in	m_addr;								// 来源地址
			APPTYPE				m_nSvrType;							// 这个连接是什么类型的Server
																	// 取值为pngs_def.h中的APPTYPE_XXX
			void				*m_pExtPtr;							// 附加指针（这个连接关联的对象指针）
			whtimequeue::ID_T	teid;
			typedef	int		(GZS_MAINSTRUCTURE::*CNTR_DEALMSG_T)(MYCNTR *pCntr, const pngs_cmd_t *pData, size_t nSize);
			CNTR_DEALMSG_T	m_p_CNTR_DEALMSG;
		public:
			MYCNTR(GZS_MAINSTRUCTURE *pHost)
			: m_pHost(pHost)
			, m_nSvrType(APPTYPE_NOTHING)
			, m_pExtPtr(NULL)
			, m_p_CNTR_DEALMSG(&GZS_MAINSTRUCTURE::CNTR_DealMsg_Dft)
			{
			}
			void	clear()
			{
				m_pHost		= NULL;
				m_nSvrType	= APPTYPE_NOTHING;
				m_pExtPtr	= NULL;
				teid.quit();
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
				return	(m_pHost->*m_p_CNTR_DEALMSG)(this, (const pngs_cmd_t *)pData, nSize);
			}
		};
	private:
		// 为TCPConnecterServer实现的
		virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
		{
			return	m_pHost->NewConnecter(sock, paddr);
		}
		virtual void		BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo)
		{
			m_pHost->BeforeDeleteConnecter(nCntrID, pCntrInfo);
		}
		virtual void		AfterAddConnecter(Connecter * pCntr)
		{
			m_pHost->AfterAddConnecter(pCntr);
		}
	public:
		MYCNTRSVR()
		: m_pHost(NULL)
		{
		}
	};
	struct	CLSUnit
	{
		enum
		{
			STATUS_NOTHING			= 0,
			STATUS_WORKING			= 1,							// 正常工作状态
		};
		int		nID;
		int		nStatus;											// 状态
		MYCNTRSVR::MYCNTR	*pCntr;									// 对应的连接器对象指针

		CLSUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, pCntr(NULL)
		{
		}

		void	clear()
		{
			nID			= 0;
			nStatus		= STATUS_NOTHING;
			pCntr		= NULL;
		}
	};
	struct	PlayerUnit
	{
		// 2005-09-30 取消了这个的Status
		int		nID;
		int		nCLSID;												// 所处的CLS的ID

		PlayerUnit()
		: nID(0)
		, nCLSID(0)
		{
		}

		void	clear()
		{
			nID				= 0;
			nCLSID			= 0;
		}
	};
	struct	TQUNIT_T
	{
		typedef	void (GZS_MAINSTRUCTURE::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// 处理该事件的函数
		// 数据部分
		union
		{
			struct	SVR_T											// 服务器定时事件
			{
				int	nID;											// 服务器的connecterid
			}svr;
		}un;
	};
	struct	CFGINFO_T		: public whdataini::obj
	{
		// 基本参数
		unsigned char		nSvrIdx;								// GZS服务器序号
		unsigned char		nForceKickOldGZS;						// 是否强制踢出原来的同SvrIdx的GZS（0:不踢出；1:温柔一踢；2:强制踢出）
		int					nTQChunkSize;							// 时间队列的每次次性分配块大小
		int					nQueueCmdInSize;						// 指令输入队列的长度
		int					nHelloTimeOut;							// 等待连接发来Hello的超时(如果没法收到Hello则断开连接)
		char				szGMSAddr[WHNET_MAXADDRSTRLEN];			// GMS的地址
		int					nConnectOtherSvrTimeOut;				// 连接GMS或其他Server的超时
		int					nMaxCmdPackSize;						// 指令打包的最大尺寸
		bool				bLogSendCmd;							// 是否记录发送的指令
		bool				bLogRecvCmd;							// 是否记录接收到的指令（包括客户端和服务器发来的）
		MYCNTRSVR::DATA_INI_INFO_T	CNTRSVR;						// TCPConnecterServer的通用配置
		CFGINFO_T()
		: nSvrIdx(0)												// 默认填0，但是配置文件中必须填写，否则不能启动
		, nForceKickOldGZS(0)
		, nTQChunkSize(100)
		, nQueueCmdInSize(2000000)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nConnectOtherSvrTimeOut(PNGS_DFT_SVRCONNECT_TIMEOUT)
		, nMaxCmdPackSize(65536)
		, bLogSendCmd(false)
		, bLogRecvCmd(false)										// 这个默认先打开吧。直到查到服务器崩溃的错误为止。
		{
			strcpy(szGMSAddr, "localhost:3000");
			CNTRSVR.nPort	= 3101;
			CNTRSVR.nPort1	= 3110;
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
private:
	typedef	CMN::ILogic					FATHERCLASS;
	// 配置结构
	CFGINFO_T							m_cfginfo;					// 配置信息
	tcpmsger::DATA_INI_INFO_T			m_MSGER_INFO;				// GZS和其他Server用TCP连接通讯的msger的配置
	// TCP服务器
	MYCNTRSVR		m_CntrSvr;
	// 对象数组
	whunitallocatorFixed<CLSUnit>		m_CLSs;
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// 当前时刻
	whtick_t		m_tickNow;
	// 状态
	enum
	{
		STATUS_ERROR_WITH_GMS			= -1,						// 和GMS断线了
		STATUS_NOTHING					= 0,
		STATUS_WORKING					= 1,
	};
	int				m_nStatus;
	// 连接GMS的msger
	class	MYMSGER_T	: public tcpretrymsger<tcpmsger>
	{
	public:
		GZS_MAINSTRUCTURE				*m_pHost;
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
	MYMSGER_T		m_msgerGMS;
	// 时间事件处理器
	whtimequeue		m_TQ;
	// 用于临时合成指令
	whvector<char>	m_vectrawbuf;
	// 输入指令队列
	whsmpqueueWL	m_queueCmdIn;
	// 需要使用到的其他logic模块
	// 游戏逻辑模块
	ILogicBase		*m_pLogicGamePlay;
	// 用于生成打包指令
	whvector<char>	m_vectCmdPack;
	whcmdshrink		m_wcsCmdPack;
	// 打包开始的标志（CmdPackBegin）
	GZSPACKET_2MS_CMD_PACK_BEGIN_T	m_cpb;
	inline void	CmdPackReset()
	{
		m_wcsCmdPack.Reset();
		m_cpb.nSendCmd	= GZSPACKET_2MS_0;
	}
	whhash<pngs_cmd_t, ILogicBase *>	m_mapCmdReg2Logic;			// 用于指令分拣，把特定的指令分拣给特定的逻辑模块去执行
																	// 这个目前处理的是GMS服务器发来的指令GMS_GZS_GAMECMD
	unsigned int	m_nNoCLIENT_RECV;								// 如果有其他模块设置了这里面的某个bit，只要这个变量非0，则不接收任何客户端的来包

	bool			m_bIsReconnect;									// 表示是重连上的
	bool			m_bConnectedToGMS;								// 表示自己和GMS的连接状况的（如果为false则逻辑层应该考虑不要进行和GMS相关的操作了）
private:
	friend	class	MYCNTRSVR;
	friend	class	MYCNTRSVR::MYCNTR;
	// 根据配置文件初始化参数
	int		Init_CFG(const char *cszCFGName);
	// 处理配置相关指令
	int		CmdIn_GZSPACKET_2MS_CONFIG(ILogicBase *pRstAccepter, GZSPACKET_2MS_CONFIG_T *pCmd, int nSize);
	// 针对一个Connecter做一次工作
	bool	CNTR_CanDealMsg() const;
	// 针对一个Connecter做一次工作
	int		CNTR_DealMsg_Dft(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_CLS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	// 配合MYCNTRSVR的函数(不用是虚的)
	Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	void	AfterAddConnecter(Connecter * pCntr);
	void	BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo);
	// 让所有连接的服务器自动退出(这样GZS的端口就不会timewait了)
	void	TellAllConnecterToQuit();
	// 让所有的CLS退出
	void	TellAllCLSQuit();
	// 让CLS踢出用户
	void	TellCLSToKickPlayer(int nClientID, unsigned char nKickCmd=GZSPACKET_2MS_KICKPLAYER_T::CMD_KICK_END);
	// 让CLS踢出所有自己相关的用户
	void	TellCLSToKickAllPlayerOfMe();
	// 告诉GMS自己是正常退出的
	void	TellGMSGoodExit();
	// 告诉GMS踢出用户，并且在自己这里Remove用户。这个必须是在CLS不存在的情况下才行的
	void	TellGMSToKickPlayerAndRemovePlayer(int nClientID);
	// 移除一个Connecter（并移除其附带的对象，如：CLSUnit等等）
	int		RemoveMYCNTR(int nCntrID);
	int		RemoveMYCNTR(MYCNTRSVR::MYCNTR *pCntr);
	int		RemovePlayerUnit(int nID);
	int		RemovePlayerUnit(PlayerUnit *pPlayer);
	// 给所有的CLS发送一条指令
	int		SendCmdToAllCLS(const void *pCmd, size_t nSize);
	// 给Player所在的CLS发送一条指令(内部直接把指令传递给CLS，没有使用m_vectrawbuf，所以上层可以使用m_vectrawbuf来生成指令)
	int		SendCmdToPlayerCLS(int nClientID, const void *pCmd, size_t nSize);
	int		SendCmdToPlayerCLS(PlayerUnit *pPlayer, const void *pCmd, size_t nSize);
	// 给其他服务器发送指令
	int		SendSvrCmdToSvrBySvrIdx(unsigned char nSvrIdx, const void *pCmd, size_t nSize);
	// 给Player发送指令
	int		SendCmdToPlayer(int nPlayerID, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToPlayer(PlayerUnit *pPlayer, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS(int nCLSID, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS(CLSUnit *pCLS, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToAllPlayer(unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	int		SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	int		SendCmdToAllPlayerByTag64(whuint64 nTag, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	// 设置Player的Tag值(内部包装成SVR_CLS_SET_TAG_TO_CLIENT发送给CLS)
	int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal);
	int		SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel);
	// 增加打包数据
	int		AddPackData(const void *pData, size_t nDSize);
	// Tick中的方法
	// 时间事件处理
	int		Tick_DealTE();
	// 其他服务器连入没有发送hello的超时
	void	TEDeal_Hello_TimeOut(TQUNIT_T *pTQUnit);
	// 处理GMS发来的指令
	int		Tick_DealGMSMsg();
	int		Tick_DealGMSMsg_GMS_GZS_CTRL(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_GZS_PLAYER_ONLINE(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_GZS_PLAYER_OFFLINE(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_GZS_GAMECMD(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_GZS_PLAYER_CHANGEID(void *pCmd, int nSize);
	// 把玩家指令传递给逻辑对象
	void	InnerRouteClientGameCmd(CLS_SVR_CLIENT_DATA_T *pCmd, size_t nSize);
	// 设置状态
	void	SetStatus(int nStatus);
	// 重连开始时需要调用的函数
	void	Retry_Worker_TRYCONNECT_Begin();
	// 当重连成功时需要调用的函数
	void	Retry_Worker_WORKING_Begin();

public:
	// 创建自己类型的对象
	static CMN::ILogic *	Create()
	{
		return	new	GZS_MAINSTRUCTURE;
	}
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_GZS_I_GZS_MAINSTRUCTURE_H__
