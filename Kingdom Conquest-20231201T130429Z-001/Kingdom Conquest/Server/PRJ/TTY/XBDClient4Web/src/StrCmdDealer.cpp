#include "../inc/StrCmdDealer.h"

int		CStrCmdDealer::_i_srad_NULL(const char* param)
{
	printf("UNKNOWN CMD:%s!%s", m_srad.m_szUnknownCmd, WHLINEEND);
	return 0;
}
int		CStrCmdDealer::_i_srad_help(const char* param)
{
	char	szBuf[4*1024];
	printf("%s%s", m_srad.Help(szBuf, sizeof(szBuf), param), WHLINEEND);
	return 0;
}
int		CStrCmdDealer::_i_srad_exit(const char* param)
{
	g_bStop	= true;
	return 0;
}
int		CStrCmdDealer::_i_srad_notify_add(const char* param)
{
	char	szTitle[MAX_BD_TEXT_LEN]	= "";
	char	szContent[MAX_BD_TEXT_LEN]	= "";
	char	szTimeStr[MAX_BD_TIMESTR_LEN]	= "";
	int		nGroupID			= 0;
	int		nTitleLen			= 0;
	int		nContentLen			= 0;

	const char*	pPos			= param;
	char*		szContentBuf	= szContent;
	for (int i=0; i<3; i++)
	{
		if ((pPos=strchr(pPos, (int)',')) == NULL)
		{
			break;
		}
		else
		{
			pPos++;
			if (i == 2)
			{
				while (pPos[0] != 0)
				{
					if (strchr(WHSTR_WHITESPACE, (int)pPos[0]) != NULL)
					{
						szContentBuf[0]		= pPos[0];
						szContentBuf++;
						pPos++;
					}
					else
					{
						break;
					}
				}
				break;
			}
		}
	}

	wh_strsplit("dssa", param, ",", &nGroupID, szTimeStr, szTitle, szContentBuf);

	nTitleLen					= strlen(szTitle) + 1;
	nContentLen					= strlen(szContent) + 1;

	m_vectrawbuf.resize(sizeof(BD_CMD_ADD_NOTIFY_T) + nTitleLen + sizeof(int) + nContentLen);
	BD_CMD_ADD_NOTIFY_T*	pCmd	= (BD_CMD_ADD_NOTIFY_T*)m_vectrawbuf.getbuf();
	pCmd->nSize					= m_vectrawbuf.size();
	pCmd->nCmd					= BD_CMD_ADD_NOTIFY;
	pCmd->nGroupID				= nGroupID;
	WH_STRNCPY0(pCmd->szTimeStr, szTimeStr);
	pCmd->nTitleLen				= nTitleLen;
	char*	pTitle				= (char*)wh_getptrnexttoptr(pCmd);
	memcpy(pTitle, szTitle, nTitleLen);
	int*	pContentLen			= (int*)wh_getoffsetaddr(pTitle, nTitleLen);
	*pContentLen				= nContentLen;
	memcpy(wh_getptrnexttoptr(pContentLen), szContent, nContentLen);

	g_pMyMsger->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_del_id(const char* param)
{
	int		nNotificationID		= -1;
	wh_strsplit("d", param, "", &nNotificationID);
	BD_CMD_DEL_NOTIFY_BY_ID_T	Cmd;
	Cmd.nSize					= sizeof(Cmd);
	Cmd.nCmd					= BD_CMD_DEL_NOTIFY_BY_ID;
	Cmd.nNotificationID			= nNotificationID;

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_del_bt(const char* param)
{
	char	szTimeStr[MAX_BD_TIMESTR_LEN];
	wh_strsplit("s", param, ",", szTimeStr);
	BD_CMD_DEL_NOTIFY_BEFORE_TIME_T	Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_DEL_NOTIFY_BEFORE_TIME;
	WH_STRNCPY0(Cmd.szTimeStr, szTimeStr);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_del_at(const char* param)
{
	char	szTimeStr[MAX_BD_TIMESTR_LEN];
	wh_strsplit("s", param, ",", szTimeStr);
	BD_CMD_DEL_NOTIFY_AFTER_TIME_T	Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_DEL_NOTIFY_AFTER_TIME;
	WH_STRNCPY0(Cmd.szTimeStr, szTimeStr);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_del_all(const char* param)
{
	BD_CMD_DEL_NOTIFY_ALL_T	Cmd;
	Cmd.nSize				= sizeof(Cmd);
	Cmd.nCmd				= BD_CMD_DEL_NOTIFY_ALL;

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_del_group(const char* param)
{
	BD_CMD_DEL_NOTIFY_BY_GROUP_T	Cmd;
	Cmd.nSize				= sizeof(Cmd);
	Cmd.nCmd				= BD_CMD_DEL_NOTIFY_BY_GROUP;
	wh_strsplit("d", param, "", &Cmd.nGroupID);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_get_id(const char* param)
{
	BD_CMD_GET_NOTIFY_BY_ID_T		Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_GET_NOTIFY_BY_ID;
	wh_strsplit("d", param, "", &Cmd.nNotificationID);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_get_bt(const char* param)
{
	BD_CMD_GET_NOTIFY_BEFORE_TIME_T		Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_GET_NOTIFY_BEFORE_TIME;
	wh_strsplit("s", param, ",", Cmd.szTimeStr);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_get_at(const char* param)
{
	BD_CMD_GET_NOTIFY_AFTER_TIME_T		Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_GET_NOTIFY_AFTER_TIME;
	wh_strsplit("s", param, ",", Cmd.szTimeStr);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_get_all(const char* param)
{
	BD_CMD_GET_NOTIFY_ALL_T		Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_GET_NOTIFY_ALL;

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_notify_get_group(const char* param)
{
	BD_CMD_GET_NOTIFY_BY_GROUP_T		Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_GET_NOTIFY_BY_GROUP;
	wh_strsplit("d", param, ",", &Cmd.nGroupID);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_recharge_diamond(const char* param)
{
	unsigned int	nGroupID		= 0;
	tty_id_t		nAccountID		= 0;
	unsigned int	nAddedDiamond	= 0;

	wh_strsplit("dId", param, ",", &nGroupID, &nAccountID, &nAddedDiamond);

	BD_CMD_RECHARGE_DIAMOND_T	Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_RECHARGE_DIAMOND;
	Cmd.nGroupID	= nGroupID;
	Cmd.nAccountID	= nAccountID;
	Cmd.nAddedDiamond	= nAddedDiamond;

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_reload_excel_table(const char* param)
{
	unsigned int	nGroupID		= 0;
	wh_strsplit("d", param, ",", &nGroupID);

	BD_CMD_RELOAD_EXCEL_TABLE_T	Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_RELOAD_EXCEL_TABLE;
	Cmd.nGroupID	= nGroupID;

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));

	return 0;
}
int		CStrCmdDealer::_i_srad_kick_char(const char* param)
{
	BD_CMD_KICK_CHAR_T	Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_KICK_CHAR;
	Cmd.nAccountID	= 0;
	Cmd.nGroupID	= 0;
	wh_strsplit("dI", param, ",", &Cmd.nGroupID, &Cmd.nAccountID);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));
	return 0;
}
int		CStrCmdDealer::_i_srad_ban_char(const char* param)
{
	BD_CMD_BAN_CHAR_T	Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_BAN_CHAR;
	Cmd.nAccountID	= 0;
	Cmd.nGroupID	= 0;
	wh_strsplit("dI", param, ",", &Cmd.nGroupID, &Cmd.nAccountID);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));
	return 0;
}
int		CStrCmdDealer::_i_srad_ban_account(const char* param)
{
	BD_CMD_BAN_ACCOUNT_T	Cmd;
	Cmd.nSize		= sizeof(Cmd);
	Cmd.nCmd		= BD_CMD_BAN_ACCOUNT;
	Cmd.nAccountID	= 0;
	Cmd.bBanDevice	= false;
	wh_strsplit("Ib", param, ",", &Cmd.nAccountID, &Cmd.bBanDevice);

	g_pMyMsger->SendMsg(&Cmd, sizeof(Cmd));
	return 0;
}
int		CStrCmdDealer::_i_srad_kick_client_all(const char* param)
{
	char			szBuf[1024]	= "";
	unsigned int	nGroupID	= 0;
	wh_strsplit("da", param, ",", &nGroupID, szBuf);
	int		nLength		= strlen(szBuf)+1;
	m_vectrawbuf.resize(sizeof(BD_CMD_KICK_CLIENT_ALL_T) + nLength);
	BD_CMD_KICK_CLIENT_ALL_T*	pCmd	= (BD_CMD_KICK_CLIENT_ALL_T*)m_vectrawbuf.getbuf();
	pCmd->nSize			= m_vectrawbuf.size();
	pCmd->nCmd			= BD_CMD_KICK_CLIENT_ALL;
	pCmd->nGroupID		= nGroupID;
	pCmd->nLength		= nLength;
	memcpy(wh_getptrnexttoptr(pCmd), szBuf, nLength);

	g_pMyMsger->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
