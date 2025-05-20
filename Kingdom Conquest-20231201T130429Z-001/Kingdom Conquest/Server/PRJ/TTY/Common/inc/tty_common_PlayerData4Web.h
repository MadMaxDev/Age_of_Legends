// 切记,网络传输的数据一定要1字节对齐,不然平台间的同一结构的大小是不一致的
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
// 君主的基本属性
struct CharAtb 
{
	// common_characters表中所存信息
	tty_id_t		nAccountID;								// 账号OnlyID
	char			szName[TTY_CHARACTERNAME_LEN];			// 角色名字
	unsigned int	nLastLoginTime;							// 登录时间
	unsigned int	nLastLogoutTime;						// 登出时间
	unsigned int	nExp;									// 当前经验值
	unsigned int	nLevel;									// 当前等级
	unsigned int	nDiamond;								// 钻石
	unsigned int	nCrystal;								// 水晶
	unsigned int	nVip;									// Vip级别
	unsigned int	nSex;									// 性别
	unsigned int	nHeadID;								// 头像ID
	unsigned int	nPosX;									// 世界X坐标
	unsigned int	nPosY;									// 世界Y坐标
	unsigned int	nDevelopment;							// 发展度
	unsigned int	nGold;									// 黄金
	unsigned int	nPopulation;							// 人口
	unsigned int	nBuildNum;								// 建造队列
	unsigned int	nAddedBuildNum;							// 附加建造队列

	// 其他表联查结果
	tty_id_t		nAllianceID;							// 联盟ID
	char			szAllianceName[TTY_ALLIANCENAME_LEN];	// 联盟名字

	unsigned int	nDrug;									// 草药
	unsigned int	nFreeDrawLotteryNum;					// 免费抽奖剩余次数
	unsigned int	nTradeNum;								// 今天剩余的跑商次数

	char			szSignature[TTY_CHARACTER_SIGNATURE_LEN];		// 个性签名

	unsigned int	nChangeNameNum;							// 改名次数

	unsigned int	nProtectTime;							// 保护期剩余时间
	unsigned int	nProtectTimeEndTick;					// 保护到期时刻

	unsigned int	nAddBuildNumTime;						// 工匠之书剩余时间
	unsigned int	nAddBuildNumTimeEndTick;				// 工匠之书到期时刻

	unsigned int	nTotalBuildNum;							// 总共的建筑队列
	unsigned int	nNotificationID;						// 最近阅读了的通知ID

	unsigned int	nAllianceDrawLotteryNum;				// 联盟抽奖今天剩余次数
	unsigned int	nTotalDiamond;							// 总充值钻石

	unsigned int	nInstanceWangzhe;						// 王者之路当前最大关卡
	unsigned int	nInstanceZhengzhan;						// 征战天下当前最大关卡

	unsigned int	nCup;									// 奖杯数
	bool			bBinded;								// 是否已经绑定账号
	bool			bVipDisplay;							// 是否显示vip
};
// 玩家名片(简单信息)
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
	char			szSignature[TTY_CHARACTER_SIGNATURE_LEN];		// 个性签名

	unsigned int	nProtectTime;							// 保护期剩余时间
	unsigned int	nLevelRank;										// 君主等级排行
	unsigned int	nCityLevel;										// 城市等级
	unsigned int	nProtectTimeEndTick;							// 保护到期时刻

	unsigned int	nInstanceWangzhe;						// 王者之路当前最大关卡
	unsigned int	nInstanceZhengzhan;						// 征战天下当前最大关卡

	unsigned int	nCup;									// 奖杯数
	bool			bVipDisplay;							// 是否显示vip
};
// 城市名片
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
	unsigned int	nProtectTime;							// 保护期剩余时间
	bool			bDisplayVip;
};
// 地理信息
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
	unsigned int	nExcelID;		// 建造新建筑时就需要autoid+excelid
	unsigned int	nType;
};
// 城池属性
struct CityAtb 
{
	whlist<BuildingUnit>	listBaseBuilding;		// 主城基本建筑
	whlist<BuildingUnit>	listResourcceBuilding;	// 资源建筑
	whlist<BuildingUnit>	listMilitaryBuilding;	// 军事建筑
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
	unsigned int	nRank;	// 联盟排名

	AllianceUnit()
	{
		WHMEMSET0THIS();
	}
};
// 联盟名片
struct AllianceCard 
{
	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
	tty_id_t		nLeaderID;
	char			szLeaderName[TTY_CHARACTERNAME_LEN];
	unsigned int	nDevelopment;
	unsigned int	nTotalDevelopment;
	unsigned int	nRank;	// 联盟排名
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
struct AllianceMemberUnit	// 其他信息再通过playercard获取
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
	// 后面还有两个字段
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
// 抽奖相关
enum
{
	lottery_probability_base		= 1000000,		// 抽奖概率的基数是1000000(1000,000代表百分百概率)
	lottery_probability_max_choice	= 10,			// 最大的抽奖选项
};
enum
{
	lottery_type_none				= 0,			// 无奖励
	lottery_type_item				= 1,			// 道具
	lottery_type_crystal			= 2,			// 水晶
	lottery_type_gold				= 3,			// 黄金
};
struct LotteryEntry 
{
	unsigned char	nType;
	unsigned int	nData;		// lottery_type_item->excel_id,其他为数量
	bool			bTopReward;	// 是否是高级奖励(废弃)
};
//////////////////////////////////////////////////////////////////////////
// 玩家数据
struct PlayerData
{
	int				m_nPlayerGID;							// 就是连接ID
	int				m_nTermType;							// 终端类型
	CharAtb			m_CharAtb;								// 角色基本属性
	CityAtb			m_CityAtb;								// 城池属性
	whlist<PrivateMailUnit*>	m_MailList;					// 私人邮件列表
	LotteryEntry	m_lottery;								// 本次抽奖结果
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

		// 城池建筑列表清除
		m_CityAtb.listBaseBuilding.clear();
		m_CityAtb.listResourcceBuilding.clear();
		m_CityAtb.listMilitaryBuilding.clear();

		// 清除邮件列表
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
				// 防止同一封邮件重复插入
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
		// 之后的一些操作
	}
};
struct GoldoreSmpInfo 
{
	tty_id_t	nAccountID;
	bool		bCanFetchGold;
	bool		bCanWaterChristmasTree;
};

//////////////////////////////////////////////////////////////////////////
// 排行榜相关
//////////////////////////////////////////////////////////////////////////
enum
{
	rank_type_char_level						= 1,		// 角色等级
	rank_type_char_gold							= 2,		// 角色黄金数量
	rank_type_char_diamond						= 3,		// 角色钻石数量
	rank_type_instance_wangzhe					= 4,		// 王者之路

	max_rank_query_num							= 50,		// 一次性查询的排行榜数据量最大值
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
// 世界名城
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
// 世界资源
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
		// 以后最好在同一个地方把所有的东西分类,thing_type_xxx
		enum
		{
			loot_type_none				= 0,		// 无
			loot_type_gold				= 1,		// 黄金
			loot_type_crystal			= 2,		// 水晶
			loot_type_diamond			= 3,		// 钻石
			loot_type_pop				= 4,		// 人口
			loot_type_item				= 5,		// 道具
		};
		unsigned int	nType;
		unsigned int	nExtData;
		unsigned int	nNum;
	};
	// combat_result,战斗结果
	enum
	{
		combat_result_win							= 1,		// 成功
		combat_result_lose							= 2,		// 失败
	};
	int					nCombatResult;
	unsigned int		nCombatType;				// 战斗类型
	tty_id_t			nAccountID;
	tty_id_t			nObjID;
	int					nRandSeed;					// 种子类型
	int					nAttackHeroNum;
	int					nDefenseHeroNum;
	int					nLootNum;
	float				fCounterRate;
	// nAttackNum*HeroUnit数组
	// nDefenseNum*HeroUnit数组
	// nLootNum*LootUnit数组
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
