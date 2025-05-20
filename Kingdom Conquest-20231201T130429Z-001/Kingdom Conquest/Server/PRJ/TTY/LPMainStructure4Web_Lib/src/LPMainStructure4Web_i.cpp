#include "../inc/LPMainStructure4Web_i.h"

using namespace n_pngs;

WHDATAPROP_MAP_BEGIN_AT_ROOT(LPMainStructure4Web_i::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxPlayer, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSafeReservePlayerNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxCLS4Web, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueCmdInSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nHelloTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSvrDropWaitTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCalcAndSendCAAFS4WebAvailPlayerTimeOut, 0)
	WHDATAPROP_ON_SETVALUE( WHDATAPROP_TYPE_int|WHDATAPROP_TYPE_ARRAYBIT, anMaxCLS4WebNumInCAAFS4WebGroup, LP_MAX_CAAFS4Web_NUM, 0)
	WHDATAPROP_ON_SETVALUE( WHDATAPROP_TYPE_int|WHDATAPROP_TYPE_ARRAYBIT, anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup, LP_MAX_CAAFS4Web_NUM, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bCLSUseCAAFSIP, 0)
WHDATAPROP_MAP_END()

int		LPMainStructure4Web_i::Organize()
{
	m_pLogicTCPReceiver	= m_pCMN->GetLogic(PNGS_DLL_NAME_TCPRECEIVER, TCPRECEIVER_VER);
	if (m_pLogicTCPReceiver == NULL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1381,LPMS_i)"Organize,m_pLogicTCPReceiver is NULL");
		return -1;
	}
	PNGS_CD2TR_CONFIG_T	cfg;
	cfg.nSubCmd			= PNGS_CD2TR_CONFIG_T::SUBCMD_SET_CMDDEALER;
	if (m_pLogicTCPReceiver->CmdIn_Instant(this, PNGS_CD2TR_CONFIG, &cfg, sizeof(cfg)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1381,LPMS_i)"Organize,m_pLogicTCPReceiver set cmddealer ERROR");
		return -2;
	}

	// Ѱ��gameplayģ��(�Ҳ���������)
	m_pLogicGamePlay	= m_pCMN->GetLogic(TTY_DLL_NAME_LP_GamePlay, LPGamePlaye_VER);
	if (m_pLogicGamePlay == NULL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1381,LPMS_i)"Organize,m_pLogicGamePlay is NULL");
		return -3;
	}

	return 0;
}
int		LPMainStructure4Web_i::Detach(bool bQuick)
{
	// ɾ������
// 	if (m_am != NULL)
// 	{
// 		m_am->m_nUseCount--;
// 		if (m_pShmData != NULL)
// 		{
// 			MYSHMDATA_T::DeleteMe(m_am, m_pShmData);
// 		}
// 		m_am		= NULL;
// 	}
	return 0;
}

// ������
LPMainStructure4Web_i::LPMainStructure4Web_i()
: m_tickNow(0)
// , m_pLogicMem(NULL)
// , m_ao(NULL)
// , m_am(NULL)
// , m_pShmData(NULL)
{
	m_vectrawbuf.reserve(PNGS_RAWBUF_SIZE);
	strcpy(FATHERCLASS::m_szLogicType, TTY_DLL_NAME_LP_MainStructure);
	FATHERCLASS::m_nVer	= LPMainStructure_VER;
}
LPMainStructure4Web_i::~LPMainStructure4Web_i()
{
}
int		LPMainStructure4Web_i::Init(const char *cszCFGName)
{
	m_tickNow	= wh_gettickcount();

	if (Init_CFG(cszCFGName) < 0)
	{
		return -1;
	}

	int	nRst	= 0;

	// ��ʼ��ʱ�����
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if ((nRst=m_TQ.Init(&tqinfo)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1381,LPMS_i)"Init,m_TQ.Init,%d", nRst);
		return -2;
	}
	// ���붨ʱ����CAAFS4Web�ɽ���������ʱ���¼�
	TQUNIT_T*	pTQUnit;
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCalcAndSendCAAFS4WebAvailPlayerTimeOut, (void**)&pTQUnit, &m_teid_CalcAndSend_CAAFS4Web_AvailPlayerNum) < 0)
	{
		assert(0);
		return -3;
	}
	pTQUnit->tefunc		= &LPMainStructure4Web_i::TEDeal_CalcAndSend_CAAFS4Web_AvailPlayerNum;
	
	// Ѱ���ڴ����ģ��(�ڴ�ģ��һ������Ҫ��һ���������)
// 	m_pLogicMem			= m_pCMN->GetLogic(PNGS_DLL_NAME_GSMEM, GSMEM_VER);
// 	if (m_pLogicMem == NULL)
// 	{
// 		return -4;
// 	}
// 	// ��ȡ�ڴ������
// 	{
// 		PNGSPACKET_2MEM_XXX_PTR_T	XPtr;
// 		XPtr.pcszKey	= PNGS_GSMEM_NAME_AM;
// 		if (m_pLogicMem->CmdIn_Instant(this, PNGSPACKET_2MEM_GET_PTR, &XPtr, sizeof(XPtr)) < 0)
// 		{
// 			return -10;
// 		}
// 		m_am			= (AM_T*)XPtr.ptr;
// 		m_am->m_nUseCount++;
// 		m_ao			= m_am->GetAO();
// 	}
// 	// �жϹ����ڴ��Ƿ��Ǽ̳е�
// 	{
// 		PNGSPACKET_2MEM_CONFIG_T	config;
// 		config.nSubCmd	= PNGSPACKET_2MEM_CONFIG_T::SUBCMD_GET_MEMSTATUS;
// 		if (m_pLogicMem->CmdIn_Instant(this, PNGSPACKET_2MEM_CONFIG, &config, sizeof(config)) < 0)
// 		{
// 			return -11;
// 		}
// 		if (config.nParam == 0)
// 		{
// 			// ������ʼ����
// 			// ���ɶ������û�ȥ
// 			m_pShmData		= MYSHMDATA_T::NewMe(m_am);
// 			assert(m_pShmData != NULL);
// 			PNGSPACKET_2MEM_XXX_PTR_T	XPtr;
// 			XPtr.pcszKey	= FATHERCLASS::m_szLogicType;
// 			XPtr.ptr		= m_pShmData;
// 			if (m_pLogicMem->CmdIn_Instant(this, PNGSPACKET_2MEM_SET_PTR, &XPtr, sizeof(XPtr)) < 0)
// 			{
// 				return -12;
// 			}
// 			// ��ʼ����������(���Ӧ�ò����ܳ����,���������ļ�����)
// 			m_pShmData->m_CLS4Webs.Init(m_ao, m_cfginfo.nMaxCLS4Web);
// 			m_pShmData->m_Players.Init(m_ao, m_cfginfo.nMaxPlayer);
// 		}
// 		else
// 		{
// 			// �Ǽ̳е�
// 			int	nOffset		= config.nParam1;
// 			// ���Ǳ߻�ö��󲢽����ڲ�����
// 			m_am->m_nAdjustCount++;
// 			PNGSPACKET_2MEM_XXX_PTR_T	XPtr;
// 			XPtr.pcszKey	= FATHERCLASS::m_szLogicType;
// 			if (m_pLogicMem->CmdIn_Instant(this, PNGSPACKET_2MEM_GET_PTR, &XPtr, sizeof(XPtr)) < 0)
// 			{
// 				return -13;
// 			}
// 			m_pShmData		= (MYSHMDATA_T*)XPtr.ptr;
// 			if (m_pShmData == NULL)
// 			{
// 				return -14;
// 			}
// 			m_pShmData->AdjustInnerPtr(m_am, nOffset);
// 			//
// 			m_am->m_nAdjustCount--;
// 		}
// 	}

	// ����ָ�����
	ILogicBase::QueueInit(m_cfginfo.nQueueCmdInSize);

	if (m_Players.Init(m_cfginfo.nMaxPlayer) < 0)
	{
		return -8;
	}
	if (m_CLS4Webs.Init(m_cfginfo.nMaxCLS4Web) < 0)
	{
		return -9;
	}

	return 0;
}
int		LPMainStructure4Web_i::Init_CFG(const char* cszCFGName)
{
	// �����ļ�
	WHDATAINI_CMN	ini;
	ini.addobj("LPMainStructure", &m_cfginfo);
	
	int	nRst	= ini.analyzefile(cszCFGName);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1381,LPMS_i)"%s,%d,ini.analyzefile,%s", __FUNCTION__, nRst, ini.printerrreport());
		return -1;
	}
	m_cfginfo.nMaxCLS4Web		= 0;
	for (int i=0; i<LP_MAX_CAAFS4Web_NUM; i++)
	{
		m_cfginfo.nMaxCLS4Web	+= m_cfginfo.anMaxCLS4WebNumInCAAFS4WebGroup[i];
	}

	return 0;
}
int		LPMainStructure4Web_i::Init_CFG_Reload(const char *cszCFGName)
{
	// �����ļ�
	WHDATAINI_CMN	ini;
	ini.addobj("LPMainStructure", &m_cfginfo);

	int	nRst	= ini.analyzefile(cszCFGName, false, 1);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1381,LPMS_i)"%s,%d,ini.analyzefile,%s", __FUNCTION__, nRst, ini.printerrreport());
		return -1;
	}
	m_cfginfo.nMaxCLS4Web		= 0;
	for (int i=0; i<LP_MAX_CAAFS4Web_NUM; i++)
	{
		m_cfginfo.nMaxCLS4Web	+= m_cfginfo.anMaxCLS4WebNumInCAAFS4WebGroup[i];
	}

	return 0;
}
LPMainStructure4Web_i::CAAFS4WebGroup*	LPMainStructure4Web_i::GetCAAFS4WebByGroupID(int nGroupID)
{
	if (m_aCAAFS4WebGroup[nGroupID].nCntrID == 0)
	{
		return NULL;
	}
	return &m_aCAAFS4WebGroup[nGroupID];
}
int		LPMainStructure4Web_i::Release()
{
	ILogicBase::QueueRelease();
	return 0;
}
int		LPMainStructure4Web_i::Tick_BeforeDealCmdIn()
{
	// ��tickʱ��
	m_tickNow	= wh_gettickcount();

	// ʱ���¼�����
	Tick_DealTE();

	return 0;
}
int		LPMainStructure4Web_i::Tick_AfterDealCmdIn()
{
	if (m_nMood == CMN::ILogic::MOOD_STOPPING)
	{
		// stopping״̬�µȴ����е�cls4web��caafs4web�ر�

		// 1.cls4web
		whunitallocatorFixed<CLS4WebUnit>::iterator	itCLS4Web	= m_CLS4Webs.begin();
		for (; itCLS4Web != m_CLS4Webs.end(); ++itCLS4Web)
		{
			CLS4WebUnit&	cls4webUnit	= (*itCLS4Web);
			if (cls4webUnit.nCntrID != 0)
			{
				wh_sleep(10);
				return 0;
			}
		}

		// 2.caafs4web
		for (int i=0; i < LP_MAX_CAAFS4Web_NUM; i++)
		{
			if (m_aCAAFS4WebGroup[i].nCntrID != 0)
			{
				wh_sleep(10);
				return 0;
			}
		}

		// �ߵ�����,˵��caafs4web��cls4web��ͣ����
		m_nMood		= CMN::ILogic::MOOD_STOPPED;
	}

	return 0;
}
int		LPMainStructure4Web_i::SureSend()
{
	return 0;
}
int		LPMainStructure4Web_i::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	return 0;
}
void	LPMainStructure4Web_i::InnerRouteClientGameCmd(CMN::cmd_t nCmd, CLS4Web_SVR_CLIENT_DATA_T* pCmd, size_t nSize)
{
	// �ϳ�ָ����ϲ��߼�����
	nSize	-= wh_offsetof(CLS4Web_SVR_CLIENT_DATA_T, data);
	LPPACKET_2GP_PLAYERCMD_T	Cmd2GP;
	Cmd2GP.nClientID	= pCmd->nClientID;
	Cmd2GP.nDSize		= nSize;
	Cmd2GP.pData		= pCmd->data;
	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, nCmd, Cmd2GP);
}
void	LPMainStructure4Web_i::TellAllConnecterToQuit()
{
	TellAllCAAFS4WebQuit();
	TellAllCLS4WebQuit();
}
void	LPMainStructure4Web_i::TellAllCAAFS4WebQuit()
{
	LP_CAAFS4Web_CTRL_T		ctrl;
	ctrl.nCmd				= LP_CAAFS4Web_CTRL;
	ctrl.nSubCmd			= LP_CAAFS4Web_CTRL_T::SUBCMD_EXIT;
	for (int i=0; i<LP_MAX_CAAFS4Web_NUM; i++)
	{
		ctrl.nCAAFS4WebIdx	= i;
		int	nCntrID			= m_aCAAFS4WebGroup[i].nCntrID;
		if (nCntrID != 0)
		{
			SendCmdToConnecter(nCntrID, &ctrl, sizeof(ctrl));
		}
	}
}
void	LPMainStructure4Web_i::TellAllCLS4WebQuit()
{
	SVR_CLS4Web_CTRL_T		ctrl;
	ctrl.nCmd				= SVR_CLS4Web_CTRL;
	ctrl.nSubCmd			= SVR_CLS4Web_CTRL_T::SUBCMD_EXIT;
	whunitallocatorFixed<CLS4WebUnit>::iterator	it	= m_CLS4Webs.begin();
	for (; it!=m_CLS4Webs.end(); ++it)
	{
		SendCmdToConnecter((*it).nCntrID, &ctrl, sizeof(ctrl));
	}
}
void	LPMainStructure4Web_i::TellCLS4WebToKickPlayer(int nClientID, pngs_cmd_t nSubCmd/* =SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID */)
{
	SVR_CLS4Web_CTRL_T	ctrl;
	ctrl.nCmd			= SVR_CLS4Web_CTRL;
	ctrl.nSubCmd		= nSubCmd;
	ctrl.nParam			= nClientID;
	// �ҵ�client���ڵ�cls������֮
	if (SendCmdToPlayerCLS4Web(nClientID, &ctrl, sizeof(ctrl)) < 0)
	{
		// ���ܷ���,��ֱ��֪ͨ�ϲ��û�offline��
		LPPACKET_2GP_PLAYEROFFLINE_T		CmdPlayerOffline;
		CmdPlayerOffline.nClientID			= nClientID;
		CmdPlayerOffline.nReason			= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CLS4Web_ERR;
		CmdOutToLogic_AUTO(m_pLogicGamePlay, LPPACKET_2GP_PLAYEROFFLINE, &CmdPlayerOffline, sizeof(CmdPlayerOffline));
	}
}
void	LPMainStructure4Web_i::TellCLS4WebToCheckPlayerOffline(CLS4WebUnit* pCLS4Web)
{
	m_vectrawbuf.resize(sizeof(SVR_CLS4Web_CTRL_T)+pCLS4Web->dlPlayer.size()*sizeof(int));
	SVR_CLS4Web_CTRL_T*	pCtrl	= (SVR_CLS4Web_CTRL_T*)m_vectrawbuf.getbuf();
	pCtrl->nCmd					= SVR_CLS4Web_CTRL;
	pCtrl->nSubCmd				= SVR_CLS4Web_CTRL_T::SUBCMD_CHECKPLAYEROFFLINE;
	pCtrl->nParam				= pCLS4Web->dlPlayer.size();
	int*	pArrPlayerID		= (int*)wh_getptrnexttoptr(pCtrl);
	// ����һ�����CLS4Web��Ӧ���������
	whDList<PlayerUnit*>::node*	pNode	= pCLS4Web->dlPlayer.begin();
	for (; pNode!=pCLS4Web->dlPlayer.end(); pNode=pNode->next)
	{
		*pArrPlayerID			= pNode->data->nID;
		++pArrPlayerID;
	}
	SendCmdToConnecter(pCLS4Web->nCntrID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}
int		LPMainStructure4Web_i::DealCLS4WebDisconnection(CLS4WebUnit* pCLS4Web)
{
	// ������к�CLS4Web��ص����(nCount�н���¼CLS4Web�е��������)
	int		nCount		= 0;
	whDList<PlayerUnit*>::node*	pNode	= pCLS4Web->dlPlayer.begin();
	while (pNode != pCLS4Web->dlPlayer.end())
	{
		PlayerUnit*	pPlayer	= pNode->data;
		// ��һ��(��ȻRemovePlayerUnit�ᵼ����һ��ʧЧ)
		pNode	= pNode->next;
		RemovePlayerUnit(pPlayer, CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_AS_DROP);
		nCount++;
	}
	// ע��:�����ȴ���������ٴ���CLS4Web,������ҿ��ܻ��õ�һЩCLS4Web������
	// ��group��ɾ��
	pCLS4Web->dlnode.leave();
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,LP_RT)"%s,0x%X,%d", __FUNCTION__, pCLS4Web->nID, nCount);
	// ɾ������
	pCLS4Web->clear();
	m_CLS4Webs.FreeUnitByPtr(pCLS4Web);
	return 0;
}
int		LPMainStructure4Web_i::RemoveConnecter(int nCntrID)
{
	// ��������ɾ������
	// �������
	// TCPRecevier��ʹ�����cntr��;���ְ���ɾ����(��������������)
	//CmdOutToLogic_AUTO(m_pLogicTCPReceiver, PNGS_CD2TR_DISCONNECT, &nCntrID, sizeof(nCntrID));

	// ʹ���첽�ľ�û������,�м�,����ط�һ��ʹ���첽
	m_pLogicTCPReceiver->CmdIn_MT(this, PNGS_CD2TR_DISCONNECT, &nCntrID, sizeof(nCntrID));
	return 0;
}
