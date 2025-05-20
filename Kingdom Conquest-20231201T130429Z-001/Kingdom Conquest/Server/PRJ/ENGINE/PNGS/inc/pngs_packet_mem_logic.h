// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_mem_logic.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的内存管理模块和其他模块的交互定义
//                这个和游戏内容相关，所以放在Common/inc中
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2007-09-26
// Change LOG   : 

#ifndef	__PNGS_PACKET_MEM_LOGIC_H__
#define	__PNGS_PACKET_MEM_LOGIC_H__

namespace n_pngs
{

// 模块版本
enum
{
	GSMEM_VER		= 1,
};

// MEM模块收到的指令
// 定义范围：1400~1499
enum
{
	PNGSPACKET_2MEM_0							= 1400,
	////////////////////////////////////////////////////////////////
	// 公共的
	////////////////////////////////////////////////////////////////
	// (立即执行)配置(如设置所有指令都应该返回给那个ILogic)
	PNGSPACKET_2MEM_CONFIG						= PNGSPACKET_2MEM_0 + 1,
	// (立即执行)获取指针
	// 数据为PNGSPACKET_2MEM_XXX_PTR_T
	PNGSPACKET_2MEM_GET_PTR						= PNGSPACKET_2MEM_0 + 12,
	// (立即执行)设置指针
	// 数据为PNGSPACKET_2MEM_XXX_PTR_T
	PNGSPACKET_2MEM_SET_PTR						= PNGSPACKET_2MEM_0 + 13,
};

#pragma pack(push, old_pack_num, 1)
struct	PNGSPACKET_2MEM_CONFIG_T
{
	enum
	{
		SUBCMD_GET_MEMSTATUS					= 1,				// 获得内存状态（由nParam返回，0是正常初始化、1继承的旧内存）
	};
	int				nSubCmd;
	int				nParam;											// 根据subcmd的不同解释成为不同的内容
	int				nParam1;										// 根据subcmd的不同解释成为不同的内容
};
struct	PNGSPACKET_2MEM_XXX_PTR_T
{
	const char		*pcszKey;										// 指针名字如果以"::"开头，则表明是内部指针（这样的指针只能get不能set）
																	// "::am"	表示是分配器指针
																	// 其他的则是各个模块自己需要的指针，一个模块最好只有一个指针
	void			*ptr;
};
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_MEM_LOGIC_H__
