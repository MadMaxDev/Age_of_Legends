#include "../inc/LPGamePlay4Web_i.h"
using namespace n_pngs;

WHDATAPROP_MAP_BEGIN_AT_ROOT(LPGamePlay4Web_i::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nQueueCmdInSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(short, nSvrGrpID, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxPlayer, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nNoLoginTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nPlayerDropWaitTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nPlayerCloseDelayTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxTransaction, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nClearOfflineTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTransactionTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxCharPerAccount, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bForceAccountNameLwr, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nReplaceCount, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nReplaceTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szGS4WebAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nDealDBTimeEventTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bCreateAccountIfNotExist, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bUseRandomAccountName, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nDealPayTimeOut, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szBadCharNameFile, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szBadAccountNameFile, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nDealCombatResultTimeOut, 0)
WHDATAPROP_MAP_END()

namespace n_pngs
{
	CLPNet*	g_pLPNet	= NULL;
}

LPGamePlay4Web_i::LPGamePlay4Web_i()
: m_tickNow(0)
, m_pTMan(NULL)
, m_pLogicMainStructure(NULL)
, m_pLogicDB(NULL)
, m_pGameMngS(NULL)
, m_bLoadTerrainOK(false)
, m_bLoadPlayerCardOK(false)
, m_bLoadExcelTextOK(false)
, m_bLoadExcelAllianceCongressOK(false)
, m_bLoadCharLevelRankOK(false)
, m_bNeedUpdatePlayerCardLevelRankData(true)
, m_bNeedGenWorldFamousCity(false)
, m_bCanGenWorldFamousCity(false)
, m_bClearWorldFamousCityFirstWhenGen(false)
, m_bAlreadyGetWorldFamousCity(false)
, m_bAlreadyLoadRank(false)
, m_bNeedGetWorldFamousCity(true)
, m_pBadCharNameFilter(NULL)
, m_pBadAccountNameFilter(NULL)
, m_bNeedGetWorldRes(true)	// 世界资源相关
, m_bNeedGenWorldRes(false)
, m_bCanGenWorldRes(false)
, m_bAlreadyGetWorldRes(false)
{
	strcpy(FATHERCLASS::m_szLogicType, TTY_DLL_NAME_LP_GamePlay);
	FATHERCLASS::m_nVer		= LPGamePlaye_VER;
	m_vectrawbuf.reserve(TTY_RAWBUF_SIZE);
	g_pLPNet				= this;
}
LPGamePlay4Web_i::~LPGamePlay4Web_i()
{
}
int		LPGamePlay4Web_i::Organize()
{
	// 获得MainStructure的指针
	m_pLogicMainStructure	= m_pCMN->GetLogic(TTY_DLL_NAME_LP_MainStructure, LPMainStructure_VER);
	if (m_pLogicMainStructure == NULL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1392,LPGP_i)"%s,,m_pLogicMainStructure is NULL", __FUNCTION__);
		return -1;
	}
	// 获取DB接口的指针(找不到就不能继续了)
	m_pLogicDB				= m_pCMN->GetLogic(TTY_DLL_NAME_GS_DB4Web, GSDB4Web_VER);
	if (m_pLogicDB == NULL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1392,LPGP_i)"%s,,m_pLogicDB is NULL", __FUNCTION__);
		return -2;
	}
	{
		// 设置他的使用者为自己
		PNGSPACKET_2DB4WEB_CONFIG_T	Cmd;
		Cmd.nSubCmd					= PNGSPACKET_2DB4WEB_CONFIG_T::SUBCMD_SET_DBUSER;
		if (m_pLogicDB->CmdIn_Instant(this, PNGSPACKET_2DB4WEB_CONFIG, &Cmd, sizeof(Cmd)) < 0)
		{
			return -11;
		}
		// 设置自己是LP
		Cmd.nSubCmd					= PNGSPACKET_2DB4WEB_CONFIG_T::SUBCMD_SET_DBUSERISWHAT;
		Cmd.nParam					= APPTYPE_LP;
		if (m_pLogicDB->CmdIn_Instant(this, PNGSPACKET_2DB4WEB_CONFIG, &Cmd, sizeof(Cmd)) < 0)
		{
			return -12;
		}
	}
	return 0;
}
int		LPGamePlay4Web_i::Detach(bool bQuick)
{
	return 0;
}
void	LPGamePlay4Web_i::SelfDestroy()
{
	delete	this;
}
int		LPGamePlay4Web_i::Init(const char *cszCFGName)
{
	int		nRst	= 0;
	if ((nRst=Init_CFG(cszCFGName)) < 0)
	{
		return -1;
	}
	
	// 初始化时间队列
	whtimequeue::INFO_T		tqinfo;
	tqinfo.nUnitLen			= sizeof(TQUNIT_T);
	tqinfo.nChunkSize		= m_cfginfo.nTQChunkSize;
	if ((nRst=m_TQ.Init(&tqinfo)) < 0)
	{
		return -2;
	}

	if ((nRst=ILogicBase::QueueInit(m_cfginfo.nQueueCmdInSize)) < 0)
	{
		return -3;
	}

	// 初始化transaction管理器
	m_pTMan					= whtransactionman::Create("LPGamePlay4Web");
	whtransactionman::INFO_T	tminfo;
	tminfo.nMaxTransaction	= m_cfginfo.nMaxTransaction;
	tminfo.nTQChunkSize		= m_cfginfo.nTQChunkSize;
	if ((nRst=m_pTMan->Init(&tminfo)) < 0)
	{
		return -4;
	}

	m_msgerGS4Web.m_pHost	= this;
	if ((nRst=m_msgerGS4Web.Init(m_msgerGS4Web_info.GetBase(), m_cfginfo.szGS4WebAddr)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,LPGamePlay4Web_i_INT_RLS)"LPGamePlay4Web_i::Init,m_msgerGS4Web.Init,%d", nRst);
		return -5;
	}

	if (m_Players.Init(m_cfginfo.nMaxPlayer) < 0)
	{
		return -9;
	}

	m_pGameMngS		= new CGameMngS();

	if (m_pGameMngS->Init(&m_GameMngCfg) < 0)
	{
		return -100;
	}

	if ((nRst=SafeInitBadNameFilter()) < 0)
	{
		return -101;
	}

	m_dtlHour.setinterval(3600);
	m_dtlMinute.setinterval(60);

	// 设置处理DB超时时间事件
	SetTE_DealDBTimeEvent();

	// 设置处理Pay超时时间事件
	SetTE_DealPayTimeEvent();

	// 设置处理AdColony的事件
	SetTE_DealAdColonyTimeEvent();

	// 设置处理战斗结果的超时
	SetTE_GetAlliInstanceCombatResultEvent();

	return 0;
}
int		LPGamePlay4Web_i::SafeInitBadNameFilter()
{
	WHSafeSelfDestroy(m_pBadCharNameFilter);
	m_pBadCharNameFilter				= whbadword::Create();
	whbadword::INIT_RST_T			rst;
	memset(&rst, 0, sizeof(rst));
	int		nRst					= m_pBadCharNameFilter->Init(m_cfginfo.szBadCharNameFile, WHCMN_getcharset(), &rst);
	if (nRst != 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BADNAME)"INIT,char_name,%d,%d,%d,%d,%d,%d"
			, nRst, rst.nErrLine, rst.nFitFactor, rst.nTotal, rst.nTotalChar, rst.nTotalEffective);
		WHSafeSelfDestroy(m_pBadCharNameFilter);
		return -1000+nRst;
	}

	m_pBadAccountNameFilter				= whbadword::Create();
	memset(&rst, 0, sizeof(rst));
	nRst							= m_pBadAccountNameFilter->Init(m_cfginfo.szBadAccountNameFile, WHCMN_getcharset(), &rst);
	if (nRst != 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BADNAME)"INIT,account_name,%d,%d,%d,%d,%d,%d"
			, nRst, rst.nErrLine, rst.nFitFactor, rst.nTotal, rst.nTotalChar, rst.nTotalEffective);
		WHSafeSelfDestroy(m_pBadAccountNameFilter);
		return -2000+nRst;
	}
	return 0;
}
int		LPGamePlay4Web_i::Init_CFG(const char* cszCFGName)
{
	int		nRst	= 0;
	WHDATAINI_CMN	ini;
	ini.addobj("LPGamePlay", &m_cfginfo);
	ini.addobj("GameMng", &m_GameMngCfg);
	ini.addobj("MSGER_GS4Web", &m_msgerGS4Web_info);

	// 默认参数
	m_msgerGS4Web_info.nSendBufSize		= 1*1024*1024;
	m_msgerGS4Web_info.nRecvBufSize		= 1*1024*1024;
	m_msgerGS4Web_info.nSockRcvBufSize	= 512*1024;
	m_msgerGS4Web_info.nSockSndBufSize	= 512*1024;

	nRst	= ini.analyzefile(cszCFGName);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1392,LPGP_i)"%s,%d,ini.analyzefile,%s", __FUNCTION__, nRst, ini.printerrreport());
		return -1;
	}

	m_tickNow		= wh_gettickcount();
	m_timeNow		= wh_time();

	// 一些配置需要从LPGamePlay配置中获取
	m_GameMngCfg.nMaxPlayer				= m_cfginfo.nMaxPlayer;
	m_GameMngCfg.nTQChunkSize			= m_cfginfo.nTQChunkSize;

	nRst	= Init_SetParams();
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1392,LPGP_i)"%s,%d,Init_SetParams", __FUNCTION__, nRst);
		return -100+nRst;
	}
	return 0;
}
int		LPGamePlay4Web_i::Init_SetParams()
{
	int		nRst	= 0;

	// 一些需要最开始调整的参数
	if (m_cfginfo.nMaxCharPerAccount > TTY_CHARACTERNUM_PER_ACCOUNT)
	{
		m_cfginfo.nMaxCharPerAccount	= TTY_CHARACTERNUM_PER_ACCOUNT;
	}

	return 0;
}
int		LPGamePlay4Web_i::Init_CFG_Reload(const char *cszCFGName)
{
	int		nRst	= 0;
	WHDATAINI_CMN	ini;
	ini.addobj("LPGamePlay", &m_cfginfo);

	nRst	= ini.analyzefile(cszCFGName, false, 1);
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1392,LPGP_i)"%s,%d,ini.analyzefile,%s", __FUNCTION__, nRst, ini.printerrreport());
		return -1;
	}

	nRst	= Init_SetParams();
	if (nRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1392,LPGP_i)"%s,%d,Init_SetParams", __FUNCTION__, nRst);
		return -100+nRst;
	}
	return 0;
}
int		LPGamePlay4Web_i::Release()
{
	ILogicBase::QueueRelease();
	if (m_pTMan != NULL)
	{
		m_pTMan->Release();
		WHSafeDelete(m_pTMan);
	}

	WHSafeDelete(m_pGameMngS);

	return 0;
}
int		LPGamePlay4Web_i::Tick_BeforeDealCmdIn()
{
	m_tickNow	= wh_gettickcount();
	m_timeNow	= wh_time();

	// 时间事件
	Tick_DealTE();

	m_pGameMngS->Logic();

	// GS4Web相关
	m_msgerGS4Web.Tick();
	Tick_DealGS4WebMsg();

	return 0;
}
int		LPGamePlay4Web_i::Tick_AfterDealCmdIn()
{
	// transaction管理器
	m_pTMan->Tick();

	if (m_dtlHour.check())
	{
		// 1.载入排行榜数据
		m_bAlreadyLoadRank			= false;
		m_pGameMngS->GetRankMng()->LoadRankFromDB();

		// 2.重新载入联盟数据 
		{
			P_DBS4WEB_LOAD_ALL_ALLIANCE_ID_T	Cmd;
			Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd				= CMDID_LOAD_ALL_ALLIANCE_ID_REQ;
			CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
		}

		// 3.销毁副本
		{
			P_DBS4WEB_ALLI_INSTANCE_SYSTEM_DESTROY_T	Cmd;
			Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd				= CMDID_ALLI_INSTANCE_SYSTEM_DESTROY_REQ;
			SendCmdToDB(&Cmd, sizeof(Cmd));
		}

		m_dtlHour.gonext();
	}

	if (m_dtlMinute.check())
	{
		DealExcelTimeEvent();
		DealGoldMarketTimeEvent();

		m_dtlMinute.gonext();
	}

	if (m_bNeedUpdatePlayerCardLevelRankData && m_bLoadCharLevelRankOK && m_bLoadPlayerCardOK)
	{
		m_pGameMngS->GetPlayerCardMng()->UpdateLevelRankData();
		m_bNeedUpdatePlayerCardLevelRankData		= false;
	}

	if (m_bLoadPlayerCardOK && m_bLoadTerrainOK)
	{
		// 假定这种千军一发之际不断线
		if (m_bNeedGetWorldFamousCity)
		{
			P_DBS4WEB_GET_WORLD_FAMOUS_CITY_T		Cmd;
			Cmd.nCmd								= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd								= CMDID_GET_WORLD_FAMOUS_CITY_REQ;
			SendCmdToDB(&Cmd, sizeof(Cmd));

			m_bNeedGetWorldFamousCity				= false;
		}
		if (m_bCanGenWorldFamousCity)
		{
			m_pGameMngS->GetWorldMng()->GenWorldFamousCity(m_bClearWorldFamousCityFirstWhenGen);
			m_bCanGenWorldFamousCity				= false;
			m_bNeedGenWorldFamousCity				= false;
		}

		// 世界资源
		if (m_bNeedGetWorldRes)
		{
			P_DBS4WEB_GET_WORLD_RES_T				Cmd;
			Cmd.nCmd								= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd								= CMDID_GET_WORLD_RES_REQ;
			SendCmdToDB(&Cmd, sizeof(Cmd));

			m_bNeedGetWorldRes						= false;
		}
		if (m_bCanGenWorldRes)
		{
			m_pGameMngS->GetWorldMng()->GenWorldRes();
			m_bCanGenWorldRes						= false;
			m_bNeedGenWorldRes						= false;
		}
	}

	return 0;
}
int		LPGamePlay4Web_i::SureSend()
{
	m_msgerGS4Web.ManualSend();
	return 0;
}
int		LPGamePlay4Web_i::GetSockets(n_whcmn::whvector<SOCKET> &vect)
{
	SOCKET	sock	= m_msgerGS4Web.GetSocket();
	if (cmn_is_validsocket(sock))
	{
		vect.push_back(sock);
	}
	return vect.size();
}
int		LPGamePlay4Web_i::SetPlayerStatus(PlayerUnit* pPlayer, int nStatus)
{
	if (nStatus == PlayerUnit::STATUS_T::INLP)
	{
		SetPlayerAccountIDInCLS4Web(pPlayer);
	}
	pPlayer->SetStatus(nStatus);
	return 0;
}
int		LPGamePlay4Web_i::SetPlayerAccountIDInCLS4Web(LPGamePlay4Web_i::PlayerUnit* pPlayer)
{
	// 这个消息流程还是要加一下的,有助于调试
	char	szAccountID[128];
	sprintf(szAccountID, "[0x%"WHINT64PRFX"X]", pPlayer->nAccountID);

	LPPACKET_2MS_CMD2CLS4Web_T		Cmd2CLS4Web;
	char	buf[256];
	SVR_CLS4Web_CTRL_T&	CLS4WebCtrl	= *(SVR_CLS4Web_CTRL_T*)buf;
	CLS4WebCtrl.nCmd				= SVR_CLS4Web_CTRL;
	CLS4WebCtrl.nSubCmd				= SVR_CLS4Web_CTRL_T::SUBCMD_SET_NAME;
	CLS4WebCtrl.nParam				= pPlayer->nID;

	int		nNameLen				= strlen(szAccountID)+1;
	memcpy(wh_getptrnexttoptr(&CLS4WebCtrl), szAccountID, nNameLen);
	Cmd2CLS4Web.nClientID			= pPlayer->nID;
	Cmd2CLS4Web.nDSize				= nNameLen + sizeof(SVR_CLS4Web_CTRL_T);
	Cmd2CLS4Web.pData				= buf;
	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicMainStructure, LPPACKET_2MS_CMD2CLS4Web, Cmd2CLS4Web);

	return 0;
}
int		LPGamePlay4Web_i::TellLogicMainStructureToKickPlayer(int nClientID, pngs_cmd_t nKickSubCmd/* =SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID */)
{
	LPPACKET_2MS_KICKPLAYER_T		Kick;
	Kick.nClientID					= nClientID;
	Kick.nKickSubCmd				= nKickSubCmd;
	CmdOutToLogic_AUTO(m_pLogicMainStructure, LPPACKET_2MS_KICKPLAYER, &Kick, sizeof(Kick));
	return 0;
}
int		LPGamePlay4Web_i::TellDBPlayerLogout(tty_id_t nAccountID)
{	
	// 大区下线
	P_DBS4WEB_ACCOUNT_OFFLINE_T	GroupOffline;
	GroupOffline.nCmd			= P_DBS4WEB_REQ_CMD;
	GroupOffline.nSubCmd		= CMDID_ACCOUNT_OFFLINE_REQ;
	GroupOffline.nAccountID		= nAccountID;
	CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &GroupOffline, sizeof(GroupOffline));

	// 全局下线
	TellGlobalDBPlayerLogout(nAccountID);

	return 0;
}
int		LPGamePlay4Web_i::TellGlobalDBPlayerLogout(tty_id_t nAccountID)
{
	GS4WEB_DB_ACCOUNT_LOGOUT_REQ_T	GlobalOffline;
	GlobalOffline.nCmd			= P_GS4WEB_DB_REQ_CMD;
	GlobalOffline.nSubCmd		= DB_ACCOUNT_LOGOUT_REQ;
	GlobalOffline.nGroupIdx		= 0;
	GlobalOffline.nAccountID	= nAccountID;
	m_msgerGS4Web.SendMsg(&GlobalOffline, sizeof(GlobalOffline));
	return 0;
}
char*	LPGamePlay4Web_i::AccountLwr(char* szName)
{
	if (m_cfginfo.bForceAccountNameLwr)
	{
		wh_strlwr(szName);
	}
	return szName;
}
int		LPGamePlay4Web_i::AntiSQLInjectionAttack(int nMaxLen, char* szStr)
{
	// 返回-1代表长度不够
	int	nRst	= 0;
	nRst		= wh_strstrreplace(szStr, nMaxLen, "\'", "\\\'");
	if (nRst < 0)
	{
		return nRst;
	}
	// # , --等字符并不异常

	// 以下两个操作在sql执行时会进行替换
	//wh_strstrreplace(szStr, nMaxLen, ";", "\\;");
	//wh_strstrreplace(szStr, nMaxLen, "\\G", "\\\\G");
	return 0;
}
bool		LPGamePlay4Web_i::IsCharNameValid(const char* cszName)
{
	// 不允许有空格
	const char* cszInvalidChars	= " \t\r\n";
	for (int i=0; cszName[i]!=0; i++)
	{
		if (strchr(cszInvalidChars, (int)cszName[i]) != NULL)
		{
			return false;
		}
	}

	if (m_pBadCharNameFilter != NULL)
	{
		return !m_pBadCharNameFilter->IsBad(cszName);
	}
	return false;
}
bool		LPGamePlay4Web_i::IsAccountNameValid(const char* cszName)
{
	// 不允许有空格
	const char* cszInvalidChars	= " \t\r\n";
	for (int i=0; cszName[i]!=0; i++)
	{
		if (strchr(cszInvalidChars, (int)cszName[i]) != NULL)
		{
			return false;
		}
	}

	if (m_pBadAccountNameFilter != NULL)
	{
		return !m_pBadAccountNameFilter->IsBad(cszName);
	}
	return false;
}

void	LPGamePlay4Web_i::SendMsgToClient(tty_id_t nAccountID, const void *pData, int nSize)
{
	PlayerUnit*	pPlayer	= GetPlayerByAccountID(nAccountID);
	if (pPlayer == NULL)
		return;
	SendCmdToClient(pPlayer->nID, pData, nSize);
}
void	LPGamePlay4Web_i::AddMoneyCostLog(tty_id_t nAccountID, unsigned int nUseType, unsigned int nMoneyType, unsigned int nMoney
	, unsigned int nExtData0, unsigned int nExtData1, unsigned int nExtData2, unsigned int nExtData3)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1001,MONEY_COST)"0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d,%d"
		, nAccountID, nUseType, nMoneyType, nMoney, nExtData0, nExtData1, nExtData2, nExtData3);
}
void	LPGamePlay4Web_i::AddGoldProduceLog(tty_id_t nAccountID, unsigned int nProduceType, unsigned int nGold)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1002,GOLD_PRODUCE)"0x%"WHINT64PRFX"X,%d,%d", nAccountID, nProduceType, nGold);
}
void	LPGamePlay4Web_i::AddGoldCostLog(tty_id_t nAccountID, unsigned int nCostType, unsigned int nGold)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1003,GOLD_COST)"0x%"WHINT64PRFX"X,%d,%d", nAccountID, nCostType, nGold);
}
void	LPGamePlay4Web_i::GetPlayerCardFromDB(tty_id_t nAccountID)
{
	P_DBS4WEB_GET_PLAYERCARD_T	Cmd;
	Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd			= CMDID_GET_PLAYERCARD_REQ;
	Cmd.nAccountID		= nAccountID;

	SendCmdToDB(&Cmd, sizeof(Cmd));
}
void	LPGamePlay4Web_i::DealExcelTimeEvent()
{
	P_DBS4WEB_DEAL_EXCEL_TE_T	Cmd;
	Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd			= CMDID_DEAL_EXCEL_TE_REQ;

	SendCmdToDB(&Cmd, sizeof(Cmd));
}
void	LPGamePlay4Web_i::DealGoldMarketTimeEvent()
{
	P_DBS4WEB_DEAL_GOLD_MARKET_TE_T	Cmd;
	Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd			= CMDID_DEAL_GOLD_MARKET_TE_REQ;
	
	SendCmdToDB(&Cmd, sizeof(Cmd));
}
void	LPGamePlay4Web_i::AddNotifyInWorldChat(unsigned int nTextLen, const char* cszText, tty_id_t nCharID/* = 0 */)
{
// 	m_vectrawbuf.resize(wh_offsetof(STC_GAMECMD_CHAT_WORLD_T, szText) + nTextLen);
// 	STC_GAMECMD_CHAT_WORLD_T*	pChatTo	= (STC_GAMECMD_CHAT_WORLD_T*)m_vectrawbuf.getbuf();
// 	pChatTo->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 	pChatTo->nGameCmd	= STC_GAMECMD_CHAT_WORLD;
// 	pChatTo->nChatID	= m_pGameMngS->GetChatMng()->NewChatID();
// 	pChatTo->nAccountID	= nCharID;
// 	pChatTo->nTime		= wh_time();
// 	pChatTo->nVip		= 0;
// 	pChatTo->nTextLen	= nTextLen;
// 	pChatTo->szFrom[0]	= 0;
// 	memcpy(pChatTo->szText, cszText, pChatTo->nTextLen);
// 	g_pLPNet->SendCmdToAllClient(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
// 	m_pGameMngS->GetChatMng()->AddWorldChatEntry(m_vectrawbuf.size(), m_vectrawbuf.getbuf(), pChatTo->nTime);

	m_vectrawbuf.resize(wh_offsetof(STC_GAMECMD_WORLD_NOTIFY_T, szText) + nTextLen);
	STC_GAMECMD_WORLD_NOTIFY_T*	pChatTo	= (STC_GAMECMD_WORLD_NOTIFY_T*)m_vectrawbuf.getbuf();
	pChatTo->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
	pChatTo->nGameCmd	= STC_GAMECMD_WORLD_NOTIFY;
	pChatTo->nChatID	= m_pGameMngS->GetChatMng()->NewChatID();
	pChatTo->nAccountID	= nCharID;
	pChatTo->nTime		= wh_time();
	pChatTo->nVip		= 0;
	pChatTo->nTextLen	= nTextLen;
	pChatTo->szFrom[0]	= 0;
	memcpy(pChatTo->szText, cszText, pChatTo->nTextLen);
	g_pLPNet->SendCmdToAllClient(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	m_pGameMngS->GetChatMng()->AddWorldChatEntry(m_vectrawbuf.size(), m_vectrawbuf.getbuf(), pChatTo->nTime);
}
