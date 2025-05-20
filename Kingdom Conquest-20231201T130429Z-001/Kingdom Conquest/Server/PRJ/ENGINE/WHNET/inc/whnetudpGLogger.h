// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpGLogger.h
// Creator      : Wei Hua (魏华)
// Comment      : 利用UDP的通用日志记录器
// CreationDate : 2003-05-21
// ChangeLOG    : 2005-09-07 增加了Log对MT的支持
//              : 2006-11-30 增加了GLOGGER_ID_FATAL
//              : 2007-02-28 增加了GLOGGER_ID_DATARECOVER
//              : 并给GLOGGER_ID_HACK, GLOGGER_ID_FATAL, GLOGGER_ID_DATARECOVER分配了新的ID
//              : 同时为了设置方便，在找不到相应的日志对象的时候默认把日志输出到CMN中
//              : 2007-03-02 把GLogger_Init的默认参数从8改为16
//              : 增加了GLoggerMan::GetLogger函数，内部实现了在Logger序号不存在的时候自动切换到默认Logger的功能（当然如果没有默认Logger那么就出错吧）
//              : 2007-04-05 把老的GLOGGER相关的ID和宏都取消了
//              : 2007-04-10 增加了GLogger_WriteFmtStd

#ifndef	__WHNETUDPGLOGGER_H__
#define	__WHNETUDPGLOGGER_H__

#include "whnetcmn.h"
#include <WHCMN/inc/whlog.h>
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whtime.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whlock.h>
#include <stdarg.h>

using n_whcmn::whtick_t;
using n_whcmn::whlooper;
using n_whcmn::whlogwriter;
using n_whcmn::whunitallocatorFixed;

namespace n_whnet
{

// GLOG会用到的结构和命令定义
enum
{
	GLOG_MAX_TAG				= 32,			// exe标签的最大长度
};
// 请求
enum
{
	GLOG_CMD_REQ_KEEPALIVE		= 0x00,			// keepalive，保持连接。
												// 注意：这个无论msg如何发送都是指定时间内发送一次。
												// 结构：GLOG_CMD_INT_T
	GLOG_CMD_REQ_CONNECT		= 0x01,			// 连接服务器
												// 结构：GLOG_CMD_REQ_MSG_T
	GLOG_CMD_REQ_DISCONNECT		= 0x02,			// 从服务器断开
												// 结构：GLOG_CMD_INT_T
	GLOG_CMD_REQ_MSG			= 0x03,			// 一条消息
												// 结构：GLOG_CMD_REQ_MSG_T
	GLOG_CMD_REQ_COUNT			= 0x04,			// 消息计数
												// 结构：GLOG_CMD_REQ_COUNT_T
	GLOG_CMD_REQ_CONTROL		= 0x10,			// 控制指令
												// 结构：GLOG_CMD_REQ_CONTROL_T
};
// 返回
enum
{
	GLOG_CMD_RPL_CONNECT		= GLOG_CMD_REQ_CONNECT,
												// 结构：GLOG_CMD_RPL_CONNECT_T
};
#pragma pack(1)
// 基本
struct	GLOG_CMD_T
{
	unsigned char	cmd;
};
struct	GLOG_CMD_INT_T
{
	unsigned char	cmd;
	int				nParam;
};
// 请求
struct	GLOG_CMD_REQ_CONNECT_T
{
	unsigned char		cmd;
	char				szExeTag[GLOG_MAX_TAG];	// Exe标识。主要用在共享方式中，标识来源。
};
struct	GLOG_CMD_REQ_MSG_T
{
	unsigned char		cmd;
	int					nID;					// 自己的ID
	char				szMsg[1];				// 数据部(长度可以通过总长度推算出来)
												// (如果是字串，发来的消息保证后面有0结尾)
};
struct	GLOG_CMD_REQ_COUNT_T
{
	unsigned char		cmd;
	int					nID;					// 自己的ID
	unsigned int		nCount;					// 已发送的消息的计数
};
struct	GLOG_CMD_REQ_CONTROL_T
{
	unsigned char		cmd;
	char				szCmdStr[1];			// 数据部(长度可以通过总长度推算出来)
};
// 结果返回
struct	GLOG_CMD_RPL_CONNECT_T
{
	enum
	{
		RST_OK				= 0x00,				// 成功
		RST_ERR				= 0xFF,				// 未知的错误
		RST_ERR_NOROOM		= 0xFE,				// 没有地方了。（已经连入Server的logger数量超过了规定值）
	};
	unsigned char		cmd;
	unsigned char		rst;					// 结果
	int					nID;					// 如果成功，会返回它的ID
};
#pragma pack()

// 新日志记录服务器
class	GLogServerer
{
public:
	struct	INFO_T
	{
		int					nSelectInterval;	// 选择的间隔
		int					nKeepAliveTimeOut;	// KeepAlive的超时(秒)
		int					nMaxLogger;			// 最多可容纳的连入日志数量(文件最高数量和这个相同)
		char				szBindAddr[WHNET_MAXADDRSTRLEN];
												// 绑定到这个端口监听
		bool				bAppendTagName;		// 把exe的名字放到每一行
		bool				bAppendSourceID;	// 把来源的ID放到Name后面
		bool				bRawWrite;			// 没有任何修饰的写
		INFO_T()
		: nSelectInterval(100)
		, nKeepAliveTimeOut(60)
		, nMaxLogger(100)
		, bAppendTagName(true)
		, bAppendSourceID(false)
		, bRawWrite(false)
		{
			strcpy(szBindAddr, ":2004");
		}
	};
protected:
	// 一个记录来源单元
	struct	LoggerUnit
	{
		int				nID;					// 自己的ID
		whtick_t		nLastKATick;			// 上次收到KeepAlive包的时间（单位毫秒）
		struct sockaddr_in	addr;				// 来源地址
		char			szExeTag[GLOG_MAX_TAG + 64];	// Exe标识(并添加一些后缀)
		bool			bCanBeRemoved;			// 可以被移除，如果在没有空间的情况下可以把这样的东东移除
		unsigned int	nCount;					// 收到的消息计数，用于检测是否丢包
		void	clear()
		{
			memset(this, 0, sizeof(*this));
		}
	};

	SOCKET					m_sock;				// 用于监听的socket
	struct sockaddr_in		m_fromaddr;			// Tick中当前处理的数据来自这个地址
	whunitallocatorFixed<LoggerUnit>	m_Loggers;
												// 用于LoggerUnit的分配
	INFO_T					m_info;				// 记录初始化信息
	int						m_nKATimeOut;		// KeepAlive的超时（单位毫秒）

	n_whcmn::whlogwriter	m_logwriter;		// 日志书写器
public:
	GLogServerer();
	~GLogServerer();
	int		Init(INFO_T *pInfo, whlogwriter::INFO_T	*pLogWRInfo);
	int		Release();
	int		Tick();								// 一次工作内容
public:
	// 用于直接操控logwriter
	inline n_whcmn::whlogwriter *	GetLogWriter()
	{
		return	&m_logwriter;
	}
private:
	int		Tick_KeepAlive();
	// 处理各种请求
	int		Tick_REQ_KEEPALIVE(GLOG_CMD_INT_T *pCmd);
	int		Tick_REQ_CONNECT(GLOG_CMD_REQ_CONNECT_T *pCmd);
	int		Tick_REQ_DISCONNECT(GLOG_CMD_INT_T *pCmd);
	int		Tick_REQ_MSG(GLOG_CMD_REQ_MSG_T *pCmd, int nSize);
	int		Tick_GLOG_CMD_REQ_COUNT(GLOG_CMD_REQ_COUNT_T *pCmd, int nSize);
	// 内部功能
	int		RemoveLoggerUnit(LoggerUnit *pLoggerUnit);
	int		RemoveLoggerUnit(int nID);
	// 发送指令到当前地址(即刚刚发来数据的地址)
	int		SendToCurAddr(const void *pCmd, size_t nSize);
};

// 新日志记录器
class	GLogger
{
public:
	struct	INFO_T
	{
		char	szDstAddr[WHNET_MAXADDRSTRLEN];
		char	szExeTag[GLOG_MAX_TAG];
		int		nKeepAliveInterval;							// KeepAlive的时间间隔(秒)
		int		nCountCheckInterval;						// 检查是否丢包的时间间隔(毫秒)
		int		nConnectTimeOut;							// 连接服务器的超时(毫秒)
		bool	bSupportMT;									// 是否支持多线程
		INFO_T()
		: nKeepAliveInterval(22)
		, nCountCheckInterval(1000)
		, nConnectTimeOut(1000)
		, bSupportMT(false)
		{
			szDstAddr[0]	= 0;
			szExeTag[0]		= 0;
		}
	};
private:
	INFO_T		m_info;										// 初始化信息
	SOCKET		m_sock;										// 通讯的socket
	int			m_nID;										// 连接GLogServerer获取的ID
	char		m_szFmtStrBuf[whlogwriter::MAXMSGLEN];		// 用于生成格式文本
	char		m_szMsgBuf[whlogwriter::MAXMSGLEN];			// 最长的数据也就这么长
	whlooper	m_keepaliveloop;							// 用于发送keepalive定时
	whlooper	m_checkcountloop;							// 用于定时检查是否丢包
	int			m_nCount;									// 消息计数，用于检测是否丢包
	n_whcmn::whlock		*m_pLock;							// 用于线程间呼哧
public:
	inline SOCKET	GetSock() const
	{
		return	m_sock;
	}
public:
	GLogger();
	~GLogger();
	enum
	{
		INITRST_OK			= 0,							// 成功
		INITRST_ERR_SOCK	= -1,							// 无法创建socket
		INITRST_ERR_ADDR	= -2,							// 无法解析地址
		INITRST_ERR_SVR		= -3,							// 连接日志服务器出错(服务器不存在或者服务器返回出错)
	};
	// Init的返回可能为上面的enum：INITRST_XXX
	int		Init(INFO_T *pInfo);							// 初始化。szDstAddr为LOG记录Server的地址，形如：ip:port，i.e."127.0.0.1:1976"
	int		Release();										// 终结。
	int		Tick();											// 主要工作是keepalive
	int		WriteBin(const void *pData, size_t nSize);		// 写二进制数据，这个必须以二进制方式打开
	int		WriteLine(const char *szStr);					// 写入文本行，这个必须以文本方式打开
	int		WriteFmtLine(const char *szFmt, ...);			// 写入格式文本，这个必须以文本方式打开
	int		WriteFmtLine0(const char *szFmt, va_list arglist);
private:
	// 返回和send返回一致
	int		RawWrite(const void *pData, size_t nSize);		// 向目的地址发送包
};

// 多logger的管理器
class	GLoggerMan
{
private:
	struct	GLOGGERUNIT_T
	{
		GLogger			*pLogger;
		GLogger::INFO_T	info;								// 生成该GLogger的信息
		GLOGGERUNIT_T()
		: pLogger(NULL)
		{
		}
	};
	n_whcmn::whvector<GLOGGERUNIT_T, true>		m_vectUnits;
	int		m_nDftLoggerIdx;								// 如果某个Logger不存在，则默认把对应的日志写到这个Logger里面（默认是0）
public:
	GLoggerMan(int nNum);
	~GLoggerMan();
	int		AddLogger(int nID, GLogger::INFO_T *pInfo);
	int		RemoveAllLogger();
	int		WriteStr(int nID, const char *szStr);
	int		WriteFmt(int nID, const char *szFmt, ...);
	int		WriteFmt0(int nID, const char *szFmt, va_list arglist);
	int		Tick();
	inline void	SetDftLoggerIdx(int nIdx)
	{
		assert(nIdx>=0 && nIdx<(int)m_vectUnits.size());
		m_nDftLoggerIdx	= nIdx;
	}
private:
	GLogger *	GetLogger(int nID);
};

// 基于GLoggerMan的全局Log函数
int		GLogger_Init(int nNum=16);
int		GLogger_Release();
int		GLogger_Add(int nID, GLogger::INFO_T *pInfo);
// GLogger_AddMulti中会改变pCmnInfo的内容
int		GLogger_AddMulti(GLogger::INFO_T *pCmnInfo, const char *cszAddrs);
int		GLogger_WriteFmt(int nID, const char *szFmt, ...);
int		GLogger_WriteStr(int nID, const char *szStr);
int		GLogger_Tick();
void	GLogger_Set_PrintInScreenOnly(bool bSet);
void	GLogger_Set_PrintInScreenToo(bool bSet);

// 先用旧的顶着
enum
{
	// 一般可以把日志写到这个ID 
	GLOGGER_ID_CMN					=	0,
	// 重要错误写到这里
	GLOGGER_ID_ERROR				=	1,
	// 基本过程相关的写到这里(比如游戏中和用户操作相关的就放到这里，对应与原来刀剑中的client日志。不过现在有一些可以分到player和npc日志中去写)
	GLOGGER_ID_STORY				=	2,
	// 预期之外的错误，可能是hack导致的
	GLOGGER_ID_HACK					=	3,
	// 非常严重的错误（比如debug版要assert这样的错误）
	GLOGGER_ID_FATAL				=	4,
	// 记录关键性的恢复数据用的日志
	GLOGGER_ID_DATARECOVER			=	5,
	// 与计费相关，包含帐号的login / logout / billing
	GLOGGER_ID_BILLING				=	6,
	// 与角色数据和角色间交互相关，包含角色login / logout / new / delete / exchange/ task等
	GLOGGER_ID_PLAYER				=	7,
	// 战斗相关日志
	GLOGGER_ID_FIGHT				=	8,
	// 与道具相关，包含生成、获得、丢弃、交易、合成等
	GLOGGER_ID_ITEM					=	9,
	// NPC相关日志
	GLOGGER_ID_NPC					=	10,
	// 与行会相关，包含行会的创建/加入/解散/授权
	GLOGGER_ID_CLAN					=	11,
	// 角色所讲的话，包含私聊/在各类频道里的说话
	GLOGGER_ID_TALK					=	12,
	// GM 的所有活动
	GLOGGER_ID_GM					=	13,
	// 用户调查日志
	GLOGGER_ID_SURVEY				=	14,
};

////////////////////////////////////////////////////////////////////
// 为DLL之间信息同步
////////////////////////////////////////////////////////////////////
void *	WHNET_UDPGLOGGER_STATIC_INFO_Out();
void	WHNET_UDPGLOGGER_STATIC_INFO_In(void *pInfo);

}		// EOF namespace n_whnet

// 切记不要在这些宏里面有任何计算相关的东西，如i++之类的，因为他们一旦被注掉了，就不会计算了

// 先用旧的顶着
#define	GLOGGER2_INIT						n_whnet::GLogger_Init
#define	GLOGGER2_RELEASE					n_whnet::GLogger_Release
#define	GLOGGER2_ADD						n_whnet::GLogger_Add
#define	GLOGGER2_ADDMULTI					n_whnet::GLogger_AddMulti
#define	GLOGGER2_WRITEFMT					n_whnet::GLogger_WriteFmt
#define	GLOGGER2_WRITESTR					n_whnet::GLogger_WriteStr
#define	GLOGGER2_TICK						n_whnet::GLogger_Tick
#define	GLOGGER2_SET_PRINTINSCREENONLY		n_whnet::GLogger_Set_PrintInScreenOnly
#define	GLOGGER2_SET_PRINTINSCREENTOO		n_whnet::GLogger_Set_PrintInScreenToo
#define	GLOGGER2_INFO_T						n_whnet::GLogger::INFO_T
// 专门用来给日志格式串前面增加代码和关键字的
#define GLGR_STD_HDR(code,keyword)			#code","#keyword","

#endif	// EOF __WHNETUDPGLOGGER_H__
