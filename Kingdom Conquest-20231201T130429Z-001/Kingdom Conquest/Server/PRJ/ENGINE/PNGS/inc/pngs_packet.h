// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的网络包公共定义
//                PNGS是Pixel Network Game Structure的缩写
//                这里面的GMS和GZS实际上代表的是他们的MainStructure
//                PNGS_SVRIDX_XXX定义在pngs_def.h中
//                !!!!注意：无论是CLIENT_CLS_DATA还是CLS_CLIENT_DATA，data部分最开始的一定是pngs_cmd_t开头的一个指令码，这个类型不能改变，否则在PNLGSClient中内部分拣指令时就有问题了。
// CreationDate : 2005-08-01
// Change LOG   : 2005-09-30 取消逻辑级别的PlayerIn和Out(即取消了GZS_GMS_PLAYER_IN_OVER和GZS_GMS_PLAYER_OUT_OVER、GZS_GMS_PLAYER_WANTOUT)。
//              : 这些都应该放到GamePlay里面去写。玩家成功连接GMS之后，GMS会和所有的GZS同步玩家的连接。
//              : 修改了GMS_CAAFS_CTRL_T::SUBCMD_EXIT的作用（参见相应的注释）
//              : 2007-02-05 因为要增加同一个CAAFS/CLS组内的根据网络类型的分布，升级整体服务器版本XXX_VER
//              : 2007-03-16 GZS的用户数据对齐发生了问题，在windows下和linux下不同，原因是windows下相邻的不能凑整就不凑了，但是linux能凑。现在修改了位域变量类型，使其恰好能凑整。
//              : 2007-10-08 在连接GMS的各个HELLO包中增加了原来的连接ID用于标记重连。各个服务器的版本也因此改变。
//				: 2008-05-07 GMS和GZS的GamePlay层面的协议变化，各自的版本修改。不能互联。

#ifndef	__PNGS_PACKET_H__
#define	__PNGS_PACKET_H__

#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetcnl2.h>
#include <WHCMN/inc/whcmn_def.h>
#include "./pngs_def.h"

using namespace n_whcmn;

namespace n_pngs
{

// 写在这个括号里表示在n_pngs这个命名空间中可以使用
using n_whnet::port_t;
using n_whnet::CNL2SlotMan;

// 网络类型定义
enum
{
	NETWORKTYPE_UNKNOWN		= 0,								// 未知类型（即如果没有匹配的就默认用这个）
	NETWORKTYPE_C_Netcom	= 1,								// 中国网通
	NETWORKTYPE_C_Telcom	= 2,								// 中国电信
	NETWORKTYPE_C_Edu		= 3,								// 中国教育网
	NETWORKTYPE_C_Unicom	= 4,								// 中国联通
	NETWORKTYPE_C_TOTAL,										// 类型总数
};

// 各个服务器的当前版本（版本不匹配的时候不能互联）
enum
{
	GMS_VER		= 10,
	CAAFS_VER	= 8,
	CLS_VER		= 8,
	GZS_VER		= 10,
	GMTOOL_VER	= 0,
};

////////////////////////////////////////////////////////////////////
// GMS、CAAFS、CLS、GZS这个互联的架子需要的网络包定义
// 这些指令的类型都是pngs_cmd_t，请参考pngs_def.h
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// 指令部分
////////////////////////////////////////////////////////////////////
enum
{
	////////////////////////////////////////////////////////////////
	// CAAFS->GMS
	////////////////////////////////////////////////////////////////
	// CAAFS连入GMS后给出自己的信息
	CAAFS_GMS_HELLO						= 1,
	// CAAFS对GMS的一些控制工作(如：GMS发来的字串指令的返回等等)
	CAAFS_GMS_CTRL						= 2,
	// 通知GMS有玩家想开始连接CLS了(玩家的一些信息附带通知给CLS)
	CAAFS_GMS_CLIENTWANTCLS				= 3,
	// CAAFS发来的一些预先请求（如：用户连上CAAFS的时候，每个人可以做一次PRELOGIN，校验一下自身的信息。用户不能发送太多这样的信息，一个连接只能发送一次）
	CAAFS_GMS_CLIENT_CMD				= 4,
	////////////////////////////////////////////////////////////////
	// GMS->CAAFS
	////////////////////////////////////////////////////////////////
	// GMS告诉CAAFS可以工作
	GMS_CAAFS_HI						= 11,
	// GMS对CAAFS的一些控制工作(如：禁止/允许用户登录、动态修改版本等等)
	GMS_CAAFS_CTRL						= 12,
	// GMS告诉CAAFS有GMS的信息变化了(如：在线人数变化等等)
	GMS_CAAFS_GMSINFO					= 13,
	// GMS告诉CAAFS可以让玩家开始连接CLS了
	// 只要相应的CLS存在则CAAFS一定会收到这个返回的(如果CLS掉了，GMS在清理CLS用户的时候也会向CAAFS发送用户连接结果的消息)
	GMS_CAAFS_CLIENTCANGOTOCLS			= 14,
	// GMS告诉CAAFS玩家请求关联队列和帐号
	GMS_CAAFS_KEEP_QUEUE_POS			= 15,
	////////////////////////////////////////////////////////////////
	// CLS->GMS
	////////////////////////////////////////////////////////////////
	// CLS连入GMS后给出自己的初始信息
	CLS_GMS_HELLO						= 21,
	CLS_GMS_HELLO1						= 22,
	// CLS通知GMS自己和某个GZS连接成功(这样GMS就可以标记这个CLS可以继续工作了)
	CLS_GMS_CONNECTEDTOGZS				= 23,
	// CLS通知GMS客户端可以来连自己了
	CLS_GMS_CLIENTCANGOTOCLS			= 24,
	// CLS通知GMS客户端连接结果
	CLS_GMS_CLIENTTOCLS_RST				= 25,
	// CLS通知GMS客户端断线(gms收到这个消息的时候用户应该已经从CLS删除了)
	CLS_GMS_CLIENT_DROP					= 26,
	// 一些控制工作（如通知自己要正常停机了）
	CLS_GMS_CTRL						= 27,
	////////////////////////////////////////////////////////////////
	// GMS->CLS
	////////////////////////////////////////////////////////////////
	// GMS告诉CLS可以工作
	GMS_CLS_HI							= 31,
	// GMS告诉CLS有GZS变化的信息(CLS收到后就会去连GZS)
	GMS_CLS_GZSINFO						= 33,
	// GMS告诉CLS将要有客户端来连接它
	GMS_CLS_CLIENTWANTCLS				= 34,
	// 通知CLS指定玩家后面需要和那个GZS通讯
	GMS_CLS_ROUTECLIENTDATATOGZS		= 35,
	// GMS公制CLS把用户ID修改为别的（这个应该是用户重连继承旧用户用的）
	GMS_CLS_PLAYER_CHANGEID				= 36,
	////////////////////////////////////////////////////////////////
	// GZS->GMS
	////////////////////////////////////////////////////////////////
	// CZS连入GMS后给出自己的初始信息(如：我是1服)
	GZS_GMS_HELLO						= 41,
	// GZS告诉GMS自己的可用地址(这个只有收到GMS的Hi之后才能有结果)
	GZS_GMS_MYINFO						= 42,
	// 游戏逻辑相关的指令(即GamePlay层需要用到的指令)
	GZS_GMS_GAMECMD						= 43,
	// 告诉GMS踢用户下线。这个应该是在GZS和CLS断连的情况下才会发出的。
	GZS_GMS_KICKPALYERBYID				= 44,
	// 一些控制工作（如通知自己要正常停机了）
	GZS_GMS_CTRL						= 45,
	////////////////////////////////////////////////////////////////
	// GMS->GZS
	////////////////////////////////////////////////////////////////
	// GMS告诉GZS可以工作
	GMS_GZS_HI							= 51,
	// GMS对GZS的一些控制工作(如：踢出某个用户、请求某些信息等)
	GMS_GZS_CTRL						= 52,
	// 游戏逻辑相关的指令(即GamePlay层需要用到的指令)
	GMS_GZS_GAMECMD						= 53,
	// 通知让玩家上线(只是连接层面的)
	GMS_GZS_PLAYER_ONLINE				= 54,
	// 通知让玩家下线(只是连接层面的)
	GMS_GZS_PLAYER_OFFLINE				= 55,
	// 通知GZS把用户ID改变
	GMS_GZS_PLAYER_CHANGEID				= 56,
	// 
	////////////////////////////////////////////////////////////////
	// CLS->GZS
	////////////////////////////////////////////////////////////////
	// CLS连入GZS后给出自己的初始信息
	CLS_GZS_HELLO						= 61,
	////////////////////////////////////////////////////////////////
	// GZS->CLS
	////////////////////////////////////////////////////////////////
	// GZS告诉CLSS可以工作
	GZS_CLS_HI							= 71,
	////////////////////////////////////////////////////////////////
	// CLIENT->CAAFS
	////////////////////////////////////////////////////////////////
	// 发送客户端的一些信息（如：版本、交换密钥的PubForServer结果）
	// CLIENT_CAAFS_HELLO					= 81, 2005-08-16 改为直接通过连接参数了
	////////////////////////////////////////////////////////////////
	// CAAFS->CLIENT
	////////////////////////////////////////////////////////////////
	// 自我介绍（比如我是刀剑II的CAAFS服务器，加密方式为0，之类的，避免其他客户端误连接，只有客户端判断可以连接才做下一步发送HELLO）
	// 客户端判断info后如果不断线的话就可以继续工作了
	CAAFS_CLIENT_CAAFSINFO				= 91,
	// 通知用户目前排队的状况
	CAAFS_CLIENT_QUEUEINFO				= 92,
	// 通知用户可以开始连接CLS了(告诉用户CLS的地址，以及连接密码)
	CAAFS_CLIENT_GOTOCLS				= 93,
	////////////////////////////////////////////////////////////////
	// CLIENT->CAAFS
	////////////////////////////////////////////////////////////////
	CLIENT_CAAFS_2GMS					= 98,
	//登录前的准备，向CAAFS发送准备登陆的帐号
	CLIENT_CAAFS_BEFORE_LOGIN				= 99,
	////////////////////////////////////////////////////////////////
	// CLIENT->CLS
	////////////////////////////////////////////////////////////////
	// 发送客户端的一些信息（如：密码、交换密钥的PubForServer结果）
	// CLIENT_CLS_HELLO					= 101,
	// 用户数据
	CLIENT_CLS_DATA						= 102,
	// 用户统计信息
	// 结构CLIENT_CLS_DATA_T
	CLIENT_CLS_STAT						= 103,
	////////////////////////////////////////////////////////////////
	// CLS->CLIENT
	////////////////////////////////////////////////////////////////
	// CLS告诉客户端身份校验是否成功（还有交换密钥的PubForClient结果），成功就可以开始工作了
	//CLS_CLIENT_HI						= 111,
	// 用户数据
	CLS_CLIENT_DATA						= 112,
	// 用户数据打包（需要先解开再传给上层）
	CLS_CLIENT_DATA_PACK				= 113,
	// 对用户进行一些通知
	CLS_CLIENT_NOTIFY					= 114,
	////////////////////////////////////////////////////////////////
	// CLS发送给服务器的公共指令
	////////////////////////////////////////////////////////////////
	// 客户端发来的数据
	CLS_SVR_CLIENT_DATA					= 201,
	// 客户端发来的统计信息
	// 结构CLS_SVR_CLIENT_DATA_T
	CLS_SVR_CLIENT_STAT					= 202,
	////////////////////////////////////////////////////////////////
	// CLS收到的服务器发来的公共指令
	////////////////////////////////////////////////////////////////
	// 服务器对CLS的一些控制工作(如：踢出某个用户、请求某些信息等)
	SVR_CLS_CTRL						= 210,
	// 设置玩家标志
	SVR_CLS_SET_TAG_TO_CLIENT			= 211,
	// 设置玩家属性（比如是否录像、是否输出包日志等等）
	SVR_CLS_SET_PROP_TO_CLIENT			= 212,
	// **** 注意：后面的XXX_PACK和XXX指令共用指令结构 ****
	// 发给单一玩家的数据
	SVR_CLS_CLIENT_DATA					= 213,
	SVR_CLS_CLIENT_DATA_PACK			= 214,
	// 发给多个玩家的数据
	SVR_CLS_MULTICLIENT_DATA			= 215,
	SVR_CLS_MULTICLIENT_DATA_PACK		= 216,
	// 发给某个标志满足条件的玩家
	SVR_CLS_TAGGED_CLIENT_DATA			= 217,
	SVR_CLS_TAGGED_CLIENT_DATA_PACK		= 218,
	// 发给所有玩家
	SVR_CLS_ALL_CLIENT_DATA				= 219,
	SVR_CLS_ALL_CLIENT_DATA_PACK		= 220,
	// 设置64位的发送标志
	SVR_CLS_SET_TAG64_TO_CLIENT			= 221,
	// 发给某个64位标志满足条件的玩家
	SVR_CLS_TAGGED64_CLIENT_DATA		= 222,
	SVR_CLS_TAGGED64_CLIENT_DATA_PACK	= 223,

	////////////////////////////////////////////////////////////////
	// GM工具向服务器发出的指令
	////////////////////////////////////////////////////////////////
	GMTOOL_SVR_HELLO					= 241,
	GMTOOL_SVR_CMD						= 242,
	////////////////////////////////////////////////////////////////
	// 服务器向GM工具发出的指令
	////////////////////////////////////////////////////////////////
	SVR_GMTOOL_HI						= 251,
	SVR_GMTOOL_CMD_RST					= 252,

	////////////////////////////////////////////////////////////////
	// CLS_SVR_CLIENT_STAT的子指令
	////////////////////////////////////////////////////////////////
	// 客户端机器启动时刻
	// 参数为一个整数
	CLIENT_STAT_STAT_SUBCMD_ONTIME		= 1,
};
////////////////////////////////////////////////////////////////////
// 数据结构部分
////////////////////////////////////////////////////////////////////
#pragma pack(push, old_pack_num, 1)
// 下面所有的指令结构都是以nCmd开头
////////////////////////////////////////////////////////////////////
// CAAFS->GMS
////////////////////////////////////////////////////////////////////
struct	CAAFS_GMS_HELLO_T
{
	pngs_cmd_t		nCmd;
	int				nGroupID;										// CAAFS组号和相应的CLS对应
	int				nCAAFSVer;										// CAAFS版本
	bool			bReConnect;										// 表示这个是重连导致的
};
struct	CAAFS_GMS_CTRL_T
{
	enum
	{
		SUBCMD_STRCMD			= 0,								// 字串指令
		SUBCMD_STRRST			= 1,								// 字串指令的返回
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nExt;											// 附加参数原样返回
	int				nParam;											// 一般默认是一个int参数。有扩展参数的时候根据nSubCmd决定
};
struct	CAAFS_GMS_CLIENTWANTCLS_T
{
	pngs_cmd_t		nCmd;
	int				nClientIDInCAAFS;								// 用户在CAAFS中的ID
	unsigned int	IP;												// 用户IP
	unsigned int	nPassword;										// 用户用来连接CLS的密码
	unsigned char	nNetworkType;									// 用户的网络类型
};
struct	CAAFS_GMS_CLIENT_CMD_T
{
	pngs_cmd_t		nCmd;
	int				nClientIDInCAAFS;								// 用户在CAAFS中的ID
	unsigned int	IP;
	// 后面跟的是相关指令和数据（比如PRELOGIN的和正常LOGIN数据类似。不过这里是底层就不写数据的具体结构了）
};
////////////////////////////////////////////////////////////////////
// GMS->CAAFS
////////////////////////////////////////////////////////////////////
struct	GMS_CAAFS_HI_T
{
	enum
	{
		RST_OK					= 0,								// 连接成功
		RST_ERR_BAD_VER			= 1,								// 版本比匹配
		RST_ERR_BAD_GROUPID		= 2,								// GroupID超界
		RST_ERR_DUP_GROUPID		= 3,								// 这个GroupID已经有CAAFS连入了
		RST_ERR_MEMERR			= 4,								// 内存分配错误
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;
	int				nGMSVer;										// GMS版本
};
struct	GMS_CAAFS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// 退出
		SUBCMD_STRCMD			= 1,								// 字串指令
		SUBCMD_STRRST			= 2,								// 字串指令的返回
		SUBCMD_CLIENTCMDNUMSUB1	= 3,								// 把用户的指令计数减1。nExt是用户的nClientIDInCAAFS
																	// 不用返回
		SUBCMD_CLIENTQUEUETRANS	= 4,								// 转移用户的排队（如：进入vip1等）。nExt是用户的nClientIDInCAAFS。nParam是目标队列。
																	// 不用返回
		SUBCMD_CLIENTQUEUEPOSRESUME = 5,	
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nCAAFSIdx;										// 这个主要是上层发给MS层用的，用来指定发给那个CAAFS
	int				nExt;											// 附加参数原样返回(一般是用来定位指令发起者的，如：GM的序号)
	int				nParam;											// 一般默认是一个int参数。有扩展参数的时候根据nSubCmd决定
};
struct	GMS_CAAFS_GMSINFO_T
{
	pngs_cmd_t		nCmd;
	unsigned short	nGMSAvailPlayer;								// GMS认为这个CAAFS还能尝试连入游戏的人数
																	// GMS会给每个CAAFS分配一定的连接数(这样多个CAAFS的时候人数就不会冲突了)
};
struct	GMS_CAAFS_CLIENTCANGOTOCLS_T
{
	enum
	{
		RST_OK					= 0,								// 成功，可以连接CLS了
		RST_ERR_MEM				= 1,								// 失败，无法分配内存
		RST_ERR_CLS				= 2,								// 失败，没有可去的CLS或者和CLS通讯过程中没有反应
		RST_ERR_NOCLSCANACCEPT	= 3,								// 失败，没有能够接纳用户的CLS了
		RST_ERR_OTHERKNOWN		= 99,								// 失败，其他未知错误
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;											// 是否真的cango
	port_t			nPort;											// CLS的端口
	unsigned int	IP;												// CLS的IP
	int				nClientIDInCAAFS;								// 用户在CAAFS中的ID
	int				nClientID;										// 用户在GMS中的ID，这个就是今后玩家在游戏过程中的统一在线ID
																	// 这个ID在连接CLS的时候用于确定自己对应的Player对象
};
struct	GMS_CAAFS_KEEP_QUEUE_POS_T
{
	pngs_cmd_t		nCmd;
	int				nClientIDInCAAFS;
	int				nCAAFSIdx;
	char			szAccount[64];
};
////////////////////////////////////////////////////////////////////
// CLS->GMS
////////////////////////////////////////////////////////////////////
struct	CLS_GMS_HELLO_T
{
	pngs_cmd_t		nCmd;
	int				nGroupID;										// CLS组号和相应的CAAFS对应
	int				nCLSVer;										// CLS版本
	int				nOldID;											// 旧的CLSID，如果非0则表示是重连的
	unsigned char	nNetworkType;									// 网络类型（NETWORKTYPE_XXX）
	bool			bCanAcceptPlayerWhenCLSConnectingGZS;			// 在CLS连接GZS的过程中允许用户连接
};
struct	CLS_GMS_HELLO1_T
{
	pngs_cmd_t		nCmd;
	port_t			nPort;											// 对Client的端口
	unsigned int	IP;												// 对Client的IP
};
struct	CLS_GMS_CONNECTEDTOGZS_T
{
	enum
	{
		RST_OK		= 0,											// 成功
		RST_FAIL	= 1,											// 连接失败（比如在链接过程中GZS死掉了）
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;											// 连接的结果
	unsigned char	nSvrIdx;										// 连接上的GZS的序号
};
struct	CLS_GMS_CLIENTCANGOTOCLS_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;										// 用户在GMS中的ID(其实也是在CLS中的ID)
};
struct	CLS_GMS_CLIENTTOCLS_RST_T
{
	enum
	{
		RST_OK					= 0,								// 连接成功
		RST_ERR_NOTCONNECT		= 1,								// 客户端根本没有来连接(超时且)
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;											// 是否成功
	int				nClientID;										// 用户在GMS中的ID(其实也是在CLS中的ID)
};
struct	CLS_GMS_CLIENT_DROP_T
{
	enum
	{
		REMOVEREASON_NOTHING	= 0,
		REMOVEREASON_SENDERR	= 1,								// 向客户端发送数据是Send返回错误（可能是缓冲满了）
		REMOVEREASON_CLIENTHACK	= 2,								// 客户端发来的指令有hack的嫌疑
		REMOVEREASON_KICKED		= 3,								// 客户端被服务器用指令踢出
		REMOVEREASON_NOTEXIST	= 4,								// 客户端对应的ID不存在
		REMOVEREASON_END		= 5,								// 服务器正常让用户下线
																	// （比如服务器要发送结果给某个客户端，或者是要踢出某个客户端时，发现对应的ID实际上不存在。或者CLS重连GMS之后，告诉GMS某些ID已经不存在了，让GMS删除一下）
		REMOVEREASON_AS_DROP	= 6,
		// 前面的应该不会超过100吧，呵呵(后面的这些都是因为Close造成的)
		REMOVEREASON_SLOTCLOSE_REASON_NOTHING		= 100 + CNL2SlotMan::SLOTCLOSE_REASON_NOTHING,
		REMOVEREASON_SLOTCLOSE_REASON_INITIATIVE	= 100 + CNL2SlotMan::SLOTCLOSE_REASON_INITIATIVE,
		REMOVEREASON_SLOTCLOSE_REASON_PASSIVE		= 100 + CNL2SlotMan::SLOTCLOSE_REASON_PASSIVE,
		REMOVEREASON_SLOTCLOSE_REASON_CLOSETimeout	= 100 + CNL2SlotMan::SLOTCLOSE_REASON_CLOSETimeout,
		REMOVEREASON_SLOTCLOSE_REASON_DROP			= 100 + CNL2SlotMan::SLOTCLOSE_REASON_DROP,
		REMOVEREASON_SLOTCLOSE_REASON_CONNECTTimeout= 100 + CNL2SlotMan::SLOTCLOSE_REASON_CONNECTTimeout,
		REMOVEREASON_SLOTCLOSE_REASON_ACCEPTTimeout	= 100 + CNL2SlotMan::SLOTCLOSE_REASON_ACCEPTTimeout,
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRemoveReason;									// 如果在RemovePlayer之前设置了这个则说明是有一下原因关闭的
	int				nClientID;										// 用户在GMS中的ID(其实也是在CLS中的ID)
};
struct	CLS_GMS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// 退出(告诉GMS自己是正常退出了)
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nParam;
};
////////////////////////////////////////////////////////////////////
// GMS->CLS
////////////////////////////////////////////////////////////////////
struct	GMS_CLS_HI_T
{
	enum
	{
		RST_OK					= 0x00,								// 连接成功
		RST_ERR_BAD_VER			= 0x01,								// 版本错误
		RST_ERR_BAD_GROUPID		= 0x02,								// GroupID超界
		RST_ERR_TOOMANY_CLS		= 0x03,								// 这个GroupID的CLS超过了总需要量
		RST_ERR_MEMERR			= 0x04,								// 内存分配错误
		RST_ERR_UNKNOWN			= 0xFF,								// 未知错误
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;
	int				nCLSID;											// 该CLS在GMS中的ID
	int				nGMSMaxPlayer;									// GMS最多支持的用户数量(这个牵扯到CLS中的Player数组的大小)
	int				nCLSMaxConnection;								// GMS规定该CLS最多可以承载的用户连接数量
	int				nGMSVer;										// GMS版本
};
struct	GMS_CLS_GZSINFO_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// GZS服务器序号(1~GZS_MAX_NUM-1)
	port_t			nPort;											// 可以连入的端口
	unsigned int	IP;												// 可以连入的IP
};
struct	GMS_CLS_CLIENTWANTCLS_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;										// 用户在GMS中的ID(其实也将是在CLS中的ID)
	unsigned int	nPassword;										// 连接密码(这个是CAAFS发给GMS，然后GMS再转发过来的)
};
struct	GMS_CLS_ROUTECLIENTDATATOGZS_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;										// 用户在GMS中的ID(其实也将是在CLS中的ID)
	unsigned char	nSvrIdx;										// 需要通讯的GZS的序号(如果为0则表示脱离开GZS，回到GMS的怀抱)
};
struct	GMS_CLS_PLAYER_CHANGEID_T
{
	pngs_cmd_t		nCmd;
	int				nFromID;
	int				nToID;
	unsigned char	nSvrIdx;										// 需要告诉一下CLS，让用户和相应的GZS关联
};
////////////////////////////////////////////////////////////////////
// GZS->GMS
////////////////////////////////////////////////////////////////////
struct	GZS_GMS_HELLO_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// GZS服务器序号(1~GZS_MAX_NUM-1)，也可用作AuthID
	unsigned char	nForceKickOldGZS;								// 如果有旧的nSvrIdx和这个一样的，就踢出旧的GZS (这个一般只能是用来踢出正在运行中的GZS了，因为是TCP所以不会出现原来的调整超时过长导致GZS还存在的现象)
																	// 其实TCP会出现很长时间不断线的情况，所以还是要有强制踢出。参见GZS_MAINSTRUCTURE::CFGINFO_T::nForceKickOldGZS说明
	int				nGZSVer;										// GZS版本
	bool			bReConnect;										// 表示这个是重连导致的
};
struct	GZS_GMS_MYINFO_T
{
	pngs_cmd_t		nCmd;
	port_t			nPort;											// 可以连入的端口
	unsigned int	IP;												// 可以连入的IP
};
struct	GZS_GMS_GAMECMD_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nToSvrIdx;										// 目标是哪个服务器，取值为PNGS_SVRIDX_XXX
	char			data[1];										// 真正的指令+数据部分
};
struct	GZS_GMS_KICKPALYERBYID_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;
};
struct	GZS_GMS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// 退出(告诉GMS自己是正常退出了)
		SUBCMD_STRCMD			= 1,								// 字串指令
																	// nParam的首地址被解释为字串首地址
		SUBCMD_STRRST			= 2,								// 字串指令的返回
		SUBCMD_CLSDOWN			= 3,								// 某个CLS宕了（和该GZS断线了）
																	// nParam就是CLSID
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nParam;
};

////////////////////////////////////////////////////////////////////
// GMS->GZS
////////////////////////////////////////////////////////////////////
struct	GMS_GZS_HI_T
{
	enum
	{
		RST_OK		= 0,
		RST_IDXDUP	= 2,											// 说明自己的Idx和别人重复了
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;
	int				nGMSMaxPlayer;									// GMS最多支持的用户数量(这个牵扯到GZS中的Player数组的大小)
	int				nGMSMaxCLS;										// GMS最多支持的CLS数量(这个牵扯到GZS中的CLS数组的大小)
	int				nGMSVer;										// GMS版本
};
struct	GMS_GZS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// 退出(如果是GMS发给其他服务器的，则相当于服务器自动退出。如果是GZS发给其他服务器的，则自动关闭和GZS的连接)
		SUBCMD_STRCMD			= 1,								// 字串指令
																	// nParam的首地址被解释为字串首地址
		SUBCMD_STRRST			= 2,								// 字串指令的返回
		SUBCMD_CLSDOWN			= 3,								// 某个CLS宕了
																	// nParam就是CLSID
		SUBCMD_CHECKPLAYEROFFLINE		= 6,						// 检查是否有用户已经下线了（GZS检查各个ID是否存在，如果不存在就直接删除相应的用户）
																	// nParam是用户ID个数，后面跟着int数组
		SUBCMD_ENABLE_CLIENT_RECV		= 10,						// 设置是否接收客户端来包的掩码
		SUBCMD_DISENABLE_CLIENT_RECV	= 11,						// 清除是否接收客户端来包的掩码
																	// 上面两个的参数就是nParam，对应第几个bit
																	// 现在先定GSDB/GMSGamePlay:0、GZSGamePlay:1
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nParam;
};
struct	GMS_GZS_GAMECMD_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nFromSvrIdx;									// 告诉它指令来自那个服务器，取值为PNGS_SVRIDX_XXX
	char			data[1];										// 真正的指令+数据部分
};
struct	GMS_GZS_PLAYER_ONLINE_T
{
	pngs_cmd_t		nCmd;
	int				nCLSID;											// 玩家对应的CLS的ID(如果没有这个那么GZS就无法直接给Player发送信息了)
	int				anClientID[1];									// 用户在GMS中的ID(其实也是在GZS中的ID)。可以是多个ID。因为可能是在GZS连接GMS之后GMS一次发送多个给GZS。
};
struct	GMS_GZS_PLAYER_OFFLINE_T
{
	pngs_cmd_t		nCmd;
	int				anClientID[1];									// 用户在GMS中的ID(其实也是在GZS中的ID)。可能是多个ID。
};
struct	GMS_GZS_PLAYER_CHANGEID_T
{
	pngs_cmd_t		nCmd;
	int				nFromID;
	int				nToID;
};
////////////////////////////////////////////////////////////////////
// CLS->GZS
////////////////////////////////////////////////////////////////////
struct	CLS_GZS_HELLO_T
{
	pngs_cmd_t		nCmd;
	int				nCLSID;											// 自己在GMS中的ID
	int				nCLSVer;										// CLS版本
};
////////////////////////////////////////////////////////////////////
// GZS->CLS
////////////////////////////////////////////////////////////////////
struct	GZS_CLS_HI_T
{
	enum
	{
		RST_OK				= 0,
		RST_ERR_BAD_VER		= 1,
		RST_ERR_MEMERR		= 2,
	};
	pngs_cmd_t		nCmd;
	unsigned char	nRst;
	int				nGZSVer;										// GZS版本
};
////////////////////////////////////////////////////////////////////
// CLIENT->CAAFS
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// CAAFS->CLIENT
////////////////////////////////////////////////////////////////////
struct	CAAFS_CLIENT_CAAFSINFO_T
{
	enum
	{
		SELF_NOTIFY_REFUSEALL			= 0x00000001,				// 拒绝所有的登录
	};
	pngs_cmd_t		nCmd;
	unsigned char	nVerCmpMode;									// 版本比较方式
	char			szVer[PNGS_VER_LEN];							// 版本
	unsigned short	nQueueSize;										// 前面排队的人数(超过一定数量就不要让用户连接了，并告诉他前面排队人数太多)
	unsigned int	nSelfNotify;									// 32bit表示32个意思
	char			info[1];										// 介绍字串
};
struct	CAAFS_CLIENT_QUEUEINFO_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nVIPChannel;									// 对应的VIP通道（如果不是0，就在客户端显示通道号）
	unsigned short	nQueueSize;										// 前面排队的人数
};
struct	CAAFS_CLIENT_GOTOCLS_T
{
	pngs_cmd_t		nCmd;
	port_t			nPort;											// CLS端口
	unsigned int	IP;												// CLS地址IP
	unsigned int	nPassword;										// 连接密码
	int				nClientID;										// 玩家ID
};
////////////////////////////////////////////////////////////////////
// CLIENT->CAAFS
////////////////////////////////////////////////////////////////////
struct	CLIENT_CAAFS_2GMS_T
{
	pngs_cmd_t		nCmd;
	char			data[1];										// 和正常login一样的数据包。不过这个是逻辑内容，这里就不写了。
};
////////////////////////////////////////////////////////////////////
// CLIENT->CLS
////////////////////////////////////////////////////////////////////
// 连接的同时客户端附带的一些信息（如：用户ID、密码）
struct	CLIENT_CLS_CONNECT_EXTINFO_T								// 注意：这个不出一条指令，只是连接所附带的参数
{
	int				nClientID;										// 玩家ID
	unsigned int	nPassword;										// 用户用来连接CLS的密码
};
// 用户数据
struct	CLIENT_CLS_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// 表明这条指令一定是给谁的。取值：PNGS_SVRIDX_XXX
																	// PNGS_SVRIDX_AUTOGZS表示给当前指定方向的GZS
																	// PNGS_SVRIDX_GMS表示给GMS
																	// 其他的表示给依次顺序的GZS
	char			data[1];
};
////////////////////////////////////////////////////////////////////
// CLS->CLIENT
////////////////////////////////////////////////////////////////////
// 用户数据
struct	CLS_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	char			data[1];
};
struct	CLS_CLIENT_DATA_PACK_T
{
	pngs_cmd_t		nCmd;
	char			data[1];
};
struct	CLS_CLIENT_NOTIFY_T
{
	enum
	{
		SUBCMD_GZSDROP	= 0x01,			// 用户所在的GZS宕机了，请用户等待其重启
		SUBCMD_STATON	= 0x02,			// 开启stat
		SUBCMD_STATOFF	= 0x03,			// 关闭stat
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	char			data[1];
};
////////////////////////////////////////////////////////////////////
// CLS向服务器发送的公共指令
////////////////////////////////////////////////////////////////////
struct	CLS_SVR_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nChannel;
	int				nClientID;
	char			data[1];										// 数据
};
////////////////////////////////////////////////////////////////////
// CLS收到的服务器发来的公共指令
////////////////////////////////////////////////////////////////////
struct	SVR_CLS_CTRL_T
{
	enum
	{
		SUBCMD_EXIT				= 0,								// 退出
																	// nParam无意义
		SUBCMD_STRCMD			= 1,								// 字串指令
																	// nParam的首地址被解释为字串首地址
		SUBCMD_STRRST			= 2,								// 字串指令的返回
																	// nParam的首地址被解释为字串首地址
		SUBCMD_KICKPALYERBYID	= 3,								// 根据ID踢出某个用户
																	// nParam为PlayerID
		SUBCMD_DROPGZS			= 4,								// 和GZS断线(这个是GMS发现GZS断线后通知CLS的，这个主要是用来防止GZS和GMS单方面断线做的)
																	// nParam为SvrIdx
		SUBCMD_KICKPLAYEROFGZS	= 5,								// 让CLS踢出所有和某个GZS相关的玩家
																	// nParam为SvrIdx
		SUBCMD_CHECKPLAYEROFFLINE		= 6,						// 检查是否有用户已经下线了（这个只有GMS会发来，CLS检查各个ID是否存在，如果不存在就重新向GMS发送用户下线消息）
																	// nParam是用户ID个数，后面跟着int数组
		SUBCMD_KICKPALYERBYID_AS_DROP	= 7,						// 和SUBCMD_KICKPALYERBYID功能一样，不过会向GMS返回用户是DROP的结果
		SUBCMD_KICKPALYERBYID_AS_END	= 8,						// 和SUBCMD_KICKPALYERBYID功能一样，不过会向GMS返回用户是正常退出的结果（用这个kick用户不会再保留在游戏中延迟一段时间了）
		SUBCMD_SET_NAME			= 9,								// 设置用户字串名（主要用于记录日志）。nParam为用户ID，后面跟着字串
		SUBCMD_PLAYER_STAT_OFF	= 10,								// 关闭统计， nParam为ClientID
		SUBCMD_PLAYER_STAT_ON	= 11,								// 开启统计， nParam为ClientID，后面还有一个整数表示发送间隔
	};
	pngs_cmd_t		nCmd;
	pngs_cmd_t		nSubCmd;
	int				nParam;
};
struct	SVR_CLS_SET_TAG_TO_CLIENT_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nTagIdx;										// nTag的索引值
	short			nTagVal;										// 序号对应的值（注意：nTagVal不能为0）
	int				nClientID;										// 玩家ID
};
struct	SVR_CLS_SET_PROP_TO_CLIENT_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nProp;											// 属性（定义为PNGS_CLS_PLAYER_PROP_LOG_XXX，在pngs_def.h中）
	int				nClientID;										// 玩家ID
};
struct	SVR_CLS_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nChannel;										// 对应的通道
	int				nClientID;										// 玩家ID
	char			data[1];										// 数据
};
struct	SVR_CLS_MULTICLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nChannel;										// 对应的通道
	unsigned short	nClientNum;										// 后面跟的ClientID的数量
	int				anClientID[1];									// nClient数组开头
	// 这一些解析完之后，后面还有附加的数据就是真实的数据部分
	int	GetDSize(int nTotalSize)
	{
		return	nTotalSize - wh_offsetof(SVR_CLS_MULTICLIENT_DATA_T, anClientID) - nClientNum*sizeof(int);
	}
	void *	GetDataPtr()
	{
		return	anClientID + nClientNum;
	}
	static int	GetTotalSize(int nCNum, int nDSize)
	{
		return	wh_offsetof(SVR_CLS_MULTICLIENT_DATA_T, anClientID) + nCNum*sizeof(int) + nDSize;
	}
};
struct	SVR_CLS_TAGGED_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// 取值可以是PNGS_SVRIDX_XXX或者是1~15的值
	unsigned char	nChannel;										// 对应的通道
	unsigned char	nTagIdx;										// nTag的索引值
	short			nTagVal;										// 序号满足的值(所有的默认值都是0)
	char			data[1];										// 数据
};
struct	SVR_CLS_ALL_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nChannel;										// 对应的通道
	unsigned char	nSvrIdx;										// 对应的服务器序号（定义为PNGS_SVRIDX_XXX）	2007-10-26加，为了今后扩充给某些GZS的用户发送广播方便
	char			data[1];										// 数据
};
struct	SVR_CLS_SET_TAG64_TO_CLIENT_T
{
	pngs_cmd_t		nCmd;
	int				nClientID;										// 玩家ID
	whuint64		nTag;
	bool			bDel;											// 如果为真则表示删除玩家的这个标记
};
struct	SVR_CLS_TAGGED64_CLIENT_DATA_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrIdx;										// 取值可以是PNGS_SVRIDX_XXX或者是1~15的值
	unsigned char	nChannel;										// 对应的通道
	whuint64		nTag;
	char			data[1];										// 数据
};

struct	GMTOOL_SVR_HELLO_T
{
	pngs_cmd_t		nCmd;
	int				nGMTOOLVer;										// GMTOOL的版本
};
struct	GMTOOL_SVR_CMD_T
{
	pngs_cmd_t		nCmd;
	int				nExt;											// 需要原样返回的数据
	char			data[1];										// 数据
};
struct	SVR_GMTOOL_HI_T
{
	pngs_cmd_t		nCmd;
	unsigned char	nSvrType;										// 服务器类型APPTYPE_XXX（定义在pngs_def.h中）
	int				nVer;											// 服务器版本
};
struct	SVR_GMTOOL_CMD_RST_T
{
	pngs_cmd_t		nCmd;
	int				nExt;											// 原样返回的数据
	char			data[1];										// 数据
};
////////////////////////////////////////////////////////////////////
// 客户端连接CAAFS时所附带的附加数据
////////////////////////////////////////////////////////////////////
struct	CLIENT_CONNECT_CAAFS_EXTINFO_T
{
	struct	CMN_T
	{
		unsigned char	nNetworkType;								// NETWORKTYPE_XXX
		CMN_T()
			: nNetworkType(NETWORKTYPE_UNKNOWN)
		{
		}
	}cmn;
	char	junk[32-sizeof(CMN_T)];
};
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_H__
