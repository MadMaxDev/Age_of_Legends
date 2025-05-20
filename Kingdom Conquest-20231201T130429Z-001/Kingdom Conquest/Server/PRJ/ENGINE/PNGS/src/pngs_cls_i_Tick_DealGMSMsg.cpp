// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick_DealGMSMsg.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CLSģ���Tick_DealGMSMsg����
//                PNGS��Pixel Network Game Structure����д
//                CLS��Connection Load Server����д�������ӷ��书���еĸ��ط�����
// CreationDate : 2005-08-08
// Change LOG   :

#include "../inc/pngs_cls_i.h"

using namespace n_pngs;

int		CLS2_I::Tick_DealGMSMsg()
{
	m_tickNow			= wh_gettickcount();
	m_nCurCmdFromSvrIdx	= PNGS_SVRIDX_GMS;

	// �����GMS��������Ϣ
	pngs_cmd_t	*pCmd;
	size_t		nSize;
	while( (pCmd=(pngs_cmd_t *)m_msgerGMS.PeekMsg(&nSize)) != NULL )
	{
		switch( *pCmd )
		{
		case	GMS_CLS_GZSINFO:
			Tick_DealGMSMsg_GMS_CLS_GZSINFO(pCmd, nSize);
			break;
		case	GMS_CLS_CLIENTWANTCLS:
			Tick_DealGMSMsg_GMS_CLS_CLIENTWANTCLS(pCmd, nSize);
			break;
		case	GMS_CLS_ROUTECLIENTDATATOGZS:
			Tick_DealGMSMsg_GMS_CLS_ROUTECLIENTDATATOGZS(pCmd, nSize);
			break;
		case	GMS_CLS_PLAYER_CHANGEID:
			Tick_DealGMSMsg_GMS_CLS_PLAYER_CHANGEID(pCmd, nSize);
			break;
		case	SVR_CLS_CTRL:
			Tick_Deal_CmnSvrMsg_SVR_CLS_CTRL(NULL, pCmd, nSize);
			break;
		case	SVR_CLS_SET_TAG_TO_CLIENT:
			// �ڲ�ʵ����һ��tagidx��tagval��ɵĴ�hash��ͬkey��������ձ�����һ��DList
			Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG_TO_CLIENT(pCmd, nSize);
			break;
		case	SVR_CLS_SET_PROP_TO_CLIENT:
			Tick_Deal_CmnSvrMsg_SVR_CLS_SET_PROP_TO_CLIENT(pCmd, nSize);
			break;
		case	SVR_CLS_CLIENT_DATA:
			Tick_Deal_CmnSvrMsg_SVR_CLS_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
			break;
		case	SVR_CLS_CLIENT_DATA_PACK:
			Tick_Deal_CmnSvrMsg_SVR_CLS_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
			break;
		case	SVR_CLS_MULTICLIENT_DATA:
			Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
			break;
		case	SVR_CLS_MULTICLIENT_DATA_PACK:
			Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
			break;
		case	SVR_CLS_TAGGED_CLIENT_DATA:
			Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
			break;
		case	SVR_CLS_TAGGED_CLIENT_DATA_PACK:
			Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
			break;
		case	SVR_CLS_ALL_CLIENT_DATA:
			Tick_Deal_CmnSvrMsg_SVR_CLS_ALL_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
			break;
		case	SVR_CLS_ALL_CLIENT_DATA_PACK:
			Tick_Deal_CmnSvrMsg_SVR_CLS_ALL_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
			break;
		case	SVR_CLS_SET_TAG64_TO_CLIENT:
			Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG64_TO_CLIENT(pCmd, nSize);
			break;
		case	SVR_CLS_TAGGED64_CLIENT_DATA:
			Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED64_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
			break;
		case	SVR_CLS_TAGGED64_CLIENT_DATA_PACK:
			Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED64_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
			break;
		default:
			// ����Ķ�������
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(640,PNGS_RT)"CLS2_I::Tick_DealGMSMsg,%d,unknown", *pCmd);
			break;
		}
		// �ͷŸոմ������Ϣ
		m_msgerGMS.FreeMsg();
	}

	return	0;
}
int		CLS2_I::Tick_DealGMSMsg_GMS_CLS_GZSINFO(void *pCmd, int nSize)
{
	GMS_CLS_GZSINFO_T	*pGZSINFO	= (GMS_CLS_GZSINFO_T *)pCmd;
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(642,PNGS_INT_RLS)"CLS2_CONNECT_GZS,0x%X,%d,%s:%d"
		, m_nCLSID
		, pGZSINFO->nSvrIdx
		, cmn_get_IP(pGZSINFO->IP), pGZSINFO->nPort);

	GZSUnit	*pGZS		= m_GZSs + pGZSINFO->nSvrIdx;
	pGZS->clear();
	pGZS->nSvrIdx		= pGZSINFO->nSvrIdx;
	assert(pGZS->nSvrIdx>0 && pGZS->nSvrIdx<GZS_MAX_NUM);
	// ��������GZS��socket
	struct sockaddr_in	addr;
	cmn_get_saaddr_by(&addr, pGZSINFO->IP, pGZSINFO->nPort);
	try
	{
		pGZS->sockGZS		= tcp_create_connecting_socket(&addr, 0);
		if( pGZS->sockGZS == INVALID_SOCKET )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(643,PNGS_INT_RLS)"CLS2_I::Tick_DealGMSMsg_GMS_CLS_GZSINFO,,tcp_create_connecting_socket,%s", cmn_get_ipnportstr_by_saaddr(&addr));
			throw	-2;
		}
		pGZS->nStatus		= GZSUnit::STATUS_CONNECTING;
		// �������ӳ�ʱ
		pGZS->tickConnectGZSTimeOut	= m_tickNow + m_cfginfo.nConnectOtherSvrTimeOut;
	}
	catch( int nErr )
	{
		// �д������ɾ��
		RemoveGZSUnit(pGZS);
		return	nErr;
	}
	return	0;
}
int		CLS2_I::Tick_DealGMSMsg_GMS_CLS_CLIENTWANTCLS(void *pCmd, int nSize)
{
	GMS_CLS_CLIENTWANTCLS_T	*pClientWantCLS	= (GMS_CLS_CLIENTWANTCLS_T *)pCmd;
	// (���ԭ����ͬidx���û�����ɾ��)
	int	nIdx	= m_Players.getrealidx(pClientWantCLS->nClientID);
	PlayerUnit	*pPlayer	= m_Players.GetByIdx(nIdx);
	if( pPlayer )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(644,PNGS_INT_RLS)"CLS2_I::Tick_DealGMSMsg_GMS_CLS_CLIENTWANTCLS,,RemovePlayerUnit Old,0x%X,%d,0x%X", pPlayer->nID, pPlayer->nStatus, pClientWantCLS->nClientID);
		RemovePlayerUnit(pPlayer);
	}

	// ����Player
	pPlayer		= m_Players.AllocUnitByID(pClientWantCLS->nClientID);
	assert(pPlayer);
	// Ϊ�˱�����clearһ��
	pPlayer->clear(this);
	// ����ID
	pPlayer->nID			= pClientWantCLS->nClientID;
	// ����Ϊ���ӹ���״̬
	pPlayer->nStatus		= PlayerUnit::STATUS_CONNECTING;
	// ��������
	pPlayer->nPasswordToConnectCLS	= pClientWantCLS->nPassword;

	// �������ӳ�ʱ��ʱ��
	SetTEDeal_nClientConnectToMeTimeOut(pPlayer);

	// ���ظ���GMS������
	CLS_GMS_CLIENTCANGOTOCLS_T	CCGC;
	CCGC.nCmd				= CLS_GMS_CLIENTCANGOTOCLS;
	CCGC.nClientID			= pClientWantCLS->nClientID;
	m_msgerGMS.SendMsg(&CCGC, sizeof(CCGC));

	return	0;
}
int		CLS2_I::Tick_DealGMSMsg_GMS_CLS_ROUTECLIENTDATATOGZS(void *pCmd, int nSize)
{
	GMS_CLS_ROUTECLIENTDATATOGZS_T	*pRouteClientDataTOGZS	= (GMS_CLS_ROUTECLIENTDATATOGZS_T *)pCmd;
	PlayerUnit	*pPlayer	= TryGetPlayerAndSendNotifyToGMSIsNotExisted(pRouteClientDataTOGZS->nClientID);
	if( !pPlayer )
	{
		// ��ҿ����Ѿ�������
		return	0;
	}
	// ���������GZS��nGZSIDΪ0
	pPlayer->nSvrIdx		= pRouteClientDataTOGZS->nSvrIdx;
	DelPlayerFromTagList(pPlayer, PNGS_CLIENTTAG_ONEGZS);
	if( pPlayer->nSvrIdx>0 )
	{
		// ������
		AddPlayerToTagList(pPlayer, PNGS_CLIENTTAG_ONEGZS, pPlayer->nSvrIdx);
	}
	return	0;
}
int		CLS2_I::Tick_DealGMSMsg_GMS_CLS_PLAYER_CHANGEID(void *pCmd, int nSize)
{
	GMS_CLS_PLAYER_CHANGEID_T	*pC	= (GMS_CLS_PLAYER_CHANGEID_T *)pCmd;
	PlayerUnit	*pPlayerTo		= m_Players.AllocUnitByID(pC->nToID);
	if( pPlayerTo )
	{
		PlayerUnit	*pPlayerFrom	= TryGetPlayerAndSendNotifyToGMSIsNotExisted(pC->nFromID);
		if( pPlayerFrom )
		{
			pPlayerTo->clear(this);
			pPlayerTo->ReadFromOther(pPlayerFrom);
			pPlayerTo->nID			= pC->nToID;
			pPlayerTo->nSvrIdx		= pC->nSvrIdx;
			// ��slot���¹���һ��
			m_pSlotMan4Client->SetExtLong(pPlayerTo->nSlot, pPlayerTo->nID);
			// ɾ����Դ
			pPlayerFrom->clear(this);
			m_Players.FreeUnitByPtr(pPlayerFrom);
		}
	}
	return	0;
}
