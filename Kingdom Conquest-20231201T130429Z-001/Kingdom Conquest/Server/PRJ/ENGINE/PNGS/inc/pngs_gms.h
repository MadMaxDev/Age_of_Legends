// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块(GMS内部实现需要的所有Logic模块)
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
//                GMS的实现主要负责底层的网络连接和对各个ILogic对象的消息分派
// Logic命名    : 1. "uGMS_MAINSTRUCTURE"
//                2. "uGMS_GAMEPLAY"
// CreationDate : 2005-07-23
// Change LOG   :

#ifndef	__PNGS_GMS_H__
#define	__PNGS_GMS_H__

#include "./pngs_cmn.h"

namespace n_pngs
{

class	GMS
{
public:
	static CMN::ILogic *	Create(const char *szLogicType);
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_GMS_H__
