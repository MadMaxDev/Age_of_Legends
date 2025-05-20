// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_gms_logic.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS内各个模块间的指令包公共定义
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-09-01
// Change LOG   : 2005-09-30 取消了GOPLAYER，这些要放到逻辑层中去写。
//              : 但是原来的GMSPACKET_2GP_PLAYERIN/OUT还是要保留，因为这是网络曾对上层的通告。否则GamePlay也无法知道用户连接成功了。以及用户什么时候断线了。

#ifndef	__PNGS_PACKET_GMS_LOGIC_H__
#define	__PNGS_PACKET_GMS_LOGIC_H__

#include "./pngs_packet.h"

namespace n_pngs
{

// 模块版本
enum
{
	GMSGAMEPLAY_VER				= 2,
	GMSMAINSTRUCTURE_VER		= 2,
};

// 这些cmd的类型是CMN::ILogic::cmd_t，现在定义为int
enum
{
	////////////////////////////////////////////////////////////////
	// GMS中的任何模块通知游戏逻辑(GP, GamePlay的缩写)
	////////////////////////////////////////////////////////////////
	GMSPACKET_2GP_0								= 100,
	// (立即执行)配置(这个一般都应该是GMSMainStructure设置GMSGamePlay用的)
	GMSPACKET_2GP_CONFIG						= GMSPACKET_2GP_0 + 1,
	// 玩家发来游戏内容相关指令(MS通知它的)
	GMSPACKET_2GP_PLAYERCMD						= GMSPACKET_2GP_0 + 2,
	// GZS发来的游戏内容相关指令(MS通知它的)
	GMSPACKET_2GP_SVRCMD						= GMSPACKET_2GP_0 + 3,
	// 玩家进入(连接完成)(MS通知它的)
	GMSPACKET_2GP_PLAYERONLINE					= GMSPACKET_2GP_0 + 4,
	// 玩家离开(断线)(MS通知它的)
	// 但是MS会先把该玩家的位置保留，知道上层真正踢出该玩家了，通知MS，MS才能恢复该位置重新被分配（这是累了防止一个人断线而导致有人马上连入失败（因为相应的位置在MP中还没有真正释放）的现象）
	GMSPACKET_2GP_PLAYEROFFLINE					= GMSPACKET_2GP_0 + 5,
	// 某个GZS连上了(MS通知它的)
	GMSPACKET_2GP_GZSONLINE						= GMSPACKET_2GP_0 + 6,
	// 某个GZS断掉了(MS通知它的)
	GMSPACKET_2GP_GZSOFFLINE					= GMSPACKET_2GP_0 + 7,
	// 某个GZS临时中断了
	GMSPACKET_2GP_GZSTEMPOFF					= GMSPACKET_2GP_0 + 8,
	// CAAFS发来的玩家指令
	GMSPACKET_2GP_PLAYERCMD_FROM_CAAFS			= GMSPACKET_2GP_0 + 9,
	// 玩家发来统计信息
	GMSPACKET_2GP_PLAYER_STAT					= GMSPACKET_2GP_0 + 10,

	////////////////////////////////////////////////////////////////
	// GMS中的任何模块通知MAINSTRUCTURE(MS为MAINSTRUCTURE的缩写)
	////////////////////////////////////////////////////////////////
	GMSPACKET_2MS_0								= 120,
	// (立即执行)配置
	GMSPACKET_2MS_CONFIG						= GMSPACKET_2MS_0 + 1,
	// 给玩家发送数据
	// 给: 一个玩家
	GMSPACKET_2MS_CMD2ONEPLAYER					= GMSPACKET_2MS_0 + 2,
	// 给: 一串玩家
	GMSPACKET_2MS_CMD2MULTIPLAYER				= GMSPACKET_2MS_0 + 3,
	// 给: 具有某种标识的玩家
	GMSPACKET_2MS_CMD2PLAYERWITHTAG				= GMSPACKET_2MS_0 + 4,
	// 给: 全部玩家
	GMSPACKET_2MS_CMD2ALLPLAYER					= GMSPACKET_2MS_0 + 5,
	// 设置玩家的标识
	GMSPACKET_2MS_SETPLAYERTAG					= GMSPACKET_2MS_0 + 6,
	// 设置玩家关联的GZS(即玩家需要发送给GZS的指令默认都发给这个GZS)
	GMSPACKET_2MS_SETCLIENTGZSROUTE				= GMSPACKET_2MS_0 + 7,
	// 踢出玩家
	GMSPACKET_2MS_KICKPLAYER					= GMSPACKET_2MS_0 + 8,
	// 发给某个GZS服务器的指令
	GMSPACKET_2MS_GZS_SVRCMD					= GMSPACKET_2MS_0 + 9,
	// 告诉MS某个位置的玩家真正被释放了，MS可以恢复被保留的位置
	GMSPACKET_2MS_UNRESERVEPLAYER				= GMSPACKET_2MS_0 + 10,
	// 发送指令给玩家所在的CLS
	GMSPACKET_2MS_CMD2CLS						= GMSPACKET_2MS_0 + 11,
	// 发送指令给相应的GZS的MainStructure
	GMSPACKET_2MS_RAWCMD2GZSMS					= GMSPACKET_2MS_0 + 12,
	// 把某个用户的ID更改为另一个ID（包括对GZS和CLS的通知）
	GMSPACKET_2MS_CHANGECLIENTID				= GMSPACKET_2MS_0 + 13,
	// 一些控制指令
	GMSPACKET_2MS_CTRL							= GMSPACKET_2MS_0 + 19,
	// 通过MS发给CAAFS的控制指令
	// 参数：GMS_CAAFS_CTRL_T ,GMS_CAAFS_KEEP_QUEUE_POS
	GMSPACKET_2MS_CAAFSCTRL						= GMSPACKET_2MS_0 + 20,
	GMSPACKET_2MS_CAAFSCTRL_QUEUE				= GMSPACKET_2MS_0 + 21,
	
};

// 所有结构的定义都需是n_pngs::CMN::ILogic::CMDIN_TMPL_T的的模板参数
#pragma pack(push, old_pack_num, 1)
////////////////////////////////////////////////////////////////////
struct	GMSPACKET_2GP_CONFIG_T
{
	int				nSubCmd;
	int				nParam;											// 根据subcmd的不同解释成为不同的内容（长度可能超过sizeof(int)）
};
struct	GMSPACKET_2GP_PLAYERCMD_T
{
	int				nClientID;
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2GP_SVRCMD_T
{
	unsigned char	nSvrIdx;										// 说明指令来源于哪个GZS服务器
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2GP_PLAYERONLINE_T
{
	int				nClientID;
	unsigned int	IP;
	unsigned char	nNetworkType;									// 玩家对应的网络类型（NETWORKTYPE_XXX）
};
struct	GMSPACKET_2GP_PLAYEROFFLINE_T
{
	int				nClientID;
	unsigned char	nReason;										// 用户下线的原因CLS_GMS_CLIENT_DROP_T::REMOVEREASON_XXX
};
struct	GMSPACKET_2GP_GZSONLINE_T
{
	unsigned char	nSvrIdx;
	bool			bReconnect;										// 是否是重连来的
};
struct	GMSPACKET_2GP_GZSOFFLINE_T
{
	enum
	{
		MODE_REALLYOFF	= 0,										// 就是下线了
		MODE_STOPPING	= 1,										// 处于停止过程中（所有进入操作应该被拒绝）
	};
	unsigned char	nSvrIdx;
	unsigned char	nMode;

	GMSPACKET_2GP_GZSOFFLINE_T()
	{
		WHMEMSET0THIS();
	}
};
struct	GMSPACKET_2GP_GZSTEMPOFF_T
{
	unsigned char	nSvrIdx;
	bool			bOff;											// 为true就说明是断了，否则就说明恢复了
};
struct	GMSPACKET_2GP_PLAYERCMD_FROM_CAAFS_T
{
	int				nCAAFSIdx;										// 来源CAAFS的序号（因为可能有多个CAAFS）
	int				nClientIDInCAAFS;
	unsigned int	IP;
	size_t			nDSize;
	void			*pData;
};

////////////////////////////////////////////////////////////////////
struct	GMSPACKET_2MS_CONFIG_T
{
	enum
	{
		SUBCMD_REG_SVRCMD			= 1,							// 注册需要缓冲的SVRCMD（即收到GMSPACKET_2MS_GZS_SVRCMD时根据内容把一些指令放入缓冲）
																	// nParam就是相应的GAMDCMD
		SUBCMD_SET_PLAYERNUM		= 2,							// 设置逻辑最大的玩家数（！这个不是instant的）
																	// nParam就是相应的玩家数
	};
	int				nSubCmd;
	int				nParam;											// 根据subcmd的不同解释成为不同的内容（长度可能超过sizeof(int)）
};
struct	GMSPACKET_2MS_CMD2ONEPLAYER_T
{
	int				nClientID;
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_CMD2MULTIPLAYER_T
{
	unsigned char	nChannel;										// 对应的通道
	unsigned short	nClientNum;										// ClientID的数量
	int				*panClientID;									// nClient数组地址
	int				nDSize;
	void			*pData;
	// (多线程才需要)根据用户个数和数据计算包总长度
	// 结构后面先跟着panClientID的真实内容，然后再是用户数据部分
	static int	MT_GetTotalSize(int nCNum, int nDSize)
	{
		return	sizeof(GMSPACKET_2MS_CMD2MULTIPLAYER_T) + nCNum*sizeof(int) + nDSize;
	}
};
struct	GMSPACKET_2MS_CMD2PLAYERWITHTAG_T
{
	unsigned char	nSvrIdx;										// 取值可以是PNGS_SVRIDX_XXX或者是1~15的值
	unsigned char	nChannel;
	unsigned char	nTagIdx;										// nTag的索引值
	short			nTagVal;										// 序号对应的值
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_CMD2ALLPLAYER_T
{
	unsigned char	nSvrIdx;										// 取值可以是PNGS_SVRIDX_XXX或者是1~15的值
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_SETPLAYERTAG_T
{
	int				nClientID;										// 玩家ID
	unsigned char	nTagIdx;										// nTag的索引值
	short			nTagVal;										// 序号对应的值
};
struct	GMSPACKET_2MS_SETCLIENTGZSROUTE_T
{
	int				nClientID;										// 玩家ID
	unsigned char	nSvrIdx;										// 关联的目标GZS的序号
};
struct	GMSPACKET_2MS_KICKPLAYER_T
{
	int				nClientID;
	pngs_cmd_t		nKickSubCmd;									// 取值为：SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_XXX。比如：是否告诉CLS返回用户DROP的结果
	GMSPACKET_2MS_KICKPLAYER_T()
		: nClientID(0)
		, nKickSubCmd(SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID)
	{}
};
struct	GMSPACKET_2MS_GZS_SVRCMD_T
{
	unsigned char	nSvrIdx;										// 目标GZS的序号
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_UNRESERVEPLAYER_T
{
	int				nClientID;										// 玩家旧在线ID，可以有它得到的序号，如果对应对象还存在则相当于Kick
};
struct	GMSPACKET_2MS_CMD2CLS_T
{
	int				nClientID;										// 玩家在线ID（根据这个找到对应的CLS）
	size_t			nDSize;
	void			*pData;											// 发给CLS的指令
};
struct	GMSPACKET_2MS_RAWCMD2GZSMS_T
{
	unsigned char	nSvrIdx;										// 目标GZS的序号
	size_t			nDSize;
	void			*pData;
};
struct	GMSPACKET_2MS_CHANGECLIENTID_T
{
	int				nFromID;
	int				nToID;
	unsigned char	nSvrIdx;										// 需要CLS中给玩家关联的gzs序号
};
struct	GMSPACKET_2MS_CTRL_T
{
	enum
	{
		SUBCMD_CLOSE_CONNECTER	= 0x01,								// 关闭某个连接的socket。参数nParam就是对应的connecterID
		SUBCMD_CLOSEALLCAAFS	= 0x02,								// 关闭所有的CAAFS
		SUBCMD_SETCLSMAXPLAYER	= 0x03,								// 设置某个CLS的最多用户数量。参数nParam是CLSID，nParam1是用户数量
		SUBCMD_PLAYER_STAT_ON	= 0x04,								// 让玩家的统计开始，nParam就是nClientID，nParam1是发送间隔（秒为单位）
		SUBCMD_PLAYER_STAT_OFF	= 0x05,								// 让玩家的统计结束，nParam就是nClientID
	};
	unsigned char	nSubCmd;
	int				nParam;
	int				nParam1;
};
////////////////////////////////////////////////////////////////////
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_GMS_LOGIC_H__
