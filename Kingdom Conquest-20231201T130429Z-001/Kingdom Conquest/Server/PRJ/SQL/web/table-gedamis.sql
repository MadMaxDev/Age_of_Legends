
-- 64位ID生成 --
create table global_id (
	svrgrp_id bigint not null default 0,
	dbs_id bigint not null default 0,
	asc_id bigint not null default 0
)engine=innodb default charset=utf8;

-- 刷新的武将
drop table if exists refresh_heros;
create table refresh_heros (
	hero_id bigint unsigned not null default 0,
	account_id bigint unsigned not null default 0,
	name char(32) not null default '',
	level int unsigned not null default 0,
	profession int unsigned not null default 0,
	model int unsigned not null default 0,

	attack int unsigned not null default 0,
	defense int unsigned not null default 0,
	health int unsigned not null default 0,
	leader int unsigned not null default 0,
	grow float(32,2) not null default 0,

	slot_idx int unsigned not null default 0,

	grow_max float(32,2) not null default 0,

	primary key (hero_id),
	index (account_id)
)engine=innodb default charset=utf8;

-- 已雇佣的武将
drop table if exists hire_heros;
create table hire_heros (
	hero_id bigint unsigned not null default 0,
	account_id bigint unsigned not null default 0,
	name char(32) not null default '',
	level int unsigned not null default 0,
	profession int unsigned not null default 0,
	model int unsigned not null default 0,

	attack_0 int unsigned not null default 0,
	attack int unsigned not null default 0,
	attack_add int unsigned not null default 0,
	attack_bonus_val int unsigned not null default 0,
	attack_bonus_pct float(32,2) not null default 0,
	defense_0 int unsigned not null default 0,
	defense int unsigned not null default 0,
	defense_add int unsigned not null default 0,
	defense_bonus_val int unsigned not null default 0,
	defense_bonus_pct float(32,2) not null default 0,
	health_0 int unsigned not null default 0,
	health int unsigned not null default 0,
	health_add int unsigned not null default 0,
	health_bonus_val int unsigned not null default 0,
	health_bonus_pct float(32,2) not null default 0,
	leader_0 int unsigned not null default 0,
	leader int unsigned not null default 0,
	leader_add int unsigned not null default 0,
	leader_bonus_val int unsigned not null default 0,
	leader_bonus_pct float(32,2) not null default 0,

	grow float(32,2) not null default 0,

	exp int unsigned not null default 0,

	army_type int unsigned not null default 0,
	army_level int unsigned not null default 0,
	army_num int unsigned not null default 0,
	army_attack int unsigned not null default 0,
	army_defense int unsigned not null default 0,
	army_health int unsigned not null default 0,
	army_prof int unsigned not null default 0,

	status int unsigned not null default 0,

	healthstate int unsigned not null default 100,

	grow_max float(32,2) not null default 0,

	primary key (hero_id),
	index (account_id)
)engine=innodb default charset=utf8;

-- 酒馆
drop table if exists taverns;
create table taverns (
	account_id bigint unsigned not null default 0,
	refresh_time int unsigned not null default 0,
	level int not null default 0,
	hero_1 bigint unsigned not null default 0,
	hero_2 bigint unsigned not null default 0,
	hero_3 bigint unsigned not null default 0,
	hero_4 bigint unsigned not null default 0,
	hero_5 bigint unsigned not null default 0,
	hero_6 bigint unsigned not null default 0,
	primary key (account_id)
)engine=innodb default charset=utf8;

-- 战斗
drop table if exists combats;
create table combats (
	combat_id bigint unsigned not null default 0,
	combat_type int unsigned not null default 0,
	account_id bigint unsigned not null default 0,
	class_id bigint unsigned not null default 0,
	obj_id bigint unsigned not null default 0,
	begin_time int unsigned not null default 0,
	use_time int unsigned not null default 0,
	status int unsigned not null default 0,
	attack_data blob not null,
	defense_data blob not null,
	misc_data blob not null,
	attack_name char(32) not null default '',
	defense_name char(32) not null default '',
	combat_rst int unsigned not null default 0,
	loot_list blob not null,
	primary key (combat_id),
	index (account_id),
	index (obj_id)
)engine=innodb default charset=utf8;

-- 战斗武将
drop table if exists combat_heros;
create table combat_heros (
	combat_id bigint unsigned not null default 0,
	slot_idx int unsigned not null default 0,
	hero_id bigint unsigned not null default 0,
	index (combat_id)
)engine=innodb default charset=utf8;

-- 城防
drop table if exists citydefenses;
create table citydefenses (
	account_id bigint unsigned not null default 0,
	hero_1 bigint unsigned not null default 0,
	hero_2 bigint unsigned not null default 0,
	hero_3 bigint unsigned not null default 0,
	hero_4 bigint unsigned not null default 0,
	hero_5 bigint unsigned not null default 0,
	auto_supply int unsigned not null default 0,
	primary key (account_id)
)engine=innodb default charset=utf8;

-- 道具
drop table if exists items;
create table items (
	item_id bigint unsigned not null default 0,
	account_id bigint unsigned not null default 0,
	excel_id int unsigned not null default 0,
	num int unsigned not null default 0,
	primary key (item_id),
	index (account_id)
)engine=innodb default charset=utf8;

-- 武将装备的道具
drop table if exists hero_items;
create table hero_items (
	hero_id bigint unsigned not null default 0,
	account_id bigint unsigned not null default 0,
	equip_type int unsigned not null default 0,
	excel_id int unsigned not null default 0,
	item_id bigint unsigned not null default 0,
	index (hero_id),
	index (account_id)
)engine=innodb default charset=utf8;

-- 装备的镶嵌
drop table if exists item_slots;
create table item_slots (
	item_id bigint unsigned not null default 0,
	account_id bigint unsigned not null default 0,
	slot_idx int unsigned not null default 0,
	slot_type int unsigned not null default 0,
	excel_id int unsigned not null default 0,
	src_id bigint unsigned not null default 0,
	primary key (item_id,slot_idx),
	index (account_id)
)engine=innodb default charset=utf8;

-- 已完成任务，不可重复任务
drop table if exists only_quests;
create table only_quests (
	account_id bigint unsigned not null default 0,
	excel_id int unsigned not null default 0,
	done_tick int unsigned not null default 0,
	primary key (account_id,excel_id)
)engine=innodb default charset=utf8;

-- 已完成任务，可重复任务
drop table if exists dup_quests;
create table dup_quests (
	account_id bigint unsigned not null default 0,
	excel_id int unsigned not null default 0,
	done_tick int unsigned not null default 0,
	day_times int unsigned not null default 0,
	primary key (account_id,excel_id)
)engine=innodb default charset=utf8;

-- 副本实例
drop table if exists instance;
create table instance (
	instance_id bigint unsigned not null default 0,
	excel_id int unsigned not null default 0,
	creator_id bigint unsigned not null default 0,
	class_id int unsigned not null default 0,
	cur_player int unsigned not null default 0,
	cur_level int unsigned not null default 0,
	cur_status int unsigned not null default 0,
	begin_time int unsigned not null default 0,
	loot_list blob not null,
	auto_combat int unsigned not null default 0,
	auto_supply int unsigned not null default 0,
	retry_times int not null default 0,
	stop_level int unsigned not null default 0,
	create_time int unsigned not null default 0,
	primary key (instance_id),
	index (create_time),
	index (excel_id)
)engine=innodb default charset=utf8;

-- 副本中的玩家
drop table if exists instance_player;
create table instance_player (
	account_id bigint unsigned not null default 0,
	excel_id int unsigned not null default 0,
	instance_id bigint unsigned not null default 0,
	hero_id blob not null,
	loot_list blob not null,
	done_tick int unsigned not null default 0,
	day_times_free int unsigned not null default 0,
	day_times_fee int unsigned not null default 0,
	day_times_free_buffer int unsigned not null default 0,
	day_times_fee_buffer int unsigned not null default 0,
	status int unsigned not null default 0,
	join_time int unsigned not null default 0,
	primary key (account_id,excel_id),
	index (instance_id)
)engine=innodb default charset=utf8;

-- 战斗时间事件
drop table if exists te_combat;
create table te_combat (
	te_id bigint unsigned not null default 0,
	te_type int unsigned not null default 0,
	combat_id bigint unsigned not null default 0,
	primary key (te_id),
	unique key (combat_id)
)engine=innodb default charset=utf8;

-- 世界金矿
drop table if exists world_goldmine;
create table world_goldmine (
	area int unsigned not null default 0,
	class int unsigned not null default 0,
	idx int unsigned not null default 0,
	account_id bigint unsigned not null default 0,
	safe_time int unsigned not null default 0,
	xlock int unsigned not null default 0,
	hero_1 bigint unsigned not null default 0,
	hero_2 bigint unsigned not null default 0,
	hero_3 bigint unsigned not null default 0,
	hero_4 bigint unsigned not null default 0,
	hero_5 bigint unsigned not null default 0,
	primary key (area,class,idx),
	index (account_id)
)engine=innodb default charset=utf8;

-- 世界金矿时间事件
drop table if exists te_worldgoldmine;
create table te_worldgoldmine (
	te_id bigint unsigned not null default 0,
	te_type int unsigned not null default 0,
	area int unsigned not null default 0,
	class int unsigned not null default 0,
	idx int unsigned not null default 0,
	account_id bigint unsigned not null default 0,
	primary key (te_id),
	unique key (area,class,idx),
	unique key (account_id)
)engine=innodb default charset=utf8;

-- 定时事件（固定小时）
drop table if exists te_fixhour;
create table te_fixhour (
	te_id bigint unsigned not null default 0,
	te_type int unsigned not null default 0,
	id int unsigned not null default 0,
	next_hour int unsigned not null default 0,
	primary key (te_id)
)engine=innodb default charset=utf8;

-- 世界名城
drop table if exists world_city;
create table world_city (
	id bigint unsigned not null default 0,
	name char(64) not null default '',
	class int unsigned not null default 0,
	pos_x int unsigned not null default 0,
	pos_y int unsigned not null default 0,
	res int unsigned not null default 0,
	can_build int unsigned not null default 0,
	floor2 int unsigned not null default 0,
	terrain_type int unsigned not null default 0,
	last_combat_time int unsigned not null default 0,
	army_1 blob not null,
	army_2 blob not null,
	army_3 blob not null,
	army_4 blob not null,
	army_5 blob not null,
	combat_id blob not null,
	win_id bigint unsigned not null default 0,
	primary key (id),
	unique key (pos_x,pos_y)
)engine=innodb default charset=utf8;

-- 世界名城恢复
drop table if exists te_worldcityrecover;
create table te_worldcityrecover(
	te_id bigint unsigned not null default 0,
	te_type int unsigned not null default 0,
	id bigint unsigned not null default 0,
	primary key (te_id),
	unique key (id)
)engine=innodb default charset=utf8;

-- 世界名城刷新记录
drop table if exists world_city_history;
create table world_city_history(
	id int unsigned not null default 0,
	create_time int unsigned not null default 0,
	primary key (id)
)engine=innodb default charset=utf8;
-- 世界名城战绩
drop table if exists world_city_log;
create table world_city_log(
	id int unsigned not null default 0,
	acct_id bigint unsigned not null default 0,
	alliance_id bigint unsigned not null default 0,
	cup int unsigned not null default 0,
	diamond int unsigned not null default 0,
	primary key (id,acct_id)
)engine=innodb default charset=utf8;

-- 充值尝试记录
drop table if exists recharge_try;
create table recharge_try(
	acct_id bigint unsigned not null default 0,
	time int unsigned not null default 0,
	diamond int unsigned not null default 0
)engine=innodb default charset=utf8;


-- -----------------------------------------------------

-- 武将品质配置表
drop table if exists excel_hero_class;
create table excel_hero_class (
	class_id int not null default 0,
	attack_min int not null default 0,
	defense_min int not null default 0,
	health_min int not null default 0,
	init_set int not null default 0,
	grow_max float(32,2) not null default 0,
	primary key (class_id)
)engine=innodb default charset=utf8;

-- 武将成长配置表
drop table if exists excel_hero_grow;
create table excel_hero_grow (
	grow float(32,2) not null default 0,
	percent float(32,2) not null default 0,
	primary key (grow)
)engine=innodb default charset=utf8;

-- 酒馆刷新配置表
drop table if exists excel_tavern_refresh;
create table excel_tavern_refresh (
	level int not null default 0,
	ref_interval int not null default 0,
	ref_prob blob not null,
	primary key (level)
)engine=innodb default charset=utf8;

-- 军事讨伐任务兵力配置
drop table if exists excel_combat_raid;
create table excel_combat_raid (
	raid_id int not null default 0,
	raid_name char(32) not null default '',
	raid_word blob not null,
	army_deploy char(10) not null default '',
	army_data char(10) not null default '',
	loot_list blob not null,
	char_exp int not null default 0,
	hero_exp int not null default 0,
	primary key (raid_id)
)engine=innodb default charset=utf8;

-- 武将名字表
drop table if exists excel_hero_name;
create table excel_hero_name (
	family_name char(32) not null default '',
	middle_1 char(32) not null default '',
	single_1 char(32) not null default '',
	double_1 char(32) not null default '',
	middle_2 char(32) not null default '',
	single_2 char(32) not null default '',
	double_2 char(32) not null default ''
)engine=innodb default charset=utf8;

-- 道具合成表
drop table if exists excel_item_compos;
create table excel_item_compos (
	dst_id int unsigned not null default 0,
	src blob not null
)engine=innodb default charset=utf8;

-- 道具分解表
drop table if exists excel_item_discompos;
create table excel_item_discompos (
	src_id int unsigned not null default 0,
	dst blob not null
)engine=innodb default charset=utf8;

-- 任务表
drop table if exists excel_quest_list;
create table excel_quest_list (
	excel_id int unsigned not null default 0,
	title blob not null,
	content blob not null,
	target_word blob not null,
	awa_word blob not null,
	isdup int not null default 0,

	open_year char(16) not null default '',
	open_month char(8) not null default '',
	open_day char(8) not null default '',
	open_weekday char(8) not null default '',
	open_hour char(8) not null default '',

	day_times int not null default 0,

	pre_level int not null default 0,
	pre_vip int not null default 0,
	pre_develop int not null default 0,
	pre_population int not null default 0,
	pre_soldier int not null default 0,
	pre_hero int not null default 0,
	pre_build blob not null,
	pre_item blob not null,
	
	use_time int not null default 0,

	awa_gold int not null default 0,
	awa_crystal int not null default 0,
	awa_diamond int not null default 0,
	awa_exp int not null default 0,
	awa_item blob not null,

	cost_gold int unsigned not null default 0,
	cost_crystal int unsigned not null default 0,
	cost_diamond int unsigned not null default 0,
	cost_item blob not null,

	gold_bonus float(32,2) not null default 0,

	pvp_attack int unsigned not null default 0,
	pvp_attack_win int unsigned not null default 0,
	pvp_defense int unsigned not null default 0,
	pvp_defense_win int unsigned not null default 0,
	pvp_attack_win_day int unsigned not null default 0,
	pvp_defense_win_day int unsigned not null default 0,

	type int unsigned not null default 0,

	wangzhe int unsigned not null default 0,
	zhengzhan int unsigned not null default 0,
	
	vip int unsigned not null default 0,
	
	zhengzhan_1 int unsigned not null default 0,
	zhengzhan_2 int unsigned not null default 0,
	zhengzhan_3 int unsigned not null default 0,
	zhengzhan_4 int unsigned not null default 0,
	zhengzhan_5 int unsigned not null default 0,
	zhengzhan_6 int unsigned not null default 0,
	zhengzhan_7 int unsigned not null default 0,
	zhengzhan_8 int unsigned not null default 0,
	zhengzhan_9 int unsigned not null default 0,
	zhengzhan_10 int unsigned not null default 0,
	zhengzhan_11 int unsigned not null default 0,
	zhengzhan_12 int unsigned not null default 0,
	zhengzhan_13 int unsigned not null default 0,
	zhengzhan_14 int unsigned not null default 0,
	zhengzhan_15 int unsigned not null default 0,
	zhengzhan_16 int unsigned not null default 0,
	zhengzhan_17 int unsigned not null default 0,
	zhengzhan_18 int unsigned not null default 0,
	zhengzhan_19 int unsigned not null default 0,
	zhengzhan_20 int unsigned not null default 0,

	primary key (excel_id)
)engine=innodb default charset=utf8;

-- 副本列表
drop table if exists excel_instance_list;
create table excel_instance_list (
	excel_id int unsigned not null default 0,
	title blob not null,

	open_year char(16) not null default '',
	open_month char(8) not null default '',
	open_day char(8) not null default '',
	open_weekday char(8) not null default '',
	open_hour char(8) not null default '',
	
	day_times_free int not null default 0,
	day_times_fee int not null default 0,
	player_num int not null default 0,
	
	pre_level int not null default 0,

	retry_times int unsigned not null default 0,

	cost_gold int unsigned not null default 0,
	cost_crystal int unsigned not null default 0,
	cost_diamond int unsigned not null default 0,
	cost_item blob not null,

	primary key (excel_id)
)engine=innodb default charset=utf8;

-- 副本详细配置，百战不殆
drop table if exists excel_baizhanbudai;
create table excel_baizhanbudai (
	level_id int unsigned not null default 0,
	army_deploy char(10) not null default '',
	army_data char(10) not null default '',
	awa_exp int unsigned not null default 0,
	awa_gold int unsigned not null default 0,
	loot_table blob not null,
	loot_num int unsigned not null default 0,
	awa_exp_char int unsigned not null default 0,
	primary key (level_id)
)engine=innodb default charset=utf8;

-- 副本详细配置，百战不殆(史诗级)
drop table if exists excel_baizhanbudai_epic;
create table excel_baizhanbudai_epic (
	level_id int unsigned not null default 0,
	army_deploy char(10) not null default '',
	army_data char(10) not null default '',
	awa_exp int unsigned not null default 0,
	awa_gold int unsigned not null default 0,
	loot_table blob not null,
	loot_num int unsigned not null default 0,
	awa_exp_char int unsigned not null default 0,
	primary key (level_id)
)engine=innodb default charset=utf8;

-- 战斗通用配置
drop table if exists excel_combat;
create table excel_combat (
	excel_id int unsigned not null default 0,
	go_time int unsigned not null default 0,
	back_time int unsigned not null default 0,
	primary key (excel_id)
)engine=innodb default charset=utf8;

-- 世界金矿
drop table if exists excel_worldgoldmine;
create table excel_worldgoldmine (
	area_id int unsigned not null default 0,
	area_name char(64) not null default '',
	clear_time int unsigned not null default 0,
	top_cfg blob not null,
	sen_cfg blob not null,
	mid_cfg blob not null,
	bas_cfg blob not null,
	primary key (area_id)
)engine=innodb default charset=utf8;

-- 世界金矿产出
drop table if exists excel_worldgoldmine_out;
create table excel_worldgoldmine_out (
	level int unsigned not null default 0,
	pct float(32,2) not null default 0,
	primary key (level)
)engine=innodb default charset=utf8;

-- 世界金矿战斗配置
drop table if exists excel_worldgoldmine_combat;
create table excel_worldgoldmine_combat (
	class int unsigned not null default 0,
	army_data blob not null,
	primary key (class)
)engine=innodb default charset=utf8;

-- 定时事件（固定小时）
drop table if exists excel_fixhour;
create table excel_fixhour (
	id int unsigned not null default 0,
	hour blob not null,
	primary key (id)
)engine=innodb default charset=utf8;

-- 兵种
drop table if exists excel_army;
create table excel_army (
	id int unsigned not null default 0,
	name char(32) not null default '',
	crush blob not null,
	bonus float(32,2) not null default 0,
	primary key (id)
)engine=innodb default charset=utf8;

-- 副本详细配置，南征北战
drop table if exists excel_nanzhengbeizhan;
create table excel_nanzhengbeizhan (
	class_id int unsigned not null default 0,
	level_id int unsigned not null default 0,
	army_deploy char(10) not null default '',
	army_data char(10) not null default '',
	awa_exp int unsigned not null default 0,
	awa_gold int unsigned not null default 0,
	loot_table blob not null,
	loot_num int unsigned not null default 0,
	awa_exp_char int unsigned not null default 0,
	primary key (class_id,level_id)
)engine=innodb default charset=utf8;

-- 疗伤效果
drop table if exists excel_heal;
create table excel_heal(
	level int unsigned not null default 0,
	val int unsigned not null default 0,
	primary key (level)
)engine=innodb default charset=utf8;

-- 草药
drop table if exists excel_drug;
create table excel_drug(
	level int unsigned not null default 0,
	max int unsigned not null default 0,
	primary key (level)
)engine=innodb default charset=utf8;

-- 可招募武将数量限制，君主等级
drop table if exists excel_hero_num;
create table excel_hero_num(
	level int unsigned not null default 0,
	num int unsigned not null default 0,
	primary key (level)
)engine=innodb default charset=utf8;

-- 杂项定义
drop table if exists excel_cmndef;
create table excel_cmndef(
	id int unsigned not null default 0,
	val_int int unsigned not null default 0,
	word blob not null,
	primary key (id)
)engine=innodb default charset=utf8;

-- 世界名城兵力
drop table if exists excel_worldcity_combat;
create table excel_worldcity_combat(
	id int unsigned not null default 0,
	army_data blob not null,
	recover int unsigned not null default 0,
	primary key (id)
)engine=innodb default charset=utf8;

-- 世界名城产出
drop table if exists excel_worldcity_out;
create table excel_worldcity_out(
	id int unsigned not null default 0,
	total int unsigned not null default 0,
	one int unsigned not null default 0,
	name blob not null,
	primary key (id)
)engine=innodb default charset=utf8;

-- 世界名城奖励
drop table if exists excel_worldcity_bonus;
create table excel_worldcity_bonus(
	id int unsigned not null default 0,
	diamond int unsigned not null default 0,
	primary key (id)
)engine=innodb default charset=utf8;

