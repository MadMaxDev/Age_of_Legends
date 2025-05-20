// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File: whconnecter.cpp
// Creator: Wei Hua (魏华)
// CreationDate: 2003-05-22
//             : transformed from pnl_connecter on 2003-12-16
// ChangeLOG   :
// 2004-02-20   : 添加了TCPConnecterServer绑定到特定IP的能力

#include "../inc/whconnecter.h"
#include "../inc/whnettcp.h"
#include <assert.h>

using namespace n_whnet;
using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// ConnecterMan
////////////////////////////////////////////////////////////////////
ConnecterMan::ConnecterMan()
{
}
ConnecterMan::~ConnecterMan()
{
	Release();
}
int		ConnecterMan::Init(int nMaxNum)
{
	if( nMaxNum>WHNET_MAX_SOCKTOSELECT )
	{
		// 太多了
		assert(0);
		return	-1;
	}

	m_bIsReleasing	= false;
	return		m_Connecters.Init(nMaxNum);
}
int		ConnecterMan::Release()
{
	m_bIsReleasing	= true;
	RemoveAllConnecter();
	return		0;
}
int		ConnecterMan::AddConnecter(Connecter * pCntr)
{
	if( !pCntr )
	{
		// 不能加入空指针
		return	-1;
	}

	CONNECTOR_INFO_T *pCntrInfo;
	int	nID;
	nID	= m_Connecters.AllocUnit(pCntrInfo);
	if( nID<0 )
	{
		// 无法加入，分配已满
		return	-1;
	}

	pCntrInfo->clear();

	pCntrInfo->pCntr	= pCntr;
	pCntr->m_nIDInMan	= nID;
	pCntr->m_pMan		= this;

	AfterAddConnecter(pCntr);

	return	nID;
}
int		ConnecterMan::RemoveConnecter(int nID)
{
	CONNECTOR_INFO_T *pCntrInfo = m_Connecters.getptr(nID);
	if( !pCntrInfo )
	{
		// 已经被删除了
		return	-1;
	}
	assert(pCntrInfo->pCntr);

	BeforeDeleteConnecter(nID, pCntrInfo);

	// 为了保险再close一下socket (2005-08-24加)
	tcp_verysafeclose(pCntrInfo->pCntr->m_sock);

	if( pCntrInfo->pCntr->m_bDeleteByMan )
	{
		delete	pCntrInfo->pCntr;
	}
	m_Connecters.FreeUnit(nID);

	return		0;
}
int		ConnecterMan::RemoveConnecter(Connecter *pCntr)
{
	return	RemoveConnecter(pCntr->m_nIDInMan);
}
int		ConnecterMan::RemoveAllConnecter()
{
	// 删除所有的Connecter
	CONNECTOR_INFO_T	*pCntrInfo;
	m_Connecters.BeginGet();
	while( (pCntrInfo=m_Connecters.GetNext())!=NULL )
	{
		if( pCntrInfo->pCntr->m_bDeleteByMan )
		{
			BeforeDeleteConnecter(pCntrInfo->pCntr->m_nIDInMan, pCntrInfo);
			// 为了保险再close一下socket (2005-08-24加)
			tcp_verysafeclose(pCntrInfo->pCntr->m_sock);
			delete	pCntrInfo->pCntr;
		}
	}
	m_Connecters.clear();
	return		0;
}
int		ConnecterMan::DoSelect(int ms)
{
	// 填写各个socket
	SOCKET	nBigestSock = 0;
	bool	bHaveSock	= false;

	FD_ZERO(&m_readfds);

	CONNECTOR_INFO_T	*pCntrInfo;
	m_Connecters.BeginGet();
	while( (pCntrInfo=m_Connecters.GetNext())!=NULL )
	{
		assert( pCntrInfo->pCntr );
		SOCKET sock	= pCntrInfo->pCntr->GetSocket();
		if( sock != INVALID_SOCKET )
		{
			FD_SET(sock, &m_readfds);
			if( nBigestSock<sock )
			{
				nBigestSock	= sock;
			}
			bHaveSock	= true;
		}
		else
		{
			// 有可能该Connecter断连，正在重连中。
		}
	}
	// 这个东西是给子类附加新的socket用的
	for(size_t i=0;i<m_extrasock.size();i++)
	{
		SOCKET	sock = m_extrasock[i];
		if( sock!=INVALID_SOCKET )
		{
			FD_SET(sock, &m_readfds);
			if( nBigestSock<sock )
			{
				nBigestSock	= sock;
			}
			bHaveSock	= true;
		}
	}
	// 没有东西可select
	if( !bHaveSock )
	{
		return	0;
	}
	// selectrd
	int		rst;
	struct timeval tv = {0, ms * 1000L};
	rst		= select(nBigestSock, &m_readfds, NULL, NULL, &tv);
	return	rst;
}
int		ConnecterMan::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	CONNECTOR_INFO_T	*pCntrInfo;
	m_Connecters.BeginGet();
	while( (pCntrInfo=m_Connecters.GetNext())!=NULL )
	{
		assert( pCntrInfo->pCntr );
		SOCKET sock	= pCntrInfo->pCntr->GetSocket();
		if( sock != INVALID_SOCKET )
		{
			vect.push_back(sock);
		}
	}
	return	0;
}
int		ConnecterMan::DoWork(bool bSureSend)
{
	m_tmpvectID.clear();

	int					nID;
	CONNECTOR_INFO_T	*pCntrInfo;
	m_Connecters.BeginGet();
	while( (pCntrInfo=m_Connecters.GetNext(&nID))!=NULL )
	{
		pCntrInfo->pCntr->Connecter_DoOneWork();
		if( pCntrInfo->pCntr->GetSocketError() )
		{
			// 删除之
			m_tmpvectID.push_back(nID);
		}
	}
	for(size_t i=0;i<m_tmpvectID.size();i++)
	{
		RemoveConnecter(m_tmpvectID[i]);
	}
	if( bSureSend )
	{
		SureSend();
	}
	return	0;
}
int		ConnecterMan::SureSend()
{
	int					nID;
	CONNECTOR_INFO_T	*pCntrInfo;
	m_Connecters.BeginGet();
	while( (pCntrInfo=m_Connecters.GetNext(&nID))!=NULL )
	{
		// 不判断是否网络错误了，反正错误的时间不长
		pCntrInfo->pCntr->Connecter_SureSend();
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// Connecter
////////////////////////////////////////////////////////////////////
Connecter::~Connecter()
{
	if( m_bAutoCloseSocket )
	{
		if( m_sock!=INVALID_SOCKET )
		{
			tcp_verysafeclose(m_sock);
			m_sock	= INVALID_SOCKET;
		}
	}
}

////////////////////////////////////////////////////////////////////
// Connecter_TCPAccepter
////////////////////////////////////////////////////////////////////
int		Connecter_TCPAccepter::Connecter_DoOneWork()
{
	// 检查有无连入
	int			rst;
	struct sockaddr_in	addr;
	SOCKET		sock;
	rst			= tcp_accept_socket(m_sock, &sock, &addr, 0);
	if( rst>0 )
	{
		if( m_pallowdeny )
		{
			// 看是否需要接受
			if( !m_pallowdeny->isallowed(addr) )
			{
				tcp_verysafeclose(sock);
				return	0;
			}
		}
		rst	= DealNewSocket(sock, &addr);
	}
	if( rst<0 )
	{
		// 无论哪里出错，这个都设置错误状态
		SetSocketError();
	}
	return	0;
}
int		Connecter_TCPAccepter::Connecter_SureSend()
{
	return	0;
}

////////////////////////////////////////////////////////////////////
// TCPConnecterServer
////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(TCPConnecterServer::DATA_INI_INFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nPort, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nPort1, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szIP, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szAllowDeny, 0)
WHDATAPROP_MAP_END()
TCPConnecterServer::~TCPConnecterServer()
{
	Release();
}
int		TCPConnecterServer::Init(INFO_T *pInfo)
{
	memset(&m_bindaddr, 0, sizeof(m_bindaddr));
	m_sock	= tcp_create_listen_socket(pInfo->szIP, pInfo->nPort, pInfo->nPort1, &m_bindaddr, pInfo->nMaxNum);
	if( m_sock == INVALID_SOCKET )
	{
		return	-1;
	}
	if( ConnecterMan::Init(pInfo->nMaxNum+1) < 0 )
	{
		return	-2;
	}
	// 允许禁止地址
	if( pInfo->szAllowDeny[0] )
	{
		if( m_AllowDeny4Connect.initfromfile(pInfo->szAllowDeny)<0 )
		{
			return	-3;
		}
		m_pallowdeny	= &m_AllowDeny4Connect;
	}
	// 把自己加入自己
	m_bDeleteByMan		= false;
	m_bAutoCloseSocket	= true;
	ConnecterMan::AddConnecter(this);
	return	0;
}
int		TCPConnecterServer::Release()
{
	if( m_sock != INVALID_SOCKET )
	{
		tcp_verysafeclose(m_sock);
		m_sock	= INVALID_SOCKET;
	}
	ConnecterMan::Release();
	return	0;
}
int		TCPConnecterServer::DeleteAllConnection()
{
	ConnecterMan::RemoveAllConnecter();
	// 再重新把自己加进去(因为自己是一直要呆在里面的)
	ConnecterMan::AddConnecter(this);
	return	0;
}
int		TCPConnecterServer::DealNewSocket(SOCKET sock, struct sockaddr_in *paddr)
{
	// 创建Connecter，加入Man
	if( GetAvailSize()==0 )
	{
		// 无法再申请了。不过这个错误可以允许。(可能只是太多人尝试连接)
		tcp_verysafeclose(sock);
		ErrNotify(ERRNOTIFYCMD_AVAILSIZEIS0, paddr);
		return	0;
	}
	Connecter *	pConnecter	= NewConnecter(sock, paddr);
	if( !pConnecter )
	{
		// 无法创建connecter。关闭这个socket。
		tcp_verysafeclose(sock);
		// 注意：不用assert(0)了，因为可能在结束的时候有重连也会这样
		return	-1;
	}
	pConnecter->SetSocket(sock);
	if( AddConnecter(pConnecter)<0 )
	{
		// 里面会自动关闭socket
		delete	pConnecter;
		assert(0);
		return	-1;
	}
	return	0;
}
int		TCPConnecterServer::GetFirstConnecterID()
{
	CONNECTOR_INFO_T	*pCInfo;
	m_Connecters.BeginGet();
	while( (pCInfo=m_Connecters.GetNext()) != NULL )
	{
		if( pCInfo->pCntr->GetIDInMan() != Connecter_TCPAccepter::m_nIDInMan )
		{
			return	pCInfo->pCntr->GetIDInMan();
		}
	}
	return	-1;
}
