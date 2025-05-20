// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : tty_common_def.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于PNGS的tty的实现的公共定义
//                PNGS是Pixel Network Game Structure的缩写
//                因为客户端和服务器间通讯需要节约带宽，所以指令数据不用结构而用流的方式，可以进行必要的压缩
//                目前已经定义的指令段：
//                客户端的：
//                1300~1399  	Chat模块收到的指令
//                1400~1499 	Chat模块发给别ClientGamePlay模块的指令
//                1500~1599		ASSIST模块收到的指令
//                1600~1699		ASSIST模块发出给接受者(客户端的gameplayer)的指令
//                服务端的：
//                1100~1199		DB模块收到的指令
//                1100~1199		DB模块发出给接受者的指令
//                1300~1399		GSGC模块收到的指令
//                1300~1399		GC模块发出给接受者的指令
//                1500~1599		PointStock模块收到的指令
//                1500~1599		PointStock模块发出给接受者的指令
//                1600~1699		LIS模块收到的指令
//                1600~1699		LIS模块发出给接受者的指令
//                1700~1799		QPoint模块收到的指令
//                1700~1799		QPoint模块发出给接受者的指令
//                1800~1899		GMLS收到的指令
//                1900~1999		LBAAUTH收到的指令
//                2000~2999		校验码模块发出和收到的指令（TTYVerifyCodeClient）的指令
//                3000~3099		反外挂模块发出和收到的指令（TTYAntiHacker）的指令
//				  3200~3299		BBS公告板系统收到和发出的指令
//                3300~3499     web系统收到和发送的指令
//                3500~3799    QQTips模块收到和发送的指令
// CreationDate : 2005-09-15 从 tty_common_packet.h中分离出来
// Change LOG   : 2006-03-07 把原来逻辑包中结果类型从unsigned char改为tty_rst_t。
//              : 2006-05-22 增加TTY_STARTGAME_RST_SUCCEED_GZS2GZS
//              : 2008-02-26 按杨纮宇的定义吧角色名长度改为32+1
//              : 2008-03-05 增加了两个LOGIN的错误提示（用来告诉用户对应的服务器工作不正常，以免用户不断尝试重连）

#ifndef	__TTY_COMMON_DEF_H__
#define	__TTY_COMMON_DEF_H__

#include	<WHCMN/inc/whcmn_def.h>
#include	<PNGS/inc/pngs_def.h>

namespace n_pngs
{

// TTY逻辑指令类型(各个逻辑模块之间通讯的指令)
typedef	pngs_cmd_t				tty_cmd_t;							// 逻辑指令（这个必须和pngs_cmd_t定义一致，否则PNGSClient和GMSMainStructure内部预先进行的指令分拣就会有问题）
typedef	int				tty_rst_t;							// 逻辑结果
typedef	n_whcmn::whint64		tty_id_t;							// 账号和角色的ID
typedef	unsigned short			tty_svrgrpid_t;						// 服务器组ID类型

//////////////////////////////////////////////////////////////////////////
// SP返回值
#define		SP_RESULT			"@result"
#define		QUERY_LEN			81920
#define		SVRGRP_DBS_IDX		30

// 用于合成ID
union	TTY_ID_UNION_T
{
	tty_id_t					id;
	struct
	{
		typedef	n_whcmn::whuint64	_T;								// 用uint是为了方便打印
		_T		incidx			: 19;								// 递增的ID部分
		_T		t				: 30;								// 时间因子
		_T		svrid			: 5;								// 大区中的单个服务器
		_T		grpid			: 9;								// 大区中的单个服务器
		_T		nouse			: 1;								// 不使用。因为MySQL中只能有63bit的整数，所以这一位没用了
	}u;
	inline void	clear()
	{
		id	= 0;
	}
};

// 常量定义
enum
{
	TTY_WHUSERNAME_LEN							= 32,				// 用于WH校验的用户名最大长度(函0结尾)
	TTY_WHPASSWORD_LEN							= 32,				// 用于WH校验的密码最大长度(函0结尾)
	TTY_ACCOUNTNAME_LONG_LEN					= 64,				// 账号名的最大长长度（只用于用户登录进来时的长度）
	TTY_ACCOUNTNAME_LEN							= 32,				// 账号名的最大长度
	TTY_ACCOUNTPASSWORD_LEN						= 256,				// 账号密码的最大长度（2009-04-28 修改，韩国要求很长）
	TTY_PASSWORD_LEN							= 32+1,				// 账号密码的长度(现在都用MD5,多出来的1用于填0)
	TTY_CHARACTERNAME_LEN						= 32,				// 角色名的最大长度
	TTY_POSITION_MARK_PROMPT_LEN				= 256,				// 位置收藏夹注释长度
	TTY_CHARACTERPASSWORD_LEN					= 32+1,				// 角色密码的最大长度
	TTY_CHARACTER_SIGNATURE_LEN					= 128+1,			// 个性签名长度 
	TTY_PAY_SERIAL_LEN							= 36,				// 交易序列号长度
	TTY_ALLIANCENAME_LEN						= 32,				// 联盟名字
	TTY_ITEM_LEN								= 32,				// 联盟名字
	TTY_DEVICE_ID_LEN							= 64,				// 设备ID长度
	TTY_MAC_LEN									= 64,				// mac地址长度
	TTY_MAX_DEPLOY_HERO_NUM						= 25,				// 最多一次部署英雄数量
	TTY_MAIL_TITLE_LEN							= 64,				// 邮件标题长度
	TTY_MAIL_TEXT_LEN							= 1024,				// 邮件内容长度
	TTY_MAIL_EXT_DATA_LEN						= 1024*6,			// 邮件附加数据大小
	TTY_ALLIANCE_MAIL_TEXT_LEN					= 1024,				// 联盟邮件长度
	TTY_ALLIANCE_LOG_TEXT_LEN					= 256,				// 联盟日志长度
	TTY_PRIVATE_LOG_TEXT_LEN					= 256,				// 私人日志长度
	TTY_RELATION_LOG_TEXT_LEN					= 256,				// 关系日志长度
	TTY_BUY_ITEM_MAX_NUM						= 1000,				// 一次最多买的数量
	TTY_OTHERNAME_LEN							= 32*3+1,			// 其他类型名称的最大长度（如itemname；2009-05-19改为97）
	TTY_CHARACTERNUM_PER_ACCOUNT				= 6,				// 每个账号最多的角色个数(包括被删除角色)
	TTY_LOGIC_DFTCHARACTERNAME_MAX				= 16,				// 逻辑上默认角色名的最大长度
	TTY_LOGIC_DFTCHARACTERNAME_MIN				= 4,				// 逻辑上默认角色名的最小长度
	TTY_MAXCARDIDLEN							= 32+1,				// 游戏卡号最大值（比如用来抽奖的卡）
	TTY_MAXCARDPASSLEN							= 32+1,				// 游戏卡密码明文最大值
	TTY_WHTESTSTR_MAX_LEN						= 128*1024,			// 客户端和服务器间测试指令的最大长度
	TTY_CHATKEY_MAX_LEN							= 256,				// 连接聊天服务器的KEY的最大长度(加密之后的)
	TTY_CHATROOMNAME_LEN						= 64,				// 聊天室名字的长度
	TTY_ORGNAME_LEN								= 32,				// 组织（家族、工会）名的最大长度
	TTY_SNWRITE_LEN								= 512,				// 玩家签名档长度
	TTY_INTRODUCTION_LEN						= 512,				// 门派简介的长度
	// 商店配置的版本串长度最大长度(不包含最后一个0结尾，所以在拷贝的时候用memcpy)
	TTY_STORECONFIG_VERSSTR_LEN					= 10,
	// 点卡交易对抗表的最大宽度
	TTY_POINT_FIGHTTABLE_WIDTH					= 10,
	// GZS相关
	TTY_MAXGZSPLAYERNUM							= 2000,				// 默认的每个GZS的最多人数
	TTY_GZSNAME_LEN								= 32,				// GZS的名字
	// 一般预留的指令缓冲的大小
	TTY_RAWBUF_SIZE								= 512*1024,
	// 最多的阵营数量
	TTY_MAX_CAMPSIDES							= 4,
	TTY_CAMPSIDE_0								= 0x01,
	TTY_CAMPSIDE_1								= 0x02,
	TTY_CAMPSIDE_2								= 0x04,
	TTY_CAMPSIDE_3								= 0x08,
	// 最高的级别
	TTY_MAX_LEVEL								= 200,
	// 最高的角色职业ID
	TTY_MAX_PRO									= 255,
	// 最高的角色形象ID
	TTY_MAX_MODEL								= 255,
	// 最高的角色职业ID
	TTY_MAX_GG_NUM								= 512,
	// 一般的transaction超时时间
	TTY_TRANSACTION_TIMEOUT						= 44*1000,			// 20秒吧（一般用户可能没有那么耐心）
	// 寻线公测的时刻
	TTY_TIME_20081027							= 1225036800,
	// billing通讯中的附加字串的长度
	TTY_BILLING_EXT_INFO_LEN					= 256,
	// GZS向GMS请求玩家在线信息时,一页的人数			
	TTY_MAX_GET_PLAYER_SVRID_MAX				= 10,
	// 快速登录动态key的长度
	QUICK_LOGIN_KEY_LENGTH						= 32,
};
// 账号属性
// 0x000000XX									GM权限
// 0x0X000000									需要保存在数据库中的用户属性
// 0xX0000000									根据其他信息生成的用户属性
enum
{
	TTY_ACCOUNTPROPERTY_ROOT					= 0x00000001,		// 系统管理员
	TTY_ACCOUNTPROPERTY_GM						= 0x00000002,		// GM
	TTY_ACCOUNTPROPERTY_ADM						= 0x00000004,		// 比GM高一些
	TTY_ACCOUNTPROPERTY_ALLGMMASK				= 0x00000007,		// 用于判断是否有GM属性。他的反可以用于清除所有GM属性。

	TTY_ACCOUNTPROPERTY_USER_LOCKED				= 0x01000000,		// 帐号被用户锁定（比如手机锁定）
	TTY_ACCOUNTPROPERTY_LOCKED					= 0x02000000,		// 帐号被锁定
	TTY_ACCOUNTPROPERTY_USER_LOCKED_COMMENT		= 0x04000000,		// 用户锁定注记（台湾首先申请使用的）
	TTY_ACCOUNTPROPERTY_IP_ESTOP				= 0x80000000,		// 帐号被IP禁言（这个不用存在数据库中，需要GMS根据用户状况填写）
};
// 账号密码模式
enum
{
	TTY_PASSMODE_MD5							= 0,				// 密码MD5；则说明后面跟着16字节的MD5
	TTY_PASSMODE_PLAIN							= 1,				// 明码；则说明是密码明码clear/plain text(带0结尾的)
};
// LOGIN结果
enum
{
	TTY_LOGIN_RST_SUCCEED						= 0x01,				// login成功
	TTY_LOGIN_RST_BADACCOUNT					= 0x02,				// 用户不存在
	TTY_LOGIN_RST_BADPASS						= 0x03,				// 密码错误
	TTY_LOGIN_RST_ALREADYINTHIS					= 0x04,				// 用户已在本服务器组在线
	TTY_LOGIN_RST_ALREADYINOTHER				= 0x05,				// 用户已在其它服务器组在线
																	// 后面会跟其他服务器组ID(tty_svrgrpid_t类型)
	TTY_LOGIN_RST_OTHEREER						= 0x06,				// billing内部的其他错误，可能是数据库连接问题（或者是内部的存储过程错误），也可能是服务器间通讯或者服务器内部问题
	TTY_LOGIN_RST_REFUSED						= 0x07,				// 被拒绝
	TTY_LOGIN_RST_LOGININPROCESS				= 0x09,				// 有相同名字的帐号正在登录过程中（2007-06-15加）
	
	TTY_LOGIN_RST_NO_DEVICEID					= 0x0A,				// 没有该DeviceID对应的账号
	TTY_LOGIN_RST_LOGININPROCESS_DEVICEID		= 0x0B,				// 有使用该DeviceID的账号正在登陆过程中
	//
	TTY_LOGIN_RST_NOCHAR						= 0x0C,				// 角色不存在
	TTY_LOGIN_RST_LOADCHAR						= 0x0E,				// 载入角色信息时出错

	TTY_LOGIN_RST_ACCOUNT_BANNED				= 0x0F,				// 账号被封
	TTY_LOGIN_RST_DEVICE_BANNED					= 0x10,				// 设备被封
	TTY_LOGIN_RST_CHAR_BANNED					= 0x11,				// 角色被封
	TTY_LOGIN_RST_GROUP_ERR						= 0x12,				// 大区不对
	// 我自己加的 0xF0~0xFF
	TTY_LOGIN_RST_DBERR							= 0xF1,				// DB无法正常使用
	TTY_LOGIN_RST_TRANSACTION_TIMEOUT			= 0xF6,				// 在transaction的某一步超时了
	TTY_LOGIN_RST_OUTOF_TRANSACTION				= 0xF7,				// 无法分配transaction了
	TTY_LOGIN_RST_SQLERR						= 0xFE,				// SQL错误
	TTY_LOGIN_RST_UNKNOWNERR					= 0xFF,				// 其他未知错误（也可能由于没有删除的用户太多，无法分配了；或者是内存，或者自动数据对象流写入错误）
};
// 创建账号结果
enum
{
	TTY_ACCOUNT_CREATE_RST_OK					= 0x00,				// 创建成功
	TTY_ACCOUNT_CREATE_RST_GS_CNTR_ERR			= 0x01,				// 和全局服的连接出错
	TTY_ACCOUNT_CREATE_RST_DB_ERR				= 0x02,				// 数据库出错(断连等)
	TTY_ACCOUNT_CREATE_RST_SQL_ERR				= 0x03,				// SQL执行出错
	TTY_ACCOUNT_CREATE_RST_TIMEOUT				= 0x04,				// 超时
	TTY_ACCOUNT_CREATE_RST_NAME_DUP				= 0x05,				// 账号已经存在
	TTY_ACCOUNT_CREATE_RST_NAME_INVALID			= 0x06,				// 名字有非法字符
	TTY_ACCOUNT_CREATE_RST_DEVICE_BANNED		= 0x07,				// 设备被封
	TTY_ACCOUNT_CREATE_RST_DEVICE_BINDED_GROUP	= 0x08,				// 设备已经在该大区绑定过了
};
// 创建角色结果
enum
{
	TTY_CHAR_CREATE_RST_OK					= 0x00,				// 创建成功
	TTY_CHAR_CREATE_RST_NAMEDUP				= 0x01,				// 名字重复
	TTY_CHAR_CREATE_RST_DB_ERR				= 0x02,				// 数据库出错(断连等)
	TTY_CHAR_CREATE_RST_SQL_ERR				= 0x03,				// SQL执行出错
	TTY_CHAR_CREATE_RST_TIMEOUT				= 0x04,				// 超时
	TTY_CHAR_CREATE_RST_ALREADYEXISTCHAR	= 0x05,				// 已经在本大区创建过角色了
	TTY_CHAR_CREATE_RST_WORLD_FULL			= 0x06,				// 世界满了
	TTY_CHAR_CREATE_RST_POSITION_DUP		= 0x07,				// 位置重复
	TTY_CHAR_CREATE_RST_NAME_INVALID		= 0x08,				// 名字有非法字符
};
// LOGIN结果
enum
{
	TTY_PRELOGIN_RST_SUCCEED					= 0x01,				// prelogin成功
	TTY_PRELOGIN_RST_BADPASS					= 0x03,				// 密码错误
	TTY_PRELOGIN_RST_DBERR						= 0xF1,				// DB无法正常使用
	TTY_PRELOGIN_RST_LBAERR						= 0xF2,				// LBA无法正常使用
	TTY_PRELOGIN_RST_GCERR						= 0xF3,				// GC模块无法正常使用
	TTY_PRELOGIN_RST_INTERSVR_DATA_ERR			= 0xF4,				// 服务器间解析数据出错
};
enum
{
	TTY_ENDGAME_RST_SUCCEED						= 0x01,				// 成功
	TTY_ENDGAME_RST_FAIL						= 0x00,				// 结束游戏失败(比如和LB连接中断)
};
enum
{
	TTY_GZS_SELECT_RST_SUCCEED					= 0x01,				// 成功
	TTY_GZS_SELECT_RST_FAIL						= 0x00,				// 失败(比如：选的GZS死机了不存在了)
};
enum
{
	TTY_REQ_FILE_RST_SUCCEED					= 0x01,				// 文件获取成功（后面应该跟着文件数据，如果没有文件数据）
	TTY_REQ_FILE_RST_SUCCEED_SAME				= 0x00,				// 文件MD5没有改变（客户端应该直接使用本地文件即可）
	TTY_REQ_FILE_RST_ERR_NOTFOUND				= 0xF0,				// 文件不存在
};
enum
{
	TTY_STORE_BUY_RST_SUCCEED					= 0x01,				// 购买成功
	TTY_STORE_BUY_RST_ERR_MONEY_NOT_ENOUGH		= 0x00,				// 点数不够
	TTY_STORE_BUY_RST_ERR_ITEM_NOT_EXIST		= 0xF0,				// 有物品不存在
	TTY_STORE_BUY_RST_ERR_REFUSED				= 0xF1,				// 操作被拒绝，可能是上一个和交易相关的操作还没有结束
	TTY_STORE_BUY_RST_ERR_ADDMSG				= 0xF2,				// 添加物品消息时出错（这个应该让用户和运营商联系一下了，因为点已经扣过了）
	TTY_STORE_BUY_RST_ERR_RETURNMONEY			= 0xF3,				// 退还点数时出错（要是这样用户就钱物两空了:(）
	TTY_STORE_BUY_RST_ERR_CLOSE					= 0xF4,				// 功能被关闭
	TTY_STORE_BUY_RST_ERR_GIFT_ITEM_NOT_EXIST	= 0xF5,				// 赠品不存在
	TTY_STORE_BUY_RST_ERR_UNKNOWN				= 0xFF,				// 未知错误

	TTY_RANKLIST_MAX							= 1000,				// 排行榜表的真实ID不能超过这个（1～96为atb表，96～499为任意表，500~699为家族排行帮）
};
enum
{
	TTY_LOTTERY_RST_SUCCEED						= 0x01,				// 兑奖成功（请注意查收奖品消息）
	TTY_LOTTERY_RST_NOLUCKY						= 0x02,				// 运气不佳，没有兑到奖
	TTY_LOTTERY_RST_USED						= 0x03,				// 卡号已经被使用过了
	TTY_LOTTERY_RST_NOTEXIST					= 0x04,				// 错误的卡号
	TTY_LOTTERY_RST_ACCOUNT_LIMIT				= 0x05,				// 帐号被限制只能领取N次
	TTY_LOTTERY_RST_CHAR_LIMIT					= 0x06,				// 角色被限制只能领取N次
	TTY_LOTTERY_RST_ERR_NET						= 0xFE,				// 网络错误
	TTY_LOTTERY_RST_ERR_UNKNOWN					= 0xFF,				// 未知错误
};
// 一些常用的atb序号的定义
enum
{
	TTY_ATB_LEVEL								= 1,				// 级别
	TTY_ATB_SCN									= 22,				// 下线场景
	TTY_ATB_CAREER								= 42,				// 职业
	TTY_ATB_MONEY								= 35,				// 金钱
	TTY_ATB_EXP									= 41,				// 当前经验
	TTY_ATB_MODEL								= 42,				// 角色系
	TTY_ATB_LEAVE_TIME							= 47,				// 上次离开时间
	TTY_ATB_SIDE								= 58,				// 角色阵营
	TTY_ATB_SVRIDX								= 59,				// 角色绑定服务器序号
	TTY_ATB_LASTSVRIDX							= 60,				// 上次下线的服务器序号
};
enum
{
	TTY_GLB_ATB_Login_Count						= 1000,				// 用户登录的计数
	TTY_GLB_ATB_BindCount_GZS0					= 1000,				// 从1001~1030，记录每个服务器的绑定用户数量（这只是用来大概统计的，如果用户有转服这个值并不会一定发生变化）
	TTY_GLB_ATB_BindCount_GZS30					= 1030,				//
};
// 组织类型定义
enum
{
	TTY_ORG_TYPE_FAMILY							= 1,				// 家族
	TTY_ORG_TYPE_GUILD							= 2,				// 工会
};
////////////////////////////////////////////////////////////////////
// 服务器和客户端需要同步的文件序号
////////////////////////////////////////////////////////////////////
enum
{
	TTY_CS_FILESYNC_IDX_STORE					= 1,				// 虚拟商城文件
	TTY_CS_FILESYNC_IDX_RANKLIST_0				= 1000,				// 排行榜文件基础id
	TTY_CS_FILESYNC_IDX_RANKLIST_POINT_0		= 1400,				// 点卡消费排行榜文件基础id（1:day	2:week	3:month	4:total）
	TTY_CS_FILESYNC_IDX_RANKLIST_POINT_END		= 1410,				// 结束
	TTY_CS_FILESYNC_IDX_RANKLIST_FAMILY_0		= 1500,				// 家族排行榜文件基础id
	TTY_CS_FILESYNC_IDX_RANKLIST_FAMILY_END		= 1599,				// 家族排行榜文件结束id
	TTY_CS_FILESYNC_IDX_RANKLIST_GUILD_0		= 1600,				// 公会排行榜文件基础id
	TTY_CS_FILESYNC_IDX_RANKLIST_GUILD_END		= 1699,				// 公会排行榜文件结束id
	TTY_CS_FILESYNC_IDX_KGRAPH_0				= 1000000,			// K线图文件基础id
	TTY_CS_FILESYNC_IDX_KGRAPH_END				= 1999999,			// K线图文件结束id
																	// 注：K线图文件名的结构为（1GGGWWW，其中GGG代表三位的大区ID，WWW代表三位的周序号（从公测开始（20081027）为第0周））
																	// 除了当天图（2分钟可以重新提交一次请求），其他所有的图如果存在就不重新向LBA提交请求
};

// 时间获得对应的星期（2008-10-27 00:00:00，1225036800，作为起始点）
inline int	TTYGetWeekByTime(time_t t)
{
	return	(t-TTY_TIME_20081027)/(3600*24*7);
}
// 根据Week获取起始和结束时刻
inline void	TTYGetWeekT0T1(int nWeek, time_t &nT0, time_t &nT1)
{
	nT0	= TTY_TIME_20081027 + nWeek*7*24*3600;
	nT1	= nT0 + 7*24*3600;
}
// 从KGRAPH文件ID中获取对应的星期和大区号
inline void	TTYGetGroupIDAndWeekFromKGRAPHID(int nID, int *pnGroupID, int *pnWeek)
{
	*pnGroupID	= (nID - TTY_CS_FILESYNC_IDX_KGRAPH_0) / 1000;
	*pnWeek		= nID % 1000;
}
// 通过大区号和星期生成KGRAPH文件ID
inline int	TTYMakeKGRAPHID(int nGroupID, int nWeek)
{
	return	TTY_CS_FILESYNC_IDX_KGRAPH_0 + nGroupID*1000 + nWeek;
}

// 腾讯的用户身份定义位
enum
{
	TX_ID_CLUB		= 0x00,		//会员身份
	TX_ID_163S		= 0x01,		//163固定帐号用户(20040920改为20元6位靓号标志)
	TX_ID_163C		= 0x02,		//163上网卡用户
	TX_ID_263U		= 0x03,		//263拨号用户
	TX_ID_MCLB		= 0x04,		//移动会员
	TX_ID_TCNT		= 0x05,		//腾讯员工
	TX_ID_168U		= 0x06,		//168托收用户
	TX_ID_QQXP		= 0x07,		//普通QQ行用户
	TX_ID_QQXM		= 0x08,		//手机QQ行用户
	TX_ID_QLVP		= 0x09,		//Q交友高级用户（预付费）
	TX_ID_QLVM		= 0x0A,		//Q交友高级用户（手机）
	TX_ID_SMSE		= 0x0B,		//短信易用户
	TX_ID_ECARD		= 0x0C,		//QQ贺卡用户
	TX_ID_GAME		= 0x0D,		//GAME会员
	TX_ID_QQHOME	= 0x0E,		//QQHOME会员
	TX_ID_QQSHOW	= 0x0F,		//QQSHOW包月用户
	TX_ID_QQTANG	= 0x10,		//QQTANG用户
	TX_ID_VNETCLB	= 0x11,		//Vnet会员用户
	TX_ID_OMUSIC	= 0x12,		//在线音乐
	TX_ID_MUSIC		= 0x13,		//QQ音乐
	TX_ID_MGFACE	= 0x14,		//魔法表情
	TX_ID_GAMEK		= 0x15,		//游戏的王冠
	TX_ID_RING		= 0x16,		//炫铃
	TX_ID_SHOW30	= 0x17,		//30元QQSHOW包月用户
	TX_ID_ICOKE		= 0x18,		//3DShow(
	TX_ID_QQDNA		= 0x19,		//QQDNA项目
	TX_ID_QQNOTICE	= 0x1A,		//QQNOTICE
	TX_ID_MUSICJL	= 0x1B,		//QQMUSIC
	TX_ID_QQPET		= 0x1C,		//宠物粉钻
	TX_ID_QQSPEED	= 0x1D,		//飞车紫钻
	TX_ID_QQXU		= 0x1E,		//QQ行号码
	TX_ID_QQXL		= 0x1F,		//QQ行号码允许登录
	TX_ID_DNF		= 0x20,		// DNF 包月用户
	TX_ID_CF		= 0x21,		//CF包月服务
	TX_ID_XX		= 0x24,		//寻仙VIP（2009-04-21才知道）对应的bit为：0x1000000000

	TX_ID_MAX
};

}		// EOF namespace n_pngs

// 服务器的
#define	TTY_DLL_NAME_GS_DB						"uGS_DB"			// 和DB连接的模块
#define	TTY_DLL_NAME_GS_GC						"uGS_GC_TTY"		// 和全局控制服务器连接的模块
#define	TTY_DLL_NAME_GS_POINTSTOCK				"uGS_POINTSTOCK"	// 进行点卡交易的模块
#define	TTY_DLL_NAME_GS_QPoint					"uGS_QPoint"		// Q点、Q币相关的模块
#define	TTY_DLL_NAME_GS_Auth					"uGS_Auth"			// 身份验证
#define	TTY_DLL_NAME_LBA						"uLBA"				// LBA主逻辑
#define	TTY_DLL_NAME_GMLS						"uGMLS"				// 运维业务系统本地支撑
#define	TTY_DLL_NAME_LIS						"uLIS"				// 本地信息系统（比如帐号、卡号抽奖之类的）
#define	TTY_DLL_NAME_VERIFYCODE					"uVERIFYCODE"		// 验证码系统服务端
#define	TTY_DLL_NAME_VERIFYCODECLIENT			"uVERIFYCODECLIENT"	// 验证码系统连接模块
#define	TTY_DLL_NAME_APEXANTIHACKER				"uAPEXANTIHACKER"		// APEX反外挂系统连接模块
#define	TTY_DLL_NAME_XTRAPANTIHACKER			"uXTRAPANTIHACKER"	// XTRAP反外挂系统连接模块
#define	TTY_DLL_NAME_MALAY_Auth					"uMALAY_Auth"		// 马来西亚登陆验证LBA插件
#define	TTY_DLL_NAME_GLVMALAY					"uGLVMALAY"			// 马来西亚登陆验证服务
#define TTY_DLL_NAME_BBS						"uBBS"				// 公告板模块
#define	TTY_DLL_NAME_GS_DB4Web					"uGS_DB4Web"		// 和DBS4Web连接的模块,added by yuezhongyue 2011-10-12
#define TTY_DLL_NAME_LP_MainStructure			"uLPMainStructure"	// LP的底层连接处理模块,added by yuezhongyue 2011-11-24
#define TTY_DLL_NAME_LP_GamePlay				"uLPGamePlay"		// LP的逻辑层,added by yuezhongyue 2011-11-24
#define TTY_DLL_NAME_Postman4Web				"uPostman4Web"		// 和LP通信的模块(给非web的游戏进程使用),added by yuezhongyue 2011-11-24
#define TTY_DLL_NAME_GS4Web						"uGS4Web"			// 全局服务器,added by yuezhongyue 2012-03-14
#define TTY_DLL_NAME_BD4Web						"uBD4Web"			// 业务受理模块,added by yuezhongyue 2012-03-14
#define TTY_DLL_NAME_QQTIPS						"uQQTIPS"			// QQTIPS模块
// 客户端的
#define	TTY_DLL_NAME_GC_CHAT					"uGC_CHAT_TTY"		// 客户端的聊天模块
#define	TTY_DLL_NAME_GC_ASSIST					"uGC_ASSIST_TTY"	// 客户端辅助模块（一些杂项功能在这里面）

#define	TTY_EMPTY_MD5							"d41d8cd98f00b204e9800998ecf8427e"
																	// 空字串的md5码

#endif	// EOF __TTY_COMMON_DEF_H__
