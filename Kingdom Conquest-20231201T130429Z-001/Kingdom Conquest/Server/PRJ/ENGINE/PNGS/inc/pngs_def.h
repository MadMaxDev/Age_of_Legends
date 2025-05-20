// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_def.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的一些公共定义
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-07-28
// Change LOG   : 2006-08-30 把PNGS_DFT_HELLOW_TIMEOUT改为两分钟

#ifndef	__PNGS_DEF_H__
#define	__PNGS_DEF_H__

namespace n_pngs
{

// 公共常量定义
enum
{
	PNGS_DFT_SVRCONNECT_TIMEOUT	= 20000,							// 服务器之间连接的默认超时(毫秒)
	PNGS_DFT_HELLOW_TIMEOUT		= 4*60*1000,						// 等待Hello握手包的默认超时(毫秒)
	//	windows下只能支持这么多,modified by yuezhongyue 2011-10-18
#ifdef	WIN32
	GZS_MAX_NUM					= 26,								// 最多的GZS数量，不过一般第0个是不用的
#else
	GZS_MAX_NUM					= 52,								// 最多的GZS数量，不过一般第0个是不用的
#endif
	GZS_STRCMD_RST_LEN_MAX		= 4096,								// 服务器相关的字串指令返回长度的最大值
	CAAFS_CHANNEL_INOUT_NUM		= 16,								// CAAFS与客户端通讯的Channel的In、Out队列的长度
	GMS_MAX_CAAFS_NUM			= 4,								// 最多的CAAFS的组数
																	// 这个也决定了最多的CAAFS所属Group的数量
	GMS_DFT_PORT				= 3000,								// GMS默认被其他Server连接的端口
	PNGS_MAX_SOCKETTOSELECT		= 512,								// 可供选择的最多socket数量
	// 一般预留的指令缓冲的大小
	PNGS_RAWBUF_SIZE			= 512*1024,
	// 版本串的长度
	PNGS_VER_LEN				= 32,
	// 逻辑名字的长度
	PNGS_LOGICNAME_LEN			= 32,
	// 一个tick超过多少毫秒就认为是big tick
	PNGS_BIG_TICK_LIMIT			= 200,
	// 客户端和CLS通讯的SLOTINOUTNUM
	PNGS_SLOTINOUTNUM			= 256,
	// added by yuezhongyue, web extension
	LP_MAX_CAAFS4Web_NUM		= GMS_MAX_CAAFS_NUM,			// 最多的CAAFS的组数
};
enum
{
	PNGS_SVRIDX_AUTOGZS			= 0xFF,
	PNGS_SVRIDX_ALLGZSEXCEPTME	= 0xFE,								// 发送给所有的GZS，除了来源的那个
	PNGS_SVRIDX_ALLGZS			= 0xFA,								// 发送给所有的GZS（也可以解释为发送给所有用户（包括GMS上的用户））
	PNGS_SVRIDX_LBA				= 29,								// 根据64bit统一ID协议修改。
	PNGS_SVRIDX_GMS				= 30,								// 根据64bit统一ID协议修改。
	PNGS_SVRIDX_HYDBS			= 31,								// 根据64bit统一ID协议修改。
	PNGS_SVRIDX_GZS0			= 0,								// 根据64bit统一ID协议修改。（0这个序号是保留不能使用的，真正的GZS序号是从1~15，如果在Go指令中使用这个，则表示需要离开游戏即ZOUT）
};
enum
{
	PNGS_CLIENTTAG_RESERVED		= 0,
	PNGS_CLIENTTAG_ONEGZS		= 1,								// 在同一个GZS内的玩家都会以服务器序号为值
																	// 这个用于在标记发送时向同一个GZS中的所有玩家发送相同的指令
};
enum
{
	PNGS_CLS_PLAYER_PROP_LOG_SEND		= 0x01,						// 发向客户端的数据记录到日志中
	PNGS_CLS_PLAYER_PROP_LOG_RECV		= 0x02,						// 从客户端收到的数据记录到日志中
	PNGS_CLS_PLAYER_PROP_LOG_STAT		= 0x04,						// 进行流量统计并定时输出到日志中
	PNGS_CLS_PLAYER_PROP_ALL	= 0xFF
};

// 各个应用程序的类型标记
// 比如：GMS就根据这个来判断连接自己的程序是什么
enum	APPTYPE
{
	APPTYPE_NOTHING				= 0,
	APPTYPE_GMS					= 1,
	APPTYPE_CAAFS				= 2,
	APPTYPE_CLS					= 3,
	APPTYPE_GZS					= 4,
	APPTYPE_LBA					= 5,
	APPTYPE_DBA					= 6,
	APPTYPE_GMTOOL				= 11,

	// added by yuezhongyue for web extension
	APPTYPE_CLS4WEB				= 12,
	APPTYPE_CAAFS4WEB			= 13,
	APPTYPE_LP					= 14,
	APPTYPE_LBA4Web				= 15,
};

// 对Client的关闭参数
enum
{
	CLOSE_PARAM_TO_CLIENT_SVRMEM		= 1,						// 服务器内存分配错误
	CLOSE_PARAM_TO_CLIENT_BADCMD		= 2,						// 客户端发来的指令（或参数）有问题
	CLOSE_PARAM_TO_CLIENT_NOIDINCLS		= 3,						// 客户端对应的玩家对象在CLS中不存在
	CLOSE_PARAM_TO_CLIENT_PLAYERSTATUSNOTCONNECTING	= 4,			// CLS中的Player不是connecting状态
	CLOSE_PARAM_TO_CLIENT_ERR_EXTINFO	= 5,						// 错误的附加信息
};

}		// EOF namespace n_pngs

// 其他定义
#define	PNGS_DFT_CAAFS_SELFINFO_STRING	"BO2 CAAFS2"

#define	PNGS_GSMEM_NAME_AM				"::am"

#define	PNGS_DLL_NAME_GMS_MAINSTRUCTURE	"uGMS_MAINSTRUCTURE"
#define	PNGS_DLL_NAME_GZS_MAINSTRUCTURE	"uGZS_MAINSTRUCTURE"
#define	PNGS_DLL_NAME_GMS_GAMEPLAY		"uGMS_GAMEPLAY"
#define	PNGS_DLL_NAME_GZS_GAMEPLAY		"uGZS_GAMEPLAY"
#define	PNGS_DLL_NAME_CLIENT_GAMEPLAY	"uCLIENT_GAMEPLAY"
#define	PNGS_DLL_NAME_GSMEM				"uGS_MEM"

// pngs的指令类型(1byte足够了)
typedef	unsigned char					pngs_cmd_t;

// CLS用户用户标记的数据类型（暂时不用了）
typedef	unsigned int					pngs_tag_t;

#endif	// EOF __PNGS_DEF_H__
