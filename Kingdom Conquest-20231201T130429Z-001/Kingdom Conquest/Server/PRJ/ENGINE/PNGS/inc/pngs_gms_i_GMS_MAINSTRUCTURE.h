// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTURE���Logicģ���ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-31
// Change LOG   : 2005-09-27 SvrIdx��GZS����Ķ�Ӧ��ʽ��ȫ��m_GZSs���ڲ������±���ʵ�֣�ʡȥ��ԭ������ֵ��Ӧ
//              : 2005-09-29 GoPlayerToSvr���ǲ��ô��ݸ��Ӳ����ˣ���Ϊ����һ����Դ����DB������GZS����ֱ�Ӵ�DB��ȡ���ݵĺ�
//              : 2005-09-30 ȡ����ת�����̡��û����߾ʹ���������GZS�С�ȡ����PlayerUnit�е�nGZSID��

#ifndef	__PNGS_GMS_I_GMS_MAINSTRUCTURE_H__
#define	__PNGS_GMS_I_GMS_MAINSTRUCTURE_H__

#include "./pngs_packet.h"
#include "./pngs_gms.h"
#include "./pngs_packet_logic.h"
#include "./pngs_packet_gms_logic.h"
#include "./pngs_packet_mem_logic.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whconnecter.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whgener.h>
#include <WHCMN/inc/whunitallocator3.h>
#include <WHCMN/inc/whcmd.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

////////////////////////////////////////////////////////////////////
// GMS_MAINSTRUCTURE
////////////////////////////////////////////////////////////////////
class	GMS_MAINSTRUCTURE	: public CMN::ILogic
{
// Ϊ�ӿ�ʵ�ֵ�
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	GMS_MAINSTRUCTURE();
	~GMS_MAINSTRUCTURE();
	virtual	void	SelfDestroy();
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		SureSend();
private:
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		Tick_BeforeDealCmdIn();
	virtual int		Tick_AfterDealCmdIn();
// �Լ��õ�
public:
	class	MYCNTRSVR	: public	TCPConnecterServer
	{
	public:
		GMS_MAINSTRUCTURE	*m_pHost;
	public:
		class	MYCNTR	: public	TCPConnecterMsger<tcpmsger>
		{
		public:
			GMS_MAINSTRUCTURE	*m_pHost;
			struct sockaddr_in	m_addr;								// ��Դ��ַ
			APPTYPE				m_nSvrType;							// ���������ʲô���͵�Server
																	// ȡֵΪpngs_def.h�е�APPTYPE_XXX
			void				*m_pExtPtr;							// ����ָ�루������ӹ����Ķ���ָ�룩
			whtimequeue::ID_T	teid;
			typedef	int		(GMS_MAINSTRUCTURE::*CNTR_DEALMSG_T)(MYCNTR *pCntr, const pngs_cmd_t *pData, size_t nSize);
			CNTR_DEALMSG_T	m_p_CNTR_DEALMSG;
		public:
			MYCNTR(GMS_MAINSTRUCTURE *pHost)
			: m_pHost(pHost)
			, m_nSvrType(APPTYPE_NOTHING)
			, m_pExtPtr(NULL)
			, m_p_CNTR_DEALMSG(&GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft)
			{
			}
			void	clear()
			{
				m_pHost		= NULL;
				m_nSvrType	= APPTYPE_NOTHING;
				m_pExtPtr	= NULL;
				teid.quit();
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
				return	(m_pHost->*m_p_CNTR_DEALMSG)(this, (const pngs_cmd_t *)pData, nSize);
			}
		};
	private:
		// ΪTCPConnecterServerʵ�ֵ�
		virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
		{
			return	m_pHost->NewConnecter(sock, paddr);
		}
		virtual void		BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo)
		{
			m_pHost->BeforeDeleteConnecter(nCntrID, pCntrInfo);
		}
		virtual void	AfterAddConnecter(Connecter * pCntr)
		{
			m_pHost->AfterAddConnecter(pCntr);
		}
	public:
		MYCNTRSVR()
		: m_pHost(NULL)
		{
		}
	};
	struct	CLSUnit;
	struct	PlayerUnit;
	struct	CAAFSGroup
	{
		MYCNTRSVR::MYCNTR	*pCntr;									// ��Ӧ��CAAFS����������ָ�룬���ΪNULL���ʾ��Ӧ��CAAFS��û�����ӽ���
		whDList<CLSUnit *>	dlCLS;									// ��ͳ���CLS���б�

		CAAFSGroup()
		: pCntr(NULL)
		{
		}

		void	clear()
		{
			pCntr	= NULL;
			dlCLS.reset();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			pCntr		= NULL;
			dlCLS.AdjustInnerPtr(nOffset);
			// ÿ����Ԫ��ָ�����ݾ���CLSs��������ݣ����Լ��������ˣ����ԾͲ��õ�����
		}
	};
	struct	CLSUnit
	{
		enum
		{
			STATUS_NOTHING			= 0,
			STATUS_CONNECTINGGZS	= 1,							// �����µ�GZS��
			STATUS_WORKING			= 2,							// ��������״̬
		};
		enum
		{
			PROP_GOODEXIT	= 0x01,									// �����Լ��������˳���
		};
		int		nID;
		int		nStatus;											// ״̬
		MYCNTRSVR::MYCNTR	*pCntr;									// ��Ӧ������������ָ��
		unsigned char		nProp;									// ��������
		unsigned char		nNetworkType;							// ��Ӧ���������ͣ�NETWORKTYPE_XXX��
		port_t				nPort;									// ��Client�Ķ˿�	
		unsigned int		IP;										// ��Client��IP
		int		nGroupID;											// ���һ��CAAFS������CLS��һ��
		int		nMaxPlayer;											// ��CLS�������ɵ�����û�����(��GMS����)
		int		nGZSNumToConnect;									// ����Ҫ������ô��GZS�Ϳ�����ʽ������
		bool	bCanAcceptPlayerWhenCLSConnectingGZS;				// ��CLS����GZS�Ĺ����������û�����
		whDList<CLSUnit *>::node	dlnode;							// ��CAAFSGroup�е��б��еĽڵ�
		whDList<PlayerUnit *>		dlPlayer;						// �������CLS��Player�б�
		whtimequeue::ID_T	teid;									// ����ĳЩ��GZS��ص��¼�
		whvector<int>		*pvectClientID;							// �������ͻ����������ǹ������CLS�Ŀͻ�ID�б�����ʱ��Ҫ���·���һ�£�
		whDList<CLSUnit *>::node	dlnodeInSCTMP;					// �ڸ�SendCmdToMultiPlayer��ʹ�õ���ʱ�ڵ�

		CLSUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, pCntr(NULL)
		, nProp(0)
		, nNetworkType(NETWORKTYPE_UNKNOWN)
		, nPort(0)
		, IP(0)
		, nGroupID(-1)
		, nMaxPlayer(0)
		, nGZSNumToConnect(0)
		, bCanAcceptPlayerWhenCLSConnectingGZS(false)
		{
			dlnode.data		= this;
			dlnodeInSCTMP.data	= this;
			pvectClientID	= new whvector<int>;
			pvectClientID->reserve(64);
		}
		~CLSUnit()
		{
			if( pvectClientID )
			{
				delete	pvectClientID;
				pvectClientID	= NULL;
			}
		}

		void	clear()
		{
			nID			= 0;
			nStatus		= STATUS_NOTHING;
			pCntr		= NULL;
			nProp		= 0;
			nPort		= 0;
			IP			= 0;
			nGroupID	= -1;
			nMaxPlayer	= 0;
			nGZSNumToConnect	= 0;
			bCanAcceptPlayerWhenCLSConnectingGZS	= false;
			dlnode.leave();
			dlPlayer.clear();
			teid.quit();
		}
		// ����Ѿ�ӵ�е��������
		inline int	GetCurPlayerNum() const
		{
			return	dlPlayer.size();
		}
		// ��û����Խ��ɵ��������
		inline int	GetAcceptablePlayerNum() const
		{
			return	nMaxPlayer - GetCurPlayerNum();
		}
		// �����Լ���ǰ��״̬�Ƿ�����������
		inline bool	CanAcceptPlayer() const
		{
#ifdef	_DEBUG
			return	nStatus == STATUS_WORKING || (bCanAcceptPlayerWhenCLSConnectingGZS && nStatus == STATUS_CONNECTINGGZS);
#else
			return	nStatus == STATUS_WORKING;
#endif
		}
		void	AdjustInnerPtr(int nOffset)
		{
			pCntr		= NULL;
			teid.clear();
			// dlnode����ΪCAAFSGroup���Ѿ������ˣ���������Ͳ��õ����ˣ�
			// ����б���Ҫ����һ��
			dlPlayer.AdjustInnerPtr(nOffset);
			for(whDList<PlayerUnit *>::node *pNode=dlPlayer.begin(); pNode!=dlPlayer.end(); pNode=pNode->next)
			{
				wh_adjustaddrbyoffset( pNode->data, nOffset );
			}
			dlnode.data		= this;	// ��������Ӧ���Ѿ�У����node�е������ײ�ָ�������
			pvectClientID	= new whvector<int>;
			pvectClientID->reserve(64);

			dlnodeInSCTMP.data	= this;
			dlnodeInSCTMP.reset();
		}
		inline int	SendMsg(const void *pData, size_t nSize)
		{
			if( pCntr )
			{
				return	pCntr->SendMsg(pData, nSize);
			}
			return	-1;
		}
	};
	struct	GZSUnit
	{
		enum
		{
			PROP_GOODEXIT	= 0x01,									// �����Լ��������˳���
		};
		MYCNTRSVR::MYCNTR	*pCntr;									// ��Ӧ������������ָ�루�������ǿ�˵��GZS����GMS�ɹ��ˣ�
		unsigned char		nSvrIdx;								// ��Ӧ�����
		unsigned char		nProp;									// ��������
		port_t				nPort;									// ��������Ķ˿�
		unsigned int		IP;										// ���������IP
		whtimequeue::ID_T	teid;									// ����ĳЩ��GZS��ص��¼�

		GZSUnit()
		: pCntr(NULL)
		, nSvrIdx(0)
		, nProp(0)
		, nPort(0)
		, IP(0)
		{
		}

		void	clear()
		{
			pCntr	= NULL;
			nSvrIdx	= 0;
			nProp	= 0;
			nPort	= 0;
			IP		= 0;
			teid.quit();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			pCntr		= NULL;
			teid.clear();
		}
	};
	struct	GMTOOLUnit
	{
		int		nID;
		MYCNTRSVR::MYCNTR	*pCntr;									// ��Ӧ������������ָ��

		GMTOOLUnit()
		: nID(0)
		, pCntr(NULL)
		{
		}

		void	clear()
		{
			WHMEMSET0THIS();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			pCntr		= NULL;
		}
	};
	struct	PlayerUnit
	{
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_WANTCLS				= 1,						// GMS������CLS�ύ������
			STATUS_GOINGTOCLS			= 2,						// �������ȥ��CLS��
			STATUS_GAMING				= 3,						// ��������ϷͨѶ��
		};
		int		nID;
		int		nStatus;											// ��ǰ��״̬
		int		nCLSID;												// ������CLS��ID
		int		nClientIDInCAAFS;									// �������CAAFS�е�ID�������������CAAFS�ж�λ����ģ�
		unsigned int	IP;											// �û�IP
		unsigned char	nNetworkType;								// ��Ҷ�Ӧ���������ͣ�NETWORKTYPE_XXX��
		unsigned char	nSvrIdx;									// ������Ŀ��GZS�����
		whDList<PlayerUnit *>::node		dlnodeInCLS;				// ��CLS��Player�б��еĽڵ�Ԫ��

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nCLSID(0)
		, nClientIDInCAAFS(0)
		, IP(0)
		, nNetworkType(NETWORKTYPE_UNKNOWN)
		, nSvrIdx(0)
		{
			dlnodeInCLS.data	= this;
		}

		void	clear()
		{
			nID					= 0;
			nStatus				= STATUS_NOTHING;
			nCLSID				= 0;
			nClientIDInCAAFS	= 0;
			nSvrIdx				= 0;
			dlnodeInCLS.leave();
		}
		void	AdjustInnerPtr(int nOffset)
		{
			// Ŀǰû��ʲôҪ������
		}
		void	ReadFromOther(PlayerUnit *pOther)
		{
			nStatus				= pOther->nStatus;
			nCLSID				= pOther->nCLSID;
			IP					= pOther->IP;
			nNetworkType		= pOther->nNetworkType;
			nSvrIdx				= pOther->nSvrIdx;
			if( pOther->dlnodeInCLS.isinlist() )
			{
				dlnodeInCLS.insertbefore(&pOther->dlnodeInCLS);
			}
		}
	};
	struct	TQUNIT_T
	{
		typedef	void (GMS_MAINSTRUCTURE::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// ������¼��ĺ���
		// ���ݲ���
		union
		{
			struct	SVR_T											// ��������ʱ�¼�
			{
				int	nID;											// ��������connecterid����������ID
			}svr;
			struct	GZS_T											// GZS�Ķ�ʱ�¼�
			{
				int	nIdx;											// GZS���
			}gzs;
		}un;
	};
	struct	CFGINFO_T		: public whdataini::obj
	{
		// ��������
		int					nMaxPlayer;								// ���ɳ��ص��������
		int					nSafeReservePlayerNum;					// Ϊ�˻�����������ͳ�Ʋ�׼������û��Ž����������û�����ʧ�ܵĻ��巧ֵ
		int					nMaxCLS;								// ���ɳ��ص�CLS����(�����ͨ��anMaxCLSNumInCAAFSGroup��ӵõ��ģ�������������������ļ�������)
		int					nMaxGMTOOL;								// ���ɳ��ص�GMTOOL����
		int					nTQChunkSize;							// ʱ����е�ÿ�δ��Է�����С
		int					nQueueCmdInSize;						// ָ��������еĳ���
		int					nHelloTimeOut;							// �ȴ����ӷ���Hello�ĳ�ʱ(���û���յ�Hello��Ͽ�����)
		int					nSvrDropWaitTimeOut;					// ��������з�������GZS��CLS�����ߣ����Եȴ�һ��ʱ��֮����ɾ������ΪҲ��������Ϊ��ʱ������ԭ�򣬲��ǶԷ������ˡ�
		int					nCalcAndSendCAAFSAvailPlayerTimeOut;	// ��ʱ�������CAAFS��Ŀɽ��������ļ��
		int					nSvrCmdQueueSize;						// ���ڻ���GZSָ��Ķ��г���
		int					nSvrCmdPerSecond;						// ÿ���ӿ��Է�����GZSָ������
		bool				bGZSDownRestorePlayer;					// �Ƿ���Ҫ��GZS崻�����֮������н�ɫ�Զ��ָ��������GMSGamePlay�е����ö���Ҫһ�£�
		int					anMaxCLSNumInCAAFSGroup[GMS_MAX_CAAFS_NUM];
																	// ����CAAFS��������CLS����
		int					anMaxPlayerNumInCLSOfCAAFSGroup[GMS_MAX_CAAFS_NUM];
																	// ����CAAFS����ÿ��CLS�����������
		MYCNTRSVR::DATA_INI_INFO_T	CNTRSVR;						// TCPConnecterServer��ͨ������
		CFGINFO_T()
		: nMaxPlayer(128)											// д��һ�㣬�������������ᷢ�ֵıȽ���
		, nSafeReservePlayerNum(128)
		, nMaxCLS(0)
		, nMaxGMTOOL(8)
		, nTQChunkSize(100)
		, nQueueCmdInSize(2000000)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nSvrDropWaitTimeOut(20*1000)
		, nCalcAndSendCAAFSAvailPlayerTimeOut(10000)
		, nSvrCmdQueueSize(10*1024*1024)
		, nSvrCmdPerSecond(200)										// ��Ҫָ���̫��
		, bGZSDownRestorePlayer(false)
		{
			CNTRSVR.nPort	= GMS_DFT_PORT;
			memset(anMaxCLSNumInCAAFSGroup, 0, sizeof(anMaxCLSNumInCAAFSGroup));
			memset(anMaxPlayerNumInCLSOfCAAFSGroup, 0, sizeof(anMaxPlayerNumInCLSOfCAAFSGroup));
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// ���ڹ����ڴ��еĽṹ
	// ��������
	typedef	whunitallocatorFixed3<CLSUnit, AO_T>		UAF_CLS_T;
	typedef	whunitallocatorFixed3<GMTOOLUnit, AO_T>		UAF_GMTOOL_T;
	typedef	whunitallocatorFixed3<PlayerUnit, AO_T>		UAF_Player_T;
	struct	MYSHMDATA_T
	{
		UAF_CLS_T		m_CLSs;
		UAF_GMTOOL_T	m_GMTOOLs;
		UAF_Player_T	m_Players;
		CAAFSGroup		m_aCAAFSGroup[GMS_MAX_CAAFS_NUM];
		GZSUnit			m_GZSs[GZS_MAX_NUM];

		MYSHMDATA_T()
		{
		}

		template<typename _Ty>
		void	AdjustInnerPtrAndEveryUnit(AM_T *pAM, _Ty &t, int nOffset)
		{
			t.AdjustInnerPtr(pAM->GetAO(), nOffset);
			// ���Է��������е�ÿ����Ԫ������ҲAdjustһ��
			for(typename _Ty::iterator it=t.begin(); it!=t.end(); ++it)
			{
				(*it).AdjustInnerPtr(nOffset);
			}
		}
		void	AdjustInnerPtr(AM_T *pAM, int nOffset)
		{
			AdjustInnerPtrAndEveryUnit(pAM, m_CLSs, nOffset);
			AdjustInnerPtrAndEveryUnit(pAM, m_GMTOOLs, nOffset);
			AdjustInnerPtrAndEveryUnit(pAM, m_Players, nOffset);
			wh_adjustarraybyoffset(m_aCAAFSGroup, GMS_MAX_CAAFS_NUM, nOffset);
			wh_adjustarraybyoffset(m_GZSs, GZS_MAX_NUM, nOffset);
			// Ȼ��Ӧ�ð�m_GMTOOLs��գ���Ϊ����ǲ���Ҫ�������õ�
			m_GMTOOLs.clear();
		}
		// ����ָ����CAAFS�����
		inline int	GetCAAFSGroupID(CAAFSGroup *pCAAFS)
		{
			return	wh_getidxinarray(m_aCAAFSGroup, pCAAFS);
		}
		//
		static MYSHMDATA_T *	NewMe(AM_T *pAM)
		{
			MYSHMDATA_T	*pObj;
			if( !pAM->New(pObj) )
			{
				return	NULL;
			}
			return	pObj;
		}
		static void	DeleteMe(AM_T *pAM, MYSHMDATA_T *&pObj)
		{
			if( !pObj )
			{
				return;
			}
			// ɾ�������Ӷ����е�����
			pObj->m_CLSs.Release();
			pObj->m_GMTOOLs.Release();
			pObj->m_Players.Release();
			// ɾ��������
			pAM->Delete(pObj);
			pObj	= NULL;
		}
	};
private:
	typedef	CMN::ILogic					FATHERCLASS;
	// ���ýṹ
	CFGINFO_T							m_cfginfo;					// GMS������
	tcpmsger::DATA_INI_INFO_T			m_MSGER_INFO;				// GMS������Server��TCP����ͨѶ��msger������
	// TCP������
	MYCNTRSVR		m_CntrSvr;
	// ��ǰʱ��
	whtick_t		m_tickNow;
	// ʱ���¼�������
	whtimequeue		m_TQ;
	// ������ʱ�ϳ�ָ��(���в��ö��������ָ��Ӧ�ö�����ͨ����������)
	// ����ʹ��������: Sendϵ����Ҫָ�����ݵĺ���������������Ŀ����Player�ĺ���
	// Tick��Ҳ����ʹ���������ɸ�CLS��ָ��
	whvector<char>	m_vectrawbuf;
	// ��ʱͳ�Ƹ�CAAFS��Ŀɽ�������
	whtimequeue::ID_T	m_teid_CalcAndSend_CAAFS_AvailPlayerNum;
	// ��Ҫʹ�õ�������logicģ��(���û����������ȼ�һ���ٵ�)
	ILogicBase		*m_pLogicGamePlay;
	whhash<pngs_cmd_t, ILogicBase *>	m_mapCmdReg2Logic;			// ����ָ��ּ𣬰��ض���ָ��ּ���ض����߼�ģ��ȥִ��
	whhashset<pngs_cmd_t>				m_setRegGZSSvrCmd;			// ���ڰѴ���GZS���ض�ָ���Ȼ��壬Ȼ����һ�����ʷ���
	// ���ڻ���GZSSvrCmd�Ķ���
	whsmpqueue							m_queueGZSSvrCmd;
	// ��������ÿ��tickת����connecting���еĸ���
	whgener								m_generSendGZSSvrCmd;
	// ���ڻ��ÿ��tick��ʱ����
	whinterval							m_itv;
	// �ڴ����ģ��
	ILogicBase		*m_pLogicMEM;
	AO_T			*m_ao;
	AM_T			*m_am;
	MYSHMDATA_T		*m_pSHMData;
	// �߼��Ͽ������ɵ��������
	int				m_nGMSLogicMaxPlayer;
private:
	friend	class	MYCNTRSVR;
	friend	class	MYCNTRSVR::MYCNTR;
	// ���������ļ���ʼ������
	int		Init_CFG(const char *cszCFGName);
	// ͨ��GroupID�ҵ�CAAFS
	CAAFSGroup *	GetCAAFSByGroupID(int nGroupID);
	// ���һ��Connecter��һ�ι���
	bool	CNTR_CanDealMsg() const;
	// ���һ��Connecter��һ�ι���
	int		CNTR_DealMsg_Dft(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_CAAFS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_CLS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_GZS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_GMTOOL(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	// ���MYCNTRSVR�ĺ���(���������)
	Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	void	AfterAddConnecter(Connecter * pCntr);
	void	BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo);
	// ���������ӵķ������Զ��˳�(����GMS�Ķ˿ھͲ���timewait��)
	void	TellAllConnecterToQuit();
	// �����е�CAAFS�˳�
	void	TellAllCAAFSQuit();
	// �����е�CLS�˳�
	void	TellAllCLSQuit();
	// �����е�GZS�˳�
	void	TellAllGZSQuit();
	// ��CLS�߳��û�(֮��������д��Ϊ��ǿ���߳��û���Զ���Ǵ�CLS�����)
	void	TellCLSToKickPlayer(int nClientID, pngs_cmd_t nSubCmd = SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID);
	//
	void	TellCLSToCheckPlayerOffline(CLSUnit *pCLS);
	void	TellGZSToCheckPlayerOffline(GZSUnit *pGZS);
	// �Ƴ�һ��Connecter�����Ƴ��丽���Ķ����磺CAAFSGroup/CLSUnit/GZSUnit�ȵȣ�
	int		RemoveMYCNTR(int nCntrID);
	int		RemoveMYCNTR(MYCNTRSVR::MYCNTR *pCntr);
	int		RemovePlayerUnit(int nID, unsigned char nRemoveReason = CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTHING);
	int		RemovePlayerUnit(PlayerUnit *pPlayer, unsigned char nRemoveReason = CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTHING);
	// ����һ��GZS����Ϣ��һ��CLS(����CLS�Ϳ��Կ�ʼ�������ˡ������ڲ�����ı�CLS��״̬ΪSTATUS_CONNECTINGGZS)
	int		SendGZSInfo0ToCLSAndChangeCLSStatus(GZSUnit *pGZS, CLSUnit *pCLS);
	// ����CLS�����������û�ID��GZS
	int		SendPlayerIDInCLSToGZS(CLSUnit *pCLS, GZSUnit *pGZS);
	// �����е�CLS����һ��ָ��
	int		SendCmdToAllCLS(const void *pCmd, size_t nSize);
	// ��Player���ڵ�CLS����һ��ָ��(�ڲ�ֱ�Ӱ�ָ��ݸ�CLS��û��ʹ��m_vectrawbuf�������ϲ����ʹ��m_vectrawbuf������ָ��)
	int		SendCmdToPlayerCLS(int nClientID, const void *pCmd, size_t nSize);
	int		SendCmdToPlayerCLS(PlayerUnit *pPlayer, const void *pCmd, size_t nSize);
	// �����е�GZS����һ��ָ��
	int		SendCmdToAllGZS(const void *pCmd, size_t nSize);
	// ��һ����GZS���͵ĵͼ�ָ����뻺����У���������ʱ��Ϊ�˱���ָ�����
	int		QueueRawCmdToGZS(unsigned char nSvrIdx, const void *pCmd, size_t nSize);
	// ��GZS��һ��SvrCmd
	int		SendSvrCmdToAllGZS(const void *pCmd, size_t nSize, unsigned char nFromSvrIdx, unsigned char nExceptSvrIdx=0);
	int		SendSvrCmdToGZS(unsigned char nSvrIdx, const void *pCmd, size_t nSize, unsigned char nFromSvrIdx);
	int		SendSvrCmdToGZS(GZSUnit *pGZS, const void *pCmd, size_t nSize, unsigned char nFromSvrIdx);
	// �����е�GMTOOL����һ��ָ��
	int		SendCmdToAllGMTOOL(const void *pCmd, size_t nSize);
	// ��Player����ָ��(�ڲ����װ�ɷ���CLS��ָ��)
	int		SendCmdToPlayer(int nPlayerID, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToPlayer(PlayerUnit *pPlayer, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS(int nCLSID, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS(CLSUnit *pCLS, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToAllPlayer(unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	int		SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	// ����Player��Tagֵ(�ڲ���װ��SVR_CLS_SET_TAG_TO_CLIENT���͸�CLS)
	int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal);
	// Tick�еķ���
	// ʱ���¼�����
	int		Tick_DealTE();
	// ��������������û�з���hello�ĳ�ʱ
	void	TEDeal_Hello_TimeOut(TQUNIT_T *pTQUnit);
	// ����������¼�
	void	SetTE_Hello_TimeOut(Connecter * pCntr);
	// GZS���ߵȴ��ĳ�ʱ
	void	TEDeal_GZS_DropWait_TimeOut(TQUNIT_T *pTQUnit);
	// ����������¼�
	void	SetTE_GZS_DropWait_TimeOut(GZSUnit *pGZS);
	// CLS���ߵȴ��ĳ�ʱ
	void	TEDeal_CLS_DropWait_TimeOut(TQUNIT_T *pTQUnit);
	// ����������¼�
	void	SetTE_CLS_DropWait_TimeOut(CLSUnit *pCLS);
	// ���ڼ������CAAFS���ɽ�����û������ļ��
	void	TEDeal_CalcAndSend_CAAFS_AvailPlayerNum(TQUNIT_T *pTQUnit);
	// �����û�������ָ��(�ڲ�Ӧ����ת������Ӧ���߼�����Ԫ)
	void	InnerRouteClientGameCmd(CMN::cmd_t nCmd, CLS_SVR_CLIENT_DATA_T *pCmd, size_t nSize);
	// ����GZS������ָ��(�ڲ�����Ŀ������������Ĳ�ͬת����GMSGamePlay��������ӦGZS��GZSGamePlay)
	void	InnerRouteGZSGameCmd(MYCNTRSVR::MYCNTR *pCntr, GZS_GMS_GAMECMD_T *pGameCmd, size_t nSize);
	// �����й�GZS���ߵ�һЩ���飬�����˭˭����ʲô��Ϣ֮���
	int		DealGZSDisconnection(GZSUnit *pGZS);
	// �����й�CLS���ߵ�һЩ���飬�����˭˭����ʲô��Ϣ֮���
	int		DealCLSDisconnection(CLSUnit *pCLS);

	// GMָ���
	int		DealCmdIn_PNGSPACKET_2LOGIC_GMCMD(ILogicBase *pRstAccepter, PNGSPACKET_2LOGIC_GMCMD_T *pCmd);
	// ֻ�����ִ�ָ��Ĳ���
	int		InitTstStrMap();
	int		DealTstStr(const char *cszTstStr, char *szRstStr, size_t *pnSize);
	///////////////////////////////////////
	// �����ִ�ָ����Ҫ�õ�����������
	///////////////////////////////////////
	whcmn_strcmd_reganddeal_DECLARE_m_srad(GMS_MAINSTRUCTURE);
	// ��ǰ����д�����Ļ���ָ�뿪ͷ���������Ų��ϵ�д��������仯��
	char	*m_pszRstStr;
	// ����д��ĳ���
	size_t	*m_pnRstSize;
	//
	int		_i_srad_NULL(const char *param);
	int		_i_srad_help(const char *param);
	int		_i_srad_getclsinfo(const char *param);
	int		_i_srad_setclsinfo(const char *param);

public:
	// �����Լ����͵Ķ���
	static CMN::ILogic *	Create()
	{
		return	new	GMS_MAINSTRUCTURE;
	}
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_GMS_I_GMS_MAINSTRUCTURE_H__
