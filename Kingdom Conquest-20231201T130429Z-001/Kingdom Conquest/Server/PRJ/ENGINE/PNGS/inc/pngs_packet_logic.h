// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_packet_logic.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的模块间的指令包公共定义(保留0~19，别的Logic不能占用这些)
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-09-20
// Change LOG   :

#ifndef	__PNGS_PACKET_LOGIC_H__
#define	__PNGS_PACKET_LOGIC_H__

namespace n_pngs
{

// 这些cmd的类型是CMN::ILogic::cmd_t，现在定义为int
// 下面的为通用的定义，范围为：0~100
// 其他logic定义不会使用这些值
enum
{
	// (立即执行)通知它在标准输出输出版本列表
	PNGSPACKET_2LOGIC_DISPHISTORY	= 0,
	// 版本校验（放弃，直接在获取模块时校验）
	// PNGSPACKET_2LOGIC_VERIFY_VER	= 1,
	// 通用服务器框架向ILogic设置MOOD
	PNGSPACKET_2LOGIC_SETMOOD		= 2,
	// 分发指令（一般用于mainstructure给其他logic分派）
	// 数据部分就是指令体
	PNGSPACKET_2LOGIC_DISPATCHCMD	= 3,
	// 向mainstructure注册指令。pRstAccepter就是可以处理指令的逻辑模块。
	PNGSPACKET_2LOGIC_REGCMDDEAL	= 4,
	// GM指令（注意：PNGSPACKET_2LOGIC_GMCMD和PNGSPACKET_2LOGIC_GMCMD_RST都使用这个结构）
	PNGSPACKET_2LOGIC_GMCMD			= 10,
	// GM指令返回（对于纯管发送的模块收到这个就发送给相应的连接）
	PNGSPACKET_2LOGIC_GMCMD_RST		= 11,
	// 通知
	PNGSPACKET_2LOGIC_NOTIFY		= 12,
};
#pragma pack(push, old_pack_num, 1)
////////////////////////////////////////////////////////////////////
struct	PNGSPACKET_2LOGIC_DISPHISTORY_T
{
	char			szVerToQuery[16];								// 用于查询的版本号
};
//struct	PNGSPACKET_2LOGIC_VERIFY_VER_T
//{
//	unsigned int	nVerExpected;									// 期望的版本
//	unsigned int	nVerReal;										// 实际的版本
//	bool			bOK;											// 是否校验成功
//	PNGSPACKET_2LOGIC_VERIFY_VER_T()
//		: nVerExpected(0)
//		, nVerReal(0)
//		, bOK(true)													// 如果不需要校验版本这个就不会变化，就直接返回成功了
//	{
//	}
//	inline void	check(int _nVerReal)
//	{
//		nVerReal	= _nVerReal;
//		bOK			= (nVerExpected==nVerReal);
//	}
//};
struct	PNGSPACKET_2LOGIC_REGCMDDEAL_T
{
	pngs_cmd_t		nCmdReg;
};
struct	PNGSPACKET_2LOGIC_GMCMD_T									// 注意PNGSPACKET_2LOGIC_GMCMD和PNGSPACKET_2LOGIC_GMCMD_RST都使用这个结构
{
	int				nCntrID;										// 对应的连接ID（原样返回）
	int				nExt;											// 原样返回
	size_t			nDSize;
	void			*pData;											// 逻辑模块自行解释pData的内容
																	// 一般pData的最前面pngs_cmd_t是GM指令类型，默认0表示字串，其他的值由各个模块自己定义
};
typedef	PNGSPACKET_2LOGIC_GMCMD_T	PNGSPACKET_2LOGIC_GMCMD_RST_T;
struct	PNGSPACKET_2LOGIC_SETMOOD_T
{
	int				nMood;											// CMN::ILogic::MOOD_XXX
};
struct	PNGSPACKET_2LOGIC_NOTIFY_T
{
	int				nWhat;											// 通知的内容是什么（这个需要通知者和被通知者在一个共享的头文件中定义好）
	size_t			nDSize;
	void			*pData;
};
////////////////////////////////////////////////////////////////////
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_PACKET_LOGIC_H__
