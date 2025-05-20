#ifndef __GameMngS_H__
#define __GameMngS_H__

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whvector.h"
#include "WHCMN/inc/whtime.h"
#include "WHCMN/inc/whcmd.h"
#include "WHCMN/inc/whtimequeue.h"

#include "WHNET/inc/whnetudpGLogger.h"

#include "GameUtilityLib/inc/SoloFileFunc.h"

#include "../../Common/inc/pngs_packet_lp_logic.h"
#include "../../Common/inc/tty_lp_client_packet.h"
#include "../../Common/inc/tty_lp_client_gamecmd_packet.h"
#include "../../Common/inc/tty_game_def.h"
#include "../../Common/inc/tty_def_dbs4web.h"

#include "LPNet.h"
#include "PlayerMngS.h"
#include "NotificationMngS.h"
#include "PlayerCardMngS.h"
#include "WorldMngS.h"
#include "AllianceMngS.h"
#include "ExcelTextMngS.h"
#include "ChatMngS.h"
#include "LotteryMngS.h"
#include "RankMngS.h"
#include "InstanceMngS.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class CPlayerMngS;
class CNotificationMngS;
class CPlayerCardMngS;
class CWorldMngS;
class CAllianceMngS;
class CChatMngS;
class CLotteryMngS;
class CRankMngS;
class CInstanceMngS;

class CGameMngS
{
public:
	struct CFGINFO_T  : public whdataini::obj
	{
		int		nMaxPlayer;
		int		nGroupPlayerTotalNumExpect;				// �����Ĵ����������Ŀ(Ĭ��1w�Ͳ����,��Ҫ��Ϊ��һЩ��ҪԤ���������Ϣ׼��)
		int		nTQChunkSize;							// ʱ�����һ�η��䵥Ԫ��
		int		nNotificationSendToClientNum;			// ������ô��Ĺ�����ͻ���
		int		nWorldXMax;								// ����������X���ֵ
		int		nWorldYMax;								// ����������Y���ֵ
		int		nAllianceNumExpect;						// ��������������,������
		int		nStoredChatEntryNum;					// ������������
		int		nSendChatEntryNum;						// ���͵���������
		char	szMBDir[WH_MAX_PATH];					// mb�ļ���·��
		char	szRankFileDir[WH_MAX_PATH];				// ���а񻺴��ļ���ַ
		int		nRefreshMailTime;						// ˢ���ʼ���ʱ

		CFGINFO_T()
		: nMaxPlayer(100)
		, nGroupPlayerTotalNumExpect(10000)
		, nTQChunkSize(100)
		, nNotificationSendToClientNum(10)
		, nWorldXMax(200)
		, nWorldYMax(200)
		, nAllianceNumExpect(1000)
		, nStoredChatEntryNum(20)
		, nSendChatEntryNum(20)
		, nRefreshMailTime(1*60*1000)
		{
			WH_STRNCPY0(szMBDir, "../excel_tables/");
			WH_STRNCPY0(szRankFileDir, "rank_file/");
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
public:
	// ��ʼ��
	int		Init(CFGINFO_T* pInfo);
	// ��������CLPNet����Ϣ
	void	Receive(int nCmd, const void* pData, size_t nDSize);
	// �߼�tick
	void	Logic();
public:
	inline const CFGINFO_T* GetCfgInfo()
	{
		return m_pInfo;
	}
	inline CPlayerMngS*	GetPlayerMng()
	{
		return m_pPlayerMng;
	}
	inline	CNotificationMngS*	GetNotificactionMng()
	{
		return m_pNotificationMng;
	}
	inline	CPlayerCardMngS*	GetPlayerCardMng()
	{
		return m_pPlayerCardMng;
	}
	inline CWorldMngS*	GetWorldMng()
	{
		return m_pWorldMng;
	}
	inline CAllianceMngS*	GetAllianceMng()
	{
		return m_pAllianceMng;
	}
	inline CExcelTextMngS* GetExcelTextMngS()
	{
		return m_pExcelTextMng;
	}
	inline CChatMngS* GetChatMng()
	{
		return m_pChatMng;
	}
	inline CLotteryMngS* GetLotteryMng()
	{
		return m_pLotteryMng;
	}
	inline CRankMngS* GetRankMng()
	{
		return m_pRankMng;
	}
	inline CInstanceMngS* GetInstanceMng()
	{
		return m_pInstanceMng;
	}
	inline void	LoadAllianceFromDB(tty_id_t nAllianceID)
	{
		P_DBS4WEB_LOAD_ALLIANCE_T	Cmd;
		Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
		Cmd.nSubCmd			= CMDID_LOAD_ALLIANCE_REQ;
		Cmd.nAllianceID		= nAllianceID;

		g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
	}
	int		ReloadExcelTable();
public:
	int		SendMailListToClient(int nClientID);
	int		SendPersonalRelationToClient(int nClientID, unsigned char nType);
	int		PlayerOffline(int nClientID);
	int		AddPopulationProductionTE(tty_id_t nAccountID);
	int		AddEnemy(tty_id_t nAccountID, tty_id_t nEnemyID);
	int		AddAllianceDevelopment(tty_id_t nAllianceID, int nDeltaDevelopment);

	// ս������
	// Ⱥս
	int		CombatCalc_Raid(CombatUnit *aA, CombatUnit *aD, P_DBS4WEBUSER_GET_COMBAT_T *pData);
	// Ⱥս���ι�������
	int		CombatCalc_Raid_Round(CombatUnit *pA, CombatUnit *pD, int &nProfResA, int &nProfResD);
	// ֪ͨdb�����佫����
	int		UpdateHeroToDB(tty_id_t nHeroID, int nArmyNum);

	inline	void	LogMail(PlayerData* pPlayer, unsigned int nMailID)
	{		
		whlist<PrivateMailUnit*>::iterator	it		= pPlayer->m_MailList.begin();
		for (; it!=pPlayer->m_MailList.end(); ++it)
		{
			PrivateMailUnit*	pMail		= *it;
			if (pMail->nMailID == nMailID)
			{
				int	nExtDataLen	= pMail->GetExtDataLen();
				char	cEnd	= '\0';
				whvector<char>	vectBuf1;
				whvector<char>	vectBuf2;
				vectBuf1.reserve(pMail->nTextLen+1);
				vectBuf2.reserve(nExtDataLen+1);
				memcpy(vectBuf1.getbuf(), pMail->szText, pMail->nTextLen);
				memcpy(vectBuf2.getbuf(), pMail->GetExtData(), nExtDataLen);
				vectBuf1.resize(pMail->nTextLen);
				vectBuf2.resize(nExtDataLen);
				vectBuf1.push_back(cEnd);
				vectBuf2.push_back(cEnd);
				GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,MAIL)"%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%u,[%s],[%s]"
					, pMail->nMailID, pMail->nType, pMail->nFlag, pPlayer->m_CharAtb.nAccountID, pMail->nSenderID
					, (int)pMail->bReaded, pMail->nTime, vectBuf1.getbuf(), vectBuf2.getbuf());
			}
			break;
		}
	}
public:
	CGameMngS();
	virtual	~CGameMngS();
private:
	// ���ֹ�����ָ��
	CPlayerMngS*		m_pPlayerMng;
	CNotificationMngS*	m_pNotificationMng;
	CPlayerCardMngS*	m_pPlayerCardMng;
	CWorldMngS*			m_pWorldMng;
	CAllianceMngS*		m_pAllianceMng;
	CExcelTextMngS*		m_pExcelTextMng;
	CChatMngS*			m_pChatMng;
	CLotteryMngS*		m_pLotteryMng;
	CRankMngS*			m_pRankMng;
	CInstanceMngS*		m_pInstanceMng;
	// ��ʱƴ��ָ��
	whvector<char>	m_vectrawbuf;
	CFGINFO_T*		m_pInfo;
	whvector<char>	m_vectrawbuf_terrain;
	whvector<char>	m_vectrawbuf_city;
	whvector<char>	m_vectrawbuf_worldres;
	// ʱ�����
	whtimequeue		m_TQ;
	whtick_t		m_tickNow;

public:
	ArmyDesc		m_army[5+1];

public:
	//////////////////////////////////////////////////////////////////////////
	// ʱ���¼����
	//////////////////////////////////////////////////////////////////////////
	struct TQUNIT_T
	{
		typedef	void	(CGameMngS::*TEDEAL_T)(TQUNIT_T*);
		TEDEAL_T	tefunc;								// �����¼��ĺ���
		// ���ݲ���
		union
		{
			tty_id_t	nI64;
			int			nI32;
			void*		pPtr;
		}un;
	};
	// ʱ���¼�����
	int		Tick_DealTE();
};
}

#endif
