// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CAAFS��Tick���ֵľ���ʵ��
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

	// ��Ϣ����
	// ����GMS��������Ϣ
	m_msgerGMS.Tick();

	// �����GMS��������Ϣ(һ��Ӧ���ǲ��᷵�ظ����ġ�����������Ҫ�˳�Tick)
	if( Tick_DealGMSMsg()<0 )
	{
		return	-3;
	}

	// ���մ��û���������Ϣ
	m_pSlotMan4Client->TickRecv();
	// ������û���������Ϣ
	if( Tick_DealClientMsg()<0 )
	{
		return	-4;
	}

	// ������м��ת��
	if( Tick_DealQueue()<0 )
	{
		return	-5;
	}

	// ����ʱ�¼�
	if( Tick_DealTE()<0 )
	{
		return	-6;
	}

	// ��Client����ָ��
	m_pSlotMan4Client->TickLogicAndSend();

	// ��GMS����ָ��(������������Ϊǰ��Ĵ����п��ܻᴥ����Ҫ��GMS���͵�ָ��)
	m_msgerGMS.ManualSend();

	return	0;
}
