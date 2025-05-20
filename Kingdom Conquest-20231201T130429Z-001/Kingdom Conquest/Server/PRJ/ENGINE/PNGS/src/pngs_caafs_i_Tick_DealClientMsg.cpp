// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick_DealClientMsg.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CAAFS��Tick_DealClientMsg���ֵľ���ʵ��
// CreationDate : 2005-08-03
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::Tick_DealClientMsg()
{
	m_tickNow	= wh_gettickcount();

	// ���ͻ��˵�����ȥȥ
	CNL2SlotMan::CONTROL_T	*pCOCmd;
	size_t	nSize;
	while( m_pSlotMan4Client->ControlOut(&pCOCmd, &nSize)==0 )
	{
		switch( pCOCmd->nCmd )
		{
			case	CNL2SlotMan::CONTROL_OUT_SLOT_ACCEPTED:
			{
				Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED(pCOCmd, nSize);
			}
			break;
			case	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSED:
			{
				Tick_DealClientMsg_CONTROL_OUT_SLOT_CLOSED(pCOCmd, nSize);
			}
			break;
			case	CNL2SlotMan::CONTROL_OUT_SHOULD_STOP:
			{
				m_bShouldStop	= true;
			}
			break;
			default:
			{
				// ������Ӧ�ò����յ���
			}
			break;
		}
	}

	// ����ͻ��˷�����ָ��(�ͻ��˿��ܷ���prelogin��ָ��)
	// ���ǿͻ��˲����ܷ��͹����ָ�Ŀǰ�������4����
	// ����ͻ��˷�����ָ��
	int			nSlot;
	int			nChannel;
	pngs_cmd_t	*pClientCmd;
	while( m_pSlotMan4Client->Recv(&nSlot, &nChannel, (void **)&pClientCmd, &nSize)==0 )
	{
		// �ҵ���ص��û�
		PlayerUnit	*pPlayer	= GetPlayerBySlot(nSlot);
		if( !pPlayer )
		{
			// ����Ѿ������ڣ����slotҲӦ�ùر���
			// �������ұ�Remove���ǶԷ�slot�л��ж�������������»����
			m_pSlotMan4Client->Close(nSlot);
			continue;
		}
		if( pPlayer->nRecvCmdNum++ > m_cfginfo.nMaxPlayerRecvCmdNum )
		{
			// ָ��̫���ˣ��Ͽ����û�
			m_pSlotMan4Client->Close(nSlot);
			continue;
		}
		if( nSize < sizeof(pngs_cmd_t) )
		{
			// ���ݳ��ȷǷ�
			m_pSlotMan4Client->Close(nSlot);
			continue;
		}
		switch( *pClientCmd )
		{
			case	CLIENT_CAAFS_2GMS:
			{
				// ��װһ��Ȼ�󴫸�GMS
				m_vectrawbuf.resize(sizeof(CAAFS_GMS_CLIENT_CMD_T) + nSize-sizeof(*pClientCmd));
				CAAFS_GMS_CLIENT_CMD_T	*pCmd2GMS	= (CAAFS_GMS_CLIENT_CMD_T *)m_vectrawbuf.getbuf();
				pCmd2GMS->nCmd						= CAAFS_GMS_CLIENT_CMD;
				pCmd2GMS->nClientIDInCAAFS			= pPlayer->nID;
				pCmd2GMS->IP						= pPlayer->IP;
				memcpy(wh_getptrnexttoptr(pCmd2GMS), wh_getptrnexttoptr(pClientCmd), nSize-sizeof(*pClientCmd));
				m_msgerGMS.SendMsg(pCmd2GMS, m_vectrawbuf.size());
			}break;
			case	CLIENT_CAAFS_BEFORE_LOGIN:
			{
				char	szAccount[TTY_ACCOUNTNAME_LONG_LEN]	= "";
				memcpy(szAccount,wh_getptrnexttoptr(pClientCmd),nSize-sizeof(*pClientCmd));
#ifdef UNITTEST
				printf("BEFORE LOGIN: %s\n",szAccount);
				DisplayQueueInfo();
#endif
				DealKeepQueuePosReq(pPlayer,szAccount);
#ifdef UNITTEST
				DisplayQueueInfo();
#endif
			}break;
			
			default:
				// ָ��Ƿ�
				m_pSlotMan4Client->Close(nSlot);
			break;
		}
	}

	return	0;
}
int		CAAFS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize)
{
	whvector<char>	*pvectExt	= m_pSlotMan4Client->GetConnectExtData(pCOCmd->data[0]);
	if( !pvectExt )
	{
		// �ܿ����Ǵ�������ж���
		return	0;
	}
	if( pvectExt->size() != sizeof(CLIENT_CONNECT_CAAFS_EXTINFO_T) )
	{
		// �������ݲ��ԣ�������hacker�ɣ�
		m_pSlotMan4Client->Close(pCOCmd->data[0], CLOSE_PARAM_TO_CLIENT_BADCMD);
		GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(556,PNGS_RT)"CAAFS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED,,connect extinfo size error,%d,%d", pvectExt->size(),sizeof(CLIENT_CONNECT_CAAFS_EXTINFO_T));
		return	0;
	}
	// ��������û�
	// Ӧ�ò����з��������ΪSlotMan�������������Players������һ��
	PlayerUnit	*pPlayer;
	int			nID	= m_Players.AllocUnit(pPlayer);
	if( nID<0 )
	{
		// ����С����¼��־���Ͽ��û�
		m_pSlotMan4Client->Close(pCOCmd->data[0], CLOSE_PARAM_TO_CLIENT_SVRMEM);
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(557,PNGS_RT)"CAAFS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED,,m_Players.AllocUnit ERROR");
		return	0;
	}


	CLIENT_CONNECT_CAAFS_EXTINFO_T	*pExtInfo	= (CLIENT_CONNECT_CAAFS_EXTINFO_T *)pvectExt->getbuf();
	// ��������
	pPlayer->nID				= nID;
	pPlayer->nSlot				= pCOCmd->data[0];
	pPlayer->IP					= m_pSlotMan4Client->GetSlotInfo(pPlayer->nSlot)->peerAddr.sin_addr.s_addr;
	// ��slot����(�ǵ�RemovePlayerUnit��ʱ��Ҫ����������)
	m_pSlotMan4Client->SetExtLong(pPlayer->nSlot, pPlayer->nID);
	// ����vipchannel:0
	PutPlayerInVIPChannel(pPlayer, 0);
	// ����Ϊ�ȴ�״̬
	pPlayer->nStatus			= PlayerUnit::STATUS_WAITING;
	pPlayer->nNetworkType		= pExtInfo->cmn.nNetworkType;

	// ����szSelfInfoToClient
	char	buf[1024];
	CAAFS_CLIENT_CAAFSINFO_T	*pCAAFSINFO	= (CAAFS_CLIENT_CAAFSINFO_T *)buf;
	pCAAFSINFO->nCmd			= CAAFS_CLIENT_CAAFSINFO;
	pCAAFSINFO->nVerCmpMode		= m_cfginfo.nVerCmpMode;
	strcpy(pCAAFSINFO->szVer, m_cfginfo.szVer);
	pCAAFSINFO->nQueueSize		= pPlayer->nSeq - pPlayer->nSeq0;
	memcpy(pCAAFSINFO->info, m_cfginfo.szSelfInfoToClient, m_cfginfo._nSelfInfoToClientLen);
	pCAAFSINFO->nSelfNotify		= m_nSelfNotify;
	nSize						= wh_offsetof(CAAFS_CLIENT_CAAFSINFO_T, info) + m_cfginfo._nSelfInfoToClientLen;
	m_pSlotMan4Client->Send(pPlayer->nSlot, pCAAFSINFO, nSize, 0);

	SetTE_QueueSeq(pPlayer, true);

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(913,PNGS_RT)"PlayerAccepted,0x%X,%d,%d,%d,%d", pPlayer->nID, m_nConnectingNum, m_nGMSAvailPlayerNum, m_Players.size(),pPlayer->nSeq);

	return	0;
}
int		CAAFS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_CLOSED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize)
{
	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T	*pCOCmdReason	= (CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T *)pCOCmd;
	// �ҵ���Ӧ��Player���Ƴ��� 
	PlayerUnit		*pPlayer		= m_Players.getptr((int)pCOCmdReason->nSlotExt);
	if( pPlayer )
	{
		pPlayer->nSlot	= 0;		// ���Remove��ʱ����Closeһ��
		RemovePlayerUnit(pPlayer);
	}

	return	0;
}
