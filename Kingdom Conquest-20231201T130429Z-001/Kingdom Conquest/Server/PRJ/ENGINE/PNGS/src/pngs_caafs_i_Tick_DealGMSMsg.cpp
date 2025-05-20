// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick_DealGMSMsg.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CAAFS��Tick_DealGMSMsg���ֵľ���ʵ��
// CreationDate : 2005-08-03
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::Tick_DealGMSMsg()
{
	m_tickNow	= wh_gettickcount();

	// �ܵ�����˵��GMS�����ӻ�����

	// �����GMS��������Ϣ
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
				// ����Ķ�������
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(558,PNGS_RT)"CAAFS2_I::Tick_DealGMSMsg,%d,unknown", *pCmd);
			}
			break;
		}
		// �ͷŸոմ������Ϣ
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
			// ��������ܿ����˳���
			m_bShouldStop	= true;
		}
		break;
	case	GMS_CAAFS_CTRL_T::SUBCMD_STRCMD:
		{
			DealStrCmd((char*)&pCtrl->nParam);
			// ���������ǿմ���ѽ�����ظ�GMS
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
				// ����û����ڣ�����û�ת��ָ���Ķ�����ȥ
				if( PutPlayerInVIPChannel(pPlayer, pCtrl->nParam) )
				{
					// �����ȷ��תchannel�˾ͷ��Ͷ�����ʾ
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

	// �����û��Ƿ񻹴��ڣ�������˵���Ѿ���CAAFS��ɾ���ˣ�Ӧ������������CLS�Ĺ����У�
	PlayerUnit	*pPlayer			= m_Players.getptr(pClientCanGoCLS->nClientIDInCAAFS);
	if( !pPlayer )
	{
		// �û������ھͲ��ù���(CLS�Ǳ�Ӧ���г�ʱ��)
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1016,PLAYER_IN)"GMS_CAAFS_CLIENTCANGOTOCLS_T,Player Lost,0x%X", pClientCanGoCLS->nClientIDInCAAFS);
		return	0;
	}

	// ����������
	switch( pClientCanGoCLS->nRst )
	{
	case	GMS_CAAFS_CLIENTCANGOTOCLS_T::RST_OK:
		{
			// ���ɸ��ͻ��˵�ָ��
			CAAFS_CLIENT_GOTOCLS_T			GoToCLS;
			GoToCLS.nCmd					= CAAFS_CLIENT_GOTOCLS;
			GoToCLS.nPort					= pClientCanGoCLS->nPort;
			GoToCLS.IP						= pClientCanGoCLS->IP;
			GoToCLS.nPassword				= pPlayer->nPasswordToConnectCLS;
			GoToCLS.nClientID				= pClientCanGoCLS->nClientID;
			// ���͸��ͻ��ˣ��ͻ����յ������Ӧ�û������ر����ӣ�
			m_pSlotMan4Client->Send(pPlayer->nSlot, &GoToCLS, sizeof(GoToCLS), 0);
			// �ͻ����յ������Ϣ���Ӧ��ȥ����CLS��ͬʱ�ر���������ˡ�
			// ����ʱ������û��ٳٲ��ر����ӣ���ʱ��ֱ�ӹر�����
			TQUNIT_T	*pTQUnit;
			if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCloseCheckTimeOut, (void **)&pTQUnit, &pPlayer->teid)<0 )
			{
				// ��ô���޷�����ʱ���¼�����
				assert(0);
				// �Ƴ��û�
				RemovePlayerUnit(pPlayer);
				return	-1;
			}
			pTQUnit->tefunc					= &CAAFS2_I::TEDeal_Close_TimeOut;
			pTQUnit->un.player.nID			= pPlayer->nID;
		}
		break;
	default:
		// �����ľͶ�Ӧ���ǲ��ɹ��ˣ�Ӧ��ɾ���û�
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
