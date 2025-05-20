// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cmn.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的通用构架模块
//                PNGS是Pixel Network Game Structure的缩写
//                关于Logic和Interface的Type需要在具体应用中具体定义
// CreationDate : 2005-07-25
// Change LOG   : 2006-03-20 把CMNBody::DetachAllLogic改为先调用各个Logic的Detach，然后再把所有Logic删除。

#include "../inc/pngs_cmn.h"
#include "../inc/pngs_def.h"
#include "../inc/pngs_packet_gms_logic.h"
#include "../inc/pngs_packet_logic.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whnetudp.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

////////////////////////////////////////////////////////////////////
// ILogicBase
////////////////////////////////////////////////////////////////////
void	ILogicBase::SetStopTick(bool bSet)
{
	m_bStopTick	= bSet;
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1129,PNGS_RT)"%s,CMN::ILogic::SetStopTick,%d", GetLogicType(), bSet);
}
int		ILogicBase::QueueInit(int nQueueSize)
{
	if( m_queueCmdIn.Init(nQueueSize)<0 )
	{
		return	-1;
	}
	m_queueCmdIn.setUseLock(true);
	return	0;
}
int		ILogicBase::QueueRelease()
{
	m_queueCmdIn.Release();
	return	0;
}
int		ILogicBase::DealCmdIn()
{
	// 从队列中读出指令，处理之
	CMDHDR_T	*pHdr;
	size_t		nSize;
	while( (pHdr=(CMDHDR_T *)m_queueCmdIn.OutPeek(&nSize)) != NULL )
	{
		void	*pData	= pHdr+1;
		DealCmdIn_One(pHdr->pRstAccepter, pHdr->nCmd, pData, nSize - sizeof(CMDHDR_T));
		// 从queue中释放这个处理过的cmd(!!!!注意前面不能有return啊)
		m_queueCmdIn.FreeN(1);
	}

	return	0;
}
int		ILogicBase::CmdIn_MT(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	m_queueCmdIn.lock();
	CMDHDR_T	*pHdr	= (CMDHDR_T *)m_queueCmdIn.whsmpqueue::InAlloc( sizeof(CMDHDR_T) + nDSize );
	if( pHdr==NULL )	// 上层应该判断返回值，或者对指令加上超时
	{
		m_queueCmdIn.unlock();
		assert(0);
		return	-1;
	}
	pHdr->pRstAccepter	= pRstAccepter;
	pHdr->nCmd			= nCmd;
	// 拷贝数据到后面
	if( nDSize>0 )
	{
		memcpy(pHdr+1, pData, nDSize);
	}
	m_queueCmdIn.unlock();

	return	0;
}
int		ILogicBase::CmdIn_MT_lock(ILogicBase *pRstAccepter, cmd_t nCmd, void **ppData, int nDSize)
{
	m_queueCmdIn.lock();
	CMDHDR_T	*pHdr	= (CMDHDR_T *)m_queueCmdIn.whsmpqueue::InAlloc( sizeof(CMDHDR_T) + nDSize );
	if( !pHdr )
	{	
		*ppData			= NULL;
		m_queueCmdIn.unlock();
		assert(0);
		return	-1;
	}
	pHdr->pRstAccepter	= pRstAccepter;
	pHdr->nCmd			= nCmd;
	*ppData				= pHdr+1;
	return	0;
}
int		ILogicBase::CmdIn_MT_unlock()
{
	m_queueCmdIn.unlock();
	return	0;
}

////////////////////////////////////////////////////////////////////
// CMN::ILogic
////////////////////////////////////////////////////////////////////
// 工作一次(里面再调用子类的Tick_BeforeCmdIn()和Tick_AfterCmdIn())
int		CMN::ILogic::Tick()
{
	// 如果这里不让tick，就不继续了
	if( m_bStopTick )
	{
		return	0;
	}

dbgtimeuse	m_dtu;
m_dtu.BeginMark();
m_dtu.Mark();

	if( Tick_BeforeDealCmdIn()<0 )
	{
		return	-1;
	}

m_dtu.Mark();

	DealCmdIn();

m_dtu.Mark();

	if( Tick_AfterDealCmdIn()<0 )
	{
		return	-2;
	}

m_dtu.Mark();

if( m_dtu.GetDiff()>=200 )
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BIGTICK)"%s,CMN::ILogic::Tick,%s", GetLogicType(), m_dtu.Disp());
}

	return	0;
}

////////////////////////////////////////////////////////////////////
// DummyLogic
////////////////////////////////////////////////////////////////////
class	PNGSDummyLogic	: public CMN::ILogic
{
public:
	PNGSDummyLogic()
	{
		m_nVer	= -1;	// 写成-1就不会校验版本了
	}
private:
	virtual	int		Organize()
	{
		return	0;
	}
	virtual	void	SelfDestroy()
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,TEMP)"%s PNGSDummyLogic::SelfDestroy", m_szLogicType);
		delete	this;
	}
	// 初始化
	virtual	int		Init(const char *cszCFGName)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,TEMP)"%s PNGSDummyLogic::Init", m_szLogicType);
		return	0;
	}
	virtual	int		Init_CFG_Reload(const char *cszCFGName)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,TEMP)"%s PNGSDummyLogic::Init_CFG_Reload", m_szLogicType);
		return	0;
	}
	// 终结
	virtual	int		Release()
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,TEMP)"%s PNGSDummyLogic::Release", m_szLogicType);
		return	0;
	}
	// 获得可被外界做select的SOCKET，append到vector的尾部
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect)
	{
		return	0;
	}
	virtual	int		SureSend()
	{
		return	0;
	}
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,TEMP)"%s PNGSDummyLogic::DealCmdIn_One_Instant process cmd:%d dsize:%d", m_szLogicType, nCmd, nDSize);
		return	0;
	}
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,TEMP)"%s PNGSDummyLogic::DealCmdIn_One process cmd:%d dsize:%d", m_szLogicType, nCmd, nDSize);
		switch(nCmd)
		{
		case	PNGSPACKET_2LOGIC_SETMOOD:
			{
				PNGSPACKET_2LOGIC_SETMOOD_T	*pCmdSetMood	= (PNGSPACKET_2LOGIC_SETMOOD_T *)pData;
				switch( pCmdSetMood->nMood )
				{
				case	CMN::ILogic::MOOD_STOPPING:
					{
						// 默认就可以直接停止了
						m_nMood	= CMN::ILogic::MOOD_STOPPED;
					}
					break;
				}
			}
			break;
		}
		return	0;
	}
	virtual int		Tick_BeforeDealCmdIn()
	{
		return	0;
	}
	virtual int		Tick_AfterDealCmdIn()
	{
		return	0;
	}
public:
	void	SetLogicType(const char *cszLogicName)
	{
		strcpy(m_szLogicType, cszLogicName);
	}
};
CMN::ILogic *		CMN::CreateDummyLogic(const char *cszLogicName)
{
	PNGSDummyLogic	*pILogic	= new PNGSDummyLogic;
	pILogic->SetLogicType(cszLogicName);
	return	pILogic;
}

////////////////////////////////////////////////////////////////////
// CMN & CMNBody
////////////////////////////////////////////////////////////////////
static int		Dummy_GLogger_WriteFmt(int nID, const char *szFmt, ...)
{
	return	0;
}
CMN::CMN()
: m_fn_GLogger_WriteFmt(&Dummy_GLogger_WriteFmt)
{
}

CMNBody::CMNBody()
: m_bThreadOn(false)
, m_bShouldStop(false)
{
	m_vectLogic.reserve(16);
	m_vectSOCKET.SetNoResize(true);
	m_vectSOCKET.reserve(PNGS_MAX_SOCKETTOSELECT);
	m_vectThread.reserve(16);
}
CMNBody::~CMNBody()
{
}
// 关联逻辑模块
int		CMNBody::AttachLogic(ILogic *pLogic)
{
	if( pLogic->m_szLogicType[0] == 'u' )
	{
		// 则先查找名字是否重复
		if( GetLogic(pLogic->m_szLogicType, -1) != NULL )
		{
			// 名字重复
			(*m_fn_GLogger_WriteFmt)(GLOGGER_ID_ERROR, GLGR_STD_HDR(541,PNGS_INT_RLS)"CMNBody::AttachLogic,,dup type,%s", pLogic->GetLogicType());
			return	-1;
		}
	}
	bool	bUseOld	= false;
	size_t	i;
	for(i=0;i<m_vectLogic.size();i++)
	{
		if( m_vectLogic[i] == NULL )
		{
			bUseOld	= true;
			break;
		}
	}
	// 相互关联
	pLogic->m_nPos4CMN	= i;
	pLogic->m_pCMN		= this;
	if( bUseOld )
	{
		m_vectLogic[i]	= pLogic;
	}
	else
	{
		m_vectLogic.push_back(pLogic);
	}
	return	0;
}
// 解除关联
// 逻辑之间的相互连接(这个必须是在所有逻辑都attach完毕之后做)
int		CMNBody::OrganizeLogic()
{
	for(size_t i=0;i<m_vectLogic.size();i++)
	{
		if( m_vectLogic[i] != NULL )
		{
			int	rst	= m_vectLogic[i]->Organize();
			if( rst<0 )
			{
				(*m_fn_GLogger_WriteFmt)(GLOGGER_ID_ERROR, GLGR_STD_HDR(542,PNGS_INT_RLS)"CMNBody::OrganizeLogic,%d,%d,%s", rst, i, m_vectLogic[i]->GetLogicType());
				return	-1-i;
			}
		}
	}
	return	0;
}
// 作一次所有SOCKET的select操作
int		CMNBody::DoSelect(int nMS, int nThreadTag)
{
	m_vectSOCKET.clear();
	if( GetSockets(m_vectSOCKET, nThreadTag)>0 )
	{
		return	cmn_select_rd_array(m_vectSOCKET.getbuf(), m_vectSOCKET.size(), nMS);
	}
	else
	{
		// 免得在没有任何一个socket的时候占用CPU太高
		wh_sleep(nMS);
		return	0;
	}
}
size_t	CMNBody::GetSockets(n_whcmn::whvector<SOCKET> &vect, int nThreadTag)
{
	for(size_t i=0;i<m_vectLogic.size();i++)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL && (!m_bThreadOn || pLogic->GetThreadTag()==nThreadTag) )
		{
			pLogic->GetSockets(vect);
		}
	}
	return	vect.size();
}
// 工作一次
int		CMNBody::AllTick(int nThreadTag)
{
	size_t	i;
	// 原来是在一个TickLogic（即现在的AllTick+AllSureSend）中调用两次logic，然是这样可能造成第二次中处理了指令但是没有被发送的情况
	for(i=0;i<m_vectLogic.size();i++)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL && (!m_bThreadOn || pLogic->GetThreadTag()==nThreadTag) )
		{
			whtick_t	t1	= wh_gettickcount();
			pLogic->Tick();
			int	nDiff	= wh_tickcount_diff(wh_gettickcount(), t1);
			if( nDiff>200 )
			{
				(*m_fn_GLogger_WriteFmt)(GLOGGER_ID_ERROR, GLGR_STD_HDR(723,PNGS_RT)"CMNBody::AllTick,big tick,%d,%d,%s,%d", nDiff, i, pLogic->GetLogicType(), nThreadTag);
			}
		}
	}
	return	0;
}
int		CMNBody::AllSureSend(int nThreadTag)
{
	size_t	i;
	for(i=0;i<m_vectLogic.size();i++)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL && (!m_bThreadOn || pLogic->GetThreadTag()==nThreadTag) )
		{
			whtick_t	t1	= wh_gettickcount();
			pLogic->SureSend();
			int	nDiff	= wh_tickcount_diff(wh_gettickcount(), t1);
			if( nDiff>200 )
			{
				(*m_fn_GLogger_WriteFmt)(GLOGGER_ID_ERROR, GLGR_STD_HDR(723,PNGS_RT)"CMNBody::AllSureSend,big tick,%d,%d,%s,%d", nDiff, i, pLogic->GetLogicType(), nThreadTag);
			}
		}
	}
	return	0;
}
int		CMNBody::ReloadAllCFG(const char *cszCFG)
{
	LockThreads();

	size_t	i;
	for(i=0;i<m_vectLogic.size();i++)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL )
		{
			pLogic->Init_CFG_Reload(cszCFG);
		}
	}

	UnlockThreads();
	return	0;
}

// 获得逻辑模块
CMNBody::ILogic *	CMNBody::GetLogic(const char *cszType, int nVer)
{
	for(size_t i=0;i<m_vectLogic.size();i++)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL )
		{
			if( stricmp(pLogic->m_szLogicType, cszType) == 0 )
			{
				if( pLogic->m_nVer<0 || nVer<0 || pLogic->m_nVer==nVer )
				{
					return	pLogic;
				}
				// 版本不对
				(*m_fn_GLogger_WriteFmt)(GLOGGER_ID_ERROR, GLGR_STD_HDR(543,PNGS_INT_RLS)"CMNBody::GetLogic,VER ERROR,%d,%s,%d,%d", i, cszType, nVer, pLogic->m_nVer);
				return	NULL;
			}
		}
	}
	return	NULL;
}
int		CMNBody::GetMultiLogic(const char *cszType, int nVer, n_whcmn::whvector<ILogic *> &vect)
{
	vect.clear();
	for(size_t i=0;i<m_vectLogic.size();i++)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL )
		{
			if( stricmp(pLogic->m_szLogicType, cszType) == 0 )
			{
				if( pLogic->m_nVer<0 || nVer<0 || pLogic->m_nVer==nVer )
				{
					vect.push_back(pLogic);
				}
			}
		}
	}
	return	0;
}
bool	CMNBody::ShouldStop() const
{
	return	m_bShouldStop;
}
void	CMNBody::SetAllLogicToStopMood()
{
	for(size_t i=0;i<m_vectLogic.size();i++)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL )
		{
			PNGSPACKET_2LOGIC_SETMOOD_T	Cmd;
			Cmd.nMood	= ILogic::MOOD_STOPPING;
			CmdOutToLogic_AUTO(pLogic, PNGSPACKET_2LOGIC_SETMOOD, &Cmd, sizeof(Cmd));
		}
	}
}
bool	CMNBody::AreAllLogicsReallyStopped()
{
	for(size_t i=0;i<m_vectLogic.size();i++)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL )
		{
			if( pLogic->GetWorkMood() != ILogic::MOOD_STOPPED )
			{
				return	false;
			}
		}
	}
	return	true;
}
int		CMNBody::DetachAllLogic(bool bDestroy, bool bQuick)
{
	for(int i=(int)m_vectLogic.size()-1;i>=0;i--)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL )
		{
			// 这两步原来是DetachLogic中做的。
			pLogic->Detach(bQuick);
		}
	}
	if( bDestroy )
	{
		for(int i=(int)m_vectLogic.size()-1;i>=0;i--)
		{
			ILogic	*pLogic	= m_vectLogic[i];
			if( pLogic != NULL )
			{
				pLogic->Release();
				pLogic->SelfDestroy();
			}
		}
	}
	return	0;
}
namespace
{
	struct	THRAED_INFO_T
	{
		UDPEvent	*pEvent;
		CMNBody		*pCmn;
		int			nThreadTag;
		whlock		*pLock;
	};
	static void *	_CMNBody_ThreadProc(void *pParam)
	{

		THRAED_INFO_T	info	= *(THRAED_INFO_T *)pParam;
		// 通知StartThreads函数，可以进行下面的操作了
		info.pEvent->AddEvent();
		n_whcmn::whvector<SOCKET>	vectSOCKET;
		vectSOCKET.reserve(16);
		CMNBody			&svr	= *info.pCmn;
		while( !svr.ShouldStop() )
		{
			vectSOCKET.clear();
			svr.GetSockets(vectSOCKET, info.nThreadTag);
			cmn_select_rd_array(vectSOCKET.getbuf(), vectSOCKET.size(), 20);

			info.pLock->lock();

			svr.AllTick(info.nThreadTag);
			svr.AllSureSend(info.nThreadTag);
			// 比较担心的是fileman没有按照多线程调用的方式设计，不知道会不会出问题（所以目前不要在多线程模块中使用fileman）

			info.pLock->unlock();
		}
		return	0;
	}
}
int		CMNBody::StartThreads()
{
	UDPEvent	l_event;
	l_event.Init();
	whhashset<int>	setThread;
	m_bThreadOn	= true;
	// 看看哪些模块的threadid不是0，就启动线程
	for(int i=(int)m_vectLogic.size()-1;i>=0;i--)
	{
		ILogic	*pLogic	= m_vectLogic[i];
		if( pLogic != NULL )
		{
			int	nThreadTag	= pLogic->GetThreadTag();
			if( nThreadTag>0 && setThread.find( nThreadTag ) == setThread.end() )
			{
				MY_THEADINFO_T	&myinfo	= *m_vectThread.push_back();
				myinfo.pLock	= new whlock;

				// 原来没有，创建新的thread
				whtid_t			tid;
				THRAED_INFO_T	info;
				info.pEvent		= &l_event;
				info.pCmn		= this;
				info.nThreadTag	= nThreadTag;
				info.pLock		= myinfo.pLock;
				if( whthread_create(&tid, _CMNBody_ThreadProc, &info)<0 )
				{
					// 不应该这样的
					assert(0);
					return	-1;
				}

				myinfo.tid		= tid;

				// 等待一下有线程已经初始化好了在进行后面的
				cmn_select_rd(l_event.GetSocket(), 1000);
				l_event.ClrAllEvent();
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1030,PNGS_INT_RLS)"StartThread,%s", pLogic->GetLogicType());
			}
		}
	}
	l_event.Release();
	return	0;
}
int		CMNBody::StopThreads()
{
	// 所有的线程应该都看到m_bShouldStop就停止了
	m_bShouldStop	= true;
	// 这里只需要等待所有的线程都结束了
	for(size_t i=0;i<m_vectThread.size();i++)
	{
		whthread_waitend_or_terminate(m_vectThread[i].tid, 10*1000);
		delete	m_vectThread[i].pLock;
	}
	m_bThreadOn	= false;

	return	0;
}
int		CMNBody::LockThreads()
{
	if( !m_bThreadOn )
	{
		return	true;
	}

	for(size_t i=0;i<m_vectThread.size();i++)
	{
		m_vectThread[i].pLock->lock();
	}

	return	true;
}
int		CMNBody::UnlockThreads()
{
	if( !m_bThreadOn )
	{
		return	true;
	}

	for(size_t i=0;i<m_vectThread.size();i++)
	{
		m_vectThread[i].pLock->unlock();
	}

	return	true;
}

int		CMNBody::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	return	0;
}
int		CMNBody::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch( nCmd )
	{
	case	CMD2CMN_SHOULDSTOP:
		m_bShouldStop	= true;
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1091,PNGS_INT_RLS)"I am told to stop,0x%X,%s", pRstAccepter, pRstAccepter->GetLogicType());
		break;
	case	CMD2CMN_STOPNOW:
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1091,PNGS_INT_RLS)"I am told to stop now,0x%X,%s", pRstAccepter, pRstAccepter->GetLogicType());
#ifdef	_DEBUG
		assert(0);
#else
		exit(0);
#endif
		break;
	}
	return	0;
}

// 版本比较，看是否允许
bool	pngs_isverallowed(int nMode, int nBaseVerMain, int nBaseVerSub, int nInputVerMain, int nInputVerSub)
{
	switch( nMode )
	{
		case	PNGS_VERCMP_MODE_GE:
		{
			if( nBaseVerMain <= nInputVerMain && nBaseVerSub <= nInputVerSub )
			{
				return	true;
			}
		}
		break;
		case	PNGS_VERCMP_MODE_EQUAL:
		default:
		{
			if( nBaseVerMain == nInputVerMain && nBaseVerSub == nInputVerSub )
			{
				return	true;
			}
		}
		break;
	}
	return	false;
}
bool	pngs_isverallowed(int nMode, const char * szBaseVer, const char * szInputVer)
{
	switch( nMode )
	{
		case	PNGS_VERCMP_MODE_GE:
		{
			if( strcmp(szBaseVer, szInputVer)<=0 )
			{
				return	true;
			}
		}
		break;
		case	PNGS_VERCMP_MODE_EQUAL:
		default:
		{
			if( strcmp(szBaseVer, szInputVer)==0 )
			{
				return	true;
			}
		}
		break;
	}
	return	false;
}

// 配置CLS和Client的slot的固定参数
int		pngs_config_CLSClient_Slot_Channel(n_whnet::CNL2SlotMan::INFO_T *pInfo, int nInOutNum, bool bIsClient)
{
	// 4个channel
	// 0	safe		双向
	// 1	safe		双向
	// 2	lastsafe	CLS->Client		2005-10-26 修改，为了避免服务器判断指令是否来自于某个通道的麻烦
	// 3	rt			CLS->Client
	for(int i=0;i<4;i++)
	{
		pInfo->channelinfo[i].nType		= CNL2_CHANNELTYPE_SAFE;
		pInfo->channelinfo[i].nInNum	= nInOutNum;
		pInfo->channelinfo[i].nOutNum	= nInOutNum;
		pInfo->channelinfo[i].nProp		= CNL2CHANNELINFO_T::PROP_CANSEND | CNL2CHANNELINFO_T::PROP_CANRECV;
	}
	pInfo->channelinfo[2].nType			= CNL2_CHANNELTYPE_LASTSAFE;
	pInfo->channelinfo[3].nType			= CNL2_CHANNELTYPE_RTDATA;
	if( bIsClient )
	{
		// 客户端不能发只能收
		pInfo->channelinfo[2].nProp		= CNL2CHANNELINFO_T::PROP_CANRECV;
		pInfo->channelinfo[3].nProp		= CNL2CHANNELINFO_T::PROP_CANRECV;
	}
	else
	{
		// 服务器端不许收只能发
		pInfo->channelinfo[2].nProp		= CNL2CHANNELINFO_T::PROP_CANSEND;
		pInfo->channelinfo[3].nProp		= CNL2CHANNELINFO_T::PROP_CANSEND;
	}
	return	0;
}

}		// EOF namespace n_pngs
