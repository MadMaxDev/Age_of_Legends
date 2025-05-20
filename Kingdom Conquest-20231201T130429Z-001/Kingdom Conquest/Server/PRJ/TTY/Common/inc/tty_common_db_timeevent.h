#ifndef __tty_common_db_timeevent_H__
#define __tty_common_db_timeevent_H__

// time_event 大类
enum
{
	te_type_building				= 1,		// 建筑时间事件
	te_type_once_perperson			= 2,		// 每个瞬间每个人只能有一次的时间事件(如许愿树、人口增长)
	te_type_production				= 3,		// 生产事件，目前只有金矿生产，需要记录产量
	te_type_research				= 4,		// 研究
	te_type_alliance_building		= 5,		// 联盟建设
	te_type_hero_training			= 6,		// 将领训练
	te_type_alliance_trade			= 7,		// 联盟跑商
	te_type_alliance_donate_soldier	= 8,		// 联盟送兵
	te_type_alli_instance_march_advance	= 9,	// 联盟副本前往战斗
	te_type_alli_instance_march_back	= 10,	// 联盟副本战斗返回

	te_type_combat_go				= 101,		// 战斗前往
	te_type_combat_back				= 102,		// 战斗返回

	te_type_worldgoldmine_gain		= 105,		// 世界金矿收获

	te_type_fixhour					= 110,		// 定时事件（固定小时）

	te_type_worldcityrecover		= 120,		// 世界名城恢复
};

// te_type_building,建筑时间事件的子类
enum
{
	te_subtype_building_build		= 1,		// 建造
	te_subtype_building_upgrade		= 2,		// 升级
};

// te_type_once_perperson,一人一次时间事件的子类
enum
{
	te_subtype_once_christmas_tree	= 1,		// 圣诞树成熟事件
	te_subtype_once_population		= 2,		// 人口增长事件
	te_subtype_once_build_num		= 3,		// 增加建筑队列事件
};

// te_type_production,生产事件子类
enum
{
	te_subtype_production_gold				= 1,		// 黄金生产
	te_subtype_production_christmas_tree	= 2,		// 圣诞树成熟
	te_subtype_production_gold_protection	= 3,		// 黄金保护时间
};

// te_type_alliance_build,联盟建造事件
enum
{
	te_subtype_alliance_building_build		= 1,		// 建造
	te_subtype_alliance_building_upgrade	= 2,		// 升级
};

// te_type_alliance_donate_soldier
enum
{
	te_subtype_ads_send						= 1,		// 派遣
	te_subtype_ads_recall					= 2,		// 召回
};

// relation事件
enum
{
	relation_event_watering_christmas_tree		= 1,		// 浇水
	relation_event_steal_gold					= 2,		// 偷黄金,data_0->auto_id,data_1->production
};

// relation_log_type
enum
{
	relation_log_watering_christmas_tree		= 1,		// 浇水
	relation_log_steal_gold						= 2,		// 偷黄金
};

// wait_deal_event_soon,需要立即处理的事件
enum
{
	wdes_type_gen_world_famous_city				= 1,		// 生成世界名城
};

// lock类型
enum
{
	lock_deal_adcolony							= 1,		// 处理adcolony_transaction
	lock_deal_excel_time_event					= 2,		// 处理excel时间事件
	lock_get_te									= 3,		// 获取时间事件的lock
	lock_deal_gold_market_te					= 4,		// 处理金币市场
};

// instance_status
enum
{
	instance_status_normal						= 0,		// 无状态(还未开始)
	instance_status_start						= 1,		// 副本已经开始
	instance_status_fight						= 2,		// 战斗
};
// instance_player_status
enum
{
	instance_player_status_normal				= 0,		// 无状态
	instance_player_status_ready				= 1,		// 准备就绪
	instance_player_status_start				= 2,		// 副本已经开始
};
// march_type,行军类型
enum
{
	march_type_advance							= 1,		// 前往
	march_type_return							= 2,		// 返回
	march_type_recall							= 3,		// 召回
};
// combat_type,战斗类型
enum
{
	combat_type_alli_instance					= 102,		// 联盟副本
	combat_type_arena							= 200,		// 竞技场挑战
};
enum
{
	combat_result_none							= 0,		// 无
	combat_result_win							= 1,		// 胜利
	combat_result_lose							= 2,		// 失败
};
#endif
