// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : 
// File         : tty_game_def.h
// Creator      : gedamis
// Comment      : �߼��йصĶ���
// CreationDate : 2012-06-06

#ifndef __TTY_GAME_DEF_H__
#define __TTY_GAME_DEF_H__

#include "tty_common_def.h"

using namespace n_pngs;

enum
{
	TAVERN_REFRESH_NUM				= 6,						// �ƹݵ���ˢ���佫����
	COMBATFIELD_CHANNEL				= 5,						// ս��ͨ������
};
enum
{
	COMBAT_RST_VICTORY				= 1,						// ս��ʤ��
	COMBAT_RST_LOSE					= 2,						// ս��ʧ��
};
enum
{
	COMBAT_PVE_RAID					= 1,						// ��ͨս�����ַ�
	COMBAT_PVP_ATTACK				= 2,						// ��Ҷ�ս������
	COMBAT_PVP_DEFENSE				= COMBAT_PVP_ATTACK + 1,	// ��Ҷ�ս���ط�
	COMBAT_PVE_WORLDGOLDMINE		= 10,						// �����󣨶�AI��
	COMBAT_PVP_WORLDGOLDMINE_ATTACK	= COMBAT_PVE_WORLDGOLDMINE + 1,	// �����󣨶���ң�������
	COMBAT_PVP_WORLDGOLDMINE_DEFENSE= COMBAT_PVE_WORLDGOLDMINE + 2,	// �����󣨶���ң����ط�
	COMBAT_WORLDCITY				= 20,						// ��������
	COMBAT_INSTANCE_BAIZHANBUDAI	= 100,						// ��������ս����
	COMBAT_INSTANCE_NANZHENGBEIZHAN	= 101,						// ������������ս
	COMBAT_ALLI_INSTANCE			= 102,						// ���˸���

	COMBAT_ARENA					= 200,						// ��������ս

	COMBAT_HERO_TRAINING			= 1000,						// �������佫������
	COMBAT_PVE_WORLD_RES			= 1001,						// ��ȡ������Դ
};
// ��������
enum
{
	SYNC_TYPE_HERO					= 1,						// �佫
	SYNC_TYPE_CHAR					= 2,						// ����
	SYNC_TYPE_ITEM					= 3,						// ����
	SYNC_TYPE_DRUG					= 4,						// ��ҩ��
};
// �������
enum
{
	WORLDGOLDMINE_CLASS_TOP			= 1,						// �ؼ�
	WORLDGOLDMINE_CLASS_SEN			= 2,						// �߼�
	WORLDGOLDMINE_CLASS_MID			= 3,						// �м�
	WORLDGOLDMINE_CLASS_BAS			= 4,						// ����
};
// ��ʱ�¼�����
enum
{
	FIXHOUR_TYPE_WORLDGOLDMINE		= 10,						// ����������
	FIXHOUR_TYPE_DRUG				= 1000,						// ������ҩ
};

#pragma pack(1)
// ս����λ
struct CombatUnit
{
	int			nSlot;				// ս��ͨ��
	tty_id_t	nHeroID;			// �佫ID
	char		szName[32];			// �佫����
	int			nArmyType;			// ��������
	int			nArmyLevel;			// ���ӵȼ�
	int			nArmyNum;			// ��������
	int			nAttack;			// ���ӹ���
	int			nDefense;			// ���ӷ���
	int			nHealth;			// ��������
	int			nModel;				// ͷ��
	int			nLevel;				// �佫�ȼ�
	int			nHealthState;		// ����״��

	CombatUnit()
	: nSlot(0)
	, nHeroID(0)
	, nArmyType(0)
	, nArmyLevel(0)
	, nArmyNum(0)
	, nAttack(0)
	, nDefense(0)
	, nHealth(0)
	, nModel(0)
	, nLevel(0)
	, nHealthState(0)
	{
		szName[0] = 0;
	}
};
// ս����λ����־��
struct CombatUnit4Log
{
	int			nSlot;				// ս��ͨ��
	int			nLevel;				// �佫�ȼ�
	char		szName[32];			// �佫����
	int			nArmyType;			// ��������
	int			nArmyLevel;			// ���ӵȼ�
	int			nArmyNumBefore;		// ������������սǰ
	int			nArmyNumAfter;		// ������������ս��
	tty_id_t	nHeroID;			// �佫ID
	int			nArmyNumRecover;	// �����������ָ�ֵ
	int			nArmyAttack;		// 
	int			nArmyDefense;		// 
	int			nArmyHealth;		// 
};
// ս�����
struct CombatRst
{
	tty_id_t	nHeroID;			// �佫ID
	int			nArmyNum;			// ��������
	int			nArmyAttack;		// ���ӹ���
	int			nArmyDefense;		// ���ӷ���
	int			nArmyHealth;		// ��������
	int			nHealthState;		// ����״��
	int			nArmyProf;			// ս��
	int			nArmyNumRecover;	// �����������ָ�ֵ
	int			nHeroLevel;			// �佫�ȼ�
	int			nHeroExp;			// �佫��ǰ����
};
// ˢ���佫
struct RefreshHero
{
	int			nSlotID;		// ���λ��
	tty_id_t	nHeroID;		// �佫ID
	char		szName[32];		// ����
	int			nProfession;	// ְҵ
	int			nAttack;		// ����
	int			nDefense;		// ����
	int			nHealth;		// ����
	int			nModel;			// ģ��ID
	float		fGrow;			// �ɳ���
	float		fGrowMax;		// �ɳ�������

	RefreshHero()
	: nSlotID(0)
	, nHeroID(0)
	, nProfession(0)
	, nAttack(0)
	, nDefense(0)
	, nHealth(0)
	, nModel(0)
	, fGrow(0.0f)
	, fGrowMax(0.0f)
	{
		szName[0] = 0;
	}
};
// ��Ӷ�佫
struct HireHero
{
	tty_id_t	nHeroID;		// �佫ID
	char		szName[32];		// ����
	int			nProfession;	// ְҵ
	int			nAttack;		// ����
	int			nDefense;		// ����
	int			nHealth;		// ����
	float		fGrow;			// �ɳ���
	int			nArmyType;		// ��������
	int			nArmyLevel;		// ���ӵȼ�
	int			nArmyNum;		// �������� 
	int			nModel;			// ģ��ID
	int			nLeader;		// ͳ��
	int			nStatus;		// �Ƿ����
	int			nAttackAdd;		// ������װ���ӳɵ�
	int			nDefenseAdd;	// ������װ���ӳɵ�
	int			nHealthAdd;		// ������װ���ӳɵ�
	int			nLeaderAdd;		// ͳ�ʣ�װ���ӳɵ�
	int			nLevel;			// �ȼ�
	int			nProf;			// ս��
	int			nHealthState;	// ����״��
	int			nExp;			// ����
	float		fGrowMax;		// �ɳ�������

	HireHero()
	: nHeroID(0)
	, nProfession(0)
	, nAttack(0)
	, nDefense(0)
	, nHealth(0)
	, fGrow(0.0f)
	, nArmyType(0)
	, nArmyLevel(0)
	, nArmyNum(0)
	, nModel(0)
	, nLeader(0)
	, nStatus(0)
	, nAttackAdd(0)
	, nDefenseAdd(0)
	, nHealthAdd(0)
	, nLeaderAdd(0)
	, nLevel(0)
	, nProf(0)
	, nHealthState(0)
	, nExp(0)
	, fGrowMax(0)
	{
		szName[0] = 0;
	}
};
// ս��ժҪ
struct CombatDesc
{
	tty_id_t		nCombatID;		// ս��ID
	int				nCombatType;	// ս������
	tty_id_t		nObjID;			// Ŀ��ID
	int				nResTime;		// ʣ��ʱ��
	int				nStatus;		// ս��״̬
	char			szName[32];		// Ŀ������

	CombatDesc()
	: nCombatID(0)
	, nCombatType(0)
	, nObjID(0)
	, nResTime(0)
	, nStatus(0)
	{
		szName[0]	= 0;
	}
};
// ����ժҪ
struct ItemDesc
{
	tty_id_t		nItemID;		// ����ID
	int				nExcelID;		// excel ID
	int				nNum;			// ����
};
// �佫װ������ժҪ
struct EquipDesc
{
	tty_id_t		nEquipID;		// װ��ID
	int				nEquipType;		// װ������
	int				nExcelID;		// װ��excel id
	tty_id_t		nHeroID;		// �佫ID
};
// װ����ʯ����ժҪ
struct GemDesc
{
	tty_id_t		nEquipID;		// װ��ID
	int				nSlotIdx;		// ��ۺ�
	int				nSlotType;		// �������
	int				nExcelID;		// ��ʯexcel id
	tty_id_t		nGemID;			// ��ʯΨһid
};
// ����ժҪ
struct QuestDesc
{
	int				nExcelID;		// ����ID
	int				nDone;			// �Ƿ�����
};
// ����״̬ժҪ
struct InstanceStatus
{
	int				nExcelID;		// ����ID
	int				nResDayTimesFree;	// ����ʣ����Ѵ���
	int				nDayTimesFree;		// 
	int				nResDayTimesFee;	// ����ʣ���շѴ���
	int				nDayTimesFee;		// 
	tty_id_t		nInstanceID;		// ʵ��ID��>0��ʾ��ǰ���ڸ�����
	int				nStatus;			// 
	int				nClassID;			// 
	tty_id_t		nCreatorID;			// 
	int				nCurLevel;			// 
};
// ����ʵ��ժҪ
struct InstanceDesc
{
	tty_id_t		nInstanceID;	// ʵ��ID
	char			szName[32];		// ʵ������
	int				nCurLevel;		// ��ǰ�ؿ���
	int				nLeaderLevel;	// �ӳ��ȼ�
	int				nCurPlayer;		// ��ǰ����
	tty_id_t		nCreatorID;		// 
};
// ������Ա
struct InstanceData
{
	tty_id_t		nAccountID;		// ��Ա�˺�ID
	char			szName[32];		// ��Ա�˺�����
	tty_id_t		n1Hero;			// �佫ID
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
	int				nStatus;		// �Ƿ����
};
// �佫����ժҪ
struct HeroDesc
{
	int				nAttackBase;
	int				nAttackAdd;
	int				nDefenseBase;
	int				nDefenseAdd;
	int				nHealthBase;
	int				nHealthAdd;
	int				nLeaderBase;
	int				nLeaderAdd;
	float			fGrow;
	tty_id_t		nHeroID;
	int				nHealthState;
	int				nArmyType;
	int				nArmyLevel;
	int				nArmyNum;
	int				nExp;
	int				nLevel;
	int				nProf;
};
// ��������ժҪ
struct CharDesc
{
	int				nLevel;
	int				nExp;
	int				nDiamond;
	int				nCrystal;
	int				nGold;
	int				nVIP;
	int				nPopulation;
	int				nProtectTime;
};
// ����ժҪ
struct LootDesc
{
	int				nExcelID;
	int				nNum;
};
// ��������ժҪ
struct InstanceLootDesc
{
	tty_id_t		nItemID;
	int				nExcelID;
	int				nNum;
};
// �����佫ժҪ�����������ֶ�������
struct InstanceHeroDesc
{
	tty_id_t		nHeroID;
	int				nArmyNum;
	int				nArmyProf;
	int				nHealthState;
	int				nLevel;
	int				nMaxArmyNum;
};
// ������ժҪ
struct WorldGoldMineDesc
{
	int				nClass;			// ���ȼ�
	int				nIdx;			// ������
	tty_id_t		nAccountID;		// �˺�ID
	char			szName[32];		// �˺�����
	int				nSafeTime;		// ��ȫʱ��
};
// ս��
struct CombatProf
{
	int				nProf;			// ս��
	int				n1ArmyType;		// ��������
	int				n2ArmyType;		// ��������
	int				n3ArmyType;		// ��������
	int				n4ArmyType;		// ��������
	int				n5ArmyType;		// ��������
};
// ��������
struct ArmyDesc
{
	int				aCrush[2];		// ���Ʊ���
	float			fBonus;			// �����ӳɰٷֱ�
};
// ս����־
struct CombatLog
{
	int				nCombatType;	// ս������
	int				nCombatRst;		// ս�����
	char			szAttackName[32];	// ��������
	char			szDefenseName[32];	// �ط�����
	int				nNumA;			// �����佫����
	int				nNumD;			// �ط��佫����
	int				nAttackHeadID;
	int				nDefenseHeadID;
	tty_id_t		nObjID;
	int				nPosX;
	int				nPosY;
	// ����� �佫��Ϣ CombatUnit4Log
	// ����� ������Ϣ������ nCombatType ����
};
// ս����־������Ϣ���ַ�
struct CombatLog_PVE_Raid
{
	int				nExcelID;		// ����ID
	int				nNum;			// ��������
};
// ս����־������Ϣ����Ҷ�ս
struct CombatLog_PVP
{
	int				nGold;			// ��ʧ���
	int				nPopulation;	// �Ӷ��˿�
	int				nAddPopulation;	// ʵ������
	int				nCup;			// ����
};
// ս����־������Ϣ��������
struct CombatLog_WorldGoldMine
{
	int				nArea;			// �������
	int				nClass;			// ���ȼ�
	int				nIdx;			// ������
};
// ս����־������Ϣ����������
struct	CombatLog_InstanceLoot
{
	int				nNum;			// ��������
	// ����� LootDesc
};
// ս����־������Ϣ����������
struct	CombatLog_WorldCity
{
	tty_id_t		nID;			// ��������ID
	int				nCup;			// ��õĽ�������
	int				nRes;			// ʣ����
};

// ʹ�õ���(���)����ժҪ
struct UseItemDesc
{
	int				nNum;
};
struct AddItemDesc
{
	tty_id_t		nItemID;
	int				nExcelID;
	int				nNum;
};

// ʹ�õ��ߣ���ҩ����
struct DrugDesc
{
	int				nNum;			// �����Ĳ�ҩ����
};
struct AcctDrugDesc
{
	tty_id_t		nID;			// �˺�ID
	int				nNum;			// �����Ĳ�ҩ����
};

// ��������
struct	WorldCity
{
	tty_id_t		nID;			// ID
	int				nClass;			// �ȼ�
	int				nPosX;			// ����
	int				nPosY;
	int				nRecover;		// ����ָ���ʱ��
	int				nCup;			// ������
	char			szName[64];		// ����
	int				nInitProf;		// ��ʼս��
	int				nInitRecover;	// �ָ����
	int				nCurProf;		// ��ǰս��
};
// ����ս������
struct	WorldCityLog
{
	int				nID;			// ����
	tty_id_t		nAccountID;		// �˺�ID
	char			szName[32];		// ����
	int				nHeadID;		// ͷ��
	int				nDiamond;		// ��ʯ��
	int				nCup;			// ������
};
// ����ս������
struct	WorldCityLogAlliance
{
	int				nID;			// ����
	tty_id_t		nAllianceID;	// ����ID
	char			szName[32];		// ����
	int				nDiamond;		// ��ʯ��
	int				nCup;			// ������
};
// ������������
struct	WorldCityRankMan
{
	int				nID;			// ����
	tty_id_t		nAccountID;		// �˺�ID
	char			szName[32];		// ����
	int				nHeadID;		// ͷ��
	int				nLevel;			// �ȼ�
	int				nCup;			// ������
};
// ������������
struct	WorldCityRankAlliance
{
	int				nID;			// ����
	tty_id_t		nAllianceID;	// ����ID
	char			szName[32];		// ����
	int				nLevel;			// �ȼ�
	int				nCup;			// ������
};


#pragma pack()


#endif

