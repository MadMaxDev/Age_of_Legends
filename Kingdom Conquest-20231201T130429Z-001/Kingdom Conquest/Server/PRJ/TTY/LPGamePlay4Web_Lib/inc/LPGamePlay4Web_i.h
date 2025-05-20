//////////////////////////////////////////////////////////////////////////
// web��չ���߼���
// Ŀǰ���е���Ϣ����û��ʹ��wcs,����ֱ���ýṹ(wcs�ᰲȫ�ܶ�,��Ϣ�ṹ�ܶ�ط�������Ҫ�жϳ��ȵ�)
// �м�:һ��Ҫ��֤CLS4Web,LPMainStructure4Web,LPGamePlay4Web,GameMngS��Player ID��һ����
// Ŀǰֻ���ڽ�ɫ�б�����������Ϸ�в��ܵǳ�,������Ϸ�в�����ѡ����
// �м�:SendErrRstToClient_And_Done(ֻҪ������IAmDone)��Ҫ����return,��Ϊ�����Ѿ�������
// 
// DoNext_TTY_TC_PLAYER_DROP����������PlayerUnit�ͷ�,��ȻPlayerDrop��TimeEventͬʱ����ʱ�������(��PlayerDrop,DBS����,Transaction�ȴ�DBS����)
// ��TimeEvent����֮ǰ,��PlayerUnit�����ͷ�,������ҵ�¼����(LPGamePlay��LPMainStructure�е�Players���ܱ���һ�µ���)
//////////////////////////////////////////////////////////////////////////
#ifndef __LPGamePlay4Web_i_H__
#define __LPGamePlay4Web_i_H__

#include "PNGS/inc/pngs_def.h"
#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_packet_logic.h"

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whunitallocator3.h"
#include "WHCMN/inc/whtransaction.h"
#include "WHCMN/inc/whcmd.h"
#include "WHCMN/inc/whmd5.h"
#include "WHCMN/inc/whgoodstr.h"

#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnettcp.h"

#include "../../Common/inc/pngs_packet_db4web_logic.h"
#include "../../Common/inc/pngs_packet_lp_logic.h"
#include "../../Common/inc/pngs_packet_web_extension.h"
#include "../../Common/inc/tty_def_transaction_cmd.h"
#include "../../Common/inc/tty_def_gs4web.h"
#include "../../Common/inc/tty_common_db_timeevent.h"
#include "../../Common/inc/tty_common_db_timeevent.h"
#include "../../Common/inc/tty_common_excel_def.h"

#include "LPNet.h"
#include "GameMngS.h"

namespace n_pngs
{
//////////////////////////////////////////////////////////////////////////
// LPGamePlay
//////////////////////////////////////////////////////////////////////////
class LPGamePlay4Web_i : public CMN::ILogic, public CLPNet
{
// Ϊ�ӿ�ʵ��
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy();
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
// �Լ��õ�
public:
	LPGamePlay4Web_i();
	~LPGamePlay4Web_i();
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		// ��������
		int					nQueueCmdInSize;					// ָ��������еĳ���
		unsigned short		nSvrGrpID;							// ��������ID
		int					nMaxPlayer;							// ���ɳ��ص��������
		int					nTQChunkSize;						// ʱ�����ÿ�η���Ŀ��С
		int					nNoLoginTimeOut;					// ��LoginOK״̬�ĳ�ʱ
		int					nPlayerDropWaitTimeOut;				// �û�����֮��ȴ�һ��ʱ����ɾ��
		int					nPlayerCloseDelayTimeOut;			// �û������ر�����֮��Ҳ��Ҫ�ȴ�һ��ʱ�����֪ͨ�û�����
		int					nMaxTransaction;					// ����Transaction����
		int					nClearOfflineTimeOut;				// ����û�offline��ǵĳ�ʱ
		int					nTransactionTimeOut;				// һ���transaction��ʱʱ��
		int					nMaxCharPerAccount;					// ÿ���ʺ�ӵ�е�����ɫ��
		bool				bForceAccountNameLwr;				// �Ƿ���Ҫ���ʺ�ǿ��ת��ΪСд
		int					nReplaceCount;						// �˺ű��˳�����������Ĵ���(����һ�������ͻᱻ����),<=0��ʾ�����ɶ�
		int					nReplaceTimeOut;					// ����ϴα����Զ����ʱ��������ڳ��������ʱ��nReplaceCount�����»ָ���ʼֵ(��λ:s)
		char				szGS4WebAddr[WHNET_MAXADDRSTRLEN];	// gs4web�ĵ�ַ
		int					nDealDBTimeEventTimeOut;			// ����DBʱ���¼��ĳ�ʱ
		bool				bCreateAccountIfNotExist;			// û���˺�ʱ����д���
		bool				bUseRandomAccountName;				// ���˺Ų�������Ҫ�����˺�ʱ�Ƿ�ʹ������˺�
																// ��,ʹ��91�˺ŵ�һ�ε�½ʱ(�ɿͻ��˴���,��ʱ����bUseRandomAccountName=false)
																// ֻ��������bCreateAccountIfNotExist=trueʱ�ò�����������
																// bCreateAccountIfNotExist=falseʱ�Ƿ�ʹ������������ɿͻ��˴�����
		int					nDealPayTimeOut;					// �����ֵ��ʱ���¼�
		char				szBadCharNameFile[WH_MAX_PATH];			// �Ƿ���ɫ�����б�
		char				szBadAccountNameFile[WH_MAX_PATH];		// �Ƿ��˺������б�

		int					nDealCombatResultTimeOut;			// ����ս������ĳ�ʱ
		
		CFGINFO_T()
		: nQueueCmdInSize(2*1024*1024)							// ����Ǹ��첽����ָ��ʹ�õ�,û��Ҫ̫��
		, nSvrGrpID(0)
		, nMaxPlayer(128)
		, nTQChunkSize(100)
		, nNoLoginTimeOut(1*60*1000)
		, nPlayerDropWaitTimeOut(2*60*1000)
		, nPlayerCloseDelayTimeOut(10*1000)
		, nMaxTransaction(100*1000)
		, nClearOfflineTimeOut(20*1000)
		, nTransactionTimeOut(TTY_TRANSACTION_TIMEOUT)
		, bForceAccountNameLwr(true)
		, nReplaceCount(1)
		, nReplaceTimeOut(60)
		, nDealDBTimeEventTimeOut(2*1000)
		, bCreateAccountIfNotExist(false)
		, bUseRandomAccountName(true)
		, nDealPayTimeOut(2*1000)
		, nDealCombatResultTimeOut(200)
		{
			WH_STRNCPY0(szGS4WebAddr, "localhost:4800");
			WH_STRNCPY0(szBadCharNameFile, "../excel_tables/etc/badname.txt");
			WH_STRNCPY0(szBadAccountNameFile, "../excel_tables/etc/badname_account.txt");
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};

	// �߼���Ҷ���
	struct PlayerUnit 
	{
		struct STATUS_T 
		{
			// ����״̬
			enum
			{
				NOTHING			= 0,						// Ҳ������Ϊ��PRELOGINING
				CREATEACCOUNT	= 1,						// �����˺�
				LOGINING		= 2,						// ����LOGIN������,���LOGINʧ������Ҫ����Ӧӳ��ɾ��
				CREATECHAR		= 3,						// �ڴ�����ɫ������
				INLP			= 4,						// ��LP������������
				DROPWAIT		= 5,						// ����֮��ĵȴ�״̬
				CLOSEWAIT		= 6,						// �رյȴ���
				MAXNUM,
			};
			// ���ִ���������
			// ������ҵ���
			typedef	void	(LPGamePlay4Web_i::*Player_Func_Deal_Drop_T)(PlayerUnit* pPlayer);
			// ������ҷ�����ָ��
			typedef	void	(LPGamePlay4Web_i::*Player_Func_Deal_PlayerCmd_T)(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
			
			int				nStatus;						// ״ֵ̬
			// ����������
			Player_Func_Deal_Drop_T			pFuncDealDrop;
			Player_Func_Deal_PlayerCmd_T	pFuncDealCreateAccount;			// �����˺ŵ�
			Player_Func_Deal_PlayerCmd_T	pFuncDealLogin;
			Player_Func_Deal_PlayerCmd_T	pFuncDealCreateChar;			// ������ɫ			
			Player_Func_Deal_PlayerCmd_T	pFuncDealGameCmd;
			Player_Func_Deal_PlayerCmd_T	pFuncDealLogout;
			Player_Func_Deal_PlayerCmd_T	pFuncDealLoginByDeviceID;		// ͨ���豸�ŵ�½
			
			STATUS_T();
		};
		static	STATUS_T	aStatus[STATUS_T::MAXNUM];
		// Ϊ��ͬ״̬��亯��
		static	void		StatusObjInit();

		int					nID;										// �������е�ID(Client ID)
		int					nStatus;									// ���������Ϊ�˴ӹ����ڴ��лָ���ʱ������ָ���µ�status����
		STATUS_T*			pStatus;									// ״ָ̬��
		unsigned long		IP;											// ��ԴIP
		char				szAccount[TTY_ACCOUNTNAME_LEN];				// �ʺ�,��ʱ��ʵûɶ��
		char				szDeviceID[TTY_DEVICE_ID_LEN];				// �����豸ID
		char				szPass[TTY_PASSWORD_LEN];
		time_t				nInTime;									// ���ӳɹ���ʱ��
		int					nPassTryCount;
		unsigned char		nBinded;									// �Ƿ��EMail��
		unsigned int		nGiftAppID;									// һ̨�豸��һ�δ����˺�ʱ���ɵ�,֮����豸ĳ���汾�ĳ���Ϊ0
		unsigned int		nAppID;										// ���ε�¼��appid

		enum
		{
			PROCESSINGCMD_CREATEACCOUNT		= 0x01,						// ���ڴ������˺�����
			PROCESSINGCMD_CREATECHAR		= 0x02,						// ���ڴ�������ɫ����
		};
		unsigned char		nProcessingCmd;								// ���������ڴ���ʲô����ָ��
		unsigned char		nDropReason;								// �û����ߵ�ԭ��(CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_XXX)
		int					nCurTransactionID;							// ��ǰ������transaction id
		whtimequeue::ID_T	teid;
// 		// ���µ�������Ҫ�ں���ʵ��
		tty_id_t			nAccountID;									// �ʺ�OnlyID
 		unsigned char		nReplaceCount;								// ���˳�����������Ĵ���(����һ�����ͱ�����)
 		time_t				nLastReplaceTime;							// �ϴα��˳��Զ���ʱ��,���̫������¼���
		int					nTermType;									// �ն�����

		PlayerUnit();
		void		clear();
		inline	void	SetStatus(int s)
		{
			assert(s>=0 && s<STATUS_T::MAXNUM);
			nStatus		= s;
			pStatus		= &aStatus[nStatus];
		}
		inline	int		GetStatus() const
		{
			return pStatus->nStatus;
		}
		inline void	SetTransactionID(int nID)
		{
			assert(nCurTransactionID<=0);
			nCurTransactionID	= nID;
		}
	};

	// ʱ���¼���Ԫ
	struct TQUNIT_T
	{
		typedef	void	(LPGamePlay4Web_i::*TEDEAL_T)(TQUNIT_T*);
		TEDEAL_T	tefunc;								// �����¼��ĺ���
		// ���ݲ���
		union
		{
			struct PLAYER_T 
			{
				int	nID;								// ���ID
			}player;
			void*	pExt;
			int		nExt;
		}un;
	};

private:
	friend	struct	PlayerUnit;
	friend	struct	PlayerUnit::STATUS_T;
	typedef	CMN::ILogic	FATHERCLASS;
	// ���ýṹ
	CFGINFO_T		m_cfginfo;
	// ��ǰʱ��
	whtick_t		m_tickNow;			// ��ǰtick
	time_t			m_timeNow;			// ��ǰʱ��
	// ������ʱ�ϳ�ָ��
	whvector<char>	m_vectrawbuf;
	// �ʺ�ID��Ӧ���ID
	whhash<whint64, int>				m_mapAccountID2ClientID;
	// ����Login�е��ʺ�����Ӧ���ID
	whhash<whstrptr4hash, int>			m_mapTryLoginAccountName2ClientID;
	// ʱ�����
	whtimequeue							m_TQ;
	// ���������
	whtransactionman*					m_pTMan;
	// ��ҵ�Ԫ
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// �����ģ��
	ILogicBase*							m_pLogicMainStructure;
	ILogicBase*							m_pLogicDB;
private:
	CGameMngS::CFGINFO_T				m_GameMngCfg;
	CGameMngS*							m_pGameMngS;
private:
	bool								m_bLoadTerrainOK;
	bool								m_bLoadPlayerCardOK;
	bool								m_bLoadExcelTextOK;
	bool								m_bLoadExcelAllianceCongressOK;
	bool								m_bLoadCharLevelRankOK;
	bool								m_bNeedUpdatePlayerCardLevelRankData;
	bool								m_bNeedGetWorldFamousCity;
	bool								m_bNeedGenWorldFamousCity;
	bool								m_bCanGenWorldFamousCity;
	bool								m_bClearWorldFamousCityFirstWhenGen;
	bool								m_bAlreadyGetWorldFamousCity;

	bool								m_bNeedGetWorldRes;
	bool								m_bNeedGenWorldRes;
	bool								m_bCanGenWorldRes;
	bool								m_bAlreadyGetWorldRes;
private:
	whdaytimelooper						m_dtlHour;
	whdaytimelooper						m_dtlMinute;
	bool								m_bAlreadyLoadRank;
	whbadword*							m_pBadCharNameFilter;
	whbadword*							m_pBadAccountNameFilter;
	int		SafeInitBadNameFilter();
private:
	// ���������ļ���ʼ������
	int		Init_CFG(const char* cszCFGName);
	// ���������ļ�����һЩ����
	int		Init_SetParams();
	//////////////////////////////////////////////////////////////////////////
	// �������ָ��
	//////////////////////////////////////////////////////////////////////////
	// �����Ӻ������ܻ��׳��Ĵ���
	enum
	{
		PLAYERCMD_ERR_SUCCESS			= 0,					// û��
		PLAYERCMD_ERR_NOTSUPPORT		= 1,					// ��֧��������ָ��
		PLAYERCMD_ERR_INVALIDCMD		= 2,					// ����ʶ��ָ��
		PLAYERCMD_ERR_CMDREAD			= 3,					// ����ָ��ʱ������
		PLAYERCMD_ERR_NOCHAR			= 5,					// û�д�����ɫ������²��ܽ�����Ϸ
		PLAYERCMD_ERR_INVALIDPARAM		= 8,					// ����Ĳ���
		PLAYERCMD_ERR_BADPASSWD			= 9,					// ���������
		PLAYERCMD_ERR_RESTRICTED		= 10,					// ��ɫ����ֹ��Ϸ
		PLAYERCMD_ERR_BADGROUPID		= 11,					// ����Ĵ�����
		PLAYERCMD_ERR_REFUSED			= 81,					// �������ܾ�
		PLAYERCMD_ERR_TRANSACTION		= 254,					// transaction�������
		PLAYERCMD_ERR_UNKNOWN			= 255,					// δ֪����
	};
	int		DealCmdIn_LPPACKET_2GP_PLAYERCMD(ILogicBase* pRstAccepter, LPPACKET_2GP_PLAYERCMD_T* pCmd);
	// ����CAAFS4Web���������ָ��
	void	DealCmdIn_LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web(ILogicBase* pRstAccepter, LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web_T* pCmd);
	
	//////////////////////////////////////////////////////////////////////////
	// ����player״̬��ͬ�Ĳ�ͬ������
	//////////////////////////////////////////////////////////////////////////
	// ������Ҷ��ߵ�
	// ʲôҲ����
	void	Player_Func_Deal_Drop_Ignore(PlayerUnit* pPlayer);
	// ���ô���(ֻ��ɾ������)
	void	Player_Func_Deal_Drop_Nothing(PlayerUnit* pPlayer);
	// �ڿ�ʼLOGIN֮��Ĵ���(������û���ܵ����ص����)
	void	Player_Func_Deal_Drop_AfterLogin(PlayerUnit* pPlayer);
	
	// �������ָ��
	// ״̬��֧�������Ĵ���
	void	Player_Func_Deal_PlayerCmd_NotSupport(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// ����ָ��
	void	Player_Func_Deal_PlayerCmd_Ignore(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// �����˺�
	void	Player_Func_Deal_PlayerCmd_Client_CreateAccount(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// ����Login(ֻ����STATUS::NOTHING״̬��ʹ��)
	void	Player_Func_Deal_PlayerCmd_Client_Login(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// ����LoginByDeviceID(ֻ����STATUS::NOTHING״̬��ʹ��)
	void	Player_Func_Deal_PlayerCmd_Client_LoginByDeviceID(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// ������ɫָ��(���ֻ����STATUS_T::LOGINING֮����Ϸ��ʼǰʹ��)
	void	Player_Func_Deal_PlayerCmd_Client_CreateChar(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// ��ʼ��Ϸ
	// ��Ϸ��
	void	Player_Func_Deal_PlayerCmd_Client_GameCmd(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);
	// �ǳ���Ϸ
	void	Player_Func_Deal_PlayerCmd_Client_Logout(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize);

	//////////////////////////////////////////////////////////////////////////
	// ����
	//////////////////////////////////////////////////////////////////////////
	// ��MS�߳��û�(�����߳��û��Ĳ�������������CLS4Web��ʼ)
	int		TellLogicMainStructureToKickPlayer(int nClientID, pngs_cmd_t nKickSubCmd=SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID);
	int		KickPlayerOffline(int nClientID, pngs_cmd_t nKickSubCmd/* =SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID */)
	{
		return TellLogicMainStructureToKickPlayer(nClientID, nKickSubCmd);
	}
	// ֪ͨMS����õ��������
	int		TellLogicMainStructureLogicAvailPlayerNum();
	// ֪ͨDB�û�Ӧ��logout(�������߱�ɾ��)
	int		TellDBPlayerLogout(tty_id_t nAccountID);
	// ֪ͨȫ���û�����(��Ҫ�Ǹ�������ʱ��)
	int		TellGlobalDBPlayerLogout(tty_id_t nAccountID);
	
	// �������״̬
	int		SetPlayerStatus(PlayerUnit* pPlayer, int nStatus);
	// ����CLS4Web�ж�Ӧ�����ִ�
	int		SetPlayerAccountIDInCLS4Web(PlayerUnit* pPlayer);

	// �û���LOGIN״̬�µ�������
	int		ClearPlayerAfterLogin(PlayerUnit* pPlayer);

	// ���ʺ���תΪСд(�����Ҫ�Ļ�)
	char*	AccountLwr(char* szName);

	// �������ݿ�ָ��
	int		DealDBCmd(P_DBS4WEB_CMD_T* pBaseCmd, size_t nDSize);

	//////////////////////////////////////////////////////////////////////////
	// ����ָ����û�
	//////////////////////////////////////////////////////////////////////////
	int		SendCmdToClient(int nClientID, const void* pCmd, size_t nSize);
	int		SendCmdToMultiClient(int* aClientID, int nClientNum, const void* pCmd, size_t nSize);
	int		SendCmdToClientWithTag(unsigned char nTagIdx, short nTagVal, const void* pCmd, size_t nSize);
	int		SendCmdToClientWithTag64(whuint64 nTag, const void* pCmd, size_t nSize);
	int		SendCmdToAllClient(const void* pCmd, size_t nSize);
	int		SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal);
	int		SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel);

	int		SendAlliInstanceNotify(tty_id_t nInstanceID, tty_id_t nAccountID, int nAction);

	// ���������������õ�m_vectrawbuf
	void	SendPlayerCharAtbToClient(PlayerData* pPlayer, bool bSimple=true);
	void	RefreshPlayerCharAtbFromDB(tty_id_t nAccountID, bool bSimple=true, bool bCheckOnline=false);
	void	RefreshPlayerCharAtbFromDB(unsigned int nNum, tty_id_t* pAccountIDs, bool bSimple=true, bool bCheckOnline=false);
	void	RefreshPlayerCharAtbFromDB(const char* cszAccountIDs, bool bSimple=true, bool bCheckOnline=false);
	void	GetHeroSimpleData(unsigned int nCombatType, tty_id_t nExtID, const char* cszHeroIDs, unsigned int nReason); // ,�ָ���hero_id��
	void	GetHeroSimpleData(unsigned int nCombatType, tty_id_t nExtID, unsigned int nNum, tty_id_t*	pHeroIDs, unsigned int nReason);
	void	RefreshDrug(const char* cszAccountIDs); // ,�ָ���account_id��
	void	RefreshDrug(unsigned int nNum, tty_id_t* pAccountIDs);
	int		AddMail(tty_id_t nSenderID, const char*	cszSenderName, tty_id_t nReceiverID, bool bNeedRst, unsigned char nType, unsigned char nFlag, bool bSendLimit, int nTextLen, const char* cszText, int nExtDataLen, void* pExtData, int pAddedExtData[PNGSPACKET_2DB4WEB_EXT_NUM]);
	int		RechargeDiamond(tty_id_t nAccountID, unsigned int nAddedDiamond, bool bFromGS=false, const int* pExt=NULL);
	int		NotifyMeRefreshLevelRankOfPlayerCard(bool bLoadLevelRankOK)
	{
		m_bNeedUpdatePlayerCardLevelRankData	= true;
		m_bLoadCharLevelRankOK					= bLoadLevelRankOK;
		return 0;
	}
	// ��ֹע��ʽ����
	int		AntiSQLInjectionAttack(int nMaxLen, char* szStr);
	// �����Ƿ�Ϸ�
	bool	IsCharNameValid(const char* cszName);
	bool	IsAccountNameValid(const char* cszName);

	void	AddMoneyCostLog(tty_id_t nAccountID, unsigned int nUseType, unsigned int nMoneyType, unsigned int nMoney
		, unsigned int nExtData0=0, unsigned int nExtData1=0, unsigned int nExtData2=0, unsigned int nExtData3=0);
	void	AddGoldProduceLog(tty_id_t nAccountID, unsigned int nProduceType, unsigned int nGold);
	void	AddGoldCostLog(tty_id_t nAccountID, unsigned int nCostType, unsigned int nGold);

	void	GetPlayerCardFromDB(tty_id_t nAccountID);
	void	AddNotifyInWorldChat(unsigned int nTextLen, const char* cszText, tty_id_t nCharID=0);

	// ��ȡ�������
	unsigned char		GetSvrGroupIndex()
	{
		return (unsigned char)m_cfginfo.nSvrGrpID;
	}

	unsigned int	GetPlayerAppID(unsigned int nClientID)
	{
		PlayerUnit*	pPlayer	= GetPlayerByID(nClientID);
		if (pPlayer != NULL)
		{
			return pPlayer->nAppID;
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// �����ݿⷢ���첽ָ��
	//////////////////////////////////////////////////////////////////////////
	int		SendCmdToDB(const void* pData, size_t nSize);

	//////////////////////////////////////////////////////////////////////////
	// ������Ϣ��ȫ�ַ�����
	//////////////////////////////////////////////////////////////////////////
	int		SendCmdToGS(const void* pData, size_t nSize)
	{
		return m_msgerGS4Web.SendMsg(pData, nSize);
	}

	//////////////////////////////////////////////////////////////////////////
	// ʱ���¼����
	//////////////////////////////////////////////////////////////////////////
	// ʱ���¼�����
	int		Tick_DealTE();
	// ���ڴ����login ok״̬�ĳ�ʱ
	void	TEDeal_NoLogin_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	void	SetPlayerTE_NoLogin(PlayerUnit* pPlayer);
	// ���ڴ����û����ߵ���ʱ
	void	TEDeal_PlayerDropWait_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	void	SetPlayerTE_PlayerDropWait(PlayerUnit* pPlayer);
	// ���ڴ���رճ�ʱ
	void	TEDeal_PlayerCloseDelay_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	void	SetPlayerTE_PlayerCloseDelay(PlayerUnit* pPlayer, int nDelay);
	// ���ڷ���DBʱ���¼�����
	void	TEDeal_Deal_DB_TimeEvent_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	void	SetTE_DealDBTimeEvent();
	// ���ڷ������ֵʱ���¼�����
	void	TEDeal_Deal_Pay_TimeEvent_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	void	SetTE_DealPayTimeEvent();
	// ���ڷ���AdColonyʱ���¼�����
	void	TEDeal_Deal_AdColony_TimeEvent_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	void	SetTE_DealAdColonyTimeEvent();
	// ����ս������ĳ�ʱ
	void	TEDeal_Deal_GetAlliInstanceCombatResultEvent(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	void	SetTE_GetAlliInstanceCombatResultEvent();

	void	DealExcelTimeEvent();
	void	DealGoldMarketTimeEvent();

	//////////////////////////////////////////////////////////////////////////
	// ������ص�
	//////////////////////////////////////////////////////////////////////////
	// ���Լ��û�����
	int		TryReplacePlayer(int nOldClientID, const char* cszCharPassMD5, unsigned int nFromIP, bool& bHasPass);
	// ���µ��û��̳о��û�
	// ����,0->�ɹ��̳�,1->���벻��
	int		InheritDropWaitPlayer(int nNewClientID, int nOldClientID, unsigned char nSvrIdx, const char* cszCharPassMD5);
	// �Ѵ���dropwait����closewait״̬���û������(�û�ѡ���)
	int		CleanDropWaitPlayer(int nClientID);
	int		CleanDropWaitPlayer(PlayerUnit* pPlayer);
	// �ӳ��߳������û�
	int		WaitAndKickPlayer(PlayerUnit* pPlayer);
	// �������˳��Ե�¼��warning���û�
	int		SendReplaceWarningToPlayer(int nClientID, unsigned int nFromIP);
	
	//////////////////////////////////////////////////////////////////////////
	// �����Ϣ��ز���
	//////////////////////////////////////////////////////////////////////////
	// ͨ������ID�����Ҷ���ָ��
	inline	PlayerUnit*	GetPlayerByID(int nClientID)
	{
		return m_Players.getptr(nClientID);
	}
	inline	PlayerUnit* GetPlayerByAccountID(tty_id_t nAccountID)
	{
		int	nClientID	= 0;
		if (m_mapAccountID2ClientID.get(nAccountID, nClientID))
		{
			return m_Players.getptr(nClientID);
		}
		return NULL;
	}
	PlayerData*	GetPlayerData(tty_id_t nAccountID)
	{
		PlayerUnit*	pPlayer	= GetPlayerByAccountID(nAccountID);
		if (pPlayer != NULL)
		{
			return	m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pPlayer->nID);
		}
		return NULL;
	}

	// 
	// ֪ͨclientս�����
	void	SendMsgToClient(tty_id_t nAccountID, const void *pData, int nSize);

public:
	//////////////////////////////////////////////////////////////////////////
	// ����transaction�Ķ���
	//////////////////////////////////////////////////////////////////////////
	// LOGIN����
	class Transaction_Login : public whtransactionbasewithTQID
	{
	// Ϊ�ӿ�ʵ��
	public:
		virtual	~Transaction_Login();
		virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize);
		virtual	int		DealTimeEvent(int nEvent);
		WHTRANSACTION_DECLARE_GETTYPESTR(LP, Transaction_Login);
	// �Լ��õ�
	public:
		LPGamePlay4Web_i*	m_pHost;
		PlayerUnit*			m_pPlayer;								// ֻ����Init����,֮�󶼿���ʧЧ!!!
		int					m_nClientID;
		unsigned int		m_nIP;
		whint64				m_nAccountID;
		unsigned int		m_nDeviceType;
		char				m_szAccount[TTY_ACCOUNTNAME_LEN];
		char				m_szPassword[TTY_ACCOUNTPASSWORD_LEN];
		char				m_szDeviceID[TTY_DEVICE_ID_LEN];
		int					m_nPassLen;
		bool				m_bPlayerDropped;						// ��ʾ�û���;������
		int					m_nCurTE;								// ��ǰ��ʱ���¼�
		char				m_szRealDeviceID[TTY_DEVICE_ID_LEN];
	public:
		Transaction_Login(LPGamePlay4Web_i* pHost);
	private:
		int		DoNext_TTY_TC_INIT(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GLOBAL_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_CHAR_LOAD_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize);
		// ���ʹ����Ľ�����ͻ���(ֻ�г����˲����������)
		int		SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt="");
		int		ClearTryLoginPlayer_And_Done();
		int		ClearLoginPlayer_And_Done();
	private:
		enum
		{
			TE_NONE					= 0,
			TE_GLOBAL_LOGIN			= 1,
			TE_GROUP_LOGIN			= 2,
			TE_CHAR_LOAD			= 3,
		};
	};
	friend class Transaction_Login;

	// LOGIN����
	class Transaction_LoginByDeviceID : public whtransactionbasewithTQID
	{
		// Ϊ�ӿ�ʵ��
	public:
		virtual	~Transaction_LoginByDeviceID();
		virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize);
		virtual	int		DealTimeEvent(int nEvent);
		WHTRANSACTION_DECLARE_GETTYPESTR(LP, Transaction_LoginByDeviceID);
		// �Լ��õ�
	public:
		LPGamePlay4Web_i*	m_pHost;
		PlayerUnit*			m_pPlayer;								// ֻ����Init����,֮�󶼿���ʧЧ!!!
		int					m_nClientID;
		unsigned int		m_nIP;
		whint64				m_nAccountID;
		unsigned int		m_nDeviceType;
		char				m_szDeviceID[TTY_DEVICE_ID_LEN];
		bool				m_bPlayerDropped;						// ��ʾ�û���;������
		int					m_nCurTE;								// ��ǰ��ʱ���¼�
		char				m_szRealDeviceID[TTY_DEVICE_ID_LEN];
	public:
		Transaction_LoginByDeviceID(LPGamePlay4Web_i* pHost);
	private:
		int		DoNext_TTY_TC_INIT(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GLOBAL_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_CHAR_LOAD_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize);
		// ���ʹ����Ľ�����ͻ���(ֻ�г����˲����������)
		int		SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt="");
		int		ClearTryLoginPlayer_And_Done();
		int		ClearLoginPlayer_And_Done();
	private:
		enum
		{
			TE_NONE					= 0,
			TE_GLOBAL_LOGIN			= 1,
			TE_GROUP_LOGIN			= 2,
			TE_CHAR_LOAD			= 3,
		};
	};
	friend class Transaction_LoginByDeviceID;

	// CREATEACCOUNT����
	class Transaction_CreateAccount : public whtransactionbasewithTQID
	{
	// Ϊ�ӿ�ʵ��
	public:
		virtual	~Transaction_CreateAccount();
		virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize);
		virtual	int		DealTimeEvent(int nEvent);
		WHTRANSACTION_DECLARE_GETTYPESTR(LP, Transaction_CreateAccount);
	// �Լ��õ�
	public:
		LPGamePlay4Web_i*	m_pHost;
		int					m_nClientID;
		bool				m_bPlayerDropped;						// ��ʾ�û���;������
		char				m_szAccountName[TTY_ACCOUNTNAME_LEN];
		char				m_szPass[TTY_ACCOUNTPASSWORD_LEN];
		char				m_szDeviceID[TTY_DEVICE_ID_LEN];
		unsigned int		m_nDeviceType;
		bool				m_bUseRandomName;						// �Ƿ�ʹ���������
		unsigned int		m_nAppID;
	public:
		Transaction_CreateAccount(LPGamePlay4Web_i* pHost);
	private:
		int		DoNext_TTY_TC_INIT(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GLOBAL_ACCOUNT_CREATE(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize);
		int		SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt="");
		int		ClearPlayer_And_Done();
	};
	friend class Transaction_CreateAccount;

	// CREATECHAR����
	class Transaction_CreateChar : public whtransactionbasewithTQID
	{
	// Ϊ�ӿ�ʵ��
	public:
		virtual	~Transaction_CreateChar();
		virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize);
		virtual	int		DealTimeEvent(int nEvent);
		WHTRANSACTION_DECLARE_GETTYPESTR(LP, Transaction_CreateChar);
	// �Լ��õ�
	public:
		LPGamePlay4Web_i*	m_pHost;
		int					m_nClientID;
		tty_id_t			m_nAccountID;
		unsigned int		m_nPosX;
		unsigned int		m_nPosY;
		bool				m_bPlayerDropped;						// ��ʾ�û���;������
		bool				m_bCreateOK;							// �Ƿ��Ѿ���ɴ�����
		unsigned char		m_nBinded;
		unsigned int		m_nGiftAppID;
	public:
		Transaction_CreateChar(LPGamePlay4Web_i* pHost);
	private:
		int		DoNext_TTY_TC_INIT(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GROUP_CHAR_CREATE(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_CHAR_LOAD_RPL(const void* pData, size_t nDSize);
		int		DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize);
		int		SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt="");
		int		ClearPlayer_And_Done();
	};

	//////////////////////////////////////////////////////////////////////////
	// ��GS4Web������
	//////////////////////////////////////////////////////////////////////////
	tcpmsger::DATA_INI_INFO_T		m_msgerGS4Web_info;
	class MYMSGER_T : public tcpretrymsger<tcpmsger>
	{
	public:
		LPGamePlay4Web_i*	m_pHost;
	public:
		virtual	void	i_Worker_WORKING_Begin()
		{
			GS4WEB_FIRSTIN_T	Cmd;
			Cmd.nCmd			= P_GS4WEB_FIRSTIN;
			Cmd.nSubCmd			= 0;
			Cmd.nGroupID		= m_pHost->m_cfginfo.nSvrGrpID;
			SendMsg(&Cmd, sizeof(Cmd));
		}
	};
	friend	class			MYMSGER_T;
	MYMSGER_T				m_msgerGS4Web;

	//////////////////////////////////////////////////////////////////////////
	// GS4Web���
	//////////////////////////////////////////////////////////////////////////
	int			Tick_DealGS4WebMsg();
	int			_Tick_DealGS4WebMsg_DB_PRL(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize);
	int			_Tick_DealGS4WebMsg_Assist_PRL(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize);
	int			_Tick_DealGS4WebMsg_Group_REQ(GS4WEB_CMD_BASE_T* pBaseCmd, size_t nSize);
};
}
#endif
