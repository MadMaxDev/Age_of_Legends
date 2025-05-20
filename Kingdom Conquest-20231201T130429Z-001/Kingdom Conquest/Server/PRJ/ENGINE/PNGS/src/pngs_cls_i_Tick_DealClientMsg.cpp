// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick_DealClientMsg.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CLS��Tick_DealClientMsg���ֵľ���ʵ��
// CreationDate : 2005-08-15
// Change LOG   :

#include "../inc/pngs_cls_i.h"

using namespace n_pngs;

int		CLS2_I::Tick_DealClientMsg()
{
	m_tickNow	= wh_gettickcount();

	// ���ͻ��˵�����ȥȥ
	CNL2SlotMan::CONTROL_T	*pCOCmd;
	size_t	nSize;
	int		nSlot;
	int		nChannel;
	pngs_cmd_t	*pClientCmd;
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

	// ����ͻ��˷�����ָ��
	while( m_pSlotMan4Client->Recv(&nSlot, &nChannel, (void **)&pClientCmd, &nSize)==0 )
	{
		// �ҵ���ص��û�
		PlayerUnit	*pPlayer	= GetPlayerBySlot(nSlot);
		if( !pPlayer )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(634,PNGS_RT)"CLS2_I::Tick_DealClientMsg,,Player missing for slot,0x%X", nSlot);
			// ����Ѿ������ڣ����slotҲӦ�ùر���
			// �������ұ�Remove���ǶԷ�slot�л��ж�������������»����
			m_pSlotMan4Client->Close(nSlot);
			continue;
		}
		// ��ָ��ߴ��Ƿ�����
		if( nSize < sizeof(pngs_cmd_t) )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(635,PNGS_RT)"CLS2_I::Tick_DealClientMsg,,bad client cmd size,0x%X,%d", pPlayer->nID, nSize);
			pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
			RemovePlayerUnit(pPlayer);
			continue;
		}
		if( (pPlayer->nProp & PNGS_CLS_PLAYER_PROP_LOG_RECV) != 0 )
		{
			char	buf[1024];
			size_t	nPSize	= sizeof(buf)/8;
			if( nSize<nPSize )
			{
				nPSize	= nSize;
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(778,DBG)"Player2CLS,0x%X,%s,%d,%d,%s", pPlayer->nID, pPlayer->szName, nChannel, nSize, dbg_printmem(pClientCmd, nPSize, buf));
		}

		//
		switch( *pClientCmd )
		{
			case	CLIENT_CLS_DATA:
				Tick_DealClientMsg_CLIENT_CLS_DATA(CLS_SVR_CLIENT_DATA, pPlayer, nChannel, pClientCmd, nSize);
			break;
			case	CLIENT_CLS_STAT:
				Tick_DealClientMsg_CLIENT_CLS_DATA(CLS_SVR_CLIENT_STAT, pPlayer, nChannel, pClientCmd, nSize);
			break;
			default:
				// ��֧�ֵ�ָ��ر�slot
				GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(636,PNGS_RT)"CLS2_I::Tick_DealClientMsg,%d,unknown ClientCmd,0x%X,0x%X,%d"
					, *pClientCmd, nSlot, pPlayer->nID, nSize
					);
				pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
				RemovePlayerUnit(pPlayer);
			break;
		}
		// �����ͷ�ָ���ΪRecv�ڲ�����һ����ʱ���������pData��
	}

	return	0;
}

int		CLS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize)
{
	int	nSlot	= pCOCmd->data[0];
	// ���ӵĸ��Ӳ�����Ӧ���и��Ӳ���
	whvector<char>	*pvectExtInfo	= m_pSlotMan4Client->GetConnectExtData(nSlot);
	if( !pvectExtInfo )
	{
		// ��ͬһ��tick�ھͱ�ɾ���ˣ�����ְ���Ӧ�ò�������Ϊ�ر�û�п�����ô���
		// 2008-07-10 ��ʵ֤���������ϸ�һ��tick�ڳ���������������������ڶԷ����ƽ�Э����ɵġ��������ĵ�һ�����������ɸ����Ƿ���CNL����Ϊ�յ���һ����������ACCEPTED״̬��Ȼ����Ϊ���Ƿ��Ͱ�SLOT�ر���
		//assert(0);
		return	0;
	}
	if( pvectExtInfo->size()!=sizeof(CLIENT_CLS_CONNECT_EXTINFO_T) )
	{
		// ֱ�ӹر��������
		GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(637,PNGS_RT)"CLS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED,,ext info bad size,%d,%d", pvectExtInfo->size(),sizeof(CLIENT_CLS_CONNECT_EXTINFO_T));
		m_pSlotMan4Client->Close(nSlot, CLOSE_PARAM_TO_CLIENT_BADCMD);
		return	0;
	}
	CLIENT_CLS_CONNECT_EXTINFO_T	*pExtInfo	= (CLIENT_CLS_CONNECT_EXTINFO_T *)pvectExtInfo->getbuf();
	// �ҵ���Ҷ���
	PlayerUnit	*pPlayer	= m_Players.getptr(pExtInfo->nClientID);
	if( !pPlayer )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(638,PNGS_RT)"CLS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED,,clientid not exist,0x%X", pExtInfo->nClientID);
		m_pSlotMan4Client->Close(nSlot, CLOSE_PARAM_TO_CLIENT_NOIDINCLS);
		return	0;
	}
	// ���ԭ��״̬��������״̬�����������ֱ�ӹر�
	if( pPlayer->nStatus != PlayerUnit::STATUS_CONNECTING )
	{
		m_pSlotMan4Client->Close(nSlot, CLOSE_PARAM_TO_CLIENT_PLAYERSTATUSNOTCONNECTING);
		return	0;
	}
	// �Ƚ�����
	if( pPlayer->nPasswordToConnectCLS != pExtInfo->nPassword )
	{
		m_pSlotMan4Client->Close(nSlot, CLOSE_PARAM_TO_CLIENT_BADCMD);
		return	0;
	}
	// ���ȷ�ϳɹ�
	// �����ʱ�¼�
	pPlayer->teid.quit();
	// ����slot��player
	pPlayer->nSlot		= nSlot;
	m_pSlotMan4Client->SetExtLong(nSlot, pPlayer->nID);
	// ���ù���״̬
	pPlayer->nStatus	= PlayerUnit::STATUS_WORKING;
	// ������Ϣ��GMS˵�û��Ѿ����ӳɹ���
	CLS_GMS_CLIENTTOCLS_RST_T	CTCRst;
	CTCRst.nCmd			= CLS_GMS_CLIENTTOCLS_RST;
	CTCRst.nRst			= CLS_GMS_CLIENTTOCLS_RST_T::RST_OK;
	CTCRst.nClientID	= pPlayer->nID;
	m_msgerGMS.SendMsg(&CTCRst, sizeof(CTCRst));

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(639,PNGS_RT)"CLS2_I Player accepted,0x%X,%d", pPlayer->nID, m_Players.size());

	// ���ɼ��hack�Ķ�ʱ�¼�
	SetTEDeal_nCheckRecvInterval(pPlayer);

	return	0;
}
int		CLS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_CLOSED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize)
{
	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T	*pCOCmdReason	= (CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T *)pCOCmd;
	// �ҵ���Ӧ��Player���Ƴ��� 
	PlayerUnit		*pPlayer		= m_Players.getptr((int)pCOCmdReason->nSlotExt);
	if( pPlayer )
	{
		switch( pCOCmdReason->nReason )
		{
			case	CNL2SlotMan::SLOTCLOSE_REASON_INITIATIVE:
				pPlayer->nRemoveReason		= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_SLOTCLOSE_REASON_INITIATIVE;
			break;
			case	CNL2SlotMan::SLOTCLOSE_REASON_PASSIVE:
				pPlayer->nRemoveReason		= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_SLOTCLOSE_REASON_PASSIVE;
			break;
			case	CNL2SlotMan::SLOTCLOSE_REASON_CLOSETimeout:
				pPlayer->nRemoveReason		= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_SLOTCLOSE_REASON_CLOSETimeout;
			break;
			case	CNL2SlotMan::SLOTCLOSE_REASON_DROP:
				pPlayer->nRemoveReason		= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_SLOTCLOSE_REASON_DROP;
			break;
			case	CNL2SlotMan::SLOTCLOSE_REASON_CONNECTTimeout:
				pPlayer->nRemoveReason		= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_SLOTCLOSE_REASON_CONNECTTimeout;
			break;
			case	CNL2SlotMan::SLOTCLOSE_REASON_ACCEPTTimeout:
				pPlayer->nRemoveReason		= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_SLOTCLOSE_REASON_ACCEPTTimeout;
			break;
			default:
				pPlayer->nRemoveReason		= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_SLOTCLOSE_REASON_NOTHING;
			break;
		}
		pPlayer->nSlot	= 0;
		RemovePlayerUnit(pPlayer);
	}

	return	0;
}
int		CLS2_I::Tick_DealClientMsg_CLIENT_CLS_DATA(pngs_cmd_t nCmd, PlayerUnit *pPlayer, int nChannel, pngs_cmd_t *pCmd, int nSize)
{
	CLIENT_CLS_DATA_T	*pCCData	= (CLIENT_CLS_DATA_T *)pCmd;
	int	nDSize	= nSize - wh_offsetof(CLIENT_CLS_DATA_T, data);
	if( nDSize<=0 )
	{
		// ������������������
		pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
		RemovePlayerUnit(pPlayer);
		return	0;
	}

	// ����������
	pPlayer->nRecvCount			++;
	pPlayer->nRecvSize			+= nSize;

	m_vectrawbuf.resize(wh_offsetof(CLS_SVR_CLIENT_DATA_T, data) + nDSize);
	CLS_SVR_CLIENT_DATA_T	&CSCD	= *(CLS_SVR_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	CSCD.nCmd		= nCmd;
	CSCD.nChannel	= nChannel;
	CSCD.nClientID	= pPlayer->nID;
	memcpy(CSCD.data, pCCData->data, nDSize);
	switch( pCCData->nSvrIdx )
	{
		case	PNGS_SVRIDX_GMS:
		{
			m_msgerGMS.SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
		default:
		{
			// ��߾�Ӧ����
			GZSUnit	*pGZS	= NULL;
			if( pCCData->nSvrIdx == PNGS_SVRIDX_AUTOGZS )
			{
				// ���������Լ���¼��GZSID���pGZS
				pGZS		= m_GZSs + pPlayer->nSvrIdx;
			}
			else
			{
				if( pCCData->nSvrIdx>=GZS_MAX_NUM )
				{
					// ���ǿͻ��˵�hack
					pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
					RemovePlayerUnit(pPlayer);
					return	0;
				}
				else
				{
					pGZS		= m_GZSs + pCCData->nSvrIdx;
				}
			}
			// ����û�����GZS��GZS���Թ��������ͣ�����ͺ���
			if( pGZS && pGZS->nStatus==GZSUnit::STATUS_WORKING )
			{
				pGZS->msgerGZS.SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
		}
		break;
	}
	return	0;
}
