#ifndef __tty_common_BD4Web_H__
#define __tty_common_BD4Web_H__

#include "tty_common_def.h"

using namespace n_pngs;
// 业务受理系统字符集使用UTF8，字节序使用网络字节序 

enum
{
	GROUP_ID_GLOBAL					= 0,			// 0->代表全局服
	GROUP_ID_ALL_WITHOUT_GLOBAL		= 0xFFFE,		// 所有服务器组(不包括全局服)
	GROUP_ID_ALL					= 0xFFFF,		// 所有服务器组(包括全局服)
	// 其他服务器组的ID就是大区号,如体验服的大区ID为3
};

// 返回值
enum
{
	BD_RST_OK						= 0,			// 成功
	BD_RST_DB_ERR					= -1,			// 数据库出错
	BD_RST_TIMEOUT					= -2,			// 请求超时
	BD_RST_GROUP_NOT_EXIST			= -3,			// 服务器并不存在,或者断线,或者还没启动
	BD_RST_TIME_FORMAT_ERR			= -4,			// 错误的时间格式
	BD_RST_NOTIFY_LOADING			= -5,			// 通知载入中
	BD_RST_NOTIFY_NOT_EXIST			= -6,			// 通知不存在
	BD_RST_BD_SUPPORT_NOT_OPEN		= -7,			// 未打开业务受理系统指令支持
};

typedef	unsigned int	bd_cmd_t;
typedef	unsigned int	bd_size_t;

//////////////////////////////////////////////////////////////////////////
// 指令
//////////////////////////////////////////////////////////////////////////
// 请求
enum
{
	//////////////////////////////////////////////////////////////////////////
	// 通知相关
	//////////////////////////////////////////////////////////////////////////
	// 让服务器重新加载通知数据
	// group id为	0,则只有全局服重新从数据库加载一次通知;
	//				0xFFFE,则把全局服内存中的通知刷新到所有大区;
	//				0xFFFF,则全局服先从数据库加载通知,然后将通知刷新到所有服务器
	BD_CMD_RELOAD_NOTIFY				= 0x0000,

	// 添加新通知
	BD_CMD_ADD_NOTIFY					= 0x0001,

	// 根据通知ID删除通知
	BD_CMD_DEL_NOTIFY_BY_ID				= 0x0010,
	// 删除某个时间点之前发布的通知,时间格式为YYYY-MM-DD[ HH:MM:SS]([]内为可选项,即时分秒)
	BD_CMD_DEL_NOTIFY_BEFORE_TIME		= 0x0011,
	// 删除某个时间点之后发布的通知,时间格式为YYYY-MM-DD[ HH:MM:SS]([]内为可选项,即时分秒)
	BD_CMD_DEL_NOTIFY_AFTER_TIME		= 0x0012,
	// 删除所有通知
	BD_CMD_DEL_NOTIFY_ALL				= 0x0013,
	// 删除具有某个group id的通知(如nGroupID=0xFFFE,则删除所有group_id为0xFFFE的通知)
	BD_CMD_DEL_NOTIFY_BY_GROUP			= 0x0014,

	// 根据通知ID获取通知内容
	BD_CMD_GET_NOTIFY_BY_ID				= 0x0020,
	// 获取某个时间之前发布的所有通知,时间格式为YYYY-MM-DD[ HH:MM:SS]([]内为可选项,即时分秒)
	BD_CMD_GET_NOTIFY_BEFORE_TIME		= 0x0021,
	// 获取某个时间之后发布的所有通知,时间格式为YYYY-MM-DD[ HH:MM:SS]([]内为可选项,即时分秒)
	BD_CMD_GET_NOTIFY_AFTER_TIME		= 0x0022,
	// 获取所有通知
	BD_CMD_GET_NOTIFY_ALL				= 0x0023,
	// 根据大区ID获取通知(nGroupID为0xFFFF,获取nGroupID==0xFFFF的通知)
	BD_CMD_GET_NOTIFY_BY_GROUP			= 0x0024,

	// 修改通知,暂不提供功能(要修改通知,应该先删除原有的,再发布一条新的)
	BD_CMD_UPDATE_NOTIFY				= 0x0030,

	//////////////////////////////////////////////////////////////////////////
	// GM指令相关
	//////////////////////////////////////////////////////////////////////////
	BD_CMD_RECHARGE_DIAMOND				= 0x0100,
	BD_CMD_RELOAD_EXCEL_TABLE			= 0x0101,
	BD_CMD_KICK_CHAR					= 0x0102,
	BD_CMD_BAN_CHAR						= 0x0103,
	BD_CMD_BAN_ACCOUNT					= 0x0104,
	BD_CMD_KICK_CLIENT_ALL				= 0x0105,

	// 大区不存在
	BD_CMD_GROUP_NOT_EXIST				= 0xFFFFFFFE,
	// 没有打开BD支持
	BD_CMD_BD_NOT_OPEN					= 0xFFFFFFFF,
};
// 返回
enum
{
	// 添加新通知
	BD_CMD_ADD_NOTIFY_RPL				= 0x0001,

	// 根据通知ID删除通知
	BD_CMD_DEL_NOTIFY_BY_ID_RPL			= 0x0010,
	// 删除某个时间点之前发布的通知,时间格式为YYYY-MM-DD[ HH:MM:SS]([]内为可选项,即时分秒)
	BD_CMD_DEL_NOTIFY_BEFORE_TIME_RPL	= 0x0011,
	// 删除某个时间点之后发布的通知,时间格式为YYYY-MM-DD[ HH:MM:SS]([]内为可选项,即时分秒)
	BD_CMD_DEL_NOTIFY_AFTER_TIME_RPL	= 0x0012,
	// 删除所有通知
	BD_CMD_DEL_NOTIFY_ALL_RPL			= 0x0013,
	// 删除某个大区的通知
	BD_CMD_DEL_NOTIFY_BY_GROUP_RPL		= 0x0014,

	// 根据通知ID获取通知内容
	BD_CMD_GET_NOTIFY_BY_ID_RPL			= 0x0020,
	// 获取某个时间之前发布的所有通知,时间格式为YYYY-MM-DD[ HH:MM:SS]([]内为可选项,即时分秒)
	BD_CMD_GET_NOTIFY_BEFORE_TIME_RPL	= 0x0021,
	// 获取某个时间之后发布的所有通知,时间格式为YYYY-MM-DD[ HH:MM:SS]([]内为可选项,即时分秒)
	BD_CMD_GET_NOTIFY_AFTER_TIME_RPL	= 0x0022,
	// 获取所有通知
	BD_CMD_GET_NOTIFY_ALL_RPL			= 0x0023,
	// 根据大区ID获取通知
	BD_CMD_GET_NOTIFY_BY_GROUP_RPL		= 0x0024,

	//////////////////////////////////////////////////////////////////////////
	// GM指令相关
	//////////////////////////////////////////////////////////////////////////
	BD_CMD_RECHARGE_DIAMOND_RPL			= 0x0100,
	BD_CMD_RELOAD_EXCEL_TABLE_RPL		= 0x0101,
	BD_CMD_KICK_CHAR_RPL				= 0x0102,
	BD_CMD_BAN_CHAR_RPL					= 0x0103,
	BD_CMD_BAN_ACCOUNT_RPL				= 0x0104,
	BD_CMD_KICK_CLIENT_ALL_RPL			= 0x0105,

	// 大区不存在
	BD_CMD_GROUP_NOT_EXIST_RPL			= 0xFFFFFFFE,
	// 没有打开BD支持
	BD_CMD_BD_NOT_OPEN_RPL				= 0xFFFFFFFF,
};
//////////////////////////////////////////////////////////////////////////
// 消息结构
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct BD_CMD_BASE_T
{
	bd_size_t			nSize;					// 整个消息包的长度(包括nSize这个字段)
	bd_cmd_t			nCmd;
};

enum
{
	MAX_BD_CMD_SIZE		= 16*1024,						// 指令最大长度
	MIN_BD_CMD_SIZE		= sizeof(BD_CMD_BASE_T),		// 指令最小长度
	MAX_BD_TEXT_LEN		= 15*1024,						// 文本最大长度15k(包括公告等)
	MAX_BD_TIMESTR_LEN	= 32,							// 时间串的长度
};

// 请求
struct BD_CMD_ADD_NOTIFY_T : public BD_CMD_BASE_T
{
	char				szTimeStr[MAX_BD_TIMESTR_LEN];		// 空字串代表设置公告时间为当前时间
	int					nGroupID;
	int					nTitleLen;
//	char				szTitle[];
//	int					nContentLen;		// szNotification的长度,包括结尾的'\0'
//	char				szContent[];		// szNotification的长度为nNotificationLen
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
// 返回
struct BD_CMD_ADD_NOTIFY_RPL_T : public BD_CMD_BASE_T 
{
	int					nRst;					// 返回值,是否成功等
	// 后面跟着WebNotification_T
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
	// 后面跟字符串,appid*version
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
