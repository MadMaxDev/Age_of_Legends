#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

//初始化静态状态对象数组
LPGamePlay4Web_i::PlayerUnit::STATUS_T	LPGamePlay4Web_i::PlayerUnit::aStatus[LPGamePlay4Web_i::PlayerUnit::STATUS_T::MAXNUM];

// 给所有状态的成员重新设置指针
namespace
{
	struct	AUTOCALL_StatusObjInit
	{
		AUTOCALL_StatusObjInit()
		{
			LPGamePlay4Web_i::PlayerUnit::StatusObjInit();
		}
	};
	static AUTOCALL_StatusObjInit	l_ac_StatusObjInit;
};

LPGamePlay4Web_i::PlayerUnit::PlayerUnit()
: nID(0)
, IP(0)
, nInTime(0)
, nPassTryCount(0)
, nProcessingCmd(0)
, nDropReason(0)
, nCurTransactionID(0)
, nAccountID(0)
, nReplaceCount(0)
, nLastReplaceTime(0)
, nTermType(TERM_TYPE_NONE)
, nBinded(0)
, nGiftAppID(0)
, nAppID(0)
{
	SetStatus(STATUS_T::NOTHING);
	szAccount[0]		= 0;
	szDeviceID[0]		= 0;
	szPass[0]			= 0;
}
void	LPGamePlay4Web_i::PlayerUnit::clear()
{
	nID					= 0;
	IP					= 0;
	nInTime				= 0;
	nProcessingCmd		= 0;
	nCurTransactionID	= 0;
	nDropReason			= 0;
	SetStatus(STATUS_T::NOTHING);
	szAccount[0]		= 0;
	nAccountID			= 0;
	nLastReplaceTime	= 0;
	nReplaceCount		= 0;
	nBinded				= 0;
	szDeviceID[0]		= 0;
	szPass[0]			= 0;
	nGiftAppID			= 0;
	nAppID				= 0;
	teid.quit();
}
LPGamePlay4Web_i::PlayerUnit::STATUS_T::STATUS_T()
{
	nStatus				= NOTHING;
	pFuncDealDrop		= &LPGamePlay4Web_i::Player_Func_Deal_Drop_Nothing;
	pFuncDealLogin		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_NotSupport;
	pFuncDealCreateChar	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_NotSupport;
	pFuncDealCreateAccount	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_NotSupport;
	pFuncDealGameCmd	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_NotSupport;
	pFuncDealLogout		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_NotSupport;
	pFuncDealLoginByDeviceID= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_NotSupport;
}
void	LPGamePlay4Web_i::PlayerUnit::StatusObjInit()
{
	// 给所有status对象设置状态值
	for (int i=STATUS_T::NOTHING; i<STATUS_T::MAXNUM; i++)
	{
		aStatus[i].nStatus		= i;
	}
	//
	STATUS_T*	pStatus			= NULL;
	//
	pStatus						= &aStatus[STATUS_T::NOTHING];
	pStatus->pFuncDealLogin		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_Login;
	pStatus->pFuncDealCreateAccount	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_CreateAccount;
	pStatus->pFuncDealLoginByDeviceID		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_LoginByDeviceID;
	//
	pStatus						= &aStatus[STATUS_T::CREATEACCOUNT];
	pStatus->pFuncDealDrop		= &LPGamePlay4Web_i::Player_Func_Deal_Drop_AfterLogin;
	pStatus->pFuncDealLogin		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealCreateAccount	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealLoginByDeviceID		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	//
	pStatus						= &aStatus[STATUS_T::LOGINING];
	pStatus->pFuncDealDrop		= &LPGamePlay4Web_i::Player_Func_Deal_Drop_AfterLogin;
	pStatus->pFuncDealLogin		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;					// 玩家可能按多次Login,所以直接忽略吧
	pStatus->pFuncDealLoginByDeviceID		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;					// 玩家可能按多次Login,所以直接忽略吧
	//
	pStatus						= &aStatus[STATUS_T::CREATECHAR];
	pStatus->pFuncDealDrop		= &LPGamePlay4Web_i::Player_Func_Deal_Drop_AfterLogin;
	pStatus->pFuncDealLogin		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealCreateChar	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_CreateChar;
	pStatus->pFuncDealCreateAccount	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealGameCmd	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealLogout	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_Logout;
	pStatus->pFuncDealLoginByDeviceID		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	//
	pStatus						= &aStatus[STATUS_T::INLP];
	pStatus->pFuncDealDrop		= &LPGamePlay4Web_i::Player_Func_Deal_Drop_AfterLogin;
	pStatus->pFuncDealLogin		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealCreateChar	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealCreateAccount	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealGameCmd	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_GameCmd;
	pStatus->pFuncDealLogout	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_Logout;
	pStatus->pFuncDealLoginByDeviceID		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	//
	pStatus						= &aStatus[STATUS_T::DROPWAIT];
	pStatus->pFuncDealDrop		= &LPGamePlay4Web_i::Player_Func_Deal_Drop_Ignore;
	//
	pStatus						= &aStatus[STATUS_T::CLOSEWAIT];
	pStatus->pFuncDealDrop		= &LPGamePlay4Web_i::Player_Func_Deal_Drop_Ignore;
	pStatus->pFuncDealLogin		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealCreateChar	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealGameCmd	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealLogout	= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
	pStatus->pFuncDealLoginByDeviceID		= &LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore;
}
void	LPGamePlay4Web_i::Player_Func_Deal_Drop_Ignore(PlayerUnit* pPlayer)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,0x%X,%d,%s", __FUNCTION__, pPlayer->nID, pPlayer->GetStatus(), pPlayer->szAccount);
}
void	LPGamePlay4Web_i::Player_Func_Deal_Drop_Nothing(PlayerUnit* pPlayer)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,0x%X,%d,%s", __FUNCTION__, pPlayer->nID, pPlayer->GetStatus(), pPlayer->szAccount);
	// 清空player的状态
	SetPlayerStatus(pPlayer, PlayerUnit::STATUS_T::NOTHING);
	// 下面这行注释掉,是因为掉线肯定是MS通知上来的,通知上来的时候MS、CLS4Web、CAAFS4Web已经删除过了
	//TellLogicMainStructureToKickPlayer(pPlayer->nID);
	pPlayer->clear();
	m_Players.FreeUnitByPtr(pPlayer);
}
void	LPGamePlay4Web_i::Player_Func_Deal_Drop_AfterLogin(PlayerUnit* pPlayer)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,0x%X,%d,%s", __FUNCTION__, pPlayer->nID, pPlayer->GetStatus(), pPlayer->szAccount);
	whtransactionbase*	pT	= NULL;
	if (pPlayer->nCurTransactionID>0
		&& (pT=m_pTMan->GetTransaction(pPlayer->nCurTransactionID))!=NULL)
	{
		// 说明应该是在LOGIN过程中,或者在其他某个角色处理过程中
		pT->DoNext(TTY_TC_PLAYER_DROP, NULL, 0);
	}
	else
	{
		ClearPlayerAfterLogin(pPlayer);
	}
}
void	LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_NotSupport(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,HACK,0x%X,%d,%s,%d", __FUNCTION__, pPlayer->nID, pPlayer->GetStatus(), pPlayer->szAccount, (int)nTTYCmd);
	// 应该是hack
	TellLogicMainStructureToKickPlayer(pPlayer->nID);
}
void	LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Ignore(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize)
{

}
void	LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_Login(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize)
{
	// 获得用户名和密码
	TTY_CLIENT_LPGAMEPLAY_LOGIN_T*	pLogin		= (TTY_CLIENT_LPGAMEPLAY_LOGIN_T*)pData;
	// 验证消息的合法性
	{
		if (nSize<wh_offsetof(TTY_CLIENT_LPGAMEPLAY_LOGIN_T, szPass))
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,HACK,cmd size too small,%d,0x%X,%d,%s", __FUNCTION__, nSize, pPlayer->nID, pPlayer->GetStatus(), pPlayer->szAccount);
			TellLogicMainStructureToKickPlayer(pPlayer->nID);
			return;
		}
		
		pLogin->szAccount[TTY_ACCOUNTNAME_LEN-1]	= 0;

		if (pLogin->nPassLen<=0 || pLogin->nPassLen>TTY_ACCOUNTPASSWORD_LEN)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,HACK,invalid password len,%d,0x%X,%d,%s", __FUNCTION__, pLogin->nPassLen, pPlayer->nID, pPlayer->GetStatus(), pLogin->szAccount);
			TellLogicMainStructureToKickPlayer(pPlayer->nID);
			return;
		}
		else
		{
			int	nPassLen	= nSize - wh_offsetof(TTY_CLIENT_LPGAMEPLAY_LOGIN_T, szPass);
			if (nPassLen != pLogin->nPassLen)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,HACK,invalid password len,%d!=%d,0x%X,%d,%s", __FUNCTION__, pLogin->nPassLen, nPassLen, pPlayer->nID, pPlayer->GetStatus(), pLogin->szAccount);
				TellLogicMainStructureToKickPlayer(pPlayer->nID);
				return;
			}
		}
		// 指令是合法的
		WH_CLEAR_STRNCPY0(pPlayer->szAccount, pLogin->szAccount);
	}

	//看看有没有这种情况发生
	if (pPlayer->nCurTransactionID > 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1397,LPGP_RT)"%s,transaction id not 0,%d/%d,0x%X,%s", __FUNCTION__, pPlayer->nCurTransactionID, m_pTMan->GetNum(), pPlayer->nID, pPlayer->szAccount);
	}
	// 生成LOGIN的transaction
	Transaction_Login*	pT					= new Transaction_Login(this);
	assert(pT != NULL);
	try
	{
		// 添加到TransactionMan中
		pPlayer->SetTransactionID(m_pTMan->AddTransaction(pT));
		if (pPlayer->nCurTransactionID <= 0)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,m_pTMan->AddTransaction,%d/%d,0x%X,%s", __FUNCTION__, pPlayer->nCurTransactionID, m_pTMan->GetNum(), pPlayer->nID, pPlayer->szAccount);
			throw PLAYERCMD_ERR_UNKNOWN;
		}
		// pT数据赋值
		pT->m_nClientID		= pPlayer->nID;
		pT->m_pPlayer		= pPlayer;
		pT->m_nPassLen		= pLogin->nPassLen + 1;
		WH_CLEAR_STRNCPY0(pT->m_szAccount, pPlayer->szAccount);
		WH_STRNCPY0(pT->m_szDeviceID, pLogin->szDeviceID);
		memcpy(pT->m_szPassword, pLogin->szPass, pLogin->nPassLen);
		pT->m_szPassword[pT->m_nPassLen-1]	= 0;
		pT->m_nIP			= (unsigned int)pPlayer->IP;
		pT->m_nDeviceType	= pLogin->nDeviceType;

		pPlayer->nAppID		= pLogin->nAppID;
	}
	catch (int nErrCode)
	{
		pT->IAmDone();
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}

	pT->DoNext(TTY_TC_INIT, NULL, 0);
}
void	LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_LoginByDeviceID(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize)
{
	// 获得用户名和密码
	TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID_T*	pLogin		= (TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID_T*)pData;
		
	// 指令是合法的
	WH_CLEAR_STRNCPY0(pPlayer->szDeviceID, pLogin->szDeviceID);

	//看看有没有这种情况发生
	if (pPlayer->nCurTransactionID > 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1397,LPGP_RT)"%s,transaction id not 0,%d/%d,0x%X,%s", __FUNCTION__, pPlayer->nCurTransactionID, m_pTMan->GetNum(), pPlayer->nID, pPlayer->szAccount);
	}
	// 生成LOGIN的transaction
	Transaction_LoginByDeviceID*	pT					= new Transaction_LoginByDeviceID(this);
	assert(pT != NULL);
	try
	{
		// 添加到TransactionMan中
		pPlayer->SetTransactionID(m_pTMan->AddTransaction(pT));
		if (pPlayer->nCurTransactionID <= 0)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,m_pTMan->AddTransaction,%d/%d,0x%X,%s", __FUNCTION__, pPlayer->nCurTransactionID, m_pTMan->GetNum(), pPlayer->nID, pPlayer->szDeviceID);
			throw PLAYERCMD_ERR_UNKNOWN;
		}
		// pT数据赋值
		pT->m_nClientID		= pPlayer->nID;
		pT->m_pPlayer		= pPlayer;
		WH_CLEAR_STRNCPY0(pT->m_szDeviceID, pPlayer->szDeviceID);
		pT->m_nIP			= (unsigned int)pPlayer->IP;
		pT->m_nDeviceType	= pLogin->nDeviceType;

		pPlayer->nAppID		= pLogin->nAppID;
	}
	catch (int nErrCode)
	{
		pT->IAmDone();
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}

	pT->DoNext(TTY_TC_INIT, NULL, 0);
}
void	LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_CreateChar(LPGamePlay4Web_i::PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize)
{
	if (pPlayer->nProcessingCmd == PlayerUnit::PROCESSINGCMD_CREATECHAR)
	{
		return;
	}
	//看看有没有这种情况发生
	if (pPlayer->nCurTransactionID > 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1397,LPGP_RT)"%s,transaction id not 0,%d/%d,0x%X,%s,0x%"WHINT64PRFX"X", __FUNCTION__, pPlayer->nCurTransactionID, m_pTMan->GetNum(), pPlayer->nID, pPlayer->szAccount, pPlayer->nAccountID);
	}
	// 生成CREATECHAR的transaction
	Transaction_CreateChar*	pT	= new Transaction_CreateChar(this);
	pPlayer->nProcessingCmd		= PlayerUnit::PROCESSINGCMD_CREATECHAR;
	assert(pT != NULL);
	try
	{
		// 添加到TransactionMan中
		pPlayer->SetTransactionID(m_pTMan->AddTransaction(pT));
		if (pPlayer->nCurTransactionID <= 0)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,m_pTMan->AddTransaction,%d/%d,0x%X,%s,0x%"WHINT64PRFX"X", __FUNCTION__, pPlayer->nCurTransactionID, m_pTMan->GetNum(), pPlayer->nID, pPlayer->szAccount, pPlayer->nAccountID);
			throw PLAYERCMD_ERR_UNKNOWN;
		}
		// pT数据赋值
		pT->m_nClientID		= pPlayer->nID;
		pT->m_nAccountID	= pPlayer->nAccountID;
		pT->m_nBinded		= pPlayer->nBinded;
		pT->m_nGiftAppID	= pPlayer->nGiftAppID;
	}
	catch (int nErrCode)
	{
		pT->IAmDone();
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}

	pT->DoNext(TTY_TC_INIT, pData, nSize);
}
void	LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_CreateAccount(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize)
{
	//看看有没有这种情况发生
	if (pPlayer->nCurTransactionID > 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1397,LPGP_RT)"%s,transaction id not 0,%d/%d,0x%X,%s", __FUNCTION__, pPlayer->nCurTransactionID, m_pTMan->GetNum(), pPlayer->nID, pPlayer->szAccount);
	}
	Transaction_CreateAccount*	pT		= new Transaction_CreateAccount(this);
	assert(pT != NULL);
	try
	{
		// 添加到TransactionMan中
		pPlayer->SetTransactionID(m_pTMan->AddTransaction(pT));
		if (pPlayer->nCurTransactionID <= 0)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"%s,m_pTMan->AddTransaction,%d/%d,0x%X", __FUNCTION__, pPlayer->nCurTransactionID, m_pTMan->GetNum(), pPlayer->nID);
			throw PLAYERCMD_ERR_UNKNOWN;
		}
		TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT_T*	pCreate		= (TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT_T*)pData;
		// pT数据赋值
		pT->m_nClientID		= pPlayer->nID;
		WH_STRNCPY0(pT->m_szAccountName, pCreate->szAccountName);
		memcpy(pT->m_szPass, pCreate->szPass, sizeof(pT->m_szPass));
		WH_STRNCPY0(pT->m_szDeviceID, pCreate->szDeviceID);
		pT->m_nDeviceType	= pCreate->nDeviceType;
		pT->m_bUseRandomName= pCreate->bUseRandomName;
		pT->m_nAppID		= pCreate->nAppID;

		pPlayer->nAppID		= pCreate->nAppID;
	}
	catch (int nErrCode)
	{
		pT->IAmDone();
		TellLogicMainStructureToKickPlayer(pPlayer->nID);
		return;
	}

	pT->DoNext(TTY_TC_INIT, NULL, 0);
}
void	LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_GameCmd(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize)
{
	m_pGameMngS->Receive(LPPACKET_2GP_PLAYERCMD, pData, nSize);
}
void	LPGamePlay4Web_i::Player_Func_Deal_PlayerCmd_Client_Logout(PlayerUnit* pPlayer, tty_cmd_t nTTYCmd, void* pData, size_t nSize)
{
	// 这个应该做成一个transaction,处理以下情况(2、3步应该和玩家掉线等处理一致)
	// 1.告诉客户端可以断连(进入closewait状态)(暂时让玩家正常下线,这样简单点)
	// 2.(1)收到客户端返回,退出(2)超时退出
	// 3.数据库清理
	// 告诉客户端可以正常退出了
	TTY_LPGAMEPLAY_CLIENT_LOGOUT_RST_T	rstCmd;
	rstCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_LOGOUT_RST;
	rstCmd.nRst			= TTY_LPGAMEPLAY_CLIENT_LOGOUT_RST_T::RST_OK;
	SendCmdToClient(pPlayer->nID, &rstCmd, sizeof(rstCmd));
	// 准备踢出用户
	TellLogicMainStructureToKickPlayer(pPlayer->nID, SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID_AS_END);
}
int		LPGamePlay4Web_i::ClearPlayerAfterLogin(LPGamePlay4Web_i::PlayerUnit* pPlayer)
{
	// 让玩家从数据库下线
	TellDBPlayerLogout(pPlayer->nAccountID);
	// 删除account_id映射
	int		nClientID				= 0;
	if (m_mapAccountID2ClientID.get(pPlayer->nAccountID, nClientID))
	{
		if (nClientID == pPlayer->nID)
		{
			m_mapAccountID2ClientID.erase(pPlayer->nAccountID);
		}
	}
	// 告诉GameMngS也要删除上层玩家了
	LPPACKET_2GP_PLAYEROFFLINE_T	cmd;
	cmd.nClientID					= pPlayer->nID;
	cmd.nReason						= pPlayer->nDropReason;
	m_pGameMngS->Receive(LPPACKET_2GP_PLAYEROFFLINE, &cmd, sizeof(cmd));
	// 后面的
	Player_Func_Deal_Drop_Nothing(pPlayer);
	return 0;
}
int		LPGamePlay4Web_i::TryReplacePlayer(int nOldClientID, const char* cszCharPassMD5, unsigned int nFromIP, bool& bHasPass)
{
	LPGamePlay4Web_i::PlayerUnit*	pOldPlayer	= GetPlayerByID(nOldClientID);
	if (pOldPlayer == NULL)
	{
		return 0;
	}

	switch (pOldPlayer->GetStatus())
	{
	case PlayerUnit::STATUS_T::DROPWAIT:
	case PlayerUnit::STATUS_T::CLOSEWAIT:
		{
			return 1;
		}
		break;
	}

	// 通知在线的这个用户有人尝试用这个账号登录
	SendReplaceWarningToPlayer(nOldClientID, nFromIP);

	if (m_cfginfo.nReplaceCount > 0)
	{
		time_t	nNow	= m_timeNow;
		if ((nNow-pOldPlayer->nLastReplaceTime) >= m_cfginfo.nReplaceTimeOut)
		{
			// 时间太久,重新设置nReplaceCount
			pOldPlayer->nReplaceCount	= m_cfginfo.nReplaceCount;
		}
		pOldPlayer->nLastReplaceTime	= nNow;
		if ((--pOldPlayer->nReplaceCount) <= 0)
		{
			// 踢掉用户
			TellLogicMainStructureToKickPlayer(pOldPlayer->nID, SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID_AS_DROP);
			return 0;
		}
	}
	else
	{
		return 2;
	}
	return 0;
}

