//////////////////////////////////////////////////////////////////////////
//web��չ���Ӹ��ط�����
//�ƶ���չҲ��Ҫʹ��
//��Ӧ��CAAFS��ָCAAFS4Web
//����ӳٷ��ͻ���(2012-01-06),ʵ���������epoll_server�ĳ�connecterͳһʹ��һ��buf������(����connecter���Ӹ�buf���뻺��),�Ͳ���Ҫ����ӳٷ��ͻ�����
//////////////////////////////////////////////////////////////////////////
#ifndef __CLS4Web_i_H__
#define __CLS4Web_i_H__

#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_packet_logic.h"

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whtimequeue.h"

#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnetepoll.h"
#include "WHNET/inc/whnettcp.h"

#include "../../Common/inc/pngs_packet_web_extension.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class CLS4Web_i : public CMN::ILogic
{
// ILogic�ӿ�
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char* cszCFGName);
	virtual	int		Init_CFG_Reload(const char* cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(whvector<SOCKET>& vect);
	virtual	int		SureSend();
private:
	virtual	int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		Tick_BeforeDealCmdIn();
	virtual	int		Tick_AfterDealCmdIn();
// �Լ��Ķ���
public:
	enum
	{
		STATUS_ERR_NETWORK			= -1,			// �������������
		STATUS_NOTHING				= 0,
		STATUS_WORKING				= 1,			// ���ӳɹ�,������
		STATUS_KICK_ALL_PLAYER		= 2,			// ����ӦLP4Web��Ҫ�����ʺ�������
	};
public:
	CLS4Web_i();
	virtual	~CLS4Web_i();
public:
	// ��ʼ������
	struct CFGINFO_T : public whdataini::obj 
	{
		char			szWelcomeAddr[WHNET_MAXADDRSTRLEN];				// ������ַ
		char			szLogicProxyAddr[WHNET_MAXADDRSTRLEN];			// gms�����ӵ�ַ
		int				nGroupID;										// ����Ӧ��	CAAFS��Ӧ
		int				nTQChunkSize;									// ʱ�����ÿ�η�����С
		int				nConnectOtherSvrTimeOut;						// ���������������ĳ�ʱ
		int				nCloseFromLPTimeOut;							// ֪ͨLP�Լ�Ҫ�˳���,�ȴ�LP�ر����ӵĳ�ʱ
		int				nHelloTimeOut;									// �ȴ����ӷ�����hello�ĳ�ʱ
		int				nClientConnectToMeTimeOut;						// �û�����CLS�ĳ�ʱ(��ô��û���ӳɹ��ͳ�ʱ)
		int				nCntrSendRecvBuf;								// ÿ�����ӵ�send��recvbuf�Ĵ�С
		int				nCmdStatInterval;								// ����ָ��ͳ�Ƶļ��(<=0��ʾ��ͳ��)
		char			szSelfInfoToClient[256];						// �û����ӻ��ȿ�������һ�仰,�������ȽϿ������Ƿ���ȷ
		int				nMaxRecvCount;									// һ��ʱ�������ɽ��յİ���
		int				nMaxRecvSize;									// һ��ʱ�������ɽ��յ�������
		int				nCheckRecvInterval;								// ��������������ļ��
		int				nBigTickFilter;									// big tick log����ķ�ֵ
	
		CFGINFO_T()
		: nGroupID(0)
		, nTQChunkSize(1024)
		, nConnectOtherSvrTimeOut(PNGS_DFT_SVRCONNECT_TIMEOUT)
		, nCloseFromLPTimeOut(10*1000)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nClientConnectToMeTimeOut(30*1000)
		, nCntrSendRecvBuf(64*1024)
		, nCmdStatInterval(20*1000)
		, nMaxRecvCount(5*60*3)											// ƽ��ÿ��3����
		, nMaxRecvSize(5*60*4*1024)										// ƽ��ÿ��4k������
		, nCheckRecvInterval(5*60*1000)									// Ĭ�������ͳ��һ��
		, nBigTickFilter(150)
		{
			strcpy(szWelcomeAddr, "localhost:15101,15110");
			strcpy(szLogicProxyAddr, "localhost:4400");
			strcpy(szSelfInfoToClient, "CLS24Web");
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// ���
	struct	PlayerUnit;
	typedef	whDList<PlayerUnit*>		DLPLAYER_T;
	typedef	DLPLAYER_T::node			DLPLAYER_NODE_T;
	struct	PlayerUnit 
	{
		enum
		{
			MAX_TAG_NUM					= 16,							// �����Ա�ǵ�tag����
		};
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_CONNECTING			= 1,							// �������ӹ�����
			STATUS_WORKING				= 2,							// ���ӳɹ�,����ͨѶ��
		};
		int						nID;									// ��Players�����е�ID(�������LP�е�ID������)
		char					szName[64];								// ���ֱ�־(������ϲ�����,����:�ʺ�.��ɫ,��Ҫ��Ϊ�˹ؼ�����־�ܹ���ӳ���û���,���ڽ�����ѯ)
		int						nStatus;								// ��ǰ״̬
		int						nCntrID;								// ��Ӧ������ID
		unsigned int			nPasswordToConnectCLS4Web;				// ����CLS�õ�����
		unsigned char			nRemoveReason;							// CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_XXX
		unsigned char			nProp;									// ����,����ΪPNGS_CLS_PLAYER_PROP_LOG_XXX,��pngs_def.h��
		unsigned char			nSvrIdx;								// ��Ӧ��GZS���(�����������������ʵGZS�е��û�����ʱʹ��,�ݲ�ʹ��)
		int						nRecvCount;								// ��һ��ʱ�����յ��İ�������
		int						nRecvSize;								// ��һ��ʱ�����յ���������
		whtimequeue::ID_T		teid;									// Ŀǰ��;:�û����볬ʱ,ÿ��һ��ʱ�����Ƿ��͹���İ�

		int						nTermType;								// �ն�����

		struct TAGINFO_T 
		{
			short				nValue;									// ��Ӧ��tagֵ(0��ʾ�����κζ�����)
			DLPLAYER_NODE_T		nodeTag;								// ����Ӧ��tag�����еĽڵ�

			TAGINFO_T()
			: nValue(0)
			{
			}

			void clear()
			{
				nValue	= 0;
				nodeTag.leave();
			}
		};
		TAGINFO_T				taginfo[MAX_TAG_NUM];
		whhash<whuint64, DLPLAYER_NODE_T*>	map64Tag2Node;

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nCntrID(0)
		, nPasswordToConnectCLS4Web(0)
		, nRemoveReason(CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING)
		, nProp(0)
		, nSvrIdx(0)
		, nTermType(TERM_TYPE_NONE)
		{
			szName[0]			= 0;
			for (int i=0; i<MAX_TAG_NUM; i++)
			{
				taginfo[i].nodeTag.data		= this;
			}
		}

		void	clear(CLS4Web_i* pCLS)
		{
			for (int i=0; i<MAX_TAG_NUM; i++)
			{
				pCLS->DelPlayerFromTagList(this, i);
			}
			whhash<whuint64, DLPLAYER_NODE_T*>::kv_iterator	it	= map64Tag2Node.begin();
			while (it != map64Tag2Node.end())
			{
				whuint64	nTag	= it.getkey();
				++it;
				pCLS->DelPlayerFromTag64List(this, nTag);
			}

			teid.quit();

			nID					= 0;
			nSvrIdx				= 0;
			nStatus				= STATUS_NOTHING;
			nCntrID				= 0;
			nPasswordToConnectCLS4Web	= 0;
			nRemoveReason				= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING;
			nProp				= 0;
			szName[0]			= 0;
			nRecvCount			= 0;
			nRecvSize			= 0;
		}
		void	ReadFromOther(PlayerUnit* pOther)
		{
			//nID����Ҫ
			memcpy(szName, pOther->szName, sizeof(szName));
			nStatus				= pOther->nStatus;
			nCntrID				= pOther->nCntrID;
			nPasswordToConnectCLS4Web	= pOther->nPasswordToConnectCLS4Web;
			nRemoveReason		= pOther->nRemoveReason;
			nProp				= pOther->nProp;
			nSvrIdx				= pOther->nSvrIdx;
			teid.quit();		// ������ö�ȡ���˵�
			for (int i=0; i<MAX_TAG_NUM; i++)
			{
				if (pOther->taginfo[i].nodeTag.isinlist())
				{
					taginfo[i].nodeTag.insertbefore(&pOther->taginfo[i].nodeTag);
				}
			}
		}
	};
	struct	TQUNIT_T;
	typedef	void	(CLS4Web_i::*TEDEAL_T)(TQUNIT_T*);
	struct	TQUNIT_T 
	{
		TEDEAL_T	tefunc;
		// ���ݲ���
		union
		{
			struct PLAYER_T						// ��ҵĶ�ʱ�¼�
			{
				int	nID;						// ���ID
			}player;
			struct CNTR_T						// ����ID
			{
				int	nID;
			}cntr;
		}un;
	};
	struct	DLPUNIT_T 
	{
		DLPLAYER_T*	pDLP;

		DLPUNIT_T()
		: pDLP(NULL)
		{
		}

		void	clear(CLS4Web_i* pCLS)
		{
			pCLS->SafeDeleteDLP(pDLP);
		}
	};
private:
	friend	struct	PlayerUnit;
	friend	struct	DLPUNIT_T;
	// ����
	CFGINFO_T							m_cfginfo;						// CLS4Web������
	tcpmsger::DATA_INI_INFO_T			m_MSGER_INFO;					// ��LP���ӵ�msger����
	epoll_server::info_T				m_epollServerInfo;
	// ��ǰ�Ĺ���״̬
	int									m_nStatus;
	// ��ǰ��ʱ��
	whtick_t							m_tickNow;
	// �Լ���LP�е�ID(��������ʱ������)
	int									m_nCLS4WebID;
	// epoll���ӹ���
	epoll_server*						m_pEpollServer;
	// ʱ�����
	whtimequeue							m_TQ;
	// �������
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// ����LP��msger
	class	MYMSGER_T : public tcpretrymsger<tcpmsger>
	{
	public:
		CLS4Web_i*		m_pHost;
		whtick_t		m_tickWorkBegin;
	public:
		virtual	void	i_Worker_TRYCONNECT_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
		}
		virtual	void	i_Worker_WORKING_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	friend	class	MYMSGER_T;
	MYMSGER_T							m_msgerLP;
	// ����ɾ�������player
	whvector<int>						m_vectIDToDestroy;
	// tag����Ӧ��DList��ӳ��
	whhash<unsigned int, DLPUNIT_T>		m_mapTag2DL;
	// 64λ��ӳ��
	whhash<whuint64, DLPUNIT_T>			m_map64Tag2DL;
	// ��ʱ��װָ��
	whvector<char>						m_vectrawbuf;
	// �����ϲ�Ӧ���˳�
	bool								m_bShouldStop;
	// ����ͳ��ָ���Ƶ��
	struct CMDSTAT_T 
	{
		unsigned int		nByte;		// �ۼ��ֽ���
		unsigned int		nCount;		// �ۼ�����

		inline	bool	operator < (const CMDSTAT_T& other) const
		{
			if (nByte != other.nByte)
			{
				return nByte > other.nByte;
			}
			return nCount > other.nCount;
		}
	};
	whvector<CMDSTAT_T>					m_vectCmdStat;
	// ���ڶ�ʱ������log
	whlooper							m_loopCmdStat;
private:
	bool	ShouldStop() const;
	// ���õ�ǰ��״̬
	void	SetStatus(int nStatus);
	// ��õ�ǰ״̬
	int		GetStatus() const;
	// �Ƴ�player
	int		RemovePlayerUnit(int nPlayerID);
	int		RemovePlayerUnit(PlayerUnit* pPlayer);
	// ͨ��cntrID���Player
	PlayerUnit*		GetPlayerByCntrID(int nCntrID);
	// ���Ի�ȡ���,��������ھ���LP�����û����ߵ�֪ͨ
	PlayerUnit*		TryGetPlayerAndSendNotifyToLPIsNotExisted(int nPlayerID);
	// �򵥸���ҷ�������(������Ҫ�����ݺϳ�CLS4Web_CLIENT_DATA_T����ʽ,Ȼ���ٵ���SendRawCmdToSinglePlayer)
	int		SendDataToSinglePlayer(PlayerUnit* pPlayer, void* pData, int nSize, pngs_cmd_t nSendCmd);
	// �򵥸���ҷ��ʹ�ָ��
	int		SendRawCmdToSinglePlayer(int nPlayerID, void* pRawCmd, int nSize);
	int		SendRawCmdToSinglePlayer(PlayerUnit* pPlayer, void* pRawCmd, int nSize);
	// ��������ҷ��ʹ�ָ��
	int		SendRawCmdToAllPlayer(void* pRawCmd, int nSize);
	// ����idx��val�ϳ�һ��hash��ֵ
	inline	unsigned int		CreateHashKey(unsigned char nTagIdx, short nTagVal)
	{
		return (((unsigned int)nTagIdx)<<16) | nTagVal;
	}
	// ��hash��ֵ�еõ�idx��val
	inline	void	GetIdxAndVal(unsigned int nHashKey, unsigned char* pnTagIdx, short* pnTagVal)
	{
		*pnTagIdx		= (unsigned char)((nHashKey & 0xFF0000)>>16);
		*pnTagVal		= (short)(nHashKey&0xFFFF);
	}
	// ����Ҽ���ĳ��tag������
	int		AddPlayerToTagList(PlayerUnit* pPlayer, unsigned char nTagIdx, short nTagVal);
	// ����Ҵ�ĳ��tag�������Ƴ���
	int		DelPlayerFromTagList(PlayerUnit* pPlayer, unsigned char nTagIdx);
	// ����Ҽ���ĳ��tag64������
	int		AddPlayerToTag64List(PlayerUnit* pPlayer, whuint64 nTag);
	// ����Ҵ�ĳ��tag64�������Ƴ���
	int		DelPlayerFromTag64List(PlayerUnit* pPlayer, whuint64 nTag);

	// Newһ��DLPLAYER_T����
	DLPLAYER_T*		NewDLP();
	// Deleteһ��DLPLAYER_T����
	void	SafeDeleteDLP(DLPLAYER_T*& pDLP);
	// Init�Ӻ���
	int		Init_CFG(const char* cszCFG);
	// ����tickwork
	int		Tick_DealLogicProxyMsg();
	int		Tick_DealLogicProxyMsg_LP_CLS4Web_CLIENTWANTCLS4Web(const char* pCmd, size_t nSize);
	
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_CTRL(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_CLIENT_DATA(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_MULTICLIENT_DATA(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_SET_TAG_TO_CLIENT(void* pCmd, int nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_TAGGED_CLIENT_DATA(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_ALL_CLIENT_DATA(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_SET_TAG64_TO_CLIENT(void* pCmd, size_t nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS4Web_TAGGED64_CLIENT_DATA(void* pCmd, size_t nSize);
	
	int		Tick_DealClientMsg();
	int		Tick_DealClientMsg_CONTROL_OUT_CNTR_ACCEPTED(epoll_server::ctrl_out_T* pCOCmd, int nSize);
	int		Tick_DealClientMsg_CONTROL_OUT_CNTR_CLOSED(epoll_server::ctrl_out_T* pCOCmd, int nSize);
	int		Tick_DealClientMsg_CLIENT_DATA(pngs_cmd_t nCmd, PlayerUnit* pPlayer, pngs_cmd_t* pCmd, int nSize);
	int		Tick_DealTE();
	int		Tick_DoMyOwnWork();
	// �������ݽ���

	// �������ɹ�ʱ��Ҫ���õĺ���
	void	Retry_Worker_WORKING_Begin();

	// ʱ���¼�����
	// �ͻ������ӳ�ʱ
	void	TEDeal_ClientConnnectToMeTimeOut(TQUNIT_T* pTQUnit);
	// ���������
	void	SetTEDeal_ClientConnectToMeTimeOut(PlayerUnit* pPlayer);
	// ��ʱ����û������İ���Ƶ��
	void	TEDeal_CheckRecvInterval(TQUNIT_T* pTQUnit);
	// ���������
	void	SetTEDeal_CheckRecvInterval(PlayerUnit* pPlayer);
};
}

#endif
