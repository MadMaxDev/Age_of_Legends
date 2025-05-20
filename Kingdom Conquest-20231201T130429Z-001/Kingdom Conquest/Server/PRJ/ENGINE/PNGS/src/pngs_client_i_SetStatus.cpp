// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_client_i_SetStatus.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的Client模块内部实现的SetStatus函数部分
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-08-25
// Change LOG   :

#include "../inc/pngs_client_i.h"

using namespace n_pngs;

void	PNGSClient_I::SetStatus(int nStatus)
{
	switch( nStatus )
	{
		case	STATUS_CONNECTINGCAAFS:
			m_pTickControlOut	= &PNGSClient_I::Tick_ControlOut_STATUS_CONNECTINGCAAFS;
			m_pTickDealRecv		= &PNGSClient_I::Tick_DealRecv_STATUS_CONNECTINGCAAFS;
		break;
		case	STATUS_WAITINGINCAAFS:
			m_pTickControlOut	= &PNGSClient_I::Tick_ControlOut_STATUS_WAITINGINCAAFS;
			m_pTickDealRecv		= &PNGSClient_I::Tick_DealRecv_STATUS_WAITINGINCAAFS;
		break;
		case	STATUS_CONNECTINGCLS:
			m_pTickControlOut	= &PNGSClient_I::Tick_ControlOut_STATUS_CONNECTINGCLS;
			m_pTickDealRecv		= &PNGSClient_I::Tick_DealRecv_STATUS_CONNECTINGCLS;
		break;
		case	STATUS_WORKING:
			m_pTickControlOut	= &PNGSClient_I::Tick_ControlOut_STATUS_WORKING;
			m_pTickDealRecv		= &PNGSClient_I::Tick_DealRecv_STATUS_WORKING;
			// 开始工作后再开始使用发送统计数据
			// SetTE_PlayStat(); 这个现在由GMS来通知开启或关闭
		break;
		case	STATUS_DROPPED:
		case	STATUS_CONNECTFAIL:
		default:
			m_pTickControlOut	= &PNGSClient_I::Tick_ControlOut_STATUS_NOTHING;
			m_pTickDealRecv		= &PNGSClient_I::Tick_DealRecv_STATUS_NOTHING;
			m_teidStat.quit();
		break;
	}
	// 记录日志
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(609,PNGS_RT)"PNGSClient_I::SetStatus,%d,%d,0x%p,0x%p", m_nStatus, nStatus, this, m_pSlotMan);
	// 给上层状态改变通知
	CONTROL_T		*pCOCmd;
	ControlOutAlloc(pCOCmd);
	pCOCmd->nCmd		= CONTROL_OUT_STATUS;
	pCOCmd->nParam[0]	= nStatus;
	// 设置状态
	m_nStatus		= nStatus;
}
