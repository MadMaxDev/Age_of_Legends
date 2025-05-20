// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick_DealTE.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CLS��Tick_DealTE���ֵľ���ʵ�֣��͸���TEDeal_XXX����
// CreationDate : 2005-08-15
// Change LOG   :

#include "../inc/pngs_cls_i.h"

using namespace n_pngs;

int		CLS2_I::Tick_DealTE()
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

void	CLS2_I::TEDeal_nClientConnectToMeTimeOut(TQUNIT_T *pTQUnit)
{
	// ɾ�����(ɾ�������ڲ���֪ͨGMS��)
	RemovePlayerUnit(pTQUnit->un.player.nID);
}
void	CLS2_I::SetTEDeal_nClientConnectToMeTimeOut(PlayerUnit *pPlayer)
{
	TQUNIT_T	*pTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nClientConnectToMeTimeOut, (void **)&pTQUnit, &pPlayer->teid)<0 )
	{
		// ��ô���޷�����ʱ���¼�����
		assert(0);
		// �Ƴ��û�(�������֪ͨGMS����ʧ��)
		RemovePlayerUnit(pPlayer);
		return;
	}
	pTQUnit->tefunc			= &CLS2_I::TEDeal_nClientConnectToMeTimeOut;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
void	CLS2_I::TEDeal_nCheckRecvInterval(TQUNIT_T *pTQUnit)
{
	// ��ȡplayer����
	PlayerUnit	*pPlayer	= m_Players.getptr(pTQUnit->un.player.nID);
	if( pPlayer )
	{
		if( pPlayer->nRecvCount>m_cfginfo.nMaxRecvCount || pPlayer->nRecvSize>m_cfginfo.nMaxRecvSize )
		{
			// ���û�����
			GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(1089,PNGS_RT)"0x%X,%s,%d,%d", pPlayer->nID, pPlayer->szName, pPlayer->nRecvCount, pPlayer->nRecvSize);
			pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
			RemovePlayerUnit(pPlayer);
		}
		else
		{
			// ��գ�����ͳ��
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
		// ��ô���޷�����ʱ���¼�����
		assert(0);
		// �Ƴ��û�(�������֪ͨGMS����ʧ��)
		RemovePlayerUnit(pPlayer);
		return;
	}
	pTQUnit->tefunc			= &CLS2_I::TEDeal_nCheckRecvInterval;
	pTQUnit->un.player.nID	= pPlayer->nID;
}
