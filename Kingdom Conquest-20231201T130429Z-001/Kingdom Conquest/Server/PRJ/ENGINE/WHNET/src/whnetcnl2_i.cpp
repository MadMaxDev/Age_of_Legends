// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CNL2���ڲ�ʵ��
// CreationDate : 2005-04-17
// ChangeLog    : 2005-07-18 SlotChannelConfirm_LastSafe��Ҳ������RTT����
//              : 2005-08-03 ������SlotMan��CloseAll
//              : 2006-05-22 ��CNL2SlotMan_I_UDP::SetICryptFactory�������˳�ʼ��������ӵĴ��롣ԭ���������Ϳͻ��˲�����pubdata������ͬ�ģ��Ǻǡ�
//              : 2007-08-09 ��SlotChannelConfirm_Safe���������յ�����İ��Ļ�Ӧ����������ǰ��İ��Ļ��ơ�
//              : 2007-12-29 �����AddSlot��RemoveSlot����־����ҪΪ�˲�CAAFS����slot�޹���ʧ������

#include "../inc/whnetcnl2_i.h"
#include <WHCMN/inc/whbits.h>

using namespace	n_whnet;

////////////////////////////////////////////////////////////////////
// CNL2SLOT_I::CHANNEL_T
////////////////////////////////////////////////////////////////////
void	CNL2SLOT_I::CHANNEL_T::INUNIT_T::clear(CNL2SlotMan_I_UDP *pMan)
{
	pMan->FreeAndClearData(pData);
}
void	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T::clear(CNL2SlotMan_I_UDP *pMan)
{
	pMan->FreeAndClearData(pCmd);
	nTimeEventID.quit();								// ǿ���˳�ʱ���¼�����
	nTimeEventID.clear();								// �������һ���Ƿ�Ҫȡ��
	nRSCount	= 1;									// ��ס���Ĭ�ϱ�����1����Ϊ��һ��resend��Ҫ��2*nRTT֮��
}

CNL2SLOT_I::CHANNEL_T::CHANNEL_T()
: pSlotMan(NULL)
, pSlot(NULL)
, nChannel(0), nProp(CNL2CHANNELINFO_T::PROP_CANSEND|CNL2CHANNELINFO_T::PROP_CANRECV)
, nType(CNL2_CHANNELTYPE_NOTHING)
, nInIdx0(0), nOutIdx0(0)
, nInIdxConfirm(0), nOutIdxConfirm(0)
, pInQueue(NULL), pOutQueue(NULL)
, pICryptorSend(NULL), pICryptorRecv(NULL)
{
	nodeHavingData.data			= this;
}
CNL2SLOT_I::CHANNEL_T::~CHANNEL_T()
{
	Release();
}
int	CNL2SLOT_I::CHANNEL_T::Init(CNL2CHANNELINFO_T *pInfo)
{
	assert(!pOutQueue && !pInQueue);

	nType		= pInfo->nType;
	nProp		= pInfo->nProp;

	switch( nType )
	{
		case	CNL2_CHANNELTYPE_NOTHING:
			// �൱��û�г�ʼ��
		break;
		default:
		{
			if( pInfo->nInNum>CNL2_MAX_INOUTQUEUESIZE
			||  pInfo->nOutNum>CNL2_MAX_INOUTQUEUESIZE
			)
			{
				// ����������ô��Ļ���
				assert(0);
				return	-1;
			}
			if( nType == CNL2_CHANNELTYPE_LASTSAFE )
			{
				// ������͵�Ϊ׼����ô��ֻ��Ҫһ����Ԫ������
				pInfo->nInNum	= 1;
				pInfo->nOutNum	= 1;
			}
			pInQueue	= new whvectorqueue<INUNIT_T>;
			if( pInQueue->Init(pInfo->nInNum)<0 )
			{
				return	-2;
			}
			if( nType != CNL2_CHANNELTYPE_RTDATA )
			{
				pOutQueue	= new whvectorqueue<OUTUNIT_T>;
				if( pOutQueue->Init(pInfo->nOutNum)<0 )
				{
					return	-3;
				}
			}
		}
		break;
	}

	return	0;
}
int	CNL2SLOT_I::CHANNEL_T::Release()
{
	if(pOutQueue)
	{
		// ɾ��ÿ����Ԫ����Ķ����ڴ�
		pOutQueue->BeginGet();
		OUTUNIT_T	*pOutUnit;
		while( (pOutUnit=pOutQueue->GetNext()) != NULL )
		{
			pOutUnit->clear(pSlotMan);
		}
		// ɾ������
		pOutQueue->Release();
		delete	pOutQueue;
		pOutQueue	= NULL;
	}
	if(pInQueue)
	{
		// ɾ��ÿ����Ԫ����Ķ����ڴ�
		pInQueue->BeginGet();
		INUNIT_T	*pInUnit;
		while( (pInUnit=pInQueue->GetNext()) != NULL )
		{
			pInUnit->clear(pSlotMan);
		}
		// ɾ������
		pInQueue->Release();
		delete	pInQueue;
		pInQueue	= NULL;
	}
	nodeHavingData.leave();
	nodeHavingDataToSend.leave();

	WHSafeSelfDestroy(pICryptorSend);
	WHSafeSelfDestroy(pICryptorRecv);

	// Ӧ����channel���ͷ�֮ǰ�������е�����
	whDList<CMDPACKUNIT_T>::node	*pNode	= dlistCmdPack.begin();
	while( pNode!=dlistCmdPack.end() )
	{
		pSlotMan->FreeData(pNode->data.pCmd);
		pNode->leave();
		pSlotMan->FreeData(pNode);
		// ��һ��
		pNode	= dlistCmdPack.begin();
	}

	nChannel	= 0;
	nType		= 0;
	nInIdx0		= 0;
	nOutIdx0	= 0;
	nInIdxConfirm	= 0;
	nOutIdxConfirm	= 0;

	return	0;
}
static void	_vn_set(unsigned short nCount, void *&pData, size_t &nSize)
{
	int	nAddSize	= whbit_vn1_set(pData, nCount);
	nSize			+= nAddSize;
	pData			= wh_getoffsetaddr(pData, nAddSize);
}
size_t	CNL2SLOT_I::CHANNEL_T::MakeInQueueConfirm_Safe(void *pData)
{
	CNL2_CMD_DATA_CONFIRM_T::CONFIRM_T	*pConfirm	= (CNL2_CMD_DATA_CONFIRM_T::CONFIRM_T *)pData;
	pConfirm->nInIdxConfirm	= nInIdxConfirm;
	pConfirm->nInIdx0		= nInIdx0;
	size_t	nSize			= sizeof(*pConfirm);
	// ָ�����
	pData					= wh_getptrnexttoptr(pConfirm);
	// ��nInIdxConfirm��ʼ����InQueue��д������
	int	nStart				= idxdiff(nInIdxConfirm, nInIdx0);
	pInQueue->BeginGet(nStart);
	INUNIT_T				*pInUnit;
	bool					bGot	= false;
	unsigned short			nCount	= 0;
	// �����治��Ҫ�ж�bGot==true�εĸ����Ƿ񳬹�MAXSECTNUM����Ϊ�Է��յ����Զ����ߵ�
	while( (pInUnit=pInQueue->GetNext()) != NULL )
	{
		if( bGot == (pInUnit->pData != NULL) )
		{
			// ˵������һ����״��һ��
			nCount			++;
		}
		else
		{
			// ״����ת�����޵��л���е���
			_vn_set(nCount, pData, nSize);
			bGot			= !bGot;
			nCount			= 1;
		}
	}
	if( nCount>0 )
	{
		// ���һ��(���nCount��0�����һ����got�ģ����������һ�������߼���д����)
		assert( bGot );
		_vn_set(nCount, pData, nSize);
	}
	// 0��β
	_vn_set(0, pData, nSize);
	return	nSize;
}
size_t	CNL2SLOT_I::CHANNEL_T::MakeInQueueConfirm_LastSafe(void *pData)
{
	CNL2_CMD_DATA_CONFIRM_T::CONFIRM_T	*pConfirm	= (CNL2_CMD_DATA_CONFIRM_T::CONFIRM_T *)pData;
	pConfirm->nInIdxConfirm	= nInIdxConfirm;
	pConfirm->nInIdx0		= nInIdx0;
	size_t	nSize			= sizeof(*pConfirm);
	// ָ�����
	pData					= wh_getptrnexttoptr(pConfirm);
	// 0��β
	_vn_set(0, pData, nSize);
	return	nSize;
}
void	CNL2SLOT_I::CHANNEL_T::AddCmdPackUnit(whDList<CMDPACKUNIT_T>::node *pNode)
{
	dlistCmdPack.AddToTail(pNode);
	nodeHavingDataToSend.leave();
	pSlotMan->m_dlistChannelHaveDataToSend.AddToTail(&nodeHavingDataToSend);
}


////////////////////////////////////////////////////////////////////
// CNL2SLOT_I
////////////////////////////////////////////////////////////////////
CNL2SLOT_I::CNL2SLOT_I()
: bClosePassive(false)
, nCloseCount(0)
, nCryptorType(0)
, nPeerNotConfirmedChannelMask(0)
, nNoopCount(0)
, bNoopMustBeSend(false)
, nPeerNoopSendTime(0)
, nCloseExtDataSend(0)
, nCloseExtDataRecv(0)
, pIKeyExchanger(NULL)
, nExt(0)
, pSlotMan(NULL)
{
	for(int i=0;i<DLISTIDX_MAX;i++)
	{
		dlistNode[i].data		= this;
	}
}
void	CNL2SLOT_I::clear()
{
	slotinfo.clear();
	int	i;
	// �Զ��б�
	for(i=0;i<DLISTIDX_MAX;i++)
	{
		dlistNode[i].leave();
	}
	// ʱ���¼�
	ClearAllTE();
	//
	for(i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		channel[i].Release();
	}
	// �ͷ��ڴ�
	vectExtSlotData.destroy();
	nCloseExtDataSend				= 0;
	nCloseExtDataRecv				= 0;
	WHSafeSelfDestroy( pIKeyExchanger );
	nExt							= 0;

	bClosePassive					= false;
	bIsClient						= false;
	nCloseCount						= 0;
	nCryptorType					= 0;
	nPeerNotConfirmedChannelMask	= 0;
	nNoopCount						= 0;
	bNoopMustBeSend					= false;
	nPeerNoopSendTime				= wh_gettickcount();
}
void	CNL2SLOT_I::ClearAllTE()
{
	for(int i=0;i<CNL2SLOT_I::TE_IDX_MAX;i++)
	{
		teid[i].quit();
	}
}
void	CNL2SLOT_I::ClearTE(int nIdx)
{
	teid[nIdx].quit();
}
void	CNL2SLOT_I::ClearChannelAllTE()
{
	for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		CNL2SLOT_I::CHANNEL_T	&chn	= channel[i];
		if( chn.pOutQueue )
		{
			chn.pOutQueue->BeginGet();
			CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit;
			while( (pOutUnit=chn.pOutQueue->GetNext()) != NULL )
			{
				pOutUnit->nTimeEventID.quit();
			}
		}
	}
}
void	CNL2SLOT_I::SetSlotMan(CNL2SlotMan_I_UDP *pMan)
{
	pSlotMan	= pMan;
	for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		CHANNEL_T	&c	= channel[i];
		c.pSlot		= this;
		c.nodeHavingDataToSend.data.nSlot		= slotinfo.nSlot;
		c.nodeHavingDataToSend.data.nChannel	= i;
		c.pSlotMan	= pSlotMan;
	}
}

////////////////////////////////////////////////////////////////////
// CNL2SlotMan_I_UDP
////////////////////////////////////////////////////////////////////
CNL2SlotMan_I_UDP::CNL2SlotMan_I_UDP()
: m_nConnectRSTimeout(0), m_nNoopTimeout(0), m_nMightDropTimeout(0)
, m_sock(INVALID_SOCKET), m_bListen(false)
, m_pCmd(NULL), m_pCmd1(NULL), m_pCmdData(NULL)
, m_nCmdSize(0)
, m_pData(NULL), m_nDataSize(0)
, m_pSlot(NULL)
, m_pChannel(NULL)
, m_bUseAuth(false)
, m_tickNow(0)
, m_pICryptFactory(NULL)
, m_nMyAddrSum(0)
, m_nLastSwitchAddrTime(0)
{
	memset(m_TickRecv_CMD_Others_DATA_Deal, 0, sizeof(m_TickRecv_CMD_Others_DATA_Deal));
	memset(m_Send_DATA, 0, sizeof(m_Send_DATA));
	memset(m_SCC, 0, sizeof(m_SCC));
}
CNL2SlotMan_I_UDP::~CNL2SlotMan_I_UDP()
{
	Release();
}
int		CNL2SlotMan_I_UDP::GetMaxSingleDataSize()
{
	return	CNL2_MAX_SINGLEDATASIZE;
}
void	CNL2SlotMan_I_UDP::SelfDestroy()
{
	delete	this;
}
void	CNL2SlotMan_I_UDP::SetUseAuth()
{
	m_bUseAuth	= true;
}
// biggest rst err:	-7
int		CNL2SlotMan_I_UDP::Init(CNL2SlotMan::INFO_T *pInfo, struct sockaddr_in *pAddr)
{
	maketicknow();

	// Ԥ������
	assert(m_sock == INVALID_SOCKET);

	int	rst;
	if( (rst=ReConfigChannel(pInfo))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1,CNL_INT_RLS)"CNL2SlotMan_I_UDP::Init,%d,ReConfigChannel", rst);
		return	-1;
	}

	// ��ʽ����
	memcpy(&m_info, pInfo, sizeof(m_info));

	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_info.nTQChunkSize;
	if( m_tq.Init(&tqinfo)<0 )
	{
		return	-4;
	}
	// ��ʼCA��ʱ(���������m_tq��ʼ��֮�����)
	if( m_bUseAuth )
	{
		m_vectAuth.clear();
		m_vectRSA.clear();
		SetTE_CA(&CNL2SlotMan_I_UDP::TEDeal_CA0, 4*1024);
		SetTE_CA(&CNL2SlotMan_I_UDP::TEDeal_CA1, 16*1024);
	}

	if( m_queueControlOut.Init(m_info.nControlOutQueueSize)<0 )
	{
		return	-5;
	}

	struct sockaddr_in	addr;
	m_sock	= udp_create_socket_by_ipnportstr(m_info.szBindAddr, &addr);
	if( m_sock == INVALID_SOCKET )
	{
		return	-6;
	}
	m_nMyAddrSum	= ntohs(addr.sin_port);
	if( pAddr )
	{
		memcpy(pAddr, &addr, sizeof(addr));
	}

	// ������������һЩ��ز���
	Init_SetParams();

	if( m_Slots.Init(m_info.nMaxConnection)<0 )
	{
		return	-7;
	}

//	rst	= m_wca.Init(&m_a, m_info.nAllocChunkSize, m_info.nAlloc_MAXSIZENUM, m_info.nAlloc_HASHSTEP1, m_info.nAlloc_HASHSTEP2);
	if( rst<0 )
	{
		return	-8;
	}

	// �����ַ�б�
	// ��ַ�����ֹ�б�
	if( m_info.szConnectAllowDeny[0] )
	{
		// �ļ���������ʵ���൱��û���κ�policy
		m_ConnectAllowDeny.initfromfile(m_info.szConnectAllowDeny);
		m_fcdAllowDeny.SetFile(m_info.szConnectAllowDeny);
		// ���ü����
		Set_TEDeal_AllowDenyFileChangeCheck();
	}

	// ������һ�������ֵ
	m_nLastSwitchAddrTime	= wh_gettickcount();

	// ��ʼ������ͳ�ƣ���ʱͳ�ƣ�
	Set_TEDeal_StatInterval();

	return	0;
}
int		CNL2SlotMan_I_UDP::Init_SetParams()
{

	// ����socket�ĵײ㻺�����ߴ�
	if( m_info.nSockSndBufSize )
	{
		cmn_setsockopt_int(m_sock, SOL_SOCKET, SO_SNDBUF, m_info.nSockSndBufSize);
	}
	if( m_info.nSockRcvBufSize )
	{
		cmn_setsockopt_int(m_sock, SOL_SOCKET, SO_RCVBUF, m_info.nSockRcvBufSize);
	}

	// ������ô����Ϊ������RT�����Լ�������safe���ݻᷢ�ܴ�
	if( m_info.nMaxSinglePacketSize>CNL2_MAX_DATATOSEND )
	{
		m_info.nMaxSinglePacketSize	= CNL2_MAX_DATATOSEND;
	}
	m_vectrawbuf_recv.reserve(m_info.nMaxSinglePacketSize);
	m_vectrawbuf.reserve(m_info.nMaxSinglePacketSize);
	m_vectpackbuf.reserve(m_info.nMaxSinglePacketSize);

	// �����ط��ļ��
	if(m_info.nMaxNoopTime<=m_info.nMinNoopTime)
	{
		m_info.nMaxNoopTime	= m_info.nMinNoopTime*2;
	}
	m_nConnectRSTimeout	= m_info.nConnectTimeout / m_info.nConnectRSDiv;
	m_nNoopTimeout		= m_info.nDropTimeout / m_info.nNoopDivDrop;
	if( m_nNoopTimeout>m_info.nMaxNoopTime )
	{
		m_nNoopTimeout	= m_info.nMaxNoopTime;
	}
	else if( m_nNoopTimeout<m_info.nMinNoopTime )
	{
		m_nNoopTimeout	= m_info.nMinNoopTime;
	}
	m_nMightDropTimeout	= m_nNoopTimeout*3;

	return	0;
}
int		CNL2SlotMan_I_UDP::Init_Reload(INFO_T *pInfo)
{
	// ��ʽ����
	memcpy(&m_info, pInfo, sizeof(m_info));

	return	0;
}
int		CNL2SlotMan_I_UDP::ReConfigChannel(INFO_T *pInfo)
{
	// �������
	// ��֤ͨ��0Ϊ�ɿ�ͨ��
	if( pInfo->channelinfo[0].nType != CNL2_CHANNELTYPE_SAFE )
	{
		assert(0);
		return	-1;
	}
	// ��֤ԭ����slot���Ѿ��ر�
	if( m_Slots.size()>0 )
	{
		assert(0);
		return	-2;
	}
	// ��֤���ж��г��Ȳ��ܳ���65536
	for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		switch( pInfo->channelinfo[i].nType )
		{
			case	CNL2_CHANNELTYPE_NOTHING:
				// ʲôҲ����
				m_TickRecv_CMD_Others_DATA_Deal[i]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_NotSupport;
				m_Send_DATA[i]						= &CNL2SlotMan_I_UDP::Send_DATA_NotSupport;
				m_SCC[i]							= &CNL2SlotMan_I_UDP::SlotChannelConfirm_NotSupport;
			break;
			default:
			{
				if( pInfo->channelinfo[i].nInNum>0x10000 || pInfo->channelinfo[i].nOutNum>0x10000 )
				{
					return	-11;
				}
				switch( pInfo->channelinfo[i].nType )
				{
					case	CNL2_CHANNELTYPE_RTDATA:
						m_TickRecv_CMD_Others_DATA_Deal[i]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_RT;
						m_Send_DATA[i]						= &CNL2SlotMan_I_UDP::Send_DATA_RT;
						m_SCC[i]							= &CNL2SlotMan_I_UDP::SlotChannelConfirm_RT;
					break;
					case	CNL2_CHANNELTYPE_SAFE:
						m_TickRecv_CMD_Others_DATA_Deal[i]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_Safe;
						if( m_info.bUseTickPack )
						{
							m_Send_DATA[i]					= &CNL2SlotMan_I_UDP::Send_DATA_Safe_Pack;
						}
						else
						{
							m_Send_DATA[i]					= &CNL2SlotMan_I_UDP::Send_DATA_Safe;
						}
						m_SCC[i]							= &CNL2SlotMan_I_UDP::SlotChannelConfirm_Safe;
					break;
					case	CNL2_CHANNELTYPE_LASTSAFE:
						m_TickRecv_CMD_Others_DATA_Deal[i]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_LastSafe;
						m_Send_DATA[i]						= &CNL2SlotMan_I_UDP::Send_DATA_LastSafe;
						m_SCC[i]							= &CNL2SlotMan_I_UDP::SlotChannelConfirm_LastSafe;
					break;
					default:
						assert(0);
						return	-12;
					break;
				}
				// �����ͨ����֧��Send
				if( (pInfo->channelinfo[i].nProp & CNL2CHANNELINFO_T::PROP_CANSEND) == 0 )
				{
					m_Send_DATA[i]						= &CNL2SlotMan_I_UDP::Send_DATA_NotSupport;
				}
				// �����ͨ����֧��Recv
				if( (pInfo->channelinfo[i].nProp & CNL2CHANNELINFO_T::PROP_CANRECV) == 0 )
				{
					m_TickRecv_CMD_Others_DATA_Deal[i]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_NotSupport;
				}
			}
			break;
		}
	}

	// ����ͨ����Ϣ
	memcpy(&m_info.channelinfo, &pInfo->channelinfo, sizeof(m_info.channelinfo));

	return	0;
}
int		CNL2SlotMan_I_UDP::Release()
{
	// �ر�
	Listen(false);
	CloseAll();
	// �ȴ�һ��nCloseTimeout*2;
	whtick_t	t=wh_gettickcount();
	while( wh_tickcount_diff(wh_gettickcount(), t) < m_info.nCloseTimeout*2 )
	{
		DoSelect(50);
		TickRecv();
		TickLogicAndSend();
		if( m_Slots.size()==0 )
		{
			break;
		}
	}

	// ֱ�ӵ�������slot��clear
	whunitallocatorFixed_AllClear(m_Slots);

	// ��û�ж��ߵ����Ӿ��������Լ���ʱ��
	m_Slots.Release();

	// ���ʱ�����(�����Ҫ��CloseAll֮����)
	m_tq.Release();
	m_queueControlOut.Release();

	cmn_safeclosesocket(m_sock);
	// ��յ�ַӳ��
	m_mapaddr2slot.clear();

	// С�ڴ�������ͷ�
//	m_wca.Release();

	return	0;
}
int		CNL2SlotMan_I_UDP::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	vect.push_back(m_sock);
	return	0;
}
int		CNL2SlotMan_I_UDP::DoSelect(int nMS)
{
	return	cmn_select_rd(m_sock, nMS);
}
int		CNL2SlotMan_I_UDP::TickLogicAndSend()
{
dbgtimeuse	m_dtu;
m_dtu.BeginMark();
m_dtu.Mark();
int	nCDTS	= m_dlistChannelHaveDataToSend.size();
	// �Ա�tick���ۻ�����Ҫ����Ķ������д��
	TickLogic_PackAndCompression();
m_dtu.Mark();
	// ʱ�����(�����������������Send��Resend)
	TickLogic_TE();
m_dtu.Mark();
	// ���Ƿ�����ҪConfirm��
	_TickLogic_DealSlotInDList(CNL2SLOT_I::DLISTIDX_COFIRM, &CNL2SlotMan_I_UDP::_DealSlotInDList_Confirm);
m_dtu.Mark();
	// ���Ƿ�����ҪNOOP��
	_TickLogic_DealSlotInDList(CNL2SLOT_I::DLISTIDX_NOOP, &CNL2SlotMan_I_UDP::_DealSlotInDList_Noop);
m_dtu.Mark();
if( m_dtu.GetDiff()>=100 )
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1063,BIGTICK)"CNL2SlotMan_I_UDP::TickLogicAndSend,%d,%d,%s", nCDTS, m_dtu.GetDiff(), m_dtu.Disp());
}

	return	0;
}
int		CNL2SlotMan_I_UDP::TickLogic_PackAndCompression()
{

	if( m_dlistChannelHaveDataToSend.size() == 0 )
	{
		return	0;
	}
	whDList<CNL_SLOT_CHANNEL_T>::node	*pNode;
	whvector<char>	&vectbuf	= m_vectpackbuf;
	// ����ۻ�����
	for(pNode=m_dlistChannelHaveDataToSend.begin();pNode!=m_dlistChannelHaveDataToSend.end();pNode=pNode->next)
	{
	
		

		m_pSlot		= m_Slots.getptr(pNode->data.nSlot);
		if( !m_pSlot )
		{
			// Ӧ����slot�ڱ�tick�պñ�ɾ����
			continue;
		}
		m_pChannel	= &m_pSlot->channel[pNode->data.nChannel];

		try
		{
#ifdef _DEBUG
			dbgtimeuse	m_dtu;
			m_dtu.BeginMark();
			m_dtu.Mark();
#endif
			if( m_pChannel->dlistCmdPack.size() == 1 )
			{
				// ֻ��һ����������ͨ�ķ�ʽ����
				whDList<CMDPACKUNIT_T>::node	*pNode	= m_pChannel->dlistCmdPack.begin();
				_Send_DATA_Safe(pNode->data.pCmd, pNode->data.nSize, 0);
				FreeAndClearData(pNode->data.pCmd);
				pNode->leave();
				FreeData(pNode);
			}
			else
			{
				// ����һ�������մ���ķ�ʽ����
				// �ȴ��
				// ���ڴ���Ļ��壨һ��������ָ������ܳ���CNL������ͳ��ȣ�
				vectbuf.resize(m_info.nMaxSinglePacketSize);
				whcmdshrink		wcs;
				wcs.SetShrink(vectbuf.getbuf(), vectbuf.size());
				whDList<CMDPACKUNIT_T>::node	*pNode	= m_pChannel->dlistCmdPack.begin();
				while( pNode!=m_pChannel->dlistCmdPack.end() )
				{
					
					if( wcs.WriteVData(pNode->data.pCmd, pNode->data.nSize)<0 )	// Ӧ���ǳ��Ȳ���
					{
						// �ѻ���Ӵ�

						vectbuf.resize(vectbuf.size()+pNode->data.nSize+4);
						wcs.SetShrinkResize(vectbuf.getbuf(), vectbuf.size());
						if( wcs.WriteVData(pNode->data.pCmd, pNode->data.nSize)<0 )
						{
							
							// �����С��0����������
							assert(0);
						}
#ifdef _DEBUG
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(000,CLSDEBUG)"CNL2SlotMan_I_UDP::TickLogic_PackAndCompression,resize:%d",vectbuf.size());
#endif
					}
					FreeAndClearData(pNode->data.pCmd);
					pNode->leave();
					FreeData(pNode);
					// ��һ��
					pNode	= m_pChannel->dlistCmdPack.begin();
				}
				wcs.WriteEnd();
				// �ٷ���
#ifdef _DEBUG
				m_dtu.Mark();
#endif
				_Send_DATA_Safe(wcs.GetShrinkBuf(), wcs.GetShrinkSize(), CNL2_CMD_DATA_T::PROP_PACK);
#ifdef _DEBUG
				m_dtu.Mark();

				if (m_dtu.GetDiff() >= 60)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(000,CLSDEBUG)"CNL2SlotMan_I_UDP::TickLogic_PackAndCompression,bigDIFFER,%d,%s",m_dtu.GetDiff(),m_dtu.Disp());
				}
#endif
			}
		}
		catch( const char *czsErr )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(3,CNL_RT)"CNL2SlotMan_I_UDP::TickLogic_PackAndCompression,%s,WRITE PCK DATA ERR,0x%X %d %d", czsErr, pNode->data.nSlot, pNode->data.nChannel, m_pChannel->dlistCmdPack.size());
		}
	}
	// ����ۻ�����
	m_dlistChannelHaveDataToSend.clear();
	return	0;
}

int		CNL2SlotMan_I_UDP::_TickLogic_DealSlotInDList(int nIdx, DEALSLOT_T func)
{
	// ��������
	whDList<CNL2SLOT_I *>		&DL		= m_dlistSlot[nIdx];
	whDList<CNL2SLOT_I *>::node	*pNode	= DL.begin();
	whDList<CNL2SLOT_I *>::node	*pCur;
	while(pNode!=DL.end())
	{
		pCur	= pNode;
		pNode	= pNode->next;
		// ����
		(this->*func)(pCur->data);
		// �ϵ����
		pCur->leave();
	}
	return	0;
}
void	CNL2SlotMan_I_UDP::_DealSlotInDList_Confirm(CNL2SLOT_I *pSlot)
{
	// ���ȫ��channel���Ѿ�confirm����ʲôҲ����
	if( pSlot->nPeerNotConfirmedChannelMask==0 )
	{
		return;
	}
	// ���Ͷ�Ӧ����channel��confirm
	CNL2_CMD_DATA_CONFIRM_T	*pCmd	= (CNL2_CMD_DATA_CONFIRM_T *)m_vectrawbuf.getbuf();
	pCmd->cmd			= CNL2_CMD_DATA_CONFIRM;
	pCmd->slotidx		= pSlot->slotinfo.nPeerSlotIdx;
	pCmd->channelmask	= pSlot->nPeerNotConfirmedChannelMask;
	size_t				nSize		= sizeof(*pCmd);
	void				*pData		= wh_getptrnexttoptr(pCmd);
	// ��������
	for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		if( whbit_uchar_chk(&pCmd->channelmask, i) )
		{
			size_t	nAddSize=0;
			CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[i];
			switch( channel.nType )
			{
				case	CNL2_CHANNELTYPE_SAFE:
					nAddSize	= channel.MakeInQueueConfirm_Safe(pData);
				break;
				case	CNL2_CHANNELTYPE_LASTSAFE:
					nAddSize	= channel.MakeInQueueConfirm_LastSafe(pData);
				break;
				default:
					// ��������Ϊ������CNL2_RECV_TRIGER_CONFIRM��ɵ�
					whbit_uchar_clr(&pCmd->channelmask, i);
					continue;
				break;
			}
			pData	= wh_getoffsetaddr(pData, nAddSize);
			nSize	+= nAddSize;
		}
	}
	// ��������
	pCmd->calcsetcrc(nSize);
	slot_send(pSlot, pCmd, nSize);
	// ���
	pSlot->nPeerNotConfirmedChannelMask	= 0;
}
void	CNL2SlotMan_I_UDP::_DealSlotInDList_Noop(CNL2SLOT_I *pSlot)
{
	CNL2_CMD_NOOP_T			Cmd;
	Cmd.cmd					= CNL2_CMD_NOOP;
	Cmd.channel				= 0;
	Cmd.slotidx				= pSlot->slotinfo.nPeerSlotIdx;
	Cmd.nSendTime			= m_tickNow;
	Cmd.nPeerSendTime		= pSlot->nPeerNoopSendTime;
	Cmd.nCount				= pSlot->nNoopCount;
	size_t			nSize	= sizeof(Cmd);
	// ��������
	Cmd.calcsetcrc(nSize);
	slot_send(pSlot, &Cmd, nSize);
#ifdef	_WH_LOWLEVELDEBUG
	if( pSlot->nNoopCount>0 && wh_tickcount_diff(wh_gettickcount(), pSlot->slotinfo.nLastRecv)>200 )
	{
		WHTMPPRINTF("strange NOOP %lu %lu %lu%s", wh_gettickcount(), m_tickNow, pSlot->slotinfo.nLastRecv, WHLINEEND);
	}
#endif
}
int		CNL2SlotMan_I_UDP::Listen(bool bListen)
{
	m_bListen	= bListen;
	return	0;
}
int	CNL2SlotMan_I_UDP::Connect(struct sockaddr_in *addr, void *pExtData, int nExtSize)
{
	if( nExtSize>255 )
	{
		assert(0);
		return	-1;
	}
	// ���
	int	nSlot;
	if( AddSlot(*addr, CNL2SLOTINFO_T::STATUS_CONNECTING, 0, pExtData, nExtSize, &nSlot, NULL, 0, ntohs(addr->sin_port))<0 )
	{
		return	-2;
	}
	return	nSlot;
}
int	CNL2SlotMan_I_UDP::Connect(const char *cszAddr, void *pExtData, int nExtSize)
{
	struct sockaddr_in	addr;
	if( !cmn_get_saaddr_by_ipnportstr(&addr, cszAddr) )
	{
		return	-1;
	}
	return	Connect(&addr, pExtData, nExtSize);
}
int		CNL2SlotMan_I_UDP::Close(int nSlot, int nExtData)
{
	CNL2SLOT_I	*pSlot	= m_Slots.getptr(nSlot);
	if( !pSlot )
	{
		return	-1;
	}
	pSlot->nCloseExtDataSend	= nExtData;
	if( Close(pSlot)<0 )
	{
		return	-2;
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::Close(CNL2SLOT_I *pSlot)
{
	SetCloseStatus(pSlot, false);
	return	0;
}
int		CNL2SlotMan_I_UDP::CloseAll()
{
	for(whunitallocatorFixed<CNL2SLOT_I>::iterator it=m_Slots.begin(); it!=m_Slots.end(); ++it)
	{
		CNL2SLOT_I	*pSlot	= &(*it);
		Close(pSlot);	// ������ᵼ��slot��remove�����Կ�����iteratorѭ������������
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::CloseOnErr(CNL2SLOT_I *pSlot, int nErrCode, int nExt)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(2,CNL_RT)"CNL2SlotMan_I_UDP::CloseOnErr,%d,%d,0x%lX,%s", nErrCode, nExt, pSlot->slotinfo.nSlot,cmn_get_ipnportstr_by_saaddr(&pSlot->slotinfo.peerAddr) );
	return	Close(pSlot);
}
void	CNL2SlotMan_I_UDP::SetCloseStatus(CNL2SLOT_I *pSlot, bool bClosePassive)
{
	if( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_CLOSING )
	{
		// �Ѿ��ڹر�״̬
		return;
	}
	// ���bClosePassiveΪfalse���������ر�
	pSlot->bClosePassive	= bClosePassive;
	pSlot->slotinfo.nStatus	= CNL2SLOTINFO_T::STATUS_CLOSING;
	// ��ȡ��ǰʱ�����ע���ط��¼�ʱ��Ҫ
	maketicknow();
	// �������������ʱ���¼��͸���channel��ص�ʱ���¼�
	ClearSlotAllTEAndChannelAllTE(pSlot);
	// ���ùرճ�ʱ
	RegCloseTQ(pSlot);
	if( !bClosePassive )
	{
		// �����ر�
		// �����ط���ʱ
		RegCloseRSTQ(pSlot);
		// ���͹ر�ָ��
		SendClose(pSlot);
		// ��������(�������յ��Է���Ӧ��ʱ��ͻ�֪�����Խ�����)
		pSlot->nCloseCount				= 1;
	}
	else
	{
		// �����ر�(����Ҫ��ʱ�ط���ֻ��Ҫ�յ��Է���CLOSE������Ӧ����)
	}
}
int		CNL2SlotMan_I_UDP::ControlOut(CONTROL_T **ppCmd, size_t *pnSize)
{
	*pnSize	= m_queueControlOut.PeekSize();
	if( *pnSize<=0 )
	{
		return	-1;
	}
	m_vectControlOutCmd.resize(*pnSize);
	*ppCmd	= (CONTROL_T *)m_vectControlOutCmd.getbuf();
	m_queueControlOut.Out(*ppCmd, pnSize);
	return	0;
}
CNL2SLOTINFO_T *	CNL2SlotMan_I_UDP::GetSlotInfo(int nSlot)
{
	CNL2SLOT_I	*pSlot	= m_Slots.getptr(nSlot);
	if( !pSlot )
	{
		return	NULL;
	}
	return	&pSlot->slotinfo;
}
bool	CNL2SlotMan_I_UDP::IsSlotExist(int nSlot) const
{
	return	m_Slots.IsUnitUsed(nSlot);
}
cmn_addr_allowdeny &	CNL2SlotMan_I_UDP::GetConnectAllowDeny()
{
	return	m_ConnectAllowDeny;
}
whvector<char> *	CNL2SlotMan_I_UDP::GetConnectExtData(int nSlot)
{
	CNL2SLOT_I	*pSlot	= m_Slots.getptr(nSlot);
	if( !pSlot )
	{
		return	NULL;
	}
	return	&pSlot->vectExtSlotData;
}
int		CNL2SlotMan_I_UDP::SetExtLong(int nSlot, long nExt)
{
	CNL2SLOT_I	*pSlot	= m_Slots.getptr(nSlot);
	if( !pSlot )
	{
		return	-1;
	}
	pSlot->nExt	= nExt;
	return	0;
}
int		CNL2SlotMan_I_UDP::GetExtLong(int nSlot, long *pnExt)
{
	CNL2SLOT_I	*pSlot	= m_Slots.getptr(nSlot);
	if( !pSlot )
	{
		return	-1;
	}
	*pnExt	= pSlot->nExt;
	return	0;
}
void	CNL2SlotMan_I_UDP::SetICryptFactory(ICryptFactory *pICryptFactory)
{
	m_pICryptFactory			= pICryptFactory;
	// ��ʼ��������ӣ�m_pICryptFactory����Ϊ�գ���˵��������û�н��ã����ܽ�󽨺ã�
	if( m_pICryptFactory )
	{
		m_pICryptFactory->RandInit();
	}
}
int		CNL2SlotMan_I_UDP::GetSlotNum() const
{
	return	m_Slots.size();
}

int		CNL2SlotMan_I_UDP::AddSlot(const struct sockaddr_in &addr, int nStatus, short nPeerSlotIdx, const void *pExtData, int nExtSize, int *pnSlot, const void *pPubData, int nPubSize, int nKeyExchangerType)
{
	// ԭ��һֱû���������(2006-02-05)
	maketicknow();

	*pnSlot	= 0;
	// ����Ƿ��ַ�Ѿ�����
	if( m_mapaddr2slot.has(addr) )
	{
		return	ADDSLOT_RST_HASADDR;
	}
	// ����slot
	int				nSlot	= m_Slots.AllocUnit(m_pSlot);
	if( nSlot<0 )
	{
		// û�еط���
		return	ADDSLOT_RST_SLOTFULL;
	}
	// ��ʼ��
	m_pSlot->clear();			// Ϊ�˱��ջ���Ҫ��ʼ��һ��(����lastsend/lastrecvʱ��Ͳ���Ҫ����ʱ��ʼ��)
	m_pSlot->slotinfo.nStatus	= nStatus;
	m_pSlot->slotinfo.nSlot		= nSlot;
	m_pSlot->slotinfo.peerAddr	= addr;
	m_pSlot->slotinfo.nPeerSlotIdx	= nPeerSlotIdx;
	m_pSlot->SetSlotMan(this);	// ����������slotinfo.nSlot������֮��
	// Ϊ�˱��������Ҫ�ȼٶ�һ����ֵ
	m_pSlot->slotinfo.nLastRecv	= m_tickNow;
	// ������Կ������(������ζ�Ҫ�������)
	m_pSlot->pIKeyExchanger		= m_pICryptFactory->CreateIKeyExchanger(nKeyExchangerType);;
	if( m_pSlot->pIKeyExchanger == NULL )
	{
		return	ADDSLOT_RST_KEYEXCHANGER_INITERR;
	}
	if( m_pSlot->pIKeyExchanger->MakeOwnData() < 0 )
	{
		return	ADDSLOT_RST_KEYEXCHANGER_INITERR;
	}
	// ���õ�ַӳ��
	m_mapaddr2slot.put(m_pSlot->slotinfo.peerAddr, nSlot);
	// ���ú���ָ��
	FillSlotWithDefaultTickRecvDealFunc(m_pSlot);
	// ��������(����Ҫ������ķ���֮ǰ����)
	m_pSlot->vectExtSlotData.resize(nExtSize);
	if( nExtSize>0 )
	{
		memcpy(m_pSlot->vectExtSlotData.getbuf(), pExtData, nExtSize);
	}
	try
	{
		// ��ʼ��������ͨ��
		for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
		{
			m_pSlot->channel[i].nChannel	= i;
			if( m_pSlot->channel[i].Init(&m_info.channelinfo[i])<0 )
			{
				throw	(int)ADDSLOT_RST_MEMERR;
			}
		}
		switch( nStatus )
		{
			case	CNL2SLOTINFO_T::STATUS_CONNECTING:
			{
				m_pSlot->bIsClient			= true;
				// ���������Accept�Ĳ�ͬ
				m_pSlot->dealfuncRecv[CNL2_CMD_DATA]		= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA;
				m_pSlot->dealfuncRecv[CNL2_CMD_NOOP]		= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP;
				// �������ӳ�ʱ�¼�
				TQUNIT_T					tqu;
				tqu.pSlot					= m_pSlot;
				tqu.tefunc					= &CNL2SlotMan_I_UDP::TEDeal_Connect;
				if( m_tq.Add(m_tickNow+m_info.nConnectTimeout, &tqu, &m_pSlot->teid[CNL2SLOT_I::TE_IDX_CONNECT])<0 )
				{
					throw	(int)ADDSLOT_RST_MEMERR;
				}
				// ����ָ��
				SendConnectAndRegTQ(m_pSlot);
			}
			break;
			case	CNL2SLOTINFO_T::STATUS_ACCEPTING:
			{
				m_pSlot->bIsClient			= false;
				// ������һ�ּ��ܷ�ʽ
				WHRANGE_T<int>	r(m_info.nCryptorType1, m_info.nCryptorType2);
				m_pSlot->nCryptorType		= r.GetRand();
				// ������Կ
				if( m_pSlot->pIKeyExchanger->Agree1(pPubData, nPubSize)<0 )
				{
					throw	(int)ADDSLOT_RST_KEYEXCHANGER_INITERR;
				}
				// ����Key��MD5
				whmd5(m_pSlot->pIKeyExchanger->GetKey(), m_pSlot->pIKeyExchanger->GetKeySize(), m_pSlot->szKeyMD5);
				// ����Accept��ʱ
				TQUNIT_T					tqu;
				tqu.pSlot					= m_pSlot;
				tqu.tefunc					= &CNL2SlotMan_I_UDP::TEDeal_Accept;
				if( m_tq.Add(m_tickNow+m_info.nConnectTimeout, &tqu, &m_pSlot->teid[CNL2SLOT_I::TE_IDX_ACCEPT])<0 )
				{
					throw	(int)ADDSLOT_RST_MEMERR;
				}
				// ����agree�������ط���ʱ
				SendConnectAgreeAndRegTQ(m_pSlot);
			}
			break;
		}
	}
	catch( int nErr )
	{
		RemoveSlot(m_pSlot);
		m_pSlot	= NULL;
		return	nErr;
	}
	*pnSlot	= nSlot;

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(903,CNL_RT)"SLOT ADD,%s,0x%X", cmn_get_ipnportstr_by_saaddr(&m_pSlot->slotinfo.peerAddr), m_pSlot->slotinfo.nSlot);

	return	0;
}
int		CNL2SlotMan_I_UDP::RemoveSlot(int nSlot)
{
	CNL2SLOT_I	*pSlot	= m_Slots.getptr(nSlot);
	if( !pSlot )
	{
		return	-1;
	}
	return	RemoveSlot(pSlot);
}
int		CNL2SlotMan_I_UDP::RemoveSlot(CNL2SLOT_I *pSlot)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(904,CNL_RT)"SLOT REMOVE,%s,0x%X", cmn_get_ipnportstr_by_saaddr(&pSlot->slotinfo.peerAddr), pSlot->slotinfo.nSlot);
	// ɾ��ʱ���¼��͸���channel��ص�ʱ���¼�
	ClearSlotAllTEAndChannelAllTE(pSlot);
	// �����ַӳ��
	m_mapaddr2slot.erase(pSlot->slotinfo.peerAddr);
	// ��ղ�ɾ��slot��Ԫ
	int	nSlot	= pSlot->slotinfo.nSlot;
	pSlot->clear();
	m_Slots.FreeUnit(nSlot);
	return	0;
}
void	CNL2SlotMan_I_UDP::SendConnectAndRegTQ(CNL2SLOT_I *pSlot)
{
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_CONNECTING );
	TQUNIT_T						tqu;
	tqu.pSlot						= pSlot;
	tqu.tefunc						= &CNL2SlotMan_I_UDP::TEDeal_Connect_RS;
	if( m_tq.Add(m_tickNow+m_nConnectRSTimeout, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_CONNECT_RS])<0 )
	{
		return;
	}
	// ����(+1����Ϊ��Ҫ��һ���ֽڼ�¼PubData�ĳ���)
	m_vectrawbuf.resize(sizeof(CNL2_CMD_CONNECT_T) + pSlot->pIKeyExchanger->GetPubSize()+sizeof(CONNECT_PUBKEY_LEN_T) + pSlot->vectExtSlotData.size()+sizeof(CONNECT_EXT_LEN_T));
	CNL2_CMD_CONNECT_T	&Cmd	= *(CNL2_CMD_CONNECT_T *)m_vectrawbuf.getbuf();
	Cmd.cmd				= CNL2_CMD_CONNECT;
	Cmd.ver				= m_info.ver;
	Cmd.clientslotidx	= m_Slots.getrealidx(pSlot->slotinfo.nSlot);
	void	*pExt		= wh_getptrnexttoptr(&Cmd);
	// ע�⣺��Ϊ��Կ���ܴ���256�ֽڣ������ÿɱ�����ݣ�Ϊ��ͳһ����ԭ���ĸ�������Ҳֱ���ÿɱ������ˣ�
	pExt				= wh_getoffsetaddr(pExt, whvldata_write(pSlot->pIKeyExchanger->GetPubData(), pSlot->pIKeyExchanger->GetPubSize(), (CONNECT_PUBKEY_LEN_T *)pExt));
//	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "��ʱ��־ Connect PubData size:%d data:%s", pSlot->pIKeyExchanger->GetPubSize(), wh_hex2str((const unsigned char*)pSlot->pIKeyExchanger->GetPubData(), pSlot->pIKeyExchanger->GetPubSize()));
	pExt				= wh_getoffsetaddr(pExt, whvldata_write(pSlot->vectExtSlotData.getbuf(), pSlot->vectExtSlotData.size(), (CONNECT_EXT_LEN_T *)pExt));
	Cmd.calcsetcrc(m_vectrawbuf.size());
	if( slot_send(pSlot, m_vectrawbuf.getbuf(), m_vectrawbuf.size())<0 )
	{
		return;
	}
}
void	CNL2SlotMan_I_UDP::SendConnectAgreeAndRegTQ(CNL2SLOT_I *pSlot)
{
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_ACCEPTING );
	TQUNIT_T						tqu;
	tqu.pSlot						= pSlot;
	tqu.tefunc						= &CNL2SlotMan_I_UDP::TEDeal_Accept_RS;
	if( m_tq.Add(m_tickNow+m_nConnectRSTimeout, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_ACCEPT_RS])<0 )
	{
		return;
	}
	// ����(+sizeof(CONNECT_PUBKEY_LEN_T)����Ϊ��Ҫ��¼PubData�ĳ���)
	m_vectrawbuf.resize(sizeof(CNL2_CMD_CONNECT_AGREE_T) + pSlot->pIKeyExchanger->GetPubSize()+sizeof(CONNECT_PUBKEY_LEN_T));
	CNL2_CMD_CONNECT_AGREE_T		&Cmd	= *(CNL2_CMD_CONNECT_AGREE_T *)m_vectrawbuf.getbuf();
	Cmd.cmd							= CNL2_CMD_CONNECT_AGREE;
	Cmd.channel						= 0;
	Cmd.slotidx						= pSlot->slotinfo.nPeerSlotIdx;
	Cmd.serverslotidx				= m_Slots.getrealidx(pSlot->slotinfo.nSlot);
	Cmd.cryptortype					= pSlot->nCryptorType;
	Cmd.nNoopTimeout				= m_nNoopTimeout;
	Cmd.nDropTimeout				= m_info.nDropTimeout;
	Cmd.nMightDropTimeout			= m_nMightDropTimeout;
	void	*pExt		= wh_getptrnexttoptr(&Cmd);
	pExt				= wh_getoffsetaddr(pExt, whvldata_write(pSlot->pIKeyExchanger->GetPubData(), pSlot->pIKeyExchanger->GetPubSize(), (CONNECT_PUBKEY_LEN_T*)pExt));
//	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "��ʱ��־ Agree PubData size:%d data:%s", pSlot->pIKeyExchanger->GetPubSize(), wh_hex2str((const unsigned char*)pSlot->pIKeyExchanger->GetPubData(), pSlot->pIKeyExchanger->GetPubSize()));
	// ����pExtӦ��ָ��պ�û�����ݵĵط�������ûʲô�ã���������
	Cmd.calcsetcrc(m_vectrawbuf.size());
	if( slot_send(pSlot, m_vectrawbuf.getbuf(), m_vectrawbuf.size())<0 )
	{
		return;
	}
	return;
}
void	CNL2SlotMan_I_UDP::SendClose(CNL2SLOT_I *pSlot)
{
	// ����
	CNL2_CMD_CLOSE_T				Cmd;
	Cmd.cmd							= CNL2_CMD_CLOSE;
	Cmd.channel						= 0;
	Cmd.slotidx						= pSlot->slotinfo.nPeerSlotIdx;
	Cmd.nExtData					= pSlot->nCloseExtDataSend;
	Cmd.calcsetcrc(sizeof(Cmd));
	if( slot_send(pSlot, &Cmd, sizeof(Cmd))<0 )
	{
		return;
	}
	return;
}
void	CNL2SlotMan_I_UDP::SendNoopThisTick(CNL2SLOT_I *pSlot)
{
	pSlot->bNoopMustBeSend	= true;
	AddSlotToDListIfNeeded(pSlot, CNL2SLOT_I::DLISTIDX_NOOP);
}
void	CNL2SlotMan_I_UDP::SendConfirmThisTick(CNL2SLOT_I *pSlot)
{
	AddSlotToDListIfNeeded(pSlot, CNL2SLOT_I::DLISTIDX_COFIRM);
}
void	CNL2SlotMan_I_UDP::RegNoopTQ(CNL2SLOT_I *pSlot, whtick_t nTime)
{
	TQUNIT_T						tqu;
	tqu.pSlot						= pSlot;
	tqu.tefunc						= &CNL2SlotMan_I_UDP::TEDeal_Noop;
	// ԭ��������д��
	//m_tq.Add(pSlot->slotinfo.nLastSend+m_nNoopTimeout, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_NOOP]);
	// ������ΪNoopʵ���ϻ�û�е��·��Ͷ���������pSlot->slotinfo.nLastSend���ɺ�С�����ʱ���������
	m_tq.Add(nTime, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_NOOP]);
}
void	CNL2SlotMan_I_UDP::RegDropTQ(CNL2SLOT_I *pSlot)
{
	TQUNIT_T						tqu;
	tqu.pSlot						= pSlot;
	tqu.tefunc						= &CNL2SlotMan_I_UDP::TEDeal_Drop;
	m_tq.Add(pSlot->slotinfo.nLastRecv+m_info.nDropTimeout, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_DROP]);
}
void	CNL2SlotMan_I_UDP::RegMightDropTQ(CNL2SLOT_I *pSlot, whtick_t nTime)
{
	TQUNIT_T						tqu;
	tqu.pSlot						= pSlot;
	tqu.tefunc						= &CNL2SlotMan_I_UDP::TEDeal_MightDrop;
	m_tq.Add(nTime, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_MIGHTDROP]);
}
void	CNL2SlotMan_I_UDP::RegSwitchAddrReq(CNL2SLOT_I *pSlot, whtick_t nTime)
{
	pSlot->teid[CNL2SLOT_I::TE_IDX_SWITCHADDRREQ].quit();	// �����������Ϊ�ڶ��߹�����Ҳ��ᵼ�����ɶ��Req������ÿ��һ��һ��Ҫȷ����ԭ�����������
	TQUNIT_T						tqu;
	tqu.pSlot						= pSlot;
	tqu.tefunc						= &CNL2SlotMan_I_UDP::TEDeal_SwitchAddrReq;
	m_tq.Add(nTime, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_SWITCHADDRREQ]);
}
void	CNL2SlotMan_I_UDP::RegCloseTQ(CNL2SLOT_I *pSlot)
{
	TQUNIT_T						tqu;
	tqu.pSlot						= pSlot;
	tqu.tefunc						= &CNL2SlotMan_I_UDP::TEDeal_Close;
	m_tq.Add(m_tickNow+m_info.nCloseTimeout, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_CLOSE]);
}
void	CNL2SlotMan_I_UDP::RegCloseRSTQ(CNL2SLOT_I *pSlot)
{
	TQUNIT_T						tqu;
	tqu.pSlot						= pSlot;
	tqu.tefunc						= &CNL2SlotMan_I_UDP::TEDeal_Close_RS;
	m_tq.Add(m_tickNow+pSlot->slotinfo.nRTT, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_CLOSE_RS]);
}
void *	CNL2SlotMan_I_UDP::ControlOutAlloc(size_t nSize)
{
	void *ptr	= m_queueControlOut.InAlloc(nSize);
	if( !ptr )
	{
		// Ӧ���Ǻ����ص������ˣ���ô����أ���¼��־!
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(4,CNL_RT)"CNL2SlotMan_I_UDP::ControlOutAlloc,,m_queueControlOut.InAlloc,%u", nSize);
	}
	return		ptr;
}
void	CNL2SlotMan_I_UDP::ControlOutReasonAndRemoveSlot(CNL2SLOT_I *pSlot, int nCmd, int nReason)
{
	// ����ָ����ϲ�
	CONTROL_OUT_SLOT_CLOSE_REASON_T	*pCO;
	ControlOutAlloc(pCO);
	if( pCO )
	{
		pCO->nCmd				= nCmd;
		pCO->nSlot				= pSlot->slotinfo.nSlot;
		pCO->peerAddr			= pSlot->slotinfo.peerAddr;
		pCO->nReason			= nReason;
		pCO->nExtData			= pSlot->nCloseExtDataRecv;
		pCO->nSlotExt			= pSlot->nExt;
	}
	// �Ƴ�
	RemoveSlot(pSlot);
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	(this->*m_SCC[nChannel])(pSlot, nChannel, nIdx, nNum);
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm_NotSupport(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	// ֱ�ӹر�
	CloseOnErr(pSlot, CLOSEONERR_SlotChannelConfirm_NotSupport);
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm_RT(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	// RT��Ӧ����Confirm
	CloseOnErr(pSlot, CLOSEONERR_SlotChannelConfirm_RT);
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm_Safe(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[nChannel];
	if( channel.pOutQueue->size() == 0 )
	{
		// û�б�ҪУ���ˣ���Ϊû������
		return;
	}
	bool	bChanged	= false;
	if( nNum==0 )
	{
again:
		// �ж��Ƿ����µ�ȷ������
		if( idxdiff(nIdx, channel.nOutIdxConfirm)<=0 )
		{
			// �����µģ�ֱ�ӷ���
			return;
		}
		if( idxdiff(nIdx, channel.nOutIdx0)>(int)channel.pOutQueue->size() )
		{
			// ���ݳ��磬hacker
			CloseOnErr(pSlot, CLOSEONERR_IdxOutOfRange);
			return;
		}
		// �ϴ�У���λ�õ����idx����ȷ�ϡ�ͬʱ�����Ӧ���ط��¼�
		channel.pOutQueue->BeginGet( idxdiff(channel.nOutIdxConfirm, channel.nOutIdx0) );
		while( channel.nOutIdxConfirm!=nIdx )
		{
			bChanged	|= _SlotChannelConfirm_Safe_SetConfirm(channel.pOutQueue->GetNext());
			channel.nOutIdxConfirm	++;
		}
	}
	else
	{
		// ��������Ƿ�ֵ��У��
		cnl2idx_t	nLast	= nIdx + nNum;
		int			nDiff	= idxdiff(nLast, channel.nOutIdxConfirm);
		if( nDiff<=0 )
		{
			// û�����������(̫С��)
			return;
		}
		nDiff	= idxdiff(nLast, channel.nOutIdx0);
		if( nDiff>(int)channel.pOutQueue->size() )
		{
			// ���ݳ��磬hacker
			CloseOnErr(pSlot, CLOSEONERR_IdxOutOfRange);
			return;
		}
		nDiff	= idxdiff(nIdx, channel.nOutIdxConfirm);
		if( nDiff<=0 )
		{
			// ��ʼλ��������ȷ��λ��֮ǰ
			// ���������Ľ���
			nIdx	= nLast;
			goto	again;
		}
		// ��ǰ�������û�����İ��ڱ�tick����������2007-08-14 ������ԭ���ǰ����еİ�������û��ȷ�϶����ˣ�
		channel.pOutQueue->BeginGet( idxdiff(channel.nOutIdxConfirm, channel.nOutIdx0) );
		while( nDiff-- > 0 )	// ��ס�������Ǵ��ڵ��ڣ���Ϊ����Ҫ��֮ǰ�ģ���������ķ���һ��
		{
			CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= channel.pOutQueue->GetNext();
			if( pOutUnit->nTimeEventID.IsValid() )
			{
				// pOutUnit->nRSCount	= 1;
				// ����ӳ�һ��RTT�ͷ���
				SendOutUnitWithDelay(pSlot, pOutUnit, pSlot->slotinfo.nRTT);
			}
		}
		// ֮���ȷ��
		while( nNum>0 )
		{
			bChanged	|= _SlotChannelConfirm_Safe_SetConfirm(channel.pOutQueue->GetNext());
			nNum	--;
		}
	}
}
bool	CNL2SlotMan_I_UDP::_SlotChannelConfirm_Safe_SetConfirm(CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit)
{
	if( !pOutUnit->nTimeEventID.IsValid() )
	{
		// ˵���Ѿ���ȷ�Ϲ���
		return	false;
	}
	// ����Ping
	m_pSlot->slotinfo.calcRTT(wh_tickcount_diff(m_tickNow, pOutUnit->nSendTime));
	// ȡ����Ӧ��ʱ���¼�
	pOutUnit->nTimeEventID.quit();
	return		true;
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm_LastSafe(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[nChannel];
	if( channel.pOutQueue->size() == 0 )
	{
		// û�б�ҪУ���ˣ���Ϊû������
		return;
	}
	// nNum��Զ����
	if( nNum!=0 )
	{
		CloseOnErr(pSlot, CLOSEONERR_IdxOutOfRange);
		return;
	}
	// ��Ч��
	int	nDiff	= idxdiff(nIdx, channel.nOutIdxConfirm);
	if( nDiff!=1 )
	{
		// ̫�ϵ�ȷ�ϰ�����hack���İ�
		return;
	}
	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= channel.pOutQueue->GetFirst();
	// ����ping
	m_pSlot->slotinfo.calcRTT(wh_tickcount_diff(m_tickNow, pOutUnit->nSendTime));
	// ȡ�������Ԫ
	pOutUnit->clear(this);
	// �������
	channel.pOutQueue->Clean();
	// ��������
	channel.nOutIdx0		++;
	channel.nOutIdxConfirm	= channel.nOutIdx0;
}
void	CNL2SlotMan_I_UDP::SlotOutChannelMove(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx0)
{
	CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[nChannel];
	int		nDiff	= idxdiff(nIdx0, channel.nOutIdx0);
	if( nDiff <= 0 )
	{
		// ��Ҫ̫С�������Ǻ��ϵİ��ӳٹ�����
		return;
	}
	if( nDiff > channel.pOutQueue->size() )
	{
		// Ӧ���ǲ����ܵģ���Ӧ��Ҫ���ͷŻ�û�зų�������
		CloseOnErr(pSlot, CLOSEONERR_IdxOutOfRange);
		return;
	}
	// ѭ���ͷ������ڴ�
	channel.pOutQueue->BeginGet();
	for(int i=0;i<nDiff;i++)
	{
		channel.pOutQueue->GetNext()->clear(this);
	}
	channel.pOutQueue->FreeN(nDiff);
	channel.nOutIdx0	= nIdx0;
}
void	CNL2SlotMan_I_UDP::SlotInChannelMove(CNL2SLOT_I::CHANNEL_T*pChannel, int nNum)
{
	if( nNum<=0 )
	{
		return;
	}
	for(int i=0;i<nNum;i++)
	{
		CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= pChannel->pInQueue->Get(i);
		assert(pInUnit);	// �ƶ��ĸ������ϲ㱣֤�е�
		pInUnit->clear(this);
	}
	pChannel->pInQueue->FreeN(nNum);
	pChannel->nInIdx0	+= nNum;
}
void	CNL2SlotMan_I_UDP::InitChannelCryptor(CNL2SLOT_I *pSlot)
{
	if( pSlot->pIKeyExchanger==NULL )
	{
		return;
	}
	// �����е�safeͨ�������ܳ�ֵ
	const void	*pKey	= pSlot->pIKeyExchanger->GetKey();
	int	nKSize	= pSlot->pIKeyExchanger->GetKeySize();

	for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[i];
		// ֻ��safeͨ�������м��ܣ�RTͨ����LastSafeͨ�������ܽ��������Լ���
		if( channel.nType == CNL2_CHANNELTYPE_SAFE )
		{
			WHSafeSelfDestroy(channel.pICryptorSend);
			channel.pICryptorSend	= m_pICryptFactory->CreateICryptor(pSlot->nCryptorType);
			assert(channel.pICryptorSend);
			channel.pICryptorSend->SetEncryptKey(pKey, nKSize);

			WHSafeSelfDestroy(channel.pICryptorRecv);
			channel.pICryptorRecv	= m_pICryptFactory->CreateICryptor(pSlot->nCryptorType);
			assert(channel.pICryptorRecv);
			channel.pICryptorRecv->SetDecryptKey(pKey, nKSize);
		}
	}
	// ���Key������
	WHSafeSelfDestroy(pSlot->pIKeyExchanger);
}
void	CNL2SlotMan_I_UDP::MakeSlotSeedKeyMD5(CNL2SLOT_I *pSlot, unsigned char *szKeyMD5)
{
	++	pSlot->slotinfo.nSpecialSeed;
	unsigned char	szTmp[WHMD5LEN+sizeof(pSlot->slotinfo.nSpecialSeed)];
	memcpy(szTmp, &pSlot->slotinfo.nSpecialSeed, sizeof(pSlot->slotinfo.nSpecialSeed));
	memcpy(szTmp+sizeof(pSlot->slotinfo.nSpecialSeed), pSlot->szKeyMD5, WHMD5LEN);
	whmd5(szTmp, sizeof(szTmp), szKeyMD5);
}
bool	CNL2SlotMan_I_UDP::VerifySlotSeedKeyMD5(CNL2SLOT_I *pSlot, unsigned int nSeed, unsigned char *szKeyMD5)
{
	if( nSeed<=pSlot->slotinfo.nSpecialSeed )
	{
		return	false;
	}
	unsigned char	szTmp[WHMD5LEN+sizeof(nSeed)];
	memcpy(szTmp, &nSeed, sizeof(nSeed));
	memcpy(szTmp+sizeof(nSeed), pSlot->szKeyMD5, WHMD5LEN);
	unsigned char	szTmpMD5[WHMD5LEN];
	whmd5(szTmp, sizeof(szTmp), szTmpMD5);
	if( memcmp(szTmpMD5, szKeyMD5, WHMD5LEN)!=0 )
	{
		return	false;
	}
	pSlot->slotinfo.nSpecialSeed	= nSeed;
	return	true;
}
int	CNL2SlotMan_I_UDP::SwitchAddr(CNL2SLOT_I *pSlot)
{
	if( wh_tickcount_diff(m_tickNow, m_nLastSwitchAddrTime)<m_nMightDropTimeout )
	{
		// ̫Ƶ����������ת��
		return	0;
	}
	// ������socket�������ڷ���SWITCHADDR
	cmn_safeclosesocket(m_sock);
	struct sockaddr_in	addr;
	m_sock	= udp_create_socket_by_ipnportstr(m_info.szBindAddr, &addr);
	if( m_sock == INVALID_SOCKET )
	{
		// �������socketʧ�ܾ����ˣ�������ʵ�����߰ɣ�
		return	-1;
	}
	m_nMyAddrSum	= ntohs(addr.sin_port);
	// ��¼���ʱ��
	m_nLastSwitchAddrTime	= m_tickNow;
	// ��¼��־
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(807,CNL_RT)"CNL2SlotMan_I_UDP,I switch addr,0x%X,%d,%s", pSlot->slotinfo.nSlot, pSlot->slotinfo.nSpecialSeed, cmn_get_ipnportstr_by_saaddr(&addr));
	return	1;
}
