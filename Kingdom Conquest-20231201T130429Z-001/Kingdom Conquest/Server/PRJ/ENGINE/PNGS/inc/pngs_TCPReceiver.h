// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_TCPReceiver.h
// Creator      : Wei Hua (κ��)
// Comment      : TCPָ�����ģ��
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2007-02-25
// Change LOG   : 2008-05-20 ��PNGS_TR2CD_CMD_T��������IP

#ifndef	__PNGS_TCPRECEIVER_H__
#define	__PNGS_TCPRECEIVER_H__

#include "pngs_cmn.h"
#include <WHNET/inc/whconnecter.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whgener.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

////////////////////////////////////////////////////////////////////
// PNGS_TCPRECEIVER
////////////////////////////////////////////////////////////////////
class	PNGS_TCPRECEIVER	: public CMN::ILogic, public TCPConnecterServer
{
///////////////////////////////////////////
// Ϊ�ӿ�ʵ�ֵ�
///////////////////////////////////////////
protected:
	// CMN::ILogic��
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	PNGS_TCPRECEIVER();
	~PNGS_TCPRECEIVER();
	// CMN::ILogic��
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		SureSend();

private:
	// CMN::ILogic��
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		Tick_BeforeDealCmdIn();
	virtual int		Tick_AfterDealCmdIn();
	// TCPConnecterServer��
	virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	virtual void		BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo);
	virtual void		AfterAddConnecter(Connecter * pCntr);
// �Լ��õ�
private:
	struct	CFGINFO_T		: public whdataini::obj
	{
		int										nQueueCmdInSize;	// ָ��������еĳ���
		int										nMaxCmdPerSec;		// ÿ���������Դ���ķ�����ָ������
		TCPConnecterServer::DATA_INI_INFO_T		CNTRSVR;			// TCPConnecterServer��ͨ������
		tcpmsger::DATA_INI_INFO_T				MSGER;				// ÿ��TCP���ӵ�ͨѶmsger������

		CFGINFO_T()
			: nQueueCmdInSize(2000000)
			, nMaxCmdPerSec(2000)
		{
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	class	MYCNTR	: public	TCPConnecterMsger<tcpmsger>
	{
	public:
		PNGS_TCPRECEIVER	*m_pHost;
		struct sockaddr_in	m_addr;								// ��Դ��ַ
	public:
		MYCNTR(PNGS_TCPRECEIVER *pHost)
			: m_pHost(pHost)
		{
		}
		// ��ԭ����Ȼ�������ȥ���ˣ���
		virtual void *	QueryInterface(const char *cszIName)
		{
			// ����������Ϳ��Ժ�Accepter�����ˣ�Accepter����NULL��
			return	this;
		}
		// �Ƿ��пմ�����Ϣ
		virtual bool	CanDealMsg() const
		{
			return	m_pHost->CNTR_CanDealMsg();
		}
		// ������ʵ����Ϣ����
		virtual int		DealMsg(const void *pData, size_t nSize)
		{
			return	(m_pHost->CNTR_DealMsg)(this, pData, nSize);
		}
	};

	CFGINFO_T	m_cfginfo;
	ILogicBase	*m_pLogicDftCmdDealer;	// Ĭ�ϵ�ָ���ģ��
	typedef		whhash<pngs_cmd_t, whlist<ILogicBase *> * >	MAPCMD2LOGIC_T;
	typedef		MAPCMD2LOGIC_T::kv_iterator					MAPCMD2LOGIC_IT_T;
	MAPCMD2LOGIC_T	m_mapCmdReg2Logic;						// ����ָ��ּ𣬰��ض���ָ��ּ���ض����߼�ģ��ȥִ��

	// �������ɵ�λʱ����Դ�������ָ������
	whgener		m_generDealGMSCmdNum;
	// ���ڻ��ÿ��tick��ʱ����
	whinterval	m_itv;
	// ��ǰtickʣ��ɴ����ָ������
	int			m_nCmdNumThisTick;

private:
	friend	class	MYCNTR;
	// �����ļ���ȡ
	int		Init_CFG(const char *cszCFGName);
	// ���һ��Connecter��һ�ι���
	bool	CNTR_CanDealMsg() const;
	// ���һ��Connecter��һ�ι���
	int		CNTR_DealMsg(MYCNTR *pCntr, const void *pCmd, size_t nSize);

public:
	// �����Լ����͵Ķ���
	static CMN::ILogic *	Create()
	{
		return	new	PNGS_TCPRECEIVER;
	}
};

// �����ڲ��Է��ص�CmdDealer���Ѵ�����ָ��ԭ�����أ�
class	PNGS_TCPRECEIVER_DUMMYCMDDEALER	: public CMN::ILogic
{
///////////////////////////////////////////
// Ϊ�ӿ�ʵ�ֵ�
///////////////////////////////////////////
protected:
	// CMN::ILogic��
	virtual	int		Organize();
public:
	PNGS_TCPRECEIVER_DUMMYCMDDEALER();
	~PNGS_TCPRECEIVER_DUMMYCMDDEALER();
	// CMN::ILogic��
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
private:
	// CMN::ILogic��
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		Tick_BeforeDealCmdIn();
	virtual int		Tick_AfterDealCmdIn();
///////////////////////////////////////////
// ���õ�
///////////////////////////////////////////
private:
	ILogicBase	*m_pLogicTCPReceiver;

public:
	// �����Լ����͵Ķ���
	static CMN::ILogic *	Create()
	{
		return	new	PNGS_TCPRECEIVER_DUMMYCMDDEALER;
	}
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_TCPRECEIVER_H__
