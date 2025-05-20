// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : tty_def_transaction_cmd.h
// Creator      : Wei Hua (κ��)
// Comment      : ����transaction������Ҫ���õ�ָ���
// CreationDate : 2007-06-21
// ChangeLog    :

#ifndef	__TTY_DEF_TRANSACTION_CMD_H__
#define	__TTY_DEF_TRANSACTION_CMD_H__

#include "tty_common_def.h"

namespace n_pngs
{

////////////////////////////////////////////////////////////
enum
{
	TTY_TC_0							= 0,
	TTY_TC_INIT							= TTY_TC_0 + 0,				// Transaction�����ʼ��������Ϊ����transaction�Լ������
	TTY_TC_P_LBA_I_RPL_LOGIN_VERIFY		= TTY_TC_0 + 1,				// LBA��LOGIN_VERIFY���ء�����ΪP_LBA_I_RPL_LOGIN_VERIFY_T
	TTY_TC_P_LBA_I_RPL_LOGIN			= TTY_TC_0 + 2,				// LBA��LOGIN���ء�����ΪP_LBA_I_RPL_LOGIN_T
	TTY_TC_P_LBA_I_RPL_LOGOUT			= TTY_TC_0 + 3,				// LBA��LOGOUT���ء�����ΪP_LBA_I_RPL_LOGOUT_T
	TTY_TC_P_LBA_I_RPL_STARTGAME		= TTY_TC_0 + 4,				// LBA��STARTGAME���ء�����ΪP_LBA_I_RPL_STARTGAME_T
	TTY_TC_PLAYER_DROP					= TTY_TC_0 + 5,				// ��Transaction�������û������ˡ�û�в�����
	TTY_TC_PLAYER_NORESPONSE			= TTY_TC_0 + 6,				// ��Transaction�������û�û����Ӧ��
	TTY_TC_PLAYER_VERIFYCODE_CHECK		= TTY_TC_0 + 7,				// �û�����verifycode�Ľ��
	TTY_TC_PLAYER_VERIFYCODE_CHANGE		= TTY_TC_0 + 8,				// �û�ˢ����֤��
	TTY_TC_PLAYER_VERIFYCODE_CANCEL		= TTY_TC_0 + 9,				// �û�ȡ����֤
	TTY_TC_PLAYER_STARTGAME				= TTY_TC_0 + 10,			// �û���ʼ��Ϸ
};
// ָ�����ݽṹ
#pragma pack(push, old_pack_num, 1)
////////////////////////////////////////////////////////////
// ����ָ��
////////////////////////////////////////////////////////////
#pragma pack(pop, old_pack_num)

}		// EOF namespace n_pngs

#endif	// EOF __TTY_DEF_TRANSACTION_CMD_H__
