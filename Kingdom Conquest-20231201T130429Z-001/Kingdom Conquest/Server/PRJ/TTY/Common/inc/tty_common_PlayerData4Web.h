// �м�,���紫�������һ��Ҫ1�ֽڶ���,��Ȼƽ̨���ͬһ�ṹ�Ĵ�С�ǲ�һ�µ�
#ifndef __tty_common_PlayerData4Web_H__
#define __tty_common_PlayerData4Web_H__

#include "GameUtilityLib/inc/DMtype.h"

#include "WHCMN/inc/whlist.h"
#include "WHCMN/inc/whtime.h"

#include "tty_common_def.h"
#include "pngs_packet_db4web_logic.h"
#include "pngs_packet_web_extension.h"
#include "tty_common_excel_def.h"

using namespace n_whcmn;

namespace n_pngs
{
#pragma pack(1)
struct Position_T 
{
	unsigned int	nPosX;
	unsigned int	nPosY;

	Position_T(unsigned int _nPosX=0, unsigned int _nPosY=0)
		: nPosX(_nPosX),nPosY(_nPosY) 
	{

	}
};
// �����Ļ�������
struct CharAtb 
{
	// common_characters����������Ϣ
	tty_id_t		nAccountID;								// �˺�OnlyID
	char			szName[TTY_CHARACTERNAME_LEN];			// ��ɫ����
	unsigned int	nLastLoginTime;							// ��¼ʱ��
	unsigned int	nLastLogoutTime;						// �ǳ�ʱ��
	unsigned int	nExp;									// ��ǰ����ֵ
	unsigned int	nLevel;									// ��ǰ�ȼ�
	unsigned int	nDiamond;								// ��ʯ
	unsigned int	nCrystal;								// ˮ��
	unsigned int	nVip;									// Vip����
	unsigned int	nSex;									// �Ա�
	unsigned int	nHeadID;								// ͷ��ID
	unsigned int	nPosX;									// ����X����
	unsigned int	nPosY;									// ����Y����
	unsigned int	nDevelopment;							// ��չ��
	unsigned int	nGold;									// �ƽ�
	unsigned int	nPopulation;							// �˿�
	unsigned int	nBuildNum;								// �������
	unsigned int	nAddedBuildNum;							// ���ӽ������

	// ������������
	tty_id_t		nAllianceID;							// ����ID
	char			szAllianceName[TTY_ALLIANCENAME_LEN];	// ��������

	unsigned int	nDrug;									// ��ҩ
	unsigned int	nFreeDrawLotteryNum;					// ��ѳ齱ʣ�����
	unsigned int	nTradeNum;								// ����ʣ������̴���

	char			szSignature[TTY_CHARACTER_SIGNATURE_LEN];		// ����ǩ��

	unsigned int	nChangeNameNum;							// ��������

	unsigned int	nProtectTime;							// ������ʣ��ʱ��
	unsigned int	nProtectTimeEndTick;					// ��������ʱ��

	unsigned int	nAddBuildNumTime;						// ����֮��ʣ��ʱ��
	unsigned int	nAddBuildNumTimeEndTick;				// ����֮�鵽��ʱ��

	unsigned int	nTotalBuildNum;							// �ܹ��Ľ�������
	unsigned int	nNotificationID;						// ����Ķ��˵�֪ͨID

	unsigned int	nAllianceDrawLotteryNum;				// ���˳齱����ʣ�����
	unsigned int	nTotalDiamond;							// �ܳ�ֵ��ʯ

	unsigned int	nInstanceWangzhe;						// ����֮·��ǰ���ؿ�
	unsigned int	nInstanceZhengzhan;						// ��ս���µ�ǰ���ؿ�

	unsigned int	nCup;									// ������
	bool			bBinded;								// �Ƿ��Ѿ����˺�
	bool			bVipDisplay;							// �Ƿ���ʾvip
};
// �����Ƭ(����Ϣ)
struct PlayerCard 
{
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nVip;
	unsigned int	nSex;
	unsigned int	nHeadID;
	unsigned int	nPosX;
	unsigned int	nPosY;

	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];

	unsigned int	nDevelopment;
	char			szSignature[TTY_CHARACTER_SIGNATURE_LEN];		// ����ǩ��

	unsigned int	nProtectTime;							// ������ʣ��ʱ��
	unsigned int	nLevelRank;										// �����ȼ�����
	unsigned int	nCityLevel;										// ���еȼ�
	unsigned int	nProtectTimeEndTick;							// ��������ʱ��

	unsigned int	nInstanceWangzhe;						// ����֮·��ǰ���ؿ�
	unsigned int	nInstanceZhengzhan;						// ��ս���µ�ǰ���ؿ�

	unsigned int	nCup;									// ������
	bool			bVipDisplay;							// �Ƿ���ʾvip
};
// ������Ƭ
struct CityCard
{
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nVip;
	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
	unsigned int	nPosX;
	unsigned int	nPosY;
	unsigned int	nProtectTime;							// ������ʣ��ʱ��
	bool			bDisplayVip;
};
// ������Ϣ
struct TerrainInfo 
{
	bool			bCanBuild;
	unsigned char	nFloor1;
	unsigned char	nFloor2;

	TerrainInfo()
	: bCanBuild(true)
	, nFloor1(0)
	, nFloor2(0)
	{

	}

	bool IsTerrain()
	{
		return !bCanBuild || nFloor1!=0 || nFloor2!=0;
	}
};
struct TerrainUnit 
{
	unsigned int	nPosX;
	unsigned int	nPosY;
	TerrainInfo		info;
};
struct BuildingUnit
{
	unsigned int	nAutoID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
};
struct BuildingTEUnit 
{
	unsigned int	nBeginTime;
	unsigned int	nEndTime;
	unsigned int	nAutoID;
	unsigned int	nExcelID;		// �����½���ʱ����Ҫautoid+excelid
	unsigned int	nType;
};
// �ǳ�����
struct CityAtb 
{
	whlist<BuildingUnit>	listBaseBuilding;		// ���ǻ�������
	whlist<BuildingUnit>	listResourcceBuilding;	// ��Դ����
	whlist<BuildingUnit>	listMilitaryBuilding;	// ���½���
};
struct StoreItem
{
	unsigned int	nItemExcelID;
	unsigned int	nCrystal;
	unsigned int	nDiamond;
	bool			bCanCrystalBuy;
};
struct ProductionEvent 
{
	unsigned int	nAutoID;
	unsigned int	nProduction;
	unsigned int	nType;
};
struct ProductionTimeEvent 
{
	unsigned int	nAutoID;
	unsigned int	nProduction;
	unsigned int	nType;
	unsigned int	nBeginTime;
	unsigned int	nEndTime;
};
struct TechnologyUnit
{
	unsigned int	nExcelID;
	unsigned int	nLevel;
};
struct ResearchTimeEvent
{
	unsigned int	nExcelID;
	unsigned int	nBeginTime;
	unsigned int	nEndTime;
};
struct SoldierUnit 
{
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nNum;
};
struct AllianceUnit
{
	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
	tty_id_t		nLeaderID;
	char			szLeaderName[TTY_CHARACTERNAME_LEN];
	unsigned int	nDevelopment;
	unsigned int	nTotalDevelopment;
	char			szIntroduction[TTY_INTRODUCTION_LEN];
	unsigned int	nRank;	// ��������

	AllianceUnit()
	{
		WHMEMSET0THIS();
	}
};
// ������Ƭ
struct AllianceCard 
{
	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
	tty_id_t		nLeaderID;
	char			szLeaderName[TTY_CHARACTERNAME_LEN];
	unsigned int	nDevelopment;
	unsigned int	nTotalDevelopment;
	unsigned int	nRank;	// ��������
	unsigned int	nLevel;
	unsigned int	nMemberNum;
	unsigned int	nMaxMemberNum;
	int				nIntroductionLen;
	char			szIntroduction[1];

	AllianceCard()
	{
		WHMEMSET0THIS();
	}
};
struct AllianceMemberUnit	// ������Ϣ��ͨ��playercard��ȡ
{
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nPosition;
	unsigned int	nDevelopment;
	unsigned int	nTotalDevelopment;
	unsigned int	nLevel;
	unsigned int	nCup;

	AllianceMemberUnit(tty_id_t _nAccountID)
	: nAccountID(_nAccountID)
	{
	}

	bool operator == (const AllianceMemberUnit& other)
	{
		return nAccountID == other.nAccountID;
	}
};
struct AllianceBuildingTimeEvent
{
	unsigned int	nExcelID;
	unsigned int	nBeginTime;
	unsigned int	nEndTime;
	unsigned int	nType;

	AllianceBuildingTimeEvent(unsigned int _nExcelID, unsigned int _nType)
	: nExcelID(_nExcelID), nType(_nType)
	{

	}

	bool operator == (const AllianceBuildingTimeEvent& other)
	{
		return nExcelID==other.nExcelID && nType==other.nType;
	}
};
struct AllianceBuildingUnit 
{
	unsigned int	nExcelID;
	unsigned int	nLevel;
};
struct AllianceJoinEvent 
{
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nHeadID;

	AllianceJoinEvent(tty_id_t nID)
	: nAccountID(nID)
	{

	}

	bool operator == (const AllianceJoinEvent& other)
	{
		return nAccountID == other.nAccountID;
	}
};
struct PrivateMailUnit 
{
	unsigned int	nMailID;
	tty_id_t		nSenderID;
	unsigned char	nType;
	unsigned char	nFlag;
	bool			bReaded;
	time_t			nTime;
	char			szSender[TTY_CHARACTERNAME_LEN];
	unsigned int	nTextLen;
	char			szText[1];
	// ���滹�������ֶ�
	// unsigned int	nExtDataLen;
	// char			szExtData[1];

	void*	GetExtData()
	{
		return wh_getoffsetaddr(this, sizeof(PrivateMailUnit)-1+nTextLen+sizeof(int));
	}
	int		GetExtDataLen()
	{
		return *((int*)wh_getoffsetaddr(this, sizeof(PrivateMailUnit)-1+nTextLen));
	}
};
struct FriendUnit 
{
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nHeadID;
};
struct EnemyUnit 
{
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nHeadID;
};
struct FriendApplyUnit 
{
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nHeadID;
};
struct AllianceMailUnit 
{
	unsigned int	nMailID;
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	time_t			nTime;
	unsigned int	nTextLen;
	char			szText[1];
};
struct AllianceLogUnit 
{
	unsigned int	nLogID;
	time_t			nTime;
	unsigned int	nTextLen;
	char			szText[1];
};
struct PrivateLogUnit 
{
	unsigned int	nLogID;
	time_t			nTime;
	unsigned int	nTextLen;
	char			szText[1];
};
struct RelationLogUnit 
{
	unsigned int	nLogID;
	tty_id_t		nAccountID;
	unsigned int	nType;
	time_t			nTime;
	unsigned int	nTextLen;
	char			szText[1];
};
struct TrainingTimeEvent 
{
	time_t			nBeginTime;
	time_t			nEndTime;
	tty_id_t		nHeroID;
	unsigned int	nExp;
};
//////////////////////////////////////////////////////////////////////////
// �齱���
enum
{
	lottery_probability_base		= 1000000,		// �齱���ʵĻ�����1000000(1000,000����ٷְٸ���)
	lottery_probability_max_choice	= 10,			// ���ĳ齱ѡ��
};
enum
{
	lottery_type_none				= 0,			// �޽���
	lottery_type_item				= 1,			// ����
	lottery_type_crystal			= 2,			// ˮ��
	lottery_type_gold				= 3,			// �ƽ�
};
struct LotteryEntry 
{
	unsigned char	nType;
	unsigned int	nData;		// lottery_type_item->excel_id,����Ϊ����
	bool			bTopReward;	// �Ƿ��Ǹ߼�����(����)
};
//////////////////////////////////////////////////////////////////////////
// �������
struct PlayerData
{
	int				m_nPlayerGID;							// ��������ID
	int				m_nTermType;							// �ն�����
	CharAtb			m_CharAtb;								// ��ɫ��������
	CityAtb			m_CityAtb;								// �ǳ�����
	whlist<PrivateMailUnit*>	m_MailList;					// ˽���ʼ��б�
	LotteryEntry	m_lottery;								// ���γ齱���
	tty_id_t		m_nGroupID;
	unsigned int	m_nAppID;

	PlayerData()
	{
		Clear();
	}

	inline unsigned int	GetVip()
	{
		return m_CharAtb.bVipDisplay?m_CharAtb.nVip:0;
	}

	void	AddMoney(unsigned int nMoneyType, unsigned int nNum)
	{
		switch (nMoneyType)
		{
		case money_type_diamond:
			{
				m_CharAtb.nDiamond	+= nNum;
			}
			break;
		case money_type_crystal:
			{
				m_CharAtb.nCrystal	+= nNum;
			}
			break;
		case money_type_gold:
			{
				m_CharAtb.nGold	+= nNum;
			}
			break;
		}
	}
	void	SubMoney(unsigned int nMoneyType, unsigned int nNum)
	{
		switch (nMoneyType)
		{
		case money_type_diamond:
			{
				if (m_CharAtb.nDiamond >= nNum)
				{
					m_CharAtb.nDiamond	-= nNum;
				}
				else
				{
					m_CharAtb.nDiamond	= 0;
				}
			}
			break;
		case money_type_crystal:
			{
				if (m_CharAtb.nCrystal >= nNum)
				{
					m_CharAtb.nCrystal	-= nNum;
				}
				else
				{
					m_CharAtb.nCrystal	= 0;
				}
			}
			break;
		case money_type_gold:
			{
				if (m_CharAtb.nGold >= nNum)
				{
					m_CharAtb.nGold	-= nNum;
				}
				else
				{
					m_CharAtb.nGold	= 0;
				}
			}
			break;
		}
	}

	void	Clear()
	{
		m_nPlayerGID	= 0;
		m_nTermType		= TERM_TYPE_NONE;
		m_nGroupID		= 0;
		m_nAppID		= 0;
		memset(&m_CharAtb, 0, sizeof(m_CharAtb));
		memset(&m_lottery, 0, sizeof(m_lottery));

		// �ǳؽ����б����
		m_CityAtb.listBaseBuilding.clear();
		m_CityAtb.listResourcceBuilding.clear();
		m_CityAtb.listMilitaryBuilding.clear();

		// ����ʼ��б�
		ClearMailList();
	}

	void	ClearMailList()
	{
		for (whlist<PrivateMailUnit*>::iterator it=m_MailList.begin(); it!=m_MailList.end(); ++it)
		{
			delete[]	(char*)(*it);
		}
		m_MailList.clear();
	}

	void	ClearMailList(unsigned int nMailType)
	{
		whlist<PrivateMailUnit*>::iterator	it		= m_MailList.begin();
		whlist<PrivateMailUnit*>::iterator	itNext	= it;
		for (; it!=m_MailList.end(); )
		{
			++itNext;
			if ((*it)->nType&nMailType)
			{
				m_MailList.erase(it);
			}
			it	= itNext;
		}
	}

	void	DeleteMail(unsigned int nMailID)
	{
		whlist<PrivateMailUnit*>::iterator	it		= m_MailList.begin();
		for (; it!=m_MailList.end(); ++it)
		{
			PrivateMailUnit*	pMail		= *it;
			if (pMail->nMailID == nMailID)
			{
				m_MailList.erase(it);
			}
			break;
		}
	}

	void	AddNewMail(PrivateMailUnit* pNewMail)
	{
		whlist<PrivateMailUnit*>::iterator	it		= m_MailList.begin();
		if (it == m_MailList.end())
		{
			m_MailList.push_back(pNewMail);
			return;
		}
		for (; it!=m_MailList.end(); ++it)
		{
			PrivateMailUnit*	pMail		= *it;
			if (pNewMail->nMailID == pMail->nMailID)
			{
				// ��ֹͬһ���ʼ��ظ�����
				delete[] (char*)pNewMail;
				return;
			}
			if (pNewMail->nMailID > pMail->nMailID)
			{
				m_MailList.insertbefore(it, pNewMail);
				return;
			}
		}
		m_MailList.push_back(pNewMail);
	}

	unsigned int	GetMaxMailID()
	{
		unsigned int	nMailID		= 0;
		whlist<PrivateMailUnit*>::iterator	it		= m_MailList.begin();
		if (it!=m_MailList.end())
		{
			nMailID					= (*it)->nMailID;
		}

		return nMailID;
	}

	void	LoadFromDBPacket(void* pData)
	{
		void*	pPos	= pData;
		memcpy(&m_CharAtb, pPos, sizeof(CharAtb));
		pPos			= wh_getoffsetaddr(pPos, sizeof(CharAtb));
		// ֮���һЩ����
	}
};
struct GoldoreSmpInfo 
{
	tty_id_t	nAccountID;
	bool		bCanFetchGold;
	bool		bCanWaterChristmasTree;
};

//////////////////////////////////////////////////////////////////////////
// ���а����
//////////////////////////////////////////////////////////////////////////
enum
{
	rank_type_char_level						= 1,		// ��ɫ�ȼ�
	rank_type_char_gold							= 2,		// ��ɫ�ƽ�����
	rank_type_char_diamond						= 3,		// ��ɫ��ʯ����
	rank_type_instance_wangzhe					= 4,		// ����֮·

	max_rank_query_num							= 50,		// һ���Բ�ѯ�����а����������ֵ
};
struct RankData 
{
	tty_id_t		nOnlyID;
	unsigned int	nData;
};
struct CharRankInfo 
{
	tty_id_t		nAccountID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nHeadID;
	unsigned int	nRank;
	unsigned int	nData;
};
//////////////////////////////////////////////////////////////////////////
// ��������
//////////////////////////////////////////////////////////////////////////
struct WorldFamousCity 
{
	unsigned int	nType;
	unsigned int	nPosX;
	unsigned int	nPosY;
	bool			bCanBuild;
	unsigned char	nFloor2;
	unsigned int	nSrcTerrainType;

	WorldFamousCity()
	{
		WHMEMSET0THIS();
	}

	bool	operator == (const WorldFamousCity& city)
	{
		if (this == &city)
		{
			return true;
		}
		return nPosX==city.nPosX && nPosY==city.nPosY;
	}
};
//////////////////////////////////////////////////////////////////////////
// ������Դ
//////////////////////////////////////////////////////////////////////////
struct WorldResSimpleInfo 
{
	tty_id_t				nID;
	unsigned int			nType;
	unsigned int			nLevel;
	unsigned int			nPosX;
	unsigned int			nPosY;
};
struct WorldRes2Client
{
	tty_id_t				nID;
	unsigned int			nType;
	unsigned int			nLevel;
	unsigned int			nPosX;
	unsigned int			nPosY;
	unsigned int			nGold;
	unsigned int			nPop;
	unsigned int			nCrystal;
	unsigned int			nForce;
};
struct WorldRes 
{
	tty_id_t				nID;
	unsigned int			nType;
	unsigned int			nLevel;
	unsigned int			nPosX;
	unsigned int			nPosY;
	unsigned int			nGold;
	unsigned int			nPop;
	unsigned int			nCrystal;
	unsigned int			nForce;
	char					szArmyDeploy[10];
	char					szArmyData[10];

	bool			bCanBuild;
	unsigned char	nFloor2;
	unsigned int	nSrcTerrainType;

	WorldRes()
	{
		WHMEMSET0THIS();
	}

	bool	operator == (const WorldRes& res)
	{
		if (this == &res)
		{
			return true;
		}
		return nPosX==res.nPosX && nPosY==res.nPosY;
	}
};

struct AlliInstanceCombatLog 
{
	struct HeroUnit 
	{
		char			szHeroName[TTY_CHARACTERNAME_LEN];
		unsigned int	nHeroLevel;
		unsigned int	nProf;
		unsigned int	nRow;
		unsigned int	nCol;
		unsigned int	nArmyType;
		unsigned int	nArmyLevel;
		unsigned int	nArmyNum;
		unsigned int	nArmyDeathNum;
		unsigned int	nArmyRestoreNum;
		tty_id_t		nHeroID;
		char			szCharName[TTY_CHARACTERNAME_LEN];
	};
	struct LootUnit 
	{
		// �Ժ������ͬһ���ط������еĶ�������,thing_type_xxx
		enum
		{
			loot_type_none				= 0,		// ��
			loot_type_gold				= 1,		// �ƽ�
			loot_type_crystal			= 2,		// ˮ��
			loot_type_diamond			= 3,		// ��ʯ
			loot_type_pop				= 4,		// �˿�
			loot_type_item				= 5,		// ����
		};
		unsigned int	nType;
		unsigned int	nExtData;
		unsigned int	nNum;
	};
	// combat_result,ս�����
	enum
	{
		combat_result_win							= 1,		// �ɹ�
		combat_result_lose							= 2,		// ʧ��
	};
	int					nCombatResult;
	unsigned int		nCombatType;				// ս������
	tty_id_t			nAccountID;
	tty_id_t			nObjID;
	int					nRandSeed;					// ��������
	int					nAttackHeroNum;
	int					nDefenseHeroNum;
	int					nLootNum;
	float				fCounterRate;
	// nAttackNum*HeroUnit����
	// nDefenseNum*HeroUnit����
	// nLootNum*LootUnit����
};
struct HeroSimpleData
{
	tty_id_t		nHeroID;
	int				nArmyType;
	int				nArmyLevel;
	int				nArmyNum;
	int				nProf;
	int				nHealthState;
	int				nLevel;
	int				nExp;
	int             nStatus;
};
struct ArenaHeroDeploy 
{
	unsigned int	nCol;
	unsigned int	nRow;
	tty_id_t		nHeroID;
	int				nArmyType;
	int				nArmyLevel;
	int				nArmyNum;
	int				nProf;
};
struct SimpleInstanceData 
{
	tty_id_t		nInstanceID;
	tty_id_t		nAccountID;
};
struct HeroDeploy 
{
	unsigned int	nCol;
	unsigned int	nRow;
	tty_id_t		nHeroID;
};

struct PositionMark 
{
	unsigned int	nPosX;
	unsigned int	nPosY;
	unsigned int	nHeadID;
	int				nLength;

	inline int	Size()
	{
		return sizeof(PositionMark)+nLength;
	}
};
#pragma pack()
}

#endif
