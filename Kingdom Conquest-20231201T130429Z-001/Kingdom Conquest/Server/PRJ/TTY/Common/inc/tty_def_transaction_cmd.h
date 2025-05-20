// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : tty_def_transaction_cmd.h
// Creator      : Wei Hua (魏华)
// Comment      : 所有transaction对象需要调用的指令定义
// CreationDate : 2007-06-21
// ChangeLog    :

#ifndef	__TTY_DEF_TRANSACTION_CMD_H__
#define	__TTY_DEF_TRANSACTION_CMD_H__

#include "tty_common_def.h"

namespace n_pngs
{

////////////////////////////////////////////////////////////
enum
{
	TTY_TC_0							= 0,
	TTY_TC_INIT							= TTY_TC_0 + 0,				// Transaction对象初始化。参数为各个transaction自己定义的
	TTY_TC_P_LBA_I_RPL_LOGIN_VERIFY		= TTY_TC_0 + 1,				// LBA的LOGIN_VERIFY返回。参数为P_LBA_I_RPL_LOGIN_VERIFY_T
	TTY_TC_P_LBA_I_RPL_LOGIN			= TTY_TC_0 + 2,				// LBA的LOGIN返回。参数为P_LBA_I_RPL_LOGIN_T
	TTY_TC_P_LBA_I_RPL_LOGOUT			= TTY_TC_0 + 3,				// LBA的LOGOUT返回。参数为P_LBA_I_RPL_LOGOUT_T
	TTY_TC_P_LBA_I_RPL_STARTGAME		= TTY_TC_0 + 4,				// LBA的STARTGAME返回。参数为P_LBA_I_RPL_STARTGAME_T
	TTY_TC_PLAYER_DROP					= TTY_TC_0 + 5,				// 在Transaction过程中用户断线了。没有参数。
	TTY_TC_PLAYER_NORESPONSE			= TTY_TC_0 + 6,				// 在Transaction过程中用户没有响应。
	TTY_TC_PLAYER_VERIFYCODE_CHECK		= TTY_TC_0 + 7,				// 用户发来verifycode的结果
	TTY_TC_PLAYER_VERIFYCODE_CHANGE		= TTY_TC_0 + 8,				// 用户刷新验证码
	TTY_TC_PLAYER_VERIFYCODE_CANCEL		= TTY_TC_0 + 9,				// 用户取消验证
	TTY_TC_PLAYER_STARTGAME				= TTY_TC_0 + 10,			// 用户开始游戏
};
// 指令数据结构
#pragma pack(push, old_pack_num, 1)
////////////////////////////////////////////////////////////
// 请求指令
////////////////////////////////////////////////////////////
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __TTY_DEF_TRANSACTION_CMD_H__
