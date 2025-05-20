// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetepoll.h
// Creator      : Yue Zhongyue
// Comment      : 简单的epoll模型
// CreationDate : 2011-11-02
// ChangeLog    :

#ifndef __WHNETEPOLL_H__
#define __WHNETEPOLL_H__

#include "whnetcmn.h"
#include "whnetepoll_packet.h"
#include "whnettcp.h"

#include "WHCMN/inc/whunitallocator.h"
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whtimequeue.h"
#include "WHCMN/inc/whDList.h"
#include "WHCMN/inc/whqueue.h"
#include "WHCMN/inc/whstreambuffer.h"
#include "WHCMN/inc/whcrc.h"

#include <map>
using namespace n_whcmn;
using namespace std;

#ifdef __GNUC__
#include <sys/epoll.h>
#endif

//发送和接收数据包的最大时间间隔,超过则记录日志
enum
{
	epoll_max_send_recv_time	= 10,
};

#ifdef WIN32
union whnet_epoll_data_t
{
	void*		ptr;
	SOCKET		fd;
	unsigned int		u32;
	n_whcmn::whuint64	u64;
};
struct whnet_epoll_event 
{
	unsigned int		events;
	whnet_epoll_data_t	data;
};
inline bool	operator==(const whnet_epoll_event& e1, const whnet_epoll_event& e2)
{
	return e1.data.fd==e2.data.fd && e1.events==e2.events;
}
enum
{
	EPOLLIN		= 0x001,
	EPOLLPRI	= 0x002,
	EPOLLOUT	= 0x004,
	EPOLLERR	= 0x008,
	EPOLLHUP	= 0x010,
	EPOLLET		= (1 << 31)
};
enum
{
	EPOLL_CTL_ADD	= 1,
	EPOLL_CTL_DEL	= 2,
	EPOLL_CTL_MOD	= 3,
};
#endif

#ifdef __GNUC__
typedef	epoll_data_t	whnet_epoll_data_t;
typedef	epoll_event		whnet_epoll_event;
#endif

namespace n_whnet
{
//使用完后记得关闭epoll句柄
int		whnet_epoll_create(int iSize);
//返回0表示成功,-1表示失败
int		whnet_epoll_ctl(int epfd, int iOpCode, int fd, whnet_epoll_event* pEvent);
//返回值表示通知数量,-1表示失败
int		whnet_epoll_wait(int epfd, whnet_epoll_event* pEvents, int iMaxEvents, int iTimeout);
//关闭epoll句柄
int		whnet_epoll_close(int epfd);

int		whnet_epoll_init();
int		whnet_epoll_release();

//初始化epoll相关功能
class whnet_epoll_ir_obj
{
public:
	whnet_epoll_ir_obj();
	~whnet_epoll_ir_obj();
};

//////////////////////////////////////////////////////////////////////////
//epoll服务器使用的连接器
//////////////////////////////////////////////////////////////////////////
class epoll_server;
class epoll_connecter
{
public:
	friend class epoll_server;
	enum
	{
		no_error							= 0,
		why_error_cntrid_notmatch			= 1,
		why_error_datasize_zero				= 2,
		why_error_datasize_toobig			= 3,
		why_error_key_notagree				= 4,
		why_error_decrypt					= 5,
		why_error_encrypt					= 6,
		why_error_recv						= 7,
		why_error_send						= 8,
		why_error_passive_close				= 9,
	};
	int		iErrorReson;
	struct info_T
	{
		enum
		{
			status_nothing					= 0,
			status_exchanging_key			= 1,
			status_working					= 2,
			status_closing					= 3,		// 这个状态可以考虑去掉
			status_error					= 4,
		};
		int									iStatus;
		int									iCntrID;
		sockaddr_in							addrPeer;
		int									iUpBPS;
		int									iDownBPS;

		whtick_t							tLastSend;
		whtick_t							tLastRecv;

		epoll_server*						pHost;
		SOCKET								sock;

		int									iSendBufSize;
		int									iRecvBufSize;

		void	Clear()
		{
			WHMEMSET0THIS();

			tLastSend		= wh_gettickcount();
			tLastRecv		= tLastSend;
			sock			= INVALID_SOCKET;
			iSendBufSize	= 0;
			iRecvBufSize	= 0;
			iCntrID			= -1;
			iStatus			= status_nothing;
		}
		info_T()
		{
			Clear();
		}
	};
	struct data_unit_T
	{
		size_t		nSize;
		size_t		nSendedSize;
		char*		pData;
	};
	info_T									cntrInfo;
	whstreambuffer							bufRecv;
	whlist<data_unit_T>						listDataSend;
	bool									bClosePassive;
	bool									bClosing;				// 是否处于结束状态
	int										iCloseExtData;
	int										iExt;					// 附加数据
	whvector<char>							vectCntrExtData;		// 连接附加数据

	// 编码器,加密解密相关
	unsigned char							szKeyD[WHMD5LEN];
	unsigned char							szKeyE[WHMD5LEN];


	void	SetCodeKey(const unsigned char* cszKey)
	{
		unsigned char	szTmpBuf[WHMD5LEN];
		whmd5(cszKey, WHMD5LEN, szTmpBuf);
		unsigned char	ucChar	= szTmpBuf[2];
		szTmpBuf[2]				= szTmpBuf[11];
		szTmpBuf[11]			= ucChar;
		ucChar					= szTmpBuf[5];
		szTmpBuf[5]				= szTmpBuf[7];
		szTmpBuf[7]				= ucChar;
		whmd5(szTmpBuf, WHMD5LEN, szTmpBuf);

		memcpy(szKeyD, szTmpBuf, WHMD5LEN);
		memcpy(szKeyE, szTmpBuf, WHMD5LEN);
	}
public:
	enum
	{
		te_idx_exchange_key					= 0,
		te_idx_drop							= 0,
		te_idx_close						= 0,
		te_idx_send_ka						= 1,		// 服务器也应该向客户端发送心跳包,这样才能保证链路层断开,客户端也能及时知道
		te_idx_max							= 2,
	};
	whtimequeue::ID_T						teid[te_idx_max];
	enum
	{
		dlist_idx_having_data2send			= 0,
		dlist_idx_having_data2recv			= 1,
		dlist_idx_cntr_err					= 2,
		dlist_idx_max						= 3,
	};
	whDList<epoll_connecter*>::node			arrDlistNode[dlist_idx_max];
public:
	epoll_connecter();
public:
	int		SendMsg(const void* pData, size_t iSize);
	int		RecvMsg(void* pData, size_t* piSize);
	const void*	PeekMsg(size_t* piSize);
	epoll_msg_head_t*	GetAvailMsgHead();
	int		_SendMsgFillData(epoll_msg_head_t* pHead, const void* pData, size_t iSize);
	void	FreeMsg();
	size_t	GetNextSizeToRecv();
	void	SetStatus(int iStatus, int iWhyErr=no_error);
	inline	bool	IsStatusError()
	{
		return cntrInfo.iStatus==info_T::status_error;
	}
public:
	int		ManualRecv();
	int		ManualSend();
public:
	int		Init(info_T* pInfo);
	void	Clear();					// Clear用于将数据设为无效值
	void	ClearAllTE();
	void	Release();					// Release用于释放数据	
public:
	inline	bool	IsPassiveCloseStatus()
	{
		if (bClosing && bClosePassive)
		{
			return true;
		}
		return false;
	}
};

//////////////////////////////////////////////////////////////////////////
//epoll实现的tcp服务器
//////////////////////////////////////////////////////////////////////////
class epoll_server
{
public:
	enum
	{
#ifdef WIN32
		epoll_max_connections		= 16,
#else
		epoll_max_connections		= 10000,
#endif
	};
	struct info_T : public whdataini::obj
	{
		int			iMaxConnections;
		int			iPort;
		int			iPort1;
		char		szIP[WHNET_MAXIPSTRLEN];
		bool		bNBO;
		int			iDropTimeout;				// 单位ms
		int			iKATimeout;					// 单位ms
		int			iCloseTimeout;				// 单位ms
		int			iExchangeKeyTimeout;		// 单位ms
		int			iSendBufSize;
		int			iRecvBufSize;
		int			iCtrlOutQueueSize;
		int			iTQChunkSize;

		bool		bNeedCode;
		int			iCodeDataBufSize;
		int			iCodeKeyBufSize;
		int			iCodeNKeyBufSize;

		info_T()
			: iMaxConnections(epoll_max_connections)
			, iPort(0), iPort1(0), bNBO(false)
			, iDropTimeout(10*1000)
			, iKATimeout(3*1000)
			, iCloseTimeout(2*1000)
			, iExchangeKeyTimeout(5*1000)
			, iSendBufSize(64*1024), iRecvBufSize(64*1024)
			, iCtrlOutQueueSize(512*1024), iTQChunkSize(100)
			, bNeedCode(false), iCodeDataBufSize(4*1024*1024)
			, iCodeKeyBufSize(WHMD5LEN*2), iCodeNKeyBufSize(WHMD5LEN*2)
		{
			szIP[0]		= 0;
		}

		WHDATAPROP_DECLARE_MAP(info_T);
	};
public:
	enum
	{
		ctrl_out_cntr_accepted				= 1,
		ctrl_out_cntr_closed				= 2,
		ctrl_out_refused				= 3,
		ctrl_out_should_stop			= 4,
	};
	enum
	{
		close_reason_unknown			= 0x00,
		close_reason_initiative			= 0x01,
		close_reason_passive			= 0x02,
		close_reason_close_timeout		= 0x03,
		close_reason_drop				= 0x04,
		close_reason_connect_timeout	= 0x05,
		close_reason_accept_timeout		= 0x06,
		close_reason_key_not_agree		= 0x07,
		close_reason_cntrid_notmatch	= 0x08,
		close_reason_datasize_zero		= 0x09,
		close_reason_datasize_toobig	= 0x0A,
		close_reason_decrypt			= 0x0B,
		close_reason_encrypt			= 0x0C,
		close_reason_recv				= 0x0D,
		close_reason_send				= 0x0E,
	};
	enum
	{
		connect_refuse_reason_unknown	= 0x00,
		connect_refuse_reason_bad_ver	= 0x01,
		connect_refuse_reason_slot_full	= 0x02,
		connect_refuse_reason_bad_data	= 0x03,
		connect_refuse_reason_inter_err	= 0xFF,
	};
#pragma pack(1)
	struct ctrl_out_T 
	{
		int				iCmd;
		int				data[1];
	};
	struct ctrl_out_cntr_accept_T 
	{
		int				iCmd;
		int				iCntrID;
		sockaddr_in		addrPeer;
	};
	struct ctrl_out_cntr_close_reason_T 
	{
		int				iCmd;
		int				iCntrID;
		sockaddr_in		addrPeer;
		int				iReason;
		int				iExtData;
		int				iCntrExt;
	};
#pragma pack()
public:
	struct TQUNIT_T;
	typedef	void	(epoll_server::*te_deal_func_T)(TQUNIT_T*);
#pragma pack(1)
	struct TQUNIT_T
	{
		epoll_connecter*	pCntr;
		te_deal_func_T		teFunc;

		TQUNIT_T()
		{
			WHMEMSET0THIS();
		}
	};
#pragma pack()
public:
	epoll_server();
	virtual	~epoll_server();
	int		Init(info_T* pInfo);
	int		Release();
	void	SelfDestroy()	{delete this;}
	int		ControlOut(ctrl_out_T** ppCtrlOut, size_t* piSize);
	int		ControlOutReasonAndRemoveCntr(epoll_connecter* pCntr, int iCmd, int iReason);
public:
	inline	void	MakeTickNow()
	{
		m_tickNow	= wh_gettickcount();
	}
public:
	virtual	int		TickRecv();
	virtual	int		TickLogic();
	virtual	int		TickSend();
	virtual	int		Close(int iCntrID, int iCloseExtData=0);
	virtual	int		CloseAll();
	virtual	int		Send(int iCntrID, const void* pData, size_t iSize);
	virtual	int		Recv(int* piCntrID, void** ppData, size_t* piSize);
	virtual int		GetSockets(whvector<SOCKET>& vect);
	virtual	int		GetCntrNum();
private:
	int		_Deal_RecvKA(epoll_connecter* pCntr, epoll_msg_ka_t* pMsgKA);
	int		_Deal_RecvClose(epoll_connecter* pCntr, epoll_msg_close_t* pMsgClose);
	int		_Deal_RecvExchangeKey(epoll_connecter* pCntr, epoll_msg_exchange_key_t* pMsgExKey, int iSize);
private:
	typedef	void	(epoll_server::* deal_cntr_func_T)(epoll_connecter* pCntr);
	int		_TickLogic_TE();
	int		_TickLogic_DealCntrInDList(int iIdx, deal_cntr_func_T pFunc);
	void	_DealCntrInDList_Err(epoll_connecter* pCntr);
public:
	void	TEDeal_ExchangeKey(TQUNIT_T* pTE);
	void	TEDeal_Drop(TQUNIT_T* pTE);
	void	TEDeal_Close(TQUNIT_T* pTE);
	void	TEDeal_SendKA(TQUNIT_T* pTE);

	void	RegExchangeKeyTQ(epoll_connecter* pCntr);
	void	RegDropTQ(epoll_connecter* pCntr);
	void	RegCloseTQ(epoll_connecter* pCntr);
	void	RegSendKATQ(epoll_connecter* pCntr);
public:
	enum
	{
		add_cntr_rst_ok							= 0,
		add_cntr_rst_full						= -1,
		add_cntr_rst_err_init					= -2,
		add_cntr_rst_err_send_key				= -3,
	};
	int		AddConnector(SOCKET sock, int* piCntrID, const sockaddr_in& addrPeer);
	int		RemoveConnector(int iCntrID);
	int		RemoveConnector(epoll_connecter* pCntr);
public:
	enum
	{
		close_on_err_unknown					= 0,
		close_on_err_baddata_size				= 1,
	};
	//关闭的流程，主动方A,被动方B
	//1.A发送close包给B,然后A设置状态为关闭状态
	//2.B收到A的close包,回应一个close包,然后设置状态为关闭状态
	//3.A收到close包,然后关闭
	//4.B超时,关闭
	int		Close(epoll_connecter* pCntr);
	int		CloseOnErr(epoll_connecter* pCntr, int iErrCode=close_on_err_unknown);
	int		SetStatusClose(epoll_connecter* pCntr, bool bClosePassive);
	void	SendClose(epoll_connecter* pCntr);
public:
	bool	IsCntrExist(int iCntrID);
	int		GetExt(int iCntrID, int* piExt);
	int		SetExt(int iCntrID, int iExt);
	void	SetCntr_ConnectExtData(int iCntrID, void* pData, size_t iSize);							// 设置connecter的连接时的附加数据
	void*	GetCntr_ConnectExtData(int iCntrID, size_t* piSize);
private:
	enum
	{
		server_self_error_listen_socket			= -1,			//listen socket出问题了
	};
	void	_DealServerSelfError();
private:
	whsmpqueue					m_queueCtrlOut;
	whunitallocatorFixed<epoll_connecter>		m_connecters;
	whDList<epoll_connecter*>					m_dlistCntrs[epoll_connecter::dlist_idx_max];
	whvector<epoll_connecter*>	m_vectCntrTmp;
	whtimequeue					m_tq;
	map<SOCKET, int>			m_sock2clientID;
	whtick_t					m_tickNow;
	info_T						m_info;
	whvector<char>				m_vectCtrlOutCmd;
	whvector<char>				m_vectRawBuf_Tmp;
	whvector<char>				m_vectRawBuf_Recv;
	whnet_epoll_event*			m_pEvents;
	SOCKET						m_listenSock;
	int							m_iMaxEventNum;
	int							m_iServerSelfError;
	int							m_epfd;
	whvector<unsigned char>		m_vectCodeDataBuf;
	whvector<unsigned char>		m_vectCodeKeyBuf;
	whvector<unsigned char>		m_vectCodeNKeyBuf;
	friend	class	epoll_connecter;
private:
	inline	int	_RegEpollEventAndSetSocketOpt(SOCKET sock)
	{
		whnet_epoll_event		ev;
		ev.data.fd				= sock;
		ev.events				= EPOLLIN;
		if (whnet_epoll_ctl(m_epfd, EPOLL_CTL_ADD, sock, &ev) < 0)
		{
			cmn_safeclosesocket(sock);
			return -1;
		}

		//为了保证关闭时服务器的socket资源被正确释放
		cmn_setsock_nonblock(sock, true);
		tcp_set_socket_nodelay(sock, 1);
		cmn_setsock_linger(sock, 0, 0);
		
		return 0;
	}
	inline	int	_ModEpollEvent(SOCKET sock, unsigned int uiEvent)
	{
		whnet_epoll_event	ev;
		ev.data.fd			= sock;
		ev.events			= uiEvent;
		whnet_epoll_ctl(m_epfd, EPOLL_CTL_MOD, sock, &ev);
		return 0;
	}
	inline	int	_DelEpollEvent(SOCKET sock)
	{
		whnet_epoll_event	ev;
		whnet_epoll_ctl(m_epfd, EPOLL_CTL_DEL, sock, &ev);
		return 0;
	}
	inline	int	_GetCntrIDBySocket(SOCKET sock)
	{
		map<SOCKET, int>::iterator	it	= m_sock2clientID.find(sock);
		if (it != m_sock2clientID.end())
		{
			return it->second;
		}
		return -1;
	}
	inline	epoll_connecter*	_GetCntrBySocket(SOCKET sock)
	{
		map<SOCKET, int>::iterator it	= m_sock2clientID.find(sock);
		if (it != m_sock2clientID.end())
		{
			return m_connecters.getptr(it->second);
		}
		return NULL;
	}
	inline	void	_EraseSocket2CntrIDMap(SOCKET sock)
	{
		map<SOCKET, int>::iterator	it	= m_sock2clientID.find(sock);
		if (it != m_sock2clientID.end())
		{
			m_sock2clientID.erase(it);
		}
	}
	inline	int		_CreateListenSocket()
	{
		sockaddr_in		addr;
		m_listenSock	= tcp_create_listen_socket(m_info.szIP, m_info.iPort, m_info.iPort1, &addr, m_info.iMaxConnections);
		return m_listenSock==INVALID_SOCKET?-1:0;
	}
	template<typename _Ty>
	inline	void*	_CtrlOutAlloc(_Ty*& ptr)
	{
		ptr		= (_Ty*)m_queueCtrlOut.InAlloc(sizeof(_Ty));
		return ptr;
	}
};

// 静态库指针共享
void*	WHNET_Get_EpollSharedData();
void	WHNET_Set_EpollSharedData(void* pData);
}

#endif
