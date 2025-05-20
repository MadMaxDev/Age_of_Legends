//////////////////////////////////////////////////////////////////////////
//web��չ�߼����������
//�ƶ���չҲ��Ҫʹ��
//��ʱӦ���൱��web��GMS,���Ҿ߱��򵥵��߼�������
//���Ը����е�CLS��CLS4Web����Ϣ,����ֻ�ܹ���CLS4Web,���ܹ���CLS
//��CLS����Ϣ�����ǵ����(CLS<-LP),��ʱ
//���������µ���Ϣͨ��,��Ȼ�кܶ�ط����ܲ���ͨ��У��
//!!!��ô���ֶ�������������崻�����ÿ�����Ӽ��ϸ��������룬���߶Բ��Ͼ�����һ����崻��ģ�ֱ���ߵ�����(��һ���޸�)
//��������ݲ�����:LPû��崻�,CLS4Webȫ������(��ÿ��CAAFS4Web��CLS4Web���ӵ�����),��LP���ֶ�����ʱ֮ǰ,CLS4Web���ܵò�������(�յ�HIȴ����ûλ����,��֪ͨCLS4Web�˳�)
//�Ժ�Ľ�:��ʵCLS4WebҲ�������,ÿ�����Ӵ�һ����������,��CLS4Web������(������ID)�������������ж��Ƿ�������,���ܱ����������
//////////////////////////////////////////////////////////////////////////

#ifndef __LPMainStructure4Web_i_H__
#define __LPMainStructure4Web_i_H__

#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_def.h"
#include "PNGS/inc/pngs_TCPReceiver_def.h"
#include "PNGS/inc/pngs_packet_mem_logic.h"
#include "PNGS/inc/pngs_packet_logic.h"
#include "PNGS/inc/pngs_packet.h"
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whunitallocator3.h"
#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnettcp.h"
#include "../../Common/inc/pngs_packet_web_extension.h"
#include "../../Common/inc/tty_common_def.h"
#include "../../Common/inc/pngs_packet_lp_logic.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class LPMainStructure4Web_i : public CMN::ILogic
{
// ILogic�ӿ�
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
	int				Tick_DealTE();
// �Լ��Ķ���
public:
	LPMainStructure4Web_i();
	~LPMainStructure4Web_i();
	int				Init_CFG(const char* cszCFGName);
	int				DealCmdIn_One_PNGS_TR2CD_CMD(const void *pData, int nDSize);
private:
	// ʱ������еĵ�Ԫ
	struct TQUNIT_T 
	{
		typedef	void	(LPMainStructure4Web_i::*TEDEAL_T)(TQUNIT_T*);
		TEDEAL_T		tefunc;
		// ���ݲ���
		union
		{
			struct SVR_T 
			{
				int	nID;
			}svr;
			void*	ptr;
		}un;
	};
public:
	struct	CLS4WebUnit;
	struct	PlayerUnit;
	struct	CAAFS4WebGroup
	{
		int						nCntrID;		// ��Ӧ������������ָ��,���Ϊ0��ʾCAAFS4Web��û�����ӽ���
		whDList<CLS4WebUnit*>	dlCLS4Web;		// ͳ���һƱCLS4Web�б�
	
		CAAFS4WebGroup()
		: nCntrID(0)
		{}

		void	clear()
		{
			nCntrID		= 0;
			dlCLS4Web.reset();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			nCntrID		= 0;
			dlCLS4Web.AdjustInnerPtr(nOffset);
			// ÿ����Ԫ��ָ�����ݾ���CLS4Webs���������,���Լ���������,���ԾͲ��õ�����
		}
	};
	struct	CLS4WebUnit 
	{
		enum
		{
			STATUS_NOTHING		= 0,
			STATUS_WORKING		= 1,					// ��������
		};
		enum
		{
			PROP_GOODEXIT		= 0x01,					// �����Լ��������˳���
		};
		int				nID;
		int				nStatus;						// ״̬
		int				nCntrID;						// ��Ӧ������ID
		int				nProp;							// ��������
		port_t			nPort;							// ��Client�Ķ˿�
		unsigned int	IP;								// ��Client��IP
		int				nGroupID;						// һ��CAAFS4Web������CLS4WebΪһ��
		int				nMaxPlayer;						// ��CLS4Web�����ɵ�����û�����(��LP����)
		whDList<CLS4WebUnit*>::node		dlnode;			// ��CAAFS4WebGroup�е��б��еĽڵ�
		whDList<PlayerUnit*>			dlPlayer;		// �������CLS4Web��Player�б�
		whtimequeue::ID_T				teid;			// ʱ���¼�
		whvector<int>*	pVectClientID;					// �����ͻ��˷�������ʱ�������CLS4Web�Ŀͻ���ID�б�(����ʱ��Ҫ���·���)
		whDList<CLS4WebUnit*>::node		dlnodeInSCTMP;	// �ڸ�SendCmdToMultiPlayer��ʹ�õ���ʱ�ڵ�
	
		CLS4WebUnit()
		: nID(0), nStatus(STATUS_NOTHING)
		, nCntrID(0), nProp(0)
		, nPort(0), IP(0)
		, nGroupID(-1), nMaxPlayer(0)
		{
			dlnode.data			= this;
			dlnodeInSCTMP.data	= this;
			pVectClientID		= new whvector<int>;
			pVectClientID->reserve(64);
		}
		~CLS4WebUnit()
		{
			WHSafeDelete(pVectClientID);
		}
		void	clear()
		{
			nID			= 0;
			nStatus		= STATUS_NOTHING;
			nCntrID		= 0;
			nProp		= 0;
			nPort		= 0;
			IP			= 0;
			nGroupID	= -1;
			nMaxPlayer	= 0;
			dlnode.leave();
			dlPlayer.clear();
			teid.quit();
			dlnodeInSCTMP.leave();
		}
		// ����Ѿ�ӵ�е��������
		inline int	GetCurPlayerNum() const
		{
			return dlPlayer.size();
		}
		// ��û����Խ��ɵ��������
		inline int	GetAcceptablePlayerNum() const
		{
			return nMaxPlayer - GetCurPlayerNum();
		}
		// �����Լ���ǰ��״̬�Ƿ�����������
		inline bool	CanAcceptPlayer() const
		{
			return nStatus == STATUS_WORKING;
		}
		void	AdjustInnerPtr(int nOffset)
		{
			nCntrID		= 0;
			teid.clear();
			// dlnode�Ѿ���������
			dlPlayer.AdjustInnerPtr(nOffset);
			whDList<PlayerUnit*>::node*	pNode	= dlPlayer.begin();
			for (; pNode!=dlPlayer.end(); pNode=pNode->next)
			{
				wh_adjustaddrbyoffset(pNode->data, nOffset);
			}
			dlnode.data		= this;
			pVectClientID	= new whvector<int>;
			pVectClientID->reserve(64);

			dlnodeInSCTMP.data	= this;
			dlnodeInSCTMP.reset();
		}
	};
	struct	PlayerUnit
	{
		enum
		{
			STATUS_NOTHING			= 0,
			STATUS_WANTCLS4Web		= 1,				// LP������CLS4Web�ύ������
			STATUS_GOINGTOCLS4Web	= 2,				// �������ȥ��CLS4Web��
			STATUS_GAMING			= 3,				// ��������ϷͨѶ��
		};
		int		nID;
		int		nStatus;
		int		nCLS4WebID;
		int		nClientIDInCAAFS4Web;
		unsigned int	IP;
		whDList<PlayerUnit*>::node	dlnodeInCLS4Web;
		int		nTermType;

		PlayerUnit()
		: nID(0), nStatus(STATUS_NOTHING)
		, nCLS4WebID(0), nClientIDInCAAFS4Web(0)
		, IP(0)
		, nTermType(TERM_TYPE_NONE)
		{
			dlnodeInCLS4Web.data	= this;
		}
		void	clear()
		{
			nID						= 0;
			nStatus					= STATUS_NOTHING;
			nCLS4WebID				= 0;
			nClientIDInCAAFS4Web	= 0;
			dlnodeInCLS4Web.leave();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			// Ŀǰû��ʲô�ɵ�����(dlnodeInCLS4Web��dlPlayer�е�������)
		}
		void	ReadFromOther(PlayerUnit* pOther)
		{
			nStatus			= pOther->nStatus;
			nCLS4WebID		= pOther->nCLS4WebID;
			IP				= pOther->IP;
			if (pOther->dlnodeInCLS4Web.isinlist())
			{
				dlnodeInCLS4Web.insertbefore(&pOther->dlnodeInCLS4Web);
			}
		}
	};
public:
	struct	CFGINFO_T : public whdataini::obj
	{
		// ��������
		int				nMaxPlayer;									// �����Գ��ص��������
		int				nSafeReservePlayerNum;						// Ϊ�˻�����������ͳ�Ʋ�׼������û��Ž����������û�����ʧ�ܵĻ��巧ֵ
		int				nMaxCLS4Web;								// �����Գ��ص�CLS4Web����
		int				nTQChunkSize;								// ʱ����е�ÿ�δ��Է�����С
		int				nQueueCmdInSize;							// ָ��������еĳ���
		int				nHelloTimeOut;								// �ȴ����ӷ���Hello�ĳ�ʱ
		int				nSvrDropWaitTimeOut;						// ����з���������,���Եȴ�һ��ʱ�����ɾ��,��Ϊ��������ʱ������ԭ��,���ǶԷ���ĵ���
		int				nCalcAndSendCAAFS4WebAvailPlayerTimeOut;	// ��ʱ�������CAAFS4Web��Ŀɽ��������ļ��
		int				anMaxCLS4WebNumInCAAFS4WebGroup[LP_MAX_CAAFS4Web_NUM];				// ����CAAFS4Web��������CLS4Web����
		int				anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup[LP_MAX_CAAFS4Web_NUM];		// ����CAAFS4Web����ÿ��CLS4Web�����������
		bool			bCLSUseCAAFSIP;								// Ϊtrue��ʱ��CLS��CAAFSʹ��ͬһ����ַ(Ϊ������������ͨ��)
		
		CFGINFO_T()
		: nMaxPlayer(256)
		, nSafeReservePlayerNum(128)
		, nMaxCLS4Web(0)
		, nTQChunkSize(100)
		, nQueueCmdInSize(2*1024*1024)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nSvrDropWaitTimeOut(20*1000)
		, nCalcAndSendCAAFS4WebAvailPlayerTimeOut(10*1000)
		, bCLSUseCAAFSIP(true)
		{
			memset(anMaxCLS4WebNumInCAAFS4WebGroup, 0, sizeof(anMaxCLS4WebNumInCAAFS4WebGroup));
			memset(anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup, 0, sizeof(anMaxPlayerNumInCLS4WebOfCAAFS4WebGroup));
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
private:
	typedef	CMN::ILogic			FATHERCLASS;
	// ���ýṹ
	CFGINFO_T		m_cfginfo;
	// �ײ�tcpͨѶ���
	ILogicBase*		m_pLogicTCPReceiver;
	// gameplayģ��
	ILogicBase*		m_pLogicGamePlay;
	whtick_t		m_tickNow;
	// ʱ���¼�������
	whtimequeue		m_TQ;
	// ������ʱ�ϳ�ָ��(���в��ö��������ָ��Ӧ�ö�����ͨ��������)
	// ����ʹ��������:Sendϵ����Ҫָ�����ݵĺ���,����������Ŀ����Player�ĺ���
	// Tick�п������������ɸ�CLS4Web��ָ��
	whvector<char>	m_vectrawbuf;
	// ��ʱͳ�Ƹ�CAAFS4Web��Ŀɽ�������
	whtimequeue::ID_T	m_teid_CalcAndSend_CAAFS4Web_AvailPlayerNum;
	whunitallocatorFixed<CLS4WebUnit>	m_CLS4Webs;
	whunitallocatorFixed<PlayerUnit>	m_Players;
	CAAFS4WebGroup	m_aCAAFS4WebGroup[LP_MAX_CAAFS4Web_NUM];
	// cntr id�����͵�ӳ��
	struct AppInfo_T 
	{
		int			nAppType;		// Ӧ������
		void*		pAppUnit;		// ��Ԫָ��,ָ��

		AppInfo_T()
		: nAppType(APPTYPE_NOTHING)
		, pAppUnit(NULL)
		{
		}
	};
	map<int, AppInfo_T>	m_mapCntrID2AppInfo;
private:
	// ͨ��GroupID�ҵ�CAAFS4WebGroup
	CAAFS4WebGroup*	GetCAAFS4WebByGroupID(int nGroupID);
	// ���������ӵķ������Զ��˳�(����LP�Ķ˿ھͲ���timewait��)
	void	TellAllConnecterToQuit();
	// ������CAAFS4Web�˳�
	void	TellAllCAAFS4WebQuit();
	// ������CLS4Web�˳�
	void	TellAllCLS4WebQuit();
	// ��CLS4Web�߳��û�(����д��Ϊ��ǿ���߳��û����Ǵ�CLS��ʼ��)
	void	TellCLS4WebToKickPlayer(int nClientID, pngs_cmd_t nSubCmd=SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID);
	//
	void	TellCLS4WebToCheckPlayerOffline(CLS4WebUnit* pCLS4Web);
	//
	int		RemovePlayerUnit(int nID, unsigned char nRemoveReason=CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING);
	int		RemovePlayerUnit(PlayerUnit* pPlayer, unsigned char nRemoveReason=CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTHING);
	// �����е�CLS4Web��һ����Ϣ
	int		SendCmdToAllCLS4Web(const void* pCmd, size_t nSize);
	// �����е�CLS����һ����Ϣ
	//int		SendCmdToAllCLS(const void* pCmd, size_t nSize);
	// ��Player���ڵ�CLS4Web����һ��ָ��(�ڲ�ֱ�Ӱ�ָ��ת��CLS4Web)
	int		SendCmdToPlayerCLS4Web(int nClientID, const void* pCmd, size_t nSize);
	int		SendCmdToPlayerCLS4Web(PlayerUnit* pPlayer, const void* pCmd, size_t nSize);
	// ��Player����ָ��(�ڲ����װ�ɷ���CLS4Web��ָ��)
	int		SendCmdToPlayer(int nPlayerID, const void* pCmd, size_t nSize);
	int		SendCmdToPlayer(PlayerUnit* pPlayer, const void* pCmd, size_t nSize);
	int		SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS4Web(int nCLS4WebID, int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS4Web(CLS4WebUnit* pCLS4Web, int *paPlayerID, int nPlayerNum, const void *pCmd, size_t nSize);
	int		SendCmdToAllPlayer(const void* pData, size_t nSize);
	int		SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, const void* pCmd, size_t nSize);
	int		SendCmdToAllPlayerByTag64(whuint64 nTag, const void* pCmd, size_t nSize);
	// ����player��tagֵ
	int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal);
	int		SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel);
	
	// Tick�еķ���
	// ʱ���¼�����
	// ��������������û�з���hello/hi�ĳ�ʱ
	void	TEDeal_Hello_TimeOut(TQUNIT_T* pTQUnit);
	// ����������¼�
	void	SetTE_Hello_TimeOut(int nCntrID);
	// CLS4Web���ߵȴ���ʱ
	void	TEDeal_CLS4Web_DropWait_TimeOut(TQUNIT_T* pTQUnit);
	// ����������¼�
	void	SetTE_CLS4Web_DropWait_TimeOut(CLS4WebUnit* pCLS4Web);
	// ���ڼ������CAAFS4Web���ɽ�����û������ļ��
	void	TEDeal_CalcAndSend_CAAFS4Web_AvailPlayerNum(TQUNIT_T* pTQUnit);
	// �����û�������ָ��(�ڲ�Ӧ����ת������Ӧ���߼�����Ԫ)
	void	InnerRouteClientGameCmd(CMN::cmd_t nCmd, CLS4Web_SVR_CLIENT_DATA_T* pCmd, size_t nSize);
	// ����CLS4Web������ʱ

	// ����CLS4Web���ߵ�һЩ����
	int		DealCLS4WebDisconnection(CLS4WebUnit* pCLS4Web);
	int		SendCmdToConnecter(int nCntrID, const void* pData, int nDSize);
	int		RemoveConnecter(int nCntrID);

	// ����ָ���ȡCAAFS4Web�����
	inline	int		GetCAAFS4WebGroupID(CAAFS4WebGroup* pCAAFS)
	{
		return wh_getidxinarray(m_aCAAFS4WebGroup, pCAAFS);
	}
};
}

#endif

