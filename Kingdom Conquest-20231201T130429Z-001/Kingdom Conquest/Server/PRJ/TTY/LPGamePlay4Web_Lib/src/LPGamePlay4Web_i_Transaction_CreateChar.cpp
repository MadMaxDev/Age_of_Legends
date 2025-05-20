#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

//////////////////////////////////////////////////////////////////////////
// Transaction_CreateChar
//////////////////////////////////////////////////////////////////////////
LPGamePlay4Web_i::Transaction_CreateChar::Transaction_CreateChar(LPGamePlay4Web_i* pHost)
: m_pHost(pHost)
, m_nClientID(0)
, m_nAccountID(0)
, m_nPosX(-1)
, m_nPosY(-1)
, m_bPlayerDropped(false)
, m_bCreateOK(false)
{

}
LPGamePlay4Web_i::Transaction_CreateChar::~Transaction_CreateChar()
{

}
int		LPGamePlay4Web_i::Transaction_CreateChar::DoNext(int nCmd, const void *pData, size_t nDSize)
{
	switch (nCmd)
	{
	case TTY_TC_INIT:
		{
			DoNext_TTY_TC_INIT(pData, nDSize);
		}
		break;
	case PNGSPACKET_2DB4WEBUSER_RPL:
		{
			P_DBS4WEB_CMD_T*	pBaseCmd	= (P_DBS4WEB_CMD_T*)pData;
			switch (pBaseCmd->nSubCmd)
			{
			case CMDID_ACCOUNT_ONLINE_RPL:
				{
					DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(pData, nDSize);
				}
				break;
			case CMDID_LOAD_CHAR_RPL:
				{
					DoNext_TTY_TC_CHAR_LOAD_RPL(pData, nDSize);
				}
				break;
			case CMDID_CHAR_CREATE_RPL:
				{
					DoNext_TTY_TC_GROUP_CHAR_CREATE(pData, nDSize);
				}
				break;
			default:
				{
					assert(0);
				}
				break;
			}
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
int		LPGamePlay4Web_i::Transaction_CreateChar::DoNext_TTY_TC_INIT(const void* pData, size_t nDSize)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(,GP_Transaction_CreateChar)"INIT,0x%X,0x%"WHINT64PRFX"X", m_nClientID, m_nAccountID);

	TTY_CLIENT_LPGAMEPLAY_CREATECHAR_T*	pCreateChar	= (TTY_CLIENT_LPGAMEPLAY_CREATECHAR_T*)pData;

	if (!m_pHost->IsCharNameValid(pCreateChar->szName))
	{
		// 发送创建结果
		TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST_T	Rst;
		Rst.nCmd			= TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST;
		Rst.nRst			= TTY_CHAR_CREATE_RST_NAME_INVALID;
		m_pHost->SendCmdToClient(m_nClientID, &Rst, sizeof(Rst));

		ClearPlayer_And_Done();
		return 0;
	}
	
	// 后续开发需要做以下工作后才能进一步进行数据库请求
	// 1.性别是否合法 2.头像ID是否合法 3.国家是否合法 4.分配位置

	int	nRst	= m_pHost->m_pGameMngS->GetWorldMng()->GetAvailablePos(pCreateChar->nCountry, m_nPosX, m_nPosY, true);
	if (nRst < 0)
	{
		return SendErrRstToClient_And_Done(TTY_CHAR_CREATE_RST_WORLD_FULL, "WORLD FULL");
	}

	// 向数据库发送请求,生成角色信息
	P_DBS4WEB_CHAR_CREATE_T		Cmd;
	Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd					= CMDID_CHAR_CREATE_REQ;
	Cmd.nAccountID				= m_nAccountID;
	WH_STRNCPY0(Cmd.szCharName, pCreateChar->szName);
	Cmd.nSex					= pCreateChar->nSex;
	Cmd.nHeadID					= pCreateChar->nHeadID;
	Cmd.nPosX					= m_nPosX;
	Cmd.nPosY					= m_nPosY;
	Cmd.nBinded					= m_nBinded;
	Cmd.nGiftAppID				= m_nGiftAppID;
	Cmd.nExt[0]					= m_nTID;
	m_pHost->CmdOutToLogic_AUTO(m_pHost->m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
	
	// 注册时间事件
	RegTimeEvent(m_pHost->m_cfginfo.nTransactionTimeOut, 0);

	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateChar::DoNext_TTY_TC_GROUP_CHAR_CREATE(const void* pData, size_t nDSize)
{
	P_DBS4WEBUSER_CHAR_CREATE_T*	pRst	= (P_DBS4WEBUSER_CHAR_CREATE_T*)pData;
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(,GP_Transaction_CreateChar)"CreateChar,0x%X,0x%"WHINT64PRFX"X,%d", m_nClientID, m_nAccountID, pRst->nRst);
	if (m_bPlayerDropped)
	{
		if (pRst->nRst == TTY_CHAR_CREATE_RST_OK)
		{
			m_pHost->GetPlayerCardFromDB(m_nAccountID);
		}
		ClearPlayer_And_Done();
		return 0;
	}
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer == NULL)
	{
		ClearPlayer_And_Done();
		return 0;
	}
	switch (pRst->nRst)
	{
	case TTY_CHAR_CREATE_RST_OK:
		{
			m_bCreateOK				= true;

			// 1.通知大区上线
			P_DBS4WEB_ACCOUNT_ONLINE_T	Cmd;
			Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd				= CMDID_ACCOUNT_ONLINE_REQ;
			Cmd.nExt[0]				= m_nTID;							
			Cmd.nAccountID			= m_nAccountID;
			Cmd.nBinded				= pPlayer->nBinded;
			Cmd.nIP					= pPlayer->IP;
			m_pHost->CmdOutToLogic_AUTO(m_pHost->m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
		
			// n.注册时间事件
			RegTimeEvent(m_pHost->m_cfginfo.nTransactionTimeOut, 0);
		}
		break;
	default:
		{
			if (pRst->nRst == TTY_CHAR_CREATE_RST_POSITION_DUP)
			{
				// 这个错误极其严重!!!
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,CREATE_CHAR)"POSITION_DUP,%d,%d", m_nPosX, m_nPosY);
			}
			// 超时也应该做做这个,掉线也需要..
			m_pHost->m_pGameMngS->GetWorldMng()->SetTileType(m_nPosX, m_nPosY, TileUnit::TILETYPE_NONE);
			SendErrRstToClient_And_Done(pRst->nRst);
		}
		break;
	}
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateChar::DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize)
{
	P_DBS4WEBUSER_ACCOUNT_ONLINE_T*	pRst	= (P_DBS4WEBUSER_ACCOUNT_ONLINE_T*)pData;
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(,GP_Transaction_CreateChar)"GroupOnline,0x%X,0x%"WHINT64PRFX"X,%d", m_nClientID, m_nAccountID, pRst->nRst);
	if (m_bPlayerDropped)
	{
		ClearPlayer_And_Done();
		return 0;
	}

	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer == NULL)
	{
		ClearPlayer_And_Done();
		return 0;
	}
	switch (pRst->nRst)
	{
	case TTY_LOGIN_RST_SUCCEED:
		{
			// 获取角色详细信息
			P_DBS4WEB_LOAD_CHAR_T	Cmd;
			Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd				= CMDID_LOAD_CHAR_REQ;
			Cmd.nExt[0]				= m_nTID;
			Cmd.nExt[1]				= m_nClientID;
			Cmd.nExt[2]				= pPlayer->nTermType;
			Cmd.nAccountID			= m_nAccountID;
			m_pHost->CmdOutToLogic_AUTO(m_pHost->m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			RegTimeEvent(m_pHost->m_cfginfo.nTransactionTimeOut, 0);
		}
		break;
	default:
		{
			SendErrRstToClient_And_Done(pRst->nRst, "");
		}
		break;
	}
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateChar::DoNext_TTY_TC_CHAR_LOAD_RPL(const void* pData, size_t nDSize)
{
	P_DBS4WEBUSER_LOAD_CHAR_T*	pRst	= (P_DBS4WEBUSER_LOAD_CHAR_T*)pData;
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(,GP_Transaction_CreateChar)"LoadChar,0x%X,0x%"WHINT64PRFX"X,%d", m_nClientID, m_nAccountID, pRst->nRst);
	if (m_bPlayerDropped)
	{
		ClearPlayer_And_Done();
		return 0;
	}
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer == NULL)
	{
		ClearPlayer_And_Done();
		return 0;
	}
	
	switch (pRst->nRst)
	{
	case P_DBS4WEBUSER_LOAD_CHAR_T::RST_OK:
		{
			// 修改状态
			m_pHost->SetPlayerStatus(pPlayer, LPGamePlay4Web_i::PlayerUnit::STATUS_T::INLP);
			m_pHost->m_pGameMngS->Receive(PNGSPACKET_2DB4WEBUSER_RPL, pData, nDSize);
			pPlayer->nCurTransactionID	= 0;
			pPlayer->nProcessingCmd		= 0;

			// 登录结果
			TTY_LPGAMEPLAY_CLIENT_LOGIN_RST_T	rst;
			rst.nCmd				= TTY_LPGAMEPLAY_CLIENT_LOGIN_RST;
			rst.nRst				= TTY_LOGIN_RST_SUCCEED;
			WH_STRNCPY0(rst.szDeviceID, pPlayer->szDeviceID);
			WH_STRNCPY0(rst.szAccount, pPlayer->szAccount);
			WH_STRNCPY0(rst.szPass, pPlayer->szPass);
			m_pHost->SendCmdToClient(pPlayer->nID, &rst, sizeof(rst));

			// 结束
			IAmDone();
		}
		break;
	default:
		{
			SendErrRstToClient_And_Done(TTY_LOGIN_RST_LOADCHAR, "");
		}
		break;
	}

	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateChar::DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GP_Transaction_CreateChar)"Player Dropped,0x%X,0x%"WHINT64PRFX"X", m_nClientID, m_nAccountID);
	m_bPlayerDropped	= true;
	ClearPlayer_And_Done();
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateChar::DealTimeEvent(int nEvent)
{
	// 发送结果给客户端
	SendErrRstToClient_And_Done(TTY_CHAR_CREATE_RST_TIMEOUT, "");
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateChar::SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt/* = */)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GP_Transaction_CreateChar)"SendErrRstToClient_And_Done,%s,0x%X,0x%X,0x%"WHINT64PRFX"X", pszPrompt, m_nClientID, (int)nRst, m_nAccountID);

	if (m_bCreateOK)
	{
		// 发送登录结果
		TTY_LPGAMEPLAY_CLIENT_LOGIN_RST_T	rst;
		rst.nCmd				= TTY_LPGAMEPLAY_CLIENT_LOGIN_RST;
		rst.nRst				= nRst;
		m_pHost->SendCmdToClient(m_nClientID, &rst, sizeof(rst));
	}
	else
	{
		// 发送创建结果
		TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST_T	Rst;
		Rst.nCmd			= TTY_LPGAMEPLAY_CLIENT_CREATECHAR_RST;
		Rst.nRst			= nRst;
		m_pHost->SendCmdToClient(m_nClientID, &Rst, sizeof(Rst));
	}

	ClearPlayer_And_Done();
	return 0;
}
int		LPGamePlay4Web_i::Transaction_CreateChar::ClearPlayer_And_Done()
{
	if (m_bPlayerDropped)
	{
		if (m_bCreateOK)
		{
			m_pHost->TellDBPlayerLogout(m_nAccountID);
		}
		else
		{
			m_pHost->TellGlobalDBPlayerLogout(m_nAccountID);
		}
	}

	if (m_bPlayerDropped || m_bCreateOK)
	{
		// (1)m_bCreateOK玩家会转为NOTHING
		// (2)m_bPlayerDropped不影响下一次登录
		m_pHost->m_mapAccountID2ClientID.erase(m_nAccountID);
	}

	LPGamePlay4Web_i::PlayerUnit*	pPlayer		= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer != NULL)
	{
		pPlayer->nCurTransactionID	= 0;
		pPlayer->nProcessingCmd		= 0;
		if (m_bCreateOK)
		{
			m_pHost->SetPlayerStatus(pPlayer, LPGamePlay4Web_i::PlayerUnit::STATUS_T::NOTHING);
		}
		if (m_bPlayerDropped)
		{
			m_pHost->Player_Func_Deal_Drop_Nothing(pPlayer);
		}
	}
	
	// 删除自己
	IAmDone();
	return 0;
}
