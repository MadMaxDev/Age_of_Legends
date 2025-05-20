#include "../inc/whnetepoll_client.h"

using namespace n_whnet;

WHDATAPROP_MAP_BEGIN_AT_ROOT(epoll_connecter_client::info_T)
	WHDATAPROP_ON_SETVALUE_smp(int, iSendBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iRecvBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iCtrlOutQueueSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szIP, 0)
	WHDATAPROP_ON_SETVALUE_smp(short, iPort, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iConnectTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bPrintKA, 0)
WHDATAPROP_MAP_END()

epoll_connecter_client::epoll_connecter_client()
: m_bNeedCode(false)
, m_iCntrIDPeer(-1)
, m_iCryptType(-1)
, m_iKeyLen(0)
, m_iKATimeout(0)
, m_iDropTimeout(0)
, m_bClosePassive(true)
, m_bClosing(false)
, m_tLastRecvTick(0)
, m_iStatus(status_nothing)
, m_sock(INVALID_SOCKET)
{
	m_szKeyD[0]		= 0;
	m_szKeyE[0]		= 0;
}
epoll_connecter_client::~epoll_connecter_client()
{
	Release();
}
int		epoll_connecter_client::Init(info_T* pInfo)
{
	int	iRst		= 0;
	if (pInfo == NULL)
	{
		return -1;
	}
	if (pInfo != &m_info)
	{
		memcpy(&m_info, pInfo, sizeof(m_info));
	}

	m_queueCtrlOut.Release();
	if ((iRst=m_queueCtrlOut.Init(pInfo->iCtrlOutQueueSize)) < 0)
	{
		return -2;
	}
	
	whtimequeue::INFO_T	info;
	info.nChunkSize		= pInfo->iTQChunkSize;
	info.nUnitLen		= sizeof(tq_unit_T);
	if ((iRst=m_TQ.Init(&info)) < 0)
	{
		return -3;
	}

	m_bufRecv.Init(m_info.iRecvBufSize, 32, m_info.iRecvBufSize/4);
	m_bufSend.Init(m_info.iSendBufSize, 32, m_info.iSendBufSize/4);

	m_vectCodeDataBuf.reserve(m_info.nCodeDataBufSize);
	m_vectCodeKeyBuf.reserve(m_info.nCodeKeyBufSize);
	m_vectCodeNKeyBuf.reserve(m_info.nCodeNKeyBufSize);

	// 创建套接字
	m_sock	= tcp_create_connect_socket(m_info.szIP, m_info.iPort, 0, m_info.iConnectTimeOut);
	if (m_sock == INVALID_SOCKET)
	{
		return -4;
	}

	m_vectRawBuf.reserve(max_epoll_msg_size);

	m_iStatus	= status_connecting;

	m_bClosePassive	= true;
	m_bClosing		= false;

	return 0;
}
int		epoll_connecter_client::Release()
{
	cmn_safeclosesocket(m_sock);
	m_bufSend.Release();
	m_bufRecv.Release();
	m_TQ.Release();
	//m_queueCtrlOut.Release();//改成在Init的时候release了,不然ctrlout不行

	return 0;
}
int		epoll_connecter_client::Work()
{
	ManualRecv();
	TickLogic_TE();
	ManualSend();
	return 0;
}
int		epoll_connecter_client::Close()
{
	if (m_iStatus==status_error || m_iStatus==status_nothing)
	{
		return -1;
	}
	m_bClosePassive		= false;
	SendClose();		// 这里会设置m_bClosing为true
	RegClose();
	return 0;
}
void	epoll_connecter_client::SendClose()
{
	epoll_msg_close_t	closeMsg;
	closeMsg.iCmd		= epoll_msg_close;
	closeMsg.iExtData	= 0;
	SendRawMsg(&closeMsg, sizeof(closeMsg));

	m_bClosing			= true;
}
void	epoll_connecter_client::SendKA()
{
	epoll_msg_ka_t		kaMsg;
	kaMsg.iCmd			= epoll_msg_ka;
	SendRawMsg(&kaMsg, sizeof(kaMsg));
}
void	epoll_connecter_client::TickLogic_TE()
{
	whtick_t	t		= wh_gettickcount();
	tq_unit_T*	pTQUnit	= NULL;
	whtimequeue::ID_T	id;
	while (m_TQ.GetUnitBeforeTime(t, (void**)&pTQUnit, &id) == 0)
	{
		(this->*pTQUnit->pFunc)();
		m_TQ.Del(id);
	}
}
void	epoll_connecter_client::RegKA()
{
	whtick_t	t		= wh_gettickcount();
	tq_unit_T	tqUnit;
	tqUnit.pFunc		= &epoll_connecter_client::TEDeal_KA;
	whtimequeue::ID_T	id;
	m_TQ.Add(t+m_iKATimeout, &tqUnit, &id);
}
void	epoll_connecter_client::RegClose()
{
	whtick_t	t		= wh_gettickcount();
	tq_unit_T	tqUnit;
	tqUnit.pFunc		= &epoll_connecter_client::TEDeal_Close;
	whtimequeue::ID_T	id;
	m_TQ.Add(t+m_info.iCloseTimeOut, &tqUnit, &id);
}
void	epoll_connecter_client::RegDrop()
{
	whtick_t	t		= wh_gettickcount();
	tq_unit_T	tqUnit;
	tqUnit.pFunc		= &epoll_connecter_client::TEDeal_Drop;
	whtimequeue::ID_T	id;
	m_TQ.Add(t+m_iDropTimeout, &tqUnit, &id);
}
void	epoll_connecter_client::TEDeal_KA()
{
	SendKA();
	RegKA();
}
void	epoll_connecter_client::TEDeal_Close()
{
	ControlOutReasonAndRelease(ctrlout_closed);
}
void	epoll_connecter_client::TEDeal_Drop()
{
	whtick_t	t	= wh_gettickcount();
	if ((t-m_tLastRecvTick) >= m_iDropTimeout)
	{
		ControlOutReasonAndRelease(ctrlout_closed);
	}
	else
	{
		RegDrop();
	}
}
int		epoll_connecter_client::ControlOut(ctrl_out_T** ppCmd, size_t* piSize)
{
	*piSize	= m_queueCtrlOut.PeekSize();
	if (*piSize <= 0)
	{
		return -1;
	}
	m_vectCtrlOutCmd.resize(*piSize);
	*ppCmd	= (ctrl_out_T*)m_vectCtrlOutCmd.getbuf();
	return m_queueCtrlOut.Out(*ppCmd, piSize);
}
void	epoll_connecter_client::SetExtData(void* pData, int iSize)
{
	m_vectCntrExtData.resize(iSize);
	if (iSize>0 && pData!=NULL)
	{
		memcpy(m_vectCntrExtData.getbuf(), pData, iSize);
	}
}
void*	epoll_connecter_client::GetExtData(int* piSize)
{
	*piSize	= m_vectCntrExtData.size();
	return m_vectCntrExtData.getbuf();
}
int		epoll_connecter_client::ManualRecv(int iItv/* =0 */)
{
	if (m_iStatus==status_nothing || m_iStatus==status_error)
	{
		return 0;
	}
	int	iRst	= 0;
	if (m_bufRecv.GetSizeLeft() > 0)
	{
		if ((iRst=cmn_select_rd(m_sock, iItv)) > 0)
		{
			m_tLastRecvTick	= wh_gettickcount();
			int	iSize		= tcp_saferecv(m_sock, m_bufRecv.GetTail(), m_bufRecv.GetSizeLeft());
			if (iSize > 0)
			{
				m_bufRecv.InAlloc(iSize);
			}
			else
			{
				ControlOutReasonAndRelease(ctrlout_error, err_reason_recv);
				return -1;
			}
		}
		else if (iRst < 0)
		{
			ControlOutReasonAndRelease(ctrlout_error, err_reason_select_rd);
			return -2;
		}
	}
	else if (m_bufSend.GetSize()==0 && iItv>0)
	{
		wh_sleep(iItv);
	}
	return 0;
}
int		epoll_connecter_client::ManualSend(int iItv/* =0 */)
{
	if (m_iStatus==status_nothing || m_iStatus==status_error)
	{
		return 0;
	}
	if (m_bufSend.GetSize() > 0)
	{
		int	iRst	= cmn_select_wr(m_sock, iItv);
		if (iRst > 0)
		{
			int	iSize	= send(m_sock, m_bufSend.GetBuf(), m_bufSend.GetSize(), 0);
			if (iSize > 0)
			{
				m_bufSend.Out(iSize);
			}
			else
			{
				ControlOutReasonAndRelease(ctrlout_error, err_reason_send);
				return -1;
			}
		}
		else if (iRst < 0)
		{
			ControlOutReasonAndRelease(ctrlout_error, err_reason_select_wr);
			return -2;
		}
	}
	return 0;
}
int		epoll_connecter_client::ControlOutReasonAndRelease(int iCtrlOutCmd, int iErrReason/* =err_reason_ok */)
{
	switch (iCtrlOutCmd)
	{
	case ctrlout_closed:
		{
			m_iStatus	= status_nothing;
		}
		break;
	case ctrlout_error:
		{
			if (m_bClosing && m_bClosePassive)
			{
				m_iStatus	= status_nothing;
				iCtrlOutCmd	= ctrlout_closed;
				iErrReason	= err_reason_ok;
			}
			else
			{
				m_iStatus	= status_error;
				if (m_bClosing && !m_bClosePassive)
				{
					iErrReason	= err_reason_init_close;
				}
			}
		}
		break;
	}
	ctrl_out_T*	pCO	= (ctrl_out_T*)m_queueCtrlOut.InAlloc(sizeof(ctrl_out_T));
	pCO->iCmd		= iCtrlOutCmd;
	pCO->iParam		= iErrReason;
	Release();
	return 0;
}
int		epoll_connecter_client::SendMsg(const void* pData, size_t iSize)
{
	epoll_msg_head_t*	pHead	= (epoll_msg_head_t*)m_bufSend.InAlloc(sizeof(epoll_msg_head_t)+sizeof(epoll_msg_data_t)+iSize);
	if (pHead == NULL)
	{
		return -1;
	}
	pHead->iCntrID	= m_iCntrIDPeer;
	pHead->iSize	= sizeof(epoll_msg_data_t)+iSize;
	epoll_msg_data_t*	pBase	= (epoll_msg_data_t*)wh_getptrnexttoptr(pHead);
	unsigned char*	pDataBuf	= (unsigned char*)wh_getptrnexttoptr(pBase);
	memcpy(pDataBuf, pData, iSize);
	pBase->iCmd		= epoll_msg_data;
	pBase->iCrc		= crc8((char*)pDataBuf, (int)iSize);
	return 0;
}
int		epoll_connecter_client::SendRawMsg(const void* pData, size_t iSize)
{
	epoll_msg_head_t*	pHead	= (epoll_msg_head_t*)m_bufSend.InAlloc(sizeof(epoll_msg_head_t)+iSize);
	if (pHead == NULL)
	{
		return -1;
	}
	pHead->iCntrID	= m_iCntrIDPeer;
	pHead->iSize	= iSize;
	memcpy(wh_getptrnexttoptr(pHead), pData, iSize);
	return 0;
}
int		epoll_connecter_client::RecvMsg(void* pData, size_t* piSize)
{
begin:
	epoll_msg_head_t*	pHead	= NULL;
	pHead	= GetAvailMsgHead();
	if (pHead == NULL)
	{
		return 0;
	}
	if ((size_t)pHead->iSize > *piSize)
	{
		return -1;
	}
	epoll_msg_base_t*	pBase	= (epoll_msg_base_t*)wh_getptrnexttoptr(pHead);
	switch (pBase->iCmd)
	{
	case epoll_msg_exchange_key:
		{
			m_iStatus		= status_working;
			{
				epoll_msg_exchange_key_t*	pEx	= (epoll_msg_exchange_key_t*)pBase;
				m_iKATimeout	= pEx->iKATimeout;
				m_iDropTimeout	= pEx->iKATimeout*3;

				// 设置key
				SetCodeKey((const unsigned char*)pEx->szKey);
			}
			m_vectRawBuf.resize(pHead->iSize+m_vectCntrExtData.size());
			char*	pBuf	= m_vectRawBuf.getbuf();
			memcpy(pBuf, pBase, pHead->iSize);
			if (m_vectCntrExtData.size() != 0)
			{
				pBuf		+= pHead->iSize;
				memcpy(pBuf, m_vectCntrExtData.getbuf(), m_vectCntrExtData.size());
			}
			m_iCntrIDPeer	= pHead->iCntrID;
			SendRawMsg(m_vectRawBuf.getbuf(), m_vectRawBuf.size());
			FreeMsg();
			// 通知上层连接OK
			ctrl_out_T*	pCO	= (ctrl_out_T*)m_queueCtrlOut.InAlloc(sizeof(ctrl_out_T));
			pCO->iCmd		= ctrlout_working;
			// 注册心跳包
			RegKA();
			// 注册掉线事件
			RegDrop();
			goto begin;
		}
		break;
	case epoll_msg_close:
		{
			if (m_bClosing)
			{
				ControlOutReasonAndRelease(ctrlout_closed);
				return 0;
			}
			else
			{
				SendClose();
				RegClose();
				FreeMsg();
				goto begin;
			}
		}
		break;
	case epoll_msg_data:
		{
			if (m_iStatus != status_working)
			{
				ControlOutReasonAndRelease(ctrlout_error, err_reason_recvdata_not_work);
				return -10;
			}
			epoll_msg_data_t*	pMsgData	= (epoll_msg_data_t*)pBase;
			int		iRealSize	= pHead->iSize - sizeof(epoll_msg_data_t);
			*piSize				= iRealSize;
			memcpy(pData, wh_getptrnexttoptr(pMsgData), iRealSize);
			if (pMsgData->iCrc != crc8((char*)pData, iRealSize))
			{
				ControlOutReasonAndRelease(ctrlout_error, err_reason_crc);
				return -100;
			}
		}
		break;
	case epoll_msg_ka:
		{
			FreeMsg();
			if (m_info.bPrintKA)
			{
				printf("recv_keep_alive%s", WHLINEEND);
			}
			goto begin;
		}
		break;
	default:
		{
			ControlOutReasonAndRelease(ctrlout_error, err_reason_err_packet);
			return -20;
		}
		break;
	}

	FreeMsg();
	return 1;
}
const void*	epoll_connecter_client::PeekMsg(size_t* piSize)
{
begin:
	epoll_msg_head_t*	pHead	= NULL;
	pHead	= GetAvailMsgHead();
	if (pHead == NULL)
	{
		return NULL;
	}
	epoll_msg_base_t*	pBase	= (epoll_msg_base_t*)wh_getptrnexttoptr(pHead);
	switch (pBase->iCmd)
	{
	case epoll_msg_exchange_key:
		{
			m_iStatus		= status_working;
			{
				epoll_msg_exchange_key_t*	pEx	= (epoll_msg_exchange_key_t*)pBase;
				m_iKATimeout	= pEx->iKATimeout;
				m_iDropTimeout	= pEx->iKATimeout*3;

				// 设置key
				SetCodeKey((const unsigned char*)pEx->szKey);
			}
			m_vectRawBuf.resize(pHead->iSize+m_vectCntrExtData.size());
			char*	pBuf	= m_vectRawBuf.getbuf();
			memcpy(pBuf, pBase, pHead->iSize);
			if (m_vectCntrExtData.size() != 0)
			{
				pBuf		+= pHead->iSize;
				memcpy(pBuf, m_vectCntrExtData.getbuf(), m_vectCntrExtData.size());
			}
			m_iCntrIDPeer	= pHead->iCntrID;
			SendRawMsg(m_vectRawBuf.getbuf(), m_vectRawBuf.size());
			FreeMsg();
			// 通知上层连接OK
			ctrl_out_T*	pCO	= (ctrl_out_T*)m_queueCtrlOut.InAlloc(sizeof(ctrl_out_T));
			pCO->iCmd		= ctrlout_working;
			// 注册心跳包
			RegKA();
			// 注册掉线事件
			RegDrop();
			goto begin;
		}
		break;
	case epoll_msg_close:
		{
			if (m_bClosing)
			{
				ControlOutReasonAndRelease(ctrlout_closed);
				return NULL;
			}
			else
			{
				SendClose();
				RegClose();
				FreeMsg();
				goto begin;
			}
		}
		break;
	case epoll_msg_data:
		{
			if (m_iStatus != status_working)
			{
				ControlOutReasonAndRelease(ctrlout_error, err_reason_recvdata_not_work);
				return NULL;
			}
			int		iRealSize	= pHead->iSize - sizeof(epoll_msg_data_t);
			*piSize				= iRealSize;
			epoll_msg_data_t*	pMsgData	= (epoll_msg_data_t*)pBase;
			unsigned char*		pData		= (unsigned char*)wh_getptrnexttoptr(pMsgData);
			if (pMsgData->iCrc != crc8((char*)pData, iRealSize))
			{
				ControlOutReasonAndRelease(ctrlout_error, err_reason_crc);
				return NULL;
			}
			return pData;
		}
		break;
	case epoll_msg_ka:
		{
			FreeMsg();
			if (m_info.bPrintKA)
			{
				printf("recv_keep_alive%s", WHLINEEND);
			}
			goto begin;
		}
		break;
	default:
		{
			ControlOutReasonAndRelease(ctrlout_error, err_reason_err_packet);
			return NULL;
		}
		break;
	}

	return NULL;
}
epoll_msg_head_t*	epoll_connecter_client::GetAvailMsgHead()
{
	if (m_bufRecv.GetSize() < min_epoll_msg_size)
	{
		return NULL;
	}
	epoll_msg_head_t*	pHead	= (epoll_msg_head_t*)m_bufRecv.GetBuf();
	if (pHead->iSize < min_epoll_msg_data_size)
	{
		ControlOutReasonAndRelease(ctrlout_error, err_reason_err_packet);
		return NULL;
	}
// 	if (pHead->iSize > max_epoll_msg_data_size)
// 	{
// 		ControlOutReasonAndRelease(ctrlout_error, err_reason_err_packet);
// 		return NULL;
// 	}
	if ((pHead->iSize+sizeof(epoll_msg_head_t)) > m_bufRecv.GetSize())
	{
		return NULL;
	}
	return pHead;
}
void	epoll_connecter_client::FreeMsg()
{
	epoll_msg_head_t*	pHead	= GetAvailMsgHead();
	if (pHead == NULL)
	{
		return;
	}
	m_bufRecv.Out(sizeof(epoll_msg_head_t)+pHead->iSize);
}
int		epoll_connecter_client::GetSockets(whvector<SOCKET>& vect)
{
	if (cmn_is_validsocket(m_sock))
	{
		vect.push_back(m_sock);
	}
	return vect.size();
}
