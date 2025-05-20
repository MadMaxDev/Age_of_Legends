// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : 
// File         : tty_game_def.h
// Creator      : gedamis
// Comment      : 逻辑有关的定义
// CreationDate : 2012-06-06

#ifndef __TTY_GAME_DEF_H__
#define __TTY_GAME_DEF_H__

#include "tty_common_def.h"

using namespace n_pngs;

enum
{
	TAVERN_REFRESH_NUM				= 6,						// 酒馆单次刷新武将个数
	COMBATFIELD_CHANNEL				= 5,						// 战场通道数量
};
enum
{
	COMBAT_RST_VICTORY				= 1,						// 战斗胜利
	COMBAT_RST_LOSE					= 2,						// 战斗失败
};
enum
{
	COMBAT_PVE_RAID					= 1,						// 普通战斗，讨伐
	COMBAT_PVP_ATTACK				= 2,						// 玩家对战，攻方
	COMBAT_PVP_DEFENSE				= COMBAT_PVP_ATTACK + 1,	// 玩家对战，守方
	COMBAT_PVE_WORLDGOLDMINE		= 10,						// 世界金矿（对AI）
	COMBAT_PVP_WORLDGOLDMINE_ATTACK	= COMBAT_PVE_WORLDGOLDMINE + 1,	// 世界金矿（对玩家），攻方
	COMBAT_PVP_WORLDGOLDMINE_DEFENSE= COMBAT_PVE_WORLDGOLDMINE + 2,	// 世界金矿（对玩家），守方
	COMBAT_WORLDCITY				= 20,						// 世界名城
	COMBAT_INSTANCE_BAIZHANBUDAI	= 100,						// 副本，百战不殆
	COMBAT_INSTANCE_NANZHENGBEIZHAN	= 101,						// 副本，南征北战
	COMBAT_ALLI_INSTANCE			= 102,						// 联盟副本

	COMBAT_ARENA					= 200,						// 竞技场挑战

	COMBAT_HERO_TRAINING			= 1000,						// 将领在武将馆修炼
	COMBAT_PVE_WORLD_RES			= 1001,						// 获取世界资源
};
// 数据类型
enum
{
	SYNC_TYPE_HERO					= 1,						// 武将
	SYNC_TYPE_CHAR					= 2,						// 君主
	SYNC_TYPE_ITEM					= 3,						// 道具
	SYNC_TYPE_DRUG					= 4,						// 草药包
};
// 金矿类型
enum
{
	WORLDGOLDMINE_CLASS_TOP			= 1,						// 特级
	WORLDGOLDMINE_CLASS_SEN			= 2,						// 高级
	WORLDGOLDMINE_CLASS_MID			= 3,						// 中级
	WORLDGOLDMINE_CLASS_BAS			= 4,						// 初级
};
// 定时事件类型
enum
{
	FIXHOUR_TYPE_WORLDGOLDMINE		= 10,						// 重置世界金矿
	FIXHOUR_TYPE_DRUG				= 1000,						// 补给草药
};

#pragma pack(1)
// 战斗单位
struct CombatUnit
{
	int			nSlot;				// 战场通道
	tty_id_t	nHeroID;			// 武将ID
	char		szName[32];			// 武将名字
	int			nArmyType;			// 部队类型
	int			nArmyLevel;			// 部队等级
	int			nArmyNum;			// 部队数量
	int			nAttack;			// 部队攻击
	int			nDefense;			// 部队防御
	int			nHealth;			// 部队生命
	int			nModel;				// 头像
	int			nLevel;				// 武将等级
	int			nHealthState;		// 健康状况

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
// 战斗单位，日志用
struct CombatUnit4Log
{
	int			nSlot;				// 战场通道
	int			nLevel;				// 武将等级
	char		szName[32];			// 武将名字
	int			nArmyType;			// 部队类型
	int			nArmyLevel;			// 部队等级
	int			nArmyNumBefore;		// 部队数量，开战前
	int			nArmyNumAfter;		// 部队数量，开战后
	tty_id_t	nHeroID;			// 武将ID
	int			nArmyNumRecover;	// 部队数量，恢复值
	int			nArmyAttack;		// 
	int			nArmyDefense;		// 
	int			nArmyHealth;		// 
};
// 战斗结果
struct CombatRst
{
	tty_id_t	nHeroID;			// 武将ID
	int			nArmyNum;			// 部队数量
	int			nArmyAttack;		// 部队攻击
	int			nArmyDefense;		// 部队防御
	int			nArmyHealth;		// 部队生命
	int			nHealthState;		// 健康状况
	int			nArmyProf;			// 战力
	int			nArmyNumRecover;	// 部队数量，恢复值
	int			nHeroLevel;			// 武将等级
	int			nHeroExp;			// 武将当前经验
};
// 刷新武将
struct RefreshHero
{
	int			nSlotID;		// 插槽位置
	tty_id_t	nHeroID;		// 武将ID
	char		szName[32];		// 名字
	int			nProfession;	// 职业
	int			nAttack;		// 攻击
	int			nDefense;		// 防御
	int			nHealth;		// 生命
	int			nModel;			// 模型ID
	float		fGrow;			// 成长率
	float		fGrowMax;		// 成长率上限

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
// 雇佣武将
struct HireHero
{
	tty_id_t	nHeroID;		// 武将ID
	char		szName[32];		// 名字
	int			nProfession;	// 职业
	int			nAttack;		// 攻击
	int			nDefense;		// 防御
	int			nHealth;		// 生命
	float		fGrow;			// 成长率
	int			nArmyType;		// 部队类型
	int			nArmyLevel;		// 部队等级
	int			nArmyNum;		// 部队数量 
	int			nModel;			// 模型ID
	int			nLeader;		// 统率
	int			nStatus;		// 是否出征
	int			nAttackAdd;		// 攻击，装备加成的
	int			nDefenseAdd;	// 防御，装备加成的
	int			nHealthAdd;		// 生命，装备加成的
	int			nLeaderAdd;		// 统率，装备加成的
	int			nLevel;			// 等级
	int			nProf;			// 战力
	int			nHealthState;	// 健康状况
	int			nExp;			// 经验
	float		fGrowMax;		// 成长率上限

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
// 战斗摘要
struct CombatDesc
{
	tty_id_t		nCombatID;		// 战斗ID
	int				nCombatType;	// 战斗类型
	tty_id_t		nObjID;			// 目标ID
	int				nResTime;		// 剩余时间
	int				nStatus;		// 战斗状态
	char			szName[32];		// 目标名字

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
// 道具摘要
struct ItemDesc
{
	tty_id_t		nItemID;		// 道具ID
	int				nExcelID;		// excel ID
	int				nNum;			// 数量
};
// 武将装备关联摘要
struct EquipDesc
{
	tty_id_t		nEquipID;		// 装备ID
	int				nEquipType;		// 装备类型
	int				nExcelID;		// 装备excel id
	tty_id_t		nHeroID;		// 武将ID
};
// 装备宝石关联摘要
struct GemDesc
{
	tty_id_t		nEquipID;		// 装备ID
	int				nSlotIdx;		// 插槽号
	int				nSlotType;		// 插槽类型
	int				nExcelID;		// 宝石excel id
	tty_id_t		nGemID;			// 宝石唯一id
};
// 任务摘要
struct QuestDesc
{
	int				nExcelID;		// 任务ID
	int				nDone;			// 是否可完成
};
// 副本状态摘要
struct InstanceStatus
{
	int				nExcelID;		// 副本ID
	int				nResDayTimesFree;	// 今天剩余免费次数
	int				nDayTimesFree;		// 
	int				nResDayTimesFee;	// 今天剩余收费次数
	int				nDayTimesFee;		// 
	tty_id_t		nInstanceID;		// 实例ID，>0表示当前正在副本里
	int				nStatus;			// 
	int				nClassID;			// 
	tty_id_t		nCreatorID;			// 
	int				nCurLevel;			// 
};
// 副本实例摘要
struct InstanceDesc
{
	tty_id_t		nInstanceID;	// 实例ID
	char			szName[32];		// 实例名字
	int				nCurLevel;		// 当前关卡号
	int				nLeaderLevel;	// 队长等级
	int				nCurPlayer;		// 当前人数
	tty_id_t		nCreatorID;		// 
};
// 副本成员
struct InstanceData
{
	tty_id_t		nAccountID;		// 成员账号ID
	char			szName[32];		// 成员账号名字
	tty_id_t		n1Hero;			// 武将ID
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
	int				nStatus;		// 是否就绪
};
// 武将属性摘要
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
// 君主属性摘要
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
// 掉落摘要
struct LootDesc
{
	int				nExcelID;
	int				nNum;
};
// 副本掉落摘要
struct InstanceLootDesc
{
	tty_id_t		nItemID;
	int				nExcelID;
	int				nNum;
};
// 副本武将摘要，现在用在手动补给后
struct InstanceHeroDesc
{
	tty_id_t		nHeroID;
	int				nArmyNum;
	int				nArmyProf;
	int				nHealthState;
	int				nLevel;
	int				nMaxArmyNum;
};
// 世界金矿摘要
struct WorldGoldMineDesc
{
	int				nClass;			// 金矿等级
	int				nIdx;			// 金矿序号
	tty_id_t		nAccountID;		// 账号ID
	char			szName[32];		// 账号名字
	int				nSafeTime;		// 安全时间
};
// 战力
struct CombatProf
{
	int				nProf;			// 战力
	int				n1ArmyType;		// 部队类型
	int				n2ArmyType;		// 部队类型
	int				n3ArmyType;		// 部队类型
	int				n4ArmyType;		// 部队类型
	int				n5ArmyType;		// 部队类型
};
// 兵种数据
struct ArmyDesc
{
	int				aCrush[2];		// 克制兵种
	float			fBonus;			// 攻击加成百分比
};
// 战斗日志
struct CombatLog
{
	int				nCombatType;	// 战斗类型
	int				nCombatRst;		// 战斗结果
	char			szAttackName[32];	// 攻方名字
	char			szDefenseName[32];	// 守方名字
	int				nNumA;			// 攻方武将数量
	int				nNumD;			// 守方武将数量
	int				nAttackHeadID;
	int				nDefenseHeadID;
	tty_id_t		nObjID;
	int				nPosX;
	int				nPosY;
	// 后面跟 武将信息 CombatUnit4Log
	// 后面跟 附加信息，根据 nCombatType 区分
};
// 战斗日志附加信息，讨伐
struct CombatLog_PVE_Raid
{
	int				nExcelID;		// 道具ID
	int				nNum;			// 道具数量
};
// 战斗日志附加信息，玩家对战
struct CombatLog_PVP
{
	int				nGold;			// 损失金币
	int				nPopulation;	// 掠夺人口
	int				nAddPopulation;	// 实际增加
	int				nCup;			// 奖杯
};
// 战斗日志附加信息，世界金矿
struct CombatLog_WorldGoldMine
{
	int				nArea;			// 金矿区域
	int				nClass;			// 金矿等级
	int				nIdx;			// 金矿序号
};
// 战斗日志附加信息，副本掉落
struct	CombatLog_InstanceLoot
{
	int				nNum;			// 掉落数量
	// 后面跟 LootDesc
};
// 战斗日志附加信息，世界名城
struct	CombatLog_WorldCity
{
	tty_id_t		nID;			// 世界名城ID
	int				nCup;			// 获得的奖杯数量
	int				nRes;			// 剩余量
};

// 使用道具(礼包)道具摘要
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

// 使用道具（草药包）
struct DrugDesc
{
	int				nNum;			// 补给的草药数量
};
struct AcctDrugDesc
{
	tty_id_t		nID;			// 账号ID
	int				nNum;			// 补给的草药数量
};

// 世界名城
struct	WorldCity
{
	tty_id_t		nID;			// ID
	int				nClass;			// 等级
	int				nPosX;			// 坐标
	int				nPosY;
	int				nRecover;		// 距离恢复的时间
	int				nCup;			// 奖杯数
	char			szName[64];		// 名字
	int				nInitProf;		// 初始战力
	int				nInitRecover;	// 恢复间隔
	int				nCurProf;		// 当前战力
};
// 奖杯战绩个人
struct	WorldCityLog
{
	int				nID;			// 名次
	tty_id_t		nAccountID;		// 账号ID
	char			szName[32];		// 名字
	int				nHeadID;		// 头像
	int				nDiamond;		// 钻石数
	int				nCup;			// 奖杯数
};
// 奖杯战绩联盟
struct	WorldCityLogAlliance
{
	int				nID;			// 名次
	tty_id_t		nAllianceID;	// 联盟ID
	char			szName[32];		// 名字
	int				nDiamond;		// 钻石数
	int				nCup;			// 奖杯数
};
// 奖杯排名个人
struct	WorldCityRankMan
{
	int				nID;			// 名次
	tty_id_t		nAccountID;		// 账号ID
	char			szName[32];		// 名字
	int				nHeadID;		// 头像
	int				nLevel;			// 等级
	int				nCup;			// 奖杯数
};
// 奖杯排名联盟
struct	WorldCityRankAlliance
{
	int				nID;			// 名次
	tty_id_t		nAllianceID;	// 联盟ID
	char			szName[32];		// 名字
	int				nLevel;			// 等级
	int				nCup;			// 奖杯数
};


#pragma pack()


#endif

