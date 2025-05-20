#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

//////////////////////////////////////////////////////////////////////////
// CLS4Web发来的
//////////////////////////////////////////////////////////////////////////
int		LPGamePlay4Web_i::DealCmdIn_LPPACKET_2GP_PLAYERCMD(ILogicBase* pRstAccepter, LPPACKET_2GP_PLAYERCMD_T* pCmd)
{
	// 这个时候的消息都是经过验证过有效性的,其他有效性后面验证
	// pCmd->nDSize是有效的,底层计算好后的实际尺寸
	LPGamePlay4Web_i::PlayerUnit*	pPlayer	= GetPlayerByID(pCmd->nClientID);
	if (pPlayer == NULL)
	{
		// 这个玩家下线了
		return 0;
	}
	tty_cmd_t*	pTTYCmd	= (tty_cmd_t*)pCmd->pData;
	switch (*pTTYCmd)
	{
	case TTY_CLIENT_LPGAMEPLAY_LOGIN:
		{
			(this->*pPlayer->pStatus->pFuncDealLogin)(pPlayer, *pTTYCmd, pCmd->pData, pCmd->nDSize);
		}
		break;
	case TTY_CLIENT_LPGAMEPLAY_CREATECHAR:
		{
			(this->*pPlayer->pStatus->pFuncDealCreateChar)(pPlayer, *pTTYCmd, pCmd->pData, pCmd->nDSize);
		}
		break;
	case TTY_CLIENT_LPGAMEPLAY_CREATEACCOUNT:
		{
			(this->*pPlayer->pStatus->pFuncDealCreateAccount)(pPlayer, *pTTYCmd, pCmd->pData, pCmd->nDSize);
		}
		break;
	case TTY_CLIENT_LPGAMEPLAY_GAME_CMD:
		{
			(this->*pPlayer->pStatus->pFuncDealGameCmd)(pPlayer, *pTTYCmd, pCmd, sizeof(LPPACKET_2GP_PLAYERCMD_T)+pCmd->nDSize);
		}
		break;
	case TTY_CLIENT_LPGAMEPLAY_LOGOUT:
		{
			(this->*pPlayer->pStatus->pFuncDealLogout)(pPlayer, *pTTYCmd, pCmd, pCmd->nDSize);
		}
		break;
	case TTY_CLIENT_LPGAMEPLAY_LOGIN_BY_DEVICEID:
		{
			(this->*pPlayer->pStatus->pFuncDealLoginByDeviceID)(pPlayer, *pTTYCmd, pCmd->pData, pCmd->nDSize);
		}
		break;
	default:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1397,LPGP_RT)"%s,%d,0x%X,%d", __FUNCTION__, (int)*pTTYCmd, pPlayer->nID, pPlayer->pStatus->nStatus);
			TellLogicMainStructureToKickPlayer(pPlayer->nID);
		}
		break;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// 处理CAAFS4Web发来的玩家指令
//////////////////////////////////////////////////////////////////////////
void	LPGamePlay4Web_i::DealCmdIn_LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web(ILogicBase* pRstAccepter, LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web_T* pCmd)
{
	// 忽略,暂时没有什么需要做的
}
