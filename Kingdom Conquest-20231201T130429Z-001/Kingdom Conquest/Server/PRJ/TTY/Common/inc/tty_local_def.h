// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// File         : tty_local_def.h
// Creator      : liuyi
// Comment      : 定义各个国家区域的宏，头文件，lib等
// CreationDate : 2009-05-14
// Change LOG   : 

#ifndef	__TTY_LOCAL_DEF_H__
#define	__TTY_LOCAL_DEF_H__


// 理论上一个版本，只应该设置一个区域，其他的注掉即可

/////////////////////////////////////////////////////////////////////////////
// 大陆寻仙相关的宏

#define LOCAL_CN

#ifdef LOCAL_CN
		#define ANTIBOT_TX			// 腾讯反外挂
		#define RS_TX				// 腾讯的QQ状态通知接口
		#define TQOS_TX				// 腾讯的TQOS信息上报系统
		#define	CROSS_TX			// 腾讯的Cross系统
		#define TCLS_TX				// 腾讯的TCLS登陆系统
        #define TALK_STRS     "|%d|%s|%s|%s|%d|%s|%s"			// 腾讯聊天使用”|“作为分隔符
		#define TALK_STRD     "|%d|%s|%s|%s|%d|%d|%s"			// 腾讯聊天使用”|“作为分隔符
		#define PRIMSG_STR		"|%s|%d|0x%"INT64PRFX"x|%s|%s|%s|%s|%d|%d" //邮件使用“|”作为日志分隔符
		#define AHMSG_STR      "|%s|||%s|%s||||%d|0x%"INT64PRFX"x" 
		#define RENAME_STR		"|%s||0x%"INT64PRFX"x|%s|%s||||%d|%d"
        //#define TDIRTY_TX
#endif

#ifndef LOCAL_CN
//默认的聊天日志格式，腾讯会特别DEFINE
#define TALK_STRS     "%d,%s,%s,%s,%d,%s,%s"			// 聊天使用”,“作为分隔符
#define TALK_STRD     "%d,%s,%s,%s,%d,%d,%s"			// 聊天使用”,“作为分隔符  
#define PRIMSG_STR		"%s,%d,0x%"INT64PRFX"x,%s,%s,%s,%s,%d,%d" //邮件使用“，”作为日志分隔符
#define AHMSG_STR      "%s,,,%s,%s,,,,%d,0x%"INT64PRFX"x" 
#define RENAME_STR		"%s,,0x%"INT64PRFX"x,%s,%s,,,,%d,%d"
#endif
/////////////////////////////////////////////////////////////////////////////
// 台湾寻仙相关的宏

//#define LOCAL_TW

	#ifdef LOCAL_TW
		#define APEX_XX				// APEX反外挂
	#endif

/////////////////////////////////////////////////////////////////////////////
// 香港寻仙相关的宏

//#define LOCAL_HK

	#ifdef LOCAL_HK
		#define APEX_XX				// APEX反外挂
	#endif

/////////////////////////////////////////////////////////////////////////////
// 韩国寻仙相关的宏

//#define LOCAL_KR

	#ifdef LOCAL_KR
		#define X_TRAP_CJ			// 韩国反外挂
		#define VER_KR_LOGIN		// 韩国登录相关接口
		#define WEBSHOP_CJ			// 韩国商城IE界面（需要VER_KR_LOGIN相关的支持）
	#endif

/////////////////////////////////////////////////////////////////////////////
// 马来西亚寻仙相关的宏

//#define LOCAL_MY

	#ifdef LOCAL_MY
		#define APEX_XX				// APEX反外挂
	#endif

/////////////////////////////////////////////////////////////////////////////
// 欧美寻仙相关的宏

//#define LOCAL_US
	#ifdef LOCAL_US
		#define SOCIALITY_MAKEFRIENDS //北美加好友功能
	#endif

////////////////////////////////////////////////////////////////////////////

#endif	// __TTY_LOCAL_DEF_H__
