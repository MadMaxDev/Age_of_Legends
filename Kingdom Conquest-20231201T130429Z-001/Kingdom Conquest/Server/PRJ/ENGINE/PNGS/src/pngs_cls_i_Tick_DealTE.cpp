// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick_DealTE.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CLS中Tick_DealTE部分的具体实现，和各个TEDeal_XXX函数
// CreationDate : 2005-08-15
// Change LOG   :

#include "../inc/pngs_cls_i.h"

using namespace n_pngs;

int		CLS2_I::Tick_DealTE()
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

void	CLS2_I::TEDeal_nClientConnectToMeTimeOut(TQUNIT_T *pTQUnit)
{
	// 删除玩家(删除函数内部会通知GMS的)
	RemovePlayerUnit(pTQUnit->un.player.nID);
}
void	CLS2_I::SetTEDeal_nClientConnectToMeTimeOut(PlayerUnit *pPlayer)
{
	TQUNIT_T	*pTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nClientConnectToMeTimeOut, (void **)&pTQUnit, &pPlayer->teid)<0 )
	{
		// 怎么会无法加入时间事件涅？
		assert(0);
		// 移除用户(这里面会通知GMS连接失败)
		RemovePlayerUnit(pPlayer);
		return;
	}
	pTQUnit->tefunc			= &CLS2_I::TEDeal_nClientConnectToMeTimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	CLS2_I::TEDeal_nCheckRecvInterval(TQUNIT_T *pTQUnit)
{
	// 获取player对象
	PlayerUnit	*pPlayer	= m_Players.getptr(pTQUnit->un.player.nID);
	if( pPlayer )
	{
		if( pPlayer->nRecvCount>m_cfginfo.nMaxRecvCount || pPlayer->nRecvSize>m_cfginfo.nMaxRecvSize )
		{
			// 踢用户下线
			GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(1089,PNGS_RT)"0x%X,%s,%d,%d", pPlayer->nID, pPlayer->szName, pPlayer->nRecvCount, pPlayer->nRecvSize);
			pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
			RemovePlayerUnit(pPlayer);
		}
		else
		{
			// 清空，重新统计
			pPlayer->nRecvCount	= 0;
			pPlayer->nRecvSize	= 0;
			SetTEDeal_nCheckRecvInterval(pPlayer);
		}
	}
}
void	CLS2_I::SetTEDeal_nCheckRecvInterval(PlayerUnit *pPlayer)
{
	TQUNIT_T	*pTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCheckRecvInterval, (void **)&pTQUnit, &pPlayer->teid)<0 )
	{
		// 怎么会无法加入时间事件涅？
		assert(0);
		// 移除用户(这里面会通知GMS连接失败)
		RemovePlayerUnit(pPlayer);
		return;
	}
	pTQUnit->tefunc			= &CLS2_I::TEDeal_nCheckRecvInterval;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
