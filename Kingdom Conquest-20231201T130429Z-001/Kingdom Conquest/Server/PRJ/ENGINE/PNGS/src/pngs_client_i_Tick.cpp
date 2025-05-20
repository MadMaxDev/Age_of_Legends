// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_client_i_Tick.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的Client模块内部实现的Tick函数部分
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-08-25
// Change LOG   :

#include "../inc/pngs_client_i.h"
#include "../inc/pngs_cmn.h"
#include <WHCMN/inc/whcmd.h>

using namespace n_pngs;

int		PNGSClient_I::TickRecv()
{
	if( m_tid == INVALID_TID )
	{
		return	_TickRecv();
	}
	return	0;
}
int		PNGSClient_I::_TickRecv()
{
	m_tickNow	= wh_gettickcount();

	// 接收数据
	m_pSlotMan->TickRecv();

	// 逻辑处理
	// 连接处理
	CNL2SlotMan::CONTROL_T	*pCOCmd;
	size_t	nSize;
	while( m_pSlotMan->ControlOut(&pCOCmd, &nSize)==0 )
	{
		(this->*m_pTickControlOut)(pCOCmd, nSize);
	}

	// 收到的数据处理
	// 处理客户端发来的指令
	int	nSlot;
	int		nChannel;
	pngs_cmd_t	*pClientCmd;
	while( m_pSlotMan->Recv(&nSlot, &nChannel, (void **)&pClientCmd, &nSize)==0 )
	{
		(this->*m_pTickDealRecv)(nChannel, pClientCmd, nSize);
	}

	// 不是线程状态才调用其他模块的tick
	if( m_tid == INVALID_TID )
	{
		DealCmdIn();
	
		// 时间队列也是在线程过程中不要调用的（因为里面会导致发包）
		// 看看时间队列中是否有东西
		whtimequeue::ID_T	id;
		TQUNIT_T			*pUnit;
		while( m_TQ.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
		{
			assert(pUnit->tefunc!=NULL);
			(this->*pUnit->tefunc)(pUnit);	// 这里面可以申请别的时间事件，反正和这个不干扰的。
			// 这个get出来的只能通过Del删除，不能自己删
			m_TQ.Del(id);
		}
	}

	// PlugIn相关的Tick
	CMNBody::AllTick();

	return	0;
}
int		PNGSClient_I::TickSend()
{
	if( m_tid == INVALID_TID )
	{
		return	_TickSend();
	}
	return	0;
}
int		PNGSClient_I::_TickSend()
{
	// 发送数据
	m_pSlotMan->TickLogicAndSend();

	// PlugIn相关的Tick
	CMNBody::AllSureSend();

	return	0;
}
int		PNGSClient_I::Tick_ControlOut_STATUS_NOTHING(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize)
{
	// 这个就什么也不用干
	return	0;
}
int		PNGSClient_I::Tick_ControlOut_STATUS_CONNECTINGCAAFS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize)
{
	switch( pCOCmd->nCmd )
	{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CONNECTED:
		{
			// 和CAAFS连接上了
			// 变成waiting状态
			SetStatus(STATUS_WAITINGINCAAFS);
		}
		break;
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSED:
		{
			// 这个应该很难发生的
			// 告诉上层连接失败
			CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
			pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_NET_ERR_CAAFS;
			SetStatus(STATUS_CONNECTFAIL);
		}
		break;
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CONNECTREFUSED:
		{
			// 连接被拒绝
			// 告诉上层连接失败
			CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
			pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_REFUSED_BY_CAAFS;
			SetStatus(STATUS_CONNECTFAIL);
		}
		break;
		default:
			// 其他的状况就不处理了
		break;
	}
	return	0;
}
int		PNGSClient_I::Tick_ControlOut_STATUS_WAITINGINCAAFS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize)
{
	switch( pCOCmd->nCmd )
	{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSED:
		{
			// 告诉上层连接中断
			CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
			pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_NET_ERR_DROP_CAAFS;
			SetStatus(STATUS_CONNECTFAIL);
		}
		break;
		default:
			// 其他的状况就不处理了
		break;
	}
	return	0;
}
int		PNGSClient_I::Tick_ControlOut_STATUS_CONNECTINGCLS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize)
{
	switch( pCOCmd->nCmd )
	{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CONNECTED:
		{
			assert(pCOCmd->data[0] == m_nSlotCLS);
			// 和CLS连接上了
			// 变成working状态
			SetStatus(STATUS_WORKING);
			// 补上连接正常的通知（原来竟然一直都没有写）
			CONTROL_OUT_CONNECT_RST_T	*pRst;
			ControlOutAlloc(pRst);
			pRst->nCmd		= CONTROL_OUT_CONNECT_RST;
			pRst->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_OK;
			pRst->nClientID	= m_GoToCLSInfo.nClientID;
			cmn_get_saaddr_by_ipnport(&pRst->CLSAddr, m_GoToCLSInfo.IP, m_GoToCLSInfo.nPort);
			// 其实现在也可以开始listen了(如果将来需要品p2p的话)
		}
		break;
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSED:
		{
			CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T	*pClose	= (CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T *)pCOCmd;
			if( pClose->nSlot == m_nSlotCAAFS )
			{
				// 如果是CAAFS的断线了则是正常的
				m_nSlotCAAFS	= 0;
				// 设置和CLS配合的channel配置
				m_CLIENT_SLOTMAN_INFO.clearallchannelinfo();
				pngs_config_CLSClient_Slot_Channel(m_CLIENT_SLOTMAN_INFO.GetBase(), m_cfginfo.nSlotInOutNum, true);
				m_pSlotMan->ReConfigChannel(m_CLIENT_SLOTMAN_INFO.GetBase());
				// 生成连接附加数据
				// 开始连接CLS
				struct sockaddr_in	addr;
				cmn_get_saaddr_by_ipnport(&addr, m_GoToCLSInfo.IP, m_GoToCLSInfo.nPort);
				CLIENT_CLS_CONNECT_EXTINFO_T	ExtInfo;
				ExtInfo.nClientID	= m_GoToCLSInfo.nClientID;
				ExtInfo.nPassword	= m_GoToCLSInfo.nPassword;
				m_nSlotCLS			= m_pSlotMan->Connect(&addr, &ExtInfo, sizeof(ExtInfo));
				if( m_nSlotCLS<0 )
				{
					// 连接失败
					CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
					ControlOutAlloc(pCOCmd);
					pCOCmd->nCmd	= CONTROL_OUT_CONNECT_RST;
					pCOCmd->nRst	= CONTROL_OUT_CONNECT_RST_T::RST_NET_ERR_CLS;
				}
				else
				{
					// 继续后面的等待
				}
			}
			else
			{
				// 应该是和CLS连接的
				// 这个应该很难发生的
				// 告诉上层连接失败
				CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
				ControlOutAlloc(pCOCmd);
				pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
				pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_NET_ERR_CLS;
				SetStatus(STATUS_CONNECTFAIL);
			}
		}
		break;
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CONNECTREFUSED:
		{
			// 连接被拒绝
			// 告诉上层连接失败
			CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
			pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_REFUSED_BY_CLS;
			SetStatus(STATUS_CONNECTFAIL);
		}
		break;
		default:
			// 其他的状况就不处理了
		break;
	}
	return	0;
}
int		PNGSClient_I::Tick_ControlOut_STATUS_WORKING(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize)
{
	switch( pCOCmd->nCmd )
	{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSED:
		{
			CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T	*pClose	= (CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T *)pCOCmd;
			if( m_nSlotCLS == pClose->nSlot )
			{
				// 告诉上层连接中断
				CONTROL_T		*pCOCmd;
				ControlOutAlloc(pCOCmd);
				pCOCmd->nCmd		= CONTROL_OUT_DROP;
				pCOCmd->nParam[0]	= pClose->nReason;
				SetStatus(STATUS_DROPPED);
			}
		}
		break;
		default:
			// 其他的状况就不处理了
		break;
	}
	return	0;
}
int		PNGSClient_I::Tick_DealRecv_STATUS_NOTHING(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize)
{
	// 这个就什么也不用干
	return	0;
}
int		PNGSClient_I::Tick_DealRecv_STATUS_CONNECTINGCAAFS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize)
{
	// 这个阶段不会收到任何东西
	return	0;
}
int		PNGSClient_I::Tick_DealRecv_STATUS_WAITINGINCAAFS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize)
{
	switch( *pClientCmd )
	{
		case	CAAFS_CLIENT_CAAFSINFO:
		{
			CAAFS_CLIENT_CAAFSINFO_T	*pCAAFSInfo	= (CAAFS_CLIENT_CAAFSINFO_T *)pClientCmd;
			try
			{
				// 比较介绍字串是否一致
				if( strcmp(m_cfginfo.szCAAFSSelfInfoToClient, pCAAFSInfo->info)!=0 )
				{
					// 不是刀剑II的服务器
					throw	(int)CONTROL_OUT_CONNECT_RST_T::RST_BAD_TYPE_SERVER;
				}
				// 比较版本
				if( !pngs_isverallowed( pCAAFSInfo->nVerCmpMode, pCAAFSInfo->szVer, m_szVer) )
				{
					// 不允许的版本
					throw	(int)CONTROL_OUT_CONNECT_RST_T::RST_BAD_VER;
				}
				// 看nSelfNotify中是否有和登录相关的东西
				if( pCAAFSInfo->nSelfNotify & CAAFS_CLIENT_CAAFSINFO_T::SELF_NOTIFY_REFUSEALL )
				{
					// 服务器现在禁止登录
					throw	(int)CONTROL_OUT_CONNECT_RST_T::RST_CAAFS_REFUSEALL;
				}
			}
			catch( int nRstCode )
			{
				// 告诉上层结果
				CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
				ControlOutAlloc(pCOCmd);
				pCOCmd->nCmd	= CONTROL_OUT_CONNECT_RST;
				pCOCmd->nRst	= nRstCode;
				// 关闭Slot
				m_pSlotMan->Close(m_nSlotCAAFS);
				m_nSlotCAAFS	= 0;
				// 退出本处理函数
				return	0;
			}
			// 排队相关
			// 通知上层前面还有多少
			CONTROL_OUT_QUEUEINFO_T		*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd				= CONTROL_OUT_QUEUEINFO;
			pCOCmd->nVIPChannel			= 0;
			pCOCmd->nNumToWait			= pCAAFSInfo->nQueueSize;
		}
		break;
		case	CAAFS_CLIENT_QUEUEINFO:
		{
			CAAFS_CLIENT_QUEUEINFO_T	*pQueueInfo	= (CAAFS_CLIENT_QUEUEINFO_T *)pClientCmd;
			CONTROL_OUT_QUEUEINFO_T		*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd				= CONTROL_OUT_QUEUEINFO;
			pCOCmd->nVIPChannel			= pQueueInfo->nVIPChannel;
			pCOCmd->nNumToWait			= pQueueInfo->nQueueSize;
		}
		break;
		case	CAAFS_CLIENT_GOTOCLS:
		{
			CAAFS_CLIENT_GOTOCLS_T		*pGoToCLS	= (CAAFS_CLIENT_GOTOCLS_T *)pClientCmd;
			memcpy(&m_GoToCLSInfo, pGoToCLS, sizeof(m_GoToCLSInfo));
			// 同CAAFS断线（如果m_nSlotCAAFS为0说明是刚刚被CAAFS拒绝然后断线的）
			if( m_nSlotCAAFS!=0 )
			{
				m_pSlotMan->Close(m_nSlotCAAFS);
				// 设置新状态
				SetStatus(STATUS_CONNECTINGCLS);
			}
		}
		break;
	}
	return	0;
}
int		PNGSClient_I::Tick_DealRecv_STATUS_CONNECTINGCLS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize)
{
	// 这期间应该不会处理任何数据的
	return	0;
}
int		PNGSClient_I::Tick_DealRecv_STATUS_WORKING(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize)
{
	switch( *pClientCmd )
	{
	case	CLS_CLIENT_NOTIFY:
		{
			CLS_CLIENT_NOTIFY_T	*pNotify	= (CLS_CLIENT_NOTIFY_T *)pClientCmd;
			switch( pNotify->nSubCmd )
			{
			case	CLS_CLIENT_NOTIFY_T::SUBCMD_GZSDROP:
				{
					// 告诉上层GZS中断，让用户等待
					CONTROL_T		*pCOCmd;
					ControlOutAlloc(pCOCmd);
					pCOCmd->nCmd		= CONTROL_OUT_SERVICE_INTERMIT;
				}
				break;
			case	CLS_CLIENT_NOTIFY_T::SUBCMD_STATON:
				{
					if( !m_teidStat.IsValid() )
					{
						m_cfginfo.nPlayStatInteravl	= 1000 * (unsigned char)pNotify->data[0];
						SetTE_PlayStat();
					}
				}
				break;
			case	CLS_CLIENT_NOTIFY_T::SUBCMD_STATOFF:
				{
					if( m_teidStat.IsValid() )
					{
						m_teidStat.quit();
					}
				}
				break;
			default:
				{
					// 不认识的就不处理了
				}
				break;
			}
		}
		break;
	case	CLS_CLIENT_DATA:
		{
			// 收到数据就传给数据队列
			CLS_CLIENT_DATA_T	*pData	= (CLS_CLIENT_DATA_T *)pClientCmd;
			nSize	-= wh_offsetof(CLS_CLIENT_DATA_T, data);
			Deal_CLS_CLIENT_DATA_One(nChannel, pData->data, nSize);
		}
		break;
	case	CLS_CLIENT_DATA_PACK:
		{
			CLS_CLIENT_DATA_PACK_T	*pData	= (CLS_CLIENT_DATA_PACK_T *)pClientCmd;
			// 一部分一部分解开
			nSize	-= wh_offsetof(CLS_CLIENT_DATA_PACK_T, data);
			whcmdshrink	wcs;
			wcs.SetShrink(pData->data, nSize);
			try
			{
				while( wcs.GetSizeLeft()>0 )
				{
					// 读出尺寸
					int	nDSize;
					wcs.ReadVSize(&nDSize);
					if( wcs.GetSizeLeft()<nDSize )
					{
						throw	"VSize error";
					}
					Deal_CLS_CLIENT_DATA_One(nChannel, wcs.GetCurPtr(), nDSize);
					wcs.Seek(nDSize);
				}
			}
			catch( const char *cszErr )
			{
				// 不管了
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(610,PNGS_RT)"PNGSClient_I::Tick_DealRecv_STATUS_WORKING,%s,CLS_CLIENT_DATA_PACK", cszErr);
			}
		}
		break;
	default:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(611,PNGS_RT)"PNGSClient_I::Tick_DealRecv_STATUS_WORKING,%d,unknown", *pClientCmd);
		}
		break;
	}
	return	0;
}
int		PNGSClient_I::Deal_CLS_CLIENT_DATA_One(int nChannel, const void *pData, size_t nDSize)
{
	// 先看看指令是否是自己内部可以解析的
	pngs_cmd_t	*pnCmd	= (pngs_cmd_t *)pData;
	ILogicBase	*pLogic;
	if( m_mapCmdReg2Logic.get(*pnCmd, pLogic) )
	{
		// 找到了，就派发给相应的逻辑
		CLIENTPACKET_2OTHER_DISPATCHCMD_T	DisCmd;
		DisCmd.nChannel		= nChannel;
		DisCmd.nDSize		= nDSize;
		DisCmd.pData		= pData;
		if( pLogic->CmdIn_NMT(this, CLIENTPACKET_2OTHER_DISPATCHCMD, &DisCmd, sizeof(DisCmd))<0 )
		{
			return	-1;
		}
		return	0;
	}
	//
	DATA_T	*pDataToQueue	= (DATA_T *)m_queueData.InAlloc(nDSize + wh_offsetof(DATA_T, data));
	if( pDataToQueue )
	{
		pDataToQueue->nSize		= nDSize;
		pDataToQueue->nChannel	= nChannel;
		memcpy(pDataToQueue->data, pData, nDSize);
	}
	else
	{
		assert(0);
		return	-10;
	}
	return	0;
}

// 定时相关的
void	PNGSClient_I::TEDeal_PlayStat(TQUNIT_T * pTQUnit)
{
	if( m_cfginfo.nPlayStatInteravl > 0 )
	{
#ifndef	_DEBUG
		// 正式用的时候，间隔不要太小
		if( m_cfginfo.nPlayStatInteravl<20*1000 )
		{
			m_cfginfo.nPlayStatInteravl	= 20*1000;
		}
#endif
		// 获取开机时刻（误差100毫秒）
		// 发送给服务器
		m_vectrawbuf.resize(wh_offsetof(CLIENT_CLS_DATA_T, data) + sizeof(pngs_cmd_t) + sizeof(int) );
		CLIENT_CLS_DATA_T	*pData	= (CLIENT_CLS_DATA_T *)m_vectrawbuf.getbuf();
		pData->nCmd			= CLIENT_CLS_STAT;
		pData->nSvrIdx		= PNGS_SVRIDX_GMS;
		*(pngs_cmd_t *)pData->data	= CLIENT_STAT_STAT_SUBCMD_ONTIME;
		*(int *)wh_getoffsetaddr(pData->data, sizeof(pngs_cmd_t))	= wh_getontime();

		m_pSlotMan->Send(m_nSlotCLS, m_vectrawbuf.getbuf(), m_vectrawbuf.size(), 0);
	}

	// 设置下一个
	SetTE_PlayStat();
}
void	PNGSClient_I::SetTE_PlayStat()
{
	TQUNIT_T	*pNextTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+(m_cfginfo.nPlayStatInteravl>0?m_cfginfo.nPlayStatInteravl:66*1000), (void **)&pNextTQUnit, &m_teidStat)<0 )
	{
		// 怎么会无法加入时间事件涅？
		return;
	}
	pNextTQUnit->tefunc				= &PNGSClient_I::TEDeal_PlayStat;
}
