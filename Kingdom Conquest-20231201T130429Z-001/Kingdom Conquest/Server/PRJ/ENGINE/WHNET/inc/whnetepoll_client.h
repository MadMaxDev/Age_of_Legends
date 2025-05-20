#ifndef __whnetepoll_client_H__
#define __whnetepoll_client_H__

#include "whnetcmn.h"
#include "whnetepoll_packet.h"
#include "whnettcp.h"

#include "WHCMN/inc/whqueue.h"
#include "WHCMN/inc/whtimequeue.h"
#include "WHCMN/inc/whstreambuffer.h"
#include "WHCMN/inc/whvector.h"
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whcrc.h"

using namespace n_whcmn;

namespace n_whnet
{
//////////////////////////////////////////////////////////////////////////
// epoll客户端使用的连接器
//////////////////////////////////////////////////////////////////////////
class epoll_connecter_client
{
public:
	epoll_connecter_client();
	virtual	~epoll_connecter_client();
// 外部主要使用的接口
public:
	struct info_T;
	int		Init(info_T* pInfo);
	int		Release();
	int		Close();
	int		Work();
	int		SendMsg(const void* pData, size_t iSize);
	int		RecvMsg(void* pData, size_t* piSize);
	const void*	PeekMsg(size_t* piSize);
	void	FreeMsg();
	int		GetSockets(whvector<SOCKET>& vect);
	inline info_T*	GetInfo()
	{
		return &m_info;
	}
	inline int	GetStatus()
	{
		return m_iStatus;
	}
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

		memcpy(m_szKeyD, szTmpBuf, WHMD5LEN);
		memcpy(m_szKeyE, szTmpBuf, WHMD5LEN);
	}
// 一般不需要由外部调用的接口
public:
	int		ManualRecv(int iItv=0);
	int		ManualSend(int iItv=0);
	epoll_msg_head_t*	GetAvailMsgHead();
public:
	struct info_T : whdataini::obj
	{
		int			iSendBufSize;
		int			iRecvBufSize;
		int			iCtrlOutQueueSize;
		char		szIP[WHNET_MAXIPSTRLEN];
		port_t		iPort;
		int			iTQChunkSize;
		int			iConnectTimeOut;
		int			iCloseTimeOut;
		int			nCodeDataBufSize;
		int			nCodeKeyBufSize;
		int			nCodeNKeyBufSize;
		bool		bPrintKA;

		info_T()
		: iSendBufSize(1*1024*1024)
		, iRecvBufSize(1*1024*1024)
		, iCtrlOutQueueSize(10*1024)
		, iPort(0)
		, iTQChunkSize(1024)
		, iConnectTimeOut(10*1000)
		, iCloseTimeOut(2*1000)
		, nCodeDataBufSize(2*1024*1024)
		, nCodeKeyBufSize(WHMD5LEN*2)
		, nCodeNKeyBufSize(WHMD5LEN*2)
		, bPrintKA(false)
		{
			szIP[0]	= 0;
		}

		WHDATAPROP_DECLARE_MAP(info_T);
	};
	enum
	{
		ctrlout_working		= 1,
		ctrlout_closed		= 2,
		ctrlout_error		= 3,
	};
	enum
	{
		status_nothing		= 0,
		status_connecting	= 1,
		status_working		= 2,
		status_error		= 3,
	};
	enum
	{
		err_reason_ok					= 0,	// 正常关闭
		err_reason_recv					= -1,
		err_reason_send					= -2,
		err_reason_select_rd			= -3,
		err_reason_select_wr			= -4,
		err_reason_recvdata_not_work	= -5,	// 非工作状态下不能接收data包
		err_reason_err_packet			= -6,
		err_reason_init_close			= -7,	// 主动关闭过程中出错,可能是另一方被动关闭超时,而close包未到这边
		err_reason_crc					= -8,
	};
	struct ctrl_out_T 
	{
		int	iCmd;
		int	iParam;
	};
protected:
	typedef	void	(epoll_connecter_client::* te_deal_func_T)();
	struct tq_unit_T 
	{
		te_deal_func_T	pFunc;
	};
	void	TickLogic_TE();
	void	RegKA();
	void	RegClose();
	void	RegDrop();
	void	TEDeal_KA();
	void	TEDeal_Close();
	void	TEDeal_Drop();
	void	SendClose();
	void	SendKA();
	int		SendRawMsg(const void* pData, size_t iSize);
public:
	int		ControlOut(ctrl_out_T** ppCmd, size_t* piSize);
	int		ControlOutReasonAndRelease(int iCtrlOutCmd, int iErrReason=err_reason_ok);
	void	SetExtData(void* pData, int iSize);
	void*	GetExtData(int* piSize);
public:
	bool				m_bNeedCode;
protected:
	int					m_iCntrIDPeer;
	int					m_iCryptType;
	unsigned char		m_szKeyD[WHMD5LEN];
	unsigned char		m_szKeyE[WHMD5LEN];
	size_t				m_iKeyLen;
	int					m_iKATimeout;
	int					m_iDropTimeout;
	whsmpqueue			m_queueCtrlOut;
	whtimequeue			m_TQ;
	bool				m_bClosePassive;
	bool				m_bClosing;
	whvector<char>		m_vectCtrlOutCmd;
	whvector<char>		m_vectCntrExtData;
	whstreambuffer		m_bufSend;
	whstreambuffer		m_bufRecv;
	whtick_t			m_tLastRecvTick;
	int					m_iStatus;
	info_T				m_info;
	SOCKET				m_sock;
	whvector<char>		m_vectRawBuf;

	whvector<unsigned char>		m_vectCodeDataBuf;
	whvector<unsigned char>		m_vectCodeKeyBuf;
	whvector<unsigned char>		m_vectCodeNKeyBuf;
};
}

#endif
