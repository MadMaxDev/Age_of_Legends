// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CAAFSģ����ڲ�ʵ��
//                PNGS��Pixel Network Game Structure����д
//                CAAFS��Connection Allocation and Address Filter Server����д�������ӷ��书���е����ӷ���͵�ַ���˷�����
//                CAAFS����ֻ�������û����ȴ����ӵģ��Ŷӵģ������������еģ����Ӻõ��û����뿪��CAAFS������CAAFS�ǿ���崵��������ġ�
//                CAAFS�в���¼����CLS����Ϣ����Щ��Ϣ������GMS�м�¼�ġ�CAAFS������ֻ���ύ�û�ϣ������������GMS��
// CreationDate : 2005-07-26
// Change LOG   :

#ifndef	__PNGS_CAAFS_I_H__
#define	__PNGS_CAAFS_I_H__

#include "./pngs_caafs.h"
#include "./pngs_packet.h"
#include "./pngs_cmn.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whnetcnl2.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whDList.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whgener.h>
#include <WHCMN/inc/whcmd.h>
#include <set>
#include <map>
#include <string>

using namespace n_whnet;
using namespace n_whcmn;
using namespace std;

#define TTY_ACCOUNTNAME_LONG_LEN 64  //����include̫�ණ���ˣ��������ﶨ����
//#define UNITTEST

namespace n_pngs
{

class	CAAFS2_I	: public CAAFS2
{
public:
	virtual	void	SelfDestroy();
	virtual	int		Init(const char *cszCFG);
	virtual	int		Init_CFG_Reload(const char *cszCFG);
	virtual	int		Release();
	virtual	int		SetICryptFactory(ICryptFactory *pFactory);
	virtual size_t	GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		Tick();
	virtual	int		GetStatus() const;
	virtual	bool	ShouldStop() const;
public:
	CAAFS2_I();
	~CAAFS2_I();
private:
	// ���ֽṹ����
	// ��ʼ������
	struct	CFGINFO_T		: public whdataini::obj
	{
		// ��������
		int					nVerCmpMode;							// ȡֵΪ(PNGS_VERCMP_MODE_XXX������pngs_cmn.h)
		char				szVer[PNGS_VER_LEN];					// �汾���ͻ��˱��������汾�Ա�һ���˲��У�
		char				szWelcomeAddr[WHNET_MAXADDRSTRLEN];		// �û������ķ�������ַ
		char				szGMSAddr[WHNET_MAXADDRSTRLEN];			// GMS�ȴ��������������ӵ�ַ(���з���������GMS�ĵ�ַ�������)
		int					nConnectGMSTimeOut;						// ����GMS�ĳ�ʱ
		int					nSelectInterval;						// Tick����select�ļ��(����)
		int					nGroupID;								// ����Ӧ��CLS��Ӧ
		int					nTQChunkSize;							// ʱ����е�ÿ�δ��Է�����С
		int					nMaxPlayerRecvCmdNum;					// ÿ����ҿ��Է���������ָ������
		// ������صĲ���
		int					nClientWaitingNumMax;					// ���ڵȴ�״̬����ҵ��������
		int					nClientConnectingNumMax;				// �������ӹ����е���ҵ��������
		int					nHelloCheckTimeOut;						// �������Ƿ���Hello�ĳ�ʱ(����)
		int					nCloseCheckTimeOut;						// �����ҹرպ�CAAFS���ӵĳ�ʱ(����)���������֪ͨ�û�����CLS֮�������ġ�
		int					nQueueSeqSendInterval;					// ����ҷ���ǰ�滹�ж����˵ļ��(����)
		float				fWaitTransToConnectRate;				// �ӵȴ�ת��Ϊ���ӵ�����(��/��)
		bool				bAtLeastOneTransToConnectPerTick;		// ÿ��tick����ת��һ���û���������
		int					nQueueChannelNum;						// ��������
		// ���÷����
		char				szSelfInfoToClient[256];				// �û����ӻ��ȿ�������һ�仰���������ȽϿ������Ƿ���ȷ
		unsigned int		nSelfNotify;							// 32bit��ʾ32����˼
		int					_nSelfInfoToClientLen;					// szSelfInfoToClient�ִ��ĳ���(����û��������õ�)
		int					nQueuePosKeepTime;						// ����λ�ñ���ʱ��(��)�����߶�ú����ӻ��ܻص�ԭ�����Ŷ�λ��
		
		CFGINFO_T()
		: nVerCmpMode(PNGS_VERCMP_MODE_EQUAL)
		, nConnectGMSTimeOut(PNGS_DFT_SVRCONNECT_TIMEOUT)
		, nSelectInterval(50)
		, nGroupID(0)
		, nTQChunkSize(100)
		, nMaxPlayerRecvCmdNum(4)
		, nClientWaitingNumMax(2000)
		, nClientConnectingNumMax(32)
		, nHelloCheckTimeOut(2000)
		, nCloseCheckTimeOut(4000)
		, nQueueSeqSendInterval(5000)
		, fWaitTransToConnectRate(16.f)
		, bAtLeastOneTransToConnectPerTick(false)
		, nQueueChannelNum(9)
		, nSelfNotify(0)
		, _nSelfInfoToClientLen(0)
		, nQueuePosKeepTime(10*60)
		{
			strcpy(szVer, "");
			strcpy(szWelcomeAddr, "localhost:3100");
			sprintf(szGMSAddr, "localhost:%d", GMS_DFT_PORT);
			strcpy(szSelfInfoToClient, PNGS_DFT_CAAFS_SELFINFO_STRING);
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// ���
	struct	PlayerUnit
	{
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_WAITING				= 1,						// �Ŷӵȴ����ӵĻ���
			STATUS_CONNECTINGCLS		= 2,						// CAAFS������CLS����������
																	// Client���ں�CLS����������
		};
		int					nID;									// ��Players�����е�ID
		int					nStatus;								// ��ǰ״̬
		int					nSlot;									// ��Ӧ��ͨѶslot��
		unsigned int		IP;										// �û���IP��ַ
		unsigned short		nSeq0;									// �ϴ�֪ͨʱ���Seq0���
		unsigned short		nSeq;									// ����Ŷӵ����
		unsigned int		nPasswordToConnectCLS;					// ����CLS�õ�����
		whDList<PlayerUnit *>::node		nodeWC;						// ��ӦWaiting��Connecting���У���Ϊ��ֻ�����������������е�һ���У�������һ��node����
		whtimequeue::ID_T	teid;
		unsigned char		nNetworkType;							// ��Ӧ����������
		unsigned char		nRecvCmdNum;							// Ŀǰ�յ���ָ�����������ܳ���ָ��ֵ�����������ң�
		unsigned char		nVIPChannel;							// �Լ������Ǹ�vipchannel

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nSlot(0)
		, IP(0)
		, nSeq0(0)
		, nSeq(0)
		, nPasswordToConnectCLS(0)
		, nNetworkType(NETWORKTYPE_UNKNOWN)
		, nRecvCmdNum(0)
		, nVIPChannel(0)
		{
			nodeWC.data	= this;
		}

		void	clear()
		{
			nID				= 0;
			nStatus			= STATUS_NOTHING;
			nSlot			= 0;
			IP				= 0;
			nSeq0			= 0;
			nSeq			= 0;
			nPasswordToConnectCLS		= 0;
			nodeWC.leave();											// Ϊ�˱��ղ���ô���ġ��ϲ�Ӧ�ñ�֤ʹ�����˾�leave��
			teid.quit();
			nNetworkType	= NETWORKTYPE_UNKNOWN;
			nRecvCmdNum		= 0;
			nVIPChannel		= 0;
		}
	};
	struct	TQUNIT_T
	{
		typedef	void (CAAFS2_I::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// ������¼��ĺ���
		// ���ݲ���
		union
		{
			struct	PLAYER_T										// ��Ҷ�ʱ�¼�
			{
				int	nID;
			}player;
		}un;
	};
private:
	// ���ýṹ
	CFGINFO_T							m_cfginfo;					// CAAFS������
	tcpmsger::DATA_INI_INFO_T			m_MSGERGMS_INFO;			// ��GMS���ӵ�msger������
	CNL2SlotMan::DATA_INI_INFO_T		m_CLIENT_SLOTMAN_INFO;		// �ȴ��ͻ������ӵ�slotman������
	int									m_nStatus;					// CAAFS2::STATUS_XXX
	// ��ǰ��tickʱ��
	whtick_t							m_tickNow;
	// ���ڸ�������client���е�һ��֪ͨ
	unsigned int						m_nSelfNotify;
	// ���ܹ���
	ICryptFactory						*m_pICryptFactory;
	// ��Client�����ӹ���
	CNL2SlotMan							*m_pSlotMan4Client;
	// ʱ�����
	// ���磺�������û��೤ʱ��֪ͨһ��ǰ�滹�ж��ٵȴ�����
	// ���磺�೤ʱ��ͳ��һ������״̬�ж����ˣ����µĵȴ��û��������Ӷ�����
	whtimequeue							m_TQ;
	// �������
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// �Ŷ��е�����б���ͨ����
	struct	WAITCHANNEL_T
	{
		whDList<PlayerUnit *>			*pdlWait;
		// Waiting�����еĵ�һ���˵����
		unsigned short					nWaitingSeq0;
		// Waiting�����е�׼�������˵����(ÿ��һ�����˾�++)
		unsigned short					nWaitingSeqNew;

		void	Init()
		{
			WHMEMSET0THIS();
			pdlWait	= new whDList<PlayerUnit *>;
			nWaitingSeq0 = 1;
			nWaitingSeqNew = 1;
		}
		void	Release()
		{
			if( pdlWait )
			{
				delete	pdlWait;
				pdlWait	= NULL;
			}
		}
	};
	whvector<  WAITCHANNEL_T, true >	m_vectWaitChannel;
	// ���ӹ����е�����б�
	whDList<PlayerUnit *>				m_dlPlayerConnecting;
	// ��������״̬������
	int									m_nConnectingNum;
	// ����GMS��msger
	class	MYMSGER_T	: public tcpretrymsger<tcpmsger>
	{
	public:
		CAAFS2_I		*m_pHost;
		whtick_t		m_tickWorkBegin;
	public:
		virtual void	i_Worker_TRYCONNECT_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_TRYCONNECT_Begin();
		}
		virtual void	i_Worker_WORKING_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	friend class MYMSGER_T;
	MYMSGER_T							m_msgerGMS;
	// GMS���ܹ����ܵ��������
	int									m_nGMSAvailPlayerNum;
	// ��������ÿ��tickת����connecting���еĸ���
	whgener								m_generWaitTransToConnect;
	// ���ڻ��ÿ��tick��ʱ����
	whinterval							m_itv;
	// �ִ�ָ�����ص�
	whcmn_strcmd_reganddeal_DECLARE_m_srad(CAAFS2_I);
	// DealStrCmd�����Ľ�����������
	char								m_szRstStr[GZS_STRCMD_RST_LEN_MAX];
	// �����ϲ�Ӧ���˳�
	bool								m_bShouldStop;
	// �Ƿ���������
	bool								m_bIsReconnect;
	// ��ʱ����ָ��
	whvector<char>						m_vectrawbuf;

	/* 
	�Ŷӵ��ߺ�ָ�����λ���õ�
	*/
	struct ResumedPlayerUnit{
		unsigned short nSeq;
		unsigned short nSeq0;
		unsigned char nVIPChannel;
		unsigned int  nLeaveTime;
		ResumedPlayerUnit():nSeq(0),nSeq0(0),nVIPChannel(0),nLeaveTime(0){};

	};
	map<string,ResumedPlayerUnit> m_wait2resume_map;
	map<int,string> m_clientID2Account_map;
	whtimequeue::ID_T m_resume_map_tid;
	
private:
	// ����Ҽ���Waiting����(�������֮��Ӧ�����ϱ�����Waiting����)
	int		AddPlayerToWaitingList(PlayerUnit *pPlayer);
	// �����ת�Ƶ�Connecting����
	int		AddPlayerToConnectingList(PlayerUnit *pPlayer);
	// ������Ƴ���ǰ����
	int		DelPlayerFromList(PlayerUnit *pPlayer);
	// ���õ�ǰ״̬
	void	SetStatus(int nStatus);
	// �Ƴ�Player
	int		RemovePlayerUnit(int nPlayerID);
	int		RemovePlayerUnit(PlayerUnit *pPlayer);
	// ͨ��Slot���Player
	PlayerUnit *	GetPlayerBySlot(int nSlot);
	// ��������CLS������
	unsigned int	GenPasswordToConnectCLS();
	// ����Ҵ�Waitת�Ƶ�Connecting����
	int		TransPlayerFromWaitToConnectQueue(PlayerUnit *pPlayer);
	// Init�Ӻ���
	int		Init_CFG(const char *cszCFG);
	// ����TickWork
	int		Tick_DealGMSMsg();
	int		Tick_DealGMSMsg_GMS_CAAFS_CTRL(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CAAFS_GMSINFO(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CAAFS_CLIENTCANGOTOCLS(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CAAFS_KEEP_QUEUE_POS(void *pCmd,	int nSize);
	int		Tick_DealClientMsg();
	int		Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize);
	int		Tick_DealClientMsg_CONTROL_OUT_SLOT_CLOSED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize);
	int		Tick_DealQueue();
	int		Tick_DealTE();
	// ����TEDeal����
	// �ȴ��û�say hello�ĳ�ʱ
	void	TEDeal_HELLO_TimeOut(TQUNIT_T * pTQUnit);
	// �ȴ��û��ر����ӵĳ�ʱ
	void	TEDeal_Close_TimeOut(TQUNIT_T * pTQUnit);
	// ����ǰ����������ĳ�ʱ
	void	TEDeal_QueueSeq(TQUNIT_T * pTQUnit);
	// ����������¼�
	void	SetTE_QueueSeq(PlayerUnit *pPlayer, bool bRand=false);
	// ǰ��CLS�Ĺ����г�ʱ
	void	TEDeal_ClientGoCLSTimeOut(TQUNIT_T * pTQUnit);
	////////////////////////////////////////////////////////////////
	// �ִ�ָ���
	////////////////////////////////////////////////////////////////
	// ����ָ��ͺ�����Ӧ��ϵ
	int		InitStrCmdMap();
	// �����ִ�ָ�������m_szRstStr��
	int		DealStrCmd(const char *cszStrCmd);
	// ������ִ�ָ�����
	int		_i_srad_NULL(const char *param);
	int		_i_srad_setver(const char *param);

	// ������ʼʱ��Ҫ���õĺ���
	void	Retry_Worker_TRYCONNECT_Begin();
	// �������ɹ�ʱ��Ҫ���õĺ���
	void	Retry_Worker_WORKING_Begin();

	// ����ҷ���һ��vipchannel
	bool	PutPlayerInVIPChannel(PlayerUnit *pPlayer, unsigned char nVIPChannel);
	// ������ҵ�ǰ���Ŷ���Ϣ
	void	SendPlayerQueueInfo(PlayerUnit *pPlayer, bool bMustSend);

	// ��������ʺŵ��Ŷ�����
	void	DealKeepQueuePosReq(PlayerUnit	*pPlayer,const char * szAccount); 
	// �ָ�����ڶ������λ�ã�Ӧ�����Ŷӵ�ʱ�������
	bool    ResumePlayerPosInVIPChannel(PlayerUnit *pPlayer, unsigned char nVIPChannel);
	// ��ӡȫ��������Ϣ���������ԣ�
	void    DisplayQueueInfo();
	// �����Ŷӵ��ߵĳ�ʱ
	void	TEDeal_ResumeMapTimeOut(TQUNIT_T * pTQUnit);
	// �����Ŷӵ��ߵĳ�ʱ
	void	SetTE_ResumeMapTimeOut();
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CAAFS_I_H__
