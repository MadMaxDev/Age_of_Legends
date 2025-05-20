#ifndef __tty_common_db_timeevent_H__
#define __tty_common_db_timeevent_H__

// time_event ����
enum
{
	te_type_building				= 1,		// ����ʱ���¼�
	te_type_once_perperson			= 2,		// ÿ��˲��ÿ����ֻ����һ�ε�ʱ���¼�(����Ը�����˿�����)
	te_type_production				= 3,		// �����¼���Ŀǰֻ�н����������Ҫ��¼����
	te_type_research				= 4,		// �о�
	te_type_alliance_building		= 5,		// ���˽���
	te_type_hero_training			= 6,		// ����ѵ��
	te_type_alliance_trade			= 7,		// ��������
	te_type_alliance_donate_soldier	= 8,		// �����ͱ�
	te_type_alli_instance_march_advance	= 9,	// ���˸���ǰ��ս��
	te_type_alli_instance_march_back	= 10,	// ���˸���ս������

	te_type_combat_go				= 101,		// ս��ǰ��
	te_type_combat_back				= 102,		// ս������

	te_type_worldgoldmine_gain		= 105,		// �������ջ�

	te_type_fixhour					= 110,		// ��ʱ�¼����̶�Сʱ��

	te_type_worldcityrecover		= 120,		// �������ǻָ�
};

// te_type_building,����ʱ���¼�������
enum
{
	te_subtype_building_build		= 1,		// ����
	te_subtype_building_upgrade		= 2,		// ����
};

// te_type_once_perperson,һ��һ��ʱ���¼�������
enum
{
	te_subtype_once_christmas_tree	= 1,		// ʥ���������¼�
	te_subtype_once_population		= 2,		// �˿������¼�
	te_subtype_once_build_num		= 3,		// ���ӽ��������¼�
};

// te_type_production,�����¼�����
enum
{
	te_subtype_production_gold				= 1,		// �ƽ�����
	te_subtype_production_christmas_tree	= 2,		// ʥ��������
	te_subtype_production_gold_protection	= 3,		// �ƽ𱣻�ʱ��
};

// te_type_alliance_build,���˽����¼�
enum
{
	te_subtype_alliance_building_build		= 1,		// ����
	te_subtype_alliance_building_upgrade	= 2,		// ����
};

// te_type_alliance_donate_soldier
enum
{
	te_subtype_ads_send						= 1,		// ��ǲ
	te_subtype_ads_recall					= 2,		// �ٻ�
};

// relation�¼�
enum
{
	relation_event_watering_christmas_tree		= 1,		// ��ˮ
	relation_event_steal_gold					= 2,		// ͵�ƽ�,data_0->auto_id,data_1->production
};

// relation_log_type
enum
{
	relation_log_watering_christmas_tree		= 1,		// ��ˮ
	relation_log_steal_gold						= 2,		// ͵�ƽ�
};

// wait_deal_event_soon,��Ҫ����������¼�
enum
{
	wdes_type_gen_world_famous_city				= 1,		// ������������
};

// lock����
enum
{
	lock_deal_adcolony							= 1,		// ����adcolony_transaction
	lock_deal_excel_time_event					= 2,		// ����excelʱ���¼�
	lock_get_te									= 3,		// ��ȡʱ���¼���lock
	lock_deal_gold_market_te					= 4,		// �������г�
};

// instance_status
enum
{
	instance_status_normal						= 0,		// ��״̬(��δ��ʼ)
	instance_status_start						= 1,		// �����Ѿ���ʼ
	instance_status_fight						= 2,		// ս��
};
// instance_player_status
enum
{
	instance_player_status_normal				= 0,		// ��״̬
	instance_player_status_ready				= 1,		// ׼������
	instance_player_status_start				= 2,		// �����Ѿ���ʼ
};
// march_type,�о�����
enum
{
	march_type_advance							= 1,		// ǰ��
	march_type_return							= 2,		// ����
	march_type_recall							= 3,		// �ٻ�
};
// combat_type,ս������
enum
{
	combat_type_alli_instance					= 102,		// ���˸���
	combat_type_arena							= 200,		// ��������ս
};
enum
{
	combat_result_none							= 0,		// ��
	combat_result_win							= 1,		// ʤ��
	combat_result_lose							= 2,		// ʧ��
};
#endif
