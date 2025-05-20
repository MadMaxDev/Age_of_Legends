// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_RemovePlayerUnit.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTUREģ���RemovePlayerUnit��������ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Zone Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-09-22
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gzs_logic.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::RemovePlayerUnit(int nID)
{
	return	RemovePlayerUnit( m_Players.getptr(nID) );
}
int		GZS_MAINSTRUCTURE::RemovePlayerUnit(PlayerUnit *pPlayer)
{
	if( !pPlayer )
	{
		return	-1;
	}

	// ���ﲻ��֪ͨGamePlay������ֻ�������ϵ���Ƴ����߼���ϵ����ҪGamePlay֮�����֪ͨ�ġ�

	// �Ƴ����
	pPlayer->clear();
	m_Players.FreeUnitByPtr(pPlayer);

	return	0;
}
