// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i_TickRecv.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CNL2���ڲ�ʵ��
// CreationDate : 2005-04-17
// ChangeLog    : 2005-07-13 cmn_getsizetorecv���ܻᵼ�����ݺܾ��Ժ���ܱ��յ�
//                2005-07-14 ������ԭ�����Ӻ�Client������NOOP��ʧ���������Ӻ����̷��������ݲ��ܱ�����������Ĵ���
//                �������ڲ�����_TickRecv_FIRST_CMD_ON_ACCEPTING����NOOP_ON_ACCEPTING��DATA_ON_ACCEPTING�ж�ʹ������ת��״̬
//                2006-02-05 ������TickRecv_CMD_Others_DATA_Deal_Safe���շ���˳��ͬ���½���˳������BUG��

#include "../inc/whnetcnl2_i.h"
#include <WHCMN/inc/whbits.h>

using namespace	n_whnet;
using namespace	n_whcmn;

int		CNL2SlotMan_I_UDP::TickRecv()
{
	maketicknow();

	//while( (m_nCmdSize=cmn_getsizetorecv(m_sock)) > 0 )
	while( cmn_select_rd(m_sock, 0) > 0 )
	{
		m_pCmd			= (CNL2_CMD_0_T *)m_vectrawbuf.getbuf();
		if( (m_nCmdSize=udp_recv(m_pCmd, m_vectrawbuf.capacity(), &m_addr))<0 )
		{
			break;
		}
		if( m_nCmdSize<=0 )
		{
			// ��������������˲����󣬻��ߵ��Թ�����ɵ�
#ifdef	_WH_LOWLEVELDEBUG
			assert(0);
#endif
			continue;
		}
		// У��CRC
		if( !m_pCmd->checkcrc(m_nCmdSize) )
		{
#ifdef	_WH_LOWLEVELDEBUG
			// ȡ������ɣ�����Ƿ������������˷���������ͻص���debug��ķ�����崵���
			// assert(0);
#endif
			continue;
		}

		m_vectrawbuf.resize(m_nCmdSize);
		// ֻ��һ���ж�����Ч�ʱ��ú��������(��ʵ6���ж�����ϵͳ�����Ż��ıȽϺã���ֻ���Ե�6��)
		switch( m_pCmd->cmd )
		{
		case	CNL2_CMD_EXTEND:
			{
				TickRecv_CMD_EXTEND();
			}
			break;
		case	CNL2_CMD_CONNECT:
			{
				TickRecv_CMD_CONNECT();
			}
			break;
		default:
			{
				TickRecv_CMD_Others();
			}
			break;
		}
	}
	return	0;
}
void	CNL2SlotMan_I_UDP::TickRecv_CMD_EXTEND()
{
	CNL2_CMD_EXTEND_T			*pExt	= (CNL2_CMD_EXTEND_T *)m_pCmd;
	switch( pExt->subcmd )
	{
	case	CNL2_CMD_EXTEND_T::CMD_ECHO:
		{
			// ����ָ������������ԭ������
			pExt->subcmd		= CNL2_CMD_EXTEND_T::CMD_ECHO_RST;
			pExt->calcsetcrc(m_nCmdSize);
			udp_sendto(pExt, m_nCmdSize, &m_addr);
		}
		break;
	default:
		{
			// ����ʶ�ľͲ��ô�����
		}
		break;
	}
}
void	CNL2SlotMan_I_UDP::TickRecv_CMD_CONNECT()
{
	// �����Ƿ���listen
	if( !m_bListen )
	{
		// ֱ�Ӻ��ԣ��öԷ���ʱȥ��
		return;
	}
	// ע��������ķ��Ͳ�����slot_send����Ϊ������Ķ��Ǵ����ͣ�slotӦ�û�������
	// ����Ƿ��ַ�Ѿ�����
	if( m_mapaddr2slot.has(m_addr) )
	{
		// �Ѿ����ڵ����ӣ�����������ָ��
		return;
	}
	// �����Ƿ�������IP��Χ
	if( !m_ConnectAllowDeny.isallowed(m_addr) )
	{
		// ���÷����κ���ʾ
		return;
	}
	CNL2_CMD_CONNECT_T			*pCmdConnect	= (CNL2_CMD_CONNECT_T *)m_pCmd;
	CNL2_CMD_CONNECT_REFUSE_T	CmdRefuse;
	CmdRefuse.cmd				= CNL2_CMD_CONNECT_REFUSE;
	CmdRefuse.channel			= 0;
	CmdRefuse.slotidx			= pCmdConnect->clientslotidx;
	// �жϰ汾
	if( m_info.ver != pCmdConnect->ver )
	{
		CmdRefuse.reason		= CONNECTREFUSE_REASON_BADVER;
		CmdRefuse.calcsetcrc(sizeof(CmdRefuse));
		udp_sendto(&CmdRefuse, sizeof(CmdRefuse), &m_addr);
		return;
	}
	// ��������������ݲ���
	const void	*pPubForServer=NULL, *pExtData=NULL;
	int		nPubForServerSize=0, nExtData=0;
	void	*pRear		= wh_getptrnexttoptr(pCmdConnect);
	int		nRearSize	= m_nCmdSize - sizeof(*pCmdConnect);

	try
	{
		// ע�⣺��Ϊ��Կ���ܴ���256�ֽڣ�����Ҫ��short���������ݲ��ܳ���255�ֽڣ�������char
		nPubForServerSize	= whvldata_read_ref((CONNECT_PUBKEY_LEN_T*)pRear, pPubForServer);
		if( nPubForServerSize<=0 )
		{
			throw	1;
		}
		nRearSize	-= sizeof(CONNECT_PUBKEY_LEN_T)+nPubForServerSize;
		if( nRearSize<0 )
		{
			throw	2;
		}
		pRear		= wh_getoffsetaddr(pRear, sizeof(CONNECT_PUBKEY_LEN_T)+nPubForServerSize);
		nExtData	= whvldata_read_ref((CONNECT_EXT_LEN_T*)pRear, pExtData);
		nRearSize	-= sizeof(CONNECT_EXT_LEN_T)+nExtData;
		if( nRearSize<0 )
		{
			throw	11;
		}
	}
	catch( int nErr )
	{
		// Ӧ����hack������
		CmdRefuse.reason	= CONNECTREFUSE_REASON_BADDATA;
		CmdRefuse.calcsetcrc(sizeof(CmdRefuse));
		udp_sendto(&CmdRefuse, sizeof(CmdRefuse), &m_addr);
		GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(7,CNL_RT)"CNL2SlotMan_I_UDP::TickRecv_CMD_CONNECT,%d,BAD REQUEST,%s", nErr, cmn_get_ipnportstr_by_saaddr(&m_addr));
		return;
	}

	// slot����man��AddSlot������¼LastRecvΪ��ǰʱ�̣�
	int	nSlot;
	int	rst	= AddSlot(m_addr, CNL2SLOTINFO_T::STATUS_ACCEPTING, pCmdConnect->clientslotidx, pExtData, nExtData, &nSlot, pPubForServer, nPubForServerSize, m_nMyAddrSum);
	if( rst<0 )
	{
		CmdRefuse.reason			= CONNECTREFUSE_REASON_INTERNALERROR;
		switch( rst )
		{
			case	ADDSLOT_RST_SLOTFULL:
				CmdRefuse.reason	= CONNECTREFUSE_REASON_SLOTFULL;
			break;
		}
		// ���ش���
		CmdRefuse.calcsetcrc(sizeof(CmdRefuse));
		udp_sendto(&CmdRefuse, sizeof(CmdRefuse), &m_addr);
		return;
	}
}
void	CNL2SlotMan_I_UDP::TickRecv_CMD_SWITCHADDR()
{
	CNL2_CMD_SWITCHADDR_T	*pCmdSA	= (CNL2_CMD_SWITCHADDR_T *)m_pCmd1;
	//
	if( m_pSlot->bIsClient )
	{
		if( pCmdSA->nSubCmd == CNL2_CMD_SWITCHADDR_T::SUBCMD_ORDER )
		{
			if( SwitchAddr(m_pSlot)<0 )
			{
				return;
			}
			// ����ת������
			SendSwitchAddrReq(m_pSlot);
		}
		else
		{
			// ˵������ǶԷ��Ѿ��յ��������ҾͲ��ü������͸ı��ַָ����
			// Ȼ�������channel��Ҫ�ط���ָ�������tick��ȫ���ط�
			SendAllUnConfirmedDataThisTick(m_pSlot);
		}
		// �������һ���յ����ݵ�ʱ��
		m_pSlot->slotinfo.nLastRecv	= m_tickNow;
	}
	else
	{
		if( pCmdSA->nSubCmd == CNL2_CMD_SWITCHADDR_T::SUBCMD_REQ )
		{
			// �Ƚ�һ��MD5�Ƿ���ȷ
			if( !VerifySlotSeedKeyMD5(m_pSlot, pCmdSA->nSeed, pCmdSA->szKeyMD5) )
			{
				return;
			}
			// �������һ���յ����ݵ�ʱ��
			m_pSlot->slotinfo.nLastRecv	= m_tickNow;

			// �������ͻ�Ӧ
			CNL2_CMD_SWITCHADDR_T	CmdSA;
			CmdSA.cmd				= CNL2_CMD_SWITCHADDR;
			CmdSA.nSubCmd			= CNL2_CMD_SWITCHADDR_T::SUBCMD_RPL;
			CmdSA.nSeed				= (unsigned short)wh_time();
			wh_randhex(CmdSA.szKeyMD5, sizeof(CmdSA.szKeyMD5));		// ���ֻ��Ϊ���Ի����
			CmdSA.calcsetcrc(sizeof(CmdSA));
			udp_sendto(&CmdSA, sizeof(CmdSA), &m_addr);
			if( m_addr != m_pSlot->slotinfo.peerAddr )
			{
				// ��ַ��ԭ����ͬ����Ҫ�л�һ�£������˵����ԭ���Ѿ��յ����İ��ˣ�
				// ��Ҫ�ȼ�¼һ����־
				char	szIP1[WHNET_MAXADDRSTRLEN];
				char	szIP2[WHNET_MAXADDRSTRLEN];
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(798,CNL_RT)"CNL2SlotMan_I_UDP,peer switch addr,0x%X,%d,%s,%s", m_pSlot->slotinfo.nSlot, CmdSA.nSeed, cmn_get_ipnportstr_by_saaddr(&m_pSlot->slotinfo.peerAddr, szIP1) , cmn_get_ipnportstr_by_saaddr(&m_addr, szIP2));
				// ɾ���ɵ�ַ
				m_mapaddr2slot.erase(m_pSlot->slotinfo.peerAddr);
				// �����µ�ַӳ��
				m_pSlot->slotinfo.peerAddr	= m_addr;
				m_mapaddr2slot.put(m_pSlot->slotinfo.peerAddr, m_pSlot->slotinfo.nSlot);
				// Ȼ�������channel��Ҫ�ط���ָ�������tick��ȫ���ط�
				SendAllUnConfirmedDataThisTick(m_pSlot);
			}
		}
	}
}
void	CNL2SlotMan_I_UDP::TickRecv_CMD_Others()
{
	m_pCmd1	= (CNL2_CMD_1_T *)m_pCmd;
	// �ҵ���Ӧ��slot������������
	m_pSlot		= m_Slots.GetByIdx(m_pCmd1->slotidx);
	if( !m_pSlot )
	{
		return;
	}

	// CNL2_CMD_SWITCHADDR�����ǻ���Դ��ַ��ԭ��ַ��ͬ��
	if( CNL2_CMD_SWITCHADDR == m_pCmd1->cmd )
	{
		TickRecv_CMD_SWITCHADDR();
		return;
	}

	// �ж���Դ��ַ��slot��ַ�Ƿ�һ��
	// ������ô��������close�ͱ���Ҫ�������������˿��ܶ���ر������û���slot
	if( m_addr != m_pSlot->slotinfo.peerAddr )
	{
		return;
	}
	// �����������Щ
	if( TickRecv_CMD_Others_Call(m_pCmd1->cmd)==0 )
	{
		// �������ɹ�����������һ���յ����ݵ�ʱ��
		m_pSlot->slotinfo.nLastRecv	= m_tickNow;
	}
}
void	CNL2SlotMan_I_UDP::FillSlotWithDefaultTickRecvDealFunc(CNL2SLOT_I *pSlot)
{
	// ���ú���ָ��
	for(int i=0;i<CNL2_CMD_MAX;i++)
	{
		pSlot->dealfuncRecv[i]					= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_Unknown;
	}
	// ע�⣺CNL2_CMD_CONNECT����Ҫ���⴦��ģ���TickRecv_CMD_CONNECT������
	// ������ָ���ɺ���ķֱ���
	pSlot->dealfuncRecv[CNL2_CMD_CONNECT_AGREE]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CONNECT_AGREE;
	pSlot->dealfuncRecv[CNL2_CMD_CONNECT_REFUSE]= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CONNECT_REFUSE;
	pSlot->dealfuncRecv[CNL2_CMD_CLOSE]			= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CLOSE;
	pSlot->dealfuncRecv[CNL2_CMD_DATA_CONFIRM]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_CONFIRM;
	pSlot->dealfuncRecv[CNL2_CMD_DATA]			= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_ON_ACCEPTING;
	pSlot->dealfuncRecv[CNL2_CMD_NOOP]			= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP_ON_ACCEPTING;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_Unknown()
{
	// Ӧ���Ƕ��⹥��
	// �����رո�Slot
	CloseOnErr(m_pSlot, CLOSEONERR_TickRecv_CMD_Others_Unknown);
	return	-1;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CONNECT_AGREE()
{
	// �жϸ�slot�Ƿ��Ѿ�֪����
	switch( m_pSlot->slotinfo.nStatus )
	{
		case	CNL2SLOTINFO_T::STATUS_CONNECTING:
		{
			CNL2_CMD_CONNECT_AGREE_T	*pCmdAgree	= (CNL2_CMD_CONNECT_AGREE_T *)m_pCmd;
			m_pSlot->slotinfo.nStatus			= CNL2SLOTINFO_T::STATUS_WORKING;
			m_pSlot->slotinfo.nPeerSlotIdx		= pCmdAgree->serverslotidx;
			// �ͷ�������NoopTimeout����һ��
			m_nNoopTimeout						= pCmdAgree->nNoopTimeout;
			m_info.nDropTimeout					= pCmdAgree->nDropTimeout;
			m_nMightDropTimeout					= pCmdAgree->nMightDropTimeout;
			// ���ԭ���ĳ�ʱ�¼�(��Ϊchannel��ʱ��û���γɣ����Բ��ùر�channel���¼���)
			m_pSlot->ClearAllTE();
			try
			{
				// �����������ݻ�ý�������Կ
				void	*pRear		= wh_getptrnexttoptr(pCmdAgree);
				int		nRearSize	= m_nCmdSize - sizeof(*pCmdAgree);
				const void	*pPubForClient	= NULL;
				int		nPubForClientSize	= whvldata_read_ref((CONNECT_PUBKEY_LEN_T*)pRear, pPubForClient);
				if( nPubForClientSize<=0 )
				{
					throw	1;
				}
				nRearSize	-= sizeof(CONNECT_PUBKEY_LEN_T)+nPubForClientSize;
				if( nRearSize<0 )	// ��ʵ�������Ӧ�õ���0
				{
					throw	2;
				}
				// ������Կ
				if( m_pSlot->pIKeyExchanger->Agree2(pPubForClient, nPubForClientSize)<0 )
				{
					throw	3;
				}
				// ����Key��MD5
				whmd5(m_pSlot->pIKeyExchanger->GetKey(), m_pSlot->pIKeyExchanger->GetKeySize(), m_pSlot->szKeyMD5);
				// ��ʼ������channel�ļ�����
				m_pSlot->nCryptorType	= pCmdAgree->cryptortype;
				InitChannelCryptor(m_pSlot);
				// ����NOOP��ʱ(�ٺ���)
				RegNoopTQ(m_pSlot, m_tickNow+m_nNoopTimeout);
				// ���ö��߳�ʱ
				RegDropTQ(m_pSlot);
				// ��������л���ַ�������á�Ҳ����ߡ���ʱ
				if( m_info.bCanSwitchAddr )
				{
					RegMightDropTQ(m_pSlot, m_tickNow+m_nMightDropTimeout);
				}
				// ֪ͨ�ϲ����ӳɹ�
				CONTROL_T	*pCOCmd;
				ControlOutAlloc(pCOCmd);
				if( pCOCmd )
				{
					pCOCmd->nCmd	= CONTROL_OUT_SLOT_CONNECTED;
					pCOCmd->data[0]	= m_pSlot->slotinfo.nSlot;
				}
			}
			catch( ... )
			{
				// Ӧ����hack�����ݣ��ر�slot
				ControlOutCloseAndRemoveSlot(m_pSlot, SLOTCLOSE_REASON_KEYNOTAGREE);
				return	0;
			}
		}
		break;
		case	CNL2SLOTINFO_T::STATUS_WORKING:
		{
			// �Ѿ����빤��״̬��˵��ԭ���Ѿ��յ���AGREE���ˡ�
		}
		break;
		default:
		{
			// ���ǲ�Ӧ�õġ������к���Ķ����ˡ�(�����˳����Ȼ)
			return	-1;
		}
		break;
	}
	// ��tick��Ҫ��������Noop�����öԷ�֪�����ӳɹ���
	m_pSlot->nNoopCount			= 0;
	SendNoopThisTick(m_pSlot);
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CONNECT_REFUSE()
{
	CNL2_CMD_CONNECT_REFUSE_T	*pCmd	= (CNL2_CMD_CONNECT_REFUSE_T *)m_pCmd;
	ControlOutConnectRefuseAndRemoveSlot(m_pSlot, pCmd->reason);
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CLOSE()
{
	// �����ø�������
	CNL2_CMD_CLOSE_T	*pCmd	= (CNL2_CMD_CLOSE_T *)m_pCmd;
	m_pSlot->nCloseExtDataRecv	= pCmd->nExtData;
	// ���̷���һ��CLOSE
	SendClose(m_pSlot);
	// ��������
	m_pSlot->nCloseCount	++;
	if( m_pSlot->nCloseCount == 1 )
	{
		// ˵��Count������0
		// �����Ǳ����ر�
		SetCloseStatus(m_pSlot, true);
	}
	else	// ��һ����>=2��
	{
		// ���Խ�����
		int	nReason	= SLOTCLOSE_REASON_INITIATIVE;
		if( m_pSlot->bClosePassive )
		{
			nReason	= SLOTCLOSE_REASON_PASSIVE;
		}
		ControlOutCloseAndRemoveSlot(m_pSlot, nReason);
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_CONFIRM()
{
	CNL2_CMD_DATA_CONFIRM_T	*pCmd	= (CNL2_CMD_DATA_CONFIRM_T *)m_pCmd;
	int		nSize			= ((int)m_vectrawbuf.size()) - sizeof(*pCmd);
	if( nSize<0 )
	{
		// ���������
		CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_SIZE_SMALL, nSize);
		return	-1;
	}
	// �ж��Ƿ��Ѿ�û�к�������
	if( nSize == 0 )
	{
		return	0;
	}
	void	*pConfirmData		= wh_getptrnexttovar(pCmd->channelmask);
	// ����ÿ��channel��confirm��Ϣ
	int	i;
	CNL2_CMD_DATA_CONFIRM_T::CONFIRM_T	*pConfirmHdr;
	for(i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		if( whbit_uchar_chk(&pCmd->channelmask, i) )
		{
			switch( m_pSlot->channel[i].nType )
			{
				case	CNL2_CHANNELTYPE_SAFE:
				case	CNL2_CHANNELTYPE_LASTSAFE:
				break;
				default:
					// ���ݴ���
					CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_BADCHANNELTYPE, m_pSlot->channel[i].nType);
					return	-2;
				break;
			}
			// �ֽ����ȷ�����ݣ���������ز���
			if( nSize<(int)sizeof(*pConfirmHdr) )
			{
				// ���ݴ���
				CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_HDR_SIZE_SMALL, nSize);
				return	-3;
			}
			wh_settypedptr(pConfirmHdr, pConfirmData);
			nSize			-= sizeof(*pConfirmHdr);
			pConfirmData	= wh_getoffsetaddr(pConfirmData, sizeof(*pConfirmHdr));
			// �ȸ��ݵ�һidx����ȷ�ϸ���
			// ֻ�ж���cnl2idx_t�������Զ�����
			cnl2idx_t	nIdx	= pConfirmHdr->nInIdxConfirm;
			if( idxdiff(nIdx, pConfirmHdr->nInIdx0) < 0 )
			{
				// ������������(������nB_InConfirmIdx��nB_InIdx0֮ǰ��������һ����hack)
				CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_IDX_SMALL, nIdx);
				return	-4;
			}
			SlotChannelConfirm(m_pSlot, i, nIdx, 0);
			// ������е�ͷ�������ƶ���pConfirmHdr->nInIdx0
			SlotOutChannelMove(m_pSlot, i, pConfirmHdr->nInIdx0);
			//
			unsigned short	n;
			bool	bIsGot	= true;
			int		nCount	= 0;
			while( nSize>0 )
			{
				int	nVNSize	= whbit_vn1_get(pConfirmData, &n);
				if( nVNSize<=0 )
				{
					break;
				}
				// ���ȸı�
				nSize			-= nVNSize;
				// ָ�����
				pConfirmData	= wh_getoffsetaddr(pConfirmData, nVNSize);
				// ���������ȷ�����confirm
				if( nSize>=0 )
				{
					if( n==0 )
					{
						// ˵���������channelȷ�����ݵĽ�β
						break;
					}
					bIsGot	= !bIsGot;
					if( bIsGot )
					{
						// ������Ӧ����m_pSlot->channel[i].pInQueue->GetMaxUnit()/2������һ�㲻������hack
						if( ++nCount > m_pSlot->channel[i].pInQueue->GetMaxUnit() )
						{
							// ˵��������hack���ݣ���Ϊ���ᷢ����ô�����
							// �����Ҳ˵��������̫�����ˣ�Ӧ�ö�����
							CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_COUNT_BIG, nSize);
							return	-5;
						}
						// ˵�����յ��Ĳ���
						// ��������ȷ�ϱ�ע
						SlotChannelConfirm(m_pSlot, i, nIdx, n);
					}
					nIdx	+= n;
				}
			}
		}
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::_TickRecv_FIRST_CMD_ON_ACCEPTING()
{
	// ״̬��Ϊconnected
	m_pSlot->slotinfo.nStatus	= CNL2SLOTINFO_T::STATUS_WORKING;
	// �������ʱ���¼�
	m_pSlot->ClearAllTE();
	// ���ö��߳�ʱ
	RegDropTQ(m_pSlot);
	// ֪ͨ�ϲ�(ControlOut��֪�ϲ�)
	CONTROL_T	*pCmd;
	ControlOutAlloc(pCmd);
	if( pCmd )
	{
		pCmd->nCmd		= CONTROL_OUT_SLOT_ACCEPTED;
		pCmd->data[0]	= m_pSlot->slotinfo.nSlot;
	}
	// ��������Ϊ������
	m_pSlot->dealfuncRecv[CNL2_CMD_DATA]		= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA;
	m_pSlot->dealfuncRecv[CNL2_CMD_NOOP]		= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP;
	// ��ʼ������channel�ļ�����
	InitChannelCryptor(m_pSlot);
	// ��������л���ַ�������á�Ҳ����ߡ���ʱ
	if( m_info.bCanSwitchAddr )
	{
		RegMightDropTQ(m_pSlot, m_tickNow+m_nMightDropTimeout);
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA()
{
	// ����channel������ͬ�Ĵ�����
	m_pCmdData	= (CNL2_CMD_DATA_T *)m_pCmd1;
	m_pData		= wh_getptrnexttoptr(m_pCmdData);
	m_nDataSize	= m_nCmdSize - sizeof(*m_pCmdData);
	if( m_nDataSize<=0 )
	{
		CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_SIZE);
		return	-1;
	}
	// ���dataphase���ԣ��͹ر�
	if( m_pCmdData->prop & CNL2_CMD_DATA_T::PROP_ALLNEG )
	{
		CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONTENT);
		return	-2;
	}
	m_pChannel	= &m_pSlot->channel[m_pCmdData->channel];
	return	(this->*m_TickRecv_CMD_Others_DATA_Deal[m_pCmdData->channel])();
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_ON_ACCEPTING()
{
	// �Ƚ���״̬ת��
	_TickRecv_FIRST_CMD_ON_ACCEPTING();
	// ����������
	TickRecv_CMD_Others_DATA();
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_NotSupport()
{
	// ��hacker���ر�����
	CloseOnErr(m_pSlot, CLOSEONERR_TickRecv_CMD_Others_DATA_Deal_NotSupport);
	return	-1;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_RT()
{
	CNL2SLOT_I::CHANNEL_T	&channel	= *m_pChannel;
	// ֱ�Ӱ�˳�������ն���
	CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= channel.pInQueue->Alloc();
	if( !pInUnit )
	{
		// ������������������ݣ�����һ�˰����ͷ���
		return	0;
	}
	// ��unitӦ�����ϴ�free��ʱ���Ѿ�������˲���Ϊ�˱��ջ�����clearһ��
	pInUnit->clear(this);
	SetChannelDataUnit(pInUnit, m_nDataSize, m_pData);
	pInUnit->dataphase	= m_pCmdData->dataphase;
	pInUnit->prop		= m_pCmdData->prop;
	// ���������ݵĶ������ϲ���
	ChannelHaveDataSoAddToDList(m_pSlot, &channel);
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_Safe()
{
	CNL2SLOT_I::CHANNEL_T	&channel			= *m_pChannel;
	CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= NULL;
	// �Ƚ�������ݺ���ʼ��Ĳ��
	int	nDiff	= idxdiff(m_pCmdData->nIdx, channel.nInIdx0);
	if( nDiff>=channel.pInQueue->GetMaxUnit() )
	{
		// ������ų���
		CloseOnErr(m_pSlot, CLOSEONERR_IdxOutOfRange);
		return	-1;
	}
	if( nDiff<0 )
	{
		// ̫�ϵ����ݣ����ԡ����ǻ���Ҫ����ȷ�ϣ���Ϊ�����ǶԷ�û���յ�ȷ�ϵ��µ��ط���
		goto	end;
	}
	if( nDiff<channel.pInQueue->size() )
	{
		pInUnit	= channel.pInQueue->Get(nDiff);
		// ����ֱ�ӷ���
		if( pInUnit->pData )
		{
			// ������Ѿ��յ��������ˣ���Ҫ���
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "��ʱ��־,ALREADY RECVED,idx:%d,size:%d,this:%p,ip:%s", m_pCmdData->nIdx, m_nDataSize, this, cmn_get_ipnportstr_by_saaddr(&m_addr));
			pInUnit	= NULL;
		}
	}
	else
	{
		// ��չ����
		int	nExtent	= nDiff - channel.pInQueue->size() + 1;
		while( nExtent>0 )
		{
			pInUnit	= channel.pInQueue->Alloc();
			assert(pInUnit);
			// Ϊ�˱�����clearһ��
			pInUnit->clear(this);
			nExtent	--;
		}
		// ����pInUnit����������Ҫ��
	}
	// ���Է������ݣ����������������н��ܣ���Ҫ����˳��⣬������ܻ��Ƚ��˺�������ܣ����¼ӽ���˳��ͬ�Ĵ���
	if( pInUnit )
	{
		assert( channel.pICryptorRecv );
		// �ȿ�������
		SetChannelDataUnit(pInUnit, m_nDataSize, m_pData);
		pInUnit->dataphase	= m_pCmdData->dataphase;
		pInUnit->prop		= m_pCmdData->prop;
		// ����nInIdxConfirm
		if( m_pCmdData->nIdx == channel.nInIdxConfirm )
		{
			// ���������һֱ�ڵȴ����Ǹ�idx
			int	nIdx	= idxdiff(channel.nInIdxConfirm, channel.nInIdx0);
			// �������ж��������յ������ݣ�����Щ���������ݽ��н��ܣ�
			do
			{
				// ����
				CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= channel.pInQueue->Get(nIdx);
				assert(pInUnit);
				channel.pICryptorRecv->Decrypt(pInUnit->pData, pInUnit->nSize, pInUnit->pData);
				// ��һ��
				channel.nInIdxConfirm	++;
				if( ( ++nIdx >= (int)channel.pInQueue->size() ) )
				{
					// ���˽�β��
					break;
				}
				// �Ǿ�һ����ȡ��nIdx�±�Ķ������
				if( channel.pInQueue->Get(nIdx)->pData == NULL )
				{
					// �����û�б�ȷ�ϣ�����
					break;
				}
			}while(1);
			// ����������¾Ϳ��Է��������ݵĶ������ϲ���
			ChannelHaveDataSoAddToDList(m_pSlot, &channel);
		}
	}
end:
	// �������channel��Ҫȷ��
	whbit_uchar_set(&m_pSlot->nPeerNotConfirmedChannelMask, channel.nChannel);
	// ������Σ�ֻҪ�յ����ݣ�����������tick����ȷ�ϰ�
	SendConfirmThisTick(m_pSlot);

	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_LastSafe()
{
	// ����������Զ��ֻ�����һ��Ϊ׼������ǰ����û��ȡ�����ݾ�ֱ���滻��
	CNL2SLOT_I::CHANNEL_T	&channel			= *m_pChannel;
	CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= NULL;
	// �Ƚ�������ݺ���ʼ��Ĳ��
	int	nDiff	= idxdiff(m_pCmdData->nIdx, channel.nInIdx0);
	if( nDiff<0 )
	{
		// û�õ�����(�ȷ�����)
		if( nDiff == -1 )
		{
			// ���ǲ�-1�����������Ϊû���յ�ȷ����ɵģ�����Ӧ���������ĸ�ֵ��
			// ������Ҫ�ٴη���ȷ��
			goto	end;
		}
		return	0;
	}
	// �ж��Ƿ��Ѿ��������ˣ�����о���ɾ��
	pInUnit	= channel.pInQueue->GetFirst();
	if( pInUnit )
	{
		if( nDiff == 0 )
		{
			// �Ѿ��յ������ݣ���ô���ٴη���ȷ�ϼ���
			goto	end;
		}
		// ����˵���������ݣ���ɾ��������
		pInUnit->clear(this);
	}
	else
	{
		// �����µ�unit
		pInUnit	= channel.pInQueue->Alloc();
	}
	assert(pInUnit);

	SetChannelDataUnit(pInUnit, m_nDataSize, m_pData);
	pInUnit->dataphase	= m_pCmdData->dataphase;
	pInUnit->prop		= m_pCmdData->prop;
	// ���������ݵĶ������ϲ���
	ChannelHaveDataSoAddToDList(m_pSlot, &channel);
	// ����nInIdx0��nInIdxConfirm
	// �������յ����������Ϊ׼����Ϊ��ֻҪһ���������Ȼ��Խ��Խ�ã�
	channel.nInIdx0			= m_pCmdData->nIdx;
	channel.nInIdxConfirm	= m_pCmdData->nIdx + 1;

end:
	// �������channel��Ҫȷ��
	whbit_uchar_set(&m_pSlot->nPeerNotConfirmedChannelMask, channel.nChannel);
	// ������Σ�ֻҪ�յ����ݣ�����������tick����ȷ�ϰ�
	SendConfirmThisTick(m_pSlot);

	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP_ON_ACCEPTING()
{
	// �Ƚ���״̬ת��
	_TickRecv_FIRST_CMD_ON_ACCEPTING();
	// ����������
	TickRecv_CMD_Others_NOOP();
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP()
{
	CNL2_CMD_NOOP_T	*pCmd	= (CNL2_CMD_NOOP_T *)m_pCmd;
	int		nSize			= ((int)m_vectrawbuf.size()) - sizeof(*pCmd);
	if( nSize<0 )
	{
		// ���������
		CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_NOOP);
		return	-1;
	}
	if( pCmd->nCount>0 )
	{
		// �����µ�pingֵ
		int	nDiff	= wh_tickcount_diff(m_tickNow, pCmd->nPeerSendTime);
		m_pSlot->slotinfo.calcRTT(nDiff);
#ifdef	_DEBUG
#ifdef	_WH_LOWLEVELDEBUG
		if( nDiff>500 )
		{
			WHTMPPRINTF("Diff:%d too big!%s", nDiff, WHLINEEND);
			//assert(0);
		}
#endif
#endif
	}
	if( pCmd->nCount<2 )
	{
		// ���ش���������2�����Է��ͷ���
		m_pSlot->nPeerNoopSendTime	= pCmd->nSendTime;
		m_pSlot->nNoopCount			= pCmd->nCount+1;
		SendNoopThisTick(m_pSlot);
	}
	return	0;
}
