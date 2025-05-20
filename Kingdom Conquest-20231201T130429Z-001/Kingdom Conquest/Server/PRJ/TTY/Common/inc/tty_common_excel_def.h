#ifndef __tty_common_excel_def_H__
#define __tty_common_excel_def_H__

namespace n_pngs
{
enum
{
	money_type_none				= 0,		// ��
	money_type_diamond			= 1,		// ��ʯ
	money_type_crystal			= 2,		// ˮ��
	money_type_gold				= 3,		// �ƽ�
	money_type_alliance_contribute				= 4,		// ���˹��׶�Ҳ������Ϊһ��ͨ�û���
};
enum
{
	money_use_type_none							= 0,
	money_use_type_buy_item						= 1,		// �̵깺�����
	money_use_type_accelerate_build				= 2,		// ���ٽ���
	money_use_type_accelerate_goldore			= 3,		// �����������
	money_use_type_accelerate_research			= 4,		// �����о�
	money_use_type_accelerate_march				= 5,		// �о�����
	money_use_type_lottery						= 6,		// �齱
	money_use_type_change_char_name				= 7,		// ��������
	money_use_type_change_hero_name				= 8,		// Ӣ�۸���
	money_use_type_change_alliance_name			= 9,		// ���˸���
	money_use_type_store_buy_gold				= 10,		// ����ƽ�
	money_use_type_arena_pay					= 11,		// ��������ս
	money_use_type_gold_market					= 12,		// �ƽ��г�
};
// �ƽ��������
enum
{
	gold_produce_none							= 0,
	gold_produce_goldore						= 1,		// �������
	gold_produce_raid							= 2,		// ���֮���Ӷ�
	gold_produce_steal							= 3,		// ���͵ȡ
	gold_produce_task							= 4,		// ����
	gold_produce_store_buy						= 5,		// �̵깺��
	gold_produce_sell_item						= 6,		// ��������
	gold_produce_lottery						= 7,		// �齱
	gold_produce_gold_market					= 8,		// ����г�
};
// �ƽ�����
enum
{
	gold_cost_none								= 0,
	gold_cost_conscript_soldier					= 1,		// �б�
	gold_cost_upgrade_soldier					= 2,		// ������
	gold_cost_building							= 3,		// ����
	gold_cost_research							= 4,		// �Ƽ��о�
	gold_cost_be_raided							= 5,		// ���Ӷ�
	gold_cost_hero_train						= 6,		// ����ѵ��
	gold_cost_gold_market						= 7,		// ����г�
};
// ���߸���
enum
{
	item_track_none								= 0,		
	item_track_sell								= 1,		// �۳�
};
// table_id
enum
{
	table_id_text								= 5,		// �ı�
	table_id_soldier							= 15,		// ʿ����
	tabel_id_alliance_position_right			= 21,		// ����ְλȨ��
	table_id_lottery_probability				= 30,		// �齱���ʱ�
	table_id_lottery_content					= 31,		// �齱���ݱ�
	table_id_lottery_content_alliance			= 34,		// ���˳齱���ݱ�
	table_id_player_birthplace					= 37,		// ��ҳ��������ñ�
	table_id_world_famous_city					= 42,		// ��������
	table_id_world_resource						= 217,		// ������Դ
};
// ˽�˹�ϵ,pr->personal relation
enum
{
	pr_type_friend				= 1,		// ����
	pr_type_enemy				= 2,		// ���
};

enum
{
	alliance_position_none			= 0,	// ���Ǹ����˵�
	alliance_position_leader		= 1,	// ����
	alliance_position_vice_leader	= 2,	// ������
	alliance_position_manager		= 3,	// ����
	alliance_position_member		= 4,	// ��Ա
};

enum
{
	apr_col_id						= 0,	// id
	apr_col_name					= 1,	// ְλ����
};

// �õ����ı�id,excel_text
enum
{
	// pl->private_log
	text_id_pl_expel_alliance	= 5,		// ������������
	text_id_pl_exit_alliance	= 7,		// �˳�����
	text_id_pl_set_alliance_pos = 9,		// ����ְλ 

	// al->alliance_log
	text_id_al_expel_alliance	= 6,		// ����������
	text_id_al_exit_alliance	= 8,		// �˳�����
	text_id_al_set_alliance_pos	= 10,		// ����ְλ

	// rl->relation_log
	text_id_rl_watering_christmas_tree	= 11,	// ��Ը����ˮ
	text_id_rl_steal_gold				= 12,	// ͵�Իƽ�
	
	text_id_in_alliance			= 13,		// ��׼��������
	text_id_in_alliance_new		= 14,		// �³�Ա��������

	text_id_apply_alliance_refused	= 15,	// �������뱻�ܾ�
	text_id_abdicate_alliance		= 16,	// ��������
	text_id_dismiss_alliance		= 17,	// ��ɢ����
	text_id_set_alliance_position	= 18,	// ��������ְλ

	text_id_alliance_trade_end		= 19,	// �������̽���
	text_id_alliance_contribute		= 20,	// ���˾���

	text_id_lottery_item			= 21,	// �齱���е���
	text_id_lottery_crystal			= 22,
	text_id_lottery_gold			= 23,

	text_id_attack_alliance			= 465,	// �������˳�Ա
};

enum
{
	excel_text_max_length		= 512,		// excel_text��һ���ı���󳤶�
	excel_alliance_position_name_length	= 32,	// ����ְλ���ֳ���
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

// index_list���е�ö��ֵ
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
// index_list��ʹ������ö��ֵ
enum
{
	use_type_all				= 0,		// �ͻ��˷�����������
	use_type_server				= 1,		// ����������
	use_type_client				= 2,		// �ͻ��˼���
	use_type_none				= 3,		// ��������
};
// lp->lottery_probability
enum
{
	lp_col_choice				= 0,		// ����ѡ��
	lp_col_probability			= 1,		// ����
};
// lc->lottery_content
enum
{
	lc_col_type					= 0,		// ��������
	lc_col_data					= 1,		// ��������
	lc_col_probability_choice	= 2,		// ��������ѡ��
	lc_col_is_top_reward		= 3,		// �Ƿ���Ҫ����Щ������¼��־
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
	pbp_col_playernum			= 0,		// ����������ʼ
	pbp_col_begin_x				= 1,		// ��ʼλ��x
	pbp_col_begin_y				= 2,		// ��ʼλ��y
	pbp_col_end_x				= 3,		// ����λ��x
	pbp_col_end_y				= 4,		// ����λ��y
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
	wr_col_army_data			= 8,		// ���ӵȼ�*����
	wr_col_gold_loot			= 9,		// ����ƽ�
	wr_col_pop_loot				= 10,		// �����˿�
	wr_col_crystal_loot			= 11,		// ����ˮ��
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
	excel_te_id_refresh_char_day_te			= 1,		// ˢ�½�ɫÿ������
	excel_te_id_refresh_world_famous_city	= 3,		// ˢ����������
	excel_te_id_cup_activity_end			= 4,		// ���������(����,���¼�����3)
	excel_te_id_refresh_world_res			= 5,		// ˢ��������Դ
	excel_te_id_arena_reward				= 6,		// ��������������
};
}

#endif
