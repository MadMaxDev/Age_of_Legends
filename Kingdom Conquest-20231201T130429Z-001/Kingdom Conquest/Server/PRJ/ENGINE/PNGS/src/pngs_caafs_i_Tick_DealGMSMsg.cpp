// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick_DealGMSMsg.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CAAFS中Tick_DealGMSMsg部分的具体实现
// CreationDate : 2005-08-03
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::Tick_DealGMSMsg()
{
	m_tickNow	= wh_gettickcount();

	// 能到这里说明GMS的连接还存在

	// 处理从GMS发来的消息
	pngs_cmd_t	*pCmd;
	size_t		nSize;
	while( (pCmd=(pngs_cmd_t *)m_msgerGMS.PeekMsg(&nSize)) != NULL )
	{
		switch( *pCmd )
		{
		case	GMS_CAAFS_CTRL:
			{
				Tick_DealGMSMsg_GMS_CAAFS_CTRL(pCmd, nSize);
			}
			break;
		case	GMS_CAAFS_GMSINFO:
			{
				Tick_DealGMSMsg_GMS_CAAFS_GMSINFO(pCmd, nSize);
			}
			break;
		case	GMS_CAAFS_CLIENTCANGOTOCLS:
			{
				Tick_DealGMSMsg_GMS_CAAFS_CLIENTCANGOTOCLS(pCmd, nSize);
			}
			break;
		case	GMS_CAAFS_KEEP_QUEUE_POS:
			{
				Tick_DealGMSMsg_GMS_CAAFS_KEEP_QUEUE_POS(pCmd,nSize);
			}break;
		default:
			{
				// 其余的都不处理
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(558,PNGS_RT)"CAAFS2_I::Tick_DealGMSMsg,%d,unknown", *pCmd);
			}
			break;
		}
		// 释放刚刚处理的消息
		m_msgerGMS.FreeMsg();
	}
	return	0;
}
int		CAAFS2_I::Tick_DealGMSMsg_GMS_CAAFS_CTRL(void *pCmd, int nSize)
{
	GMS_CAAFS_CTRL_T	*pCtrl	= (GMS_CAAFS_CTRL_T *)pCmd;
	switch( pCtrl->nSubCmd )
	{
	case	GMS_CAAFS_CTRL_T::SUBCMD_EXIT:
		{
			// 告诉主框架可以退出了
			m_bShouldStop	= true;
		}
		break;
	case	GMS_CAAFS_CTRL_T::SUBCMD_STRCMD:
		{
			DealStrCmd((char*)&pCtrl->nParam);
			// 如果结果不是空串则把结果返回给GMS
			int		nRstLen		= strlen(m_szRstStr)+1;
			if( nRstLen>1 )
			{
				char	buf[sizeof(m_szRstStr) + sizeof(CAAFS_GMS_CTRL_T)];
				CAAFS_GMS_CTRL_T	&Ctrl	= *(CAAFS_GMS_CTRL_T *)buf;
				Ctrl.nCmd			= CAAFS_GMS_CTRL;
				Ctrl.nSubCmd		= CAAFS_GMS_CTRL_T::SUBCMD_STRRST;
				Ctrl.nExt			= pCtrl->nExt;
				memcpy(&Ctrl.nParam, m_szRstStr, nRstLen);
				m_msgerGMS.SendMsg(buf, nRstLen + wh_offsetof(CAAFS_GMS_CTRL_T, nParam));
			}
		}
		break;
	case	GMS_CAAFS_CTRL_T::SUBCMD_CLIENTCMDNUMSUB1:
		{
			PlayerUnit	*pPlayer	= m_Players.getptr(pCtrl->nExt);
			if( pPlayer )
			{
				if( pPlayer->nRecvCmdNum>0 )
				{
					pPlayer->nRecvCmdNum	--;
				}
			}
		}
		break;
	case	GMS_CAAFS_CTRL_T::SUBCMD_CLIENTQUEUETRANS:
		{
			PlayerUnit	*pPlayer	= m_Players.getptr(pCtrl->nExt);
			if( pPlayer )
			{
				// 如果用户还在，则把用户转到指定的队列中去
				if( PutPlayerInVIPChannel(pPlayer, pCtrl->nParam) )
				{
					// 如果的确是转channel了就发送队列提示
					SendPlayerQueueInfo(pPlayer, true);
				}
			}
		}
		break;
	}
	return	0;
}
int		CAAFS2_I::Tick_DealGMSMsg_GMS_CAAFS_GMSINFO(void *pCmd, int nSize)
{
	GMS_CAAFS_GMSINFO_T	*pGMSInfo	= (GMS_CAAFS_GMSINFO_T *)pCmd;
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(911,PNGS_RT)"GMS_CAAFS_GMSINFO,%d,%d", m_nGMSAvailPlayerNum, pGMSInfo->nGMSAvailPlayer);
	m_nGMSAvailPlayerNum		= pGMSInfo->nGMSAvailPlayer;
	return	0;
}
int		CAAFS2_I::Tick_DealGMSMsg_GMS_CAAFS_CLIENTCANGOTOCLS(void *pCmd, int nSize)
{
	GMS_CAAFS_CLIENTCANGOTOCLS_T	*pClientCanGoCLS	= (GMS_CAAFS_CLIENTCANGOTOCLS_T *)pCmd;

	// 查找用户是否还存在（不存在说明已经从CAAFS中删除了，应该是正在连接CLS的过程中）
	PlayerUnit	*pPlayer			= m_Players.getptr(pClientCanGoCLS->nClientIDInCAAFS);
	if( !pPlayer )
	{
		// 用户不存在就不用管了(CLS那边应该有超时的)
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1016,PLAYER_IN)"GMS_CAAFS_CLIENTCANGOTOCLS_T,Player Lost,0x%X", pClientCanGoCLS->nClientIDInCAAFS);
		return	0;
	}

	// 看看结果如何
	switch( pClientCanGoCLS->nRst )
	{
	case	GMS_CAAFS_CLIENTCANGOTOCLS_T::RST_OK:
		{
			// 生成给客户端的指令
			CAAFS_CLIENT_GOTOCLS_T			GoToCLS;
			GoToCLS.nCmd					= CAAFS_CLIENT_GOTOCLS;
			GoToCLS.nPort					= pClientCanGoCLS->nPort;
			GoToCLS.IP						= pClientCanGoCLS->IP;
			GoToCLS.nPassword				= pPlayer->nPasswordToConnectCLS;
			GoToCLS.nClientID				= pClientCanGoCLS->nClientID;
			// 发送给客户端（客户端收到这个后应该会主动关闭连接）
			m_pSlotMan4Client->Send(pPlayer->nSlot, &GoToCLS, sizeof(GoToCLS), 0);
			// 客户端收到这个消息后就应该去连接CLS。同时关闭这个连接了。
			// 做超时，如果用户迟迟不关闭连接，则超时后直接关闭连接
			TQUNIT_T	*pTQUnit;
			if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCloseCheckTimeOut, (void **)&pTQUnit, &pPlayer->teid)<0 )
			{
				// 怎么会无法加入时间事件涅？
				assert(0);
				// 移除用户
				RemovePlayerUnit(pPlayer);
				return	-1;
			}
			pTQUnit->tefunc					= &CAAFS2_I::TEDeal_Close_TimeOut;
			pTQUnit->un.player.nID			= pPlayer->nID;
		}
		break;
	default:
		// 其他的就都应该是不成功了，应该删除用户
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(831,PLAYER_IN)"GMS_CAAFS_CLIENTCANGOTOCLS_T,%d,0x%X,bad rst,player removed", pClientCanGoCLS->nRst, pPlayer->nID);
		RemovePlayerUnit(pPlayer);
		break;
	}

	return	0;
}

int		CAAFS2_I::Tick_DealGMSMsg_GMS_CAAFS_KEEP_QUEUE_POS(void *pCmd, int nSize)
{
	GMS_CAAFS_KEEP_QUEUE_POS_T * pReq = (GMS_CAAFS_KEEP_QUEUE_POS_T *)pCmd;
	PlayerUnit	*pPlayer	= m_Players.getptr(pReq->nClientIDInCAAFS);
	if( !pPlayer )
	{
		return -1;
	}
#ifdef UNITTEST
	printf("BEFORE LOGIN: %s\n",pReq->szAccount);
	DisplayQueueInfo();
#endif
	DealKeepQueuePosReq(pPlayer,pReq->szAccount);
#ifdef UNITTEST
	DisplayQueueInfo();
#endif
	return 0;
}
