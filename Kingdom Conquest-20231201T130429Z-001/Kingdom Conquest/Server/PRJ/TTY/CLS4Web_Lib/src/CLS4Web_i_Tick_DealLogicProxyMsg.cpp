#include "../inc/CLS4Web_i.h"

using namespace n_pngs;

int		CLS4Web_i::Tick_DealLogicProxyMsg()
{
	m_tickNow				= wh_gettickcount();
	
	// �����LP��������Ϣ
	pngs_cmd_t*		pCmd	= NULL;
	size_t			nSize	= 0;
	while ((pCmd=(pngs_cmd_t*)m_msgerLP.PeekMsg(&nSize)) != NULL)
	{
		switch (*pCmd)
		{
		case LP_CLS4Web_CLIENTWANTCLS4Web:
			{
				Tick_DealLogicProxyMsg_LP_CLS4Web_CLIENTWANTCLS4Web((const char*)pCmd, nSize);
			}
			break;
		case SVR_CLS4Web_CTRL:
			{
				Tick_Deal_CmnSvrMsg_SVR_CLS4Web_CTRL(pCmd, nSize);
			}
			break;
		case SVR_CLS4Web_SET_TAG_TO_CLIENT:
			{
				Tick_Deal_CmnSvrMsg_SVR_CLS4Web_SET_TAG_TO_CLIENT(pCmd, nSize);
			}
			break;
		case SVR_CLS4Web_CLIENT_DATA:
			{
				Tick_Deal_CmnSvrMsg_SVR_CLS4Web_CLIENT_DATA(pCmd, nSize);
			}
			break;
		case SVR_CLS4Web_MULTICLIENT_DATA:
			{
				Tick_Deal_CmnSvrMsg_SVR_CLS4Web_MULTICLIENT_DATA(pCmd, nSize);
			}
			break;
		case SVR_CLS4Web_TAGGED_CLIENT_DATA:
			{
				Tick_Deal_CmnSvrMsg_SVR_CLS4Web_TAGGED_CLIENT_DATA(pCmd, nSize);
			}
			break;
		case SVR_CLS4Web_ALL_CLIENT_DATA:
			{
				Tick_Deal_CmnSvrMsg_SVR_CLS4Web_ALL_CLIENT_DATA(pCmd, nSize);
			}
			break;
		case SVR_CLS4Web_SET_TAG64_TO_CLIENT:
			{
				Tick_Deal_CmnSvrMsg_SVR_CLS4Web_SET_TAG64_TO_CLIENT(pCmd, nSize);
			}
			break;
		case SVR_CLS4Web_TAGGED64_CLIENT_DATA:
			{
				Tick_Deal_CmnSvrMsg_SVR_CLS4Web_TAGGED64_CLIENT_DATA(pCmd, nSize);
			}
			break;
		}
		// �ͷŸոմ������Ϣ
		m_msgerLP.FreeMsg();
	}

	return 0;
}
int		CLS4Web_i::Tick_DealLogicProxyMsg_LP_CLS4Web_CLIENTWANTCLS4Web(const char* pCmd, size_t nSize)
{
	LP_CLS4Web_CLIENTWANTCLS4Web_T*	pClientWantCLS4Web	= (LP_CLS4Web_CLIENTWANTCLS4Web_T*)pCmd;
	// ���ԭ������ͬidx���û�����ɾ��
	int		nIdx			= m_Players.getrealidx(pClientWantCLS4Web->nClientID);
	PlayerUnit*	pPlayer		= m_Players.GetByIdx(nIdx);
	if (pPlayer != NULL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"LP_CLS4Web_CLIENTWANTCLS4Web_T,,RemovePlayerUnit Old,0x%X,%d,0x%X", pPlayer->nID, pPlayer->nStatus, pClientWantCLS4Web->nClientID);
		RemovePlayerUnit(pPlayer);
	}

	// ����Player
	pPlayer		= m_Players.AllocUnitByID(pClientWantCLS4Web->nClientID);
	assert(pPlayer != NULL);
	// Ϊ�˱�����clearһ��
	pPlayer->clear(this);
	// ����ID
	pPlayer->nID						= pClientWantCLS4Web->nClientID;
	// ����Ϊ���ӹ���״̬
	pPlayer->nStatus					= PlayerUnit::STATUS_CONNECTING;
	// ��������
	pPlayer->nPasswordToConnectCLS4Web	= pClientWantCLS4Web->nPassword;
	// �����ն�����
	pPlayer->nTermType					= pClientWantCLS4Web->nTermType;

	// �������ӳ�ʱ��ʱ��
	SetTEDeal_ClientConnectToMeTimeOut(pPlayer);

	// ���ظ���LP������
	{
		CLS4Web_LP_CLIENTCANGOTOCLS4Web_T	Cmd;
		Cmd.nCmd		= CLS4Web_LP_CLIENTCANGOTOCLS4Web;
		Cmd.nClientID	= pClientWantCLS4Web->nClientID;
		m_msgerLP.SendMsg(&Cmd, sizeof(Cmd));
		GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"CLS4Web_LP_CLIENTCANGOTOCLS4Web_T,Add Player,0x%X,%d,0x%X", pPlayer->nID, pPlayer->nStatus, pClientWantCLS4Web->nClientID);
	}

	return 0;
}
int		CLS4Web_i::Tick_Deal_CmnSvrMsg_SVR_CLS4Web_CTRL(void* pCmd, size_t nSize)
{
	SVR_CLS4Web_CTRL_T*	pCtrl	= (SVR_CLS4Web_CTRL_T*)pCmd;
	switch (pCtrl->nSubCmd)
	{
	case SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID:
	case SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID_AS_DROP:
	case SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID_AS_END:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"KICK,%d,0x%X", pCtrl->nSubCmd, pCtrl->nParam);
			// �߳��û�
			PlayerUnit*	pPlayer	= m_Players.getptr(pCtrl->nParam);
			if (pPlayer != NULL)
			{
				switch (pCtrl->nSubCmd)
				{
				case SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID_AS_DROP:
					{
						pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_AS_DROP;
					}
					break;
				case SVR_CLS4Web_CTRL_T::SUBCMD_KICKPLAYERBYID_AS_END:
					{
						pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_END;
					}
					break;
				default:
					{
						pPlayer->nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_KICKED;
					}
					break;
				}
				RemovePlayerUnit(pPlayer);
			}
			else
			{
				// ֱ�Ӱ����û��Ѿ��������˷��ͽ����LP
				CLS4Web_LP_CLIENT_DROP_T	CD;
				CD.nCmd				= CLS4Web_LP_CLIENT_DROP;
				CD.nRemoveReason	= CLS4Web_LP_CLIENT_DROP_T::REMOVEREASON_NOTEXIST;
				CD.nClientID		= pCtrl->nParam;
				m_msgerLP.SendMsg(&CD, sizeof(CD));
			}
		}
		break;
	case SVR_CLS4Web_CTRL_T::SUBCMD_SET_NAME:
		{
			PlayerUnit*	pPlayer	= m_Players.getptr(pCtrl->nParam);
			const char*	cszName	= (const char*)wh_getptrnexttoptr(pCtrl);
			if (pPlayer != NULL)
			{
				WH_STRNCPY0(pPlayer->szName, cszName);
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1395,CLS4Web_RT)"SVR_CLS4Web_CTRL_T::SUBCMD_SET_NAME,0x%X,0x%X,%s", pPlayer, pCtrl->nParam, cszName);
		}
		break;
	case SVR_CLS4Web_CTRL_T::SUBCMD_EXIT:
		{
			m_nMood	= CMN::ILogic::MOOD_STOPPED;
			CmdOutToLogic_AUTO(m_pCMN, CMN::CMD2CMN_SHOULDSTOP, NULL, 0);
		}
		break;
	case SVR_CLS4Web_CTRL_T::SUBCMD_KICK_ALL_PLAYER:
		{
			m_nStatus				= STATUS_KICK_ALL_PLAYER;
			// �������������
			m_pEpollServer->CloseAll();
		}
		break;
	}

	return 0;
}
