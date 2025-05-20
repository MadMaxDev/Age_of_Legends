#ifndef __StrCmdDealer_H__
#define __StrCmdDealer_H__

#include "WHCMN/inc/whcmd.h"
#include "WHCMN/inc/whvector.h"

#include "../../Common/inc/tty_common_BD4Web.h"

#include "MyMsger.h"

using namespace n_whcmn;

extern	MyMsger*	g_pMyMsger;
extern	bool		g_bStop;

class CStrCmdDealer
{
private:
	// 用于组装较长的指令
	whvector<char>	m_vectrawbuf;
public:
	CStrCmdDealer()
	{
		m_vectrawbuf.reserve(MAX_BD_CMD_SIZE);
		// 字串处理函数
		whcmn_strcmd_reganddeal_REGCMD_m_srad(NULL, _i_srad_NULL, NULL);
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(help, "[cmd]");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(exit, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_add, "group_id,time,title,notification // time format:YYYY-MM-DD[ HH:MM:SS]");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_del_id, "notification_id");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_del_bt, "time// delete notification before time,time format:YYYY-MM-DD[ HH:MM:SS]");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_del_at, "time// delete notification after time,time format:YYYY-MM-DD[ HH:MM:SS]");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_del_all, "");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_del_group, "group_id");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_get_id, "notification_id");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_get_bt, "time// get notification before time,time format:YYYY-MM-DD[ HH:MM:SS]");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_get_at, "time// get notification after time,time format:YYYY-MM-DD[ HH:MM:SS]");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_get_all, "");
 		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(notify_get_group, "group_id");

		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(recharge_diamond, "group_id,account_id,added_diamond");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(reload_excel_table, "group_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(kick_char, "group_id,account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(ban_char, "group_id,account_id");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(ban_account, "account_id,ban_device");
		whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(kick_client_all, "group_id,appid*version,...,appid*version");
	}
	whcmn_strcmd_reganddeal_DECLARE_m_srad(CStrCmdDealer);
	int		_i_srad_NULL(const char* param);
	int		_i_srad_help(const char* param);
	int		_i_srad_exit(const char* param);
	int		_i_srad_notify_add(const char* param);
	int		_i_srad_notify_del_id(const char* param);
	int		_i_srad_notify_del_bt(const char* param);
	int		_i_srad_notify_del_at(const char* param);
	int		_i_srad_notify_del_all(const char* param);
	int		_i_srad_notify_del_group(const char* param);
	int		_i_srad_notify_get_id(const char* param);
	int		_i_srad_notify_get_bt(const char* param);
	int		_i_srad_notify_get_at(const char* param);
	int		_i_srad_notify_get_all(const char* param);
	int		_i_srad_notify_get_group(const char* param);
	int		_i_srad_recharge_diamond(const char* param);
	int		_i_srad_reload_excel_table(const char* param);
	int		_i_srad_kick_char(const char* param);
	int		_i_srad_ban_char(const char* param);
	int		_i_srad_ban_account(const char* param);
	int		_i_srad_kick_client_all(const char* param);
};

#endif
