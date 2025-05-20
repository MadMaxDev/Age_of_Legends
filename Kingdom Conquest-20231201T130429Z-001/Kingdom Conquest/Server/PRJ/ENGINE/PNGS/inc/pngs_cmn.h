// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cmn.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的网络服务通用构架模块(GMS、GZS都可以依据这个框架，或者可以理解为所有的逻辑服务器都可以依据这个框架)
//                PNGS是Pixel Network Game Structure的缩写
//                关于Logic和Interface的Type需要在具体应用中具体定义，这个一定要定义，才可以保证不重复。
//                framecontrol在更上面的一层来做
//                ILogic模块可以在DLL中实现，不过最好遵循下面的准则(其中XXX代表模块的名字)：
//                1. 编译出来的DLL应该叫XXX.so或XXX_d.so
//                2. 创建对象的函数应该叫XXX_Create
//                3. m_szLogicType命名规范:
//                3.1. 名字只能由小写字母开头，中间可以有字母、数字、横线(这些要求主要是为了能方便地和文件能够一致，当然不一致也没关系)
//                3.2. 名字的第一个小写字母表示了这个Logic的属性
//                3.2.1. 'u'表示这个名字不能重复
//                3.2.2. 'm'表示这个名字可以重复
//                目前已经使用的名字有：（参见tty_common_def.h和pngs_def.h里面的宏）
//                uGMS_MAINSTRUCTURE	GMS主框架
//                uGMS_GAMEPLAY			GMS游戏内容相关
//                uGS_GMTOOLSINTERFACE	服务器的GMTOOLS接口
//                uGS_DB				数据库
//                uGZS_MAINSTRUCTURE	GZS主框架
//                uGZS_GAMEPLAY			GZS游戏内容相关
//                uGS_CHAT				聊天相关的服务器模块接口
//                uGC_CHAT				聊天相关的客户端模块接口
//                4. 关于兼容多线程的Logic模块间指令传递
//                原则是：如果不是多线程就直接传递命令指针。如果是多线程就放入指令队列。
//                对于特殊的指令需要拷贝大量数据的，可是让所有指令的格式一致，对于数据都用指针引用。如果是多线程就把数据附加在后面。
//                比如这样指令的最后面都是int nSize; void *pData; // 注意不要用const void *，因为还需要拷贝数据方便
//                如果是需要放入队列的就在后面附加数据的部分，pData然后指向这个部分。
/*
对于Logic模块间进行比较大块的指令传递。一般用法是这样的。
if( IsSameThread(m_pLogicGamePlay) )
{
	GMSPACKET_2GP_SVRCMD_T	Cmd2GP;
	...
	Cmd2GP.nDSize			= nDSize;
	Cmd2GP.pData			= pGameCmd->data;
	m_pLogicGamePlay->CmdIn_NMT(this, GMSPACKET_2GP_SVRCMD, &Cmd2GP, sizeof(Cmd2GP));
}
else
{
	// 从对方申请内存
	GMSPACKET_2GP_SVRCMD_T	*pCmd2GP;
	if( m_pLogicGamePlay->CmdIn_MT_lock(this, GMSPACKET_2GP_SVRCMD, (void **)&pCmd2GP, nSize + sizeof(*pCmd2GP))<0 )
	{
		return;
	}
	...
	pCmd2GP->nDSize			= nDSize;
	pCmd2GP->pData			= pCmd2GP + 1;
	if( nDSize>0 )
	{
		memcpy(pCmd2GP->pData, pGameCmd->data, nDSize);
	}
	m_pLogicGamePlay->CmdIn_MT_unlock();
}
或者先填写好Cmd2GP之后调用
	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, GMSPACKET_2GP_SVRCMD, &Cmd2GP, sizeof(Cmd2GP));
对于较小的指令传递，可以直接这样使用
	CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSOFFLINE, &GZSOL, sizeof(GZSOL));
*/
// 注意：所有Logic都需要通过GLOGGER2_WRITEFMT这个宏来写日志，因为如果Logic在DLL中则GLOGGER2_WRITEFMT是没有初始化的
// CreationDate : 2005-07-25
// Change LOG   : 2005-09-07 把CMN转化为接口，因为ILogic内部会使用CMN的方法，如果CMN不是接口，则ILogic实现的部分就需要依赖PNGS工程，这样不好。
//                2005-09-09 把Logic类型改为字串类型，这样就不需要在公共的头文件中进行定义了，之需要有大家约定的字串名文件即可
//                2005-10-12 把CmdIn机制写在基类里，同时考虑多线程和非多限制的传递方式
//                2006-03-20 增加了ILogic对象的虚方法Detach()，用于在对象集体销毁前调用。
//                2006-08-15 增加了ILogic::Detach的bQuick参数，这样在初始化出错的时候如果结束程序就不会调用tick而导致可能的对象引用错误了。
//                2006-08-22 增加了文件管理器设置和获取。子类可以通过GetFileMan()获得文件管理器，然后进行文件相关的操作。
//                2006-01-23 把TickLogic拆开为AllTick和AllSureSend，为了CMNBody将来也可以被用于别的模块内的使用（用于给模块增加子插件功能）
//                2006-12-01 把m_szLogicType从CMN::ILogic移到ILogicBase中了。因为ILogicBase可能是需要获取名字的。

#ifndef	__PNGS_CMN_H__
#define	__PNGS_CMN_H__

#include <WHCMN/inc/whvector.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whfile_base.h>
#include <WHCMN/inc/whallocmgr2.h>
#include <WHCMN/inc/whthread.h>
#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetcnl2.h>								// 后面的pngs_config_CLSClient_Slot_Channel需要
#include "pngs_def.h"

namespace n_pngs
{

class	ILogicBase
{
public:
	typedef	int							cmd_t;						// 内存间传递，cmd_t可以大一些
	typedef	n_whcmn::whallocationobjwithonemem<>	AO_T;			// 针对一块大内存的分配器
	typedef	n_whcmn::whallocmgr2<AO_T>				AM_T;
	// 这个在内部队列中存储指令头信息的时候用
	struct	CMDHDR_T
	{
		ILogicBase	*pRstAccepter;
		cmd_t		nCmd;
	};
private:
	// 线程标识(如果标识一致则说明两个Logic在一个线程内)
	// 如果上层不设置则所有Logic都会默认是0
	int						m_nThreadTag;
	// 输入指令队列
	n_whcmn::whsmpqueueWL	m_queueCmdIn;
	// 文件管理器指针
	n_whcmn::whfileman		*m_pFM;
protected:
	char					m_szLogicType[PNGS_LOGICNAME_LEN];		// 这个在继承类中应该填写好(大家应该有约定哪些可以重复哪些不能重复的，匿名的默认可以重复)
	int						m_nVer;									// 这个也在继承类中填好（如果填成-1则表示不用检查版本）（默认写0则表示如果继承类中不填写那么多半要报错滴）
	// 关于Type的命名规范请参考Comment中的3
	// 标记是否停止tick
	bool					m_bStopTick;
protected:
	// 这两个在子类的Init和Release中调用
	int		QueueInit(int nQueueSize);
	int		QueueRelease();
	// 这个在子类的Tick中调用
	int		DealCmdIn();
public:
	ILogicBase()
		: m_nThreadTag(0)
		, m_pFM(NULL)
		, m_nVer(0)													// 如果填写-1表示不用校验VER（默认写0则表示如果继承类中不填写那么多半要报错滴）
		, m_bStopTick(false)
	{
		m_szLogicType[0]	= 0;									// 表示默认模块不需要名字
	}
	// 获得逻辑类型名
	inline	const char *	GetLogicType() const
	{
		return	m_szLogicType;
	}
	// 获得版本
	inline int	GetVer() const
	{
		return	m_nVer;
	}
	virtual	~ILogicBase()	{};
	// 获得线程标识
	inline int	GetThreadTag() const
	{
		return	m_nThreadTag;
	}
	// 如果上层根不用MT则可以强制把所有的Tag设置一致的
	inline void	SetThreadTag(int nTag)
	{
		m_nThreadTag	= nTag;
	}
	// 判断自己是否和对方是一个线程
	// 如果为真则说明不用MT了
	inline bool	IsSameThread(ILogicBase *pOther) const
	{
		return	m_nThreadTag == pOther->GetThreadTag();
	}
	// 设置文件管理器
	inline void	SetFileMan(n_whcmn::whfileman *pFM)
	{
		m_pFM	= pFM;
	}
	// 获得文件管理器
	inline n_whcmn::whfileman *	GetFileMan()
	{
		return	m_pFM;
	}
	// stoptick相关
	void	SetStopTick(bool bSet);
	// 输入指令，如果有返回且pRstAccepter非空则可以返回给pRstAccepter
	// ILogicBase的所有方法（除了CmdIn）都应该是在同一个线程内调用的（并不一定是所有ILogicBase都从属于一个线程，但是一个Logic应该只属于一个线程）
	// 需要立即执行的指令(这个不分MT和NMT)
	virtual int		CmdIn_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
	{
		return	DealCmdIn_One_Instant(pRstAccepter, nCmd, pData, nDSize);
	}
	// (调用方，应该已经判断了两个是否在一个线程内了)
	// 非多线程调用的指令
	inline int		CmdIn_NMT(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
	{
		return	DealCmdIn_One(pRstAccepter, nCmd, pData, nDSize);
	}
	// 多线程调用的指令
	int		CmdIn_MT(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	// 申请并锁定一段内存
	int		CmdIn_MT_lock(ILogicBase *pRstAccepter, cmd_t nCmd, void **ppData, int nDSize);
	// 解锁刚刚lock的内存
	int		CmdIn_MT_unlock();

	// 向对方逻辑发送简单的指令，自动判断是否需要MT方式发送
	inline int	CmdOutToLogic_AUTO(ILogicBase *pDstLogic, cmd_t nCmd, const void *pData, int nDSize)
	{
		if( pDstLogic )
		{
			if( IsSameThread(pDstLogic) )
			{
				return	pDstLogic->CmdIn_NMT(this, nCmd, pData, nDSize);
			}
			else
			{
				return	pDstLogic->CmdIn_MT(this, nCmd, pData, nDSize);
			}
		}
		else
		{
			return	-1;
		}
	}
private:
	// 处理立即执行的单条指令（在CmdIn_Instant中调用的）
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
																	= 0;
	// 处理单条指令
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
																	= 0;
};

class	CMN			: public ILogicBase
{
public:
	// 游戏逻辑模块
	class	ILogic	: public ILogicBase
	{
	public:
		enum	MOOD_T
		{
			MOOD_WORKING		= 0,								// 正常工作的模式（这个一般是开始就自动成功这种模式的，不用外界设置）
			MOOD_STOPPING		= 1,								// 处于结束过程中（外界设置）
			MOOD_STOPPED		= 2,								// 处于结束完毕的状态（内部如果结束了就自己设置结束）
		};
	protected:
		friend class CMNBody;
		// 这几个变量都是给CMN用的
		MOOD_T	m_nMood;
		int		m_nPos4CMN;											// CMN会把Logic在CMN中的数组下标填写到这里
		CMN		*m_pCMN;											// CMN会把自己的this指针填到这里
		bool	m_bIsInitOver;										// 告诉上层Init是否结束了（比如GSDB最开始的clean过程，这个过程如果不结束则不能让其他的模块开始初始化）
	protected:
		// 下面几个protected方法也只能CMN使用
		// 逻辑内部组织(比如和CMN的其他逻辑的连接，这个在所有logic都被attach之g后由CMN统一调用)
		// 这个也可以是在有旧逻辑被detach或者有新的逻辑被attach之后进行的
		// 所以每个逻辑应该内部自己记录着自己是否是第一次被Organize的
		// 另外在有Logic脱离的情况下也应该重新组织一下(这个主要是为了动态拔插Logic，慎用)，上层还需要保证在没有重新Organize之前不能调用任何逻辑tick(否则里面记录的老的Logic指针可能会导致内存错误)
		// 可以把Organize的功能只限定于获取其他模块的指针
		virtual	int		Organize()									= 0;
		// Detach是在所有ILogic对象被销毁之前集体调用的，因为有些销毁的操作需要依赖其他对象，所以必须提前调用
		// bQuick的意思是不用等待（因为正常的detach可能会等待然后多调几个tick以变让逻辑正常走完）
		virtual	int		Detach(bool bQuick)							{return 0;}
	protected:
		ILogic()
			: m_nMood(MOOD_WORKING)
			, m_nPos4CMN(-1), m_pCMN(NULL)
			, m_bIsInitOver(true)
		{
		}
	public:
		// 自我销毁
		virtual	void	SelfDestroy()								= 0;
		// 初始化
		virtual	int		Init(const char *cszCFGName)				= 0;
		// 重新载入配置
		virtual	int		Init_CFG_Reload(const char *cszCFGName)		= 0;
		// 告诉上层Init是否真正结束了
		inline bool		IsInitOver() const							{return m_bIsInitOver;}
		// 终结
		virtual	int		Release()									= 0;
		// 获得工作模式
		inline MOOD_T	GetWorkMood() const							{return m_nMood;}

		// 获得可被外界做select的SOCKET，append到vector的尾部
		virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect)	{return 0;}
		// 保证该发送的数据能够立即被发送(注意：Tick函数中很可能没有Send操作)
		virtual	int		SureSend()									{return 0;}
		// 设置
		// 工作一次(里面再调用子类的Tick_BeforeDealCmdIn()和Tick_AfterDealCmdIn())
		int		Tick();
	private:
		// 子类需要实现的
		virtual int		Tick_BeforeDealCmdIn()						= 0;
		virtual int		Tick_AfterDealCmdIn()						= 0;
	};
protected:
	CMN();
public:
	// （日志函数指针）为所有子Logic提供写日志的功能
	int		(*m_fn_GLogger_WriteFmt)(int nID, const char *szFmt, ...);

	// CMN可能处理的cmd，这个也是cmd_t类型的
	// 立即型的
	enum
	{
		CMD2CMN_INSTANT_REGISTERCMDDEAL		= 1,					// （目前应该是具有网络指令分拣处理功能的主框架会收到这个，如PNGSClient），注册一个pngs_cmd_t指令对应的指令处理逻辑模块
																	// pRstAccepter就是可以处理指令的逻辑模块
	};
	// 普通的
	enum
	{
		CMD2CMN_SHOULDSTOP					= 1,					// 任何一个模块都可以向主框架发送这个，让整个服务器停止运行（这个一般是在模块发现严重错误的时候）
																	// 一般发送方式为 CmdOutToLogic_AUTO(m_pCMN, CMN::CMD2CMN_SHOULDSTOP, NULL, 0);
		CMD2CMN_STOPNOW						= 2,					// 任何一个模块都可以向主框架发送这个，让整个服务器立即停止运行，完全不进行任何释放工作
																	// 一般发送方式为 CmdOutToLogic_AUTO(m_pCMN, CMN::CMD2CMN_STOPNOW, NULL, 0);
	};
	// 上面需要的指令结构
	#pragma pack(push, old_pack_num, 1)
	struct	CMD2CMN_INSTANT_REGISTERCMDDEAL_T
	{
		pngs_cmd_t		nCmdReg;
	};
	#pragma pack(pop, old_pack_num)

	// 创建一个假的ILogic
	static ILogic *		CreateDummyLogic(const char *cszLogicName);

	// 子类需要调用的功能（注意：不要取消了，原来怎样就保持怎样! 这样最稳定）
	virtual	ILogic *	GetLogic(const char *cszType, int nVer)		= 0;
	virtual	int			GetMultiLogic(const char *cszType, int nVer, n_whcmn::whvector<ILogic *> &vect)
																	= 0;

	// 可以创建ILogic对象的通用函数指针类型
	typedef	n_pngs::CMN::ILogic *	(*fn_ILOGIC_CREATE_T)();
};

class	CMNBody	: public CMN
{
private:
	bool	m_bThreadOn;											// 是否开启了线程模式
	bool	m_bShouldStop;											// 收到模块发来的CMD2CMN_SHOULDSTOP时会被设置为true。最外面的循环侦测到这个标志就会让整个服务器停掉了。
	n_whcmn::whvector<ILogic *>		m_vectLogic;					// 默认reserve 16个位置
	n_whcmn::whvector<SOCKET>		m_vectSOCKET;					// 默认reserve PNGS_MAX_SOCKETTOSELECT个位置（而且不resize）
	struct	MY_THEADINFO_T
	{
		n_whcmn::whtid_t		tid;
		n_whcmn::whlock			*pLock;
	};
	n_whcmn::whvector<MY_THEADINFO_T>	m_vectThread;				// 默认reserve 16个位置
public:
	CMNBody();
	~CMNBody();
	// 关联逻辑模块(注意：需要先AttachLogic，然后再调用Logic的Init函数，因为Init中也可以要用到写日志，这个需要调用CMN的成员来实现)
	int		AttachLogic(ILogic *pLogic);
	// 逻辑之间的相互连接(这个必须是在所有逻辑都attach完毕之后做)
	// 返回0表示成功，返回-i表示第i个logic出的问题
	int		OrganizeLogic();
	// 作一次所有SOCKET的select操作
	// 返回cmn_select可能返回的结果
	int		DoSelect(int nMS, int nThreadTag=0);
	// 如果上层希望做统一的select，那么上层就调用下面的函数获得seocket，就不用调用DoSelect了
	// 返回vect的尺寸（里面获得的socket是append到vect尾部的，所以在最开始使用之前必须要先把vect清空）
	size_t	GetSockets(n_whcmn::whvector<SOCKET> &vect, int nThreadTag=0);
	// 所有逻辑工作一次(CMN自己的工作CMN单独写，或者也简化为一个logic)
	// 里面调用各个子逻辑模块的Tick和SureSend
	int		AllTick(int nThreadTag=0);
	// 里面调用各个子逻辑模块的SureSend
	int		AllSureSend(int nThreadTag=0);
	// 获得逻辑模块(这个nType就是AttachLogic时的参数nType)
	ILogic *	GetLogic(const char *cszType, int nVer);
	int			GetMultiLogic(const char *cszType, int nVer, n_whcmn::whvector<ILogic *> &vect);

	// 重新载入所有配置
	int		ReloadAllCFG(const char *cszCFG);

	// 判断是否应该停止（由于内部原因导致要停止）
	bool	ShouldStop() const;
	// 设置所有Logic都进入停止状态
	void	SetAllLogicToStopMood();
	// 判断是否所有Logic都已经完成了Stop工作，可以真正停止了
	bool	AreAllLogicsReallyStopped();
	// 脱离所有的Logic并删除所有的Logic(如果bDestroy为真的话)
	// 如果是bQuick的话就不用在结束的时候调用tick了
	int		DetachAllLogic(bool bDestroy, bool bQuick=false);
	// 输出没有停止的logic的表达字串
	const char *	PrintLogicNotStopped();

	/////////////////////
	// 线程相关
	/////////////////////
	int		StartThreads();
	int		StopThreads();
	// 暂停所有thread
	int		LockThreads();
	// 恢复所有thread
	int		UnlockThreads();

private:
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
};

// 版本比较，看是否允许
enum
{
	PNGS_VERCMP_MODE_EQUAL			= 0,							// 必须完全相等才算匹配
	PNGS_VERCMP_MODE_GE				= 1,							// 输入大于等于基准才算匹配
};
bool	pngs_isverallowed(int nMode, int nBaseVerMain, int nBaseVerSub, int nInputVerMain, int nInputVerSub);
bool	pngs_isverallowed(int nMode, const char * szBaseVer, const char * szInputVer);

// 配置CLS和Client的slot的固定参数
int		pngs_config_CLSClient_Slot_Channel(n_whnet::CNL2SlotMan::INFO_T *pInfo, int nInOutNum, bool bIsClient);

// 载入ILogic模块的DLL并发给它一条指令
int		pngs_load_logic_dll_and_send_cmd(const char *cszDLLFile, const char *cszCreateFuncName, CMN::cmd_t nCmd, const void *pData, size_t nDSize);
// 载入ILogic模块的DLL并发给它版本历史查询的指令指令
int		pngs_load_logic_dll_and_query_history(const char *cszDLLFile, const char *cszCreateFuncName, const char *cszVer);

// 在Logic内部使用，向目标Logic传送指令（内部自动区分是否在同一线程，以便调用不同的方式来传递数据）
// _Ty结构的最后必须是
//	int				nDSize;
//	void			*pData;	// 或 char *pData也行（主要用于传送字串）。或者前加const也行。
//	用于记录附加数据的长度和临时指针
template<typename _Ty>
inline int	CMN_LOGIC_CMDIN_AUTO(ILogicBase *pLThis, ILogicBase *pLDst, CMN::cmd_t nCmd, _Ty &data)
{
	if( pLDst )
	{
		if( pLThis->IsSameThread(pLDst) || data.nDSize==0 )	// 如果在同一个线程内或者没有附加数据
		{
			pLDst->CmdIn_NMT(pLThis, nCmd, &data, sizeof(data));
		}
		else
		{
			// 从对方申请内存
			_Ty	*pCmd2GP;
			if( pLDst->CmdIn_MT_lock(pLThis, nCmd, (void **)&pCmd2GP, data.nDSize + sizeof(data))<0 )
			{
				return	-1;
			}
			// 拷贝全部数据过去
			memcpy(pCmd2GP, &data, sizeof(data));
			// 重新设置pData指向的地址
			wh_settypedptr(pCmd2GP->pData, pCmd2GP + 1);
			// 拷贝数据部分
			memcpy((void *)pCmd2GP->pData, data.pData, data.nDSize);
			pLDst->CmdIn_MT_unlock();
		}
		return	0;
	}
	else
	{
		return	-1;
	}
}

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CMN_H__
