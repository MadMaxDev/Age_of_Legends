#ifndef __tty_common_BD4Web_H__
#define __tty_common_BD4Web_H__

#include "tty_common_def.h"

using namespace n_pngs;
// ҵ������ϵͳ�ַ���ʹ��UTF8���ֽ���ʹ�������ֽ��� 

enum
{
	GROUP_ID_GLOBAL					= 0,			// 0->����ȫ�ַ�
	GROUP_ID_ALL_WITHOUT_GLOBAL		= 0xFFFE,		// ���з�������(������ȫ�ַ�)
	GROUP_ID_ALL					= 0xFFFF,		// ���з�������(����ȫ�ַ�)
	// �������������ID���Ǵ�����,��������Ĵ���IDΪ3
};

// ����ֵ
enum
{
	BD_RST_OK						= 0,			// �ɹ�
	BD_RST_DB_ERR					= -1,			// ���ݿ����
	BD_RST_TIMEOUT					= -2,			// ����ʱ
	BD_RST_GROUP_NOT_EXIST			= -3,			// ��������������,���߶���,���߻�û����
	BD_RST_TIME_FORMAT_ERR			= -4,			// �����ʱ���ʽ
	BD_RST_NOTIFY_LOADING			= -5,			// ֪ͨ������
	BD_RST_NOTIFY_NOT_EXIST			= -6,			// ֪ͨ������
	BD_RST_BD_SUPPORT_NOT_OPEN		= -7,			// δ��ҵ������ϵͳָ��֧��
};

typedef	unsigned int	bd_cmd_t;
typedef	unsigned int	bd_size_t;

//////////////////////////////////////////////////////////////////////////
// ָ��
//////////////////////////////////////////////////////////////////////////
// ����
enum
{
	//////////////////////////////////////////////////////////////////////////
	// ֪ͨ���
	//////////////////////////////////////////////////////////////////////////
	// �÷��������¼���֪ͨ����
	// group idΪ	0,��ֻ��ȫ�ַ����´����ݿ����һ��֪ͨ;
	//				0xFFFE,���ȫ�ַ��ڴ��е�֪ͨˢ�µ����д���;
	//				0xFFFF,��ȫ�ַ��ȴ����ݿ����֪ͨ,Ȼ��֪ͨˢ�µ����з�����
	BD_CMD_RELOAD_NOTIFY				= 0x0000,

	// �����֪ͨ
	BD_CMD_ADD_NOTIFY					= 0x0001,

	// ����֪ͨIDɾ��֪ͨ
	BD_CMD_DEL_NOTIFY_BY_ID				= 0x0010,
	// ɾ��ĳ��ʱ���֮ǰ������֪ͨ,ʱ���ʽΪYYYY-MM-DD[ HH:MM:SS]([]��Ϊ��ѡ��,��ʱ����)
	BD_CMD_DEL_NOTIFY_BEFORE_TIME		= 0x0011,
	// ɾ��ĳ��ʱ���֮�󷢲���֪ͨ,ʱ���ʽΪYYYY-MM-DD[ HH:MM:SS]([]��Ϊ��ѡ��,��ʱ����)
	BD_CMD_DEL_NOTIFY_AFTER_TIME		= 0x0012,
	// ɾ������֪ͨ
	BD_CMD_DEL_NOTIFY_ALL				= 0x0013,
	// ɾ������ĳ��group id��֪ͨ(��nGroupID=0xFFFE,��ɾ������group_idΪ0xFFFE��֪ͨ)
	BD_CMD_DEL_NOTIFY_BY_GROUP			= 0x0014,

	// ����֪ͨID��ȡ֪ͨ����
	BD_CMD_GET_NOTIFY_BY_ID				= 0x0020,
	// ��ȡĳ��ʱ��֮ǰ����������֪ͨ,ʱ���ʽΪYYYY-MM-DD[ HH:MM:SS]([]��Ϊ��ѡ��,��ʱ����)
	BD_CMD_GET_NOTIFY_BEFORE_TIME		= 0x0021,
	// ��ȡĳ��ʱ��֮�󷢲�������֪ͨ,ʱ���ʽΪYYYY-MM-DD[ HH:MM:SS]([]��Ϊ��ѡ��,��ʱ����)
	BD_CMD_GET_NOTIFY_AFTER_TIME		= 0x0022,
	// ��ȡ����֪ͨ
	BD_CMD_GET_NOTIFY_ALL				= 0x0023,
	// ���ݴ���ID��ȡ֪ͨ(nGroupIDΪ0xFFFF,��ȡnGroupID==0xFFFF��֪ͨ)
	BD_CMD_GET_NOTIFY_BY_GROUP			= 0x0024,

	// �޸�֪ͨ,�ݲ��ṩ����(Ҫ�޸�֪ͨ,Ӧ����ɾ��ԭ�е�,�ٷ���һ���µ�)
	BD_CMD_UPDATE_NOTIFY				= 0x0030,

	//////////////////////////////////////////////////////////////////////////
	// GMָ�����
	//////////////////////////////////////////////////////////////////////////
	BD_CMD_RECHARGE_DIAMOND				= 0x0100,
	BD_CMD_RELOAD_EXCEL_TABLE			= 0x0101,
	BD_CMD_KICK_CHAR					= 0x0102,
	BD_CMD_BAN_CHAR						= 0x0103,
	BD_CMD_BAN_ACCOUNT					= 0x0104,
	BD_CMD_KICK_CLIENT_ALL				= 0x0105,

	// ����������
	BD_CMD_GROUP_NOT_EXIST				= 0xFFFFFFFE,
	// û�д�BD֧��
	BD_CMD_BD_NOT_OPEN					= 0xFFFFFFFF,
};
// ����
enum
{
	// �����֪ͨ
	BD_CMD_ADD_NOTIFY_RPL				= 0x0001,

	// ����֪ͨIDɾ��֪ͨ
	BD_CMD_DEL_NOTIFY_BY_ID_RPL			= 0x0010,
	// ɾ��ĳ��ʱ���֮ǰ������֪ͨ,ʱ���ʽΪYYYY-MM-DD[ HH:MM:SS]([]��Ϊ��ѡ��,��ʱ����)
	BD_CMD_DEL_NOTIFY_BEFORE_TIME_RPL	= 0x0011,
	// ɾ��ĳ��ʱ���֮�󷢲���֪ͨ,ʱ���ʽΪYYYY-MM-DD[ HH:MM:SS]([]��Ϊ��ѡ��,��ʱ����)
	BD_CMD_DEL_NOTIFY_AFTER_TIME_RPL	= 0x0012,
	// ɾ������֪ͨ
	BD_CMD_DEL_NOTIFY_ALL_RPL			= 0x0013,
	// ɾ��ĳ��������֪ͨ
	BD_CMD_DEL_NOTIFY_BY_GROUP_RPL		= 0x0014,

	// ����֪ͨID��ȡ֪ͨ����
	BD_CMD_GET_NOTIFY_BY_ID_RPL			= 0x0020,
	// ��ȡĳ��ʱ��֮ǰ����������֪ͨ,ʱ���ʽΪYYYY-MM-DD[ HH:MM:SS]([]��Ϊ��ѡ��,��ʱ����)
	BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL	= 0x0021,
	// ��ȡĳ��ʱ��֮�󷢲�������֪ͨ,ʱ���ʽΪYYYY-MM-DD[ HH:MM:SS]([]��Ϊ��ѡ��,��ʱ����)
	BD_CMD_GET_NOTIFY_AFTER_TIME_RPL	= 0x0022,
	// ��ȡ����֪ͨ
	BD_CMD_GET_NOTIFY_ALL_RPL			= 0x0023,
	// ���ݴ���ID��ȡ֪ͨ
	BD_CMD_GET_NOTIFY_BY_GROUP_RPL		= 0x0024,

	//////////////////////////////////////////////////////////////////////////
	// GMָ�����
	//////////////////////////////////////////////////////////////////////////
	BD_CMD_RECHARGE_DIAMOND_RPL			= 0x0100,
	BD_CMD_RELOAD_EXCEL_TABLE_RPL		= 0x0101,
	BD_CMD_KICK_CHAR_RPL				= 0x0102,
	BD_CMD_BAN_CHAR_RPL					= 0x0103,
	BD_CMD_BAN_ACCOUNT_RPL				= 0x0104,
	BD_CMD_KICK_CLIENT_ALL_RPL			= 0x0105,

	// ����������
	BD_CMD_GROUP_NOT_EXIST_RPL			= 0xFFFFFFFE,
	// û�д�BD֧��
	BD_CMD_BD_NOT_OPEN_RPL				= 0xFFFFFFFF,
};
//////////////////////////////////////////////////////////////////////////
// ��Ϣ�ṹ
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct BD_CMD_BASE_T
{
	bd_size_t			nSize;					// ������Ϣ���ĳ���(����nSize����ֶ�)
	bd_cmd_t			nCmd;
};

enum
{
	MAX_BD_CMD_SIZE		= 16*1024,						// ָ����󳤶�
	MIN_BD_CMD_SIZE		= sizeof(BD_CMD_BASE_T),		// ָ����С����
	MAX_BD_TEXT_LEN		= 15*1024,						// �ı���󳤶�15k(���������)
	MAX_BD_TIMESTR_LEN	= 32,							// ʱ�䴮�ĳ���
};

// ����
struct BD_CMD_ADD_NOTIFY_T : public BD_CMD_BASE_T
{
	char				szTimeStr[MAX_BD_TIMESTR_LEN];		// ���ִ��������ù���ʱ��Ϊ��ǰʱ��
	int					nGroupID;
	int					nTitleLen;
//	char				szTitle[];
//	int					nContentLen;		// szNotification�ĳ���,������β��'\0'
//	char				szContent[];		// szNotification�ĳ���ΪnNotificationLen
};
struct BD_CMD_DEL_NOTIFY_BY_ID_T : public BD_CMD_BASE_T 
{
	unsigned int		nNotificationID;
};
struct BD_CMD_DEL_NOTIFY_BEFORE_TIME_T : public BD_CMD_BASE_T 
{
	char				szTimeStr[MAX_BD_TIMESTR_LEN];
};
struct BD_CMD_DEL_NOTIFY_AFTER_TIME_T : public BD_CMD_BASE_T 
{
	char				szTimeStr[MAX_BD_TIMESTR_LEN];
};
struct BD_CMD_DEL_NOTIFY_ALL_T : public BD_CMD_BASE_T 
{
};
struct BD_CMD_DEL_NOTIFY_BY_GROUP_T : public BD_CMD_BASE_T 
{
	int					nGroupID;
};

struct BD_CMD_GET_NOTIFY_BY_ID_T : public BD_CMD_BASE_T 
{
	unsigned int		nNotificationID;
};
struct BD_CMD_GET_NOTIFY_BEFORE_TIME_T : public BD_CMD_BASE_T 
{
	char				szTimeStr[MAX_BD_TIMESTR_LEN];
};
struct BD_CMD_GET_NOTIFY_AFTER_TIME_T : public BD_CMD_BASE_T 
{
	char				szTimeStr[MAX_BD_TIMESTR_LEN];
};
struct BD_CMD_GET_NOTIFY_ALL_T : public BD_CMD_BASE_T 
{
};
struct BD_CMD_GET_NOTIFY_BY_GROUP_T : public BD_CMD_BASE_T 
{
	int					nGroupID;
};
// ����
struct BD_CMD_ADD_NOTIFY_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;					// ����ֵ,�Ƿ�ɹ���
	// �������WebNotification_T
};
struct BD_CMD_DEL_NOTIFY_BY_ID_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	unsigned int		nNotificationID;
};
struct BD_CMD_DEL_NOTIFY_BEFORE_TIME_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	char				szTimeStr[MAX_BD_TIMESTR_LEN];
};
struct BD_CMD_DEL_NOTIFY_AFTER_TIME_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	char				szTimeStr[MAX_BD_TIMESTR_LEN];
};
struct BD_CMD_DEL_NOTIFY_ALL_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
};
struct BD_CMD_DEL_NOTIFY_BY_GROUP_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	int					nGroupID;
};

struct BD_CMD_GET_NOTIFY_BY_ID_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
};
struct BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	int					nNum;
};
struct BD_CMD_GET_NOTIFY_AFTER_TIME_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	int					nNum;
};
struct BD_CMD_GET_NOTIFY_ALL_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	int					nNum;
};
struct BD_CMD_GET_NOTIFY_BY_GROUP_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	int					nNum;
};
struct BD_CMD_RECHARGE_DIAMOND_T : public BD_CMD_BASE_T 
{
	unsigned int		nGroupID;
	tty_id_t			nAccountID;
	unsigned int		nAddedDiamond;
};
struct BD_CMD_RECHARGE_DIAMOND_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;
	unsigned int		nGroupID;
	tty_id_t			nAccountID;
	unsigned int		nAddedDiamond;
	unsigned int		nVip;
};
struct BD_CMD_GROUP_NOT_EXIST_RPL_T : public BD_CMD_BASE_T
{
	unsigned int		nGroupID;
};
struct BD_CMD_RELOAD_EXCEL_TABLE_T : public BD_CMD_BASE_T
{
	unsigned int		nGroupID;
};
struct BD_CMD_RELOAD_EXCEL_TABLE_RPL_T : public BD_CMD_BASE_T 
{
	enum
	{
		RST_OK			= 0,
	};
	int					nRst;
	unsigned int		nGroupID;
};

struct BD_CMD_KICK_CHAR_T : public BD_CMD_BASE_T 
{
	unsigned int		nGroupID;
	tty_id_t			nAccountID;
};
struct BD_CMD_BAN_CHAR_T : public BD_CMD_BASE_T 
{
	unsigned int		nGroupID;
	tty_id_t			nAccountID;
};
struct BD_CMD_BAN_ACCOUNT_T	: public BD_CMD_BASE_T 
{
	tty_id_t			nAccountID;
	bool				bBanDevice;
};
struct BD_CMD_KICK_CHAR_RPL_T : public BD_CMD_BASE_T 
{
	enum
	{
		RST_OK			= 0,
	};
	int					nRst;
	unsigned int		nGroupID;
	tty_id_t			nAccountID;
};
struct BD_CMD_BAN_CHAR_RPL_T : public BD_CMD_BASE_T 
{
	enum
	{
		RST_OK			= 0,
	};
	int					nRst;
	unsigned int		nGroupID;
	tty_id_t			nAccountID;
};
struct BD_CMD_BAN_ACCOUNT_RPL_T	: public BD_CMD_BASE_T 
{
	enum
	{
		RST_OK			= 0,
	};
	int					nRst;
	tty_id_t			nAccountID;
	bool				bBanDevice;
	char				szDeviceID[TTY_DEVICE_ID_LEN];
};

struct BD_CMD_KICK_CLIENT_ALL_T : public BD_CMD_BASE_T 
{
	unsigned int		nGroupID;
	unsigned int		nLength;
	// ������ַ���,appid*version
};
struct BD_CMD_KICK_CLIENT_ALL_RPL_T : public BD_CMD_BASE_T 
{
	enum
	{
		RST_OK			= 0,
	};
	int					nRst;
	unsigned int		nGroupID;
};
#pragma pack()

#endif
