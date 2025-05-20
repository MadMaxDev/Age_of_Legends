#ifndef __tty_lp_client_packet_H__
#define __tty_lp_client_packet_H__

#include "WHCMN/inc/whcmn_def.h"

#include "tty_common_def.h"
#include "tty_common_Base.h"

using namespace n_whcmn;

namespace n_pngs
{
//////////////////////////////////////////////////////////////////////////
// ָ���
//////////////////////////////////////////////////////////////////////////
// CLIENT->LP
enum
{
	// ��¼ָ��
	TTY_CLIENT_LPGAMEPLAY_LOGIN				= 0x03,
	// ������ɫ
	TTY_CLIENT_LPGAMEPLAY_CREATECHAR		= 0x04,
	// �����˺�
	TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT		= 0x05,
	// ��Ϸ��ʹ�õ�ָ��
	TTY_CLIENT_LPGAMEPLAY_GAME_CMD			= 0x06,
	// �ǳ�
	TTY_CLIENT_LPGAMEPLAY_LOGOUT			= 0x07,
	// ͨ���豸ID��½
	TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID	= 0x08,
};
// LP->CLIENT
enum
{
	// ��¼���
	TTY_LPGAMEPLAY_CLIENT_LOGIN_RST			= 0x03,
	// ������ɫ
	TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST	= 0x04,
	// �����˺�
	TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST	= 0x05,
	// ��Ϸָ��
	TTY_LPGAMEPLAY_CLIENT_GAME_CMD			= 0x06,
	// �ǳ�����
	TTY_LPGAMEPLAY_CLIENT_LOGOUT_RST		= 0x07,
	// ͨ���豸ID��½
	TTY_LPGAMEPLAY_CLIENT_LOGIN_BY_DEVICEID	= 0x08,
	// ֪ͨ
	TTY_LPGAMEPLAY_CLIENT_NOTIFY			= 0x20,
};
#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
// CLINET->LP
//////////////////////////////////////////////////////////////////////////
struct TTY_CLIENT_LPGAMEPLAY_LOGIN_T
{
	tty_cmd_t			nCmd;
	char				szAccount[TTY_ACCOUNTNAME_LEN];
	unsigned int		nDeviceType;
	unsigned int		nAppID;
	char				szDeviceID[TTY_DEVICE_ID_LEN];
	int					nPassLen;
	char				szPass[1];
};
struct TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT_T
{
	tty_cmd_t			nCmd;
	char				szAccountName[TTY_ACCOUNTNAME_LEN];
	char				szPass[TTY_ACCOUNTPASSWORD_LEN];
	char				szDeviceID[TTY_DEVICE_ID_LEN];
	unsigned int		nDeviceType;
	bool				bUseRandomName;
	unsigned int		nAppID;							// ��ϷID
};
struct TTY_CLIENT_LPGAMEPLAY_CREATECHAR_T 
{
	tty_cmd_t			nCmd;
	char				szName[TTY_CHARACTERNAME_LEN];
	int					nSex;							// �Ա�
	int					nHeadID;						// ͷ��
	int					nCountry;						// ����
};
struct TTY_CLIENT_LPGAMEPLAY_GAME_CMD_T 
{
	tty_cmd_t			nCmd;
	unsigned int		nGameCmd;
};
struct TTY_CLIENT_LPGAMEPLAY_LOGOUT_T 
{
	tty_cmd_t			nCmd;
};
struct TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID_T
{
	tty_cmd_t			nCmd;
	char				szDeviceID[TTY_DEVICE_ID_LEN];
	unsigned int		nDeviceType;
	unsigned int		nAppID;
};
//////////////////////////////////////////////////////////////////////////
// LP->CLIENT
//////////////////////////////////////////////////////////////////////////
struct TTY_LPGAMEPLAY_CLIENT_LOGIN_RST_T 
{
	tty_cmd_t			nCmd;
	tty_rst_t			nRst;
	char				szDeviceID[TTY_DEVICE_ID_LEN];
	char				szAccount[TTY_ACCOUNTNAME_LEN];
	char				szPass[TTY_PASSWORD_LEN];

	TTY_LPGAMEPLAY_CLIENT_LOGIN_RST_T()
	{
		szDeviceID[0]	= 0;
		szAccount[0]	= 0;
		szPass[0]		= 0;
	}
};
struct TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST_T 
{
	tty_cmd_t			nCmd;
	tty_rst_t			nRst;
};
struct TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST_T 
{
	tty_cmd_t			nCmd;
	tty_rst_t			nRst;
	char				szAccount[TTY_ACCOUNTNAME_LEN];
};
struct TTY_LPGAMEPLAY_CLIENT_GAME_CMD_T 
{
	tty_cmd_t			nCmd;
	unsigned int		nGameCmd;
};
struct TTY_LPGAMEPLAY_CLIENT_LOGOUT_RST_T
{
	enum
	{
		RST_OK			= 0,
	};
	tty_cmd_t			nCmd;
	unsigned char		nRst;
};
enum
{
	WEB_NOTIFY_WHAT_OTHERTRYREPLACE		= 1,		// �����˳�������˺ŵ�¼
};
struct TTY_LPGAMEPLAY_CLIENT_NOTIFY_T 
{
	tty_cmd_t			nCmd;
	int					nWhat;			// ��TTY_CLIENT_NOTIFY_WHAT_OTHERTRYREPLACE_WEB
};
struct TTY_LPGAMEPLAY_CLIENT_NOTIFY_OTHERTRYREPLACE_T : public TTY_LPGAMEPLAY_CLIENT_NOTIFY_T
{
	unsigned int		nFromIP;
};
#pragma pack()
}

#endif
