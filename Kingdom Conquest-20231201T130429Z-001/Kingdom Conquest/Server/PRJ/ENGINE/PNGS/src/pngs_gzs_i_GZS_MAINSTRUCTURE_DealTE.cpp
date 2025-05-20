// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_DealTE.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTUREģ����ʱ�䴦���ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Zone Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-09-21
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::Tick_DealTE()
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

void	GZS_MAINSTRUCTURE::TEDeal_Hello_TimeOut(TQUNIT_T *pTQUnit)
{
	// ��ָ��ʱ��û���յ��Է�������Hello��
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(675,PNGS_RT)"GZS_MAINSTRUCTURE::TEDeal_Hello_TimeOut,,0x%X", pTQUnit->un.svr.nID);
	// ɾ����Ӧ��Connecter
	RemoveMYCNTR(pTQUnit->un.svr.nID);
}
