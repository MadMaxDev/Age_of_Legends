// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTUREģ���ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-22
// Change LOG   : 2006-06-07 ��Organize���ж�m_pLogicGamePlay���Ϊ���򷵻ش���ԭ��û�з��أ������յ�GMS������Ϣ��GZS֪ͨ��ʱ�����˴���

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gzs_logic.h"
#include <WHCMN/inc/whdbg.h>

using namespace n_pngs;

////////////////////////////////////////////////////////////////////
// GZS_MAINSTRUCTURE
////////////////////////////////////////////////////////////////////
// ����CFGINFO_T
WHDATAPROP_MAP_BEGIN_AT_ROOT(GZS_MAINSTRUCTURE::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(whbyte, nSvrIdx, 0)
	WHDATAPROP_ON_SETVALUE_smp(whbyte, nForceKickOldGZS, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueCmdInSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nHelloTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szGMSAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nConnectOtherSvrTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nMaxCmdPackSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bLogSendCmd, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bLogRecvCmd, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(unknowntype, CNTRSVR, 0)
WHDATAPROP_MAP_END()

// ������
GZS_MAINSTRUCTURE::GZS_MAINSTRUCTURE()
: m_tickNow(0)
, m_nStatus(STATUS_NOTHING)
, m_pLogicGamePlay(NULL)
, m_nNoCLIENT_RECV(0)
, m_bIsReconnect(false)
, m_bConnectedToGMS(false)
{
	strcpy(FATHERCLASS::m_szLogicType, PNGS_DLL_NAME_GZS_MAINSTRUCTURE);
	FATHERCLASS::m_nVer	= GZSMAINSTRUCTURE_VER;
	m_CntrSvr.m_pHost	= this;
	m_vectrawbuf.reserve(PNGS_RAWBUF_SIZE);
}
GZS_MAINSTRUCTURE::~GZS_MAINSTRUCTURE()
{
}
void	GZS_MAINSTRUCTURE::SelfDestroy()
{
	delete	this;
}
int		GZS_MAINSTRUCTURE::Init(const char *cszCFGName)
{
	m_tickNow	= wh_gettickcount();

	int	rst;

	// ����������Ϣ
	if( Init_CFG(cszCFGName)<0 )
	{
		return	-1;
	}

	if( m_cfginfo.nSvrIdx<=0
#ifdef	__GNUC__					// �󲿷��˶�ʹ��windows�µ��Խ�������,idx�ᳬ��26,����win�²����ж���,ʵ����������岻��(�����linux�汾���Ὠ̫���idx)��yuezhongyue
		||m_cfginfo.nSvrIdx>=GZS_MAX_NUM 
#endif
		)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(663,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,%d,bad m_cfginfo.nSvrIdx", m_cfginfo.nSvrIdx);
		return	-13;
	}

	m_vectCmdPack.resize(m_cfginfo.nMaxCmdPackSize);
	m_wcsCmdPack.SetShrink(m_vectrawbuf.getbuf(), m_vectrawbuf.size());	// �����ǳ�ʼ��
	CmdPackReset();

	// ���ӡ�����GMS�������Ƿ�ɹ�
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(660,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,connect to GMS,%s", m_cfginfo.szGMSAddr);
	// ���Լ�����һ�£����Ҫ����ǰ�棬����Init�����õ�����Ͳ����ˣ�
	m_msgerGMS.m_pHost	= this;
	// ��������GMS�Ķ���
	if( (rst=m_msgerGMS.Init(m_MSGER_INFO.GetBase(), m_cfginfo.szGMSAddr))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(662,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,%d,m_msgerGMS.Init", rst);
		return	-12;
	}

	// ѭ���ȴ�����
	bool		bStop	= false;
	size_t		nSize;
	GMS_GZS_HI_T	*pHI= NULL;
	while( !bStop )
	{
		m_bIsReconnect	= false;	// ��֤�������״̬���ж�������reconnect
		wh_sleep(10);
		if( m_msgerGMS.GetStatus() == MYMSGER_T::STATUS_TRYCONNECT )
		{
			if( wh_tickcount_diff(wh_gettickcount(), m_msgerGMS.m_tickWorkBegin) >= m_cfginfo.nConnectOtherSvrTimeOut )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(661,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,,connect to GMS fail,%s", m_cfginfo.szGMSAddr);
				return	-20;
			}
			m_msgerGMS.Tick();
			continue;
		}
		// �ܵ�����˵���Ѿ�������
		if( wh_tickcount_diff(wh_gettickcount(), m_msgerGMS.m_tickWorkBegin) >= m_cfginfo.nHelloTimeOut )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(845,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,,GMS did not respone to HELLO");
			return	-21;
		}
		m_msgerGMS.Tick();
		// �����Ƿ��յ�����
		pHI	= (GMS_GZS_HI_T *)m_msgerGMS.PeekMsg(&nSize);
		if( pHI )
		{
			switch( pHI->nCmd )
			{
				case	GMS_GZS_HI:
				{
					// ֹͣѭ��
					bStop		= true;
				}
				break;
			}
			m_msgerGMS.FreeMsg();
		}
	}

	// �жϽ���Ƿ�����
	if( pHI->nRst != GMS_GZS_HI_T::RST_OK )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(667,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,%d,GMS_GZS_HI_T bad rst,Program abort!", pHI->nRst);
		switch(pHI->nRst)
		{
		case	GMS_GZS_HI_T::RST_IDXDUP:
			{
				dbg_ouput_to_msgbox("ERROR", "GZSIdx DUP! Please check your configuration file.");
			}
			break;
		default:
			{
				char	buf[256];
				sprintf(buf, "GMS_GZS_HI_T rst:%d", pHI->nRst);
				dbg_ouput_to_msgbox("ERROR", buf);
			}
			break;
		}
		return	-23;
	}
	if( GMS_VER != pHI->nGMSVer )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(668,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,,GMS_GZS_HI bad gmsver,%d,%d", pHI->nGMSVer,GMS_VER);
		return	-24;
	}

	// ��ʼ��TCPSERVER
	m_cfginfo.CNTRSVR.nMaxNum	= pHI->nGMSMaxCLS;
	if( (rst=m_CntrSvr.Init(m_cfginfo.CNTRSVR.GetBase()))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(669,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,%d,m_CntrSvr.Init", rst);
		return	-31;
	}

	// ��ʼ��ʱ�����
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		assert(0);
		return	-32;
	}

	// ��ʼ����������(���Ӧ�ò����ܳ���ģ����������ļ�����)
	m_CLSs.Init(pHI->nGMSMaxCLS);
	m_Players.Init(pHI->nGMSMaxPlayer);

	if( ILogic::QueueInit(m_cfginfo.nQueueCmdInSize)<0 )
	{
		return	-33;
	}

	// ��ʼ������
	// �Ѱ󶨵ĵ�ַ����GMS
	struct sockaddr_in	&addr	= m_CntrSvr.GetBindAddr();
	GZS_GMS_MYINFO_T	MyInfo;
	MyInfo.nCmd			= GZS_GMS_MYINFO;
	MyInfo.nPort		= ntohs(addr.sin_port);
	MyInfo.IP			= addr.sin_addr.s_addr;
	if( MyInfo.IP == 0  )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(670,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,,must set a IP to bind (GZS->CNTRSVR->szIP)");
		return	-41;
	}
	m_msgerGMS.SendMsg(&MyInfo, sizeof(MyInfo));
	m_msgerGMS.ManualSend();

	m_nNoCLIENT_RECV	= 0;
	// �����ҪGMS����ָ������
	whbit_ulong_set(&m_nNoCLIENT_RECV, 0);

	// ��ʼ����
	SetStatus(STATUS_WORKING);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(671,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,OK,WORK BEGIN");

	return	0;
}
int		GZS_MAINSTRUCTURE::Init_CFG(const char *cszCFGName)
{
	// �����ļ�
	WHDATAINI_CMN	ini;
	ini.addobj("GZS", &m_cfginfo);
	ini.addobj("MSGERGZS", &m_MSGER_INFO);
	// ����һ��Ĭ��ֵ����������������ֵĬ�����ô�һЩ�ɣ�
	m_MSGER_INFO.nSendBufSize		= 10*1024*1024;
	m_MSGER_INFO.nRecvBufSize		= 10*1024*1024;
	m_MSGER_INFO.nSockSndBufSize	= 1*1024*1024;
	m_MSGER_INFO.nSockRcvBufSize	= 1*1024*1024;
	//
	int	rst = ini.analyzefile(cszCFGName);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(658,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init_CFG,%d,ini.analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// �������

	// !������������Ƿ�������(���������ֻ��Ϊ��Ӧ�Էǳ���С�ĵķ�����������Ա����Ҫ��)
	if( m_cfginfo.nSvrIdx <=0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(659,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init_CFG,%d,BAD m_cfginfo.nSvrIdx", m_cfginfo.nSvrIdx);
		return	-2;
	}

	return	0;
}
int		GZS_MAINSTRUCTURE::Init_CFG_Reload(const char *cszCFGName)
{
	// �����ļ�
	WHDATAINI_CMN	ini;
	ini.addobj("GZS", &m_cfginfo);
	ini.addobj("MSGERGZS", &m_MSGER_INFO);

	//
	int	rst = ini.analyzefile(cszCFGName, false, 1);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(658,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init_CFG_Reload,%d,ini.analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	m_vectCmdPack.resize(m_cfginfo.nMaxCmdPackSize);
	m_wcsCmdPack.SetShrink(m_vectrawbuf.getbuf(), m_vectrawbuf.size());	// �����ǳ�ʼ��
	CmdPackReset();

	return	0;
}
int		GZS_MAINSTRUCTURE::Release()
{
	ILogicBase::QueueRelease();

	return	0;
}
int		GZS_MAINSTRUCTURE::Detach(bool bQuick)
{
	if( !bQuick )
	{
		// �ø���Connecter�Լ��˳�������timewait�Ͳ��������listen�����
		TellAllConnecterToQuit();
		// �ȴ�һ��ʱ��
		for(int i=0;i<20;i++)
		{
			Tick();
			SureSend();
			wh_sleep(50);
		}
	}
	//
	m_CntrSvr.Release();
	// ��GMS��������ֹ
	m_msgerGMS.Release();

	// ʵ����Ӧ���ǵȴ�����������������������
	// ... ���� ...
	return	0;
}
int		GZS_MAINSTRUCTURE::Tick_BeforeDealCmdIn()
{
	if( m_nStatus != STATUS_WORKING )
	{
		return	-1;
	}

	// ��tickʱ��
	m_tickNow	= wh_gettickcount();

	// ʱ����д���
	Tick_DealTE();

	// ��Ϣ����

	// �����GMS��������Ϣ
	Tick_DealGMSMsg();

	return	0;
}
int		GZS_MAINSTRUCTURE::Tick_AfterDealCmdIn()
{
	// ��������������TCP���ӵĹ���
	m_CntrSvr.DoWork(false);
	return	0;
}
int		GZS_MAINSTRUCTURE::SureSend()
{
	// Ϊ��ȷ�������ٵ���һ��
	m_CntrSvr.ConnecterMan::SureSend();

	// ��GMS����ָ��(������������Ϊǰ��Ĵ����п��ܻᴥ����Ҫ��GMS���͵�ָ��)
	m_msgerGMS.ManualSend();

	return	0;
}
int		GZS_MAINSTRUCTURE::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	// ����GMS��
	SOCKET	sock	= m_msgerGMS.GetSocket();
	if( cmn_is_validsocket(sock) )
	{
		vect.push_back(sock);
	}
	// ������������������
	m_CntrSvr.GetSockets(vect);
	return	0;
}
int		GZS_MAINSTRUCTURE::Organize()
{
	// Ѱ��gameplayģ��(�Ҳ�������)
	m_pLogicGamePlay	= m_pCMN->GetLogic(PNGS_DLL_NAME_GZS_GAMEPLAY, GZSGAMEPLAY_VER);
	if( !m_pLogicGamePlay )
	{
		assert(0);
		return	-1;
	}
	// �Խ���һЩ����
	GZSPACKET_2GP_CONFIG_T	CmdConfig;
	CmdConfig.nSubCmd	= GZSPACKET_2GP_CONFIG_T::SUBCMD_PLAYERNUMMAX;
	CmdConfig.nParam	= m_Players.getmax();					// m_Players�ĳߴ�����Init��ʱ���GMS��������
	m_pLogicGamePlay->CmdIn_Instant(this, GZSPACKET_2GP_CONFIG, &CmdConfig, sizeof(CmdConfig));
	return	0;
}

Connecter *	GZS_MAINSTRUCTURE::NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
{
	if( m_nMood != MOOD_WORKING )	// ��ֹͣ״̬��ʱ��Ͳ��ܽ����κ�������
	{
		return	NULL;
	}
	MYCNTRSVR::MYCNTR	*pCntr	= new MYCNTRSVR::MYCNTR(this);
	assert(pCntr);
	// ��ʼ��֮
	MYCNTRSVR::MYCNTR::INFO_T	info;
	memcpy(&info.msgerINFO, m_MSGER_INFO.GetBase(), sizeof(info.msgerINFO));
	info.msgerINFO.sock	= sock;		// ����ɲ�������!!!!
	int	rst	= pCntr->Init(&info);
	if( rst<0 )
	{
		delete	pCntr;
		pCntr	= NULL;
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(673,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::MYCNTRSVR::NewConnecter,%d,pCntr->Init", rst);
	}
	else
	{
		memcpy(&pCntr->m_addr, paddr, sizeof(pCntr->m_addr));
	}
	return	pCntr;
}
void	GZS_MAINSTRUCTURE::AfterAddConnecter(Connecter * pCntr)
{
	MYCNTRSVR::MYCNTR	*pMyCntr	= (MYCNTRSVR::MYCNTR *)pCntr->QueryInterface();
	if( !pMyCntr )
	{
		// ���Ӧ����accepter
		return;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(674,PNGS_INT_RLS)"GZS_MAINSTRUCTURE,new TCP connecter,0x%X,%s"
		, pMyCntr->GetIDInMan()
		, cmn_get_ipnportstr_by_saaddr(&pMyCntr->m_addr)
		);
	// ������ʱ�������û���յ�Hello����Ͽ�����
	TQUNIT_T	*pTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nHelloTimeOut, (void **)&pTQUnit, &pMyCntr->teid)<0 )
	{
		// ��ô���޷�����ʱ���¼�����
		assert(0);
		// �Ƴ��û�
		RemoveMYCNTR(pMyCntr);
		return;
	}
	pTQUnit->tefunc		= &GZS_MAINSTRUCTURE::TEDeal_Hello_TimeOut;
	pTQUnit->un.svr.nID	= pMyCntr->GetIDInMan();
}
void	GZS_MAINSTRUCTURE::TellAllConnecterToQuit()
{
	TellAllCLSQuit();
}
void	GZS_MAINSTRUCTURE::TellAllCLSQuit()
{
	SVR_CLS_CTRL_T		Ctrl;
	Ctrl.nCmd			= SVR_CLS_CTRL;
	Ctrl.nSubCmd		= SVR_CLS_CTRL_T::SUBCMD_EXIT;
	for(whunitallocatorFixed<CLSUnit>::iterator it=m_CLSs.begin(); it!=m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, &Ctrl, sizeof(Ctrl));
	}
}
void	GZS_MAINSTRUCTURE::TellCLSToKickPlayer(int nClientID, unsigned char nKickCmd)
{
	SVR_CLS_CTRL_T	Ctrl;
	Ctrl.nCmd		= SVR_CLS_CTRL;
	switch( nKickCmd )
	{
	case	GZSPACKET_2MS_KICKPLAYER_T::CMD_KICK_END:
		{
			Ctrl.nSubCmd	= SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_AS_END;
		}
		break;
	default:
		{
			Ctrl.nSubCmd	= SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID;
		}
		break;
	}
	Ctrl.nParam		= nClientID;
	// �ҵ�Client���ڵ�CLS������֮
	SendCmdToPlayerCLS(nClientID, &Ctrl, sizeof(Ctrl));
}
void	GZS_MAINSTRUCTURE::TellCLSToKickAllPlayerOfMe()
{
	SVR_CLS_CTRL_T	Ctrl;
	Ctrl.nCmd		= SVR_CLS_CTRL;
	Ctrl.nSubCmd	= SVR_CLS_CTRL_T::SUBCMD_KICKPLAYEROFGZS;
	Ctrl.nParam		= m_cfginfo.nSvrIdx;
	for(whunitallocatorFixed<CLSUnit>::iterator it=m_CLSs.begin(); it!=m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, &Ctrl, sizeof(Ctrl));
	}
}
void	GZS_MAINSTRUCTURE::TellGMSGoodExit()
{
	GZS_GMS_CTRL_T	Cmd;
	Cmd.nCmd		= GZS_GMS_CTRL;
	Cmd.nSubCmd		= GZS_GMS_CTRL_T::SUBCMD_EXIT;
	Cmd.nParam		= 0;
	m_msgerGMS.SendMsg(&Cmd, sizeof(Cmd));
}
void	GZS_MAINSTRUCTURE::TellGMSToKickPlayerAndRemovePlayer(int nClientID)
{
	// ����GMS
	GZS_GMS_KICKPALYERBYID_T	Cmd;
	Cmd.nCmd		= GZS_GMS_KICKPALYERBYID;
	Cmd.nClientID	= nClientID;
	m_msgerGMS.SendMsg(&Cmd, sizeof(Cmd));
	// ɾ���Լ�������û�
	RemovePlayerUnit(nClientID);
}
void	GZS_MAINSTRUCTURE::InnerRouteClientGameCmd(CLS_SVR_CLIENT_DATA_T *pCmd, size_t nSize)
{
	if( !m_pLogicGamePlay )
	{
		// ���û�������Ȼ�Ͳ�����
		return;
	}

	if( m_nNoCLIENT_RECV )
	{
		// ��ϣ�����ڽ����û�������ֱ�Ӻ���
		return;
	}

	nSize	-= wh_offsetof(CLS_SVR_CLIENT_DATA_T, data);
	assert( (int)nSize>0 );	// CLS�ǲ��ᴫ��������0�����ݵ�
	// �ϳ�ָ��
	GZSPACKET_2GP_PLAYERCMD_T	Cmd2GP;
	Cmd2GP.nClientID		= pCmd->nClientID;
	Cmd2GP.nChannel			= pCmd->nChannel;
	Cmd2GP.nDSize			= nSize;
	Cmd2GP.pData			= pCmd->data;

	if( m_cfginfo.bLogRecvCmd )
	{
		int		nTmpSize	= nSize>256 ? 256 : nSize;
		char	szBuf[4096];
		GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(1031,CMDLOG)"CLI,%u,0x%X,%d,0x%X,%s", m_tickNow, pCmd->nClientID, nSize, *(pngs_cmd_t *)pCmd->data, wh_hex2str((unsigned char *)pCmd->data, nTmpSize, szBuf, 1, true));
	}

	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, GZSPACKET_2GP_PLAYERCMD, Cmd2GP);
}
void	GZS_MAINSTRUCTURE::SetStatus(int nStatus)
{
	m_nStatus	= nStatus;
}
void	GZS_MAINSTRUCTURE::Retry_Worker_TRYCONNECT_Begin()
{
	// ����Ϊ����״̬
	m_bIsReconnect		= true;
	m_bConnectedToGMS	= false;
}
void	GZS_MAINSTRUCTURE::Retry_Worker_WORKING_Begin()
{
	m_bConnectedToGMS	= true;
	// �ɹ������Լ�����Ϣ(��ʾ�Լ���GZS����Ҫ�����Ӧ����Ϣ)
	GZS_GMS_HELLO_T			Hello;
	Hello.nCmd				= GZS_GMS_HELLO;
	Hello.nSvrIdx			= m_cfginfo.nSvrIdx;
	Hello.nForceKickOldGZS	= m_cfginfo.nForceKickOldGZS;
	Hello.nGZSVer			= GZS_VER;
	Hello.bReConnect		= m_bIsReconnect;
	m_msgerGMS.SendMsg(&Hello, sizeof(Hello));
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(664,PNGS_INT_RLS)"GZS_MAINSTRUCTURE::Init,Send HELLO to GMS,%d", Hello.nSvrIdx);
}
