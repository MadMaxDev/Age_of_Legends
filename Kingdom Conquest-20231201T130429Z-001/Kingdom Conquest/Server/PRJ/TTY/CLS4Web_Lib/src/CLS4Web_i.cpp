#include "../inc/CLS4Web_i.h"

using namespace n_pngs;

// ����CFGINFO_T
WHDATAPROP_MAP_BEGIN_AT_ROOT(n_pngs::CLS4Web_i::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szWelcomeAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szLogicProxyAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nGroupID, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nConnectOtherSvrTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCloseFromLPTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nHelloTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nClientConnectToMeTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCntrSendRecvBuf, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCmdStatInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szSelfInfoToClient, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxRecvCount, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxRecvSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCheckRecvInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nBigTickFilter, 0)
WHDATAPROP_MAP_END()

int		CLS4Web_i::Organize()
{
	return 0;
}

int		CLS4Web_i::Detach(bool bQuick)
{
	return 0;
}

CLS4Web_i::CLS4Web_i()
: m_nStatus(STATUS_NOTHING)
, m_tickNow(0), m_nCLS4WebID(0)
, m_pEpollServer(NULL)
, m_bShouldStop(false)
{
	m_vectIDToDestroy.reserve(256);
	m_vectrawbuf.reserve(16*1024);
	m_vectCmdStat.resize(64*1024);
	memset(m_vectCmdStat.getbuf(), 0, m_vectCmdStat.totalbytes());
}
CLS4Web_i::~CLS4Web_i()
{
}
int		CLS4Web_i::Init(const char* cszCFGName)
{
	m_tickNow		= wh_gettickcount();
	int		nRst	= 0;
	if ((nRst=Init_CFG(cszCFGName)) < 0)
	{
		return -1;
	}

	// ����,����LP�������Ƿ�ɹ�
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1376,CLS4Web_INT_RLS)"CLS4Web connecting LP,%s", m_cfginfo.szLogicProxyAddr);
	// ���Լ�����һ��
	m_msgerLP.m_pHost		= this;
	// �������Ӷ���
	if ((nRst=m_msgerLP.Init(m_MSGER_INFO.GetBase(), m_cfginfo.szLogicProxyAddr)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1376,CLS4Web_INT_RLS)"m_msgerLP.Init,%d", nRst);
		return -2;
	}
	
	// ѭ���ȴ�����
	bool		bStop		= false;
	size_t		nSize		= 0;
	LP_CLS4Web_HI_T*	pHI	= NULL;
	while (!bStop)
	{
		wh_sleep(10);
		if (m_msgerLP.GetStatus() == MYMSGER_T::STATUS_TRYCONNECT)
		{
			if (wh_tickcount_diff(wh_gettickcount(), m_msgerLP.m_tickWorkBegin) >= m_cfginfo.nConnectOtherSvrTimeOut)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1376,CLS4Web_INT_RLS)"connect to LP timeout,%s", m_cfginfo.szLogicProxyAddr);
				return -11;
			}
			m_msgerLP.Tick();
			continue;
		}
		// �ܵ�����˵���Ѿ�������
		if (wh_tickcount_diff(wh_gettickcount(), m_msgerLP.m_tickWorkBegin) >= m_cfginfo.nHelloTimeOut)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1376,CLS4Web_INT_RLS)"LP did not respone to HI", m_cfginfo.szLogicProxyAddr);
			return -12;
		}
		m_msgerLP.Tick();
		// �����Ƿ��յ�����
		pHI		= (LP_CLS4Web_HI_T*)m_msgerLP.PeekMsg(&nSize);
		if (pHI != NULL)
		{
			switch (pHI->nCmd)
			{
			case LP_CLS4Web_HI:
				{
					// ֹͣѭ��
					bStop		= true;
				}
				break;
			}
			m_msgerLP.FreeMsg();		// �����ǰ�ȫ��,pHI��Ȼ����Ч��ָ��(FreeMsg�����������ڴ�)
		}
	}
	if (pHI->nRst != LP_CLS4Web_HI_T::RST_OK)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1376,CLS4Web_INT_RLS)"LP_CLS4Web_HI bad rst,%d", pHI->nRst);
		return -3;
	}
	if (LPMainStructure_VER != pHI->nLPVer)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1376,CLS4Web_INT_RLS)"LP_CLS4Web_HI bad LP ver,%d,%d", pHI->nLPVer, LPMainStructure_VER);
		return -4;
	}
	
	// �����ȴ��û����ӵ�epoll_server
	m_pEpollServer	= new epoll_server();
	if (m_pEpollServer == NULL)
	{
		assert(0);
		return -5;
	}
	// �����������LPָ��
	m_epollServerInfo.iMaxConnections	= pHI->nCLS4WebMaxConnection;
	if ((nRst=m_pEpollServer->Init(&m_epollServerInfo)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1377,CLS4Web_INT_RLS)"m_pEpollServer->Init,%d", nRst);
		return -6;
	}

	// ��ȡ�����Ƶ�ַ
	sockaddr_in				addr;
	cmn_get_saaddr_by_ip(&addr, m_epollServerInfo.szIP);
	// ���Ͷ˿ں͵�ַ
	CLS4Web_LP_HI1_T		Cmd1;
	Cmd1.nCmd				= CLS4Web_LP_HI1;
	Cmd1.nPort				= m_epollServerInfo.iPort;
	Cmd1.IP					= addr.sin_addr.s_addr;
	if (Cmd1.IP == 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1377,CLS4Web_INT_RLS)"must set a IP to bind(CLS4Web->szWelcomeAddr)");
		return -7;
	}
	m_msgerLP.SendMsg(&Cmd1, sizeof(Cmd1));
	if ((nRst=m_Players.Init(pHI->nLPMaxPlayer)) < 0)
	{
		assert(0);
		return -8;
	}

	// ��ʼ��ʱ�����
	whtimequeue::INFO_T		tqinfo;
	tqinfo.nUnitLen			= sizeof(TQUNIT_T);
	tqinfo.nChunkSize		= m_cfginfo.nTQChunkSize;
	if ((nRst=m_TQ.Init(&tqinfo)) < 0)
	{
		assert(0);
		return -9;
	}

	// �����Լ���ID
	m_nCLS4WebID			= pHI->nCLS4WebID;

	// Ȼ����빤��״̬
	SetStatus(STATUS_WORKING);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1377,CLS4Web_INT_RLS)"OK,Work begin,0x%X", m_nCLS4WebID);

	return 0;
}
int		CLS4Web_i::Init_CFG(const char* cszCFG)
{
	WHDATAINI_CMN		ini;
	ini.addobj("CLS4Web", &m_cfginfo);
	ini.addobj("MSGER_LP", &m_MSGER_INFO);
	ini.addobj("EPOLL_SERVER", &m_epollServerInfo);
	// ����һ��Ĭ��ֵ
	m_MSGER_INFO.nSendBufSize		= 10*1024*1024;
	m_MSGER_INFO.nRecvBufSize		= 10*1024*1024;
	m_MSGER_INFO.nSockSndBufSize	= 1*1024*1024;
	m_MSGER_INFO.nSockRcvBufSize	= 1*1024*1024;
	//
	int		nRst	= ini.analyzefile(cszCFG);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1377,CLS4Web_INT_RLS)"Init_CFG,ini.analyzefile,%d,%s", nRst, ini.printerrreport());
		return -1;
	}

	// �󶨵�ַ
	wh_strsplit("sd", m_cfginfo.szWelcomeAddr, ":", m_epollServerInfo.szIP, &m_epollServerInfo.iPort);

	// ����ѭ��
	m_loopCmdStat.setinterval(m_cfginfo.nCmdStatInterval);

	return 0;
}
int		CLS4Web_i::Init_CFG_Reload(const char* cszCFGName)
{
	WHDATAINI_CMN	ini;
	ini.addobj("CLS4Web", &m_cfginfo);
	ini.addobj("MSGER_LP", &m_MSGER_INFO);
	ini.addobj("EPOLL_SERVER", &m_epollServerInfo);
	//
	int		nRst	= ini.analyzefile(cszCFGName, false, 1);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1377,CLS4Web_INT_RLS)"Init_CFG_Reload,ini.analyzefile,%d,%s", nRst, ini.printerrreport());
		return -1;
	}

	// �󶨵�ַ
	wh_strsplit("sd", m_cfginfo.szWelcomeAddr, ":", m_epollServerInfo.szIP, &m_epollServerInfo.iPort);

	// ����ѭ��
	m_loopCmdStat.setinterval(m_cfginfo.nCmdStatInterval);

	return 0;
}
int		CLS4Web_i::Release()
{
	// ������Ԫ���
	m_Players.BeginGet();
	PlayerUnit*	pPlayer		= m_Players.GetNext();
	while (pPlayer != NULL)
	{
		int		nID	= pPlayer->nID;
		pPlayer		= m_Players.GetNext();
		RemovePlayerUnit(nID);
	}
	m_Players.Release();

	// tagӳ���ͷ�
	whhash<unsigned int, DLPUNIT_T>::kv_iterator	it	= m_mapTag2DL.begin();
	for (; it!=m_mapTag2DL.end(); ++it)
	{
		it.getvalue().clear(this);
	}
	m_mapTag2DL.clear();
	// 64λtagӳ���ͷ�
	whhash<whuint64, DLPUNIT_T>::kv_iterator	it64	= m_map64Tag2DL.begin();
	for (; it64!=m_map64Tag2DL.end(); ++it64)
	{
		it64.getvalue().clear(this);
	}
	m_map64Tag2DL.clear();

	// epollserver�ս�
	WHSafeSelfDestroy(m_pEpollServer);

	// �����Լ������ı�־
	CLS4Web_LP_CTRL_T	C;
	C.nCmd				= CLS4Web_LP_CTRL;
	C.nSubCmd			= CLS4Web_LP_CTRL_T::SUBCMD_EXIT;
	m_msgerLP.SendMsg(&C, sizeof(C));

	// ����LP����Ϣ����ȥ(LP��رպ�CLS4Web������)
	whtick_t	t		= wh_gettickcount();
	while (m_msgerLP.GetStatus()!=MYMSGER_T::STATUS_TRYCONNECT
		&& wh_tickcount_diff(wh_gettickcount(), t)<m_cfginfo.nCloseFromLPTimeOut)
	{
		m_msgerLP.Tick();
		wh_sleep(10);
	}
	// tcp�ս�
	m_msgerLP.Release();
	m_nStatus			= STATUS_NOTHING;

	return 0;
}
void	CLS4Web_i::SetStatus(int nStatus)
{
	m_nStatus			= nStatus;
}
int		CLS4Web_i::GetStatus() const
{
	return m_nStatus;
}
int		CLS4Web_i::RemovePlayerUnit(int nPlayerID)
{
	return RemovePlayerUnit(m_Players.getptr(nPlayerID));
}
int		CLS4Web_i::RemovePlayerUnit(PlayerUnit* pPlayer)
{
	if (pPlayer == NULL)
	{
		return -1;
	}

	if (m_nStatus == STATUS_WORKING)
	{
		// ֻ����������״̬�²���Ҫͨ���ϲ�,����,���ñ�(Ŀǰ��STATUS_KICK_ALL_PLAYER״̬)
		switch (pPlayer->nStatus)
		{
		case PlayerUnit::STATUS_CONNECTING:
			{
				CLS4Web_LP_CLIENTTOCLS4Web_RST_T	CTCRst;
				CTCRst.nCmd			= CLS4Web_LP_CLIENTTOCLS4Web_RST;
				CTCRst.nRst			= CLS4Web_LP_CLIENTTOCLS4Web_RST_T::RST_ERR_NOTCONNECT;
				CTCRst.nClientID	= pPlayer->nID;
				m_msgerLP.SendMsg(&CTCRst, sizeof(CTCRst));
			}
			break;
		case PlayerUnit::STATUS_WORKING:
			{
				CLS4Web_LP_CLIENT_DROP_T		CD;
				CD.nCmd				= CLS4Web_LP_CLIENT_DROP;
				CD.nRemoveReason	= pPlayer->nRemoveReason;
				CD.nClientID		= pPlayer->nID;
				m_msgerLP.SendMsg(&CD, sizeof(CD));
			}
			break;
		}
	}

	if (m_pEpollServer->IsCntrExist(pPlayer->nCntrID))
	{
		// ȡ����cntr�Ĺ�ϵ,����cntr֪ͨ�ϲ�close��ʱ��Ҫ��ͼɾ�����
		m_pEpollServer->SetExt(pPlayer->nCntrID, 0);
		// �ر�cntr
		m_pEpollServer->Close(pPlayer->nCntrID, pPlayer->nRemoveReason);
	}

	// ��������
	pPlayer->clear(this);
	// �Ƴ�
	m_Players.FreeUnitByPtr(pPlayer);

	return 0;
}
CLS4Web_i::PlayerUnit*		CLS4Web_i::GetPlayerByCntrID(int nCntrID)
{
	int		nExt;
	if (m_pEpollServer->GetExt(nCntrID, &nExt) < 0)
	{
		return NULL;
	}
	return m_Players.getptr(nExt);
}
int		CLS4Web_i::SendDataToSinglePlayer(PlayerUnit* pPlayer, void* pData, int nSize, pngs_cmd_t nSendCmd)
{
	if (pPlayer == NULL)
	{
		return -1;
	}
	m_vectrawbuf.resize(nSize+wh_offsetof(CLS4Web_CLIENT_DATA_T,data));
	CLS4Web_CLIENT_DATA_T*	pDataToClient		= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pDataToClient->nCmd		= nSendCmd;
	memcpy(pDataToClient->data, pData, nSize);
	return SendRawCmdToSinglePlayer(pPlayer, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}
int		CLS4Web_i::SendRawCmdToSinglePlayer(int nPlayerID, void* pRawCmd, int nSize)
{
	return SendRawCmdToSinglePlayer(m_Players.getptr(nPlayerID), pRawCmd, nSize);
}
int		CLS4Web_i::SendRawCmdToSinglePlayer(CLS4Web_i::PlayerUnit* pPlayer, void* pRawCmd, int nSize)
{
	if (pPlayer == NULL)
	{
		return -1;
	}
	// �ж����ݳ����Ƿ�������
	if (nSize <= 0)
	{
		assert(0);
		return -2;
	}
#ifdef _DEBUG
	if (nSize >= 3)
	{
		unsigned short	nIdx	= *(unsigned short*)wh_getoffsetaddr(pRawCmd, 1);
		CMDSTAT_T&		cs		= m_vectCmdStat[nIdx];
		cs.nByte				+= nSize;
		cs.nCount				++;
	}
#endif
	// ����
	int		nRst	= m_pEpollServer->Send(pPlayer->nCntrID, pRawCmd, nSize);
	if (nRst < 0)
	{
		// ���������(���ݷ��ص�nRst�����ж�)
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1379,CLS4Web_RT)"CLS4Web_i::SendRawCmdToSinglePlayer,%d,Send,0x%X,%d,0x%X", nRst, pPlayer->nID, nSize, pPlayer->nCntrID);
		pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_SENDERR;
		RemovePlayerUnit(pPlayer);
		return -3;
	}
	return 0;
}
int		CLS4Web_i::SendRawCmdToAllPlayer(void* pRawCmd, int nSize)
{
	whunitallocatorFixed<PlayerUnit>::iterator	it	= m_Players.begin();
	while (it != m_Players.end())
	{
		PlayerUnit*	pPlayer	= &(*it);
		++it;
		SendRawCmdToSinglePlayer(pPlayer, pRawCmd, nSize);
	}
	return 0;
}
int		CLS4Web_i::AddPlayerToTagList(PlayerUnit* pPlayer, unsigned char nTagIdx, short nTagVal)
{
	if (nTagVal == 0)
	{
		return 0;
	}
	PlayerUnit::TAGINFO_T&	tagInfo		= pPlayer->taginfo[nTagIdx];
	if (tagInfo.nValue != 0)
	{
		// �������ڶ�������
		assert(0);
		return -2;
	}

	// �õ�hash��ֵ
	unsigned int	nHashKey	= CreateHashKey(nTagIdx, nTagVal);
	// ���Ҷ���,���û�о�����һ��
	DLPUNIT_T*		pDLU;
	if (!m_mapTag2DL.get(nHashKey, &pDLU))
	{
		if (!m_mapTag2DL.putalloc(nHashKey, pDLU))
		{
			assert(0);
			return -3;
		}
		// ֻ���¶��в���Ҫnewһ������
		pDLU->pDLP		= NewDLP();
	}
	assert(pDLU->pDLP);
	// ����Ҽ����������
	pDLU->pDLP->AddToTail(&tagInfo.nodeTag);
	// ����ֵ
	tagInfo.nValue		= nTagVal;

	return 0;
}
int		CLS4Web_i::DelPlayerFromTagList(PlayerUnit* pPlayer, unsigned char nTagIdx)
{
	PlayerUnit::TAGINFO_T&	taginfo		= pPlayer->taginfo[nTagIdx];
	if (taginfo.nValue == 0)
	{
		// �����Ƴ���,�����Ͳ��ڶ�����
		return 0;
	}
	// �õ�hash��ֵ
	unsigned int	nHashKey	= CreateHashKey(nTagIdx, taginfo.nValue);
	// ��ö��ж���
	whhash<unsigned int, DLPUNIT_T>::kv_iterator	it	= m_mapTag2DL.find(nHashKey);
	if (it == m_mapTag2DL.end())
	{
		assert(0);
		return -1;
	}
	DLPUNIT_T&		dlu			= it.getvalue();
	if (dlu.pDLP != taginfo.nodeTag.pHost)
	{
		assert(0);
		return -2;
	}
	// ���
	taginfo.nodeTag.leave();
	// �����ϵ
	taginfo.nValue		= 0;
	// ������������Ϊ0��ɾ��֮
	if (dlu.pDLP->size() <= 0)
	{
		dlu.clear(this);
		m_mapTag2DL.erase(it);
	}
	return 0;
}
int		CLS4Web_i::AddPlayerToTag64List(PlayerUnit* pPlayer, whuint64 nTag)
{
	// ������Ҳ����Ѿ��ڶ�����
	if (pPlayer->map64Tag2Node.has(nTag))
	{
		// �Ѿ�����
		return 0;
	}
	
	// ��ö��ж���
	DLPUNIT_T*		pDLU;
	if (!m_map64Tag2DL.get(nTag, &pDLU))
	{
		if (!m_map64Tag2DL.putalloc(nTag, pDLU))
		{
			assert(0);
			return -1;
		}
		// ֻ���¶�����Ҫnewһ������
		pDLU->pDLP				= NewDLP();
	}
	assert(pDLU->pDLP);
	// ����Ҽ����������
	DLPLAYER_NODE_T*	pNode	= new DLPLAYER_NODE_T;
	assert(pNode);
	pNode->data		= pPlayer;
	pDLU->pDLP->AddToTail(pNode);
	pPlayer->map64Tag2Node.put(nTag, pNode);
	return 0;
}
int		CLS4Web_i::DelPlayerFromTag64List(PlayerUnit* pPlayer, whuint64 nTag)
{
	whhash<whuint64, DLPLAYER_NODE_T*>::kv_iterator	it	= pPlayer->map64Tag2Node.find(nTag);
	if (it == pPlayer->map64Tag2Node.end())
	{
		// �Ѿ����ڶ�����
		return 0;
	}
	DLPLAYER_NODE_T*&	pNode	= it.getvalue();
	// �˶�
	pNode->leave();
	// ɾ��
	WHSafeDelete(pNode);
	pPlayer->map64Tag2Node.erase(it);
	return 0;
}
CLS4Web_i::DLPLAYER_T*	CLS4Web_i::NewDLP()
{
	return new DLPLAYER_T;
}
void	CLS4Web_i::SafeDeleteDLP(CLS4Web_i::DLPLAYER_T*& pDLP)
{
	WHSafeDelete(pDLP);
}
int		CLS4Web_i::GetSockets(whvector<SOCKET>& vect)
{
	SOCKET	sock		= m_msgerLP.GetSocket();
	if (cmn_is_validsocket(sock))
	{
		vect.push_back(sock);
	}
	m_pEpollServer->GetSockets(vect);
	return vect.size();
}
bool	CLS4Web_i::ShouldStop() const
{
	return m_bShouldStop;
}
void	CLS4Web_i::Retry_Worker_WORKING_Begin()
{
	CLS4Web_LP_HI_T			hi;
	hi.nCmd					= CLS4Web_LP_HI;
	hi.nGroupID				= m_cfginfo.nGroupID;
	hi.nCLS4WebVer			= CLS4Web_VER;
	hi.nOldID				= m_nCLS4WebID;
	m_msgerLP.SendMsg(&hi, sizeof(hi));

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1376,CLS4Web_INT_RLS)"Send HI to LP,%d,%d,%d", hi.nGroupID, hi.nCLS4WebVer, m_nCLS4WebID);
}
CLS4Web_i::PlayerUnit*		CLS4Web_i::TryGetPlayerAndSendNotifyToLPIsNotExisted(int nPlayerID)
{
	PlayerUnit*	pPlayer		= m_Players.getptr(nPlayerID);
	if (pPlayer == NULL)
	{
		// ֱ�Ӱ����û��Ѿ������ڷ��ͽ����LP
		CLS4Web_LP_CLIENT_DROP_T	CD;
		CD.nCmd				= CLS4Web_LP_CLIENT_DROP;
		CD.nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTEXIST;
		CD.nClientID		= nPlayerID;
		m_msgerLP.SendMsg(&CD, sizeof(CD));
	}
	return pPlayer;
}
int		CLS4Web_i::SureSend()
{
	m_msgerLP.ManualSend();
	return 0;
}
