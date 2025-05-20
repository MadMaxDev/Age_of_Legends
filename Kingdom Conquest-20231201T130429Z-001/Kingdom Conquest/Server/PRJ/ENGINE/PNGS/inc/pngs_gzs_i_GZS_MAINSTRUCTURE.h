// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTURE���Logicģ���ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Zone Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-09-20
// Change LOG   :

#ifndef	__PNGS_GZS_I_GZS_MAINSTRUCTURE_H__
#define	__PNGS_GZS_I_GZS_MAINSTRUCTURE_H__

#include "./pngs_cmn.h"
#include "./pngs_packet.h"
#include "./pngs_packet_gzs_logic.h"
#include "./pngs_packet_logic.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whconnecter.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whcmd.h>
#include <WHCMN/inc/whhash.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

////////////////////////////////////////////////////////////////////
// GZS_MAINSTRUCTURE
////////////////////////////////////////////////////////////////////
class	GZS_MAINSTRUCTURE	: public CMN::ILogic
{
// Ϊ�ӿ�ʵ�ֵ�
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	GZS_MAINSTRUCTURE();
	~GZS_MAINSTRUCTURE();
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
		GZS_MAINSTRUCTURE	*m_pHost;
	public:
		class	MYCNTR	: public	TCPConnecterMsger<tcpmsger>
		{
		public:
			GZS_MAINSTRUCTURE	*m_pHost;
			struct sockaddr_in	m_addr;								// ��Դ��ַ
			APPTYPE				m_nSvrType;							// ���������ʲô���͵�Server
																	// ȡֵΪpngs_def.h�е�APPTYPE_XXX
			void				*m_pExtPtr;							// ����ָ�루������ӹ����Ķ���ָ�룩
			whtimequeue::ID_T	teid;
			typedef	int		(GZS_MAINSTRUCTURE::*CNTR_DEALMSG_T)(MYCNTR *pCntr, const pngs_cmd_t *pData, size_t nSize);
			CNTR_DEALMSG_T	m_p_CNTR_DEALMSG;
		public:
			MYCNTR(GZS_MAINSTRUCTURE *pHost)
			: m_pHost(pHost)
			, m_nSvrType(APPTYPE_NOTHING)
			, m_pExtPtr(NULL)
			, m_p_CNTR_DEALMSG(&GZS_MAINSTRUCTURE::CNTR_DealMsg_Dft)
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
		virtual void		AfterAddConnecter(Connecter * pCntr)
		{
			m_pHost->AfterAddConnecter(pCntr);
		}
	public:
		MYCNTRSVR()
		: m_pHost(NULL)
		{
		}
	};
	struct	CLSUnit
	{
		enum
		{
			STATUS_NOTHING			= 0,
			STATUS_WORKING			= 1,							// ��������״̬
		};
		int		nID;
		int		nStatus;											// ״̬
		MYCNTRSVR::MYCNTR	*pCntr;									// ��Ӧ������������ָ��

		CLSUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, pCntr(NULL)
		{
		}

		void	clear()
		{
			nID			= 0;
			nStatus		= STATUS_NOTHING;
			pCntr		= NULL;
		}
	};
	struct	PlayerUnit
	{
		// 2005-09-30 ȡ���������Status
		int		nID;
		int		nCLSID;												// ������CLS��ID

		PlayerUnit()
		: nID(0)
		, nCLSID(0)
		{
		}

		void	clear()
		{
			nID				= 0;
			nCLSID			= 0;
		}
	};
	struct	TQUNIT_T
	{
		typedef	void (GZS_MAINSTRUCTURE::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// ������¼��ĺ���
		// ���ݲ���
		union
		{
			struct	SVR_T											// ��������ʱ�¼�
			{
				int	nID;											// ��������connecterid
			}svr;
		}un;
	};
	struct	CFGINFO_T		: public whdataini::obj
	{
		// ��������
		unsigned char		nSvrIdx;								// GZS���������
		unsigned char		nForceKickOldGZS;						// �Ƿ�ǿ���߳�ԭ����ͬSvrIdx��GZS��0:���߳���1:����һ�ߣ�2:ǿ���߳���
		int					nTQChunkSize;							// ʱ����е�ÿ�δ��Է�����С
		int					nQueueCmdInSize;						// ָ��������еĳ���
		int					nHelloTimeOut;							// �ȴ����ӷ���Hello�ĳ�ʱ(���û���յ�Hello��Ͽ�����)
		char				szGMSAddr[WHNET_MAXADDRSTRLEN];			// GMS�ĵ�ַ
		int					nConnectOtherSvrTimeOut;				// ����GMS������Server�ĳ�ʱ
		int					nMaxCmdPackSize;						// ָ���������ߴ�
		bool				bLogSendCmd;							// �Ƿ��¼���͵�ָ��
		bool				bLogRecvCmd;							// �Ƿ��¼���յ���ָ������ͻ��˺ͷ����������ģ�
		MYCNTRSVR::DATA_INI_INFO_T	CNTRSVR;						// TCPConnecterServer��ͨ������
		CFGINFO_T()
		: nSvrIdx(0)												// Ĭ����0�����������ļ��б�����д������������
		, nForceKickOldGZS(0)
		, nTQChunkSize(100)
		, nQueueCmdInSize(2000000)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nConnectOtherSvrTimeOut(PNGS_DFT_SVRCONNECT_TIMEOUT)
		, nMaxCmdPackSize(65536)
		, bLogSendCmd(false)
		, bLogRecvCmd(false)										// ���Ĭ���ȴ򿪰ɡ�ֱ���鵽�����������Ĵ���Ϊֹ��
		{
			strcpy(szGMSAddr, "localhost:3000");
			CNTRSVR.nPort	= 3101;
			CNTRSVR.nPort1	= 3110;
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
private:
	typedef	CMN::ILogic					FATHERCLASS;
	// ���ýṹ
	CFGINFO_T							m_cfginfo;					// ������Ϣ
	tcpmsger::DATA_INI_INFO_T			m_MSGER_INFO;				// GZS������Server��TCP����ͨѶ��msger������
	// TCP������
	MYCNTRSVR		m_CntrSvr;
	// ��������
	whunitallocatorFixed<CLSUnit>		m_CLSs;
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// ��ǰʱ��
	whtick_t		m_tickNow;
	// ״̬
	enum
	{
		STATUS_ERROR_WITH_GMS			= -1,						// ��GMS������
		STATUS_NOTHING					= 0,
		STATUS_WORKING					= 1,
	};
	int				m_nStatus;
	// ����GMS��msger
	class	MYMSGER_T	: public tcpretrymsger<tcpmsger>
	{
	public:
		GZS_MAINSTRUCTURE				*m_pHost;
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
	MYMSGER_T		m_msgerGMS;
	// ʱ���¼�������
	whtimequeue		m_TQ;
	// ������ʱ�ϳ�ָ��
	whvector<char>	m_vectrawbuf;
	// ����ָ�����
	whsmpqueueWL	m_queueCmdIn;
	// ��Ҫʹ�õ�������logicģ��
	// ��Ϸ�߼�ģ��
	ILogicBase		*m_pLogicGamePlay;
	// �������ɴ��ָ��
	whvector<char>	m_vectCmdPack;
	whcmdshrink		m_wcsCmdPack;
	// �����ʼ�ı�־��CmdPackBegin��
	GZSPACKET_2MS_CMD_PACK_BEGIN_T	m_cpb;
	inline void	CmdPackReset()
	{
		m_wcsCmdPack.Reset();
		m_cpb.nSendCmd	= GZSPACKET_2MS_0;
	}
	whhash<pngs_cmd_t, ILogicBase *>	m_mapCmdReg2Logic;			// ����ָ��ּ𣬰��ض���ָ��ּ���ض����߼�ģ��ȥִ��
																	// ���Ŀǰ�������GMS������������ָ��GMS_GZS_GAMECMD
	unsigned int	m_nNoCLIENT_RECV;								// ���������ģ���������������ĳ��bit��ֻҪ���������0���򲻽����κοͻ��˵�����

	bool			m_bIsReconnect;									// ��ʾ�������ϵ�
	bool			m_bConnectedToGMS;								// ��ʾ�Լ���GMS������״���ģ����Ϊfalse���߼���Ӧ�ÿ��ǲ�Ҫ���к�GMS��صĲ����ˣ�
private:
	friend	class	MYCNTRSVR;
	friend	class	MYCNTRSVR::MYCNTR;
	// ���������ļ���ʼ������
	int		Init_CFG(const char *cszCFGName);
	// �����������ָ��
	int		CmdIn_GZSPACKET_2MS_CONFIG(ILogicBase *pRstAccepter, GZSPACKET_2MS_CONFIG_T *pCmd, int nSize);
	// ���һ��Connecter��һ�ι���
	bool	CNTR_CanDealMsg() const;
	// ���һ��Connecter��һ�ι���
	int		CNTR_DealMsg_Dft(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	int		CNTR_DealMsg_CLS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize);
	// ���MYCNTRSVR�ĺ���(���������)
	Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	void	AfterAddConnecter(Connecter * pCntr);
	void	BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo);
	// ���������ӵķ������Զ��˳�(����GZS�Ķ˿ھͲ���timewait��)
	void	TellAllConnecterToQuit();
	// �����е�CLS�˳�
	void	TellAllCLSQuit();
	// ��CLS�߳��û�
	void	TellCLSToKickPlayer(int nClientID, unsigned char nKickCmd=GZSPACKET_2MS_KICKPLAYER_T::CMD_KICK_END);
	// ��CLS�߳������Լ���ص��û�
	void	TellCLSToKickAllPlayerOfMe();
	// ����GMS�Լ��������˳���
	void	TellGMSGoodExit();
	// ����GMS�߳��û����������Լ�����Remove�û��������������CLS�����ڵ�����²��е�
	void	TellGMSToKickPlayerAndRemovePlayer(int nClientID);
	// �Ƴ�һ��Connecter�����Ƴ��丽���Ķ����磺CLSUnit�ȵȣ�
	int		RemoveMYCNTR(int nCntrID);
	int		RemoveMYCNTR(MYCNTRSVR::MYCNTR *pCntr);
	int		RemovePlayerUnit(int nID);
	int		RemovePlayerUnit(PlayerUnit *pPlayer);
	// �����е�CLS����һ��ָ��
	int		SendCmdToAllCLS(const void *pCmd, size_t nSize);
	// ��Player���ڵ�CLS����һ��ָ��(�ڲ�ֱ�Ӱ�ָ��ݸ�CLS��û��ʹ��m_vectrawbuf�������ϲ����ʹ��m_vectrawbuf������ָ��)
	int		SendCmdToPlayerCLS(int nClientID, const void *pCmd, size_t nSize);
	int		SendCmdToPlayerCLS(PlayerUnit *pPlayer, const void *pCmd, size_t nSize);
	// ����������������ָ��
	int		SendSvrCmdToSvrBySvrIdx(unsigned char nSvrIdx, const void *pCmd, size_t nSize);
	// ��Player����ָ��
	int		SendCmdToPlayer(int nPlayerID, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToPlayer(PlayerUnit *pPlayer, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS(int nCLSID, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToMultiPlayerInOneCLS(CLSUnit *pCLS, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize);
	int		SendCmdToAllPlayer(unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	int		SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	int		SendCmdToAllPlayerByTag64(whuint64 nTag, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx=PNGS_SVRIDX_ALLGZS);
	// ����Player��Tagֵ(�ڲ���װ��SVR_CLS_SET_TAG_TO_CLIENT���͸�CLS)
	int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal);
	int		SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel);
	// ���Ӵ������
	int		AddPackData(const void *pData, size_t nDSize);
	// Tick�еķ���
	// ʱ���¼�����
	int		Tick_DealTE();
	// ��������������û�з���hello�ĳ�ʱ
	void	TEDeal_Hello_TimeOut(TQUNIT_T *pTQUnit);
	// ����GMS������ָ��
	int		Tick_DealGMSMsg();
	int		Tick_DealGMSMsg_GMS_GZS_CTRL(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_GZS_PLAYER_ONLINE(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_GZS_PLAYER_OFFLINE(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_GZS_GAMECMD(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_GZS_PLAYER_CHANGEID(void *pCmd, int nSize);
	// �����ָ��ݸ��߼�����
	void	InnerRouteClientGameCmd(CLS_SVR_CLIENT_DATA_T *pCmd, size_t nSize);
	// ����״̬
	void	SetStatus(int nStatus);
	// ������ʼʱ��Ҫ���õĺ���
	void	Retry_Worker_TRYCONNECT_Begin();
	// �������ɹ�ʱ��Ҫ���õĺ���
	void	Retry_Worker_WORKING_Begin();

public:
	// �����Լ����͵Ķ���
	static CMN::ILogic *	Create()
	{
		return	new	GZS_MAINSTRUCTURE;
	}
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_GZS_I_GZS_MAINSTRUCTURE_H__
