// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_DealTE.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS模块内部的GZS_MAINSTRUCTURE模块中时间处理的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GZS是Game Zone Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-09-21
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// 看看时间队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_TQ.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// 这里面可以申请别的时间事件，反正和这个不干扰的。
		// 这个get出来的只能通过Del删除，不能自己删
		m_TQ.Del(id);
	}
	return	0;
}

void	GZS_MAINSTRUCTURE::TEDeal_Hello_TimeOut(TQUNIT_T *pTQUnit)
{
	// 在指定时间没有收到对方发来的Hello包
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(675,PNGS_RT)"GZS_MAINSTRUCTURE::TEDeal_Hello_TimeOut,,0x%X", pTQUnit->un.svr.nID);
	// 删除相应的Connecter
	RemoveMYCNTR(pTQUnit->un.svr.nID);
}
