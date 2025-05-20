// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetlocalhiccup_i.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 基于UDP的局域网内的应用程序授权运行的实现部分主文件
//              : hiccup是打嗝的意思，只是随便想到这个词就这么用了:P
// CreationDate : 2006-06-29
// ChangeLog    : 

#include "../inc/whnetlocalhiccup_i.h"
#include "../inc/whnetudpGLogger.h"
#include <WHCMN/inc/whthread.h>

using namespace n_whnet;

static const char *	CSZ_SUCCEED	= "Succeed";
static const char *	CSZ_FAIL	= "Fail";
static const char *	CSZ_DFTPASS	= "pass:de facto";
const int			CNPORT		= 1080;

whnetlocalhiccup_client *	whnetlocalhiccup_client::Create()
{
	return	new whnetlocalhiccup_client_I;
}
static bool	g_bStop		= false;
static whtid_t	g_tid	= 0;
static void *	_my_do_it_thread(void *ptr)
{
	whnetlocalhiccup_client_I	lhc;
	int	rst	= lhc.Init((whnetlocalhiccup_client::INFO_T*)ptr);
	if( rst<0 )
	{
		return	NULL;
	}

	while( !g_bStop )
	{
		if( cmn_select_rd(lhc.GetSocket(), 50)<0 )
		{
			break;
		}
		lhc.Tick();
		GLOGGER2_TICK();
	}

	lhc.Release();
	return	NULL;
}
int		whnetlocalhiccup_client::DoItInThread(INFO_T *pInfo)
{
	// 启动一个thread
	return	whthread_create(&g_tid, _my_do_it_thread, pInfo);
}
int		whnetlocalhiccup_client::StopTheThread()
{
	g_bStop	= true;
	whthread_waitend_or_terminate(g_tid, 5000);
	return	0;
}
whnetlocalhiccup_server *	whnetlocalhiccup_server::Create()
{
	return	new whnetlocalhiccup_server_I;
}

////////////////////////////////////////////////////////////////////
// whnetlocalhiccup_client::INFO_T
////////////////////////////////////////////////////////////////////
whnetlocalhiccup_client::INFO_T::INFO_T()
: pCallBack(NULL)
, pCBParam(NULL)
{
	sprintf(szDst, "smile:%d, ps3:%d, ps1:%d, xman:%d, weihua:%d", CNPORT, CNPORT, CNPORT, CNPORT, CNPORT);
	strcpy(szPass, CSZ_DFTPASS);
}

////////////////////////////////////////////////////////////////////
// whnetlocalhiccup_server::INFO_T
////////////////////////////////////////////////////////////////////
whnetlocalhiccup_server::INFO_T::INFO_T()
: nMaxConnection(100)
, nPassTimeOut(2000)
{
	sprintf(szBindAddr, ":%d", CNPORT);
	strcpy(szPass, CSZ_DFTPASS);
}

// 初始化CNL2的初始化信息结构（client和server公共的部分）
static void	initcnl2slotinfo(CNL2SlotMan::INFO_T *pInfo)
{
	pInfo->nConnectTimeout			= 5000;
	pInfo->nDropTimeout				= 8000;
	pInfo->nCryptorType1			= 0;
	pInfo->nCryptorType2			= 10;
	pInfo->channelinfo[0].nType		= CNL2_CHANNELTYPE_SAFE;
	pInfo->channelinfo[0].nInNum	= 16;
	pInfo->channelinfo[0].nOutNum	= 16;
	pInfo->channelinfo[0].nProp		= 3;
}

////////////////////////////////////////////////////////////////////
// whnetlocalhiccup_client_I
////////////////////////////////////////////////////////////////////
whnetlocalhiccup_client_I::whnetlocalhiccup_client_I()
: m_pSlotMan(NULL)
, m_pICryptFactory(NULL)
, m_nSlot(0)
{
	StatusObjInit();
}
whnetlocalhiccup_client_I::~whnetlocalhiccup_client_I()
{
}
int	whnetlocalhiccup_client_I::Init(INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	// 分解地址
	if( m_queueAddr.Init(4096)<0 )
	{
		return	-1;
	}
	int		nOffset	= 0;
	char	szOneAddr[256];
	while( wh_strsplit(&nOffset, "s", m_info.szDst, ",", szOneAddr)==1 )
	{
		m_queueAddr.In(szOneAddr, strlen(szOneAddr)+1);
	}
	// 创建连接对象
	m_pSlotMan	= CNL2SlotMan::Create();
	if( !m_pSlotMan )
	{
		return	-11;
	}
	CNL2SlotMan::INFO_T				sminfo;
	initcnl2slotinfo(&sminfo);
	if( m_pSlotMan->Init(&sminfo, NULL)<0 )
	{
		return	-12;
	}
	// 加密器
	m_pICryptFactory	= ICryptFactory_DHKeyAndDES_Create();
	if( !m_pICryptFactory )
	{
		return	-13;
	}
	m_pSlotMan->SetICryptFactory(m_pICryptFactory);
	// 开始工作
	SetStatus(STATUS_T::CONNECTING);
	return	0;
}
int	whnetlocalhiccup_client_I::Release()
{
	m_pSlotMan->Close(m_nSlot);
	for(int i=0;i<10;i++)
	{
		wh_sleep(50);
		Tick();
	}
	WHSafeSelfDestroy( m_pSlotMan );
	WHSafeSelfDestroy( m_pICryptFactory );
	m_queueAddr.Release();
	return	0;
}
int	whnetlocalhiccup_client_I::Tick()
{
	m_pSlotMan->TickRecv();
	(this->*m_pStatus->pWork)();
	m_pSlotMan->TickLogicAndSend();
	return	0;
}
void	whnetlocalhiccup_client_I::StatusObjInit()
{
	for(int i=STATUS_T::NOTHING; i<STATUS_T::MAXNUM; ++i)
	{
		m_aSTATUS[i].nStatus	= (STATUS_T::TYPE_T)i;
		m_aSTATUS[i].pBegin		= &whnetlocalhiccup_client_I::_SF_CMN_T_DoNothing;
		m_aSTATUS[i].pWork		= &whnetlocalhiccup_client_I::_SF_CMN_T_DoNothing;
		m_aSTATUS[i].pTrans		= &whnetlocalhiccup_client_I::_SF_TRANS_T_DoNothing;
	}
	m_pStatus		= &m_aSTATUS[STATUS_T::NOTHING];
	STATUS_T		*pStatus;
	pStatus			= &m_aSTATUS[STATUS_T::CONNECTING];
	pStatus->pBegin	= &whnetlocalhiccup_client_I::_SF_Begin_CONNECTING;
	pStatus->pWork	= &whnetlocalhiccup_client_I::_SF_Work_CONNECTING;
	pStatus			= &m_aSTATUS[STATUS_T::WORKING];
	pStatus->pBegin	= &whnetlocalhiccup_client_I::_SF_Begin_WORKING;
	pStatus->pWork	= &whnetlocalhiccup_client_I::_SF_Work_WORKING;
	pStatus			= &m_aSTATUS[STATUS_T::STOPPED];
	pStatus->pBegin	= &whnetlocalhiccup_client_I::_SF_Begin_STOPPED;
}
void	whnetlocalhiccup_client_I::SetStatus(STATUS_T::TYPE_T nType)
{
	assert(nType>=0 && nType<STATUS_T::MAXNUM);
	// 先调用老状态的Trans，然后再调用新状态的开始
	(this->*m_pStatus->pTrans)(&m_aSTATUS[nType]);
	m_pStatus	= &m_aSTATUS[nType];
	(this->*m_pStatus->pBegin)();
}
void	whnetlocalhiccup_client_I::_SF_CMN_T_DoNothing()
{
}
void	whnetlocalhiccup_client_I::_SF_TRANS_T_DoNothing(STATUS_T *pNextStatus)
{
}
void	whnetlocalhiccup_client_I::_SF_Begin_CONNECTING()
{
	// 从地址队列中取出第一个
	size_t	nSize		= 0;
	char	*pszAddr	= (char *)m_queueAddr.OutFree(&nSize);
	if( !pszAddr )
	{
		// 没了。说明连接失败了。
		// 状态变为停止
		SetStatus(STATUS_T::STOPPED);
		return;
	}
	// 关闭上一个连接（如果有的话）
	if( m_nSlot>0 )
	{
		m_pSlotMan->Close(m_nSlot);
	}
	// 尝试连接地址
	m_nSlot	= m_pSlotMan->Connect(pszAddr);
	if( m_nSlot<0 )
	{
		// 应该是网络有问题了
		// 状态变为停止
		SetStatus(STATUS_T::STOPPED);
		return;
	}
	// 后面就在tick中等待连接结果就可以了
}
void	whnetlocalhiccup_client_I::_SF_Work_CONNECTING()
{
	// 检查连接状态
	CNL2SlotMan::CONTROL_T	*pCOCmd;
	size_t	nSize;
	while( m_pSlotMan->ControlOut(&pCOCmd, &nSize)==0 )
	{
		switch( pCOCmd->nCmd )
		{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CONNECTED:
			{
				// 连接成功，发送密钥(如果校验失败，连接就会关闭；校验成功，则会发来欢迎字串)
				m_pSlotMan->Send(m_nSlot, m_info.szPass, strlen(m_info.szPass)+1, 0);
			}
			break;
		default:
			{
				// 其他任何情况都应该是连接失败了（可以是没连上也可能是校验失败）
				// 尝试下一个地址
				SetStatus(STATUS_T::CONNECTING);
				return;
			}
			break;
		}
	}
	// 等待服务器发来消息
	// 处理客户端发来的指令
	int		nSlot, nChannel;
	char	*pszCmd;
	while( m_pSlotMan->Recv(&nSlot, &nChannel, (void **)&pszCmd, &nSize)==0 )
	{
		if( strcmp(CSZ_SUCCEED, pszCmd)==0 )
		{
			// 校验成功
			// 转换为工作状态
			SetStatus(STATUS_T::WORKING);
			break;
		}
	}
}
void	whnetlocalhiccup_client_I::_SF_Begin_WORKING()
{
	// 通知上层说连接成功
	(*m_info.pCallBack)(CSZ_SUCCEED, m_info.pCBParam);
}
void	whnetlocalhiccup_client_I::_SF_Work_WORKING()
{
	// 如果收到消息则通知上层回调
	int		nSlot, nChannel;
	size_t	nSize;
	char	*pszCmd;
	while( m_pSlotMan->Recv(&nSlot, &nChannel, (void **)&pszCmd, &nSize)==0 )
	{
		// 发送给上层
		(*m_info.pCallBack)(pszCmd, m_info.pCBParam);
	}
}
void	whnetlocalhiccup_client_I::_SF_Begin_STOPPED()
{
	// 通知上层说连接失败
	(*m_info.pCallBack)(CSZ_FAIL, m_info.pCBParam);
}

////////////////////////////////////////////////////////////////////
// whnetlocalhiccup_server_I
////////////////////////////////////////////////////////////////////
whnetlocalhiccup_server_I::whnetlocalhiccup_server_I()
: m_pSlotMan(NULL)
, m_pICryptFactory(NULL)
{
}
whnetlocalhiccup_server_I::~whnetlocalhiccup_server_I()
{
}
int	whnetlocalhiccup_server_I::Init(INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	if( m_Clients.Init(m_info.nMaxConnection)<0 )
	{
		return	-1;
	}
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= 100;
	if( m_TQ.Init(&tqinfo)<0 )
	{
		return	-2;
	}
	// 创建连接对象
	m_pSlotMan	= CNL2SlotMan::Create();
	if( !m_pSlotMan )
	{
		return	-11;
	}
	CNL2SlotMan::INFO_T		sminfo;
	initcnl2slotinfo(&sminfo);
	strcpy(sminfo.szBindAddr, m_info.szBindAddr);
	sminfo.nMaxConnection	= m_info.nMaxConnection;
	if( m_pSlotMan->Init(&sminfo, NULL)<0 )
	{
		return	-12;
	}
	m_pSlotMan->Listen(true);
	// 加密器
	m_pICryptFactory	= ICryptFactory_DHKeyAndDES_Create();
	if( !m_pICryptFactory )
	{
		return	-13;
	}
	m_pSlotMan->SetICryptFactory(m_pICryptFactory);
	// 开始工作
	m_tickNow	= wh_gettickcount();
	return	0;
}
int	whnetlocalhiccup_server_I::Release()
{
	WHSafeSelfDestroy( m_pSlotMan );
	WHSafeSelfDestroy( m_pICryptFactory );
	m_Clients.Release();
	return	0;
}
int	whnetlocalhiccup_server_I::Tick()
{
	m_tickNow	= wh_gettickcount();

	m_pSlotMan->TickRecv();

	// 等待连入
	// 检查连接状态
	CNL2SlotMan::CONTROL_T	*pCOCmd;
	size_t	nSize;
	while( m_pSlotMan->ControlOut(&pCOCmd, &nSize)==0 )
	{
		switch( pCOCmd->nCmd )
		{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_ACCEPTED:
			{
				// 接受连接成功
				// 创建相应的Client对象
				ClientUnit	*pClient;
				int	nID		= m_Clients.AllocUnit(pClient);
				if( nID>0 )
				{
					pClient->clear();
					pClient->nSlot	= pCOCmd->data[0];
					// 并关联
					m_pSlotMan->SetExtLong(pClient->nSlot, nID);
					// 发起一个超时事件，等待用户发来pass
					whtimequeue::ID_T	teid;
					TQUNIT_T			*pTQUnit;
					int			rst		= m_TQ.AddGetRef(m_tickNow+m_info.nPassTimeOut, (void **)&pTQUnit, &teid);
					if( rst==0 )
					{
						pTQUnit->tefunc			= &whnetlocalhiccup_server_I::TEDeal_ClientPass_TimeOut;
						pTQUnit->un.nClientID	= nID;
					}
				}
				else
				{
					// 已经分配不了了，关闭这个连接
					m_pSlotMan->Close(pCOCmd->data[0]);
				}
			}
			break;
		default:
			{
				// 其他任何情况都应该是连接中断了
				// 删除连接对应的Client对象
				CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T	*pCloseReason	= (CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T *)pCOCmd->data;
				m_Clients.FreeUnit(pCloseReason->nSlotExt);
			}
			break;
		}
	}
	// 看用户发来的消息
	int		nSlot, nChannel;
	char	*pszCmd;
	while( m_pSlotMan->Recv(&nSlot, &nChannel, (void **)&pszCmd, &nSize)==0 )
	{
		// 保证字串不能超过一定长度
		if( wh_isstrtoolong(pszCmd, 1024) )
		{
			// 关闭该连接
			m_pSlotMan->Close(nSlot);
			continue;
		}
		// 通过slot获得client
		ClientUnit	*pClient	= GetClientBySlot(nSlot);
		if( !pClient )
		{
			continue;
		}
		// 如果用户没有校验，则判断是否是校验密码
		if( (pClient->nProp & ClientUnit::PROP_AUTHOK) == 0 )
		{
			if( strcmp(m_info.szPass, pszCmd)==0 )
			{
				// 校验成功
				// 做好标记
				pClient->nProp	|= ClientUnit::PROP_AUTHOK;
				// 向用户返回成功
				m_pSlotMan->Send(nSlot, CSZ_SUCCEED, strlen(CSZ_SUCCEED)+1, 0);
			}
			else
			{
				// 校验失败，关闭连接
				m_pSlotMan->Close(nSlot);
			}
		}
		// 否则
		else
		{
			// 今后再进一步完成吧。可以允许用户发来一些扩展的字串指令。
		}
	}

	m_pSlotMan->TickLogicAndSend();

	return	0;
}
void	whnetlocalhiccup_server_I::TEDeal_ClientPass_TimeOut(TQUNIT_T *pTQUnit)
{
	// 看看Client是否还在
	ClientUnit	*pClient	= m_Clients.getptr(pTQUnit->un.nClientID);
	if( !pClient )
	{
		// 用户已经断线了，完了，不存在了
		return;
	}
	// 判断用户是否已经授权成功
	if( (pClient->nProp & ClientUnit::PROP_AUTHOK) == 0 )
	{
		// 没有授权成功
		// 关闭连接
		m_pSlotMan->Close(pClient->nSlot);
		return;
	}
	// 成功了，就结束了，可以正常工作了
}
whnetlocalhiccup_server_I::ClientUnit *	whnetlocalhiccup_server_I::GetClientBySlot(int nSlot)
{
	long	nClientID;
	if( m_pSlotMan->GetExtLong(nSlot, &nClientID)<0 )
	{
		return	NULL;
	}
	return	m_Clients.getptr((int)nClientID);
}
