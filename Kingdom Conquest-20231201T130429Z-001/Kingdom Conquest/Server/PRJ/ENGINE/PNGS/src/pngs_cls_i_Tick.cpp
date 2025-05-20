// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的CLS模块的Tick内容
//                PNGS是Pixel Network Game Structure的缩写
//                CLS是Connection Load Server的缩写，是连接分配功能中的负载服务器
//                本文件是CLS的内部实现
//                CLS和客户端通讯使用cnl2。和服务器间通讯使用TCP。
// CreationDate : 2005-07-28
// Change LOG   :

#include "../inc/pngs_cls_i.h"
#include <set>

using namespace n_pngs;

int		CLS2_I::Tick()
{
	m_tickNow	= wh_gettickcount();

	if( m_nStatus != STATUS_WORKING )
	{
		return	-1;
	}
dbgtimeuse	m_dtu;
m_dtu.BeginMark();
m_dtu.Mark();

	// 消息处理
	// 接收GMS发来的消息
	m_msgerGMS.Tick();

m_dtu.Mark();

	// 处理从GMS发来的消息(一般应该是不会返回负数的。除非里面需要退出Tick)
	if( Tick_DealGMSMsg()<0 )
	{
		return	-3;
	}

m_dtu.Mark();

	// 接收并处理GZS发来的消息(同时判断与GZS的连接的状况)
	if( Tick_DealGZSMsg()<0 )
	{
		return	-4;
	}

m_dtu.Mark();

	// 接收从用户发来的消息
	m_pSlotMan4Client->TickRecv();

m_dtu.Mark();

	// 处理从用户发来的消息
	if( Tick_DealClientMsg()<0 )
	{
		return	-5;
	}

m_dtu.Mark();

	// 处理定时事件
	if( Tick_DealTE()<0 )
	{
		return	-6;
	}

m_dtu.Mark();

	// 向Client发送指令
	m_pSlotMan4Client->TickLogicAndSend();

m_dtu.Mark();

	// 向GZS发送指令
	DoAllGZSSend();

m_dtu.Mark();

	// 向GMS发送指令(放在这里是因为前面的处理中可能会触发需要向GMS发送的指令)
	m_msgerGMS.ManualSend();

m_dtu.Mark();

#ifdef	_DEBUG
	if( m_cfginfo.nCmdStatInterval && m_loopCmdStat.check() )
	{
		m_loopCmdStat.reset();
		// 输出一下日志
		m_vectrawbuf.clear();
		*m_vectrawbuf.getbuf()	= 0;	// 把第一个字节清零（注意size为0不用[]访问）
		int	nSec	= m_cfginfo.nCmdStatInterval / 1000;
		std::multimap<CMDSTAT_T, unsigned short>	mapSort;

		for(size_t i=0;i<m_vectCmdStat.size();i++)
		{
			CMDSTAT_T	&cs	= m_vectCmdStat[i];
			if( cs.nCount>0 )
			{
				mapSort.insert(std::make_pair(cs,i));
			}
		}
		int	nCount	= 0;
		for(std::multimap<CMDSTAT_T, unsigned short>::iterator it=mapSort.begin(); it!=mapSort.end() && nCount++<=50; ++it)
		{
			m_vectrawbuf.f_aprintf(32, ",0x%04X %d %d", (*it).second, (*it).first.nByte, (*it).first.nCount);
		}
		if( m_vectrawbuf.size()>512 )
		{
			m_vectrawbuf[512]	= 0;
		}
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1008,PNGS_RT)"cmdstat,%d,%d%s", nSec, m_pSlotMan4Client->GetSlotNum(), m_vectrawbuf.getbuf());
		// 清除统计记录
		memset(m_vectCmdStat.getbuf(), 0, m_vectCmdStat.totalbytes());
	}
#endif

if( m_dtu.GetDiff()>=m_cfginfo.nBigTickFilter )
{
	//bigtickdebug
	char GZScountBuffer[512] = "";
	int buffer_offset = 0;
	for(size_t i=0;i<GZSTICKFUNC;i++)
	{
		sprintf(GZScountBuffer+strlen(GZScountBuffer),"%u %u,",m_GZStickFuncCount[i],m_GZStickFuncTime[i]);
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BIGTICK)"CLS2_I::Tick,GZS CMD COUNT:%d,%s,,detail:%s", m_nGZScmdCount,m_dtu.Disp(),GZScountBuffer);
}
	return	0;
}
