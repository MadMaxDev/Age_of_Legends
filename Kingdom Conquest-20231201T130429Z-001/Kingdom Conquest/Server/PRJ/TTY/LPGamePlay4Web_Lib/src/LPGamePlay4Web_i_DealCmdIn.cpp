#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

int		LPGamePlay4Web_i::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	// �ȴ���һЩ���÷��͵���Ϣ�����е�ָ��
	switch (nCmd)
	{
	case PNGSPACKET_2LOGIC_DISPHISTORY:
		{

		}
		break;
	case LPPACKET_2GP_CONFIG:
		{
			
		}
		break;
	default:
		{

		}
		break;
	}
	return 0;
}
int		LPGamePlay4Web_i::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch (nCmd)
	{
	case PNGSPACKET_2LOGIC_SETMOOD:
		{
			PNGSPACKET_2LOGIC_SETMOOD_T*	pCmdSetMood	= (PNGSPACKET_2LOGIC_SETMOOD_T*)pData;
			switch (pCmdSetMood->nMood)
			{
			case CMN::ILogic::MOOD_STOPPING:
				{
					// ��DBS4Web�������û�����,ʧ��Ҳû��ϵ,����ʱ�������һ�����,DBS4Web�����ػ���ʱ��Ҳ����һ��
					// !

					m_nMood	= CMN::ILogic::MOOD_STOPPED;
				}
				break;
			}
		}
		break;
	case LPPACKET_2GP_PLAYERCMD:
		{
			DealCmdIn_LPPACKET_2GP_PLAYERCMD(pRstAccepter, (LPPACKET_2GP_PLAYERCMD_T*)pData);
		}
		break;
	case LPPACKET_2GP_PLAYERONLINE:
		{
			LPPACKET_2GP_PLAYERONLINE_T*	pPlayerOnline	= (LPPACKET_2GP_PLAYERONLINE_T*)pData;
			// ���������
			PlayerUnit*	pPlayer				= m_Players.AllocUnitByID(pPlayerOnline->nClientID);
			if (pPlayer != NULL)
			{
				pPlayer->clear();
				pPlayer->nID				= pPlayerOnline->nClientID;
				pPlayer->IP					= pPlayerOnline->IP;
				pPlayer->nInTime			= m_timeNow;
				pPlayer->nTermType			= pPlayerOnline->nTermType;
				if (pPlayer->nTermType<=0 || pPlayer->nTermType>=TERM_TYPE_NUM)
				{
					m_Players.FreeUnitByPtr(pPlayer);
					TellLogicMainStructureToKickPlayer(pPlayerOnline->nClientID);
					// �ն����Ͳ���
					GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"LPPACKET_2GP_PLAYERONLINE_T,error term type,0x%X,%d", pPlayerOnline->nClientID, pPlayer->nTermType);
					return 0;
				}
				// ���벻Login�ĳ�ʱ
				SetPlayerTE_NoLogin(pPlayer);
				GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"LPPACKET_2GP_PLAYERONLINE_T,0x%X,%s", pPlayerOnline->nClientID, cmn_get_IP(pPlayerOnline->IP));
			}
			else
			{
				// ����ʧ��
				GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"LPPACKET_2GP_PLAYERONLINE_T,can not AllocUnitByID,0x%X", pPlayerOnline->nClientID);
				// ���û�����
				TellLogicMainStructureToKickPlayer(pPlayerOnline->nClientID);
			}
		}
		break;
	case LPPACKET_2GP_PLAYEROFFLINE:
		{
			LPPACKET_2GP_PLAYEROFFLINE_T*	pPlayerOffline	= (LPPACKET_2GP_PLAYEROFFLINE_T*)pData;
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1396,WEB_PLAYEROUT)"LPPACKET_2GP_PLAYEROFFLINE_T,0x%X,%d", pPlayerOffline->nClientID, pPlayerOffline->nReason);
			// ɾ�������
			PlayerUnit*	pPlayer			= GetPlayerByID(pPlayerOffline->nClientID);
			if (pPlayer != NULL)
			{
				pPlayer->nDropReason	= pPlayerOffline->nReason;
				(this->*pPlayer->pStatus->pFuncDealDrop)(pPlayer);
			}
		}
		break;
	case LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web:
		{
			DealCmdIn_LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web(pRstAccepter, (LPPACKET_2GP_PLAYERCMD_FROM_CAAFS4Web_T*)pData);
		}
		break;
	case PNGSPACKET_2DB4WEBUSER_RPL:
		{
			DealDBCmd((P_DBS4WEB_CMD_T*)pData, nDSize);
		}
		break;
	case PNGSPACKET_2DB4WEBUSER_READY4WORK:
		{
			// ���븱����Ϣ
			{
				P_DBS4WEB_GET_INSTANCE_SIMPLE_DATA_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_GET_INSTANCE_SIMPLE_DATA_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
			// ����һЩ��Ϣ
			if (!m_bLoadPlayerCardOK)
			{
				P_DBS4WEB_LOAD_PLAYERCARD_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_PLAYERCARD_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
			if (!m_bLoadTerrainOK)
			{
				P_DBS4WEB_LOAD_TERRAIN_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_TERRAIN_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
			// ֻҪ������������
			// �������ݣ�ս������Ҫ�õ���
			P_DBS4WEB_LOAD_ARMY_T	Cmd;
			Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd				= CMDID_LOAD_ARMY_REQ;
			CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			// ��������ID
			{
				P_DBS4WEB_LOAD_ALL_ALLIANCE_ID_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_ALL_ALLIANCE_ID_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
			if (!m_bLoadExcelTextOK)
			{
				P_DBS4WEB_LOAD_EXCEL_TEXT_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_EXCEL_TEXT_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
			if (!m_bLoadExcelAllianceCongressOK)
			{
				P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_EXCEL_ALLIANCE_CONGRESS_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
			if (!m_bAlreadyLoadRank)
			{
				m_pGameMngS->GetRankMng()->LoadRankFromDB();
			}

			// ÿ�������������������¼�
			DealExcelTimeEvent();
		}
		break;
	}
	return 0;
}
