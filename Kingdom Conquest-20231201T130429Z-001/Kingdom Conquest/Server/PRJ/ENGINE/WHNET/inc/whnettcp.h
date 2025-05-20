// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnettcp.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵�TCP���繦��
//                ��עtcp_set_socket_nodelay��������tcpΪ��������
// CreationDate : 2003-05-14
// ChangeLog    :
//                2004-02-04 MIN_TCPMSG_WHOLESIZEԭ�����˼�1�ˣ��ҿ����㷢���ˣ��Ǻǡ�1�ֽڵķ��ͻ����
//                2005-08-31 tcpmsger::INFO_T::bAutoStartThread��Ĭ��ֵ��Ϊfalse����Ĭ�ϲ����߳�
//                2007-06-08 ��tcpmsger�����ݳ��ȱ�ʾ��Ԫ��Ϊint��ͬʱ��Ĭ�ϵķ��ͺͽ��ջ��������ȸ�Ϊ512K��ȡ����tcpbigmsger��
//                2007-08-10 ��tcpmsger��bKeepAliveĬ�ϸ�Ϊfalse
//                2008-05-23 ȡ��tcpmsger���ֶ�KeepAlive����ֱ��ʹ��ϵͳ��tcp��KeepAlive�Ϳ�����

#ifndef	__WHNETTCP_H__
#define	__WHNETTCP_H__

#include "whnetcmn.h"
#include <WHCMN/inc/whcmn_def.h>
#include <WHCMN/inc/whstreambuffer.h>
#include <WHCMN/inc/whthread.h>
#include <WHCMN/inc/whstatusworker.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <assert.h>

using n_whcmn::whstatusholder;
using n_whcmn::whstatusworker;
using n_whcmn::whvector;

namespace n_whnet
{

enum
{
	TCP_SOMAXCONN		= WHNET_MAX_SOCKTOSELECT	,
};

// ֻ�Ǵ���һ���Ͷ˿ڰ󶨵�socket (bind to ADDRESSANY)
SOCKET	tcp_create_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr=NULL);
// (bind to fixed address)
SOCKET	tcp_create_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr=NULL);
// ���������˿ڵ����ӣ����ͬʱ�����������Ϊ__maxconn
SOCKET	tcp_create_listen_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr=NULL, int __maxconn = TCP_SOMAXCONN);
SOCKET	tcp_create_listen_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr=NULL, int __maxconn = TCP_SOMAXCONN);
SOCKET	tcp_create_listen_socket_by_ipnportstr(const char *__ipnport, struct sockaddr_in *__addr=NULL, int __maxconn = TCP_SOMAXCONN);
// (����ʽ)���ӷ����������ض˿�Ϊ__localport
// __timeoutΪ���ӳ�ʱ(��λ����)��Ĭ��10���ӳ�ʱ
SOCKET	tcp_create_connect_socket(struct sockaddr_in *__dstaddr, port_t __localport, int __timeout);
SOCKET	tcp_create_connect_socket(const char *__ip, port_t __port, port_t __localport, int __timeout);
SOCKET	tcp_create_connect_socket_by_ipnportstr(const char *__ipnport, port_t __localport, int __timeout);
// û�г�ʱ�жϵ�����(���������ӳɹ���ϵͳ��ʱ)
SOCKET	tcp_create_connect_socket_block(const char *__ip, port_t __port, port_t __localport);
// ����һ��noblock����
SOCKET	tcp_create_connecting_socket(const char *__ip, port_t __port, port_t __localport);
SOCKET	tcp_create_connecting_socket(const char *__ipportstr, port_t __localport);
SOCKET	tcp_create_connecting_socket(const struct sockaddr_in *__addr, port_t __localport);
// ���noblock�����Ƿ��Ѿ���������(�����ӳɹ���������Զ���socket���û�blockģʽ��������Ӧ�ü�ס�����ٴ�check�ˣ�����֪����ʲô�����������û�Թ�)
enum
{
	CONNECTING_SOCKET_CHKRST_ERR			= -1,		// ���ӳ���
	CONNECTING_SOCKET_CHKRST_OK				= 0,		// ��������
	CONNECTING_SOCKET_CHKRST_CONNTECTING	= 1,		// ���ӹ�����
};
int		tcp_check_connecting_socket(SOCKET __sock, int __interval = 0);
// ��ͼ����һ������
// __addrlen	(In/Out)��ʾ��ַ�������Ĵ����С������õĴ�С
// ����(��select_rdһ��)
//      >0				��ʾ����������
//      0				��ʾû�н�������
//      SOCKET_ERROR	��ʾsocket����
int		tcp_accept_socket(SOCKET __listensock, SOCKET *__insock, struct sockaddr *__inaddr, socklen_t *__addrlen, int __timeout);
int		tcp_accept_socket(SOCKET __listensock, SOCKET *__insock, struct sockaddr_in *__inaddr, int __timeout);
// ����socketΪTCP_NODELAY
int		tcp_set_socket_nodelay(SOCKET __sock, int __val);

// ����΢���recv��bug��__size����ʱ�ᱨ��������Ҫ�������һ��
#ifdef	WIN32
inline int	tcp_saferecv(SOCKET __sock, char *__data, int __size)
{
	// 2004-09-11 ��Ϊ1M��ԭ��10M���ܶ���Щ��
	const int	CNMAXRECV	= 1000000;
	if( __size>CNMAXRECV )
	{
		__size	= CNMAXRECV;
	}
	return	::recv(__sock, __data, __size, 0);
}
#endif
// Linux���ã�ֱ��recv����
#ifdef	__GNUC__
inline int	tcp_saferecv(SOCKET __sock, char *__data, int __size)
{
	return	::recv(__sock, __data, __size, 0);
}
#endif

// �ǳ����յ�tcp�ر�(����������Ӧ�ÿ���û��TIMEWAIT)
int	tcp_verysafeclose(SOCKET &__sock);

////////////////////////////////////////////////
// ����tcp����Ϣ���ͺͽ���
////////////////////////////////////////////////
typedef unsigned short		httag_t;			// head/tail tag
typedef unsigned long		msgsize_t;

#pragma pack(1)
struct	tcpmsg_head_t
{
	httag_t		nBegin;
	msgsize_t	nSize;
	char		Data[1];
	inline msgsize_t	GetSize(bool bNBO) const
	{
		return	bNBO ? ntohl(nSize) : nSize;
	}
};
struct	tcpmsg_tail_t
{
	httag_t		nEnd;
};
#pragma pack()
enum
{
	MAX_TCPMSG_SIZE			= 100000000,				// ������Ϣ�ߴ�
	MIN_TCPMSG_WHOLESIZE	= sizeof(tcpmsg_head_t)-1 + sizeof(tcpmsg_tail_t),
													// ��Ϣ������С�ߴ�
};
inline size_t	tcpmsg_getwholesize(size_t nSize)
{
	return	sizeof(tcpmsg_head_t) - 1 + nSize + sizeof(tcpmsg_tail_t);
}
inline size_t	tcpmsg_getwholesize(tcpmsg_head_t *pMsg, bool bNBO)
{
	return	tcpmsg_getwholesize(pMsg->GetSize(bNBO));
}
inline tcpmsg_tail_t *	tcpmsg_gettail(tcpmsg_head_t *pMsg, bool bNBO)
{
	return	(tcpmsg_tail_t *)(pMsg->Data + pMsg->GetSize(bNBO));
}

class	tcpmsger
{
public:
	enum
	{
		STATUS_ERROR			= -1,		// һ����SOCKET����
		STATUS_NOTHING			= 0,
		STATUS_WORKING			= 1
	};
	struct	INFO_T
	{
		SOCKET	sock;						// �������紴�����мǲ�Ҫ��������!!!!
		int		nInterval;					// ÿ�ּ����ʱ��(���ֻ����ʹ���߳�ʱ����Ҫ!)
		size_t	nTrySendSize;				// �����ͻ��峬�����ֵ��ʱ��ͳ��Է���һ������
		size_t	nSendBufSize;				// ���ͻ��壨�߼��ϵģ�
		size_t	nRecvBufSize;				// ���ջ��壨�߼��ϵģ�
		size_t	nSockSndBufSize;			// ���ͻ��壨TCP�ײ�ģ�
		size_t	nSockRcvBufSize;			// ���ջ��壨TCP�ײ�ģ�
		httag_t	nBegin, nEnd;				// ����ǰ���־����
		bool	bAutoStartThread;			// �Զ��򿪹����߳�
											// (Ĭ��false���ڶ��socket����������һ��Ϊfalse)
		bool	bLogSendRecv;				// ��¼������־
		bool	bNonBlockSocket;			// �Ƿ�����Ϊ������socket
		int		keepalive_idle;				// ����ײ��keepalive��ʼʱ��
		int		keepalive_interval;			// ����ײ��keepalive���
		INFO_T()
		: sock(INVALID_SOCKET), nInterval(50)
		, nTrySendSize(256*1024)
		, nSendBufSize(512*1024), nRecvBufSize(512*1024)
		, nSockSndBufSize(0), nSockRcvBufSize(0)	// д0�ʹ���ֱ��ʹ��ϵͳĬ�ϵĻ����С
		, nBegin(0x55AA), nEnd(0xAA55)
		, bAutoStartThread(false)
		, bLogSendRecv(false)
		, bNonBlockSocket(true)
		, keepalive_idle(30000), keepalive_interval(5000)
		{ }	
	};
	// ���Ϊ�˿���ͨ�������ļ��Զ����
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
protected:
	INFO_T	m_info;
	n_whcmn::whstreambuffer	m_bufSend;			// ����Ļ���
	n_whcmn::whstreambuffer	m_bufRecv;			// ����Ļ���
	n_whcmn::whlock			m_lkSend, m_lkRecv;	// ��������������е���
	n_whcmn::whtid_t		m_tid;				// �����̵߳�id
	int		m_nStatus;							// ��ǰ״̬
	n_whcmn::whtick_t		m_lastrecvtime;		// ���һ���յ����ݵ�ʱ��
	n_whcmn::whtick_t		m_lastsendtime;		// ���һ�η������ݵ�ʱ��
	bool	m_bStop;							// �Ƿ�ֹͣ�߳�
	bool	m_bUseLock;							// �Ƿ�ʹ��������
	bool	m_bNBO;								// �Ƿ���ת�������ֽ���(NetByteOrder)�������ʼֻ��tcpmsg_head_t�е�nSize��Ҫת��
	n_whcmn::whtimequeue	m_TQ;				// ʱ�����
	struct	TQUNIT_T
	{
		typedef	void (tcpmsger::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T			tefunc;
	};
protected:
	tcpmsg_head_t *	GetAvailMsgHead();
												// �������������Ϣ���Ի�ã������ͷ������û��������Ҫ���������
												// *pnReason�з���û�еõ���ԭ��
	int	CleanRecvData();						// �����������ݡ�������һ��ͷ��־��
												// ������յ��������ݵ�������ã�������������һ����ʼ��־
	int	_SendMsgFillData(tcpmsg_head_t *pHead, const void *pData, size_t nSize);
												// ��SendMsg��ʹ�ã���������д��pHead��Ļ�������ȥ
public:
	inline INFO_T *	GetInfo()
	{
		return	&m_info;
	}
	inline int	GetStatus() const
	{
		return	m_nStatus;
	}
	inline size_t	GetRecvedSize() const		// ����Ѿ��յ������ݳ���
	{
		return	m_bufRecv.GetSize();
	}
	inline n_whcmn::whtick_t	GetLastRecvTime() const
	{
		return	m_lastrecvtime;
	}
	inline void	SetLastRecvTime(n_whcmn::whtick_t t)
	{
		m_lastrecvtime	= t;;
	}
	inline void		SetNBO(bool bSet)
	{
		m_bNBO	= bSet;
	}
	// �����"_"��lock��unlock�ǿ�������ֶ����õģ�һ���PeekMsg��FreeMsg��
	inline void	_sendlock()
	{
		m_lkSend.lock();
	}
	inline void	sendlock()
	{
		if( m_bUseLock )
		{
			_sendlock();
		}
	}
	inline void	_sendunlock()
	{
		m_lkSend.unlock();
	}
	inline void	sendunlock()
	{
		if( m_bUseLock )
		{
			_sendunlock();
		}
	}
	inline void	_recvlock()
	{
		m_lkRecv.lock();
	}
	inline void	recvlock()
	{
		if( m_bUseLock )
		{
			_recvlock();
		}
	}
	inline void	_recvunlock()
	{
		m_lkRecv.unlock();
	}
	inline void	recvunlock()
	{
		if( m_bUseLock )
		{
			_recvunlock();
		}
	}
	inline n_whcmn::whstreambuffer &	getbufSend()
	{
		return	m_bufSend;
	}
	inline n_whcmn::whstreambuffer &	getbufRecv()
	{
		return	m_bufRecv;
	}
public:
	tcpmsger();
	virtual	~tcpmsger();
	int	Init(INFO_T *pInfo);					// ��ʼ����
	int	Release();								// �սᲢ���������߳�(���һ�ر�ͨѶ�õ�socket)
	int	StartWorkThread();						// ��ʼ�����߳�
	int	StopWorkThread();						// ���������߳�
	int	SendMsg(const void *pData, size_t nSize);
												// ����0��ʾ�ɹ���-1��ʾ����
	int	RecvMsg(void *pData, size_t *pnSize);	// ����1��ʾ�����ݣ�0��ʾû���ݣ�-1��ʾ����
	// !!!! ע�⣺���m_bNBOΪ�棬��PeekMsg��ȡ�������ڲ����ܻ���û��ת��Ϊ�����ֽ�������ݣ����� !!!!
	const void *	PeekMsg(size_t *pnSize);	// ��ȡ����ָ�룬���ǲ��ͷ�
												// ע��Peek��Free�ڲ���û�м��������������ڶ��߳�ʱ��Ҫ�ⲿ����
	void	FreeMsg();							// �ͷ����µ�һ����Ϣ
	size_t	GetNextSizeToRecv();				// ��ÿ�����ȡ����һ����Ϣ�Ĵ�С
												// ����������ʾ������
												// ����0��ʾû��
	int	WorkThread();							// �̵߳��õ�
	// �����ʹ���߳�����Ҫ�ֶ���������ĺ���
	// ����
	// ManualRecv
	// RecvMsgֱ������
	// ...
	// SendMsg
	// ManualSend
	int	ManualRecv(int nInterval = 0);
	int	ManualSend(int nInterval = 0);
	// �����������Ժϲ�Ϊ
	int	ManualRecvAndSend(int nInterval = 0);

	// ����״̬
	void	SetStatus(int nStatus);
};

// ����tcp����Ϣ����/����
// ֻ�ܵ��߳�ʹ��!!!!
class	tcpmsgerbase
{
public:
	// ״̬
	enum
	{
		STATUS_ERROR		= -1,				// һ����SOCKET����
		STATUS_NOTHING		= 0,
		STATUS_WORKING		= 1
	};
	struct	INFO_T
	{
		SOCKET	sock;							// �������紴�����мǲ�Ҫ��������!!!!
		int		nInterval;						// ÿ�ּ����ʱ��(���ֻ����ʹ���߳�ʱ����Ҫ!)
		int		nSendBufSize;					// ���ͻ���
		int		nRecvBufSize;					// ���ջ���
		bool	bAutoCloseSocket;				// ��Releaseʱ�Զ��ر�socket
		bool	bAutoStartThread;				// �Զ��򿪹����߳�(�����Ϊ�˼����Կ��ǵ�)
		INFO_T()
		: sock(INVALID_SOCKET), nInterval(50)
		, nSendBufSize(65536), nRecvBufSize(65536)
		, bAutoCloseSocket(true)
		, bAutoStartThread(false)
		{ }
	};
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
protected:
	int						m_nStatus;
	INFO_T					m_info;
	n_whcmn::whstreambuffer	m_bufSend;			// ����Ļ���
	n_whcmn::whstreambuffer	m_bufRecv;			// ����Ļ���
	n_whcmn::whtick_t		m_lastrecvtime;		// ���һ���յ����ݵ�ʱ��
	whvector<int>			m_queueLen;			// ÿ��Unit�ĳ��ȣ��ܷŵ������˵���������㹻��ȷ�ģ�
	int						m_nCheckedLen;		// �Ѿ�����OK�ĳ���
public:
	inline const INFO_T *	GetInfo() const
	{
		return	&m_info;
	}
	inline int	GetStatus() const
	{
		return	m_nStatus;
	}
	inline int	GetRecvedSize() const			// ����Ѿ��յ������ݳ���
	{
		return	m_bufRecv.GetSize();
	}
	inline n_whcmn::whtick_t	GetLastRecvTime() const
	{
		return	m_lastrecvtime;
	}
	inline void	SetLastRecvTime(n_whcmn::whtick_t t)
	{
		m_lastrecvtime	= t;;
	}
public:
	tcpmsgerbase();
	virtual	~tcpmsgerbase();
	// ע�⣺�����ֶ�����Release����Ϊ������������������Ϊ�������Ҫָ�����Զ�����Release
	int	Init(INFO_T *pInfo);					// ��ʼ��
	int	Release();								// �ս�(���һ�ر�ͨѶ�õ�socket)
	int	SendMsg(const void *pData, size_t nSize);
												// ����0��ʾ�ɹ���-1��ʾ����
	int	RecvMsg(void *pData, size_t *pnSize);	// ����1��ʾ�����ݣ�0��ʾû���ݣ�-1��ʾ����
	const void *	PeekMsg(size_t *pnSize);	// ��ȡ����ָ�룬���ǲ��ͷ�
												// ע��Peek��Free�ڲ���û�м��������������ڶ��߳�ʱ��Ҫ�ⲿ����
	void	FreeMsg();							// �ͷ����µ�һ����Ϣ
	int	ManualRecv(int nInterval = 0);
	int	ManualSend(int nInterval = 0);
	// �����������Ժϲ�Ϊ
	int	ManualRecvAndSend(int nInterval = 0);
	// ����״̬
	void	SetStatus(int nStatus);
private:
	int	ManualRecv_AnalyzeRecvData();
protected:
	enum
	{
		CHECKUNIT_RST_ERR	= -1,				// �������ݴ���
		CHECKUNIT_RST_NA	= 0,				// û�з��ֵ�Ԫ
		CHECKUNIT_RST_OK	= 1,				// �Ѿ��ҵ���һ����Ԫ
	};
private:
	// ����pBegin��ʼ�������Ƿ������������ݵ�Ԫ
	// ע�⣬һ��Ҫ�ж����ݳ����Ƿ������С��Ԫ��׼�����翪ͷһ���ֽ�Ϊ���ȣ��Ǿ�Ҫ�ж�nSize�Ƿ�С��1��Ȼ�����ȡ��һ���ֽڣ��жϳ��ȣ�
	// pBegin		�����ݿ�ʼ
	// nSize		�������ܳ�
	// *pnUnitLen	����ҵ�һ����Ԫ���������ǵ�Ԫ���ܳ���(��pBegin��ʼ)
	//				����������ݴ�������������û��Ŀ�ʼ��λ��(Ҳ���ǳ������ݵ��ܳ���)
	// ����ֵ��		CHECKUNIT_RST_XXX
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen)
												= 0;
	// ��ô������³���
	virtual size_t	GetPackUnitSize(size_t nDSize)
												= 0;
	// ��һ�����ݴ�����ʺ�ͨѶ�ĸ�ʽ(pDstBufӦ��ʵ���Ѿ�������ˣ��ߴ�������GetPackUnitSize��õĳ���)
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf)
												= 0;
	// �Ӵ���������ҵ�����ָ�룬���������ݳ��ȣ�עnTotalSize�����unit�������ܳ��ȣ����಻�ٵģ�
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize)
												= 0;
};
// ��"\n"Ϊ�н�β�������������������һ���ֽ�
class	tcpmsger_cmnline		:	public tcpmsgerbase
{
private:
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen);
	virtual size_t	GetPackUnitSize(size_t nDSize);
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf);
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize);
};
// ��˫�ֽڿ�ͷ������ܳ���(Ҳ����˵�����Ȳ��ܳ���65536)
class	tcpmsger_smallpacket	:	public tcpmsgerbase
{
public:
	// ����ǰ��ͷ��ʾ���������ȵ�
	typedef	unsigned short		mysize_t;
private:
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen);
	virtual size_t	GetPackUnitSize(size_t nDSize);
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf);
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize);
};
// �ñ䳤�ĳ���vn2��ʾ����
class	tcpmsger_vn2_packet	:	public tcpmsgerbase
{
private:
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen);
	virtual size_t	GetPackUnitSize(size_t nDSize);
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf);
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize);
};
// ���ͽ��ն��ǹ̶����ȵģ�kr��Ȼϲ����������ͨѶ��ʽ:(��
class	tcpmsger_fixedsize	:	public tcpmsgerbase
{
private:
	int		m_nPacketSize;
public:
	tcpmsger_fixedsize()
		: m_nPacketSize(0)
	{
	}
	inline	void	SetPacketSize(int nPacketSize)
	{
		m_nPacketSize	= nPacketSize;
	}
private:
	virtual int	CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen);
	virtual size_t	GetPackUnitSize(size_t nDSize);
	virtual int	PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf);
	virtual const void *	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize);
};

// ����������
// ע�⣺�������sock������select_rd���ж��Ƿ���ã�����Ҫ��select_wr���жϣ����Բ����ǳ�ȥһ����select_rd!!!!
class	tcpretryer
{
public:
	enum
	{
		MAX_DSTADDRNUM					= 16,	// �����Գ������ӵĵ�ַ����
	};
	enum
	{
		STATUS_NOTHING					= 0,	// ʲôҲ���������ܸոտ�ʼ���������߾������ٹ�����
		STATUS_TRYCONNECT				= 1,	// ��������Ŀ��
		STATUS_WORK						= 2,	// �����Ĺ���
		STATUS_SLEEP					= 3,	// ��Ϣ
		STATUS_NEVERTRY					= 10,	// ��Զ����try��
	};
	struct	DSTADDR_T
	{
		int		nCanTryCount;					// ���Ա����ԵĴ���
		char	szAddr[WHNET_MAXADDRSTRLEN];	// ��ַ�ַ���
	};
protected:
	int		m_nStatus;							// ״̬
	SOCKET	m_sock;								// ������ӳɹ���socket������
	int		m_nTotalRetry;						// �����ܹ����ԵĴ���(<=0��ʾ���޴�����)
	int		m_nRetryCount;						// Ŀǰ�Ѿ����ԵĴ���
	int		m_nRetryTimeOut;					// �����ĳ�ʱ(��λ�����룬Ĭ��ֵ��tcpretryer::tcpretryer()�еĸ�ֵ)
	n_whcmn::whtick_t	m_nRetryStartTime;		// ������ʼ��ʱ��ʱ��
	int		m_nSleepTime;						// ��Ϣ��ʱ��(��λ�����룬Ĭ��ֵ��tcpretryer::tcpretryer()�еĸ�ֵ)
	n_whcmn::whtick_t	m_nSleepStartTime;		// ��Ϣ��ʼ��ʱ��
	// ���õ�ַ���
	DSTADDR_T	m_DstAddrs[MAX_DSTADDRNUM];		// Ŀ���ַ��Ԫ����
	DSTADDR_T	*m_pCurDstAddr;					// ��ǰ�ĵ�ַ��Ԫ
	int		m_nDstAddrNum;						// Ŀǰ�ı��õ�ַ����
	int		m_nCurDstAdrrIdx;					// ��ǰ��ַʹ�õĵ�ַ����
	int		m_nCurTryCount;						// ��ǰ��ַ�����ԵĴ���
public:
	inline bool		Failed() const				// ������Ӷ�ʧ���ˣ����Ҳ�����������
	{
		return	(m_nStatus == STATUS_NOTHING) && (!ShouldRetry());
	}
	inline SOCKET	PickSocketOut()				// ������Ӻõ�socket��������Ҫ��STATUS_WORK��״̬�µ���
	{
		SOCKET	sock = m_sock;
		m_sock	= INVALID_SOCKET;
		return	sock;
	}
	inline int		GetStatus() const			// ��õ�ǰ״̬
	{
		return	m_nStatus;
	}
	// ��õ�ǰ��Ҫ���ӵĵ�ַ
	inline const char *	GetCurDstAddr() const
	{
		assert(m_pCurDstAddr);
		return	m_pCurDstAddr->szAddr;
	}
	inline void		SetTotalRetry(int nTotal)
	{
		m_nTotalRetry	= nTotal;
	}
	inline void		SetRetryTimeOut(int nTimeOut)
	{
		m_nRetryTimeOut	= nTimeOut;				// ��λ������
	}
	inline void		SetSleepTime(int nSleepTime)
	{
		m_nSleepTime	= nSleepTime;			// ��λ������
	}
	inline int		GetRetryCount() const
	{
		return	m_nRetryCount;
	}
public:
	tcpretryer();
	~tcpretryer();
	void	ClrAllDstAddr();					// ����Ѿ����õ�����Ŀ���ַ
	int		AddDstAddr(const char *szDstAddr, int nCanTryCount);
												// ���Ŀ���ַ��nCanTryCount��ʾ�����ַ�����������ԵĴ���(������ж��Ŀ���ַ��ʱ�������)��
	void	StartWork(bool bSleepFirst=false);	// ��ʼ����(��֮��Ϳ�ʼ������ֱ�������ϻ��߳�������Ϊֹ)
												// ���bSleepFirstΪ�����ȿ�ʼ˯
	int		DoSelect(int nInterval);			// �鿴socket�Ƿ�������
	void	Work();								// ��һ�ι���(�������ͨ��tcp_check_connecting_socket���ж��Ƿ����ӳɹ�)
private:
	bool	ShouldRetry() const;				// �ж��Ƿ�Ӧ������
	void	BeginSleep();						// ��ʼ�ȴ�һ��ʱ��
	void	DoSleep();							// �ȴ�������
	void	BeginRetry();						// ��ʼ��������
	void	DoRetry();							// ���Ե�����
	void	SwitchNextDst();					// ת��Ϊ��һ�����Ե�ַ
};

// ����retry���ܵ�msger
// ע�⣬���ֻ�ܺͿ�����һ���߳��У�������ʹ��bAutoStartThread
// ���ԣ�Ҳ����Ҫʹ��select���ܣ���Ҫ�����Ƶ���tick��Ƶ��
// ���õķ�������_TyMsger��tcpretryer�ķ���Ҳ���������
// �磺SetRetryTimeOut��SetSleepTime��SetTotalRetry�ȣ�������ס�����Ǳ�����Init֮ǰ���ã���ΪInit�о��Ѿ������Ӷ�����
// ���У�SendMsg��RecvMsg��PeekMsg��FreeMsg�ȵȶ�������ͬһ�߳�ʹ��
template<class _TyMsger>
class	tcpretrymsger	: public _TyMsger, public tcpretryer, whstatusholder
{
public:
	enum
	{
		STATUS_NOTHING		= 0,
		STATUS_TRYCONNECT	= 1,
		STATUS_WORKING		= 2,
	};
public:
	bool				m_bSleepFirst;
	// ��Ϊpublic��ԭ����Ϊ�˿������Ҳֱ��ʹ��SetStatus
	whstatusworker		m_statusTRYCONNECT;
	whstatusworker		m_statusWORKING;
private:
	typename _TyMsger::INFO_T	m_msgerinfo;			// �洢��ʼ��ʱ��
	// Ϊ������
	WHSH_DECLARETHISCLASS(tcpretrymsger<_TyMsger>);
private:
	void	Worker_TRYCONNECT_Begin()
	{
		// �رվ�����
		_TyMsger::Release();
		// ��ʼ����
		tcpretryer::StartWork(m_bSleepFirst);
		// ���ӹ���
		i_Worker_TRYCONNECT_Begin();
	}
	void	Worker_TRYCONNECT_Work()
	{
		tcpretryer::Work();
		if( tcpretryer::GetStatus()==tcpretryer::STATUS_WORK )
		{
			m_msgerinfo.sock				= tcpretryer::PickSocketOut();
			m_msgerinfo.bAutoStartThread	= false;
			if( _TyMsger::Init(&m_msgerinfo)<0 )
			{
				// ���û�취��
				assert(0);
			}
			else
			{
				whstatusholder::SetStatus(m_statusWORKING);
			}
		}
		// ���ӹ���
		i_Worker_TRYCONNECT_Work();
	}
	void	Worker_WORKING_Begin()
	{
		i_Worker_WORKING_Begin();
	}
	void	Worker_WORKING_Work()
	{
		//
		_TyMsger::ManualRecv();
		// �ж��Ƿ������
		if( _TyMsger::GetStatus() != _TyMsger::STATUS_WORKING )
		{
			whstatusholder::SetStatus(m_statusTRYCONNECT);
			return;
		}
		// ����б�Ҫ����i_Worker_WORKING_Work���Զ������ManualSend
		i_Worker_WORKING_Work();
		//
		_TyMsger::ManualSend();
	}
public:
	tcpretrymsger()
	: m_bSleepFirst(false)
	, m_statusTRYCONNECT(STATUS_TRYCONNECT)
	, m_statusWORKING(STATUS_WORKING)
	{
		m_statusTRYCONNECT.SetBeginFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_TRYCONNECT_Begin));
		m_statusTRYCONNECT.SetWorkFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_TRYCONNECT_Work));
		m_statusWORKING.SetBeginFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_WORKING_Begin));
		m_statusWORKING.SetWorkFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_WORKING_Work));
	}
	// ���ֻ��Ҫһ��Ŀ���ַ
	int		Init(typename _TyMsger::INFO_T *pInfo, const char *pcszDstAddrs)
	{
		// ���������һ����������
		memcpy(&m_msgerinfo, pInfo, sizeof(m_msgerinfo));
		tcpretryer::ClrAllDstAddr();
		tcpretryer::AddDstAddr(pcszDstAddrs, 0);
		// ��ʼ����
		whstatusholder::SetStatus(m_statusTRYCONNECT);
		return	0;
	}

	// Tick�����״̬ת��(����/���ݽ���)
	int		Tick()
	{
		whstatusholder::DoWork();
		return	0;
	}
	// ״̬�������涨���STATUS_XXX
	inline int	GetStatus() const
	{
		return	whstatusholder::GetStatus();
	}
	inline void	SetStatus(whstatusworker &worker)
	{
		whstatusholder::SetStatus(worker);
	}
	// ��ȡ��ǰ����ʹ�õ�socket��������������򷵻�INVALID_SOCKET
	SOCKET	GetSocket()
	{
		if( _TyMsger::GetStatus() == _TyMsger::STATUS_WORKING )
		{
			return	_TyMsger::GetInfo()->sock;
		}
		return		INVALID_SOCKET;
	}
	// Ӧ�õ����������Щ(�������ͨ����Щ�����¹���)
	virtual void	i_Worker_TRYCONNECT_Begin()	{}
	virtual void	i_Worker_TRYCONNECT_Work()	{}
	virtual void	i_Worker_WORKING_Begin()	{}
	virtual void	i_Worker_WORKING_Work()		{}
};

// �򵥵�tcpproxy��������IP��������ݽ�����������������
// ʹ������
//	tcpproxy				tp;
//	tcpproxy::INFO_T		tpinfo;
//	tpinfo.cszDstAddr		= pszDstAddr;
//	tpinfo.cszListenAddr	= szListenAddr;
//	tp.Init(&tpinfo);
//	while(!g_bStop)
//	{
//		tp.DoSelect(10);
//		tp.DoTick();
//		... ...
//		��ʱ��������Խ���tp.SetByteRate��tp.SetDst�Ĳ���
//	}
//	tp.Release();
class	tcpproxy
{
public:
	struct	INFO_T
	{
		const char	*cszListenAddr;				// ������ַ
		const char	*cszDstAddr;				// Ŀ���ַ
		int			nMaxConnection;				// �����Գ��ܵ�������
		int			nMaxBufLen;					// ÿ�ζ�ȡ����������ݳ���
		int			nConnectTimeout;			// ����Ŀ���ַ�ĳ�ʱ
		int			nBRateUp, nBRateDown;		// �����еĴ�����(�ֽ�/��)
												// 0��ʾ������
		int			nDelayQueueSize;

		INFO_T()
		: cszListenAddr(NULL)
		, cszDstAddr(NULL)
		, nMaxConnection(20)					// ���ͬʱ�������������Ƚϴ�(���������)���������Ҫ���ø�һЩ
		, nMaxBufLen(4096)
		, nConnectTimeout(5000)
		, nBRateUp(0), nBRateDown(0)
		, nDelayQueueSize(4*1024*1024)			// Ĭ�ϸ�4M�İ�
		{
		}
	};
	struct	UNIT_T
	{
		SOCKET		sockFrom;					// ���ӷ���(����)
		SOCKET		sockTo;						// ����Ŀ��(����)
		// �ӳٷ��͵ļ���
		int			nUpCount;					// ���Է��𷽵�
		int			nDownCount;					// ����Ŀ�귽��
		// ���漸��������������
		n_whcmn::whtick_t	nFromReadNextTime;	// �´ο��Դ�From�������ݵ�ʱ��
		n_whcmn::whtick_t	nToReadNextTime;	// �´ο��Դ�To�������ݵ�ʱ��
		void	clear()
		{
			memset(this, 0, sizeof(*this));
			sockFrom	= INVALID_SOCKET;
			sockTo		= INVALID_SOCKET;
		}
		inline bool	IsTimeToReadFrom(n_whcmn::whtick_t nNow)
		{
			return	n_whcmn::wh_tickcount_diff(nNow, nFromReadNextTime) >= 0;
		}
		inline bool	IsTimeToReadTo(n_whcmn::whtick_t nNow)
		{
			return	n_whcmn::wh_tickcount_diff(nNow, nToReadNextTime) >= 0;
		}
	};
	struct	DELAY_T
	{
		int						nUnitID;		// ��Ӧ��ת��UNIT��ID
		n_whcmn::whtick_t		nSendTime;		// ��Ҫ���͵�ʱ��
	};	// ����ṹ�ĺ���͸������ݲ���
protected:
	INFO_T			m_info;
	struct sockaddr_in	m_dstaddr;				// Ŀ���ַ
	n_whcmn::WHRANGE_T<int>		m_rgDelayUp;	// �ӳٷ�Χ(����TCP��˵��ʵӦ�����������ڰ��ļ��)
	n_whcmn::WHRANGE_T<int>		m_rgDelayDown;	// 
	cmn_selector	m_selector;
	SOCKET			m_sockListen;				// ���ڼ�����socket
	n_whcmn::whunitallocatorFixed<UNIT_T>		m_Units;
	whvector<int>								m_vectUnitIDToDel;
	whvector<char>								m_vectBuf;
	n_whcmn::whsmpqueue							m_queueDelayUp;		// ���������ӳ٣�����Դ����Ŀ�꣩���͵Ķ���
	n_whcmn::whsmpqueue							m_queueDelayDown;	// ���������ӳ٣���Ŀ�귵����Դ�����͵Ķ���
	n_whcmn::whtick_t			m_tickNow;
	struct	STAT_T
	{
		int			nMaxConcurrentConnection;	// ͬʱ���������������
		STAT_T()
		{
			WHMEMSET0THIS();
		}
	}				m_stat;
public:
	tcpproxy();
	~tcpproxy();
	int		Init(INFO_T *pInfo, struct sockaddr_in *pListenAddr=NULL);
	int		Release();
	void	SetDst(const char *cszDstAddr);		// ����Ŀ���ַ�������������иı�Ŀ���ַ��
	void	SetDelay(int nUpMin, int nUpMax, int nDownMin, int nDownMax);
												// �����ӳٵ�ʱ�䷶Χ�����룩���������0���ʾ���ӳ١�
	void	SetByteRate(int nUp, int nDown);	// �����ֽ�/�룬�����0���ʾ�����ơ������������ʶ������ã�
	int		DoSelect(int nInterval);			// select���е�socket
	int		DoTick();							// ����һ�η��ͽ��ն������Լ��ڲ��߼���
	const char *	ShowInfo(char *pszBuf);		// ����Ϣ��ӡ��Buffer��
private:
	int		CheckRecvThenSend(int nUnitID, SOCKET sockIn, SOCKET sockOut, int nBRate, n_whcmn::whtick_t &nNowAndThen, n_whcmn::WHRANGE_T<int> &WR, n_whcmn::whsmpqueue &queueDelay, int &nCount);
};

// ����ķ�װ���̵߳Ķ���(With Thread)
// ʹ������
//	tcpproxyWT				tp;
//	tcpproxyWT::INFO_T		tpinfo;
//	tpinfo.cszDstAddr		= pszDstAddr;
//	tpinfo.cszListenAddr	= szListenAddr;
//	tp.Init(&tpinfo);
//	tp.StartThread();
//
//	��ʱ��������Խ���tp.SetByteRate��tp.SetDst�Ĳ���
//
//	tp.StopThread();
//	tp.Release();

class	tcpproxyWT	: public tcpproxy
{
public:
	n_whcmn::whtid_t	m_tid;
	bool				m_bStop;
public:
	tcpproxyWT();
	~tcpproxyWT();
	// nPriority�Ķ���μ���whthread.h�е�THREAD_PRIORIY_XXX��-1��ʾĬ�����ȼ�
	int		StartThread(int nPriority=-1);
	int		StopThread();
};

// ��ͨ��һЩ�Ĵ���Ľṹ
class	whtcpproxy
{
protected:
	SOCKET				m_sock[2];
	whvector<char>		m_vect[2];
public:
	whtcpproxy();
	virtual ~whtcpproxy();
	// ����ĳ��socket���������һ����û�����ã�����һ����INVALID_SOCKET����������յ��Ķ����������ۻ�����
	void	SetSocket(int nIdx, SOCKET sock);
	// ��õ�ǰ���õ�socket����vect��
	void	GetSockets(whvector<SOCKET> &vect);
	int		DoTick();							// ����һ�η��ͽ��ն������Լ��ڲ��߼���
private:
	void	FailSocket(int nIdx);
	virtual void	OnSocketFail(int nIdx)	{}
};
class	whtcpproxy_Director	: public whtcpproxy
{
public:
	class	Director
	{
	public:
		virtual	~Director()					{}
		enum
		{
			WORKRST_OK			= 0,
			WORKRST_WAIT		= 1,
			WORKRST_ERR			= -1,
		};
		virtual	int	WorkTick()				= 0;
		virtual	SOCKET	GetSocket()			= 0;
	};
	Director	*m_aD[2];
public:
	whtcpproxy_Director();
	~whtcpproxy_Director();
	int		SetListenDirection(int nIdx, const char *cszAddr);
	int		SetConnectDirection(int nIdx, const char *cszAddr);
	int		DoTick();
};

template<class _Ty>
int	SafeSendMsg(_Ty *pMsger, const void *pData, size_t nSize)
{
	if( pMsger )
	{
		return	pMsger->SendMsg(pData, nSize);
	}
	return	-1;
}


}		// EOF namespace n_whnet

#endif	// EOF __WHNETTCP_H__
