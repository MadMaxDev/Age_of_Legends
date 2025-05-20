// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ���ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-22
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

////////////////////////////////////////////////////////////////////
// GMS_MAINSTRUCTURE
////////////////////////////////////////////////////////////////////
// ����CFGINFO_T
WHDATAPROP_MAP_BEGIN_AT_ROOT(GMS_MAINSTRUCTURE::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxPlayer, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSafeReservePlayerNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxGMTOOL, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueCmdInSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nHelloTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSvrDropWaitTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCalcAndSendCAAFSAvailPlayerTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSvrCmdQueueSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSvrCmdPerSecond, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bGZSDownRestorePlayer, 0)
	WHDATAPROP_ON_SETVALUE( WHDATAPROP_TYPE_int|WHDATAPROP_TYPE_ARRAYBIT, anMaxCLSNumInCAAFSGroup, GMS_MAX_CAAFS_NUM, 1)
	WHDATAPROP_ON_SETVALUE( WHDATAPROP_TYPE_int|WHDATAPROP_TYPE_ARRAYBIT, anMaxPlayerNumInCLSOfCAAFSGroup, GMS_MAX_CAAFS_NUM, 1)
	WHDATAPROP_ON_SETVALUE_smp(unknowntype, CNTRSVR, 0)
WHDATAPROP_MAP_END()

// ������
GMS_MAINSTRUCTURE::GMS_MAINSTRUCTURE()
: m_tickNow(0)
, m_pLogicGamePlay(NULL)
, m_pLogicMEM(NULL)
, m_ao(NULL)
, m_am(NULL)
, m_pSHMData(NULL)
, m_nGMSLogicMaxPlayer(100)
{
	strcpy(FATHERCLASS::m_szLogicType, PNGS_DLL_NAME_GMS_MAINSTRUCTURE);
	FATHERCLASS::m_nVer	= GMSMAINSTRUCTURE_VER;
	m_CntrSvr.m_pHost	= this;
	m_vectrawbuf.reserve(PNGS_RAWBUF_SIZE);
	m_pszRstStr			= NULL;
	m_pnRstSize			= NULL;
}
GMS_MAINSTRUCTURE::~GMS_MAINSTRUCTURE()
{
}
void	GMS_MAINSTRUCTURE::SelfDestroy()
{
	delete	this;
}
int		GMS_MAINSTRUCTURE::Init(const char *cszCFGName)
{
	m_tickNow	= wh_gettickcount();

	if( Init_CFG(cszCFGName)<0 )
	{
		return	-1;
	}

	int	rst;

	// ��ʼ��TCPSERVER
	if( (rst=m_CntrSvr.Init(m_cfginfo.CNTRSVR.GetBase()))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(22,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::Init,%d,m_CntrSvr.Init", rst);
		return	-2;
	}

	// ��ʼ��ʱ�����
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(23,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::Init,%d,m_TQ.Init", rst);
		return	-3;
	}
	// ���붨ʱ����CAAFS�ɽ���������ʱ���¼�
	TQUNIT_T	*pTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCalcAndSendCAAFSAvailPlayerTimeOut, (void **)&pTQUnit, &m_teid_CalcAndSend_CAAFS_AvailPlayerNum)<0 )
	{
		// �����������ʱ����޷�����ʱ���¼�
		assert(0);
		return	-4;
	}
	pTQUnit->tefunc		= &GMS_MAINSTRUCTURE::TEDeal_CalcAndSend_CAAFS_AvailPlayerNum;

	// Ѱ���ڴ����ģ�飨����Ƚ���ҪӦ������Init�е��õģ������ڴ�ģ��һ������Ҫ��һ��������ģ�
	m_pLogicMEM			= m_pCMN->GetLogic(PNGS_DLL_NAME_GSMEM, GSMEM_VER);
	if( !m_pLogicMEM )
	{
		return			-11;
	}
	// ��ȡ�ڴ������
	{
		PNGSPACKET_2MEM_XXX_PTR_T	XPtr;
		XPtr.pcszKey	= PNGS_GSMEM_NAME_AM;
		if( m_pLogicMEM->CmdIn_Instant(this, PNGSPACKET_2MEM_GET_PTR, &XPtr, sizeof(XPtr))<0 )
		{
			return		-12;
		}
		m_am			= (AM_T *)XPtr.ptr;
		m_am->m_nUseCount	++;
		m_ao			= m_am->GetAO();
	}
	// �жϹ����ڴ��Ƿ��Ǽ̳е�
	{
		PNGSPACKET_2MEM_CONFIG_T	Config;
		Config.nSubCmd	= PNGSPACKET_2MEM_CONFIG_T::SUBCMD_GET_MEMSTATUS;
		if( m_pLogicMEM->CmdIn_Instant(this, PNGSPACKET_2MEM_CONFIG, &Config, sizeof(Config))<0 )
		{
			return		-13;
		}
		if( Config.nParam==0 )
		{
			// ��������ʼ����
			// ���ɶ������û�ȥ
			m_pSHMData	= MYSHMDATA_T::NewMe(m_am);
			assert(m_pSHMData);
			PNGSPACKET_2MEM_XXX_PTR_T	XPtr;
			XPtr.pcszKey	= FATHERCLASS::m_szLogicType;
			XPtr.ptr		= m_pSHMData;
			if( m_pLogicMEM->CmdIn_Instant(this, PNGSPACKET_2MEM_SET_PTR, &XPtr, sizeof(XPtr))<0 )
			{
				return	-14;
			}
			// ��ʼ����������(���Ӧ�ò����ܳ���ģ����������ļ�����)
			m_pSHMData->m_CLSs.Init(m_ao, m_cfginfo.nMaxCLS);
			m_pSHMData->m_GMTOOLs.Init(m_ao, m_cfginfo.nMaxGMTOOL);
			m_pSHMData->m_Players.Init(m_ao, m_cfginfo.nMaxPlayer);
		}
		else
		{
			// �Ǽ̳е�
			int	nOffset	= Config.nParam1;
			// ���Ǳ߻�ö��󲢽����ڲ�����
			m_am->m_nAdjustCount	++;
			PNGSPACKET_2MEM_XXX_PTR_T	XPtr;
			XPtr.pcszKey	= FATHERCLASS::m_szLogicType;
			if( m_pLogicMEM->CmdIn_Instant(this, PNGSPACKET_2MEM_GET_PTR, &XPtr, sizeof(XPtr))<0 )
			{
				return	-15;
			}
			m_pSHMData	= (MYSHMDATA_T *)XPtr.ptr;
			if( !m_pSHMData )
			{
				return	-16;
			}
			m_pSHMData->AdjustInnerPtr(m_am, nOffset);
			//
			m_am->m_nAdjustCount	--;
		}
	}

	m_queueGZSSvrCmd.Init(m_cfginfo.nSvrCmdQueueSize);
	m_generSendGZSSvrCmd.SetGenSpeed((float)m_cfginfo.nSvrCmdPerSecond);
	m_itv.reset();

	// ����ָ�����
	ILogicBase::QueueInit(m_cfginfo.nQueueCmdInSize);

	// ��ʼ���ִ�ָ��ӳ���
	InitTstStrMap();

	return	0;
}
int		GMS_MAINSTRUCTURE::Init_CFG(const char *cszCFGName)
{
	// �����ļ�
	WHDATAINI_CMN	ini;
	ini.addobj("GMS", &m_cfginfo);
	ini.addobj("MSGERGMS", &m_MSGER_INFO);
	// ����һ��Ĭ��ֵ����������������ֵĬ�����ô�һЩ�ɣ�
	m_MSGER_INFO.nSendBufSize		= 10*1024*1024;
	m_MSGER_INFO.nRecvBufSize		= 10*1024*1024;
	m_MSGER_INFO.nSockSndBufSize	= 1*1024*1024;
	m_MSGER_INFO.nSockRcvBufSize	= 1*1024*1024;
	//
	int	rst = ini.analyzefile(cszCFGName);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(24,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::Init_CFG,%d,ini.analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// �������
	// �ȼ���nMaxCLS
	m_cfginfo.nMaxCLS			= 0;
	for(int i=0;i<GMS_MAX_CAAFS_NUM;i++)
	{
		m_cfginfo.nMaxCLS		+= m_cfginfo.anMaxCLSNumInCAAFSGroup[i];
	}
	// �ټ������������ص�
	m_cfginfo.CNTRSVR.nMaxNum	= GMS_MAX_CAAFS_NUM + m_cfginfo.nMaxCLS + GZS_MAX_NUM + m_cfginfo.nMaxGMTOOL;

	return	0;
}
int		GMS_MAINSTRUCTURE::Init_CFG_Reload(const char *cszCFGName)
{
	// �����ļ�
	WHDATAINI_CMN	ini;
	ini.addobj("GMS", &m_cfginfo);
	ini.addobj("MSGERGMS", &m_MSGER_INFO);
	// ����һ��Ĭ��ֵ����������������ֵĬ�����ô�һЩ�ɣ�
	m_MSGER_INFO.nSendBufSize		= 10*1024*1024;
	m_MSGER_INFO.nRecvBufSize		= 10*1024*1024;
	m_MSGER_INFO.nSockSndBufSize	= 1*1024*1024;
	m_MSGER_INFO.nSockRcvBufSize	= 1*1024*1024;
	//
	int	rst = ini.analyzefile(cszCFGName, false, 1);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(24,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::Init_CFG_Reload,%d,ini.analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// �������
	// �ȼ���nMaxCLS
	m_cfginfo.nMaxCLS			= 0;
	for(int i=0;i<GMS_MAX_CAAFS_NUM;i++)
	{
		m_cfginfo.nMaxCLS		+= m_cfginfo.anMaxCLSNumInCAAFSGroup[i];
	}
	// �ټ������������ص�
	m_cfginfo.CNTRSVR.nMaxNum	= GMS_MAX_CAAFS_NUM + m_cfginfo.nMaxCLS + GZS_MAX_NUM + m_cfginfo.nMaxGMTOOL;

	return	0;
}
GMS_MAINSTRUCTURE::CAAFSGroup *	GMS_MAINSTRUCTURE::GetCAAFSByGroupID(int nGroupID)
{
	if( m_pSHMData->m_aCAAFSGroup[nGroupID].pCntr == NULL )
	{
		return	NULL;
	}
	return	&m_pSHMData->m_aCAAFSGroup[nGroupID];
}
int		GMS_MAINSTRUCTURE::Release()
{
	// ���ø����Release
	ILogicBase::QueueRelease();
	return	0;
}
int		GMS_MAINSTRUCTURE::Detach(bool bQuick)
{
	// �ȴ�һ��ʱ��
	if( !bQuick )
	{
		for(int i=0;i<20;i++)
		{
			Tick();
			wh_sleep(10);
		}
	}
	//
	m_CntrSvr.Release();
	// ʵ����Ӧ���ǵȴ�����������������������

	// ɾ������
	if( m_am )
	{
		m_am->m_nUseCount	--;
		if( m_pSHMData )
		{
			MYSHMDATA_T::DeleteMe(m_am, m_pSHMData);
		}
		m_am	= NULL;
	}
	// ... ���� ...
	return	0;
}
int		GMS_MAINSTRUCTURE::Tick_BeforeDealCmdIn()
{
	// ��tickʱ��
	m_tickNow	= wh_gettickcount();

	// ��ñ�tick��ʱ����
	m_itv.tick();

	// ʱ����д���
	Tick_DealTE();

	return	0;
}
int		GMS_MAINSTRUCTURE::Tick_AfterDealCmdIn()
{
	// ��������������TCP���ӵĹ���
	m_CntrSvr.DoWork(false);

	// ��GZS���ͻ���ָ��
	// ��ÿ��Լ�����ٸ�
	int		nNum	= m_generSendGZSSvrCmd.MakeDecision(m_itv.getfloatInterval());
	if( nNum>m_cfginfo.nSvrCmdPerSecond )
	{
		nNum		= m_cfginfo.nSvrCmdPerSecond;
	}
	while( nNum-- > 0 )
	{
		size_t	nSize	= 0;
		unsigned char	*pnSvrIdx		= (unsigned char *)m_queueGZSSvrCmd.OutFree(&nSize);	// ��Ϊû�ж��̣߳����Կ�����OutFree
		if( !pnSvrIdx )
		{
			break;
		}
		nSize	-= sizeof(unsigned char);
		SafeSendMsg(m_pSHMData->m_GZSs[*pnSvrIdx].pCntr, wh_getptrnexttoptr(pnSvrIdx), nSize);
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SureSend()
{
	// Ϊ��ȷ�������ٵ���һ��
	m_CntrSvr.ConnecterMan::SureSend();
	return	0;
}
int		GMS_MAINSTRUCTURE::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	// ������������������
	m_CntrSvr.GetSockets(vect);
	return	0;
}
int		GMS_MAINSTRUCTURE::Organize()
{
	// Ѱ��gameplayģ��(�Ҳ�������)
	m_pLogicGamePlay	= m_pCMN->GetLogic(PNGS_DLL_NAME_GMS_GAMEPLAY, GMSGAMEPLAY_VER);
	if( !m_pLogicGamePlay )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(26,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::Organize,,m_pLogicGamePlay is NULL");
		return	-10;
	}
	return	0;
}
Connecter *	GMS_MAINSTRUCTURE::NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
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
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(27,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::NewConnecter,%d,pCntr->Init,%s", rst, cmn_get_ipnportstr_by_saaddr(paddr));
	}
	else
	{
		memcpy(&pCntr->m_addr, paddr, sizeof(pCntr->m_addr));
	}
	return	pCntr;
}
void	GMS_MAINSTRUCTURE::AfterAddConnecter(Connecter * pCntr)
{
	MYCNTRSVR::MYCNTR	*pMyCntr	= (MYCNTRSVR::MYCNTR *)pCntr->QueryInterface();
	if( !pMyCntr )
	{
		// ���Ӧ����accepter
		return;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(28,PNGS_INT_RLS)"GMS_MAINSTRUCTURE new connecter,0x%X,%s", pMyCntr->GetIDInMan(), cmn_get_ipnportstr_by_saaddr(&pMyCntr->m_addr));
	// ������ʱ�������û���յ�Hello����Ͽ�����
	SetTE_Hello_TimeOut(pCntr);
}
void	GMS_MAINSTRUCTURE::InnerRouteClientGameCmd(CMN::cmd_t nCmd, CLS_SVR_CLIENT_DATA_T *pCmd, size_t nSize)
{
	nSize	-= wh_offsetof(CLS_SVR_CLIENT_DATA_T, data);
	assert( (int)nSize>0 );	// CLS�ǲ��ᴫ��������0�����ݵ�
	// �ϳ�ָ��
	// ͬ�߳�
	GMSPACKET_2GP_PLAYERCMD_T	Cmd2GP;
	Cmd2GP.nClientID		= pCmd->nClientID;
	Cmd2GP.nChannel			= pCmd->nChannel;
	Cmd2GP.nDSize			= nSize;
	Cmd2GP.pData			= pCmd->data;
	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, nCmd, Cmd2GP);
}
void	GMS_MAINSTRUCTURE::InnerRouteGZSGameCmd(MYCNTRSVR::MYCNTR *pCntr, GZS_GMS_GAMECMD_T *pGameCmd, size_t nSize)
{
	GZSUnit	*pGZS	= (GZSUnit *)pCntr->m_pExtPtr;
	size_t	nDSize	= nSize - wh_offsetof(GZS_GMS_GAMECMD_T, data);
	assert( (int)nDSize>0 );
	if( pGameCmd->nToSvrIdx == PNGS_SVRIDX_GMS )
	{
		// �Ǹ�GMS��
		if( !m_pLogicGamePlay )
		{
			// ���û�������Ȼ�Ͳ�����
			return;
		}

		// �ϳ�ָ��
		GMSPACKET_2GP_SVRCMD_T	Cmd2GP;
		Cmd2GP.nSvrIdx			= pGZS->nSvrIdx;
		Cmd2GP.nDSize			= nDSize;
		Cmd2GP.pData			= pGameCmd->data;
		CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, GMSPACKET_2GP_SVRCMD, Cmd2GP);
	}
	else if( pGameCmd->nToSvrIdx == PNGS_SVRIDX_ALLGZS )
	{
		// ���е�GZS
		SendSvrCmdToAllGZS(pGameCmd->data, nDSize, pGZS->nSvrIdx);
	}
	else if( pGameCmd->nToSvrIdx == PNGS_SVRIDX_ALLGZSEXCEPTME )
	{
		// ���е�GZS
		SendSvrCmdToAllGZS(pGameCmd->data, nDSize, pGZS->nSvrIdx, pGZS->nSvrIdx);
	}
	else if( pGameCmd->nToSvrIdx>=GZS_MAX_NUM )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(,PNGS_RT)"GMS_MAINSTRUCTURE::InnerRouteGZSGameCmd,%d,bad nToSvrIdx,0x%X,%d", pGameCmd->nToSvrIdx);
	}
	else
	{
		// �����ת����Ӧ��GZS
		SendSvrCmdToGZS(pGameCmd->nToSvrIdx, pGameCmd->data, nDSize, pGZS->nSvrIdx);
	}
}
void	GMS_MAINSTRUCTURE::TellAllConnecterToQuit()
{
	TellAllCAAFSQuit();
	TellAllCLSQuit();
	TellAllGZSQuit();
}
void	GMS_MAINSTRUCTURE::TellAllCAAFSQuit()
{
	GMS_CAAFS_CTRL_T	Ctrl;
	Ctrl.nCmd			= GMS_CAAFS_CTRL;
	Ctrl.nSubCmd		= GMS_CAAFS_CTRL_T::SUBCMD_EXIT;
	for(int i=0;i<GMS_MAX_CAAFS_NUM;i++)
	{
		Ctrl.nCAAFSIdx	= i;
		MYCNTRSVR::MYCNTR	*pCntr	= m_pSHMData->m_aCAAFSGroup[i].pCntr;
		if( pCntr != NULL )
		{
			pCntr->SendMsg(&Ctrl, sizeof(Ctrl));
		}
	}
}
void	GMS_MAINSTRUCTURE::TellAllCLSQuit()
{
	SVR_CLS_CTRL_T		Ctrl;
	Ctrl.nCmd			= SVR_CLS_CTRL;
	Ctrl.nSubCmd		= SVR_CLS_CTRL_T::SUBCMD_EXIT;
	for(UAF_CLS_T::iterator it=m_pSHMData->m_CLSs.begin(); it!=m_pSHMData->m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, &Ctrl, sizeof(Ctrl));
	}
}
void	GMS_MAINSTRUCTURE::TellAllGZSQuit()
{
	GMS_GZS_CTRL_T		Ctrl;
	Ctrl.nCmd			= GMS_GZS_CTRL;
	Ctrl.nSubCmd		= GMS_GZS_CTRL_T::SUBCMD_EXIT;
	SendCmdToAllGZS(&Ctrl, sizeof(Ctrl));
	// �������е�GZSΪ�����˳�״̬
	for(int i=1;i<GZS_MAX_NUM;++i)
	{
		GZSUnit	*pGZS	= &m_pSHMData->m_GZSs[i];
		pGZS->nProp		|= GZSUnit::PROP_GOODEXIT;
	}
}
void	GMS_MAINSTRUCTURE::TellCLSToKickPlayer(int nClientID, pngs_cmd_t nSubCmd)
{
	SVR_CLS_CTRL_T	Ctrl;
	Ctrl.nCmd		= SVR_CLS_CTRL;
	Ctrl.nSubCmd	= nSubCmd;
	Ctrl.nParam		= nClientID;
	// �ҵ�Client���ڵ�CLS������֮
	if( SendCmdToPlayerCLS(nClientID, &Ctrl, sizeof(Ctrl)) < 0 )
	{
		// ������ܷ����ˣ���ֱ��ģ������ϲ��û�OFFLINE��
		GMSPACKET_2GP_PLAYEROFFLINE_T	CmdPlayerOut;
		CmdPlayerOut.nClientID	= nClientID;
		CmdPlayerOut.nReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTEXIST;

		CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_PLAYEROFFLINE, &CmdPlayerOut, sizeof(CmdPlayerOut));
	}
}
void	GMS_MAINSTRUCTURE::TellCLSToCheckPlayerOffline(CLSUnit *pCLS)
{
	m_vectrawbuf.resize(sizeof(SVR_CLS_CTRL_T)+pCLS->dlPlayer.size()*sizeof(int));
	SVR_CLS_CTRL_T	*pCtrl	= (SVR_CLS_CTRL_T *)m_vectrawbuf.getbuf();
	pCtrl->nCmd				= SVR_CLS_CTRL;
	pCtrl->nSubCmd			= SVR_CLS_CTRL_T::SUBCMD_CHECKPLAYEROFFLINE;
	pCtrl->nParam			= pCLS->dlPlayer.size();
	int		*panID			= (int *)wh_getptrnexttoptr(pCtrl);
	// ����һ�����CLS��Ӧ���������
	for(whDList<PlayerUnit *>::node *pNode=pCLS->dlPlayer.begin(); pNode!=pCLS->dlPlayer.end(); pNode=pNode->next)
	{
		*panID				= pNode->data->nID;
		++panID;
	}
	pCLS->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}
void	GMS_MAINSTRUCTURE::TellGZSToCheckPlayerOffline(GZSUnit *pGZS)
{
	int	nNum				= m_pSHMData->m_Players.size();
	m_vectrawbuf.resize(sizeof(GMS_GZS_CTRL_T)+nNum*sizeof(int));
	GMS_GZS_CTRL_T	*pCtrl	= (GMS_GZS_CTRL_T *)m_vectrawbuf.getbuf();
	pCtrl->nCmd				= GMS_GZS_CTRL;
	pCtrl->nSubCmd			= GMS_GZS_CTRL_T::SUBCMD_CHECKPLAYEROFFLINE;
	pCtrl->nParam			= nNum;
	int		*panID			= (int *)wh_getptrnexttoptr(pCtrl);
	// ����һ�����GZS��Ӧ���������
	for(UAF_Player_T::iterator it=m_pSHMData->m_Players.begin(); it!=m_pSHMData->m_Players.end(); ++it)
	{
		*panID				= (*it).nID;
		++panID;
	}
	pGZS->pCntr->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}
int		GMS_MAINSTRUCTURE::DealGZSDisconnection(GZSUnit *pGZS)
{
	unsigned char	nProp	= pGZS->nProp;
	// ֪ͨ����CLS��GZS���ߣ���ֹ����GZS�ĵ����������Ϊ��
	// һ��GZS��CLS������CLS�������к͸�GZS��ص��û�����
	SVR_CLS_CTRL_T	Cmd;
	Cmd.nCmd		= SVR_CLS_CTRL;
	Cmd.nSubCmd		= SVR_CLS_CTRL_T::SUBCMD_DROPGZS;
	Cmd.nParam		= pGZS->nSvrIdx;
	SendCmdToAllCLS(&Cmd, sizeof(Cmd));
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(850,PNGS_RT)"DealGZSDisconnection,%d", pGZS->nSvrIdx);
	// ���GZS�Ѿ�֪ͨ������������˳��ģ���֪ͨ����CLS����͸�GZS�йص��û�
	// ����Ǳ�Ҫ�ģ���Ϊ������������GZS�����е��û�
	if( !m_cfginfo.bGZSDownRestorePlayer )
	{
		nProp	|= GZSUnit::PROP_GOODEXIT;
	}
	if( (nProp & GZSUnit::PROP_GOODEXIT) )
	{
		Cmd.nSubCmd	= SVR_CLS_CTRL_T::SUBCMD_KICKPLAYEROFGZS;
		Cmd.nParam	= pGZS->nSvrIdx;
		SendCmdToAllCLS(&Cmd, sizeof(Cmd));
	}
	// ֪ͨGP
	GMSPACKET_2GP_GZSOFFLINE_T	GZSOL;
	GZSOL.nSvrIdx	= pGZS->nSvrIdx;
	GZSOL.nMode		= GMSPACKET_2GP_GZSOFFLINE_T::MODE_REALLYOFF;
	CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSOFFLINE, &GZSOL, sizeof(GZSOL));
	// Free���GZS
	pGZS->clear();
	// ��prop�����ȥ�������´�ʹ�õ�ʱ�����֪���ϴ��Ƿ��������˳���
	pGZS->nProp		= nProp;
	return	0;
}
int		GMS_MAINSTRUCTURE::DealCLSDisconnection(CLSUnit *pCLS)
{
	// ������к�CLS��ص���ң�nCount�н���¼CLS�е����������
	int	nCount	= 0;
	whDList<PlayerUnit *>::node	*pNode	= pCLS->dlPlayer.begin();
	while( pNode != pCLS->dlPlayer.end() )
	{
		PlayerUnit	*pPlayer	= pNode->data;
		// ��һ��(����RemovePlayerUnit�ᵼ����һ��ʧЧ)
		pNode	= pNode->next;
		// �Ǻ����CLS��ص�
		RemovePlayerUnit(pPlayer, CLS_GMS_CLIENT_DROP_T::REMOVEREASON_AS_DROP);
		nCount	++;
	}
	// ע�⣺�����ȴ���������ٴ���CLS��������ҿ��ܻ��õ�CLS��һЩ����
	// ��Group��ɾ��
	pCLS->dlnode.leave();
	// ɾ������
	pCLS->clear();
	m_pSHMData->m_CLSs.FreeUnitByPtr(pCLS);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,PNGS_RT)"DealCLSSDisconnection,0x%X,%d", pCLS->nID, nCount);
	return	0;
}
