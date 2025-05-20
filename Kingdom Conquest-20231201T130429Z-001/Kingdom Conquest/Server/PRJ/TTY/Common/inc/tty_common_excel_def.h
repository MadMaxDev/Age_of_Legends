#ifndef __tty_common_excel_def_H__
#define __tty_common_excel_def_H__

namespace n_pngs
{
enum
{
	money_type_none				= 0,		// 无
	money_type_diamond			= 1,		// 钻石
	money_type_crystal			= 2,		// 水晶
	money_type_gold				= 3,		// 黄金
	money_type_alliance_contribute				= 4,		// 联盟贡献度也可以作为一种通用货币
};
enum
{
	money_use_type_none							= 0,
	money_use_type_buy_item						= 1,		// 商店购买道具
	money_use_type_accelerate_build				= 2,		// 加速建筑
	money_use_type_accelerate_goldore			= 3,		// 金矿生产加速
	money_use_type_accelerate_research			= 4,		// 加速研究
	money_use_type_accelerate_march				= 5,		// 行军加速
	money_use_type_lottery						= 6,		// 抽奖
	money_use_type_change_char_name				= 7,		// 君主改名
	money_use_type_change_hero_name				= 8,		// 英雄改名
	money_use_type_change_alliance_name			= 9,		// 联盟改名
	money_use_type_store_buy_gold				= 10,		// 购买黄金
	money_use_type_arena_pay					= 11,		// 竞技场挑战
	money_use_type_gold_market					= 12,		// 黄金市场
};
// 黄金产出类型
enum
{
	gold_produce_none							= 0,
	gold_produce_goldore						= 1,		// 金矿生产
	gold_produce_raid							= 2,		// 玩家之间掠夺
	gold_produce_steal							= 3,		// 金矿偷取
	gold_produce_task							= 4,		// 任务
	gold_produce_store_buy						= 5,		// 商店购买
	gold_produce_sell_item						= 6,		// 卖出道具
	gold_produce_lottery						= 7,		// 抽奖
	gold_produce_gold_market					= 8,		// 金币市场
};
// 黄金消耗
enum
{
	gold_cost_none								= 0,
	gold_cost_conscript_soldier					= 1,		// 招兵
	gold_cost_upgrade_soldier					= 2,		// 升级兵
	gold_cost_building							= 3,		// 建筑
	gold_cost_research							= 4,		// 科技研究
	gold_cost_be_raided							= 5,		// 被掠夺
	gold_cost_hero_train						= 6,		// 将领训练
	gold_cost_gold_market						= 7,		// 金币市场
};
// 道具跟踪
enum
{
	item_track_none								= 0,		
	item_track_sell								= 1,		// 售出
};
// table_id
enum
{
	table_id_text								= 5,		// 文本
	table_id_soldier							= 15,		// 士兵表
	tabel_id_alliance_position_right			= 21,		// 联盟职位权力
	table_id_lottery_probability				= 30,		// 抽奖概率表
	table_id_lottery_content					= 31,		// 抽奖内容表
	table_id_lottery_content_alliance			= 34,		// 联盟抽奖内容表
	table_id_player_birthplace					= 37,		// 玩家出生点配置表
	table_id_world_famous_city					= 42,		// 世界名城
	table_id_world_resource						= 217,		// 世界资源
};
// 私人关系,pr->personal relation
enum
{
	pr_type_friend				= 1,		// 好友
	pr_type_enemy				= 2,		// 仇敌
};

enum
{
	alliance_position_none			= 0,	// 不是该联盟的
	alliance_position_leader		= 1,	// 盟主
	alliance_position_vice_leader	= 2,	// 副盟主
	alliance_position_manager		= 3,	// 管事
	alliance_position_member		= 4,	// 成员
};

enum
{
	apr_col_id						= 0,	// id
	apr_col_name					= 1,	// 职位名字
};

// 用到的文本id,excel_text
enum
{
	// pl->private_log
	text_id_pl_expel_alliance	= 5,		// 被开除出联盟
	text_id_pl_exit_alliance	= 7,		// 退出联盟
	text_id_pl_set_alliance_pos = 9,		// 设置职位 

	// al->alliance_log
	text_id_al_expel_alliance	= 6,		// 开除出联盟
	text_id_al_exit_alliance	= 8,		// 退出联盟
	text_id_al_set_alliance_pos	= 10,		// 设置职位

	// rl->relation_log
	text_id_rl_watering_christmas_tree	= 11,	// 许愿树浇水
	text_id_rl_steal_gold				= 12,	// 偷窃黄金
	
	text_id_in_alliance			= 13,		// 获准加入联盟
	text_id_in_alliance_new		= 14,		// 新成员加入联盟

	text_id_apply_alliance_refused	= 15,	// 联盟申请被拒绝
	text_id_abdicate_alliance		= 16,	// 禅让盟主
	text_id_dismiss_alliance		= 17,	// 解散联盟
	text_id_set_alliance_position	= 18,	// 设置联盟职位

	text_id_alliance_trade_end		= 19,	// 联盟跑商结束
	text_id_alliance_contribute		= 20,	// 联盟捐献

	text_id_lottery_item			= 21,	// 抽奖抽中道具
	text_id_lottery_crystal			= 22,
	text_id_lottery_gold			= 23,

	text_id_attack_alliance			= 465,	// 攻打联盟成员
};

enum
{
	excel_text_max_length		= 512,		// excel_text中一条文本最大长度
	excel_alliance_position_name_length	= 32,	// 联盟职位名字长度
};
struct ExcelText
{
	unsigned int	nExcelID;
	char			szText[excel_text_max_length];

	ExcelText()
	: nExcelID(0)
	{
		szText[0]	= 0;
	}
};

// index_list各列的枚举值
enum
{
	index_col_comment			= 0,
	index_col_table_id			= 1,
	index_col_file_name			= 2,
	index_col_excel_id			= 3,
	index_col_begin_row			= 4,
	index_col_use_type			= 5,
	index_col_column_type		= 6,
	index_col_mysql_table		= 7,
	index_col_checksum			= 8,
	index_col_max				= 9,
};
// index_list的使用类型枚举值
enum
{
	use_type_all				= 0,		// 客户端服务器都加载
	use_type_server				= 1,		// 服务器加载
	use_type_client				= 2,		// 客户端加载
	use_type_none				= 3,		// 都不加载
};
// lp->lottery_probability
enum
{
	lp_col_choice				= 0,		// 概率选项
	lp_col_probability			= 1,		// 概率
};
// lc->lottery_content
enum
{
	lc_col_type					= 0,		// 奖励类型
	lc_col_data					= 1,		// 奖励数据
	lc_col_probability_choice	= 2,		// 奖励概率选项
	lc_col_is_top_reward		= 3,		// 是否需要将这些奖励记录日志
};
struct ExcelAllianceCongress 
{
	unsigned int	nLevel;
	unsigned int	nDevelopment;
	unsigned int	nMemberNum;
	unsigned int	nViceLeaderNum;
	unsigned int	nManagerNum;
};
struct ExcelAlliancePosition 
{
	unsigned int	nExcelID;
	char			szName[excel_alliance_position_name_length];
};
// pbp->player_birthplace
enum
{
	pbp_col_playernum			= 0,		// 开放人数起始
	pbp_col_begin_x				= 1,		// 起始位置x
	pbp_col_begin_y				= 2,		// 起始位置y
	pbp_col_end_x				= 3,		// 结束位置x
	pbp_col_end_y				= 4,		// 结束位置y
};
struct ExcelPlayerBirthPlace 
{
	unsigned int	nPlayerNum;
	unsigned int	nBeginX;
	unsigned int	nBeginY;
	unsigned int	nEndX;
	unsigned int	nEndY;
};

enum
{
	wfc_col_type				= 0,
	wfc_col_num					= 1,
	wfc_col_begin_x				= 2,
	wfc_col_begin_y				= 3,
	wfc_col_end_x				= 4,
	wfc_col_end_y				= 5,
};
struct ExcelWorldFamousCity 
{
	unsigned int	nType;
	unsigned int	nNum;
	unsigned int	nBeginX;
	unsigned int	nBeginY;
	unsigned int	nEndX;
	unsigned int	nEndY;
};

enum
{
	wr_col_type					= 0,
	wr_col_level				= 1,
	wr_col_refresh_num			= 2,
	wr_col_total_num			= 3,
	wr_col_begin_x				= 4,
	wr_col_begin_y				= 5,
	wr_col_end_x				= 6,
	wr_col_end_y				= 7,
	wr_col_army_data			= 8,		// 部队等级*数量
	wr_col_gold_loot			= 9,		// 掉落黄金
	wr_col_pop_loot				= 10,		// 掉落人口
	wr_col_crystal_loot			= 11,		// 掉落水晶
};
struct ExcelWorldRes
{
	unsigned int		nType;
	unsigned int		nLevel;
	unsigned int		nRefreshNum;
	unsigned int		nTotalNum;
	unsigned int		nBeginX;
	unsigned int		nBeginY;
	unsigned int		nEndX;
	unsigned int		nEndY;
	char				szArmyData[32];
	char				szGoldLoot[32];
	char				szPopLoot[32];
	char				szCrystalLoot[32];
};

enum
{
	s_col_type					= 0,
	s_col_level					= 2,
	s_col_atk					= 4,
	s_col_def					= 5,
	s_col_hp					= 6,
};
struct ExcelSoldier 
{
	unsigned int		nType;
	unsigned int		nLevel;
	unsigned int		nATK;
	unsigned int		nDEF;
	unsigned int		nHP;
};

enum
{
	excel_te_id_refresh_char_day_te			= 1,		// 刷新角色每日数据
	excel_te_id_refresh_world_famous_city	= 3,		// 刷新世界名城
	excel_te_id_cup_activity_end			= 4,		// 奖杯活动结算(废弃,该事件合入3)
	excel_te_id_refresh_world_res			= 5,		// 刷新世界资源
	excel_te_id_arena_reward				= 6,		// 竞技场奖励发放
};
}

#endif
