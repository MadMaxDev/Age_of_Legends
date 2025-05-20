// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File         : whconnecter.h
// Creator      : Wei Hua (κ��)
// Comment      : ͨ�õ�������
// CreationDate : 2003-05-21
// ChangeLOG    : 2005-09-02 ����TCPConnecterMsger<_TyMsger>::DoOneWork()�����ʱ��ܳ���ͳ��(Debug�����)
//              : 2005-10-24 ������SureSend���ƣ�ȷ��һ��tick�ڵ���������
//              : 2007-03-08 ��Connecter��SureSend���ָ��ˣ�����ڶ�̳е�ʱ������˵����ֳ�ͻ��

#ifndef	__WHCONNECTER_H__
#define	__WHCONNECTER_H__

#include "whnetcmn.h"
#ifdef	_DEBUG
#include "whnetudpGLogger.h"
#endif
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtime.h>

namespace n_whnet
{

class	Connecter;

class	ConnecterMan
{
public:
	struct	CONNECTOR_INFO_T
	{
		Connecter *	pCntr;
		int			nExt;									// һ���������ݣ����ϲ����ã���Ĭ��Ϊ0
		CONNECTOR_INFO_T()
			: pCntr(NULL)
			, nExt(0)
		{
		}
		void	clear()
		{
			WHMEMSET0THIS();
		}
	};
public:
	n_whcmn::whvector<SOCKET>	m_extrasock;				// ��selectʱ���ӵ�socket
protected:
	fd_set					m_readfds;						// ���ڶ�ѡ��
	fd_set					m_exceptfds;					// ���ڳ���ѡ��
	n_whcmn::whunitallocatorFixed<CONNECTOR_INFO_T>	m_Connecters;
															// ���Connecterָ��ķ�����
	n_whcmn::whvector<int>	m_tmpvectID;					// ������ʱ�����Ҫɾ����ID�б�
protected:
	bool	m_bIsReleasing;									// ������������Release��������Relase�е��õĺ���(�磺BeforeDeleteConnecter���ܾ�Ҫ����ͬ����)
	int		Release();
public:
	// ��û��Ѿ������Connecter������
	inline int		GetSize() const
	{
		return	m_Connecters.size();
	}
	// ��û����Լ����Connecter������
	inline int		GetAvailSize() const
	{
		return	m_Connecters.availsize();
	}
	inline Connecter *	GetConnecterPtr(int nID)
	{
		CONNECTOR_INFO_T	*pCInfo	= m_Connecters.getptr(nID);
		if( pCInfo )
		{
			return	pCInfo->pCntr;
		}
		return NULL;
	}
public:
	ConnecterMan();
	virtual	~ConnecterMan();
	int		Init(int nMaxNum);
	// ���һ�����ӣ��������ӵ����(ͬʱpCntr��Ҳ����������)
	int		AddConnecter(Connecter * pCntr);
	// ��������Ƴ�һ������(�Ƴ�ʱ��ɾ��Connecter���󣬶�Connecter����Ĭ�����Լ��ر�socket)
	// ��������BeforeDeleteConnecter
	int		RemoveConnecter(int nID);
	// ���ݶ���ָ���Ƴ�һ������
	int		RemoveConnecter(Connecter *pCntr);
	// �Ƴ����е����ӣ����ӿ϶�Ҳ��Ҫ�жϵ�����
	int		RemoveAllConnecter();
	// ��ms������������socket��selectrd�����Դ�m_readfds�л��select�Ľ��
	// ���غ�cmn_select_rdһ�£�>0��ʾ�ж�����0��ʾû������<0��ʾsocket����
	int		DoSelect(int ms);
	// ������е�socket��appent��vect����
	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	// ����Connecter���߼�����(Ӧ�ð���Read/Logic/Write)
	// ���bSureSendΪ���������Ҫֱ��Send
	int		DoWork(bool bSureSend);
	// ȷ��������Ԫ��Send�������tick�ڵ���
	int		SureSend();
	// ע��һ������DoSelect��Ȼ����DoWork����Ȼ����ⲿ��select�˾Ͳ����ˣ�
private:
	// ��һ�����Ӷϵ�ʱ��Ҫ������(������������������ص���Ϣ)
	// �������RemoveConnecter�е��õ�
	virtual void	BeforeDeleteConnecter(int nCntrID, CONNECTOR_INFO_T *pCntrInfo)
	{}
	// �����һ��Connecter��һЩ֪ͨ�ԵĶ���
	virtual void	AfterAddConnecter(Connecter * pCntr)
	{
	}
};

class	Connecter
{
	friend	class ConnecterMan;
protected:
	bool			m_bDeleteByMan;							// �Ƿ�Ӧ�ñ�������ɾ��(��Щ������Ҫ����������ɾ��������һ��Connecter�������Man)
	bool			m_bAutoCloseSocket;						// �Ƿ����������Զ�ɾ��socket
	bool			m_bSocketError;							// ��ʾ���������
	SOCKET			m_sock;
	ConnecterMan	*m_pMan;								// ��Ӧ��������ָ��
	int				m_nIDInMan;								// ��ConnecterMan�е����
public:
	Connecter()
	: m_bDeleteByMan(true), m_bAutoCloseSocket(true), m_bSocketError(false)
	, m_sock(INVALID_SOCKET), m_pMan(NULL), m_nIDInMan(-1)
	{}
	virtual ~Connecter();
	// ��Ҫ��Ϊ����������Ը����Լ��Ĳ���(���Ӧ�ò����ã����Ծ����ַ�������)
	// ���磺���������������TCPConnecterServer��this������Connecter
	virtual void *	QueryInterface(const char *cszIName=NULL)
	{
		return	NULL;
	}

	// �����ص�socket����Ҫ����select��
	inline SOCKET	GetSocket() const
	{
		return	m_sock;
	}
	inline void		SetSocket(SOCKET sock)
	{
		m_sock	= sock;
	}
	inline int	GetIDInMan() const
	{
		return	m_nIDInMan;
	}
	inline bool	GetSocketError() const
	{
		return	m_bSocketError;
	}
	inline void	SetSocketError()
	{
		m_bSocketError	= true;
	}
protected:
	virtual	int		Connecter_DoOneWork()	= 0;
	virtual	int		Connecter_SureSend()	= 0;
};

// ���ӽ�����
class	Connecter_TCPAccepter	: public Connecter
{
// Ϊ����ʵ�ֵ�
protected:
	virtual	int		Connecter_DoOneWork();
	virtual	int		Connecter_SureSend();
// ������Ϊ�Լ�ʵ��
public:
	Connecter_TCPAccepter()
	: m_pallowdeny(NULL)
	{
	}
public:
	cmn_addr_allowdeny	*m_pallowdeny;
private:
	// ������һ��������ʱ�Ĵ���
	// ����0�ɹ���-1ʧ��
	virtual int		DealNewSocket(SOCKET sock, struct sockaddr_in *paddr)
															= 0;
};

// �������TCP������
// �����̳еĶ���Ҫʵ��QueryInterface����Ϊ���������ڱ���Connecterʱ���ֳ��Լ�
class	TCPConnecterServer		: public ConnecterMan, public Connecter_TCPAccepter
{
public:
	struct	INFO_T
	{
		int		nMaxNum;		// ����������
		int		nPort;			// �����Ķ˿�
		int		nPort1;			// ���ӵĶ˿�(�����Ҫ��Χ�󶨵Ļ�)
		char	szIP[WHNET_MAXIPSTRLEN];
								// �󶨵����IP
		char	szAllowDeny[WH_MAX_PATH];
								// �����ֹ��ַ
		INFO_T()
		: nMaxNum(16), nPort(0), nPort1(0)
		{
			szIP[0]			= 0;
			szAllowDeny[0]	= 0;
		}
	};
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
	enum
	{
		ERRNOTIFYCMD_AVAILSIZEIS0		= 0,	// ���õ�������Ϊ0
	};
public:
	~TCPConnecterServer();
	int	Init(INFO_T *pInfo);
	int	Release();
	// ɾ����������
	int	DeleteAllConnection();
	// ��õ�һ�����õ�������ID
	int	GetFirstConnecterID();
public:
	// ��û��Ѿ���������ӵ�����
	inline int		GetSize() const
	{
		return	ConnecterMan::GetSize()-1;
	}
	inline struct sockaddr_in &	GetBindAddr()
	{
		return	m_bindaddr;
	};
private:
	cmn_addr_allowdeny	m_AllowDeny4Connect;
	struct sockaddr_in	m_bindaddr;				// �Լ��󶨵ĵ�ַ
private:
	// �����Ϊ��Connecter_TCPAccepterʵ�ֵ�
	virtual int			DealNewSocket(SOCKET sock, struct sockaddr_in *paddr);
	// ���ഴ�����ʵ�Connecter���������DealNewSocket�е��õģ�
	virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
																	= 0;
	// ���ͨ��
	virtual void		ErrNotify(int nNotifyCmd, void *pExtData)	{}
};

// �ɽ���С��64K��Ϣ�����Connecter
template<class _TyMsger>
class	TCPConnecterMsger	: public Connecter, public _TyMsger
{
public:
	struct	INFO_T
	{
		// ���ڳ�ʼ��tcpmsger
		typename _TyMsger::INFO_T	msgerINFO;
		// �Լ��õ���Ϣ
	};
public:
	int	Init(INFO_T *pInfo);
	inline SOCKET	GetSocket()
	{
		return	Connecter::GetSocket();
	}
protected:
	virtual	int		Connecter_DoOneWork();
	virtual	int		Connecter_SureSend();
private:
	// �Ƿ��пմ�����Ϣ
	virtual bool	CanDealMsg() const							= 0;
	// ������ʵ����Ϣ����
	virtual int		DealMsg(const void *pData, size_t nSize)	= 0;
};
template<class _TyMsger>
int		TCPConnecterMsger<_TyMsger>::Init(INFO_T *pInfo)
{
	// ��֤���÷���Ҫ��
	// ���ﲻ���ж���߳�
	pInfo->msgerINFO.bAutoStartThread	= false;
	// �����ҵ�sock
	m_sock	= pInfo->msgerINFO.sock;
	// ����msger�ĳ�ʼ��
	if( _TyMsger::Init(&pInfo->msgerINFO)<0 )
	{
		return	-1;
	}
	return	0;
}
template<class _TyMsger>
int		TCPConnecterMsger<_TyMsger>::Connecter_DoOneWork()
{
	// �ֹ���һ������
	_TyMsger::ManualRecv(0);
	// ����������������
	const void	*pData;
	size_t		nSize;
	// �ϲ���Կ���CanDealMsg�������Ƿ���Դ�������Ϣ(�������ⲿ����ָ���ͨ��������)
	// ������ʱ����ָ���һ����Ϊ�����Ѿ������ˣ�������Ҫע�⴦�����(�������뷽�ͷ��������ߺ�Ի�������ָ��Ĵ���)
	while( CanDealMsg() && (pData = _TyMsger::PeekMsg(&nSize))!=NULL )
	{
		// ����֮
		DealMsg(pData, nSize);
		// �����ݵ���
		_TyMsger::FreeMsg();
	}
	// �ж��Ƿ��д���
	switch( _TyMsger::GetStatus() )
	{
		case	_TyMsger::STATUS_ERROR:
			Connecter::SetSocketError();
		break;
	}
	return	0;
}
template<class _TyMsger>
int		TCPConnecterMsger<_TyMsger>::Connecter_SureSend()
{
	_TyMsger::ManualSend(0);
	return	0;
}

// �򵥵���Ϣ����
template<class _TyMsger>
class	TCPConnecterMsger_Echo	: public TCPConnecterMsger<_TyMsger>
{
private:
	typedef	TCPConnecterMsger<_TyMsger>	FATHERCLASS;
private:
	// ����ֻ�ǰ���Ϣԭ������(������)
	virtual int		DealMsg(const void *pData, size_t nSize)
	{
		return	FATHERCLASS::SendMsg(pData, nSize);
	}
	virtual bool	CanDealMsg() const
	{
		return	true;
	}
};

}		// EOF namespace n_whnet

#endif	// EOF __WHCONNECTER_H__
