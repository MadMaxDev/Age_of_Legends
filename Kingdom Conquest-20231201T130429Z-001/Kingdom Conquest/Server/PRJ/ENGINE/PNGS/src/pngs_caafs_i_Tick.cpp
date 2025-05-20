// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CAAFS中Tick部分的具体实现
// CreationDate : 2005-07-28
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::Tick()
{
	m_tickNow	= wh_gettickcount();

	if( m_nStatus != STATUS_WORKING )
	{
		return	-1;
	}

	m_itv.tick();

	// 消息处理
	// 接收GMS发来的消息
	m_msgerGMS.Tick();

	// 处理从GMS发来的消息(一般应该是不会返回负数的。除非里面需要退出Tick)
	if( Tick_DealGMSMsg()<0 )
	{
		return	-3;
	}

	// 接收从用户发来的消息
	m_pSlotMan4Client->TickRecv();
	// 处理从用户发来的消息
	if( Tick_DealClientMsg()<0 )
	{
		return	-4;
	}

	// 处理队列间的转换
	if( Tick_DealQueue()<0 )
	{
		return	-5;
	}

	// 处理定时事件
	if( Tick_DealTE()<0 )
	{
		return	-6;
	}

	// 向Client发送指令
	m_pSlotMan4Client->TickLogicAndSend();

	// 向GMS发送指令(放在这里是因为前面的处理中可能会触发需要向GMS发送的指令)
	m_msgerGMS.ManualSend();

	return	0;
}
