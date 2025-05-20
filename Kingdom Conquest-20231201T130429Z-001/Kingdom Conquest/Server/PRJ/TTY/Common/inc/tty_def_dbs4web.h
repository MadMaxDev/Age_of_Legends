#ifndef __tty_def_dbs4web_H__
#define __tty_def_dbs4web_H__

#include "pngs_packet_db4web_logic.h"
#include "tty_game_def.h"
#include "tty_def_gs4web.h"
#include "tty_common_PlayerData4Web.h"

namespace n_pngs
{
// 大指令
enum
{
	P_DBS4WEB_REQ_CMD				= 0x01,
	P_DBS4WEB_RPL_CMD				= 0x02,
};
// 小指令(P_DBS4WEB_REQ_CMD的sub cmd)
// 请求
enum
{
	CMDID_ACCOUNT_ONLINE_REQ		= 0x01,
	CMDID_LOAD_CHAR_REQ				= 0x02,
	CMDID_ACCOUNT_OFFLINE_REQ		= 0x03,
	CMDID_CHAR_CREATE_REQ			= 0x04,
	CMDID_LOAD_PLAYERCARD_REQ		= 0x05,
	CMDID_LOAD_TERRAIN_REQ			= 0x06,
	CMDID_GET_TIMEEVENT_LIST_REQ	= 0x07,
	CMDID_DEAL_BUILDING_TE_REQ		= 0x08,
	CMDID_GET_BUILDING_LIST_REQ		= 0x09,
	CMDID_GET_BUILDING_TE_LIST_REQ	= 0x0A,
	CMDID_BUILD_BUILDING_REQ		= 0x0B,
	CMDID_UPGRADE_BUILDING_REQ		= 0x0C,
	CMDID_DEAL_ONCE_PERPERSON_TE_REQ		= 0x0D,
	CMDID_ADD_ONCE_PERPERSON_TE_REQ	= 0x0E,
	CMDID_DEAL_PRODUCTION_TE_REQ	= 0x0F,
	CMDID_ADD_PRODUCTION_TE_REQ		= 0x10,
	CMDID_FETCH_PRODUCTION_REQ		= 0x11,
	CMDID_GET_PRODUCTION_EVENT_REQ	= 0x12,
	CMDID_GET_PRODUCTION_TE_REQ		= 0x13,
	CMDID_ACCELERATE_REQ			= 0x14,
	CMDID_DEAL_RESEARCH_TE_REQ		= 0x15,
	CMDID_ADD_RESEARCH_TE_REQ		= 0x16,
	CMDID_GET_TECHNOLOGY_REQ		= 0x17,
	CMDID_GET_RESEARCH_TE_REQ		= 0x18,
	CMDID_CONSCRIPT_SOLDIER_REQ		= 0x19,
	CMDID_UPGRADE_SOLDIER_REQ		= 0x1A,
	CMDID_GET_SOLDIER_REQ			= 0x1B,
	// 联盟相关
	CMDID_ACCEPT_ALLIANCE_MEMBER_REQ	= 0x1C,
	CMDID_BUILD_ALLIANCE_BUILDING_REQ	= 0x1D,
	CMDID_CANCEL_JOIN_ALLIANCE_REQ		= 0x1E,
	CMDID_CREATE_ALLIANCE_REQ			= 0x1F,
	CMDID_DEAL_ALLIANCE_BUILDING_TE_REQ	= 0x20,
	CMDID_DISMISS_ALLIANCE_REQ			= 0x21,
	CMDID_EXIT_ALLIANCE_REQ				= 0x22,
	CMDID_EXPEL_ALLIANCE_MEMBER_REQ		= 0x23,
	CMDID_JOIN_ALLIANCE_REQ				= 0x24,
	CMDID_SET_ALLIANCE_POSITION_REQ		= 0x25,
	CMDID_GET_ALLIANCE_INFO_REQ			= 0x26,
	CMDID_GET_ALLIANCE_MEMBER_REQ		= 0x27,
	CMDID_GET_ALLIANCE_BUILDING_TE_REQ	= 0x28,
	CMDID_GET_ALLIANCE_BUILDING_REQ		= 0x29,
	CMDID_GET_ALLIANCE_JOIN_EVENT_REQ	= 0x2A,
	// 邮件相关
	CMDID_ADD_PRIVATE_MAIL_REQ		= 0x2B,
	CMDID_GET_PRIVATE_MAIL_REQ		= 0x2C,
	CMDID_READ_PRIVATE_MAIL_REQ		= 0x2D,
	CMDID_DEL_PRIVATE_MAIL_REQ		= 0x2E,
	CMDID_NEW_PRIVATE_MAIL_REQ		= 0x2F,
	CMDID_GET_PRIVATE_MAIL_1_REQ	= 0x30,		// 获取一封邮件
	
	CMDID_REFUSE_JOIN_ALLIANCE_REQ	= 0x31,		// 拒绝加入联盟申请
	CMDID_ABDICATE_ALLIANCE_REQ		= 0x32,		// 禅让联盟

	// 好友
	CMDID_GET_MY_FRIEND_APPLY_REQ			= 0x33,
	CMDID_GET_OTHERS_FRIEND_APPLY_REQ		= 0x34,
	CMDID_GET_FRIEND_LIST_REQ				= 0x35,
	CMDID_APPROVE_FRIEND_REQ				= 0x36,
	CMDID_APPLY_FRIEND_REQ					= 0x37,
	CMDID_REFUSE_FRIEND_APPLY_REQ			= 0x38,
	CMDID_CANCEL_FRIEND_APPLY_REQ			= 0x39,
	CMDID_DELETE_FRIEND_REQ					= 0x3A,

	CMDID_LOAD_ALLIANCE_REQ			= 0x3B,		// 载入联盟信息
	CMDID_LOAD_ALL_ALLIANCE_ID_REQ	= 0x3C,		// 获取所有的联盟ID

	CMDID_LOAD_EXCEL_TEXT_REQ		= 0x3D,		// 载入文本表格

	CMDID_ADD_ALLIANCE_MAIL_REQ		= 0x3E,		// 发送联盟邮件
	CMDID_ADD_ALLIANCE_LOG_REQ		= 0x3F,		// 添加联盟日志
	CMDID_ADD_PRIVATE_LOG_REQ		= 0x40,		// 添加私人日志
	CMDID_GET_ALLIANCE_MAIL_REQ		= 0x41,		// 获取联盟邮件
	CMDID_GET_ALLIANCE_LOG_REQ		= 0x42,		// 获取联盟日志
	CMDID_GET_PRIVATE_LOG_REQ		= 0x43,		// 获取私人日志

	CMDID_BUY_ITEM_REQ				= 0x44,		// 通过钻石购买道具
	CMDID_SELL_ITEM_REQ				= 0x46,		// 出售道具

	CMDID_GET_RELATION_LOG_REQ		= 0x47,		// 获取关系日志
	CMDID_FETCH_CHRISTMAS_TREE_REQ	= 0x48,		// 收获许愿树
	CMDID_WATERING_CHRISTMAS_TREE_REQ	= 0x49,	// 许愿树浇水
	CMDID_STEAL_GOLD_REQ			= 0x4A,		// 偷窃黄金
	CMDID_ADD_RELATION_LOG_REQ		= 0x4B,		// 添加关系日志
	CMDID_GET_TRAINING_TE_REQ		= 0x4C,		// 获取训练时间事件
	CMDID_ADD_TRAINING_REQ			= 0x4D,		// 添加训练时间事件
	CMDID_EXIT_TRAINING_REQ			= 0x4E,		// 退出训练
	CMDID_DEAL_TRAINING_TE_REQ		= 0x4F,		// 处理训练事件
	CMDID_CONTRIBUTE_ALLIANCE_REQ	= 0x50,		// 联盟捐献
	CMDID_CONVERT_DIAMOND_GOLD_REQ	= 0x51,		// 钻石兑换黄金
	CMDID_JOIN_ALLIANCE_NAME_REQ	= 0x52,		// 通过名字加入联盟
	CMDID_GET_MY_ALLIANCE_JOIN_REQ	= 0x53,		// 获取我的联盟申请事件
	CMDID_LOAD_EXCEL_ALLIANCE_CONGRESS_REQ	= 0x54,	// 载入excel_alliance_congress到内存
	CMDID_SET_ALLI_INTRO_REQ		= 0x55,		// 设置联盟简介
	CMDID_GET_OTHER_GOLDORE_POSITION_REQ	= 0x56,		// 获取别人的金矿信息
	CMDID_GET_OTHER_GOLDORE_PRODUCTION_EVENT_REQ	= 0x57,	// 获取金矿成熟事件
	CMDID_GET_OTHER_GOLDORE_PRODUCTION_TE_REQ		= 0x58,	// 获取金矿时间事件
	CMDID_GET_OTHER_GOLDORE_FETCH_INFO_REQ		= 0x59,		// 获取金矿的摘取信息
	CMDID_GET_OTHER_CHRISTMAS_TREE_INFO_REQ		= 0x5A,		// 获取圣诞树信息
	CMDID_CHECK_DRAW_LOTTERY_REQ	= 0x5B,		// 检验抽奖条件
	CMDID_FETCH_LOTTERY_REQ			= 0x5C,		// 获取抽奖结果
	CMDID_GET_CHRISTMAS_TREE_REQ	= 0x5D,		// 获取许愿树信息
	CMDID_GET_OTHER_GOLDORE_INFO_REQ= 0x5E,		// 获取别人金矿信息
	CMDID_GET_GOLDORE_SMP_INFO_ALL_REQ			= 0x5F,		// 获取一群人的金矿是否有事件发生
	CMDID_GET_ALLIANCE_TRADE_INFO_REQ	= 0x60,		// 获取联盟跑商状态
	CMDID_ALLIANCE_TRADE_REQ		= 0x61,		// 联盟跑商
	CMDID_DEAL_ALLIANCE_TREADE_TE_REQ			= 0x62,		// 处理联盟跑商事件
	CMDID_GET_ENEMY_LIST_REQ		= 0x63,		// 获取仇人列表
	CMDID_DELETE_ENEMY_REQ			= 0x64,		// 删除仇人关系
	CMDID_ADD_ENEMY_REQ				= 0x65,		// 添加仇人
	CMDID_REFRESH_CHAR_ATB_REQ		= 0x66,		// 刷新角色属性从DB到内存
	CMDID_ADD_ALLIANCE_DEV_REQ		= 0x67,		// 添加联盟发展度(负数则减)
	CMDID_LOAD_RANK_LIST_REQ		= 0x68,		// 载入排行榜列表
	CMDID_RECHARGE_DIAMOND_REQ		= 0x69,		// 钻石充值
	CMDID_WRITE_CHAR_SIGNATURE_REQ	= 0x6A,		// 书写角色签名
	CMDID_GET_INSTANCE_ID_REQ		= 0x6B,		// 获取副本only id
	CMDID_CHANGE_NAME_REQ			= 0x6C,		// 修改角色名字
	CMDID_MOVE_POSITION_REQ			= 0x6D,		// 移动城市位置
	CMDID_GET_PAY_SERIAL_REQ		= 0x6E,		// 获取充值序列号
	CMDID_GET_PAY_LIST_REQ			= 0x6F,		// 获取充值请求列表
	CMDID_DEAL_ONE_PAY_REQ			= 0x70,		// 处理一单充值交易
	CMDID_READ_NOTIFICATION_REQ		= 0x71,		// 阅读通知
	CMDID_CHANGE_HERO_NAME_REQ		= 0x72,		// 修改英雄名字
	CMDID_READ_MAIL_ALL_REQ			= 0x73,		// 标记所有邮件为已读
	CMDID_DELETE_MAIL_ALL_REQ		= 0x74,		// 删除所有邮件
	CMDID_ADD_GM_MAIL_REQ			= 0x75,		// 添加GM邮件
	CMDID_GET_PLAYERCARD_REQ		= 0x76,		// 从数据库中载入一个玩家的PlayerCard
	CMDID_DEAL_EXCEL_TE_REQ			= 0x77,		// 处理excel time event
	CMDID_CHANGE_ALLIANCE_NAME_REQ	= 0x78,		// 修改联盟名字
	CMDID_GEN_WORLD_FAMOUS_CITY_REQ	= 0x79,		// 生成世界名城
	CMDID_GET_WORLD_FAMOUS_CITY_REQ	= 0x7A,		// 从数据库中载入名城信息
	CMDID_CUP_ACTIVITY_END_REQ		= 0x7B,		// 奖杯活动结算
	CMDID_FETCH_MAIL_REWARD_REQ		= 0x7C,		// 获取邮件附件奖励
	CMDID_UPLOAD_BILL_REQ			= 0x7D,		// 客户端上传账单数据
	CMDID_SET_BINDED_REQ			= 0x7E,		// 设置绑定账号
	CMDID_ADD_LOTTERY_LOG_REQ		= 0x7F,		// 记录抽奖日志

	CMDID_TAVERN_REFRESH_REQ		= 0x80,		// 酒馆刷新
	CMDID_HIRE_HERO_REQ				= 0x81,		// 招募武将
	CMDID_FIRE_HERO_REQ				= 0x82,		// 解雇武将
	CMDID_LEVELUP_HERO_REQ			= 0x83,		// 武将升级
	CMDID_ADD_GROW_REQ				= 0x84,		// 增加武将成长率
	CMDID_CONFIG_HERO_REQ			= 0x85,		// 武将配兵
	CMDID_START_COMBAT_REQ			= 0x86,		// 开始战斗
	CMDID_STOP_COMBAT_REQ			= 0x87,		// 结束战斗
	CMDID_GET_COMBAT_REQ			= 0x88,		// 获取战斗数据
	CMDID_GET_HERO_REFRESH_REQ		= 0x89,		// 获取刷新武将
	CMDID_GET_HERO_HIRE_REQ			= 0x8A,		// 获取雇佣武将
	CMDID_UPDATE_HERO_REQ			= 0x8B,		// 更新武将数据
	CMDID_LIST_COMBAT_REQ			= 0x8C,		// 获取所有战斗摘要
	CMDID_CONFIG_CITYDEFENSE_REQ	= 0x8D,		// 配置城防武将

	CMDID_GET_ITEM_REQ				= 0x8E,		// 获取道具
	CMDID_ADD_ITEM_REQ				= 0x8F,		// 添加道具
	CMDID_DEL_ITEM_REQ				= 0x90,		// 删除道具
	CMDID_EQUIP_ITEM_REQ			= 0x91,		// 装备道具
	CMDID_DISEQUIP_ITEM_REQ			= 0x92,		// 卸下装备
	CMDID_MOUNT_ITEM_REQ			= 0x93,		// 镶嵌道具
	CMDID_UNMOUNT_ITEM_REQ			= 0x94,		// 卸下镶嵌
	CMDID_COMPOS_ITEM_REQ			= 0x95,		// 合成道具
	CMDID_DISCOMPOS_ITEM_REQ		= 0x96,		// 分解道具

	CMDID_GET_QUEST_REQ				= 0x97,		// 获取当前可接受的任务
	CMDID_DONE_QUEST_REQ			= 0x98,		// 完成任务

	CMDID_GET_INSTANCESTATUS_REQ	= 0xA0,		// 获取玩家副本当前状态
	CMDID_GET_INSTANCEDESC_REQ		= 0xA2,		// 获取副本实例摘要
	CMDID_CREATE_INSTANCE_REQ		= 0xA3,		// 创建副本
	CMDID_JOIN_INSTANCE_REQ			= 0xA4,		// 加入副本
	CMDID_QUIT_INSTANCE_REQ			= 0xA5,		// 退出副本
	CMDID_DESTROY_INSTANCE_REQ		= 0xA6,		// 销毁副本
	CMDID_GET_INSTANCEDATA_REQ		= 0xA7,		// 获取副本数据
	CMDID_CONFIG_INSTANCEHERO_REQ	= 0xA8,		// 配置副本武将
	CMDID_GET_INSTANCELOOT_REQ		= 0xA9,		// 获取副本掉落

	CMDID_EQUIP_ITEM_ALL_REQ		= 0xAA,		// 一键换装
	CMDID_MOUNT_ITEM_ALL_REQ		= 0xAB,		// 一键镶嵌

	CMDID_USE_ITEM_REQ				= 0xAE,		// 使用道具
	CMDID_AUTO_COMBAT_REQ			= 0xAF,		// 自动战斗开关

	CMDID_DEAL_COMBAT_TE_REQ		= 0xB0,		// 获取战斗超时事件
	CMDID_DEAL_COMBAT_BACK_REQ		= 0xB1,		// 进入战斗返回状态

	CMDID_SYNC_CHAR_REQ				= 0xB5,		// 同步数据，君主
	CMDID_SYNC_HERO_REQ				= 0xB6,		// 同步数据，武将

	CMDID_AUTO_SUPPLY_REQ			= 0xB7,		// 自动补给开关
	CMDID_GET_EQUIP_REQ				= 0xB8,		// 获取已装备的装备
	CMDID_GET_GEM_REQ				= 0xB9,		// 获取已镶嵌的宝石
	CMDID_MANUAL_SUPPLY_REQ			= 0xBA,		// 手动补给
	CMDID_REPLACE_EQUIP_REQ			= 0xBB,		// 替换装备

	CMDID_GET_WORLDGOLDMINE_REQ		= 0xC0,		// 获取世界金矿
	CMDID_CONFIG_WORLDGOLDMINE_HERO_REQ		= 0xC1,		// 配置金矿武将
	CMDID_ROB_WORLDGOLDMINE_REQ		= 0xC2,		// 夺取世界金矿
	CMDID_DROP_WORLDGOLDMINE_REQ	= 0xC3,		// 放弃世界金矿
	CMDID_DEAL_WORLDGOLDMINE_GAIN_TE_REQ	= 0xC4,		// 世界金矿收获超时事件
	CMDID_MY_WORLDGOLDMINE_REQ		= 0xC5,		// 我的世界金矿

	CMDID_DEAL_FIXHOUR_TE_REQ		= 0xCA,		// 定时事件（固定小时）
	CMDID_COMBAT_PROF_REQ			= 0xCB,		// 获取战力

	CMDID_LOAD_ARMY_REQ				= 0xCC,		// 加载兵种数据

	CMDID_KICK_INSTANCE_REQ			= 0xD0,		// 踢人
	CMDID_START_INSTANCE_REQ		= 0xD1,		// 副本出征
	CMDID_PREPARE_INSTANCE_REQ		= 0xD2,		// 副本就绪
	CMDID_SUPPLY_INSTANCE_REQ		= 0xD3,		// 副本补给

	CMDID_GET_CITYDEFENSE_REQ		= 0xD4,		// 获取城防武将

	CMDID_USE_DRUG_REQ				= 0xD5,		// 使用草药

	CMDID_ARMY_ACCELERATE_REQ		= 0xDA,		// 行军加速
	CMDID_ARMY_BACK_REQ				= 0xDB,		// 召回

	CMDID_DEAL_WORLDCITYRECOVER_TE_REQ	= 0xDC,	// 世界名城恢复

	CMDID_WORLDCITY_GET_REQ			= 0xDD,		// 获得世界名城信息
	CMDID_WORLDCITY_GETLOG_REQ		= 0xDE,		// 获得世界名城奖杯战绩个人

	CMDID_WORLDCITY_RANK_MAN_REQ		= 0xE0,		// 获得奖杯个人排名
	CMDID_WORLDCITY_RANK_ALLIANCE_REQ	= 0xE1,		// 获得奖杯联盟排名

	CMDID_WORLDCITY_GETLOG_ALLIANCE_REQ		= 0xE2,		// 获得世界名城奖杯战绩联盟

	CMDID_RECHARGE_TRY_REQ			= 0xE3,		// 尝试充值

	CMDID_DEAL_CHAR_DAY_TE_REQ			= 0x1001,	// 处理时间事件
	CMDID_DEAL_ADCOLONY_TE_REQ			= 0x1002,	// 处理AdColony
	CMDID_DISMISS_SOLDIER_REQ			= 0x1003,	// 解散士兵
	CMDID_REFRESH_MAIL_REQ				= 0x1004,	// 刷新新邮件
	CMDID_BAN_CHAR_REQ					= 0x1005,	// 封角色
	CMDID_NEW_ADCOLONY_REQ				= 0x1006,	// 新的ADCOLONY

	CMDID_ALLI_DONATE_SOLDIER_REQ		= 0x1007,	// 联盟送兵
	CMDID_ALLI_RECALL_SOLDIER_REQ		= 0x1008,	// 召回联盟送兵
	CMDID_ALLI_ACCE_SOLDIER_REQ			= 0x1009,	// 加速联盟送兵
	CMDID_GET_DONATE_SOLDIER_QUEUE_REQ	= 0x100A,	// 获取联盟送兵队列
	CMDID_GET_LEFT_DONATE_SOLDIER_REQ	= 0x100B,	// 获取剩余可送兵
	CMDID_DEAL_DONATE_SOLDIER_TE_REQ	= 0x100C,	// 处理送兵事件

	CMDID_ADD_GOLD_DEAL_REQ				= 0x100D,	// 增加黄金交易
	CMDID_BUY_GOLD_IN_MARKET_REQ		= 0x100E,	// 市场买黄金
	CMDID_CANCEL_GOLD_DEAL_REQ			= 0x100F,	// 取消黄金交易
	CMDID_GET_GOLD_DEAL_REQ				= 0x1010,	// 获取交易列表
	CMDID_GET_SELF_GOLD_DEAL_REQ		= 0x1011,	// 获取自己的黄金交易
	CMDID_DEAL_GOLD_MARKET_TE_REQ		= 0x1012,	// 处理黄金交易超时
	CMDID_GEN_WORLD_RES_REQ				= 0x1013,	// 生成世界资源
	CMDID_GET_WORLD_RES_REQ				= 0x1014,	// 从数据库中读取世界资源
	CMDID_REFRESH_DRUG_REQ				= 0x1015,	// 刷新草药

	CMDID_ALLI_INSTANCE_CREATE_REQ			= 0x1100,	// 创建副本
	CMDID_ALLI_INSTANCE_JOIN_REQ			= 0x1101,	// 加入副本
	CMDID_ALLI_INSTANCE_EXIT_REQ			= 0x1102,	// 退出副本
	CMDID_ALLI_INSTANCE_KICK_REQ			= 0x1103,	// 踢出副本
	CMDID_ALLI_INSTANCE_DESTROY_REQ			= 0x1104,	// 销毁副本
	CMDID_ALLI_INSTANCE_CONFIG_HERO_REQ		= 0x1105,	// 配置将领
	CMDID_ALLI_INSTANCE_START_REQ			= 0x1106,	// 副本出征
	CMDID_ALLI_INSTANCE_READY_REQ			= 0x1107,	// 准备就绪
	CMDID_ALLI_INSTANCE_GET_LIST_REQ		= 0x1108,	// 获取副本列表
	CMDID_ALLI_INSTANCE_GET_CHAR_DATA_REQ	= 0x1109,	// 获取副本角色详细信息
	CMDID_ALLI_INSTANCE_GET_HERO_DATA_REQ	= 0x110A,	// 获取副本将领详细信息
	CMDID_ALLI_INSTANCE_GET_STATUS_REQ		= 0x110B,	// 自己的副本状态
	CMDID_ALLI_INSTANCE_GET_LOOT_REQ		= 0x110C,	// 获取掉落
	CMDID_ALLI_INSTANCE_SAVE_HERO_DEPLOY_REQ= 0x110D,	// 保存将领布局
	CMDID_ALLI_INSTANCE_START_COMBAT_REQ	= 0x110E,	// 开始战斗
	CMDID_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_REQ		= 0x110F,	// 处理战斗前往事件
	CMDID_ALLI_INSTANCE_DEAL_MARCH_BACK_REQ			= 0x1110,	// 处理战斗返回事件
	CMDID_ALLI_INSTANCE_DEAL_COMBAT_RESULT_REQ		= 0x1111,	// 处理战斗结果
	CMDID_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_REQ	= 0x1112,	// 获取战斗结果事件
	CMDID_ALLI_INSTANCE_GET_INSTANCE_ID_REQ			= 0x1113,	// 获取副本ID
	CMDID_ALLI_INSTANCE_GET_COMBAT_LOG_REQ			= 0x1114,	// 获取战斗简单日志
	CMDID_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_REQ	= 0x1115,	// 获取战斗详细日志
	CMDID_ALLI_INSTANCE_STOP_COMBAT_REQ		= 0x1116,	// 停止战斗
	CMDID_ALLI_INSTANCE_SUPPLY_REQ			= 0x1117,	// 补给
	CMDID_ALLI_INSTANCE_SYSTEM_DESTROY_REQ	= 0x1118,	// 系统销毁
	CMDID_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_REQ		= 0x1119,	// 获取防守方战斗布局

	CMDID_KICK_CLIENT_ALL_REQ				= 0x1200,	// 踢出所有客户端
	CMDID_GET_KICK_CLIENT_ALL_REQ			= 0x1201,	// 获取踢出所有客户端

	CMDID_GET_HERO_SIMPLE_DATA_REQ			= 0x1202,	// 获取将领简单信息
	CMDID_GET_INSTANCE_SIMPLE_DATA_REQ		= 0x1203,	// 获取简单副本信息

	CMDID_ARENA_UPLOAD_DATA_REQ				= 0x1300,	// 上传竞技场数据
	CMDID_ARENA_GET_RANK_LIST_REQ			= 0x1301,	// 竞技场排行榜
	CMDID_ARENA_GET_STATUS_REQ				= 0x1302,	// 获取状态,挑战列表
	CMDID_ARENA_PAY_REQ						= 0x1303,	// 竞技场付费
	CMDID_ARENA_DEAL_REWARD_EVENT_REQ		= 0x1304,	// 竞技场奖励发送
	CMDID_ARENA_GET_DEPLOY_REQ				= 0x1305,	// 获取竞技场英雄布局

	CMDID_HERO_SUPPLY_REQ					= 0x1400,	// 英雄补给
	CMDID_SET_VIP_DISPLAY_REQ				= 0x1401,	// 设置vip显示

	CMDID_POSITION_MARK_ADD_RECORD_REQ		= 0x1500,	// 添加位置收藏
	CMDID_POSITION_MARK_CHG_RECORD_REQ		= 0x1501,	// 修改位置收藏
	CMDID_POSITION_MARK_DEL_RECORD_REQ		= 0x1502,	// 删除位置收藏
	CMDID_POSITION_MARK_GET_RECORD_REQ		= 0x1503,	// 获取位置收藏
};
// 响应
enum
{
	CMDID_ACCOUNT_ONLINE_RPL		= 0x01,
	CMDID_LOAD_CHAR_RPL				= 0x02,
	CMDID_ACCOUNT_OFFLINE_RPL		= 0x03,
	CMDID_CHAR_CREATE_RPL			= 0x04,
	CMDID_LOAD_PLAYERCARD_RPL		= 0x05,
	CMDID_LOAD_TERRAIN_RPL			= 0x06,
	CMDID_GET_TIMEEVENT_LIST_RPL	= 0x07,
	CMDID_DEAL_BUILDING_TE_RPL		= 0x08,
	CMDID_GET_BUILDING_LIST_RPL		= 0x09,
	CMDID_GET_BUILDING_TE_LIST_RPL	= 0x0A,
	CMDID_BUILD_BUILDING_RPL		= 0x0B,
	CMDID_UPGRADE_BUILDING_RPL		= 0x0C,
	CMDID_DEAL_ONCE_PERPERSON_TE_RPL		= 0x0D,
	CMDID_ADD_ONCE_PERPERSON_TE_RPL	= 0x0E,
	CMDID_DEAL_PRODUCTION_TE_RPL	= 0x0F,
	CMDID_ADD_PRODUCTION_TE_RPL		= 0x10,
	CMDID_FETCH_PRODUCTION_RPL		= 0x11,
	CMDID_GET_PRODUCTION_EVENT_RPL	= 0x12,
	CMDID_GET_PRODUCTION_TE_RPL		= 0x13,
	CMDID_ACCELERATE_RPL			= 0x14,
	CMDID_DEAL_RESEARCH_TE_RPL		= 0x15,
	CMDID_ADD_RESEARCH_TE_RPL		= 0x16,
	CMDID_GET_TECHNOLOGY_RPL		= 0x17,
	CMDID_GET_RESEARCH_TE_RPL		= 0x18,
	CMDID_CONSCRIPT_SOLDIER_RPL		= 0x19,
	CMDID_UPGRADE_SOLDIER_RPL		= 0x1A,
	CMDID_GET_SOLDIER_RPL			= 0x1B,
	// 联盟相关
	CMDID_ACCEPT_ALLIANCE_MEMBER_RPL	= 0x1C,
	CMDID_BUILD_ALLIANCE_BUILDING_RPL	= 0x1D,
	CMDID_CANCEL_JOIN_ALLIANCE_RPL		= 0x1E,
	CMDID_CREATE_ALLIANCE_RPL			= 0x1F,
	CMDID_DEAL_ALLIANCE_BUILDING_TE_RPL	= 0x20,
	CMDID_DISMISS_ALLIANCE_RPL			= 0x21,
	CMDID_EXIT_ALLIANCE_RPL				= 0x22,
	CMDID_EXPEL_ALLIANCE_MEMBER_RPL		= 0x23,
	CMDID_JOIN_ALLIANCE_RPL				= 0x24,
	CMDID_SET_ALLIANCE_POSITION_RPL		= 0x25,
	CMDID_GET_ALLIANCE_INFO_RPL			= 0x26,
	CMDID_GET_ALLIANCE_MEMBER_RPL		= 0x27,
	CMDID_GET_ALLIANCE_BUILDING_TE_RPL	= 0x28,
	CMDID_GET_ALLIANCE_BUILDING_RPL		= 0x29,
	CMDID_GET_ALLIANCE_JOIN_EVENT_RPL	= 0x2A,
	// 邮件相关
	CMDID_ADD_PRIVATE_MAIL_RPL		= 0x2B,
	CMDID_GET_PRIVATE_MAIL_RPL		= 0x2C,
	CMDID_READ_PRIVATE_MAIL_RPL		= 0x2D,
	CMDID_DEL_PRIVATE_MAIL_RPL		= 0x2E,
	CMDID_NEW_PRIVATE_MAIL_RPL		= 0x2F,
	CMDID_GET_PRIVATE_MAIL_1_RPL	= 0x30,		// 获取一封邮件

	CMDID_REFUSE_JOIN_ALLIANCE_RPL	= 0x31,		// 拒绝加入联盟申请
	CMDID_ABDICATE_ALLIANCE_RPL		= 0x32,		// 禅让联盟

	// 好友
	CMDID_GET_MY_FRIEND_APPLY_RPL			= 0x33,
	CMDID_GET_OTHERS_FRIEND_APPLY_RPL		= 0x34,
	CMDID_GET_FRIEND_LIST_RPL				= 0x35,
	CMDID_APPROVE_FRIEND_RPL				= 0x36,
	CMDID_APPLY_FRIEND_RPL					= 0x37,
	CMDID_REFUSE_FRIEND_APPLY_RPL			= 0x38,
	CMDID_CANCEL_FRIEND_APPLY_RPL			= 0x39,
	CMDID_DELETE_FRIEND_RPL					= 0x3A,

	CMDID_LOAD_ALLIANCE_RPL			= 0x3B,		// 载入联盟信息
	CMDID_LOAD_ALL_ALLIANCE_ID_RPL	= 0x3C,		// 获取所有联盟ID

	CMDID_LOAD_EXCEL_TEXT_RPL		= 0x3D,		// 载入文本表格

	CMDID_ADD_ALLIANCE_MAIL_RPL		= 0x3E,		// 发送联盟邮件
	CMDID_ADD_ALLIANCE_LOG_RPL		= 0x3F,		// 添加联盟日志
	CMDID_ADD_PRIVATE_LOG_RPL		= 0x40,		// 添加私人日志
	CMDID_GET_ALLIANCE_MAIL_RPL		= 0x41,		// 获取联盟邮件
	CMDID_GET_ALLIANCE_LOG_RPL		= 0x42,		// 获取联盟日志
	CMDID_GET_PRIVATE_LOG_RPL		= 0x43,		// 获取私人日志

	CMDID_BUY_ITEM_RPL				= 0x44,		// 通过钻石购买道具
	CMDID_SELL_ITEM_RPL				= 0x46,		// 出售道具

	CMDID_GET_RELATION_LOG_RPL		= 0x47,		// 获取关系日志
	CMDID_FETCH_CHRISTMAS_TREE_RPL	= 0x48,		// 收获许愿树
	CMDID_WATERING_CHRISTMAS_TREE_RPL	= 0x49,	// 许愿树浇水
	CMDID_STEAL_GOLD_RPL			= 0x4A,		// 偷窃黄金
	CMDID_ADD_RELATION_LOG_RPL		= 0x4B,		// 添加关系日志
	CMDID_GET_TRAINING_TE_RPL		= 0x4C,		// 获取训练时间事件
	CMDID_ADD_TRAINING_RPL			= 0x4D,		// 添加训练时间事件
	CMDID_EXIT_TRAINING_RPL			= 0x4E,		// 退出训练
	CMDID_DEAL_TRAINING_TE_RPL		= 0x4F,		// 处理训练事件
	CMDID_CONTRIBUTE_ALLIANCE_RPL	= 0x50,		// 联盟捐献
	CMDID_CONVERT_DIAMOND_GOLD_RPL	= 0x51,		// 钻石兑换黄金
	CMDID_JOIN_ALLIANCE_NAME_RPL	= 0x52,		// 通过名字加入联盟
	CMDID_GET_MY_ALLIANCE_JOIN_RPL	= 0x53,		// 获取我的联盟申请事件
	CMDID_LOAD_EXCEL_ALLIANCE_CONGRESS_RPL	= 0x54,	// 载入excel_alliance_congress到内存
	CMDID_SET_ALLI_INTRO_RPL		= 0x55,		// 设置联盟简介
	CMDID_GET_OTHER_GOLDORE_POSITION_RPL	= 0x56,		// 获取别人的金矿信息
	CMDID_GET_OTHER_GOLDORE_PRODUCTION_EVENT_RPL	= 0x57,	// 获取金矿成熟事件
	CMDID_GET_OTHER_GOLDORE_PRODUCTION_TE_RPL		= 0x58,	// 获取金矿时间事件
	CMDID_GET_OTHER_GOLDORE_FETCH_INFO_RPL		= 0x59,		// 获取金矿的摘取信息
	CMDID_GET_OTHER_CHRISTMAS_TREE_INFO_RPL		= 0x5A,		// 获取圣诞树信息
	CMDID_CHECK_DRAW_LOTTERY_RPL	= 0x5B,		// 检验抽奖条件
	CMDID_FETCH_LOTTERY_RPL			= 0x5C,		// 获取抽奖结果
	CMDID_GET_CHRISTMAS_TREE_RPL	= 0x5D,		// 获取许愿树信息
	CMDID_GET_OTHER_GOLDORE_INFO_RPL= 0x5E,		// 获取别人金矿信息
	CMDID_GET_GOLDORE_SMP_INFO_ALL_RPL			= 0x5F,		// 获取一群人的金矿是否有事件发生
	CMDID_GET_ALLIANCE_TRADE_INFO_RPL	= 0x60,		// 获取联盟跑商状态
	CMDID_ALLIANCE_TRADE_RPL		= 0x61,		// 联盟跑商
	CMDID_DEAL_ALLIANCE_TREADE_TE_RPL			= 0x62,		// 处理联盟跑商事件
	CMDID_GET_ENEMY_LIST_RPL		= 0x63,		// 获取仇人列表
	CMDID_DELETE_ENEMY_RPL			= 0x64,		// 删除仇人关系
	CMDID_ADD_ENEMY_RPL				= 0x65,		// 添加仇人
	CMDID_REFRESH_CHAR_ATB_RPL		= 0x66,		// 刷新角色属性从DB到内存
	CMDID_ADD_ALLIANCE_DEV_RPL		= 0x67,		// 添加联盟发展度(负数则减)
	CMDID_LOAD_RANK_LIST_RPL		= 0x68,		// 载入排行榜列表
	CMDID_RECHARGE_DIAMOND_RPL		= 0x69,		// 钻石充值
	CMDID_WRITE_CHAR_SIGNATURE_RPL	= 0x6A,		// 书写角色签名
	CMDID_GET_INSTANCE_ID_RPL		= 0x6B,		// 获取副本only id
	CMDID_CHANGE_NAME_RPL			= 0x6C,		// 修改角色名字
	CMDID_MOVE_POSITION_RPL			= 0x6D,		// 移动城市位置
	CMDID_GET_PAY_SERIAL_RPL		= 0x6E,		// 获取充值序列号
	CMDID_GET_PAY_LIST_RPL			= 0x6F,		// 获取充值请求列表
	CMDID_DEAL_ONE_PAY_RPL			= 0x70,		// 处理一单充值交易
	CMDID_READ_NOTIFICATION_RPL		= 0x71,		// 阅读通知
	CMDID_CHANGE_HERO_NAME_RPL		= 0x72,		// 修改英雄名字
	CMDID_READ_MAIL_ALL_RPL			= 0x73,		// 标记所有邮件为已读
	CMDID_DELETE_MAIL_ALL_RPL		= 0x74,		// 删除所有邮件
	CMDID_ADD_GM_MAIL_RPL			= 0x75,		// 添加GM邮件
	CMDID_GET_PLAYERCARD_RPL		= 0x76,		// 从数据库中载入一个玩家的PlayerCard
	CMDID_DEAL_EXCEL_TE_RPL			= 0x77,		// 处理excel time event
	CMDID_CHANGE_ALLIANCE_NAME_RPL	= 0x78,		// 修改联盟名字
	CMDID_GEN_WORLD_FAMOUS_CITY_RPL	= 0x79,		// 生成世界名城
	CMDID_GET_WORLD_FAMOUS_CITY_RPL	= 0x7A,		// 从数据库中载入名城信息
	CMDID_CUP_ACTIVITY_END_RPL		= 0x7B,		// 奖杯活动结算
	CMDID_FETCH_MAIL_REWARD_RPL		= 0x7C,		// 获取邮件附件奖励
	CMDID_UPLOAD_BILL_RPL			= 0x7D,		// 客户端上传账单数据
	CMDID_SET_BINDED_RPL			= 0x7E,		// 设置绑定账号
	CMDID_ADD_LOTTERY_LOG_RPL		= 0x7F,		// 记录抽奖日志

	CMDID_TAVERN_REFRESH_RPL		= 0x80,		// 酒馆刷新
	CMDID_HIRE_HERO_RPL				= 0x81,		// 招募武将
	CMDID_FIRE_HERO_RPL				= 0x82,		// 解雇武将
	CMDID_LEVELUP_HERO_RPL			= 0x83,		// 武将升级
	CMDID_ADD_GROW_RPL				= 0x84,		// 增加武将成长率
	CMDID_CONFIG_HERO_RPL			= 0x85,		// 武将配兵
	CMDID_START_COMBAT_RPL			= 0x86,		// 开始战斗
	CMDID_STOP_COMBAT_RPL			= 0x87,		// 结束战斗
	CMDID_GET_COMBAT_RPL			= 0x88,		// 获取战斗数据
	CMDID_GET_HERO_REFRESH_RPL		= 0x89,		// 获取刷新武将
	CMDID_GET_HERO_HIRE_RPL			= 0x8A,		// 获取雇佣武将
	CMDID_LIST_COMBAT_RPL			= 0x8B,		// 获取所有战斗摘要
	CMDID_CONFIG_CITYDEFENSE_RPL	= 0x8C,		// 配置城防武将

	CMDID_GET_ITEM_RPL				= 0x8E,		// 获取道具
	CMDID_ADD_ITEM_RPL				= 0x8F,		// 添加道具
	CMDID_DEL_ITEM_RPL				= 0x90,		// 删除道具
	CMDID_EQUIP_ITEM_RPL			= 0x91,		// 装备道具
	CMDID_DISEQUIP_ITEM_RPL			= 0x92,		// 卸下装备
	CMDID_MOUNT_ITEM_RPL			= 0x93,		// 镶嵌道具
	CMDID_UNMOUNT_ITEM_RPL			= 0x94,		// 卸下镶嵌
	CMDID_COMPOS_ITEM_RPL			= 0x95,		// 合成道具
	CMDID_DISCOMPOS_ITEM_RPL		= 0x96,		// 分解道具

	CMDID_GET_QUEST_RPL				= 0x97,		// 获取当前可接受的任务
	CMDID_DONE_QUEST_RPL			= 0x98,		// 完成任务

	CMDID_GET_INSTANCESTATUS_RPL	= 0xA0,		// 获取玩家副本当前状态
	CMDID_GET_INSTANCEDESC_RPL		= 0xA2,		// 获取副本实例摘要
	CMDID_CREATE_INSTANCE_RPL		= 0xA3,		// 创建副本
	CMDID_JOIN_INSTANCE_RPL			= 0xA4,		// 加入副本
	CMDID_QUIT_INSTANCE_RPL			= 0xA5,		// 退出副本
	CMDID_DESTROY_INSTANCE_RPL		= 0xA6,		// 销毁副本
	CMDID_GET_INSTANCEDATA_RPL		= 0xA7,		// 获取副本数据
	CMDID_CONFIG_INSTANCEHERO_RPL	= 0xA8,		// 配置副本武将
	CMDID_GET_INSTANCELOOT_RPL		= 0xA9,		// 获取副本掉落

	CMDID_EQUIP_ITEM_ALL_RPL		= 0xAA,		// 一键换装
	CMDID_MOUNT_ITEM_ALL_RPL		= 0xAB,		// 一键镶嵌

	CMDID_USE_ITEM_RPL				= 0xAE,		// 使用道具
	CMDID_AUTO_COMBAT_RPL			= 0xAF,		// 自动战斗开关

	CMDID_DEAL_COMBAT_TE_RPL		= 0xB0,		// 获取战斗超时事件
	CMDID_DEAL_COMBAT_BACK_RPL		= 0xB1,		// 进入战斗返回状态

	CMDID_SYNC_CHAR_RPL				= 0xB5,		// 同步数据，君主
	CMDID_SYNC_HERO_RPL				= 0xB6,		// 同步数据，武将

	CMDID_AUTO_SUPPLY_RPL			= 0xB7,		// 自动补给开关
	CMDID_GET_EQUIP_RPL				= 0xB8,		// 获取已装备的装备
	CMDID_GET_GEM_RPL				= 0xB9,		// 获取已镶嵌的宝石
	CMDID_MANUAL_SUPPLY_RPL			= 0xBA,		// 手动补给
	CMDID_REPLACE_EQUIP_RPL			= 0xBB,		// 替换装备

	CMDID_GET_WORLDGOLDMINE_RPL		= 0xC0,		// 获取世界金矿
	CMDID_CONFIG_WORLDGOLDMINE_HERO_RPL		= 0xC1,		// 配置金矿武将
	CMDID_ROB_WORLDGOLDMINE_RPL		= 0xC2,		// 夺取世界金矿
	CMDID_DROP_WORLDGOLDMINE_RPL	= 0xC3,		// 放弃世界金矿
	CMDID_DEAL_WORLDGOLDMINE_GAIN_TE_RPL	= 0xC4,		// 世界金矿收获超时事件
	CMDID_MY_WORLDGOLDMINE_RPL		= 0xC5,		// 我的世界金矿

	CMDID_GET_INSTANCELOOT_ADDITEM_RPL		= 0xCA,		// 增加道具，副本掉落
	CMDID_COMBAT_PROF_RPL			= 0xCB,		// 获取战力

	CMDID_LOAD_ARMY_RPL				= 0xCC,		// 加载兵种数据
	
	CMDID_NEWMAIL_NOTIFY			= 0xCD,		// 新邮件通知，combat_log完成后会发到XLP，然后再走邮件通知的流程

	CMDID_KICK_INSTANCE_RPL			= 0xD0,		// 踢人
	CMDID_START_INSTANCE_RPL		= 0xD1,		// 副本出征
	CMDID_PREPARE_INSTANCE_RPL		= 0xD2,		// 副本就绪
	CMDID_SUPPLY_INSTANCE_RPL		= 0xD3,		// 副本补给

	CMDID_GET_CITYDEFENSE_RPL		= 0xD4,		// 获取城防武将

	CMDID_USE_DRUG_RPL				= 0xD5,		// 使用草药

	CMDID_COMBAT_RST				= 0xD6,		// 战斗结果

	CMDID_FIXHOUR_RPL				= 0xD7,		// 定时事件返回

	CMDID_PVP_RST					= 0xD8,		// 玩家对战结果

	CMDID_ARMY_ACCELERATE_RPL		= 0xDA,		// 行军加速
	CMDID_ARMY_BACK_RPL				= 0xDB,		// 召回

	CMDID_SENDALLCLIENT				= 0xDC,		// 全服通告

	CMDID_WORLDCITY_GET_RPL			= 0xDD,		// 获得世界名城信息
	CMDID_WORLDCITY_GETLOG_RPL		= 0xDE,		// 获得世界名城奖杯战绩个人

	CMDID_WORLDCITY_RANK_MAN_RPL		= 0xE0,		// 获得奖杯个人排名
	CMDID_WORLDCITY_RANK_ALLIANCE_RPL	= 0xE1,		// 获得奖杯联盟排名

	CMDID_WORLDCITY_GETLOG_ALLIANCE_RPL		= 0xE2,		// 获得世界名城奖杯战绩联盟

	CMDID_ATB						= 0xE3,		// 通知LP刷新ATB


	CMDID_DEAL_CHAR_DAY_TE_RPL			= 0x1001,	// 处理时间事件
	CMDID_DEAL_ADCOLONY_TE_RPL			= 0x1002,	// 处理AdColony
	CMDID_DISMISS_SOLDIER_RPL			= 0x1003,	// 解散士兵
	CMDID_REFRESH_MAIL_RPL				= 0x1004,	// 刷新新邮件
	CMDID_BAN_CHAR_RPL					= 0x1005,	// 封角色
	CMDID_NEW_ADCOLONY_RPL				= 0x1006,	// 新的ADCOLONY

	CMDID_ALLI_DONATE_SOLDIER_RPL		= 0x1007,	// 联盟送兵
	CMDID_ALLI_RECALL_SOLDIER_RPL		= 0x1008,	// 召回联盟送兵
	CMDID_ALLI_ACCE_SOLDIER_RPL			= 0x1009,	// 加速联盟送兵
	CMDID_GET_DONATE_SOLDIER_QUEUE_RPL	= 0x100A,	// 获取联盟送兵队列
	CMDID_GET_LEFT_DONATE_SOLDIER_RPL	= 0x100B,	// 获取剩余可送兵
	CMDID_DEAL_DONATE_SOLDIER_TE_RPL	= 0x100C,	// 处理送兵事件

	CMDID_ADD_GOLD_DEAL_RPL				= 0x100D,	// 增加黄金交易
	CMDID_BUY_GOLD_IN_MARKET_RPL		= 0x100E,	// 市场买黄金
	CMDID_CANCEL_GOLD_DEAL_RPL			= 0x100F,	// 取消黄金交易
	CMDID_GET_GOLD_DEAL_RPL				= 0x1010,	// 获取交易列表
	CMDID_GET_SELF_GOLD_DEAL_RPL		= 0x1011,	// 获取自己的黄金交易
	CMDID_DEAL_GOLD_MARKET_TE_RPL		= 0x1012,	// 处理黄金交易超时
	CMDID_GEN_WORLD_RES_RPL				= 0x1013,	// 生成世界资源
	CMDID_GET_WORLD_RES_RPL				= 0x1014,	// 从数据库中读取世界资源
	CMDID_REFRESH_DRUG_RPL				= 0x1015,	// 刷新草药

	CMDID_ALLI_INSTANCE_CREATE_RPL			= 0x1100,	// 创建副本
	CMDID_ALLI_INSTANCE_JOIN_RPL			= 0x1101,	// 加入副本
	CMDID_ALLI_INSTANCE_EXIT_RPL			= 0x1102,	// 退出副本
	CMDID_ALLI_INSTANCE_KICK_RPL			= 0x1103,	// 踢出副本
	CMDID_ALLI_INSTANCE_DESTROY_RPL			= 0x1104,	// 销毁副本
	CMDID_ALLI_INSTANCE_CONFIG_HERO_RPL		= 0x1105,	// 配置将领
	CMDID_ALLI_INSTANCE_START_RPL			= 0x1106,	// 副本出征
	CMDID_ALLI_INSTANCE_READY_RPL			= 0x1107,	// 准备就绪
	CMDID_ALLI_INSTANCE_GET_LIST_RPL		= 0x1108,	// 获取副本列表
	CMDID_ALLI_INSTANCE_GET_CHAR_DATA_RPL	= 0x1109,	// 获取副本角色详细信息
	CMDID_ALLI_INSTANCE_GET_HERO_DATA_RPL	= 0x110A,	// 获取副本将领详细信息
	CMDID_ALLI_INSTANCE_GET_STATUS_RPL		= 0x110B,	// 自己的副本状态
	CMDID_ALLI_INSTANCE_GET_LOOT_RPL		= 0x110C,	// 获取掉落
	CMDID_ALLI_INSTANCE_SAVE_HERO_DEPLOY_RPL= 0x110D,	// 保存将领布局
	CMDID_ALLI_INSTANCE_START_COMBAT_RPL	= 0x110E,	// 开始战斗
	CMDID_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_RPL		= 0x110F,	// 处理战斗前往事件
	CMDID_ALLI_INSTANCE_DEAL_MARCH_BACK_RPL			= 0x1110,	// 处理战斗返回事件
	CMDID_ALLI_INSTANCE_DEAL_COMBAT_RESULT_RPL		= 0x1111,	// 处理战斗结果
	CMDID_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_RPL	= 0x1112,	// 获取战斗结果事件
	CMDID_ALLI_INSTANCE_GET_INSTANCE_ID_RPL			= 0x1113,	// 获取副本ID
	CMDID_ALLI_INSTANCE_GET_COMBAT_LOG_RPL			= 0x1114,	// 获取战斗简单日志
	CMDID_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_RPL	= 0x1115,	// 获取战斗详细日志
	CMDID_ALLI_INSTANCE_STOP_COMBAT_RPL		= 0x1116,	// 停止战斗
	CMDID_ALLI_INSTANCE_SUPPLY_RPL			= 0x1117,	// 补给
	CMDID_ALLI_INSTANCE_SYSTEM_DESTROY_RPL	= 0x1118,	// 系统销毁
	CMDID_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_RPL		= 0x1119,	// 获取防守方战斗布局

	CMDID_KICK_CLIENT_ALL_RPL				= 0x1200,	// 踢出所有客户端
	CMDID_GET_KICK_CLIENT_ALL_RPL			= 0x1201,	// 获取踢出所有客户端

	CMDID_GET_HERO_SIMPLE_DATA_RPL			= 0x1202,	// 获取将领简单信息
	CMDID_GET_INSTANCE_SIMPLE_DATA_RPL		= 0x1203,	// 获取简单副本信息

	CMDID_ARENA_UPLOAD_DATA_RPL				= 0x1300,	// 上传竞技场数据
	CMDID_ARENA_GET_RANK_LIST_RPL			= 0x1301,	// 竞技场排行榜
	CMDID_ARENA_GET_STATUS_RPL				= 0x1302,	// 获取状态,挑战列表
	CMDID_ARENA_PAY_RPL						= 0x1303,	// 竞技场付费
	CMDID_ARENA_DEAL_REWARD_EVENT_RPL		= 0x1304,	// 竞技场奖励发送
	CMDID_ARENA_GET_DEPLOY_RPL				= 0x1305,	// 获取竞技场英雄布局

	CMDID_HERO_SUPPLY_RPL					= 0x1400,	// 英雄补给
	CMDID_SET_VIP_DISPLAY_RPL				= 0x1401,	// 设置vip显示

	CMDID_POSITION_MARK_ADD_RECORD_RPL		= 0x1500,	// 添加位置收藏
	CMDID_POSITION_MARK_CHG_RECORD_RPL		= 0x1501,	// 修改位置收藏
	CMDID_POSITION_MARK_DEL_RECORD_RPL		= 0x1502,	// 删除位置收藏
	CMDID_POSITION_MARK_GET_RECORD_RPL		= 0x1503,	// 获取位置收藏
};
#pragma pack(1)
struct P_DBS4WEB_CMD_T 
{
	unsigned char	nCmd;
	unsigned int	nSubCmd;
};
//////////////////////////////////////////////////////////////////////////
// DB4WEB
//////////////////////////////////////////////////////////////////////////
struct P_DBS4WEB_ACCOUNT_ONLINE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nIP;
	unsigned char	nBinded;
};
struct P_DBS4WEB_LOAD_CHAR_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_ACCOUNT_OFFLINE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_CHAR_CREATE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	char			szCharName[TTY_CHARACTERNAME_LEN];
	int				nSex;							// 性别
	int				nHeadID;						// 头像
	int				nPosX;							// 世界X坐标
	int				nPosY;							// 世界Y坐标
	unsigned char	nBinded;						
	unsigned int	nGiftAppID;
};
struct P_DBS4WEB_LOAD_PLAYERCARD_T : public P_DBS4WEB_CMD_T 
{

};
struct P_DBS4WEB_LOAD_TERRAIN_T : public P_DBS4WEB_CMD_T 
{

};
struct P_DBS4WEB_GET_DB_TIMEEVENT_LIST_T : public P_DBS4WEB_CMD_T 
{
};
struct P_DBS4WEB_DEAL_BUILDING_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_GET_BUILDING_LIST_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_BUILDING_TE_LIST_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_BUILD_BUILDING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nAutoID;
};
struct P_DBS4WEB_UPGRADE_BUILDING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nAutoID;
};
struct P_DBS4WEB_DEAL_ONCE_PERPERSON_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_ADD_ONCE_PERPERSON_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
	unsigned int	nType;
};
struct P_DBS4WEB_DEAL_PRODUCTION_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_ADD_PRODUCTION_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nAutoID;
	unsigned int	nType;
	unsigned int	nProductionChoice;
};
struct P_DBS4WEB_FETCH_PRODUCTION_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nAutoID;
	unsigned int	nType;
};
struct P_DBS4WEB_GET_PRODUCTION_EVENT_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_PRODUCTION_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_ACCELERATE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nType;
	unsigned int	nSubType;
	unsigned int	nAutoID;
	unsigned int	nExcelID;
	unsigned int	nMoneyType;
	unsigned int	nTime;
};
struct P_DBS4WEB_DEAL_RESEARCH_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_ADD_RESEARCH_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nExcelID;
};
struct P_DBS4WEB_GET_TECHNOLOGY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_RESEARCH_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_CONSCRIPT_SOLDIER_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nNum;
};
struct P_DBS4WEB_UPGRADE_SOLDIER_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nFromLevel;
	unsigned int	nToLevel;
	unsigned int	nNum;
};
struct P_DBS4WEB_GET_SOLDIER_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_ACCEPT_ALLIANCE_MEMBER_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nManagerID;
	tty_id_t		nAllianceID;
	tty_id_t		nNewMemberID;
};
struct P_DBS4WEB_BUILD_ALLIANCE_BUILDING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
};
struct P_DBS4WEB_CANCEL_JOIN_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_CREATE_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
};
struct P_DBS4WEB_DEAL_ALLIANCE_BUILDING_TE_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_DISMISS_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_EXIT_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_EXPEL_ALLIANCE_MEMBER_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nManagerID;
	tty_id_t		nAllianceID;
	tty_id_t		nMemberID;
};
struct P_DBS4WEB_JOIN_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nAllianceID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nHeadID;
};
struct P_DBS4WEB_SET_ALLIANCE_POSITION_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nManagerID;
	tty_id_t		nAllianceID;
	tty_id_t		nMemberID;
	unsigned int	nPosition;
};
struct P_DBS4WEB_GET_ALLIANCE_INFO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_GET_ALLIANCE_MEMBER_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_GET_ALLIANCE_BUILDING_TE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_GET_ALLIANCE_BUILDING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_GET_ALLIANCE_JOIN_EVENT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_LOAD_ALLIANCE_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_LOAD_ALL_ALLIANCE_ID_T : public P_DBS4WEB_CMD_T 
{
};
// 邮件相关
struct P_DBS4WEB_ADD_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	bool			bNeedRst;
	tty_id_t		nSenderID;
	char			szSender[TTY_CHARACTERNAME_LEN];
	tty_id_t		nReceiverID;
	unsigned char	nType;
	unsigned char	nFlag;
	bool			bSendLimit;		// 是否需要限制发送和接收
	int				nTextLen;
	char			szText[1];
	// int			nExtDataLen;
	// char			szExtData[1];
};
struct P_DBS4WEB_GET_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_READ_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nMailID;
};
struct P_DBS4WEB_DEL_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nMailID;
};
struct P_DBS4WEB_GET_PRIVATE_MAIL_1_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
	unsigned int	nMailID;
};
struct P_DBS4WEB_REFUSE_JOIN_ALLIANCE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nManagerID;
	tty_id_t		nApplicantID;
};
struct P_DBS4WEB_ABDICATE_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nLeaderID;
	char			szMemberName[TTY_CHARACTERNAME_LEN];
};
// 好友
struct P_DBS4WEB_GET_MY_FRIEND_APPLY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_OTHERS_FRIEND_APPLY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_FRIEND_LIST_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_APPROVE_FRIEND_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEB_APPLY_FRIEND_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEB_REFUSE_FRIEND_APPLY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEB_CANCEL_FRIEND_APPLY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEB_DELETE_FRIEND_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEB_LOAD_EXCEL_TEXT_T : public P_DBS4WEB_CMD_T 
{
};
struct P_DBS4WEB_LOAD_EXCEL_ALLIANCE_CONGRESS_T : public P_DBS4WEB_CMD_T 
{
};
struct P_DBS4WEB_ADD_ALLIANCE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	char			szSender[TTY_CHARACTERNAME_LEN];
	unsigned int	nTextLen;
	char			szText[1];
};
struct P_DBS4WEB_ADD_ALLIANCE_LOG_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nAllianceID;
	unsigned int	nTextLen;
	char			szText[1];
};
struct P_DBS4WEB_ADD_PRIVATE_LOG_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
	unsigned int	nTextLen;
	char			szText[1];
};
struct P_DBS4WEB_GET_ALLIANCE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_GET_ALLIANCE_LOG_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_GET_PRIVATE_LOG_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_BUY_ITEM_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nNum;
	unsigned int	nMoneyType;
};
struct P_DBS4WEB_SELL_ITEM_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nItemID;
	unsigned int	nNum;
};
struct P_DBS4WEB_GET_RELATION_LOG_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_FETCH_CHRISTMAS_TREE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_WATERING_CHRISTMAS_TREE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nWateringAccountID;
};
struct P_DBS4WEB_STEAL_GOLD_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nThiefAccountID;
	unsigned int	nAutoID;
};
struct P_DBS4WEB_ADD_RELATION_LOG_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
	unsigned int	nType;
	unsigned int	nTextLen;
	char			szText[1];
};
struct P_DBS4WEB_GET_TRAINING_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_ADD_TRAINING_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nTimeUnitNum;
	int				nNum;
};
struct P_DBS4WEB_EXIT_TRAINING_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
};
struct P_DBS4WEB_DEAL_TRAINING_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};

struct P_DBS4WEB_TAVERN_REFRESH_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nUseItem;
};
struct P_DBS4WEB_HIRE_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nSlotID;
	tty_id_t		nHeroID;
};
struct P_DBS4WEB_FIRE_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
};
struct P_DBS4WEB_LEVELUP_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
};
struct P_DBS4WEB_ADD_GROW_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	float			fGrow;
};
struct P_DBS4WEB_CONFIG_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	int				nArmyType;
	int				nArmyLevel;
	int				nArmyNum;
};
struct P_DBS4WEB_GET_HERO_REFRESH_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_HERO_HIRE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_START_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nObjID;
	int				nCombatType;
	int				nAutoCombat;
	int				nAutoSupply;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
	int				nStopLevel;
};
struct P_DBS4WEB_STOP_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nCombatID;
	int				nCombatType;
};
struct P_DBS4WEB_GET_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nCombatID;
};
struct P_DBS4WEB_UPDATE_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nHeroID;
	int				nArmyNum;
};
struct P_DBS4WEB_LIST_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_CONFIG_CITYDEFENSE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
	int				nAutoSupply;
};
struct P_DBS4WEB_GET_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_ADD_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	int				nNum;
};
struct P_DBS4WEB_DEL_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nItemID;
	int				nNum;
};
struct P_DBS4WEB_EQUIP_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	tty_id_t		nItemID;
};
struct P_DBS4WEB_DISEQUIP_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	tty_id_t		nItemID;
};
struct P_DBS4WEB_MOUNT_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	tty_id_t		nEquipID;
	int				nSlotIdx;
	int				nExcelID;
};
struct P_DBS4WEB_UNMOUNT_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	tty_id_t		nEquipID;
	int				nSlotIdx;
};
struct P_DBS4WEB_COMPOS_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	int				nNum;
};
struct P_DBS4WEB_DISCOMPOS_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	int				nNum;
};
struct P_DBS4WEB_EQUIP_ITEM_ALL_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	tty_id_t		n1HeadID;
	tty_id_t		n2ChestID;
	tty_id_t		n3ShoeID;
	tty_id_t		n4WeaponID;
	tty_id_t		n5TrinketID;
};
struct P_DBS4WEB_MOUNT_ITEM_ALL_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	tty_id_t		nEquipID;
	int				n1ExcelID;
	int				n2ExcelID;
	int				n3ExcelID;
	int				n4ExcelID;
};

struct P_DBS4WEB_GET_QUEST_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_DONE_QUEST_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
};
struct P_DBS4WEB_GET_INSTANCESTATUS_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_INSTANCEDESC_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	int				nClassID;
};
struct P_DBS4WEB_CREATE_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	int				nClassID;
};
struct P_DBS4WEB_JOIN_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_QUIT_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_DESTROY_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_GET_INSTANCEDATA_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_DEAL_COMBAT_TE_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_CONFIG_INSTANCEHERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
};
struct P_DBS4WEB_GET_INSTANCELOOT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_AUTO_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	int				nVal;
};
struct P_DBS4WEB_DEAL_COMBAT_BACK_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nCombatID;
	int				nCombatRst;
	int				nCombatType;
	tty_id_t		nAccountID;		// 如果是普通战斗，攻方账号ID
	tty_id_t		nObjID;			// 如果是玩家对战，守方账号ID
	tty_id_t		nAccountID1;	// 如果是副本战斗，关联的5个账号ID
	tty_id_t		nAccountID2;	// 
	tty_id_t		nAccountID3;	// 
	tty_id_t		nAccountID4;	// 
	tty_id_t		nAccountID5;	// 
	int				nNumA;
	int				nNumD;
	// 后面跟 CombatUnit4Log
};
struct P_DBS4WEB_USE_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	int				nExcelID;
	int				nNum;
};
struct P_DBS4WEB_SYNC_CHAR_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_SYNC_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
};
struct P_DBS4WEB_AUTO_SUPPLY_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	int				nVal;
};
struct P_DBS4WEB_GET_EQUIP_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_GEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_MANUAL_SUPPLY_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	int				nCombatType;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
};
struct P_DBS4WEB_REPLACE_EQUIP_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	int				nEquipType;
	tty_id_t		nItemID;
};
struct P_DBS4WEB_GET_WORLDGOLDMINE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nArea;
	int				nClass;
	int				nIdx;
};
struct P_DBS4WEB_CONFIG_WORLDGOLDMINE_HERO_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
};
struct P_DBS4WEB_ROB_WORLDGOLDMINE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nArea;
	int				nClass;
	int				nIdx;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
};
struct P_DBS4WEB_DROP_WORLDGOLDMINE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_DEAL_WORLDGOLDMINE_GAIN_TE_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_MY_WORLDGOLDMINE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_DEAL_FIXHOUR_TE_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_COMBAT_PROF_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nClassID;
	tty_id_t		nObjID;
	int				nCombatType;
};
struct P_DBS4WEB_LOAD_ARMY_T : public P_DBS4WEB_CMD_T
{
};
struct P_DBS4WEB_KICK_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
	tty_id_t		nObjID;
};
struct P_DBS4WEB_START_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_PREPARE_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
	int				nVal;
};
struct P_DBS4WEB_SUPPLY_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_GET_CITYDEFENSE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_USE_DRUG_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nNum;
};
struct P_DBS4WEB_ARMY_ACCELERATE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nCombatID;
	int				nTime;
	unsigned char	byType;
};
struct P_DBS4WEB_ARMY_BACK_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nCombatID;
};
struct P_DBS4WEB_DEAL_WORLDCITYRECOVER_TE_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nEventID;
};
struct P_DBS4WEB_WORLDCITY_GET_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
};
struct P_DBS4WEB_WORLDCITY_GETLOG_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	int				nPageNum;
	int				nPageIdx;
};
struct P_DBS4WEB_WORLDCITY_GETLOG_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	int				nPageNum;
	int				nPageIdx;
};
struct P_DBS4WEB_WORLDCITY_RANK_MAN_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	int				nPageNum;
	int				nPageIdx;
};
struct P_DBS4WEB_WORLDCITY_RANK_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	int				nPageNum;
	int				nPageIdx;
};
struct P_DBS4WEB_RECHARGE_TRY_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
	int				nDiamond;
};

//////////////////////////////////////////////////////////////////////////
// DB4WEBUSER
//////////////////////////////////////////////////////////////////////////
struct P_DBS4WEBUSER_ACCOUNT_ONLINE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_LOAD_CHAR_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK					= 0,
		RST_NOCHAR				= 1,
		RST_DBTIMEOUT			= 2,
		RST_DBERR				= 3,
		RST_SQLERR				= 4,
	};
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_ACCOUNT_OFFLINE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_CHAR_CREATE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_LOAD_PLAYERCARD_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK					= 0,
		RST_DBERR				= 1,
		RST_SQLERR				= 2,
	};
	tty_rst_t		nRst;
	int				nNum;
	// 后面跟着nNum个PlayerCard
};
struct P_DBS4WEBUSER_LOAD_TERRAIN_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK					= 0,
		RST_DBERR				= 1,
		RST_SQLERR				= 2,
	};
	tty_rst_t		nRst;
	int				nNum;
	// 后面跟着nNum个TerrainUnit
};
struct P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T : public P_DBS4WEB_CMD_T
{
	int				nNum;
	// 后面跟着nNum个
	struct TE_UNIT_T 
	{
		tty_id_t		nEventID;
		unsigned int	nType;
	};
};
struct P_DBS4WEBUSER_DEAL_BUILDING_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nAutoID;
	unsigned int	nExcelID;
	unsigned int	nType;
};
struct P_DBS4WEBUSER_GET_BUILDING_LIST_T : public P_DBS4WEB_CMD_T 
{
	int		nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	int		nNum;
	// nNum个BuildingUnit
};
struct P_DBS4WEBUSER_GET_BUILDING_TE_LIST_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nTimeNow;
	int				nNum;
	// nNum个BuildingTEUnit
};
struct P_DBS4WEBUSER_BUILD_BUILDING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nExcelID;
	unsigned int	nAutoID;
	unsigned int	nAddedCharExp;
	unsigned int	nAddedCharDev;
	unsigned int	nCurCharExp;
	unsigned int	nCurCharLevel;
	unsigned int	nGoldCost;
	unsigned int	nPopulationCost;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_UPGRADE_BUILDING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nAutoID;
	unsigned int	nExcelID;
	unsigned int	nAddedCharExp;
	unsigned int	nAddedCharDev;
	unsigned int	nCurCharExp;
	unsigned int	nCurCharLevel;
	unsigned int	nGoldCost;
	unsigned int	nPopulationCost;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_DEAL_ONCE_PERPERSON_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nResult;
	tty_id_t		nAccountID;
	int				nType;
	int				nProduction;
};
struct P_DBS4WEBUSER_DEAL_PRODUCTION_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_rst_t		nResult;
	tty_id_t		nAccountID;
	unsigned int	nAutoID;
	unsigned int	nProduction;
	unsigned int	nType;
};
struct P_DBS4WEBUSER_ADD_PRODUCTION_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nResult;
	unsigned int	nAutoID;
	unsigned int	nType;
	unsigned int	nProduction;
};
struct P_DBS4WEBUSER_FETCH_PRODUCTION_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nResult;
	tty_id_t		nAccountID;
	unsigned int	nAutoID;
	unsigned int	nType;
	unsigned int	nProduction;
};
struct P_DBS4WEBUSER_GET_PRODUCTION_EVENT_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nResult;
	tty_id_t		nAccountID;
	int				nNum;
};
struct P_DBS4WEBUSER_GET_PRODUCTION_TE_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nResult;
	tty_id_t		nAccountID;
	unsigned int	nTimeNow;
	int				nNum;
};
struct P_DBS4WEBUSER_ACCELERATE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nResult;
	tty_id_t		nAccountID;
	unsigned int	nTime;
	unsigned int	nMoneyNum;
	unsigned int	nMoneyType;
	unsigned int	nType;
	unsigned int	nSubType;
	unsigned int	nAutoID;
	unsigned int	nExcelID;
	unsigned int	nAcceTime;
};
struct P_DBS4WEBUSER_DEAL_RESEARCH_TE_T : public P_DBS4WEB_CMD_T 
{
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
};
struct P_DBS4WEBUSER_ADD_RESEARCH_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nResult;
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nTime;
	unsigned int	nGold;
};
struct P_DBS4WEBUSER_GET_TECHNOLOGY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	// nNum个TechnologyUnit
};
struct P_DBS4WEBUSER_GET_RESEARCH_TE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nTimeNow;
	int				nNum;
	// nNum个ResearchTimeEvent
};
struct P_DBS4WEBUSER_CONSCRIPT_SOLDIER_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nNum;
	unsigned int	nGold;
};
struct P_DBS4WEBUSER_UPGRADE_SOLDIER_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nFromLevel;
	unsigned int	nToLevel;
	unsigned int	nNum;
	unsigned int	nGold;
};
struct P_DBS4WEBUSER_GET_SOLDIER_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
	// 后面跟nNum个SoldierUnit
};
// alliance
struct P_DBS4WEBUSER_ACCEPT_ALLIANCE_MEMBER_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nNewMemberID;
	tty_id_t		nAllianceID;
	tty_id_t		nAccountID;
	unsigned int	nDevelopment;
};
struct P_DBS4WEBUSER_BUILD_ALLIANCE_BUILDING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nExcelID;
	unsigned int	nDevelopment;
	unsigned int	nTime;
	unsigned int	nType;
	tty_id_t		nAllianceID;
};
struct P_DBS4WEBUSER_CANCEL_JOIN_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAllianceID;
	tty_id_t		nAccountID;
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_CREATE_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nGold;
	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
};
struct P_DBS4WEBUSER_DEAL_ALLIANCE_BUILDING_TE_T : public P_DBS4WEB_CMD_T
{
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	unsigned int	nExcelID;
	unsigned int	nType;
};
struct P_DBS4WEBUSER_DISMISS_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
};
struct P_DBS4WEBUSER_EXIT_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	tty_id_t		nAccountID;
	unsigned int	nDevelopment;
};
struct P_DBS4WEBUSER_EXPEL_ALLIANCE_MEMBER_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nManagerID;
	tty_id_t		nAllianceID;
	tty_id_t		nMemberID;
	unsigned int	nDevelopment;
};
struct P_DBS4WEBUSER_JOIN_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nAllianceID;
	char			szName[TTY_CHARACTERNAME_LEN];
	unsigned int	nLevel;
	unsigned int	nHeadID;
};
struct P_DBS4WEBUSER_SET_ALLIANCE_POSITION_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nManagerID;
	tty_id_t		nAllianceID;
	tty_id_t		nMemberID;
	unsigned int	nPosition;
};
struct P_DBS4WEBUSER_GET_ALLIANCE_INFO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
};
struct P_DBS4WEBUSER_GET_ALLIANCE_MEMBER_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	unsigned int	nNum;
	// 后面跟nNum个AllianceMemberUnit
};
struct P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_TE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	unsigned int	nTimeNow;
	unsigned int	nNum;
	// 后面跟nNum个AllianceBuildingTimeEvent
};
struct P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	unsigned int	nNum;
	// 后面跟nNum个AllianceBuildingUnit
};
struct P_DBS4WEBUSER_GET_ALLIANCE_JOIN_EVENT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	unsigned int	nNum;
	// 后面跟nNum个AllianceJoinEvent
};
// 邮件相关
struct P_DBS4WEBUSER_ADD_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	bool			bNeedRst;
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_GET_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_READ_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nMailID;
};
struct P_DBS4WEBUSER_DEL_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nMailID;
};
struct P_DBS4WEBUSER_NEW_PRIVATE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
	// PrivateMailUnit
};
struct P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK					= 0,
		RST_NOT_EXIST			= -1,	// 不存在

		// 其他返回值
		RST_DB_ERR				= -200,
		RST_SQL_ERR				= -201,
	};
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	// 后面跟一个PrivateMailUnit
};
struct P_DBS4WEBUSER_REFUSE_JOIN_ALLIANCE_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	tty_id_t		nManagerID;
	tty_id_t		nApplicantID;
};
struct P_DBS4WEBUSER_ABDICATE_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	tty_id_t		nMemberID;
};
// 好友
struct P_DBS4WEBUSER_GET_MY_FRIEND_APPLY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_GET_OTHERS_FRIEND_APPLY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_GET_FRIEND_LIST_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_APPROVE_FRIEND_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEBUSER_APPLY_FRIEND_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEBUSER_REFUSE_FRIEND_APPLY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEBUSER_CANCEL_FRIEND_APPLY_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};
struct P_DBS4WEBUSER_DELETE_FRIEND_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nPeerAccountID;
};

struct P_DBS4WEBUSER_TAVERN_REFRESH_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nCD;
};
struct P_DBS4WEBUSER_HIRE_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nHeroID;
	int				nSlotID;
};
struct P_DBS4WEBUSER_FIRE_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_LEVELUP_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_ADD_GROW_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_CONFIG_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nProf;
};
struct P_DBS4WEBUSER_GET_HERO_REFRESH_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_GET_HERO_HIRE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_START_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nCombatID;
	int				nGoTime;
	int				nBackTime;
	int				nCombatType;
	tty_id_t		nAccountID1;
	tty_id_t		nAccountID2;
	tty_id_t		nAccountID3;
	tty_id_t		nAccountID4;
	tty_id_t		nAccountID5;
	tty_id_t		nObjID;
	int				nAutoCombat;
	tty_id_t		nAllianceID;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_STOP_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nCombatRst;
	int				nCombatType;
	tty_id_t		nCombatID;
	tty_id_t		nNewCombatID;
	int				nGoTime;
	int				nBackTime;
	tty_id_t		nAccountID;		// 如果是普通战斗，攻方账号ID
	tty_id_t		nAccountID1;	// 如果是副本战斗，关联的5个账号ID
	tty_id_t		nAccountID2;	// 
	tty_id_t		nAccountID3;	// 
	tty_id_t		nAccountID4;	// 
	tty_id_t		nAccountID5;	// 
	tty_id_t		nHero1;
	tty_id_t		nHero2;
	tty_id_t		nHero3;
	tty_id_t		nHero4;
	tty_id_t		nHero5;
	int				nDrug1;
	int				nDrug2;
	int				nDrug3;
	int				nDrug4;
	int				nDrug5;
	int				nNum;
	// 是副本战斗并且开启自动补给，后面跟 InstanceHeroDesc
};
struct P_DBS4WEBUSER_GET_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nCombatID;
	int				nCombatType;
	tty_id_t		nAccountID;		// 如果是普通战斗，攻方账号ID
	tty_id_t		nObjID;			// 如果是玩家对战，守方账号ID
	char			szObjName[32];
	int				nObjHeadID;
	tty_id_t		nAccountID1;	// 如果是副本战斗，关联的5个账号ID
	tty_id_t		nAccountID2;	// 
	tty_id_t		nAccountID3;	// 
	tty_id_t		nAccountID4;	// 
	tty_id_t		nAccountID5;	// 
	int				nNumA;			// 攻方部队数量
	int				nNumD;			// 守方部队数量
};
struct P_DBS4WEBUSER_LIST_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_CONFIG_CITYDEFENSE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_GET_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_ADD_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_DEL_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_EQUIP_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_DISEQUIP_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_MOUNT_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_UNMOUNT_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_COMPOS_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nItemID;
};
struct P_DBS4WEBUSER_DISCOMPOS_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_EQUIP_ITEM_ALL_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_MOUNT_ITEM_ALL_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};

struct P_DBS4WEBUSER_GET_QUEST_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_DONE_QUEST_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_GET_INSTANCESTATUS_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_GET_INSTANCEDESC_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nClassID;
	int				nNum;
};
struct P_DBS4WEBUSER_CREATE_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nInstanceID;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_JOIN_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nExcelID;
	tty_id_t		nInstanceID;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_QUIT_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nExcelID;
	tty_id_t		nInstanceID;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_DESTROY_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nInstanceID;
	tty_id_t		nID1;
	tty_id_t		nID2;
	tty_id_t		nID3;
	tty_id_t		nID4;
	tty_id_t		nID5;
};
struct P_DBS4WEBUSER_GET_INSTANCEDATA_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nCurLevel;
	int				nAutoCombat;
	int				nAutoSupply;
	int				nRetryTimes;
	tty_id_t		nInstanceID;
	int				nNum;
	int				nNumHero;
};
struct P_DBS4WEBUSER_GET_INSTANCELOOT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_DEAL_COMBAT_TE_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nCombatID;
	int				nCombatType;
	int				nStatus;
};
struct P_DBS4WEBUSER_CONFIG_INSTANCEHERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_AUTO_COMBAT_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_USE_ITEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nType;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_SYNC_CHAR_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_SYNC_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_AUTO_SUPPLY_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_GET_EQUIP_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_GET_GEM_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
};
struct P_DBS4WEBUSER_MANUAL_SUPPLY_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	int				nExcelID;
	tty_id_t		nInstanceID;
	tty_id_t		nAccountID1;
	tty_id_t		nAccountID2;
	tty_id_t		nAccountID3;
	tty_id_t		nAccountID4;
	tty_id_t		nAccountID5;
	int				nDrug1;
	int				nDrug2;
	int				nDrug3;
	int				nDrug4;
	int				nDrug5;
	tty_id_t		nAccountID;
	// InstanceHeroDesc
};
struct P_DBS4WEBUSER_REPLACE_EQUIP_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_GET_WORLDGOLDMINE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	int				nArea;
	// WorldGoldMineDesc
};
struct P_DBS4WEBUSER_CONFIG_WORLDGOLDMINE_HERO_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_ROB_WORLDGOLDMINE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nCombatID;
};
struct P_DBS4WEBUSER_DROP_WORLDGOLDMINE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEBUSER_DEAL_WORLDGOLDMINE_GAIN_TE_T : public P_DBS4WEB_CMD_T
{
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	int				nArea;
	int				nClass;
	int				nIdx;
	int				nGold;
};
struct P_DBS4WEBUSER_MY_WORLDGOLDMINE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nArea;
	int				nClass;
	int				nIdx;
	int				nSafeTime;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
};
struct P_DBS4WEBUSER_GET_INSTANCELOOT_ADDITEM_RPL_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	int				nCombatType;
	int				nObjID;
	int				nNum;
	int				nClassID;
	int				nCup;
	// 后面跟 InstanceLootDesc
};
struct P_DBS4WEBUSER_COMBAT_PROF_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	// 后面跟 CombatProf
};
struct P_DBS4WEBUSER_LOAD_ARMY_T : public P_DBS4WEB_CMD_T
{
	tty_rst_t		nRst;
	// 后面跟 ArmyDesc
};
struct P_DBS4WEBUSER_NEWMAIL_NOTIFY_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
	int				nMailID;
};
struct P_DBS4WEBUSER_KICK_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nExcelID;
	tty_id_t		nInstanceID;
	tty_id_t		nObjID;
};
struct P_DBS4WEBUSER_START_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
};
struct P_DBS4WEBUSER_PREPARE_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nStatus;
	int				nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_SUPPLY_INSTANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nExcelID;
	tty_id_t		nInstanceID;
	tty_id_t		nAccountID1;
	tty_id_t		nAccountID2;
	tty_id_t		nAccountID3;
	tty_id_t		nAccountID4;
	tty_id_t		nAccountID5;
	int				nDrug1;
	int				nDrug2;
	int				nDrug3;
	int				nDrug4;
	int				nDrug5;
};
struct P_DBS4WEBUSER_GET_CITYDEFENSE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		n1Hero;
	tty_id_t		n2Hero;
	tty_id_t		n3Hero;
	tty_id_t		n4Hero;
	tty_id_t		n5Hero;
	int				nAutoSupply;
};
struct P_DBS4WEBUSER_USE_DRUG_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	int				nNum;
};
struct P_DBS4WEBUSER_COMBAT_RST_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nCombatID;
	int				nCombatRst;
	int				nCombatType;
	tty_id_t		nAccountID;		// 如果是普通战斗，攻方账号ID
	tty_id_t		nObjID;			// 如果是玩家对战，守方账号ID
	int				nClassID;
	char			szObjName[32];
	int				nObjHeadID;
	tty_id_t		nAccountID1;	// 如果是副本战斗，关联的5个账号ID
	tty_id_t		nAccountID2;	// 
	tty_id_t		nAccountID3;	// 
	tty_id_t		nAccountID4;	// 
	tty_id_t		nAccountID5;	// 
	int				nNumA;
	int				nNumD;
	tty_id_t		nWorldCityID;
	int				nWorldCityRes;
	// 后面跟 CombatRst
};
struct P_DBS4WEBUSER_FIXHOUR_RST_T : public P_DBS4WEB_CMD_T
{
	tty_rst_t		nRst;
	int				nType;			// 定时事件类型
	union
	{
		int			nNum;			// 账号ID数量
	} u;
	// 后面跟 nAccountID
};
struct P_DBS4WEBUSER_PVP_RST_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nCombatID;		// 战斗ID
	int				nCombatRst;		// 战斗结果
	int				nGold;			// 金币
	int				nMan;			// 人口
	int				nCup;			// 奖杯
	tty_id_t		nAccountID;		// 
	tty_id_t		nObjID;			// 
};
struct P_DBS4WEBUSER_ARMY_ACCELERATE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nCombatID;		// 战斗ID
	int				nCostCrystal;	// 
	int				nFinTime;		// 
	tty_id_t		nAccountID;		// 
	unsigned char	byType;
};
struct P_DBS4WEBUSER_ARMY_BACK_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nCombatID;		// 战斗ID
	int				nBackTime;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_SENDALLCLIENT_T : public P_DBS4WEB_CMD_T
{
	int				nType;
	tty_id_t		nAllianceID;
	tty_id_t		nAccountID;
	int				nTextLen;
};
struct P_DBS4WEBUSER_WORLDCITY_GET_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	int				nRefreshTime;
	int				nBonusTime;
};
struct P_DBS4WEBUSER_WORLDCITY_GETLOG_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	int				nMaxPage;
};
struct P_DBS4WEBUSER_WORLDCITY_GETLOG_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	int				nMaxPage;
};
struct P_DBS4WEBUSER_WORLDCITY_RANK_MAN_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	int				nMaxPage;
};
struct P_DBS4WEBUSER_WORLDCITY_RANK_ALLIANCE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	int				nMaxPage;
};
struct P_DBS4WEBUSER_ATB_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
};

struct P_DBS4WEBUSER_GET_TRAINING_TE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nNum;
	// 后面跟nNum个
};
struct P_DBS4WEBUSER_ADD_TRAINING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nGold;
};
struct P_DBS4WEBUSER_EXIT_TRAINING_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	HireHero		hero;
};
struct P_DBS4WEBUSER_DEAL_TRAINING_TE_T : public P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
	unsigned int	nExp;
	HireHero		hero;
};

struct P_DBS4WEBUSER_LOAD_ALLIANCE_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK						= 0,
		RST_NOT_EXIST				= -1,

		// 其他错误
		RST_DB_ERR					= -200,
		RST_SQL_ERR					= -201,
	};
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
};
struct P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};
	tty_rst_t		nRst;
	int				nNum;
};

struct P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};

	tty_rst_t		nRst;
	size_t			nSize;
	int				nNum;
	// 后面跟nNum个ExcelText
};
struct P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};

	tty_rst_t		nRst;
	int				nNum;
	// 后面跟nNum个ExcelAllianceCongress
};
struct P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	// 后面跟个AllianceMailUnit
};
struct P_DBS4WEBUSER_ADD_ALLIANCE_LOG_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	unsigned int	nLogID;
};
struct P_DBS4WEBUSER_ADD_PRIVATE_LOG_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};
	tty_rst_t		nRst;
	unsigned int	nLogID;
};
struct P_DBS4WEBUSER_ADD_RELATION_LOG_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};
	tty_rst_t		nRst;
	unsigned int	nLogID;
};
struct P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_GET_ALLIANCE_LOG_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_GET_PRIVATE_LOG_T : public P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK			= 0,

		// 其他错误
		RST_DB_ERR		= -200,
		RST_SQL_ERR		= -201,
	};
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_BUY_ITEM_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nNum;
	unsigned int	nMoneyNum;
	unsigned int	nMoneyType;
	unsigned int	nItemIDNum;
	// 后面跟nItemIDNum个item_id(tty_id_t)
};
struct P_DBS4WEBUSER_SELL_ITEM_T : public P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nItemID;
	unsigned int	nExcelID;
	unsigned int	nNum;
	unsigned int	nGold;
	unsigned int	nCup;
};
struct P_DBS4WEBUSER_GET_RELATION_LOG_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_FETCH_CHRISTMAS_TREE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nCrystal;
	unsigned int	nExcelID;
	tty_id_t		nItemID;
};
struct P_DBS4WEBUSER_WATERING_CHRISTMAS_TREE_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nWateringAccountID;
	bool			bReadyRipe;
	unsigned int	nRipeTimeInterval;
	unsigned int	nBeginTime;
};
struct P_DBS4WEBUSER_STEAL_GOLD_T : public P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;			// 被偷者
	tty_id_t		nThiefAccountID;
	unsigned int	nAutoID;
	unsigned int	nGold;
};

struct P_DBS4WEB_CONTRIBUTE_ALLIANCE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nGold;
};
struct P_DBS4WEB_CONVERT_DIAMOND_GOLD_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nDiamond;
};
struct P_DBS4WEBUSER_CONTRIBUTE_ALLIANCE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nGold;
	unsigned int	nContributeNum;
	tty_id_t		nAllianceID;
	unsigned int	nCongressLevel;
};
struct P_DBS4WEBUSER_CONVERT_DIAMOND_GOLD_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nDiamond;
	unsigned int	nGold;
};
struct P_DBS4WEB_JOIN_ALLIANCE_NAME_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
};
struct P_DBS4WEB_GET_MY_ALLIANCE_JOIN_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_JOIN_ALLIANCE_NAME_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
};
struct P_DBS4WEBUSER_GET_MY_ALLIANCE_JOIN_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
};
struct P_DBS4WEB_SET_ALLI_INTRO_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nAllianceID;
	char			szIntroduction[TTY_INTRODUCTION_LEN];
};
struct P_DBS4WEBUSER_SET_ALLI_INTRO_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nAllianceID;
	char			szIntroduction[TTY_INTRODUCTION_LEN];
};
struct P_DBS4WEB_GET_OTHER_GOLDORE_POSITION_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_OTHER_GOLDORE_PRODUCTION_TE_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_OTHER_GOLDORE_FETCH_INFO_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nMyAccountID;
};
struct P_DBS4WEB_GET_OTHER_CHRISTMAS_TREE_INFO_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nMyAccountID;
};
struct P_DBS4WEBUSER_GET_OTHER_GOLDORE_POSITION_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	int				nNum;
};
struct P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	int				nNum;
};
struct P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_TE_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	int				nNum;
};
struct P_DBS4WEBUSER_GET_OTHER_GOLDORE_FETCH_INFO_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	int				nNum;
};
struct P_DBS4WEBUSER_GET_OTHER_CHRISTMAS_TREE_INFO_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned char	nState;
	unsigned int	nCountDownTime;	// 有成熟时间事件时间倒计时
	unsigned int	nWateredNum;	// 已浇水次数
	bool			bICanWater;		// 我是否可以浇水 
};
struct P_DBS4WEB_CHECK_DRAW_LOTTERY_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nAllianceID;
	unsigned int	nType;
};
struct P_DBS4WEBUSER_CHECK_DRAW_LOTTERY_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nType;
	unsigned int	nPrice;			// money_type_none,price为0
	tty_id_t		nAccountID;
	tty_id_t		nAllianceID;
};
struct P_DBS4WEB_FETCH_LOTTERY_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nType;
	unsigned int	nData;
	bool			bTopReward;
};
struct P_DBS4WEBUSER_FETCH_LOTTERY_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nType;
	unsigned int	nData;
	tty_id_t		nItemID;
	bool			bTopReward;
	bool			bNotifyWorld;
	unsigned int	nValue;
	char			szItemName[TTY_ITEM_LEN];
};
struct P_DBS4WEB_GET_CHRISTMAS_TREE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_GET_CHRISTMAS_TREE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned char	nState;
	unsigned int	nCountDownTime;	// 有成熟时间事件时间倒计时
	unsigned int	nCanWaterCountDownTime;	// 能够浇水的时间事件倒计时
	unsigned int	nWateredNum;	// 已浇水次数 
	bool			bICanWater;		// 我是否可以浇水  
};
struct P_DBS4WEB_GET_OTHER_GOLDORE_INFO_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nMyAccountID;
};
struct P_DBS4WEBUSER_GET_OTHER_GOLDORE_INFO_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_rst_t		nRst;
	unsigned char	nCTState;
	unsigned int	nCTCountDownTime;	// 有成熟时间事件时间倒计时 
	unsigned int	nCTWateredNum;		// 已浇水次数 
	bool			bCTICanWater;		// 我是否可以浇水 
	int				nNum;
	// 后面跟nNum个GoldoreInfo
};
struct P_DBS4WEB_GET_GOLDORE_SMP_INFO_ALL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nNum;
	// 后面跟nNum个tty_id_t
};
struct P_DBS4WEBUSER_GET_GOLDORE_SMP_INFO_ALL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	// 后面跟nNum个GoldoreSmpInfo
};
struct P_DBS4WEB_GET_ALLIANCE_TRADE_INFO_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_ALLIANCE_TRADE_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_DEAL_ALLIANCE_TREADE_TE_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nEventID;
};
struct P_DBS4WEBUSER_GET_ALLIANCE_TRADE_INFO_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nCountDownTime;
	unsigned int	nGold;
	unsigned int	nTradeNum;
	unsigned int	nExcelTradeNum;
	unsigned int	nExcelTradeTime;
	unsigned int	nExcelTradeGold;	
};
struct P_DBS4WEBUSER_ALLIANCE_TRADE_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nCountDownTime;
	unsigned int	nGold;
};
struct P_DBS4WEBUSER_DEAL_ALLIANCE_TREADE_TE_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
	unsigned int	nGold;
};
struct P_DBS4WEB_GET_ENEMY_LIST_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_DELETE_ENEMY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nEnemyID;
};
struct P_DBS4WEB_ADD_ENEMY_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
	tty_id_t		nEnemyID;
};
struct P_DBS4WEBUSER_GET_ENEMY_LIST_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	// 后面跟nNum个EnemyUnit
};
struct P_DBS4WEBUSER_DELETE_ENEMY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nEnemyID;
};
struct P_DBS4WEBUSER_ADD_ENEMY_T : P_DBS4WEB_CMD_T
{
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nEnemyID;
};
struct P_DBS4WEB_REFRESH_CHAR_ATB_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
	bool			bSimple;
};
struct P_DBS4WEBUSER_REFRESH_CHAR_ATB_T : P_DBS4WEB_CMD_T 
{
	bool			bSimple;
	// CharAtb			charAtb;
};
struct P_DBS4WEB_ADD_ALLIANCE_DEV_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nAllianceID;
	int				nDeltaDevelopment;
};
struct P_DBS4WEBUSER_ADD_ALLIANCE_DEV_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nAllianceID;
	int				nDeltaDevelopment;
};
struct P_DBS4WEB_LOAD_RANK_LIST_T : P_DBS4WEB_CMD_T
{
	unsigned int	nType;
};
struct P_DBS4WEBUSER_LOAD_RANK_LIST_T : P_DBS4WEB_CMD_T
{
	unsigned int	nType;
	unsigned int	nNum;
	// RankData
};
struct P_DBS4WEB_RECHARGE_DIAMOND_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nAddedDiamond;
	bool			bFromGS;
};
struct P_DBS4WEBUSER_RECHARGE_DIAMOND_T : P_DBS4WEB_CMD_T 
{
	enum
	{
		RST_OK					= 0,
		RST_NO_CHAR				= -1,		// 角色不存在

		// 其他错误
		RST_DB_ERR				= -200,
		RST_SQL_ERR				= -201,
	};
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nAddedDiamond;
	unsigned int	nGiftDiamond;			// 充值附送的钻石
	unsigned int	nVip;
	bool			bFromGS;
};
struct P_DBS4WEB_WRITE_CHAR_SIGNATURE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	char			szSignature[TTY_CHARACTER_SIGNATURE_LEN];
};
struct P_DBS4WEBUSER_WRITE_CHAR_SIGNATURE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	char			szSignature[TTY_CHARACTER_SIGNATURE_LEN];
};
struct P_DBS4WEB_GET_INSTANCE_ID_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
};
struct P_DBS4WEBUSER_GET_INSTANCE_ID_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_CHANGE_NAME_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	char			szNewName[TTY_CHARACTERNAME_LEN];
};
struct P_DBS4WEBUSER_CHANGE_NAME_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	char			szNewName[TTY_CHARACTERNAME_LEN];
	char			szOldName[TTY_CHARACTERNAME_LEN];
	unsigned int	nCostDiamond;
};
struct P_DBS4WEB_MOVE_POSITION_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nSrcPosX;
	unsigned int	nSrcPosY;
	unsigned int	nDstPosX;
	unsigned int	nDstPosY;
};
struct P_DBS4WEBUSER_MOVE_POSITION_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nSrcPosX;
	unsigned int	nSrcPosY;
	unsigned int	nDstPosX;
	unsigned int	nDstPosY;
};
struct P_DBS4WEB_GET_PAY_SERIAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_GET_PAY_SERIAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	char			szPaySerial[TTY_PAY_SERIAL_LEN];
};
struct P_DBS4WEB_GET_PAY_LIST_T : P_DBS4WEB_CMD_T 
{

};
struct P_DBS4WEBUSER_GET_PAY_LIST_T : P_DBS4WEB_CMD_T 
{
	int				nNum;
};
struct P_DBS4WEB_DEAL_ONE_PAY_T : P_DBS4WEB_CMD_T 
{
	char			szPaySerial[TTY_PAY_SERIAL_LEN+1];
};
struct P_DBS4WEBUSER_DEAL_ONE_PAY_T : P_DBS4WEB_CMD_T  
{
	tty_id_t		nAccountID;
	unsigned int	nAddedDiamond;
	unsigned int	nVip;
};
struct P_DBS4WEB_READ_NOTIFICATION_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nNotificationID;
};
struct P_DBS4WEBUSER_READ_NOTIFICATION_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nNotificationID;
};
struct P_DBS4WEB_CHANGE_HERO_NAME_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nHeroID;
	char			szName[TTY_CHARACTERNAME_LEN];
};
struct P_DBS4WEBUSER_CHANGE_HERO_NAME_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nDiamond;
};
struct P_DBS4WEB_READ_MAIL_ALL_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nMailType;
	char			szTypeStr[1];	// 0结尾的字串
};
struct P_DBS4WEB_DELETE_MAIL_ALL_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nMailType;
	char			szTypeStr[1];	// 0结尾的字串
};
struct P_DBS4WEBUSER_READ_MAIL_ALL_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nMailType;
};
struct P_DBS4WEBUSER_DELETE_MAIL_ALL_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nMailType;
};
struct P_DBS4WEB_ADD_GM_MAIL_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nLength;
};
struct P_DBS4WEBUSER_ADD_GM_MAIL_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEB_GET_PLAYERCARD_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_GET_PLAYERCARD_T : P_DBS4WEB_CMD_T
{
	enum
	{
		RST_OK					= 0,
		RST_NOCHAR				= -1,

		RST_DB_ERR				= -200,
		RST_SQL_ERR				= -201,
	};
	tty_rst_t		nRst;
	// PlayerCard		card;
};
struct P_DBS4WEB_DEAL_EXCEL_TE_T : P_DBS4WEB_CMD_T
{
};
struct P_DBS4WEBUSER_DEAL_EXCEL_TE_T : P_DBS4WEB_CMD_T
{
	struct ExcelTERes 
	{
		unsigned int	nExcelID;
		unsigned int	nData;
	};
	unsigned int	nNum;
	// 后面跟nNum个ExcelTERes
};
struct P_DBS4WEB_CHANGE_ALLIANCE_NAME_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
};
struct P_DBS4WEBUSER_CHANGE_ALLIANCE_NAME_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nAllianceID;
	char			szAllianceName[TTY_ALLIANCENAME_LEN];
	unsigned int	nDiamond;
};
struct P_DBS4WEB_GEN_WORLD_FAMOUS_CITY_T : P_DBS4WEB_CMD_T
{
	bool			bClearFirst;
	int				nNum;
	// WorldFamousCity
};
struct P_DBS4WEBUSER_GEN_WORLD_FAMOUS_CITY_T : P_DBS4WEB_CMD_T
{

};
struct P_DBS4WEB_GET_WORLD_FAMOUS_CITY_T : P_DBS4WEB_CMD_T
{
	
};
struct P_DBS4WEBUSER_GET_WORLD_FAMOUS_CITY_T : P_DBS4WEB_CMD_T
{
	unsigned int	nNum;
	// WorldFamousCity
};
struct P_DBS4WEB_CUP_ACTIVITY_END_T : P_DBS4WEB_CMD_T
{
	
};
struct P_DBS4WEBUSER_CUP_ACTIVITY_END_T : P_DBS4WEB_CMD_T
{
	
};
struct P_DBS4WEB_FETCH_MAIL_REWARD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nMailID;
};
struct P_DBS4WEBUSER_FETCH_MAIL_REWARD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nMailID;
	unsigned int	nNum;
	// 后面跟nNum个
};
struct P_DBS4WEB_UPLOAD_BILL_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nClientTime;
	unsigned int	nServerTime;
	unsigned int	nAppID;
	unsigned int	nTextLen;
	char			szText[1];
};
struct P_DBS4WEB4USER_UPLOAD_BILL_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nClientTime;
	unsigned int	nServerTime;
};
struct P_DBS4WEB_SET_BINDED_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;	
};
struct P_DBS4WEB4USER_SET_BINDED_T : P_DBS4WEB_CMD_T
{
	
};
struct P_DBS4WEB_ADD_LOTTERY_LOG_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
	unsigned char	nType;
	unsigned int	nData;		// lottery_type_item->excel_id,其他为数量
	unsigned int	nTime;
};
struct P_DBS4WEBUSER_ADD_LOTTERY_LOG_T : P_DBS4WEB_CMD_T
{

};
struct P_DBS4WEB_DEAL_CHAR_DAY_TE_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_DEAL_CHAR_DAY_TE_T : P_DBS4WEB_CMD_T
{
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_DEAL_ADCOLONY_TE_T : P_DBS4WEB_CMD_T
{

};
struct P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T : P_DBS4WEB_CMD_T
{
	struct AdColonyAward 
	{
		tty_id_t		nAccountID;
		unsigned int	nMoneyType;
		unsigned int	nNum;
	};
	unsigned int	nNum;
};

struct P_DBS4WEB_DISMISS_SOLDIER_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_DISMISS_SOLDIER_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nNum;
};
struct P_DBS4WEB_REFRESH_MAIL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nMailID;
};
struct P_DBS4WEBUSER_REFRESH_MAIL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
};
struct P_DBS4WEB_BAN_CHAR_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_BAN_CHAR_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_NEW_ADCOLONY_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nTransactionID;
	unsigned int	nAmount;
	char			szName[32];
	unsigned int	nTime;
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_NEW_ADCOLONY_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nTransactionID;
};

struct P_DBS4WEB_ALLI_DONATE_SOLDIER_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nObjID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nNum;
};
struct P_DBS4WEB_ALLI_RECALL_SOLDIER_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nEventID;
};
struct P_DBS4WEB_ALLI_ACCE_SOLDIER_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nEventID;
	unsigned int	nTime;
	unsigned int	nMoneyType;
};
struct P_DBS4WEB_GET_DONATE_SOLDIER_QUEUE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_GET_LEFT_DONATE_SOLDIER_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_DEAL_DONATE_SOLDIER_TE_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};

struct P_DBS4WEBUSER_ALLI_DONATE_SOLDIER_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nObjID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nNum;
	unsigned int	nTime;
	tty_id_t		nEventID;
};
struct P_DBS4WEBUSER_ALLI_RECALL_SOLDIER_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nOldEventID;
	tty_id_t		nNewEventID;
	unsigned int	nTime;
	tty_id_t		nObjID;
};
struct P_DBS4WEBUSER_ALLI_ACCE_SOLDIER_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nEventID;
	unsigned int	nMoneyType;
	unsigned int	nMoney;
	unsigned int	nTime;
	tty_id_t		nObjID;
};
struct P_DBS4WEBUSER_GET_DONATE_SOLDIER_QUEUE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_GET_LEFT_DONATE_SOLDIER_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nLeftNum;
	unsigned int	nMaxNum;
};
struct P_DBS4WEBUSER_DEAL_DONATE_SOLDIER_TE_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
	tty_id_t		nAccountID;
	tty_id_t		nObjID;
	unsigned int	nExcelID;
	unsigned int	nLevel;
	unsigned int	nNum;
	unsigned int	nType;
};

struct P_DBS4WEB_ADD_GOLD_DEAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nGold;
	unsigned int	nPrice;
};
struct P_DBS4WEB_BUY_GOLD_IN_MARKET_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nTransactionID;
	unsigned int	nMoneyType;
};
struct P_DBS4WEB_CANCEL_GOLD_DEAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nTransactionID;
};
struct P_DBS4WEB_GET_GOLD_DEAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nPage;
	unsigned int	nPageSize;
};
struct P_DBS4WEB_GET_SELF_GOLD_DEAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_DEAL_GOLD_MARKET_TE_T : P_DBS4WEB_CMD_T 
{
	
};
struct P_DBS4WEBUSER_ADD_GOLD_DEAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nGold;
	tty_id_t		nTransactionID;
};
struct P_DBS4WEBUSER_BUY_GOLD_IN_MARKET_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nSellerID;
	unsigned int	nGold;
	unsigned int	nMoneyType;
	unsigned int	nTotalPrice;
	unsigned int	nBuyerMailID;
	unsigned int	nSellerMailID;
};
struct P_DBS4WEBUSER_CANCEL_GOLD_DEAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nMailID;
	unsigned int	nGold;
};
struct P_DBS4WEBUSER_GET_GOLD_DEAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nTotalNum;
	unsigned int	nPage;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_GET_SELF_GOLD_DEAL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T : P_DBS4WEB_CMD_T 
{
	struct UNIT_T
	{
		tty_id_t		nAccountID;
		unsigned int	nMailID;
		unsigned int	nGold;
	};
	unsigned int	nNum;
};
struct P_DBS4WEB_GEN_WORLD_RES_T : P_DBS4WEB_CMD_T 
{
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_GEN_WORLD_RES_T : P_DBS4WEB_CMD_T 
{

};
struct P_DBS4WEB_GET_WORLD_RES_T : P_DBS4WEB_CMD_T 
{
	
};
struct P_DBS4WEBUSER_GET_WORLD_RES_T : P_DBS4WEB_CMD_T 
{
	unsigned int	nNum;
};

//////////////////////////////////////////////////////////////////////////
// 联盟副本
//////////////////////////////////////////////////////////////////////////
struct P_DBS4WEB_ALLI_INSTANCE_CREATE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_CREATE_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nInstanceID;
	tty_id_t		nAccountID;
};
struct P_DBS4WEB_ALLI_INSTANCE_JOIN_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
}; 
struct P_DBS4WEBUSER_ALLI_INSTANCE_JOIN_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
}; 
struct P_DBS4WEB_ALLI_INSTANCE_EXIT_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_EXIT_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_ALLI_INSTANCE_KICK_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	tty_id_t		nObjID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_KICK_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nObjID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_ALLI_INSTANCE_DESTROY_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_DESTROY_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_ALLI_INSTANCE_CONFIG_HERO_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	tty_id_t		n1HeroID;
	tty_id_t		n2HeroID;
	tty_id_t		n3HeroID;
	tty_id_t		n4HeroID;
	tty_id_t		n5HeroID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_CONFIG_HERO_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEB_ALLI_INSTANCE_START_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_START_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	int				nNum;
	// 后面跟nNum个account_id,表示未就绪玩家ID,nRst=0,代表队伍的ID
};
struct P_DBS4WEB_ALLI_INSTANCE_READY_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	bool			bReady;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_READY_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	bool			bReady;
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_LIST_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_LIST_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	int				nNum;
	// 后面跟ROOM_T
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_CHAR_DATA_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_CHAR_DATA_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nInstanceID;
	int				nNum;
	// 后面跟CharData
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_HERO_DATA_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_HERO_DATA_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nInstanceID;
	int				nNum;
	// 后面跟HeroData
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_STATUS_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nCombatType;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_STATUS_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nCombatType;
	unsigned int	nTotalFreeTimes;
	unsigned int	nUsedFreeTimes;
	tty_id_t		nInstanceID;
	unsigned int	nCharStatus;				// instance_player_status_xxx
	unsigned int	nInstanceStatus;			// instance_status_xxx
	unsigned int	nCurLevel;
	bool			bAutoCombat;
	bool			bAutoSupply;
	unsigned int	nRetryTimes;
	tty_id_t		nCreatorID;
	unsigned int	nStopLevel;
	unsigned int	nMaxLevel;
	unsigned int	nLastCombatResult;
};
struct P_DBS4WEB_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nDeployDataLen;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_LOOT_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_LOOT_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nInstanceID;
	tty_rst_t		nRst;
	// 字符串
};
struct P_DBS4WEB_ALLI_INSTANCE_START_COMBAT_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nObjID;
	unsigned int	nCombatType;
	bool			bAutoCombat;
	bool			bAutoSupply;
	unsigned int	nStopLevel;
	unsigned int	nLength;
	// hero_deploy
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_START_COMBAT_T : P_DBS4WEB_CMD_T  
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nEventID;
	unsigned int	nMarchTime;
	tty_id_t		nAccountID;
	tty_id_t		nObjID;
	unsigned int	nCombatType;
};

struct P_DBS4WEB_KICK_CLIENT_ALL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned int	nLength;
	// 字符串
};
struct P_DBS4WEBUSER_KICK_CLIENT_ALL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEB_GET_KICK_CLIENT_ALL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
};
struct P_DBS4WEBUSER_GET_KICK_CLIENT_ALL_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2DB4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nLength;
};
struct P_DBS4WEB_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_T : P_DBS4WEB_CMD_T 
{
	tty_rst_t		nRst;
};
struct P_DBS4WEB_ALLI_INSTANCE_DEAL_MARCH_BACK_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_MARCH_BACK_T : P_DBS4WEB_CMD_T 
{
	int				nCombatType;
	tty_id_t		nAccountID;
	tty_id_t		nObjID;
	bool			bInstanceDestroyed;
	unsigned int	nInstanceLevel;
	tty_id_t		nEventID;
	bool			bAutoSupply;
	// hero_deploy
};
struct P_DBS4WEB_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nEventID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T : P_DBS4WEB_CMD_T 
{
	tty_rst_t		nRst;
	int				nCombatResult;
	unsigned int	nCombatType;
	tty_id_t		nAccountID;
	tty_id_t		nObjID;
	unsigned int	nBackLeftTime;
	unsigned int	nInstanceLevel;
	bool			bRefreshCharAtb;
	tty_id_t		nEventID;
	// 后面跟
	// int			nMailIDsLen;
	// char			szMailIDs[nMailIDsLen];
	// int			nNotifyLen;
	// char			szNotify[nNotifyLen];
	// int			nCombatLogSize;
	// 战报
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T : P_DBS4WEB_CMD_T 
{

};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T : P_DBS4WEB_CMD_T 
{
	unsigned int	nNum;
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_INSTANCE_ID_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_INSTANCE_ID_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nAccountID;
	unsigned int	nNum;
};

struct P_DBS4WEB_GET_HERO_SIMPLE_DATA_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nID;
	unsigned int	nReason;
	unsigned int	nCombatType;		// 为0代表自己同步,其他为战斗同步
	unsigned int	nNum;
//	tty_id_t		nHeroID;
};
struct P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T : P_DBS4WEB_CMD_T 
{
	tty_id_t		nID;
	unsigned int	nReason;
	unsigned int	nCombatType;		// 为0代表自己同步,其他为战斗同步
	unsigned int	nNum;
//	HeroSimpleData	hero;
};
struct P_DBS4WEB_GET_INSTANCE_SIMPLE_DATA_T : P_DBS4WEB_CMD_T 
{

};
struct P_DBS4WEBUSER_GET_INSTANCE_SIMPLE_DATA_T : P_DBS4WEB_CMD_T 
{
	unsigned int	nNum;
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_COMBAT_LOG_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nCombatType;
	tty_id_t		nID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_LOG_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nCombatType;
	tty_id_t		nID;
	tty_rst_t		nRst;
	// 战报AlliInstanceCombatLog
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nCombatType;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T : P_DBS4WEB_CMD_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_rst_t		nRst;
	unsigned int	nBackLeftTime;			// 返回倒计时
	unsigned int	nCombatType;
	unsigned int	nCombatResult;
	unsigned int	nInstanceLevel;
	tty_id_t		nInstanceID;
};
struct P_DBS4WEB_ALLI_INSTANCE_STOP_COMBAT_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	unsigned int	nCombatType;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_STOP_COMBAT_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	unsigned int	nCombatType;
	tty_rst_t		nRst;
};
struct P_DBS4WEB_ALLI_INSTANCE_SUPPLY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	unsigned int	nCombatType;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_SUPPLY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_id_t		nInstanceID;
	unsigned int	nCombatType;
	tty_rst_t		nRst;
	// char_ids
};
struct P_DBS4WEB_ALLI_INSTANCE_SYSTEM_DESTROY_T : P_DBS4WEB_CMD_T
{

};
struct P_DBS4WEBUSER_ALLI_INSTANCE_SYSTEM_DESTROY_T : P_DBS4WEB_CMD_T
{

};
// 竞技场
struct P_DBS4WEB_ARENA_UPLOAD_DATA_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nLength;
};
struct P_DBS4WEBUSER_ARENA_UPLOAD_DATA_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_rst_t		nRst;
	unsigned int	nRank;
	bool			bFirstUpload;
};
struct P_DBS4WEB_ARENA_GET_RANK_LIST_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nPageSize;
	unsigned int	nPage;
};
struct P_DBS4WEBUSER_ARENA_GET_RANK_LIST_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_rst_t		nRst;
	unsigned int	nTotalNum;
	unsigned int	nPage;
	int				nNum;
};
struct P_DBS4WEB_ARENA_GET_STATUS_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	int				nNum;
};
struct P_DBS4WEBUSER_ARENA_GET_STATUS_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	tty_rst_t		nRst;
	unsigned int	nChallengeTimesLeft;	// 今日剩余挑战次数
	unsigned int	nUploadTimeLeft;		// 下一次可以上传镜像的CD
	unsigned int	nPayTimesLeft;
	unsigned int	nRank;
	int				nNum;
};
struct P_DBS4WEB_ARENA_PAY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nMoneyType;
	unsigned int	nNum;
};
struct P_DBS4WEBUSER_ARENA_PAY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	unsigned int	nMoneyType;
	unsigned int	nPrice;
};
struct P_DBS4WEB_ARENA_DEAL_REWARD_EVENT_T : P_DBS4WEB_CMD_T
{

};
struct P_DBS4WEBUSER_ARENA_DEAL_REWARD_EVENT_T : P_DBS4WEB_CMD_T
{
	// mail_ids字串,0结尾
};
struct P_DBS4WEB_ARENA_GET_DEPLOY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_ARENA_GET_DEPLOY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	// 后面为0结尾的串
};
struct P_DBS4WEB_HERO_SUPPLY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nCombatType;
	// 0结尾字串
};
struct P_DBS4WEBUSER_HERO_SUPPLY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nCombatType;
	tty_rst_t		nRst;
};
struct P_DBS4WEB_REFRESH_DRUG_T : P_DBS4WEB_CMD_T
{
	// 字符串数组
};
struct P_DBS4WEBUSER_REFRESH_DRUG_T : P_DBS4WEB_CMD_T
{
	struct DrugInfo
	{
		tty_id_t		nAccountID;
		unsigned int	nDrug;
	};
	unsigned int	nNum;
};
struct P_DBS4WEB_SET_VIP_DISPLAY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	bool			bVipDisplay;
};
struct P_DBS4WEBUSER_SET_VIP_DISPLAY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	bool			bVipDisplay;
};
struct P_DBS4WEB_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned int	nExcelID;			// 副本ID
	unsigned int	nLevel;
};
struct P_DBS4WEBUSER_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nExcelID;			// 副本ID
	unsigned int	nLevel;
	unsigned int	nNum;
};

// 位置收藏夹
struct P_DBS4WEB_POSITION_MARK_ADD_RECORD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	// PositionMark
};
struct P_DBS4WEBUSER_POSITION_MARK_ADD_RECORD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEB_POSITION_MARK_CHG_RECORD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	// PositionMark
};
struct P_DBS4WEBUSER_POSITION_MARK_CHG_RECORD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEB_POSITION_MARK_DEL_RECORD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nPosX;
	unsigned int	nPosY;
};
struct P_DBS4WEBUSER_POSITION_MARK_DEL_RECORD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
};
struct P_DBS4WEB_POSITION_MARK_GET_RECORD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
struct P_DBS4WEBUSER_POSITION_MARK_GET_RECORD_T : P_DBS4WEB_CMD_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nNum;
	// PositionMark
};
#pragma pack()
}

#endif
