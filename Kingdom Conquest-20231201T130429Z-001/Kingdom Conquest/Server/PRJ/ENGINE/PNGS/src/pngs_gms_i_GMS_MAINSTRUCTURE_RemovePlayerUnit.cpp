// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_RemovePlayerUnit.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ���RemovePlayerUnit��������ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-24
// Change LOG   : 2007-09-19 �������û�������CLS�����б��Ƴ�����idx���Ǳ�reserve������

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::RemovePlayerUnit(int nID, unsigned char nRemoveReason)
{
	return	RemovePlayerUnit( m_pSHMData->m_Players.GetPtrByID(nID), nRemoveReason );
}
int		GMS_MAINSTRUCTURE::RemovePlayerUnit(PlayerUnit *pPlayer, unsigned char nRemoveReason)
{
	if( !pPlayer )
	{
		return	-1;
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(833,PLAYER_OUT)"GMS_MAINSTRUCTURE::RemovePlayerUnit,0x%X,0x%X,%d,%d,%d", pPlayer->nID, pPlayer->nClientIDInCAAFS, pPlayer->nStatus, nRemoveReason, m_pSHMData->m_Players.size());

	// �������״̬�ȷ���һЩ��Ϣ������������
	// ����֪ͨ���е�GZS������������
	GMS_GZS_PLAYER_OFFLINE_T	PO;
	PO.nCmd						= GMS_GZS_PLAYER_OFFLINE;
	PO.anClientID[0]			= pPlayer->nID;
	SendCmdToAllGZS(&PO, sizeof(PO));

	// �ж��û��Ƿ���֪ͨGP������
	bool	bGPOnline	= true;
	switch( pPlayer->nStatus )
	{
		case	PlayerUnit::STATUS_WANTCLS:
		case	PlayerUnit::STATUS_GOINGTOCLS:
		{
			bGPOnline	= false;
		}
		break;
		default:
		break;
	}

	// ͨ��Player�ҵ�CLS���ҵ�CAAFS����ʱ��CLS������ɾ���ģ�
	CLSUnit		*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pPlayer->nCLSID);
	assert(pCLS);
	CAAFSGroup	*pCAAFS	= GetCAAFSByGroupID(pCLS->nGroupID);
	// ��ʱ���ǿ���CAAFS�ر���
	if(pCAAFS)
	{
		// ׼������
		GMS_CAAFS_CLIENTCANGOTOCLS_T	ClientCanGo;
		ClientCanGo.nCmd				= GMS_CAAFS_CLIENTCANGOTOCLS;
		ClientCanGo.nClientIDInCAAFS	= pPlayer->nClientIDInCAAFS;
		if( !bGPOnline )
		{
			ClientCanGo.nRst		= GMS_CAAFS_CLIENTCANGOTOCLS_T::RST_ERR_OTHERKNOWN;
			pCAAFS->pCntr->SendMsg(&ClientCanGo, sizeof(ClientCanGo));
		}
		else
		{
			// ������״̬�Ͳ��÷���ʲô������
		}
	}

	// ����Ҫ֪ͨ�������߼�ģ��(����GamePlay)
	GMSPACKET_2GP_PLAYEROFFLINE_T	CmdPlayerOut;
	CmdPlayerOut.nClientID	= pPlayer->nID;
	CmdPlayerOut.nReason	= nRemoveReason;

	// �Ƴ����
	// LOG��д��clearǰ�棬�������ݾͱ����ˣ�д��������
	pPlayer->clear();
	m_pSHMData->m_Players.FreeUnitByPtr(pPlayer);
	// �����ȱ���ԭ������ţ��ȵ��ϲ��߼�����ɾ������Ӧ����Ҳ��ܰ���ŷŻ�����
	if( bGPOnline )
	{
		m_pSHMData->m_Players.ReserveUnitByPtr(pPlayer);
	}

	// �����ָ���Ϊ���ڵ��̳߳�����˵���ܻ����������������������MS˵���԰ѱ�����PlayerUnit�ͷ�ΪFree���ˣ�
	CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_PLAYEROFFLINE, &CmdPlayerOut, sizeof(CmdPlayerOut));

	return	0;
}
