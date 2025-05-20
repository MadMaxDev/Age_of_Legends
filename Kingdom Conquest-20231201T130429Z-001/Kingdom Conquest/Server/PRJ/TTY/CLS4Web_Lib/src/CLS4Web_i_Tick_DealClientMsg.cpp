#include "../inc/CLS4Web_i.h"

using namespace n_pngs;

int		CLS4Web_i::Tick_DealClientMsg()
{
	m_tickNow			= wh_gettickcount();

	// 看客户端的来来去去
	epoll_server::ctrl_out_T*	pCOCmd	= NULL;
	size_t	nSize		= 0;
	int		nCntrID		= 0;
	pngs_cmd_t*	pClientCmd		= NULL;
	while (m_pEpollServer->ControlOut(&pCOCmd, &nSize) == 0)
	{
		switch (pCOCmd->iCmd)
		{
		case epoll_server::ctrl_out_cntr_accepted:
			{
				Tick_DealClientMsg_CONTROL_OUT_CNTR_ACCEPTED(pCOCmd, nSize);
			}
			break;
		case epoll_server::ctrl_out_cntr_closed:
			{
				Tick_DealClientMsg_CONTROL_OUT_CNTR_CLOSED(pCOCmd, nSize);
			}
			break;
		case epoll_server::ctrl_out_should_stop:
			{
				m_bShouldStop	= true;
			}
			break;
		default:
			{
				// 应该没有其他的可能性
			}
			break;
		}
	}

	// 处理客户端发来的消息
	while (m_pEpollServer->Recv(&nCntrID, (void**)&pClientCmd, &nSize) > 0)
	{
		switch (*pClientCmd)
		{
		case CLIENT_CLS4Web_DATA:
			{
				PlayerUnit*	pPlayer	= GetPlayerByCntrID(nCntrID);
				if (pPlayer == NULL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,CLIENT_CLS4Web_DATA)"Player missing for cntr 0x%X", nCntrID);
					m_pEpollServer->Close(nCntrID);
					continue;
				}
				Tick_DealClientMsg_CLIENT_DATA(CLS4Web_SVR_CLIENT_DATA, pPlayer, pClientCmd, nSize);
			}
			break;
		default:
			{
				// 不支持的指令,关闭连接
				PlayerUnit*	pPlayer	= GetPlayerByCntrID(nCntrID);
				if (pPlayer != NULL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(1394,CLS4Web_RT)"%s,%d,unknown ClientCmd,0x%X,0x%X,%d", __FUNCTION__, *pClientCmd, nCntrID, pPlayer->nID, nSize);
					pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
					RemovePlayerUnit(pPlayer);
				}
				else
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_HACK, GLGR_STD_HDR(1394,CLS4Web_RT)"%s,%d,unknown ClientCmd,0x%X,%d", __FUNCTION__, *pClientCmd, nCntrID, nSize);
					m_pEpollServer->Close(nCntrID);
				}
			}
			break;
		}
	}
	return 0;
}
int		CLS4Web_i::Tick_DealClientMsg_CONTROL_OUT_CNTR_ACCEPTED(epoll_server::ctrl_out_T* pCOCmd, int nSize)
{
	epoll_server::ctrl_out_cntr_accept_T*	pAccept	= (epoll_server::ctrl_out_cntr_accept_T*)pCOCmd;
	size_t		nExtDataSize			= 0;
	CLIENT_CLS4Web_EXTINFO_T* pExtInfo	= (CLIENT_CLS4Web_EXTINFO_T*)m_pEpollServer->GetCntr_ConnectExtData(pAccept->iCntrID, &nExtDataSize);
	if (pExtInfo==NULL || nExtDataSize==0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"Extinfo error,0x%X,%p,%d,%s", pAccept->iCntrID, pExtInfo, nExtDataSize, cmn_get_ipnportstr_by_saaddr(&pAccept->addrPeer));
		m_pEpollServer->Close(pAccept->iCntrID, CLOSE_PARAM_TO_CLIENT_BADCMD);
		return 0;
	}
	PlayerUnit*	pPlayer			= m_Players.getptr(pExtInfo->nClientID);
	if (pPlayer == NULL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"Player not exist,0x%X,0x%X,%s", pAccept->iCntrID, pExtInfo->nClientID, cmn_get_ipnportstr_by_saaddr(&pAccept->addrPeer));
		m_pEpollServer->Close(pAccept->iCntrID, CLOSE_PARAM_TO_CLIENT_NOIDINCLS);
		return 0;
	}
	if (pPlayer->nStatus != PlayerUnit::STATUS_CONNECTING)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"Player error status,0x%X,0x%X,%d,%s", pAccept->iCntrID, pExtInfo->nClientID, pPlayer->nStatus, cmn_get_ipnportstr_by_saaddr(&pAccept->addrPeer));
		m_pEpollServer->Close(pAccept->iCntrID, CLOSE_PARAM_TO_CLIENT_PLAYERSTATUSNOTCONNECTING);
		return 0;
	}
	if (pPlayer->nPasswordToConnectCLS4Web != pExtInfo->nPassword)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"Player error password,0x%X,0x%X,%u,%u,%s", pAccept->iCntrID, pExtInfo->nClientID, pPlayer->nPasswordToConnectCLS4Web, pExtInfo->nPassword, cmn_get_ipnportstr_by_saaddr(&pAccept->addrPeer));
		m_pEpollServer->Close(pAccept->iCntrID, CLOSE_PARAM_TO_CLIENT_BADCMD);
		return 0;
	}
	// 身份确认成功
	// 清除超时事件
	pPlayer->teid.quit();
	// 关联cntr和player
	pPlayer->nCntrID	= pAccept->iCntrID;
	m_pEpollServer->SetExt(pAccept->iCntrID, pPlayer->nID);
	// 设置工作状态
	pPlayer->nStatus	= PlayerUnit::STATUS_WORKING;

	// 发送消息给LP说明用户连接成功了
	CLS4Web_LP_CLIENTTOCLS4Web_RST_T	CTCRst;
	CTCRst.nCmd			= CLS4Web_LP_CLIENTTOCLS4Web_RST;
	CTCRst.nRst			= CLS4Web_LP_CLIENTTOCLS4Web_RST_T::RST_OK;
	CTCRst.nClientID	= pPlayer->nID;
	m_msgerLP.SendMsg(&CTCRst, sizeof(CTCRst));

	// 生成定时检查hack的事件
	SetTEDeal_CheckRecvInterval(pPlayer);
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"CLS4Web Player accepted,0x%X,0x%X,%s", pAccept->iCntrID, pExtInfo->nClientID, cmn_get_ipnportstr_by_saaddr(&pAccept->addrPeer));
	return 0;
}
int		CLS4Web_i::Tick_DealClientMsg_CONTROL_OUT_CNTR_CLOSED(epoll_server::ctrl_out_T* pCOCmd, int nSize)
{
	epoll_server::ctrl_out_cntr_close_reason_T*	pClose	= (epoll_server::ctrl_out_cntr_close_reason_T*)pCOCmd;
	// 找到对应的player,移除它
	PlayerUnit*	pPlayer		= m_Players.getptr(pClose->iCntrExt);
	if (pPlayer != NULL)
	{
		switch (pClose->iReason)
		{
		case epoll_server::close_reason_initiative:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_INITIATIVE;
			}
			break;
		case epoll_server::close_reason_passive:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_PASSIVE;
			}
			break;
		case epoll_server::close_reason_close_timeout:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_CLOSE_TIMEOUT;
			}
			break;
		case epoll_server::close_reason_drop:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_DROP;
			}
			break;
		case epoll_server::close_reason_connect_timeout:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_CONNECT_TIMEOUT;
			}
			break;
		case epoll_server::close_reason_accept_timeout:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_ACCEPT_TIMEOUT;
			}
			break;
		case epoll_server::close_reason_key_not_agree:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_KEY_NOT_AGREE;
			}
			break;
		case epoll_server::close_reason_cntrid_notmatch:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_CNTRID_NOT_MATCH;
			}
		case epoll_server::close_reason_datasize_zero:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_DATASIZE_ZERO;
			}
			break;
		case epoll_server::close_reason_datasize_toobig:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_DATASIZE_TOOBIG;
			}
			break;
		case epoll_server::close_reason_decrypt:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_DECRYPT;
			}
			break;
		case epoll_server::close_reason_encrypt:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_ENCRYPT;
			}
			break;
		case epoll_server::close_reason_recv:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_RECV;
			}
			break;
		case epoll_server::close_reason_send:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_SEND;
			}
			break;
		default:
			{
				pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CNTRCLOSE_REASON_UNKNOWN;
			}
			break;
		}
		GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"CNTR_CLOSED,%d,0x%X,0x%X", pPlayer->nRemoveReason, pPlayer->nID, pPlayer->nCntrID);
		pPlayer->nCntrID	= 0;
		RemovePlayerUnit(pPlayer);
	}
	return 0;
}
int		CLS4Web_i::Tick_DealClientMsg_CLIENT_DATA(pngs_cmd_t nCmd, CLS4Web_i::PlayerUnit* pPlayer, pngs_cmd_t* pCmd, int nSize)
{
	CLIENT_CLS4Web_DATA_T*	pCCData	= (CLIENT_CLS4Web_DATA_T*)pCmd;
	int	nDSize		= nSize - wh_offsetof(CLIENT_CLS4Web_DATA_T, data);
	if (nDSize <= 0)
	{
		// 不允许有这样的数据
		pPlayer->nRemoveReason		= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_CLIENTHACK;
		RemovePlayerUnit(pPlayer);
		return 0;
	}

	// 计数器增加
	pPlayer->nRecvCount	++;
	pPlayer->nRecvSize	+= nSize;

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_SVR_CLIENT_DATA_T, data) + nDSize);
	CLS4Web_SVR_CLIENT_DATA_T*	pCSCData	= (CLS4Web_SVR_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCSCData->nCmd		= nCmd;
	pCSCData->nClientID	= pPlayer->nID;
	memcpy(pCSCData->data, pCCData->data, nDSize);
	m_msgerLP.SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
