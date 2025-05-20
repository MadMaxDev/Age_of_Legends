// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i_Recv.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CNL2���ڲ�ʵ��
// CreationDate : 2005-05-12
// ChangeLog    : 

#include "../inc/whnetcnl2_i.h"
#include <WHCMN/inc/whbits.h>

using namespace n_whnet;
using namespace n_whcmn;

// �Ƿ���Ҫ��Recv�д���ȷ�ϣ��Ա���In������֮ǰ����Recv�����·��ͷ���Զ�޷����͵������
// ��������ÿ�ν������ݵ�ʱ���һ��㣬�����ɿ���
#define	CNL2_RECV_TRIGER_CONFIRM

int		CNL2SlotMan_I_UDP::Recv(int *pnSlot, int *pnChannel, void **ppData, size_t *pnSize)
{
havepack:
	if( m_pack_info.nSlot>0 )
	{
		*pnSlot		= m_pack_info.nSlot;
		*pnChannel	= m_pack_info.nChannel;
		int	nSize	= 0;
		try
		{
			// �����ߴ�
			m_pack_info.wcs.ReadVSize(&nSize);
			*pnSize	= nSize;
			// �õ�����ָ��
			*ppData	= m_pack_info.wcs.GetCurPtr();
			// ��������
			m_pack_info.wcs.Seek(nSize);
			// ����Ѿ�������������slotΪ0����ʾ���Լ�����������ȡ����İ���
			if( m_pack_info.wcs.GetSizeLeft()==0 )
			{
				m_pack_info.nSlot	= 0;
			}
		}
		catch(const char *cszErr)
		{
			// ������û�����
			GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(5,CNL_RT)"CNL2SlotMan_I_UDP::Recv,%s,BAD PACK DATA,0x%X %d %s", cszErr, *pnSlot, *pnChannel, cmn_get_ipnportstr_by_saaddr(&m_addr));
			Close(*pnSlot, 0);
			// ͬʱֱ�ӷ����������������
			m_pack_info.nSlot	= 0;
			return	-1;
		}
		// ���ء���ȡ�ɹ���
		if( m_info.bLogSendRecv )
		{
			CNL2SLOTINFO_T	*pInfo	= GetSlotInfo(*pnSlot);
			const char		*pszIP	= cmn_get_ipnportstr_by_saaddr(&pInfo->peerAddr);
			if( !pszIP )
			{
				pszIP				= "no ip";
			}
			size_t	nNSize	= *pnSize<64 ? *pnSize:64;
			char	buf[1024];
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,TEMP)"CNL RECV,%s,%d,%d,%s", pszIP, *pnChannel, *pnSize, wh_hex2str((const unsigned char *)(*ppData), nNSize, buf, 1, true));
		}
		return	0;
	}

	// ��������ݵĶ���
	// �ҵ���ȷ�����ݵĵ�һ��
	whDList<CNL2SLOT_I *>			&DL	= m_dlistSlot[CNL2SLOT_I::DLISTIDX_HAVINGDATA];
	whDList<CNL2SLOT_I *>::node	*pNode	= DL.begin();
	while(pNode!=DL.end())
	{
		CNL2SLOT_I	*pSlot	= pNode->data;
		// �쿴����channel���Ƿ������ݣ�����͵�����ǰ����
		try
		{
			whDList<CNL2SLOT_I::CHANNEL_T *>::node	*pCurNode	= pSlot->dlistChannelWithData.begin();
			while( pCurNode != pSlot->dlistChannelWithData.end() )
			{
				// ���Ƿ�������(�оͿ���ֱ�ӷ�����)
				m_vectrawbuf_recv.clear();
				CNL2SLOT_I::CHANNEL_T	&channel			= *pCurNode->data;

				// RT��SAFE�������Ҫ���ջ��������
				CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit;
				channel.pInQueue->BeginGet();
				// ��һ��
				if( (pInUnit=channel.pInQueue->GetNext())==NULL || pInUnit->pData==NULL )
				{
					// û�����ݣ������������Ϊ�Ƚ��յ��˺����safe����
					// �����������channelû�пɶ�����
				}
				else
				{
					int		nCount		= 1;
					bool	bHaveData	= false;
					// ��һ�����뻺��
					m_vectrawbuf_recv.pushn_back((char *)pInUnit->pData, pInUnit->nSize);
					// ������һ���ǲ���single
					if( pInUnit->dataphase == CNL2_CMD_0_T::DATAPHASE_SINGLE )
					{
						// ���Խ�����
						bHaveData		= true;
					}
					else
					{
						// ��һ���������ǵ��ڵİ�
						if( pInUnit->dataphase != CNL2_CMD_0_T::DATAPHASE_HEAD )
						{
							throw	(int)CLOSEONERR_BAD_DATA_CONTENT;
						}
						while( (pInUnit=channel.pInQueue->GetNext())!=NULL && pInUnit->pData )
						{
							// ���治������head��
							if( pInUnit->dataphase == CNL2_CMD_0_T::DATAPHASE_HEAD )
							{
								throw	(int)CLOSEONERR_BAD_DATA_CONTENT;
							}
							nCount		++;
							m_vectrawbuf_recv.pushn_back((char *)pInUnit->pData, pInUnit->nSize);
							// �ж��Ƿ��ǽ�β
							if( pInUnit->dataphase == CNL2_CMD_0_T::DATAPHASE_TAIL )
							{
								// ���Խ�����
								bHaveData	= true;
								break;
							}
							// ����ǷǴ�����ݣ����ж�һ�������Ƿ񳬳�
							if( (pInUnit->prop&CNL2_CMD_DATA_T::PROP_PACK) == 0 )
							{
								// �ж�������ݳ�����ر�slot
								if( (int)m_vectrawbuf_recv.size() >= m_info.nMaxSinglePacketSize )
								{
									throw	(int)CLOSEONERR_BAD_DATA_SIZE;
								}
							}
						}
					}
					if( bHaveData )
					{
						// �����ƶ�, �������
						SlotInChannelMove(&channel, nCount);
#ifdef	CNL2_RECV_TRIGER_CONFIRM
						// ���ø�channel��ҪConfirm
						whbit_uchar_set(&pSlot->nPeerNotConfirmedChannelMask, channel.nChannel);
						// ����Slot��ҪConfirm 
						SendConfirmThisTick(pSlot);
#endif
						// ����Ǵ�������򷵻غ���ͷ����������ڶ�ε����ݣ�ÿ��unit��Ӧ����prop��
						if( (pInUnit->prop&CNL2_CMD_DATA_T::PROP_PACK) != 0 )
						{
							m_pack_info.nSlot		= pSlot->slotinfo.nSlot;
							m_pack_info.nChannel	= channel.nChannel;
							m_pack_info.wcs.SetShrink(m_vectrawbuf_recv.getbuf(), m_vectrawbuf_recv.size());
							goto	havepack;
						}
						// �����ֱ�Ӱ��ռ����ݷ��ظ��ϲ�
						*pnSlot		= pSlot->slotinfo.nSlot;
						*pnChannel	= channel.nChannel;
						*ppData		= m_vectrawbuf_recv.getbuf();
						*pnSize		= m_vectrawbuf_recv.size();

						if( m_info.bLogSendRecv )
						{
							const char		*pszIP	= cmn_get_ipnportstr_by_saaddr(&pSlot->slotinfo.peerAddr);
							if( !pszIP )
							{
								pszIP				= "no ip";
							}
							size_t	nNSize	= *pnSize<64 ? *pnSize:64;
							char	buf[1024];
							GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,TEMP)"CNL RECV,%s,%d,%d,%s", pszIP, *pnChannel, *pnSize, wh_hex2str((const unsigned char *)(*ppData), nNSize, buf, 1, true));
						}
						// �ɹ�����
						return	0;
					}
				}
				// û�����ݾͿ���һ��
				pCurNode->leave();
				pCurNode	= pSlot->dlistChannelWithData.begin();
			}
		}
		catch( int nErrToClose )
		{
			CloseOnErr(pSlot, nErrToClose);
		}
		// �ϵ����
		pNode->leave();
		// ��ͷ����µ�
		pNode	= DL.begin();
	}
	// û������
	return	-1;
}
