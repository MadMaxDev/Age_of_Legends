#include "../inc/CAAFS4Web_i.h"

using namespace n_pngs;

int		CAAFS4Web_i::Tick_DealLogicProxyMsg()
{
	m_tickNow	= wh_gettickcount();
	
	pngs_cmd_t*	pCmd	= NULL;
	size_t		nSize	= 0;

	while ((pCmd=(pngs_cmd_t*)m_msgerLogicProxy.PeekMsg(&nSize)) != NULL)
	{
		switch (*pCmd)
		{
		case LP_CAAFS4Web_CTRL:
			{
				Tick_DealLogicProxyMsg_LP_CAAFS4Web_CTRL(pCmd, nSize);
			}
			break;
		case LP_CAAFS4Web_LPINFO:
			{
				Tick_DealLogicProxyMsg_LP_CAAFS4Web_LPINFO(pCmd, nSize);
			}
			break;
		case LP_CAAFS4Web_CLIENTCANGOTOCLS4Web:
			{
				Tick_DealLogicProxyMsg_LP_CAAFS4Web_CLIENTCANGOTOCLS4Web(pCmd, nSize);
			}
			break;
		}

		// 释放刚刚处理的消息
		m_msgerLogicProxy.FreeMsg();
	}

	return 0;
}
int		CAAFS4Web_i::Tick_DealLogicProxyMsg_LP_CAAFS4Web_CTRL(void* pCmd, int nSize)
{
	LP_CAAFS4Web_CTRL_T*	pCtrl	= (LP_CAAFS4Web_CTRL_T*)pCmd;
	switch (pCtrl->nSubCmd)
	{
	case LP_CAAFS4Web_CTRL_T::SUBCMD_CLIENTQUEUETRANS:		// (实际上这个现在没意义了)
		{
			PlayerUnit*		pPlayer	= m_Players.getptr(pCtrl->nExt);
			if (pPlayer != NULL)
			{
				// 如果用户还在,则把用户转到指定队列中去
				if (PutPlayerInChannel(pPlayer, pCtrl->nParam))
				{
					SendPlayerQueueInfo(pPlayer, true);
				}
			}
		}
		break;
	case LP_CAAFS4Web_CTRL_T::SUBCMD_EXIT:
		{
			m_nMood	= CMN::ILogic::MOOD_STOPPED;
			CmdOutToLogic_AUTO(m_pCMN, CMN::CMD2CMN_SHOULDSTOP, NULL, 0);
		}
		break;
	}
	return 0;
}
int		CAAFS4Web_i::Tick_DealLogicProxyMsg_LP_CAAFS4Web_LPINFO(void* pCmd, int nSize)
{
	LP_CAAFS4Web_LPINFO_T*	pLPInfo	= (LP_CAAFS4Web_LPINFO_T*)pCmd;
	m_nLogicProxyAvailPlayerNum		= pLPInfo->nLPAvailPlayer;
	return 0;
}
int		CAAFS4Web_i::Tick_DealLogicProxyMsg_LP_CAAFS4Web_CLIENTCANGOTOCLS4Web(void* pCmd, int nSize)
{
	LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T*	pClientCanGo	= (LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T*)pCmd;

	// 查找用户是否还存在(不存在说明已经从caafs4web中删除了)
	PlayerUnit*		pPlayer	= m_Players.getptr(pClientCanGo->nClientIDInCAAFS4Web);
	if (pPlayer == NULL)
	{
		// 用户不存在就不用管了
		GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T,Player Lost,0x%X,0x%X", pClientCanGo->nClientID, pClientCanGo->nClientIDInCAAFS4Web);
		return 0;
	}

	// 看看结果如何
	switch (pClientCanGo->nRst)
	{
	case LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T::RST_OK:
		{
			// 生成给客户端的指令
			CAAFS4Web_CLIENT_GOTOCLS4Web_T	GoToCLS4Web;
			GoToCLS4Web.nCmd				= CAAFS4Web_CLIENT_GOTOCLS4Web;
			GoToCLS4Web.nPort				= pClientCanGo->nPort;
			GoToCLS4Web.IP					= pClientCanGo->IP;
			GoToCLS4Web.nPassword			= pPlayer->nPasswordToConnectCLS4Web;
			GoToCLS4Web.nClientID			= pClientCanGo->nClientID;
			// 发送给客户端(客户端收到这个之后会主动关闭连接)
			m_pEpollServer->Send(pPlayer->nCntrID, &GoToCLS4Web, sizeof(GoToCLS4Web));
			// 客户端收到这个消息后就应该去连接CLS4Web,同时关闭这个连接了
			// 做超时,如果用户迟迟不关闭连接,则超时后直接关闭连接
			TQUNIT_T*	pTQUnit	= NULL;
			if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCloseCheckTimeout, (void**)&pTQUnit, &pPlayer->teid) < 0)
			{
				assert(0);
				// 移除用户
				RemovePlayerUnit(pPlayer);
				return -1;
			}
			pTQUnit->tefunc			= &CAAFS4Web_i::TEDeal_Close_TimeOut;
			pTQUnit->un.player.nID	= pPlayer->nID;
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T,0x%X,0x%X,Player Can Go", pClientCanGo->nClientID, pClientCanGo->nClientIDInCAAFS4Web);
		}
		break;
	default:
		{
			// 其他的就都应该是不成功了,应该删除用户
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T,Bad Rst,%d,0x%X,Player Removed", pClientCanGo->nRst, pClientCanGo->nClientIDInCAAFS4Web);
			RemovePlayerUnit(pPlayer);
		}
		break;
	}
	return 0;
}
