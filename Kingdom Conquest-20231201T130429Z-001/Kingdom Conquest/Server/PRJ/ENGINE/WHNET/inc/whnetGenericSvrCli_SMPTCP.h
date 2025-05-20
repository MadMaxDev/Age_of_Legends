// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetGenericSvrCli_SMPTCP.h
// Creator      : Wei Hua (κ��)
// Comment      : ���ڼ򵥵�TCP�����whnetGenericSvrCli��ʵ��
// CreationDate : 2005-03-04
// Change LOG   :

#ifndef	__WHNETGENERICSVRCLI_SMPTCP_H__
#define	__WHNETGENERICSVRCLI_SMPTCP_H__

#include "whnetGenericSvrCli.h"
#include "whconnecter.h"
#include "whnettcp.h"
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whcmd.h>

using namespace n_whcmn;

namespace n_whnet
{

class	whnetGenericSvrCli_SMTCPSVR	: public whnetGenericSvrCli, public TCPConnecterServer
{
public:
	WHDATAINI_STRUCT_DECLARE(CNTRSVRINFO_T, TCPConnecterServer::INFO_T);
	WHDATAINI_STRUCT_DECLARE(MSGERINFO_T, tcpmsger::INFO_T)
	struct	CFGINFO_T		: public whdataini::obj
	{
		CNTRSVRINFO_T	CNTRSVRINFO;
		MSGERINFO_T		MSGERINFO;
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
public:
	whnetGenericSvrCli_SMTCPSVR();
	virtual ~whnetGenericSvrCli_SMTCPSVR();
public:
	// ΪwhnetGenericSvrCliʵ��
	virtual	int	Init(const char *cszCFG);
	virtual	int	Release();
	virtual	int	DoSelect(int nInterval);
	virtual	int	Tick();
	virtual int	CmdIn(int nCmd, const void *pData, size_t nDSize);
private:
	// ΪTCPConnecterServerʵ��
	virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	virtual void	AfterAddConnecter(Connecter * pCntr);
	virtual void	BeforeDeleteConnecter(int nCntrID, CONNECTOR_INFO_T *pCntrInfo);
private:
	// �Լ��õĳ�Ա����
	// ������Ϣ
	CFGINFO_T		m_cfginfo;
	// ������ʱ���ָ��Ļ�����
	whvector<char>	m_vectTmpBuf;
	// ���ڴ����ҪRemove��Connect��ID
	whvector<int>	m_vectConnecterToRemove;
	// ״̬
	int				m_nStatus;
	// ָ��ӳ����
	WHCMN_CMD_REGANDDEAL_DECLARE_m_rad(whnetGenericSvrCli_SMTCPSVR);
private:
	// ����ָ���Ӧ�ĺ���
	int	Cmd_SVRCMD_REQ_STARTSERVER(const void *pData, size_t nDSize);
	int	Cmd_SVRCMD_REQ_STOPSERVER(const void *pData, size_t nDSize);
	int	Cmd_SVRCMD_REQ_SENDDATA(const void *pData, size_t nDSize);
	int	Cmd_SVRCMD_REQ_DISCONNECTCLIENT(const void *pData, size_t nDSize);
	int	Cmd_SVRCMD_REQ_SETCLIENTTAG(const void *pData, size_t nDSize);
	// ֪ͨ�ϲ��Լ���״̬
	void	StatusNotify();
	// ����״̬ͬʱ֪ͨ�ϲ�(ͬʱ�������ݲ�ͬ��״̬��һЩ����)
	void	SetStatusAndNofity(int nStatus);
public:
	// ��Connecter���õĺ���
	bool	CanDealMsg() const;
	int		DealMsg(int nIDInMan, const void *pData, size_t nSize);
private:
	// �Լ��õ���Connecter�ṹ
	class	I_Connecter	: public TCPConnecterMsger<tcpmsger>
	{
	public:
		enum
		{
			MAXTAGNUM					= 32,
		};
		whnetGenericSvrCli_SMTCPSVR		*m_pHost;
		int								m_aTag[MAXTAGNUM];
		struct sockaddr_in				m_addr;						// �û���ַ
	public:
		I_Connecter(whnetGenericSvrCli_SMTCPSVR *pHost)
		: m_pHost(pHost)
		{
			memset(m_aTag, 0, sizeof(m_aTag));
			memset(&m_addr, 0, sizeof(m_addr));
		}
		// �丸�඼��������
		virtual ~I_Connecter()
		{
		}
		virtual void *	QueryInterface(const char *cszIName)
		{
			// �����
			return	this;
		}
	public:
		// ���ñ��
		int	SetTag(int nIdx, int nValue)
		{
			if( nIdx<0 || nIdx>= MAXTAGNUM )
			{
				// ����
				return	-1;
			}
			m_aTag[nIdx]	= nValue;
			return	0;
		}
		inline int	GetTag(int nIdx) const
		{
			return	m_aTag[nIdx];
		}
	private:
		// ΪTCPConnecterMsgerʵ�ֵ�
		bool	CanDealMsg() const
		{
			return	m_pHost->CanDealMsg();
		}
		int		DealMsg(const void *pData, size_t nSize)
		{
			return	m_pHost->DealMsg(m_nIDInMan, pData, nSize);
		}
	};
};

class	whnetGenericSvrCli_SMTCPCLI	: public whnetGenericSvrCli
{
public:
	WHDATAINI_STRUCT_DECLARE(MSGERINFO_T, tcpmsger::INFO_T)
	struct	CFGINFO_T		: public whdataini::obj
	{
		MSGERINFO_T		MSGERINFO;
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
public:
	whnetGenericSvrCli_SMTCPCLI();
	virtual ~whnetGenericSvrCli_SMTCPCLI();
public:
	virtual	int	Init(const char *cszCFG);
	virtual	int	Release();
	virtual	int	DoSelect(int nInterval);
	virtual	int	Tick();
	virtual int	CmdIn(int nCmd, const void *pData, size_t nDSize);
private:
	// �Լ��õĳ�Ա����
	// ״̬
	int				m_nStatus;
	// ��ʱ��ʱ��
	whtick_t		m_nTimeOutTime;
	// ������Ϣ
	CFGINFO_T		m_cfginfo;
	// ͨѶ��
	tcpmsger		m_msger;
	// ������ʱ���ָ��Ļ�����
	whvector<char>	m_vectTmpBuf;
	// ָ��ӳ����
	WHCMN_CMD_REGANDDEAL_DECLARE_m_rad(whnetGenericSvrCli_SMTCPCLI);
private:
	// ����ָ���Ӧ�ĺ���
	int	Cmd_CLICMD_REQ_CONNECT(const void *pData, size_t nDSize);
	int	Cmd_CLICMD_REQ_DISCONNECT(const void *pData, size_t nDSize);
	int	Cmd_CLICMD_REQ_SENDDATA(const void *pData, size_t nDSize);
	// ֪ͨ�ϲ��Լ���״̬
	void	StatusNotify();
	// ����״̬ͬʱ֪ͨ�ϲ�(ͬʱ�������ݲ�ͬ��״̬��һЩ����)
	void	SetStatusAndNofity(int nStatus);
};

}

#endif	// EOF __WHNETGENERICSVRCLI_SMPTCP_H__
