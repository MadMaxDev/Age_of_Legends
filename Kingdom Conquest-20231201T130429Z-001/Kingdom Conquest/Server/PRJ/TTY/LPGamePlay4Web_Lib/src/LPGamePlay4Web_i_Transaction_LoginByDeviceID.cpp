//////////////////////////////////////////////////////////////////////////
// (1)玩家在线,其他模块没返回,Transaction超时,时间事件会清空玩家
// (2)玩家掉线,其他模块没返回,Transaction超时,时间事件会清空玩家
// (3)玩家掉线,其他模块返回,ClearTryLoginPlayer_And_Done会清空玩家
// (4)m_bPlayerDropped是为了transaction执行过程中做一些回退而使用的
//////////////////////////////////////////////////////////////////////////
#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

//////////////////////////////////////////////////////////////////////////
// Transaction_LoginByDeviceID
//////////////////////////////////////////////////////////////////////////
LPGamePlay4Web_i::Transaction_LoginByDeviceID::Transaction_LoginByDeviceID(LPGamePlay4Web_i* pHost)
: m_pHost(pHost)
, m_pPlayer(NULL)
, m_nClientID(0)
, m_nDeviceType(0)
, m_bPlayerDropped(false)
, m_nCurTE(TE_NONE)
{
	m_szDeviceID[0]		= 0;
	m_szRealDeviceID[0]	= 0;
}
LPGamePlay4Web_i::Transaction_LoginByDeviceID::~Transaction_LoginByDeviceID()
{
}
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::DoNext(int nCmd, const void *pData, size_t nDSize)
{
	switch (nCmd)
	{
	case TTY_TC_INIT:
		{
			DoNext_TTY_TC_INIT(pData, nDSize);
		}
		break;
	case DB_ACCOUNT_LOGIN_RPL:
		{
			DoNext_TTY_TC_GLOBAL_ACCOUNT_LOGIN_RPL(pData, nDSize);
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
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::DoNext_TTY_TC_INIT(const void* pData, size_t nDSize)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(,GP_Transaction_LoginByDeviceID)"INIT,%s,0x%X", m_szDeviceID, m_nClientID);
	if (m_pHost->m_mapTryLoginAccountName2ClientID.has(m_szDeviceID))
	{
		SendErrRstToClient_And_Done(TTY_LOGIN_RST_LOGININPROCESS_DEVICEID, "");
		return 0;
	}
	// 添加尝试登录表中
	m_pHost->m_mapTryLoginAccountName2ClientID.put(m_szDeviceID, m_nClientID);
	// 设置状态为正常LOGIN中
	m_pPlayer->SetStatus(LPGamePlay4Web_i::PlayerUnit::STATUS_T::LOGINING);

	// 向GS4Web提出正式LOGIN请求
	m_pHost->m_vectrawbuf.resize(sizeof(GS4WEB_DB_ACCOUNT_LOGIN_DEVICEID_REQ_T));
	GS4WEB_DB_ACCOUNT_LOGIN_DEVICEID_REQ_T*	pCmd	= (GS4WEB_DB_ACCOUNT_LOGIN_DEVICEID_REQ_T*)m_pHost->m_vectrawbuf.getbuf();
	memset(pCmd->nExt, 0, sizeof(pCmd->nExt));
	pCmd->nCmd			= P_GS4WEB_DB_REQ_CMD;
	pCmd->nSubCmd		= DB_ACCOUNT_LOGIN_DEVICEID_REQ;
	pCmd->nGroupIdx		= m_pHost->m_cfginfo.nSvrGrpID;
	WH_STRNCPY0(pCmd->szDeviceID, m_szDeviceID);
	pCmd->nDeviceType	= m_nDeviceType;
	pCmd->nExt[0]		= m_nTID;
	if (m_pHost->m_msgerGS4Web.SendMsg(m_pHost->m_vectrawbuf.getbuf(), m_pHost->m_vectrawbuf.size()) < 0)
	{
		m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
		SendErrRstToClient_And_Done(TTY_LOGIN_RST_UNKNOWNERR, "m_msgerGS4Web.SendMsg error");
		return 0;
	}

	// 发起一个超时事件
	RegTimeEvent(m_pHost->m_cfginfo.nTransactionTimeOut, TE_GLOBAL_LOGIN);
	m_nCurTE		= TE_GLOBAL_LOGIN;

	return 0;
}
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::DoNext_TTY_TC_GLOBAL_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize)
{
	GS4WEB_DB_ACCOUNT_LOGIN_RPL_T*	pRst	= (GS4WEB_DB_ACCOUNT_LOGIN_RPL_T*)pData;
	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(,GP_Transaction_LoginByDeviceID)"GLOBAL_RPL,0x%"WHINT64PRFX"X,%d", pRst->nAccountID, pRst->nRst);
	if (pRst->nRst == TTY_LOGIN_RST_SUCCEED)
	{
		m_nAccountID		= pRst->nAccountID;
		WH_STRNCPY0(m_szRealDeviceID, pRst->szDeviceID);
	}
	if (m_bPlayerDropped)
	{
		if (pRst->nRst == TTY_LOGIN_RST_SUCCEED)
		{
			// 告诉全局账号下线
			m_pHost->TellGlobalDBPlayerLogout(m_nAccountID);
		}
		ClearTryLoginPlayer_And_Done();
		return 0;
	}

	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer == NULL)
	{
		// 把用户从TryLogin表中移出
		m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
		if (pRst->nRst == TTY_LOGIN_RST_SUCCEED)
		{
			// 告诉全局账号下线
			m_pHost->TellGlobalDBPlayerLogout(m_nAccountID);
		}
		IAmDone();
		return 0;
	}
	switch (pRst->nRst)
	{
	case TTY_LOGIN_RST_SUCCEED:
		{
			// 看看这个账号是否已经在本大区了
			int		nOldClientID	= 0;
			if (m_pHost->m_mapAccountID2ClientID.get(m_nAccountID, nOldClientID))
			{
				bool	bHasPass	= false;
				switch (m_pHost->TryReplacePlayer(nOldClientID, NULL, pPlayer->IP, bHasPass))
				{
				case 0:		// 成功
					{
						// TryReplacePlayer内部已经通知原用户下线了
						// 先删除老的才能加入成功
						m_pHost->m_mapAccountID2ClientID.erase(m_nAccountID);
					}
					break;
				default:
					{
						// 把用户从TryLogin表中移出
						m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
						SendErrRstToClient_And_Done(TTY_LOGIN_RST_ALREADYINTHIS, "");
						return 0;
					}
					break;
				}
			}
			// 保存账号ID
			pPlayer->nAccountID		= m_nAccountID;
			pPlayer->nBinded		= pRst->nBinded;
			WH_STRNCPY0(pPlayer->szAccount, pRst->szAccountName);
			WH_STRNCPY0(pPlayer->szDeviceID, pRst->szDeviceID);
			WH_STRNCPY0(pPlayer->szPass, pRst->szPass);
			pPlayer->nGiftAppID		= pRst->nGiftAppID;
			// 把用户从TryLogin表中移出放入Login表
			m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
			m_pHost->m_mapAccountID2ClientID.put(m_nAccountID, pPlayer->nID);

			// 清除NOLOGIN
			pPlayer->teid.quit();

			// 大区上线
			P_DBS4WEB_ACCOUNT_ONLINE_T	Cmd;
			Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd				= CMDID_ACCOUNT_ONLINE_REQ;
			Cmd.nExt[0]				= m_nTID;
			Cmd.nAccountID			= m_nAccountID;
			Cmd.nIP					= m_nIP;
			Cmd.nBinded				= pRst->nBinded;
			m_pHost->CmdOutToLogic_AUTO(m_pHost->m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			RegTimeEvent(m_pHost->m_cfginfo.nTransactionTimeOut, TE_GROUP_LOGIN);
			m_nCurTE		= TE_GROUP_LOGIN;
		}
		break;
	case TTY_LOGIN_RST_NO_DEVICEID:
		{
			if (m_pHost->m_cfginfo.bCreateAccountIfNotExist)
			{
				// 把用户从TryLogin表中移出
				m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
				pPlayer->nCurTransactionID	= 0;

				// 生成新的创建账号transaction
				{
					Transaction_CreateAccount*	pT		= new Transaction_CreateAccount(m_pHost);
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

						// pT数据赋值
						pT->m_nClientID		= pPlayer->nID;
						WH_STRNCPY0(pT->m_szDeviceID, m_szDeviceID);
						pT->m_szAccountName[0]	= 0;
						pT->m_szPass[0]		= 0;
						pT->m_nDeviceType	= m_nDeviceType;
						pT->m_bUseRandomName= m_pHost->m_cfginfo.bUseRandomAccountName;
						pT->m_nAppID		= 0;

						pPlayer->nGiftAppID	= 0;
					}
					catch (int nErrCode)
					{
						pT->IAmDone();
						m_pHost->TellLogicMainStructureToKickPlayer(pPlayer->nID);

						// 自己结束!!!
						IAmDone();
						return 0;
					}

					pT->DoNext(TTY_TC_INIT, NULL, 0);
				}

				// 自己结束
				IAmDone();
				return 0;
			}
			else // 和default的处理需要完全一致
			{
				// 把用户从TryLogin表中移出
				m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
				SendErrRstToClient_And_Done(pRst->nRst, "");
			}
		}
		break;
	default:
		{
			// 把用户从TryLogin表中移出
			m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
			SendErrRstToClient_And_Done(pRst->nRst, "");
			return 0;
		}
		break;
	}

	return 0;
}
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::DoNext_TTY_TC_GROUP_ACCOUNT_LOGIN_RPL(const void* pData, size_t nDSize)
{
	P_DBS4WEBUSER_ACCOUNT_ONLINE_T*	pRst	= (P_DBS4WEBUSER_ACCOUNT_ONLINE_T*)pData;
	if (m_bPlayerDropped)
	{
		if (pRst->nRst == TTY_LOGIN_RST_SUCCEED)
		{
			m_pHost->TellDBPlayerLogout(m_nAccountID);
		}
		else
		{
			m_pHost->TellGlobalDBPlayerLogout(m_nAccountID);
		}
		ClearLoginPlayer_And_Done();
		return 0;
	}

	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer == NULL)
	{
		// 下线
		if (pRst->nRst == TTY_LOGIN_RST_SUCCEED)
		{
			m_pHost->TellDBPlayerLogout(m_nAccountID);
		}
		else
		{
			m_pHost->TellGlobalDBPlayerLogout(m_nAccountID);
		}
		ClearLoginPlayer_And_Done();
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
			RegTimeEvent(m_pHost->m_cfginfo.nTransactionTimeOut, TE_CHAR_LOAD);
			m_nCurTE		= TE_CHAR_LOAD;
		}
		break;
	case TTY_LOGIN_RST_NOCHAR:
		{
			// 上面已经判断,pPlayer存在,玩家未掉线

			// 1.设置为创建角色状态
			m_pHost->SetPlayerStatus(pPlayer, LPGamePlay4Web_i::PlayerUnit::STATUS_T::CREATECHAR);

			// 2.发送结果
			TTY_LPGAMEPLAY_CLIENT_LOGIN_RST_T	rst;
			rst.nCmd			= TTY_LPGAMEPLAY_CLIENT_LOGIN_RST;
			rst.nRst			= TTY_LOGIN_RST_NOCHAR;
			WH_STRNCPY0(rst.szDeviceID, pPlayer->szDeviceID);
			WH_STRNCPY0(rst.szAccount, pPlayer->szAccount);
			WH_STRNCPY0(rst.szPass, pPlayer->szPass);
			m_pHost->SendCmdToClient(m_nClientID, &rst, sizeof(rst));

			// 3.清除transaction
			pPlayer->nCurTransactionID	= 0;
			IAmDone();
		}
		break;
	default:
		{
			// 把用户从Login表中移出
			m_pHost->m_mapAccountID2ClientID.erase(m_nAccountID);
			SendErrRstToClient_And_Done(pRst->nRst, "");
		}
		break;
	}

	return 0;
}
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::DoNext_TTY_TC_CHAR_LOAD_RPL(const void* pData, size_t nDSize)
{
	P_DBS4WEBUSER_LOAD_CHAR_T*	pRst	= (P_DBS4WEBUSER_LOAD_CHAR_T*)pData;
	if (m_bPlayerDropped)
	{
		m_pHost->TellDBPlayerLogout(m_nAccountID);
		ClearLoginPlayer_And_Done();
		return 0;
	}
	
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer == NULL)
	{
		m_pHost->TellDBPlayerLogout(m_nAccountID);
		ClearLoginPlayer_And_Done();
		return 0;
	}
	switch (pRst->nRst)
	{
	case P_DBS4WEBUSER_LOAD_CHAR_T::RST_OK:
		{
			m_pHost->SetPlayerStatus(pPlayer, LPGamePlay4Web_i::PlayerUnit::STATUS_T::INLP);
			m_pHost->m_pGameMngS->Receive(PNGSPACKET_2DB4WEBUSER_RPL, pData, nDSize);
			pPlayer->nCurTransactionID	= 0;

			// 发送LOGIN结果给客户端
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
			return 0;
		}
		break;
	}
	return 0;
}
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::DoNext_TTY_TC_PLAYER_DROP(const void* pData, size_t nDSize)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GP_Transaction_LoginByDeviceID)"Player Dropped,%s,0x%X", m_szDeviceID, m_nClientID);
	m_bPlayerDropped	= true;

	switch (m_nCurTE)
	{
	case TE_GLOBAL_LOGIN:
		{
			m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
		}
		break;
	case TE_GROUP_LOGIN:
		{
			m_pHost->TellGlobalDBPlayerLogout(m_nAccountID);
			// 删除account_id映射
			m_pHost->m_mapAccountID2ClientID.erase(m_nAccountID);
		}
		break;
	case TE_CHAR_LOAD:
		{
			m_pHost->TellDBPlayerLogout(m_nAccountID);
			// 删除account_id映射
			m_pHost->m_mapAccountID2ClientID.erase(m_nAccountID);
		}
		break;
	}
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	if (pPlayer != NULL)
	{
		m_pHost->Player_Func_Deal_Drop_Nothing(pPlayer);
	}
	IAmDone();
	return 0;
}
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::DealTimeEvent(int nEvent)
{
	switch (nEvent)
	{
	case TE_GLOBAL_LOGIN:
		{
			m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
		}
		break;
	case TE_GROUP_LOGIN:
		{
			m_pHost->TellGlobalDBPlayerLogout(m_nAccountID);
			// 删除account_id映射
			m_pHost->m_mapAccountID2ClientID.erase(m_nAccountID);
		}
		break;
	case TE_CHAR_LOAD:
		{
			m_pHost->TellDBPlayerLogout(m_nAccountID);
			// 删除account_id映射
			m_pHost->m_mapAccountID2ClientID.erase(m_nAccountID);
		}
		break;
	}
	// 发送结果给客户端并结束
	SendErrRstToClient_And_Done(TTY_LOGIN_RST_TRANSACTION_TIMEOUT);
	return 0;
}
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::SendErrRstToClient_And_Done(tty_rst_t nRst, const char* pszPrompt/* = */)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GP_Transaction_LoginByDeviceID)"SendErrRstToClient_And_Done,%s,0x%X,%s,0x%X", pszPrompt, m_nClientID, m_szDeviceID, (int)nRst);
	// 发送结果
	TTY_LPGAMEPLAY_CLIENT_LOGIN_RST_T	rst;
	rst.nCmd			= TTY_LPGAMEPLAY_CLIENT_LOGIN_RST;
	rst.nRst			= nRst;

	m_pHost->SendCmdToClient(m_nClientID, &rst, sizeof(rst));

	// 把玩家恢复到LOGIN之前的状态
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
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
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::ClearTryLoginPlayer_And_Done()
{
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	m_pHost->m_mapTryLoginAccountName2ClientID.erase(m_szDeviceID);
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
	IAmDone();
	return 0;
}
int		LPGamePlay4Web_i::Transaction_LoginByDeviceID::ClearLoginPlayer_And_Done()
{
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= m_pHost->GetPlayerByID(m_nClientID);
	m_pHost->m_mapAccountID2ClientID.erase(m_nAccountID);
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
	IAmDone();
	return 0;
}
