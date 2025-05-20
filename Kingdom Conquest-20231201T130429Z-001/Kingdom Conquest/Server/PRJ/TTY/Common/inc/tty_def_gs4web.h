#ifndef __tty_def_gs4web_H__
#define __tty_def_gs4web_H__

#include "tty_common_def.h"
#include "tty_common_BD4Web.h"
#include "pngs_packet_web_extension.h"

namespace n_pngs
{
//////////////////////////////////////////////////////////////////////////
// 一些预定义的值
//////////////////////////////////////////////////////////////////////////
enum
{
	PNGSPACKET_2GS4WEB_EXT_NUM			= 4,				// ext整型数据的个数
};
//////////////////////////////////////////////////////////////////////////
// 通用的数据结构
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct Web_Notification_T 
{
	int		nGroupID;
	int		nNotificationID;
	time_t	nTime;
	int		nTitleLen;
//	char*	pszTitle;
//	int		nContentLen;
//	char*	pszContent;

	Web_Notification_T()
	: nGroupID(-1)
	, nNotificationID(-1)
	, nTime(0)
	, nTitleLen(0)
	{
	}
	~Web_Notification_T()
	{
		clear();
	}
	inline int	GetTotalSize()
	{
		int		nMailSize	= sizeof(Web_Notification_T) + nTitleLen;	// int是nContentLen
		int*	pContentLen	= (int*)wh_getoffsetaddr(this, nMailSize);
		nMailSize			+= ((*pContentLen)+sizeof(int));

		return	nMailSize;
	}
	static int	GetTotalSize(int nTitleLen, int nContentLen)
	{
		return	sizeof(Web_Notification_T) + nTitleLen + sizeof(int) + nContentLen;
	}

	void	clear()
	{
		nGroupID		= -1;
		nNotificationID	= -1;
		nTime			= 0;
		nTitleLen		= 0;
	}
};
#pragma pack()
//////////////////////////////////////////////////////////////////////////
// 大指令
//////////////////////////////////////////////////////////////////////////
enum
{
	P_GS4WEB_DB_REQ_CMD					= 0x00,
	P_GS4WEB_DB_RPL_CMD					= 0x01,
	P_GS4WEB_ASSIST_REQ_CMD				= 0x02,
	P_GS4WEB_ASSIST_RPL_CMD				= 0x03,
	P_GS4WEB_FIRSTIN					= 0x04,
	P_GS4WEB_GROUP_REQ_CMD				= 0x05,				// 发往大区的
	P_GS4WEB_GROUP_RPL_CMD				= 0x06,				// 大区返回
};
// 小指令
//////////////////////////////////////////////////////////////////////////
// DB相关
//////////////////////////////////////////////////////////////////////////
// DB请求
enum
{
	DB_ACCOUNT_LOGIN_REQ				= 0x0100,
	DB_ACCOUNT_LOGOUT_REQ				= 0x0101,
	DB_ADD_NOTIFY_REQ					= 0x0102,
	DB_LOAD_NOTIFY_REQ					= 0x0103,
	
	DB_DEL_NOTIFY_BY_ID_REQ				= 0x0104,
	DB_DEL_NOTIFY_BT_REQ				= 0x0105,
	DB_DEL_NOTIFY_AT_REQ				= 0x0106,
	DB_DEL_NOTIFY_ALL_REQ				= 0x0107,
	DB_DEL_NOTIFY_BY_GROUP_REQ			= 0x0108,

	// 创建账号
	DB_ACCOUNT_CREATE_REQ				= 0x010E,

	DB_ACCOUNT_LOGIN_DEVICEID_REQ		= 0x010F,		// 通过设备ID登陆
	DB_REGISTER_REQ						= 0x0110,		// 注册账号/修改密码
	DB_BIND_DEVICE_REQ					= 0x0111,		// 绑定设备

	// 封账号
	DB_BAN_ACCOUNT_REQ					= 0x0112,

	DB_GET_ADCOLONY_REQ					= 0x0113,
	DB_DEAL_ADCOLONY_REQ				= 0x0114,
};
// DB返回
enum
{
	DB_ACCOUNT_LOGIN_RPL				= 0x0100,
	DB_ACCOUNT_LOGOUT_RPL				= 0x0101,
	DB_ADD_NOTIFY_RPL					= 0x0102,
	DB_LOAD_NOTIFY_RPL					= 0x0103,

	DB_DEL_NOTIFY_BY_ID_RPL				= 0x0104,
	DB_DEL_NOTIFY_BT_RPL				= 0x0105,
	DB_DEL_NOTIFY_AT_RPL				= 0x0106,
	DB_DEL_NOTIFY_ALL_RPL				= 0x0107,
	DB_DEL_NOTIFY_BY_GROUP_RPL			= 0x0108,

	// 创建账号
	DB_ACCOUNT_CREATE_RPL				= 0x010E,

	DB_ACCOUNT_LOGIN_DEVICEID_RPL		= 0x010F,		// 通过设备ID登陆(占位)
	DB_REGISTER_RPL						= 0x0110,		// 注册账号/修改密码
	DB_BIND_DEVICE_RPL					= 0x0111,		// 绑定设备

	// 封账号
	DB_BAN_ACCOUNT_RPL					= 0x0112,
	DB_GET_ADCOLONY_RPL					= 0x0113,
	DB_DEAL_ADCOLONY_RPL				= 0x0114,
};
//////////////////////////////////////////////////////////////////////////
// 指令
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct GS4WEB_CMD_BASE_T 
{
	unsigned char	nCmd;
	unsigned int	nSubCmd;
};
// DB请求
struct GS4WEB_DB_ACCOUNT_LOGIN_REQ_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned char	nGroupIdx;
	char			szAccount[TTY_ACCOUNTNAME_LEN];
	unsigned int	nDeviceType;
	char			szDeviceID[TTY_DEVICE_ID_LEN];
	int				nPassLen;
	char			szPass[1];
};
struct GS4WEB_DB_ACCOUNT_LOGIN_DEVICEID_REQ_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned char	nGroupIdx;
	char			szDeviceID[TTY_DEVICE_ID_LEN];
	unsigned int	nDeviceType;
};
struct GS4WEB_DB_ACCOUNT_LOGOUT_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned char	nGroupIdx;
};
struct GS4WEB_DB_ADD_NOTIFY_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nGroupID;
	unsigned int	nTime;
	int				nTitleLen;
//	char*			pTitle;
//	int				nContentLen;
//	char*			pContent;
};
struct GS4WEB_DB_LOAD_NOTIFY_REQ_T : public GS4WEB_CMD_BASE_T 
{
};
struct GS4WEB_DB_DEL_NOTIFY_BY_ID_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nNotificationID;
};
struct GS4WEB_DB_DEL_NOTIFY_BT_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned int	nTime;
};
struct GS4WEB_DB_DEL_NOTIFY_AT_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned int	nTime;
};
struct GS4WEB_DB_DEL_NOTIFY_ALL_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
};
struct GS4WEB_DB_DEL_NOTIFY_BY_GROUP_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nGroupID;
};
struct GS4WEB_DB_ACCOUNT_CREATE_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned char	nGroupIdx;
	char			szAccountName[TTY_ACCOUNTNAME_LEN];
	char			szPass[TTY_ACCOUNTPASSWORD_LEN];
	bool			bUseRandomAccountName;
	char			szDeviceID[TTY_DEVICE_ID_LEN];
	unsigned int	nDeviceType;
	unsigned int	nAppID;
};
struct GS4WEB_DB_REGISTER_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned char	nGroupIdx;
	tty_id_t		nAccountID;
	char			szAccount[TTY_ACCOUNTNAME_LEN];
	int				nPassLen;
	//char			szPass[nPassLen];
	// 1.修改密码则后面跟着老密码串
	// 2.注册则将nOldPassLen设置为0
	//int			nOldPassLen;
	//char			szOldPass[nOldPassLen];
};
struct GS4WEB_DB_BIND_DEVICE_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned char	nGroupIdx;
	tty_id_t		nAccountID;
	char			szDeviceID[TTY_DEVICE_ID_LEN];
};
// DB返回
struct GS4WEB_DB_ACCOUNT_LOGIN_RPL_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;			// 见TTY_LOGIN_RST_XXX,如TTY_LOGIN_RST_SUCCEED
	tty_id_t		nAccountID;		// 账号ID
	unsigned char	nGroupIdx;		// 在线的大区号
	unsigned char	nFromGroupIdx;	// 指令来自哪个大区
	unsigned char	nBinded;		// 是否绑定过邮箱了
	char			szDeviceID[TTY_DEVICE_ID_LEN];
	char			szAccountName[TTY_ACCOUNTNAME_LEN];
	char			szPass[TTY_PASSWORD_LEN];
	unsigned int	nGiftAppID;
};
struct GS4WEB_DB_ACCOUNT_LOGOUT_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;			
	unsigned char	nGroupIdx;		// 在线的大区号
	unsigned char	nFromGroupIdx;	// 指令来自哪个大区
};
struct GS4WEB_DB_ADD_NOTIFY_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nRst;
	// WebNotification_T 
};
struct GS4WEB_DB_LOAD_NOTIFY_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nRst;
	int				nNum;
};
struct GS4WEB_DB_DEL_NOTIFY_BY_ID_RPL_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nNotificationID;
	int				nRst;
};
struct GS4WEB_DB_DEL_NOTIFY_BT_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned int	nTime;
	int				nRst;
};
struct GS4WEB_DB_DEL_NOTIFY_AT_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned int	nTime;
	int				nRst;
};
struct GS4WEB_DB_DEL_NOTIFY_ALL_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nRst;
};
struct GS4WEB_DB_DEL_NOTIFY_BY_GROUP_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nGroupID;
	int				nRst;
};
struct GS4WEB_DB_ACCOUNT_CREATE_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;										// 返回结果
	tty_id_t		nAccountID;
	char			szAccount[TTY_ACCOUNTNAME_LEN];				// 自动生成的Account名
	unsigned char	nFromGroupIdx;
	char			szPass[TTY_PASSWORD_LEN];
	unsigned int	nGiftAppID;
};
struct GS4WEB_DB_REGISTER_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	tty_id_t		nAccountID;
	bool			bIsRegister;
	unsigned char	nFromGroupIdx;
};
struct GS4WEB_DB_BIND_DEVICE_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned char	nFromGroupIdx;
};

struct GS4WEB_DB_BAN_ACCOUNT_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	bool			bBanDevice;
};
// DB返回
struct GS4WEB_DB_BAN_ACCOUNT_RPL_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;			
	tty_id_t		nAccountID;		
	bool			bBanDevice;		
	char			szDeviceID[TTY_DEVICE_ID_LEN];
};

struct GS4WEB_DB_GET_ADCOLONY_REQ_T : public GS4WEB_CMD_BASE_T 
{
};
struct GS4WEB_DB_DEAL_ADCOLONY_REQ_T : public GS4WEB_CMD_BASE_T 
{
	tty_id_t		nTransactionID;
};
// DB返回
struct GS4WEB_DB_GET_ADCOLONY_RPL_T : public GS4WEB_CMD_BASE_T
{
	struct AdColony_T 
	{
		tty_id_t		nTransactionID;
		unsigned int	nAmount;
		char			szName[32];
		unsigned int	nTime;
		tty_id_t		nAccountID;
		unsigned int	nGroupID;
	};
	int				nNum;
};
struct GS4WEB_DB_DEAL_ADCOLONY_RPL_T : public GS4WEB_CMD_BASE_T
{
	tty_id_t		nTransactionID;
};
//////////////////////////////////////////////////////////////////////////
struct GS4WEB_FIRSTIN_T : public GS4WEB_CMD_BASE_T 
{
	int				nGroupID;
};
//////////////////////////////////////////////////////////////////////////
// 和大区的交互指令,比如GM、通知等(下发给大区的)
//////////////////////////////////////////////////////////////////////////
enum
{
	GROUP_LOAD_NOTIFY_REQ				= 0x0001,

	GROUP_DEL_NOTIFY_BY_ID_REQ			= 0x0002,
	GROUP_DEL_NOTIFY_BT_REQ				= 0x0003,
	GROUP_DEL_NOTIFY_AT_REQ				= 0x0004,
	GROUP_DEL_NOTIFY_ALL_REQ			= 0x0005,
	GROUP_DEL_NOTIFY_BY_GROUP_REQ		= 0x0006,

	GROUP_ADD_NOTIFY_REQ				= 0x0007,

	GROUP_GET_GROUP_INFO_REQ			= 0x0008,


	// 充值
	GROUP_RECHARGE_DIAMOND_REQ			= 0x0100,
	GROUP_RELOAD_EXCEL_TABLE_REQ		= 0x0101,

	// 踢号
	GROUP_KICK_CHAR_REQ					= 0x0102,
	// 封角色
	GROUP_BAN_CHAR_REQ					= 0x0103,
	// 新的AdColony
	GROUP_NEW_ADCOLONY_REQ				= 0x0104,
	
	// 让客户端退出游戏
	GROUP_KICK_CLIENT_ALL_REQ			= 0x0105,
};
enum
{
	GROUP_LOAD_NOTIFY_RPL				= 0x0001,

	GROUP_DEL_NOTIFY_BY_ID_RPL			= 0x0002,
	GROUP_DEL_NOTIFY_BT_RPL				= 0x0003,
	GROUP_DEL_NOTIFY_AT_RPL				= 0x0004,
	GROUP_DEL_NOTIFY_ALL_RPL			= 0x0005,
	GROUP_DEL_NOTIFY_BY_GROUP_RPL		= 0x0006,

	GROUP_ADD_NOTIFY_RPL				= 0x0007,

	GROUP_GET_GROUP_INFO_RPL			= 0x0008,

	// 充值
	GROUP_RECHARGE_DIAMOND_RPL			= 0x0100,
	GROUP_RELOAD_EXCEL_TABLE_RPL		= 0x0101,

	// 踢号
	GROUP_KICK_CHAR_RPL					= 0x0102,
	// 封角色
	GROUP_BAN_CHAR_RPL					= 0x0103,
	// 新的AdColony
	GROUP_NEW_ADCOLONY_RPL				= 0x0104,

	// 让客户端退出游戏
	GROUP_KICK_CLIENT_ALL_RPL			= 0x0105,
};
// 请求
struct GS4WEB_GROUP_LOAD_NOTIFY_REQ_T : public GS4WEB_CMD_BASE_T
{
	int		nNum;
	// 后面跟着nNum个Web_Notification_T
};
struct GS4WEB_GROUP_DEL_NOTIFY_BY_ID_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int		nNotificationID;
};
struct GS4WEB_GROUP_DEL_NOTIFY_BT_REQ_T : public GS4WEB_CMD_BASE_T 
{
	unsigned int	nTime;
};
struct GS4WEB_GROUP_DEL_NOTIFY_AT_REQ_T : public GS4WEB_CMD_BASE_T 
{
	unsigned int	nTime;
};
struct GS4WEB_GROUP_DEL_NOTIFY_ALL_REQ_T : public GS4WEB_CMD_BASE_T 
{

};
struct GS4WEB_GROUP_DEL_NOTIFY_BY_GROUP_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nGroupID;
};

struct GS4WEB_GROUP_ADD_NOTIFY_REQ_T : public GS4WEB_CMD_BASE_T 
{
	// 一个Web_Notification_T
};
struct GS4WEB_GROUP_GET_GROUP_INFO_REQ_T : public GS4WEB_CMD_BASE_T 
{
};
// 返回,暂时木有,以后可以做
struct GS4WEB_GROUP_GET_GROUP_INFO_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int			nPlayerTotalNum;
	int			nMaxPlayerNum;
	int			aPlayerNum[TERM_TYPE_NUM];
};
struct GS4WEB_GROUP_RECHARGE_DIAMOND_REQ_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
	unsigned int	nAddedDiamond;
};
struct GS4WEB_GROUP_RECHARGE_DIAMOND_RPL_T : public GS4WEB_CMD_BASE_T 
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nRst;
	tty_id_t		nAccountID;
	unsigned int	nAddedDiamond;
	unsigned int	nVip;
};

struct GS4WEB_GROUP_RELOAD_EXCEL_TABLE_REQ_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
};
struct GS4WEB_GROUP_RELOAD_EXCEL_TABLE_RPL_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nRst;
	unsigned int	nGroupID;
};

// 踢号
struct GS4WEB_GROUP_KICK_CHAR_REQ_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
	// 封角色
struct GS4WEB_GROUP_BAN_CHAR_REQ_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_id_t		nAccountID;
};
// 踢号
struct GS4WEB_GROUP_KICK_CHAR_RPL_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nRst;
	unsigned int	nGroupID;
	tty_id_t		nAccountID;
};
// 封角色
struct GS4WEB_GROUP_BAN_CHAR_RPL_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	int				nRst;
	unsigned int	nGroupID;
	tty_id_t		nAccountID;
};
// 新的AdColony
struct GS4WEB_GROUP_NEW_ADCOLONY_REQ_T : public GS4WEB_CMD_BASE_T
{
	tty_id_t		nTransactionID;
	unsigned int	nAmount;
	char			szName[32];
	unsigned int	nTime;
	tty_id_t		nAccountID;
};
// 新的AdColony
struct GS4WEB_GROUP_NEW_ADCOLONY_RPL_T : public GS4WEB_CMD_BASE_T
{
	enum
	{
		RST_OK		= 0,
	};
	int				nRst;
	tty_id_t		nTransactionID;
};

// 让客户端退出游戏
struct GS4WEB_GROUP_KICK_CLIENT_ALL_REQ_T : public GS4WEB_CMD_BASE_T
{
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	unsigned int	nLength;
	// 后面跟字符串,appid*version
};
// 让客户端退出游戏
struct GS4WEB_GROUP_KICK_CLIENT_ALL_RPL_T : public GS4WEB_CMD_BASE_T
{
	enum
	{
		RST_OK		= 0,
	};
	int				nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	tty_rst_t		nRst;
	unsigned int	nGroupID;
};
#pragma pack()

//////////////////////////////////////////////////////////////////////////
// 4500~4599
//////////////////////////////////////////////////////////////////////////
enum
{
	PNGSPACKET_BD4WEB_CMD_0			= 4500,
	PNGSPACKET_BD4WEB_CMD_REQ		= PNGSPACKET_BD4WEB_CMD_0 + 1,
	PNGSPACKET_BD4WEB_CMD_RPL		= PNGSPACKET_BD4WEB_CMD_0 + 2,
	PNGSPACKET_BD4WEB_CMD_SET_PTR	= PNGSPACKET_BD4WEB_CMD_0 + 3,
};
#pragma pack(1)
struct PNGSPACKET_BD4WEB_CMD_REQ_T 
{
	int						nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	size_t					nDSize;
	const BD_CMD_BASE_T*	pData;
};
struct PNGSPACKET_BD4WEB_CMD_RPL_T 
{
	int						nExt[PNGSPACKET_2GS4WEB_EXT_NUM];
	size_t					nDSize;
	const BD_CMD_BASE_T*	pData;
};
#pragma pack()
}

#endif
