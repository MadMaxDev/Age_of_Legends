// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CNL2的内部实现
// CreationDate : 2005-04-17
// ChangeLog    : 2005-07-18 SlotChannelConfirm_LastSafe中也增加了RTT计算
//              : 2005-08-03 增加在SlotMan中CloseAll
//              : 2006-05-22 在CNL2SlotMan_I_UDP::SetICryptFactory中增加了初始化随机种子的代码。原来服务器和客户端产生的pubdata都是相同的，呵呵。
//              : 2007-08-09 在SlotChannelConfirm_Safe中增加了收到后面的包的回应会立即发出前面的包的机制。
//              : 2007-12-29 添加了AddSlot和RemoveSlot的日志，主要为了插CAAFS中有slot无辜消失的问题

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
	nTimeEventID.quit();								// 强制退出时间事件队列
	nTimeEventID.clear();								// 这个考虑一下是否要取消
	nRSCount	= 1;									// 记住这个默认必须是1，因为第一次resend需要是2*nRTT之后
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
			// 相当于没有初始化
		break;
		default:
		{
			if( pInfo->nInNum>CNL2_MAX_INOUTQUEUESIZE
			||  pInfo->nOutNum>CNL2_MAX_INOUTQUEUESIZE
			)
			{
				// 不能申请这么大的缓冲
				assert(0);
				return	-1;
			}
			if( nType == CNL2_CHANNELTYPE_LASTSAFE )
			{
				// 以最后发送的为准，那么就只需要一个单元作缓冲
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
		// 删除每个单元分配的额外内存
		pOutQueue->BeginGet();
		OUTUNIT_T	*pOutUnit;
		while( (pOutUnit=pOutQueue->GetNext()) != NULL )
		{
			pOutUnit->clear(pSlotMan);
		}
		// 删除队列
		pOutQueue->Release();
		delete	pOutQueue;
		pOutQueue	= NULL;
	}
	if(pInQueue)
	{
		// 删除每个单元分配的额外内存
		pInQueue->BeginGet();
		INUNIT_T	*pInUnit;
		while( (pInUnit=pInQueue->GetNext()) != NULL )
		{
			pInUnit->clear(pSlotMan);
		}
		// 删除队列
		pInQueue->Release();
		delete	pInQueue;
		pInQueue	= NULL;
	}
	nodeHavingData.leave();
	nodeHavingDataToSend.leave();

	WHSafeSelfDestroy(pICryptorSend);
	WHSafeSelfDestroy(pICryptorRecv);

	// 应该在channel被释放之前清理所有的内容
	whDList<CMDPACKUNIT_T>::node	*pNode	= dlistCmdPack.begin();
	while( pNode!=dlistCmdPack.end() )
	{
		pSlotMan->FreeData(pNode->data.pCmd);
		pNode->leave();
		pSlotMan->FreeData(pNode);
		// 下一个
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
	// 指针后移
	pData					= wh_getptrnexttoptr(pConfirm);
	// 从nInIdxConfirm开始遍历InQueue填写连续段
	int	nStart				= idxdiff(nInIdxConfirm, nInIdx0);
	pInQueue->BeginGet(nStart);
	INUNIT_T				*pInUnit;
	bool					bGot	= false;
	unsigned short			nCount	= 0;
	// 这里面不需要判断bGot==true段的个数是否超过MAXSECTNUM，因为对方收到会自动断线的
	while( (pInUnit=pInQueue->GetNext()) != NULL )
	{
		if( bGot == (pInUnit->pData != NULL) )
		{
			// 说明和上一个的状况一致
			nCount			++;
		}
		else
		{
			// 状况跳转，从无到有或从有到无
			_vn_set(nCount, pData, nSize);
			bGot			= !bGot;
			nCount			= 1;
		}
	}
	if( nCount>0 )
	{
		// 最后一个(如果nCount非0则这个一定是got的，如果不是那一定是我逻辑上写错了)
		assert( bGot );
		_vn_set(nCount, pData, nSize);
	}
	// 0结尾
	_vn_set(0, pData, nSize);
	return	nSize;
}
size_t	CNL2SLOT_I::CHANNEL_T::MakeInQueueConfirm_LastSafe(void *pData)
{
	CNL2_CMD_DATA_CONFIRM_T::CONFIRM_T	*pConfirm	= (CNL2_CMD_DATA_CONFIRM_T::CONFIRM_T *)pData;
	pConfirm->nInIdxConfirm	= nInIdxConfirm;
	pConfirm->nInIdx0		= nInIdx0;
	size_t	nSize			= sizeof(*pConfirm);
	// 指针后移
	pData					= wh_getptrnexttoptr(pConfirm);
	// 0结尾
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
	// 自动列表
	for(i=0;i<DLISTIDX_MAX;i++)
	{
		dlistNode[i].leave();
	}
	// 时间事件
	ClearAllTE();
	//
	for(i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		channel[i].Release();
	}
	// 释放内存
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

	// 预备工作
	assert(m_sock == INVALID_SOCKET);

	int	rst;
	if( (rst=ReConfigChannel(pInfo))<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1,CNL_INT_RLS)"CNL2SlotMan_I_UDP::Init,%d,ReConfigChannel", rst);
		return	-1;
	}

	// 正式工作
	memcpy(&m_info, pInfo, sizeof(m_info));

	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_info.nTQChunkSize;
	if( m_tq.Init(&tqinfo)<0 )
	{
		return	-4;
	}
	// 开始CA定时(这个必须在m_tq初始化之后才行)
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

	// 根据配置设置一些相关参数
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

	// 允许地址列表
	// 地址允许禁止列表
	if( m_info.szConnectAllowDeny[0] )
	{
		// 文件不存在其实就相当于没有任何policy
		m_ConnectAllowDeny.initfromfile(m_info.szConnectAllowDeny);
		m_fcdAllowDeny.SetFile(m_info.szConnectAllowDeny);
		// 设置检查间隔
		Set_TEDeal_AllowDenyFileChangeCheck();
	}

	// 先设置一个合理的值
	m_nLastSwitchAddrTime	= wh_gettickcount();

	// 初始化流量统计（定时统计）
	Set_TEDeal_StatInterval();

	return	0;
}
int		CNL2SlotMan_I_UDP::Init_SetParams()
{

	// 设置socket的底层缓冲区尺寸
	if( m_info.nSockSndBufSize )
	{
		cmn_setsockopt_int(m_sock, SOL_SOCKET, SO_SNDBUF, m_info.nSockSndBufSize);
	}
	if( m_info.nSockRcvBufSize )
	{
		cmn_setsockopt_int(m_sock, SOL_SOCKET, SO_RCVBUF, m_info.nSockRcvBufSize);
	}

	// 保留这么大是为了以免RT数据以及单独的safe数据会发很大
	if( m_info.nMaxSinglePacketSize>CNL2_MAX_DATATOSEND )
	{
		m_info.nMaxSinglePacketSize	= CNL2_MAX_DATATOSEND;
	}
	m_vectrawbuf_recv.reserve(m_info.nMaxSinglePacketSize);
	m_vectrawbuf.reserve(m_info.nMaxSinglePacketSize);
	m_vectpackbuf.reserve(m_info.nMaxSinglePacketSize);

	// 计算重发的间隔
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
	// 正式工作
	memcpy(&m_info, pInfo, sizeof(m_info));

	return	0;
}
int		CNL2SlotMan_I_UDP::ReConfigChannel(INFO_T *pInfo)
{
	// 参数检查
	// 保证通道0为可靠通道
	if( pInfo->channelinfo[0].nType != CNL2_CHANNELTYPE_SAFE )
	{
		assert(0);
		return	-1;
	}
	// 保证原来的slot都已经关闭
	if( m_Slots.size()>0 )
	{
		assert(0);
		return	-2;
	}
	// 保证所有队列长度不能超过65536
	for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		switch( pInfo->channelinfo[i].nType )
		{
			case	CNL2_CHANNELTYPE_NOTHING:
				// 什么也不做
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
				// 如果该通道不支持Send
				if( (pInfo->channelinfo[i].nProp & CNL2CHANNELINFO_T::PROP_CANSEND) == 0 )
				{
					m_Send_DATA[i]						= &CNL2SlotMan_I_UDP::Send_DATA_NotSupport;
				}
				// 如果该通道不支持Recv
				if( (pInfo->channelinfo[i].nProp & CNL2CHANNELINFO_T::PROP_CANRECV) == 0 )
				{
					m_TickRecv_CMD_Others_DATA_Deal[i]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_NotSupport;
				}
			}
			break;
		}
	}

	// 拷贝通道信息
	memcpy(&m_info.channelinfo, &pInfo->channelinfo, sizeof(m_info.channelinfo));

	return	0;
}
int		CNL2SlotMan_I_UDP::Release()
{
	// 关闭
	Listen(false);
	CloseAll();
	// 等待一会nCloseTimeout*2;
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

	// 直接调用所有slot的clear
	whunitallocatorFixed_AllClear(m_Slots);

	// 还没有断线的连接就让他们自己超时吧
	m_Slots.Release();

	// 清空时间队列(这个需要在CloseAll之后做)
	m_tq.Release();
	m_queueControlOut.Release();

	cmn_safeclosesocket(m_sock);
	// 清空地址映射
	m_mapaddr2slot.clear();

	// 小内存分配器释放
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
	// 对本tick内累积的需要打包的东西进行打包
	TickLogic_PackAndCompression();
m_dtu.Mark();
	// 时间队列(里面包括数据真正的Send、Resend)
	TickLogic_TE();
m_dtu.Mark();
	// 看是否有需要Confirm的
	_TickLogic_DealSlotInDList(CNL2SLOT_I::DLISTIDX_COFIRM, &CNL2SlotMan_I_UDP::_DealSlotInDList_Confirm);
m_dtu.Mark();
	// 看是否有需要NOOP的
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
	// 浏览累积队列
	for(pNode=m_dlistChannelHaveDataToSend.begin();pNode!=m_dlistChannelHaveDataToSend.end();pNode=pNode->next)
	{
	
		

		m_pSlot		= m_Slots.getptr(pNode->data.nSlot);
		if( !m_pSlot )
		{
			// 应该是slot在本tick刚好被删除了
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
				// 只有一个，则按照普通的方式发送
				whDList<CMDPACKUNIT_T>::node	*pNode	= m_pChannel->dlistCmdPack.begin();
				_Send_DATA_Safe(pNode->data.pCmd, pNode->data.nSize, 0);
				FreeAndClearData(pNode->data.pCmd);
				pNode->leave();
				FreeData(pNode);
			}
			else
			{
				// 超过一个，则按照打包的方式发送
				// 先打包
				// 用于打包的缓冲（一条打包后的指令最长不能超过CNL的最大发送长度）
				vectbuf.resize(m_info.nMaxSinglePacketSize);
				whcmdshrink		wcs;
				wcs.SetShrink(vectbuf.getbuf(), vectbuf.size());
				whDList<CMDPACKUNIT_T>::node	*pNode	= m_pChannel->dlistCmdPack.begin();
				while( pNode!=m_pChannel->dlistCmdPack.end() )
				{
					
					if( wcs.WriteVData(pNode->data.pCmd, pNode->data.nSize)<0 )	// 应该是长度不够
					{
						// 把缓冲加大

						vectbuf.resize(vectbuf.size()+pNode->data.nSize+4);
						wcs.SetShrinkResize(vectbuf.getbuf(), vectbuf.size());
						if( wcs.WriteVData(pNode->data.pCmd, pNode->data.nSize)<0 )
						{
							
							// 如果还小于0就有问题了
							assert(0);
						}
#ifdef _DEBUG
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(000,CLSDEBUG)"CNL2SlotMan_I_UDP::TickLogic_PackAndCompression,resize:%d",vectbuf.size());
#endif
					}
					FreeAndClearData(pNode->data.pCmd);
					pNode->leave();
					FreeData(pNode);
					// 下一个
					pNode	= m_pChannel->dlistCmdPack.begin();
				}
				wcs.WriteEnd();
				// 再发送
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
	// 清空累积队列
	m_dlistChannelHaveDataToSend.clear();
	return	0;
}

int		CNL2SlotMan_I_UDP::_TickLogic_DealSlotInDList(int nIdx, DEALSLOT_T func)
{
	// 遍历队列
	whDList<CNL2SLOT_I *>		&DL		= m_dlistSlot[nIdx];
	whDList<CNL2SLOT_I *>::node	*pNode	= DL.begin();
	whDList<CNL2SLOT_I *>::node	*pCur;
	while(pNode!=DL.end())
	{
		pCur	= pNode;
		pNode	= pNode->next;
		// 处理
		(this->*func)(pCur->data);
		// 老的离队
		pCur->leave();
	}
	return	0;
}
void	CNL2SlotMan_I_UDP::_DealSlotInDList_Confirm(CNL2SLOT_I *pSlot)
{
	// 如果全部channel都已经confirm了则什么也不作
	if( pSlot->nPeerNotConfirmedChannelMask==0 )
	{
		return;
	}
	// 发送对应各个channel的confirm
	CNL2_CMD_DATA_CONFIRM_T	*pCmd	= (CNL2_CMD_DATA_CONFIRM_T *)m_vectrawbuf.getbuf();
	pCmd->cmd			= CNL2_CMD_DATA_CONFIRM;
	pCmd->slotidx		= pSlot->slotinfo.nPeerSlotIdx;
	pCmd->channelmask	= pSlot->nPeerNotConfirmedChannelMask;
	size_t				nSize		= sizeof(*pCmd);
	void				*pData		= wh_getptrnexttoptr(pCmd);
	// 后续数据
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
					// 可能是因为定义了CNL2_RECV_TRIGER_CONFIRM造成的
					whbit_uchar_clr(&pCmd->channelmask, i);
					continue;
				break;
			}
			pData	= wh_getoffsetaddr(pData, nAddSize);
			nSize	+= nAddSize;
		}
	}
	// 发送数据
	pCmd->calcsetcrc(nSize);
	slot_send(pSlot, pCmd, nSize);
	// 清空
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
	// 发送数据
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
	// 添加
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
		Close(pSlot);	// 这个不会导致slot被remove，所以可以在iterator循环中这样调用
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
		// 已经在关闭状态
		return;
	}
	// 如果bClosePassive为false则是主动关闭
	pSlot->bClosePassive	= bClosePassive;
	pSlot->slotinfo.nStatus	= CNL2SLOTINFO_T::STATUS_CLOSING;
	// 获取当前时间后面注册重发事件时需要
	maketicknow();
	// 清除正常工作的时间事件和各个channel相关的时间事件
	ClearSlotAllTEAndChannelAllTE(pSlot);
	// 设置关闭超时
	RegCloseTQ(pSlot);
	if( !bClosePassive )
	{
		// 主动关闭
		// 设置重发超时
		RegCloseRSTQ(pSlot);
		// 发送关闭指令
		SendClose(pSlot);
		// 计数增加(这样在收到对方回应的时候就会知道可以结束了)
		pSlot->nCloseCount				= 1;
	}
	else
	{
		// 被动关闭(不需要定时重发，只需要收到对方的CLOSE后做回应即可)
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
	// 初始化随机种子（m_pICryptFactory可能为空，空说明工厂还没有建好，可能今后建好）
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
	// 原来一直没有设置这个(2006-02-05)
	maketicknow();

	*pnSlot	= 0;
	// 检查是否地址已经存在
	if( m_mapaddr2slot.has(addr) )
	{
		return	ADDSLOT_RST_HASADDR;
	}
	// 创建slot
	int				nSlot	= m_Slots.AllocUnit(m_pSlot);
	if( nSlot<0 )
	{
		// 没有地方了
		return	ADDSLOT_RST_SLOTFULL;
	}
	// 初始化
	m_pSlot->clear();			// 为了保险还是要初始化一下(比如lastsend/lastrecv时间就不需要在这时初始化)
	m_pSlot->slotinfo.nStatus	= nStatus;
	m_pSlot->slotinfo.nSlot		= nSlot;
	m_pSlot->slotinfo.peerAddr	= addr;
	m_pSlot->slotinfo.nPeerSlotIdx	= nPeerSlotIdx;
	m_pSlot->SetSlotMan(this);	// 这个必须放在slotinfo.nSlot被设置之后
	// 为了保险这个需要先假定一个初值
	m_pSlot->slotinfo.nLastRecv	= m_tickNow;
	// 创建密钥交换器(无论如何都要设置这个)
	m_pSlot->pIKeyExchanger		= m_pICryptFactory->CreateIKeyExchanger(nKeyExchangerType);;
	if( m_pSlot->pIKeyExchanger == NULL )
	{
		return	ADDSLOT_RST_KEYEXCHANGER_INITERR;
	}
	if( m_pSlot->pIKeyExchanger->MakeOwnData() < 0 )
	{
		return	ADDSLOT_RST_KEYEXCHANGER_INITERR;
	}
	// 设置地址映射
	m_mapaddr2slot.put(m_pSlot->slotinfo.peerAddr, nSlot);
	// 设置函数指针
	FillSlotWithDefaultTickRecvDealFunc(m_pSlot);
	// 附加数据(必须要在下面的发送之前设置)
	m_pSlot->vectExtSlotData.resize(nExtSize);
	if( nExtSize>0 )
	{
		memcpy(m_pSlot->vectExtSlotData.getbuf(), pExtData, nExtSize);
	}
	try
	{
		// 初始化，设置通道
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
				// 这个函数和Accept的不同
				m_pSlot->dealfuncRecv[CNL2_CMD_DATA]		= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA;
				m_pSlot->dealfuncRecv[CNL2_CMD_NOOP]		= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP;
				// 设置连接超时事件
				TQUNIT_T					tqu;
				tqu.pSlot					= m_pSlot;
				tqu.tefunc					= &CNL2SlotMan_I_UDP::TEDeal_Connect;
				if( m_tq.Add(m_tickNow+m_info.nConnectTimeout, &tqu, &m_pSlot->teid[CNL2SLOT_I::TE_IDX_CONNECT])<0 )
				{
					throw	(int)ADDSLOT_RST_MEMERR;
				}
				// 发送指令
				SendConnectAndRegTQ(m_pSlot);
			}
			break;
			case	CNL2SLOTINFO_T::STATUS_ACCEPTING:
			{
				m_pSlot->bIsClient			= false;
				// 随机获得一种加密方式
				WHRANGE_T<int>	r(m_info.nCryptorType1, m_info.nCryptorType2);
				m_pSlot->nCryptorType		= r.GetRand();
				// 交换密钥
				if( m_pSlot->pIKeyExchanger->Agree1(pPubData, nPubSize)<0 )
				{
					throw	(int)ADDSLOT_RST_KEYEXCHANGER_INITERR;
				}
				// 计算Key的MD5
				whmd5(m_pSlot->pIKeyExchanger->GetKey(), m_pSlot->pIKeyExchanger->GetKeySize(), m_pSlot->szKeyMD5);
				// 设置Accept超时
				TQUNIT_T					tqu;
				tqu.pSlot					= m_pSlot;
				tqu.tefunc					= &CNL2SlotMan_I_UDP::TEDeal_Accept;
				if( m_tq.Add(m_tickNow+m_info.nConnectTimeout, &tqu, &m_pSlot->teid[CNL2SLOT_I::TE_IDX_ACCEPT])<0 )
				{
					throw	(int)ADDSLOT_RST_MEMERR;
				}
				// 发送agree并设置重发超时
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
	// 删除时间事件和各个channel相关的时间事件
	ClearSlotAllTEAndChannelAllTE(pSlot);
	// 清除地址映射
	m_mapaddr2slot.erase(pSlot->slotinfo.peerAddr);
	// 清空并删除slot单元
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
	// 发送(+1是因为需要有一个字节记录PubData的长度)
	m_vectrawbuf.resize(sizeof(CNL2_CMD_CONNECT_T) + pSlot->pIKeyExchanger->GetPubSize()+sizeof(CONNECT_PUBKEY_LEN_T) + pSlot->vectExtSlotData.size()+sizeof(CONNECT_EXT_LEN_T));
	CNL2_CMD_CONNECT_T	&Cmd	= *(CNL2_CMD_CONNECT_T *)m_vectrawbuf.getbuf();
	Cmd.cmd				= CNL2_CMD_CONNECT;
	Cmd.ver				= m_info.ver;
	Cmd.clientslotidx	= m_Slots.getrealidx(pSlot->slotinfo.nSlot);
	void	*pExt		= wh_getptrnexttoptr(&Cmd);
	// 注意：因为密钥可能大于256字节，所以用可变的数据（为了统一所以原本的附加数据也直接用可变数据了）
	pExt				= wh_getoffsetaddr(pExt, whvldata_write(pSlot->pIKeyExchanger->GetPubData(), pSlot->pIKeyExchanger->GetPubSize(), (CONNECT_PUBKEY_LEN_T *)pExt));
//	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "临时日志 Connect PubData size:%d data:%s", pSlot->pIKeyExchanger->GetPubSize(), wh_hex2str((const unsigned char*)pSlot->pIKeyExchanger->GetPubData(), pSlot->pIKeyExchanger->GetPubSize()));
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
	// 发送(+sizeof(CONNECT_PUBKEY_LEN_T)是因为需要记录PubData的长度)
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
//	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "临时日志 Agree PubData size:%d data:%s", pSlot->pIKeyExchanger->GetPubSize(), wh_hex2str((const unsigned char*)pSlot->pIKeyExchanger->GetPubData(), pSlot->pIKeyExchanger->GetPubSize()));
	// 现在pExt应该指向刚好没有数据的地方，不过没什么用，先这样吧
	Cmd.calcsetcrc(m_vectrawbuf.size());
	if( slot_send(pSlot, m_vectrawbuf.getbuf(), m_vectrawbuf.size())<0 )
	{
		return;
	}
	return;
}
void	CNL2SlotMan_I_UDP::SendClose(CNL2SLOT_I *pSlot)
{
	// 发送
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
	// 原来是这样写的
	//m_tq.Add(pSlot->slotinfo.nLastSend+m_nNoopTimeout, &tqu, &pSlot->teid[CNL2SLOT_I::TE_IDX_NOOP]);
	// 这样因为Noop实际上还没有导致发送动作，所以pSlot->slotinfo.nLastSend依旧很小，结果时间队列死锁
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
	pSlot->teid[CNL2SLOT_I::TE_IDX_SWITCHADDRREQ].quit();	// 保险起见（因为在断线过程中也许会导致生成多次Req，所以每次一次一定要确保把原来的清除掉）
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
		// 应该是很严重的问题了，怎么溢出呢？记录日志!
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(4,CNL_RT)"CNL2SlotMan_I_UDP::ControlOutAlloc,,m_queueControlOut.InAlloc,%u", nSize);
	}
	return		ptr;
}
void	CNL2SlotMan_I_UDP::ControlOutReasonAndRemoveSlot(CNL2SLOT_I *pSlot, int nCmd, int nReason)
{
	// 发送指令给上层
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
	// 移除
	RemoveSlot(pSlot);
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	(this->*m_SCC[nChannel])(pSlot, nChannel, nIdx, nNum);
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm_NotSupport(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	// 直接关闭
	CloseOnErr(pSlot, CLOSEONERR_SlotChannelConfirm_NotSupport);
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm_RT(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	// RT不应该有Confirm
	CloseOnErr(pSlot, CLOSEONERR_SlotChannelConfirm_RT);
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm_Safe(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[nChannel];
	if( channel.pOutQueue->size() == 0 )
	{
		// 没有必要校验了，因为没有数据
		return;
	}
	bool	bChanged	= false;
	if( nNum==0 )
	{
again:
		// 判断是否是新的确认数据
		if( idxdiff(nIdx, channel.nOutIdxConfirm)<=0 )
		{
			// 不是新的，直接返回
			return;
		}
		if( idxdiff(nIdx, channel.nOutIdx0)>(int)channel.pOutQueue->size() )
		{
			// 数据超界，hacker
			CloseOnErr(pSlot, CLOSEONERR_IdxOutOfRange);
			return;
		}
		// 上次校验的位置到这个idx都被确认。同时清除相应的重发事件
		channel.pOutQueue->BeginGet( idxdiff(channel.nOutIdxConfirm, channel.nOutIdx0) );
		while( channel.nOutIdxConfirm!=nIdx )
		{
			bChanged	|= _SlotChannelConfirm_Safe_SetConfirm(channel.pOutQueue->GetNext());
			channel.nOutIdxConfirm	++;
		}
	}
	else
	{
		// 看看这段是否值得校验
		cnl2idx_t	nLast	= nIdx + nNum;
		int			nDiff	= idxdiff(nLast, channel.nOutIdxConfirm);
		if( nDiff<=0 )
		{
			// 没有意义的数据(太小了)
			return;
		}
		nDiff	= idxdiff(nLast, channel.nOutIdx0);
		if( nDiff>(int)channel.pOutQueue->size() )
		{
			// 数据超界，hacker
			CloseOnErr(pSlot, CLOSEONERR_IdxOutOfRange);
			return;
		}
		nDiff	= idxdiff(nIdx, channel.nOutIdxConfirm);
		if( nDiff<=0 )
		{
			// 起始位置在已有确认位置之前
			// 按照连续的进行
			nIdx	= nLast;
			goto	again;
		}
		// 把前面的所有没发出的包在本tick立即发出（2007-08-14 修正，原来是把所有的包不管有没有确认都发了）
		channel.pOutQueue->BeginGet( idxdiff(channel.nOutIdxConfirm, channel.nOutIdx0) );
		while( nDiff-- > 0 )	// 记住，不能是大于等于，因为这是要把之前的，不含这个的发送一下
		{
			CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= channel.pOutQueue->GetNext();
			if( pOutUnit->nTimeEventID.IsValid() )
			{
				// pOutUnit->nRSCount	= 1;
				// 最多延迟一个RTT就发出
				SendOutUnitWithDelay(pSlot, pOutUnit, pSlot->slotinfo.nRTT);
			}
		}
		// 之后的确认
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
		// 说明已经被确认过了
		return	false;
	}
	// 计算Ping
	m_pSlot->slotinfo.calcRTT(wh_tickcount_diff(m_tickNow, pOutUnit->nSendTime));
	// 取消相应的时间事件
	pOutUnit->nTimeEventID.quit();
	return		true;
}
void	CNL2SlotMan_I_UDP::SlotChannelConfirm_LastSafe(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum)
{
	CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[nChannel];
	if( channel.pOutQueue->size() == 0 )
	{
		// 没有必要校验了，因为没有数据
		return;
	}
	// nNum永远忽略
	if( nNum!=0 )
	{
		CloseOnErr(pSlot, CLOSEONERR_IdxOutOfRange);
		return;
	}
	// 有效的
	int	nDiff	= idxdiff(nIdx, channel.nOutIdxConfirm);
	if( nDiff!=1 )
	{
		// 太老的确认包或者hack来的包
		return;
	}
	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= channel.pOutQueue->GetFirst();
	// 计算ping
	m_pSlot->slotinfo.calcRTT(wh_tickcount_diff(m_tickNow, pOutUnit->nSendTime));
	// 取消这个单元
	pOutUnit->clear(this);
	// 队列清空
	channel.pOutQueue->Clean();
	// 索引后移
	channel.nOutIdx0		++;
	channel.nOutIdxConfirm	= channel.nOutIdx0;
}
void	CNL2SlotMan_I_UDP::SlotOutChannelMove(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx0)
{
	CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[nChannel];
	int		nDiff	= idxdiff(nIdx0, channel.nOutIdx0);
	if( nDiff <= 0 )
	{
		// 需要太小，可能是很老的包延迟过来了
		return;
	}
	if( nDiff > channel.pOutQueue->size() )
	{
		// 应该是不可能的，不应该要求释放还没有放出的数据
		CloseOnErr(pSlot, CLOSEONERR_IdxOutOfRange);
		return;
	}
	// 循环释放所有内存
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
		assert(pInUnit);	// 移动的个数是上层保证有的
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
	// 给所有的safe通道赋加密初值
	const void	*pKey	= pSlot->pIKeyExchanger->GetKey();
	int	nKSize	= pSlot->pIKeyExchanger->GetKeySize();

	for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		CNL2SLOT_I::CHANNEL_T	&channel	= pSlot->channel[i];
		// 只有safe通道才能有加密，RT通道和LastSafe通道都不能进行连续性加密
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
	// 清除Key交换器
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
		// 太频繁，还不用转换
		return	0;
	}
	// 生成新socket，并定期发送SWITCHADDR
	cmn_safeclosesocket(m_sock);
	struct sockaddr_in	addr;
	m_sock	= udp_create_socket_by_ipnportstr(m_info.szBindAddr, &addr);
	if( m_sock == INVALID_SOCKET )
	{
		// 如果生成socket失败就算了（接收事实，断线吧）
		return	-1;
	}
	m_nMyAddrSum	= ntohs(addr.sin_port);
	// 记录这次时间
	m_nLastSwitchAddrTime	= m_tickNow;
	// 记录日志
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(807,CNL_RT)"CNL2SlotMan_I_UDP,I switch addr,0x%X,%d,%s", pSlot->slotinfo.nSlot, pSlot->slotinfo.nSpecialSeed, cmn_get_ipnportstr_by_saaddr(&addr));
	return	1;
}
