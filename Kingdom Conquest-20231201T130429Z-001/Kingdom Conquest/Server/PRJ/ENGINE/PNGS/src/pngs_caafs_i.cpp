// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的CAAFS模块的内部实现
//                PNGS是Pixel Network Game Structure的缩写
//                CAAFS是Connection Allocation and Address Filter Server的缩写，是连接分配功能中的连接分配和地址过滤服务器
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
// 配置CFGINFO_T
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

// 类内容
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
	// 读入配置信息
	if( (rst=Init_CFG(cszCFG))<0 )
	{
		return	-1;
	}

	m_generWaitTransToConnect.SetGenSpeed(m_cfginfo.fWaitTransToConnectRate);
	m_itv.reset();

	// 创建等待用户连接的CNL2
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

	// 连接。检查和GMS的连接是否成功
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(545,PNGS_INT_RLS)"CAAFS connecting GMS,%s", m_cfginfo.szGMSAddr);
	// 和自己关联一下（这个要放在前面，否则Init里面用到这个就不对了）
	m_msgerGMS.m_pHost	= this;
	// 创建连接GMS的对象
	if( (rst=m_msgerGMS.Init(m_MSGERGMS_INFO.GetBase(), m_cfginfo.szGMSAddr))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(547,PNGS_INT_RLS)"CAAFS2_I::Init,%d,m_msgerGMS.Init", rst);
		return	-22;
	}

	// 循环等待返回
	bool		bStop	= false;
	size_t		nSize;
	GMS_CAAFS_HI_T	*pHI;
	while( !bStop )
	{
		m_bIsReconnect	= false;	// 保证最初连接状态都中都不会是reconnect
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
		// 看看是否收到返回
		pHI	= (GMS_CAAFS_HI_T *)m_msgerGMS.PeekMsg(&nSize);
		if( pHI )
		{
			switch( pHI->nCmd )
			{
			case	GMS_CAAFS_HI:
				{
					// 停止循环
					bStop		= true;
				}
				break;
			}
			m_msgerGMS.FreeMsg();
		}
	}
	// 能到这里说明一定是收到了HI消息
	// 判断结果如何
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

	// 设置还能承受的玩家数量
	m_nGMSAvailPlayerNum	= 0;
	// 初始化玩家数组
	if( (rst=m_Players.Init(m_cfginfo.nClientWaitingNumMax + m_cfginfo.nClientConnectingNumMax))<0 )
	{
		return	-41;
	}
	// 初始化时间队列
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		return	-42;
	}

	// 创建排队的vip通道
	if( m_cfginfo.nQueueChannelNum<=0 )
	{
		// 至少得有一个通道
		m_cfginfo.nQueueChannelNum	= 1;
	}
	m_vectWaitChannel.resize(m_cfginfo.nQueueChannelNum);
	for(size_t i=0;i<m_vectWaitChannel.size();i++)
	{
		m_vectWaitChannel[i].Init();
	}

	// 载入字串指令处理映射(这个绝对不会错)
	InitStrCmdMap();

	// 然后就进入工作状态
	SetStatus(STATUS_WORKING);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(553,PNGS_INT_RLS)"CAAFS2_I::Init,OK,Work begin");

	// 一些变量初始化
	m_nConnectingNum	= 0;

	return	0;
}
int		CAAFS2_I::Init_CFG(const char *cszCFG)
{
	WHDATAINI_CMN	ini;
	ini.addobj("CAAFS", &m_cfginfo);
	ini.addobj("MSGERGMS", &m_MSGERGMS_INFO);
	ini.addobj("CLIENT_SLOTMAN", &m_CLIENT_SLOTMAN_INFO);

	// 默认参数
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

	// 根据变量值计算其他一些相关变量
	m_cfginfo._nSelfInfoToClientLen					= strlen(m_cfginfo.szSelfInfoToClient) + 1;
	// 最多连接数不能超过逻辑上的最多人数
	m_CLIENT_SLOTMAN_INFO.nMaxConnection			= m_cfginfo.nClientWaitingNumMax + m_cfginfo.nClientConnectingNumMax;
	// slot的固定参数
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nType		= CNL2_CHANNELTYPE_SAFE;
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nInNum		= CAAFS_CHANNEL_INOUT_NUM;
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nOutNum	= CAAFS_CHANNEL_INOUT_NUM;
	// 设置通道只能发送数据，但不能接收数据（2008-12-24改为可以接收，因为需要预先校验用户身份）
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nProp		= CNL2CHANNELINFO_T::PROP_CANSEND | CNL2CHANNELINFO_T::PROP_CANRECV;

	// 绑定的地址
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

	// 根据变量值计算其他一些相关变量
	m_cfginfo._nSelfInfoToClientLen					= strlen(m_cfginfo.szSelfInfoToClient) + 1;

	// 重新设置进入速率
	m_generWaitTransToConnect.SetGenSpeed(m_cfginfo.fWaitTransToConnectRate);

	m_pSlotMan4Client->Init_Reload(m_CLIENT_SLOTMAN_INFO.GetBase());
	return	0;
}
int		CAAFS2_I::Release()
{
	// 各个单元清空(要在后面的清理前写，否则如果用到后面的管理器就惨了)
	whunitallocatorFixed_AllClear(m_Players);

	// 各个数组释放
	m_Players.Release();

	// 时间队列
	m_TQ.Release();

	// CNL2终结
	WHSafeSelfDestroy( m_pSlotMan4Client );
	// TCP终结
	m_msgerGMS.Release();
	cmn_safeclosesocket(m_MSGERGMS_INFO.sock);

	// 排队通道销毁
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
	// CAAFS删除玩家的时候应该不用通知GMS(因为对于等待中的玩家GMS不知道，对于连接中得的玩家，GMS不需要理会)

	if( pPlayer == NULL )
	{
		return	-1;
	}

	// 如果在连接队列中则连接人数--
	// clear中会离队
	if( pPlayer->nStatus == PlayerUnit::STATUS_CONNECTINGCLS )
	{
		// 下个Tick的时候如果发现这个数没有达到上限就可以继续把人往里面塞
		m_nConnectingNum	--;
		if( m_nConnectingNum<0 )
		{
			m_nConnectingNum	= 0;
			GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(0,TEMP)"CAAFS2_I::RemovePlayerUnit m_nConnectingNum is negative!!!");
		}
	}

	if( m_pSlotMan4Client->IsSlotExist(pPlayer->nSlot) )
	{
		// 取消关联(免得在slot关闭的时候想删除玩家出错)
		m_pSlotMan4Client->SetExtLong(pPlayer->nSlot, 0);
		// 关闭Slot
		m_pSlotMan4Client->Close(pPlayer->nSlot);
	}


	//add by 张昊阳 2012/3/20
	//在此之前，激活resumemap
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
			printf("用户离开，保护现场: %s,seq0 : %d,seq : %d\n",client_iter->second.c_str(),iter->second.nSeq0,iter->second.nSeq);
#endif
		}
		m_clientID2Account_map.erase(client_iter);
	}
	
	
	// 自我清理(这个得在前面其他的清理工作完成之后才能做，否则变量情况了就不知道原来有没有其他关系了)
	pPlayer->clear();
	// 移除
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
	// 系统应该早就初始化过随机种子了
	return	wh_time() ^ rand();
}
size_t	CAAFS2_I::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	// 连接GMS的
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
	// 设置为重连状态
	m_bIsReconnect		= true;
	// 把GMS可用人数改为0（重连之后GMS自然会发送结果过来）
	m_nGMSAvailPlayerNum	= 0;
}
void	CAAFS2_I::Retry_Worker_WORKING_Begin()
{
	// 成功则发送自己的信息(表示自己是CAAFS，需要获得相应的信息)
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
		// 已经在这个channel了就不用继续放了
		return	false;
	}

	// 为了保险原来如果在那个里面就先退出一下
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

	// 先把原来的给退了
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
	//赶紧给玩家发送一下当前状态
	SendPlayerQueueInfo(pPlayer, true);
	return	true;
}
void	CAAFS2_I::SendPlayerQueueInfo(PlayerUnit *pPlayer, bool bMustSend)
{
	WAITCHANNEL_T		*pC	= m_vectWaitChannel.getptr(pPlayer->nVIPChannel);
	if( bMustSend || pC->nWaitingSeq0 != pPlayer->nSeq0 )
	{
		// 需要重新发送
		CAAFS_CLIENT_QUEUEINFO_T	QueueInfo;
		QueueInfo.nCmd				= CAAFS_CLIENT_QUEUEINFO;
		QueueInfo.nVIPChannel		= pPlayer->nVIPChannel;
		pPlayer->nSeq0				= pC->nWaitingSeq0;
		if(pPlayer->nSeq < pPlayer->nSeq0)
			QueueInfo.nQueueSize = 7; //这个是用来骗客户端的假数字，因为是插队进来的，不知道前面有几个人
		else
			QueueInfo.nQueueSize		= pPlayer->nSeq - pPlayer->nSeq0;
		m_pSlotMan4Client->Send(pPlayer->nSlot, &QueueInfo, sizeof(QueueInfo), 0);
	}
}

void CAAFS2_I::DisplayQueueInfo()
{
	printf("-----开始打印队列信息-----\n\n");
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
				printf("-- ID: %d\tSeq: %u\tIP:%s\t帐号:%s\n ",pNode->data->nID,pNode->data->nSeq,inet_ntoa(a),acct_name.c_str());
				pNode		= pNode->next;
			}
			printf("-----------------\n\n");
		}
	}
}

void	CAAFS2_I::DealKeepQueuePosReq(PlayerUnit *pPlayer,const char * szAccount)
{
	
		//判断是不是需要resume
		map<string,ResumedPlayerUnit>::iterator iter = m_wait2resume_map.find(szAccount);
		if(iter!=m_wait2resume_map.end())
		{
			//老人了，有优待
#ifdef UNITTEST
			printf("老用户了,优待 : %s\n",szAccount);
#endif
			//和新的绑定了
			m_clientID2Account_map.insert(make_pair(pPlayer->nID,szAccount));
			//他上次数据就不大对头
			if(iter->second.nSeq0 == 0)
			{
#ifdef UNITTEST
				printf("BEFORE SEQ = 0 ,erase!\n");
#endif
				m_wait2resume_map.erase(iter);
			}else{

				//说明上一位还在呢，这又来一个，
				if(iter->second.nLeaveTime==0)
				{
#ifdef UNITTEST
					assert(0);
#endif
					//应该拒绝他的这个请求。
					return;
				}

				WAITCHANNEL_T			*pC	= m_vectWaitChannel.getptr(iter->second.nVIPChannel);
				//已经叫过号了，直接进去
				if( pC->nWaitingSeq0 >=iter->second.nSeq0)
				{
					pPlayer->nSeq = iter->second.nSeq;
					pPlayer->nSeq0 = iter->second.nSeq0;

				}else{
					//还没到呢，但是可以插队
					pPlayer->nSeq0 = pC->nWaitingSeq0;
					pPlayer->nSeq = iter->second.nSeq;
				}
				ResumePlayerPosInVIPChannel(pPlayer,iter->second.nVIPChannel);
				return;
			}
		}
		//新来的，加进去
#ifdef UNITTEST
		printf("新来的吧,加入 : %s\n",szAccount);
#endif
		ResumedPlayerUnit resumed_player_t;
		m_wait2resume_map.insert(make_pair(szAccount,resumed_player_t));
		m_clientID2Account_map.insert(make_pair(pPlayer->nID,szAccount));
		return ;
}
