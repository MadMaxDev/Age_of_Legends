// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_client_i_Tick.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��Clientģ���ڲ�ʵ�ֵ�Tick��������
//                PNGS��Pixel Network Game Structure����д
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

	// ��������
	m_pSlotMan->TickRecv();

	// �߼�����
	// ���Ӵ���
	CNL2SlotMan::CONTROL_T	*pCOCmd;
	size_t	nSize;
	while( m_pSlotMan->ControlOut(&pCOCmd, &nSize)==0 )
	{
		(this->*m_pTickControlOut)(pCOCmd, nSize);
	}

	// �յ������ݴ���
	// ����ͻ��˷�����ָ��
	int	nSlot;
	int		nChannel;
	pngs_cmd_t	*pClientCmd;
	while( m_pSlotMan->Recv(&nSlot, &nChannel, (void **)&pClientCmd, &nSize)==0 )
	{
		(this->*m_pTickDealRecv)(nChannel, pClientCmd, nSize);
	}

	// �����߳�״̬�ŵ�������ģ���tick
	if( m_tid == INVALID_TID )
	{
		DealCmdIn();
	
		// ʱ�����Ҳ�����̹߳����в�Ҫ���õģ���Ϊ����ᵼ�·�����
		// ����ʱ��������Ƿ��ж���
		whtimequeue::ID_T	id;
		TQUNIT_T			*pUnit;
		while( m_TQ.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
		{
			assert(pUnit->tefunc!=NULL);
			(this->*pUnit->tefunc)(pUnit);	// ���������������ʱ���¼�����������������ŵġ�
			// ���get������ֻ��ͨ��Delɾ���������Լ�ɾ
			m_TQ.Del(id);
		}
	}

	// PlugIn��ص�Tick
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
	// ��������
	m_pSlotMan->TickLogicAndSend();

	// PlugIn��ص�Tick
	CMNBody::AllSureSend();

	return	0;
}
int		PNGSClient_I::Tick_ControlOut_STATUS_NOTHING(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize)
{
	// �����ʲôҲ���ø�
	return	0;
}
int		PNGSClient_I::Tick_ControlOut_STATUS_CONNECTINGCAAFS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize)
{
	switch( pCOCmd->nCmd )
	{
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CONNECTED:
		{
			// ��CAAFS��������
			// ���waiting״̬
			SetStatus(STATUS_WAITINGINCAAFS);
		}
		break;
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSED:
		{
			// ���Ӧ�ú��ѷ�����
			// �����ϲ�����ʧ��
			CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
			pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_NET_ERR_CAAFS;
			SetStatus(STATUS_CONNECTFAIL);
		}
		break;
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CONNECTREFUSED:
		{
			// ���ӱ��ܾ�
			// �����ϲ�����ʧ��
			CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
			pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_REFUSED_BY_CAAFS;
			SetStatus(STATUS_CONNECTFAIL);
		}
		break;
		default:
			// ������״���Ͳ�������
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
			// �����ϲ������ж�
			CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
			pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_NET_ERR_DROP_CAAFS;
			SetStatus(STATUS_CONNECTFAIL);
		}
		break;
		default:
			// ������״���Ͳ�������
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
			// ��CLS��������
			// ���working״̬
			SetStatus(STATUS_WORKING);
			// ��������������֪ͨ��ԭ����Ȼһֱ��û��д��
			CONTROL_OUT_CONNECT_RST_T	*pRst;
			ControlOutAlloc(pRst);
			pRst->nCmd		= CONTROL_OUT_CONNECT_RST;
			pRst->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_OK;
			pRst->nClientID	= m_GoToCLSInfo.nClientID;
			cmn_get_saaddr_by_ipnport(&pRst->CLSAddr, m_GoToCLSInfo.IP, m_GoToCLSInfo.nPort);
			// ��ʵ����Ҳ���Կ�ʼlisten��(���������ҪƷp2p�Ļ�)
		}
		break;
		case	CNL2SlotMan::CONTROL_OUT_SLOT_CLOSED:
		{
			CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T	*pClose	= (CNL2SlotMan::CONTROL_OUT_SLOT_CLOSE_REASON_T *)pCOCmd;
			if( pClose->nSlot == m_nSlotCAAFS )
			{
				// �����CAAFS�Ķ���������������
				m_nSlotCAAFS	= 0;
				// ���ú�CLS��ϵ�channel����
				m_CLIENT_SLOTMAN_INFO.clearallchannelinfo();
				pngs_config_CLSClient_Slot_Channel(m_CLIENT_SLOTMAN_INFO.GetBase(), m_cfginfo.nSlotInOutNum, true);
				m_pSlotMan->ReConfigChannel(m_CLIENT_SLOTMAN_INFO.GetBase());
				// �������Ӹ�������
				// ��ʼ����CLS
				struct sockaddr_in	addr;
				cmn_get_saaddr_by_ipnport(&addr, m_GoToCLSInfo.IP, m_GoToCLSInfo.nPort);
				CLIENT_CLS_CONNECT_EXTINFO_T	ExtInfo;
				ExtInfo.nClientID	= m_GoToCLSInfo.nClientID;
				ExtInfo.nPassword	= m_GoToCLSInfo.nPassword;
				m_nSlotCLS			= m_pSlotMan->Connect(&addr, &ExtInfo, sizeof(ExtInfo));
				if( m_nSlotCLS<0 )
				{
					// ����ʧ��
					CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
					ControlOutAlloc(pCOCmd);
					pCOCmd->nCmd	= CONTROL_OUT_CONNECT_RST;
					pCOCmd->nRst	= CONTROL_OUT_CONNECT_RST_T::RST_NET_ERR_CLS;
				}
				else
				{
					// ��������ĵȴ�
				}
			}
			else
			{
				// Ӧ���Ǻ�CLS���ӵ�
				// ���Ӧ�ú��ѷ�����
				// �����ϲ�����ʧ��
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
			// ���ӱ��ܾ�
			// �����ϲ�����ʧ��
			CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
			ControlOutAlloc(pCOCmd);
			pCOCmd->nCmd		= CONTROL_OUT_CONNECT_RST;
			pCOCmd->nRst		= CONTROL_OUT_CONNECT_RST_T::RST_REFUSED_BY_CLS;
			SetStatus(STATUS_CONNECTFAIL);
		}
		break;
		default:
			// ������״���Ͳ�������
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
				// �����ϲ������ж�
				CONTROL_T		*pCOCmd;
				ControlOutAlloc(pCOCmd);
				pCOCmd->nCmd		= CONTROL_OUT_DROP;
				pCOCmd->nParam[0]	= pClose->nReason;
				SetStatus(STATUS_DROPPED);
			}
		}
		break;
		default:
			// ������״���Ͳ�������
		break;
	}
	return	0;
}
int		PNGSClient_I::Tick_DealRecv_STATUS_NOTHING(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize)
{
	// �����ʲôҲ���ø�
	return	0;
}
int		PNGSClient_I::Tick_DealRecv_STATUS_CONNECTINGCAAFS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize)
{
	// ����׶β����յ��κζ���
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
				// �ȽϽ����ִ��Ƿ�һ��
				if( strcmp(m_cfginfo.szCAAFSSelfInfoToClient, pCAAFSInfo->info)!=0 )
				{
					// ���ǵ���II�ķ�����
					throw	(int)CONTROL_OUT_CONNECT_RST_T::RST_BAD_TYPE_SERVER;
				}
				// �Ƚϰ汾
				if( !pngs_isverallowed( pCAAFSInfo->nVerCmpMode, pCAAFSInfo->szVer, m_szVer) )
				{
					// ������İ汾
					throw	(int)CONTROL_OUT_CONNECT_RST_T::RST_BAD_VER;
				}
				// ��nSelfNotify���Ƿ��к͵�¼��صĶ���
				if( pCAAFSInfo->nSelfNotify & CAAFS_CLIENT_CAAFSINFO_T::SELF_NOTIFY_REFUSEALL )
				{
					// ���������ڽ�ֹ��¼
					throw	(int)CONTROL_OUT_CONNECT_RST_T::RST_CAAFS_REFUSEALL;
				}
			}
			catch( int nRstCode )
			{
				// �����ϲ���
				CONTROL_OUT_CONNECT_RST_T	*pCOCmd;
				ControlOutAlloc(pCOCmd);
				pCOCmd->nCmd	= CONTROL_OUT_CONNECT_RST;
				pCOCmd->nRst	= nRstCode;
				// �ر�Slot
				m_pSlotMan->Close(m_nSlotCAAFS);
				m_nSlotCAAFS	= 0;
				// �˳���������
				return	0;
			}
			// �Ŷ����
			// ֪ͨ�ϲ�ǰ�滹�ж���
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
			// ͬCAAFS���ߣ����m_nSlotCAAFSΪ0˵���Ǹոձ�CAAFS�ܾ�Ȼ����ߵģ�
			if( m_nSlotCAAFS!=0 )
			{
				m_pSlotMan->Close(m_nSlotCAAFS);
				// ������״̬
				SetStatus(STATUS_CONNECTINGCLS);
			}
		}
		break;
	}
	return	0;
}
int		PNGSClient_I::Tick_DealRecv_STATUS_CONNECTINGCLS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize)
{
	// ���ڼ�Ӧ�ò��ᴦ���κ����ݵ�
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
					// �����ϲ�GZS�жϣ����û��ȴ�
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
					// ����ʶ�ľͲ�������
				}
				break;
			}
		}
		break;
	case	CLS_CLIENT_DATA:
		{
			// �յ����ݾʹ������ݶ���
			CLS_CLIENT_DATA_T	*pData	= (CLS_CLIENT_DATA_T *)pClientCmd;
			nSize	-= wh_offsetof(CLS_CLIENT_DATA_T, data);
			Deal_CLS_CLIENT_DATA_One(nChannel, pData->data, nSize);
		}
		break;
	case	CLS_CLIENT_DATA_PACK:
		{
			CLS_CLIENT_DATA_PACK_T	*pData	= (CLS_CLIENT_DATA_PACK_T *)pClientCmd;
			// һ����һ���ֽ⿪
			nSize	-= wh_offsetof(CLS_CLIENT_DATA_PACK_T, data);
			whcmdshrink	wcs;
			wcs.SetShrink(pData->data, nSize);
			try
			{
				while( wcs.GetSizeLeft()>0 )
				{
					// �����ߴ�
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
				// ������
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
	// �ȿ���ָ���Ƿ����Լ��ڲ����Խ�����
	pngs_cmd_t	*pnCmd	= (pngs_cmd_t *)pData;
	ILogicBase	*pLogic;
	if( m_mapCmdReg2Logic.get(*pnCmd, pLogic) )
	{
		// �ҵ��ˣ����ɷ�����Ӧ���߼�
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

// ��ʱ��ص�
void	PNGSClient_I::TEDeal_PlayStat(TQUNIT_T * pTQUnit)
{
	if( m_cfginfo.nPlayStatInteravl > 0 )
	{
#ifndef	_DEBUG
		// ��ʽ�õ�ʱ�򣬼����Ҫ̫С
		if( m_cfginfo.nPlayStatInteravl<20*1000 )
		{
			m_cfginfo.nPlayStatInteravl	= 20*1000;
		}
#endif
		// ��ȡ����ʱ�̣����100���룩
		// ���͸�������
		m_vectrawbuf.resize(wh_offsetof(CLIENT_CLS_DATA_T, data) + sizeof(pngs_cmd_t) + sizeof(int) );
		CLIENT_CLS_DATA_T	*pData	= (CLIENT_CLS_DATA_T *)m_vectrawbuf.getbuf();
		pData->nCmd			= CLIENT_CLS_STAT;
		pData->nSvrIdx		= PNGS_SVRIDX_GMS;
		*(pngs_cmd_t *)pData->data	= CLIENT_STAT_STAT_SUBCMD_ONTIME;
		*(int *)wh_getoffsetaddr(pData->data, sizeof(pngs_cmd_t))	= wh_getontime();

		m_pSlotMan->Send(m_nSlotCLS, m_vectrawbuf.getbuf(), m_vectrawbuf.size(), 0);
	}

	// ������һ��
	SetTE_PlayStat();
}
void	PNGSClient_I::SetTE_PlayStat()
{
	TQUNIT_T	*pNextTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+(m_cfginfo.nPlayStatInteravl>0?m_cfginfo.nPlayStatInteravl:66*1000), (void **)&pNextTQUnit, &m_teidStat)<0 )
	{
		// ��ô���޷�����ʱ���¼�����
		return;
	}
	pNextTQUnit->tefunc				= &PNGSClient_I::TEDeal_PlayStat;
}
