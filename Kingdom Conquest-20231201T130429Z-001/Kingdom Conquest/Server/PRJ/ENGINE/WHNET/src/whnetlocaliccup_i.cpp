// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetlocalhiccup_i.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ����UDP�ľ������ڵ�Ӧ�ó�����Ȩ���е�ʵ�ֲ������ļ�
//              : hiccup�Ǵ��õ���˼��ֻ������뵽����ʾ���ô����:P
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
	// ����һ��thread
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

// ��ʼ��CNL2�ĳ�ʼ����Ϣ�ṹ��client��server�����Ĳ��֣�
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
	// �ֽ��ַ
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
	// �������Ӷ���
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
	// ������
	m_pICryptFactory	= ICryptFactory_DHKeyAndDES_Create();
	if( !m_pICryptFactory )
	{
		return	-13;
	}
	m_pSlotMan->SetICryptFactory(m_pICryptFactory);
	// ��ʼ����
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
	// �ȵ�����״̬��Trans��Ȼ���ٵ�����״̬�Ŀ�ʼ
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
	// �ӵ�ַ������ȡ����һ��
	size_t	nSize		= 0;
	char	*pszAddr	= (char *)m_queueAddr.OutFree(&nSize);
	if( !pszAddr )
	{
		// û�ˡ�˵������ʧ���ˡ�
		// ״̬��Ϊֹͣ
		SetStatus(STATUS_T::STOPPED);
		return;
	}
	// �ر���һ�����ӣ�����еĻ���
	if( m_nSlot>0 )
	{
		m_pSlotMan->Close(m_nSlot);
	}
	// �������ӵ�ַ
	m_nSlot	= m_pSlotMan->Connect(pszAddr);
	if( m_nSlot<0 )
	{
		// Ӧ����������������
		// ״̬��Ϊֹͣ
		SetStatus(STATUS_T::STOPPED);
		return;
	}
	// �������tick�еȴ����ӽ���Ϳ�����
}
void	whnetlocalhiccup_client_I::_SF_Work_CONNECTING()
{
	// �������״̬
	CNL2SlotMan::CONTROL_T	*pCOCmd;
	size_t	nSize;
	while( m_pSlotMan->ControlOut(&pCOCmd, &nSize)==0 )
	{
		switch( pCOCmd->nCmd )
		{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CONNECTED:
			{
				// ���ӳɹ���������Կ(���У��ʧ�ܣ����Ӿͻ�رգ�У��ɹ�����ᷢ����ӭ�ִ�)
				m_pSlotMan->Send(m_nSlot, m_info.szPass, strlen(m_info.szPass)+1, 0);
			}
			break;
		default:
			{
				// �����κ������Ӧ��������ʧ���ˣ�������û����Ҳ������У��ʧ�ܣ�
				// ������һ����ַ
				SetStatus(STATUS_T::CONNECTING);
				return;
			}
			break;
		}
	}
	// �ȴ�������������Ϣ
	// ����ͻ��˷�����ָ��
	int		nSlot, nChannel;
	char	*pszCmd;
	while( m_pSlotMan->Recv(&nSlot, &nChannel, (void **)&pszCmd, &nSize)==0 )
	{
		if( strcmp(CSZ_SUCCEED, pszCmd)==0 )
		{
			// У��ɹ�
			// ת��Ϊ����״̬
			SetStatus(STATUS_T::WORKING);
			break;
		}
	}
}
void	whnetlocalhiccup_client_I::_SF_Begin_WORKING()
{
	// ֪ͨ�ϲ�˵���ӳɹ�
	(*m_info.pCallBack)(CSZ_SUCCEED, m_info.pCBParam);
}
void	whnetlocalhiccup_client_I::_SF_Work_WORKING()
{
	// ����յ���Ϣ��֪ͨ�ϲ�ص�
	int		nSlot, nChannel;
	size_t	nSize;
	char	*pszCmd;
	while( m_pSlotMan->Recv(&nSlot, &nChannel, (void **)&pszCmd, &nSize)==0 )
	{
		// ���͸��ϲ�
		(*m_info.pCallBack)(pszCmd, m_info.pCBParam);
	}
}
void	whnetlocalhiccup_client_I::_SF_Begin_STOPPED()
{
	// ֪ͨ�ϲ�˵����ʧ��
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
	// �������Ӷ���
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
	// ������
	m_pICryptFactory	= ICryptFactory_DHKeyAndDES_Create();
	if( !m_pICryptFactory )
	{
		return	-13;
	}
	m_pSlotMan->SetICryptFactory(m_pICryptFactory);
	// ��ʼ����
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

	// �ȴ�����
	// �������״̬
	CNL2SlotMan::CONTROL_T	*pCOCmd;
	size_t	nSize;
	while( m_pSlotMan->ControlOut(&pCOCmd, &nSize)==0 )
	{
		switch( pCOCmd->nCmd )
		{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_ACCEPTED:
			{
				// �������ӳɹ�
				// ������Ӧ��Client����
				ClientUnit	*pClient;
				int	nID		= m_Clients.AllocUnit(pClient);
				if( nID>0 )
				{
					pClient->clear();
					pClient->nSlot	= pCOCmd->data[0];
					// ������
					m_pSlotMan->SetExtLong(pClient->nSlot, nID);
					// ����һ����ʱ�¼����ȴ��û�����pass
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
					// �Ѿ����䲻���ˣ��ر��������
					m_pSlotMan->Close(pCOCmd->data[0]);
				}
			}
			break;
		default:
			{
				// �����κ������Ӧ���������ж���
				// ɾ�����Ӷ�Ӧ��Client����
				CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T	*pCloseReason	= (CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T *)pCOCmd->data;
				m_Clients.FreeUnit(pCloseReason->nSlotExt);
			}
			break;
		}
	}
	// ���û���������Ϣ
	int		nSlot, nChannel;
	char	*pszCmd;
	while( m_pSlotMan->Recv(&nSlot, &nChannel, (void **)&pszCmd, &nSize)==0 )
	{
		// ��֤�ִ����ܳ���һ������
		if( wh_isstrtoolong(pszCmd, 1024) )
		{
			// �رո�����
			m_pSlotMan->Close(nSlot);
			continue;
		}
		// ͨ��slot���client
		ClientUnit	*pClient	= GetClientBySlot(nSlot);
		if( !pClient )
		{
			continue;
		}
		// ����û�û��У�飬���ж��Ƿ���У������
		if( (pClient->nProp & ClientUnit::PROP_AUTHOK) == 0 )
		{
			if( strcmp(m_info.szPass, pszCmd)==0 )
			{
				// У��ɹ�
				// ���ñ��
				pClient->nProp	|= ClientUnit::PROP_AUTHOK;
				// ���û����سɹ�
				m_pSlotMan->Send(nSlot, CSZ_SUCCEED, strlen(CSZ_SUCCEED)+1, 0);
			}
			else
			{
				// У��ʧ�ܣ��ر�����
				m_pSlotMan->Close(nSlot);
			}
		}
		// ����
		else
		{
			// ����ٽ�һ����ɰɡ����������û�����һЩ��չ���ִ�ָ�
		}
	}

	m_pSlotMan->TickLogicAndSend();

	return	0;
}
void	whnetlocalhiccup_server_I::TEDeal_ClientPass_TimeOut(TQUNIT_T *pTQUnit)
{
	// ����Client�Ƿ���
	ClientUnit	*pClient	= m_Clients.getptr(pTQUnit->un.nClientID);
	if( !pClient )
	{
		// �û��Ѿ������ˣ����ˣ���������
		return;
	}
	// �ж��û��Ƿ��Ѿ���Ȩ�ɹ�
	if( (pClient->nProp & ClientUnit::PROP_AUTHOK) == 0 )
	{
		// û����Ȩ�ɹ�
		// �ر�����
		m_pSlotMan->Close(pClient->nSlot);
		return;
	}
	// �ɹ��ˣ��ͽ����ˣ���������������
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
