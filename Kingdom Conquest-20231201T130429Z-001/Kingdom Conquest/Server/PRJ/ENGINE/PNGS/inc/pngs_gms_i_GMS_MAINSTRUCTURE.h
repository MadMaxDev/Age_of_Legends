// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE这个Logic模块的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-08-31
// Change LOG   : 2005-09-27 SvrIdx和GZS对象的对应方式完全由m_GZSs的内部对象下标来实现，省去了原来的数值对应
//              : 2005-09-29 GoPlayerToSvr还是不用传递附加参数了，因为反正一切来源都是DB，所以GZS还是直接从DB获取数据的好
//              : 2005-09-30 取消了转服过程。用户上线就存在于所有GZS中。取消了PlayerUnit中的nGZSID。

#ifndef	__PNGS_GMS_I_GMS_MAINSTRUCTURE_H__
#define	__PNGS_GMS_I_GMS_MAINSTRUCTURE_H__

#include "./pngs_packet.h"
#include "./pngs_gms.h"
#include "./pngs_packet_logic.h"
#include "./pngs_packet_gms_logic.h"
#include "./pngs_packet_mem_logic.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whconnecter.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whgener.h>
#include <WHCMN/inc/whunitallocator3.h>
#include <WHCMN/inc/whcmd.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

////////////////////////////////////////////////////////////////////
// GMS_MAINSTRUCTURE
////////////////////////////////////////////////////////////////////
class	GMS_MAINSTRUCTURE	: public CMN::ILogic
{
// 为接口实现的
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	GMS_MAINSTRUCTURE();
	~GMS_MAINSTRUCTURE();
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
		GMS_MAINSTRUCTURE	*m_pHost;
	public:
		class	MYCNTR	: public	TCPConnecterMsger<tcpmsger>
		{
		public:
			GMS_MAINSTRUCTURE	*m_pHost;
			struct sockaddr_in	m_addr;								// 来源地址
			APPTYPE				m_nSvrType;							// 这个连接是什么类型的Server
																	// 取值为pngs_def.h中的APPTYPE_XXX
			void				*m_pExtPtr;							// 附加指针（这个连接关联的对象指针）
			whtimequeue::ID_T	teid;
			typedef	int		(GMS_MAINSTRUCTURE::*CNTR_DEALMSG_T)(MYCNTR *pCntr, const pngs_cmd_t *pData, size_t nSize);
			CNTR_DEALMSG_T	m_p_CNTR_DEALMSG;
		public:
			MYCNTR(GMS_MAINSTRUCTURE *pHost)
			: m_pHost(pHost)
			, m_nSvrType(APPTYPE_NOTHING)
			, m_pExtPtr(NULL)
			, m_p_CNTR_DEALMSG(&GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft)
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
		virtual void	AfterAddConnecter(Connecter * pCntr)
		{
			m_pHost->AfterAddConnecter(pCntr);
		}
	public:
		MYCNTRSVR()
		: m_pHost(NULL)
		{
		}
	};
	struct	CLSUnit;
	struct	PlayerUnit;
	struct	CAAFSGroup
	{
		MYCNTRSVR::MYCNTR	*pCntr;									// 对应的CAAFS连接器对象指针，如果为NULL则表示相应的CAAFS还没有连接进来
		whDList<CLSUnit *>	dlCLS;									// 它统领的CLS的列表

		CAAFSGroup()
		: pCntr(NULL)
		{
		}

		void	clear()
		{
			pCntr	= NULL;
			dlCLS.reset();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			pCntr		= NULL;
			dlCLS.AdjustInnerPtr(nOffset);
			// 每个单元的指针内容就是CLSs里面的内容，它自己调整过了，所以就不用调整了
		}
	};
	struct	CLSUnit
	{
		enum
		{
			STATUS_NOTHING			= 0,
			STATUS_CONNECTINGGZS	= 1,							// 连接新的GZS中
			STATUS_WORKING			= 2,							// 正常工作状态
		};
		enum
		{
			PROP_GOODEXIT	= 0x01,									// 标明自己是正常退出的
		};
		int		nID;
		int		nStatus;											// 状态
		MYCNTRSVR::MYCNTR	*pCntr;									// 对应的连接器对象指针
		unsigned char		nProp;									// 附加属性
		unsigned char		nNetworkType;							// 对应的网络类型（NETWORKTYPE_XXX）
		port_t				nPort;									// 对Client的端口	
		unsigned int		IP;										// 对Client的IP
		int		nGroupID;											// 组号一个CAAFS和若干CLS是一组
		int		nMaxPlayer;											// 该CLS可以容纳的最多用户数量(由GMS设置)
		int		nGZSNumToConnect;									// 还需要连接这么多GZS就可以正式工作了
		bool	bCanAcceptPlayerWhenCLSConnectingGZS;				// 在CLS连接GZS的过程中允许用户连接
		whDList<CLSUnit *>::node	dlnode;							// 在CAAFSGroup中的列表中的节点
		whDList<PlayerUnit *>		dlPlayer;						// 属于这个CLS的Player列表
		whtimequeue::ID_T	teid;									// 用于某些和GZS相关的事件
		whvector<int>		*pvectClientID;							// 在向多个客户发送数据是关联这个CLS的客户ID列表（整理时需要重新分配一下）
		whDList<CLSUnit *>::node	dlnodeInSCTMP;					// 在给SendCmdToMultiPlayer中使用的临时节点

		CLSUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, pCntr(NULL)
		, nProp(0)
		, nNetworkType(NETWORKTYPE_UNKNOWN)
		, nPort(0)
		, IP(0)
		, nGroupID(-1)
		, nMaxPlayer(0)
		, nGZSNumToConnect(0)
		, bCanAcceptPlayerWhenCLSConnectingGZS(false)
		{
			dlnode.data		= this;
			dlnodeInSCTMP.data	= this;
			pvectClientID	= new whvector<int>;
			pvectClientID->reserve(64);
		}
		~CLSUnit()
		{
			if( pvectClientID )
			{
				delete	pvectClientID;
				pvectClientID	= NULL;
			}
		}

		void	clear()
		{
			nID			= 0;
			nStatus		= STATUS_NOTHING;
			pCntr		= NULL;
			nProp		= 0;
			nPort		= 0;
			IP			= 0;
			nGroupID	= -1;
			nMaxPlayer	= 0;
			nGZSNumToConnect	= 0;
			bCanAcceptPlayerWhenCLSConnectingGZS	= false;
			dlnode.leave();
			dlPlayer.clear();
			teid.quit();
		}
		// 获得已经拥有的玩家数量
		inline int	GetCurPlayerNum() const
		{
			return	dlPlayer.size();
		}
		// 获得还可以接纳的玩家数量
		inline int	GetAcceptablePlayerNum() const
		{
			return	nMaxPlayer - GetCurPlayerNum();
		}
		// 返回自己当前的状态是否允许接纳玩家
		inline bool	CanAcceptPlayer() const
		{
#ifdef	_DEBUG
			return	nStatus == STATUS_WORKING || (bCanAcceptPlayerWhenCLSConnectingGZS && nStatus == STATUS_CONNECTINGGZS);
#else
			return	nStatus == STATUS_WORKING;
#endif
		}
		void	AdjustInnerPtr(int nOffset)
		{
			pCntr		= NULL;
			teid.clear();
			// dlnode（因为CAAFSGroup中已经调整了，所以这个就不用调整了）
			// 玩家列表需要调整一下
			dlPlayer.AdjustInnerPtr(nOffset);
			for(whDList<PlayerUnit *>::node *pNode=dlPlayer.begin(); pNode!=dlPlayer.end(); pNode=pNode->next)
			{
				wh_adjustaddrbyoffset( pNode->data, nOffset );
			}
			dlnode.data		= this;	// 主队列中应该已经校正了node中的其他底层指针参数了
			pvectClientID	= new whvector<int>;
			pvectClientID->reserve(64);

			dlnodeInSCTMP.data	= this;
			dlnodeInSCTMP.reset();
		}
		inline int	SendMsg(const void *pData, size_t nSize)
		{
			if( pCntr )
			{
				return	pCntr->SendMsg(pData, nSize);
			}
			return	-1;
		}
	};
	struct	GZSUnit
	{
		enum
		{
			PROP_GOODEXIT	= 0x01,									// 标明自己是正常退出的
		};
		MYCNTRSVR::MYCNTR	*pCntr;									// 对应的连接器对象指针（如果这个非空说明GZS连接GMS成功了）
		unsigned char		nSvrIdx;								// 对应的序号
		unsigned char		nProp;									// 附加属性
		port_t				nPort;									// 可以连入的端口
		unsigned int		IP;										// 可以连入的IP
		whtimequeue::ID_T	teid;									// 用于某些和GZS相关的事件

		GZSUnit()
		: pCntr(NULL)
		, nSvrIdx(0)
		, nProp(0)
		, nPort(0)
		, IP(0)
		{
		}

		void	clear()
		{
			pCntr	= NULL;
			nSvrIdx	= 0;
			nProp	= 0;
			nPort	= 0;
			IP		= 0;
			teid.quit();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			pCntr		= NULL;
			teid.clear();
		}
	};
	struct	GMTOOLUnit
	{
		int		nID;
		MYCNTRSVR::MYCNTR	*pCntr;									// 对应的连接器对象指针

		GMTOOLUnit()
		: nID(0)
		, pCntr(NULL)
		{
		}

		void	clear()
		{
			WHMEMSET0THIS();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			pCntr		= NULL;
		}
	};
	struct	PlayerUnit
	{
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_WANTCLS				= 1,						// GMS正在向CLS提交请求中
			STATUS_GOINGTOCLS			= 2,						// 玩家正在去往CLS中
			STATUS_GAMING				= 3,						// 正常的游戏通讯中
		};
		int		nID;
		int		nStatus;											// 当前的状态
		int		nCLSID;												// 所处的CLS的ID
		int		nClientIDInCAAFS;									// 该玩家在CAAFS中的ID（这个是用于在CAAFS中定位对象的）
		unsigned int	IP;											// 用户IP
		unsigned char	nNetworkType;								// 玩家对应的网络类型（NETWORKTYPE_XXX）
		unsigned char	nSvrIdx;									// 关联的目标GZS的序号
		whDList<PlayerUnit *>::node		dlnodeInCLS;				// 在CLS的Player列表中的节点元素

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nCLSID(0)
		, nClientIDInCAAFS(0)
		, IP(0)
		, nNetworkType(NETWORKTYPE_UNKNOWN)
		, nSvrIdx(0)
		{
			dlnodeInCLS.data	= this;
		}

		void	clear()
		{
			nID					= 0;
			nStatus				= STATUS_NOTHING;
			nCLSID				= 0;
			nClientIDInCAAFS	= 0;
			nSvrIdx				= 0;
			dlnodeInCLS.leave();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			// 目前没有什么要调整的
		}
		void	ReadFromOther(PlayerUnit *pOther)
		{
			nStatus				= pOther->nStatus;
			nCLSID				= pOther->nCLSID;
			IP					= pOther->IP;
			nNetworkType		= pOther->nNetworkType;
			nSvrIdx				= pOther->nSvrIdx;
			if( pOther->dlnodeInCLS.isinlist() )
			{
				dlnodeInCLS.insertbefore(&pOther->dlnodeInCLS);
			}
		}
	};
	struct	TQUNIT_T
	{
		typedef	void (GMS_MAINSTRUCTURE::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// 处理该事件的函数
		// 数据部分
		union
		{
			struct	SVR_T											// 服务器定时事件
			{
				int	nID;											// 服务器的connecterid，或者其他ID
			}svr;
			struct	GZS_T											// GZS的定时事件
			{
				int	nIdx;											// GZS序号
			}gzs;
		}un;
	};
	struct	CFGINFO_T		: public whdataini::obj
	{
		// 基本参数
		int					nMaxPlayer;								// 最多可承载的玩家数量
		int					nSafeReservePlayerNum;					// 为了缓解由于人数统计不准错误把用户放进来而导致用户连接失败的缓冲阀值
		int					nMaxCLS;								// 最多可承载的CLS数量(这个是通过anMaxCLSNumInCAAFSGroup相加得到的，所以这个不会在配置文件中配置)
		int					nMaxGMTOOL;								// 最多可承载的GMTOOL数量
		int					nTQChunkSize;							// 时间队列的每次次性分配块大小
		int					nQueueCmdInSize;						// 指令输入队列的长度
		int					nHelloTimeOut;							// 等待连接发来Hello的超时(如果没法收到Hello则断开连接)
		int					nSvrDropWaitTimeOut;					// 如果发现有服务器（GZS或CLS）断线，可以等待一定时间之后再删除，因为也可能是因为临时的网络原因，不是对方真的宕了。
		int					nCalcAndSendCAAFSAvailPlayerTimeOut;	// 定时计算各个CAAFS组的可进入人数的间隔
		int					nSvrCmdQueueSize;						// 用于缓冲GZS指令的队列长度
		int					nSvrCmdPerSecond;						// 每秒钟可以发出的GZS指令数量
		bool				bGZSDownRestorePlayer;					// 是否需要在GZS宕机重启之后把所有角色自动恢复（这个和GMSGamePlay中的配置定义要一致）
		int					anMaxCLSNumInCAAFSGroup[GMS_MAX_CAAFS_NUM];
																	// 各个CAAFS组中最多的CLS数量
		int					anMaxPlayerNumInCLSOfCAAFSGroup[GMS_MAX_CAAFS_NUM];
																	// 各个CAAFS组中每个CLS最多的玩家数量
		MYCNTRSVR::DATA_INI_INFO_T	CNTRSVR;						// TCPConnecterServer的通用配置
		CFGINFO_T()
		: nMaxPlayer(128)											// 写少一点，这样如果出问题会发现的比较早
		, nSafeReservePlayerNum(128)
		, nMaxCLS(0)
		, nMaxGMTOOL(8)
		, nTQChunkSize(100)
		, nQueueCmdInSize(2000000)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nSvrDropWaitTimeOut(20*1000)
		, nCalcAndSendCAAFSAvailPlayerTimeOut(10000)
		, nSvrCmdQueueSize(10*1024*1024)
		, nSvrCmdPerSecond(200)										// 重要指令不宜太多
		, bGZSDownRestorePlayer(false)
		{
			CNTRSVR.nPort	= GMS_DFT_PORT;
			memset(anMaxCLSNumInCAAFSGroup, 0, sizeof(anMaxCLSNumInCAAFSGroup));
			memset(anMaxPlayerNumInCLSOfCAAFSGroup, 0, sizeof(anMaxPlayerNumInCLSOfCAAFSGroup));
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// 用于共享内存中的结构
	// 对象数组
	typedef	whunitallocatorFixed3<CLSUnit, AO_T>		UAF_CLS_T;
	typedef	whunitallocatorFixed3<GMTOOLUnit, AO_T>		UAF_GMTOOL_T;
	typedef	whunitallocatorFixed3<PlayerUnit, AO_T>		UAF_Player_T;
	struct	MYSHMDATA_T
	{
		UAF_CLS_T		m_CLSs;
		UAF_GMTOOL_T	m_GMTOOLs;
		UAF_Player_T	m_Players;
		CAAFSGroup		m_aCAAFSGroup[GMS_MAX_CAAFS_NUM];
		GZSUnit			m_GZSs[GZS_MAX_NUM];

		MYSHMDATA_T()
		{
		}

		template<typename _Ty>
		void	AdjustInnerPtrAndEveryUnit(AM_T *pAM, _Ty &t, int nOffset)
		{
			t.AdjustInnerPtr(pAM->GetAO(), nOffset);
			// 把以分配链表中的每个单元的内容也Adjust一下
			for(typename _Ty::iterator it=t.begin(); it!=t.end(); ++it)
			{
				(*it).AdjustInnerPtr(nOffset);
			}
		}
		void	AdjustInnerPtr(AM_T *pAM, int nOffset)
		{
			AdjustInnerPtrAndEveryUnit(pAM, m_CLSs, nOffset);
			AdjustInnerPtrAndEveryUnit(pAM, m_GMTOOLs, nOffset);
			AdjustInnerPtrAndEveryUnit(pAM, m_Players, nOffset);
			wh_adjustarraybyoffset(m_aCAAFSGroup, GMS_MAX_CAAFS_NUM, nOffset);
			wh_adjustarraybyoffset(m_GZSs, GZS_MAX_NUM, nOffset);
			// 然后应该把m_GMTOOLs清空，因为这个是不需要重新利用的
			m_GMTOOLs.clear();
		}
		// 根据指针获得CAAFS的序号
		inline int	GetCAAFSGroupID(CAAFSGroup *pCAAFS)
		{
			return	wh_getidxinarray(m_aCAAFSGroup, pCAAFS);
		}
		//
		static MYSHMDATA_T *	NewMe(AM_T *pAM)
		{
			MYSHMDATA_T	*pObj;
			if( !pAM->New(pObj) )
			{
				return	NULL;
			}
			return	pObj;
		}
		static void	DeleteMe(AM_T *pAM, MYSHMDATA_T *&pObj)
		{
			if( !pObj )
			{
				return;
			}
			// 删除各个子对象中的内容
			pObj->m_CLSs.Release();
			pObj->m_GMTOOLs.Release();
			pObj->m_Players.Release();
			// 删除本对象
			pAM->Delete(pObj);
			pObj	= NULL;
		}
	};
private:
	typedef	CMN::ILogic					FATHERCLASS;
	// 配置结构
	CFGINFO_T							m_cfginfo;					// GMS的配置
	tcpmsger::DATA_INI_INFO_T			m_MSGER_INFO;				// GMS和其他Server用TCP连接通讯的msger的配置
	// TCP服务器
	MYCNTRSVR		m_CntrSvr;
	// 当前时刻
	whtick_t		m_tickNow;
	// 时间事件处理器
	whtimequeue		m_TQ;
	// 用于临时合成指令(所有不用二次整理的指令应该都可以通过他来发送)
	// 现在使用它的有: Send系列需要指令内容的函数，即所有最终目标是Player的函数
	// Tick中也可以使用它来生成给CLS的指令
	whvector<char>	m_vectrawbuf;
	// 定时统计个CAAFS组的可进入人数
	whtimequeue::ID_T	m_teid_CalcAndSend_CAAFS_AvailPlayerNum;
	// 需要使用到的其他logic模块(如果没有载入可以先加一个假的)
	ILogicBase		*m_pLogicGamePlay;
	whhash<pngs_cmd_t, ILogicBase *>	m_mapCmdReg2Logic;			// 用于指令分拣，把特定的指令分拣给特定的逻辑模块去执行
	whhashset<pngs_cmd_t>				m_setRegGZSSvrCmd;			// 用于把传给GZS的特定指令先缓冲，然后以一定速率发出
	// 用于缓冲GZSSvrCmd的队列
	whsmpqueue							m_queueGZSSvrCmd;
	// 用于生成每个tick转换到connecting队列的个数
	whgener								m_generSendGZSSvrCmd;
	// 用于获得每个tick的时间间隔
	whinterval							m_itv;
	// 内存管理模块
	ILogicBase		*m_pLogicMEM;
	AO_T			*m_ao;
	AM_T			*m_am;
	MYSHMDATA_T		*m_pSHMData;
	// 逻辑上可以容纳的最多人数
	int				m_nGMSLogicMaxPlayer;
private:
	friend	class	MYCNTRSVR;
	friend	class	MYCNTRSVR::MYCNTR;
	// 根据配置文件初始化参数
	int		Init_CFG(const char *cszCFGName);
	// 通过GroupID找到CAAFS
	CAAFSGroup *	GetCAAFSByGroupID(int nGroupID);
	// 针对一个Connecter做一次工作
	bool	CNTR_CanDealMsg() const;
	// 针对一个Connecter做一次工作
	int		CNTR_DealMsg_Dft(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_CAAFS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_CLS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_GZS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_GMTOOL(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	// 配合MYCNTRSVR的函数(不用是虚的)
	Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	void	AfterAddConnecter(Connecter * pCntr);
	void	BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo);
	// 让所有连接的服务器自动退出(这样GMS的端口就不会timewait了)
	void	TellAllConnecterToQuit();
	// 让所有的CAAFS退出
	void	TellAllCAAFSQuit();
	// 让所有的CLS退出
	void	TellAllCLSQuit();
	// 让所有的GZS退出
	void	TellAllGZSQuit();
	// 让CLS踢出用户(之所以这样写是为了强调踢出用户永远都是从CLS发起的)
	void	TellCLSToKickPlayer(int nClientID, pngs_cmd_t nSubCmd = SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID);
	//
	void	TellCLSToCheckPlayerOffline(CLSUnit *pCLS);
	void	TellGZSToCheckPlayerOffline(GZSUnit *pGZS);
	// 移除一个Connecter（并移除其附带的对象，如：CAAFSGroup/CLSUnit/GZSUnit等等）
	int		RemoveMYCNTR(int nCntrID);
	int		RemoveMYCNTR(MYCNTRSVR::MYCNTR *pCntr);
	int		RemovePlayerUnit(int nID, unsigned char nRemoveReason = CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTHING);
	int		RemovePlayerUnit(PlayerUnit *pPlayer, unsigned char nRemoveReason = CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTHING);
	// 发送一个GZS的信息给一个CLS(这样CLS就可以开始连接它了。函数内部还会改变CLS的状态为STATUS_CONNECTINGGZS)
	int		SendGZSInfo0ToCLSAndChangeCLSStatus(GZSUnit *pGZS, CLSUnit *pCLS);
	// 发送CLS的所有在线用户ID给GZS
	int		SendPlayerIDInCLSToGZS(CLSUnit *pCLS, GZSUnit *pGZS);
	// 给所有的CLS发送一条指令
	int		SendCmdToAllCLS(const void *pCmd, size_t nSize);
	// 给Player所在的CLS发送一条指令(内部直接把指令传递给CLS，没有使用m_vectrawbuf，所以上层可以使用m_vectrawbuf来生成指令)
	int		SendCmdToPlayerCLS(int nClientID, const void *pCmd, size_t nSize);
	int		SendCmdToPlayerCLS(PlayerUnit *pPlayer, const void *pCmd, size_t nSize);
	// 给所有的GZS发送一条指令
	int		SendCmdToAllGZS(const void *pCmd, size_t nSize);
	// 把一条向GZS发送的低级指令放入缓冲队列（这样做有时是为了保持指令次序）
	int		QueueRawCmdToGZS(unsigned char nSvrIdx, const void *pCmd, size_t nSize);
	// 向GZS发一条SvrCmd
	int		SendSvrCmdToAllGZS(const void *pCmd, size_t nSize, unsigned char nFromSvrIdx, unsigned char nExceptSvrIdx=0);
	int		SendSvrCmdToGZS(unsigned char nSvrIdx, const void *pCmd, size_t nSize, unsigned char nFromSvrIdx);
	int		SendSvrCmdToGZS(GZSUnit *pGZS, const void *pCmd, size_t nSize, unsigned char nFromSvrIdx);
	// 给所有的GMTOOL发送一条指令
	int		SendCmdToAllGMTOOL(const void *pCmd, size_t nSize);
	// 给Player发送指令(内部会包装成发给CLS的指令)
	int		SendCmdToPlayer(int nPlayerID, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToPlayer(PlayerUnit *pPlayer, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS(int nCLSID, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS(CLSUnit *pCLS, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToAllPlayer(unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	int		SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	// 设置Player的Tag值(内部包装成SVR_CLS_SET_TAG_TO_CLIENT发送给CLS)
	int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal);
	// Tick中的方法
	// 时间事件处理
	int		Tick_DealTE();
	// 其他服务器连入没有发送hello的超时
	void	TEDeal_Hello_TimeOut(TQUNIT_T *pTQUnit);
	// 设置上面的事件
	void	SetTE_Hello_TimeOut(Connecter * pCntr);
	// GZS断线等待的超时
	void	TEDeal_GZS_DropWait_TimeOut(TQUNIT_T *pTQUnit);
	// 设置上面的事件
	void	SetTE_GZS_DropWait_TimeOut(GZSUnit *pGZS);
	// CLS断线等待的超时
	void	TEDeal_CLS_DropWait_TimeOut(TQUNIT_T *pTQUnit);
	// 设置上面的事件
	void	SetTE_CLS_DropWait_TimeOut(CLSUnit *pCLS);
	// 定期计算各组CAAFS还可进入的用户数量的间隔
	void	TEDeal_CalcAndSend_CAAFS_AvailPlayerNum(TQUNIT_T *pTQUnit);
	// 处理用户发来的指令(内部应该是转发给相应的逻辑处理单元)
	void	InnerRouteClientGameCmd(CMN::cmd_t nCmd, CLS_SVR_CLIENT_DATA_T *pCmd, size_t nSize);
	// 处理GZS发来的指令(内部根据目标服务器索引的不同转发给GMSGamePlay或者是相应GZS的GZSGamePlay)
	void	InnerRouteGZSGameCmd(MYCNTRSVR::MYCNTR *pCntr, GZS_GMS_GAMECMD_T *pGameCmd, size_t nSize);
	// 处理有关GZS断线的一些事情，比如给谁谁发送什么消息之类的
	int		DealGZSDisconnection(GZSUnit *pGZS);
	// 处理有关CLS断线的一些事情，比如给谁谁发送什么消息之类的
	int		DealCLSDisconnection(CLSUnit *pCLS);

	// GM指令处理
	int		DealCmdIn_PNGSPACKET_2LOGIC_GMCMD(ILogicBase *pRstAccepter, PNGSPACKET_2LOGIC_GMCMD_T *pCmd);
	// 只处理字串指令的部分
	int		InitTstStrMap();
	int		DealTstStr(const char *cszTstStr, char *szRstStr, size_t *pnSize);
	///////////////////////////////////////
	// 处理字串指令需要用到的其他东西
	///////////////////////////////////////
	whcmn_strcmd_reganddeal_DECLARE_m_srad(GMS_MAINSTRUCTURE);
	// 当前可以写入结果的缓冲指针开头（可能随着不断的写入而发生变化）
	char	*m_pszRstStr;
	// 可以写入的长度
	size_t	*m_pnRstSize;
	//
	int		_i_srad_NULL(const char *param);
	int		_i_srad_help(const char *param);
	int		_i_srad_getclsinfo(const char *param);
	int		_i_srad_setclsinfo(const char *param);

public:
	// 创建自己类型的对象
	static CMN::ILogic *	Create()
	{
		return	new	GMS_MAINSTRUCTURE;
	}
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_GMS_I_GMS_MAINSTRUCTURE_H__
