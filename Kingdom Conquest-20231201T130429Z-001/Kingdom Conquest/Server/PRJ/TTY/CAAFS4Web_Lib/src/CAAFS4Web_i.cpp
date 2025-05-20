#include "../inc/CAAFS4Web_i.h"

using namespace n_pngs;

//////////////////////////////////////////////////////////////////////////
//CAAFS4Web_i
//////////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(CAAFS4Web_i::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nVerCmpMode, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szVer, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szWelcomeAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szLogicProxyAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nConnectLogicProxyTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSelectInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nGroupID, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxPlayerRecvCmdNum, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nClientWaitingNumMax, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nClientConnectingNumMax, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nHelloCheckTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCloseCheckTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueSeqSendInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(float, fWaitTransToConnectRate, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bAtLeastOnTransToConnectPerTick, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueChannelNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szSelfInfoToClient, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSelfNotify, 0)
WHDATAPROP_MAP_END()

int		CAAFS4Web_i::Organize()
{
	return 0;
}

int		CAAFS4Web_i::Detach(bool bQuick)
{
	return 0;
}

CAAFS4Web_i::CAAFS4Web_i()
: m_tickNow(0)
, m_nSelfNotify(0)
, m_nConnectingNum(0)
{
}

CAAFS4Web_i::~CAAFS4Web_i()
{
}

int		CAAFS4Web_i::Init(const char *cszCFGName)
{
	m_tickNow		= wh_gettickcount();
	
	int		iRst	= 0;
	if ((iRst=Init_CFG(cszCFGName)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1367,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,Init_CFG,%d", iRst);
		return -1;
	}

	m_generWaitTransToConnect.SetGenSpeed(m_cfginfo.fWaitTransToConnectRate);
	m_itv.reset();

	// 创建等待用户连接的epoll_server
	m_pEpollServer		= new epoll_server();
	if (m_pEpollServer == NULL)
	{
		assert(0);
		return -2;
	}
	if ((iRst=m_pEpollServer->Init(&m_epollServerInfo)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1367,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,m_pEpollServer->Init,%d", iRst);
		return -3;
	}
	// 连接,检查和CAAFS4Web的连接是否成功
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1368,CAAFS4Web_INT_RLS)"CAAFS4Web connecting LP,%s", m_cfginfo.szLogicProxyAddr);
	// 和自己关联一下
	m_msgerLogicProxy.m_pHost	= this;
	// 创建连接对象
	if ((iRst=m_msgerLogicProxy.Init(m_MsgerLogicProxy_info.GetBase(), m_cfginfo.szLogicProxyAddr)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1368,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,m_msgerLogicProxy.Init,%d", iRst);
		return -4;
	}
	
	// 循环等待返回
	bool		bStop			= false;
	size_t		nSize;
	LP_CAAFS4Web_HI_T*		pHI;
	int			nDiff;
	while (!bStop)
	{
		m_bIsReconnect			= false;			// 这个时候实际上m_msgerLogicProxy已经设置了状态为tryconnect了,不会调用tryconnect的work_begin了
		wh_sleep(10);
		if (m_msgerLogicProxy.GetStatus() == MYMSGER_T::STATUS_TRYCONNECT)
		{
			if ((nDiff=wh_tickcount_diff(wh_gettickcount(), m_msgerLogicProxy.m_tickWorkBegin)) >= m_cfginfo.nConnectLogicProxyTimeout)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1368,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,connect to LP timeout,%d,%s", nDiff, m_cfginfo.szLogicProxyAddr);
				return -30;
			}
			m_msgerLogicProxy.Tick();
			continue;
		}
		if ((nDiff=wh_tickcount_diff(wh_gettickcount(), m_msgerLogicProxy.m_tickWorkBegin)) >= m_cfginfo.nConnectLogicProxyTimeout)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1368,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,LP not respond to HELLO,%d,%s", nDiff, m_cfginfo.szLogicProxyAddr);
			return -31;
		}
		m_msgerLogicProxy.Tick();
		// 看看是否收到返回
		pHI		= (LP_CAAFS4Web_HI_T*)m_msgerLogicProxy.PeekMsg(&nSize);
		if (pHI != NULL)
		{
			switch (pHI->nCmd)
			{
			case LP_CAAFS4Web_HI:
				{
					bStop		= true;
				}
				break;
			}
			m_msgerLogicProxy.FreeMsg();
		}
	}
	// 能到这里说明已经收到了HI消息
	if (pHI->nRst != LP_CAAFS4Web_HI_T::RST_OK)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1368,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,%d,LP_CAAFS4Web_HI_T bad rst", pHI->nRst);
		return -32;
	}
	if (LPMainStructure_VER != pHI->nLPVer)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1368,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,%d,LP_CAAFS4Web_HI_T bad ver", pHI->nLPVer);
		return -33;
	}
	
	// 设置还能承受的玩家数量
	m_nLogicProxyAvailPlayerNum		= 0;
	// 初始化玩家数组
	if ( (iRst=m_Players.Init(m_cfginfo.nClientWaitingNumMax+m_cfginfo.nClientConnectingNumMax)) < 0 )
	{
		return -41;
	}
	// 初始化时间队列
	whtimequeue::INFO_T		tqinfo;
	tqinfo.nUnitLen			= sizeof(TQUNIT_T);
	tqinfo.nChunkSize		= m_cfginfo.nTQChunkSize;
	if ((iRst=m_TQ.Init(&tqinfo)) < 0)
	{
		return -42;
	}

	// 创建排队通道
	if (m_cfginfo.nQueueChannelNum <= 0)
	{
		m_cfginfo.nQueueChannelNum	= 1;
	}
	m_vectWaitChannel.resize(m_cfginfo.nQueueChannelNum);
	for (size_t i=0; i<m_vectWaitChannel.size(); i++)
	{
		m_vectWaitChannel[i].Init();
	}

	// 进入工作状态
	SetStatus(STATUS_WORKING);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1369,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,OK,Work begin");

	return 0;
}

int		CAAFS4Web_i::Init_CFG(const char* cszCFG)
{
	WHDATAINI_CMN	ini;
	ini.addobj("CAAFS4Web", &m_cfginfo);
	ini.addobj("MSGER_LP", &m_MsgerLogicProxy_info);
	ini.addobj("EPOLL_SERVER", &m_epollServerInfo);

	// 默认参数
	m_MsgerLogicProxy_info.nSendBufSize			= 1*1024*1024;
	m_MsgerLogicProxy_info.nRecvBufSize			= 1*1024*1024;
	m_MsgerLogicProxy_info.nSockRcvBufSize		= 512*1024;
	m_MsgerLogicProxy_info.nSockSndBufSize		= 512*1024;
	
	int	iRst		= ini.analyzefile(cszCFG);
	if (iRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1367,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init_CFG,%d,analyzefile,%s", iRst, ini.printerrreport());
		return -1;
	}

	// 根据变量值计算其他一些相关变量
	m_cfginfo._nSelfInfoToClientLen				= strlen(m_cfginfo.szSelfInfoToClient);
	// 最多连接数不能超过逻辑上的最多人数
	m_epollServerInfo.iMaxConnections			= m_cfginfo.nClientConnectingNumMax+m_cfginfo.nClientWaitingNumMax;
	// 绑定的地址
	wh_strsplit("sd", m_cfginfo.szWelcomeAddr, ":", m_epollServerInfo.szIP, &m_epollServerInfo.iPort);
	
	return 0;
}

int		CAAFS4Web_i::Init_CFG_Reload(const char *cszCFGName)
{
	WHDATAINI_CMN	ini;
	ini.addobj("CAAFS4Web", &m_cfginfo);
	ini.addobj("MSGER_LP", &m_MsgerLogicProxy_info);
	ini.addobj("EPOLL_SERVER", &m_epollServerInfo);
	
	int	iRst		= ini.analyzefile(cszCFGName, false, 1);
	if (iRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1367,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init_CFG_Reload,%d,analyzefile,%s", iRst, ini.printerrreport());
		return -1;
	}

	// 根据变量值计算其他的一些相关变量
	m_cfginfo._nSelfInfoToClientLen			= strlen(m_cfginfo.szSelfInfoToClient);

	// 重新设置进入速率
	m_generWaitTransToConnect.SetGenSpeed(m_cfginfo.fWaitTransToConnectRate);

	return 0;
}

int		CAAFS4Web_i::Release()
{
	// 各个单元清除
	whunitallocatorFixed_AllClear(m_Players);
	m_Players.Release();
	m_TQ.Release();
	WHSafeSelfDestroy(m_pEpollServer);
	m_msgerLogicProxy.Release();
	m_MsgerLogicProxy_info.sock	= INVALID_SOCKET;	

	// 排队通道销毁
	for (size_t i=0; i<m_vectWaitChannel.size(); i++)
	{
		m_vectWaitChannel[i].Release();
	}

	m_nStatus		= STATUS_NOTHING;

	return 0;
}

int		CAAFS4Web_i::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	SOCKET	sock		= m_msgerLogicProxy.GetSocket();
	if (cmn_is_validsocket(sock))
	{
		vect.push_back(sock);
	}
	m_pEpollServer->GetSockets(vect);
	return vect.size();
}

int		CAAFS4Web_i::SureSend()
{
	// 向LP发送指令,因为经过Tick的处理中可能会触发向LP发送的指令
	m_msgerLogicProxy.ManualSend();
	return 0;
}

void	CAAFS4Web_i::SetStatus(int nStatus)
{
	m_nStatus		= nStatus;
}

int		CAAFS4Web_i::GetStatus() const
{
	return m_nStatus;
}

int		CAAFS4Web_i::RemovePlayerUnit(int nPlayerID)
{
	return RemovePlayerUnit(m_Players.getptr(nPlayerID));
}

int		CAAFS4Web_i::RemovePlayerUnit(PlayerUnit* pPlayer)
{
	if (pPlayer == NULL)
	{
		return -1;
	}

	if (pPlayer->nStatus == PlayerUnit::STATUS_CONNECTINGCLS4Web)
	{
		m_nConnectingNum--;
		if (m_nConnectingNum < 0)
		{
			m_nConnectingNum	= 0;
		}
	}

	if (m_pEpollServer->IsCntrExist(pPlayer->nCntrID))
	{
		m_pEpollServer->SetExt(pPlayer->nCntrID, 0);
		// 关闭cntr
		m_pEpollServer->Close(pPlayer->nCntrID);
	}

	pPlayer->clear();
	m_Players.FreeUnitByPtr(pPlayer);

	return 0;
}

CAAFS4Web_i::PlayerUnit*	CAAFS4Web_i::GetPlayerByCntrID(int iCntrID)
{
	int		iExt;
	if (m_pEpollServer->GetExt(iCntrID, &iExt) < 0)
	{
		return NULL;
	}
	return m_Players.getptr(iExt);
}

unsigned int	CAAFS4Web_i::GenPasswordToConnectCLS4Web()
{
	return wh_time()^rand();
}

void	CAAFS4Web_i::Retry_Worker_TRYCONNECT_Begin()
{
	m_bIsReconnect				= true;
	m_nLogicProxyAvailPlayerNum	= 0;
}

void	CAAFS4Web_i::Retry_Worker_WORKING_Begin()
{
	CAAFS4Web_LP_HI_T	Cmd;
	Cmd.nCmd			= CAAFS4Web_LP_HI;
	Cmd.nGroupID		= m_cfginfo.nGroupID;
	Cmd.nCAAFS4WebVer	= CAAFS4Web_VER;
	Cmd.bReconnect		= m_bIsReconnect;
	m_msgerLogicProxy.SendMsg(&Cmd, sizeof(Cmd));
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1368,CAAFS4Web_INT_RLS)"CAAFS4Web_i::Init,Send HI to LP,Wait for reply,%d,%d", m_cfginfo.nGroupID, m_bIsReconnect);
}

void	CAAFS4Web_i::SendPlayerQueueInfo(PlayerUnit* pPlayer, bool bMustSend)
{
	WAITCHANNEL_T*		pC	= m_vectWaitChannel.getptr(pPlayer->nChannel);
	if (bMustSend || pC->nWaitingSeq0!=pPlayer->nSeq0)
	{
		// 需要重新发送
		CAAFS4Web_CLIENT_QUEUEINFO_T	queueInfo;
		queueInfo.nCmd			= CAAFS4Web_CLIENT_QUEUEINFO;
		queueInfo.nChannel		= pPlayer->nChannel;
		pPlayer->nSeq0			= pC->nWaitingSeq0;
		queueInfo.nQueueSize	= pPlayer->nSeq - pPlayer->nSeq0;
		m_pEpollServer->Send(pPlayer->nCntrID, &queueInfo, sizeof(queueInfo));
	}
}

bool	CAAFS4Web_i::PutPlayerInChannel(CAAFS4Web_i::PlayerUnit* pPlayer, unsigned char nChannel)
{
	if (pPlayer->nodeWC.isinlist() && pPlayer->nChannel==nChannel)
	{
		// 已经在这个channel中了
		return false;
	}

	// 为了保险,原来的先退一下
	pPlayer->nodeWC.quit();

	if (nChannel >= m_vectWaitChannel.size())
	{
		nChannel	= m_vectWaitChannel.size()-1;
	}
	WAITCHANNEL_T*	pChannel	= m_vectWaitChannel.getptr(nChannel);
	pPlayer->nSeq0	= pChannel->nWaitingSeq0;
	pPlayer->nSeq	= pChannel->nWaitingSeqNew++;
	pPlayer->nChannel	= nChannel;
	pChannel->pdlWait->AddToTail(&pPlayer->nodeWC);

	return true;
}
