// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_RemovePlayerUnit.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS模块内部的GZS_MAINSTRUCTURE模块的RemovePlayerUnit函数功能实现
//                PNGS是Pixel Network Game Structure的缩写
//                GZS是Game Zone Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-09-22
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gzs_logic.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::RemovePlayerUnit(int nID)
{
	return	RemovePlayerUnit( m_Players.getptr(nID) );
}
int		GZS_MAINSTRUCTURE::RemovePlayerUnit(PlayerUnit *pPlayer)
{
	if( !pPlayer )
	{
		return	-1;
	}

	// 这里不会通知GamePlay，这里只是网络关系的移除。逻辑关系是需要GamePlay之间进行通知的。

	// 移除玩家
	pPlayer->clear();
	m_Players.FreeUnitByPtr(pPlayer);

	return	0;
}
