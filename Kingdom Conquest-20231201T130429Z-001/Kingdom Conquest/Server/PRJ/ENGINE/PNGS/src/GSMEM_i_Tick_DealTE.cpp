// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : GSMEM_i_Tick_DealTE.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CAAFS中Tick_DealTE部分的具体实现，和各个TEDeal_XXX函数
// CreationDate : 2007-09-29
// Change LOG   :

#include "../inc/GSMEM_i.h"

using namespace n_pngs;

int		GSMEM::Tick_DealTE()
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

void	GSMEM::TEDeal_OutputStat(TQUNIT_T * pTQUnit)
{
	// 输出统计
	AO_T::STAT_T	s;
	m_aoone->GetStat(&s);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(839,GSMEM_RT)"GSMEM stat,%d,%d/%d", s.nSizeNum, s.nOffset, m_aoone->GetChunkSize());
	if( m_cfginfo.nMinMemWarn )
	{
		if( (int)(m_aoone->GetChunkSize()-s.nOffset) <= m_cfginfo.nMinMemWarn )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1050,GSMEM_RT)"SHM NOT ENOUGH! Server SHOULD STOP!");
			CmdOutToLogic_AUTO(m_pCMN, CMN::CMD2CMN_SHOULDSTOP, NULL, 0);
		}
	}
		
	// 注册下一个事件
	SetTE_OutputStat();
}
void	GSMEM::SetTE_OutputStat()
{
	whtimequeue::ID_T	id;
	TQUNIT_T	*pNextTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nOutputStatInterval, (void **)&pNextTQUnit, &id)<0 )
	{
		// 怎么会无法加入时间事件涅？
		assert(0);
		return;
	}
	pNextTQUnit->tefunc				= &GSMEM::TEDeal_OutputStat;
}
