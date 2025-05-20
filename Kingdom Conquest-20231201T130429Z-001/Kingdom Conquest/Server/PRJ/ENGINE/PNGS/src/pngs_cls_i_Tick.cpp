// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CLSģ���Tick����
//                PNGS��Pixel Network Game Structure����д
//                CLS��Connection Load Server����д�������ӷ��书���еĸ��ط�����
//                ���ļ���CLS���ڲ�ʵ��
//                CLS�Ϳͻ���ͨѶʹ��cnl2���ͷ�������ͨѶʹ��TCP��
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

	// ��Ϣ����
	// ����GMS��������Ϣ
	m_msgerGMS.Tick();

m_dtu.Mark();

	// �����GMS��������Ϣ(һ��Ӧ���ǲ��᷵�ظ����ġ�����������Ҫ�˳�Tick)
	if( Tick_DealGMSMsg()<0 )
	{
		return	-3;
	}

m_dtu.Mark();

	// ���ղ�����GZS��������Ϣ(ͬʱ�ж���GZS�����ӵ�״��)
	if( Tick_DealGZSMsg()<0 )
	{
		return	-4;
	}

m_dtu.Mark();

	// ���մ��û���������Ϣ
	m_pSlotMan4Client->TickRecv();

m_dtu.Mark();

	// ������û���������Ϣ
	if( Tick_DealClientMsg()<0 )
	{
		return	-5;
	}

m_dtu.Mark();

	// ����ʱ�¼�
	if( Tick_DealTE()<0 )
	{
		return	-6;
	}

m_dtu.Mark();

	// ��Client����ָ��
	m_pSlotMan4Client->TickLogicAndSend();

m_dtu.Mark();

	// ��GZS����ָ��
	DoAllGZSSend();

m_dtu.Mark();

	// ��GMS����ָ��(������������Ϊǰ��Ĵ����п��ܻᴥ����Ҫ��GMS���͵�ָ��)
	m_msgerGMS.ManualSend();

m_dtu.Mark();

#ifdef	_DEBUG
	if( m_cfginfo.nCmdStatInterval && m_loopCmdStat.check() )
	{
		m_loopCmdStat.reset();
		// ���һ����־
		m_vectrawbuf.clear();
		*m_vectrawbuf.getbuf()	= 0;	// �ѵ�һ���ֽ����㣨ע��sizeΪ0����[]���ʣ�
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
		// ���ͳ�Ƽ�¼
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
