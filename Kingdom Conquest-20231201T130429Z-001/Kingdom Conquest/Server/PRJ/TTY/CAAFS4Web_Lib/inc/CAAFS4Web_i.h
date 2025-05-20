//////////////////////////////////////////////////////////////////////////
// web��չ���ӷ��������
// �ƶ���չҲ��Ҫʹ��
// ��Ӧ��CLS��ָCLS4Web
// 1.prelogin��ʱ����
//////////////////////////////////////////////////////////////////////////
#ifndef __CAAFS4Web_i_H__
#define __CAAFS4Web_i_H__

#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_packet_logic.h"

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whgener.h"
#include "WHCMN/inc/whtimequeue.h"

#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnetepoll.h"
#include "WHNET/inc/whnettcp.h"

#include "../../Common/inc/pngs_packet_web_extension.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class CAAFS4Web_i : public CMN::ILogic
{
//ILogic�ӿ�
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		SureSend();
private:
	virtual	int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		Tick_BeforeDealCmdIn();
	virtual	int		Tick_AfterDealCmdIn();
//�Լ��Ķ���
public:
	enum
	{
		STATUS_ERR_NETWORK		= -1,			// �������������
		STATUS_NOTHING			= 0,
		STATUS_WORKING			= 1,			// ���ӳɹ�,������
	};
public:
	CAAFS4Web_i();
	virtual	~CAAFS4Web_i();
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		// ��������
		int				nVerCmpMode;							// ȡֵΪPNGS_VERCMP_MODE_XXX,��pngs_cmn.h
		char			szVer[PNGS_VER_LEN];					// �汾
		char			szWelcomeAddr[WHNET_MAXADDRSTRLEN];		// �û������ķ�������ַ
		char			szLogicProxyAddr[WHNET_MAXADDRSTRLEN];	// �����������ַ
		int				nConnectLogicProxyTimeout;				// �����߼�����������ĳ�ʱ
		int				nSelectInterval;						// tick����select�ļ��(ms)
		int				nGroupID;								// ��ID,��CLS��Ӧ
		int				nTQChunkSize;							// ʱ�����ÿ���Է�����С
		int				nMaxPlayerRecvCmdNum;					// ÿ����ҿ��Է���������ָ������
		// ������ز���
		int				nClientWaitingNumMax;					// ���ڵȴ�״̬����ҵ��������
		int				nClientConnectingNumMax;				// �������ӹ����е���ҵ��������
		int				nHelloCheckTimeout;						// �������Ƿ���hello�ĳ�ʱ(ms)
		int				nCloseCheckTimeout;						// �����ҹرպ�CAAFS���ӵĳ�ʱ(ms)
		int				nQueueSeqSendInterval;					// ����ҷ���ǰ�滹�ж����˵ļ��(ms)
		float			fWaitTransToConnectRate;				// �ӵȴ�ת��Ϊ���ӵ�����(��/s)
		bool			bAtLeastOnTransToConnectPerTick;		// ÿ��tick����ת��һ���û���������
		int				nQueueChannelNum;						// ��������
		// ���÷����
		char			szSelfInfoToClient[256];				// �û����ӻ��ȿ�������һ�仰,�������ȽϿ������Ƿ���ȷ
		unsigned int	nSelfNotify;							// 32bit��ʾ32����˼
		int				_nSelfInfoToClientLen;					// szSelfInfoToClient�ִ��ĳ���
	
		CFGINFO_T()
		: nVerCmpMode(PNGS_VERCMP_MODE_EQUAL)
		, nConnectLogicProxyTimeout(PNGS_DFT_SVRCONNECT_TIMEOUT)
		, nSelectInterval(50)
		, nGroupID(0)
		, nTQChunkSize(100)
		, nMaxPlayerRecvCmdNum(4)
		, nClientWaitingNumMax(2000)
		, nClientConnectingNumMax(256)
		, nHelloCheckTimeout(2000)
		, nCloseCheckTimeout(1000)
		, nQueueSeqSendInterval(5000)
		, fWaitTransToConnectRate(32.f)
		, bAtLeastOnTransToConnectPerTick(false)
		, nQueueChannelNum(9)
		, nSelfNotify(0)
		{
			strcpy(szVer, "");
			strcpy(szWelcomeAddr, "localhost:4200");
			strcpy(szLogicProxyAddr, "localhost:4400");
			strcpy(szSelfInfoToClient, "caafs4web");
			_nSelfInfoToClientLen	= strlen(szSelfInfoToClient)+1;
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// ���
	struct PlayerUnit 
	{
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_WAITING				= 1,				// �Ŷӵȴ����ӵĻ���
			STATUS_CONNECTINGCLS4Web		= 2,				// CAAFS������CLS����������,Client���ں�CLS����������
		};
		int					nID;							// ��Players�����е�ID
		int					nStatus;						// ��ǰ״̬
		int					nCntrID;						// ��Ӧ��ͨѶconnecter��
		unsigned int		IP;								// �û���IP��ַ
		unsigned short		nPort;							// �û��Ķ˿�
		unsigned short		nSeq0;							// �ϴ�֪ͨʱ���Seq0���
		unsigned short		nSeq;							// ����Ŷӵ����
		unsigned int		nPasswordToConnectCLS4Web;			// ����CLS�õ�����
		whDList<PlayerUnit*>::node	nodeWC;					// ��Ӧwaiting��connecting����
		whtimequeue::ID_T	teid;
		unsigned char		nRecvCmdNum;					// Ŀǰ�յ���ָ������(���ܳ���ָ��ֵ)
		unsigned char		nChannel;						// �Լ�������channel
		int					nTermType;						// ʹ�õ��ն�����

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nCntrID(0)
		, IP(0)
		, nSeq0(0)
		, nSeq(0)
		, nPasswordToConnectCLS4Web(0)
		, nRecvCmdNum(0)
		, nTermType(TERM_TYPE_NONE)
		{
			nodeWC.data		= this;
		}

		void	clear()
		{
			nID				= 0;
			nStatus			= STATUS_NOTHING;
			nCntrID			= 0;
			IP				= 0;
			nSeq0			= 0;
			nSeq			= 0;
			nPasswordToConnectCLS4Web		= 0;
			nodeWC.leave();
			teid.quit();
			nRecvCmdNum		= 0;
		}
	};
	struct TQUNIT_T 
	{
		typedef	void (CAAFS4Web_i::*TEDeal_T)(TQUNIT_T*);
		TEDeal_T	tefunc;										// �¼�������
		// ���ݲ���
		union
		{
			struct PLAYER_T										// ��Ҷ�ʱ�¼�
			{
				int		nID;
			}player;
		}un;
	};
private:
	// ���ýṹ
	CFGINFO_T								m_cfginfo;
	tcpmsger::DATA_INI_INFO_T				m_MsgerLogicProxy_info;
	epoll_server::info_T					m_epollServerInfo;
	int										m_nStatus;
	whtick_t								m_tickNow;
	unsigned int							m_nSelfNotify;
	epoll_server*							m_pEpollServer;
	whtimequeue								m_TQ;
	whunitallocatorFixed<PlayerUnit>		m_Players;
	// �Ŷ��е�����б�
	struct WAITCHANNEL_T 
	{
		whDList<PlayerUnit*>*				pdlWait;
		unsigned short						nWaitingSeq0;
		unsigned short						nWaitingSeqNew;

		void	Init()
		{
			WHMEMSET0THIS();
			pdlWait		= new whDList<PlayerUnit*>;
		}
		void	Release()
		{
			WHSafeDelete(pdlWait);
		}
	};
	// ����Ƿ�Ϊcppû�������,��ΪWAITCHANNEL_Tû���Զ��幹�����������
	whvector<WAITCHANNEL_T, true>			m_vectWaitChannel;
	whDList<PlayerUnit*>					m_dlPlayerConnecting;
	int										m_nConnectingNum;
	class MYMSGER_T : public tcpretrymsger<tcpmsger>
	{
	public:
		CAAFS4Web_i*		m_pHost;
		whtick_t			m_tickWorkBegin;
	public:
		virtual	void	i_Worker_TRYCONNECT_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_TRYCONNECT_Begin();
		}
		virtual	void	i_Worker_WORKING_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	friend class	MYMSGER_T;
	MYMSGER_T								m_msgerLogicProxy;
	int										m_nLogicProxyAvailPlayerNum;
	whgener									m_generWaitTransToConnect;
	whinterval								m_itv;
	bool									m_bShouldStop;
	bool									m_bIsReconnect;
	whvector<char>							m_vectrawbuf;
private:
	// ���õ�ǰ״̬
	void	SetStatus(int nStatus);
	// ��õ�ǰ״̬
	int		GetStatus() const;
	// �Ƴ�player
	int		RemovePlayerUnit(int nPlayerID);
	int		RemovePlayerUnit(PlayerUnit* pPlayer);
	// ͨ��cntrid���Player
	PlayerUnit*		GetPlayerByCntrID(int iCntrID);
	// ��������CLS������
	unsigned int	GenPasswordToConnectCLS4Web();
	// ����Ҵ�waitת�Ƶ�connecting����
	int		TransPlayerFromWaitToConnectQueue(PlayerUnit* pPlayer);
	// Init�Ӻ���
	int		Init_CFG(const char* cszCFG);
	// ����tickwork
	int		Tick_DealLogicProxyMsg();
	int		Tick_DealLogicProxyMsg_LP_CAAFS4Web_CTRL(void* pCmd, int nSize);
	int		Tick_DealLogicProxyMsg_LP_CAAFS4Web_LPINFO(void* pCmd, int nSize);
	int		Tick_DealLogicProxyMsg_LP_CAAFS4Web_CLIENTCANGOTOCLS4Web(void* pCmd, int nSize);
	int		Tick_DealClientMsg();
	int		Tick_DealClientMsg_CONTROL_OUT_CNTR_ACCEPTED(epoll_server::ctrl_out_T* pCOCmd, int nSize);
	int		Tick_DealClientMsg_CONTROL_OUT_CNTR_CLOSED(epoll_server::ctrl_out_T* pCOCmd, int nSize);
	int		TIck_DealClientMsg_CLIENT_CLS4Web_DATA(PlayerUnit* pPlayer, pngs_cmd_t nCmd, pngs_cmd_t* pCmd, int nSize);
	int		Tick_DealQueue();
	int		Tick_DealTE();
	int		Tick_DoMyOwnWork();				// ����tick�����Ʒ,Tick����virtual����
	// ����TEDeal����
	// �ȴ��û�say hello�ĳ�ʱ
	void	TEDeal_HELLO_TimeOut(TQUNIT_T* pTQUnit);
	// �ȴ��û��ر����ӵĳ�ʱ
	void	TEDeal_Close_TimeOut(TQUNIT_T* pTQUnit);
	// ����ǰ����������ĳ�ʱ
	void	TEDeal_QueueSeq(TQUNIT_T* pTQUnit);
	// ����������¼�
	void	SetTE_QueueSeq(PlayerUnit* pPlayer, bool bRand=false);
	// ǰ��CLS�����еĳ�ʱ
	void	TEDeal_ClientGoCLSTimeOut(TQUNIT_T* pTQUnit);
	// ���¿�ʼʱ��Ҫ���õĺ���
	void	Retry_Worker_TRYCONNECT_Begin();
	// �������ɹ�ʱ��Ҫ���õĺ���
	void	Retry_Worker_WORKING_Begin();
	// ������ҵ�ǰ���Ŷ���Ϣ
	void	SendPlayerQueueInfo(PlayerUnit* pPlayer, bool bMustSend);
	// ����ҷ���һ��channel
	bool	PutPlayerInChannel(PlayerUnit* pPlayer, unsigned char nChannel);
};
}

#endif
