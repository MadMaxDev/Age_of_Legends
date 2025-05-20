// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_client_i.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��Clientģ���ڲ�ʵ��
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2005-08-25
// Change LOG   :

#ifndef	__PNGS_CLIENT_I_H__
#define	__PNGS_CLIENT_I_H__

#include "./pngs_client.h"
#include "./pngs_packet.h"
#include "./pngs_def.h"
#include "./pngs_packet_client_logic.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whnetcnl2.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whdll.h>
#include <WHCMN/inc/whthread.h>

//#define CLIENT_STATIC_ANTIHACK	// ���ڲ���������ɾ�̬�ģ���ֹ��Ҵ������������

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

class	PNGSClient_I	: public PNGSClient, public CMNBody
{
public:
	// PNGSClient�õ�
	virtual	void	SelfDestroy();
	virtual	int		Init(const char *cszCFG);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		SetICryptFactory(n_whcmn::ICryptFactory *pFactory);
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		TickRecv();
	virtual	int		TickSend();
	virtual	void	SetVer(const char *szVer);
	virtual	const char *	GetVer();
	virtual	int		Connect(const char *cszAddr, unsigned char nNetworkType);
	virtual	int		Disconnect();
	virtual	int		ControlOut(CONTROL_T **ppCmd, size_t *pnSize);
	virtual	int		Send(unsigned char nSvrIdx, unsigned char nChannel, const void *pCmd, size_t nSize);
	virtual	int		SendCAAFS(const void *pCmd, size_t nSize);
	virtual	int		Recv(unsigned char *pnChannel, void **ppCmd, size_t *pnSize);
	virtual	int		GetLastError() const;
	virtual	void	SetLastError(int nErrCode);
	virtual void	SetFileMan(n_whcmn::whfileman *pFileMan);
	virtual	ILogicBase *	GetPlugIn(const char *cszPlugInName, int nVer);
	virtual int		GetPing() const;
	virtual int		StartThread();
	virtual int		StopThread();
private:
	// CMNBody��չ��
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
// �Լ�ʹ�õ�
public:
	PNGSClient_I();
	~PNGSClient_I();
private:
	// ���ֽṹ����
	// ��ʼ������
	struct	CFGINFO_T		: public whdataini::obj
	{
		// ��������(��������صĶ���DATA_INI_INFO_T�ж�����)
		char				szCAAFSSelfInfoToClient[256];			// �û�����CAAFS���ȿ�������һ�仰���������ȽϿ������Ƿ���ȷ
		int					nSlotInOutNum;							// ����slot��In/Out������һ����
		int					nTQChunkSize;							// ʱ����е�ÿ�δ��Է�����С
		int					nControlOutQueueSize;					// ����֪ͨ�ϲ��ָ����г���
		int					nDataQueueSize;							// ����֪ͨ�ϲ����ݵĶ��г���
		int					nQueueCmdInSize;
		int					nPlayStatInteravl;						// ����Ϊ<=0��ʾ����ͳ��

		CFGINFO_T()
		: nSlotInOutNum(PNGS_SLOTINOUTNUM)
		, nTQChunkSize(100)
		, nControlOutQueueSize(10000)
		, nDataQueueSize(PNGS_RAWBUF_SIZE)							// Ĭ�ϴ�һ��ɡ������ۼƺܶ�ָ��Ͷ��ˡ�
		, nQueueCmdInSize(512*1024)
		, nPlayStatInteravl(66*1000)
		{
			strcpy(szCAAFSSelfInfoToClient, PNGS_DFT_CAAFS_SELFINFO_STRING);
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// ʱ���¼��ṹ
	struct	TQUNIT_T
	{
		typedef	void (PNGSClient_I::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// ������¼��ĺ���
		// ���ݲ���
		union
		{
		}un;
	};
	// ��m_queueData��������ݸ�ʽ(�ڲ��ã����Բ���pack����)
	struct	DATA_T
	{
		size_t			nSize;
		unsigned char	nChannel;
		char			data[1];
	};
	// Ϊ��PlugIn׼����
	// ����װ��DLL�еĺ�����ָ��
	typedef	n_pngs::CMN::ILogic *	(*fn_ILOGIC_CREATE_T)();
	typedef	void					(*fn_WHNET_STATIC_INFO_In)(void *);
	typedef	void					(*fn_WHCMN_STATIC_INFO_In)(void *);
	struct	PLUGIN_CFGINFO_T	: public whdataini::obj
	{
		PNGSClient_I			*pHost;

		WHDATAPROP_SETVALFUNC_DECLARE(DLL)
		{
			return	pHost->AddPlugIn(cszVal);
		}
		// ����Ϊ�г�Աӳ��Ľṹ
		WHDATAPROP_DECLARE_MAP(PLUGIN_CFGINFO_T)
	};
private:
	// ���ݲ���
	CFGINFO_T		m_cfginfo;										// ����
	CNL2SlotMan::DATA_INI_INFO_T		m_CLIENT_SLOTMAN_INFO;		// �ͻ������ӷ�������slotman������
	int				m_nStatus;										// ��ǰ״̬
	int				m_nLastError;									// ������õĴ�����
	whtick_t		m_tickNow;
	int				m_nClientID;									// �ڷ������е�ID
	char			m_szVer[PNGS_VER_LEN];							// �汾��

#ifdef CLIENT_STATIC_ANTIHACK
	// ���ܹ���
	static ICryptFactory	*m_pICryptFactory;
	// ���ӹ���
	static CNL2SlotMan		*m_pSlotMan;
#else
	// ���ܹ���
	ICryptFactory	*m_pICryptFactory;
	// ���ӹ���
	CNL2SlotMan		*m_pSlotMan;
#endif

	// slot��
	int				m_nSlotCAAFS;									// ��CAAFS���ӵ�
	int				m_nSlotCLS;										// ��CLS���ӵ�
	// ��¼�ϴ�CAAFS_CLIENT_GOTOCLS_T������
	CAAFS_CLIENT_GOTOCLS_T	m_GoToCLSInfo;
	// ʱ����С����磺���ӳ�ʱ
	whtimequeue		m_TQ;
	// ControlOut�Ķ���
	whsmpqueue		m_queueControlOut;
	// ��ʱ���m_queueControlOut���ϲ��ָ��
	whvector<char>	m_vectControlOut;
	// �յ����ݵĶ���
	whsmpqueue		m_queueData;
	// ��ʱ���m_queueData���ϲ������
	whvector<char>	m_vectData;
	// ������ʱ�ϳ�ָ���
	whvector<char>	m_vectrawbuf;

	// ״̬������ָ��
	typedef	int	(PNGSClient_I::*TICK_CONTROLOUT_STATUS_FUNC_T)(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	TICK_CONTROLOUT_STATUS_FUNC_T	m_pTickControlOut;
	typedef	int	(PNGSClient_I::*TICK_DEALRECV_STATUS_FUNC_T)(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	TICK_DEALRECV_STATUS_FUNC_T		m_pTickDealRecv;

	// ��PlugIn��ص�
	PLUGIN_CFGINFO_T				m_PlugInCfg;
	const char						*m_pcszCFG;						// PlugIn��ʼ��ʱ��ҪĬ�ϵ������ļ���
	whvector<WHDLL_Loader *>		m_vectDLLLoader;				// ����DLL������
	bool							m_bQuickDetachLogic;			// �����Init�����г����ˡ���Releaseʱ�������Ҫ��Ϊtrue��
	whhash<pngs_cmd_t, ILogicBase *>	m_mapCmdReg2Logic;			// ����ָ��ּ𣬰��ض���ָ��ּ���ض����߼�ģ��ȥִ��

	// �ڲ��߳�
	whtid_t							m_tid;
	bool							m_bStopThread;					// ����Ƿ�ֹͣ�߳�

	// stat���
	whtimequeue::ID_T				m_teidStat;
private:
	// Init�Ӻ���
	int		Init_CFG(const char *cszCFG);
	// ����״̬��ͬʱ������ص����ԣ��磺������������
	void	SetStatus(int nStatus);
	// ����ControlOut���ϲ�
	void *	ControlOutAlloc(size_t nSize);
	template<typename _Ty>
	inline void	ControlOutAlloc(_Ty *&ptr)
	{
		ptr	= (_Ty *)ControlOutAlloc(sizeof(_Ty));
	}
	// Tick�Ӻ���
	int		Tick_ControlOut_STATUS_NOTHING(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_ControlOut_STATUS_CONNECTINGCAAFS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_ControlOut_STATUS_WAITINGINCAAFS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_ControlOut_STATUS_CONNECTINGCLS(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_ControlOut_STATUS_WORKING(CNL2SlotMan::CONTROL_T *pCOCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_NOTHING(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_CONNECTINGCAAFS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_WAITINGINCAAFS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_CONNECTINGCLS(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	int		Tick_DealRecv_STATUS_WORKING(int nChannel, pngs_cmd_t *pClientCmd, size_t nSize);
	// ����һ��CLS_CLIENT_DATA���ݰ��������ڲ��ּ�Ȼ��ѹ����ϲ�Ķ��У�
	int		Deal_CLS_CLIENT_DATA_One(int nChannel, const void *pData, size_t nDSize);

	// ��ʱ��ص�
	void	TEDeal_PlayStat(TQUNIT_T * pTQUnit);
	void	SetTE_PlayStat();
private:
	// �ڲ�ʹ�õĸ�����ṩ���������ǲ��֣���Ҫ��Ϊ�˸��߳����ģ�
	int		_GetSockets(n_whcmn::whvector<SOCKET> &vect);
	int		_TickRecv();
	int		_TickSend();
public:
	int		AddPlugIn(const char *cszVal);
	// �̺߳���
	static void *	thread_func(void *ptr);
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CLIENT_I_H__
