<?php

include_once "db-config.inc.php";

// 系统的名字
define('g_sysname','TinyKingdom');
define('g_sysver','V0.10');

// gm邮件状态定义
// 处理完毕
define('g_gm_mail_status_over', 0);
// 等待处理
define('g_gm_mail_status_pending', 1);
// 暂时挂起
define('g_gm_mail_status_suspend', 2);
// 需要注意
define('g_gm_mail_status_important', 10);

$g_gm_page_size = 50;

// 状态对应字串
$g_gm_mail_status_STR	= array();
$g_gm_mail_status_STR[g_gm_mail_status_over]		= '处理完毕';
$g_gm_mail_status_STR[g_gm_mail_status_pending]		= '等待处理';
$g_gm_mail_status_STR[g_gm_mail_status_suspend]		= '暂时挂起';
$g_gm_mail_status_STR[g_gm_mail_status_important]	= '需要注意!!';

// 将领状态
$g_gm_hero_status_STR	= array();
$g_gm_hero_status_STR[0]		= '空闲';
$g_gm_hero_status_STR[1]		= '普通战斗，讨伐';
$g_gm_hero_status_STR[2]		= '玩家对战，攻方';
$g_gm_hero_status_STR[3]		= '玩家对战，守方';
$g_gm_hero_status_STR[10]		= '世界金矿（对AI）';
$g_gm_hero_status_STR[11]		= '世界金矿（对玩家），攻方';
$g_gm_hero_status_STR[12]		= '世界金矿（对玩家），守方';
$g_gm_hero_status_STR[20]		= '世界名城';
$g_gm_hero_status_STR[100]		= '副本，百战不殆';
$g_gm_hero_status_STR[101]		= '副本，南征北战';
$g_gm_hero_status_STR[1000]		= '在武将馆修炼';

// 用户信息field的中文名
$g_gm_pinfo_field_STR	= array();
$g_gm_pinfo_field_STR['name']		= '角色名';
$g_gm_pinfo_field_STR['ctime']		= '创建时间';
$g_gm_pinfo_field_STR['intime']		= '上次登录';
$g_gm_pinfo_field_STR['outtime']	= '上次注销';
$g_gm_pinfo_field_STR['exp']		= '经验';
$g_gm_pinfo_field_STR['level']		= '级别';
$g_gm_pinfo_field_STR['population']	    	= '人口';
$g_gm_pinfo_field_STR['diamond']	= '钻石';
$g_gm_pinfo_field_STR['crystal']	= '水晶';
$g_gm_pinfo_field_STR['vip']		= 'VIP级别';
$g_gm_pinfo_field_STR['development']		= '发展度';
$g_gm_pinfo_field_STR['gold']		= '金币';
$g_gm_pinfo_field_STR['total_diamond']		= '累计钻石充值';

// 抽奖货币类型
$g_gm_money_type[0]         = '免费';
$g_gm_money_type[1]         = '钻石';
$g_gm_money_type[2]         = '水晶';
$g_gm_money_type[3]         = '金币';
$g_gm_money_type[4]         = '联盟贡献度';

// 抽奖获取到的奖品类型
define('g_gm_lottery_rst_type_none',          0);
define('g_gm_lottery_rst_type_item',          1);
define('g_gm_lottery_rst_type_crystal',       2);
define('g_gm_lottery_rst_type_gold',          3);

$g_gm_lottery_rst_type_STR[0]    = '未中奖';
$g_gm_lottery_rst_type_STR[1]    = '物品';
$g_gm_lottery_rst_type_STR[2]    = '水晶';
$g_gm_lottery_rst_type_STR[3]    = '金币';

// 兵类型
$g_gm_army_type[1]      = '骑兵';
$g_gm_army_type[2]      = '枪兵';
$g_gm_army_type[3]      = '剑士';
$g_gm_army_type[4]      = '弓兵';
$g_gm_army_type[5]      = '器械';

// 角色日志查询类型
$g_gm_logtype_STR['stone_consume']  = '钻石/水晶消费统计';
$g_gm_logtype_STR['item_sell']      = '道具售出记录';
$g_gm_logtype_STR['lottery_gain']   = '抽奖获得记录';

define('g_gm_money_use_type_none', 0);
define('g_gm_money_use_type_buy_item', 1);        // 商店购买道具
define('g_gm_money_use_type_accelerate_build', 2);        // 加速建筑
define('g_gm_money_use_type_accelerate_goldore', 3);        // 金矿生产加速
define('g_gm_money_use_type_accelerate_research', 4);        // 加速研究
define('g_gm_money_use_type_accelerate_march', 5);        // 行军加速
define('g_gm_money_use_type_lottery', 6);        // 抽奖
define('g_gm_money_use_type_change_char_name', 7);        // 君主改名
define('g_gm_money_use_type_change_hero_name', 8);        // 英雄改名
define('g_gm_money_use_type_change_alliance_name', 9);        // 联盟改名
define('g_gm_money_use_type_store_buy_gold', 10);       // 购买金币

// 钻石水晶消费类型
$g_gm_money_use_type_STR[0] = "未定义";
$g_gm_money_use_type_STR[1] = "商店购买道具";
$g_gm_money_use_type_STR[2] = "加速建筑";
$g_gm_money_use_type_STR[3] = "金矿生产加速";
$g_gm_money_use_type_STR[4] = "加速研究";
$g_gm_money_use_type_STR[5] = "行军加速";
$g_gm_money_use_type_STR[6] = "抽奖";
$g_gm_money_use_type_STR[7] = "君主改名";
$g_gm_money_use_type_STR[8] = "英雄改名";
$g_gm_money_use_type_STR[9] = "联盟改名";
$g_gm_money_use_type_STR[10] = "购买金币";

// 奖励邮件定义
// 奖励类型
$g_gm_mail_reward_type[1]   = '道具';
$g_gm_mail_reward_type[2]   = '钻石';
$g_gm_mail_reward_type[3]   = '水晶';
$g_gm_mail_reward_type[4]   = '金币';
// 注：附件格式为'类型*数量*附加数据,类型*数量*附加数据
//     flag为3（奖励邮件标识），type为4（系统邮件），奖励类型为道具则附加数据为excel_id，否则填0(方便客户端解析)
//     ext_data:'type*num*excel_id,'(金币钻石水晶的excel_id填0,尾巴需要加一个,),如'1*2*3,1*5*5,2*5*0,3*10*0,1*100*1001,'

// 设备ID定义
$g_gm_device_type[0]    = 'PC';
$g_gm_device_type[2]    = 'iPhone3G';
$g_gm_device_type[3]    = 'iPhone3GS';
$g_gm_device_type[8]    = 'iPhone4';
$g_gm_device_type[11]   = 'iPhone4S';
$g_gm_device_type[13]   = 'iPhone5';
$g_gm_device_type[6]    = 'iPodTouch3Gen';
$g_gm_device_type[9]    = 'iPodTouch4Gen';
$g_gm_device_type[14]   = 'iPodTouch5Gen';
$g_gm_device_type[7]    = 'iPad1Gen';
$g_gm_device_type[10]   = 'iPad2Gen';
$g_gm_device_type[12]   = 'iPad3Gen';

?>
