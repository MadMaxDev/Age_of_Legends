// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CAAFSģ����ڲ�ʵ��
//                PNGS��Pixel Network Game Structure����д
//                CAAFS��Connection Allocation and Address Filter Server����д�������ӷ��书���е����ӷ���͵�ַ���˷�����
// CreationDate : 2005-07-26
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

CAAFS2 *	CAAFS2::Create()
{
	return	new CAAFS2_I;
}

////////////////////////////////////////////////////////////////////
// CAAFS2_I
////////////////////////////////////////////////////////////////////
// ����CFGINFO_T
WHDATAPROP_MAP_BEGIN_AT_ROOT(CAAFS2_I::CFGINFO_T)
WHDATAPROP_ON_SETVALUE_smp_reload1(int, nVerCmpMode, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szVer, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szWelcomeAddr, 0)
WHDATAPROP_ON_SETVALUE_smp(charptr, szGMSAddr, 0)
WHDATAPROP_ON_SETVALUE_smp(int, nConnectGMSTimeOut, 0)
WHDATAPROP_ON_SETVALUE_smp(int, nGroupID, 0)
WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(int, nMaxPlayerRecvCmdNum, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(int, nClientWaitingNumMax, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(int, nClientConnectingNumMax, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(int, nHelloCheckTimeOut, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCloseCheckTimeOut, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(int, nQueueSeqSendInterval, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(float, fWaitTransToConnectRate, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bAtLeastOneTransToConnectPerTick, 0)
WHDATAPROP_ON_SETVALUE_smp(int, nQueueChannelNum, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szSelfInfoToClient, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSelfNotify, 0)
WHDATAPROP_ON_SETVALUE_smp_reload1(int,nQueuePosKeepTime,0)
WHDATAPROP_MAP_END()

// ������
CAAFS2_I::CAAFS2_I()
: m_nStatus(STATUS_NOTHING)
, m_tickNow(0)
, m_nSelfNotify(0)
, m_pICryptFactory(NULL)
, m_pSlotMan4Client(NULL)
, m_nConnectingNum(0)
, m_nGMSAvailPlayerNum(0)
, m_bShouldStop(false)
, m_bIsReconnect(false)
{
	m_vectrawbuf.reserve(32*1024);
}
CAAFS2_I::~CAAFS2_I()
{
}
void	CAAFS2_I::SelfDestroy()
{
	delete	this;
}
int		CAAFS2_I::Init(const char *cszCFG)
{
	m_tickNow	= wh_gettickcount();

	int	rst;
	// ����������Ϣ
	if( (rst=Init_CFG(cszCFG))<0 )
	{
		return	-1;
	}

	m_generWaitTransToConnect.SetGenSpeed(m_cfginfo.fWaitTransToConnectRate);
	m_itv.reset();

	// �����ȴ��û����ӵ�CNL2
	m_pSlotMan4Client		= CNL2SlotMan::Create();
	if( !m_pSlotMan4Client )
	{
		assert(0);
		return	-11;
	}
	m_pSlotMan4Client->SetUseAuth();
	if( (rst=m_pSlotMan4Client->Init(m_CLIENT_SLOTMAN_INFO.GetBase(), NULL))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(544,PNGS_INT_RLS)"CAAFS2_I::Init,%d,m_pSlotMan4Client->Init", rst);
		return	-12;
	}
	m_pSlotMan4Client->Listen(true);
	m_pSlotMan4Client->SetICryptFactory(m_pICryptFactory);

	// ���ӡ�����GMS�������Ƿ�ɹ�
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(545,PNGS_INT_RLS)"CAAFS connecting GMS,%s", m_cfginfo.szGMSAddr);
	// ���Լ�����һ�£����Ҫ����ǰ�棬����Init�����õ�����Ͳ����ˣ�
	m_msgerGMS.m_pHost	= this;
	// ��������GMS�Ķ���
	if( (rst=m_msgerGMS.Init(m_MSGERGMS_INFO.GetBase(), m_cfginfo.szGMSAddr))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(547,PNGS_INT_RLS)"CAAFS2_I::Init,%d,m_msgerGMS.Init", rst);
		return	-22;
	}

	// ѭ���ȴ�����
	bool		bStop	= false;
	size_t		nSize;
	GMS_CAAFS_HI_T	*pHI;
	while( !bStop )
	{
		m_bIsReconnect	= false;	// ��֤�������״̬���ж�������reconnect
		wh_sleep(10);
		if( m_msgerGMS.GetStatus() == MYMSGER_T::STATUS_TRYCONNECT )
		{
			if( wh_tickcount_diff(wh_gettickcount(), m_msgerGMS.m_tickWorkBegin) >= m_cfginfo.nConnectGMSTimeOut )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(546,PNGS_INT_RLS)"CAAFS2_I::Init,,connect to GMS fail,%s", m_cfginfo.szGMSAddr);
				return	-30;
			}
			m_msgerGMS.Tick();
			continue;
		}
		if( wh_tickcount_diff(wh_gettickcount(), m_msgerGMS.m_tickWorkBegin) >= m_cfginfo.nConnectGMSTimeOut )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(549,PNGS_INT_RLS)"CAAFS2_I::Init,,GMS not responed to HELLO");
			return	-31;
		}
		m_msgerGMS.Tick();
		// �����Ƿ��յ�����
		pHI	= (GMS_CAAFS_HI_T *)m_msgerGMS.PeekMsg(&nSize);
		if( pHI )
		{
			switch( pHI->nCmd )
			{
			case	GMS_CAAFS_HI:
				{
					// ֹͣѭ��
					bStop		= true;
				}
				break;
			}
			m_msgerGMS.FreeMsg();
		}
	}
	// �ܵ�����˵��һ�����յ���HI��Ϣ
	// �жϽ�����
	if( pHI->nRst != GMS_CAAFS_HI_T::RST_OK )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(551,PNGS_INT_RLS)"CAAFS2_I::Init,%d,GMS_CAAFS_HI bad rst", pHI->nRst);
		return	-33;
	}
	if( GMS_VER != pHI->nGMSVer )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(552,PNGS_INT_RLS)"CAAFS2_I::Init,%d,GMS_CAAFS_HI bad ver", pHI->nGMSVer);
		return	-34;
	}

	// ���û��ܳ��ܵ��������
	m_nGMSAvailPlayerNum	= 0;
	// ��ʼ���������
	if( (rst=m_Players.Init(m_cfginfo.nClientWaitingNumMax + m_cfginfo.nClientConnectingNumMax))<0 )
	{
		return	-41;
	}
	// ��ʼ��ʱ�����
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		return	-42;
	}

	// �����Ŷӵ�vipͨ��
	if( m_cfginfo.nQueueChannelNum<=0 )
	{
		// ���ٵ���һ��ͨ��
		m_cfginfo.nQueueChannelNum	= 1;
	}
	m_vectWaitChannel.resize(m_cfginfo.nQueueChannelNum);
	for(size_t i=0;i<m_vectWaitChannel.size();i++)
	{
		m_vectWaitChannel[i].Init();
	}

	// �����ִ�ָ���ӳ��(������Բ����)
	InitStrCmdMap();

	// Ȼ��ͽ��빤��״̬
	SetStatus(STATUS_WORKING);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(553,PNGS_INT_RLS)"CAAFS2_I::Init,OK,Work begin");

	// һЩ������ʼ��
	m_nConnectingNum	= 0;

	return	0;
}
int		CAAFS2_I::Init_CFG(const char *cszCFG)
{
	WHDATAINI_CMN	ini;
	ini.addobj("CAAFS", &m_cfginfo);
	ini.addobj("MSGERGMS", &m_MSGERGMS_INFO);
	ini.addobj("CLIENT_SLOTMAN", &m_CLIENT_SLOTMAN_INFO);

	// Ĭ�ϲ���
	m_MSGERGMS_INFO.nSendBufSize		= 1*1024*1024;
	m_MSGERGMS_INFO.nRecvBufSize		= 1*1024*1024;
	m_MSGERGMS_INFO.nSockSndBufSize		= 512*1024;
	m_MSGERGMS_INFO.nSockRcvBufSize		= 512*1024;
	m_CLIENT_SLOTMAN_INFO.nAllocChunkSize		= 1024*1024;
	m_CLIENT_SLOTMAN_INFO.nControlOutQueueSize	= 10*1024*1024;

	int	rst = ini.analyzefile(cszCFG);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(554,PNGS_INT_RLS)"CAAFS2_I::Init_CFG,%d,analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// ���ݱ���ֵ��������һЩ��ر���
	m_cfginfo._nSelfInfoToClientLen					= strlen(m_cfginfo.szSelfInfoToClient) + 1;
	// ������������ܳ����߼��ϵ��������
	m_CLIENT_SLOTMAN_INFO.nMaxConnection			= m_cfginfo.nClientWaitingNumMax + m_cfginfo.nClientConnectingNumMax;
	// slot�Ĺ̶�����
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nType		= CNL2_CHANNELTYPE_SAFE;
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nInNum		= CAAFS_CHANNEL_INOUT_NUM;
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nOutNum	= CAAFS_CHANNEL_INOUT_NUM;
	// ����ͨ��ֻ�ܷ������ݣ������ܽ������ݣ�2008-12-24��Ϊ���Խ��գ���Ϊ��ҪԤ��У���û���ݣ�
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nProp		= CNL2CHANNELINFO_T::PROP_CANSEND | CNL2CHANNELINFO_T::PROP_CANRECV;

	// �󶨵ĵ�ַ
	strcpy(m_CLIENT_SLOTMAN_INFO.szBindAddr, m_cfginfo.szWelcomeAddr);

	return	0;
}

int		CAAFS2_I::Init_CFG_Reload(const char *cszCFG)
{
	WHDATAINI_CMN	ini;
	ini.addobj("CAAFS", &m_cfginfo);
	ini.addobj("MSGERGMS", &m_MSGERGMS_INFO);
	ini.addobj("CLIENT_SLOTMAN", &m_CLIENT_SLOTMAN_INFO);

	int	rst = ini.analyzefile(cszCFG, false, 1);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(554,PNGS_INT_RLS)"CAAFS2_I::Init_CFG_Reload,%d,analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// ���ݱ���ֵ��������һЩ��ر���
	m_cfginfo._nSelfInfoToClientLen					= strlen(m_cfginfo.szSelfInfoToClient) + 1;

	// �������ý�������
	m_generWaitTransToConnect.SetGenSpeed(m_cfginfo.fWaitTransToConnectRate);

	m_pSlotMan4Client->Init_Reload(m_CLIENT_SLOTMAN_INFO.GetBase());
	return	0;
}
int		CAAFS2_I::Release()
{
	// ������Ԫ���(Ҫ�ں��������ǰд����������õ�����Ĺ������Ͳ���)
	whunitallocatorFixed_AllClear(m_Players);

	// ���������ͷ�
	m_Players.Release();

	// ʱ�����
	m_TQ.Release();

	// CNL2�ս�
	WHSafeSelfDestroy( m_pSlotMan4Client );
	// TCP�ս�
	m_msgerGMS.Release();
	cmn_safeclosesocket(m_MSGERGMS_INFO.sock);

	// �Ŷ�ͨ������
	for(size_t i=0;i<m_vectWaitChannel.size();i++)
	{
		m_vectWaitChannel[i].Release();
	}

	m_nStatus	= STATUS_NOTHING;

	return	0;
}
int		CAAFS2_I::SetICryptFactory(ICryptFactory *pFactory)
{
	m_pICryptFactory	= pFactory;
	if( m_pSlotMan4Client )
	{
		m_pSlotMan4Client->SetICryptFactory(m_pICryptFactory);
	}
	return	0;
}

void	CAAFS2_I::SetStatus(int nStatus)
{
	m_nStatus	= nStatus;
}
int		CAAFS2_I::GetStatus() const
{
	return	m_nStatus;
}

int		CAAFS2_I::RemovePlayerUnit(int nPlayerID)
{
	return	RemovePlayerUnit(m_Players.getptr(nPlayerID));
}
int		CAAFS2_I::RemovePlayerUnit(PlayerUnit *pPlayer)
{
	// CAAFSɾ����ҵ�ʱ��Ӧ�ò���֪ͨGMS(��Ϊ���ڵȴ��е����GMS��֪�������������еõ���ң�GMS����Ҫ���)

	if( pPlayer == NULL )
	{
		return	-1;
	}

	// ��������Ӷ���������������--
	// clear�л����
	if( pPlayer->nStatus == PlayerUnit::STATUS_CONNECTINGCLS )
	{
		// �¸�Tick��ʱ��������������û�дﵽ���޾Ϳ��Լ���������������
		m_nConnectingNum	--;
		if( m_nConnectingNum<0 )
		{
			m_nConnectingNum	= 0;
			GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(0,TEMP)"CAAFS2_I::RemovePlayerUnit m_nConnectingNum is negative!!!");
		}
	}

	if( m_pSlotMan4Client->IsSlotExist(pPlayer->nSlot) )
	{
		// ȡ������(�����slot�رյ�ʱ����ɾ����ҳ���)
		m_pSlotMan4Client->SetExtLong(pPlayer->nSlot, 0);
		// �ر�Slot
		m_pSlotMan4Client->Close(pPlayer->nSlot);
	}


	//add by ����� 2012/3/20
	//�ڴ�֮ǰ������resumemap
	map<int,string>::iterator  client_iter = m_clientID2Account_map.find(pPlayer->nID);
	if(client_iter!=m_clientID2Account_map.end())
	{
		map<string,ResumedPlayerUnit>::iterator iter = m_wait2resume_map.find(client_iter->second);
		if(iter!=m_wait2resume_map.end())
		{

			iter->second.nLeaveTime = wh_gettickcount();
			iter->second.nSeq = pPlayer->nSeq;
			iter->second.nSeq0 = pPlayer->nSeq0;
			iter->second.nVIPChannel = pPlayer->nVIPChannel;
#ifdef UNITTEST
			printf("�û��뿪�������ֳ�: %s,seq0 : %d,seq : %d\n",client_iter->second.c_str(),iter->second.nSeq0,iter->second.nSeq);
#endif
		}
		m_clientID2Account_map.erase(client_iter);
	}
	
	
	// ��������(�������ǰ�����������������֮��������������������˾Ͳ�֪��ԭ����û��������ϵ��)
	pPlayer->clear();
	// �Ƴ�
	m_Players.FreeUnitByPtr(pPlayer);

	return	0;
}
CAAFS2_I::PlayerUnit *	CAAFS2_I::GetPlayerBySlot(int nSlot)
{
	long	nExt;
	if( m_pSlotMan4Client->GetExtLong(nSlot, &nExt)<0 )
	{
		return	NULL;
	}
	return	m_Players.getptr((int)nExt);
}
unsigned int	CAAFS2_I::GenPasswordToConnectCLS()
{
	// ϵͳӦ����ͳ�ʼ�������������
	return	wh_time() ^ rand();
}
size_t	CAAFS2_I::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	// ����GMS��
	SOCKET	sock	= m_msgerGMS.GetSocket();
	if( cmn_is_validsocket(sock) )
	{
		vect.push_back(sock);
	}
	//
	m_pSlotMan4Client->GetSockets(vect);
	return	vect.size();
}
bool	CAAFS2_I::ShouldStop() const
{
	return	m_bShouldStop;
}
void	CAAFS2_I::Retry_Worker_TRYCONNECT_Begin()
{
	// ����Ϊ����״̬
	m_bIsReconnect		= true;
	// ��GMS����������Ϊ0������֮��GMS��Ȼ�ᷢ�ͽ��������
	m_nGMSAvailPlayerNum	= 0;
}
void	CAAFS2_I::Retry_Worker_WORKING_Begin()
{
	// �ɹ������Լ�����Ϣ(��ʾ�Լ���CAAFS����Ҫ�����Ӧ����Ϣ)
	CAAFS_GMS_HELLO_T	Cmd;
	Cmd.nCmd			= CAAFS_GMS_HELLO;
	Cmd.nGroupID		= m_cfginfo.nGroupID;
	Cmd.nCAAFSVer		= CAAFS_VER;
	Cmd.bReConnect		= m_bIsReconnect;
	m_msgerGMS.SendMsg(&Cmd, sizeof(Cmd));
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(548,PNGS_INT_RLS)"CAAFS2_I::Init,Send HELLO to GMS,Wait for reply,%d,%d", m_cfginfo.nGroupID, m_bIsReconnect);
}
bool	CAAFS2_I::PutPlayerInVIPChannel(PlayerUnit *pPlayer, unsigned char nVIPChannel)
{
	if( pPlayer->nodeWC.isinlist() && pPlayer->nVIPChannel==nVIPChannel )
	{
		// �Ѿ������channel�˾Ͳ��ü�������
		return	false;
	}

	// Ϊ�˱���ԭ��������Ǹ���������˳�һ��
	pPlayer->nodeWC.quit();

	if( nVIPChannel>=m_vectWaitChannel.size() )
	{
		nVIPChannel	= m_vectWaitChannel.size()-1;
	}
	WAITCHANNEL_T			*pC	= m_vectWaitChannel.getptr(nVIPChannel);

	pPlayer->nSeq0				= pC->nWaitingSeq0;
	pPlayer->nSeq				= pC->nWaitingSeqNew++;
	pPlayer->nVIPChannel		= nVIPChannel;
	pC->pdlWait->AddToTail(&pPlayer->nodeWC);

	return	true;
}
bool	CAAFS2_I::ResumePlayerPosInVIPChannel(PlayerUnit *pPlayer, unsigned char nVIPChannel)
{

	// �Ȱ�ԭ���ĸ�����
	pPlayer->nodeWC.quit();

	if( nVIPChannel>=m_vectWaitChannel.size() )
	{
		nVIPChannel	= m_vectWaitChannel.size()-1;
	}
	WAITCHANNEL_T			*pC	= m_vectWaitChannel.getptr(nVIPChannel);
	whDList<PlayerUnit *>::node	*pNode	= pC->pdlWait->begin();
	while( pNode != pC->pdlWait->end() )
	{

		if(pNode->data->nSeq > pPlayer->nSeq)
		{
#ifdef UNITTEST
			printf("Resume Player POS in channel : %d , Seq0ID : %u, SeqID : %u, PreSeqID: %u\n",nVIPChannel,pPlayer->nSeq0,pPlayer->nSeq,pNode->data->nSeq);
#endif
			pPlayer->nodeWC.insertbefore(pNode);
			break;
		}
		pNode = pNode->next;
	}
	//�Ͻ�����ҷ���һ�µ�ǰ״̬
	SendPlayerQueueInfo(pPlayer, true);
	return	true;
}
void	CAAFS2_I::SendPlayerQueueInfo(PlayerUnit *pPlayer, bool bMustSend)
{
	WAITCHANNEL_T		*pC	= m_vectWaitChannel.getptr(pPlayer->nVIPChannel);
	if( bMustSend || pC->nWaitingSeq0 != pPlayer->nSeq0 )
	{
		// ��Ҫ���·���
		CAAFS_CLIENT_QUEUEINFO_T	QueueInfo;
		QueueInfo.nCmd				= CAAFS_CLIENT_QUEUEINFO;
		QueueInfo.nVIPChannel		= pPlayer->nVIPChannel;
		pPlayer->nSeq0				= pC->nWaitingSeq0;
		if(pPlayer->nSeq < pPlayer->nSeq0)
			QueueInfo.nQueueSize = 7; //���������ƭ�ͻ��˵ļ����֣���Ϊ�ǲ�ӽ����ģ���֪��ǰ���м�����
		else
			QueueInfo.nQueueSize		= pPlayer->nSeq - pPlayer->nSeq0;
		m_pSlotMan4Client->Send(pPlayer->nSlot, &QueueInfo, sizeof(QueueInfo), 0);
	}
}

void CAAFS2_I::DisplayQueueInfo()
{
	printf("-----��ʼ��ӡ������Ϣ-----\n\n");
	for(int i=m_vectWaitChannel.size()-1;i>=0;i--)
	{
		WAITCHANNEL_T	*pC	= m_vectWaitChannel.getptr(i);
		if( pC->pdlWait->size()>0 )
		{
			printf("---channel : %d Seq0 : %d , NewSeq : %d ---\n",i,pC->nWaitingSeq0,pC->nWaitingSeqNew);
			whDList<PlayerUnit *>::node	*pNode	= pC->pdlWait->begin();
			while( pNode != pC->pdlWait->end() )
			{
				struct in_addr a;  
				a.s_addr = pNode->data->IP;  
				string acct_name;
				map<int,string>::const_iterator iter = m_clientID2Account_map.find(pNode->data->nID);
				if (iter != m_clientID2Account_map.end())
				{
					acct_name = m_clientID2Account_map[pNode->data->nID];
				}
				printf("-- ID: %d\tSeq: %u\tIP:%s\t�ʺ�:%s\n ",pNode->data->nID,pNode->data->nSeq,inet_ntoa(a),acct_name.c_str());
				pNode		= pNode->next;
			}
			printf("-----------------\n\n");
		}
	}
}

void	CAAFS2_I::DealKeepQueuePosReq(PlayerUnit *pPlayer,const char * szAccount)
{
	
		//�ж��ǲ�����Ҫresume
		map<string,ResumedPlayerUnit>::iterator iter = m_wait2resume_map.find(szAccount);
		if(iter!=m_wait2resume_map.end())
		{
			//�����ˣ����Ŵ�
#ifdef UNITTEST
			printf("���û���,�Ŵ� : %s\n",szAccount);
#endif
			//���µİ���
			m_clientID2Account_map.insert(make_pair(pPlayer->nID,szAccount));
			//���ϴ����ݾͲ����ͷ
			if(iter->second.nSeq0 == 0)
			{
#ifdef UNITTEST
				printf("BEFORE SEQ = 0 ,erase!\n");
#endif
				m_wait2resume_map.erase(iter);
			}else{

				//˵����һλ�����أ�������һ����
				if(iter->second.nLeaveTime==0)
				{
#ifdef UNITTEST
					assert(0);
#endif
					//Ӧ�þܾ������������
					return;
				}

				WAITCHANNEL_T			*pC	= m_vectWaitChannel.getptr(iter->second.nVIPChannel);
				//�Ѿ��й����ˣ�ֱ�ӽ�ȥ
				if( pC->nWaitingSeq0 >=iter->second.nSeq0)
				{
					pPlayer->nSeq = iter->second.nSeq;
					pPlayer->nSeq0 = iter->second.nSeq0;

				}else{
					//��û���أ����ǿ��Բ��
					pPlayer->nSeq0 = pC->nWaitingSeq0;
					pPlayer->nSeq = iter->second.nSeq;
				}
				ResumePlayerPosInVIPChannel(pPlayer,iter->second.nVIPChannel);
				return;
			}
		}
		//�����ģ��ӽ�ȥ
#ifdef UNITTEST
		printf("�����İ�,���� : %s\n",szAccount);
#endif
		ResumedPlayerUnit resumed_player_t;
		m_wait2resume_map.insert(make_pair(szAccount,resumed_player_t));
		m_clientID2Account_map.insert(make_pair(pPlayer->nID,szAccount));
		return ;
}
