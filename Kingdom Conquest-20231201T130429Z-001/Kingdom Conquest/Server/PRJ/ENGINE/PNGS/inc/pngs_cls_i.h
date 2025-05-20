// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CLSģ��
//                PNGS��Pixel Network Game Structure����д
//                CLS��Connection Load Server����д�������ӷ��书���еĸ��ط�����
//                ���ļ���CLS���ڲ�ʵ��
//                CLS�Ϳͻ���ͨѶʹ��cnl2���ͷ�������ͨѶʹ��TCP��
//                �û�Tagʹ��idx+val��hash��ʵ�֣�ͬtag+val�ļ����ٶȱȾɵ�CLSҪ�졣
// CreationDate : 2005-07-22
// ChangeLOG    : 2006-12-19 ������szFakeWelcomeAddr

#ifndef	__PNGS_CLS_I_H__
#define	__PNGS_CLS_I_H__
#define GZSTICKFUNC 15
#include "./pngs_cls.h"
#include "./pngs_packet.h"
#include <WHNET/inc/whnetudpGLogger.h>
#include <WHNET/inc/whnetcnl2.h>
#include <WHNET/inc/whnettcp.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whlist.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whdbg.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

class	CLS2_I		: public CLS2
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
// �Լ�ʹ�õ�
public:
	CLS2_I();
	~CLS2_I();
private:
	// ���ֽṹ����
	// ��ʼ������
	struct	CFGINFO_T		: public whdataini::obj
	{
		char				szWelcomeAddr[WHNET_MAXADDRSTRLEN];		// ���û������ӵķ�������ַ
		char				szFakeWelcomeAddr[WHNET_MAXADDRSTRLEN];	// ��ٵĻ�ӭ��ַ������ǿ������������û���һ������ת�����Ի��ߴ���
																	// ��ַ�ĸ�ʽΪ��IP:deltaport����
																	// ���硰IP2:0���ͱ�ʾ�õ��Լ�ȷ�а󶨵ĵ�ַ���޸�IPΪIP2���˿ڲ���
																	// ���硰:1000���ͱ�ʾ�õ��Լ�ȷ�а󶨵ĵ�ַ��IP���䣬�˿ڶ�����1000
		char				szGMSAddr[WHNET_MAXADDRSTRLEN];			// GMS�ȴ��������������ӵ�ַ(���з���������GMS�ĵ�ַ�������)
		unsigned char		nNetworkType;							// һ������Ҳ�����Ƕ�Ӧ���ɲ�ͬ������ģ����磬һЩCLS��Ӧ��ͨ��һЩCLS��Ӧ���ţ�
		bool				bCanAcceptPlayerWhenCLSConnectingGZS;	// ��CLS����GZS�Ĺ����������û�����
		bool				bLogSvrCmd;								// �Ƿ��¼������������ָ��
		int					nGroupID;								// ����Ӧ��CAAFS��Ӧ
		int					nTQChunkSize;							// ʱ����е�ÿ�δ��Է�����С
		int					nConnectOtherSvrTimeOut;				// ����GMS��GZS������Server�ĳ�ʱ
		int					nCloseFromGMSTimeOut;					// ֪ͨGMS�Լ�Ҫ�˳��󣬵ȴ�GMS�ر����ӵĳ�ʱ
		int					nHelloTimeOut;							// �ȴ����ӷ���Hello����Hi�ĳ�ʱ
		int					nClientConnectToMeTimeOut;				// �û�����CLS�ĳ�ʱ(��ô��ʱ��û�����ӳɹ��ͳ�ʱ)
		int					nSlotInOutNum;							// ����slot��In/Out������һ����
		int					nCmdStatInterval;						// ����ָ��ͳ�Ƶļ�������Ϊ0���ʾ����ͳ�ƣ�
		//
		char				szSelfInfoToClient[256];				// �û����ӻ��ȿ�������һ�仰���������ȽϿ������Ƿ���ȷ
		// �����
		int					nMaxRecvCount;							// һ��ʱ�������ɽ��յİ���
		int					nMaxRecvSize;							// һ��ʱ�������ɽ��յ�������
		int					nCheckRecvInterval;						// ��������������ļ��
		int					nBigTickFilter;							// BIGtick log����ķ�ֵ
		CFGINFO_T()
		: nNetworkType(NETWORKTYPE_UNKNOWN)
		, bCanAcceptPlayerWhenCLSConnectingGZS(false)
		, bLogSvrCmd(false)
		, nGroupID(0)
		, nTQChunkSize(1024)
		, nConnectOtherSvrTimeOut(PNGS_DFT_SVRCONNECT_TIMEOUT)		// �����һЩ��Ϊ���·�������ʼ�����ܻ��˷�һЩʱ��
		, nCloseFromGMSTimeOut(10*1000)
		, nHelloTimeOut(PNGS_DFT_HELLOW_TIMEOUT)
		, nClientConnectToMeTimeOut(30000)
		, nSlotInOutNum(PNGS_SLOTINOUTNUM)
		, nCmdStatInterval(20*1000)
		, nMaxRecvCount(5*60*3)										// ƽ��ÿ�벻Ӧ�ó���3����
		, nMaxRecvSize(5*60*2*1024)									// �ٶ����ÿ��2k�����ݰ�
		, nCheckRecvInterval(5*60*1000)								// Ĭ��5����ͳ��һ��
		, nBigTickFilter(150)
		{
			strcpy(szWelcomeAddr, "localhost:15901,15910");
			szFakeWelcomeAddr[0]	= 0;
			sprintf(szGMSAddr, "localhost:%d", GMS_DFT_PORT);
			strcpy(szSelfInfoToClient, "BO2 CLS2");
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// ��Ӧһ��GZS����
	struct	GZSUnit
	{
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_CONNECTING			= 1,						// �������Ӹ÷������Ĺ����У������ʱ��û����������DROP���û���
			STATUS_SHAKEHAND			= 2,						// Hello/Hi��
			STATUS_WORKING				= 3,						// ���ӳɹ�������ͨѶ��
		};
		int					nStatus;								// ��ǰ״̬
		unsigned char		nSvrIdx;								// GZS���������(1~GZS_MAX_NUM-1)
		SOCKET				sockGZS;								// ����GZS��socket
		whtick_t			tickConnectGZSTimeOut;					// ����GZS��ʱ��ʱ��
																	// ��ΪDoAllGZSRecv�з���ҲҪ��ѯһ��GZS�����Ծ��������������жϳ�ʱ��
		tcpmsger			msgerGZS;								// ����GZS��msger

		GZSUnit()
		: nStatus(STATUS_NOTHING)
		, nSvrIdx(0)												// 0���ʾ��û�ж�Ӧ������GZS��
		, sockGZS(INVALID_SOCKET)
		, tickConnectGZSTimeOut(0)
		{
		}

		void	clear()
		{
			nStatus		= STATUS_NOTHING;
			nSvrIdx		= 0;
			cmn_safeclosesocket(sockGZS);							// Ϊ�˱��չر�һ��
			tickConnectGZSTimeOut	= 0;
			msgerGZS.Release();										// Ϊ�˱����ͷ�һ��
		}
	};
	// ���
	struct	PlayerUnit;
	typedef	whDList<PlayerUnit *>		DLPLAYER_T;
	typedef	DLPLAYER_T::node			DLPLAYER_NODE_T;
	struct	PlayerUnit
	{
		enum
		{
			MAX_TAG_NUM					= 16,						// �����Ա�ǵ�tag����
		};
		enum
		{
			STATUS_NOTHING				= 0,
			STATUS_CONNECTING			= 1,						// �������ӹ����У������ʱ��û����������DROP���û���
			STATUS_WORKING				= 2,						// ���ӳɹ�������ͨѶ��
		};
		int					nID;									// ��Players�����е�ID������Ǹ���GMS�е�ID�����ģ�
		char				szName[64];								// ���ֱ�־��������ϲ����ã����磺�˺�.��ɫ����Ҫ��Ϊ�˹ؼ�����־�ܹ���Ӧ���û��������ڽ�����ѯ��
		int					nStatus;								// ��ǰ״̬
		int					nSlot;									// ��Ӧ��ͨѶslot��
		unsigned int		nPasswordToConnectCLS;					// ����CLS�õ����루0��ʾû�����룩
		unsigned char		nRemoveReason;							// CLS_GMS_CLIENT_DROP_T::REMOVEREASON_XXX
		unsigned char		nProp;									// ���ԡ�����ΪPNGS_CLS_PLAYER_PROP_LOG_XXX����pngs_def.h��
		unsigned char		nSvrIdx;								// ��Ӧ��GZS���
																	// �����0���ʾ��û�н���GZS��������GMS�У�
																	// ��ĳ��GZS��CLS���ߵ�ʱ��������ݿ��������߳����к�GZS��ص��û�
																	// !!!!����Ǳ���ģ���Ϊ����Ҫ���û�֪���Լ����ڴ����Ǹ�GZS
		int					nRecvCount;								// ��һ��ʱ�����յ��İ������������ڷ���ң�
		int					nRecvSize;								// ��һ��ʱ�����յ��������������ڷ���ң�
		whtimequeue::ID_T	teid;									// Ŀǰ��;���û����볬ʱ��ÿ��һ��ʱ����һ���Ƿ��͹���İ�

		struct	TAGINFO_T
		{
			short			nValue;									// ��Ӧ��tagֵ(0��ʾ�����κζ�����)
			DLPLAYER_NODE_T	nodeTag;								// ����Ӧ��tag�����еĽڵ�

			TAGINFO_T()
			: nValue(0)
			{
			}

			void	clear()
			{
				nValue	= 0;
				nodeTag.leave();
			}
		};
		TAGINFO_T			taginfo[MAX_TAG_NUM];
		whhash<whuint64, DLPLAYER_NODE_T *>	map64Tag2Node;

		PlayerUnit()
		: nID(0)
		, nStatus(STATUS_NOTHING)
		, nSlot(0)
		, nPasswordToConnectCLS(0)
		, nRemoveReason(CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTHING)
		, nProp(0)
		, nSvrIdx(0)
		{
			szName[0]	= 0;
			for(int i=0;i<MAX_TAG_NUM;i++)
			{
				taginfo[i].nodeTag.data	= this;
			}
		}
		void	clear(CLS2_I *pCLS)
		{
			int	i;
			for(i=0;i<MAX_TAG_NUM;i++)
			{
				pCLS->DelPlayerFromTagList(this, i);
			}
			whhash<whuint64, DLPLAYER_NODE_T *>::kv_iterator it=map64Tag2Node.begin();
			while(it!=map64Tag2Node.end())
			{
				whuint64	nTag	= it.getkey();
				++it;
				pCLS->DelPlayerFrom64TagList(this, nTag);
			}

			teid.quit();

			nID			= 0;
			nSvrIdx		= 0;
			nStatus		= STATUS_NOTHING;
			nSlot		= 0;
			nPasswordToConnectCLS	= 0;
			nRemoveReason			= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTHING;
			nProp		= 0;
			szName[0]	= 0;
			nRecvCount	= 0;
			nRecvSize	= 0;
		}
		void	ReadFromOther(PlayerUnit *pOther)
		{
			//nID				= pOther->nID;	�������
			memcpy(szName, pOther->szName, sizeof(szName));
			nStatus			= pOther->nStatus;
			nSlot			= pOther->nSlot;
			nPasswordToConnectCLS	= pOther->nPasswordToConnectCLS;
			nRemoveReason	= pOther->nRemoveReason;
			nProp			= pOther->nProp;
			nSvrIdx			= pOther->nSvrIdx;
			teid.quit();	// ������ö�ȡ���˵�
			for(int i=0;i<MAX_TAG_NUM;i++)
			{
				if( pOther->taginfo[i].nodeTag.isinlist() )
				{
					taginfo[i].nodeTag.insertbefore(&pOther->taginfo[i].nodeTag);
				}
			}
		}
	};
	struct	TQUNIT_T;
	typedef	void (CLS2_I::*TEDEAL_T)(TQUNIT_T *);
	struct	TQUNIT_T
	{
		TEDEAL_T	tefunc;											// ������¼��ĺ���
		// ���ݲ���
		union
		{
			struct	PLAYER_T										// ��Ҷ�ʱ�¼�
			{
				int	nID;											// ���ID
			}player;
			struct	GZS_T											// GZS��صĶ�ʱʱ�䣨�磺��
			{
				int	nID;											// 
			}gzs;
		}un;
	};
	struct	DLPUNIT_T
	{
		DLPLAYER_T	*pDLP;

		DLPUNIT_T()
		: pDLP(NULL)
		{
		}

		void	clear(CLS2_I *pCLS)
		{
			// SafeDeleteDLP������Զ��ж�pDLP�Ƿ�Ϊ��
			pCLS->SafeDeleteDLP(pDLP);
		}
	};
private:
	friend	struct	PlayerUnit;
	friend	struct	DLPUNIT_T;
	// ���ýṹ
	CFGINFO_T							m_cfginfo;					// CLS������
	tcpmsger::DATA_INI_INFO_T			m_MSGER_INFO;				// ��GMS�Լ�GZS���ӵ�msger������
	CNL2SlotMan::DATA_INI_INFO_T		m_CLIENT_SLOTMAN_INFO;		// �ȴ��ͻ������ӵ�slotman������
	// ��ǰ����״̬
	int									m_nStatus;					// CLS2::STATUS_XXX
	// ��ǰ��ʱ��
	whtick_t							m_tickNow;
	// �Լ���GMS�е�ID����������ʱ�����ã�
	int									m_nCLSID;
	// ���ܹ���
	ICryptFactory						*m_pICryptFactory;
	// ��Client�����ӹ���
	CNL2SlotMan							*m_pSlotMan4Client;
	// ʱ�����
	// ���磺��������״̬���û��೤ʱ�������û�����ӳɹ������
	whtimequeue							m_TQ;
	// ����������
	GZSUnit								m_GZSs[GZS_MAX_NUM];
	// �������
	whunitallocatorFixed<PlayerUnit>	m_Players;
	// ����GMS��msger
	class	MYMSGER_T	: public tcpretrymsger<tcpmsger>
	{
	public:
		CLS2_I			*m_pHost;
		whtick_t		m_tickWorkBegin;
	public:
		virtual void	i_Worker_TRYCONNECT_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
		}
		virtual void	i_Worker_WORKING_Begin()
		{
			m_tickWorkBegin	= wh_gettickcount();
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	friend class MYMSGER_T;
	MYMSGER_T							m_msgerGMS;
	// ����ɾ�������GZS��Player
	whvector<int>						m_vectIDToDestroy;
	// tag����Ӧ��DList��ӳ��
	whhash<unsigned int, DLPUNIT_T>		m_mapTag2DL;
	// 64λ��ӳ��
	whhash<whuint64, DLPUNIT_T>			m_map64Tag2DL;
	// ��ʱ��װָ���GMS��GZS����Client��
	whvector<char>						m_vectrawbuf;
	// �����ϲ�Ӧ���˳�
	bool								m_bShouldStop;
	// ��ǰ����ָ���GZS�������������������
	unsigned char						m_nCurCmdFromSvrIdx;
	// ����ͳ��ָ���Ƶ��
	struct	CMDSTAT_T
	{
		unsigned int	nByte;			// �ۼ��ֽ���
		unsigned int	nCount;			// �ۼ�����
		inline bool	operator < (const CMDSTAT_T &other) const
		{
			if( nByte != other.nByte )	return	nByte > other.nByte;
			return	nCount>other.nCount;
		}
	};
	whvector<CMDSTAT_T>					m_vectCmdStat;
	// ���ڶ�ʱ������log
	whlooper							m_loopCmdStat;

	// Ϊ�˲���CLSΪʲô����ԭ��
//	dbgtimeuse							m_dtu;
	unsigned int									m_nGZScmdCount;
	unsigned int									m_GZStickFuncCount[GZSTICKFUNC];
	unsigned int									m_GZStickFuncTime[GZSTICKFUNC];
private:
	// ���õ�ǰ״̬
	void	SetStatus(int nStatus);
	// �Ƴ�Player
	int		RemovePlayerUnit(int nPlayerID);
	int		RemovePlayerUnit(PlayerUnit *pPlayer);
	void	RemoveAllPlayerOfGZS(unsigned char nSvrIdx);
	// ͨ��Slot���Player
	PlayerUnit *	GetPlayerBySlot(int nSlot);
	// ���Ի�ȡ��ң���������ھ���GMS�����û����ߵ�֪ͨ
	PlayerUnit *	TryGetPlayerAndSendNotifyToGMSIsNotExisted(int nPlayerID);
	// �Ƴ�GZS
	int		RemoveGZSUnit(int nSvrIdx);
	int		RemoveGZSUnit(GZSUnit *pGZS);
	// �򵥸���ҷ�������(������Ҫ�����ݺϳ�ΪCLS_CLIENT_DATA_T����ʽ��Ȼ�����SendRawCmdToSinglePlayer)
	int		SendDataToSinglePlayer(PlayerUnit *pPlayer, unsigned char nChannel, void *pData, int nSize, pngs_cmd_t nSendCmd);
	// �򵥸���ҷ��Ͳ������κϳɵĴ�ָ��
	int		SendRawCmdToSinglePlayer(int nPlayerID, unsigned char nChannel, void *pRawCmd, int nSize);
	int		SendRawCmdToSinglePlayer(PlayerUnit *pPlayer, unsigned char nChannel, void *pRawCmd, int nSize);
	// ��������ҷ��Ͳ������κϳɵĴ�ָ��
	int		SendRawCmdToAllPlayer(unsigned char nChannel, void *pRawCmd, int nSize);
	// �����к�ĳ��GZS��ص���ҷ��Ͳ������κϳɵĴ�ָ��
	int		SendRawCmdToAllPlayerOfGZS(unsigned char nSvrIdx, unsigned char nChannel, void *pRawCmd, int nSize);
	// ����idx��val�ϳ�һ��hash��ֵ
	inline	unsigned int	CreateHashKey(unsigned char nTagIdx, short nTagVal)
	{
		return	(((unsigned int)nTagIdx) << 16) | nTagVal;
	}
	// ��hash��ֵ�еõ�idx��val
	inline	void	GetIdxAndVal(unsigned int nHashKey, unsigned char *pnTagIdx, short *pnTagVal)
	{
		*pnTagIdx	= (unsigned char)((nHashKey & 0xFF0000) >> 16);
		*pnTagVal	= (short)(nHashKey & 0xFFFF);
	}
	// ����Ҽ���ĳ��tag������
	int		AddPlayerToTagList(PlayerUnit *pPlayer, unsigned char nTagIdx, short nTagVal);
	// ����Ҵ�ĳ��tag�������Ƴ���
	int		DelPlayerFromTagList(PlayerUnit *pPlayer, unsigned char nTagIdx);
	// ����Ҽ���ĳ��tag64������
	int		AddPlayerToTag64List(PlayerUnit *pPlayer, whuint64 nTag);
	// ����Ҵ�ĳ��tag64�������Ƴ���
	int		DelPlayerFrom64TagList(PlayerUnit *pPlayer, whuint64 nTag);

	// Newһ��DLPLAYER_T����
	DLPLAYER_T *	NewDLP();
	// Deleteһ��DLPLAYER_T����
	void	SafeDeleteDLP(DLPLAYER_T *&pDLP);
	// Init�Ӻ���
	int		Init_CFG(const char *cszCFG);
	// ���GZS������socket
	int		GetGZSSockets(whvector<SOCKET> &vect);
	// ��GZS����ָ��
	int		DoAllGZSSend();
	// ����TickWork
	int		Tick_DealGMSMsg();
	int		Tick_DealGMSMsg_GMS_CLS_GZSINFO(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CLS_CLIENTWANTCLS(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CLS_ROUTECLIENTDATATOGZS(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_GMS_CLS_PLAYER_CHANGEID(void *pCmd, int nSize);
	int		Tick_DealGMSMsg_SVR_CLS_CLIENT_DATA(void *pCmd, int nSize);
	int		Tick_DealGZSMsg();										// ���ղ�����GZS��������Ϣ(ͬʱ�ж���GZS�����ӵ�״��)
	int		Tick_DealClientMsg();
	int		Tick_DealClientMsg_CONTROL_OUT_SLOT_ACCEPTED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize);
	int		Tick_DealClientMsg_CONTROL_OUT_SLOT_CLOSED(CNL2SlotMan::CONTROL_T *pCOCmd, int nSize);
	int		Tick_DealClientMsg_CLIENT_CLS_DATA(pngs_cmd_t nCmd, PlayerUnit *pPlayer, int nChannel, pngs_cmd_t *pCmd, int nSize);
	int		Tick_DealTE();
	// �������������ݽ���
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_CTRL(GZSUnit *pGZS, void *pCmd, int nSize);	// ���pGZSΪ���������GMS
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG_TO_CLIENT(void *pCmd, int nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_SET_PROP_TO_CLIENT(void *pCmd, int nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_ALL_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG64_TO_CLIENT(void *pCmd, int nSize);
	int		Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED64_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd);

	// �������ɹ�ʱ��Ҫ���õĺ���
	void	Retry_Worker_WORKING_Begin();

	// ʱ���¼�����
	// �ͻ������ӳ�ʱ
	void	TEDeal_nClientConnectToMeTimeOut(TQUNIT_T *pTQUnit);
	// ���������
	void	SetTEDeal_nClientConnectToMeTimeOut(PlayerUnit *pPlayer);
	// ��ʱ����û������İ���Ƶ��
	void	TEDeal_nCheckRecvInterval(TQUNIT_T *pTQUnit);
	// ���������
	void	SetTEDeal_nCheckRecvInterval(PlayerUnit *pPlayer);

};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CLS_I_H__
