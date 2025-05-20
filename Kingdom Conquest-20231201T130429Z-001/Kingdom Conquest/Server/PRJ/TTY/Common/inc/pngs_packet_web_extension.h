#ifndef __PNGS_PACKET_WEB_EXTENSION_H__
#define __PNGS_PACKET_WEB_EXTENSION_H__

#include "PNGS/inc/pngs_def.h"
#include "WHNET/inc/whnetcmn.h"
#include "WHNET/inc/whnetepoll.h"

using namespace n_whnet;

namespace n_pngs
{
// 各个服务器的当前版本(版本不匹配的时候不能互联)
enum
{
	LPMainStructure_VER			= 1,
	LPGamePlaye_VER				= 1,
	CAAFS4Web_VER				= 1,
	CLS4Web_VER					= 1,
	Postman4Web_VER				= 1,
	GS4Web_VER					= 1,
	BD4Web_VER					= 1,
};

// 终端类型
enum
{
	TERM_TYPE_NONE				= 0,			// 啥也不是
	TERM_TYPE_IOS				= 1,			// 苹果手机
	TERM_TYPE_ANDROID			= 2,			// 安卓
	TERM_TYPE_WP				= 3,			// windows phone
	TERM_TYPE_WEB				= 4,			// web端
	TERM_TYPE_IPAD				= 5,			// iPad
	TERM_TYPE_NUM,								// 最大值
};

//////////////////////////////////////////////////////////////////////////
// 指令部分
//////////////////////////////////////////////////////////////////////////
enum
{
	//////////////////////////////////////////////////////////////////////////
	// CAAFS4Web->LP
	//////////////////////////////////////////////////////////////////////////
	// CAAFS4Web连入LP后给出自己的信息
	CAAFS4Web_LP_HI							= 1,
	// 通知LP有玩家想开始连接CLS4Web了(玩家的一些信息附带通知给CLS4Web)
	CAAFS4Web_LP_CLIENTWANTCLS4Web			= 3,
	// CAAFS4Web发来的一些预先请求(如:用户连上CAAFS4Web的时候,每个人可以做一次prelogin,一个连接只能发送一次)
	CAAFS4Web_LP_CLIENT_CMD					= 4,

	//////////////////////////////////////////////////////////////////////////
	// LP->CAAFS4Web
	//////////////////////////////////////////////////////////////////////////
	// LP告诉CAAFS4Web可以工作了
	LP_CAAFS4Web_HI							= 11,
	// LP对CAAFS4Web的一些控制工作
	LP_CAAFS4Web_CTRL						= 12,
	// LP告诉CAAFS4Web有LP的信息变化了(如:在线人数变化等等)
	LP_CAAFS4Web_LPINFO						= 13,
	// LP告诉CAAFS4Web可以让玩家连接CLS4Web了
	// 只要对应的CLS4Web存在则CAAFS4Web一定会收到这个返回的(如果CLS4Web掉了,LP在清理CLS4Web用户的时候也会向CAAFS4Web发送用户连接结果的消息)
	LP_CAAFS4Web_CLIENTCANGOTOCLS4Web		= 14,

	//////////////////////////////////////////////////////////////////////////
	//CLS4Web->LP
	//////////////////////////////////////////////////////////////////////////
	// CLS4Web连入LP后给出自己的初始信息
	CLS4Web_LP_HI							= 21,
	CLS4Web_LP_HI1							= 22,
	// CLS4Web通知LP客户端可以来连自己了
	CLS4Web_LP_CLIENTCANGOTOCLS4Web			= 24,
	// CLS4Web通知LP客户端连接结果
	CLS4Web_LP_CLIENTTOCLS4Web_RST			= 25,
	// CLS4Web通知LP客户端断线(GMS收到这个消息的时候用户应该已经从CLS删除了)
	CLS4Web_LP_CLIENT_DROP					= 26,
	// 一些控制工作(如通知自己正常停机了)
	CLS4Web_LP_CTRL							= 27,

	//////////////////////////////////////////////////////////////////////////
	// LP->CLS4Web
	//////////////////////////////////////////////////////////////////////////
	// LP告诉CLS4Web可以正常工作了
	LP_CLS4Web_HI							= 31,
	// LP告诉CLS4Web将要有客户端来连接它
	LP_CLS4Web_CLIENTWANTCLS4Web			= 34,

	//////////////////////////////////////////////////////////////////////////
	// CAAFS4Web->CLIENT
	//////////////////////////////////////////////////////////////////////////
	// 自我介绍,客户端判断info后如果不断线的话就可以继续工作
	CAAFS4Web_CLIENT_CAAFS4WebINFO			= 91,
	// 通知用户目前排队的状况
	CAAFS4Web_CLIENT_QUEUEINFO				= 92,
	// 通知用户可以开始连接CLS4Web了(告诉用户CLS4Web的地址,以及连接密码)
	CAAFS4Web_CLIENT_GOTOCLS4Web			= 93,

	//////////////////////////////////////////////////////////////////////////
	// CLIENT->CAAFS4Web
	//////////////////////////////////////////////////////////////////////////
	CLIENT_CAAFS4Web_2LP					= 98,

	//////////////////////////////////////////////////////////////////////////
	// CLIENT->CLS4Web
	//////////////////////////////////////////////////////////////////////////
	// 客户端发送给CLS4Web的信息(连接密码)
	CLIENT_CLS4Web_CONNECT_EXTINFO			= 101,
	// 用户数据
	CLIENT_CLS4Web_DATA						= 102,

	//////////////////////////////////////////////////////////////////////////
	// CLS4Web->CLIENT
	//////////////////////////////////////////////////////////////////////////
	// 用户数据
	CLS4Web_CLIENT_DATA						= 112,

	//////////////////////////////////////////////////////////////////////////
	// CLS4Web发给服务器的公共指令
	//////////////////////////////////////////////////////////////////////////
	// 客户端发来的数据
	CLS4Web_SVR_CLIENT_DATA					= 201,

	//////////////////////////////////////////////////////////////////////////
	// CLS收到的服务器发来的公共指令
	//////////////////////////////////////////////////////////////////////////
	// 服务器对CLS的一些控制工作(如:踢出某个用户、请求某些信息等)
	SVR_CLS4Web_CTRL						= 210,
	// 设置玩家标志
	SVR_CLS4Web_SET_TAG_TO_CLIENT			= 211,
	// 发给单一玩家的数据
	SVR_CLS4Web_CLIENT_DATA					= 213,
	// 发给多个玩家的数据
	SVR_CLS4Web_MULTICLIENT_DATA			= 215,
	// 发给某个标志满足条件的玩家
	SVR_CLS4Web_TAGGED_CLIENT_DATA			= 217,
	// 发给所有玩家
	SVR_CLS4Web_ALL_CLIENT_DATA				= 219,
	// 设置64位标志
	SVR_CLS4Web_SET_TAG64_TO_CLIENT			= 221,
	// 发给某个64位标志满足条件的玩家
	SVR_CLS4Web_TAGGED64_CLIENT_DATA		= 222,
};

#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
// CAAFS4Web->LP
//////////////////////////////////////////////////////////////////////////
struct CAAFS4Web_LP_HI_T 
{
	pngs_cmd_t			nCmd;
	int					nGroupID;					// CAAFS组号和相应的CLS对应
	int					nCAAFS4WebVer;				// CAAFS版本
	bool				bReconnect;					// 表示这个是重连导致的
};
struct CAAFS4Web_LP_CLIENTWANTCLS4Web_T 
{
	pngs_cmd_t			nCmd;
	int					nClientIDInCAAFS4Web;		// 用户在CAAFS4Web中的ID
	unsigned int		IP;							// 用户的IP
	unsigned int		nPassword;					// 用户连接CLS4Web的密码
	int					nTermType;					// 终端类型
};
struct CAAFS4Web_LP_CLIENT_CMD_T 
{
	pngs_cmd_t			nCmd;
	int					nClientIDInCAAFS4Web;		// 用户在CAAFS4Web中的ID
	unsigned int		IP;							// 用户IP
	// 后面跟的是相关指令和数据
};
//////////////////////////////////////////////////////////////////////////
// LP->CAAFS4Web
//////////////////////////////////////////////////////////////////////////
struct LP_CAAFS4Web_HI_T
{
	enum
	{
		RST_OK						= 0,			// 连接成功
		RST_ERR_BAD_VER				= 1,			// 版本不匹配
		RST_ERR_BAD_GROUPID			= 2,			// group id越界
		RST_ERR_DUP_GROUPID			= 3,			// group id重复
		RST_ERR_MEMERR				= 4,			// 内存分配错误
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRst;
	int					nLPVer;						// LP的版本号
};
struct LP_CAAFS4Web_CTRL_T 
{
	enum
	{
		SUBCMD_EXIT					= 0,			// 退出
		SUBCMD_STRCMD				= 1,			// 字串指令
		SUBCMD_STRRST				= 2,			// 字串指令的返回
		SUBCMD_CLIENTCMDNUMSUB1		= 3,			// 把用户的指令计数-1,nExt是用户的nClientIDInCAAFS4Web
													// 不用返回
		SUBCMD_CLIENTQUEUETRANS		= 4,			// 转移用户的排队,nExt是用户的nClientIDInCAAFS4Web,nParam是目标队列
													// 不用返回
	};
	pngs_cmd_t			nCmd;
	pngs_cmd_t			nSubCmd;
	int					nCAAFS4WebIdx;
	int					nExt;
	int					nParam;
};
struct LP_CAAFS4Web_LPINFO_T 
{
	pngs_cmd_t			nCmd;
	unsigned short		nLPAvailPlayer;
};
struct LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T 
{
	enum
	{
		RST_OK						= 0,			// 成功,可以连接CLS4Web
		RST_ERR_MEM					= 1,			// 失败,无法分配内存
		RST_ERR_CLS4Web				= 2,			// 失败,没有可去的CLS4Web
		RST_ERR_NOCLS4WebCANACCEPT	= 3,			// 失败,没有可以接受用户的CLS了
		RST_ERR_UNKNOWN				= 255,			// 失败,其他位置错误
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRst;						// 结果
	port_t				nPort;						// CLS4Web的端口
	unsigned int		IP;							// CLS4Web的IP
	int					nClientIDInCAAFS4Web;		// 用户在CAAFS4Web中的ID
	int					nClientID;					// 用户在LP中的ID,这个就是今后玩家在游戏过程中的统一在线ID
};
//////////////////////////////////////////////////////////////////////////
// CLS4Web->LP
//////////////////////////////////////////////////////////////////////////
struct CLS4Web_LP_HI_T
{
	pngs_cmd_t			nCmd;
	int					nGroupID;					// CLS4Web组号,和对应的CAAFS4Web对应
	int					nCLS4WebVer;				// CLS4Web版本
	int					nOldID;						// 旧的CLS4WebID,如果非0表示重连的
};
struct CLS4Web_LP_HI1_T 
{
	pngs_cmd_t			nCmd;
	port_t				nPort;						// 对Client的端口
	unsigned int		IP;							// 对Client的IP
};
struct CLS4Web_LP_CLIENTCANGOTOCLS4Web_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;					// 用户在LP/CLS中的ID
};
struct CLS4Web_LP_CLIENTTOCLS4Web_RST_T 
{
	enum
	{
		RST_OK						= 0,			// 连接成功
		RST_ERR_NOTCONNECT			= 1,			// 客户端根本没有来连接
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRst;						// 是否成功
	int					nClientID;					// 用户在LP中的ID(也是CLS中的)
};
struct CLS4Web_LP_CLIENT_DROP_T 
{
	enum
	{
		REMOVEREASON_NOTHING		= 0,
		REMOVEREASON_SENDERR		= 1,			// 向客户端发送数据时Send返回错误(可能是换充满了)
		REMOVEREASON_CLIENTHACK		= 2,			// 客户端发来的指令有hack嫌疑
		REMOVEREASON_KICKED			= 3,			// 客户端被服务器用指令踢出
		REMOVEREASON_NOTEXIST		= 4,			// 客户端对应的ID不存在
		REMOVEREASON_END			= 5,			// 服务器让客户端正常下线
		REMOVEREASON_AS_DROP		= 6,			// 
		REMOVEREASON_CLS4Web_ERR	= 7,			// 向CLS4Web发送消息失败等等
		// 前面的应该不会超过100,(后面的这些是因为Close造成的)
		REMOVEREASON_CNTRCLOSE_REASON_UNKNOWN			= 100+epoll_server::close_reason_unknown,
		REMOVEREASON_CNTRCLOSE_REASON_INITIATIVE		= 100+epoll_server::close_reason_initiative,
		REMOVEREASON_CNTRCLOSE_REASON_PASSIVE			= 100+epoll_server::close_reason_passive,
		REMOVEREASON_CNTRCLOSE_REASON_CLOSE_TIMEOUT		= 100+epoll_server::close_reason_close_timeout,
		REMOVEREASON_CNTRCLOSE_REASON_DROP				= 100+epoll_server::close_reason_drop,
		REMOVEREASON_CNTRCLOSE_REASON_CONNECT_TIMEOUT	= 100+epoll_server::close_reason_connect_timeout,
		REMOVEREASON_CNTRCLOSE_REASON_ACCEPT_TIMEOUT	= 100+epoll_server::close_reason_accept_timeout,
		REMOVEREASON_CNTRCLOSE_REASON_KEY_NOT_AGREE		= 100+epoll_server::close_reason_key_not_agree,
		REMOVEREASON_CNTRCLOSE_REASON_CNTRID_NOT_MATCH	= 100+epoll_server::close_reason_cntrid_notmatch,
		REMOVEREASON_CNTRCLOSE_REASON_DATASIZE_ZERO		= 100+epoll_server::close_reason_datasize_zero,
		REMOVEREASON_CNTRCLOSE_REASON_DATASIZE_TOOBIG	= 100+epoll_server::close_reason_datasize_toobig,
		REMOVEREASON_CNTRCLOSE_REASON_DECRYPT			= 100+epoll_server::close_reason_decrypt,
		REMOVEREASON_CNTRCLOSE_REASON_ENCRYPT			= 100+epoll_server::close_reason_encrypt,
		REMOVEREASON_CNTRCLOSE_REASON_RECV				= 100+epoll_server::close_reason_recv,
		REMOVEREASON_CNTRCLOSE_REASON_SEND				= 100+epoll_server::close_reason_send,
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRemoveReason;				// 如果在RemovePlayer之前设置了这个则说明关闭原因
	int					nClientID;					// 用户在LP中的ID(也是CLS中的)
};
struct CLS4Web_LP_CTRL_T 
{
	enum
	{
		SUBCMD_EXIT					= 0,			// 退出(告诉LP自己正常退出了)
	};
	pngs_cmd_t			nCmd;
	pngs_cmd_t			nSubCmd;
	int					nParam;
};
//////////////////////////////////////////////////////////////////////////
// LP->CLS4Web
//////////////////////////////////////////////////////////////////////////
struct LP_CLS4Web_HI_T 
{
	enum
	{
		RST_OK						= 0x00,			// 连接成功
		RST_ERR_BAD_VER				= 0x01,			// 版本错误
		RST_ERR_BAD_GROUPID			= 0x02,			// groupid超界
		RST_ERR_TOOMANY_CLS4Web		= 0x03,			// 这个groupid的CLS4Web太多了
		RST_ERR_MEMERR				= 0x04,			// 内存分配错误
		RST_ERR_UNKNOWN				= 0xFF,			// 未知错误
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRst;
	int					nCLS4WebID;
	int					nLPMaxPlayer;
	int					nCLS4WebMaxConnection;
	int					nLPVer;
};
struct LP_CLS4Web_CLIENTWANTCLS4Web_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;					// 用户在LP中的ID(实际上也将是在CLS4Web中的ID)
	unsigned int		nPassword;					// 连接密码(CAAFS4Web发给LP,LP再转发过来的)
	int					nTermType;					// 终端类型
};
//////////////////////////////////////////////////////////////////////////
// CAAFS4Web->CLIENT
//////////////////////////////////////////////////////////////////////////
struct CAAFS4Web_CLIENT_CAAFS4WebINFO_T 
{
	enum
	{
		SELF_NOTIFY_REFUSEALL		= 0x00000001,	// 拒绝所有登录
	};
	pngs_cmd_t			nCmd;
	unsigned char		nVerCmpMode;				// 版本比较方式
	char				szVer[PNGS_VER_LEN];		// 版本
	unsigned short		nQueueSize;					// 前面排队的人数(超过一定量就不要让用户连接)
	unsigned int		nSelfNotify;				// 32bit表示32个意思
	int					nInfoLen;					// info的长度
	char				info[1];					// 介绍字串
};
struct CAAFS4Web_CLIENT_QUEUEINFO_T 
{
	pngs_cmd_t			nCmd;
	unsigned char		nChannel;					// 对应的通道
	unsigned char		nQueueSize;					// 前面排队的人数
};
struct CAAFS4Web_CLIENT_GOTOCLS4Web_T 
{
	pngs_cmd_t			nCmd;
	port_t				nPort;						// CLS4Web端口
	unsigned int		IP;							// CLS4Web地址IP
	unsigned int		nPassword;					// 连接密码
	int					nClientID;					// 玩家ID
};
//////////////////////////////////////////////////////////////////////////
// CLIENT->CAAFS4Web->LP
//////////////////////////////////////////////////////////////////////////
struct CLIENT_CAAFS4Web_2LP_T 
{
	pngs_cmd_t			nCmd;
	char				data[1];					// 和正常的login一样的数据包,不过这个是逻辑内容
};
struct CLIENT_CAAFS4Web_EXTINFO_T 
{
	int					nTermType;					// 终端类型
};
//////////////////////////////////////////////////////////////////////////
// CLIENT->CLS4Web
//////////////////////////////////////////////////////////////////////////
struct CLIENT_CLS4Web_EXTINFO_T 
{
	int					nClientID;					// 玩家ID
	unsigned int		nPassword;					// 用来连接CLS4Web的密码
};
struct CLIENT_CLS4Web_DATA_T 
{
	pngs_cmd_t			nCmd;
	char				data[1];
};
//////////////////////////////////////////////////////////////////////////
// CLS4Web->CLIENT
//////////////////////////////////////////////////////////////////////////
// 用户数据
struct CLS4Web_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	char				data[1];
};
//////////////////////////////////////////////////////////////////////////
// CLS4Web向服务器发送的公共指令
//////////////////////////////////////////////////////////////////////////
struct CLS4Web_SVR_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;
	char				data[1];					// 数据
};
//////////////////////////////////////////////////////////////////////////
// CLS4Web收到的服务器发来的公共指令
//////////////////////////////////////////////////////////////////////////
struct SVR_CLS4Web_CTRL_T 
{
	enum
	{
		SUBCMD_EXIT						= 0,			// 退出,nParam无意义
		SUBCMD_STRCMD					= 1,			// 字串指令,nParam的首地址被解释为字串首地址
		SUBCMD_STRRST					= 2,			// 字串指令的返回,nParam的首地址被解释为字串首地址
		SUBCMD_KICKPLAYERBYID			= 3,			// 根据ID踢出某个用户,nParam为PlayerID
		SUBCMD_CHECKPLAYEROFFLINE		= 6,			// 检查是否有用户已经下线了(这个只有LP会发来,CLS4Web检查各个ID是否存在,如果不存在就重新向LP发送用户下线消息),nParam是用户ID个数,后面跟着int数组
		SUBCMD_KICKPLAYERBYID_AS_DROP	= 7,			// 和SUBCMD_KICKPLAYERBYID一样,不过会向LP返回用户是DROP的结果
		SUBCMD_KICKPLAYERBYID_AS_END	= 8,			// 和SUBCMD_KICKPLAYERBYID一样,不过会向LP返回用户是正常退出的结果
		SUBCMD_SET_NAME					= 9,			// 设置用户名字串,主要用于记录日志,nParam为用户ID,后面跟着字串
		SUBCMD_PLAYER_STAT_OFF			= 10,			// 关闭统计,nParam为ClientID
		SUBCMD_PLAYER_STAT_ON			= 11,			// 开启统计,nParam为ClientID,后面还有一个整数表示发送间隔
		SUBCMD_KICK_ALL_PLAYER			= 12,			// 将所有玩家踢下线
	};
	pngs_cmd_t			nCmd;
	pngs_cmd_t			nSubCmd;
	int					nParam;
};
struct SVR_CLS4Web_SET_TAG_TO_CLIENT_T 
{
	pngs_cmd_t			nCmd;
	unsigned char		nTagIdx;
	short				nTagVal;
	int					nClientID;
};
struct SVR_CLS4Web_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;						// 玩家ID
	char				data[1];						// 数据
};
struct SVR_CLS4Web_MULTICLIENT_DATA_T
{
	pngs_cmd_t			nCmd;
	unsigned short		nClientNum;
	int					anClientID[1];
	// 这个之后就是真实数据
	int		GetDSize(int nTotalSize)
	{
		return nTotalSize - wh_offsetof(SVR_CLS4Web_MULTICLIENT_DATA_T, anClientID) - nClientNum*sizeof(int);
	}
	void*	GetDataPtr()
	{
		return anClientID + nClientNum;
	}
	static int	GetTotalSize(int nClientIDNum, int nDSize)
	{
		return wh_offsetof(SVR_CLS4Web_MULTICLIENT_DATA_T, anClientID) + nClientIDNum*sizeof(int) + nDSize;
	}
};
struct SVR_CLS4Web_TAGGED_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	unsigned char		nTagIdx;
	short				nTagVal;
	char				data[1];
};
struct SVR_CLS4Web_ALL_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	char				data[1];
};
struct SVR_CLS4Web_SET_TAG64_TO_CLIENT_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;
	whuint64			nTag;
	bool				bDel;
};
struct SVR_CLS4Web_TAGGED64_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	whuint64			nTag;
	char				data[1];
};
#pragma pack()
}

#endif
