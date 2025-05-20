// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_gzs_logic.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS内各个模块间的指令包公共定义
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-09-01
// Change LOG   : 2005-09-30 取消了GZSPACKET_2GP_PLAYERIN、GZSPACKET_2GP_PLAYEROUT、GOPLAYER，这些要放到逻辑层中去写。

#ifndef	__PNGS_PACKET_GZS_LOGIC_H__
#define	__PNGS_PACKET_GZS_LOGIC_H__

#include <WHCMN/inc/whcmn_def.h>
using namespace n_whcmn;

namespace n_pngs
{

enum
{
	GZSGAMEPLAY_VER				= 2,
	GZSMAINSTRUCTURE_VER		= 2,
};

// 这些cmd的类型是CMN::ILogic::cmd_t，参见：pngs_cmn.h(现在定义为int)
enum
{
	////////////////////////////////////////////////////////////////
	// GZS任何模块通知游戏逻辑(GP, GamePlay的缩写)
	////////////////////////////////////////////////////////////////
	// 定义范围：200~219
	GZSPACKET_2GP_0							= 200,
	// (立即执行)配置(这个一般都应该是GZSMainStructure设置GZSGamePlay用的)
	GZSPACKET_2GP_CONFIG					= GZSPACKET_2GP_0 + 1,
	// 玩家发来游戏内容相关指令(MS通知它的)
	GZSPACKET_2GP_PLAYERCMD					= GZSPACKET_2GP_0 + 2,
	// 其他服务器发来的游戏内容相关指令(MS通知它的)
	GZSPACKET_2GP_SVRCMD					= GZSPACKET_2GP_0 + 3,

	////////////////////////////////////////////////////////////////
	// GZS任何模块通知MAINSTRUCTURE(MS为MAINSTRUCTURE的缩写)
	////////////////////////////////////////////////////////////////
	// 定义范围：220~239，共20个
	GZSPACKET_2MS_0							= 220,
	// (立即执行)配置（也可以在运行成功中执行相应的设置）
	GZSPACKET_2MS_CONFIG					= GZSPACKET_2MS_0 + 1,
	// 给玩家发送数据
	// 给: 一个玩家
	GZSPACKET_2MS_CMD2ONEPLAYER				= GZSPACKET_2MS_0 + 2,
	// 给: 一串玩家
	GZSPACKET_2MS_CMD2MULTIPLAYER			= GZSPACKET_2MS_0 + 3,
	// 给: 具有某种标识的玩家
	GZSPACKET_2MS_CMD2PLAYERWITHTAG			= GZSPACKET_2MS_0 + 4,
	// 给: 全部玩家
	GZSPACKET_2MS_CMD2ALLPLAYER				= GZSPACKET_2MS_0 + 5,
	// 设置玩家的标识
	GZSPACKET_2MS_SETPLAYERTAG				= GZSPACKET_2MS_0 + 6,
	// 踢出玩家
	GZSPACKET_2MS_KICKPLAYER				= GZSPACKET_2MS_0 + 7,
	// 发给某个GZS服务器(或者GMS)的指令
	GZSPACKET_2MS_SVRCMD					= GZSPACKET_2MS_0 + 8,
	// 指令打包的操作（不停的向包中添加数据，直到下一条发送指令（数据长度为0），具体的发送方式和channel是有最后的发送指令决定的）
	GZSPACKET_2MS_CMD_PACK_ADD				= GZSPACKET_2MS_0 + 9,
	// 打包开始（为了逻辑那边改动最小）
	GZSPACKET_2MS_CMD_PACK_BEGIN			= GZSPACKET_2MS_0 + 10,
	// 设置玩家的64位标识
	GZSPACKET_2MS_SETPLAYERTAG64			= GZSPACKET_2MS_0 + 11,
	// 给 具有某种64位标识的玩家发送信息
	GZSPACKET_2MS_CMD2PLAYERWITHTAG64		= GZSPACKET_2MS_0 + 12,
};

// 所有结构的定义都需是n_pngs::CMN::ILogic::CMDIN_TMPL_T的的模板参数
#pragma pack(push, old_pack_num, 1)
////////////////////////////////////////////////////////////////////
struct	GZSPACKET_2GP_CONFIG_T
{
	enum
	{
		SUBCMD_PLAYERNUMMAX			= 1,							// 设置最多的Player数量
																	// nParam就是数量
		SUBCMD_SVRIDX				= 2,							// 设置SvrIdx
																	// nParam就是SvrIdx
	};
	int				nSubCmd;
	int				nParam;											// 根据subcmd的不同解释成为不同的内容（长度可能超过sizeof(int)）
};
struct	GZSPACKET_2GP_PLAYERCMD_T
{
	int				nClientID;
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GZSPACKET_2GP_SVRCMD_T
{
	unsigned char	nFromSvrIdx;									// 表示来自那个服务器。PNGS_SVRIDX_GMS表示GMS、1~GZS_MAX_NUM表示GZS
	size_t			nDSize;
	void			*pData;
};
////////////////////////////////////////////////////////////////////
// 这些指令发送结构和GMS中的是一样的
struct	GZSPACKET_2MS_CONFIG_T
{
	enum
	{
		SUBCMD_GET_SVRIDX				= 1,						// 要获取GZS_MS的服务器需要，因为这个GZSGamePlay也需要
																	// 返回的结果存放在nParam中
		SUBCMD_GET_CONNECTEDTOGMSPTR	= 2,						// 获得是否和GMS连接上的bool变量的指针
																	// nParam中是相应的指针
	};
	int				nSubCmd;
	int				nParam;											// 根据subcmd的不同解释成为不同的内容（长度可能超过sizeof(int)）
};
struct	GZSPACKET_2MS_CMD2ONEPLAYER_T
{
	int				nClientID;
	unsigned char	nChannel;
	size_t			nDSize;
	void			*pData;
};
struct	GZSPACKET_2MS_CMD2MULTIPLAYER_T
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
		return	sizeof(GZSPACKET_2MS_CMD2MULTIPLAYER_T) + nCNum*sizeof(int) + nDSize;
	}
};
struct	GZSPACKET_2MS_CMD2PLAYERWITHTAG_T
{
	unsigned char	nSvrIdx;										// 取值可以是PNGS_SVRIDX_XXX或者是1~15的值
	unsigned char	nChannel;
	unsigned char	nTagIdx;										// nTag的索引值
	short			nTagVal;										// 序号对应的值
	size_t			nDSize;
	void			*pData;
};
struct	GZSPACKET_2MS_CMD2ALLPLAYER_T
{
	unsigned char	nChannel;
	unsigned char	nSvrIdx;										// 对应的服务器序号（定义为PNGS_SVRIDX_XXX，pngs_def.h中）
	size_t			nDSize;
	void			*pData;

	GZSPACKET_2MS_CMD2ALLPLAYER_T()
		: nSvrIdx(PNGS_SVRIDX_ALLGZS)
	{}
};
struct	GZSPACKET_2MS_SETPLAYERTAG_T
{
	int				nClientID;										// 玩家ID
	unsigned char	nTagIdx;										// nTag的索引值
	short			nTagVal;										// 序号对应的值
};
struct	GZSPACKET_2MS_KICKPLAYER_T
{
	enum
	{
		CMD_KICK_END	= 0,										// 踢出用户立即下线
		CMD_KICK_DELAY	= 1,										// 踢出用户但是需要有指定延迟后再下线
	};
	int				nClientID;
	unsigned char	nKickCmd;
	GZSPACKET_2MS_KICKPLAYER_T()
		: nClientID(0)
		, nKickCmd(CMD_KICK_END)
	{
	}
};
struct	GZSPACKET_2MS_SVRCMD_T
{
	int				nSvrIdx;										// 目标GZS的序号，可能是PNGS_SVRIDX_XXX
	size_t			nDSize;
	void			*pData;
};
struct GZSPACKET_2MS_CMD_PACK_ADD_T
{
	size_t			nDSize;
	void			*pData;
};
struct	GZSPACKET_2MS_CMD_PACK_BEGIN_T
{
	int				nSendCmd;										// 对应的Send指令类型GZSPACKET_2MS_CMD2XXX
																	// 如果是GZSPACKET_2MS_0则表示没有打包
};
struct	GZSPACKET_2MS_SETPLAYERTAG64_T
{
	int				nClientID;										// 玩家ID
	whuint64		nTag;											// 对应的标记
	bool			bDel;
};
struct	GZSPACKET_2MS_CMD2PLAYERWITHTAG64_T
{
	unsigned char	nSvrIdx;										// 取值可以是PNGS_SVRIDX_XXX或者是1~15的值
	unsigned char	nChannel;
	whuint64		nTag;											// 对应的标记
	size_t			nDSize;
	void			*pData;
};

////////////////////////////////////////////////////////////////////
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_GZS_LOGIC_H__
