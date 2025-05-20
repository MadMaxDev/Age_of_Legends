// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_TCPReceiver_def.h
// Creator      : Wei Hua (魏华)
// Comment      : TCP指令接受模块与其他模块交流的指令定义
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2007-02-25
// Change LOG   : 2007-06-07 修改TCPRECEIVER_VER版本。因为服务器间的tcpmsger的内部包格式改了

#ifndef	__PNGS_TCPRECEIVER_DEF_H__
#define __PNGS_TCPRECEIVER_DEF_H__

namespace n_pngs
{

enum
{
	TCPRECEIVER_VER				= 2,
};

////////////////////////////////////////////////////////////////////
// 指令部分
// 注：TR是TCPReceiver的缩写，CD是CmdDealer的缩写
////////////////////////////////////////////////////////////////////
// 给CmdDealer的
enum
{
	PNGS_TR2CD_0				= 300,
	PNGS_TR2CD_NOTIFY			= PNGS_TR2CD_0 + 1,					// 通知一些事情
	PNGS_TR2CD_CMD				= PNGS_TR2CD_0 + 4,					// 需要处理的指令
};
// 从CmdDealer来的
enum
{
	PNGS_CD2TR_0				= 300,
	PNGS_CD2TR_CONFIG			= PNGS_CD2TR_0 + 1,					// 配置（比如设置CmdDealer的指针）
	PNGS_CD2TR_DISCONNECT		= PNGS_CD2TR_0 + 2,					// 断开某个连接（参数就是整数，ConnecterID）
	PNGS_CD2TR_SETEXT			= PNGS_CD2TR_0 + 3,					// 设置某个连接的附加参数
	PNGS_CD2TR_CMD				= PNGS_CD2TR_0 + 4,					// 指令处理完的结果（或者是反向发送的指令）
	PNGS_CD2TR_FLUSH			= PNGS_CD2TR_0 + 5,					// 让tr把已经收到的消息先都发送一次(以免上层buf不够用,这个是instant的,需要立即执行)
																	// for web,added by yuezhongyue 2011-12-29,不这样做,DBS4Web在加载图片缓存时m_queueCmdIn不够用
};

////////////////////////////////////////////////////////////////////
// 数据结构部分
////////////////////////////////////////////////////////////////////
#pragma pack(push, old_pack_num, 1)
// 给CmdDealer的
struct	PNGS_TR2CD_NOTIFY_T
{
	enum
	{
		SUBCMD_DISCONNECT		= 0x01,								// 有连接断了
																	// nParam1为nCntrID，nParam2为附加参数（可以通过这个找到上层对应的东西）
		SUBCMD_CONNECT			= 0x02,								// 有新连接,nParam1为nCntrID,added by yuezhongyue
	};
	unsigned char	nSubCmd;
	int				nParam1;										// 连接ID
	int				nParam2;										// 连接附加的正数数据
};
struct	PNGS_TR2CD_CMD_T
{
	int				nConnecterID;									// 对应发来指令的连接器ID
	unsigned int	IP;												// 来源地址IP
	size_t			nDSize;											// 整个的长度
	void			*pData;											// 指令的开头
};
// 从CmdDealer来的
struct	PNGS_CD2TR_CONFIG_T
{
	enum
	{
		SUBCMD_SET_CMDDEALER	= 1,								// 设置指令处理器
																	// pRstAccepter就是需要的处理器指针
		SUBCMD_REGISTERCMDDEAL	= 2,								// 设置对特定指令的处理器（可以一个指令给多个单元处理）
																	// nParam可以转换为pngs_cmd_t的指令
																	// pRstAccepter就是需要的处理器指针
	};
	int				nSubCmd;
	int				nParam;											// 根据subcmd的不同解释成为不同的内容（长度可能超过sizeof(int)）
};
struct	PNGS_CD2TR_SETEXT_T
{
	int				nConnecterID;									// 要设置附加数据的连接器ID
	int				nExt;											// 附加数据
};
struct	PNGS_CD2TR_CMD_T
{
	int				nConnecterID;									// 对应发来指令的连接器ID（如果nConnecterID为-8888则广播给所有的连接）
	size_t			nDSize;											// 整个的长度
	void			*pData;											// 结果的开头
};
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

// 其他定义
// TCPReceiver的模块名字（CmdDealer需要找到这个名字然后把自己的指针告诉它）
#define	PNGS_DLL_NAME_TCPRECEIVER	"uTCPReceiver"

#endif	// EOF __PNGS_TCPRECEIVER_DEF_H__
