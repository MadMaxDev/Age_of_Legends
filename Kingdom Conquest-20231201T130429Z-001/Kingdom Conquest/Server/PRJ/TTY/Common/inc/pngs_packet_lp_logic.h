#ifndef __pngs_packet_lp_logic_H__
#define __pngs_packet_lp_logic_H__

#include "WHCMN/inc/whcmn_def.h"

using namespace n_whcmn;

namespace n_pngs
{
enum
{
	MAIL_FLAG_NORMAL		= 1,		// 普通邮件
	MAIL_FLAG_FIGHT			= 2,		// 附加信息为战斗
	MAIL_FLAG_REWARD		= 3,		// 奖励，附件为奖励
	MAIL_FLAG_COMBAT		= 4,		// 新的战斗模式(联盟副本)
};
enum
{
	MAIL_TYPE_PRIVATE		= 0x01,		// 私人邮件
	MAIL_TYPE_ALLIANCE		= 0x02,		// 联盟邮件
	MAIL_TYPE_SYSTEM		= 0x04,		// 系统邮件
	MAIL_TYPE_ALLIANCE_EVENT	= 0x08,		// 联盟事件
	MAIL_TYPE_GOLDORE_EVENT		= 0x10,		// 金矿事件,发送者是偷金者或者浇水者
};
enum
{
	mail_reward_type_none				= 0,			// 无奖励
	mail_reward_type_item				= 1,			// 道具
	mail_reward_type_diamond			= 2,			// 钻石
	mail_reward_type_crystal			= 3,			// 水晶
	mail_reward_type_gold				= 4,			// 黄金
};
// cmd的类型都是int
// 很多指令是web中不需要的,先预留
enum
{
	//////////////////////////////////////////////////////////////////////////
	// LP中的任何模块通知游戏逻辑(GP,GamePlay的缩写)
	//////////////////////////////////////////////////////////////////////////
	LPPACKET_2GP_0									= 100,
	// (立即执行)配置(这个一般都应该是LPMainStructure设置LPGamePlay用的)(废弃)
	LPPACKET_2GP_CONFIG								= LPPACKET_2GP_0 + 1,
	// 玩家发来游戏内容相关指令(MS通知它的)
	LPPACKET_2GP_PLAYERCMD							= LPPACKET_2GP_0 + 2,
	// 玩家进入(连接完成)(MS通知它的)
	LPPACKET_2GP_PLAYERONLINE						= LPPACKET_2GP_0 + 3,
	// 玩家离开(断线)(MS通知它的)
	LPPACKET_2GP_PLAYEROFFLINE						= LPPACKET_2GP_0 + 4,
	// CAAFS4Web发来的玩家指令
	LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web			= LPPACKET_2GP_0 + 5,
	// 玩家发来的统计信息
	LPPACKET_2GP_PLAYER_STAT						= LPPACKET_2GP_0 + 6,

	//////////////////////////////////////////////////////////////////////////
	// LP中任何模块通知MainStructure(MS为MainStructure的缩写)
	//////////////////////////////////////////////////////////////////////////
	LPPACKET_2MS_0									= 120,
	// (立即执行)配置
	LPPACKET_2MS_CONFIG								= LPPACKET_2MS_0 + 1,
	// 给玩家发送数据
	// to:一个玩家
	LPPACKET_2MS_CMD2ONEPLAYER						= LPPACKET_2MS_0 + 2,
	// to:一串玩家
	LPPACKET_2MS_CMD2MULTIPLAYER					= LPPACKET_2MS_0 + 3,
	// to:具有某种标识的玩家
	LPPACKET_2MS_CMD2PLAYERWITHTAG					= LPPACKET_2MS_0 + 4,
	// to:全部玩家
	LPPACKET_2MS_CMD2ALLPLAYER						= LPPACKET_2MS_0 + 5,
	// 设置玩家的标识
	LPPACKET_2MS_SETPLAYERTAG						= LPPACKET_2MS_0 + 6,
	// 踢出玩家
	LPPACKET_2MS_KICKPLAYER							= LPPACKET_2MS_0 + 8,
	// 告诉MS某个位置的玩家真正被释放了,MS可以恢复被保留的位置
	LPPACKET_2MS_UNRESERVEPLAYER					= LPPACKET_2MS_0 + 9,
	// 发送指令给玩家所在的CLS4Web
	LPPACKET_2MS_CMD2CLS4Web						= LPPACKET_2MS_0 + 10,
	// 把某个用户的ID更改为另一个ID(包括对CLS4Web的通知)
	LPPACKET_2MS_CHANGECLIENTID						= LPPACKET_2MS_0 + 11,
	// 给具有某种64位标识的玩家发送指令
	LPPACKET_2MS_CMD2PLAYERWITHTAG64				= LPPACKET_2MS_0 + 12,
	// 设置玩家的64位标识
	LPPACKET_2MS_SETPLAYERTAG64						= LPPACKET_2MS_0 + 13,
	// 一些控制指令
	LPPACKET_2MS_CTRL								= LPPACKET_2MS_0 + 14,
	// 通过MS发给CAAFS4Web的控制指令
	// 参数:LP_CAAFS4Web_CTRL_T
	LPPACKET_2MS_CAAFS4WebCTRL						= LPPACKET_2MS_0 + 15,
};

#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
// 2GP
//////////////////////////////////////////////////////////////////////////
struct LPPACKET_2GP_PLAYERCMD_T 
{
	int					nClientID;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web_T
{
	int					nCAAFS4WebIdx;										// 来源CAAFS4Web的序号(可能有多个CAAFS4Web)
	int					nClientIDInCAAFS4Web;
	unsigned int		IP;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2GP_PLAYERONLINE_T 
{
	int					nClientID;
	unsigned int		IP;
	int					nTermType;											// 终端类型
};
struct LPPACKET_2GP_PLAYEROFFLINE_T 
{
	int					nClientID;
	unsigned char		nReason;											// 用户下线原因CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_XXX
};
//////////////////////////////////////////////////////////////////////////
// 2MS
//////////////////////////////////////////////////////////////////////////
// 废弃的指令
struct LPPACKET_2MS_CONFIG_T 
{
	enum
	{
		SUBCMD_REG_SVRCMD				= 1,								// 注册需要缓冲的svrcmd
		SUBCMD_SET_PLAYERNUM			= 2,								// 设置逻辑最大的玩家数(这个不是instant的!!!)
	};
	int					nSubCmd;
	int					nParam;
};
struct LPPACKET_2MS_CMD2CLS4Web_T 
{
	int					nClientID;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_KICKPLAYER_T 
{
	int					nClientID;
	unsigned char		nKickSubCmd;										// SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_XXX
};
struct LPPACKET_2MS_CMD2ONEPLAYER_T 
{
	int					nClientID;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_CMD2MULTIPLAYER_T 
{
	unsigned short		nClientNum;
	int*				paClientID;
	int					nDSize;
	void*				pData;
};
struct LPPACKET_2MS_CMD2PLAYERWITHTAG_T 
{
	unsigned char		nTagIdx;
	short				nTagVal;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_CMD2ALLPLAYER_T 
{
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_SETPLAYERTAG_T 
{
	int					nClientID;
	unsigned char		nTagIdx;
	short				nTagVal;
};
struct LPPACKET_2MS_CMD2PLAYERWITHTAG64_T 
{
	whuint64			nTag;
	size_t				nDSize;
	void*				pData;
};
struct LPPACKET_2MS_SETPLAYERTAG64_T 
{
	int					nClientID;
	whuint64			nTag;
	bool				bDel;
};
#pragma pack()
}

#endif
