//////////////////////////////////////////////////////////////////////////
// logout,disconnect暂时不要使用,有问题
// 一定要有exit指令
// whcmn_nml_console线程的退出是根据系统原生指令exit来做的(汗!!!)
//////////////////////////////////////////////////////////////////////////
#ifndef __StrCmdDealer_H__
#define __StrCmdDealer_H__

#include "WHCMN/inc/whcmd.h"
#include "WHCMN/inc/whvector.h"

#include "WHNET/inc/whnetepoll_client.h"

#include "PNGS/inc/pngs_def.h"

#include "../../Common/inc/pngs_packet_web_extension.h"
#include "../../Common/inc/tty_lp_client_packet.h"
#include "../../Common/inc/tty_lp_client_gamecmd_packet.h"
#include "../../Common/inc/tty_common_def.h"
#include "../../Common/inc/tty_common_PlayerData4Web.h"

using namespace n_whcmn;
using namespace n_whnet;
using namespace n_pngs;

extern epoll_connecter_client*	g_pEpollConnecterClient;
extern bool	g_bStop;
extern int	g_nTermType;

class CStrCmdDealer
{
private:
	// 用于组装较长的指令
	whvector<char>	m_vectrawbuf;
public:
	CStrCmdDealer()
	{
		m_vectrawbuf.reserve(4*1024);
		// 字串处理函数
		whcmn_strcmd_reganddeal_REGCMD_m_srad(NULL, _i_srad_NULL, NULL);
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(help, "[cmd]");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(connect, "type // 1->iOS,2->Android,3->WP7,4->Web");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(login, "account,device_type,[password,appid]");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(login_device, "device_id,device_type,appid");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(exit, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_get, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(create_account, "device_id,device_type,[use_random_name,account_name,password,appid]");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(create_char, "char_name[,sex,head_id,country]");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_playercard, "account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_playercard_by_position, "pos_x,pos_y");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_playercard_by_name, "char_name");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_tile_info, "pos_x,pos_y");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_building, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_building_te, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(build_building, "excel_id,auto_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(upgrade_building, "auto_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(produce_gold, "auto_id,choice // choice:0~3");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(fetch_gold, "auto_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_production_event, "account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_production_te, "account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(accelerate_gold_produce, "auto_id,time,money_type // money_type:1->diamond,2->crystal");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(accelerate_building, "auto_id,time,type,money_type // type:1->build,2->upgrade,money_type:1->diamond,2->crystal");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(accelerate_research, "excel_id,time,money_type // money_type:1->diamond,2->crystal");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_technology, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_research_te, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(research, "excel_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(conscript_soldier, "excel_id,level,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(upgrade_soldier, "excel_id,from_level,to_level,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_soldier, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(accept_alliance_member, "new_member_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(build_alliance_building, "excel_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(cancel_join_alliance, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(create_alliance, "alliance_name");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(dismiss_alliance, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(exit_alliance, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(expel_alliance_member, "member_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(join_alliance, "alliance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(set_alliance_position, "member_id,position");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_info, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_member, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_building_te, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_building, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_join_event, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(refuse_join_alliance, "applicant_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(abdicate_alliance, "member_name");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_char_atb, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(chat_private, "peer_account_id,chat_text");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(chat_alliance, "chat_text");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(chat_world, "chat_text");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(chat_group, "chat_text");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(add_mail, "receiver_id,text");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(add_mail_with_ext_data, "receiver_id,text,ext_data");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_mail, "from_num,to_num,type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(read_mail, "mail_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(delete_mail, "mail_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_mail_1, "mail_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_my_friend_apply, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_others_friend_apply, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_friend, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(approve_friend, "peer_account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(apply_friend, "peer_account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(refuse_friend, "peer_account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(cancel_friend, "peer_account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(delete_friend, "peer_account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_world_area, "from_x,from_y,to_x,to_y");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(add_alliance_mail, "text");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_mail, "from_num,to_num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_log, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_private_log, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(buy_item, "excel_id,num,money_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(sell_item, "item_id,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_relation_log, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(fetch_christmas_tree, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(watering_christmas_tree, "peer_account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(steal_gold, "peer_account_id,auto_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_training_te, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(add_training, "time_unit_num,hero_id,hero_id... // time_unit_num:1~10");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(exit_training, "hero_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_recent_chat, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(contribute_alliance, "gold");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(convert_diamond_gold, "diamond");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(join_alliance_name, "alliance_name");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_my_alliance_join, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_card_by_id, "alliance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_card_by_rank, "from_rank,to_rank // 0~n");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(set_alliance_introduction, "introduction");
		//whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(draw_lottery, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_other_goldore, "account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(check_draw_lottery, "0->free,1->diamond,2->crystal");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(fetch_lottery, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_christmas_tree, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_goldore_smp_info_all, "account_id1,account_id2,...");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_alliance_trade_info, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alliance_trade, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_enemy_list, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(delete_enemy, "enemy_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_rank_list, "rank_type,from,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_my_rank, "rank_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(write_signature, "signature");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_simple_char_atb, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(change_name, "new_name");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(move_position, "pos_x,pos_y");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_pay_serial, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(read_notification, "notification_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(change_hero_name, "hero_id,new_name");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(read_mail_all, "mail_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(delete_mail_all, "mail_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(add_gm_mail, "text");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(set_alli_name, "name");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_famous_city_list, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(fetch_mail_reward, "mail_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(upload_bill, "bill_text");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(register, "account,pass,old_pass");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(bind_device, "device_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_server_time, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(dismiss_soldier, "excel_id,level,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_donate_soldier, "obj_id,excel_id,level,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_recall_soldier, "event_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(acce_donate_soldier, "event_id,time,money_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_donate_soldier_queue, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_left_donate_soldier, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_world_res_card, "");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(add_gold_deal, "gold,price");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(buy_gold_in_market, "transaction_id,money_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(cancel_gold_deal, "transaction_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_gold_deal, "page,page_size");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_self_gold_deal, "");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(tavern_refresh, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(hire_hero, "slot_id,hero_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(fire_hero, "hero_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(levelup_hero, "hero_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(add_grow, "hero_id,grow");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(config_hero, "hero_id,army_type,army_level,army_num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_hero_refresh, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_hero_hire, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(start_combat, "obj_id,combat_type,auto_combat,auto_supply,hero1,hero2,hero3,hero4,hero5");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(stop_combat, "combat_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_combat, "combat_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(save_combat, "combat_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(list_combat, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(config_citydefense, "hero1,hero2,hero3,hero4,hero5");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_item, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(add_item, "excel_id,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(del_item, "item_id,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(equip_item, "hero_id,item_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(disequip_item, "hero_id,item_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(mount_item, "hero_id,equip_id,slot_idx,excel_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(unmount_item, "hero_id,equip_id,slot_idx");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(compos_item, "excel_id,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(discompos_item, "excel_id,num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(equip_item_all, "hero_id,item1,item2,item3,item4,item5");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(mount_item_all, "hero_id,equip_id,excel1,excel2,excel3,excel4");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(replace_equip, "hero_id,equip_type,item_id");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_quest, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(done_quest, "excel_id");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_instancestatus, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_instancedesc, "excel_id,class_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(create_instance, "excel_id,class_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(join_instance, "excel_id,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(quit_instance, "excel_id,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(destroy_instance, "excel_id,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_instancedata, "excel_id,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(config_instancehero, "excel_id,instance_id,hero1,hero2,hero3,hero4,hero5");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_instanceloot, "excel_id,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(auto_combat, "instance_id,val");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(auto_supply, "instance_id,val");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(manual_supply, "instance_id,combat_type,hero1,hero2,hero3,hero4,hero5");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(kick_instance, "excel_id,instance_id,obj_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(start_instance, "excel_id,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(prepare_instance, "excel_id,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(supply_instance, "excel_id,instance_id");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(use_item, "hero_id,excel_id,num");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(sync_char, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(sync_hero, "hero_id");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_equip, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_gem, "");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_worldgoldmine, "area");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(config_worldgoldmine_hero, "hero1,hero2,hero3,hero4,hero5");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(rob_worldgoldmine, "area,class,idx,hero1,hero2,hero3,hero4,hero5");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(drop_worldgoldmine, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(my_worldgoldmine, "");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(combat_prof, "class_id,obj_id,combat_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_citydefense, "");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(use_drug, "num");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(army_accelerate, "combat_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(army_back, "combat_id");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(worldcity, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(worldcity_log, "pagenum,pageidx");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(worldcity_log_alliance, "pagenum,pageidx");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(worldcity_rank_man, "pagenum,pageidx");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(worldcity_rank_alliance, "pagenum,pageidx");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(recharge_try, "diamond");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_create, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_get_list, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_join, "instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_exit, "instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_kick, "instance_id,obj_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_destroy, "instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_config_hero, "instance_id,hero1,hero2,hero3,hero4,hero5");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_start, "instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_ready, "instance_id,ready");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_get_char_data, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_get_hero_data, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_get_status, "combat_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_save_hero_deploy, "col,row,hero_id,...,col,row,hero_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_get_combat_log, "combat_type,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_get_simple_combat_log, "combat_type,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_stop_combat, "combat_type,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_supply, "combat_type,instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_get_loot, "instance_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(alli_instance_get_defender_deploy, "excel_id,level");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(get_kick_client_all, "");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(arena_upload_data, "col,row,hero_id,...,col,row,hero_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(arena_get_status, "num");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(arena_pay, "money_type");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(arena_get_rank_list, "page,page_size");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(arena_get_deploy, "");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(position_mark_add,"pos_x,pos_y,head_id,prompt");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(position_mark_del,"pos_x,pos_y");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(position_mark_chg,"pos_x,pos_y,head_id,prompt");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(position_mark_get,"");
	}
	~CStrCmdDealer()
	{

	}
	whcmn_strcmd_reganddeal_DECLARE_m_srad(CStrCmdDealer);
	int		_i_srad_NULL(const char* param);
	int		_i_srad_help(const char* param);
	int		_i_srad_connect(const char* param);
	int		_i_srad_login(const char* param);
	int		_i_srad_login_device(const char* param);
	int		_i_srad_exit(const char* param);
	int		_i_srad_notify_get(const char* param);
	int		_i_srad_create_account(const char* param);
	int		_i_srad_create_char(const char* param);
	int		_i_srad_get_playercard(const char* param);
	int		_i_srad_get_playercard_by_position(const char* param);
	int		_i_srad_get_playercard_by_name(const char* param);
	int		_i_srad_get_tile_info(const char* param);
	int		_i_srad_get_building(const char* param);
	int		_i_srad_get_building_te(const char* param);
	int		_i_srad_build_building(const char* param);
	int		_i_srad_upgrade_building(const char* param);
	int		_i_srad_produce_gold(const char* param);
	int		_i_srad_fetch_gold(const char* param);
	int		_i_srad_get_production_event(const char* param);
	int		_i_srad_get_production_te(const char* param);
	int		_i_srad_accelerate_gold_produce(const char* param);
	int		_i_srad_accelerate_building(const char* param);
	int		_i_srad_accelerate_research(const char* param);
	int		_i_srad_get_technology(const char* param);
	int		_i_srad_get_research_te(const char* param);
	int		_i_srad_research(const char* param);
	int		_i_srad_conscript_soldier(const char* param);
	int		_i_srad_upgrade_soldier(const char* param);
	int		_i_srad_get_soldier(const char* param);
	int		_i_srad_accept_alliance_member(const char* param);
	int		_i_srad_build_alliance_building(const char* param);
	int		_i_srad_cancel_join_alliance(const char* param);
	int		_i_srad_create_alliance(const char* param);
	int		_i_srad_dismiss_alliance(const char* param);
	int		_i_srad_exit_alliance(const char* param);
	int		_i_srad_expel_alliance_member(const char* param);
	int		_i_srad_join_alliance(const char* param);
	int		_i_srad_set_alliance_position(const char* param);
	int		_i_srad_get_alliance_info(const char* param);
	int		_i_srad_get_alliance_member(const char* param);
	int		_i_srad_get_alliance_building_te(const char* param);
	int		_i_srad_get_alliance_building(const char* param);
	int		_i_srad_get_alliance_join_event(const char* param);
	int		_i_srad_refuse_join_alliance(const char* param);
	int		_i_srad_abdicate_alliance(const char* param);
	int		_i_srad_get_char_atb(const char* param);
	int		_i_srad_chat_private(const char* param);
	int		_i_srad_chat_alliance(const char* param);
	int		_i_srad_chat_world(const char* param);
	int		_i_srad_chat_group(const char* param);
	int		_i_srad_add_mail(const char* param);
	int		_i_srad_add_mail_with_ext_data(const char* param);
	int		_i_srad_get_mail(const char* param);
	int		_i_srad_read_mail(const char* param);
	int		_i_srad_delete_mail(const char* param);
	int		_i_srad_get_mail_1(const char* param);
	int		_i_srad_get_my_friend_apply(const char* param);
	int		_i_srad_get_others_friend_apply(const char* param);
	int		_i_srad_get_friend(const char* param);
	int		_i_srad_approve_friend(const char* param);
	int		_i_srad_apply_friend(const char* param);
	int		_i_srad_refuse_friend(const char* param);
	int		_i_srad_cancel_friend(const char* param);
	int		_i_srad_delete_friend(const char* param);
	int		_i_srad_get_world_area(const char* param);
	int		_i_srad_add_alliance_mail(const char* param);
	int		_i_srad_get_alliance_mail(const char* param);
	int		_i_srad_get_alliance_log(const char* param);
	int		_i_srad_get_private_log(const char* param);
	int		_i_srad_buy_item(const char* param);
	int		_i_srad_sell_item(const char* param);
	int		_i_srad_get_relation_log(const char* param);
	int		_i_srad_fetch_christmas_tree(const char* param);
	int		_i_srad_watering_christmas_tree(const char* param);
	int		_i_srad_steal_gold(const char* param);
	int		_i_srad_get_training_te(const char* param);
	int		_i_srad_add_training(const char* param);
	int		_i_srad_exit_training(const char* param);
	int		_i_srad_get_recent_chat(const char* param);
	int		_i_srad_contribute_alliance(const char* param);
	int		_i_srad_convert_diamond_gold(const char* param);
	int		_i_srad_join_alliance_name(const char* param);
	int		_i_srad_get_my_alliance_join(const char* param);
	int		_i_srad_get_alliance_card_by_id(const char* param);
	int		_i_srad_get_alliance_card_by_rank(const char* param);
	int		_i_srad_set_alliance_introduction(const char* param);
//	int		_i_srad_draw_lottery(const char* param);
	int		_i_srad_get_other_goldore(const char* param);
	int		_i_srad_check_draw_lottery(const char* param);
	int		_i_srad_fetch_lottery(const char* param);
	int		_i_srad_get_christmas_tree(const char* param);
	int		_i_srad_get_goldore_smp_info_all(const char* param);
	int		_i_srad_get_alliance_trade_info(const char* param);
	int		_i_srad_alliance_trade(const char* param);
	int		_i_srad_get_enemy_list(const char* param);
	int		_i_srad_delete_enemy(const char* param);
	int		_i_srad_get_rank_list(const char* param);
	int		_i_srad_get_my_rank(const char* param);
	int		_i_srad_write_signature(const char* param);
	int		_i_srad_get_simple_char_atb(const char* param);
	int		_i_srad_change_name(const char* param);
	int		_i_srad_move_position(const char* param);
	int		_i_srad_get_pay_serial(const char* param);
	int		_i_srad_read_notification(const char* param);
	int		_i_srad_change_hero_name(const char* param);
	int		_i_srad_read_mail_all(const char* param);
	int		_i_srad_delete_mail_all(const char* param);
	int		_i_srad_add_gm_mail(const char* param);
	int		_i_srad_set_alli_name(const char* param);
	int		_i_srad_get_famous_city_list(const char* param);
	int		_i_srad_fetch_mail_reward(const char* param);
	int		_i_srad_upload_bill(const char* param);
	int		_i_srad_register(const char* param);
	int		_i_srad_bind_device(const char* param);
	int		_i_srad_get_server_time(const char* param);
	int		_i_srad_dismiss_soldier(const char* param);
	int		_i_srad_get_world_res_card(const char* param);

	int		_i_srad_alli_donate_soldier(const char* param);
	int		_i_srad_alli_recall_soldier(const char* param);
	int		_i_srad_acce_donate_soldier(const char* param);
	int		_i_srad_get_donate_soldier_queue(const char* param);
	int		_i_srad_get_left_donate_soldier(const char* param);

	int		_i_srad_add_gold_deal(const char* param);
	int		_i_srad_buy_gold_in_market(const char* param);
	int		_i_srad_cancel_gold_deal(const char* param);
	int		_i_srad_get_gold_deal(const char* param);
	int		_i_srad_get_self_gold_deal(const char* param);

	int		_i_srad_tavern_refresh(const char* param);
	int		_i_srad_hire_hero(const char* param);
	int		_i_srad_fire_hero(const char* param);
	int		_i_srad_levelup_hero(const char* param);
	int		_i_srad_add_grow(const char* param);
	int		_i_srad_config_hero(const char* param);
	int		_i_srad_get_hero_refresh(const char* param);
	int		_i_srad_get_hero_hire(const char* param);
	int		_i_srad_start_combat(const char* param);
	int		_i_srad_stop_combat(const char* param);
	int		_i_srad_get_combat(const char* param);
	int		_i_srad_save_combat(const char* param);
	int		_i_srad_list_combat(const char* param);
	int		_i_srad_config_citydefense(const char* param);

	int		_i_srad_get_item(const char* param);
	int		_i_srad_add_item(const char* param);
	int		_i_srad_del_item(const char* param);
	int		_i_srad_equip_item(const char* param);
	int		_i_srad_disequip_item(const char* param);
	int		_i_srad_mount_item(const char* param);
	int		_i_srad_unmount_item(const char* param);
	int		_i_srad_compos_item(const char* param);
	int		_i_srad_discompos_item(const char* param);
	int		_i_srad_equip_item_all(const char* param);
	int		_i_srad_mount_item_all(const char* param);
	int		_i_srad_replace_equip(const char* param);

	int		_i_srad_get_quest(const char* param);
	int		_i_srad_done_quest(const char* param);

	int		_i_srad_get_instancestatus(const char* param);
	int		_i_srad_get_instancedesc(const char* param);
	int		_i_srad_create_instance(const char* param);
	int		_i_srad_join_instance(const char* param);
	int		_i_srad_quit_instance(const char* param);
	int		_i_srad_destroy_instance(const char* param);
	int		_i_srad_get_instancedata(const char* param);
	int		_i_srad_config_instancehero(const char* param);
	int		_i_srad_get_instanceloot(const char* param);
	int		_i_srad_auto_combat(const char* param);
	int		_i_srad_auto_supply(const char* param);
	int		_i_srad_manual_supply(const char* param);
	int		_i_srad_kick_instance(const char* param);
	int		_i_srad_start_instance(const char* param);
	int		_i_srad_prepare_instance(const char* param);
	int		_i_srad_supply_instance(const char* param);

	int		_i_srad_use_item(const char* param);

	int		_i_srad_sync_char(const char* param);
	int		_i_srad_sync_hero(const char* param);

	int		_i_srad_get_equip(const char* param);
	int		_i_srad_get_gem(const char* param);

	int		_i_srad_get_worldgoldmine(const char* param);
	int		_i_srad_config_worldgoldmine_hero(const char* param);
	int		_i_srad_rob_worldgoldmine(const char* param);
	int		_i_srad_drop_worldgoldmine(const char* param);
	int		_i_srad_my_worldgoldmine(const char* param);

	int		_i_srad_combat_prof(const char* param);
	int		_i_srad_get_citydefense(const char* param);

	int		_i_srad_use_drug(const char* param);

	int		_i_srad_army_accelerate(const char* param);
	int		_i_srad_army_back(const char* param);

	int		_i_srad_worldcity(const char* param);
	int		_i_srad_worldcity_log(const char* param);
	int		_i_srad_worldcity_log_alliance(const char* param);

	int		_i_srad_worldcity_rank_man(const char* param);
	int		_i_srad_worldcity_rank_alliance(const char* param);

	int		_i_srad_recharge_try(const char* param);

	int		_i_srad_alli_instance_create(const char* param);
	int		_i_srad_alli_instance_get_list(const char* param);
	int		_i_srad_alli_instance_join(const char* param);
	int		_i_srad_alli_instance_exit(const char* param);
	int		_i_srad_alli_instance_kick(const char* param);
	int		_i_srad_alli_instance_destroy(const char* param);
	int		_i_srad_alli_instance_config_hero(const char* param);
	int		_i_srad_alli_instance_start(const char* param);
	int		_i_srad_alli_instance_ready(const char* param);
	int		_i_srad_alli_instance_get_char_data(const char* param);
	int		_i_srad_alli_instance_get_hero_data(const char* param);
	int		_i_srad_alli_instance_get_status(const char* param);
	int		_i_srad_alli_instance_save_hero_deploy(const char* param);
	int		_i_srad_alli_instance_get_combat_log(const char* param);
	int		_i_srad_alli_instance_get_simple_combat_log(const char* param);
	int		_i_srad_alli_instance_stop_combat(const char* param);
	int		_i_srad_alli_instance_supply(const char* param);
	int		_i_srad_alli_instance_get_loot(const char* param);
	int		_i_srad_alli_instance_get_defender_deploy(const char* param);

	int		_i_srad_arena_upload_data(const char* param);
	int		_i_srad_arena_get_status(const char* param);
	int		_i_srad_arena_pay(const char* param);
	int		_i_srad_arena_get_rank_list(const char* param);
	int		_i_srad_arena_get_deploy(const char* param);

	int		_i_srad_get_kick_client_all(const char* param);

	int		_i_srad_position_mark_add(const char* param);
	int		_i_srad_position_mark_del(const char* param);
	int		_i_srad_position_mark_chg(const char* param);
	int		_i_srad_position_mark_get(const char* param);
};

#endif
