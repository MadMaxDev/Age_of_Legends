// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i_Recv.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CNL2的内部实现
// CreationDate : 2005-05-12
// ChangeLog    : 

#include "../inc/whnetcnl2_i.h"
#include <WHCMN/inc/whbits.h>

using namespace n_whnet;
using namespace n_whcmn;

// 是否需要在Recv中触发确认（以避免In队列满之前都不Recv而导致发送发永远无法发送的情况）
// 这样会在每次接收数据的时候费一点点，不过可靠。
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
			// 读出尺寸
			m_pack_info.wcs.ReadVSize(&nSize);
			*pnSize	= nSize;
			// 得到数据指针
			*ppData	= m_pack_info.wcs.GetCurPtr();
			// 跳过数据
			m_pack_info.wcs.Seek(nSize);
			// 如果已经读完了则，设置slot为0，表示可以继续从网络收取后面的包了
			if( m_pack_info.wcs.GetSizeLeft()==0 )
			{
				m_pack_info.nSlot	= 0;
			}
		}
		catch(const char *cszErr)
		{
			// 踢这个用户下线
			GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(5,CNL_RT)"CNL2SlotMan_I_UDP::Recv,%s,BAD PACK DATA,0x%X %d %s", cszErr, *pnSlot, *pnChannel, cmn_get_ipnportstr_by_saaddr(&m_addr));
			Close(*pnSlot, 0);
			// 同时直接放弃这个的所有数据
			m_pack_info.nSlot	= 0;
			return	-1;
		}
		// 返回。读取成功。
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

	// 浏览有数据的队列
	// 找到的确有数据的第一个
	whDList<CNL2SLOT_I *>			&DL	= m_dlistSlot[CNL2SLOT_I::DLISTIDX_HAVINGDATA];
	whDList<CNL2SLOT_I *>::node	*pNode	= DL.begin();
	while(pNode!=DL.end())
	{
		CNL2SLOT_I	*pSlot	= pNode->data;
		// 察看所有channel看是否有数据，如果就弹出当前数据
		try
		{
			whDList<CNL2SLOT_I::CHANNEL_T *>::node	*pCurNode	= pSlot->dlistChannelWithData.begin();
			while( pCurNode != pSlot->dlistChannelWithData.end() )
			{
				// 看是否有数据(有就可以直接返回了)
				m_vectrawbuf_recv.clear();
				CNL2SLOT_I::CHANNEL_T	&channel			= *pCurNode->data;

				// RT和SAFE数据如果要接收缓冲则必须
				CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit;
				channel.pInQueue->BeginGet();
				// 第一个
				if( (pInUnit=channel.pInQueue->GetNext())==NULL || pInUnit->pData==NULL )
				{
					// 没有数据，这个可能是因为先接收到了后面的safe数据
					// 但是现在这个channel没有可读数据
				}
				else
				{
					int		nCount		= 1;
					bool	bHaveData	= false;
					// 第一个进入缓冲
					m_vectrawbuf_recv.pushn_back((char *)pInUnit->pData, pInUnit->nSize);
					// 看看第一个是不是single
					if( pInUnit->dataphase == CNL2_CMD_0_T::DATAPHASE_SINGLE )
					{
						// 可以结束了
						bHaveData		= true;
					}
					else
					{
						// 第一个包必须是等于的包
						if( pInUnit->dataphase != CNL2_CMD_0_T::DATAPHASE_HEAD )
						{
							throw	(int)CLOSEONERR_BAD_DATA_CONTENT;
						}
						while( (pInUnit=channel.pInQueue->GetNext())!=NULL && pInUnit->pData )
						{
							// 里面不能再有head包
							if( pInUnit->dataphase == CNL2_CMD_0_T::DATAPHASE_HEAD )
							{
								throw	(int)CLOSEONERR_BAD_DATA_CONTENT;
							}
							nCount		++;
							m_vectrawbuf_recv.pushn_back((char *)pInUnit->pData, pInUnit->nSize);
							// 判断是否是结尾
							if( pInUnit->dataphase == CNL2_CMD_0_T::DATAPHASE_TAIL )
							{
								// 可以结束了
								bHaveData	= true;
								break;
							}
							// 如果是非打包数据，则判断一下数据是否超长
							if( (pInUnit->prop&CNL2_CMD_DATA_T::PROP_PACK) == 0 )
							{
								// 判断如果数据超长则关闭slot
								if( (int)m_vectrawbuf_recv.size() >= m_info.nMaxSinglePacketSize )
								{
									throw	(int)CLOSEONERR_BAD_DATA_SIZE;
								}
							}
						}
					}
					if( bHaveData )
					{
						// 队列移动, 清除数据
						SlotInChannelMove(&channel, nCount);
#ifdef	CNL2_RECV_TRIGER_CONFIRM
						// 设置该channel需要Confirm
						whbit_uchar_set(&pSlot->nPeerNotConfirmedChannelMask, channel.nChannel);
						// 设置Slot需要Confirm 
						SendConfirmThisTick(pSlot);
#endif
						// 如果是打包数据则返回函数头部（如果对于多段的数据，每个unit都应该有prop）
						if( (pInUnit->prop&CNL2_CMD_DATA_T::PROP_PACK) != 0 )
						{
							m_pack_info.nSlot		= pSlot->slotinfo.nSlot;
							m_pack_info.nChannel	= channel.nChannel;
							m_pack_info.wcs.SetShrink(m_vectrawbuf_recv.getbuf(), m_vectrawbuf_recv.size());
							goto	havepack;
						}
						// 否则就直接按照简单数据返回给上层
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
						// 成功返回
						return	0;
					}
				}
				// 没有数据就看下一个
				pCurNode->leave();
				pCurNode	= pSlot->dlistChannelWithData.begin();
			}
		}
		catch( int nErrToClose )
		{
			CloseOnErr(pSlot, nErrToClose);
		}
		// 老的离队
		pNode->leave();
		// 从头获得新的
		pNode	= DL.begin();
	}
	// 没有数据
	return	-1;
}
