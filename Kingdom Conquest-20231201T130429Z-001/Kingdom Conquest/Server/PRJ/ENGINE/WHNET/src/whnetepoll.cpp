#include "../inc/whnetepoll.h"
#include "../inc/whnetudpGLogger.h"
using namespace n_whnet;

WHDATAPROP_MAP_BEGIN_AT_ROOT(n_whnet::epoll_server::info_T)
	WHDATAPROP_ON_SETVALUE_smp(int, iMaxConnections, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iPort, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iPort1, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szIP, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bNBO, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iDropTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iKATimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iCloseTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iExchangeKeyTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iSendBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iRecvBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iCtrlOutQueueSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bNeedCode, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iCodeDataBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iCodeKeyBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, iCodeNKeyBufSize, 0)
WHDATAPROP_MAP_END()

#ifdef WIN32

#include "whcmn/inc/whlist.h"

namespace n_whnet
{
#define MAX_EPOLL_FD	5
typedef	map<SOCKET, whnet_epoll_event>		map_socket2event_T;

class epoll_mgr
{
public:
	epoll_mgr()
	{
		m_epfdMgr.init(MAX_EPOLL_FD);
		memset(m_epSize, 0, sizeof(m_epSize));
	}
	int		CreateEpfd(int iSize)
	{
		int	epfd	= m_epfdMgr.borrowidx()+1;
		if (epfd <= 0)
		{
			return -1;
		}
		assert(m_epSize[epfd] == 0);
		m_epSocket[epfd].reserve(iSize);
		m_epSize[epfd]		= iSize;
		return epfd;
	}
	void	CloseEpfd(int epfd)
	{
		if (epfd < 0)
		{
			return;
		}
		m_epfdMgr.returnidx(epfd-1);
		m_epSize[epfd]		= 0;
		whvector<whnet_epoll_event>&	vec	= m_epSocket[epfd];
		for (int i=0; i<vec.size(); i++)
		{
			cmn_safeclosesocket(vec[i].data.fd);
			vec[i].events	= 0;
		}
		m_epSocket[epfd].resize(0);
	}
	int		EpollCtl(int epfd, int iOpCode, int iFd, whnet_epoll_event* pEvent)
	{
		if (epfd < 0)
		{
			return 0;
		}
		SOCKET	fd		= iFd;
		int		iRst	= 0;
		whvector<whnet_epoll_event>&	vec	= m_epSocket[epfd];
		switch (iOpCode)
		{
		case EPOLL_CTL_ADD:
			{
				if (vec.size() >= m_epSize[epfd])
				{
					return -1;
				}
				vec.push_back(*pEvent);
			}
			break;
		case EPOLL_CTL_DEL:
			{
				for (int i=0; i<vec.size(); i++)
				{
					if (vec[i].data.fd == fd)
					{
						*pEvent		= vec[i];
						vec.delvalue(vec[i]);
					}
				}
			}
			break;
		case EPOLL_CTL_MOD:
			{
				for (int i=0; i<vec.size(); i++)
				{
					if (vec[i].data.fd == fd)
					{
						vec[i]	= *pEvent;
					}
				}
			}
			break;
		default:
			{
				assert(0);
				iRst	= -2;
			}
			break;
		}
		return iRst;
	}
	int		EpollWait(int epfd, whnet_epoll_event* pEvents, int iMaxEvents, int iTimeout)
	{
		if (epfd < 0)
		{
			return 0;
		}
		whvector<whnet_epoll_event>&	vec	= m_epSocket[epfd];
		fd_set	fdRead;
		fd_set	fdWrite;
		fd_set	fdErr;
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdErr);
		whvector<SOCKET>	vecFdRead;
		whvector<SOCKET>	vecFdWrite;
		whvector<SOCKET>	vecFdErr;
		SOCKET	sockMax	= 0;
		for (int i=0; i<vec.size(); i++)
		{
			whnet_epoll_event&	epEvent	= vec[i];
			if (epEvent.events & EPOLLIN)
			{
				FD_SET(epEvent.data.fd, &fdRead);
				vecFdRead.push_back(epEvent.data.fd);
			}
			if (epEvent.events & EPOLLOUT)
			{
				FD_SET(epEvent.data.fd, &fdWrite);
				vecFdWrite.push_back(epEvent.data.fd);
			}
			if (epEvent.events & EPOLLERR)
			{
				FD_SET(epEvent.data.fd, &fdErr);
				vecFdErr.push_back(epEvent.data.fd);
			}
			if (epEvent.data.fd > sockMax)
			{
				sockMax		= epEvent.data.fd;
			}
		}
		timeval	tv;
		cmn_set_timevalfromms(tv, iTimeout);
		int		iRst		= select(sockMax+1, &fdRead, &fdWrite, &fdErr, &tv);
		if (iRst < 0)
		{
			return iRst;
		}
		map_socket2event_T	mapSock2Event;
		if (iRst > 0)
		{
			for (int i=0; i<vecFdRead.size(); i++)
			{
				if (FD_ISSET(vecFdRead[i], &fdRead))
				{
					map_socket2event_T::iterator	it	= mapSock2Event.find(vecFdRead[i]);
					if (it != mapSock2Event.end())
					{
						it->second.events	|= EPOLLIN;
					}
					else
					{
						whnet_epoll_event	ev;
						ev.data.fd			= vecFdRead[i];
						ev.events			= EPOLLIN;
						mapSock2Event.insert(map_socket2event_T::value_type(vecFdRead[i], ev));
					}
				}
			}
			for (int i=0; i<vecFdWrite.size(); i++)
			{
				if (FD_ISSET(vecFdWrite[i], &fdWrite))
				{
					map_socket2event_T::iterator	it	= mapSock2Event.find(vecFdWrite[i]);
					if (it != mapSock2Event.end())
					{
						it->second.events	|= EPOLLOUT;
					}
					else
					{
						whnet_epoll_event	ev;
						ev.data.fd			= vecFdWrite[i];
						ev.events			= EPOLLOUT;
						mapSock2Event.insert(map_socket2event_T::value_type(vecFdWrite[i], ev));
					}
				}
			}
			for (int i=0; i<vecFdErr.size(); i++)
			{
				if (FD_ISSET(vecFdErr[i], &fdErr))
				{
					map_socket2event_T::iterator	it	= mapSock2Event.find(vecFdErr[i]);
					if (it != mapSock2Event.end())
					{
						it->second.events	|= EPOLLERR;
					}
					else
					{
						whnet_epoll_event	ev;
						ev.data.fd			= vecFdErr[i];
						ev.events			= EPOLLERR;
						mapSock2Event.insert(map_socket2event_T::value_type(vecFdErr[i], ev));
					}
				}
			}
		}
		int		iRstSize	= 0;
		for (map_socket2event_T::iterator it=mapSock2Event.begin(); it!=mapSock2Event.end() && iRstSize<iMaxEvents; ++it)
		{
			pEvents[iRstSize]	= it->second;
			iRstSize++;
		}
		return iRstSize;
	}
private:
	whidxlist						m_epfdMgr;
	whvector<whnet_epoll_event>		m_epSocket[MAX_EPOLL_FD+1];
	int								m_epSize[MAX_EPOLL_FD+1];
};
static	epoll_mgr*	s_pEpollMgr		= NULL;
int		whnet_epoll_create(int iSize)
{
	if (s_pEpollMgr == NULL)
	{
		return -100;
	}
	return s_pEpollMgr->CreateEpfd(iSize);
}
int		whnet_epoll_ctl(int epfd, int iOpCode, int fd, whnet_epoll_event* pEvent)
{
	if (s_pEpollMgr == NULL)
	{
		return -100;
	}
	return s_pEpollMgr->EpollCtl(epfd, iOpCode, fd, pEvent);
}
int		whnet_epoll_wait(int epfd, whnet_epoll_event* pEvents, int iMaxEvents, int iTimeout)
{
	if (s_pEpollMgr == NULL)
	{
		return -100;
	}
	return s_pEpollMgr->EpollWait(epfd, pEvents, iMaxEvents, iTimeout);
}
int		whnet_epoll_close(int epfd)
{
	if (s_pEpollMgr == NULL)
	{
		return -100;
	}
	s_pEpollMgr->CloseEpfd(epfd);
	return 0;
}
int		whnet_epoll_init()
{
	if (s_pEpollMgr != NULL)
	{
		return -1;
	}
	s_pEpollMgr		= new epoll_mgr;
	return 0;
}
int		whnet_epoll_release()
{
	if (s_pEpollMgr == NULL)
	{
		return -1;
	}
	WHSafeDelete(s_pEpollMgr);
	return 0;
}
}

#endif

#ifdef __GNUC__
namespace n_whnet
{
int		whnet_epoll_create(int iSize)
{
	return epoll_create(iSize);
}
int		whnet_epoll_ctl(int epfd, int iOpCode, int fd, whnet_epoll_event* pEvent)
{
	return epoll_ctl(epfd, iOpCode, fd, pEvent);
}
int		whnet_epoll_wait(int epfd, whnet_epoll_event* pEvents, int iMaxEvents, int iTimeout)
{
	return epoll_wait(epfd, pEvents, iMaxEvents, iTimeout);
}
int		whnet_epoll_close(int epfd)
{
	close(epfd);
	return 0;
}
int		whnet_epoll_init()
{
	return 0;
}
int		whnet_epoll_release()
{
	return 0;
}
}
#endif

namespace n_whnet
{
//////////////////////////////////////////////////////////////////////////
//whnet_epoll_ir_obj
//////////////////////////////////////////////////////////////////////////
whnet_epoll_ir_obj::whnet_epoll_ir_obj()
{
	whnet_epoll_init();
}
whnet_epoll_ir_obj::~whnet_epoll_ir_obj()
{
	whnet_epoll_release();
}
//////////////////////////////////////////////////////////////////////////
//epoll_connecter
//////////////////////////////////////////////////////////////////////////
epoll_connecter::epoll_connecter()
: iErrorReson(no_error)
, bClosePassive(true)
, bClosing(false)
, iCloseExtData(0)
, iExt(0)
{
	for (int i=0; i<dlist_idx_max; i++)
	{
		arrDlistNode[i].data	= this;
	}
}
int		epoll_connecter::Init(epoll_connecter::info_T* pInfo)
{
	if (pInfo == NULL)
	{
		return -1;
	}
	if (&cntrInfo != pInfo)
	{
		memcpy(&cntrInfo, pInfo, sizeof(cntrInfo));
	}
	
	bufRecv.Init(cntrInfo.iRecvBufSize, 0, cntrInfo.iRecvBufSize/4);

	return 0;
}
void	epoll_connecter::Clear()
{
	for (int i=0; i<dlist_idx_max; i++)
	{
		arrDlistNode[i].leave();
	}

	ClearAllTE();
	
	whlist<data_unit_T>::iterator	it	= listDataSend.begin();
	for (; it != listDataSend.end(); ++it)
	{
		delete[] (*it).pData;
	}
	listDataSend.clear();
	bufRecv.Release();

	cntrInfo.Clear();

	iErrorReson		= no_error;
	bClosePassive	= true;
	iCloseExtData	= 0;
	bClosing		= false;
	iExt			= 0;
}
void	epoll_connecter::ClearAllTE()
{
	for (int i=0; i<te_idx_max; i++)
	{
		teid[i].quit();
	}
}
void	epoll_connecter::Release()
{
	cmn_safeclosesocket(cntrInfo.sock);
	Clear();
}
int		epoll_connecter::SendMsg(const void* pData, size_t iSize)
{
	if (cntrInfo.iStatus == info_T::status_error)
	{
		return -1;
	}
	if (cntrInfo.status_working == INVALID_SOCKET)
	{
		assert(0);
		return -2;
	}
// 	if (iSize > max_epoll_msg_data_size)
// 	{
// 		return -3;
// 	}
	int		iTotalSize			= sizeof(epoll_msg_head_t)+iSize;
	data_unit_T*	pDataUnit	= listDataSend.push_back();
	pDataUnit->pData			= new char[iTotalSize];
	pDataUnit->nSendedSize		= 0;
	pDataUnit->nSize			= iTotalSize;
	epoll_msg_head_t*	pHead	= (epoll_msg_head_t*)pDataUnit->pData;
	if (_SendMsgFillData(pHead, pData, iSize) < 0)
	{
		return -10;
	}

	if (!arrDlistNode[dlist_idx_having_data2send].isinlist())
	{
		cntrInfo.pHost->m_dlistCntrs[dlist_idx_having_data2send].AddToTail(&arrDlistNode[dlist_idx_having_data2send]);
	}

	return 0;
}
int		epoll_connecter::_SendMsgFillData(epoll_msg_head_t* pHead, const void* pData, size_t iSize)
{
	pHead->iSize		= iSize;
	pHead->iCntrID		= cntrInfo.iCntrID;
	memcpy(wh_getptrnexttoptr(pHead), pData, iSize);
	return 0;
}
int		epoll_connecter::RecvMsg(void* pData, size_t* piSize)
{
	epoll_msg_head_t*	pHead	= GetAvailMsgHead();
	if (pHead == NULL)
	{
		return 0;
	}
	if (pHead->iSize > (*piSize))
	{
		return -1;
	}
	int		iTotalSize			= sizeof(epoll_msg_head_t)+pHead->iSize;
	memcpy(pData, pHead, iTotalSize);
	if (pHead->iCntrID != cntrInfo.iCntrID)
	{
		SetStatus(info_T::status_error, why_error_cntrid_notmatch);
		return -3;
	}
	*piSize		= iTotalSize;
	FreeMsg();
	return 1;
}
const void*	epoll_connecter::PeekMsg(size_t* piSize)
{
	*piSize		= 0;
	epoll_msg_head_t*	pHead		= GetAvailMsgHead();
	if (pHead == NULL)
	{
		return NULL;
	}
	if (pHead->iCntrID != cntrInfo.iCntrID)
	{
		SetStatus(info_T::status_error, why_error_cntrid_notmatch);
		return NULL;
	}
	*piSize		= pHead->iSize+sizeof(epoll_msg_head_t);
	return pHead;
}
epoll_msg_head_t*	epoll_connecter::GetAvailMsgHead()
{
	if (bufRecv.GetSize() < min_epoll_msg_size)
	{
		return NULL;
	}
	epoll_msg_head_t*	pHead	= (epoll_msg_head_t*)bufRecv.GetBuf();
	if (pHead->iSize < min_epoll_msg_data_size)
	{
		SetStatus(info_T::status_error, why_error_datasize_zero);
		return NULL;
	}
	if (pHead->iSize > max_epoll_msg_data_size)
	{
		SetStatus(info_T::status_error, why_error_datasize_toobig);
		return NULL;
	}
	if ((pHead->iSize+sizeof(epoll_msg_head_t)) > bufRecv.GetSize())
	{
		return NULL;
	}
	return pHead;
}
void	epoll_connecter::FreeMsg()
{
	epoll_msg_head_t*	pHead	= GetAvailMsgHead();
	if (pHead == NULL)
	{
		return;
	}
	bufRecv.Out(sizeof(epoll_msg_head_t)+pHead->iSize);
}
size_t	epoll_connecter::GetNextSizeToRecv()
{
	epoll_msg_head_t*	pHead	= GetAvailMsgHead();
	if (pHead != NULL)
	{
		return pHead->iSize+sizeof(epoll_msg_head_t);
	}
	return 0;
}
void	epoll_connecter::SetStatus(int iStatus, int iWhyErr/* =no_error */)
{
	if (cntrInfo.iStatus==info_T::status_error && iStatus==info_T::status_error)
	{
		//GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_CONNECTER_STATUS)"%d,%d,%d,%s", cntrInfo.iStatus, iErrorReson, iWhyErr, __FUNCTION__);
	}
	if (IsPassiveCloseStatus())
	{
		//被动关闭的超时等待
		return;
	}
	//交换key的时候出错,不认为是真正的连接上
	if (cntrInfo.iStatus==info_T::status_exchanging_key && iStatus==info_T::status_error)
	{
		iErrorReson			= iWhyErr;
		cntrInfo.pHost->m_dlistCntrs[dlist_idx_cntr_err].AddToTail(&arrDlistNode[dlist_idx_cntr_err]);
		return;
	}
	if (IsPassiveCloseStatus() && iStatus==info_T::status_error)
	{
		//出现这种情况,是因为对方closesocket了
		iErrorReson			= why_error_passive_close;
	}
	else
	{
		cntrInfo.iStatus		= iStatus;
		iErrorReson				= iWhyErr;
	}
	
	if (iStatus == info_T::status_error)
	{
		if (!arrDlistNode[dlist_idx_cntr_err].isinlist())
		{
			cntrInfo.pHost->m_dlistCntrs[dlist_idx_cntr_err].AddToTail(&arrDlistNode[dlist_idx_cntr_err]);
		}
	}
}
int		epoll_connecter::ManualRecv()
{
	if (bufRecv.GetSizeLeft() <= 0)
	{
		return -1;
	}
	cntrInfo.tLastRecv		= wh_gettickcount();
	int		iTotalSize		= 0;
	int		iRecvSize		= 0;
	while (bufRecv.GetSizeLeft()>0
		&& (iRecvSize=recv(cntrInfo.sock, bufRecv.GetTail(), bufRecv.GetSizeLeft(), 0))>0)
	{
		iTotalSize			+= iRecvSize;
		bufRecv.InAlloc(iRecvSize);
	}
	if (iTotalSize == 0)
	{
		SetStatus(info_T::status_error, why_error_recv);
		return -2;
	}
	int	iDiff		= wh_tickcount_diff(wh_gettickcount(), cntrInfo.tLastRecv);
	if (iDiff > epoll_max_send_recv_time)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_RECV)"recv time too long,%d,%s", iDiff, __FUNCTION__);
	}
	epoll_msg_head_t*	pHead		= GetAvailMsgHead();
	while (pHead != NULL)
	{
		epoll_msg_base_t*	pBase	= (epoll_msg_base_t*)wh_getptrnexttoptr(pHead);
		switch (pBase->iCmd)
		{
		case epoll_msg_exchange_key:
			{
				cntrInfo.pHost->_Deal_RecvExchangeKey(this, (epoll_msg_exchange_key_t*)pBase, pHead->iSize);
			}
			break;
		case epoll_msg_ka:
			{
				cntrInfo.pHost->_Deal_RecvKA(this, (epoll_msg_ka_t*)pBase);
			}
			break;
		case epoll_msg_close:
			{
				cntrInfo.pHost->_Deal_RecvClose(this, (epoll_msg_close_t*)pBase);
			}
			break;
		default:
			{
				if (!arrDlistNode[dlist_idx_having_data2recv].isinlist())
				{
					cntrInfo.pHost->m_dlistCntrs[dlist_idx_having_data2recv].AddToTail(&arrDlistNode[dlist_idx_having_data2recv]);
				}
				return 0;
			}
			break;
		}
		if (cntrInfo.iSendBufSize == info_T::status_nothing)
		{
			// 说明在上面的某个操作中,该cntr已经被释放掉了
			return 0;
		}
		bufRecv.Out(sizeof(epoll_msg_head_t)+pHead->iSize);
		pHead				= GetAvailMsgHead();
	}
	return 0;
}
int		epoll_connecter::ManualSend()
{
	cntrInfo.tLastSend			= wh_gettickcount();
	int		iSendSize			= 0;
	int		iTotalSize			= 0;
	bool	bNeedBreak			= false;
	while (listDataSend.size() > 0)
	{
		data_unit_T*	pDataUnit	= (data_unit_T*)&(*listDataSend.begin());
		iSendSize		= send(cntrInfo.sock, pDataUnit->pData+pDataUnit->nSendedSize, pDataUnit->nSize-pDataUnit->nSendedSize, 0);
		if (iSendSize <= 0)
		{
			break;
		}
		if (iSendSize < (pDataUnit->nSize-pDataUnit->nSendedSize))	// 发不完,说明也需要退出了
		{
			bNeedBreak	= true;
		}
		pDataUnit->nSendedSize		+= iSendSize;
		iTotalSize		+= iSendSize;
		if (bNeedBreak)
		{
			break;
		}
		if (pDataUnit->nSendedSize >= pDataUnit->nSize)
		{
			// 发送完了
			delete[] pDataUnit->pData;
			listDataSend.pop_front();
		}
	}
	if (iTotalSize == 0)
	{
		SetStatus(info_T::status_error, why_error_send);
		return -1;
	}
	int		iDiff				= wh_tickcount_diff(wh_gettickcount(), cntrInfo.tLastSend);
	if (iDiff > epoll_max_send_recv_time)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_SEND)"send time too long,%d,%s", iDiff, __FUNCTION__);
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//epoll_server
//////////////////////////////////////////////////////////////////////////
epoll_server::epoll_server()
: m_tickNow(0)
, m_pEvents(NULL)
, m_listenSock(INVALID_SOCKET)
, m_iMaxEventNum(0)
, m_epfd(-1)
{
	cmn_set_reuseaddr(true);
}
epoll_server::~epoll_server()
{
	Release();
}
int		epoll_server::Init(info_T* pInfo)
{
	MakeTickNow();

	if (pInfo != &m_info)
	{
		memcpy(&m_info, pInfo, sizeof(m_info));
	}

	m_iMaxEventNum		= m_info.iMaxConnections+1;

	m_vectCodeDataBuf.reserve(m_info.iCodeDataBufSize);
	m_vectCodeKeyBuf.reserve(m_info.iCodeKeyBufSize);
	m_vectCodeNKeyBuf.reserve(m_info.iCodeNKeyBufSize);

	try
	{
		if (_CreateListenSocket() < 0)
		{
			throw -1;
		}
		if ((m_epfd=whnet_epoll_create(m_iMaxEventNum)) < 0)
		{
			throw -2;
		}
		if (_RegEpollEventAndSetSocketOpt(m_listenSock) < 0)
		{
			throw -3;
		}

		m_vectRawBuf_Tmp.reserve(max_epoll_msg_size);
		m_vectRawBuf_Recv.reserve(max_epoll_msg_size);
		m_vectCntrTmp.reserve(1000);

		whtimequeue::INFO_T		tqInfo;
		tqInfo.nUnitLen			= sizeof(TQUNIT_T);
		tqInfo.nChunkSize		= m_info.iTQChunkSize;
		if (m_tq.Init(&tqInfo) < 0)
		{
			throw -4;
		}
		if (m_connecters.Init(m_info.iMaxConnections) < 0)
		{
			throw -5;
		}
		if (m_queueCtrlOut.Init(m_info.iCtrlOutQueueSize) < 0)
		{
			throw -6;
		}
	}
	catch (int iErrCode)
	{
		cmn_safeclosesocket(m_listenSock);
		return iErrCode;
	}

	m_pEvents		= new whnet_epoll_event[m_iMaxEventNum];
	return 0;
}
int		epoll_server::Release()
{
	CloseAll();
	whtick_t	t	= wh_gettickcount();
	
	//关闭监听套接字(这样就不会接收新连接了)
	_DelEpollEvent(m_listenSock);
	cmn_safeclosesocket(m_listenSock);

	while (wh_tickcount_diff(wh_gettickcount(), t) < 2*m_info.iCloseTimeout)
	{
		TickRecv();
		TickLogic();
		TickSend();
		if (m_connecters.size() == 0)
		{
			break;
		}
	}

	whunitallocatorFixed<epoll_connecter>::iterator it	= m_connecters.begin();
	for (; it!=m_connecters.end(); ++it)
	{
		(*it).Release();
	}

	m_connecters.Release();

	m_tq.Release();
	m_queueCtrlOut.Release();

	m_sock2clientID.clear();

	whnet_epoll_close(m_epfd);
	m_epfd			= -1;
	if (m_pEvents)
	{
		delete[] m_pEvents;
		m_pEvents	= NULL;
	}

	return 0;
}
int		epoll_server::ControlOut(ctrl_out_T** ppCtrlOut, size_t* piSize)
{
	*piSize		= m_queueCtrlOut.PeekSize();
	if (*piSize <= 0)
	{
		return -1;
	}
	m_vectCtrlOutCmd.resize(*piSize);
	*ppCtrlOut	= (ctrl_out_T*)m_vectCtrlOutCmd.getbuf();
	return m_queueCtrlOut.Out(*ppCtrlOut, piSize);
}
int		epoll_server::TickRecv()
{
	int	iEventNum		= whnet_epoll_wait(m_epfd, m_pEvents, m_iMaxEventNum, 0);
	if (iEventNum < 0)
	{
		return -1;
	}
	else if (iEventNum > 0)
	{
		for (int i=0; i<iEventNum; i++)
		{
			if (m_pEvents[i].data.fd == m_listenSock)
			{
				sockaddr_in	addr;
				socklen_t	iAddrSize	= sizeof(addr);
				SOCKET		sock		= accept(m_listenSock, (sockaddr*)&addr, &iAddrSize);
				if (sock == INVALID_SOCKET)
				{
					int	iErrCode		= cmn_getsockerror(m_listenSock);
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_ACCEPT)"accept INVALID_SOCKET,%d,%s", iErrCode, __FUNCTION__);
					continue;
				}
				if (m_connecters.size() >= m_info.iMaxConnections)
				{
					cmn_safeclosesocket(sock);
					continue;
				}
				if (_RegEpollEventAndSetSocketOpt(sock) < 0)
				{
					cmn_safeclosesocket(sock);
					continue;
				}
				int		iCntrID		= 0;
				int		iRst		= AddConnector(sock, &iCntrID, addr);
				if (iRst != add_cntr_rst_ok)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_ADD_CNTR)"add cntr error,%d,%s", iRst, __FUNCTION__);
					_DelEpollEvent(sock);
					cmn_safeclosesocket(sock);
					continue;
				}
			}
			else if (m_pEvents[i].events & EPOLLIN)
			{
				epoll_connecter*	pCntr	= _GetCntrBySocket(m_pEvents[i].data.fd);
				if (pCntr == NULL)
				{
#ifdef _DEBUG
					assert(0);	// linux下assert在release版本有效
#endif
					_DelEpollEvent(m_pEvents[i].data.fd);
					m_sock2clientID.erase(m_pEvents[i].data.fd);
#ifdef __GNUC__
					int sock = m_pEvents[i].data.fd;
					cmn_safeclosesocket(sock);
                    m_pEvents[i].data.fd = sock;
#endif
#ifdef WIN32
					cmn_safeclosesocket(m_pEvents[i].data.fd);
#endif
					continue;
				}
				pCntr->ManualRecv();
			}
		}
	}
	else if (m_dlistCntrs[epoll_connecter::dlist_idx_having_data2send].size() == 0)
	{
		wh_sleep(10);		//防止CPU空转
	}
	return iEventNum;
}
int		epoll_server::TickLogic()
{
	_TickLogic_TE();
	_TickLogic_DealCntrInDList(epoll_connecter::dlist_idx_cntr_err, &epoll_server::_DealCntrInDList_Err);
	return 0;
}
int		epoll_server::TickSend()
{
	whDList<epoll_connecter*>&			dlist	= m_dlistCntrs[epoll_connecter::dlist_idx_having_data2send];
	whDList<epoll_connecter*>::node*	pNode	= dlist.begin();
	m_vectCntrTmp.resize(0);
	while (pNode != dlist.end())
	{
		epoll_connecter*	pCntr	= pNode->data;
		_ModEpollEvent(pCntr->cntrInfo.sock, EPOLLOUT);
		pNode->leave();
		pNode						= dlist.begin();
		m_vectCntrTmp.push_back(pCntr);
	}
	whDList<epoll_connecter*>		dlistTmp;
	int		iEventNum				= whnet_epoll_wait(m_epfd, m_pEvents, m_iMaxEventNum, 0);
	if (iEventNum < 0)
	{
		return -1;
	}
	for (int i=0; i<iEventNum; i++)
	{
		if (m_pEvents[i].events&EPOLLOUT)
		{
			epoll_connecter* pCntr	= _GetCntrBySocket(m_pEvents[i].data.fd);
			if (pCntr)
			{
				pCntr->ManualSend();
			}
			else
			{
				assert(0);
			}
		}
	}
	for (size_t i=0; i<m_vectCntrTmp.size(); i++)
	{
		epoll_connecter*	pCntr	= m_vectCntrTmp[i];
		if (!pCntr->IsStatusError())
		{
			_ModEpollEvent(pCntr->cntrInfo.sock, EPOLLIN);
			if (pCntr->listDataSend.size() > 0)
			{
				dlist.AddToTail(&pCntr->arrDlistNode[epoll_connecter::dlist_idx_having_data2send]);
			}
		}
	}
	return 0;
}
int		epoll_server::_TickLogic_TE()
{
	MakeTickNow();
	whtimequeue::ID_T		id;
	TQUNIT_T*				pTQUnit	= NULL;
	while (m_tq.GetUnitBeforeTime(m_tickNow, (void**)&pTQUnit, &id) == 0)
	{
		(this->*pTQUnit->teFunc)(pTQUnit);
		m_tq.Del(id);
	}
	return 0;
}
int		epoll_server::_TickLogic_DealCntrInDList(int iIdx, deal_cntr_func_T pFunc)
{
	whDList<epoll_connecter*>&			dlist	= m_dlistCntrs[iIdx];
	whDList<epoll_connecter*>::node*	pNode	= dlist.begin();
	while (pNode != dlist.end())
	{
		(this->*pFunc)(pNode->data);
		pNode->leave();
		pNode		= dlist.begin();
	}
	return 0;
}
void	epoll_server::_DealCntrInDList_Err(epoll_connecter* pCntr)
{
	int		iReason		= close_reason_unknown;
	switch (pCntr->iErrorReson)
	{
	case epoll_connecter::why_error_cntrid_notmatch:
		{
			iReason		= close_reason_cntrid_notmatch;
		}
		break;
	case epoll_connecter::why_error_datasize_zero:
		{
			iReason		= close_reason_datasize_zero;
		}
		break;
	case epoll_connecter::why_error_datasize_toobig:
		{
			iReason		= close_reason_datasize_toobig;
		}
		break;
	case epoll_connecter::why_error_key_notagree:
		{
			iReason		= close_reason_key_not_agree;
		}
		break;
	case epoll_connecter::why_error_decrypt:
		{
			iReason		= close_reason_decrypt;
		}
		break;
	case epoll_connecter::why_error_encrypt:
		{
			iReason		= close_reason_encrypt;
		}
		break;
	case epoll_connecter::why_error_recv:
		{
			iReason		= close_reason_recv;
		}
		break;
	case epoll_connecter::why_error_send:
		{
			iReason		= close_reason_send;
		}
		break;
	case epoll_connecter::why_error_passive_close:
		{
			iReason		= close_reason_passive;
			break;
		}
	default:
		{
			//assert(0);
		}
		break;
	}
	if (pCntr->cntrInfo.iStatus == epoll_connecter::info_T::status_exchanging_key)
	{
		RemoveConnector(pCntr);
	}
	else
	{
		ControlOutReasonAndRemoveCntr(pCntr, ctrl_out_cntr_closed, iReason);
	}
}
int		epoll_server::ControlOutReasonAndRemoveCntr(epoll_connecter* pCntr, int iCmd, int iReason)
{
	ctrl_out_cntr_close_reason_T*	pCtrlOut	= NULL;
	_CtrlOutAlloc(pCtrlOut);
	if (pCtrlOut != NULL)
	{
		pCtrlOut->iCmd		= iCmd;
		pCtrlOut->iCntrID	= pCntr->cntrInfo.iCntrID;
		pCtrlOut->addrPeer	= pCntr->cntrInfo.addrPeer;
		pCtrlOut->iReason	= iReason;
		pCtrlOut->iExtData	= pCntr->iCloseExtData;
		pCtrlOut->iCntrExt	= pCntr->iExt;
	}
	return RemoveConnector(pCntr);
}
int		epoll_server::AddConnector(SOCKET sock, int* piCntrID, const sockaddr_in& addrPeer)
{
	MakeTickNow();

	int		iTmpCntrID			= _GetCntrIDBySocket(sock);
	if (iTmpCntrID >= 0)
	{
		assert(0);
		m_connecters.FreeUnit(iTmpCntrID);
		_EraseSocket2CntrIDMap(sock);
	}

	epoll_connecter*	pCntr	= NULL;
	int		iCntrID				= m_connecters.AllocUnit(pCntr);
	if (iCntrID < 0)
	{
		return add_cntr_rst_full;
	}

	pCntr->Clear();
	pCntr->cntrInfo.iStatus			= epoll_connecter::info_T::status_exchanging_key;
	pCntr->cntrInfo.addrPeer		= addrPeer;
	pCntr->cntrInfo.iSendBufSize	= m_info.iSendBufSize;
	pCntr->cntrInfo.iRecvBufSize	= m_info.iRecvBufSize;
	pCntr->cntrInfo.sock			= sock;
	pCntr->cntrInfo.iCntrID			= iCntrID;
	pCntr->cntrInfo.pHost			= this;
	
	if (pCntr->Init(&pCntr->cntrInfo) < 0)
	{
		m_connecters.FreeUnit(iCntrID);
		return add_cntr_rst_err_init;
	}

	//1.生成密钥
	unsigned char	szKey[max_key_len];
	unsigned int	nPass			= wh_time()^rand();
	whmd5(&nPass, sizeof(nPass), szKey);
	int		iSize					= WHMD5LEN;
	pCntr->SetCodeKey(szKey);
	//2.组装包
	int		iTotalSize				= sizeof(epoll_msg_head_t)+sizeof(epoll_msg_exchange_key_t)-1+iSize;
	m_vectRawBuf_Tmp.resize(iTotalSize);
	epoll_msg_head_t*	pHead		= (epoll_msg_head_t*)m_vectRawBuf_Tmp.getbuf();
	epoll_msg_exchange_key_t*	pPack	= (epoll_msg_exchange_key_t*)wh_getptrnexttoptr(pHead);
	pPack->iCmd						= epoll_msg_exchange_key;
	pPack->iCryptType				= 0;
	pPack->iKATimeout				= m_info.iKATimeout;
	pPack->iKeyLen					= iSize;
	memcpy(pPack->szKey, szKey, iSize);
	//包头数据
	pHead->iSize					= iTotalSize-sizeof(epoll_msg_head_t);
	pHead->iCntrID					= iCntrID;
	//3.发包
	{
		char*	pBuf				= m_vectRawBuf_Tmp.getbuf();
		int		iLen				= iTotalSize;
		int		iSendSize			= 0;
		whtick_t	t1				= wh_gettickcount();
		while (iLen>0 && (iSendSize=send(sock, pBuf, iLen, 0)) > 0)
		{
			iLen					-= iSendSize;
			pBuf					+= iSendSize;
		}
		whtick_t	t2				= wh_gettickcount();
		if (wh_tickcount_diff(t2, t1) > epoll_max_send_recv_time)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_EXCHANGE_KEY_SEND)"t1:%u,t2:%u", t1, t2);
		}
		if (iLen > 0)
		{
			m_connecters.FreeUnit(iCntrID);
			return add_cntr_rst_err_send_key;
		}
	}
	//4.注册密钥超时事件
	RegExchangeKeyTQ(pCntr);

	m_sock2clientID.insert(map<SOCKET, int>::value_type(sock, iCntrID));
	*piCntrID		= iCntrID;

	return add_cntr_rst_ok;
}
int		epoll_server::RemoveConnector(int iCntrID)
{
	epoll_connecter*	pCntr	= m_connecters.getptr(iCntrID);
	if (pCntr == NULL)
	{
		return -1;
	}
	return RemoveConnector(pCntr);
}
int		epoll_server::RemoveConnector(epoll_connecter* pCntr)
{
	m_sock2clientID.erase(pCntr->cntrInfo.sock);
	_DelEpollEvent(pCntr->cntrInfo.sock);
	int		iCntrID	= pCntr->cntrInfo.iCntrID;
	pCntr->Release();
	m_connecters.FreeUnit(iCntrID);

	return 0;
}
int		epoll_server::Close(int iCntrID, int iCloseExtData)
{
	epoll_connecter*	pCntr	= m_connecters.getptr(iCntrID);
	if (pCntr == NULL)
	{
		return -1;
	}
	if (Close(pCntr) < 0)
	{
		return -2;
	}
	pCntr->iCloseExtData		= iCloseExtData;
	return 0;
}
int		epoll_server::Close(epoll_connecter* pCntr)
{
	if (pCntr->cntrInfo.iStatus == epoll_connecter::info_T::status_closing)
	{
		return -1;
	}
	SetStatusClose(pCntr, false);
	return 0;
}
int		epoll_server::SetStatusClose(epoll_connecter* pCntr, bool bClosePassive)
{
	pCntr->bClosePassive		= bClosePassive;
	pCntr->cntrInfo.iStatus		= epoll_connecter::info_T::status_closing;
	SendClose(pCntr);
	RegCloseTQ(pCntr);
	pCntr->SetStatus(epoll_connecter::info_T::status_closing);
	return 0;
}
int		epoll_server::CloseAll()
{
	whunitallocatorFixed<epoll_connecter>::iterator	it;
	for (it=m_connecters.begin(); it!=m_connecters.end(); ++it)
	{
		Close(&(*it));
	}
	return 0;
}
int		epoll_server::CloseOnErr(epoll_connecter* pCntr, int iErrCode/* =close_on_err_unknown */)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_CLOSE_CONNECTER)"%d,%d", pCntr->cntrInfo.iCntrID, iErrCode);
	return Close(pCntr);
}
void	epoll_server::SendClose(epoll_connecter* pCntr)
{
	epoll_msg_close_t	pack;
	pack.iCmd				= epoll_msg_close;
	pack.iExtData			= pCntr->iCloseExtData;
	pCntr->bClosing			= true;
	pCntr->SendMsg(&pack, sizeof(pack));
}
bool	epoll_server::IsCntrExist(int iCntrID)
{
	return m_connecters.IsUnitUsed(iCntrID);
}
int		epoll_server::GetExt(int iCntrID, int* piExt)
{
	epoll_connecter*	pCntr	= m_connecters.getptr(iCntrID);
	if (pCntr == NULL)
	{
		return -1;
	}
	*piExt	= pCntr->iExt;
	return 0;
}
int		epoll_server::SetExt(int iCntrID, int iExt)
{
	epoll_connecter*	pCntr	= m_connecters.getptr(iCntrID);
	if (pCntr == NULL)
	{
		return -1;
	}
	pCntr->iExt	= iExt;
	return 0;
}
void	epoll_server::SetCntr_ConnectExtData(int iCntrID, void* pData, size_t iSize)
{
	epoll_connecter*	pCntr	= m_connecters.getptr(iCntrID);
	if (pCntr == NULL)
	{
		return;
	}
	pCntr->vectCntrExtData.resize(iSize);
	memcpy(pCntr->vectCntrExtData.getbuf(), pData, iSize);
}
void*	epoll_server::GetCntr_ConnectExtData(int iCntrID, size_t* piSize)
{
	epoll_connecter*	pCntr	= m_connecters.getptr(iCntrID);
	if (pCntr==NULL || pCntr->vectCntrExtData.size()==0)
	{
		return NULL;
	}
	*piSize	= pCntr->vectCntrExtData.size();
	return pCntr->vectCntrExtData.getbuf();
}
int		epoll_server::Send(int iCntrID, const void* pData, size_t iSize)
{
// 	if (iSize > max_epoll_msg_data_size)
// 	{
// 		assert(0);
// 		return -1;
// 	}
	epoll_connecter*	pCntr	= m_connecters.getptr(iCntrID);
	if (pCntr == NULL)
	{
		return -2;
	}
	m_vectRawBuf_Tmp.resize(sizeof(epoll_msg_data_t)+iSize);
	epoll_msg_data_t*	pMD		= (epoll_msg_data_t*)m_vectRawBuf_Tmp.getbuf();
	pMD->iCmd					= epoll_msg_data;
	pMD->iCrc					= crc8((char*)pData, (int)iSize);
	unsigned char*	pDataBuf	= (unsigned char*)wh_getptrnexttoptr(pMD);
	memcpy(pDataBuf, pData, iSize);
	int		iRst				= pCntr->SendMsg(pMD, m_vectRawBuf_Tmp.size());
	if (iRst < 0)
	{
		return -100+iRst;
	}
	return 0;
}
int		epoll_server::Recv(int* piCntrID, void** ppData, size_t* piSize)
{
	whDList<epoll_connecter*>&			dlist	= m_dlistCntrs[epoll_connecter::dlist_idx_having_data2recv];
	whDList<epoll_connecter*>::node*	pNode	= dlist.begin();
	void*	pBuf		= m_vectRawBuf_Recv.getbuf();
	*piSize				= m_vectRawBuf_Recv.capacity();
begin:
	while (pNode != dlist.end())
	{
		epoll_connecter*	pCntr	= pNode->data;
		while (pCntr->RecvMsg(pBuf, piSize) > 0)
		{
			epoll_msg_head_t*	pHead	= (epoll_msg_head_t*)pBuf;
			epoll_msg_base_t*	pBase	= (epoll_msg_base_t*)wh_getptrnexttoptr(pHead);
			switch (pBase->iCmd)
			{
			case epoll_msg_ka:
				{
					_Deal_RecvKA(pCntr, (epoll_msg_ka_t*)pBase);
				}
				break;
			case epoll_msg_close:
				{
					_Deal_RecvClose(pCntr, (epoll_msg_close_t*)pBase);
				}
				break;
			case epoll_msg_data:
				{
					if (pCntr->cntrInfo.iStatus!=epoll_connecter::info_T::status_working)
					{
						// 还没开始工作就发来数据或者发来不认识的消息
						pNode->leave();
						pNode			= dlist.begin();
						RemoveConnector(pCntr);
						continue;
					}
					
					epoll_msg_data_t*	pMsgData	= (epoll_msg_data_t*)pBase;
					*piCntrID	= pHead->iCntrID;
					*piSize		-= (sizeof(epoll_msg_head_t)+sizeof(epoll_msg_data_t));
					*ppData		= wh_getptrnexttoptr(pMsgData);

					// crc校验
					if (pMsgData->iCrc != crc8((char*)*ppData, (int)*piSize))
					{
						// crc校验出错
						pNode->leave();
						pNode			= dlist.begin();
						RemoveConnector(pCntr);
						goto begin;
					}
					else
					{
						return 1;
					}
				}
				break;
			default:
				{
					// 还没开始工作就发来数据或者发来不认识的消息
					pNode->leave();
					pNode			= dlist.begin();
					RemoveConnector(pCntr);
					goto begin;
				}
				break;
			}
		}
		pNode->leave();
		pNode			= dlist.begin();
	}
	return 0;
}
void	epoll_server::TEDeal_ExchangeKey(TQUNIT_T* pTE)
{
	RemoveConnector(pTE->pCntr);
}
void	epoll_server::TEDeal_Drop(TQUNIT_T* pTE)
{
	epoll_connecter*	pCntr	= pTE->pCntr;
	if (wh_tickcount_diff(m_tickNow, pCntr->cntrInfo.tLastRecv) >= m_info.iDropTimeout)
	{
		ControlOutReasonAndRemoveCntr(pCntr, ctrl_out_cntr_closed, close_reason_drop);
	}
	else
	{
		RegDropTQ(pCntr);
	}
}
void	epoll_server::TEDeal_Close(TQUNIT_T* pTE)
{
	if (pTE->pCntr->IsPassiveCloseStatus())
	{
		ControlOutReasonAndRemoveCntr(pTE->pCntr, ctrl_out_cntr_closed, close_reason_passive);
	}
	else
	{
		ControlOutReasonAndRemoveCntr(pTE->pCntr, ctrl_out_cntr_closed, close_reason_close_timeout);
	}
}
void	epoll_server::TEDeal_SendKA(TQUNIT_T* pTE)
{
	epoll_msg_ka_t		kaMsg;
	kaMsg.iCmd			= epoll_msg_ka;
	pTE->pCntr->SendMsg(&kaMsg, sizeof(kaMsg));
	RegSendKATQ(pTE->pCntr);
}
void	epoll_server::RegExchangeKeyTQ(epoll_connecter* pCntr)
{
	TQUNIT_T		tqu;
	tqu.pCntr		= pCntr;
	tqu.teFunc		= &epoll_server::TEDeal_ExchangeKey;
	m_tq.Add(m_tickNow+m_info.iExchangeKeyTimeout, &tqu, &pCntr->teid[epoll_connecter::te_idx_exchange_key]);
}
void	epoll_server::RegDropTQ(epoll_connecter* pCntr)
{
	TQUNIT_T		tqu;
	tqu.pCntr		= pCntr;
	tqu.teFunc		= &epoll_server::TEDeal_Drop;
	m_tq.Add(m_tickNow+m_info.iDropTimeout, &tqu, &pCntr->teid[epoll_connecter::te_idx_drop]);
}
void	epoll_server::RegCloseTQ(epoll_connecter* pCntr)
{
	TQUNIT_T		tqu;
	tqu.pCntr		= pCntr;
	tqu.teFunc		= &epoll_server::TEDeal_Close;
	m_tq.Add(m_tickNow+m_info.iCloseTimeout, &tqu, &pCntr->teid[epoll_connecter::te_idx_close]);
}
void	epoll_server::RegSendKATQ(epoll_connecter* pCntr)
{
	TQUNIT_T		tqu;
	tqu.pCntr		= pCntr;
	tqu.teFunc		= &epoll_server::TEDeal_SendKA;
	m_tq.Add(m_tickNow+m_info.iKATimeout, &tqu, &pCntr->teid[epoll_connecter::te_idx_send_ka]);
}
int		epoll_server::_Deal_RecvKA(epoll_connecter* pCntr, epoll_msg_ka_t* pMsgKA)
{
	return 0;		//就是告诉服务器我还活着
}
int		epoll_server::_Deal_RecvClose(epoll_connecter* pCntr, epoll_msg_close_t* pMsgClose)
{
	if (!pCntr->bClosePassive)
	{
		ControlOutReasonAndRemoveCntr(pCntr, ctrl_out_cntr_closed, close_reason_initiative);
	}
	else
	{
		SetStatusClose(pCntr, true);
	}
	return 0;
}
int		epoll_server::_Deal_RecvExchangeKey(epoll_connecter* pCntr, epoll_msg_exchange_key_t* pMsgExKey, int iSize)
{
	if (pCntr->cntrInfo.iStatus == epoll_connecter::info_T::status_exchanging_key)
	{
		bool	bHasExtData	= false;
		size_t	iExtDataLen	= 0;
		void*	pExtData	= NULL;
		// 看看key有没有超长
		{
			if (pMsgExKey->iKeyLen>max_key_len || pMsgExKey->iKeyLen<0)
			{
				RemoveConnector(pCntr->cntrInfo.iCntrID);
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_ADD_CNTR)"invalid key len,%d", pMsgExKey->iKeyLen);
			}
			int iRealKeyLen	= iSize-(sizeof(epoll_msg_exchange_key_t)-1);
			if ((size_t)iRealKeyLen < pMsgExKey->iKeyLen)
			{
				RemoveConnector(pCntr->cntrInfo.iCntrID);
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_ADD_CNTR)"invalid key len,%d,%d", iRealKeyLen, pMsgExKey->iKeyLen);
			}
			else if ((size_t)iRealKeyLen > pMsgExKey->iKeyLen)
			{
				bHasExtData	= true;
				iExtDataLen	= iRealKeyLen-pMsgExKey->iKeyLen;
				pExtData	= wh_getoffsetaddr(pMsgExKey, iSize-iExtDataLen);
			}
		}
		// 看看有没有附加数据
		if (bHasExtData)
		{
			pCntr->vectCntrExtData.resize(iExtDataLen);
			memcpy(pCntr->vectCntrExtData.getbuf(), pExtData, iExtDataLen);
		}
		pCntr->cntrInfo.iStatus	= epoll_connecter::info_T::status_working;
		ctrl_out_cntr_accept_T*	pCtrlOut	= NULL;
		_CtrlOutAlloc(pCtrlOut);
		if (pCtrlOut != NULL)
		{
			pCtrlOut->iCmd		= epoll_server::ctrl_out_cntr_accepted;
			pCtrlOut->iCntrID	= pCntr->cntrInfo.iCntrID;
			pCtrlOut->addrPeer	= pCntr->cntrInfo.addrPeer;
			RegDropTQ(pCntr);
			// 也注册心跳包
			RegSendKATQ(pCntr);
		}
		else
		{
			RemoveConnector(pCntr->cntrInfo.iCntrID);
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,EPOLL_ADD_CNTR)"cannot ctrlout");
		}
	}
	return 0;
}

int		epoll_server::GetSockets(whvector<SOCKET>& vect)
{
	if (cmn_is_validsocket(m_listenSock))
	{
		vect.push_back(m_listenSock);
	}
	return vect.size();
}

int		epoll_server::GetCntrNum()
{
	return m_connecters.size();
}

void*	WHNET_Get_EpollSharedData()
{
#ifdef WIN32
	return s_pEpollMgr;
#else
	return NULL;
#endif
}
void	WHNET_Set_EpollSharedData(void* pData)
{
#ifdef WIN32
	s_pEpollMgr			= (epoll_mgr*)pData;
#endif
}
}
