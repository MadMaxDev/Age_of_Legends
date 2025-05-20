// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick_DealClientMsg.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CAAFS中Tick_DealClientMsg部分的具体实现
// CreationDate : 2005-08-03
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::Tick_DealClientMsg()
{
	m_tickNow	= wh_gettickcount();

	// 看客户端的来来去去
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
				// 其他的应该不会收到的
			}
			break;
		}
	}

	// 处理客户端发来的指令(客户端可能发来prelogin等指令)
	// 但是客户端不可能发送过多的指令（目前限制最多4条）
	// 处理客户端发来的指令
	int			nSlot;
	int			nChannel;
	pngs_cmd_t	*pClientCmd;
	while( m_pSlotMan4Client->Recv(&nSlot, &nChannel, (void **)&pClientCmd, &nSize)==0 )
	{
		// 找到相关的用户
		PlayerUnit	*pPlayer	= GetPlayerBySlot(nSlot);
		if( !pPlayer )
		{
			// 玩家已经不存在，这个slot也应该关闭了
			// 这个在玩家被Remove但是对方slot中还有东西发来的情况下会出现
			m_pSlotMan4Client->Close(nSlot);
			continue;
		}
		if( pPlayer->nRecvCmdNum++ > m_cfginfo.nMaxPlayerRecvCmdNum )
		{
			// 指令太多了，断开该用户
			m_pSlotMan4Client->Close(nSlot);
			continue;
		}
		if( nSize < sizeof(pngs_cmd_t) )
		{
			// 数据长度非法
			m_pSlotMan4Client->Close(nSlot);
			continue;
		}
		switch( *pClientCmd )
		{
			case	CLIENT_CAAFS_2GMS:
			{
				// 包装一下然后传给GMS
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
				// 指令非法
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
		// 很可能是处理过程中断了
		return	0;
	}
	if( pvectExt->size() != sizeof(CLIENT_CONNECT_CAAFS_EXTINFO_T) )
	{
		// 连接数据不对（可能是hacker吧）
		m_pSlotMan4Client->Close(pCOCmd->data[0], CLOSE_PARAM_TO_CLIENT_BADCMD);
		GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(556,PNGS_RT)"CAAFS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED,,connect extinfo size error,%d,%d", pvectExt->size(),sizeof(CLIENT_CONNECT_CAAFS_EXTINFO_T));
		return	0;
	}
	// 有连入的用户
	// 应该不会有分配错误，因为SlotMan的最多连接数和Players的总数一样
	PlayerUnit	*pPlayer;
	int			nID	= m_Players.AllocUnit(pPlayer);
	if( nID<0 )
	{
		// 错误不小，记录日志，断开用户
		m_pSlotMan4Client->Close(pCOCmd->data[0], CLOSE_PARAM_TO_CLIENT_SVRMEM);
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(557,PNGS_RT)"CAAFS2_I::Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED,,m_Players.AllocUnit ERROR");
		return	0;
	}


	CLIENT_CONNECT_CAAFS_EXTINFO_T	*pExtInfo	= (CLIENT_CONNECT_CAAFS_EXTINFO_T *)pvectExt->getbuf();
	// 设置数据
	pPlayer->nID				= nID;
	pPlayer->nSlot				= pCOCmd->data[0];
	pPlayer->IP					= m_pSlotMan4Client->GetSlotInfo(pPlayer->nSlot)->peerAddr.sin_addr.s_addr;
	// 和slot关联(记得RemovePlayerUnit的时候要清除这个关联)
	m_pSlotMan4Client->SetExtLong(pPlayer->nSlot, pPlayer->nID);
	// 加入vipchannel:0
	PutPlayerInVIPChannel(pPlayer, 0);
	// 设置为等待状态
	pPlayer->nStatus			= PlayerUnit::STATUS_WAITING;
	pPlayer->nNetworkType		= pExtInfo->cmn.nNetworkType;

	// 发送szSelfInfoToClient
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
	// 找到对应的Player，移除它 
	PlayerUnit		*pPlayer		= m_Players.getptr((int)pCOCmdReason->nSlotExt);
	if( pPlayer )
	{
		pPlayer->nSlot	= 0;		// 免得Remove的时候再Close一次
		RemovePlayerUnit(pPlayer);
	}

	return	0;
}
