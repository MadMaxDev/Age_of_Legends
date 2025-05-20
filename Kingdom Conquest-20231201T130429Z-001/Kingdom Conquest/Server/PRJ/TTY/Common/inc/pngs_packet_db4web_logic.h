#ifndef __PNGS_PACKET_DB4WEB_LOGIC_H_
#define	__PNGS_PACKET_DB4WEB_LOGIC_H_

#include "GameUtilityLib/inc/DMtype.h"
#include "./tty_common_def.h"

enum
{
	GSDB4Web_VER							= 1,
};
enum
{
	PNGSPACKET_2DB4WEB_EXT_NUM				= 4,				// ext整型数据的个数
};
//for DB 3300~3499,大指令下需要细分,不然不够用
enum
{
	PNGSPACKET_2DB4WEB_0					= 3300,
	// (立即执行)配置
	PNGSPACKET_2DB4WEB_CONFIG				= PNGSPACKET_2DB4WEB_0 + 1,
	// 发送给DB的请求(这里做了二级指令,主要是为了应付可能无限扩张的数据库请求)
	PNGSPACKET_2DB4WEB_REQ					= PNGSPACKET_2DB4WEB_0 + 2,				// 二级指令见tty_def_dbs4web.h
};
//for DB user 3300~3499,大指令下需要细分,不然不够用
enum
{
	PNGSPACKET_2DB4WEBUSER_0				= 3300,
	PNGSPACKET_2DB4WEBUSER_RPL				= PNGSPACKET_2DB4WEBUSER_0 + 2,
	PNGSPACKET_2DB4WEBUSER_READY4WORK		= PNGSPACKET_2DB4WEBUSER_0 + 3,			// 连接完成,准备工作了
};

namespace n_pngs
{
#pragma pack(push, old_pack_num, 1)
//////////////////////////////////////////////////////////////////////////
// 2DB
//////////////////////////////////////////////////////////////////////////
struct PNGSPACKET_2DB4WEB_CONFIG_T 
{
	enum
	{
		SUBCMD_SET_DBUSER				= 1,
		SUBCMD_SET_DBUSERISWHAT			= 2,
	};
	int					nSubCmd;
	int					nParam;
};
#pragma pack(pop, old_pack_num)

}
#endif
