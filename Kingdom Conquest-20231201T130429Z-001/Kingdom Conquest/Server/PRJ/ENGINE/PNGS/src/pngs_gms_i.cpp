// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的各个Logic模块的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-08-01
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

CMN::ILogic *	GMS::Create(const char *szLogicType)
{
	if( stricmp(szLogicType, "uGMS_MAINSTRUCTURE" )==0 )
	{
		return	new	GMS_MAINSTRUCTURE;
	}
	return	NULL;
}

