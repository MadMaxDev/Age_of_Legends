//////////////////////////////////////////////////////////////////////////
// web扩展的逻辑层
// 目前所有的消息处理都没有使用wcs,而是直接用结构(wcs会安全很多,消息结构很多地方可能需要判断长度等)
// 切记:一定要保证CLS4Web,LPMainStructure4Web,LPGamePlay4Web,GameMngS中Player ID的一致性
// 目前只能在角色列表界面和正常游戏中才能登出,正常游戏中才能重选人物
// 切记:SendErrRstToClient_And_Done(只要包含了IAmDone)就要立即return,因为对象已经析构了
// 
// DoNext_TTY_TC_PLAYER_DROP中立即进行PlayerUnit释放,不然PlayerDrop和TimeEvent同时触发时会出问题(如PlayerDrop,DBS当掉,Transaction等待DBS返回)
// 在TimeEvent触发之前,该PlayerUnit不被释放,导致玩家登录不了(LPGamePlay和LPMainStructure中的Players不能保持一致导致)
//////////////////////////////////////////////////////////////////////////
#ifndef __LPGamePlay4Web_i_H__
#define __LPGamePlay4Web_i_H__

#include "PNGS/inc/pngs_def.h"
#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_packet_logic.h"

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whunitallocator3.h"
#include "WHCMN/inc/whtransaction.h"
#include "WHCMN/inc/whcmd.h"
#include "WHCMN/inc/whmd5.h"
#include "WHCMN/inc/whgoodstr.h"

#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnettcp.h"

#include "../../Common/inc/pngs_packet_db4web_logic.h"
#include "../../Common/inc/pngs_packet_lp_logic.h"
#include "../../Common/inc/pngs_packet_web_extension.h"
#include "../../Common/inc/tty_def_transaction_cmd.h"
#include "../../Common/inc/tty_def_gs4web.h"
#include "../../Common/inc/tty_common_db_timeevent.h"
#include "../../Common/inc/tty_common_db_timeevent.h"
#include "../../Common/inc/tty_common_excel_def.h"

#include "LPNet.h"
#include "GameMngS.h"

namespace n_pngs
{
//////////////////////////////////////////////////////////////////////////
// LPGamePlay
//////////////////////////////////////////////////////////////////////////
class LPGamePlay4Web_i : public CMN::ILogic, public CLPNet
{
// 为接口实现
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy();
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
// 自己用的
public:
	LPGamePlay4Web_i();
	~LPGamePlay4Web_i();
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		// 基本参数
		int					nQueueCmdInSize;					// 指令输入队列的长度
		unsigned short		nSvrGrpID;							// 服务器组ID
		int					nMaxPlayer;							// 最多可承载的玩家数量
		int					nTQChunkSize;						// 时间队列每次分配的块大小
		int					nNoLoginTimeOut;					// 非LoginOK状态的超时
		int					nPlayerDropWaitTimeOut;				// 用户断线之后等待一段时间再删除
		int					nPlayerCloseDelayTimeOut;			// 用户主动关闭连接之后也需要等待一段时间才能通知用户下线
		int					nMaxTransaction;					// 最多的Transaction数量
		int					nClearOfflineTimeOut;				// 清除用户offline标记的超时
		int					nTransactionTimeOut;				// 一般的transaction超时时间
		int					nMaxCharPerAccount;					// 每个帐号拥有的最大角色数
		bool				bForceAccountNameLwr;				// 是否需要把帐号强制转换为小写
		int					nReplaceCount;						// 账号被人尝试连续顶替的次数(超过一定数量就会被顶掉),<=0表示永不可顶
		int					nReplaceTimeOut;					// 如果上次被尝试顶替的时间距离现在超过这个超时则nReplaceCount会重新恢复初始值(单位:s)
		char				szGS4WebAddr[WHNET_MAXADDRSTRLEN];	// gs4web的地址
		int					nDealDBTimeEventTimeOut;			// 处理DB时间事件的超时
		bool				bCreateAccountIfNotExist;			// 没有账号时则进行创建
		bool				bUseRandomAccountName;				// 在账号不存在需要创建账号时是否使用随机账号
																// 如,使用91账号第一次登陆时(由客户端传入,此时配置bUseRandomAccountName=false)
																// 只有设置了bCreateAccountIfNotExist=true时该参数才有意义
																// bCreateAccountIfNotExist=false时是否使用随机名字是由客户端传来的
		int					nDealPayTimeOut;					// 处理充值的时间事件
		char				szBadCharNameFile[WH_MAX_PATH];			// 非法角色名字列表
		char				szBadAccountNameFile[WH_MAX_PATH];		// 非法账号名字列表

		int					nDealCombatResultTimeOut;			// 处理战斗结果的超时
		
		CFGINFO_T()
		: nQueueCmdInSize(2*1024*1024)							// 这个是给异步输入指令使用的,没必要太大
		, nSvrGrpID(0)
		, nMaxPlayer(128)
		, nTQChunkSize(100)
		, nNoLoginTimeOut(1*60*1000)
		, nPlayerDropWaitTimeOut(2*60*1000)
		, nPlayerCloseDelayTimeOut(10*1000)
		, nMaxTransaction(100*1000)
		, nClearOfflineTimeOut(20*1000)
		, nTransactionTimeOut(TTY_TRANSACTION_TIMEOUT)
		, bForceAccountNameLwr(true)
		, nReplaceCount(1)
		, nReplaceTimeOut(60)
		, nDealDBTimeEventTimeOut(2*1000)
		, bCreateAccountIfNotExist(false)
		, bUseRandomAccountName(true)
		, nDealPayTimeOut(2*1000)
		, nDealCombatResultTimeOut(200)
		{
			WH_STRNCPY0(szGS4WebAddr, "localhost:4800");
			WH_STRNCPY0(szBadCharNameFile, "../excel_tables/etc/badname.txt");
			WH_STRNCPY0(szBadAccountNameFile, "../excel_tables/etc/badname_account.txt");
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};

	// 逻辑玩家对象
	struct PlayerUnit 
	{
		struct STATUS_T 
		{
			// 各种状态
			enum
			{
				NOTHING			= 0,						// 也可以认为是PRELOGINING
				CREATEACCOUNT	= 1,						// 创建账号
				LOGINING		= 2,						// 正在LOGIN过程中,如果LOGIN失败则需要将相应映射删除
				CREATECHAR		= 3,						// 在创建角色过程中
				INLP			= 4,						// 在LP中正常工作了
				DROPWAIT		= 5,						// 断线之后的等待状态
				CLOSEWAIT		= 6,						// 关闭等待中
				MAXNUM,
			};
			// 各种处理函数类型
			// 处理玩家掉线
			typedef	void	(LPGamePlay4Web_i::*Player_Func_Deal_Drop_T)(PlayerUnit* pPlayer);
			// 处理玩家发来的指令
			typedef	void	(LPGamePlay4Web_i::*Player_Func_Deal_PlayerCmd_T)(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
			
			int				nStatus;						// 状态值
			// 各个处理函数
			Player_Func_Deal_Drop_T			pFuncDealDrop;
			Player_Func_Deal_PlayerCmd_T	pFuncDealCreateAccount;			// 创建账号等
			Player_Func_Deal_PlayerCmd_T	pFuncDealLogin;
			Player_Func_Deal_PlayerCmd_T	pFuncDealCreateChar;			// 创建角色			
			Player_Func_Deal_PlayerCmd_T	pFuncDealGameCmd;
			Player_Func_Deal_PlayerCmd_T	pFuncDealLogout;
			Player_Func_Deal_PlayerCmd_T	pFuncDealLoginByDeviceID;		// 通过设备号登陆
			
			STATUS_T();
		};
		static	STATUS_T	aStatus[STATUS_T::MAXNUM];
		// 为不同状态填充函数
		static	void		StatusObjInit();

		int					nID;										// 在数组中的ID(Client ID)
		int					nStatus;									// 加上这个是为了从共享内存中恢复的时候重新指向新的status对象
		STATUS_T*			pStatus;									// 状态指针
		unsigned long		IP;											// 来源IP
		char				szAccount[TTY_ACCOUNTNAME_LEN];				// 帐号,暂时其实没啥用
		char				szDeviceID[TTY_DEVICE_ID_LEN];				// 机器设备ID
		char				szPass[TTY_PASSWORD_LEN];
		time_t				nInTime;									// 连接成功的时间
		int					nPassTryCount;
		unsigned char		nBinded;									// 是否绑定EMail了
		unsigned int		nGiftAppID;									// 一台设备第一次创建账号时生成的,之后该设备某个版本的程序都为0
		unsigned int		nAppID;										// 本次登录的appid

		enum
		{
			PROCESSINGCMD_CREATEACCOUNT		= 0x01,						// 正在处理创建账号请求
			PROCESSINGCMD_CREATECHAR		= 0x02,						// 正在处理创建角色请求
		};
		unsigned char		nProcessingCmd;								// 表明现在在处理什么样的指令
		unsigned char		nDropReason;								// 用户断线的原因(CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_XXX)
		int					nCurTransactionID;							// 当前关联的transaction id
		whtimequeue::ID_T	teid;
// 		// 以下的数据需要在后续实现
		tty_id_t			nAccountID;									// 帐号OnlyID
 		unsigned char		nReplaceCount;								// 被人尝试连续顶替的次数(超过一定数就被顶掉)
 		time_t				nLastReplaceTime;							// 上次被人尝试顶替时刻,如果太早就重新计数
		int					nTermType;									// 终端类型

		PlayerUnit();
		void		clear();
		inline	void	SetStatus(int s)
		{
			assert(s>=0 && s<STATUS_T::MAXNUM);
			nStatus		= s;
			pStatus		= &aStatus[nStatus];
		}
		inline	int		GetStatus() const
		{
			return pStatus->nStatus;
		}
		inline void	SetTransactionID(int nID)
		{
			assert(nCurTransactionID<=0);
			nCurTransactionID	= nID;
		}
	};

	// 时间事件单元
	struct TQUNIT_T
	{
		typedef	void	(LPGamePlay4Web_i::*TEDEAL_T)(TQUNIT_T*);
		TEDEAL_T	tefunc;								// 处理事件的函数
		// 数据部分
		union
		{
			struct PLAYER_T 
			{
				int	nID;								// 玩家ID
			}player;
			void*	pExt;
			int		nExt;
		}un;
	};

private:
	friend	struct	PlayerUnit;
	friend	struct	PlayerUnit::STATUS_T;
	typedef	CMN::ILogic	FATHERCLASS;
	// 配置结构
	CFGINFO_T		m_cfginfo;
	// 当前时刻
	whtick_t		m_tickNow;			// 当前tick
	time_t			m_timeNow;			// 当前时间
	// 用于临时合成指令
	whvector<char>	m_vectrawbuf;
	// 帐号ID对应玩家ID
	whhash<whint64, int>				m_mapAccountID2ClientID;
	// 尝试Login中的帐号名对应玩家ID
	whhash<whstrptr4hash, int>			m_mapTryLoginAccountName2ClientID;
	// 时间队列
	whtimequeue							m_TQ;
	// 事务管理器
	whtransactionman*					m_pTMan;
	// 玩家单元
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// 主框架模块
	ILogicBase*							m_pLogicMainStructure;
	ILogicBase*							m_pLogicDB;
private:
	CGameMngS::CFGINFO_T				m_GameMngCfg;
	CGameMngS*							m_pGameMngS;
private:
	bool								m_bLoadTerrainOK;
	bool								m_bLoadPlayerCardOK;
	bool								m_bLoadExcelTextOK;
	bool								m_bLoadExcelAllianceCongressOK;
	bool								m_bLoadCharLevelRankOK;
	bool								m_bNeedUpdatePlayerCardLevelRankData;
	bool								m_bNeedGetWorldFamousCity;
	bool								m_bNeedGenWorldFamousCity;
	bool								m_bCanGenWorldFamousCity;
	bool								m_bClearWorldFamousCityFirstWhenGen;
	bool								m_bAlreadyGetWorldFamousCity;

	bool								m_bNeedGetWorldRes;
	bool								m_bNeedGenWorldRes;
	bool								m_bCanGenWorldRes;
	bool								m_bAlreadyGetWorldRes;
private:
	whdaytimelooper						m_dtlHour;
	whdaytimelooper						m_dtlMinute;
	bool								m_bAlreadyLoadRank;
	whbadword*							m_pBadCharNameFilter;
	whbadword*							m_pBadAccountNameFilter;
	int		SafeInitBadNameFilter();
private:
	// 根据配置文件初始化参数
	int		Init_CFG(const char* cszCFGName);
	// 根据配置文件设置一些参数
	int		Init_SetParams();
	//////////////////////////////////////////////////////////////////////////
	// 处理玩家指令
	//////////////////////////////////////////////////////////////////////////
	// 各个子函数可能会抛出的错误
	enum
	{
		PLAYERCMD_ERR_SUCCESS			= 0,					// 没错
		PLAYERCMD_ERR_NOTSUPPORT		= 1,					// 不支持这样的指令
		PLAYERCMD_ERR_INVALIDCMD		= 2,					// 不认识的指令
		PLAYERCMD_ERR_CMDREAD			= 3,					// 解析指令时出错了
		PLAYERCMD_ERR_NOCHAR			= 5,					// 没有创建角色的情况下不能进行游戏
		PLAYERCMD_ERR_INVALIDPARAM		= 8,					// 错误的参数
		PLAYERCMD_ERR_BADPASSWD			= 9,					// 错误的密码
		PLAYERCMD_ERR_RESTRICTED		= 10,					// 角色被禁止游戏
		PLAYERCMD_ERR_BADGROUPID		= 11,					// 错误的大区号
		PLAYERCMD_ERR_REFUSED			= 81,					// 操作被拒绝
		PLAYERCMD_ERR_TRANSACTION		= 254,					// transaction分配出错
		PLAYERCMD_ERR_UNKNOWN			= 255,					// 未知错误
	};
	int		DealCmdIn_LPPACKET_2GP_PLAYERCMD(ILogicBase* pRstAccepter, LPPACKET_2GP_PLAYERCMD_T* pCmd);
	// 处理CAAFS4Web发来的玩家指令
	void	DealCmdIn_LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web(ILogicBase* pRstAccepter, LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web_T* pCmd);
	
	//////////////////////////////////////////////////////////////////////////
	// 根据player状态不同的不同处理函数
	//////////////////////////////////////////////////////////////////////////
	// 处理玩家断线的
	// 什么也不做
	void	Player_Func_Deal_Drop_Ignore(PlayerUnit* pPlayer);
	// 不用处理(只是删除即可)
	void	Player_Func_Deal_Drop_Nothing(PlayerUnit* pPlayer);
	// 在开始LOGIN之后的处理(包括还没有受到返回的情况)
	void	Player_Func_Deal_Drop_AfterLogin(PlayerUnit* pPlayer);
	
	// 处理玩家指令
	// 状态不支持这样的处理
	void	Player_Func_Deal_PlayerCmd_NotSupport(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// 忽略指令
	void	Player_Func_Deal_PlayerCmd_Ignore(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// 创建账号
	void	Player_Func_Deal_PlayerCmd_Client_CreateAccount(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// 处理Login(只能在STATUS::NOTHING状态下使用)
	void	Player_Func_Deal_PlayerCmd_Client_Login(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// 处理LoginByDeviceID(只能在STATUS::NOTHING状态下使用)
	void	Player_Func_Deal_PlayerCmd_Client_LoginByDeviceID(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// 创建角色指令(这个只能在STATUS_T::LOGINING之后游戏开始前使用)
	void	Player_Func_Deal_PlayerCmd_Client_CreateChar(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// 开始游戏
	// 游戏中
	void	Player_Func_Deal_PlayerCmd_Client_GameCmd(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// 登出游戏
	void	Player_Func_Deal_PlayerCmd_Client_Logout(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);

	//////////////////////////////////////////////////////////////////////////
	// 其他
	//////////////////////////////////////////////////////////////////////////
	// 让MS踢出用户(所有踢出用户的操作必须最终由CLS4Web开始)
	int		TellLogicMainStructureToKickPlayer(int nClientID, pngs_cmd_t nKickSubCmd=SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID);
	int		KickPlayerOffline(int nClientID, pngs_cmd_t nKickSubCmd/* =SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID */)
	{
		return TellLogicMainStructureToKickPlayer(nClientID, nKickSubCmd);
	}
	// 通知MS层可用的玩家数量
	int		TellLogicMainStructureLogicAvailPlayerNum();
	// 通知DB用户应该logout(包括在线表删除)
	int		TellDBPlayerLogout(tty_id_t nAccountID);
	// 通知全局用户下线(主要是更新下线时间)
	int		TellGlobalDBPlayerLogout(tty_id_t nAccountID);
	
	// 设置玩家状态
	int		SetPlayerStatus(PlayerUnit* pPlayer, int nStatus);
	// 设置CLS4Web中对应的名字串
	int		SetPlayerAccountIDInCLS4Web(PlayerUnit* pPlayer);

	// 用户在LOGIN状态下的清理工作
	int		ClearPlayerAfterLogin(PlayerUnit* pPlayer);

	// 把帐号名转为小写(如果需要的话)
	char*	AccountLwr(char* szName);

	// 处理数据库指令
	int		DealDBCmd(P_DBS4WEB_CMD_T* pBaseCmd, size_t nDSize);

	//////////////////////////////////////////////////////////////////////////
	// 发送指令给用户
	//////////////////////////////////////////////////////////////////////////
	int		SendCmdToClient(int nClientID, const void* pCmd, size_t nSize);
	int		SendCmdToMultiClient(int* aClientID, int nClientNum, const void* pCmd, size_t nSize);
	int		SendCmdToClientWithTag(unsigned char nTagIdx, short nTagVal, const void* pCmd, size_t nSize);
	int		SendCmdToClientWithTag64(whuint64 nTag, const void* pCmd, size_t nSize);
	int		SendCmdToAllClient(const void* pCmd, size_t nSize);
	int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal);
	int		SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel);

	int		SendAlliInstanceNotify(tty_id_t nInstanceID, tty_id_t nAccountID, int nAction);

	// 下面两个函数会用到m_vectrawbuf
	void	SendPlayerCharAtbToClient(PlayerData* pPlayer, bool bSimple=true);
	void	RefreshPlayerCharAtbFromDB(tty_id_t nAccountID, bool bSimple=true, bool bCheckOnline=false);
	void	RefreshPlayerCharAtbFromDB(unsigned int nNum, tty_id_t* pAccountIDs, bool bSimple=true, bool bCheckOnline=false);
	void	RefreshPlayerCharAtbFromDB(const char* cszAccountIDs, bool bSimple=true, bool bCheckOnline=false);
	void	GetHeroSimpleData(unsigned int nCombatType, tty_id_t nExtID, const char* cszHeroIDs, unsigned int nReason); // ,分隔的hero_id串
	void	GetHeroSimpleData(unsigned int nCombatType, tty_id_t nExtID, unsigned int nNum, tty_id_t*	pHeroIDs, unsigned int nReason);
	void	RefreshDrug(const char* cszAccountIDs); // ,分隔的account_id串
	void	RefreshDrug(unsigned int nNum, tty_id_t* pAccountIDs);
	int		AddMail(tty_id_t nSenderID, const char*	cszSenderName, tty_id_t nReceiverID, bool bNeedRst, unsigned char nType, unsigned char nFlag, bool bSendLimit, int nTextLen, const char* cszText, int nExtDataLen, void* pExtData, int pAddedExtData[PNGSPACKET_2DB4WEB_EXT_NUM]);
	int		RechargeDiamond(tty_id_t nAccountID, unsigned int nAddedDiamond, bool bFromGS=false, const int* pExt=NULL);
	int		NotifyMeRefreshLevelRankOfPlayerCard(bool bLoadLevelRankOK)
	{
		m_bNeedUpdatePlayerCardLevelRankData	= true;
		m_bLoadCharLevelRankOK					= bLoadLevelRankOK;
		return 0;
	}
	// 防止注入式攻击
	int		AntiSQLInjectionAttack(int nMaxLen, char* szStr);
	// 名字是否合法
	bool	IsCharNameValid(const char* cszName);
	bool	IsAccountNameValid(const char* cszName);

	void	AddMoneyCostLog(tty_id_t nAccountID, unsigned int nUseType, unsigned int nMoneyType, unsigned int nMoney
		, unsigned int nExtData0=0, unsigned int nExtData1=0, unsigned int nExtData2=0, unsigned int nExtData3=0);
	void	AddGoldProduceLog(tty_id_t nAccountID, unsigned int nProduceType, unsigned int nGold);
	void	AddGoldCostLog(tty_id_t nAccountID, unsigned int nCostType, unsigned int nGold);

	void	GetPlayerCardFromDB(tty_id_t nAccountID);
	void	AddNotifyInWorldChat(unsigned int nTextLen, const char* cszText, tty_id_t nCharID=0);

	// 获取大区序号
	unsigned char		GetSvrGroupIndex()
	{
		return (unsigned char)m_cfginfo.nSvrGrpID;
	}

	unsigned int	GetPlayerAppID(unsigned int nClientID)
	{
		PlayerUnit*	pPlayer	= GetPlayerByID(nClientID);
		if (pPlayer != NULL)
		{
			return pPlayer->nAppID;
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// 向数据库发送异步指令
	//////////////////////////////////////////////////////////////////////////
	int		SendCmdToDB(const void* pData, size_t nSize);

	//////////////////////////////////////////////////////////////////////////
	// 发送消息到全局服务器
	//////////////////////////////////////////////////////////////////////////
	int		SendCmdToGS(const void* pData, size_t nSize)
	{
		return m_msgerGS4Web.SendMsg(pData, nSize);
	}

	//////////////////////////////////////////////////////////////////////////
	// 时间事件相关
	//////////////////////////////////////////////////////////////////////////
	// 时间事件处理
	int		Tick_DealTE();
	// 用于处理非login ok状态的超时
	void	TEDeal_NoLogin_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	void	SetPlayerTE_NoLogin(PlayerUnit* pPlayer);
	// 用于处理用户断线的延时
	void	TEDeal_PlayerDropWait_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	void	SetPlayerTE_PlayerDropWait(PlayerUnit* pPlayer);
	// 用于处理关闭超时
	void	TEDeal_PlayerCloseDelay_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	void	SetPlayerTE_PlayerCloseDelay(PlayerUnit* pPlayer, int nDelay);
	// 用于发起DB时间事件处理
	void	TEDeal_Deal_DB_TimeEvent_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	void	SetTE_DealDBTimeEvent();
	// 用于发起处理充值时间事件处理
	void	TEDeal_Deal_Pay_TimeEvent_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	void	SetTE_DealPayTimeEvent();
	// 用于发起AdColony时间事件处理
	void	TEDeal_Deal_AdColony_TimeEvent_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	void	SetTE_DealAdColonyTimeEvent();
	// 处理战斗结果的超时
	void	TEDeal_Deal_GetAlliInstanceCombatResultEvent(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	void	SetTE_GetAlliInstanceCombatResultEvent();

	void	DealExcelTimeEvent();
	void	DealGoldMarketTimeEvent();

	//////////////////////////////////////////////////////////////////////////
	// 卡号相关的
	//////////////////////////////////////////////////////////////////////////
	// 尝试挤用户下线
	int		TryReplacePlayer(int nOldClientID, const char* cszCharPassMD5, unsigned int nFromIP, bool& bHasPass);
	// 用新的用户继承旧用户
	// 返回,0->成功继承,1->密码不对
	int		InheritDropWaitPlayer(int nNewClientID, int nOldClientID, unsigned char nSvrIdx, const char* cszCharPassMD5);
	// 把处于dropwait或者closewait状态的用户清除掉(用户选择的)
	int		CleanDropWaitPlayer(int nClientID);
	int		CleanDropWaitPlayer(PlayerUnit* pPlayer);
	// 延迟踢出卡号用户
	int		WaitAndKickPlayer(PlayerUnit* pPlayer);
	// 发送有人尝试登录的warning给用户
	int		SendReplaceWarningToPlayer(int nClientID, unsigned int nFromIP);
	
	//////////////////////////////////////////////////////////////////////////
	// 玩家信息相关操作
	//////////////////////////////////////////////////////////////////////////
	// 通过在线ID获得玩家对象指针
	inline	PlayerUnit*	GetPlayerByID(int nClientID)
	{
		return m_Players.getptr(nClientID);
	}
	inline	PlayerUnit* GetPlayerByAccountID(tty_id_t nAccountID)
	{
		int	nClientID	= 0;
		if (m_mapAccountID2ClientID.get(nAccountID, nClientID))
		{
			return m_Players.getptr(nClientID);
		}
		return NULL;
	}
	PlayerData*	GetPlayerData(tty_id_t nAccountID)
	{
		PlayerUnit*	pPlayer	= GetPlayerByAccountID(nAccountID);
		if (pPlayer != NULL)
		{
			return	m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pPlayer->nID);
		}
		return NULL;
	}

	// 
	// 通知client战斗结果
	void	SendMsgToClient(tty_id_t nAccountID, const void *pData, int nSize);

public:
	//////////////////////////////////////////////////////////////////////////
	// 各个transaction的定义
	//////////////////////////////////////////////////////////////////////////
	// LOGIN过程
	class Transaction_Login : public whtransactionbasewithTQID
	{
	// 为接口实现
	public:
		virtual	~Transaction_Login();
		virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize);
		virtual	int		DealTimeEvent(int nEvent);
		WHTRANSACTION_DECLARE_GETTYPESTR(LP, Transaction_Login);
	// 自己用的
	public:
		LPGamePlay4Web_i*	m_pHost;
		PlayerUnit*			m_pPlayer;								// 只能在Init中用,之后都可能失效!!!
		int					m_nClientID;
		unsigned int		m_nIP;
		whint64				m_nAccountID;
		unsigned int		m_nDeviceType;
		char				m_szAccount[TTY_ACCOUNTNAME_LEN];
		char				m_szPassword[TTY_ACCOUNTPASSWORD_LEN];
		char				m_szDeviceID[TTY_DEVICE_ID_LEN];
		int					m_nPassLen;
		bool				m_bPlayerDropped;						// 表示用户中途掉线了
		int					m_nCurTE;								// 当前的时间事件
		char				m_szRealDeviceID[TTY_DEVICE_ID_LEN];
	public:
		Transaction_Login(LPGamePlay4Web_i* pHost);
	private:
		int		DoNext_TTY_TC_INIT(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GLOBAL_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_CHAR_LOAD_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize);
		// 发送创建的结果给客户端(只有出错了采用这个发送)
		int		SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt="");
		int		ClearTryLoginPlayer_And_Done();
		int		ClearLoginPlayer_And_Done();
	private:
		enum
		{
			TE_NONE					= 0,
			TE_GLOBAL_LOGIN			= 1,
			TE_GROUP_LOGIN			= 2,
			TE_CHAR_LOAD			= 3,
		};
	};
	friend class Transaction_Login;

	// LOGIN过程
	class Transaction_LoginByDeviceID : public whtransactionbasewithTQID
	{
		// 为接口实现
	public:
		virtual	~Transaction_LoginByDeviceID();
		virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize);
		virtual	int		DealTimeEvent(int nEvent);
		WHTRANSACTION_DECLARE_GETTYPESTR(LP, Transaction_LoginByDeviceID);
		// 自己用的
	public:
		LPGamePlay4Web_i*	m_pHost;
		PlayerUnit*			m_pPlayer;								// 只能在Init中用,之后都可能失效!!!
		int					m_nClientID;
		unsigned int		m_nIP;
		whint64				m_nAccountID;
		unsigned int		m_nDeviceType;
		char				m_szDeviceID[TTY_DEVICE_ID_LEN];
		bool				m_bPlayerDropped;						// 表示用户中途掉线了
		int					m_nCurTE;								// 当前的时间事件
		char				m_szRealDeviceID[TTY_DEVICE_ID_LEN];
	public:
		Transaction_LoginByDeviceID(LPGamePlay4Web_i* pHost);
	private:
		int		DoNext_TTY_TC_INIT(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GLOBAL_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_CHAR_LOAD_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize);
		// 发送创建的结果给客户端(只有出错了采用这个发送)
		int		SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt="");
		int		ClearTryLoginPlayer_And_Done();
		int		ClearLoginPlayer_And_Done();
	private:
		enum
		{
			TE_NONE					= 0,
			TE_GLOBAL_LOGIN			= 1,
			TE_GROUP_LOGIN			= 2,
			TE_CHAR_LOAD			= 3,
		};
	};
	friend class Transaction_LoginByDeviceID;

	// CREATEACCOUNT过程
	class Transaction_CreateAccount : public whtransactionbasewithTQID
	{
	// 为接口实现
	public:
		virtual	~Transaction_CreateAccount();
		virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize);
		virtual	int		DealTimeEvent(int nEvent);
		WHTRANSACTION_DECLARE_GETTYPESTR(LP, Transaction_CreateAccount);
	// 自己用的
	public:
		LPGamePlay4Web_i*	m_pHost;
		int					m_nClientID;
		bool				m_bPlayerDropped;						// 表示用户中途掉线了
		char				m_szAccountName[TTY_ACCOUNTNAME_LEN];
		char				m_szPass[TTY_ACCOUNTPASSWORD_LEN];
		char				m_szDeviceID[TTY_DEVICE_ID_LEN];
		unsigned int		m_nDeviceType;
		bool				m_bUseRandomName;						// 是否使用随机名字
		unsigned int		m_nAppID;
	public:
		Transaction_CreateAccount(LPGamePlay4Web_i* pHost);
	private:
		int		DoNext_TTY_TC_INIT(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GLOBAL_ACCOUNT_CREATE(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize);
		int		SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt="");
		int		ClearPlayer_And_Done();
	};
	friend class Transaction_CreateAccount;

	// CREATECHAR过程
	class Transaction_CreateChar : public whtransactionbasewithTQID
	{
	// 为接口实现
	public:
		virtual	~Transaction_CreateChar();
		virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize);
		virtual	int		DealTimeEvent(int nEvent);
		WHTRANSACTION_DECLARE_GETTYPESTR(LP, Transaction_CreateChar);
	// 自己用的
	public:
		LPGamePlay4Web_i*	m_pHost;
		int					m_nClientID;
		tty_id_t			m_nAccountID;
		unsigned int		m_nPosX;
		unsigned int		m_nPosY;
		bool				m_bPlayerDropped;						// 表示用户中途掉线了
		bool				m_bCreateOK;							// 是否已经完成创建了
		unsigned char		m_nBinded;
		unsigned int		m_nGiftAppID;
	public:
		Transaction_CreateChar(LPGamePlay4Web_i* pHost);
	private:
		int		DoNext_TTY_TC_INIT(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GROUP_CHAR_CREATE(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_CHAR_LOAD_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize);
		int		SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt="");
		int		ClearPlayer_And_Done();
	};

	//////////////////////////////////////////////////////////////////////////
	// 和GS4Web的连接
	//////////////////////////////////////////////////////////////////////////
	tcpmsger::DATA_INI_INFO_T		m_msgerGS4Web_info;
	class MYMSGER_T : public tcpretrymsger<tcpmsger>
	{
	public:
		LPGamePlay4Web_i*	m_pHost;
	public:
		virtual	void	i_Worker_WORKING_Begin()
		{
			GS4WEB_FIRSTIN_T	Cmd;
			Cmd.nCmd			= P_GS4WEB_FIRSTIN;
			Cmd.nSubCmd			= 0;
			Cmd.nGroupID		= m_pHost->m_cfginfo.nSvrGrpID;
			SendMsg(&Cmd, sizeof(Cmd));
		}
	};
	friend	class			MYMSGER_T;
	MYMSGER_T				m_msgerGS4Web;

	//////////////////////////////////////////////////////////////////////////
	// GS4Web相关
	//////////////////////////////////////////////////////////////////////////
	int			Tick_DealGS4WebMsg();
	int			_Tick_DealGS4WebMsg_DB_PRL(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize);
	int			_Tick_DealGS4WebMsg_Assist_PRL(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize);
	int			_Tick_DealGS4WebMsg_Group_REQ(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize);
};
}
#endif
