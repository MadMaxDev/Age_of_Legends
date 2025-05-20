// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_TCPReceiver.cpp
// Creator      : Wei Hua (κ��)
// Comment      : TCPָ�����ģ��
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2007-02-25
// Change LOG   : 

#include "../inc/pngs_TCPReceiver.h"
#include "../inc/pngs_TCPReceiver_def.h"
#include "../inc/pngs_packet_logic.h"
#include <WHNET/inc/whnetudpGLogger.h>	// ��������Ϊʲôdebug�治��Ҫ�����ͷ�ļ��أ��Ժ���!!!!

using namespace n_pngs;

////////////////////////////////////////////////////////////////////
// PNGS_TCPRECEIVER
////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(PNGS_TCPRECEIVER::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueCmdInSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nMaxCmdPerSec, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(unknowntype, CNTRSVR, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(unknowntype, MSGER, 0)
WHDATAPROP_MAP_END()

PNGS_TCPRECEIVER::PNGS_TCPRECEIVER()
: m_pLogicDftCmdDealer(NULL)
, m_nCmdNumThisTick(0)
{
	strcpy(CMN::ILogic::m_szLogicType, PNGS_DLL_NAME_TCPRECEIVER);
	CMN::ILogic::m_nVer	= TCPRECEIVER_VER;
}
PNGS_TCPRECEIVER::~PNGS_TCPRECEIVER()
{
}
int		PNGS_TCPRECEIVER::Organize()
{
	return	0;
}
int		PNGS_TCPRECEIVER::Detach(bool bQuick)
{
	// û�����ʲô��Ҳ��Ӧ�ð����е����Ӷ��ϵ���
	ConnecterMan::RemoveAllConnecter();
	return	0;
}
int		PNGS_TCPRECEIVER::Init(const char *cszCFGName)
{
	if( Init_CFG(cszCFGName)<0 )
	{
		return	-1;
	}

	if( m_cfginfo.CNTRSVR.nPort == 0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(612,PNGS_INT_RLS)"PNGS_TCPRECEIVER::Init,,m_cfginfo.CNTRSVR.nPort is 0");
		return	-2;
	}

	int	rst;

	// ��ʼ��TCPSERVER
	if( (rst=TCPConnecterServer::Init(m_cfginfo.CNTRSVR.GetBase()))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(613,PNGS_INT_RLS)"PNGS_TCPRECEIVER::Init,%d,TCPConnecterServer::Init", rst);
		return	-10;
	}

	// ����ָ�����
	ILogicBase::QueueInit(m_cfginfo.nQueueCmdInSize);

	m_itv.reset();
	m_nCmdNumThisTick	= 0;

	return	0;
}
int		PNGS_TCPRECEIVER::Init_CFG(const char *cszCFGName)
{
	// �����ļ�
	WHDATAINI_CMN	ini;
	ini.addobj("TCPRECEIVER", &m_cfginfo);
	// һЩĬ��ֵ����Ϊ�Ǹ��������õģ����Ի��嶼��Ҫ���õĴ�Щ��
	m_cfginfo.MSGER.nSendBufSize	= 10*1024*1024;
	m_cfginfo.MSGER.nRecvBufSize	= 10*1024*1024;
	m_cfginfo.MSGER.nSockSndBufSize	= 1*1024*1024;
	m_cfginfo.MSGER.nSockRcvBufSize	= 1*1024*1024;

	int	rst = ini.analyzefile(cszCFGName);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(614,PNGS_INT_RLS)"PNGS_TCPRECEIVER::Init_CFG,%d,ini.analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// �������������б�Ҫ�Ļ���
	m_generDealGMSCmdNum.SetGenSpeed((float)m_cfginfo.nMaxCmdPerSec);

	return	0;
}
int		PNGS_TCPRECEIVER::Init_CFG_Reload(const char *cszCFGName)
{
	// �����ļ�
	WHDATAINI_CMN	ini;
	ini.addobj("TCPRECEIVER", &m_cfginfo);

	int	rst = ini.analyzefile(cszCFGName, false, 1);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(614,PNGS_INT_RLS)"PNGS_TCPRECEIVER::Init_CFG_Reload,%d,ini.analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// �������������б�Ҫ�Ļ���
	m_generDealGMSCmdNum.SetGenSpeed((float)m_cfginfo.nMaxCmdPerSec);

	return	0;
}
int		PNGS_TCPRECEIVER::Release()
{
	// �������ӳ������
	for(MAPCMD2LOGIC_IT_T it=m_mapCmdReg2Logic.begin(); it!=m_mapCmdReg2Logic.end(); ++it)
	{
		delete	it.getvalue();
	}
	m_mapCmdReg2Logic.clear();
	return	0;
}
int		PNGS_TCPRECEIVER::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	return	ConnecterMan::GetSockets(vect);
}
int		PNGS_TCPRECEIVER::SureSend()
{
	return	ConnecterMan::SureSend();
}
int		PNGS_TCPRECEIVER::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	switch( nCmd )
	{
	case	PNGS_CD2TR_CONFIG:
		{
			PNGS_CD2TR_CONFIG_T	*pCfg	= (PNGS_CD2TR_CONFIG_T *)pData;
			switch( pCfg->nSubCmd )
			{
			case	PNGS_CD2TR_CONFIG_T::SUBCMD_SET_CMDDEALER:
				{
					m_pLogicDftCmdDealer	= pRstAccepter;
				}
				break;
			case	PNGS_CD2TR_CONFIG_T::SUBCMD_REGISTERCMDDEAL:
				{
					whlist<ILogicBase *>	*pl;
					if( !m_mapCmdReg2Logic.get(pCfg->nParam, pl) )
					{
						// û�У��򴴽�һ����������
						pl	= new whlist<ILogicBase *>;
						m_mapCmdReg2Logic.put(pCfg->nParam, pl);
					}
					pl->push_back(pRstAccepter);
				}
			default:
				break;
			}
		}
		break;
	case PNGS_CD2TR_FLUSH:
		{
			DealCmdIn();
		}
		break;
	default:
		break;
	}
	return	0;
}
int		PNGS_TCPRECEIVER::DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	switch( nCmd )
	{
	case	PNGSPACKET_2LOGIC_SETMOOD:
		{
			// Ӧ�ý�����
			m_nMood	= CMN::ILogic::MOOD_STOPPED;
		}
		break;
	case	PNGS_CD2TR_SETEXT:
		{
			PNGS_CD2TR_SETEXT_T	*pSet	= (PNGS_CD2TR_SETEXT_T *)pData;
			CONNECTOR_INFO_T	*pInfo	= m_Connecters.getptr(pSet->nConnecterID);
			if( pInfo )
			{
				pInfo->nExt				= pSet->nExt;
			}
		}
		break;
	case	PNGS_CD2TR_DISCONNECT:
		{
			int	&nConnecterID	= *(int *)pData;
			RemoveConnecter(nConnecterID);
		}
		break;
	case	PNGS_CD2TR_CMD:
		{
			PNGS_CD2TR_CMD_T	*pRst	= (PNGS_CD2TR_CMD_T *)pData;
			if( pRst->nConnecterID == -8888 )
			{
				for(n_whcmn::whunitallocatorFixed<CONNECTOR_INFO_T>::iterator it=m_Connecters.begin(); it!=m_Connecters.end(); ++it)
				{
					MYCNTR	*pMyCntr	= (MYCNTR *)((*it).pCntr->QueryInterface());
					if( pMyCntr )		// ��ΪacceptorҲ���б��У���������������Ϣ��
					{
						pMyCntr->SendMsg(pRst->pData, pRst->nDSize);
					}
				}
			}
			else
			{
				// ������Ӧ��Connecter
				Connecter	*pCntr			= TCPConnecterServer::GetConnecterPtr(pRst->nConnecterID);
				if( pCntr )
				{
					// ת������Ӧ��Connecter
					MYCNTR	*pMyCntr		= (MYCNTR *)pCntr->QueryInterface();
					pMyCntr->SendMsg(pRst->pData, pRst->nDSize);
				}
				else
				{
					// Ӧ�ðѷ��صĽ�����浽��־�ļ���ȥ
				}
			}
		}
		break;
	default:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(615,PNGS_RT)"PNGS_TCPRECEIVER::DealCmdIn_One,%d,unknown", nCmd);
			assert(0);
		}
		break;
	}
	return	0;
}
int		PNGS_TCPRECEIVER::Tick_BeforeDealCmdIn()
{
	m_itv.tick();
	m_nCmdNumThisTick	= m_generDealGMSCmdNum.MakeDecision(m_itv.getfloatInterval());
	// �����ϻ�����Ҫ��ʱ����У���ΪCmdDealer����Ӧ�ñ�֤���г�ʱ�Ĵ���
	return	0;
}
int		PNGS_TCPRECEIVER::Tick_AfterDealCmdIn()
{
	// ��������������TCP���ӵĹ�����false��ʾ��DoWork���Ȳ���suresend�����ǵ������õģ�
	TCPConnecterServer::DoWork(false);
	return	0;
}
Connecter *	PNGS_TCPRECEIVER::NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
{
	MYCNTR	*pCntr	= new MYCNTR(this);
	assert(pCntr);
	// ��ʼ��֮
	MYCNTR::INFO_T	info;
	memcpy(&info.msgerINFO, m_cfginfo.MSGER.GetBase(), sizeof(info.msgerINFO));
	info.msgerINFO.sock	= sock;		// ����ɲ�������!!!!
	int	rst	= pCntr->Init(&info);
	if( rst<0 )
	{
		assert(0);
		delete	pCntr;
		pCntr	= NULL;
	}
	else
	{
		memcpy(&pCntr->m_addr, paddr, sizeof(pCntr->m_addr));
	}
	return	pCntr;
}
void		PNGS_TCPRECEIVER::BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo)
{
	// Ӧ��֪ͨĬ�ϵ�cmddealer��ʧȥ���ӵ���
	PNGS_TR2CD_NOTIFY_T	Cmd;
	Cmd.nSubCmd			= PNGS_TR2CD_NOTIFY_T::SUBCMD_DISCONNECT;
	Cmd.nParam1			= nCntrID;
	Cmd.nParam2			= pCntrInfo->nExt;
	CmdOutToLogic_AUTO(m_pLogicDftCmdDealer, PNGS_TR2CD_NOTIFY, &Cmd, sizeof(Cmd));
	// ֪ͨ��������֪�������Ϣ��ģ��
	whlist<ILogicBase *>	*pl;
	if( m_mapCmdReg2Logic.get((pngs_cmd_t)PNGS_TR2CD_NOTIFY, pl) )
	{
		// ������Ӧ�Ĵ�����
		for(whlist<ILogicBase *>::iterator it=pl->begin();it!=pl->end();++it)
		{
			CmdOutToLogic_AUTO((*it), PNGS_TR2CD_NOTIFY, &Cmd, sizeof(Cmd));
		}
	}
}
void		PNGS_TCPRECEIVER::AfterAddConnecter(Connecter * pCntr)
{
	MYCNTR	*pMyCntr	= (MYCNTR *)pCntr->QueryInterface();
	if( pMyCntr )
	{
		// ��¼һ����־��
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(616,PNGS_RT)"PNGS_TCPRECEIVER new TCP connecter,0x%X,%s"
			, pMyCntr->GetIDInMan()
			, cmn_get_ipnportstr_by_saaddr(&pMyCntr->m_addr)
			);
		
		// �����ϲ��������ӵ���,added by yuezhongyue
		{
			// Ӧ��֪ͨĬ�ϵ�cmddealer��ʧȥ���ӵ���
			PNGS_TR2CD_NOTIFY_T	Cmd;
			Cmd.nSubCmd			= PNGS_TR2CD_NOTIFY_T::SUBCMD_CONNECT;
			Cmd.nParam1			= pCntr->GetIDInMan();
			Cmd.nParam2			= 0;
			CmdOutToLogic_AUTO(m_pLogicDftCmdDealer, PNGS_TR2CD_NOTIFY, &Cmd, sizeof(Cmd));
			// ֪ͨ��������֪�������Ϣ��ģ��
			whlist<ILogicBase *>	*pl;
			if( m_mapCmdReg2Logic.get((pngs_cmd_t)PNGS_TR2CD_NOTIFY, pl) )
			{
				// ������Ӧ�Ĵ�����
				for(whlist<ILogicBase *>::iterator it=pl->begin();it!=pl->end();++it)
				{
					CmdOutToLogic_AUTO((*it), PNGS_TR2CD_NOTIFY, &Cmd, sizeof(Cmd));
				}
			}
		}
	}
	else
	{
		// ���Ϊ��˵���ǵ�һ��listener
	}
}
bool	PNGS_TCPRECEIVER::CNTR_CanDealMsg() const
{
	if( m_nCmdNumThisTick<=0 )
	{
		return	false;
	}
	return	true;
}
int		PNGS_TCPRECEIVER::CNTR_DealMsg(MYCNTR *pCntr, const void *pCmd, size_t nSize)
{
	// û��Ĭ�ϵ�Ҳ����ν�ˣ����ü�¼
	//if( !m_pLogicDftCmdDealer )
	//{
	//	// ����ν�ˣ����ü�¼��־
	//	return	-1;
	//}
	//
	--m_nCmdNumThisTick;
	//
	PNGS_TR2CD_CMD_T	Cmd;
	Cmd.nConnecterID	= pCntr->GetIDInMan();
	Cmd.IP				= pCntr->m_addr.sin_addr.s_addr;
	Cmd.nDSize			= nSize;
	Cmd.pData			= (void *)pCmd;
	// �ȿ��Ƿ�ע����
	whlist<ILogicBase *>	*pl;
	if( m_mapCmdReg2Logic.get(*(pngs_cmd_t*)pCmd, pl) )
	{
		// ������Ӧ�Ĵ�����
		for(whlist<ILogicBase *>::iterator it=pl->begin();it!=pl->end();++it)
		{
			CMN_LOGIC_CMDIN_AUTO(this, (*it), PNGS_TR2CD_CMD, Cmd);
		}
	}
	else
	{
		// ��ָ���Ĭ�ϴ�����
		CMN_LOGIC_CMDIN_AUTO(this, m_pLogicDftCmdDealer, PNGS_TR2CD_CMD, Cmd);
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// PNGS_TCPRECEIVER_DUMMYCMDDEALER
////////////////////////////////////////////////////////////////////
int		PNGS_TCPRECEIVER_DUMMYCMDDEALER::Organize()
{
	// ����TCPReceiver�������Լ�Ϊ����ʹ����
	m_pLogicTCPReceiver	= m_pCMN->GetLogic(PNGS_DLL_NAME_TCPRECEIVER, TCPRECEIVER_VER);
	if( !m_pLogicTCPReceiver )
	{
		return	-1;
	}
	PNGS_CD2TR_CONFIG_T	Cfg;
	Cfg.nSubCmd			= PNGS_CD2TR_CONFIG_T::SUBCMD_SET_CMDDEALER;
	if( m_pLogicTCPReceiver->CmdIn_Instant(this, PNGS_CD2TR_CONFIG, &Cfg, sizeof(Cfg))<0 )
	{
		return	-2;
	}
	return	0;
}
PNGS_TCPRECEIVER_DUMMYCMDDEALER::PNGS_TCPRECEIVER_DUMMYCMDDEALER()
: m_pLogicTCPReceiver(NULL)
{

}
PNGS_TCPRECEIVER_DUMMYCMDDEALER::~PNGS_TCPRECEIVER_DUMMYCMDDEALER()
{

}
int		PNGS_TCPRECEIVER_DUMMYCMDDEALER::Init(const char *cszCFGName)
{
	return	0;
}
int		PNGS_TCPRECEIVER_DUMMYCMDDEALER::Init_CFG_Reload(const char *cszCFGName)
{
	return	0;
}
int		PNGS_TCPRECEIVER_DUMMYCMDDEALER::Release()
{
	return	0;
}
int		PNGS_TCPRECEIVER_DUMMYCMDDEALER::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	return	0;
}
int		PNGS_TCPRECEIVER_DUMMYCMDDEALER::DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	switch(nCmd)
	{
	case	PNGSPACKET_2LOGIC_SETMOOD:
		{
			// Ӧ�ý�����
			m_nMood	= CMN::ILogic::MOOD_STOPPED;
		}
		break;
	case	PNGS_TR2CD_CMD:
		{
			// ֱ�Ӱ�ָ��ԭ�����ؼ���
			PNGS_TR2CD_CMD_T	*pReq	= ( PNGS_TR2CD_CMD_T *)pData;
			PNGS_CD2TR_CMD_T	Rpl;
			Rpl.nConnecterID			= pReq->nConnecterID;
			Rpl.nDSize					= pReq->nDSize;
			Rpl.pData					= pReq->pData;
			CMN_LOGIC_CMDIN_AUTO(this, pRstAccepter, PNGS_CD2TR_CMD, Rpl);
		}
		break;
	default:
		break;
	}
	return	0;
}
int		PNGS_TCPRECEIVER_DUMMYCMDDEALER::Tick_BeforeDealCmdIn()
{
	return	0;
}
int		PNGS_TCPRECEIVER_DUMMYCMDDEALER::Tick_AfterDealCmdIn()
{
	return	0;
}
