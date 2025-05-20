#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

//////////////////////////////////////////////////////////////////////////
// Transaction_CreateAccount
//////////////////////////////////////////////////////////////////////////
LPGamePlay4Web_i::Transaction_CreateAccount::Transaction_CreateAccount(LPGamePlay4Web_i* pHost)
: m_pHost(pHost)
, m_nClientID(0)
, m_bPlayerDropped(false)
, m_nDeviceType(0)
, m_bUseRandomName(false)
{
	m_szAccountName[0]	= 0;
	m_szDeviceID[0]		= 0;
	m_szPass[0]			= 0;
}
LPGamePlay4Web_i::Transaction_CreateAccount::~Transaction_CreateAccount()
{

}
int		LPGamePlay4Web_i::Transaction_CreateAccount::DoNext(int nCmd, const void *pData, size_t nDSize)
{
	switch (nCmd)
	{
	case TTY_TC_INIT:
		{
			DoNext_TTY_TC_INIT(pData, nDSize);
		}
		break;
	case DB_ACCOUNT_CREATE_RPL:
		{
			DoNext_TTY_TC_GLOBAL_ACCOUNT_CREATE(pData, nDSize);
		}
		break;
	case TTY_TC_PLAYER_DROP:
		{
			DoNext_TTY_TC_PLAYER_DROP(pData, nDSize);
		}
		break;
	default:
		{
			assert(0);
		}
		break;
	}
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateAccount::DoNext_TTY_TC_INIT(const void* pData, size_t nDSize)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(,GP_Transaction_CreateAccount)"INIT,0x%X", m_nClientID);
	
	if (!m_bUseRandomName && !m_pHost->IsAccountNameValid(m_szAccountName))
	{
		TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST_T	Rst;
		Rst.nCmd		= TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST;
		Rst.nRst		= TTY_ACCOUNT_CREATE_RST_NAME_INVALID;
		m_pHost->SendCmdToClient(m_nClientID, &Rst, sizeof(Rst));
		ClearPlayer_And_Done();
		return 0;
	}
	
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer != NULL)
	{
		m_pHost->SetPlayerStatus(pPlayer, LPGamePlay4Web_i::PlayerUnit::STATUS_T::CREATEACCOUNT);
	}

	// 发送创建账号的请求给全局服
	GS4WEB_DB_ACCOUNT_CREATE_REQ_T	Cmd;
	Cmd.nCmd		= P_GS4WEB_DB_REQ_CMD;
	Cmd.nSubCmd		= DB_ACCOUNT_CREATE_REQ;
	Cmd.nExt[0]		= m_nTID;
	Cmd.nGroupIdx	= m_pHost->m_cfginfo.nSvrGrpID;
	WH_STRNCPY0(Cmd.szAccountName, m_szAccountName);
	memcpy(Cmd.szPass, m_szPass, sizeof(Cmd.szPass));
	Cmd.bUseRandomAccountName	= m_bUseRandomName;
	WH_STRNCPY0(Cmd.szDeviceID, m_szDeviceID);
	Cmd.nDeviceType	= m_nDeviceType;
	Cmd.nAppID		= m_nAppID;
	if (m_pHost->m_msgerGS4Web.SendMsg(&Cmd, sizeof(Cmd)) < 0)
	{
		SendErrRstToClient_And_Done(TTY_ACCOUNT_CREATE_RST_GS_CNTR_ERR, "");
		return 0;
	}

	// 发起一个超时事件
	// 现在只有一个超时,就暂时写事件为0
	RegTimeEvent(m_pHost->m_cfginfo.nTransactionTimeOut, 0);
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateAccount::DoNext_TTY_TC_GLOBAL_ACCOUNT_CREATE(const void* pData, size_t nDSize)
{
	GS4WEB_DB_ACCOUNT_CREATE_RPL_T*	pRst	= (GS4WEB_DB_ACCOUNT_CREATE_RPL_T*)pData;
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(,GP_Transaction_CreateAccount)"Global,0x%X,%d,%s", m_nClientID, pRst->nRst, pRst->szAccount);
	if (m_bPlayerDropped)
	{
		ClearPlayer_And_Done();
		return 0;
	}

	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer == NULL)
	{
		IAmDone();
		return 0;
	}
	switch (pRst->nRst)
	{
	case TTY_ACCOUNT_CREATE_RST_OK:
		{
			pPlayer->nCurTransactionID	= 0;
			// 1.将账号ID加入列表
			m_pHost->m_mapAccountID2ClientID.put(pRst->nAccountID, pPlayer->nID);
			pPlayer->nAccountID			= pRst->nAccountID;
			WH_STRNCPY0(pPlayer->szAccount, pRst->szAccount);
			WH_STRNCPY0(pPlayer->szDeviceID, m_szDeviceID);
			WH_STRNCPY0(pPlayer->szPass, pRst->szPass);
			pPlayer->nGiftAppID			= pRst->nGiftAppID;
			// 2.将状态变为CREATECHAR
			m_pHost->SetPlayerStatus(pPlayer, LPGamePlay4Web_i::PlayerUnit::STATUS_T::CREATECHAR);
			// 3.将结果发送给客户端
			TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST_T	Rst;
			Rst.nCmd		= TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST;
			Rst.nRst		= TTY_ACCOUNT_CREATE_RST_OK;
			WH_STRNCPY0(Rst.szAccount, pRst->szAccount);
			m_pHost->SendCmdToClient(m_nClientID, &Rst, sizeof(Rst));
			// 4.取消NOLOGIN事件
			pPlayer->teid.quit();
			// 5.设置是否绑定
			pPlayer->nBinded	= (unsigned char)!m_bUseRandomName;
			// n.结束
			IAmDone();
		}
		break;
	default:
		{
			SendErrRstToClient_And_Done(pRst->nRst);
		}
		break;
	}
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateAccount::DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GP_Transaction_CreateAccount)"Player Dropped,0x%X", m_nClientID);
	m_bPlayerDropped	= true;
	ClearPlayer_And_Done();
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateAccount::DealTimeEvent(int nEvent)
{
	// 发送结果给客户端
	SendErrRstToClient_And_Done(TTY_ACCOUNT_CREATE_RST_TIMEOUT, "");
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateAccount::SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt/* = */)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GP_Transaction_CreateAccount)"SendErrRstToClient_And_Done,%s,0x%X,0x%X", pszPrompt, m_nClientID, (int)nRst);

	// 发送结果
	TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST_T	Rst;
	Rst.nCmd		= TTY_LPGAMEPLAY_CLIENT_CREATEACCOUNT_RST;
	Rst.nRst		= nRst;
	m_pHost->SendCmdToClient(m_nClientID, &Rst, sizeof(Rst));

	LPGamePlay4Web_i::PlayerUnit*	pPlayer		= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer != NULL)
	{
		// 修改用户状态为Nothing
		m_pHost->SetPlayerStatus(pPlayer, LPGamePlay4Web_i::PlayerUnit::STATUS_T::NOTHING);
		pPlayer->nCurTransactionID	= 0;

		if (m_bPlayerDropped)		// 超时+掉线,靠这个来释放内存了
		{
			m_pHost->Player_Func_Deal_Drop_Nothing(pPlayer);
		}
		else
		{
			m_pHost->SetPlayerTE_NoLogin(pPlayer);
		}
	}

	// 删除自己
	IAmDone();
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateAccount::ClearPlayer_And_Done()
{
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer != NULL)
	{
		// 修改用户状态为Nothing
		m_pHost->SetPlayerStatus(pPlayer, LPGamePlay4Web_i::PlayerUnit::STATUS_T::NOTHING);
		pPlayer->nCurTransactionID	= 0;
		if (m_bPlayerDropped)
		{
			m_pHost->Player_Func_Deal_Drop_Nothing(pPlayer);
		}
		else
		{
			m_pHost->SetPlayerTE_NoLogin(pPlayer);
		}
	}

	// 删除自己
	IAmDone();
	return 0;
}
