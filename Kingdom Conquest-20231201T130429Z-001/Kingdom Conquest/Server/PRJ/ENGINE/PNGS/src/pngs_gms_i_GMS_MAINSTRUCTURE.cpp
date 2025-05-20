// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE模块的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-08-22
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

////////////////////////////////////////////////////////////////////
// GMS_MAINSTRUCTURE
////////////////////////////////////////////////////////////////////
// 配置CFGINFO_T
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

// 类内容
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

	// 初始化TCPSERVER
	if( (rst=m_CntrSvr.Init(m_cfginfo.CNTRSVR.GetBase()))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(22,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::Init,%d,m_CntrSvr.Init", rst);
		return	-2;
	}

	// 初始化时间队列
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(23,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::Init,%d,m_TQ.Init", rst);
		return	-3;
	}
	// 加入定时计算CAAFS可进入人数的时间事件
	TQUNIT_T	*pTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCalcAndSendCAAFSAvailPlayerTimeOut, (void **)&pTQUnit, &m_teid_CalcAndSend_CAAFS_AvailPlayerNum)<0 )
	{
		// 不可能在这个时候就无法加入时间事件
		assert(0);
		return	-4;
	}
	pTQUnit->tefunc		= &GMS_MAINSTRUCTURE::TEDeal_CalcAndSend_CAAFS_AvailPlayerNum;

	// 寻找内存管理模块（这个比较重要应该是在Init中调用的，而且内存模块一定是需要第一个被载入的）
	m_pLogicMEM			= m_pCMN->GetLogic(PNGS_DLL_NAME_GSMEM, GSMEM_VER);
	if( !m_pLogicMEM )
	{
		return			-11;
	}
	// 获取内存管理器
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
	// 判断共享内存是否是继承的
	{
		PNGSPACKET_2MEM_CONFIG_T	Config;
		Config.nSubCmd	= PNGSPACKET_2MEM_CONFIG_T::SUBCMD_GET_MEMSTATUS;
		if( m_pLogicMEM->CmdIn_Instant(this, PNGSPACKET_2MEM_CONFIG, &Config, sizeof(Config))<0 )
		{
			return		-13;
		}
		if( Config.nParam==0 )
		{
			// 是正常初始化的
			// 生成对象并设置回去
			m_pSHMData	= MYSHMDATA_T::NewMe(m_am);
			assert(m_pSHMData);
			PNGSPACKET_2MEM_XXX_PTR_T	XPtr;
			XPtr.pcszKey	= FATHERCLASS::m_szLogicType;
			XPtr.ptr		= m_pSHMData;
			if( m_pLogicMEM->CmdIn_Instant(this, PNGSPACKET_2MEM_SET_PTR, &XPtr, sizeof(XPtr))<0 )
			{
				return	-14;
			}
			// 初始化各个数组(这个应该不可能出错的，除非配置文件出错)
			m_pSHMData->m_CLSs.Init(m_ao, m_cfginfo.nMaxCLS);
			m_pSHMData->m_GMTOOLs.Init(m_ao, m_cfginfo.nMaxGMTOOL);
			m_pSHMData->m_Players.Init(m_ao, m_cfginfo.nMaxPlayer);
		}
		else
		{
			// 是继承的
			int	nOffset	= Config.nParam1;
			// 从那边获得对象并进行内部调整
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

	// 输入指令队列
	ILogicBase::QueueInit(m_cfginfo.nQueueCmdInSize);

	// 初始化字串指令映射表
	InitTstStrMap();

	return	0;
}
int		GMS_MAINSTRUCTURE::Init_CFG(const char *cszCFGName)
{
	// 分析文件
	WHDATAINI_CMN	ini;
	ini.addobj("GMS", &m_cfginfo);
	ini.addobj("MSGERGMS", &m_MSGER_INFO);
	// 设置一下默认值（服务器的这两个值默认设置大一些吧）
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

	// 计算参数
	// 先计算nMaxCLS
	m_cfginfo.nMaxCLS			= 0;
	for(int i=0;i<GMS_MAX_CAAFS_NUM;i++)
	{
		m_cfginfo.nMaxCLS		+= m_cfginfo.anMaxCLSNumInCAAFSGroup[i];
	}
	// 再计算后续和它相关的
	m_cfginfo.CNTRSVR.nMaxNum	= GMS_MAX_CAAFS_NUM + m_cfginfo.nMaxCLS + GZS_MAX_NUM + m_cfginfo.nMaxGMTOOL;

	return	0;
}
int		GMS_MAINSTRUCTURE::Init_CFG_Reload(const char *cszCFGName)
{
	// 分析文件
	WHDATAINI_CMN	ini;
	ini.addobj("GMS", &m_cfginfo);
	ini.addobj("MSGERGMS", &m_MSGER_INFO);
	// 设置一下默认值（服务器的这两个值默认设置大一些吧）
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

	// 计算参数
	// 先计算nMaxCLS
	m_cfginfo.nMaxCLS			= 0;
	for(int i=0;i<GMS_MAX_CAAFS_NUM;i++)
	{
		m_cfginfo.nMaxCLS		+= m_cfginfo.anMaxCLSNumInCAAFSGroup[i];
	}
	// 再计算后续和它相关的
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
	// 调用父类的Release
	ILogicBase::QueueRelease();
	return	0;
}
int		GMS_MAINSTRUCTURE::Detach(bool bQuick)
{
	// 等待一段时间
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
	// 实际上应该是等待所有其他服务器都断线了

	// 删除数据
	if( m_am )
	{
		m_am->m_nUseCount	--;
		if( m_pSHMData )
		{
			MYSHMDATA_T::DeleteMe(m_am, m_pSHMData);
		}
		m_am	= NULL;
	}
	// ... 待续 ...
	return	0;
}
int		GMS_MAINSTRUCTURE::Tick_BeforeDealCmdIn()
{
	// 本tick时刻
	m_tickNow	= wh_gettickcount();

	// 获得本tick的时间间隔
	m_itv.tick();

	// 时间队列处理
	Tick_DealTE();

	return	0;
}
int		GMS_MAINSTRUCTURE::Tick_AfterDealCmdIn()
{
	// 和其他服务器的TCP连接的工作
	m_CntrSvr.DoWork(false);

	// 向GZS发送缓冲指令
	// 获得可以加入多少个
	int		nNum	= m_generSendGZSSvrCmd.MakeDecision(m_itv.getfloatInterval());
	if( nNum>m_cfginfo.nSvrCmdPerSecond )
	{
		nNum		= m_cfginfo.nSvrCmdPerSecond;
	}
	while( nNum-- > 0 )
	{
		size_t	nSize	= 0;
		unsigned char	*pnSvrIdx		= (unsigned char *)m_queueGZSSvrCmd.OutFree(&nSize);	// 因为没有多线程，所以可以用OutFree
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
	// 为了确保发送再调用一次
	m_CntrSvr.ConnecterMan::SureSend();
	return	0;
}
int		GMS_MAINSTRUCTURE::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	// 和其他服务器的连接
	m_CntrSvr.GetSockets(vect);
	return	0;
}
int		GMS_MAINSTRUCTURE::Organize()
{
	// 寻找gameplay模块(找不到拉倒)
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
	if( m_nMood != MOOD_WORKING )	// 在停止状态的时候就不能接受任何连接了
	{
		return	NULL;
	}
	MYCNTRSVR::MYCNTR	*pCntr	= new MYCNTRSVR::MYCNTR(this);
	assert(pCntr);
	// 初始化之
	MYCNTRSVR::MYCNTR::INFO_T	info;
	memcpy(&info.msgerINFO, m_MSGER_INFO.GetBase(), sizeof(info.msgerINFO));
	info.msgerINFO.sock	= sock;		// 这个可不能忘啊!!!!
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
		// 这个应该是accepter
		return;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(28,PNGS_INT_RLS)"GMS_MAINSTRUCTURE new connecter,0x%X,%s", pMyCntr->GetIDInMan(), cmn_get_ipnportstr_by_saaddr(&pMyCntr->m_addr));
	// 启动定时器，如果没有收到Hello包则断开连接
	SetTE_Hello_TimeOut(pCntr);
}
void	GMS_MAINSTRUCTURE::InnerRouteClientGameCmd(CMN::cmd_t nCmd, CLS_SVR_CLIENT_DATA_T *pCmd, size_t nSize)
{
	nSize	-= wh_offsetof(CLS_SVR_CLIENT_DATA_T, data);
	assert( (int)nSize>0 );	// CLS是不会传来不大于0的数据的
	// 合成指令
	// 同线程
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
		// 是给GMS的
		if( !m_pLogicGamePlay )
		{
			// 如果没有这个当然就不发了
			return;
		}

		// 合成指令
		GMSPACKET_2GP_SVRCMD_T	Cmd2GP;
		Cmd2GP.nSvrIdx			= pGZS->nSvrIdx;
		Cmd2GP.nDSize			= nDSize;
		Cmd2GP.pData			= pGameCmd->data;
		CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, GMSPACKET_2GP_SVRCMD, Cmd2GP);
	}
	else if( pGameCmd->nToSvrIdx == PNGS_SVRIDX_ALLGZS )
	{
		// 所有的GZS
		SendSvrCmdToAllGZS(pGameCmd->data, nDSize, pGZS->nSvrIdx);
	}
	else if( pGameCmd->nToSvrIdx == PNGS_SVRIDX_ALLGZSEXCEPTME )
	{
		// 所有的GZS
		SendSvrCmdToAllGZS(pGameCmd->data, nDSize, pGZS->nSvrIdx, pGZS->nSvrIdx);
	}
	else if( pGameCmd->nToSvrIdx>=GZS_MAX_NUM )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(,PNGS_RT)"GMS_MAINSTRUCTURE::InnerRouteGZSGameCmd,%d,bad nToSvrIdx,0x%X,%d", pGameCmd->nToSvrIdx);
	}
	else
	{
		// 否则就转给相应的GZS
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
	// 设置所有的GZS为正常退出状态
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
	// 找到Client所在的CLS并发送之
	if( SendCmdToPlayerCLS(nClientID, &Ctrl, sizeof(Ctrl)) < 0 )
	{
		// 如果不能发送了，就直接模拟告诉上层用户OFFLINE了
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
	// 遍历一下这个CLS对应的所有玩家
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
	// 遍历一下这个GZS对应的所有玩家
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
	// 通知所有CLS该GZS断线（防止这是GZS的单方面断线行为）
	// 一旦GZS和CLS断线则CLS会让所有和该GZS相关的用户断线
	SVR_CLS_CTRL_T	Cmd;
	Cmd.nCmd		= SVR_CLS_CTRL;
	Cmd.nSubCmd		= SVR_CLS_CTRL_T::SUBCMD_DROPGZS;
	Cmd.nParam		= pGZS->nSvrIdx;
	SendCmdToAllCLS(&Cmd, sizeof(Cmd));
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(850,PNGS_RT)"DealGZSDisconnection,%d", pGZS->nSvrIdx);
	// 如果GZS已经通知过这边是正常退出的，则通知所有CLS提出和该GZS有关的用户
	// 这个是必要的，因为还可能有连接GZS过程中的用户
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
	// 通知GP
	GMSPACKET_2GP_GZSOFFLINE_T	GZSOL;
	GZSOL.nSvrIdx	= pGZS->nSvrIdx;
	GZSOL.nMode		= GMSPACKET_2GP_GZSOFFLINE_T::MODE_REALLYOFF;
	CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSOFFLINE, &GZSOL, sizeof(GZSOL));
	// Free这个GZS
	pGZS->clear();
	// 把prop保存回去，这样下次使用的时候可以知道上次是否是正常退出的
	pGZS->nProp		= nProp;
	return	0;
}
int		GMS_MAINSTRUCTURE::DealCLSDisconnection(CLSUnit *pCLS)
{
	// 清除所有和CLS相关的玩家（nCount中将记录CLS中的玩家数量）
	int	nCount	= 0;
	whDList<PlayerUnit *>::node	*pNode	= pCLS->dlPlayer.begin();
	while( pNode != pCLS->dlPlayer.end() )
	{
		PlayerUnit	*pPlayer	= pNode->data;
		// 下一个(否则RemovePlayerUnit会导致下一个失效)
		pNode	= pNode->next;
		// 是和这个CLS相关的
		RemovePlayerUnit(pPlayer, CLS_GMS_CLIENT_DROP_T::REMOVEREASON_AS_DROP);
		nCount	++;
	}
	// 注意：必须先处理完玩家再处理CLS，否则玩家可能会用到CLS的一些数据
	// 从Group中删除
	pCLS->dlnode.leave();
	// 删除对象
	pCLS->clear();
	m_pSHMData->m_CLSs.FreeUnitByPtr(pCLS);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,PNGS_RT)"DealCLSSDisconnection,0x%X,%d", pCLS->nID, nCount);
	return	0;
}
