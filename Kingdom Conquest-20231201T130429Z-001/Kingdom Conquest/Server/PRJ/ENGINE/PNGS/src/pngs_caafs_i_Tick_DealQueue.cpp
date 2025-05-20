// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick_DealQueue.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CAAFS��Tick_DealQueue���ֵľ���ʵ��(����ȴ����к����Ӷ��еı任)
// CreationDate : 2005-08-04
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::Tick_DealQueue()
{
	// ���connecting�������п�λ�ͰѺ���Ĳ���
	int		nAvail	= m_cfginfo.nClientConnectingNumMax - m_nConnectingNum;
	if( nAvail<=0 )
	{
		// Connecting����̫����
		return	0;
	}

	// ��ÿ��Լ�����ٸ�
	int		nNum	= m_generWaitTransToConnect.MakeDecision(m_itv.getfloatInterval());
	if( nNum<nAvail )
	{
		if( nNum==0 && m_cfginfo.bAtLeastOneTransToConnectPerTick )	// ������֤��tick�����ʱ�����ٿ��Լ��һ���Ƿ��������Ҫ����
		{
			nNum	= 1;
		}
		nAvail		= nNum;
	}
	// ����Ҫ��Ķ��п�ʼ����Waiting���е�ͷ��ʼ���������nAvail������Connecting������
	for(int i=m_vectWaitChannel.size()-1;i>=0;i--)
	{
		WAITCHANNEL_T	*pC	= m_vectWaitChannel.getptr(i);
		if( pC->pdlWait->size()>0 )
		{
			whDList<PlayerUnit *>::node	*pNode	= pC->pdlWait->begin();
#ifdef UNITTEST
			if(m_nGMSAvailPlayerNum==0)	printf("---deal tq : GMS : %d,  Avail : %d , Channel : %d\n",m_nConnectingNum,nAvail,i);
#endif
			while( m_nGMSAvailPlayerNum>0 && nAvail>0 && pNode != pC->pdlWait->end() )
			{
				whDList<PlayerUnit *>::node	*pCurNode	= pNode;
				// �������һ��(��Ϊ����Ĳ����п��ܻᵼ���û��뿪Waiting��)
				nAvail					--;
				pNode					= pNode->next;
				m_nGMSAvailPlayerNum	--;
				// ��ҽ���Connecting����
				TransPlayerFromWaitToConnectQueue(pCurNode->data);
				// �ȴ���ͷ�����һ�����
				pC->nWaitingSeq0		= pCurNode->data->nSeq;
			}
		}
	}

	return	0;
}
int		CAAFS2_I::TransPlayerFromWaitToConnectQueue(PlayerUnit *pPlayer)
{
	// Ӧ�ò����ܣ���waitqueue�оͲ���������״̬
	assert( pPlayer->nStatus == PlayerUnit::STATUS_WAITING );

	// ����++
	m_nConnectingNum	++;

	// ���ԭ����ʱ���¼�
	pPlayer->teid.quit();
	// ��Waiting�����г���
	pPlayer->nodeWC.leave();
	// ���뵽Connecting������
	m_dlPlayerConnecting.AddToTail(&pPlayer->nodeWC);

	// �����û���������
	pPlayer->nPasswordToConnectCLS	= GenPasswordToConnectCLS();

	// ״̬ת��
	pPlayer->nStatus	= PlayerUnit::STATUS_CONNECTINGCLS;	

	// ��GMS�������
	CAAFS_GMS_CLIENTWANTCLS_T		ClientWantCLS;
	ClientWantCLS.nCmd				= CAAFS_GMS_CLIENTWANTCLS;
	ClientWantCLS.nClientIDInCAAFS	= pPlayer->nID;
	ClientWantCLS.IP				= pPlayer->IP;
	ClientWantCLS.nPassword			= pPlayer->nPasswordToConnectCLS;
	ClientWantCLS.nNetworkType		= pPlayer->nNetworkType;
	m_msgerGMS.SendMsg(&ClientWantCLS, sizeof(ClientWantCLS));

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(912,PNGS_RT)"TransPlayerFromWaitToConnectQueue,0x%X,%d,%d,%d,%d", pPlayer->nID, m_nConnectingNum, m_nGMSAvailPlayerNum, m_Players.size(),pPlayer->nSeq);

	// �����賬ʱ����CLSû�з�Ӧ����Ϊ��GMS��һ�����趨�����ʱ�ģ����CLSһ��ʱ����û��Ӧ����CLS�����������Ӧ����
	// GMSһ������û�����һ�����GMS_CAAFS_CLIENTCANGOTOCLS�����۳ɹ���ʧ�ܣ�

	return	0;
}
