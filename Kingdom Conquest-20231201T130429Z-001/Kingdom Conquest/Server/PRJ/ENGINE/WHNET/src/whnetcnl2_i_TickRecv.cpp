// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i_TickRecv.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CNL2的内部实现
// CreationDate : 2005-04-17
// ChangeLog    : 2005-07-13 cmn_getsizetorecv可能会导致数据很久以后才能被收到
//                2005-07-14 修正了原来连接后Client发出的NOOP丢失而导致连接后立刻发出的数据不能被服务器处理的错误
//                增加了内部函数_TickRecv_FIRST_CMD_ON_ACCEPTING，在NOOP_ON_ACCEPTING和DATA_ON_ACCEPTING中都使用它来转换状态
//                2006-02-05 修正了TickRecv_CMD_Others_DATA_Deal_Safe中收发包顺序不同导致解密顺序出错的BUG。

#include "../inc/whnetcnl2_i.h"
#include <WHCMN/inc/whbits.h>

using namespace	n_whnet;
using namespace	n_whcmn;

int		CNL2SlotMan_I_UDP::TickRecv()
{
	maketicknow();

	//while( (m_nCmdSize=cmn_getsizetorecv(m_sock)) > 0 )
	while( cmn_select_rd(m_sock, 0) > 0 )
	{
		m_pCmd			= (CNL2_CMD_0_T *)m_vectrawbuf.getbuf();
		if( (m_nCmdSize=udp_recv(m_pCmd, m_vectrawbuf.capacity(), &m_addr))<0 )
		{
			break;
		}
		if( m_nCmdSize<=0 )
		{
			// 可能是由于网卡瞬间错误，或者调试过程造成的
#ifdef	_WH_LOWLEVELDEBUG
			assert(0);
#endif
			continue;
		}
		// 校验CRC
		if( !m_pCmd->checkcrc(m_nCmdSize) )
		{
#ifdef	_WH_LOWLEVELDEBUG
			// 取消这个吧，如果是服务器，而别人发送了这个就回导致debug版的服务器宕掉的
			// assert(0);
#endif
			continue;
		}

		m_vectrawbuf.resize(m_nCmdSize);
		// 只有一个判断这样效率比用函数数组高(其实6个判断以内系统都会优化的比较好，我只测试到6个)
		switch( m_pCmd->cmd )
		{
		case	CNL2_CMD_EXTEND:
			{
				TickRecv_CMD_EXTEND();
			}
			break;
		case	CNL2_CMD_CONNECT:
			{
				TickRecv_CMD_CONNECT();
			}
			break;
		default:
			{
				TickRecv_CMD_Others();
			}
			break;
		}
	}
	return	0;
}
void	CNL2SlotMan_I_UDP::TickRecv_CMD_EXTEND()
{
	CNL2_CMD_EXTEND_T			*pExt	= (CNL2_CMD_EXTEND_T *)m_pCmd;
	switch( pExt->subcmd )
	{
	case	CNL2_CMD_EXTEND_T::CMD_ECHO:
		{
			// 改了指令内容其他的原样返回
			pExt->subcmd		= CNL2_CMD_EXTEND_T::CMD_ECHO_RST;
			pExt->calcsetcrc(m_nCmdSize);
			udp_sendto(pExt, m_nCmdSize, &m_addr);
		}
		break;
	default:
		{
			// 不认识的就不用处理了
		}
		break;
	}
}
void	CNL2SlotMan_I_UDP::TickRecv_CMD_CONNECT()
{
	// 看看是否在listen
	if( !m_bListen )
	{
		// 直接忽略，让对方超时去吧
		return;
	}
	// 注：这里面的发送不能用slot_send，因为这里面的都是错误发送，slot应该还不存在
	// 检查是否地址已经存在
	if( m_mapaddr2slot.has(m_addr) )
	{
		// 已经存在的连接，不用理会这个指令
		return;
	}
	// 看看是否不在允许IP范围
	if( !m_ConnectAllowDeny.isallowed(m_addr) )
	{
		// 不用返回任何提示
		return;
	}
	CNL2_CMD_CONNECT_T			*pCmdConnect	= (CNL2_CMD_CONNECT_T *)m_pCmd;
	CNL2_CMD_CONNECT_REFUSE_T	CmdRefuse;
	CmdRefuse.cmd				= CNL2_CMD_CONNECT_REFUSE;
	CmdRefuse.channel			= 0;
	CmdRefuse.slotidx			= pCmdConnect->clientslotidx;
	// 判断版本
	if( m_info.ver != pCmdConnect->ver )
	{
		CmdRefuse.reason		= CONNECTREFUSE_REASON_BADVER;
		CmdRefuse.calcsetcrc(sizeof(CmdRefuse));
		udp_sendto(&CmdRefuse, sizeof(CmdRefuse), &m_addr);
		return;
	}
	// 分离出各部分数据部分
	const void	*pPubForServer=NULL, *pExtData=NULL;
	int		nPubForServerSize=0, nExtData=0;
	void	*pRear		= wh_getptrnexttoptr(pCmdConnect);
	int		nRearSize	= m_nCmdSize - sizeof(*pCmdConnect);

	try
	{
		// 注意：因为密钥可能大于256字节，所以要用short，附加数据不能超过255字节，所以用char
		nPubForServerSize	= whvldata_read_ref((CONNECT_PUBKEY_LEN_T*)pRear, pPubForServer);
		if( nPubForServerSize<=0 )
		{
			throw	1;
		}
		nRearSize	-= sizeof(CONNECT_PUBKEY_LEN_T)+nPubForServerSize;
		if( nRearSize<0 )
		{
			throw	2;
		}
		pRear		= wh_getoffsetaddr(pRear, sizeof(CONNECT_PUBKEY_LEN_T)+nPubForServerSize);
		nExtData	= whvldata_read_ref((CONNECT_EXT_LEN_T*)pRear, pExtData);
		nRearSize	-= sizeof(CONNECT_EXT_LEN_T)+nExtData;
		if( nRearSize<0 )
		{
			throw	11;
		}
	}
	catch( int nErr )
	{
		// 应该是hack的数据
		CmdRefuse.reason	= CONNECTREFUSE_REASON_BADDATA;
		CmdRefuse.calcsetcrc(sizeof(CmdRefuse));
		udp_sendto(&CmdRefuse, sizeof(CmdRefuse), &m_addr);
		GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(7,CNL_RT)"CNL2SlotMan_I_UDP::TickRecv_CMD_CONNECT,%d,BAD REQUEST,%s", nErr, cmn_get_ipnportstr_by_saaddr(&m_addr));
		return;
	}

	// slot加入man（AddSlot里面会记录LastRecv为当前时刻）
	int	nSlot;
	int	rst	= AddSlot(m_addr, CNL2SLOTINFO_T::STATUS_ACCEPTING, pCmdConnect->clientslotidx, pExtData, nExtData, &nSlot, pPubForServer, nPubForServerSize, m_nMyAddrSum);
	if( rst<0 )
	{
		CmdRefuse.reason			= CONNECTREFUSE_REASON_INTERNALERROR;
		switch( rst )
		{
			case	ADDSLOT_RST_SLOTFULL:
				CmdRefuse.reason	= CONNECTREFUSE_REASON_SLOTFULL;
			break;
		}
		// 返回错误
		CmdRefuse.calcsetcrc(sizeof(CmdRefuse));
		udp_sendto(&CmdRefuse, sizeof(CmdRefuse), &m_addr);
		return;
	}
}
void	CNL2SlotMan_I_UDP::TickRecv_CMD_SWITCHADDR()
{
	CNL2_CMD_SWITCHADDR_T	*pCmdSA	= (CNL2_CMD_SWITCHADDR_T *)m_pCmd1;
	//
	if( m_pSlot->bIsClient )
	{
		if( pCmdSA->nSubCmd == CNL2_CMD_SWITCHADDR_T::SUBCMD_ORDER )
		{
			if( SwitchAddr(m_pSlot)<0 )
			{
				return;
			}
			// 发送转换请求
			SendSwitchAddrReq(m_pSlot);
		}
		else
		{
			// 说明这个是对方已经收到了请求，我就不用继续发送改变地址指令了
			// 然后把所有channel需要重发的指令在这个tick内全部重发
			SendAllUnConfirmedDataThisTick(m_pSlot);
		}
		// 更新最后一次收到数据的时间
		m_pSlot->slotinfo.nLastRecv	= m_tickNow;
	}
	else
	{
		if( pCmdSA->nSubCmd == CNL2_CMD_SWITCHADDR_T::SUBCMD_REQ )
		{
			// 比较一下MD5是否正确
			if( !VerifySlotSeedKeyMD5(m_pSlot, pCmdSA->nSeed, pCmdSA->szKeyMD5) )
			{
				return;
			}
			// 更新最后一次收到数据的时间
			m_pSlot->slotinfo.nLastRecv	= m_tickNow;

			// 立即发送回应
			CNL2_CMD_SWITCHADDR_T	CmdSA;
			CmdSA.cmd				= CNL2_CMD_SWITCHADDR;
			CmdSA.nSubCmd			= CNL2_CMD_SWITCHADDR_T::SUBCMD_RPL;
			CmdSA.nSeed				= (unsigned short)wh_time();
			wh_randhex(CmdSA.szKeyMD5, sizeof(CmdSA.szKeyMD5));		// 这个只是为了迷惑敌人
			CmdSA.calcsetcrc(sizeof(CmdSA));
			udp_sendto(&CmdSA, sizeof(CmdSA), &m_addr);
			if( m_addr != m_pSlot->slotinfo.peerAddr )
			{
				// 地址和原来不同，需要切换一下（否则就说明是原来已经收到过的包了）
				// 需要先记录一下日志
				char	szIP1[WHNET_MAXADDRSTRLEN];
				char	szIP2[WHNET_MAXADDRSTRLEN];
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(798,CNL_RT)"CNL2SlotMan_I_UDP,peer switch addr,0x%X,%d,%s,%s", m_pSlot->slotinfo.nSlot, CmdSA.nSeed, cmn_get_ipnportstr_by_saaddr(&m_pSlot->slotinfo.peerAddr, szIP1) , cmn_get_ipnportstr_by_saaddr(&m_addr, szIP2));
				// 删除旧地址
				m_mapaddr2slot.erase(m_pSlot->slotinfo.peerAddr);
				// 设置新地址映射
				m_pSlot->slotinfo.peerAddr	= m_addr;
				m_mapaddr2slot.put(m_pSlot->slotinfo.peerAddr, m_pSlot->slotinfo.nSlot);
				// 然后把所有channel需要重发的指令在这个tick内全部重发
				SendAllUnConfirmedDataThisTick(m_pSlot);
			}
		}
	}
}
void	CNL2SlotMan_I_UDP::TickRecv_CMD_Others()
{
	m_pCmd1	= (CNL2_CMD_1_T *)m_pCmd;
	// 找到对应的slot，交给它处理
	m_pSlot		= m_Slots.GetByIdx(m_pCmd1->slotidx);
	if( !m_pSlot )
	{
		return;
	}

	// CNL2_CMD_SWITCHADDR可能是会来源地址和原地址不同的
	if( CNL2_CMD_SWITCHADDR == m_pCmd1->cmd )
	{
		TickRecv_CMD_SWITCHADDR();
		return;
	}

	// 判断来源地址和slot地址是否一样
	// 必须这么做，比如close就必须要这样，否则有人可能恶意关闭其他用户的slot
	if( m_addr != m_pSlot->slotinfo.peerAddr )
	{
		return;
	}
	// 调用下面的这些
	if( TickRecv_CMD_Others_Call(m_pCmd1->cmd)==0 )
	{
		// 如果处理成功，则更新最后一次收到数据的时间
		m_pSlot->slotinfo.nLastRecv	= m_tickNow;
	}
}
void	CNL2SlotMan_I_UDP::FillSlotWithDefaultTickRecvDealFunc(CNL2SLOT_I *pSlot)
{
	// 设置函数指针
	for(int i=0;i<CNL2_CMD_MAX;i++)
	{
		pSlot->dealfuncRecv[i]					= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_Unknown;
	}
	// 注意：CNL2_CMD_CONNECT是需要特殊处理的，由TickRecv_CMD_CONNECT来处理
	// 其他的指令由后面的分别处理
	pSlot->dealfuncRecv[CNL2_CMD_CONNECT_AGREE]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CONNECT_AGREE;
	pSlot->dealfuncRecv[CNL2_CMD_CONNECT_REFUSE]= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CONNECT_REFUSE;
	pSlot->dealfuncRecv[CNL2_CMD_CLOSE]			= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CLOSE;
	pSlot->dealfuncRecv[CNL2_CMD_DATA_CONFIRM]	= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_CONFIRM;
	pSlot->dealfuncRecv[CNL2_CMD_DATA]			= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_ON_ACCEPTING;
	pSlot->dealfuncRecv[CNL2_CMD_NOOP]			= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP_ON_ACCEPTING;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_Unknown()
{
	// 应该是恶意攻击
	// 立即关闭该Slot
	CloseOnErr(m_pSlot, CLOSEONERR_TickRecv_CMD_Others_Unknown);
	return	-1;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CONNECT_AGREE()
{
	// 判断该slot是否已经知道了
	switch( m_pSlot->slotinfo.nStatus )
	{
		case	CNL2SLOTINFO_T::STATUS_CONNECTING:
		{
			CNL2_CMD_CONNECT_AGREE_T	*pCmdAgree	= (CNL2_CMD_CONNECT_AGREE_T *)m_pCmd;
			m_pSlot->slotinfo.nStatus			= CNL2SLOTINFO_T::STATUS_WORKING;
			m_pSlot->slotinfo.nPeerSlotIdx		= pCmdAgree->serverslotidx;
			// 和服务器的NoopTimeout保持一致
			m_nNoopTimeout						= pCmdAgree->nNoopTimeout;
			m_info.nDropTimeout					= pCmdAgree->nDropTimeout;
			m_nMightDropTimeout					= pCmdAgree->nMightDropTimeout;
			// 清除原来的超时事件(因为channel这时还没有形成，所以不用关闭channel的事件了)
			m_pSlot->ClearAllTE();
			try
			{
				// 读出附加数据获得交换的密钥
				void	*pRear		= wh_getptrnexttoptr(pCmdAgree);
				int		nRearSize	= m_nCmdSize - sizeof(*pCmdAgree);
				const void	*pPubForClient	= NULL;
				int		nPubForClientSize	= whvldata_read_ref((CONNECT_PUBKEY_LEN_T*)pRear, pPubForClient);
				if( nPubForClientSize<=0 )
				{
					throw	1;
				}
				nRearSize	-= sizeof(CONNECT_PUBKEY_LEN_T)+nPubForClientSize;
				if( nRearSize<0 )	// 其实正常情况应该等于0
				{
					throw	2;
				}
				// 交换密钥
				if( m_pSlot->pIKeyExchanger->Agree2(pPubForClient, nPubForClientSize)<0 )
				{
					throw	3;
				}
				// 计算Key的MD5
				whmd5(m_pSlot->pIKeyExchanger->GetKey(), m_pSlot->pIKeyExchanger->GetKeySize(), m_pSlot->szKeyMD5);
				// 初始化各个channel的加密器
				m_pSlot->nCryptorType	= pCmdAgree->cryptortype;
				InitChannelCryptor(m_pSlot);
				// 设置NOOP超时(迟后发送)
				RegNoopTQ(m_pSlot, m_tickNow+m_nNoopTimeout);
				// 设置断线超时
				RegDropTQ(m_pSlot);
				// 如果允许切换地址，则设置“也许断线”超时
				if( m_info.bCanSwitchAddr )
				{
					RegMightDropTQ(m_pSlot, m_tickNow+m_nMightDropTimeout);
				}
				// 通知上层连接成功
				CONTROL_T	*pCOCmd;
				ControlOutAlloc(pCOCmd);
				if( pCOCmd )
				{
					pCOCmd->nCmd	= CONTROL_OUT_SLOT_CONNECTED;
					pCOCmd->data[0]	= m_pSlot->slotinfo.nSlot;
				}
			}
			catch( ... )
			{
				// 应该是hack的数据，关闭slot
				ControlOutCloseAndRemoveSlot(m_pSlot, SLOTCLOSE_REASON_KEYNOTAGREE);
				return	0;
			}
		}
		break;
		case	CNL2SLOTINFO_T::STATUS_WORKING:
		{
			// 已经进入工作状态，说明原来已经收到过AGREE包了。
		}
		break;
		default:
		{
			// 这是不应该的。不进行后面的东西了。(后面的顺其自然)
			return	-1;
		}
		break;
	}
	// 本tick需要立即发送Noop，以让对方知道连接成功了
	m_pSlot->nNoopCount			= 0;
	SendNoopThisTick(m_pSlot);
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CONNECT_REFUSE()
{
	CNL2_CMD_CONNECT_REFUSE_T	*pCmd	= (CNL2_CMD_CONNECT_REFUSE_T *)m_pCmd;
	ControlOutConnectRefuseAndRemoveSlot(m_pSlot, pCmd->reason);
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_CLOSE()
{
	// 先设置附加数据
	CNL2_CMD_CLOSE_T	*pCmd	= (CNL2_CMD_CLOSE_T *)m_pCmd;
	m_pSlot->nCloseExtDataRecv	= pCmd->nExtData;
	// 立刻返回一个CLOSE
	SendClose(m_pSlot);
	// 计数增加
	m_pSlot->nCloseCount	++;
	if( m_pSlot->nCloseCount == 1 )
	{
		// 说明Count本次是0
		// 所以是被动关闭
		SetCloseStatus(m_pSlot, true);
	}
	else	// 那一定是>=2了
	{
		// 可以结束了
		int	nReason	= SLOTCLOSE_REASON_INITIATIVE;
		if( m_pSlot->bClosePassive )
		{
			nReason	= SLOTCLOSE_REASON_PASSIVE;
		}
		ControlOutCloseAndRemoveSlot(m_pSlot, nReason);
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_CONFIRM()
{
	CNL2_CMD_DATA_CONFIRM_T	*pCmd	= (CNL2_CMD_DATA_CONFIRM_T *)m_pCmd;
	int		nSize			= ((int)m_vectrawbuf.size()) - sizeof(*pCmd);
	if( nSize<0 )
	{
		// 错误的数据
		CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_SIZE_SMALL, nSize);
		return	-1;
	}
	// 判断是否已经没有后续数据
	if( nSize == 0 )
	{
		return	0;
	}
	void	*pConfirmData		= wh_getptrnexttovar(pCmd->channelmask);
	// 看看每个channel的confirm信息
	int	i;
	CNL2_CMD_DATA_CONFIRM_T::CONFIRM_T	*pConfirmHdr;
	for(i=0;i<CNL2_CHANNELNUM_MAX;i++)
	{
		if( whbit_uchar_chk(&pCmd->channelmask, i) )
		{
			switch( m_pSlot->channel[i].nType )
			{
				case	CNL2_CHANNELTYPE_SAFE:
				case	CNL2_CHANNELTYPE_LASTSAFE:
				break;
				default:
					// 数据错误
					CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_BADCHANNELTYPE, m_pSlot->channel[i].nType);
					return	-2;
				break;
			}
			// 分解出来确认数据，并进行相关步骤
			if( nSize<(int)sizeof(*pConfirmHdr) )
			{
				// 数据错误
				CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_HDR_SIZE_SMALL, nSize);
				return	-3;
			}
			wh_settypedptr(pConfirmHdr, pConfirmData);
			nSize			-= sizeof(*pConfirmHdr);
			pConfirmData	= wh_getoffsetaddr(pConfirmData, sizeof(*pConfirmHdr));
			// 先根据第一idx进行确认更新
			// 只有定义cnl2idx_t它才能自动回绕
			cnl2idx_t	nIdx	= pConfirmHdr->nInIdxConfirm;
			if( idxdiff(nIdx, pConfirmHdr->nInIdx0) < 0 )
			{
				// 数据有有问题(不可能nB_InConfirmIdx在nB_InIdx0之前，这样就一定是hack)
				CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_IDX_SMALL, nIdx);
				return	-4;
			}
			SlotChannelConfirm(m_pSlot, i, nIdx, 0);
			// 输出队列的头部可以移动到pConfirmHdr->nInIdx0
			SlotOutChannelMove(m_pSlot, i, pConfirmHdr->nInIdx0);
			//
			unsigned short	n;
			bool	bIsGot	= true;
			int		nCount	= 0;
			while( nSize>0 )
			{
				int	nVNSize	= whbit_vn1_get(pConfirmData, &n);
				if( nVNSize<=0 )
				{
					break;
				}
				// 长度改变
				nSize			-= nVNSize;
				// 指针后移
				pConfirmData	= wh_getoffsetaddr(pConfirmData, nVNSize);
				// 如果数据正确则进行confirm
				if( nSize>=0 )
				{
					if( n==0 )
					{
						// 说明到了这个channel确认数据的结尾
						break;
					}
					bIsGot	= !bIsGot;
					if( bIsGot )
					{
						// 理论上应该是m_pSlot->channel[i].pInQueue->GetMaxUnit()/2，不过一般不会有人hack
						if( ++nCount > m_pSlot->channel[i].pInQueue->GetMaxUnit() )
						{
							// 说明可能是hack数据，因为不会发送这么多过来
							// 如果是也说明网络变的太蹊跷了，应该断线了
							CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONFIRM_COUNT_BIG, nSize);
							return	-5;
						}
						// 说明是收到的部分
						// 进行连续确认标注
						SlotChannelConfirm(m_pSlot, i, nIdx, n);
					}
					nIdx	+= n;
				}
			}
		}
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::_TickRecv_FIRST_CMD_ON_ACCEPTING()
{
	// 状态变为connected
	m_pSlot->slotinfo.nStatus	= CNL2SLOTINFO_T::STATUS_WORKING;
	// 清除所有时间事件
	m_pSlot->ClearAllTE();
	// 设置断线超时
	RegDropTQ(m_pSlot);
	// 通知上层(ControlOut告知上层)
	CONTROL_T	*pCmd;
	ControlOutAlloc(pCmd);
	if( pCmd )
	{
		pCmd->nCmd		= CONTROL_OUT_SLOT_ACCEPTED;
		pCmd->data[0]	= m_pSlot->slotinfo.nSlot;
	}
	// 处理函数变为正常的
	m_pSlot->dealfuncRecv[CNL2_CMD_DATA]		= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA;
	m_pSlot->dealfuncRecv[CNL2_CMD_NOOP]		= &CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP;
	// 初始化各个channel的加密器
	InitChannelCryptor(m_pSlot);
	// 如果允许切换地址，则设置“也许断线”超时
	if( m_info.bCanSwitchAddr )
	{
		RegMightDropTQ(m_pSlot, m_tickNow+m_nMightDropTimeout);
	}
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA()
{
	// 根据channel交给不同的处理函数
	m_pCmdData	= (CNL2_CMD_DATA_T *)m_pCmd1;
	m_pData		= wh_getptrnexttoptr(m_pCmdData);
	m_nDataSize	= m_nCmdSize - sizeof(*m_pCmdData);
	if( m_nDataSize<=0 )
	{
		CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_SIZE);
		return	-1;
	}
	// 如果dataphase不对，就关闭
	if( m_pCmdData->prop & CNL2_CMD_DATA_T::PROP_ALLNEG )
	{
		CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_CONTENT);
		return	-2;
	}
	m_pChannel	= &m_pSlot->channel[m_pCmdData->channel];
	return	(this->*m_TickRecv_CMD_Others_DATA_Deal[m_pCmdData->channel])();
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_ON_ACCEPTING()
{
	// 先进行状态转换
	_TickRecv_FIRST_CMD_ON_ACCEPTING();
	// 按正常处理
	TickRecv_CMD_Others_DATA();
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_NotSupport()
{
	// 是hacker，关闭连接
	CloseOnErr(m_pSlot, CLOSEONERR_TickRecv_CMD_Others_DATA_Deal_NotSupport);
	return	-1;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_RT()
{
	CNL2SLOT_I::CHANNEL_T	&channel	= *m_pChannel;
	// 直接按顺序放入接收队列
	CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= channel.pInQueue->Alloc();
	if( !pInUnit )
	{
		// 队列满，忽略这个数据，另外一端爱发就发吧
		return	0;
	}
	// 改unit应该在上次free的时候已经清理过了不过为了保险还是再clear一下
	pInUnit->clear(this);
	SetChannelDataUnit(pInUnit, m_nDataSize, m_pData);
	pInUnit->dataphase	= m_pCmdData->dataphase;
	pInUnit->prop		= m_pCmdData->prop;
	// 放入有数据的队列让上层检查
	ChannelHaveDataSoAddToDList(m_pSlot, &channel);
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_Safe()
{
	CNL2SLOT_I::CHANNEL_T	&channel			= *m_pChannel;
	CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= NULL;
	// 比较这个数据和起始点的差别
	int	nDiff	= idxdiff(m_pCmdData->nIdx, channel.nInIdx0);
	if( nDiff>=channel.pInQueue->GetMaxUnit() )
	{
		// 数据序号超界
		CloseOnErr(m_pSlot, CLOSEONERR_IdxOutOfRange);
		return	-1;
	}
	if( nDiff<0 )
	{
		// 太老的数据，忽略。但是还是要发送确认，因为可能是对方没有收到确认导致的重发。
		goto	end;
	}
	if( nDiff<channel.pInQueue->size() )
	{
		pInUnit	= channel.pInQueue->Get(nDiff);
		// 可以直接放入
		if( pInUnit->pData )
		{
			// 这个是已经收到过数据了，不要理会
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "临时日志,ALREADY RECVED,idx:%d,size:%d,this:%p,ip:%s", m_pCmdData->nIdx, m_nDataSize, this, cmn_get_ipnportstr_by_saaddr(&m_addr));
			pInUnit	= NULL;
		}
	}
	else
	{
		// 扩展队列
		int	nExtent	= nDiff - channel.pInQueue->size() + 1;
		while( nExtent>0 )
		{
			pInUnit	= channel.pInQueue->Alloc();
			assert(pInUnit);
			// 为了保险再clear一下
			pInUnit->clear(this);
			nExtent	--;
		}
		// 最后的pInUnit就是我们需要的
	}
	// 可以放入数据（不过不能立即进行解密，需要按照顺序解，否则可能会先解了后面包的密，导致加解密顺序不同的错误）
	if( pInUnit )
	{
		assert( channel.pICryptorRecv );
		// 先拷贝数据
		SetChannelDataUnit(pInUnit, m_nDataSize, m_pData);
		pInUnit->dataphase	= m_pCmdData->dataphase;
		pInUnit->prop		= m_pCmdData->prop;
		// 更新nInIdxConfirm
		if( m_pCmdData->nIdx == channel.nInIdxConfirm )
		{
			// 这个就是我一直在等待的那个idx
			int	nIdx	= idxdiff(channel.nInIdxConfirm, channel.nInIdx0);
			// 检查后面有多少连续收到的数据（把这些连续的数据进行解密）
			do
			{
				// 解密
				CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= channel.pInQueue->Get(nIdx);
				assert(pInUnit);
				channel.pICryptorRecv->Decrypt(pInUnit->pData, pInUnit->nSize, pInUnit->pData);
				// 下一个
				channel.nInIdxConfirm	++;
				if( ( ++nIdx >= (int)channel.pInQueue->size() ) )
				{
					// 到了结尾了
					break;
				}
				// 那就一定能取出nIdx下标的对象出来
				if( channel.pInQueue->Get(nIdx)->pData == NULL )
				{
					// 这个还没有被确认，结束
					break;
				}
			}while(1);
			// 在这种情况下就可以放入有数据的队列让上层检查
			ChannelHaveDataSoAddToDList(m_pSlot, &channel);
		}
	}
end:
	// 设置这个channel需要确认
	whbit_uchar_set(&m_pSlot->nPeerNotConfirmedChannelMask, channel.nChannel);
	// 无论如何，只要收到数据，则必须在这个tick发送确认包
	SendConfirmThisTick(m_pSlot);

	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_DATA_Deal_LastSafe()
{
	// 这种数据永远都只以最后一个为准，所有前面有没收取的数据就直接替换掉
	CNL2SLOT_I::CHANNEL_T	&channel			= *m_pChannel;
	CNL2SLOT_I::CHANNEL_T::INUNIT_T	*pInUnit	= NULL;
	// 比较这个数据和起始点的差别
	int	nDiff	= idxdiff(m_pCmdData->nIdx, channel.nInIdx0);
	if( nDiff<0 )
	{
		// 没用的数据(先发后至)
		if( nDiff == -1 )
		{
			// 但是差-1这个可能是因为没有收到确认造成的（但不应该是其他的负值）
			// 所以需要再次发送确认
			goto	end;
		}
		return	0;
	}
	// 判断是否已经有数据了，如果有就先删除
	pInUnit	= channel.pInQueue->GetFirst();
	if( pInUnit )
	{
		if( nDiff == 0 )
		{
			// 已经收到的数据，那么就再次发送确认即可
			goto	end;
		}
		// 否则说明是新数据，则删除旧数据
		pInUnit->clear(this);
	}
	else
	{
		// 申请新的unit
		pInUnit	= channel.pInQueue->Alloc();
	}
	assert(pInUnit);

	SetChannelDataUnit(pInUnit, m_nDataSize, m_pData);
	pInUnit->dataphase	= m_pCmdData->dataphase;
	pInUnit->prop		= m_pCmdData->prop;
	// 放入有数据的队列让上层检查
	ChannelHaveDataSoAddToDList(m_pSlot, &channel);
	// 更新nInIdx0和nInIdxConfirm
	// 都以新收到的这个数据为准（因为我只要一个数据嘛，当然是越新越好）
	channel.nInIdx0			= m_pCmdData->nIdx;
	channel.nInIdxConfirm	= m_pCmdData->nIdx + 1;

end:
	// 设置这个channel需要确认
	whbit_uchar_set(&m_pSlot->nPeerNotConfirmedChannelMask, channel.nChannel);
	// 无论如何，只要收到数据，则必须在这个tick发送确认包
	SendConfirmThisTick(m_pSlot);

	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP_ON_ACCEPTING()
{
	// 先进行状态转换
	_TickRecv_FIRST_CMD_ON_ACCEPTING();
	// 按正常处理
	TickRecv_CMD_Others_NOOP();
	return	0;
}
int		CNL2SlotMan_I_UDP::TickRecv_CMD_Others_NOOP()
{
	CNL2_CMD_NOOP_T	*pCmd	= (CNL2_CMD_NOOP_T *)m_pCmd;
	int		nSize			= ((int)m_vectrawbuf.size()) - sizeof(*pCmd);
	if( nSize<0 )
	{
		// 错误的数据
		CloseOnErr(m_pSlot, CLOSEONERR_BAD_DATA_NOOP);
		return	-1;
	}
	if( pCmd->nCount>0 )
	{
		// 计算新的ping值
		int	nDiff	= wh_tickcount_diff(m_tickNow, pCmd->nPeerSendTime);
		m_pSlot->slotinfo.calcRTT(nDiff);
#ifdef	_DEBUG
#ifdef	_WH_LOWLEVELDEBUG
		if( nDiff>500 )
		{
			WHTMPPRINTF("Diff:%d too big!%s", nDiff, WHLINEEND);
			//assert(0);
		}
#endif
#endif
	}
	if( pCmd->nCount<2 )
	{
		// 来回次数不超过2，可以发送返回
		m_pSlot->nPeerNoopSendTime	= pCmd->nSendTime;
		m_pSlot->nNoopCount			= pCmd->nCount+1;
		SendNoopThisTick(m_pSlot);
	}
	return	0;
}
