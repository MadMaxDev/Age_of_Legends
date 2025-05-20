// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CLSģ��
//                PNGS��Pixel Network Game Structure����д
//                CLS��Connection Load Server����д�������ӷ��书���еĸ��ط�����
//                ���ļ���CLS���ڲ�ʵ��
//                CLS�Ϳͻ���ͨѶʹ��cnl2���ͷ�������ͨѶʹ��TCP��
// CreationDate : 2005-07-28
// Change LOG   : 2005-11-09 ������CLS2_I::RemoveGZSUnit����iteratorѭ����ɾ������Ĵ���

#include "../inc/pngs_cls_i.h"
#include "../inc/pngs_cmn.h"

using namespace n_pngs;

CLS2 *	CLS2::Create()
{
	return	new CLS2_I;
}

////////////////////////////////////////////////////////////////////
// CLS2_I
////////////////////////////////////////////////////////////////////
// ����CFGINFO_T
WHDATAPROP_MAP_BEGIN_AT_ROOT(CLS2_I::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szWelcomeAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szFakeWelcomeAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szGMSAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(whbyte, nNetworkType, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bCanAcceptPlayerWhenCLSConnectingGZS, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bLogSvrCmd, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nGroupID, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nConnectOtherSvrTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCloseFromGMSTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nHelloTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nClientConnectToMeTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSlotInOutNum, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCmdStatInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szSelfInfoToClient, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nMaxRecvCount, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nMaxRecvSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCheckRecvInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nBigTickFilter,0)
WHDATAPROP_MAP_END()

// ������
CLS2_I::CLS2_I()
: m_nStatus(STATUS_NOTHING)
, m_tickNow(0)
, m_nCLSID(0)
, m_pICryptFactory(NULL)
, m_pSlotMan4Client(NULL)
, m_bShouldStop(false)
, m_nCurCmdFromSvrIdx(0)
//, m_dtu(16)
{
	m_vectIDToDestroy.reserve(256);
	m_vectrawbuf.reserve(16384);
	m_vectCmdStat.resize(65536);
	memset(m_vectCmdStat.getbuf(), 0, m_vectCmdStat.totalbytes());
}
CLS2_I::~CLS2_I()
{
}
void	CLS2_I::SelfDestroy()
{
	delete	this;
}
int		CLS2_I::Init(const char *cszCFG)
{
	m_tickNow	= wh_gettickcount();

	int	rst;
	// ����������Ϣ
	if( (rst=Init_CFG(cszCFG))<0 )
	{
		return	-1;
	}

	// ���ӡ�����GMS�������Ƿ�ɹ�
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(617,PNGS_INT_RLS)"CLS2_I::Init,connection to GMS,%s", m_cfginfo.szGMSAddr);
	// ���Լ�����һ�£����Ҫ����ǰ�棬����Init�����õ�����Ͳ����ˣ�
	m_msgerGMS.m_pHost	= this;
	// ��������GMS�Ķ���
	if( (rst=m_msgerGMS.Init(m_MSGER_INFO.GetBase(), m_cfginfo.szGMSAddr))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(619,PNGS_INT_RLS)"CLS2_I::Init,%d,m_msgerGMS.Init", rst);
		return	-12;
	}

	// ѭ���ȴ�����
	bool		bStop	= false;
	size_t		nSize;
	GMS_CLS_HI_T	*pHI= NULL;
	while( !bStop )
	{
		wh_sleep(10);
		if( m_msgerGMS.GetStatus() == MYMSGER_T::STATUS_TRYCONNECT )
		{
			if( wh_tickcount_diff(wh_gettickcount(), m_msgerGMS.m_tickWorkBegin) >= m_cfginfo.nConnectOtherSvrTimeOut )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(618,PNGS_INT_RLS)"CLS2_I::Init,,connect to GMS fail,%s", m_cfginfo.szGMSAddr);
				return	-21;
			}
			m_msgerGMS.Tick();
			continue;
		}
		// �ܵ�����˵���Ѿ�������
		if( wh_tickcount_diff(wh_gettickcount(), m_msgerGMS.m_tickWorkBegin) >= m_cfginfo.nHelloTimeOut )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(621,PNGS_INT_RLS)"CLS2_I::Init,,GMS did not respone to HELLO");
			return	-21;
		}
		m_msgerGMS.Tick();
		// �����Ƿ��յ�����
		pHI	= (GMS_CLS_HI_T *)m_msgerGMS.PeekMsg(&nSize);
		if( pHI )
		{
			switch( pHI->nCmd )
			{
				case	GMS_CLS_HI:
				{
					// ֹͣѭ��
					bStop		= true;
				}
				break;
			}
			m_msgerGMS.FreeMsg();
		}
	}
	if( pHI->nRst != GMS_CLS_HI_T::RST_OK )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(623,PNGS_INT_RLS)"CLS2_I::Init,%d,GMS_CLS_HI bad rst", pHI->nRst);
		return	-23;
	}
	if( GMS_VER != pHI->nGMSVer )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(624,PNGS_INT_RLS)"CLS2_I::Init,,GMS_CLS_HI bad gmsver,%d,%d", pHI->nGMSVer,GMS_VER);
		return	-24;
	}

	// �����ȴ��û����ӵ�CNL2
	m_pSlotMan4Client		= CNL2SlotMan::Create();
	if( !m_pSlotMan4Client )
	{
		assert(0);
		return	-31;
	}
	m_pSlotMan4Client->SetUseAuth();
	// ������������GMSָ��
	m_CLIENT_SLOTMAN_INFO.nMaxConnection	= pHI->nCLSMaxConnection;
	// ���ù̶���channel����
	pngs_config_CLSClient_Slot_Channel(m_CLIENT_SLOTMAN_INFO.GetBase(), m_cfginfo.nSlotInOutNum, false);
	// ��ʼ������ö���İ󶨵�ַ
	struct sockaddr_in	addr;
	if( (rst=m_pSlotMan4Client->Init(m_CLIENT_SLOTMAN_INFO.GetBase(), &addr))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(625,PNGS_INT_RLS)"CLS2_I::Init,%d,m_pSlotMan4Client->Init", rst);
		return	-32;
	}
	m_pSlotMan4Client->Listen(true);
	m_pSlotMan4Client->SetICryptFactory(m_pICryptFactory);

	// ���Ͷ˿ں͵�ַ
	CLS_GMS_HELLO1_T				Cmd1;
	Cmd1.nCmd						= CLS_GMS_HELLO1;
	Cmd1.nPort						= ntohs( addr.sin_port );
	Cmd1.IP							= addr.sin_addr.s_addr;
	if( Cmd1.IP==0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(626,PNGS_INT_RLS)"CLS2_I::Init,must set a IP to bind (CLS->szWelcomeAddr)");
		return	-33;
	}
	if( m_cfginfo.szFakeWelcomeAddr[0] )
	{
		// �ѵ�ַ�Ͷ˿������ֿ�
		char	szAddr[WHNET_MAXADDRSTRLEN]	= "";
		int		nDPort						= 0;
		wh_strsplit("sd", m_cfginfo.szFakeWelcomeAddr, ":", szAddr, &nDPort);
		if( nDPort==0 && szAddr[0]==0 )
		{
			// ˵��û���κ��µı�ʾ��Ӧ����д����
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(627,PNGS_INT_RLS)"CLS2_I::Init,szFakeWelcomeAddr is BAD,%s", m_cfginfo.szFakeWelcomeAddr);
			return	-34;
		}
		// �޸�IP
		if( szAddr[0] )
		{
			// ��ַ
			Cmd1.IP	= cmn_get_S_addr(szAddr);
		}
		// �޸Ķ˿�
		if( nDPort )
		{
			Cmd1.nPort	+= nDPort;
		}
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(628,PNGS_INT_RLS)"CLS2_I::Init,use FakeWelcomeAddr,%s:%d,%s", cmn_get_IP(Cmd1.IP), Cmd1.nPort, m_cfginfo.szFakeWelcomeAddr);
	}
	m_msgerGMS.SendMsg(&Cmd1, sizeof(Cmd1));

	if( (rst=m_Players.Init(pHI->nGMSMaxPlayer))<0 )
	{
		assert(0);
		return	-41;
	}

	// ��ʼ��ʱ�����
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		assert(0);
		return	-42;
	}

	// �����Լ���ID
	m_nCLSID	= pHI->nCLSID;

	// Ȼ��ͽ��빤��״̬
	SetStatus(STATUS_WORKING);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(629,PNGS_INT_RLS)"CLS2_I::Init,OK Work begin,0x%X", m_nCLSID);

	return	0;
}
int		CLS2_I::Init_CFG(const char *cszCFG)
{
	WHDATAINI_CMN	ini;
	ini.addobj("CLS", &m_cfginfo);
	ini.addobj("MSGER", &m_MSGER_INFO);
	ini.addobj("CLIENT_SLOTMAN", &m_CLIENT_SLOTMAN_INFO);
	// ����һ��Ĭ��ֵ����������������ֵĬ�����ô�һЩ�ɣ�
	m_MSGER_INFO.nSendBufSize		= 10*1024*1024;
	m_MSGER_INFO.nRecvBufSize		= 10*1024*1024;
	m_MSGER_INFO.nSockSndBufSize	= 1*1024*1024;
	m_MSGER_INFO.nSockRcvBufSize	= 1*1024*1024;
	m_CLIENT_SLOTMAN_INFO.nMaxSinglePacketSize	= 500000;
	m_CLIENT_SLOTMAN_INFO.nAllocChunkSize		= 1024*1024;
	m_CLIENT_SLOTMAN_INFO.nControlOutQueueSize	= 10*1024*1024;
	//
	int	rst = ini.analyzefile(cszCFG);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(630,PNGS_INT_RLS)"CLS2_I::Init_CFG,%d,ini.analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// �󶨵�ַ
	strcpy(m_CLIENT_SLOTMAN_INFO.szBindAddr, m_cfginfo.szWelcomeAddr);

	// ����ѭ��
	m_loopCmdStat.setinterval(m_cfginfo.nCmdStatInterval);

	return	0;
}
int		CLS2_I::Init_CFG_Reload(const char *cszCFG)
{
	WHDATAINI_CMN	ini;
	ini.addobj("CLS", &m_cfginfo);
	ini.addobj("MSGER", &m_MSGER_INFO);
	ini.addobj("CLIENT_SLOTMAN", &m_CLIENT_SLOTMAN_INFO);
	//
	int	rst = ini.analyzefile(cszCFG, false, 1);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(630,PNGS_INT_RLS)"CLS2_I::Init_CFG_Reload,%d,ini.analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// �󶨵�ַ
	strcpy(m_CLIENT_SLOTMAN_INFO.szBindAddr, m_cfginfo.szWelcomeAddr);

	// ����ѭ��
	m_loopCmdStat.setinterval(m_cfginfo.nCmdStatInterval);

	m_pSlotMan4Client->Init_Reload(m_CLIENT_SLOTMAN_INFO.GetBase());

	return	0;
}
int		CLS2_I::Release()
{
	// ������Ԫ���
	m_Players.BeginGet();
	PlayerUnit	*pPlayer	= m_Players.GetNext();
	while( pPlayer != NULL )
	{
		int	nID	= pPlayer->nID;
		pPlayer	= m_Players.GetNext();
		RemovePlayerUnit(nID);
	}
	for(int i=1;i<GZS_MAX_NUM;++i)
	{
		RemoveGZSUnit(i);
	}
	// ���������ͷ�
	m_Players.Release();

	// tagӳ���ͷ�
	for(whhash<unsigned int, DLPUNIT_T>::kv_iterator it=m_mapTag2DL.begin(); it!=m_mapTag2DL.end(); ++it)
	{
		it.getvalue().clear(this);
	}
	m_mapTag2DL.clear();
	// 64λtag�ͷ�
	for(whhash<whuint64, DLPUNIT_T>::kv_iterator it64=m_map64Tag2DL.begin(); it64!=m_map64Tag2DL.end(); ++it64)
	{
		it64.getvalue().clear(this);
	}
	m_map64Tag2DL.clear();

	// CNL2�ս�
	WHSafeSelfDestroy( m_pSlotMan4Client );

	// �����Լ������ı�־
	CLS_GMS_CTRL_T	C;
	C.nCmd			= CLS_GMS_CTRL;
	C.nSubCmd		= CLS_GMS_CTRL_T::SUBCMD_EXIT;
	m_msgerGMS.SendMsg(&C, sizeof(C));

	// ����GMS����Ϣ����ȥ��GMSӦ�û�ر���CLS��socket��
	whtick_t	t	= wh_gettickcount();
	while( m_msgerGMS.GetStatus() != MYMSGER_T::STATUS_TRYCONNECT
	&&     wh_tickcount_diff(wh_gettickcount(), t)<m_cfginfo.nCloseFromGMSTimeOut
	)
	{
		m_msgerGMS.Tick();
		wh_sleep(10);
	}

	// TCP�ս�
	m_msgerGMS.Release();

	m_nStatus	= STATUS_NOTHING;

	return	0;
}
int		CLS2_I::SetICryptFactory(ICryptFactory *pFactory)
{
	m_pICryptFactory	= pFactory;
	if( m_pSlotMan4Client )
	{
		m_pSlotMan4Client->SetICryptFactory(m_pICryptFactory);
	}
	return	0;
}
void	CLS2_I::SetStatus(int nStatus)
{
	m_nStatus	= nStatus;
}
int		CLS2_I::GetStatus() const
{
	return	m_nStatus;
}
int		CLS2_I::RemovePlayerUnit(int nPlayerID)
{
	return	RemovePlayerUnit(m_Players.getptr(nPlayerID));
}
int		CLS2_I::RemovePlayerUnit(PlayerUnit *pPlayer)
{
	// CLSɾ����ҵ�ʱ��Ҫ�ǵ�֪ͨGMS(��ΪֻҪ���ӿ�ʼ֮����ҵ�һ�ж��߶��Ǵ�CLS�����)

	if( pPlayer == NULL )
	{
		return	-1;
	}

	switch( pPlayer->nStatus )
	{
		case	PlayerUnit::STATUS_CONNECTING:
		{
			CLS_GMS_CLIENTTOCLS_RST_T	CTCRst;
			CTCRst.nCmd			= CLS_GMS_CLIENTTOCLS_RST;
			CTCRst.nRst			= CLS_GMS_CLIENTTOCLS_RST_T::RST_ERR_NOTCONNECT;
			CTCRst.nClientID	= pPlayer->nID;
			m_msgerGMS.SendMsg(&CTCRst, sizeof(CTCRst));
		}
		break;
		case	PlayerUnit::STATUS_WORKING:
		{
			// ���ֻ��Ҫ֪ͨGMS���ɡ�GZS��GMS��ȥ֪ͨ������GMS�Ϳ�����һЩ�߼��������磺���û��ӳ�һ��ʱ��������֪ͨGZS�û�����֮��ģ�
			CLS_GMS_CLIENT_DROP_T	CD;
			CD.nCmd				= CLS_GMS_CLIENT_DROP;
			CD.nRemoveReason	= pPlayer->nRemoveReason;
			CD.nClientID		= pPlayer->nID;
			m_msgerGMS.SendMsg(&CD, sizeof(CD));
		}
		break;
	}

	if( m_pSlotMan4Client->IsSlotExist(pPlayer->nSlot) )
	{
		// ȡ����slot�Ĺ�ϵ(���Slot֪ͨ�ϲ�Close��ʱ��Ҫ��ͼɾ�����)
		m_pSlotMan4Client->SetExtLong(pPlayer->nSlot, 0);
		// �ر�Slot
		m_pSlotMan4Client->Close(pPlayer->nSlot, 100+pPlayer->nRemoveReason);
	}

	// ��������(�������ǰ�����������������֮��������������������˾Ͳ�֪��ԭ����û��������ϵ��)
	pPlayer->clear(this);
	// �Ƴ�
	m_Players.FreeUnitByPtr(pPlayer);

	return	0;
}
void	CLS2_I::RemoveAllPlayerOfGZS(unsigned char nSvrIdx)
{
	// �߳����к͸�GZS��ص����
	whunitallocatorFixed<PlayerUnit>::iterator	it = m_Players.begin();
	while( it!=m_Players.end() )
	{
		PlayerUnit	*pPlayer	= &(*it);
		++it;
		if( pPlayer->nSvrIdx == nSvrIdx )
		{
			pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_END;
			RemovePlayerUnit(pPlayer);
		}
	}
}
CLS2_I::PlayerUnit *	CLS2_I::GetPlayerBySlot(int nSlot)
{
	long	nExt;
	if( m_pSlotMan4Client->GetExtLong(nSlot, &nExt)<0 )
	{
		return	NULL;
	}
	return	m_Players.getptr((int)nExt);
}
int		CLS2_I::RemoveGZSUnit(int nSvrIdx)
{
	assert(nSvrIdx>0 && nSvrIdx<GZS_MAX_NUM);
	return	RemoveGZSUnit(m_GZSs + nSvrIdx);
}
int		CLS2_I::RemoveGZSUnit(GZSUnit *pGZS)
{
	if( pGZS==NULL )
	{
		return	0;
	}
	switch( pGZS->nStatus )
	{
	case	GZSUnit::STATUS_CONNECTING:
	case	GZSUnit::STATUS_SHAKEHAND:
		{
			// ������״������Ҫ֪ͨGMS����GZS�Ľ��ʧ��
			CLS_GMS_CONNECTEDTOGZS_T	ConnectedToGZS;
			ConnectedToGZS.nCmd		= CLS_GMS_CONNECTEDTOGZS;
			ConnectedToGZS.nRst		= CLS_GMS_CONNECTEDTOGZS_T::RST_FAIL;
			ConnectedToGZS.nSvrIdx	= pGZS->nSvrIdx;
			m_msgerGMS.SendMsg(&ConnectedToGZS, sizeof(ConnectedToGZS));
		}
		break;
	case	GZSUnit::STATUS_NOTHING:
		{
			// ˵���ⲻ���Ը����ڹ����е�GZS�����ԾͲ��ü�����
			return	0;
		}
		break;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(631,PNGS_RT)"CLS2_I::RemoveGZSUnit,%d,%d", pGZS->nSvrIdx, pGZS->nStatus);

	// �����к����GZS��ص��û������ߣ����ڲ���ô���ˣ���ҪGMS��������ָ��Ŷ��ߣ�
	// ����Ϊ�˳����Ȱ��Ǹ����ϰ�
	//RemoveAllPlayerOfGZS(pGZS->nSvrIdx);

	// ֪ͨ�͸�GZS��ص�������ң��������������⣬���Ժ�
	CLS_CLIENT_NOTIFY_T	Notify;
	Notify.nCmd			= CLS_CLIENT_NOTIFY;
	Notify.nSubCmd		= CLS_CLIENT_NOTIFY_T::SUBCMD_GZSDROP;
	SendRawCmdToAllPlayerOfGZS(pGZS->nSvrIdx, 0, &Notify, sizeof(Notify));

	pGZS->clear();
	return	0;
}
int		CLS2_I::SendDataToSinglePlayer(PlayerUnit *pPlayer, unsigned char nChannel, void *pData, int nSize, pngs_cmd_t nSendCmd)
{
	if( !pPlayer )
	{
		return	-1;
	}
	m_vectrawbuf.resize(nSize + wh_offsetof(CLS_CLIENT_DATA_T, data));
	CLS_CLIENT_DATA_T	*pDataToClient	= (CLS_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	pDataToClient->nCmd	= nSendCmd;
	memcpy(pDataToClient->data, pData, nSize);
	return	SendRawCmdToSinglePlayer(pPlayer, nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}
int		CLS2_I::SendRawCmdToSinglePlayer(int nPlayerID, unsigned char nChannel, void *pRawCmd, int nSize)
{
	return	SendRawCmdToSinglePlayer(m_Players.getptr(nPlayerID), nChannel, pRawCmd, nSize);
}
int		CLS2_I::SendRawCmdToSinglePlayer(PlayerUnit *pPlayer, unsigned char nChannel, void *pRawCmd, int nSize)
{
	// �ж�����Ƿ����
	if( !pPlayer )
	{
		return	-1;
	}
	// �ж����ݳ����Ƿ�������
	if( nSize<=0 )
	{
		assert(0);
		return	-2;
	}
#ifdef	_DEBUG
	// ͳ�Ʒ���ָ����ֽ���������
	if( nSize>=3 )
	{
		unsigned short	nIdx	= *(unsigned short *)wh_getoffsetaddr(pRawCmd, 1);
		CMDSTAT_T		&cs		= m_vectCmdStat[nIdx];
		cs.nByte				+= nSize;
		cs.nCount				++;
	}
#endif
	// ����
	int	rst	= m_pSlotMan4Client->Send(pPlayer->nSlot, pRawCmd, nSize, nChannel);
	if( rst<0 )
	{
		// �����������������Slot�������ˣ������Ը��ݷ���rst�ж���ʲô���󣨲μ�CNL2SlotMan_I_UDP::Send��
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(632,PNGS_RT)"CLS2_I::SendRawCmdToSinglePlayer,%d,Send,0x%X,%d,0x%X", rst, pPlayer->nID, nSize, pPlayer->nSlot);
		pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_SENDERR;
		RemovePlayerUnit(pPlayer);
	}
	if( (pPlayer->nProp & PNGS_CLS_PLAYER_PROP_LOG_SEND) != 0 )
	{
		char	buf[1024];
		int		nPSize	= sizeof(buf)/8;
		if( nSize<nPSize )
		{
			nPSize	= nSize;
		}
		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(777,DBG)"CLS2Player,0x%X,%s,%d,%d,%d,%s", pPlayer->nID, pPlayer->szName, nChannel, nSize, rst, dbg_printmem(pRawCmd, nPSize, buf));
	}
	return	0;
}
int		CLS2_I::SendRawCmdToAllPlayer(unsigned char nChannel, void *pRawCmd, int nSize)
{
	// 2006-05-26 ��ΪSendRawCmdToSinglePlayer�п��ܵ���pPlayer��ɾ�������Բ��õ���for����ʽ���ܻᵼ�³���
	whunitallocatorFixed<PlayerUnit>::iterator it=m_Players.begin();
	while( it!=m_Players.end() )
	{
		PlayerUnit	*pPlayer	= &(*it);
		++it;
		SendRawCmdToSinglePlayer(pPlayer, nChannel, pRawCmd, nSize);
	}
	return	0;
}
int		CLS2_I::SendRawCmdToAllPlayerOfGZS(unsigned char nSvrIdx, unsigned char nChannel, void *pRawCmd, int nSize)
{
	whunitallocatorFixed<PlayerUnit>::iterator it=m_Players.begin();
	while( it!=m_Players.end() )
	{
		PlayerUnit	*pPlayer	= &(*it);
		++it;
		if( pPlayer->nSvrIdx == nSvrIdx )
		{
			SendRawCmdToSinglePlayer(pPlayer, nChannel, pRawCmd, nSize);
		}
	}
	return	0;
}
int		CLS2_I::AddPlayerToTagList(PlayerUnit *pPlayer, unsigned char nTagIdx, short nTagVal)
{
	if( nTagVal==0 )
	{
		// 0ֵ�ʹ����ܽ�������(һ������֮ǰpPlayer���Ѿ��Ӷ����������)
		return	0;
	}

	PlayerUnit::TAGINFO_T	&taginfo	= pPlayer->taginfo[nTagIdx];
	if( taginfo.nValue != 0 )
	{
		// �������ڶ�����
		assert(0);
		return	-2;
	}

	// �õ�hash��ֵ
	unsigned int	nHashKey	= CreateHashKey(nTagIdx, nTagVal);
	// ���Ҷ��У����û�о�����һ��
	DLPUNIT_T		*pDLU;
	if( !m_mapTag2DL.get(nHashKey, &pDLU) )
	{
		if( !m_mapTag2DL.putalloc(nHashKey, pDLU) )
		{
			// �����ܰɣ�
			assert(0);
			return	-3;
		}
		// ֻ���¶��в���Ҫnewһ������
		pDLU->pDLP	= NewDLP();
	}
	assert(pDLU->pDLP);
	// ����Ҽ����������
	pDLU->pDLP->AddToTail(&taginfo.nodeTag);
	// ����ֵ
	taginfo.nValue	= nTagVal;

	return	0;
}
int		CLS2_I::DelPlayerFromTagList(PlayerUnit *pPlayer, unsigned char nTagIdx)
{
	PlayerUnit::TAGINFO_T	&taginfo	= pPlayer->taginfo[nTagIdx];
	if( taginfo.nValue == 0 )
	{
		// �����Ƴ��ˣ������Ͳ��ڶ�����
		return	0;
	}
	// �õ�hash��ֵ
	unsigned int	nHashKey	= CreateHashKey(nTagIdx, taginfo.nValue);
	// ��ö��ж���
	whhash<unsigned int, DLPUNIT_T>::kv_iterator	it	= m_mapTag2DL.find(nHashKey);
	if( it == m_mapTag2DL.end() )
	{
		// �����Ӧ�ã���Ϊ���Ӧ���ڶ����У����Զ��в�Ӧ�ò�����
		assert(0);
		return	-1;
	}
	DLPUNIT_T		&dlu		= it.getvalue();
	if( dlu.pDLP != taginfo.nodeTag.pHost )
	{
		assert(0);
		return	-2;
	}
	// ���
	taginfo.nodeTag.leave();
	// �����ϵ
	taginfo.nValue	= 0;
	// ��������������Ϊ0��ɾ��֮
	if( dlu.pDLP->size() <= 0 )
	{
		dlu.clear(this);
		m_mapTag2DL.erase(it);
	}
	return	0;
}
int		CLS2_I::AddPlayerToTag64List(PlayerUnit *pPlayer, whuint64 nTag)
{
	// ������Ҳ����Ѿ��ڶ�����
	if( pPlayer->map64Tag2Node.has(nTag) )
	{
		// ����ھͲ��ü�����
		return	0;
	}

	// ��ö��ж���
	DLPUNIT_T		*pDLU;
	if( !m_map64Tag2DL.get(nTag, &pDLU) )
	{
		if( !m_map64Tag2DL.putalloc(nTag, pDLU) )
		{
			// �����ܰɣ�
			assert(0);
			return	-1;
		}
		// ֻ���¶��в���Ҫnewһ������
		pDLU->pDLP	= NewDLP();
	}
	assert(pDLU->pDLP);
	// ����Ҽ����������
	DLPLAYER_NODE_T	*pNode	= new DLPLAYER_NODE_T;
	assert(pNode);
	pNode->data	= pPlayer;
	pDLU->pDLP->AddToTail(pNode);
	pPlayer->map64Tag2Node.put(nTag, pNode);
	return	0;
}
int		CLS2_I::DelPlayerFrom64TagList(PlayerUnit *pPlayer, whuint64 nTag)
{
	whhash<whuint64, DLPLAYER_NODE_T *>::kv_iterator	it = pPlayer->map64Tag2Node.find(nTag);
	if( it == pPlayer->map64Tag2Node.end() )
	{
		// �Ѿ����ڶ������ˣ����ü�����
		return	0;
	}
	DLPLAYER_NODE_T	*&pNode	= it.getvalue();
	// �˶�
	pNode->leave();
	// ɾ��
	delete	pNode;
	pNode	= NULL;
	pPlayer->map64Tag2Node.erase(it);
	return	0;
}

CLS2_I::DLPLAYER_T *	CLS2_I::NewDLP()
{
	return	new DLPLAYER_T;
}
void	CLS2_I::SafeDeleteDLP(CLS2_I::DLPLAYER_T *&pDLP)
{
	if( pDLP )
	{
		delete	pDLP;
		pDLP	= NULL;
	}
}
size_t	CLS2_I::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	// ��select
	vect.clear();
	// ����GMS��
	SOCKET	sock	= m_msgerGMS.GetSocket();
	if( cmn_is_validsocket(sock) )
	{
		vect.push_back(sock);
	}
	// ����GZS��
	GetGZSSockets(vect);
	// ����Client��
	m_pSlotMan4Client->GetSockets(vect);

	return	vect.size();
}
bool	CLS2_I::ShouldStop() const
{
	return	m_bShouldStop;
}
void	CLS2_I::Retry_Worker_WORKING_Begin()
{
	// �ɹ������Լ�����Ϣ(��ʾ�Լ���CLS����Ҫ�����Ӧ����Ϣ)
	CLS_GMS_HELLO_T					Hello;
	Hello.nCmd						= CLS_GMS_HELLO;
	Hello.nGroupID					= m_cfginfo.nGroupID;
	Hello.nCLSVer					= CLS_VER;
	Hello.nOldID					= m_nCLSID;
	Hello.nNetworkType				= m_cfginfo.nNetworkType;
	Hello.bCanAcceptPlayerWhenCLSConnectingGZS	= m_cfginfo.bCanAcceptPlayerWhenCLSConnectingGZS;
	m_msgerGMS.SendMsg(&Hello, sizeof(Hello));
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(620,PNGS_INT_RLS)"CLS2_I::Init,Send HELLO to GMS,%d,%d,%d,%d", Hello.nGroupID, Hello.nCLSVer, Hello.nNetworkType, m_nCLSID);
}
CLS2_I::PlayerUnit *	CLS2_I::TryGetPlayerAndSendNotifyToGMSIsNotExisted(int nPlayerID)
{
	PlayerUnit	*pPlayer	= m_Players.getptr(nPlayerID);
	if( pPlayer==NULL )
	{
		// ֱ�Ӱ����û��Ѿ������ڷ��ͽ����GMS
		CLS_GMS_CLIENT_DROP_T	CD;
		CD.nCmd				= CLS_GMS_CLIENT_DROP;
		CD.nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTEXIST;
		CD.nClientID		= nPlayerID;
		m_msgerGMS.SendMsg(&CD, sizeof(CD));
	}
	return	pPlayer;
}
