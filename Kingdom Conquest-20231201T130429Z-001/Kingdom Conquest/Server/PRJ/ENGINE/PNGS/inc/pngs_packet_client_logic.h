// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_client_logic.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的Client内各个模块间的指令包公共定义
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2006-11-28
// Change LOG   : 
//              : 

#ifndef	__PNGS_PACKET_CLIENT_LOGIC_H__
#define __PNGS_PACKET_CLIENT_LOGIC_H__

namespace n_pngs
{

// 模块版本
enum
{
	PNGSCLIENT_VER				= 1,
};

// 这些cmd的类型是CMN::ILogic::cmd_t，现在定义为int
enum
{
	////////////////////////////////////////////
	// 别的模块发给Client模块的
	// 100~199
	////////////////////////////////////////////
	CLIENTPACKET_2CLIENT_0						= 100,
	// 发送指令给服务器（别的模块通过这个来给服务器发送指令）
	CLIENTPACKET_2CLIENT_SENDCMD2SVR			= CLIENTPACKET_2CLIENT_0 + 1,

	////////////////////////////////////////////
	// Client模块发给别的模块的
	// 100~199
	////////////////////////////////////////////
	CLIENTPACKET_2OTHER_0						= 100,
	// 把收到的指令分派给模块（模块必须是注册了这条指令的才能处理这条指令）
	CLIENTPACKET_2OTHER_DISPATCHCMD				= CLIENTPACKET_2OTHER_0 + 1,
};

#pragma pack(push, old_pack_num, 1)
struct	CLIENTPACKET_2CLIENT_SENDCMD2SVR_T
{
	int				nChannel;					// 通道号
	int				nSvrIdx;					// 目标服务器ID（和PNGSClient_I::Send中的nSvrIdx定义一致，PNGS_SVRIDX_XXX，定义在pngs_def.h中）
	size_t			nDSize;						// 要发出的数据
	void			*pData;
};
struct	CLIENTPACKET_2OTHER_DISPATCHCMD_T
{
	int				nChannel;					// 通道号
	size_t			nDSize;						// 得到的数据
	const void		*pData;
};
#pragma pack(pop, old_pack_num)


}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_CLIENT_LOGIC_H__
