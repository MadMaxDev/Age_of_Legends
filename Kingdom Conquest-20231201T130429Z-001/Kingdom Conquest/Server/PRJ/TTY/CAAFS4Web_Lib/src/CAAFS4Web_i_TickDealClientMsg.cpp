#include "../inc/CAAFS4Web_i.h"

using namespace n_pngs;

int		CAAFS4Web_i::Tick_DealClientMsg()
{
	m_tickNow			= wh_gettickcount();

	// ������Ϣ
	epoll_server::ctrl_out_T*	pCOCmd	= NULL;
	size_t	nSize	= 0;
	while (m_pEpollServer->ControlOut(&pCOCmd, &nSize) == 0)
	{
		switch (pCOCmd->iCmd)
		{
		case epoll_server::ctrl_out_cntr_accepted:
			{
				Tick_DealClientMsg_CONTROL_OUT_CNTR_ACCEPTED(pCOCmd, nSize);
			}
			break;
		case epoll_server::ctrl_out_cntr_closed:
			{
				Tick_DealClientMsg_CONTROL_OUT_CNTR_CLOSED(pCOCmd, nSize);
			}
			break;
		case epoll_server::ctrl_out_should_stop:
			{
				m_bShouldStop	= true;
			}
			break;
		}
	}

	// ����ͻ��˷�����ָ��()
	// ���ǿͻ��˲��ܷ��͹����ָ��(Ŀǰ�������)
	// ����ͻ��˷�����ָ��
	int			nCntrID		= 0;
	pngs_cmd_t*	pClientCmd	= NULL;
	while (m_pEpollServer->Recv(&nCntrID, (void**)&pClientCmd, &nSize) > 0)
	{
		// �ҵ���Ӧ���û�
		PlayerUnit*	pPlayer	= GetPlayerByCntrID(nCntrID);
		if (pPlayer == NULL)
		{
			// ��Ҳ�������,���cntrӦ�ùر�
			m_pEpollServer->Close(nCntrID);
			continue;
		}
		if (pPlayer->nRecvCmdNum++ > m_cfginfo.nMaxPlayerRecvCmdNum)
		{
			// ָ��̫����,�Ͽ��û�
			m_pEpollServer->Close(nCntrID);
			continue;
		}
		switch (*pClientCmd)
		{
		case CLIENT_CAAFS4Web_2LP:
			{
				// ��װһ��Ȼ�󷢸�LP4Web
				m_vectrawbuf.resize(sizeof(CAAFS4Web_LP_CLIENT_CMD_T)+nSize-sizeof(*pClientCmd));
				CAAFS4Web_LP_CLIENT_CMD_T*	pCmd2LP	= (CAAFS4Web_LP_CLIENT_CMD_T*)m_vectrawbuf.getbuf();
				pCmd2LP->nCmd						= CAAFS4Web_LP_CLIENT_CMD;
				pCmd2LP->nClientIDInCAAFS4Web		= pPlayer->nID;
				pCmd2LP->IP							= pPlayer->IP;
				memcpy(wh_getptrnexttoptr(pCmd2LP), wh_getptrnexttoptr(pClientCmd), nSize-sizeof(*pClientCmd));
				m_msgerLogicProxy.SendMsg(pCmd2LP, m_vectrawbuf.size());
			}
			break;
		default:
			// ָ��Ƿ�
			m_pEpollServer->Close(nCntrID, CLOSE_PARAM_TO_CLIENT_BADCMD);
			break;
		}
	}
	return 0;
}
int		CAAFS4Web_i::Tick_DealClientMsg_CONTROL_OUT_CNTR_ACCEPTED(epoll_server::ctrl_out_T* pCOCmd, int nSize)
{
	epoll_server::ctrl_out_cntr_accept_T*	pCOAccepted	= (epoll_server::ctrl_out_cntr_accept_T*)pCOCmd;
	size_t		nExtDataSize	= 0;
	CLIENT_CAAFS4Web_EXTINFO_T*				pExtInfo	= (CLIENT_CAAFS4Web_EXTINFO_T*)m_pEpollServer->GetCntr_ConnectExtData(pCOAccepted->iCntrID, &nExtDataSize);
	if (pExtInfo==NULL || nExtDataSize!=sizeof(CLIENT_CAAFS4Web_EXTINFO_T))
	{
		m_pEpollServer->Close(pCOAccepted->iCntrID, CLOSE_PARAM_TO_CLIENT_ERR_EXTINFO);
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1390,CAAFS4Web_RT)"%s,,CLIENT_CAAFS4Web_EXTINFO_T NULL or size 0", __FUNCTION__);
		return 0;
	}
	switch (pExtInfo->nTermType)
	{
	case TERM_TYPE_IOS:
	case TERM_TYPE_ANDROID:
	case TERM_TYPE_WP:
	case TERM_TYPE_WEB:
		{

		}
		break;
	default:
		{
			m_pEpollServer->Close(pCOAccepted->iCntrID, CLOSE_PARAM_TO_CLIENT_ERR_EXTINFO);
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1390,CAAFS4Web_RT)"%s,,CLIENT_CAAFS4Web_EXTINFO_T ERROR term type,%d", __FUNCTION__, pExtInfo->nTermType);
			return 0;
		}
		break;
	}
	// ��������û�
	// Ӧ�ò����з������,��Ϊcntr�������������players������һ��
	PlayerUnit*	pPlayer	= NULL;
	int			nID		= m_Players.AllocUnit(pPlayer);
	if (nID < 0)
	{
		// ���ش���
		m_pEpollServer->Close(pCOAccepted->iCntrID, CLOSE_PARAM_TO_CLIENT_SVRMEM);
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1390,CAAFS4Web_RT)"%s,,m_Players.AllocUnit ERROR", __FUNCTION__);
		return 0;
	}
	// ��������
	pPlayer->nID		= nID;
	pPlayer->nCntrID	= pCOAccepted->iCntrID;
	pPlayer->IP			= pCOAccepted->addrPeer.sin_addr.s_addr;
	pPlayer->nPort		= pCOAccepted->addrPeer.sin_port;
	pPlayer->nTermType	= pExtInfo->nTermType;
	// ��cntr����
	m_pEpollServer->SetExt(pPlayer->nCntrID, pPlayer->nID);
	// ����channel 0
	PutPlayerInChannel(pPlayer, 0);
	// ����Ϊ�ȴ�״̬
	pPlayer->nStatus	= PlayerUnit::STATUS_WAITING;
	
	// ����szSelfInfoToClient
	char	szBuf[1024];
	CAAFS4Web_CLIENT_CAAFS4WebINFO_T*	pCAAFS4WebINFO	= (CAAFS4Web_CLIENT_CAAFS4WebINFO_T*)szBuf;
	pCAAFS4WebINFO->nCmd				= CAAFS4Web_CLIENT_CAAFS4WebINFO;
	pCAAFS4WebINFO->nVerCmpMode			= m_cfginfo.nVerCmpMode;
	strcpy(pCAAFS4WebINFO->szVer, m_cfginfo.szVer);
	pCAAFS4WebINFO->nQueueSize			= pPlayer->nSeq - pPlayer->nSeq0;
	pCAAFS4WebINFO->nInfoLen			= m_cfginfo._nSelfInfoToClientLen;
	memcpy(pCAAFS4WebINFO->info, m_cfginfo.szSelfInfoToClient, m_cfginfo._nSelfInfoToClientLen);
	pCAAFS4WebINFO->nSelfNotify			= m_nSelfNotify;
	nSize	= wh_offsetof(CAAFS4Web_CLIENT_CAAFS4WebINFO_T, info)+m_cfginfo._nSelfInfoToClientLen;
	m_pEpollServer->Send(pPlayer->nCntrID, pCAAFS4WebINFO, nSize);

	SetTE_QueueSeq(pPlayer, true);

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1391,CAAFS4Web_RT)"PlayerAccepted,0x%X,%d,%d,%d,%d", pPlayer->nID, m_nConnectingNum, m_nLogicProxyAvailPlayerNum, m_Players.size(), pPlayer->nSeq);

	return 0;
}
int		CAAFS4Web_i::Tick_DealClientMsg_CONTROL_OUT_CNTR_CLOSED(epoll_server::ctrl_out_T* pCOCmd, int nSize)
{
	epoll_server::ctrl_out_cntr_close_reason_T*	pCOClose	= (epoll_server::ctrl_out_cntr_close_reason_T*)pCOCmd;
	// �ҵ���Ӧ��Player,�Ƴ���
	PlayerUnit*		pPlayer	= m_Players.getptr((int)pCOClose->iCntrExt);
	if (pPlayer != NULL)
	{
		pPlayer->nCntrID	= 0;
		RemovePlayerUnit(pPlayer);
	}
	return 0;
}
