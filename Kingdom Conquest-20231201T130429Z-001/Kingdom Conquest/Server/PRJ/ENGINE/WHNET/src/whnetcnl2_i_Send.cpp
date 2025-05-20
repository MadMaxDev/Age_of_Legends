// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i_Send.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CNL2的内部实现
// CreationDate : 2005-05-13
// ChangeLog    : 2005-09-21 Send_DATA_NotSupport中取消assert改为日志

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
	// 形成RT包发出
	m_vectrawbuf.resize(sizeof(CNL2_CMD_DATA_T) + nSize);
	CNL2_CMD_DATA_T	*pCmdData	= (CNL2_CMD_DATA_T *)m_vectrawbuf.getbuf();
	pCmdData->cmd		= CNL2_CMD_DATA;
	pCmdData->dataphase	= nDataPhase;
	pCmdData->prop		= prop;
	pCmdData->channel	= pChannel->nChannel;
	pCmdData->slotidx	= pSlot->slotinfo.nPeerSlotIdx;
	pCmdData->nIdx		= nIdx;
	if( pChannel->pICryptorSend )	// 注意RT包这个是不回有的
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
	// 把数据加入channel列表
	m_pChannel->AddCmdPackUnit(pPackUnitNode);
	return	0;
}
int		CNL2SlotMan_I_UDP::_Send_DATA_Safe(const void *pData, size_t nSize, unsigned char nProp)
{
	// 分段
	if( nSize<=CNL2_MAX_SINGLEDATASIZE )
	{
		// 判断输出缓冲是否还有位置
		if( m_pChannel->pOutQueue->GetAvailNum() == 0 )
		{
			return	-1;
		}
		// 直接发送
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
		// 判断输出缓冲是否还有位置（如果没有位置就都不发送，不会发送一部分的）
		if( nPart > m_pChannel->pOutQueue->GetAvailNum() )
		{
			return	-11;
		}
		// 循环分片发送
		// 头
		if( _Send_DATA_Safe_Single(pData, CNL2_MAX_SINGLEDATASIZE, CNL2_CMD_0_T::DATAPHASE_HEAD, nProp)<0 )
		{
			return	-12;
		}
		// 中间
		for(int i=1;i<nPart-1;i++)
		{
			pData	= wh_getoffsetaddr(pData, CNL2_MAX_SINGLEDATASIZE);
			if( _Send_DATA_Safe_Single(pData, CNL2_MAX_SINGLEDATASIZE, CNL2_CMD_0_T::DATAPHASE_SINGLE, nProp)<0 )
			{
				return	-13;
			}
		}
		// 最后
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
	// 能调用到这句则说明缓冲一定是够用的
	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= m_pChannel->pOutQueue->Alloc();
	assert(pOutUnit);
	pOutUnit->clear(this);
	AllocAndSetData(pOutUnit->pCmd, m_vectrawbuf.size(), m_vectrawbuf.getbuf());
	pOutUnit->nSize			= m_vectrawbuf.size();
	// 记录发送时刻
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
	// 判断原来是否有还没有确认的
	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= m_pChannel->pOutQueue->GetFirst();
	if( pOutUnit )
	{
		// 如果有
		// 删除重发时间事件
		pOutUnit->clear(this);
		// 清空队列
		m_pChannel->pOutQueue->Clean();
		// 首序号增加
		m_pChannel->nOutIdx0		++;
		// 设置nOutIdxConfirm和nOutIdx0相等表示这个数据还没有被确认
		m_pChannel->nOutIdxConfirm	= m_pChannel->nOutIdx0;
	}
	// 加入队列
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
	pOutUnit->nRSCount	= 1;	// 注意这个默认初始值必须是1
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
	pOutUnit->nRSCount	= 1;	// 注意这个默认初始值必须是1
	whtick_t	t		= m_tickNow+nDelay;
	if( pOutUnit->nTimeEventID.IsValid() )
	{
		// 原定时间先于现在期望的时间
		if( nDelay>0 && wh_tickcount_diff(pOutUnit->nTimeEventID.t, t)<0 )
		{
			// 使用原定时间
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
		wh_randhex(CmdSA.szKeyMD5, sizeof(CmdSA.szKeyMD5));		// 这个只是为了迷惑敌人
	}
	CmdSA.calcsetcrc(sizeof(CmdSA));
	// 发送请求
	udp_sendto(&CmdSA, sizeof(CmdSA), &m_addr);
}
