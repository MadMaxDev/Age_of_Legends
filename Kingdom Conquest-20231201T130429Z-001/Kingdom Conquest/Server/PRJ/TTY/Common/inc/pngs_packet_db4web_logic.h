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
	PNGSPACKET_2DB4WEB_EXT_NUM				= 4,				// ext�������ݵĸ���
};
//for DB 3300~3499,��ָ������Ҫϸ��,��Ȼ������
enum
{
	PNGSPACKET_2DB4WEB_0					= 3300,
	// (����ִ��)����
	PNGSPACKET_2DB4WEB_CONFIG				= PNGSPACKET_2DB4WEB_0 + 1,
	// ���͸�DB������(�������˶���ָ��,��Ҫ��Ϊ��Ӧ�������������ŵ����ݿ�����)
	PNGSPACKET_2DB4WEB_REQ					= PNGSPACKET_2DB4WEB_0 + 2,				// ����ָ���tty_def_dbs4web.h
};
//for DB user 3300~3499,��ָ������Ҫϸ��,��Ȼ������
enum
{
	PNGSPACKET_2DB4WEBUSER_0				= 3300,
	PNGSPACKET_2DB4WEBUSER_RPL				= PNGSPACKET_2DB4WEBUSER_0 + 2,
	PNGSPACKET_2DB4WEBUSER_READY4WORK		= PNGSPACKET_2DB4WEBUSER_0 + 3,			// �������,׼��������
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
