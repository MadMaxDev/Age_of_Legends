// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i_Send.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CNL2���ڲ�ʵ��
// CreationDate : 2005-05-13
// ChangeLog    : 2005-09-21 Send_DATA_NotSupport��ȡ��assert��Ϊ��־

#include "../inc/whnetcnl2_i.h"

using namespace n_whnet;

int		CNL2SlotMan_I_UDP::Send(int nSlot, const void *pData, size_t nSize, int nChannel)
{
	assert(nChannel>=0 && nChannel<CNL2_CHANNELNUM_MAX);

	if( (int)nSize > m_info.nMaxSinglePacketSize )
	{
		assert(0);
		return	-1;
	}

	m_pSlot	= m_Slots.getptr(nSlot);
	if( !m_pSlot )
	{
		return	-2;
	}
	if( m_info.bLogSendRecv )
	{
		const char		*pszIP	= cmn_get_ipnportstr_by_saaddr(&m_pSlot->slotinfo.peerAddr);
		if( !pszIP )
		{
			pszIP				= "no ip";
		}
		size_t	nNSize	= nSize<64 ? nSize:64;
		char	buf[1024];
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,TEMP)"CNL SEND,%s,%d,%d,%s", pszIP, nChannel, nSize, wh_hex2str((const unsigned char *)pData, nNSize, buf, 1, true));
	}


	m_pChannel	= &m_pSlot->channel[nChannel];
	int	rst	= (this->*m_Send_DATA[nChannel])(pData, nSize);
	if( rst<0 )
	{
		return	-300 + rst;
	}
	return		0;
}
int		CNL2SlotMan_I_UDP::Send_DATA_NotSupport(const void *pData, size_t nSize)
{
#ifdef	_DEBUG
	GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(6,CNL_RT)"CNL2SlotMan_I_UDP::Send_DATA_NotSupport,,Send_DATA_NotSupport! Check your channel settings");
#endif
	return	-1;
}
void	CNL2SlotMan_I_UDP::_MakeDataToSend(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T *pChannel, const void *pData, size_t nSize, unsigned char nDataPhase, unsigned char prop, cnl2idx_t nIdx)
{
	// �γ�RT������
	m_vectrawbuf.resize(sizeof(CNL2_CMD_DATA_T) + nSize);
	CNL2_CMD_DATA_T	*pCmdData	= (CNL2_CMD_DATA_T *)m_vectrawbuf.getbuf();
	pCmdData->cmd		= CNL2_CMD_DATA;
	pCmdData->dataphase	= nDataPhase;
	pCmdData->prop		= prop;
	pCmdData->channel	= pChannel->nChannel;
	pCmdData->slotidx	= pSlot->slotinfo.nPeerSlotIdx;
	pCmdData->nIdx		= nIdx;
	if( pChannel->pICryptorSend )	// ע��RT������ǲ����е�
	{
		pChannel->pICryptorSend->Encrypt(pData, nSize, wh_getptrnexttoptr(pCmdData));
	}
	else
	{
		memcpy(wh_getptrnexttoptr(pCmdData), pData, nSize);
	}
	pCmdData->calcsetcrc(m_vectrawbuf.size());
}
int		CNL2SlotMan_I_UDP::Send_DATA_RT(const void *pData, size_t nSize)
{
	_MakeDataToSend(m_pSlot, m_pChannel, pData, nSize, CNL2_CMD_0_T::DATAPHASE_SINGLE, 0, m_pChannel->nOutIdx0++);
	return	slot_send(m_pSlot, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}
int		CNL2SlotMan_I_UDP::Send_DATA_Safe(const void *pData, size_t nSize)
{
	return	_Send_DATA_Safe(pData, nSize, 0);
}
int		CNL2SlotMan_I_UDP::Send_DATA_Safe_Pack(const void *pData, size_t nSize)
{
	whDList<CMDPACKUNIT_T>::node	*pPackUnitNode	= NULL;
	NewDataByType(pPackUnitNode);
	CMDPACKUNIT_T	*pPackUnit		= &pPackUnitNode->data;
	pPackUnit->nSize	= nSize;
	AllocAndSetData(pPackUnit->pCmd, nSize, pData);
	// �����ݼ���channel�б�
	m_pChannel->AddCmdPackUnit(pPackUnitNode);
	return	0;
}
int		CNL2SlotMan_I_UDP::_Send_DATA_Safe(const void *pData, size_t nSize, unsigned char nProp)
{
	// �ֶ�
	if( nSize<=CNL2_MAX_SINGLEDATASIZE )
	{
		// �ж���������Ƿ���λ��
		if( m_pChannel->pOutQueue->GetAvailNum() == 0 )
		{
			return	-1;
		}
		// ֱ�ӷ���
		if( _Send_DATA_Safe_Single(pData, nSize, CNL2_CMD_0_T::DATAPHASE_SINGLE, nProp)<0 )
		{
			return	-2;
		}
		return		0;
	}
	else
	{
		int	nResidue;
		int	nPart	= wh_getintpartnum(nSize, CNL2_MAX_SINGLEDATASIZE, &nResidue);
		assert(nPart>=2);
		// �ж���������Ƿ���λ�ã����û��λ�þͶ������ͣ����ᷢ��һ���ֵģ�
		if( nPart > m_pChannel->pOutQueue->GetAvailNum() )
		{
			return	-11;
		}
		// ѭ����Ƭ����
		// ͷ
		if( _Send_DATA_Safe_Single(pData, CNL2_MAX_SINGLEDATASIZE, CNL2_CMD_0_T::DATAPHASE_HEAD, nProp)<0 )
		{
			return	-12;
		}
		// �м�
		for(int i=1;i<nPart-1;i++)
		{
			pData	= wh_getoffsetaddr(pData, CNL2_MAX_SINGLEDATASIZE);
			if( _Send_DATA_Safe_Single(pData, CNL2_MAX_SINGLEDATASIZE, CNL2_CMD_0_T::DATAPHASE_SINGLE, nProp)<0 )
			{
				return	-13;
			}
		}
		// ���
		pData		= wh_getoffsetaddr(pData, CNL2_MAX_SINGLEDATASIZE);
		if( _Send_DATA_Safe_Single(pData, nResidue>0 ? nResidue : CNL2_MAX_SINGLEDATASIZE, CNL2_CMD_0_T::DATAPHASE_TAIL, nProp)<0 )
		{
			return	-14;
		}
		return		0;
	}
}
int		CNL2SlotMan_I_UDP::_Send_DATA_Safe_Single(const void *pData, size_t nSize, unsigned char nDataPhase, unsigned char nProp)
{
	_MakeDataToSend(m_pSlot, m_pChannel, pData, nSize, nDataPhase, nProp, m_pChannel->pOutQueue->size() + m_pChannel->nOutIdx0);
	// �ܵ��õ������˵������һ���ǹ��õ�
	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= m_pChannel->pOutQueue->Alloc();
	assert(pOutUnit);
	pOutUnit->clear(this);
	AllocAndSetData(pOutUnit->pCmd, m_vectrawbuf.size(), m_vectrawbuf.getbuf());
	pOutUnit->nSize			= m_vectrawbuf.size();
	// ��¼����ʱ��
	pOutUnit->nSendTime		= m_tickNow;
	
	if( SendOutUnitThisTick(m_pSlot, pOutUnit)<0 )
	{
		FreeAndClearData(pOutUnit->pCmd);
		return	-1;
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::Send_DATA_LastSafe(const void *pData, size_t nSize)
{
	// �ж�ԭ���Ƿ��л�û��ȷ�ϵ�
	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= m_pChannel->pOutQueue->GetFirst();
	if( pOutUnit )
	{
		// �����
		// ɾ���ط�ʱ���¼�
		pOutUnit->clear(this);
		// ��ն���
		m_pChannel->pOutQueue->Clean();
		// ���������
		m_pChannel->nOutIdx0		++;
		// ����nOutIdxConfirm��nOutIdx0��ȱ�ʾ������ݻ�û�б�ȷ��
		m_pChannel->nOutIdxConfirm	= m_pChannel->nOutIdx0;
	}
	// �������
	_Send_DATA_Safe_Single(pData, nSize, CNL2_CMD_0_T::DATAPHASE_SINGLE, 0);
	return	0;
}
void	CNL2SlotMan_I_UDP::SendAllUnConfirmedDataThisTick(CNL2SLOT_I *pSlot)
{
	for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		CNL2SLOT_I::CHANNEL_T	&chn	= pSlot->channel[i];
		if( chn.pOutQueue )
		{
			chn.pOutQueue->BeginGet();
			CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit;
			while( (pOutUnit=chn.pOutQueue->GetNext()) != NULL )
			{
				if( pOutUnit->nTimeEventID.IsValid() )
				{
					if( SendOutUnitThisTick(pSlot, pOutUnit)<0 )
					{
						return;
					}
				}
			}
		}
	}
}
int		CNL2SlotMan_I_UDP::SendOutUnitThisTick(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit)
{
	pOutUnit->nRSCount	= 1;	// ע�����Ĭ�ϳ�ʼֵ������1
	if( pOutUnit->nTimeEventID.IsValid() )
	{
		pOutUnit->nTimeEventID.quit();
	}

	TQUNIT_T				TQUnit;
	TQUNIT_T				*pTQUnit	= &TQUnit;
	pTQUnit->pSlot			= pSlot;
	pTQUnit->tefunc			= &CNL2SlotMan_I_UDP::TEDeal_Data_RS;
	pTQUnit->un.resend.pOutUnit	= pOutUnit;
	TEDeal_Data_RS(pTQUnit);
	return	0;
}
int		CNL2SlotMan_I_UDP::SendOutUnitWithDelay(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit, int nDelay)
{
	pOutUnit->nRSCount	= 1;	// ע�����Ĭ�ϳ�ʼֵ������1
	whtick_t	t		= m_tickNow+nDelay;
	if( pOutUnit->nTimeEventID.IsValid() )
	{
		// ԭ��ʱ����������������ʱ��
		if( nDelay>0 && wh_tickcount_diff(pOutUnit->nTimeEventID.t, t)<0 )
		{
			// ʹ��ԭ��ʱ��
			t			= pOutUnit->nTimeEventID.t;
		}
		pOutUnit->nTimeEventID.quit();
	}
	TQUNIT_T				*pTQUnit;
	if( m_tq.AddGetRef(t, (void **)&pTQUnit, &pOutUnit->nTimeEventID)<0 )
	{
		assert(0);
		return	-1;
	}
	pTQUnit->pSlot			= pSlot;
	pTQUnit->tefunc			= &CNL2SlotMan_I_UDP::TEDeal_Data_RS;
	pTQUnit->un.resend.pOutUnit	= pOutUnit;
	return	0;
}
void	CNL2SlotMan_I_UDP::SendSwitchAddrReq(CNL2SLOT_I *pSlot)
{
	CNL2_CMD_SWITCHADDR_T	CmdSA;
	CmdSA.slotidx			= pSlot->slotinfo.nPeerSlotIdx;
	CmdSA.cmd				= CNL2_CMD_SWITCHADDR;
	if( pSlot->bIsClient )
	{
		CmdSA.nSubCmd		= CNL2_CMD_SWITCHADDR_T::SUBCMD_REQ;
		MakeSlotSeedKeyMD5(pSlot, CmdSA.szKeyMD5);
		CmdSA.nSeed			= pSlot->slotinfo.nSpecialSeed;
	}
	else
	{
		CmdSA.nSubCmd		= CNL2_CMD_SWITCHADDR_T::SUBCMD_ORDER;
		CmdSA.nSeed			= (unsigned short)wh_time();
		wh_randhex(CmdSA.szKeyMD5, sizeof(CmdSA.szKeyMD5));		// ���ֻ��Ϊ���Ի����
	}
	CmdSA.calcsetcrc(sizeof(CmdSA));
	// ��������
	udp_sendto(&CmdSA, sizeof(CmdSA), &m_addr);
}
