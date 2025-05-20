// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_client_i.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的Client模块内部实现
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-08-25
// Change LOG   :

#include "../inc/pngs_client_i.h"

using namespace n_pngs;

#ifdef CLIENT_STATIC_ANTIHACK
ICryptFactory*	PNGSClient_I::m_pICryptFactory	= NULL;
CNL2SlotMan*	PNGSClient_I::m_pSlotMan		= NULL;
#endif

namespace
{
WHDLL_Loader_Man *	l_DLM	= NULL;
}
void	PNGSClient::DLLPool_Init()
{
	if( l_DLM==NULL )
	{
		l_DLM	= WHDLL_Loader_Man::Create();
	}
}
void	PNGSClient::DLLPool_Release()
{
	if( l_DLM!=NULL )
	{
		delete	l_DLM;
		l_DLM	= NULL;
	}
}
PNGSClient *	PNGSClient::Create()
{
	return	new PNGSClient_I;
}

////////////////////////////////////////////////////////////////////
// PNGSClient_I
////////////////////////////////////////////////////////////////////
// 配置CFGINFO_T
WHDATAPROP_MAP_BEGIN_AT_ROOT(PNGSClient_I::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szCAAFSSelfInfoToClient, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSlotInOutNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nControlOutQueueSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nDataQueueSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueCmdInSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nPlayStatInteravl, 0)
WHDATAPROP_MAP_END()
WHDATAPROP_MAP_BEGIN_AT_ROOT(PNGSClient_I::PLUGIN_CFGINFO_T)
	WHDATAPROP_ON_SETVALFUNC(DLL)
WHDATAPROP_MAP_END()

// 类内容
PNGSClient_I::PNGSClient_I()
: m_nStatus(STATUS_NOTHING)
, m_nLastError()
, m_tickNow(0)
, m_nClientID(0)
#ifndef CLIENT_STATIC_ANTIHACK
, m_pICryptFactory(NULL)
, m_pSlotMan(NULL)
#endif
, m_nSlotCAAFS(0), m_nSlotCLS(0)
, m_pTickControlOut(&PNGSClient_I::Tick_ControlOut_STATUS_NOTHING)
, m_pTickDealRecv(&PNGSClient_I::Tick_DealRecv_STATUS_NOTHING)
, m_tid(INVALID_TID)
, m_bStopThread(false)
{
	m_szVer[0]	= 0;
	m_vectControlOut.reserve(1024);
	m_vectData.reserve(16384);
	m_vectrawbuf.reserve(16384);
	// 为了避免初始化顺序的文件，就在这里直接赋值算了
	m_pcszCFG			= NULL;
	m_vectDLLLoader.reserve(16);
	m_bQuickDetachLogic	= false;
	// 作为一个插件模块的名字
	strcpy(ILogicBase::m_szLogicType, PNGS_DLL_NAME_CLIENT_GAMEPLAY);
	// 版本
	ILogicBase::m_nVer	= PNGSCLIENT_VER;
}
PNGSClient_I::~PNGSClient_I()
{
}
void	PNGSClient_I::SelfDestroy()
{
	delete	this;
}
int		PNGSClient_I::Init(const char *cszCFG)
{
	// 设置一下日志输出函数
	CMNBody::m_fn_GLogger_WriteFmt	= &GLOGGER2_WRITEFMT;

	// 只要出错了，这个就是true
	m_bQuickDetachLogic	= true;

	m_tickNow	= wh_gettickcount();

	assert( m_pSlotMan == NULL );

	int	rst;
	// 读入配置信息（附带加载PlugIn）
	if( (rst=Init_CFG(cszCFG))<0 )
	{
		return	-1;
	}

	// 时间队列
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		assert(0);
		return	-2;
	}

	// ControlOut队列
	if( m_queueControlOut.Init(m_cfginfo.nControlOutQueueSize)<0 )
	{
		return	-3;
	}
	// 给上层的数据队列
	if( m_queueData.Init(m_cfginfo.nDataQueueSize)<0 )
	{
		return	-4;
	}

	// 先初始化一下SLotMan
	m_pSlotMan	= CNL2SlotMan::Create();
	if( !m_pSlotMan )
	{
		assert(0);
		return	-11;
	}
	// 确定通道参数为只有一个通道，且只能接收不能发送
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nType		= CNL2_CHANNELTYPE_SAFE;
	if( (rst=m_pSlotMan->Init(m_CLIENT_SLOTMAN_INFO.GetBase(), NULL))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(606,PNGS_INT_RLS)"PNGSClient_I::Init,%d,m_pSlotMan->Init", rst);
		return	-12;
	}
	// 默认客户端应该是不能被连接的
	m_pSlotMan->Listen(false);

	// 设置加密工厂
	m_pSlotMan->SetICryptFactory(m_pICryptFactory);

	// 组织一下PlugIn（之间的相互关系）（因为载Init_CFG中只是加载并初始化了，没有做联合组织的工作）
	if( (rst=CMNBody::OrganizeLogic()) < 0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(607,PNGS_INT_RLS)"PNGSClient_I::Init,%d,CMNBody::OrganizeLogic", rst);
		return	-31;
	}

	// 输入指令队列
	ILogicBase::QueueInit(m_cfginfo.nQueueCmdInSize);

	// 设置状态为什么也没有
	SetStatus(STATUS_NOTHING);

	// 上面全部成功，则不用快速释放了
	m_bQuickDetachLogic	= false;

	return	0;
}
int		PNGSClient_I::Init_CFG(const char *cszCFG)
{
	m_pcszCFG			= cszCFG;
	m_PlugInCfg.pHost	= this;

	WHDATAINI_CMN	ini;
	ini.addobj("CLIENT", &m_cfginfo);
	ini.addobj("CLIENT_SLOTMAN", &m_CLIENT_SLOTMAN_INFO);
	ini.addobj("PlugIn", &m_PlugInCfg);

	// 默认改成true
	m_CLIENT_SLOTMAN_INFO.bCanSwitchAddr	= true;

	int	rst = ini.analyzefile(cszCFG);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(605,PNGS_INT_RLS)"PNGSClient_I::Init_CFG,%d,analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	// 一些固定的设置

	// 根据变量值计算其他一些相关变量

	return	0;
}
int		PNGSClient_I::Init_CFG_Reload(const char *cszCFG)
{
	m_pcszCFG			= cszCFG;

	WHDATAINI_CMN	ini;
	ini.addobj("CLIENT", &m_cfginfo);
	ini.addobj("CLIENT_SLOTMAN", &m_CLIENT_SLOTMAN_INFO);

	int	rst = ini.analyzefile(cszCFG, false, 1);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(605,PNGS_INT_RLS)"PNGSClient_I::Init_CFG_Reload,%d,analyzefile,%s", rst, ini.printerrreport());
		return	-1;
	}

	return	0;
}
int		PNGSClient_I::Release()
{
	ILogicBase::QueueRelease();
	// 为了保险一定要关闭一下thread
	StopThread();

	m_TQ.Release();

	// 脱离所有Logic
	CMNBody::DetachAllLogic(true, m_bQuickDetachLogic);
	// 卸载所有DLL
	for(size_t i=0;i<m_vectDLLLoader.size();i++)
	{
		if( m_vectDLLLoader[i] != NULL )
		{
			delete	m_vectDLLLoader[i];
			m_vectDLLLoader[i]	= NULL;
		}
	}
	// 删除
	WHSafeSelfDestroy(m_pSlotMan);
	return	0;
}
int		PNGSClient_I::SetICryptFactory(n_whcmn::ICryptFactory *pFactory)
{
	m_pICryptFactory	= pFactory;
	if( m_pSlotMan )
	{
		m_pSlotMan->SetICryptFactory(m_pICryptFactory);
	}
	return	0;
}
int		PNGSClient_I::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	if( m_tid == INVALID_TID )
	{
		return	_GetSockets(vect);
	}
	return	0;
}
int		PNGSClient_I::_GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	if( m_pSlotMan->GetSockets(vect)<0 )
	{
		return	-1;
	}
	if( CMNBody::GetSockets(vect)<0 )
	{
		return	-2;
	}
	return	0;
}
void	PNGSClient_I::SetVer(const char *szVer)
{
	strcpy(m_szVer, szVer);
}
const char *	PNGSClient_I::GetVer()
{
	return	m_szVer;
}
int		PNGSClient_I::Connect(const char *cszAddr, unsigned char nNetworkType)
{
	// 确保现在没有任何slot
	if( m_pSlotMan->GetSlotNum()>0 )
	{
		SetLastError(ERR_CONNECT_LASTCONNECTNOTOVER);
		return	ERR_CONNECT_LASTCONNECTNOTOVER;
	}
	// 确定通道参数为只有一个通道，且只能接收不能发送（2008-12-24改为可以发送，因为需要预先校验用户身份）
	m_CLIENT_SLOTMAN_INFO.clearallchannelinfo();
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nType		= CNL2_CHANNELTYPE_SAFE;
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nInNum		= 16;
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nOutNum	= 16;
	m_CLIENT_SLOTMAN_INFO.channelinfo[0].nProp		= CNL2CHANNELINFO_T::PROP_CANSEND | CNL2CHANNELINFO_T::PROP_CANRECV;
	m_pSlotMan->ReConfigChannel(m_CLIENT_SLOTMAN_INFO.GetBase());

	// 在连接过程中应该是不能被连接的
	m_pSlotMan->Listen(false);
	// 和CLS连接的slot要清空一下
	m_nSlotCLS		= 0;

	// 附加数据
	CLIENT_CONNECT_CAAFS_EXTINFO_T	ExtInfo;
	ExtInfo.cmn.nNetworkType	= nNetworkType;
	// 创建slot
	m_nSlotCAAFS	= m_pSlotMan->Connect(cszAddr, &ExtInfo, sizeof(ExtInfo));
	if( m_nSlotCAAFS<0 )
	{
		SetLastError(ERR_CONNECT_CNL2_CONNECTERROR);
		return	ERR_CONNECT_CNL2_CONNECTERROR;
	}
	// 设置连接状态
	SetStatus(STATUS_CONNECTINGCAAFS);

	return	ERR_OK;
}
int		PNGSClient_I::Disconnect()
{
	// 如果有线程必须先关闭线程
	StopThread();

	// 关闭所有的
	if( !m_pSlotMan )
	{
		return	0;
	}
	m_pSlotMan->CloseAll();
	return	0;
}
int		PNGSClient_I::ControlOut(CONTROL_T **ppCmd, size_t *pnSize)
{
	if( m_tid != INVALID_TID )
	{
		return	-100;
	}
	*pnSize	= m_queueControlOut.PeekSize();
	if( *pnSize<=0 )
	{
		return	-1;
	}
	m_vectControlOut.resize(*pnSize);
	*ppCmd	= (CONTROL_T *)m_vectControlOut.getbuf();
	m_queueControlOut.Out(m_vectControlOut.getbuf(), pnSize);
	return	0;
}
int		PNGSClient_I::SendCAAFS(const void *pCmd, size_t nSize)
{
	return	m_pSlotMan->Send(m_nSlotCAAFS, pCmd, nSize, 0);
}
int		PNGSClient_I::Send(unsigned char nSvrIdx, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	if( m_tid != INVALID_TID )
	{
		return	-100;
	}
	// 转换为CLIENT_CLS_DATA_T
	m_vectrawbuf.resize(wh_offsetof(CLIENT_CLS_DATA_T, data) + nSize);
	CLIENT_CLS_DATA_T	*pData	= (CLIENT_CLS_DATA_T *)m_vectrawbuf.getbuf();
	pData->nCmd			= CLIENT_CLS_DATA;
	pData->nSvrIdx		= nSvrIdx;
	memcpy(pData->data, pCmd, nSize);
	return	m_pSlotMan->Send(m_nSlotCLS, m_vectrawbuf.getbuf(), m_vectrawbuf.size(), nChannel);
}
int		PNGSClient_I::Recv(unsigned char *pnChannel, void **ppCmd, size_t *pnSize)
{
	if( m_tid != INVALID_TID )
	{
		return	-100;
	}
	if( m_nStatus != STATUS_WORKING )
	{
		return	-1;
	}

	size_t	nSize	= 0;	// 这个如果不设初值的话，在release版下可能会出错
	DATA_T	*pDataToQueue	= (DATA_T *)m_queueData.OutFree(&nSize);
	if( !pDataToQueue )
	{
		*ppCmd	= NULL;
		*pnSize	= 0;
		return	-2;
	}
	*pnSize		= pDataToQueue->nSize;
	m_vectData.resize(*pnSize);
	memcpy(m_vectData.getbuf(), pDataToQueue->data, *pnSize);
	*ppCmd		= m_vectData.getbuf();
	*pnChannel	= pDataToQueue->nChannel;

	return	0;
}
int		PNGSClient_I::GetLastError() const
{
	return	m_nLastError;
}
void	PNGSClient_I::SetLastError(int nErrCode)
{
	m_nLastError	= nErrCode;
}
void *	PNGSClient_I::ControlOutAlloc(size_t nSize)
{
	void *ptr	= m_queueControlOut.InAlloc(nSize);
	if( !ptr )
	{
		// 应该是很严重的问题了，怎么溢出呢？记录日志!
		assert(0);
	}
	return		ptr;
}
void	PNGSClient_I::SetFileMan(n_whcmn::whfileman *pFileMan)
{
	CMNBody::SetFileMan(pFileMan);
}
ILogicBase *	PNGSClient_I::GetPlugIn(const char *cszPlugInName, int nVer)
{
	return	CMNBody::GetLogic(cszPlugInName, nVer);
}
int		PNGSClient_I::GetPing() const
{
	CNL2SLOTINFO_T *pInfo	= m_pSlotMan->GetSlotInfo(m_nSlotCLS);
	if( pInfo )
	{
		return	pInfo->nRTT;
	}
	return	-1;
}
void *	PNGSClient_I::thread_func(void *ptr)
{
	whvector<SOCKET> vectSock;
	PNGSClient_I	*pClient	= (PNGSClient_I *)ptr;
	pClient->m_bStopThread		= false;
	while( !pClient->m_bStopThread )
	{
		vectSock.clear();
		pClient->_GetSockets(vectSock);
		cmn_select_rd_array(vectSock.getbuf(), vectSock.size(), 20);
		pClient->_TickRecv();
		pClient->_TickSend();
	}
	return	NULL;
}
int		PNGSClient_I::StartThread()
{
	if( INVALID_TID != m_tid )
	{
		// 线程已经开启了，不要start两次
		assert(0);
		return	-1;
	}
	whthread_create(&m_tid, &PNGSClient_I::thread_func, this);
	return	0;
}
int		PNGSClient_I::StopThread()
{
	if( INVALID_TID == m_tid )
	{
		// 已经关闭了，不过不用报错，允许上层为了保险多调用几次
		return	-1;
	}
	m_bStopThread	= true;
	whthread_waitend_or_terminate(m_tid, 1000);
	m_tid	= INVALID_TID;
	return	0;
}
int		PNGSClient_I::AddPlugIn(const char *cszVal)
{
	assert(l_DLM);
	char					szDLLFile[WH_MAX_PATH]	= "";
	char					szFuncName[128]			= "";
	char					szCFGFile[WH_MAX_PATH]	= "";
	int						nThreadTag				= 0;
	strcpy(szCFGFile, m_pcszCFG);
	// 格式：
	// threadtag, sonanme, createfunc[, cfgfile]
	wh_strsplit("dsss", cszVal, ",", &nThreadTag, szDLLFile, szFuncName, szCFGFile);

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(608,PNGS_INT_RLS)"AddPlugIn module,%s", szDLLFile);
	CMN::ILogic				*pLogic	= NULL;
	if( stricmp(szFuncName, "DUMMY")==0 )
	{
		// 如果是DummyDLL则创建Dummy
		pLogic	= CMN::CreateDummyLogic(szDLLFile);
	}
	else
	{
		int	rst	= 0;
		fn_ILOGIC_CREATE_T	CreateFunc;
		WHDLL_LOAD_UNIT_T	aUnits[]	=
		{
			WHDLL_LOAD_UNIT_DECLARE0(szFuncName, CreateFunc, NULL)
			WHDLL_LOAD_UNIT_DECLARE_LAST()
		};
		WHDLL_Loader		*pLoader	= l_DLM->Load(szDLLFile, aUnits);
		if( pLoader==NULL )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(578,PNGS_INT_RLS)"LoadDLL,,ERR,%s", szDLLFile);
			return	-1;
		}
		m_vectDLLLoader.push_back(pLoader);
		// 主线程和DLL之间的网络基本数据同步
		fn_WHCMN_STATIC_INFO_In		WHCMN_STATIC_INFO_In	= (fn_WHCMN_STATIC_INFO_In)pLoader->GetFunc("WHCMN_STATIC_INFO_In");
		if( WHCMN_STATIC_INFO_In != NULL )
		{
			(*WHCMN_STATIC_INFO_In)(WHCMN_STATIC_INFO_Out());
		}
		else
		{
			return	-2;
		}
		fn_WHNET_STATIC_INFO_In		WHNET_STATIC_INFO_In	= (fn_WHNET_STATIC_INFO_In)pLoader->GetFunc("WHNET_STATIC_INFO_In");
		if( WHNET_STATIC_INFO_In != NULL )
		{
			(*WHNET_STATIC_INFO_In)(WHNET_STATIC_INFO_Out());
		}
		else
		{
			return	-3;
		}
		pLogic	= CreateFunc();
		if( !pLogic )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(579,PNGS_INT_RLS)"LoadDLL,,CreateFunc,%s", szFuncName);
			return	-4;
		}
		if( (rst=CMNBody::AttachLogic(pLogic)) < 0 )
		{
			// 如果Attach失败则需要立刻删除这个，因为svr内部并没有记录它，所以后面的DetachAllLogic(true)就无法删除
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(580,PNGS_INT_RLS)"LoadDLL,%d,svr.AttachLogic,%s", rst, pLogic->GetLogicType());
			pLogic->SelfDestroy();
			return	-5;
		}
		// 设置文件管理器（2007-11-19 把这个提到Init之前，因为模块中可能在Init中就需要文件管理器指针）
		pLogic->SetFileMan(CMNBody::GetFileMan());
		//
		if( (rst=pLogic->Init(szCFGFile)) < 0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(581,PNGS_INT_RLS)"LoadDLL,%d,pLogic->Init,%s,%s", rst, szDLLFile, szCFGFile);
			return	-6;
		}
	}

	return	0;
}
int		PNGSClient_I::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch( nCmd )
	{
	case	CMD2CMN_INSTANT_REGISTERCMDDEAL:
		{
			CMD2CMN_INSTANT_REGISTERCMDDEAL_T	*pReg	= (CMD2CMN_INSTANT_REGISTERCMDDEAL_T *)pData;
			if( !m_mapCmdReg2Logic.put(pReg->nCmdReg, pRstAccepter) )
			{
				// 说明应该已经被注册过一次了
				assert(0 && "This cmd is registered!");
				return	-1;
			}
		}
		break;
	}
	return	0;
}
int		PNGSClient_I::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch( nCmd )
	{
	case	CLIENTPACKET_2CLIENT_SENDCMD2SVR:
		{
			CLIENTPACKET_2CLIENT_SENDCMD2SVR_T	*pSendCmd2Svr	= (CLIENTPACKET_2CLIENT_SENDCMD2SVR_T *)pData;
			int res;
			if ((res = Send(pSendCmd2Svr->nSvrIdx, pSendCmd2Svr->nChannel, pSendCmd2Svr->pData, pSendCmd2Svr->nDSize)) != 0)
				GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, "PNGSClient_I::DealCmdIn_One Send ERRO:%d",res);
		}
		break;
	}
	return	0;
}
