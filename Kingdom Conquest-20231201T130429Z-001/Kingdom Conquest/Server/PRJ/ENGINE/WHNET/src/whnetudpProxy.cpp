// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpProxy.cpp
// Creator      : Wei Hua (κ��)
// Comment      : UDP�˿�ת�������ʵ�ֲ���
// CreationDate : 2006-12-19
// ChangeLog    :

#include "../inc/whnetudpProxy.h"
#include "../inc/whnetudp.h"
#include "../inc/whnetudpGLogger.h"
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whhash.h>

using namespace n_whcmn;
using namespace n_whnet;

namespace n_whnet
{

class	whnetudpProxy_I	: public whnetudpProxy
{
// Ϊ�ӿ�ʵ�ֵķ���
public:
	whnetudpProxy_I();
	virtual	~whnetudpProxy_I();
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const INFO_T *pInfo);
	virtual	int		ResetInfo(const INFO_T *pInfo);
	virtual	int		Release();
	virtual	int		Tick(int nMS);
// ���õĲ���
protected:
	// �����ͽṹ����
	struct	FORWARD_T
	{
		SOCKET						sockDest;						// ��Ŀ�귢�����ݺͽ������ݵ�socket
		struct sockaddr_in			addrSrc;						// �û���Դ��ַ����sockDest�յ������ݾ�ת����addrSrcȥ��
		whtick_t					tickLastRecv;					// ���һ���յ�������˫���Ķ����ԣ�
		FORWARD_T()
			: sockDest(INVALID_SOCKET)
			, tickLastRecv(0)
		{
			memset(&addrSrc, 0, sizeof(addrSrc));
		}
		void	clear()
		{
			cmn_safeclosesocket(sockDest);
		}
	};
	struct	TQUNIT_T
	{
		typedef		void (whnetudpProxy_I::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// ������¼��ĺ���
		void		*pData;											// ��������
		size_t		nDSize;											// �������ݳߴ�
		union
		{
			int		nForwardID;										// ���¼�������Forward��Ԫ��ID�����ڣ�
		}			un;

		void		clear()
		{
			WHMEMSET0THIS();
		}
		void		SetData(const void *__pData, size_t __nDSize)
		{
			assert(pData == NULL);
			pData	= malloc(__nDSize);
			nDSize	= __nDSize;
			memcpy(pData, __pData, nDSize);
		}
		void		FreeData()
		{
			if( pData )
			{
				free(pData);
				pData	= NULL;
			}
		}
	};
	// ��Ա����
	INFO_T							m_info;							// ������Ϣ
	SOCKET							m_sockListen;					// �����û�������socket
	struct sockaddr_in				m_addrDest;						// Ŀ���ַ
	whunitallocatorFixed<FORWARD_T>	m_Forwards;						// ����Forward����������
	whhash<struct sockaddr_in, int, whcmnallocationobj, _whnet_addr_hashfunc>			m_mapAddrSrc2Forward;
																	// ��Դ��ַ��Forward��ԪID��ӳ��
	whtimequeue						m_tq;							// ʱ�����
	whtick_t						m_tickNow;
	whvector<char>					m_vectBuf;						// ������ʱ�洢�����Ļ���
	WHRANGE_T<int>					m_wr100;						// ��������100���ڵ�������ģ�ⶪ���İٷ���
	WHRANGE_T<int>					m_wrUpDelay;					// ���е��ӳ�ʱ��
	WHRANGE_T<int>					m_wrDownDelay;					// ���е��ӳ�ʱ��
	whvector<SOCKET>				m_vectSocket;					// ������ʱ�洢����select��socket
private:
	int		Tick_DoSelect(int nMS);
	int		Tick_FORWARD();
	int		Tick_TQ();
	void	Reg_TEDeal_DropCheck(int nForwardID);
	void	TEDeal_DropCheck(TQUNIT_T *pTQUnit);
	void	TEDeal_Up_Send(TQUNIT_T *pTQUnit);
	void	TEDeal_Down_Send(TQUNIT_T *pTQUnit);
};

}	// EOF namespace n_whnet

whnetudpProxy *	whnetudpProxy::Create()
{
	return	new whnetudpProxy_I;
}

whnetudpProxy_I::whnetudpProxy_I()
: m_sockListen(INVALID_SOCKET)
, m_tickNow(0)
, m_wr100(0,100)
{
	m_vectSocket.reserve(1024);
}
whnetudpProxy_I::~whnetudpProxy_I()
{
}
int		whnetudpProxy_I::Init(const INFO_T *pInfo)
{
	ResetInfo(pInfo);
	// �жϲ����Ƿ�Ϸ�

	// ��������socket
	m_sockListen	= udp_create_socket_by_ipnportstr(m_info.szLocalAddr);
	if( m_sockListen == INVALID_SOCKET )
	{
		return	-11;
	}
	// ���Զ�˵�ַ
	if( !cmn_get_saaddr_by_ipnportstr(&m_addrDest, m_info.szDestAddr) )
	{
		return	-12;
	}
	// ��ʼ������
	if( m_Forwards.Init(m_info.nMaxFORWARD)<0 )
	{
		return	-13;
	}
	// ��ʼ��ʱ�����
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_info.nTQChunkSize;
	if( m_tq.Init(&tqinfo)<0 )
	{
		return	-14;
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(16,UDPPROXY)"whnetudpProxy_I::Init over,%s,%s", m_info.szLocalAddr, m_info.szDestAddr);

	return	0;
}
int		whnetudpProxy_I::ResetInfo(const INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	m_vectBuf.resize(m_info.nPacketSize);
	m_wrUpDelay.Set(m_info.nUpDelay0, m_info.nUpDelay1);
	m_wrDownDelay.Set(m_info.nDownDelay0, m_info.nDownDelay1);
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(17,UDPPROXY)"whnetudpProxy_I::ResetInfo,%d,%d,%d,%d,%d,%d"
		, m_info.nUpLostRate, m_info.nDownLostRate
		, m_info.nUpDelay0, m_info.nUpDelay1
		, m_info.nDownDelay0, m_info.nDownDelay1
		);
	return	0;
}
int		whnetudpProxy_I::Release()
{
	cmn_safeclosesocket(m_sockListen);
	m_Forwards.Release();
	m_tq.Release();
	return	0;
}
int		whnetudpProxy_I::Tick(int nMS)
{
	m_tickNow	= wh_gettickcount();
	// �Ƚ���select
	Tick_DoSelect(nMS);
	// Ȼ���鿴���Ƿ����յ�������ͬʱ����ת����
	Tick_FORWARD();
	// ʱ����е��߼�
	Tick_TQ();
	return	0;
}
int		whnetudpProxy_I::Tick_DoSelect(int nMS)
{
	m_vectSocket.clear();
	m_vectSocket.push_back(m_sockListen);
	// Ȼ�����еĺ�dest������socket���ں���
	for(whunitallocatorFixed<FORWARD_T>::iterator it=m_Forwards.begin(); it!=m_Forwards.end(); ++it)
	{
		m_vectSocket.push_back((*it).sockDest);
	}
	return	cmn_select_rd_array_big(m_vectSocket.getbuf(), m_vectSocket.size(), nMS);
}
int		whnetudpProxy_I::Tick_FORWARD()
{
	// �ȿ���Incoming
	while( cmn_select_rd(m_sockListen)>0 )
	{
		// �������ݣ����ˣ����͸�Ŀ��
		struct sockaddr_in	addr;
		int	nRecvSize	= udp_recv(m_sockListen, m_vectBuf.getbuf(), m_vectBuf.size(), &addr);
		if( nRecvSize<=0 )
		{
			// ������
			continue;
		}
		// ���Ƿ񶪰�
		if( m_info.nUpLostRate>0
		&&  m_wr100.GetRand()<=m_info.nUpLostRate
		)
		{
			// ���Զ������ǾͲ��ü�����
			continue;
		}
		// ���Ҹ���Դ��ַ��Ӧ�Ǹ�Forward����
		int			nID;
		FORWARD_T	*pForward;
		// ���Ƿ��ǵ�һ���յ������İ�
		if( !m_mapAddrSrc2Forward.get(addr, nID) )
		{
			// ���ԭ��û�оʹ����µ�
			nID	= m_Forwards.AllocUnit(pForward);
			if( nID<0 )
			{
				// û�пռ���������ˣ�������ͱ����ԣ�
				continue;
			}
			// ����hash��
			if( !m_mapAddrSrc2Forward.put(addr, nID) )
			{
				m_Forwards.FreeUnit(nID);
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(18,UDPPROXY)"whnetudpProxy_I::Tick_FORWARD,,m_mapAddrSrc2Forward.put");
				assert(0);
				continue;
			}
			// ��ַ��ֵ
			pForward->addrSrc	= addr;
			// ����socket
			pForward->sockDest	= udp_create_socket(0);
			if( pForward->sockDest==INVALID_SOCKET )
			{
				m_mapAddrSrc2Forward.erase(addr);
				pForward->clear();
				m_Forwards.FreeUnit(nID);
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(19,UDPPROXY)"whnetudpProxy_I::Tick_FORWARD,,upd_create_socket sockDest");
				continue;
			}
			// ���ó�ʱ����¼�
			Reg_TEDeal_DropCheck(nID);
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(20,tcpproxy)"incoming,0x%X,%s,%d", nID, cmn_get_ipnportstr_by_saaddr(&pForward->addrSrc), m_Forwards.size());
		}
		else
		{
			pForward					= m_Forwards.getptr(nID);
		}
		// �����հ�ʱ��
		pForward->tickLastRecv			= m_tickNow;
		// ��������nID��pForward��һ������Ч��ֵ��
		// Ȼ���Ƿ��ӳ�
		int	nDelay	= m_wrUpDelay.GetRand();
		if( nDelay>0 )
		{
			// ����ʱ�����
			TQUNIT_T	*pTQUnit;
			whtimequeue::ID_T	tqid;
			if( m_tq.AddGetRef(m_tickNow + nDelay, (void **)&pTQUnit, &tqid)<0 )
			{
				// ����ʱ�����ʧ��
				assert(0);
				continue;
			}
			pTQUnit->clear();
			pTQUnit->un.nForwardID	= nID;
			pTQUnit->tefunc			= &whnetudpProxy_I::TEDeal_Up_Send;
			pTQUnit->SetData(m_vectBuf.getbuf(), nRecvSize);
			continue;
		}
		// ������ľͿ���ֱ�ӷ�����
		udp_sendto(pForward->sockDest, m_vectBuf.getbuf(), nRecvSize, &m_addrDest);
	}

	// �ٿ���FromDest
	for(whunitallocatorFixed<FORWARD_T>::iterator it=m_Forwards.begin(); it!=m_Forwards.end(); ++it)
	{
		FORWARD_T	*pForward	= &(*it);
		while( cmn_select_rd(pForward->sockDest)>0 )
		{
			// �������ݣ����ˣ����ظ���Դ
			struct sockaddr_in	addr;
			int	nRecvSize	= udp_recv(pForward->sockDest, m_vectBuf.getbuf(), m_vectBuf.size(), &addr);
			if( nRecvSize<=0 )
			{
				continue;
			}
			// �����հ�ʱ��
			pForward->tickLastRecv	= m_tickNow;
			// �ȿ��Ƿ񶪰�
			if( m_info.nDownLostRate>0
			&&  m_wr100.GetRand()<=m_info.nDownLostRate
			)
			{
				// ���Զ������ǾͲ��ü�����
				continue;
			}
			// Ȼ���Ƿ��ӳ�
			int	nDelay	= m_wrDownDelay.GetRand();
			if( nDelay>0 )
			{
				// ����ʱ�����
				TQUNIT_T	*pTQUnit;
				whtimequeue::ID_T	tqid;
				if( m_tq.AddGetRef(m_tickNow + nDelay, (void **)&pTQUnit, &tqid)<0 )
				{
					// ����ʱ�����ʧ��
					assert(0);
					continue;
				}
				pTQUnit->clear();
				pTQUnit->un.nForwardID	= m_Forwards.GetIDByPtr(pForward);
				pTQUnit->tefunc			= &whnetudpProxy_I::TEDeal_Down_Send;
				pTQUnit->SetData(m_vectBuf.getbuf(), nRecvSize);
				continue;
			}
			// ������ľͿ���ֱ�ӷ�����
			udp_sendto(m_sockListen, m_vectBuf.getbuf(), nRecvSize, &pForward->addrSrc);
		}
	}
	return	0;
}
int		whnetudpProxy_I::Tick_TQ()
{
	m_tickNow	= wh_gettickcount();
	// ����ʱ��������Ƿ��ж���
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_tq.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// ���������������ʱ���¼�����������������ŵġ�
		// ���get������ֻ��ͨ��Delɾ���������Լ�ɾ
		m_tq.Del(id);
	}
	return	0;
}
////////////////////////////////////////////////////////////////////
// ʱ���¼�������
////////////////////////////////////////////////////////////////////
void	whnetudpProxy_I::Reg_TEDeal_DropCheck(int nForwardID)
{
	TQUNIT_T			*pTQUnit;
	whtimequeue::ID_T	tqid;
	if( m_tq.AddGetRef(m_tickNow + m_info.nDropTimeOut/2, (void **)&pTQUnit, &tqid)==0 )
	{
		pTQUnit->clear();
		pTQUnit->tefunc			= &whnetudpProxy_I::TEDeal_DropCheck;
		pTQUnit->un.nForwardID	= nForwardID;
	}
	else
	{
		assert(0);
	}
}
void	whnetudpProxy_I::TEDeal_DropCheck(TQUNIT_T *pTQUnit)
{
	// �ҵ���Ӧ��Forwar��Ԫ
	FORWARD_T	*pForward	= m_Forwards.getptr(pTQUnit->un.nForwardID);
	if( !pForward )
	{
		return;
	}
	// �����Ƿ��ϴεĽ���ʱ���Ѿ������������ֵ
	if( wh_tickcount_diff(m_tickNow, pForward->tickLastRecv)>=m_info.nDropTimeOut )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(21,UDPPROXY)"remove by timeout,0x%X,%s,%d", pTQUnit->un.nForwardID, cmn_get_ipnportstr_by_saaddr(&pForward->addrSrc), m_Forwards.size()-1);
		// ɾ��Forward����
		// �Ӵ˲��������Forward������
		m_mapAddrSrc2Forward.erase(pForward->addrSrc);
		m_Forwards.FreeUnit(pTQUnit->un.nForwardID);
		return;
	}
	// ����������һ����ʱ����¼�
	Reg_TEDeal_DropCheck(pTQUnit->un.nForwardID);
}
void	whnetudpProxy_I::TEDeal_Up_Send(TQUNIT_T *pTQUnit)
{
	// �ҵ���Ӧ��Forwar��Ԫ
	FORWARD_T	*pForward	= m_Forwards.getptr(pTQUnit->un.nForwardID);
	if( !pForward )
	{
		return;
	}
	// ����
	udp_sendto(pForward->sockDest, pTQUnit->pData, pTQUnit->nDSize, &m_addrDest);
	// �ͷ��ڴ�
	pTQUnit->FreeData();
}
void	whnetudpProxy_I::TEDeal_Down_Send(TQUNIT_T *pTQUnit)
{
	// �ҵ���Ӧ��Forwar��Ԫ
	FORWARD_T	*pForward	= m_Forwards.getptr(pTQUnit->un.nForwardID);
	if( !pForward )
	{
		return;
	}
	// ����
	udp_sendto(m_sockListen, pTQUnit->pData, pTQUnit->nDSize, &pForward->addrSrc);
	// �ͷ��ڴ�
	pTQUnit->FreeData();
}
