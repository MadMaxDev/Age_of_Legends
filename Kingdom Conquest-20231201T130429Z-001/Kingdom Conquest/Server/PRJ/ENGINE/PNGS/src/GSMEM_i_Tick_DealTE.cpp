// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : GSMEM_i_Tick_DealTE.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CAAFS��Tick_DealTE���ֵľ���ʵ�֣��͸���TEDeal_XXX����
// CreationDate : 2007-09-29
// Change LOG   :

#include "../inc/GSMEM_i.h"

using namespace n_pngs;

int		GSMEM::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// ����ʱ��������Ƿ��ж���
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_TQ.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// ���������������ʱ���¼�����������������ŵġ�
		// ���get������ֻ��ͨ��Delɾ���������Լ�ɾ
		m_TQ.Del(id);
	}
	return	0;
}

void	GSMEM::TEDeal_OutputStat(TQUNIT_T * pTQUnit)
{
	// ���ͳ��
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
		
	// ע����һ���¼�
	SetTE_OutputStat();
}
void	GSMEM::SetTE_OutputStat()
{
	whtimequeue::ID_T	id;
	TQUNIT_T	*pNextTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nOutputStatInterval, (void **)&pNextTQUnit, &id)<0 )
	{
		// ��ô���޷�����ʱ���¼�����
		assert(0);
		return;
	}
	pNextTQUnit->tefunc				= &GSMEM::TEDeal_OutputStat;
}
