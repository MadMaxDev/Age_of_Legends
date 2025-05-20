// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS模块
//                PNGS是Pixel Network Game Structure的缩写
//                GZS是Game Zone Server的缩写，是逻辑服务器中的总控服务器
//                GZS的实现主要负责底层的网络连接和对各个ILogic对象的消息分派
// CreationDate : 2005-07-25
// Change LOG   :

#ifndef	__PNGS_GZS_H__
#define	__PNGS_GZS_H__

#include "./pngs_cmn.h"

namespace n_pngs
{

class	GZS
{
public:
	static CMN::ILogic *	Create(const char *szLogicType);
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_GZS_H__
