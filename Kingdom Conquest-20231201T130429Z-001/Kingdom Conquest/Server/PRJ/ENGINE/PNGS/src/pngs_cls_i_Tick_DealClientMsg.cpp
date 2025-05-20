// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick_DealClientMsg.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CLS中Tick_DealClientMsg部分的具体实现
// CreationDate : 2005-08-15
// Change LOG   :

#include "../inc/pngs_cls_i.h"

using namespace n_pngs;

int		CLS2_I::Tick_DealClientMsg()
{
	m_tickNow	= wh_gettickcount();

	// 看客户端的来来去去
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
				// 其他的应该不会收到的
			}
			break;
		}
	}

	// 处理客户端发来的指令
	while( m_pSlotMan4Client->Recv(&nSlot, &nChannel, (void **)&pClientCmd, &nSize)==0 )
	{
		// 找到相关的用户
		PlayerUnit	*pPlayer	= GetPlayerBySlot(nSlot);
		if( !pPlayer )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(634,PNGS_RT)"CLS2_I::Tick_DealClientMsg,,Player missing for slot,0x%X", nSlot);
			// 玩家已经不存在，这个slot也应该关闭了
			// 这个在玩家被Remove但是对方slot中还有东西发来的情况下会出现
			m_pSlotMan4Client->Close(nSlot);
			continue;
		}
		// 看指令尺寸是否正常
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
				// 不支持的指令，关闭slot
				GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(636,PNGS_RT)"CLS2_I::Tick_DealClientMsg,%d,unknown ClientCmd,0x%X,0x%X,%d"
					, *pClientCmd, nSlot, pPlayer->nID, nSize
					);
				pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
				RemovePlayerUnit(pPlayer);
			break;
		}
		// 不用释放指令，因为Recv内部是用一个临时缓冲来存放pData的
	}

	return	0;
}

int		CLS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize)
{
	int	nSlot	= pCOCmd->data[0];
	// 连接的附加参数中应该有附加参数
	whvector<char>	*pvectExtInfo	= m_pSlotMan4Client->GetConnectExtData(nSlot);
	if( !pvectExtInfo )
	{
		// 在同一个tick内就被删除了？好奇怪啊。应该不可能因为关闭没有可能那么快的
		// 2008-07-10 现实证明可能在上个一个tick内出现这种情况。可能是由于对方想破解协议造成的。发过来的第一个包或者若干个包非法。CNL先认为收到第一个包进入了ACCEPTED状态，然后认为包非法就把SLOT关闭了
		//assert(0);
		return	0;
	}
	if( pvectExtInfo->size()!=sizeof(CLIENT_CLS_CONNECT_EXTINFO_T) )
	{
		// 直接关闭这个连接
		GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(637,PNGS_RT)"CLS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED,,ext info bad size,%d,%d", pvectExtInfo->size(),sizeof(CLIENT_CLS_CONNECT_EXTINFO_T));
		m_pSlotMan4Client->Close(nSlot, CLOSE_PARAM_TO_CLIENT_BADCMD);
		return	0;
	}
	CLIENT_CLS_CONNECT_EXTINFO_T	*pExtInfo	= (CLIENT_CLS_CONNECT_EXTINFO_T *)pvectExtInfo->getbuf();
	// 找到玩家对象
	PlayerUnit	*pPlayer	= m_Players.getptr(pExtInfo->nClientID);
	if( !pPlayer )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(638,PNGS_RT)"CLS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED,,clientid not exist,0x%X", pExtInfo->nClientID);
		m_pSlotMan4Client->Close(nSlot, CLOSE_PARAM_TO_CLIENT_NOIDINCLS);
		return	0;
	}
	// 如果原来状态不是连接状态，则这个连接直接关闭
	if( pPlayer->nStatus != PlayerUnit::STATUS_CONNECTING )
	{
		m_pSlotMan4Client->Close(nSlot, CLOSE_PARAM_TO_CLIENT_PLAYERSTATUSNOTCONNECTING);
		return	0;
	}
	// 比较密码
	if( pPlayer->nPasswordToConnectCLS != pExtInfo->nPassword )
	{
		m_pSlotMan4Client->Close(nSlot, CLOSE_PARAM_TO_CLIENT_BADCMD);
		return	0;
	}
	// 身份确认成功
	// 清除超时事件
	pPlayer->teid.quit();
	// 关联slot和player
	pPlayer->nSlot		= nSlot;
	m_pSlotMan4Client->SetExtLong(nSlot, pPlayer->nID);
	// 设置工作状态
	pPlayer->nStatus	= PlayerUnit::STATUS_WORKING;
	// 发送消息给GMS说用户已经连接成功了
	CLS_GMS_CLIENTTOCLS_RST_T	CTCRst;
	CTCRst.nCmd			= CLS_GMS_CLIENTTOCLS_RST;
	CTCRst.nRst			= CLS_GMS_CLIENTTOCLS_RST_T::RST_OK;
	CTCRst.nClientID	= pPlayer->nID;
	m_msgerGMS.SendMsg(&CTCRst, sizeof(CTCRst));

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(639,PNGS_RT)"CLS2_I Player accepted,0x%X,%d", pPlayer->nID, m_Players.size());

	// 生成检查hack的定时事件
	SetTEDeal_nCheckRecvInterval(pPlayer);

	return	0;
}
int		CLS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_CLOSED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize)
{
	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T	*pCOCmdReason	= (CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T *)pCOCmd;
	// 找到对应的Player，移除它 
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
		// 不允许有这样的数据
		pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
		RemovePlayerUnit(pPlayer);
		return	0;
	}

	// 计数器增加
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
			// 这边就应该是
			GZSUnit	*pGZS	= NULL;
			if( pCCData->nSvrIdx == PNGS_SVRIDX_AUTOGZS )
			{
				// 根据用于自己记录的GZSID获得pGZS
				pGZS		= m_GZSs + pPlayer->nSvrIdx;
			}
			else
			{
				if( pCCData->nSvrIdx>=GZS_MAX_NUM )
				{
					// 这是客户端的hack
					pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
					RemovePlayerUnit(pPlayer);
					return	0;
				}
				else
				{
					pGZS		= m_GZSs + pCCData->nSvrIdx;
				}
			}
			// 如果用户连着GZS且GZS可以工作，则发送，否则就忽略
			if( pGZS && pGZS->nStatus==GZSUnit::STATUS_WORKING )
			{
				pGZS->msgerGZS.SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
		}
		break;
	}
	return	0;
}
