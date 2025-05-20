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
int		CStrCmdDealer::_i_srad_connect(const char* param)
{
	wh_strsplit("d", param, "", &g_nTermType);
	CLIENT_CAAFS4Web_EXTINFO_T	extInfo;
	extInfo.nTermType			= g_nTermType;
	g_pEpollConnecterClient->SetExtData(&extInfo, sizeof(extInfo));
	int	nRst	= g_pEpollConnecterClient->Init(g_pEpollConnecterClient->GetInfo());
	if (nRst < 0)
	{
		printf("epoll_connecter_client.Init err,rst:%d%s", nRst, WHLINEEND);
		return -1;
	}
	return 0;
}
int		CStrCmdDealer::_i_srad_login(const char* param)
{
	char	szBuf[1024];
	char	szAccount[TTY_ACCOUNTNAME_LEN]		= "";
	char	szPassword[TTY_ACCOUNTPASSWORD_LEN]	= "";
	unsigned int	nDeviceType					= 0;
	unsigned int	nAppID						= 0;
	wh_strsplit("sdsd", param, ",", szAccount, &nDeviceType, szPassword, &nAppID);
	wh_char2utf8(szAccount, strlen(szAccount), szBuf, sizeof(szBuf));
	int		nPassLen		= szPassword[0]?strlen(szPassword):1;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(TTY_CLIENT_LPGAMEPLAY_LOGIN_T, szPass) + nPassLen + 1);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	TTY_CLIENT_LPGAMEPLAY_LOGIN_T*	pLogin	= (TTY_CLIENT_LPGAMEPLAY_LOGIN_T*)pCCData->data;
	pLogin->nCmd			= TTY_CLIENT_LPGAMEPLAY_LOGIN;
	WH_STRNCPY0(pLogin->szAccount, szBuf);
	pLogin->nPassLen		= nPassLen + 1;
	strcpy(pLogin->szPass, szPassword);
	pLogin->szPass[nPassLen]	= 0;
	pLogin->nDeviceType		= nDeviceType;
	pLogin->nAppID			= nAppID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_login_device(const char* param)
{
	char	szDeviceID[TTY_DEVICE_ID_LEN]		= "";
	unsigned int	nDeviceType					= 0;
	unsigned int	nAppID						= 0;
	wh_strsplit("sdd", param, ",", szDeviceID, &nDeviceType, &nAppID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID_T*	pLogin	= (TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID_T*)pCCData->data;
	pLogin->nCmd			= TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID;
	WH_STRNCPY0(pLogin->szDeviceID, szDeviceID);
	pLogin->nDeviceType		= nDeviceType;
	pLogin->nAppID			= nAppID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_exit(const char* param)
{
	g_bStop	= true;
	return 0;
}
int		CStrCmdDealer::_i_srad_notify_get(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLIENT_CLS4Web_DATA_T, data) + sizeof(CTS_GAMECMD_GET_NOTIFICATION_T));
	CLIENT_CLS4Web_DATA_T*	pCCData				= (CLIENT_CLS4Web_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd		= CLIENT_CLS4Web_DATA;

	CTS_GAMECMD_GET_NOTIFICATION_T*	pCmd	= (CTS_GAMECMD_GET_NOTIFICATION_T*)pCCData->data;
	pCmd->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pCmd->nGameCmd		= CTS_GAMECMD_GET_NOTIFICATION;

	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_create_account(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLIENT_CLS4Web_DATA_T, data) + sizeof(TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT_T));
	CLIENT_CLS4Web_DATA_T*	pCCData				= (CLIENT_CLS4Web_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd		= CLIENT_CLS4Web_DATA;

	TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT_T*	pCmd	= (TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT_T*)pCCData->data;
	pCmd->nCmd			= TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT;
	memset(pCmd->szAccountName, 0, sizeof(pCmd->szAccountName));
	memset(pCmd->szPass, 0, sizeof(pCmd->szPass));
	pCmd->szAccountName[0]			= 0;
	pCmd->szDeviceID[0]				= 0;
	pCmd->szPass[0]					= 0;
	pCmd->nDeviceType				= 0;
	int	nUseRandomName				= 0;
	pCmd->nAppID					= 0;
	wh_strsplit("sddssd", param, ",", pCmd->szDeviceID, &pCmd->nDeviceType, &nUseRandomName, pCmd->szAccountName, pCmd->szPass, &pCmd->nAppID);
	pCmd->bUseRandomName			= (bool)nUseRandomName;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_create_char(const char* param)
{
	char	szBuf[1024];
	char	szName[TTY_ACCOUNTNAME_LEN]		= "";
	unsigned int	nSex		= 0;
	unsigned int	nHeadID		= 0;
	unsigned int	nCountry	= 0;
	
	wh_strsplit("sddd", param, ",", szName, &nSex, &nHeadID, &nCountry);
	wh_char2utf8(szName, strlen(szName), szBuf, sizeof(szBuf));
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(TTY_CLIENT_LPGAMEPLAY_CREATECHAR_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	TTY_CLIENT_LPGAMEPLAY_CREATECHAR_T*	pCreateChar	= (TTY_CLIENT_LPGAMEPLAY_CREATECHAR_T*)pCCData->data;
	pCreateChar->nCmd		= TTY_CLIENT_LPGAMEPLAY_CREATECHAR;
	WH_STRNCPY0(pCreateChar->szName, szBuf);
	pCreateChar->nSex		= nSex;
	pCreateChar->nHeadID	= nHeadID;
	pCreateChar->nCountry	= nCountry;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_playercard(const char* param)
{
	tty_id_t	nAccountID	= 0;
	wh_strsplit("I", param, ",", &nAccountID);
	
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_PLAYERCARD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_PLAYERCARD_T*	pGetCard	= (CTS_GAMECMD_GET_PLAYERCARD_T*)pCCData->data;
	pGetCard->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGetCard->nGameCmd		= CTS_GAMECMD_GET_PLAYERCARD;
	pGetCard->nAccountID	= nAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_playercard_by_position(const char* param)
{
	unsigned int	nPosX	= 0;
	unsigned int	nPosY	= 0;
	wh_strsplit("dd", param, ",", &nPosX, &nPosY);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_PLAYERCARD_POS_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_PLAYERCARD_POS_T*	pGetCard	= (CTS_GAMECMD_GET_PLAYERCARD_POS_T*)pCCData->data;
	pGetCard->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGetCard->nGameCmd		= CTS_GAMECMD_GET_PLAYERCARD_POS;
	pGetCard->nPosX			= nPosX;
	pGetCard->nPosY			= nPosY;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_playercard_by_name(const char* param)
{
	char	szName[TTY_CHARACTERNAME_LEN];
	wh_strsplit("s", param, ",", szName);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_PLAYERCARD_NAME_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_PLAYERCARD_NAME_T*	pGetCard	= (CTS_GAMECMD_GET_PLAYERCARD_NAME_T*)pCCData->data;
	pGetCard->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGetCard->nGameCmd		= CTS_GAMECMD_GET_PLAYERCARD_NAME;
	WH_STRNCPY0(pGetCard->szCharName, szName);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_tile_info(const char* param)
{
	unsigned int	nPosX	= 0;
	unsigned int	nPosY	= 0;
	wh_strsplit("dd", param, ",", &nPosX, &nPosY);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_TILE_INFO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_TILE_INFO_T*	pGetTileInfo	= (CTS_GAMECMD_GET_TILE_INFO_T*)pCCData->data;
	pGetTileInfo->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGetTileInfo->nGameCmd		= CTS_GAMECMD_GET_TILE_INFO;
	pGetTileInfo->nPosX			= nPosX;
	pGetTileInfo->nPosY			= nPosY;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_building(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_BUILDING_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_BUILDING_LIST_T*	pGetBuilding	= (CTS_GAMECMD_GET_BUILDING_LIST_T*)pCCData->data;
	pGetBuilding->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGetBuilding->nGameCmd		= CTS_GAMECMD_GET_BUILDING_LIST;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_building_te(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_BUILDING_TE_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_BUILDING_TE_LIST_T*	pGetBuilding	= (CTS_GAMECMD_GET_BUILDING_TE_LIST_T*)pCCData->data;
	pGetBuilding->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGetBuilding->nGameCmd		= CTS_GAMECMD_GET_BUILDING_TE_LIST;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_build_building(const char* param)
{
	unsigned int	nExcelID	= 0;
	unsigned int	nAutoID		= 0;

	wh_strsplit("dd", param, ",", &nExcelID, &nAutoID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_BUILD_BUILDING_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_BUILD_BUILDING_T*	pBuildBuilding	= (CTS_GAMECMD_OPERATE_BUILD_BUILDING_T*)pCCData->data;
	pBuildBuilding->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pBuildBuilding->nGameCmd		= CTS_GAMECMD_OPERATE_BUILD_BUILDING;
	pBuildBuilding->nAutoID			= nAutoID;
	pBuildBuilding->nExcelID		= nExcelID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_upgrade_building(const char* param)
{
	unsigned int	nAutoID		= 0;

	wh_strsplit("d", param, ",", &nAutoID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_UPGRADE_BUILDING_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_UPGRADE_BUILDING_T*	pUpgradeBuilding	= (CTS_GAMECMD_OPERATE_UPGRADE_BUILDING_T*)pCCData->data;
	pUpgradeBuilding->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pUpgradeBuilding->nGameCmd		= CTS_GAMECMD_OPERATE_UPGRADE_BUILDING;
	pUpgradeBuilding->nAutoID		= nAutoID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_produce_gold(const char* param)
{
	unsigned int	nAutoID		= 0;
	unsigned int	nChoice		= 0;

	wh_strsplit("dd", param, ",", &nAutoID, &nChoice);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_PRODUCE_GOLD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_PRODUCE_GOLD_T*	pProduceGold	= (CTS_GAMECMD_OPERATE_PRODUCE_GOLD_T*)pCCData->data;
	pProduceGold->nCmd		= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pProduceGold->nGameCmd	= CTS_GAMECMD_OPERATE_PRODUCE_GOLD;
	pProduceGold->nAutoID	= nAutoID;
	pProduceGold->nProductionChoice	= nChoice;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_fetch_gold(const char* param)
{
	unsigned int	nAutoID		= 0;

	wh_strsplit("d", param, ",", &nAutoID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_FETCH_GOLD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_FETCH_GOLD_T*	pFetchGold	= (CTS_GAMECMD_OPERATE_FETCH_GOLD_T*)pCCData->data;
	pFetchGold->nCmd		= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pFetchGold->nGameCmd	= CTS_GAMECMD_OPERATE_FETCH_GOLD;
	pFetchGold->nAutoID		= nAutoID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_production_event(const char* param)
{
	tty_id_t	nAccountID	= 0;
	wh_strsplit("I", param, ",", &nAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_PRODUCTION_EVENT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_PRODUCTION_EVENT_T*	pGetProduction	= (CTS_GAMECMD_GET_PRODUCTION_EVENT_T*)pCCData->data;
	pGetProduction->nCmd		= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGetProduction->nGameCmd	= CTS_GAMECMD_GET_PRODUCTION_EVENT;
	pGetProduction->nAccountID	= nAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_production_te(const char* param)
{
	tty_id_t	nAccountID	= 0;
	wh_strsplit("I", param, ",", &nAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_PRODUCTION_TE_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_PRODUCTION_TE_LIST_T*	pGetProductionTE	= (CTS_GAMECMD_GET_PRODUCTION_TE_LIST_T*)pCCData->data;
	pGetProductionTE->nCmd		= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGetProductionTE->nGameCmd	= CTS_GAMECMD_GET_PRODUCTION_TE_LIST;
	pGetProductionTE->nAccountID	= nAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_accelerate_gold_produce(const char* param)
{
	unsigned int	nAutoID	= 0;
	unsigned int	nTime	= 0;
	unsigned int	nMoneyType	= 0;

	wh_strsplit("ddd", param, ",", &nAutoID, &nTime, &nMoneyType);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE_T*	pAcce	= (CTS_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE_T*)pCCData->data;
	pAcce->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAcce->nGameCmd			= CTS_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE;
	pAcce->nAutoID			= nAutoID;
	pAcce->nTime			= nTime;
	pAcce->nMoneyType		= nMoneyType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_accelerate_building(const char* param)
{
	unsigned int	nAutoID	= 0;
	unsigned int	nTime	= 0;
	unsigned int	nType	= 0;
	unsigned int	nMoneyType	= 0;

	wh_strsplit("dddd", param, ",", &nAutoID, &nTime, &nType, &nMoneyType);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ACCE_BUILDING_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ACCE_BUILDING_T*	pAcce	= (CTS_GAMECMD_OPERATE_ACCE_BUILDING_T*)pCCData->data;
	pAcce->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAcce->nGameCmd			= CTS_GAMECMD_OPERATE_ACCE_BUILDING;
	pAcce->nAutoID			= nAutoID;
	pAcce->nTime			= nTime;
	pAcce->nType			= nType;
	pAcce->nMoneyType		= nMoneyType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_accelerate_research(const char* param)
{
	unsigned int	nExcelID	= 0;
	unsigned int	nTime		= 0;
	unsigned int	nMoneyType	= 0;

	wh_strsplit("ddd", param, ",", &nExcelID, &nTime, &nMoneyType);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ACCE_RESEARCH_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ACCE_RESEARCH_T*	pAcce	= (CTS_GAMECMD_OPERATE_ACCE_RESEARCH_T*)pCCData->data;
	pAcce->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAcce->nGameCmd			= CTS_GAMECMD_OPERATE_ACCE_RESEARCH;
	pAcce->nExcelID			= nExcelID;
	pAcce->nTime			= nTime;
	pAcce->nMoneyType		= nMoneyType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_technology(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_TECHNOLOGY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_TECHNOLOGY_T*	pGet	= (CTS_GAMECMD_GET_TECHNOLOGY_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_TECHNOLOGY;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_research_te(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_RESEARCH_TE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_RESEARCH_TE_T*	pGet	= (CTS_GAMECMD_GET_RESEARCH_TE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_RESEARCH_TE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_research(const char* param)
{
	unsigned int	nExcelID	= 0;

	wh_strsplit("d", param, ",", &nExcelID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_RESEARCH_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_RESEARCH_T*	pRes	= (CTS_GAMECMD_OPERATE_RESEARCH_T*)pCCData->data;
	pRes->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pRes->nGameCmd			= CTS_GAMECMD_OPERATE_RESEARCH;
	pRes->nExcelID			= nExcelID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_conscript_soldier(const char* param)
{
	unsigned int	nExcelID	= 0;
	unsigned int	nLevel		= 0;
	unsigned int	nNum		= 0;

	wh_strsplit("ddd", param, ",", &nExcelID, &nLevel, &nNum);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T*	pConscript	= (CTS_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T*)pCCData->data;
	pConscript->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pConscript->nGameCmd			= CTS_GAMECMD_OPERATE_CONSCRIPT_SOLDIER;
	pConscript->nExcelID			= nExcelID;
	pConscript->nLevel				= nLevel;
	pConscript->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_upgrade_soldier(const char* param)
{
	unsigned int	nExcelID	= 0;
	unsigned int	nFromLevel	= 0;
	unsigned int	nToLevel	= 0;
	unsigned int	nNum		= 0;

	wh_strsplit("dddd", param, ",", &nExcelID, &nFromLevel, &nToLevel, &nNum);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_UPGRADE_SOLDIER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_UPGRADE_SOLDIER_T*	pConscript	= (CTS_GAMECMD_OPERATE_UPGRADE_SOLDIER_T*)pCCData->data;
	pConscript->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pConscript->nGameCmd			= CTS_GAMECMD_OPERATE_UPGRADE_SOLDIER;
	pConscript->nExcelID			= nExcelID;
	pConscript->nFromLevel			= nFromLevel;
	pConscript->nToLevel			= nToLevel;
	pConscript->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_soldier(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_SOLDIER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_SOLDIER_T*	pGet	= (CTS_GAMECMD_GET_SOLDIER_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_SOLDIER;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_accept_alliance_member(const char* param)
{
	tty_id_t	nMemberID;
	wh_strsplit("I", param, ",", &nMemberID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T*	pAccept	= (CTS_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T*)pCCData->data;
	pAccept->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAccept->nGameCmd		= CTS_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER;
	pAccept->nMemberID		= nMemberID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_build_alliance_building(const char* param)
{
	unsigned int	nExcelID;
	wh_strsplit("d", param, ",", &nExcelID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T*	pAccept	= (CTS_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T*)pCCData->data;
	pAccept->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAccept->nGameCmd		= CTS_GAMECMD_OPERATE_BUILD_ALLI_BUILDING;
	pAccept->nExcelID		= nExcelID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_cancel_join_alliance(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T*	pCancel	= (CTS_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T*)pCCData->data;
	pCancel->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pCancel->nGameCmd		= CTS_GAMECMD_OPERATE_CANCEL_JOIN_ALLI;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_create_alliance(const char* param)
{
	char	szAllianceName[TTY_ALLIANCENAME_LEN];
	wh_strsplit("s", param, "", szAllianceName);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CREATE_ALLI_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CREATE_ALLI_T*	pCreate	= (CTS_GAMECMD_OPERATE_CREATE_ALLI_T*)pCCData->data;
	pCreate->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pCreate->nGameCmd		= CTS_GAMECMD_OPERATE_CREATE_ALLI;
	WH_STRNCPY0(pCreate->szAllianceName, szAllianceName);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_dismiss_alliance(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_DISMISS_ALLI_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_DISMISS_ALLI_T*	pDismiss	= (CTS_GAMECMD_OPERATE_DISMISS_ALLI_T*)pCCData->data;
	pDismiss->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pDismiss->nGameCmd		= CTS_GAMECMD_OPERATE_DISMISS_ALLI;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_exit_alliance(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_EXIT_ALLI_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_EXIT_ALLI_T*	pExit	= (CTS_GAMECMD_OPERATE_EXIT_ALLI_T*)pCCData->data;
	pExit->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pExit->nGameCmd			= CTS_GAMECMD_OPERATE_EXIT_ALLI;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_expel_alliance_member(const char* param)
{
	tty_id_t	nMemberID;
	wh_strsplit("I", param, ",", &nMemberID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T*	pExpel	= (CTS_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T*)pCCData->data;
	pExpel->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pExpel->nGameCmd		= CTS_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER;
	pExpel->nMemberID		= nMemberID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_join_alliance(const char* param)
{
	tty_id_t	nAllianceID;
	wh_strsplit("I", param, ",", &nAllianceID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_JOIN_ALLI_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_JOIN_ALLI_T*	pJoin	= (CTS_GAMECMD_OPERATE_JOIN_ALLI_T*)pCCData->data;
	pJoin->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pJoin->nGameCmd			= CTS_GAMECMD_OPERATE_JOIN_ALLI;
	pJoin->nAllianceID		= nAllianceID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_set_alliance_position(const char* param)
{
	tty_id_t		nMemberID;
	unsigned int	nPosition;
	wh_strsplit("Id", param, ",", &nMemberID, &nPosition);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_SET_ALLI_POSITION_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_SET_ALLI_POSITION_T*	pSet	= (CTS_GAMECMD_OPERATE_SET_ALLI_POSITION_T*)pCCData->data;
	pSet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSet->nGameCmd			= CTS_GAMECMD_OPERATE_SET_ALLI_POSITION;
	pSet->nMemberID			= nMemberID;
	pSet->nPosition			= nPosition;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_info(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_INFO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_INFO_T*	pGet	= (CTS_GAMECMD_GET_ALLIANCE_INFO_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_INFO;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_member(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_MEMBER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_MEMBER_T*	pGet	= (CTS_GAMECMD_GET_ALLIANCE_MEMBER_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_MEMBER;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_building_te(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_BUILDING_TE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_BUILDING_TE_T*	pGet	= (CTS_GAMECMD_GET_ALLIANCE_BUILDING_TE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_BUILDING_TE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_building(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_BUILDING_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_BUILDING_T*	pGet	= (CTS_GAMECMD_GET_ALLIANCE_BUILDING_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_BUILDING;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_join_event(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T*	pGet	= (CTS_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_JOIN_EVENT;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_refuse_join_alliance(const char* param)
{
	tty_id_t		nApplicantID;
	wh_strsplit("I", param, ",", &nApplicantID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T*	pRefuse	= (CTS_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T*)pCCData->data;
	pRefuse->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pRefuse->nGameCmd		= CTS_GAMECMD_OPERATE_REFUSE_JOIN_ALLI;
	pRefuse->nApplicantID	= nApplicantID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_abdicate_alliance(const char* param)
{
	char		szMemberName[TTY_CHARACTERNAME_LEN];
	wh_strsplit("s", param, ",", &szMemberName);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ABDICATE_ALLI_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ABDICATE_ALLI_T*	pAbdicate	= (CTS_GAMECMD_OPERATE_ABDICATE_ALLI_T*)pCCData->data;
	pAbdicate->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAbdicate->nGameCmd		= CTS_GAMECMD_OPERATE_ABDICATE_ALLI;
	WH_STRNCPY0(pAbdicate->szMemberName, szMemberName);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_char_atb(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_CHAR_ATB_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_CHAR_ATB_T*	pGet	= (CTS_GAMECMD_GET_CHAR_ATB_T*)pCCData->data;
	pGet->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd		= CTS_GAMECMD_GET_CHAR_ATB;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_chat_private(const char* param)
{
	tty_id_t	nPeerID;
	char		szText[1024];
	wh_strsplit("Ia", param, ",", &nPeerID, szText);
	char		szBuf[1024];
	unsigned int	nTextLen	= wh_char2utf8(szText, strlen(szText), szBuf, sizeof(szBuf))+1;
	szBuf[nTextLen-1]		= 0;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_CHAT_PRIVATE_T, szText) + nTextLen);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_CHAT_PRIVATE_T*	pChat		= (CTS_GAMECMD_CHAT_PRIVATE_T*)pCCData->data;
	pChat->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pChat->nGameCmd			= CTS_GAMECMD_CHAT_PRIVATE;
	pChat->nPeerAccountID	= nPeerID;
	pChat->nTextLen			= nTextLen;
	memcpy(pChat->szText, szBuf, pChat->nTextLen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_chat_alliance(const char* param)
{
	char		szText[1024];
	wh_strsplit("a", param, ",", szText);
	char		szBuf[1024];
	unsigned int	nTextLen	= wh_char2utf8(szText, strlen(szText), szBuf, sizeof(szBuf))+1;
	szBuf[nTextLen-1]		= 0;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_CHAT_ALLIANCE_T, szText) + nTextLen);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_CHAT_ALLIANCE_T*	pChat	= (CTS_GAMECMD_CHAT_ALLIANCE_T*)pCCData->data;
	pChat->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pChat->nGameCmd			= CTS_GAMECMD_CHAT_ALLIANCE;
	pChat->nTextLen			= nTextLen;
	memcpy(pChat->szText, szBuf, pChat->nTextLen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_chat_world(const char* param)
{
	char		szText[1024];
	wh_strsplit("a", param, ",", szText);
	char		szBuf[1024];
	unsigned int	nTextLen	= wh_char2utf8(szText, strlen(szText), szBuf, sizeof(szBuf))+1;
	szBuf[nTextLen-1]		= 0;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_CHAT_WORLD_T, szText) + nTextLen);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_CHAT_WORLD_T*	pChat	= (CTS_GAMECMD_CHAT_WORLD_T*)pCCData->data;
	pChat->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pChat->nGameCmd			= CTS_GAMECMD_CHAT_WORLD;
	pChat->nTextLen			= nTextLen;
	memcpy(pChat->szText, szBuf, pChat->nTextLen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_chat_group(const char* param)
{
	char		szText[1024];
	wh_strsplit("a", param, ",", szText);
	char		szBuf[1024];
	unsigned int	nTextLen	= wh_char2utf8(szText, strlen(szText), szBuf, sizeof(szBuf))+1;
	szBuf[nTextLen-1]		= 0;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_CHAT_GROUP_T, szText) + nTextLen);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_CHAT_GROUP_T*	pChat	= (CTS_GAMECMD_CHAT_GROUP_T*)pCCData->data;
	pChat->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pChat->nGameCmd			= CTS_GAMECMD_CHAT_GROUP;
	pChat->nTextLen			= nTextLen;
	memcpy(pChat->szText, szBuf, pChat->nTextLen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_add_mail(const char* param)
{
	tty_id_t	nPeerAccountID	= 0;
	char		szText[TTY_MAIL_TEXT_LEN];
	wh_strsplit("Is", param, ",", &nPeerAccountID, szText);
	char		szTextBuf[TTY_MAIL_TEXT_LEN];
	int	nTextLen	= wh_char2utf8(szText, strlen(szText), szTextBuf, sizeof(szTextBuf)) + 1;
	szTextBuf[nTextLen-1]	= 0;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_ADD_MAIL_T, szText) + nTextLen);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ADD_MAIL_T*	pAdd			= (CTS_GAMECMD_ADD_MAIL_T*)pCCData->data;
	pAdd->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAdd->nGameCmd			= CTS_GAMECMD_ADD_MAIL;
	pAdd->nToCharID			= nPeerAccountID;
	pAdd->nTextLen			= nTextLen;
	memcpy(pAdd->szText, szTextBuf, pAdd->nTextLen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_add_mail_with_ext_data(const char* param)
{
	tty_id_t	nPeerAccountID	= 0;
	tty_id_t	nExtData		= 0;
	char		szText[TTY_MAIL_TEXT_LEN];
	wh_strsplit("IsI", param, ",", &nPeerAccountID, szText, &nExtData);
	char		szTextBuf[TTY_MAIL_TEXT_LEN];
	int	nTextLen	= wh_char2utf8(szText, strlen(szText), szTextBuf, sizeof(szTextBuf)) + 1;
	szTextBuf[nTextLen-1]	= 0;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_ADD_MAIL_WITH_EXT_DATA_T, szText) + nTextLen + sizeof(int) + sizeof(nExtData));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ADD_MAIL_WITH_EXT_DATA_T*	pAdd			= (CTS_GAMECMD_ADD_MAIL_WITH_EXT_DATA_T*)pCCData->data;
	pAdd->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAdd->nGameCmd			= CTS_GAMECMD_ADD_MAIL_WITH_EXT_DATA;
	pAdd->nToCharID			= nPeerAccountID;
	pAdd->nTextLen			= nTextLen;
	memcpy(pAdd->szText, szTextBuf, pAdd->nTextLen);
	int*	pExtDataLen		= (int*)wh_getoffsetaddr(pAdd, wh_offsetof(CTS_GAMECMD_ADD_MAIL_WITH_EXT_DATA_T, szText) + nTextLen);
	*pExtDataLen			= sizeof(nExtData);
	tty_id_t*	pExtData	= (tty_id_t*)wh_getptrnexttoptr(pExtDataLen);
	*pExtData				= nExtData;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_mail(const char* param)
{
	unsigned int		nFromNum			= 0;
	unsigned int		nToNum				= 0;
	unsigned char		nType				= 0;
	wh_strsplit("ddy", param, ",", &nFromNum, &nToNum, &nType);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_MAIL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_MAIL_T*	pGet	= (CTS_GAMECMD_GET_MAIL_T*)pCCData->data;
	pGet->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd		= CTS_GAMECMD_GET_MAIL;
	pGet->nFromNum		= nFromNum;
	pGet->nToNum		= nToNum;
	pGet->nType			= nType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_read_mail(const char* param)
{
	unsigned int	nMailID	= 0;
	wh_strsplit("d", param, ",", &nMailID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_READ_MAIL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_READ_MAIL_T*	pRead		= (CTS_GAMECMD_READ_MAIL_T*)pCCData->data;
	pRead->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pRead->nGameCmd			= CTS_GAMECMD_READ_MAIL;
	pRead->nMailID			= nMailID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_delete_mail(const char* param)
{
	unsigned int	nMailID	= 0;
	wh_strsplit("d", param, ",", &nMailID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_DELETE_MAIL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_DELETE_MAIL_T*	pDel		= (CTS_GAMECMD_DELETE_MAIL_T*)pCCData->data;
	pDel->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pDel->nGameCmd			= CTS_GAMECMD_DELETE_MAIL;
	pDel->nMailID			= nMailID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_mail_1(const char* param)
{
	unsigned int	nMailID	= 0;
	wh_strsplit("d", param, ",", &nMailID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_MAIL_1_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_MAIL_1_T*	pGet		= (CTS_GAMECMD_GET_MAIL_1_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_MAIL_1;
	pGet->nMailID			= nMailID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_my_friend_apply(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_MY_FRIEND_APPLY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_MY_FRIEND_APPLY_T*	pGet		= (CTS_GAMECMD_GET_MY_FRIEND_APPLY_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_MY_FRIEND_APPLY;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_others_friend_apply(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_OTHERS_FRIEND_APPLY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_OTHERS_FRIEND_APPLY_T*	pGet		= (CTS_GAMECMD_GET_OTHERS_FRIEND_APPLY_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_OTHERS_FRIEND_APPLY;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_friend(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_FRIEND_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_FRIEND_LIST_T*	pGet	= (CTS_GAMECMD_GET_FRIEND_LIST_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_FRIEND_LIST;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_approve_friend(const char* param)
{
	tty_id_t	nPeerAccountID	= 0;
	wh_strsplit("I", param, ",", &nPeerAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_APPROVE_FRIEND_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_APPROVE_FRIEND_T*	pApprove		= (CTS_GAMECMD_OPERATE_APPROVE_FRIEND_T*)pCCData->data;
	pApprove->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pApprove->nGameCmd		= CTS_GAMECMD_OPERATE_APPROVE_FRIEND;
	pApprove->nPeerAccountID	= nPeerAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_apply_friend(const char* param)
{
	tty_id_t	nPeerAccountID	= 0;
	wh_strsplit("I", param, ",", &nPeerAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_APPLY_FRIEND_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_APPLY_FRIEND_T*	pApply		= (CTS_GAMECMD_OPERATE_APPLY_FRIEND_T*)pCCData->data;
	pApply->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pApply->nGameCmd		= CTS_GAMECMD_OPERATE_APPLY_FRIEND;
	pApply->nPeerAccountID	= nPeerAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_refuse_friend(const char* param)
{
	tty_id_t	nPeerAccountID	= 0;
	wh_strsplit("I", param, ",", &nPeerAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T*	pRefuse		= (CTS_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T*)pCCData->data;
	pRefuse->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pRefuse->nGameCmd		= CTS_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY;
	pRefuse->nPeerAccountID	= nPeerAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_cancel_friend(const char* param)
{
	tty_id_t	nPeerAccountID	= 0;
	wh_strsplit("I", param, ",", &nPeerAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T*	pCancel		= (CTS_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T*)pCCData->data;
	pCancel->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pCancel->nGameCmd		= CTS_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY;
	pCancel->nPeerAccountID	= nPeerAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_delete_friend(const char* param)
{
	tty_id_t	nPeerAccountID	= 0;
	wh_strsplit("I", param, ",", &nPeerAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_DELETE_FRIEND_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_DELETE_FRIEND_T*	pDelete		= (CTS_GAMECMD_OPERATE_DELETE_FRIEND_T*)pCCData->data;
	pDelete->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pDelete->nGameCmd		= CTS_GAMECMD_OPERATE_DELETE_FRIEND;
	pDelete->nFriendID		= nPeerAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_world_area(const char* param)
{
	unsigned int	nFromX	= 0;
	unsigned int	nFromY	= 0;
	unsigned int	nToX	= 0;
	unsigned int	nToY	= 0;
	wh_strsplit("dddd", param, ",", &nFromX, &nFromY, &nToX, &nToY);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_WORLD_AREA_INFO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_WORLD_AREA_INFO_T*	pGet		= (CTS_GAMECMD_GET_WORLD_AREA_INFO_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_WORLD_AREA_INFO;
	pGet->nFromX			= nFromX;
	pGet->nFromY			= nFromY;
	pGet->nToX				= nToX;
	pGet->nToY				= nToY;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_add_alliance_mail(const char* param)
{
	char	szBuf[TTY_ALLIANCE_MAIL_TEXT_LEN];
	wh_strsplit("a", param, "", szBuf);
	char	szBufUTF8[TTY_ALLIANCE_MAIL_TEXT_LEN];
	int	nLen	= wh_char2utf8(szBuf, strlen(szBuf), szBufUTF8, sizeof(szBufUTF8));
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T, szText) + nLen);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T*	pAdd		= (CTS_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T*)pCCData->data;
	pAdd->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pAdd->nGameCmd			= CTS_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL;
	pAdd->nLen				= nLen;
	memcpy(pAdd->szText, szBufUTF8, nLen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_mail(const char* param)
{
	unsigned int	nFromNum	= 0;
	unsigned int	nToNum		= 0;
	wh_strsplit("dd", param, ",", &nFromNum, &nToNum);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_MAIL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_MAIL_T*	pGet		= (CTS_GAMECMD_GET_ALLIANCE_MAIL_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_MAIL;
	pGet->nFromNum			= nFromNum;
	pGet->nToNum			= nToNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_log(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_LOG_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_LOG_T*	pGet		= (CTS_GAMECMD_GET_ALLIANCE_LOG_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_LOG;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_private_log(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_PRIVATE_LOG_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_PRIVATE_LOG_T*	pGet		= (CTS_GAMECMD_GET_PRIVATE_LOG_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_PRIVATE_LOG;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_buy_item(const char* param)
{
	unsigned int	nExcelID	= 0;
	unsigned int	nNum		= 0;
	unsigned int	nMoneyType	= 0;
	wh_strsplit("ddd", param, ",", &nExcelID, &nNum, &nMoneyType);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_BUY_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_BUY_ITEM_T*	pReq		= (CTS_GAMECMD_OPERATE_BUY_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_BUY_ITEM;
	pReq->nExcelID			= nExcelID;
	pReq->nNum				= nNum;
	pReq->nMoneyType		= nMoneyType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_sell_item(const char* param)
{
	tty_id_t		nItemID		= 0;
	unsigned int	nNum		= 0;
	wh_strsplit("Id", param, ",", &nItemID, &nNum);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_SELL_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_SELL_ITEM_T*	pReq		= (CTS_GAMECMD_OPERATE_SELL_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_SELL_ITEM;
	pReq->nItemID			= nItemID;
	pReq->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_relation_log(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_RELATION_LOG_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_RELATION_LOG_T*	pReq		= (CTS_GAMECMD_GET_RELATION_LOG_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_RELATION_LOG;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_fetch_christmas_tree(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T*	pReq		= (CTS_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_watering_christmas_tree(const char* param)
{
	tty_id_t	nPeerAccountID	= 0;
	wh_strsplit("I", param, ",", &nPeerAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T*	pReq		= (CTS_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE;
	pReq->nPeerAccountID	= nPeerAccountID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_steal_gold(const char* param)
{
	tty_id_t		nPeerAccountID	= 0;
	unsigned int	nAutoID	= 0;
	wh_strsplit("Id", param, ",", &nPeerAccountID, &nAutoID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_STEAL_GOLD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_STEAL_GOLD_T*	pReq		= (CTS_GAMECMD_OPERATE_STEAL_GOLD_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_STEAL_GOLD;
	pReq->nPeerAccountID	= nPeerAccountID;
	pReq->nAutoID			= nAutoID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_training_te(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_TRAINING_TE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_TRAINING_TE_T*	pReq		= (CTS_GAMECMD_GET_TRAINING_TE_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_TRAINING_TE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_add_training(const char* param)
{
	tty_id_t		nHeroID			= 0;
	int				nTimeUnitNum	= 0;
	whvector<tty_id_t>	vectHeroID;
	int				nNum			= 0;
	int				nOffset			= 0;
	nNum			= wh_strsplit(&nOffset, "dI", param, ",", &nTimeUnitNum, &nHeroID);
	vectHeroID.push_back(nHeroID);
	while((nNum=wh_strsplit(&nOffset, "I", param, ",", &nHeroID)) == 1)
	{
		vectHeroID.push_back(nHeroID);
	}
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ADD_TRAINING_T) + vectHeroID.size()*sizeof(tty_id_t));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ADD_TRAINING_T*	pReq		= (CTS_GAMECMD_OPERATE_ADD_TRAINING_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_ADD_TRAINING;
	pReq->nTimeUnitNum		= nTimeUnitNum;
	pReq->nNum				= vectHeroID.size();
	memcpy(wh_getptrnexttoptr(pReq), vectHeroID.getbuf(), vectHeroID.size()*sizeof(tty_id_t));
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_exit_training(const char* param)
{
	tty_id_t		nHeroID	= 0;
	wh_strsplit("I", param, ",", &nHeroID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_EXIT_TRAINING_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_EXIT_TRAINING_T*	pReq		= (CTS_GAMECMD_OPERATE_EXIT_TRAINING_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_EXIT_TRAINING;
	pReq->nHeroID			= nHeroID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_recent_chat(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_CHAT_GET_RECENT_CHAT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData					= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_CHAT_GET_RECENT_CHAT_T*	pReq		= (CTS_GAMECMD_CHAT_GET_RECENT_CHAT_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_CHAT_GET_RECENT_CHAT;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_tavern_refresh(const char* param)
{
	unsigned int	nUseItem	= 0;

	wh_strsplit("d", param, ",", &nUseItem);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_TAVERN_REFRESH_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_TAVERN_REFRESH_T*	pGet	= (CTS_GAMECMD_OPERATE_TAVERN_REFRESH_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_OPERATE_TAVERN_REFRESH;
	pGet->nUseItem			= nUseItem;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_hire_hero(const char* param)
{
	unsigned int	nSlotID	= 0;
	tty_id_t		nHeroID	= 0;

	wh_strsplit("dI", param, ",", &nSlotID, &nHeroID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_HIRE_HERO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_HIRE_HERO_T*	pHire	= (CTS_GAMECMD_OPERATE_HIRE_HERO_T*)pCCData->data;
	pHire->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pHire->nGameCmd			= CTS_GAMECMD_OPERATE_HIRE_HERO;
	pHire->nSlotID			= nSlotID;
	pHire->nHeroID			= nHeroID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_fire_hero(const char* param)
{
	tty_id_t		nHeroID	= 0;

	wh_strsplit("I", param, ",", &nHeroID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_FIRE_HERO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_FIRE_HERO_T*	pFire	= (CTS_GAMECMD_OPERATE_FIRE_HERO_T*)pCCData->data;
	pFire->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pFire->nGameCmd			= CTS_GAMECMD_OPERATE_FIRE_HERO;
	pFire->nHeroID			= nHeroID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_levelup_hero(const char* param)
{
	tty_id_t		nHeroID	= 0;

	wh_strsplit("I", param, ",", &nHeroID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_LEVELUP_HERO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_LEVELUP_HERO_T*	pLevelup	= (CTS_GAMECMD_OPERATE_LEVELUP_HERO_T*)pCCData->data;
	pLevelup->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pLevelup->nGameCmd		= CTS_GAMECMD_OPERATE_LEVELUP_HERO;
	pLevelup->nHeroID		= nHeroID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_add_grow(const char* param)
{
	tty_id_t		nHeroID	= 0;
	float			fGrow = 0;

	wh_strsplit("If", param, ",", &nHeroID, &fGrow);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ADD_GROW_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ADD_GROW_T*	pGrow	= (CTS_GAMECMD_OPERATE_ADD_GROW_T*)pCCData->data;
	pGrow->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGrow->nGameCmd		= CTS_GAMECMD_OPERATE_ADD_GROW;
	pGrow->nHeroID		= nHeroID;
	pGrow->fGrow		= fGrow;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_config_hero(const char* param)
{
	tty_id_t		nHeroID	= 0;
	int				nArmyType = 0;
	int				nArmyLevel = 0;
	int				nArmyNum = 0;

	wh_strsplit("Iddd", param, ",", &nHeroID, &nArmyType, &nArmyLevel, &nArmyNum);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CONFIG_HERO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CONFIG_HERO_T*	pConfig	= (CTS_GAMECMD_OPERATE_CONFIG_HERO_T*)pCCData->data;
	pConfig->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pConfig->nGameCmd		= CTS_GAMECMD_OPERATE_CONFIG_HERO;
	pConfig->nHeroID		= nHeroID;
	pConfig->nArmyType		= nArmyType;
	pConfig->nArmyLevel		= nArmyLevel;
	pConfig->nArmyNum		= nArmyNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_hero_refresh(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_GET_HERO_REFRESH_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_GET_HERO_REFRESH_T*	pGet	= (CTS_GAMECMD_OPERATE_GET_HERO_REFRESH_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_OPERATE_GET_HERO_REFRESH;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_hero_hire(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_GET_HERO_HIRE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_GET_HERO_HIRE_T*	pGet	= (CTS_GAMECMD_OPERATE_GET_HERO_HIRE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_OPERATE_GET_HERO_HIRE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_start_combat(const char* param)
{
	tty_id_t		nObjID = 0;
	int				nCombatType = 0;
	int				nAutoCombat	= 0;
	int				nAutoSupply	= 0;
	tty_id_t		n1Hero	= 0;
	tty_id_t		n2Hero	= 0;
	tty_id_t		n3Hero	= 0;
	tty_id_t		n4Hero	= 0;
	tty_id_t		n5Hero	= 0;
	int				nStopLevel	= 0;

	wh_strsplit("IdddIIIIId", param, ",", &nObjID, &nCombatType, &nAutoCombat, &nAutoSupply, &n1Hero, &n2Hero, &n3Hero, &n4Hero, &n5Hero, &nStopLevel);
	
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_START_COMBAT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_START_COMBAT_T*	pSC	= (CTS_GAMECMD_OPERATE_START_COMBAT_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_OPERATE_START_COMBAT;
	pSC->nObjID			= nObjID;
	pSC->nCombatType	= nCombatType;
	pSC->nAutoCombat	= nAutoCombat;
	pSC->nAutoSupply	= nAutoSupply;
	pSC->n1Hero			= n1Hero;
	pSC->n2Hero			= n2Hero;
	pSC->n3Hero			= n3Hero;
	pSC->n4Hero			= n4Hero;
	pSC->n5Hero			= n5Hero;
	pSC->nStopLevel		= nStopLevel;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_stop_combat(const char* param)
{
	tty_id_t		nCombatID	= 0;

	wh_strsplit("I", param, ",", &nCombatID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_STOP_COMBAT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_STOP_COMBAT_T*	pSC	= (CTS_GAMECMD_OPERATE_STOP_COMBAT_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_OPERATE_STOP_COMBAT;
	pSC->nCombatID		= nCombatID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_combat(const char* param)
{
	tty_id_t		nCombatID	= 0;

	wh_strsplit("I", param, ",", &nCombatID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_GET_COMBAT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_GET_COMBAT_T*	pSC	= (CTS_GAMECMD_OPERATE_GET_COMBAT_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_OPERATE_GET_COMBAT;
	pSC->nCombatID		= nCombatID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_save_combat(const char* param)
{
	tty_id_t		nCombatID	= 0;

	wh_strsplit("I", param, ",", &nCombatID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_SAVE_COMBAT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_SAVE_COMBAT_T*	pSC	= (CTS_GAMECMD_OPERATE_SAVE_COMBAT_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_OPERATE_SAVE_COMBAT;
	pSC->nCombatID		= nCombatID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_list_combat(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_LIST_COMBAT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_LIST_COMBAT_T*	pSC	= (CTS_GAMECMD_OPERATE_LIST_COMBAT_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_OPERATE_LIST_COMBAT;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_config_citydefense(const char* param)
{
	tty_id_t		n1Hero	= 0;
	tty_id_t		n2Hero	= 0;
	tty_id_t		n3Hero	= 0;
	tty_id_t		n4Hero	= 0;
	tty_id_t		n5Hero	= 0;
	int				nAutoSupply = 0;

	wh_strsplit("IIIIId", param, ",", &n1Hero, &n2Hero, &n3Hero, &n4Hero, &n5Hero, &nAutoSupply);
	
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T*	pSC	= (CTS_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_OPERATE_CONFIG_CITYDEFENSE;
	pSC->n1Hero			= n1Hero;
	pSC->n2Hero			= n2Hero;
	pSC->n3Hero			= n3Hero;
	pSC->n4Hero			= n4Hero;
	pSC->n5Hero			= n5Hero;
	pSC->nAutoSupply	= nAutoSupply;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_item(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ITEM_T*	pGet	= (CTS_GAMECMD_GET_ITEM_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_ITEM;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_add_item(const char* param)
{
	int			nExcelID	= 0;
	int			nNum		= 0;

	wh_strsplit("dd", param, ",", &nExcelID, &nNum);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ADD_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ADD_ITEM_T*	pReq	= (CTS_GAMECMD_ADD_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_ADD_ITEM;
	pReq->nExcelID			= nExcelID;
	pReq->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_del_item(const char* param)
{
	tty_id_t	nItemID		= 0;
	int			nNum		= 0;

	wh_strsplit("Id", param, ",", &nItemID, &nNum);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_DEL_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_DEL_ITEM_T*	pReq	= (CTS_GAMECMD_DEL_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_DEL_ITEM;
	pReq->nItemID			= nItemID;
	pReq->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_equip_item(const char* param)
{
	tty_id_t	nHeroID	= 0;
	tty_id_t	nItemID	= 0;;

	wh_strsplit("II", param, ",", &nHeroID, &nItemID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_EQUIP_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_EQUIP_ITEM_T*	pReq	= (CTS_GAMECMD_EQUIP_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_EQUIP_ITEM;
	pReq->nHeroID			= nHeroID;
	pReq->nItemID			= nItemID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_disequip_item(const char* param)
{
	tty_id_t	nHeroID	= 0;
	tty_id_t	nItemID	= 0;;

	wh_strsplit("II", param, ",", &nHeroID, &nItemID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_DISEQUIP_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_DISEQUIP_ITEM_T*	pReq	= (CTS_GAMECMD_DISEQUIP_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_DISEQUIP_ITEM;
	pReq->nHeroID			= nHeroID;
	pReq->nItemID			= nItemID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_mount_item(const char* param)
{
	tty_id_t	nHeroID		= 0;
	tty_id_t	nEquipID	= 0;
	int			nSlotIdx	= 0;
	int			nExcelID	= 0;

	wh_strsplit("IIdd", param, ",", &nHeroID, &nEquipID, &nSlotIdx, &nExcelID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_MOUNT_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_MOUNT_ITEM_T*	pReq	= (CTS_GAMECMD_MOUNT_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_MOUNT_ITEM;
	pReq->nHeroID			= nHeroID;
	pReq->nEquipID			= nEquipID;
	pReq->nSlotIdx			= nSlotIdx;
	pReq->nExcelID			= nExcelID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_unmount_item(const char* param)
{
	tty_id_t	nHeroID		= 0;
	tty_id_t	nEquipID	= 0;
	int			nSlotIdx	= 0;;

	wh_strsplit("IId", param, ",", &nHeroID, &nEquipID, &nSlotIdx);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_UNMOUNT_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_UNMOUNT_ITEM_T*	pReq	= (CTS_GAMECMD_UNMOUNT_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_UNMOUNT_ITEM;
	pReq->nHeroID			= nHeroID;
	pReq->nEquipID			= nEquipID;
	pReq->nSlotIdx			= nSlotIdx;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_compos_item(const char* param)
{
	int			nExcelID	= 0;
	int			nNum	= 0;;

	wh_strsplit("dd", param, ",", &nExcelID, &nNum);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_COMPOS_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_COMPOS_ITEM_T*	pReq	= (CTS_GAMECMD_COMPOS_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_COMPOS_ITEM;
	pReq->nExcelID			= nExcelID;
	pReq->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_discompos_item(const char* param)
{
	int			nExcelID	= 0;
	int			nNum		= 0;;

	wh_strsplit("dd", param, ",", &nExcelID, &nNum);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_DISCOMPOS_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_DISCOMPOS_ITEM_T*	pReq	= (CTS_GAMECMD_DISCOMPOS_ITEM_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_DISCOMPOS_ITEM;
	pReq->nExcelID			= nExcelID;
	pReq->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_equip_item_all(const char* param)
{
	tty_id_t	nHeroID	= 0;
	tty_id_t	n1HeadID	= 0;
	tty_id_t	n2ChestID	= 0;
	tty_id_t	n3ShoeID	= 0;
	tty_id_t	n4WeaponID	= 0;
	tty_id_t	n5TrinketID	= 0;

	wh_strsplit("IIIIII", param, ",", &nHeroID, &n1HeadID, &n2ChestID, &n3ShoeID, &n4WeaponID, &n5TrinketID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_EQUIP_ITEM_ALL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_EQUIP_ITEM_ALL_T*	pReq	= (CTS_GAMECMD_EQUIP_ITEM_ALL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_EQUIP_ITEM_ALL;
	pReq->nHeroID			= nHeroID;
	pReq->n1HeadID			= n1HeadID;
	pReq->n2ChestID			= n2ChestID;
	pReq->n3ShoeID			= n3ShoeID;
	pReq->n4WeaponID		= n4WeaponID;
	pReq->n5TrinketID		= n5TrinketID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_mount_item_all(const char* param)
{
	tty_id_t	nHeroID		= 0;
	tty_id_t	nEquipID	= 0;
	int			n1ExcelID	= 0;
	int			n2ExcelID	= 0;
	int			n3ExcelID	= 0;
	int			n4ExcelID	= 0;

	wh_strsplit("IIdddd", param, ",", &nHeroID, &nEquipID, &n1ExcelID, &n2ExcelID, &n3ExcelID, &n4ExcelID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_MOUNT_ITEM_ALL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_MOUNT_ITEM_ALL_T*	pReq	= (CTS_GAMECMD_MOUNT_ITEM_ALL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_MOUNT_ITEM_ALL;
	pReq->nHeroID			= nHeroID;
	pReq->nEquipID			= nEquipID;
	pReq->n1ExcelID			= n1ExcelID;
	pReq->n2ExcelID			= n2ExcelID;
	pReq->n3ExcelID			= n3ExcelID;
	pReq->n4ExcelID			= n4ExcelID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_get_quest(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_QUEST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_QUEST_T*	pReq	= (CTS_GAMECMD_GET_QUEST_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_QUEST;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_done_quest(const char* param)
{
	int			nExcelID	= 0;

	wh_strsplit("d", param, ",", &nExcelID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_DONE_QUEST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_DONE_QUEST_T*	pReq	= (CTS_GAMECMD_DONE_QUEST_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_DONE_QUEST;
	pReq->nExcelID			= nExcelID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_instancestatus(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_INSTANCESTATUS_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_INSTANCESTATUS_T*	pGet	= (CTS_GAMECMD_GET_INSTANCESTATUS_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_INSTANCESTATUS;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_instancedesc(const char* param)
{
	int			nExcelID	= 0;
	int			nClassID	= 0;
	wh_strsplit("dd", param, ",", &nExcelID, &nClassID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_INSTANCEDESC_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_INSTANCEDESC_T*	pGet	= (CTS_GAMECMD_GET_INSTANCEDESC_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_INSTANCEDESC;
	pGet->nExcelID			= nExcelID;
	pGet->nClassID			= nClassID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_create_instance(const char* param)
{
	int			nExcelID	= 0;
	int			nClassID	= 0;
	wh_strsplit("dd", param, ",", &nExcelID, &nClassID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_CREATE_INSTANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_CREATE_INSTANCE_T*	pGet	= (CTS_GAMECMD_CREATE_INSTANCE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_CREATE_INSTANCE;
	pGet->nExcelID			= nExcelID;
	pGet->nClassID			= nClassID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_join_instance(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &nExcelID, &nInstanceID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_JOIN_INSTANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_JOIN_INSTANCE_T*	pGet	= (CTS_GAMECMD_JOIN_INSTANCE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_JOIN_INSTANCE;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_quit_instance(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &nExcelID, &nInstanceID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_QUIT_INSTANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_QUIT_INSTANCE_T*	pGet	= (CTS_GAMECMD_QUIT_INSTANCE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_QUIT_INSTANCE;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_destroy_instance(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &nExcelID, &nInstanceID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_DESTROY_INSTANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_DESTROY_INSTANCE_T*	pGet	= (CTS_GAMECMD_DESTROY_INSTANCE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_DESTROY_INSTANCE;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_instancedata(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &nExcelID, &nInstanceID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_INSTANCEDATA_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_INSTANCEDATA_T*	pGet	= (CTS_GAMECMD_GET_INSTANCEDATA_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_INSTANCEDATA;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_instanceloot(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &nExcelID, &nInstanceID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_INSTANCELOOT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_INSTANCELOOT_T*	pGet	= (CTS_GAMECMD_GET_INSTANCELOOT_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_INSTANCELOOT;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_config_instancehero(const char* param)
{
	int				nExcelID = 0;
	tty_id_t		nInstanceID = 0;
	tty_id_t		n1Hero	= 0;
	tty_id_t		n2Hero	= 0;
	tty_id_t		n3Hero	= 0;
	tty_id_t		n4Hero	= 0;
	tty_id_t		n5Hero	= 0;

	wh_strsplit("dIIIIII", param, ",", &nExcelID, &nInstanceID, &n1Hero, &n2Hero, &n3Hero, &n4Hero, &n5Hero);
	
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_CONFIG_INSTANCEHERO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_CONFIG_INSTANCEHERO_T*	pSC	= (CTS_GAMECMD_CONFIG_INSTANCEHERO_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_CONFIG_INSTANCEHERO;
	pSC->nExcelID		= nExcelID;
	pSC->nInstanceID	= nInstanceID;
	pSC->n1Hero			= n1Hero;
	pSC->n2Hero			= n2Hero;
	pSC->n3Hero			= n3Hero;
	pSC->n4Hero			= n4Hero;
	pSC->n5Hero			= n5Hero;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_auto_combat(const char* param)
{
	tty_id_t	nInstanceID	= 0;
	int		nVal	= 0;

	wh_strsplit("Id", param, ",", &nInstanceID, &nVal);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_AUTO_COMBAT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_AUTO_COMBAT_T*	pReq	= (CTS_GAMECMD_AUTO_COMBAT_T*)pCCData->data;
	pReq->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd		= CTS_GAMECMD_AUTO_COMBAT;
	pReq->nInstanceID	= nInstanceID;
	pReq->nVal			= nVal;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_kick_instance(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	tty_id_t	nObjID		= 0;
	wh_strsplit("dII", param, ",", &nExcelID, &nInstanceID, &nObjID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_KICK_INSTANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_KICK_INSTANCE_T*	pGet	= (CTS_GAMECMD_KICK_INSTANCE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_KICK_INSTANCE;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	pGet->nObjID			= nObjID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_start_instance(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &nExcelID, &nInstanceID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_START_INSTANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_START_INSTANCE_T*	pGet	= (CTS_GAMECMD_START_INSTANCE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_START_INSTANCE;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_prepare_instance(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	int			nVal		= 0;
	wh_strsplit("dId", param, ",", &nExcelID, &nInstanceID, &nVal);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_PREPARE_INSTANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_PREPARE_INSTANCE_T*	pGet	= (CTS_GAMECMD_PREPARE_INSTANCE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_PREPARE_INSTANCE;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	pGet->nVal				= nVal;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_supply_instance(const char* param)
{
	int			nExcelID	= 0;
	tty_id_t	nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &nExcelID, &nInstanceID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_SUPPLY_INSTANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_SUPPLY_INSTANCE_T*	pGet	= (CTS_GAMECMD_SUPPLY_INSTANCE_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_SUPPLY_INSTANCE;
	pGet->nExcelID			= nExcelID;
	pGet->nInstanceID		= nInstanceID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_use_item(const char* param)
{
	tty_id_t	nHeroID		= 0;
	int			nExcelID	= 0;
	int			nNum		= 0;
	wh_strsplit("Idd", param, ",", &nHeroID, &nExcelID, &nNum);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_USE_ITEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_USE_ITEM_T*	pGet	= (CTS_GAMECMD_USE_ITEM_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_USE_ITEM;
	pGet->nHeroID			= nHeroID;
	pGet->nExcelID			= nExcelID;
	pGet->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_sync_char(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_SYNC_CHAR_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_SYNC_CHAR_T*	pGet	= (CTS_GAMECMD_SYNC_CHAR_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_SYNC_CHAR;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_sync_hero(const char* param)
{
	tty_id_t	nHeroID		= 0;
	wh_strsplit("I", param, ",", &nHeroID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_SYNC_HERO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_SYNC_HERO_T*	pGet	= (CTS_GAMECMD_SYNC_HERO_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_SYNC_HERO;
	pGet->nHeroID			= nHeroID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_auto_supply(const char* param)
{
	tty_id_t	nInstanceID	= 0;
	int		nVal	= 0;

	wh_strsplit("Id", param, ",", &nInstanceID, &nVal);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_AUTO_SUPPLY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_AUTO_SUPPLY_T*	pReq	= (CTS_GAMECMD_AUTO_SUPPLY_T*)pCCData->data;
	pReq->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd		= CTS_GAMECMD_AUTO_SUPPLY;
	pReq->nInstanceID	= nInstanceID;
	pReq->nVal			= nVal;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_equip(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_EQUIP_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_EQUIP_T*	pGet	= (CTS_GAMECMD_GET_EQUIP_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_EQUIP;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_gem(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_GEM_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_GEM_T*	pGet	= (CTS_GAMECMD_GET_GEM_T*)pCCData->data;
	pGet->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pGet->nGameCmd			= CTS_GAMECMD_GET_GEM;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_manual_supply(const char* param)
{
	tty_id_t	nInstanceID	= 0;
	int			nCombatType = 0;
	tty_id_t	n1Hero		= 0;
	tty_id_t	n2Hero		= 0;
	tty_id_t	n3Hero		= 0;
	tty_id_t	n4Hero		= 0;
	tty_id_t	n5Hero		= 0;

	wh_strsplit("IdIIIII", param, ",", &nInstanceID, &nCombatType, &n1Hero, &n2Hero, &n3Hero, &n4Hero, &n5Hero);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_MANUAL_SUPPLY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_MANUAL_SUPPLY_T*	pReq	= (CTS_GAMECMD_MANUAL_SUPPLY_T*)pCCData->data;
	pReq->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd		= CTS_GAMECMD_MANUAL_SUPPLY;
	pReq->nInstanceID	= nInstanceID;
	pReq->nCombatType	= nCombatType;
	pReq->n1Hero		= n1Hero;
	pReq->n2Hero		= n2Hero;
	pReq->n3Hero		= n3Hero;
	pReq->n4Hero		= n4Hero;
	pReq->n5Hero		= n5Hero;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_replace_equip(const char* param)
{
	tty_id_t	nHeroID	= 0;
	int			nEquipType	= 0;
	tty_id_t	nItemID	= 0;;

	wh_strsplit("IdI", param, ",", &nHeroID, &nEquipType, &nItemID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_REPLACE_EQUIP_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_REPLACE_EQUIP_T*	pReq	= (CTS_GAMECMD_REPLACE_EQUIP_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_REPLACE_EQUIP;
	pReq->nHeroID			= nHeroID;
	pReq->nEquipType		= nEquipType;
	pReq->nItemID			= nItemID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_worldgoldmine(const char* param)
{
	int nArea	= 0;
	int nClass	= 0;
	int nIdx	= 0;

	wh_strsplit("ddd", param, ",", &nArea, &nClass, &nIdx);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_WORLDGOLDMINE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_WORLDGOLDMINE_T*	pReq	= (CTS_GAMECMD_GET_WORLDGOLDMINE_T*)pCCData->data;
	pReq->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd		= CTS_GAMECMD_GET_WORLDGOLDMINE;
	pReq->nArea			= nArea;
	pReq->nClass		= nClass;
	pReq->nIdx			= nIdx;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_config_worldgoldmine_hero(const char* param)
{
	tty_id_t		n1Hero	= 0;
	tty_id_t		n2Hero	= 0;
	tty_id_t		n3Hero	= 0;
	tty_id_t		n4Hero	= 0;
	tty_id_t		n5Hero	= 0;

	wh_strsplit("IIIII", param, ",", &n1Hero, &n2Hero, &n3Hero, &n4Hero, &n5Hero);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T*	pReq	= (CTS_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T*)pCCData->data;
	pReq->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd		= CTS_GAMECMD_CONFIG_WORLDGOLDMINE_HERO;
	pReq->n1Hero		= n1Hero;
	pReq->n2Hero		= n2Hero;
	pReq->n3Hero		= n3Hero;
	pReq->n4Hero		= n4Hero;
	pReq->n5Hero		= n5Hero;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_rob_worldgoldmine(const char* param)
{
	int				nArea	= 0;
	int				nClass	= 0;
	int				nIdx	= 0;
	tty_id_t		n1Hero	= 0;
	tty_id_t		n2Hero	= 0;
	tty_id_t		n3Hero	= 0;
	tty_id_t		n4Hero	= 0;
	tty_id_t		n5Hero	= 0;

	wh_strsplit("dddIIIII", param, ",", &nArea, &nClass, &nIdx, &n1Hero, &n2Hero, &n3Hero, &n4Hero, &n5Hero);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ROB_WORLDGOLDMINE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ROB_WORLDGOLDMINE_T*	pReq	= (CTS_GAMECMD_ROB_WORLDGOLDMINE_T*)pCCData->data;
	pReq->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd		= CTS_GAMECMD_ROB_WORLDGOLDMINE;
	pReq->nArea			= nArea;
	pReq->nClass		= nClass;
	pReq->nIdx			= nIdx;
	pReq->n1Hero		= n1Hero;
	pReq->n2Hero		= n2Hero;
	pReq->n3Hero		= n3Hero;
	pReq->n4Hero		= n4Hero;
	pReq->n5Hero		= n5Hero;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_drop_worldgoldmine(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_DROP_WORLDGOLDMINE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_DROP_WORLDGOLDMINE_T*	pReq	= (CTS_GAMECMD_DROP_WORLDGOLDMINE_T*)pCCData->data;
	pReq->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd		= CTS_GAMECMD_DROP_WORLDGOLDMINE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_my_worldgoldmine(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_MY_WORLDGOLDMINE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_MY_WORLDGOLDMINE_T*	pReq	= (CTS_GAMECMD_MY_WORLDGOLDMINE_T*)pCCData->data;
	pReq->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd		= CTS_GAMECMD_MY_WORLDGOLDMINE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_combat_prof(const char* param)
{
	int				nClassID	= 0;
	tty_id_t		nObjID = 0;
	int				nCombatType = 0;

	wh_strsplit("dId", param, ",", &nClassID, &nObjID, &nCombatType);
	
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_COMBAT_PROF_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_COMBAT_PROF_T*	pSC	= (CTS_GAMECMD_COMBAT_PROF_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_COMBAT_PROF;
	pSC->nClassID		= nClassID;
	pSC->nObjID			= nObjID;
	pSC->nCombatType	= nCombatType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_contribute_alliance(const char* param)
{
	int				nGold	= 0;
	wh_strsplit("d", param, ",", &nGold);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T*	pReq	= (CTS_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE;
	pReq->nGold				= nGold;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_convert_diamond_gold(const char* param)
{
	int				nDiamond	= 0;
	wh_strsplit("d", param, ",", &nDiamond);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T*	pReq	= (CTS_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD;
	pReq->nDiamond			= nDiamond;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_join_alliance_name(const char* param)
{
	char				szAllianceName[TTY_ALLIANCENAME_LEN];
	szAllianceName[0]	= 0;
	wh_strsplit("s", param, "", szAllianceName);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_JOIN_ALLI_NAME_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_JOIN_ALLI_NAME_T*	pReq	= (CTS_GAMECMD_OPERATE_JOIN_ALLI_NAME_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_JOIN_ALLI_NAME;
	WH_STRNCPY0(pReq->szAllianceName, szAllianceName);
	
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_my_alliance_join(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T*	pReq	= (CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT;

	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_card_by_id(const char* param)
{
	tty_id_t	nAllianceID	= 0;
	wh_strsplit("I", param, ",", &nAllianceID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T*	pReq	= (CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_CARD_BY_ID;
	pReq->nAllianceID		= nAllianceID;

	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_card_by_rank(const char* param)
{
	unsigned int	nFrom	= 0;
	unsigned int	nTo		= 0;
	wh_strsplit("dd", param, ",", &nFrom, &nTo);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T*	pReq	= (CTS_GAMECMD_GET_ALLIANCE_CARD_BY_RANK_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_CARD_BY_RANK;
	pReq->nFromRank			= nFrom;
	pReq->nToRank			= nTo;

	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_set_alliance_introduction(const char* param)
{
	char	szIntroduction[TTY_INTRODUCTION_LEN];
	wh_strsplit("s", param, ",", szIntroduction);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_OPERATE_SET_ALLI_INTRO_T, szIntroduction) + TTY_INTRODUCTION_LEN);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_SET_ALLI_INTRO_T*	pReq	= (CTS_GAMECMD_OPERATE_SET_ALLI_INTRO_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_SET_ALLI_INTRO;
	pReq->nLen				= strlen(szIntroduction)+1;
	memcpy(pReq->szIntroduction, szIntroduction, pReq->nLen);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_OPERATE_SET_ALLI_INTRO_T, szIntroduction) + pReq->nLen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
// int		CStrCmdDealer::_i_srad_draw_lottery(const char* param)
// {
// 	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_DRAW_LOTTERY_T));
// 	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
// 	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
// 	CTS_GAMECMD_OPERATE_DRAW_LOTTERY_T*	pReq	= (CTS_GAMECMD_OPERATE_DRAW_LOTTERY_T*)pCCData->data;
// 	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
// 	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_DRAW_LOTTERY;
// 	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
// 	return 0;
// }
int		CStrCmdDealer::_i_srad_get_other_goldore(const char* param)
{
	tty_id_t	nAccountID	= 0;
	wh_strsplit("I", param, ",", &nAccountID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_OTHER_GOLDORE_INFO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_OTHER_GOLDORE_INFO_T*	pReq	= (CTS_GAMECMD_GET_OTHER_GOLDORE_INFO_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_OTHER_GOLDORE_INFO;
	pReq->nAccountID		= nAccountID;

	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_check_draw_lottery(const char* param)
{
	unsigned int		nType	= 0;
	wh_strsplit("d", param, "", &nType);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T*	pReq	= (CTS_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY;
	pReq->nType				= nType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_fetch_lottery(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_FETCH_LOTTERY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_FETCH_LOTTERY_T*	pReq	= (CTS_GAMECMD_OPERATE_FETCH_LOTTERY_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_FETCH_LOTTERY;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_christmas_tree(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_CHRISTMAS_TREE_INFO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_CHRISTMAS_TREE_INFO_T*	pReq	= (CTS_GAMECMD_GET_CHRISTMAS_TREE_INFO_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_CHRISTMAS_TREE_INFO;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_goldore_smp_info_all(const char* param)
{
	tty_id_t	nID			= 0;
	char		szBuf[1024];
	strcpy(szBuf, param);
	szBuf[strlen(param)]	= 0;
	whvector<tty_id_t>	vectIDs;
	while(wh_strsplit("Ia", szBuf, ",", &nID, szBuf) == 2)
	{
		if (nID != 0)
		{
			vectIDs.push_back(nID);
		}
	}
	if (nID != 0)
	{
		vectIDs.push_back(nID);
	}
	int			nIDsSize	= vectIDs.size()*sizeof(tty_id_t);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T) + nIDsSize);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T*	pReq	= (CTS_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_GOLDORE_SMP_INFO_ALL;
	pReq->nNum				= vectIDs.size();
	memcpy(wh_getptrnexttoptr(pReq), vectIDs.getbuf(), nIDsSize);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_alliance_trade_info(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ALLIANCE_TRADE_INFO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ALLIANCE_TRADE_INFO_T*	pReq	= (CTS_GAMECMD_GET_ALLIANCE_TRADE_INFO_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_ALLIANCE_TRADE_INFO;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alliance_trade(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ALLIANCE_TRADE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ALLIANCE_TRADE_T*	pReq	= (CTS_GAMECMD_OPERATE_ALLIANCE_TRADE_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_ALLIANCE_TRADE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_enemy_list(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_ENEMY_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_ENEMY_LIST_T*	pReq	= (CTS_GAMECMD_GET_ENEMY_LIST_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_ENEMY_LIST;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_delete_enemy(const char* param)
{
	tty_id_t	nEnemyID	= 0;
	wh_strsplit("I", param, "", &nEnemyID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_DELETE_ENEMY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_DELETE_ENEMY_T*	pReq	= (CTS_GAMECMD_OPERATE_DELETE_ENEMY_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_DELETE_ENEMY;
	pReq->nEnemyID			= nEnemyID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_rank_list(const char* param)
{
	unsigned int	nType	= 0;
	unsigned int	nFrom	= 0;
	unsigned int	nNum	= 0;
	wh_strsplit("ddd", param, ",", &nType, &nFrom, &nNum);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_RANK_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_RANK_LIST_T*	pReq	= (CTS_GAMECMD_GET_RANK_LIST_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_RANK_LIST;
	pReq->nType				= nType;
	pReq->nFrom				= nFrom;
	pReq->nNum				= nNum;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_my_rank(const char* param)
{
	unsigned int		nType	= 0;
	wh_strsplit("d", param, "", &nType);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_MY_RANK_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_MY_RANK_T*	pReq	= (CTS_GAMECMD_GET_MY_RANK_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_MY_RANK;
	pReq->nType				= nType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_write_signature(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_WRITE_SIGNATURE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_WRITE_SIGNATURE_T*	pReq	= (CTS_GAMECMD_OPERATE_WRITE_SIGNATURE_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_WRITE_SIGNATURE;
	wh_strsplit("a", param, "", pReq->szSignature);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_simple_char_atb(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_IMPORTANT_CHAR_ATB_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_IMPORTANT_CHAR_ATB_T*	pReq	= (CTS_GAMECMD_GET_IMPORTANT_CHAR_ATB_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_IMPORTANT_CHAR_ATB;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_change_name(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CHANGE_NAME_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CHANGE_NAME_T*	pReq	= (CTS_GAMECMD_OPERATE_CHANGE_NAME_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_CHANGE_NAME;
	pReq->szNewName[0]		= 0;
	wh_strsplit("s", param, "", pReq->szNewName);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_move_position(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_MOVE_CITY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_MOVE_CITY_T*	pReq	= (CTS_GAMECMD_OPERATE_MOVE_CITY_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_MOVE_CITY;
	pReq->nPosX				= 0;
	pReq->nPosY				= 0;
	wh_strsplit("dd", param, ",", &pReq->nPosX, &pReq->nPosY);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_pay_serial(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_PAY_SERIAL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_PAY_SERIAL_T*	pReq	= (CTS_GAMECMD_GET_PAY_SERIAL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_PAY_SERIAL;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_read_notification(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_READ_NOTIFICATION_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_READ_NOTIFICATION_T*	pReq	= (CTS_GAMECMD_OPERATE_READ_NOTIFICATION_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_READ_NOTIFICATION;
	pReq->nNotificationID	= 0;
	wh_strsplit("d", param, "", &pReq->nNotificationID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_change_hero_name(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CHANGE_HERO_NAME_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CHANGE_HERO_NAME_T*	pReq	= (CTS_GAMECMD_OPERATE_CHANGE_HERO_NAME_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_CHANGE_HERO_NAME;
	pReq->szNewName[0]		= 0;
	wh_strsplit("Is", param, "", &pReq->nHeroID, pReq->szNewName);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_read_mail_all(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_READ_MAIL_ALL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_READ_MAIL_ALL_T*	pReq	= (CTS_GAMECMD_READ_MAIL_ALL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_READ_MAIL_ALL;
	pReq->nMailType			= 0;
	wh_strsplit("d", param, "", &pReq->nMailType);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_delete_mail_all(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_DELETE_MAIL_ALL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_DELETE_MAIL_ALL_T*	pReq	= (CTS_GAMECMD_DELETE_MAIL_ALL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_DELETE_MAIL_ALL;
	pReq->nMailType			= 0;
	wh_strsplit("d", param, "", &pReq->nMailType);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_add_gm_mail(const char* param)
{
	char	szBuf[TTY_MAIL_TEXT_LEN];
	wh_strsplit("a", param, "", szBuf);
	int		nLength			= strlen(szBuf)+1;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ADD_GM_MAIL_T) + nLength);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ADD_GM_MAIL_T*	pReq	= (CTS_GAMECMD_OPERATE_ADD_GM_MAIL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_ADD_GM_MAIL;
	pReq->nLength			= nLength;
	memcpy(wh_getptrnexttoptr(pReq), szBuf, nLength);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_set_alli_name(const char* param)
{
	char	szBuf[TTY_ALLIANCENAME_LEN];
	wh_strsplit("s", param, "", szBuf);
	int		nLength			= strlen(szBuf)+1;
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_SET_ALLI_NAME_T) + nLength);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_SET_ALLI_NAME_T*	pReq	= (CTS_GAMECMD_OPERATE_SET_ALLI_NAME_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_SET_ALLI_NAME;
	WH_STRNCPY0(pReq->szName, szBuf);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_famous_city_list(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T*	pReq	= (CTS_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_WORLD_FAMOUS_CITY_LIST;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_fetch_mail_reward(const char* param)
{
	unsigned int	nMailID	= 0;
	wh_strsplit("d", param, "", &nMailID);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T*	pReq	= (CTS_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_FETCH_MAIL_REWARD;
	pReq->nMailID			= nMailID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_upload_bill(const char* param)
{
	char	szText[4*1024];
	wh_strsplit("s", param, ",", szText);
	int		nTextlen	= strlen(szText);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + wh_offsetof(CTS_GAMECMD_OPERATE_UPLOAD_BILL_T, szText) + nTextlen);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_UPLOAD_BILL_T*	pReq	= (CTS_GAMECMD_OPERATE_UPLOAD_BILL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_UPLOAD_BILL;
	pReq->nClientTime		= wh_time();
	pReq->nTextLen			= nTextlen;
	memcpy(pReq->szText, szText, nTextlen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_register(const char* param)
{
	char	szAccount[TTY_ACCOUNTNAME_LEN]	= "";
	char	szPass[TTY_ACCOUNTPASSWORD_LEN]	= "";
	char	szPassOld[TTY_ACCOUNTPASSWORD_LEN]	= "";
	wh_strsplit("sss", param, ",", szAccount, szPass, szPassOld);
	int		nPassLen	= strlen(szPass);
	int		nPassOldLen	= strlen(szPassOld);
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_REGISTER_ACCOUNT_T) + nPassLen + sizeof(int)+nPassOldLen);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_REGISTER_ACCOUNT_T*	pReq	= (CTS_GAMECMD_OPERATE_REGISTER_ACCOUNT_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_REGISTER_ACCOUNT;
	WH_STRNCPY0(pReq->szAccount, szAccount);
	pReq->nPassLen			= nPassLen;
	char*	pPass			= (char*)wh_getptrnexttoptr(pReq);
	memcpy(pPass, szPass, nPassLen);
	int*	pPassOldLen		= (int*)wh_getoffsetaddr(pPass, nPassLen);
	*pPassOldLen			= nPassOldLen;
	memcpy(wh_getptrnexttoptr(pPassOldLen), szPassOld, nPassOldLen);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_bind_device(const char* param)
{
	char	szDeviceID[TTY_DEVICE_ID_LEN]	= "";
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_BIND_DEVICE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_BIND_DEVICE_T*	pReq	= (CTS_GAMECMD_OPERATE_BIND_DEVICE_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_BIND_DEVICE;
	WH_STRNCPY0(pReq->szDeviceID, szDeviceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_server_time(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_SERVER_TIME_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_SERVER_TIME_T*	pReq	= (CTS_GAMECMD_GET_SERVER_TIME_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_SERVER_TIME;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_dismiss_soldier(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_DISMISS_SOLDIER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_DISMISS_SOLDIER_T*	pReq	= (CTS_GAMECMD_OPERATE_DISMISS_SOLDIER_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_DISMISS_SOLDIER;
	wh_strsplit("ddd", param, ",", &pReq->nExcelID, &pReq->nLevel, &pReq->nNum);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_world_res_card(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_WORLD_RES_CARD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_WORLD_RES_CARD_T*	pReq	= (CTS_GAMECMD_GET_WORLD_RES_CARD_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_WORLD_RES_CARD;
	pReq->nID				= 0;
	wh_strsplit("I", param, ",", &pReq->nID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_alli_donate_soldier(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T*	pReq	= (CTS_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER;
	pReq->nExcelID			= 0;
	pReq->nLevel			= 0;
	pReq->nNum				= 0;
	pReq->nObjID			= 0;
	wh_strsplit("Iddd", param, ",", &pReq->nObjID, &pReq->nExcelID, &pReq->nLevel, &pReq->nNum);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_recall_soldier(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T*	pReq	= (CTS_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER;
	pReq->nEventID			= 0;
	wh_strsplit("I", param, ",", &pReq->nEventID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_acce_donate_soldier(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T*	pReq	= (CTS_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER;
	pReq->nEventID			= 0;
	pReq->nTime				= 0;
	pReq->nMoneyType		= 0;
	wh_strsplit("Idd", param, ",", &pReq->nEventID, &pReq->nTime, &pReq->nMoneyType);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_donate_soldier_queue(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T*	pReq	= (CTS_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_DONATE_SOLDIER_QUEUE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_left_donate_soldier(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_LEFT_DONATE_SOLDIER_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_LEFT_DONATE_SOLDIER_T*	pReq	= (CTS_GAMECMD_GET_LEFT_DONATE_SOLDIER_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_LEFT_DONATE_SOLDIER;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_add_gold_deal(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_ADD_GOLD_DEAL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_ADD_GOLD_DEAL_T*	pReq	= (CTS_GAMECMD_OPERATE_ADD_GOLD_DEAL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_ADD_GOLD_DEAL;
	pReq->nGold				= 0;
	pReq->nPrice			= 0;
	wh_strsplit("dd", param, ",", &pReq->nGold, &pReq->nPrice);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_buy_gold_in_market(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T*	pReq	= (CTS_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET;
	pReq->nTransactionID	= 0;
	pReq->nMoneyType		= 0;
	wh_strsplit("Id", param, ",", &pReq->nTransactionID, &pReq->nMoneyType);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_cancel_gold_deal(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T*	pReq	= (CTS_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_OPERATE_CANCEL_GOLD_DEAL;
	pReq->nTransactionID	= 0;
	wh_strsplit("I", param, ",", &pReq->nTransactionID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_gold_deal(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_GOLD_DEAL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_GOLD_DEAL_T*	pReq	= (CTS_GAMECMD_GET_GOLD_DEAL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_GOLD_DEAL;
	pReq->nPage				= 0;
	pReq->nPageSize			= 0;
	wh_strsplit("dd", param, ",", &pReq->nPage, &pReq->nPageSize);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_get_self_gold_deal(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_SELF_GOLD_DEAL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_SELF_GOLD_DEAL_T*	pReq	= (CTS_GAMECMD_GET_SELF_GOLD_DEAL_T*)pCCData->data;
	pReq->nCmd				= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pReq->nGameCmd			= CTS_GAMECMD_GET_SELF_GOLD_DEAL;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_get_citydefense(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_CITYDEFENSE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_CITYDEFENSE_T*	pSC	= (CTS_GAMECMD_GET_CITYDEFENSE_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_GET_CITYDEFENSE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int dbs_splitbychar(const char *src, char tag, int &idx, char *dst)
{
	const char *p = src+idx;
	if (*p == 0)
		return 0;
	
	*dst = 0;
	while (*p)
	{
		++ idx;
		if (*p == tag)
			break;

		*dst++ = *p++;
	}
	*dst = 0;
	return idx;
}
int		CStrCmdDealer::_i_srad_use_drug(const char* param)
{
	int nNum	= 0;
	char szBuf[1024];
	wh_strsplit("da", param, ",", &nNum, szBuf);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_USE_DRUG_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_USE_DRUG_T*	pSC	= (CTS_GAMECMD_USE_DRUG_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_USE_DRUG;
	pSC->nNum			= nNum;
	tty_id_t *pID		= (tty_id_t*)wh_getptrnexttoptr(pSC);
	int idx=0;
	char tmp[32];
	for (int i=0; i<nNum; ++i)
	{
		if (dbs_splitbychar(szBuf, ',', idx, tmp) > 0)
		{
			*pID	= whstr2int64(tmp);
		}
		else
		{
			*pID	= 0;
		}
		++ pID;
	}
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), (char*)pID-(char*)pCCData);
	return 0;
}
int		CStrCmdDealer::_i_srad_army_accelerate(const char* param)
{
	tty_id_t nID	= 0;
	int nTime		= 0;
	unsigned char byType	=0;
	wh_strsplit("Idy", param, ",", &nID, &nTime, byType);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ARMY_ACCELERATE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ARMY_ACCELERATE_T*	pSC	= (CTS_GAMECMD_ARMY_ACCELERATE_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ARMY_ACCELERATE;
	pSC->nCombatID		= nID;
	pSC->nTime			= nTime;
	pSC->byType			= byType;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_army_back(const char* param)
{
	tty_id_t nID	= 0;
	wh_strsplit("I", param, ",", &nID);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ARMY_BACK_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ARMY_BACK_T*	pSC	= (CTS_GAMECMD_ARMY_BACK_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ARMY_BACK;
	pSC->nCombatID		= nID;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_worldcity(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_WORLDCITY_GET_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_WORLDCITY_GET_T*	pSC	= (CTS_GAMECMD_WORLDCITY_GET_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_WORLDCITY_GET;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_worldcity_log(const char* param)
{
	int				nPageNum = 0;
	int				nPageIdx = 0;
	wh_strsplit("dd", param, ",", &nPageNum, &nPageIdx);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_WORLDCITY_GETLOG_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_WORLDCITY_GETLOG_T*	pSC	= (CTS_GAMECMD_WORLDCITY_GETLOG_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_WORLDCITY_GETLOG;
	pSC->nPageNum		= nPageNum;
	pSC->nPageIdx		= nPageIdx;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_worldcity_log_alliance(const char* param)
{
	int				nPageNum = 0;
	int				nPageIdx = 0;
	wh_strsplit("dd", param, ",", &nPageNum, &nPageIdx);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T*	pSC	= (CTS_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_WORLDCITY_GETLOG_ALLIANCE;
	pSC->nPageNum		= nPageNum;
	pSC->nPageIdx		= nPageIdx;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_worldcity_rank_man(const char* param)
{
	int				nPageNum = 0;
	int				nPageIdx = 0;
	wh_strsplit("dd", param, ",", &nPageNum, &nPageIdx);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_WORLDCITY_RANK_MAN_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_WORLDCITY_RANK_MAN_T*	pSC	= (CTS_GAMECMD_WORLDCITY_RANK_MAN_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_WORLDCITY_RANK_MAN;
	pSC->nPageNum		= nPageNum;
	pSC->nPageIdx		= nPageIdx;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_worldcity_rank_alliance(const char* param)
{
	int				nPageNum = 0;
	int				nPageIdx = 0;
	wh_strsplit("dd", param, ",", &nPageNum, &nPageIdx);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_WORLDCITY_RANK_ALLIANCE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_WORLDCITY_RANK_ALLIANCE_T*	pSC	= (CTS_GAMECMD_WORLDCITY_RANK_ALLIANCE_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_WORLDCITY_RANK_ALLIANCE;
	pSC->nPageNum		= nPageNum;
	pSC->nPageIdx		= nPageIdx;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_recharge_try(const char* param)
{
	int				nDiamond = 0;
	wh_strsplit("d", param, ",", &nDiamond);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_RECHARGE_TRY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_RECHARGE_TRY_T*	pSC	= (CTS_GAMECMD_RECHARGE_TRY_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_RECHARGE_TRY;
	pSC->nDiamond		= nDiamond;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_alli_instance_create(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_CREATE_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_CREATE_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_CREATE_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_CREATE;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_get_list(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_GET_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_GET_LIST_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_GET_LIST_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_GET_LIST;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_join(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_JOIN_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_JOIN_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_JOIN_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_JOIN;
	pSC->nInstanceID	= 0;
	wh_strsplit("I", param, ",", &pSC->nInstanceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_exit(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_EXIT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_EXIT_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_EXIT_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_EXIT;
	pSC->nInstanceID	= 0;
	wh_strsplit("I", param, ",", &pSC->nInstanceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_kick(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_KICK_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_KICK_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_KICK_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_KICK;
	pSC->nInstanceID	= 0;
	pSC->nObjID			= 0;
	wh_strsplit("II", param, ",", &pSC->nInstanceID, &pSC->nObjID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_destroy(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_DESTROY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_DESTROY_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_DESTROY_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_DESTROY;
	pSC->nInstanceID	= 0;
	wh_strsplit("I", param, ",", &pSC->nInstanceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_config_hero(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_CONFIG_HERO;
	pSC->nInstanceID	= 0;
	pSC->n1HeroID		= 0;
	pSC->n2HeroID		= 0;
	pSC->n3HeroID		= 0;
	pSC->n4HeroID		= 0;
	pSC->n5HeroID		= 0;
	wh_strsplit("IIIIII", param, ",", &pSC->nInstanceID
		, &pSC->n1HeroID, &pSC->n2HeroID, &pSC->n3HeroID, &pSC->n4HeroID, &pSC->n5HeroID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_start(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_START_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_START_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_START_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_START;
	pSC->nInstanceID	= 0;
	wh_strsplit("I", param, ",", &pSC->nInstanceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_ready(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_READY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_READY_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_READY_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_READY;
	pSC->nInstanceID	= 0;
	pSC->bReady			= false;
	wh_strsplit("Ib", param, ",", &pSC->nInstanceID, &pSC->bReady);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_get_char_data(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_get_hero_data(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_get_status(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_GET_STATUS_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_GET_STATUS_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_GET_STATUS_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_GET_STATUS;
	pSC->nCombatType	= 0;
	wh_strsplit("d", param, ",", &pSC->nCombatType);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_save_hero_deploy(const char* param)
{
	whvector<HeroDeploy>	vect;
	int		nOffset			= 0;
	unsigned int	nCol	= 0;
	unsigned int	nRow	= 0;
	tty_id_t		nHeroID	= 0;
	while (wh_strsplit(&nOffset, "ddI", param, ",", &nCol, &nRow, &nHeroID) == 3)
	{
		HeroDeploy* pUnit	= vect.push_back();
		pUnit->nCol			= nCol;
		pUnit->nRow			= nRow;
		pUnit->nHeroID		= nHeroID;
	}
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T) + vect.size()*sizeof(HeroDeploy));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY;
	pSC->nSize			= vect.size()*sizeof(HeroDeploy);
	// col,row,hero_id
	memcpy(wh_getptrnexttoptr(pSC), vect.getbuf(), pSC->nSize);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_get_combat_log(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG;
	pSC->nCombatType	= 0;
	pSC->nID	= 0;
	wh_strsplit("dI", param, ",", &pSC->nCombatType, &pSC->nID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_get_simple_combat_log(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG;
	pSC->nCombatType	= 0;
	pSC->nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &pSC->nCombatType, &pSC->nInstanceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_stop_combat(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_STOP_COMBAT;
	pSC->nCombatType	= 0;
	pSC->nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &pSC->nCombatType, &pSC->nInstanceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_supply(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_SUPPLY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_SUPPLY_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_SUPPLY_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_SUPPLY;
	pSC->nCombatType	= 0;
	pSC->nInstanceID	= 0;
	wh_strsplit("dI", param, ",", &pSC->nCombatType, &pSC->nInstanceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_get_loot(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_GET_LOOT_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_GET_LOOT;
	pSC->nInstanceID	= 0;
	wh_strsplit("I", param, ",", &pSC->nInstanceID);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_alli_instance_get_defender_deploy(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*	pSC	= (CTS_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY;
	pSC->nExcelID		= 0;
	pSC->nLevel			= 0;
	wh_strsplit("dd", param, ",", &pSC->nExcelID, &pSC->nLevel);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_arena_upload_data(const char* param)
{
	whvector<HeroDeploy>	vect;
	int		nOffset			= 0;
	unsigned int	nCol	= 0;
	unsigned int	nRow	= 0;
	tty_id_t		nHeroID	= 0;
	while (wh_strsplit(&nOffset, "ddI", param, ",", &nCol, &nRow, &nHeroID) == 3)
	{
		HeroDeploy* pUnit	= vect.push_back();
		pUnit->nCol			= nCol;
		pUnit->nRow			= nRow;
		pUnit->nHeroID		= nHeroID;
	}
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ARENA_UPLOAD_DATA_T) + vect.size()*sizeof(HeroDeploy));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ARENA_UPLOAD_DATA_T*	pSC	= (CTS_GAMECMD_ARENA_UPLOAD_DATA_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ARENA_UPLOAD_DATA;
	pSC->nSize			= vect.size()*sizeof(HeroDeploy);
	// col,row,hero_id
	memcpy(wh_getptrnexttoptr(pSC), vect.getbuf(), pSC->nSize);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_arena_get_status(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ARENA_GET_STATUS_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ARENA_GET_STATUS_T*	pSC	= (CTS_GAMECMD_ARENA_GET_STATUS_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ARENA_GET_STATUS;
	pSC->nNum			= 0;
	wh_strsplit("d", param, ",", &pSC->nNum);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_arena_pay(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ARENA_PAY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ARENA_PAY_T*	pSC	= (CTS_GAMECMD_ARENA_PAY_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ARENA_PAY;
	pSC->nMoneyType		= 0;
	wh_strsplit("d", param, ",", &pSC->nMoneyType);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_arena_get_rank_list(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ARENA_GET_RANK_LIST_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ARENA_GET_RANK_LIST_T*	pSC	= (CTS_GAMECMD_ARENA_GET_RANK_LIST_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ARENA_GET_RANK_LIST;
	pSC->nPage			= 0;
	pSC->nPageSize		= 0;
	wh_strsplit("dd", param, ",", &pSC->nPage, &pSC->nPageSize);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_arena_get_deploy(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_ARENA_GET_DEPLOY_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_ARENA_GET_DEPLOY_T*	pSC	= (CTS_GAMECMD_ARENA_GET_DEPLOY_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_ARENA_GET_DEPLOY;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_get_kick_client_all(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_GET_KICK_CLIENT_ALL_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_GET_KICK_CLIENT_ALL_T*	pSC	= (CTS_GAMECMD_GET_KICK_CLIENT_ALL_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_GET_KICK_CLIENT_ALL;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}

int		CStrCmdDealer::_i_srad_position_mark_add(const char* param)
{
	unsigned int	nPosX	= 0;
	unsigned int	nPosY	= 0;
	unsigned int	nHeadID	= 0;
	unsigned int	nLength	= 0;
	char	szBuf[TTY_POSITION_MARK_PROMPT_LEN]	= "";
	wh_strsplit("ddds", param, ",", &nPosX, &nPosY, &nHeadID, szBuf);
	nLength					= strlen(szBuf);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_POSITION_MARK_ADD_RECORD_T)
		+ sizeof(PositionMark)+nLength);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_POSITION_MARK_ADD_RECORD_T*	pSC	= (CTS_GAMECMD_POSITION_MARK_ADD_RECORD_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_POSITION_MARK_ADD_RECORD;
	PositionMark*	pUnit	= (PositionMark*)wh_getptrnexttoptr(pSC);
	pUnit->nPosX		= nPosX;
	pUnit->nPosY		= nPosY;
	pUnit->nHeadID		= nHeadID;
	pUnit->nLength		= nLength;
	memcpy(wh_getptrnexttoptr(pUnit), szBuf, nLength);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_position_mark_chg(const char* param)
{
	unsigned int	nPosX	= 0;
	unsigned int	nPosY	= 0;
	unsigned int	nHeadID	= 0;
	unsigned int	nLength	= 0;
	char	szBuf[TTY_POSITION_MARK_PROMPT_LEN]	= "";
	wh_strsplit("ddds", param, ",", &nPosX, &nPosY, &nHeadID, szBuf);
	nLength					= strlen(szBuf);

	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_POSITION_MARK_CHG_RECORD_T)
		+ sizeof(PositionMark)+nLength);
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_POSITION_MARK_CHG_RECORD_T*	pSC	= (CTS_GAMECMD_POSITION_MARK_CHG_RECORD_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_POSITION_MARK_CHG_RECORD;
	PositionMark*	pUnit	= (PositionMark*)wh_getptrnexttoptr(pSC);
	pUnit->nPosX		= nPosX;
	pUnit->nPosY		= nPosY;
	pUnit->nHeadID		= nHeadID;
	pUnit->nLength		= nLength;
	memcpy(wh_getptrnexttoptr(pUnit), szBuf, nLength);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_position_mark_del(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_POSITION_MARK_DEL_RECORD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_POSITION_MARK_DEL_RECORD_T*	pSC	= (CTS_GAMECMD_POSITION_MARK_DEL_RECORD_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_POSITION_MARK_DEL_RECORD;
	pSC->nPosX			= 0;
	pSC->nPosY			= 0;
	wh_strsplit("dd", param, ",", &pSC->nPosX, &pSC->nPosY);
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
int		CStrCmdDealer::_i_srad_position_mark_get(const char* param)
{
	m_vectrawbuf.resize(wh_offsetof(CLS4Web_CLIENT_DATA_T, data) + sizeof(CTS_GAMECMD_POSITION_MARK_GET_RECORD_T));
	CLS4Web_CLIENT_DATA_T*	pCCData			= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pCCData->nCmd			= CLIENT_CLS4Web_DATA;
	CTS_GAMECMD_POSITION_MARK_GET_RECORD_T*	pSC	= (CTS_GAMECMD_POSITION_MARK_GET_RECORD_T*)pCCData->data;
	pSC->nCmd			= TTY_CLIENT_LPGAMEPLAY_GAME_CMD;
	pSC->nGameCmd		= CTS_GAMECMD_POSITION_MARK_GET_RECORD;
	g_pEpollConnecterClient->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
