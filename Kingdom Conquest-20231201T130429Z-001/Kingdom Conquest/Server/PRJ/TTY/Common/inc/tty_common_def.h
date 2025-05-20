// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : tty_common_def.h
// Creator      : Wei Hua (κ��)
// Comment      : ����PNGS��tty��ʵ�ֵĹ�������
//                PNGS��Pixel Network Game Structure����д
//                ��Ϊ�ͻ��˺ͷ�������ͨѶ��Ҫ��Լ��������ָ�����ݲ��ýṹ�������ķ�ʽ�����Խ��б�Ҫ��ѹ��
//                Ŀǰ�Ѿ������ָ��Σ�
//                �ͻ��˵ģ�
//                1300~1399  	Chatģ���յ���ָ��
//                1400~1499 	Chatģ�鷢����ClientGamePlayģ���ָ��
//                1500~1599		ASSISTģ���յ���ָ��
//                1600~1699		ASSISTģ�鷢����������(�ͻ��˵�gameplayer)��ָ��
//                ����˵ģ�
//                1100~1199		DBģ���յ���ָ��
//                1100~1199		DBģ�鷢���������ߵ�ָ��
//                1300~1399		GSGCģ���յ���ָ��
//                1300~1399		GCģ�鷢���������ߵ�ָ��
//                1500~1599		PointStockģ���յ���ָ��
//                1500~1599		PointStockģ�鷢���������ߵ�ָ��
//                1600~1699		LISģ���յ���ָ��
//                1600~1699		LISģ�鷢���������ߵ�ָ��
//                1700~1799		QPointģ���յ���ָ��
//                1700~1799		QPointģ�鷢���������ߵ�ָ��
//                1800~1899		GMLS�յ���ָ��
//                1900~1999		LBAAUTH�յ���ָ��
//                2000~2999		У����ģ�鷢�����յ���ָ�TTYVerifyCodeClient����ָ��
//                3000~3099		�����ģ�鷢�����յ���ָ�TTYAntiHacker����ָ��
//				  3200~3299		BBS�����ϵͳ�յ��ͷ�����ָ��
//                3300~3499     webϵͳ�յ��ͷ��͵�ָ��
//                3500~3799    QQTipsģ���յ��ͷ��͵�ָ��
// CreationDate : 2005-09-15 �� tty_common_packet.h�з������
// Change LOG   : 2006-03-07 ��ԭ���߼����н�����ʹ�unsigned char��Ϊtty_rst_t��
//              : 2006-05-22 ����TTY_STARTGAME_RST_SUCCEED_GZS2GZS
//              : 2008-02-26 ��������Ķ���ɽ�ɫ�����ȸ�Ϊ32+1
//              : 2008-03-05 ����������LOGIN�Ĵ�����ʾ�����������û���Ӧ�ķ����������������������û����ϳ���������

#ifndef	__TTY_COMMON_DEF_H__
#define	__TTY_COMMON_DEF_H__

#include	<WHCMN/inc/whcmn_def.h>
#include	<PNGS/inc/pngs_def.h>

namespace n_pngs
{

// TTY�߼�ָ������(�����߼�ģ��֮��ͨѶ��ָ��)
typedef	pngs_cmd_t				tty_cmd_t;							// �߼�ָ���������pngs_cmd_t����һ�£�����PNGSClient��GMSMainStructure�ڲ�Ԥ�Ƚ��е�ָ��ּ�ͻ������⣩
typedef	int				tty_rst_t;							// �߼����
typedef	n_whcmn::whint64		tty_id_t;							// �˺źͽ�ɫ��ID
typedef	unsigned short			tty_svrgrpid_t;						// ��������ID����

//////////////////////////////////////////////////////////////////////////
// SP����ֵ
#define		SP_RESULT			"@result"
#define		QUERY_LEN			81920
#define		SVRGRP_DBS_IDX		30

// ���ںϳ�ID
union	TTY_ID_UNION_T
{
	tty_id_t					id;
	struct
	{
		typedef	n_whcmn::whuint64	_T;								// ��uint��Ϊ�˷����ӡ
		_T		incidx			: 19;								// ������ID����
		_T		t				: 30;								// ʱ������
		_T		svrid			: 5;								// �����еĵ���������
		_T		grpid			: 9;								// �����еĵ���������
		_T		nouse			: 1;								// ��ʹ�á���ΪMySQL��ֻ����63bit��������������һλû����
	}u;
	inline void	clear()
	{
		id	= 0;
	}
};

// ��������
enum
{
	TTY_WHUSERNAME_LEN							= 32,				// ����WHУ����û�����󳤶�(��0��β)
	TTY_WHPASSWORD_LEN							= 32,				// ����WHУ���������󳤶�(��0��β)
	TTY_ACCOUNTNAME_LONG_LEN					= 64,				// �˺�������󳤳��ȣ�ֻ�����û���¼����ʱ�ĳ��ȣ�
	TTY_ACCOUNTNAME_LEN							= 32,				// �˺�������󳤶�
	TTY_ACCOUNTPASSWORD_LEN						= 256,				// �˺��������󳤶ȣ�2009-04-28 �޸ģ�����Ҫ��ܳ���
	TTY_PASSWORD_LEN							= 32+1,				// �˺�����ĳ���(���ڶ���MD5,�������1������0)
	TTY_CHARACTERNAME_LEN						= 32,				// ��ɫ������󳤶�
	TTY_POSITION_MARK_PROMPT_LEN				= 256,				// λ���ղؼ�ע�ͳ���
	TTY_CHARACTERPASSWORD_LEN					= 32+1,				// ��ɫ�������󳤶�
	TTY_CHARACTER_SIGNATURE_LEN					= 128+1,			// ����ǩ������ 
	TTY_PAY_SERIAL_LEN							= 36,				// �������кų���
	TTY_ALLIANCENAME_LEN						= 32,				// ��������
	TTY_ITEM_LEN								= 32,				// ��������
	TTY_DEVICE_ID_LEN							= 64,				// �豸ID����
	TTY_MAC_LEN									= 64,				// mac��ַ����
	TTY_MAX_DEPLOY_HERO_NUM						= 25,				// ���һ�β���Ӣ������
	TTY_MAIL_TITLE_LEN							= 64,				// �ʼ����ⳤ��
	TTY_MAIL_TEXT_LEN							= 1024,				// �ʼ����ݳ���
	TTY_MAIL_EXT_DATA_LEN						= 1024*6,			// �ʼ��������ݴ�С
	TTY_ALLIANCE_MAIL_TEXT_LEN					= 1024,				// �����ʼ�����
	TTY_ALLIANCE_LOG_TEXT_LEN					= 256,				// ������־����
	TTY_PRIVATE_LOG_TEXT_LEN					= 256,				// ˽����־����
	TTY_RELATION_LOG_TEXT_LEN					= 256,				// ��ϵ��־����
	TTY_BUY_ITEM_MAX_NUM						= 1000,				// һ������������
	TTY_OTHERNAME_LEN							= 32*3+1,			// �����������Ƶ���󳤶ȣ���itemname��2009-05-19��Ϊ97��
	TTY_CHARACTERNUM_PER_ACCOUNT				= 6,				// ÿ���˺����Ľ�ɫ����(������ɾ����ɫ)
	TTY_LOGIC_DFTCHARACTERNAME_MAX				= 16,				// �߼���Ĭ�Ͻ�ɫ������󳤶�
	TTY_LOGIC_DFTCHARACTERNAME_MIN				= 4,				// �߼���Ĭ�Ͻ�ɫ������С����
	TTY_MAXCARDIDLEN							= 32+1,				// ��Ϸ�������ֵ�����������齱�Ŀ���
	TTY_MAXCARDPASSLEN							= 32+1,				// ��Ϸ�������������ֵ
	TTY_WHTESTSTR_MAX_LEN						= 128*1024,			// �ͻ��˺ͷ����������ָ�����󳤶�
	TTY_CHATKEY_MAX_LEN							= 256,				// ���������������KEY����󳤶�(����֮���)
	TTY_CHATROOMNAME_LEN						= 64,				// ���������ֵĳ���
	TTY_ORGNAME_LEN								= 32,				// ��֯�����塢���ᣩ������󳤶�
	TTY_SNWRITE_LEN								= 512,				// ���ǩ��������
	TTY_INTRODUCTION_LEN						= 512,				// ���ɼ��ĳ���
	// �̵����õİ汾��������󳤶�(���������һ��0��β�������ڿ�����ʱ����memcpy)
	TTY_STORECONFIG_VERSSTR_LEN					= 10,
	// �㿨���׶Կ���������
	TTY_POINT_FIGHTTABLE_WIDTH					= 10,
	// GZS���
	TTY_MAXGZSPLAYERNUM							= 2000,				// Ĭ�ϵ�ÿ��GZS���������
	TTY_GZSNAME_LEN								= 32,				// GZS������
	// һ��Ԥ����ָ���Ĵ�С
	TTY_RAWBUF_SIZE								= 512*1024,
	// ������Ӫ����
	TTY_MAX_CAMPSIDES							= 4,
	TTY_CAMPSIDE_0								= 0x01,
	TTY_CAMPSIDE_1								= 0x02,
	TTY_CAMPSIDE_2								= 0x04,
	TTY_CAMPSIDE_3								= 0x08,
	// ��ߵļ���
	TTY_MAX_LEVEL								= 200,
	// ��ߵĽ�ɫְҵID
	TTY_MAX_PRO									= 255,
	// ��ߵĽ�ɫ����ID
	TTY_MAX_MODEL								= 255,
	// ��ߵĽ�ɫְҵID
	TTY_MAX_GG_NUM								= 512,
	// һ���transaction��ʱʱ��
	TTY_TRANSACTION_TIMEOUT						= 44*1000,			// 20��ɣ�һ���û�����û����ô���ģ�
	// Ѱ�߹����ʱ��
	TTY_TIME_20081027							= 1225036800,
	// billingͨѶ�еĸ����ִ��ĳ���
	TTY_BILLING_EXT_INFO_LEN					= 256,
	// GZS��GMS�������������Ϣʱ,һҳ������			
	TTY_MAX_GET_PLAYER_SVRID_MAX				= 10,
	// ���ٵ�¼��̬key�ĳ���
	QUICK_LOGIN_KEY_LENGTH						= 32,
};
// �˺�����
// 0x000000XX									GMȨ��
// 0x0X000000									��Ҫ���������ݿ��е��û�����
// 0xX0000000									����������Ϣ���ɵ��û�����
enum
{
	TTY_ACCOUNTPROPERTY_ROOT					= 0x00000001,		// ϵͳ����Ա
	TTY_ACCOUNTPROPERTY_GM						= 0x00000002,		// GM
	TTY_ACCOUNTPROPERTY_ADM						= 0x00000004,		// ��GM��һЩ
	TTY_ACCOUNTPROPERTY_ALLGMMASK				= 0x00000007,		// �����ж��Ƿ���GM���ԡ����ķ����������������GM���ԡ�

	TTY_ACCOUNTPROPERTY_USER_LOCKED				= 0x01000000,		// �ʺű��û������������ֻ�������
	TTY_ACCOUNTPROPERTY_LOCKED					= 0x02000000,		// �ʺű�����
	TTY_ACCOUNTPROPERTY_USER_LOCKED_COMMENT		= 0x04000000,		// �û�����ע�ǣ�̨����������ʹ�õģ�
	TTY_ACCOUNTPROPERTY_IP_ESTOP				= 0x80000000,		// �ʺű�IP���ԣ�������ô������ݿ��У���ҪGMS�����û�״����д��
};
// �˺�����ģʽ
enum
{
	TTY_PASSMODE_MD5							= 0,				// ����MD5����˵���������16�ֽڵ�MD5
	TTY_PASSMODE_PLAIN							= 1,				// ���룻��˵������������clear/plain text(��0��β��)
};
// LOGIN���
enum
{
	TTY_LOGIN_RST_SUCCEED						= 0x01,				// login�ɹ�
	TTY_LOGIN_RST_BADACCOUNT					= 0x02,				// �û�������
	TTY_LOGIN_RST_BADPASS						= 0x03,				// �������
	TTY_LOGIN_RST_ALREADYINTHIS					= 0x04,				// �û����ڱ�������������
	TTY_LOGIN_RST_ALREADYINOTHER				= 0x05,				// �û���������������������
																	// ������������������ID(tty_svrgrpid_t����)
	TTY_LOGIN_RST_OTHEREER						= 0x06,				// billing�ڲ����������󣬿��������ݿ��������⣨�������ڲ��Ĵ洢���̴��󣩣�Ҳ�����Ƿ�������ͨѶ���߷������ڲ�����
	TTY_LOGIN_RST_REFUSED						= 0x07,				// ���ܾ�
	TTY_LOGIN_RST_LOGININPROCESS				= 0x09,				// ����ͬ���ֵ��ʺ����ڵ�¼�����У�2007-06-15�ӣ�
	
	TTY_LOGIN_RST_NO_DEVICEID					= 0x0A,				// û�и�DeviceID��Ӧ���˺�
	TTY_LOGIN_RST_LOGININPROCESS_DEVICEID		= 0x0B,				// ��ʹ�ø�DeviceID���˺����ڵ�½������
	//
	TTY_LOGIN_RST_NOCHAR						= 0x0C,				// ��ɫ������
	TTY_LOGIN_RST_LOADCHAR						= 0x0E,				// �����ɫ��Ϣʱ����

	TTY_LOGIN_RST_ACCOUNT_BANNED				= 0x0F,				// �˺ű���
	TTY_LOGIN_RST_DEVICE_BANNED					= 0x10,				// �豸����
	TTY_LOGIN_RST_CHAR_BANNED					= 0x11,				// ��ɫ����
	TTY_LOGIN_RST_GROUP_ERR						= 0x12,				// ��������
	// ���Լ��ӵ� 0xF0~0xFF
	TTY_LOGIN_RST_DBERR							= 0xF1,				// DB�޷�����ʹ��
	TTY_LOGIN_RST_TRANSACTION_TIMEOUT			= 0xF6,				// ��transaction��ĳһ����ʱ��
	TTY_LOGIN_RST_OUTOF_TRANSACTION				= 0xF7,				// �޷�����transaction��
	TTY_LOGIN_RST_SQLERR						= 0xFE,				// SQL����
	TTY_LOGIN_RST_UNKNOWNERR					= 0xFF,				// ����δ֪����Ҳ��������û��ɾ�����û�̫�࣬�޷������ˣ��������ڴ棬�����Զ����ݶ�����д�����
};
// �����˺Ž��
enum
{
	TTY_ACCOUNT_CREATE_RST_OK					= 0x00,				// �����ɹ�
	TTY_ACCOUNT_CREATE_RST_GS_CNTR_ERR			= 0x01,				// ��ȫ�ַ������ӳ���
	TTY_ACCOUNT_CREATE_RST_DB_ERR				= 0x02,				// ���ݿ����(������)
	TTY_ACCOUNT_CREATE_RST_SQL_ERR				= 0x03,				// SQLִ�г���
	TTY_ACCOUNT_CREATE_RST_TIMEOUT				= 0x04,				// ��ʱ
	TTY_ACCOUNT_CREATE_RST_NAME_DUP				= 0x05,				// �˺��Ѿ�����
	TTY_ACCOUNT_CREATE_RST_NAME_INVALID			= 0x06,				// �����зǷ��ַ�
	TTY_ACCOUNT_CREATE_RST_DEVICE_BANNED		= 0x07,				// �豸����
	TTY_ACCOUNT_CREATE_RST_DEVICE_BINDED_GROUP	= 0x08,				// �豸�Ѿ��ڸô����󶨹���
};
// ������ɫ���
enum
{
	TTY_CHAR_CREATE_RST_OK					= 0x00,				// �����ɹ�
	TTY_CHAR_CREATE_RST_NAMEDUP				= 0x01,				// �����ظ�
	TTY_CHAR_CREATE_RST_DB_ERR				= 0x02,				// ���ݿ����(������)
	TTY_CHAR_CREATE_RST_SQL_ERR				= 0x03,				// SQLִ�г���
	TTY_CHAR_CREATE_RST_TIMEOUT				= 0x04,				// ��ʱ
	TTY_CHAR_CREATE_RST_ALREADYEXISTCHAR	= 0x05,				// �Ѿ��ڱ�������������ɫ��
	TTY_CHAR_CREATE_RST_WORLD_FULL			= 0x06,				// ��������
	TTY_CHAR_CREATE_RST_POSITION_DUP		= 0x07,				// λ���ظ�
	TTY_CHAR_CREATE_RST_NAME_INVALID		= 0x08,				// �����зǷ��ַ�
};
// LOGIN���
enum
{
	TTY_PRELOGIN_RST_SUCCEED					= 0x01,				// prelogin�ɹ�
	TTY_PRELOGIN_RST_BADPASS					= 0x03,				// �������
	TTY_PRELOGIN_RST_DBERR						= 0xF1,				// DB�޷�����ʹ��
	TTY_PRELOGIN_RST_LBAERR						= 0xF2,				// LBA�޷�����ʹ��
	TTY_PRELOGIN_RST_GCERR						= 0xF3,				// GCģ���޷�����ʹ��
	TTY_PRELOGIN_RST_INTERSVR_DATA_ERR			= 0xF4,				// ��������������ݳ���
};
enum
{
	TTY_ENDGAME_RST_SUCCEED						= 0x01,				// �ɹ�
	TTY_ENDGAME_RST_FAIL						= 0x00,				// ������Ϸʧ��(�����LB�����ж�)
};
enum
{
	TTY_GZS_SELECT_RST_SUCCEED					= 0x01,				// �ɹ�
	TTY_GZS_SELECT_RST_FAIL						= 0x00,				// ʧ��(���磺ѡ��GZS�����˲�������)
};
enum
{
	TTY_REQ_FILE_RST_SUCCEED					= 0x01,				// �ļ���ȡ�ɹ�������Ӧ�ø����ļ����ݣ����û���ļ����ݣ�
	TTY_REQ_FILE_RST_SUCCEED_SAME				= 0x00,				// �ļ�MD5û�иı䣨�ͻ���Ӧ��ֱ��ʹ�ñ����ļ����ɣ�
	TTY_REQ_FILE_RST_ERR_NOTFOUND				= 0xF0,				// �ļ�������
};
enum
{
	TTY_STORE_BUY_RST_SUCCEED					= 0x01,				// ����ɹ�
	TTY_STORE_BUY_RST_ERR_MONEY_NOT_ENOUGH		= 0x00,				// ��������
	TTY_STORE_BUY_RST_ERR_ITEM_NOT_EXIST		= 0xF0,				// ����Ʒ������
	TTY_STORE_BUY_RST_ERR_REFUSED				= 0xF1,				// �������ܾ�����������һ���ͽ�����صĲ�����û�н���
	TTY_STORE_BUY_RST_ERR_ADDMSG				= 0xF2,				// �����Ʒ��Ϣʱ�������Ӧ�����û�����Ӫ����ϵһ���ˣ���Ϊ���Ѿ��۹��ˣ�
	TTY_STORE_BUY_RST_ERR_RETURNMONEY			= 0xF3,				// �˻�����ʱ����Ҫ�������û���Ǯ��������:(��
	TTY_STORE_BUY_RST_ERR_CLOSE					= 0xF4,				// ���ܱ��ر�
	TTY_STORE_BUY_RST_ERR_GIFT_ITEM_NOT_EXIST	= 0xF5,				// ��Ʒ������
	TTY_STORE_BUY_RST_ERR_UNKNOWN				= 0xFF,				// δ֪����

	TTY_RANKLIST_MAX							= 1000,				// ���а�����ʵID���ܳ��������1��96Ϊatb��96��499Ϊ�����500~699Ϊ�������а
};
enum
{
	TTY_LOTTERY_RST_SUCCEED						= 0x01,				// �ҽ��ɹ�����ע����ս�Ʒ��Ϣ��
	TTY_LOTTERY_RST_NOLUCKY						= 0x02,				// �������ѣ�û�жҵ���
	TTY_LOTTERY_RST_USED						= 0x03,				// �����Ѿ���ʹ�ù���
	TTY_LOTTERY_RST_NOTEXIST					= 0x04,				// ����Ŀ���
	TTY_LOTTERY_RST_ACCOUNT_LIMIT				= 0x05,				// �ʺű�����ֻ����ȡN��
	TTY_LOTTERY_RST_CHAR_LIMIT					= 0x06,				// ��ɫ������ֻ����ȡN��
	TTY_LOTTERY_RST_ERR_NET						= 0xFE,				// �������
	TTY_LOTTERY_RST_ERR_UNKNOWN					= 0xFF,				// δ֪����
};
// һЩ���õ�atb��ŵĶ���
enum
{
	TTY_ATB_LEVEL								= 1,				// ����
	TTY_ATB_SCN									= 22,				// ���߳���
	TTY_ATB_CAREER								= 42,				// ְҵ
	TTY_ATB_MONEY								= 35,				// ��Ǯ
	TTY_ATB_EXP									= 41,				// ��ǰ����
	TTY_ATB_MODEL								= 42,				// ��ɫϵ
	TTY_ATB_LEAVE_TIME							= 47,				// �ϴ��뿪ʱ��
	TTY_ATB_SIDE								= 58,				// ��ɫ��Ӫ
	TTY_ATB_SVRIDX								= 59,				// ��ɫ�󶨷��������
	TTY_ATB_LASTSVRIDX							= 60,				// �ϴ����ߵķ��������
};
enum
{
	TTY_GLB_ATB_Login_Count						= 1000,				// �û���¼�ļ���
	TTY_GLB_ATB_BindCount_GZS0					= 1000,				// ��1001~1030����¼ÿ���������İ��û���������ֻ���������ͳ�Ƶģ�����û���ת�����ֵ������һ�������仯��
	TTY_GLB_ATB_BindCount_GZS30					= 1030,				//
};
// ��֯���Ͷ���
enum
{
	TTY_ORG_TYPE_FAMILY							= 1,				// ����
	TTY_ORG_TYPE_GUILD							= 2,				// ����
};
////////////////////////////////////////////////////////////////////
// �������Ϳͻ�����Ҫͬ�����ļ����
////////////////////////////////////////////////////////////////////
enum
{
	TTY_CS_FILESYNC_IDX_STORE					= 1,				// �����̳��ļ�
	TTY_CS_FILESYNC_IDX_RANKLIST_0				= 1000,				// ���а��ļ�����id
	TTY_CS_FILESYNC_IDX_RANKLIST_POINT_0		= 1400,				// �㿨�������а��ļ�����id��1:day	2:week	3:month	4:total��
	TTY_CS_FILESYNC_IDX_RANKLIST_POINT_END		= 1410,				// ����
	TTY_CS_FILESYNC_IDX_RANKLIST_FAMILY_0		= 1500,				// �������а��ļ�����id
	TTY_CS_FILESYNC_IDX_RANKLIST_FAMILY_END		= 1599,				// �������а��ļ�����id
	TTY_CS_FILESYNC_IDX_RANKLIST_GUILD_0		= 1600,				// �������а��ļ�����id
	TTY_CS_FILESYNC_IDX_RANKLIST_GUILD_END		= 1699,				// �������а��ļ�����id
	TTY_CS_FILESYNC_IDX_KGRAPH_0				= 1000000,			// K��ͼ�ļ�����id
	TTY_CS_FILESYNC_IDX_KGRAPH_END				= 1999999,			// K��ͼ�ļ�����id
																	// ע��K��ͼ�ļ����ĽṹΪ��1GGGWWW������GGG������λ�Ĵ���ID��WWW������λ������ţ��ӹ��⿪ʼ��20081027��Ϊ��0�ܣ���
																	// ���˵���ͼ��2���ӿ��������ύһ�����󣩣��������е�ͼ������ھͲ�������LBA�ύ����
};

// ʱ���ö�Ӧ�����ڣ�2008-10-27 00:00:00��1225036800����Ϊ��ʼ�㣩
inline int	TTYGetWeekByTime(time_t t)
{
	return	(t-TTY_TIME_20081027)/(3600*24*7);
}
// ����Week��ȡ��ʼ�ͽ���ʱ��
inline void	TTYGetWeekT0T1(int nWeek, time_t &nT0, time_t &nT1)
{
	nT0	= TTY_TIME_20081027 + nWeek*7*24*3600;
	nT1	= nT0 + 7*24*3600;
}
// ��KGRAPH�ļ�ID�л�ȡ��Ӧ�����ںʹ�����
inline void	TTYGetGroupIDAndWeekFromKGRAPHID(int nID, int *pnGroupID, int *pnWeek)
{
	*pnGroupID	= (nID - TTY_CS_FILESYNC_IDX_KGRAPH_0) / 1000;
	*pnWeek		= nID % 1000;
}
// ͨ�������ź���������KGRAPH�ļ�ID
inline int	TTYMakeKGRAPHID(int nGroupID, int nWeek)
{
	return	TTY_CS_FILESYNC_IDX_KGRAPH_0 + nGroupID*1000 + nWeek;
}

// ��Ѷ���û���ݶ���λ
enum
{
	TX_ID_CLUB		= 0x00,		//��Ա���
	TX_ID_163S		= 0x01,		//163�̶��ʺ��û�(20040920��Ϊ20Ԫ6λ���ű�־)
	TX_ID_163C		= 0x02,		//163�������û�
	TX_ID_263U		= 0x03,		//263�����û�
	TX_ID_MCLB		= 0x04,		//�ƶ���Ա
	TX_ID_TCNT		= 0x05,		//��ѶԱ��
	TX_ID_168U		= 0x06,		//168�����û�
	TX_ID_QQXP		= 0x07,		//��ͨQQ���û�
	TX_ID_QQXM		= 0x08,		//�ֻ�QQ���û�
	TX_ID_QLVP		= 0x09,		//Q���Ѹ߼��û���Ԥ���ѣ�
	TX_ID_QLVM		= 0x0A,		//Q���Ѹ߼��û����ֻ���
	TX_ID_SMSE		= 0x0B,		//�������û�
	TX_ID_ECARD		= 0x0C,		//QQ�ؿ��û�
	TX_ID_GAME		= 0x0D,		//GAME��Ա
	TX_ID_QQHOME	= 0x0E,		//QQHOME��Ա
	TX_ID_QQSHOW	= 0x0F,		//QQSHOW�����û�
	TX_ID_QQTANG	= 0x10,		//QQTANG�û�
	TX_ID_VNETCLB	= 0x11,		//Vnet��Ա�û�
	TX_ID_OMUSIC	= 0x12,		//��������
	TX_ID_MUSIC		= 0x13,		//QQ����
	TX_ID_MGFACE	= 0x14,		//ħ������
	TX_ID_GAMEK		= 0x15,		//��Ϸ������
	TX_ID_RING		= 0x16,		//����
	TX_ID_SHOW30	= 0x17,		//30ԪQQSHOW�����û�
	TX_ID_ICOKE		= 0x18,		//3DShow(
	TX_ID_QQDNA		= 0x19,		//QQDNA��Ŀ
	TX_ID_QQNOTICE	= 0x1A,		//QQNOTICE
	TX_ID_MUSICJL	= 0x1B,		//QQMUSIC
	TX_ID_QQPET		= 0x1C,		//�������
	TX_ID_QQSPEED	= 0x1D,		//�ɳ�����
	TX_ID_QQXU		= 0x1E,		//QQ�к���
	TX_ID_QQXL		= 0x1F,		//QQ�к��������¼
	TX_ID_DNF		= 0x20,		// DNF �����û�
	TX_ID_CF		= 0x21,		//CF���·���
	TX_ID_XX		= 0x24,		//Ѱ��VIP��2009-04-21��֪������Ӧ��bitΪ��0x1000000000

	TX_ID_MAX
};

}		// EOF namespace n_pngs

// ��������
#define	TTY_DLL_NAME_GS_DB						"uGS_DB"			// ��DB���ӵ�ģ��
#define	TTY_DLL_NAME_GS_GC						"uGS_GC_TTY"		// ��ȫ�ֿ��Ʒ��������ӵ�ģ��
#define	TTY_DLL_NAME_GS_POINTSTOCK				"uGS_POINTSTOCK"	// ���е㿨���׵�ģ��
#define	TTY_DLL_NAME_GS_QPoint					"uGS_QPoint"		// Q�㡢Q����ص�ģ��
#define	TTY_DLL_NAME_GS_Auth					"uGS_Auth"			// �����֤
#define	TTY_DLL_NAME_LBA						"uLBA"				// LBA���߼�
#define	TTY_DLL_NAME_GMLS						"uGMLS"				// ��άҵ��ϵͳ����֧��
#define	TTY_DLL_NAME_LIS						"uLIS"				// ������Ϣϵͳ�������ʺš����ų齱֮��ģ�
#define	TTY_DLL_NAME_VERIFYCODE					"uVERIFYCODE"		// ��֤��ϵͳ�����
#define	TTY_DLL_NAME_VERIFYCODECLIENT			"uVERIFYCODECLIENT"	// ��֤��ϵͳ����ģ��
#define	TTY_DLL_NAME_APEXANTIHACKER				"uAPEXANTIHACKER"		// APEX�����ϵͳ����ģ��
#define	TTY_DLL_NAME_XTRAPANTIHACKER			"uXTRAPANTIHACKER"	// XTRAP�����ϵͳ����ģ��
#define	TTY_DLL_NAME_MALAY_Auth					"uMALAY_Auth"		// �������ǵ�½��֤LBA���
#define	TTY_DLL_NAME_GLVMALAY					"uGLVMALAY"			// �������ǵ�½��֤����
#define TTY_DLL_NAME_BBS						"uBBS"				// �����ģ��
#define	TTY_DLL_NAME_GS_DB4Web					"uGS_DB4Web"		// ��DBS4Web���ӵ�ģ��,added by yuezhongyue 2011-10-12
#define TTY_DLL_NAME_LP_MainStructure			"uLPMainStructure"	// LP�ĵײ����Ӵ���ģ��,added by yuezhongyue 2011-11-24
#define TTY_DLL_NAME_LP_GamePlay				"uLPGamePlay"		// LP���߼���,added by yuezhongyue 2011-11-24
#define TTY_DLL_NAME_Postman4Web				"uPostman4Web"		// ��LPͨ�ŵ�ģ��(����web����Ϸ����ʹ��),added by yuezhongyue 2011-11-24
#define TTY_DLL_NAME_GS4Web						"uGS4Web"			// ȫ�ַ�����,added by yuezhongyue 2012-03-14
#define TTY_DLL_NAME_BD4Web						"uBD4Web"			// ҵ������ģ��,added by yuezhongyue 2012-03-14
#define TTY_DLL_NAME_QQTIPS						"uQQTIPS"			// QQTIPSģ��
// �ͻ��˵�
#define	TTY_DLL_NAME_GC_CHAT					"uGC_CHAT_TTY"		// �ͻ��˵�����ģ��
#define	TTY_DLL_NAME_GC_ASSIST					"uGC_ASSIST_TTY"	// �ͻ��˸���ģ�飨һЩ������������棩

#define	TTY_EMPTY_MD5							"d41d8cd98f00b204e9800998ecf8427e"
																	// ���ִ���md5��

#endif	// EOF __TTY_COMMON_DEF_H__
