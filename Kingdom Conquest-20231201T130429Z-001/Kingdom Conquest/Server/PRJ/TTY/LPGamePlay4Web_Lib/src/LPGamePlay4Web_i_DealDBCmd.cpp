#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

int		LPGamePlay4Web_i::DealDBCmd(P_DBS4WEB_CMD_T* pBaseCmd, size_t nDSize)
{
	switch (pBaseCmd->nSubCmd)
	{
	case CMDID_ACCOUNT_ONLINE_RPL:
		{
			P_DBS4WEBUSER_ACCOUNT_ONLINE_T*	pRst	= (P_DBS4WEBUSER_ACCOUNT_ONLINE_T*)pBaseCmd;
			whtransactionbase*	pT		= m_pTMan->GetTransaction(pRst->nExt[0]);
			if (pT != NULL)
			{
				pT->DoNext(PNGSPACKET_2DB4WEBUSER_RPL, (const void*)pBaseCmd, nDSize);
			}
		}
		break;
	case CMDID_LOAD_CHAR_RPL:
		{
			P_DBS4WEBUSER_LOAD_CHAR_T*	pRst	= (P_DBS4WEBUSER_LOAD_CHAR_T*)pBaseCmd;
			whtransactionbase*	pT		= m_pTMan->GetTransaction(pRst->nExt[0]);
			if (pT != NULL)
			{
				pT->DoNext(PNGSPACKET_2DB4WEBUSER_RPL, (const void*)pBaseCmd, nDSize);
			}
		}
		break;
	case CMDID_CHAR_CREATE_RPL:
		{
			P_DBS4WEBUSER_CHAR_CREATE_T*	pRst	= (P_DBS4WEBUSER_CHAR_CREATE_T*)pBaseCmd;
			whtransactionbase*	pT		= m_pTMan->GetTransaction(pRst->nExt[0]);
			if (pT != NULL)
			{
				pT->DoNext(PNGSPACKET_2DB4WEBUSER_RPL, (const void*)pBaseCmd, nDSize);
			}
		}
		break;
	case CMDID_LOAD_PLAYERCARD_RPL:
		{
			P_DBS4WEBUSER_LOAD_PLAYERCARD_T*	pRst	= (P_DBS4WEBUSER_LOAD_PLAYERCARD_T*)pBaseCmd;
			if (pRst->nRst == P_DBS4WEBUSER_LOAD_PLAYERCARD_T::RST_OK)
			{
				m_pGameMngS->GetPlayerCardMng()->LoadCards((PlayerCard*)wh_getptrnexttoptr(pRst), pRst->nNum);
				m_bLoadPlayerCardOK		= true;
			}
			else	// 重新载入(应该不会失败好几次,不然以后做成时间回调)
			{
				P_DBS4WEB_LOAD_PLAYERCARD_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_PLAYERCARD_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_LOAD_TERRAIN_RPL:
		{
			P_DBS4WEBUSER_LOAD_TERRAIN_T*		pRst	= (P_DBS4WEBUSER_LOAD_TERRAIN_T*)pBaseCmd;
			if (pRst->nRst == P_DBS4WEBUSER_LOAD_TERRAIN_T::RST_OK)
			{
				m_pGameMngS->GetWorldMng()->LoadTerrainInfo((TerrainUnit*)wh_getptrnexttoptr(pRst), pRst->nNum);
				m_bLoadTerrainOK		= true;
			}
			else
			{
				P_DBS4WEB_LOAD_TERRAIN_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_TERRAIN_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_GET_TIMEEVENT_LIST_RPL:
		{
			P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T*	pRst	= (P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T*)pBaseCmd;
			P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T::TE_UNIT_T*	pTEUnit	= (P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T::TE_UNIT_T*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				switch (pTEUnit->nType)
				{
				case te_type_building:
					{
						P_DBS4WEB_DEAL_BUILDING_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_BUILDING_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_once_perperson:
					{
						P_DBS4WEB_DEAL_ONCE_PERPERSON_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_ONCE_PERPERSON_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_production:
					{
						P_DBS4WEB_DEAL_PRODUCTION_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_PRODUCTION_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_research:
					{
						P_DBS4WEB_DEAL_RESEARCH_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_RESEARCH_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_alliance_building:
					{
						P_DBS4WEB_DEAL_ALLIANCE_BUILDING_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_ALLIANCE_BUILDING_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_hero_training:
					{
						P_DBS4WEB_DEAL_TRAINING_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_TRAINING_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_alliance_trade:
					{
						P_DBS4WEB_DEAL_ALLIANCE_TREADE_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_ALLIANCE_TREADE_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_alliance_donate_soldier:
					{
						P_DBS4WEB_DEAL_DONATE_SOLDIER_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_DONATE_SOLDIER_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_alli_instance_march_advance:
					{
						P_DBS4WEB_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_alli_instance_march_back:
					{
						P_DBS4WEB_ALLI_INSTANCE_DEAL_MARCH_BACK_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_ALLI_INSTANCE_DEAL_MARCH_BACK_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;

				case te_type_combat_go:
				case te_type_combat_back:
					{
						P_DBS4WEB_DEAL_COMBAT_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_COMBAT_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_worldgoldmine_gain:
					{
						P_DBS4WEB_DEAL_WORLDGOLDMINE_GAIN_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_WORLDGOLDMINE_GAIN_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_fixhour:
					{
						P_DBS4WEB_DEAL_FIXHOUR_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_FIXHOUR_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				case te_type_worldcityrecover:
					{
						P_DBS4WEB_DEAL_WORLDCITYRECOVER_TE_T	Cmd;
						Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd		= CMDID_DEAL_WORLDCITYRECOVER_TE_REQ;
						Cmd.nEventID	= pTEUnit->nEventID;
						CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
					}
					break;
				default:
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DB_TIMEEVENT)"unknow db time_event type,%d,0x%"WHINT64PRFX"X", pTEUnit->nType, pTEUnit->nEventID);
					}
					break;
				}
				pTEUnit++;
			}
		}
		break;
	case CMDID_DEAL_BUILDING_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_BUILDING_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_BUILDING_TE_T*)pBaseCmd;
			if (pRst->nRst == 0)
			{
				PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					STC_GAMECMD_BUILDING_TE_T	Cmd;
					Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd	= STC_GAMECMD_BUILDING_TE;
					Cmd.nAutoID		= pRst->nAutoID;
					Cmd.nExcelID	= pRst->nExcelID;
					Cmd.nType		= pRst->nType;
					SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
				}
			}
		}
		break;
	case CMDID_GET_BUILDING_LIST_RPL:
		{
			P_DBS4WEBUSER_GET_BUILDING_LIST_T*	pRst	= (P_DBS4WEBUSER_GET_BUILDING_LIST_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_BUILDING_LIST_T) + pRst->nNum*sizeof(BuildingUnit));
			STC_GAMECMD_GET_BUILDING_LIST_T*	pRpl	= (STC_GAMECMD_GET_BUILDING_LIST_T*)m_vectrawbuf.getbuf();
			pRpl->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRpl->nGameCmd			= STC_GAMECMD_GET_BUILDING_LIST;
			pRpl->nNum				= pRst->nNum;
			pRpl->nRst				= pRst->nRst;
			if (pRpl->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pRpl), wh_getptrnexttoptr(pRst), pRpl->nNum*sizeof(BuildingUnit));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			PlayerCard*	pCard		= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
			if (pCard != NULL)
			{
				BuildingUnit*	pBuilding	= (BuildingUnit*)wh_getptrnexttoptr(pRst);
				for (int i=0; i<pRst->nNum; i++)
				{
					if (pBuilding->nExcelID == 1)	// 城堡等级就是城市等级
					{
						pCard->nCityLevel	= pBuilding->nLevel;
						break;
					}
					pBuilding++;
				}
			}
		}
		break;
	case CMDID_GET_BUILDING_TE_LIST_RPL:
		{
			P_DBS4WEBUSER_GET_BUILDING_TE_LIST_T*	pRst	= (P_DBS4WEBUSER_GET_BUILDING_TE_LIST_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_BUILDING_TE_LIST_T) + pRst->nNum*sizeof(BuildingTEUnit));
			STC_GAMECMD_GET_BUILDING_TE_LIST_T*	pRpl	= (STC_GAMECMD_GET_BUILDING_TE_LIST_T*)m_vectrawbuf.getbuf();
			pRpl->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRpl->nGameCmd			= STC_GAMECMD_GET_BUILDING_TE_LIST;
			pRpl->nNum				= pRst->nNum;
			pRpl->nRst				= pRst->nRst;
			pRpl->nTimeNow			= pRst->nTimeNow;
			if (pRpl->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pRpl), wh_getptrnexttoptr(pRst), pRpl->nNum*sizeof(BuildingTEUnit));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_BUILD_BUILDING_RPL:
		{
			P_DBS4WEBUSER_BUILD_BUILDING_T*	pRst	= (P_DBS4WEBUSER_BUILD_BUILDING_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_BUILD_BUILDING_T	RplCmd;
			RplCmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			RplCmd.nGameCmd			= STC_GAMECMD_OPERATE_BUILD_BUILDING;
			RplCmd.nAutoID			= pRst->nAutoID;
			RplCmd.nExcelID			= pRst->nExcelID;
			RplCmd.nRst				= pRst->nRst;
			RplCmd.nAddedCharExp	= pRst->nAddedCharExp;
			RplCmd.nAddedCharDev	= pRst->nAddedCharDev;
			RplCmd.nCurCharExp		= pRst->nCurCharExp;
			RplCmd.nCurCharLevel	= pRst->nCurCharLevel;
			SendCmdToClient(pRst->nExt[0], &RplCmd, sizeof(RplCmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_BUILD_BUILDING_T::RST_OK)
			{
				AddGoldCostLog(pRst->nAccountID, gold_cost_building, pRst->nGoldCost);

				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nLevel	= pRst->nCurCharLevel;
					pPlayer->m_CharAtb.nExp		= pRst->nCurCharExp;
					pPlayer->m_CharAtb.nDevelopment	+= pRst->nAddedCharDev;
					if (pPlayer->m_CharAtb.nGold >= pRst->nGoldCost)
					{
						pPlayer->m_CharAtb.nGold	-= pRst->nGoldCost;
					}
					else
					{
						// 应该记录下来日志
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Gold not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nGoldCost, pPlayer->m_CharAtb.nGold);
						pPlayer->m_CharAtb.nGold	= 0;
					}
					if (pPlayer->m_CharAtb.nPopulation >= pRst->nPopulationCost)
					{
						pPlayer->m_CharAtb.nPopulation	-= pRst->nPopulationCost;
					}
					else
					{
						// 应该记录下来日志
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Population not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nPopulationCost, pPlayer->m_CharAtb.nPopulation);
						pPlayer->m_CharAtb.nGold	= 0;
					}
				}

				PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
				if (pPlayerCard != NULL)
				{
					pPlayerCard->nLevel	= pRst->nCurCharLevel;
					pPlayerCard->nDevelopment	+= pRst->nAddedCharDev;
				}
			}
		}
		break;
	case CMDID_UPGRADE_BUILDING_RPL:
		{
			P_DBS4WEBUSER_UPGRADE_BUILDING_T*	pRst	= (P_DBS4WEBUSER_UPGRADE_BUILDING_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_UPGRADE_BUILDING_T	RplCmd;
			RplCmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			RplCmd.nGameCmd			= STC_GAMECMD_OPERATE_UPGRADE_BUILDING;
			RplCmd.nAutoID			= pRst->nAutoID;
			RplCmd.nExcelID			= pRst->nExcelID;
			RplCmd.nRst				= pRst->nRst;
			RplCmd.nAddedCharExp	= pRst->nAddedCharExp;
			RplCmd.nAddedCharDev	= pRst->nAddedCharDev;
			RplCmd.nCurCharExp		= pRst->nCurCharExp;
			RplCmd.nCurCharLevel	= pRst->nCurCharLevel;
			SendCmdToClient(pRst->nExt[0], &RplCmd, sizeof(RplCmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_UPGRADE_BUILDING_T::RST_OK)
			{
				AddGoldCostLog(pRst->nAccountID, gold_cost_building, pRst->nGoldCost);

				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nLevel	= pRst->nCurCharLevel;
					pPlayer->m_CharAtb.nExp		= pRst->nCurCharExp;
					pPlayer->m_CharAtb.nDevelopment	+= pRst->nAddedCharDev;
					if (pPlayer->m_CharAtb.nGold >= pRst->nGoldCost)
					{
						pPlayer->m_CharAtb.nGold	-= pRst->nGoldCost;
					}
					else
					{
						// 应该记录下来日志
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Gold not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nGoldCost, pPlayer->m_CharAtb.nGold);
						pPlayer->m_CharAtb.nGold	= 0;
					}
					if (pPlayer->m_CharAtb.nPopulation >= pRst->nPopulationCost)
					{
						pPlayer->m_CharAtb.nPopulation	-= pRst->nPopulationCost;
					}
					else
					{
						// 应该记录下来日志
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Population not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nPopulationCost, pPlayer->m_CharAtb.nPopulation);
						pPlayer->m_CharAtb.nGold	= 0;
					}
				}

				PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
				if (pPlayerCard != NULL)
				{
					pPlayerCard->nLevel	= pRst->nCurCharLevel;
					pPlayerCard->nDevelopment	+= pRst->nAddedCharDev;
				}
			}
		}
		break;
	case CMDID_DEAL_ONCE_PERPERSON_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_ONCE_PERPERSON_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_ONCE_PERPERSON_TE_T*)pBaseCmd;
			switch (pRst->nType)
			{
			case te_subtype_once_christmas_tree:
				{
					if (pRst->nResult == 0)
					{
						PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
						if (pPlayer != NULL)
						{
							STC_GAMECMD_CHRISTMAS_TREE_RIPE_T	Cmd;
							Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							Cmd.nGameCmd	= STC_GAMECMD_CHRISTMAS_TREE_RIPE;
							SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
						}
					}
				}
				break;
			case te_subtype_once_population:
				{
					PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
					if (pPlayer != NULL)
					{
						// 告诉客户端人口增加
						STC_GAMECMD_POPULATION_ADD_T Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_POPULATION_ADD;
						Cmd.nProduction	= pRst->nProduction;
						SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));

						// 注册新的时间事件
						m_pGameMngS->AddPopulationProductionTE(pRst->nAccountID);

						if (pRst->nResult == 0)
						{
							PlayerData*	pPlayerData	= GetPlayerData(pRst->nAccountID);
							if (pPlayerData != NULL)
							{
								pPlayerData->m_CharAtb.nPopulation	+= pRst->nProduction;
							}
						}
					}
				}
				break;
			case te_subtype_once_build_num:
				{
					PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
					if (pPlayer != NULL)
					{
						STC_GAMECMD_BUILD_ITEM_USE_OUT_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_BUILD_ITEM_USE_OUT;
						Cmd.nCurrentBuildNum	= pRst->nProduction;
						SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
					}
				}
				break;
			}
		}
		break;
	case CMDID_DEAL_PRODUCTION_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_PRODUCTION_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_PRODUCTION_TE_T*)pBaseCmd;
			switch (pRst->nType)
			{
			case te_subtype_production_gold:
				{
					PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
					if (pPlayer != NULL)
					{
						STC_GAMECMD_GOLD_CAN_FETCH_T	Cmd;
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_GOLD_CAN_FETCH;
						Cmd.nAutoID		= pRst->nAutoID;
						Cmd.nProduction	= pRst->nProduction;
						SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
					}
				}
				break;
			}
		}
		break;
	case CMDID_ADD_PRODUCTION_TE_RPL:
		{
			P_DBS4WEBUSER_ADD_PRODUCTION_TE_T*	pRst	= (P_DBS4WEBUSER_ADD_PRODUCTION_TE_T*)pBaseCmd;
			
			switch (pRst->nType)
			{
			case te_subtype_production_gold:
				{
					STC_GAMECMD_OPERATE_PRODUCE_GOLD_T	Cmd;
					Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd		= STC_GAMECMD_OPERATE_PRODUCE_GOLD;
					Cmd.nProduction		= pRst->nProduction;
					Cmd.nAutoID			= pRst->nAutoID;
					Cmd.nRst			= pRst->nResult;
					SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
				}
				break;
			}
		}
		break;
	case CMDID_FETCH_PRODUCTION_RPL:
		{
			P_DBS4WEBUSER_FETCH_PRODUCTION_T*	pRst	= (P_DBS4WEBUSER_FETCH_PRODUCTION_T*)pBaseCmd;
			
			switch (pRst->nType)
			{
			case te_subtype_production_gold:
				{
					STC_GAMECMD_OPERATE_FETCH_GOLD_T	Cmd;
					Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd		= STC_GAMECMD_OPERATE_FETCH_GOLD;
					Cmd.nAutoID			= pRst->nAutoID;
					Cmd.nProduction		= pRst->nProduction;
					Cmd.nRst			= pRst->nResult;
					SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

					if (pRst->nResult == STC_GAMECMD_OPERATE_FETCH_GOLD_T::RST_OK)
					{
						PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
						if (pPlayer != NULL)
						{
							pPlayer->m_CharAtb.nGold	+= pRst->nProduction;
						}

						AddGoldProduceLog(pRst->nAccountID, gold_produce_goldore, pRst->nProduction);
					}
				}
				break;
			}
		}
		break;
	case CMDID_GET_PRODUCTION_EVENT_RPL:
		{
			P_DBS4WEBUSER_GET_PRODUCTION_EVENT_T*	pRst	= (P_DBS4WEBUSER_GET_PRODUCTION_EVENT_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_PRODUCTION_EVENT_T) + pRst->nNum*sizeof(ProductionEvent));
			STC_GAMECMD_GET_PRODUCTION_EVENT_T*	pRpl	= (STC_GAMECMD_GET_PRODUCTION_EVENT_T*)m_vectrawbuf.getbuf();
			pRpl->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRpl->nGameCmd			= STC_GAMECMD_GET_PRODUCTION_EVENT;
			pRpl->nNum				= pRst->nNum;
			pRpl->nAccountID		= pRst->nAccountID;
			pRpl->nRst				= pRst->nResult;
			if (pRpl->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pRpl), wh_getptrnexttoptr(pRst), pRpl->nNum*sizeof(ProductionEvent));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_PRODUCTION_TE_RPL:
		{
			P_DBS4WEBUSER_GET_PRODUCTION_TE_T*	pRst	= (P_DBS4WEBUSER_GET_PRODUCTION_TE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_PRODUCTION_TE_LIST_T) + pRst->nNum*sizeof(ProductionTimeEvent));
			STC_GAMECMD_GET_PRODUCTION_TE_LIST_T*	pRpl	= (STC_GAMECMD_GET_PRODUCTION_TE_LIST_T*)m_vectrawbuf.getbuf();
			pRpl->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRpl->nGameCmd			= STC_GAMECMD_GET_PRODUCTION_TE_LIST;
			pRpl->nNum				= pRst->nNum;
			pRpl->nAccountID		= pRst->nAccountID;
			pRpl->nTimeNow			= pRst->nTimeNow;
			pRpl->nRst				= pRst->nResult;
			if (pRpl->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pRpl), wh_getptrnexttoptr(pRst), pRpl->nNum*sizeof(ProductionTimeEvent));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ACCELERATE_RPL:
		{
			P_DBS4WEBUSER_ACCELERATE_T*	pRst	= (P_DBS4WEBUSER_ACCELERATE_T*)pBaseCmd;
			unsigned int	nMoneyUseType		= money_use_type_none;
			switch (pRst->nType)
			{
			case te_type_building:
				{
					STC_GAMECMD_OPERATE_ACCE_BUILDING_T	Cmd;
					memset(&Cmd, 0, sizeof(Cmd));
					Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ACCE_BUILDING;
					Cmd.nAutoID		= pRst->nAutoID;
					Cmd.nMoneyNum	= pRst->nMoneyNum;
					Cmd.nMoneyType	= pRst->nMoneyType;
					Cmd.nRst		= pRst->nResult;
					Cmd.nTime		= pRst->nTime;
					Cmd.nType		= pRst->nSubType;
					Cmd.nExcelID	= pRst->nExcelID;
					SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

					nMoneyUseType	= money_use_type_accelerate_build;
				}
				break;
			case te_type_production:
				{
					if (pRst->nSubType == te_subtype_production_gold)
					{
						STC_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE_T Cmd;
						memset(&Cmd, 0, sizeof(Cmd));
						Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ACCE_GOLD_PRODUCE;
						Cmd.nAutoID		= pRst->nAutoID;
						Cmd.nMoneyNum	= pRst->nMoneyNum;
						Cmd.nMoneyType	= pRst->nMoneyType;
						Cmd.nRst		= pRst->nResult;
						Cmd.nTime		= pRst->nTime;
						Cmd.nExcelID	= pRst->nExcelID;
						SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

						nMoneyUseType	= money_use_type_accelerate_goldore;
					}
				}
				break;
			case te_type_research:
				{
					STC_GAMECMD_OPERATE_ACCE_RESEARCH_T	Cmd;
					memset(&Cmd, 0, sizeof(Cmd));
					Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ACCE_RESEARCH;
					Cmd.nExcelID	= pRst->nExcelID;
					Cmd.nTime		= pRst->nTime;
					Cmd.nMoneyNum	= pRst->nMoneyNum;
					Cmd.nMoneyType	= pRst->nMoneyType;
					Cmd.nRst		= pRst->nResult;
					Cmd.nExcelID	= pRst->nExcelID;
					SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

					nMoneyUseType	= money_use_type_accelerate_research;
				}
				break;
			}

			if (pRst->nResult == STC_GAMECMD_OPERATE_ACCE_T::RST_OK)
			{
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					switch (pRst->nMoneyType)
					{
					case money_type_diamond:
						{
							if (pPlayer->m_CharAtb.nDiamond >= pRst->nMoneyNum)
							{
								pPlayer->m_CharAtb.nDiamond	-= pRst->nMoneyNum;
							}
							else
							{
								pPlayer->m_CharAtb.nDiamond	= 0;
								GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Diamond not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nMoneyNum, pPlayer->m_CharAtb.nDiamond);
							}
						}
						break;
					case money_type_crystal:
						{
							if (pPlayer->m_CharAtb.nCrystal >= pRst->nMoneyNum)
							{
								pPlayer->m_CharAtb.nCrystal	-= pRst->nMoneyNum;
							}
							else
							{
								pPlayer->m_CharAtb.nCrystal	= 0;
								GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Crystal not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nMoneyNum, pPlayer->m_CharAtb.nCrystal);
							}
						}
						break;
					}

				}

				AddMoneyCostLog(pRst->nAccountID, nMoneyUseType, pRst->nMoneyType, pRst->nMoneyNum, pRst->nAcceTime, pRst->nExcelID, pRst->nAutoID);
			}
		}
		break;
	case CMDID_DEAL_RESEARCH_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_RESEARCH_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_RESEARCH_TE_T*)pBaseCmd;
			if (pRst->nRst == 0)
			{
				PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					STC_GAMECMD_RESEARCH_TE_T	Cmd;
					Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd	= STC_GAMECMD_RESEARCH_TE;
					Cmd.nExcelID	= pRst->nExcelID;
					Cmd.nLevel		= pRst->nLevel;

					SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
				}
			}
		}
		break;
	case CMDID_ADD_RESEARCH_TE_RPL:
		{
			P_DBS4WEBUSER_ADD_RESEARCH_TE_T*	pRst	= (P_DBS4WEBUSER_ADD_RESEARCH_TE_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_RESEARCH_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_RESEARCH;
			Cmd.nExcelID	= pRst->nExcelID;
			Cmd.nGold		= pRst->nGold;
			Cmd.nLevel		= pRst->nLevel;
			Cmd.nRst		= pRst->nResult;
			Cmd.nTime		= pRst->nTime;

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nResult == STC_GAMECMD_OPERATE_RESEARCH_T::RST_OK)
			{
				AddGoldCostLog(pRst->nAccountID, gold_cost_research, pRst->nGold);

				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					if (pPlayer->m_CharAtb.nGold >= pRst->nGold)
					{
						pPlayer->m_CharAtb.nGold	-= pRst->nGold;
					}
					else
					{
						// 应该记录下来日志
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Gold not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nGold, pPlayer->m_CharAtb.nGold);
						pPlayer->m_CharAtb.nGold	= 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_TECHNOLOGY_RPL:
		{
			P_DBS4WEBUSER_GET_TECHNOLOGY_T*	pRst	= (P_DBS4WEBUSER_GET_TECHNOLOGY_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_TECHNOLOGY_T) + pRst->nNum*sizeof(TechnologyUnit));
			STC_GAMECMD_GET_TECHNOLOGY_T*	pGet	= (STC_GAMECMD_GET_TECHNOLOGY_T*)m_vectrawbuf.getbuf();
			pGet->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pGet->nGameCmd	= STC_GAMECMD_GET_TECHNOLOGY;
			pGet->nNum		= pRst->nNum;
			pGet->nRst		= pRst->nRst;
			if (pGet->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pGet), wh_getptrnexttoptr(pRst), pGet->nNum*sizeof(TechnologyUnit));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_RESEARCH_TE_RPL:
		{
			P_DBS4WEBUSER_GET_RESEARCH_TE_T*	pRst	= (P_DBS4WEBUSER_GET_RESEARCH_TE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_RESEARCH_TE_T) + pRst->nNum*sizeof(ResearchTimeEvent));
			STC_GAMECMD_GET_RESEARCH_TE_T*	pGet		= (STC_GAMECMD_GET_RESEARCH_TE_T*)m_vectrawbuf.getbuf();
			pGet->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pGet->nGameCmd	= STC_GAMECMD_GET_RESEARCH_TE;
			pGet->nNum		= pRst->nNum;
			pGet->nRst		= pRst->nRst;
			pGet->nTimeNow	= pRst->nTimeNow;
			if (pGet->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pGet), wh_getptrnexttoptr(pRst), pGet->nNum*sizeof(ResearchTimeEvent));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_CONSCRIPT_SOLDIER_RPL:
		{
			P_DBS4WEBUSER_CONSCRIPT_SOLDIER_T*	pRst	= (P_DBS4WEBUSER_CONSCRIPT_SOLDIER_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER;
			Cmd.nExcelID	= pRst->nExcelID;
			Cmd.nLevel		= pRst->nLevel;
			Cmd.nNum		= pRst->nNum;
			Cmd.nGold		= pRst->nGold;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T::RST_OK)
			{
				AddGoldCostLog(pRst->nAccountID, gold_cost_conscript_soldier, pRst->nGold);

				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					if (pPlayer->m_CharAtb.nGold >= pRst->nGold)
					{
						pPlayer->m_CharAtb.nGold	-= pRst->nGold;
					}
					else
					{
						// 应该记录下来日志
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Gold not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nGold, pPlayer->m_CharAtb.nGold);
						pPlayer->m_CharAtb.nGold	= 0;
					}

					if (pPlayer->m_CharAtb.nPopulation >= pRst->nNum)
					{
						pPlayer->m_CharAtb.nPopulation	-= pRst->nNum;
					}
					else
					{
						// 应该记录下来日志
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Population not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nNum, pPlayer->m_CharAtb.nPopulation);
						pPlayer->m_CharAtb.nPopulation	= 0;
					}
				}
			}
		}
		break;
	case CMDID_UPGRADE_SOLDIER_RPL:
		{
			P_DBS4WEBUSER_UPGRADE_SOLDIER_T*	pRst	= (P_DBS4WEBUSER_UPGRADE_SOLDIER_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_UPGRADE_SOLDIER_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_UPGRADE_SOLDIER;
			Cmd.nExcelID	= pRst->nExcelID;
			Cmd.nFromLevel	= pRst->nFromLevel;
			Cmd.nToLevel	= pRst->nToLevel;
			Cmd.nNum		= pRst->nNum;
			Cmd.nGold		= pRst->nGold;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_UPGRADE_SOLDIER_T::RST_OK)
			{
				AddGoldCostLog(pRst->nAccountID, gold_cost_upgrade_soldier, pRst->nGold);

				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					if (pPlayer->m_CharAtb.nGold >= pRst->nGold)
					{
						pPlayer->m_CharAtb.nGold	-= pRst->nGold;
					}
					else
					{
						// 应该记录下来日志
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Gold not consist,%s,0x%"WHINT64PRFX"X,%d,%d", __FUNCTION__, pPlayer->m_CharAtb.nAccountID, pRst->nGold, pPlayer->m_CharAtb.nGold);
						pPlayer->m_CharAtb.nGold	= 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_SOLDIER_RPL:
		{
			P_DBS4WEBUSER_GET_SOLDIER_T*	pRst	= (P_DBS4WEBUSER_GET_SOLDIER_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_SOLDIER_T) + pRst->nNum*sizeof(SoldierUnit));
			STC_GAMECMD_GET_SOLDIER_T*	pGet		= (STC_GAMECMD_GET_SOLDIER_T*)m_vectrawbuf.getbuf();
			pGet->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pGet->nGameCmd	= STC_GAMECMD_GET_SOLDIER;
			pGet->nNum		= pRst->nNum;
			pGet->nRst		= pRst->nRst;
			if (pGet->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pGet), wh_getptrnexttoptr(pRst), pGet->nNum*sizeof(SoldierUnit));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ACCEPT_ALLIANCE_MEMBER_RPL:
		{
			P_DBS4WEBUSER_ACCEPT_ALLIANCE_MEMBER_T*	pRst	= (P_DBS4WEBUSER_ACCEPT_ALLIANCE_MEMBER_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER;
			Cmd.nMemberID	= pRst->nNewMemberID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
				}
				else
				{
					pAlliance->basicInfo.nTotalDevelopment	+= pRst->nDevelopment;
					pAlliance->basicInfo.nDevelopment		+= pRst->nDevelopment;

					for (int i=0; i<pAlliance->members.size(); i++)
					{
						const AllianceMemberUnit&	member	= pAlliance->members[i];
						if (member.nAccountID == pRst->nAccountID)
						{
							continue;
						}
						switch (member.nPosition)
						{
						case alliance_position_leader:
						case alliance_position_vice_leader:
						case alliance_position_manager:
							{
								PlayerData*	pPlayerTmp	= GetPlayerData(member.nAccountID);
								if (pPlayerTmp != NULL)
								{
									SendCmdToClient(pPlayerTmp->m_nPlayerGID, &Cmd, sizeof(Cmd));
								}
							}
							break;
						}
					}
				}

				// 设置联盟ID及tag64
				PlayerData*	pMember	= GetPlayerData(pRst->nNewMemberID);
				if (pMember != NULL)
				{
					pMember->m_CharAtb.nAllianceID	= pRst->nAllianceID;
					if (pAlliance != NULL)
					{
						WH_STRNCPY0(pMember->m_CharAtb.szAllianceName, pAlliance->basicInfo.szAllianceName);
					}
					g_pLPNet->SetPlayerTag64(pMember->m_nPlayerGID, pRst->nAllianceID, false);

					STC_GAMECMD_IN_ALLIANCE_T	Notify;
					Notify.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Notify.nGameCmd				= STC_GAMECMD_IN_ALLIANCE;
					Notify.nAllianceID			= pRst->nAllianceID;
					g_pLPNet->SendCmdToClient(pMember->m_nPlayerGID, &Notify, sizeof(Notify));
				}

				{
					// 还需要发送一封邮件
					char	szMsgBuf[1024];
					ExcelText*	pText		= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_in_alliance);
					if (pText != NULL)
					{
						const char*		pManagerName	= "";
						const char*		pAllianceName	= "";
						PlayerCard*	pManagerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
						if (pManagerCard != NULL)
						{
							pManagerName			= pManagerCard->szName;
							pAllianceName			= pManagerCard->szAllianceName;
						}
						if (pAlliance!=NULL && pAllianceName[0]!=0)
						{
							pAllianceName			= pAlliance->basicInfo.szAllianceName;
						}
						int	nTextLen			= sprintf(szMsgBuf, pText->szText, pManagerName, pAllianceName);
						AddMail(0, "", pRst->nNewMemberID, false, MAIL_TYPE_SYSTEM, MAIL_FLAG_NORMAL, false, nTextLen, szMsgBuf, 0, NULL, NULL);
					}
				}

				PlayerCard*	pCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nNewMemberID);
				if (pAlliance != NULL)
				{
					// 发送联盟邮件
					char	szMsgBuf[1024];
					ExcelText*	pText		= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_in_alliance_new);
					if (pText != NULL)
					{
						const char*	pMemberName	= "";
						if (pCard != NULL)
						{
							pMemberName		= pCard->szName;
						}
						int	nTextLen			= sprintf(szMsgBuf, pText->szText, pMemberName);
						for (int i=0; i<pAlliance->members.size(); i++)
						{
							AddMail(0, "", pAlliance->members[i].nAccountID, false, MAIL_TYPE_ALLIANCE_EVENT, MAIL_FLAG_NORMAL, false, nTextLen, szMsgBuf, 0, NULL, NULL);
						}
					}
				}

				// 添加联盟成员
				if (pAlliance != NULL)
				{
					if (pCard != NULL)
					{
						// 1.添加成员列表
						AllianceMemberUnit*	pMemberUnit	= pAlliance->members.push_back();
						pMemberUnit->nAccountID			= pCard->nAccountID;
						pMemberUnit->nDevelopment		= 0;
						pMemberUnit->nTotalDevelopment	= 0;
						pMemberUnit->nPosition			= alliance_position_member;
						pMemberUnit->nLevel				= pCard->nLevel;
						pMemberUnit->nCup				= pCard->nCup;
						WH_STRNCPY0(pMemberUnit->szName, pCard->szName);

						// 2.修改名片信息
						pCard->nAllianceID				= pRst->nAllianceID;
						WH_STRNCPY0(pCard->szAllianceName, pAlliance->basicInfo.szAllianceName);
					}

					// 3.删除申请事件
					pAlliance->joinEvents.delvalue(pRst->nNewMemberID);
				}
			}
		}
		break;
	case CMDID_BUILD_ALLIANCE_BUILDING_RPL:
		{
			P_DBS4WEBUSER_BUILD_ALLIANCE_BUILDING_T*	pRst	= (P_DBS4WEBUSER_BUILD_ALLIANCE_BUILDING_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING;
			Cmd.nExcelID	= pRst->nExcelID;
			Cmd.nType		= pRst->nType;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
				}
				else
				{
					// 修改发展度
					if (pAlliance->basicInfo.nDevelopment < pRst->nDevelopment)
					{
						pAlliance->basicInfo.nDevelopment	= 0;
					}
					else
					{
						pAlliance->basicInfo.nDevelopment	-= pRst->nDevelopment;
					}

					// 插入建筑时间事件
					AllianceBuildingTimeEvent*	pEvent		= pAlliance->buildingTEs.push_back();
					pEvent->nBeginTime			= wh_time();
					pEvent->nEndTime			= pEvent->nBeginTime + pRst->nTime;
					pEvent->nExcelID			= pRst->nExcelID;
					pEvent->nType				= pRst->nType;
				}
			}
		}
		break;
	case CMDID_CANCEL_JOIN_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_CANCEL_JOIN_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_CANCEL_JOIN_ALLIANCE_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			// 联盟加入事件是每次都从内存中获得，还是直接数据库查
			// 如果从内存中获得的话，应该返回account id,将事件插入内存,因为玩家可能下线,这样不能通过playerGID获取account id
			if (pRst->nRst==STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T::RST_OK
				&& pRst->nAllianceID!=0)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance != NULL)
				{
					pAlliance->joinEvents.delvalue(AllianceJoinEvent(pRst->nAccountID));
				}
			}
		}
		break;
	case CMDID_CREATE_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_CREATE_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_CREATE_ALLIANCE_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_CREATE_ALLI_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_CREATE_ALLI;
			Cmd.nAllianceID	= pRst->nAllianceID;
			Cmd.nRst		= pRst->nRst;
			Cmd.nGold		= pRst->nGold;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_CREATE_ALLI_T::RST_OK)
			{
				// 设置联盟ID
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nAllianceID	= pRst->nAllianceID;
					WH_STRNCPY0(pPlayer->m_CharAtb.szAllianceName, pRst->szAllianceName);
					SetPlayerTag64(pPlayer->m_nPlayerGID, pRst->nAllianceID, false);
					if (pPlayer->m_CharAtb.nGold >= pRst->nGold)
					{
						pPlayer->m_CharAtb.nGold	-= pRst->nGold;
					}
					else
					{
						pPlayer->m_CharAtb.nGold	= 0;
					}
				}

				// 修改名片
				PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pPlayer->m_CharAtb.nAccountID);
				if (pPlayerCard != NULL)
				{
					pPlayerCard->nAllianceID	= pRst->nAllianceID;
					WH_STRNCPY0(pPlayerCard->szAllianceName, pRst->szAllianceName);
				}

				// 应该向数据库请求载入联盟数据
				m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
				m_pGameMngS->GetAllianceMng()->m_vectAllianceRank.push_back(pRst->nAllianceID);
			}
		}
		break;
	case CMDID_DEAL_ALLIANCE_BUILDING_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_ALLIANCE_BUILDING_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_ALLIANCE_BUILDING_TE_T*)pBaseCmd;
			// 应该把alliance在内存中的建筑物升级
			if (pRst->nRst == 0)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
				}
				else
				{
					switch (pRst->nType)
					{
					case te_subtype_alliance_building_build:
						{
							AllianceBuildingUnit*	pBuilding	= pAlliance->buildings.push_back();
							pBuilding->nExcelID		= pRst->nExcelID;
							pBuilding->nLevel		= 1;
						}
						break;
					case te_subtype_alliance_building_upgrade:
						{
							for (unsigned int i=0; i<pAlliance->buildings.size(); i++)
							{
								if (pAlliance->buildings[i].nExcelID == pRst->nExcelID)
								{
									pAlliance->buildings[i].nLevel++;
								}
							}
						}
						break;
					}
					pAlliance->buildingTEs.delvalue(AllianceBuildingTimeEvent(pRst->nExcelID, pRst->nType));
				}
			}
		}
		break;
	case CMDID_DISMISS_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_DISMISS_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_DISMISS_ALLIANCE_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_DISMISS_ALLI_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_DISMISS_ALLI;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (Cmd.nRst == STC_GAMECMD_OPERATE_DISMISS_ALLI_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					return 0;
				}
				// 联盟邮件
				ExcelText*	pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_dismiss_alliance);
				bool	bSendMail	= false;
				char	szBuf[excel_text_max_length];
				int		nLength		= 0;
				if (pText!=NULL)
				{
					nLength				= sprintf(szBuf, pText->szText, pAlliance->basicInfo.szLeaderName);
					bSendMail			= true;
				}
				// 应该找出联盟成员,将内存中的AllianceID全设置为0,同时清除列表,给成员发送解散邮件
				for (unsigned int i=0; i<pAlliance->members.size(); i++)
				{
					// 发送解散邮件
					if (bSendMail)
					{
						AddMail(0, "", pAlliance->members[i].nAccountID, false, MAIL_TYPE_SYSTEM, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
					}
					// 名片信息
					PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pAlliance->members[i].nAccountID);
					if (pPlayerCard != NULL)
					{
						pPlayerCard->nAllianceID		= 0;
						pPlayerCard->szAllianceName[0]	= 0;
					}

					PlayerData*	pPlayerData	= GetPlayerData(pAlliance->members[i].nAccountID);
					if (pPlayerData == NULL)
					{
						continue;
					}
					// 1.设置alliance id为0
					pPlayerData->m_CharAtb.nAllianceID			= 0;
					pPlayerData->m_CharAtb.szAllianceName[0]	= 0;
					// 2.设置tag64
					SetPlayerTag64(pPlayerData->m_nPlayerGID, pRst->nAllianceID, true);
					// 3.删除联盟事件邮件
					pPlayerData->ClearMailList(MAIL_TYPE_ALLIANCE_EVENT);
				}

				// 删除联盟在内存中的数据
				m_pGameMngS->GetAllianceMng()->RemoveAlliance(pRst->nAllianceID);
				// 删除聊天相关内容
				m_pGameMngS->GetChatMng()->RemoveAllianceChat(pRst->nAllianceID);
			}
		}
		break;
	case CMDID_EXIT_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_EXIT_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_EXIT_ALLIANCE_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_EXIT_ALLI_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_EXIT_ALLI;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_EXIT_ALLI_T::RST_OK)
			{
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					// 1.设置联盟ID为0
					pPlayer->m_CharAtb.nAllianceID			= 0;
					pPlayer->m_CharAtb.szAllianceName[0]	= 0;
					// 2.设置tag64
					SetPlayerTag64(pRst->nExt[0], pRst->nAllianceID, true);
					// 3.删除联盟事件邮件
					pPlayer->ClearMailList(MAIL_TYPE_ALLIANCE_EVENT);
				}

				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
				}
				else
				{
					// 修改发展度
					pAlliance->basicInfo.nTotalDevelopment	-= pRst->nDevelopment;
					if (pAlliance->basicInfo.nDevelopment < pRst->nDevelopment)
					{
						pAlliance->basicInfo.nDevelopment	= 0;
					}
					else
					{
						pAlliance->basicInfo.nDevelopment	-= pRst->nDevelopment;
					}

					// 删除成员
					pAlliance->members.delvalue(AllianceMemberUnit(pRst->nAccountID));

					// 日志
					PlayerCard*	pMemeberCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
					if (pMemeberCard != NULL)
					{
						pMemeberCard->nAllianceID		= 0;
						pMemeberCard->szAllianceName[0]	= 0;

						char	szBuf[excel_text_max_length];
						int		nLength	= 0;
						// 添加私人日志
						ExcelText*	pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_pl_exit_alliance);
						if (pText!=NULL)
						{
							nLength	= sprintf(szBuf, pText->szText, pAlliance->basicInfo.szAllianceName);
							AddMail(0, "", pRst->nAccountID, false, MAIL_TYPE_SYSTEM, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
						}

						// 给联盟添加日志
						pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_al_exit_alliance);
						if (pText!=NULL)
						{
							nLength	= sprintf(szBuf, pText->szText, pMemeberCard->szName, pRst->nDevelopment);
							for (int i=0; i<pAlliance->members.size(); i++)
							{
								AddMail(0, "", pAlliance->members[i].nAccountID, false, MAIL_TYPE_ALLIANCE_EVENT, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
							}
						}
					}
				}
			}
		}
		break;
	case CMDID_EXPEL_ALLIANCE_MEMBER_RPL:
		{
			P_DBS4WEBUSER_EXPEL_ALLIANCE_MEMBER_T*	pRst	= (P_DBS4WEBUSER_EXPEL_ALLIANCE_MEMBER_T*)pBaseCmd;
			
			STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER;
			Cmd.nMemberID	= pRst->nMemberID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T::RST_OK)
			{
				PlayerData*	pMember	= GetPlayerData(pRst->nMemberID);
				if (pMember != NULL)
				{
					// 1.设置联盟ID为0
					pMember->m_CharAtb.nAllianceID			= 0;
					pMember->m_CharAtb.szAllianceName[0]	= 0;
					// 2.设置tag64
					SetPlayerTag64(pMember->m_nPlayerGID, pRst->nAllianceID, true);
					// 3.删除联盟事件邮件
					pMember->ClearMailList(MAIL_TYPE_ALLIANCE_EVENT);

					STC_GAMECMD_EXPELED_ALLI_T	RplCmd;
					RplCmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					RplCmd.nGameCmd				= STC_GAMECMD_EXPELED_ALLI;
					SendCmdToClient(pMember->m_nPlayerGID, &RplCmd, sizeof(RplCmd));
				}

				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
				}
				else
				{
					// 修改发展度
					pAlliance->basicInfo.nTotalDevelopment	-= pRst->nDevelopment;
					if (pAlliance->basicInfo.nDevelopment < pRst->nDevelopment)
					{
						pAlliance->basicInfo.nDevelopment	= 0;
					}
					else
					{
						pAlliance->basicInfo.nDevelopment	-= pRst->nDevelopment;
					}

					// 删除成员
					pAlliance->members.delvalue(AllianceMemberUnit(pRst->nMemberID));

					// 日志
					PlayerCard*	pManagerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nManagerID);
					PlayerCard*	pMemeberCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nMemberID);
					if (pManagerCard!=NULL && pMemeberCard!=NULL)
					{
						pMemeberCard->nAllianceID			= 0;
						pMemeberCard->szAllianceName[0]		= 0;

						char	szBuf[excel_text_max_length];
						int		nLength	= 0;
						// 添加私人日志
						ExcelText*	pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_pl_expel_alliance);
						if (pText!=NULL)
						{
							nLength	= sprintf(szBuf, pText->szText, pManagerCard->szName, pAlliance->basicInfo.szAllianceName);
							AddMail(0, "", pRst->nMemberID, false, MAIL_TYPE_SYSTEM, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
						}

						// 给联盟添加日志
						pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_al_expel_alliance);
						if (pText!=NULL)
						{
							nLength	= sprintf(szBuf, pText->szText, pMemeberCard->szName, pManagerCard->szName, pRst->nDevelopment);
							for (int i=0; i<pAlliance->members.size(); i++)
							{
								AddMail(0, "", pAlliance->members[i].nAccountID, false, MAIL_TYPE_ALLIANCE_EVENT, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
							}
						}
					}
				}
			}
		}
		break;
	case CMDID_JOIN_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_JOIN_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_JOIN_ALLIANCE_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_JOIN_ALLI_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_JOIN_ALLI;
			Cmd.nAllianceID	= pRst->nAllianceID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_JOIN_ALLI_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= (AllianceData_T*)m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance != NULL)
				{
					AllianceJoinEvent*	pJoin	= (AllianceJoinEvent*)pAlliance->joinEvents.push_back();
					pJoin->nAccountID		= pRst->nAccountID;
					WH_STRNCPY0(pJoin->szName, pRst->szName);
					pJoin->nLevel			= pRst->nLevel;
					pJoin->nHeadID			= pRst->nHeadID;
				
					// 给管理员通知
					for (int i=0; i<pAlliance->members.size(); i++)
					{
						AllianceMemberUnit& member	= pAlliance->members[i];
						if (member.nPosition != alliance_position_member)
						{
							PlayerData*	pManager	= GetPlayerData(member.nAccountID);
							if (pManager != NULL)
							{
								TTY_LPGAMEPLAY_CLIENT_GAME_CMD_T	Cmd;
								Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
								Cmd.nGameCmd	= STC_GAMECMD_NEW_ALLIANCE_JOIN;
								g_pLPNet->SendCmdToClient(pManager->m_nPlayerGID, &Cmd, sizeof(Cmd));
							}
						}
					}
				}
			}
		}
		break;
	case CMDID_SET_ALLIANCE_POSITION_RPL:
		{
			P_DBS4WEBUSER_SET_ALLIANCE_POSITION_T*	pRst	= (P_DBS4WEBUSER_SET_ALLIANCE_POSITION_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_SET_ALLI_POSITION_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_SET_ALLI_POSITION;
			Cmd.nMemberID	= pRst->nMemberID;
			Cmd.nPosition	= pRst->nPosition;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			// 内存中应该更改该玩家的职位
			if (pRst->nRst == STC_GAMECMD_OPERATE_SET_ALLI_POSITION_T::RST_OK)
			{
				PlayerData*	pPlayerTmp		= GetPlayerData(pRst->nMemberID);
				if (pPlayerTmp != NULL)
				{
					STC_GAMECMD_NEW_ALLIANCE_POSITION_T	CmdTmp;
					CmdTmp.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					CmdTmp.nGameCmd		= STC_GAMECMD_NEW_ALLIANCE_POSITION;
					CmdTmp.nPosition	= pRst->nPosition;
					SendCmdToClient(pPlayerTmp->m_nPlayerGID, &CmdTmp, sizeof(CmdTmp));
				}

				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance != NULL)
				{
					SExcelLine*	pLine		= g_ExcelTableMng.SafeGetLine(tabel_id_alliance_position_right, pRst->nPosition);
					const char*	cszPositionName	= "";
					if (pLine != NULL)
					{
						cszPositionName		= pLine->FastGetStr(apr_col_name, true);
					}
					ExcelText*	pText		= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_set_alliance_position);
					bool	bSendMail		= false;
					const char*	cszManagerName	= "";
					const char*	cszMemberName	= "";
					PlayerCard*	pManagerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nManagerID);
					if (pManagerCard != NULL)
					{
						cszManagerName			= pManagerCard->szName;
					}
					PlayerCard*	pMemberCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nMemberID);
					if (pMemberCard != NULL)
					{
						cszMemberName			= pMemberCard->szName;
					}
					char	szBuf[excel_text_max_length];
					int		nLength	= 0;
					if (pText != NULL)
					{
						nLength	= sprintf(szBuf, pText->szText, cszManagerName, cszMemberName, cszPositionName);
						bSendMail		= true;
					}
					for (unsigned int i=0; i<pAlliance->members.size(); i++)
					{
						if (pAlliance->members[i].nAccountID == pRst->nMemberID)
						{
							pAlliance->members[i].nPosition	= pRst->nPosition;
						}
						if (bSendMail)
						{
							AddMail(0, "", pAlliance->members[i].nAccountID, false, MAIL_TYPE_ALLIANCE_EVENT, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
						}
					}
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_INFO_RPL:
		{
			P_DBS4WEBUSER_GET_ALLIANCE_INFO_T*	pRst	= (P_DBS4WEBUSER_GET_ALLIANCE_INFO_T*)pBaseCmd;

			STC_GAMECMD_GET_ALLIANCE_INFO_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_GET_ALLIANCE_INFO;
			Cmd.nRst		= pRst->nRst;
			if (Cmd.nRst != STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_OK)
			{
				Cmd.alliance.nAllianceID	= pRst->nAllianceID;
			}
			else
			{
				memcpy(&Cmd.alliance, wh_getptrnexttoptr(pRst), sizeof(Cmd.alliance));
			}
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_GET_ALLIANCE_MEMBER_RPL:
		{
			P_DBS4WEBUSER_GET_ALLIANCE_MEMBER_T*	pRst	= (P_DBS4WEBUSER_GET_ALLIANCE_MEMBER_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_MEMBER_T) + pRst->nNum*sizeof(AllianceMemberUnit));
			STC_GAMECMD_GET_ALLIANCE_MEMBER_T*	pGet	= (STC_GAMECMD_GET_ALLIANCE_MEMBER_T*)m_vectrawbuf.getbuf();
			pGet->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pGet->nGameCmd	= STC_GAMECMD_GET_ALLIANCE_MEMBER;
			pGet->nNum		= pRst->nNum;
			pGet->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pGet), wh_getptrnexttoptr(pRst), pRst->nNum*sizeof(AllianceMemberUnit));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_ALLIANCE_BUILDING_TE_RPL:
		{
			P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_TE_T*	pRst	= (P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_TE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T) + pRst->nNum*sizeof(AllianceBuildingTimeEvent));
			STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T*	pGet	= (STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T*)m_vectrawbuf.getbuf();
			pGet->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pGet->nGameCmd	= STC_GAMECMD_GET_ALLIANCE_BUILDING_TE;
			pGet->nNum		= pRst->nNum;
			pGet->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pGet), wh_getptrnexttoptr(pRst), pRst->nNum*sizeof(AllianceBuildingTimeEvent));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_ALLIANCE_BUILDING_RPL:
		{
			P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_T*	pRst	= (P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_BUILDING_T) + pRst->nNum*sizeof(AllianceBuildingUnit));
			STC_GAMECMD_GET_ALLIANCE_BUILDING_T*	pGet	= (STC_GAMECMD_GET_ALLIANCE_BUILDING_T*)m_vectrawbuf.getbuf();
			pGet->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pGet->nGameCmd	= STC_GAMECMD_GET_ALLIANCE_BUILDING;
			pGet->nNum		= pRst->nNum;
			pGet->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pGet), wh_getptrnexttoptr(pRst), pRst->nNum*sizeof(AllianceBuildingUnit));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_ALLIANCE_JOIN_EVENT_RPL:
		{
			P_DBS4WEBUSER_GET_ALLIANCE_JOIN_EVENT_T*	pRst	= (P_DBS4WEBUSER_GET_ALLIANCE_JOIN_EVENT_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T) + pRst->nNum*sizeof(AllianceJoinEvent));
			STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T*	pGet		= (STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T*)m_vectrawbuf.getbuf();
			pGet->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pGet->nGameCmd	= STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT;
			pGet->nNum		= pRst->nNum;
			pGet->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pGet), wh_getptrnexttoptr(pRst), pRst->nNum*sizeof(AllianceJoinEvent));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ADD_PRIVATE_MAIL_RPL:
		{
			P_DBS4WEBUSER_ADD_PRIVATE_MAIL_T*	pRst	= (P_DBS4WEBUSER_ADD_PRIVATE_MAIL_T*)pBaseCmd;
			if (pRst->bNeedRst)
			{
				STC_GAMECMD_ADD_MAIL_T	Cmd;
				Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				Cmd.nGameCmd			= STC_GAMECMD_ADD_MAIL;
				Cmd.nRst				= pRst->nRst;
				SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_GET_PRIVATE_MAIL_RPL:
		{
			P_DBS4WEBUSER_GET_PRIVATE_MAIL_T*	pRst	= (P_DBS4WEBUSER_GET_PRIVATE_MAIL_T*)pBaseCmd;
// 			int	nMailTotalSize				= nDSize - sizeof(P_DBS4WEBUSER_GET_PRIVATE_MAIL_T);
// 			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_MAIL_T) + nMailTotalSize);
// 			STC_GAMECMD_GET_MAIL_T*	pGet	= (STC_GAMECMD_GET_MAIL_T*)m_vectrawbuf.getbuf();
// 			pGet->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 			pGet->nGameCmd			= STC_GAMECMD_GET_MAIL;
// 			pGet->nNum				= pRst->nNum;
// 			pGet->nRst				= pRst->nRst;
// 			memcpy(wh_getptrnexttoptr(pGet), wh_getptrnexttoptr(pRst), nMailTotalSize);
// 			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			// 使用缓存了
			PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
			if (pPlayer != NULL)
			{
				pPlayer->ClearMailList();
				PrivateMailUnit*	pMailUnit	= (PrivateMailUnit*)wh_getptrnexttoptr(pRst);
				for (unsigned int i=0; i<pRst->nNum; i++)
				{
					int	nMailSize		= wh_offsetof(PrivateMailUnit, szText) + pMailUnit->nTextLen;
					int*	pExtDataLen	= (int*)wh_getoffsetaddr(pMailUnit, nMailSize);
					void*	pExtData	= (void*)wh_getptrnexttoptr(pExtDataLen);
					nMailSize			+= (sizeof(int) + *pExtDataLen);
					PrivateMailUnit*	pNewMail= (PrivateMailUnit*)(new char[nMailSize]);
					memcpy(pNewMail, pMailUnit, nMailSize);
					pPlayer->AddNewMail(pNewMail);
					pMailUnit			= (PrivateMailUnit*)wh_getoffsetaddr(pMailUnit, nMailSize);
				}
			}
		}
		break;
	case CMDID_READ_PRIVATE_MAIL_RPL:
		{
			P_DBS4WEBUSER_READ_PRIVATE_MAIL_T*	pRst	= (P_DBS4WEBUSER_READ_PRIVATE_MAIL_T*)pBaseCmd;
			STC_GAMECMD_READ_MAIL_T	Cmd;
			Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd			= STC_GAMECMD_READ_MAIL;
			Cmd.nMailID				= pRst->nMailID;
			Cmd.nRst				= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_DEL_PRIVATE_MAIL_RPL:
		{
			P_DBS4WEBUSER_DEL_PRIVATE_MAIL_T*	pRst	= (P_DBS4WEBUSER_DEL_PRIVATE_MAIL_T*)pBaseCmd;
			
			PlayerData*	pPlayer			= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
			if (pPlayer != NULL)
			{
				unsigned char	nMailType	= 0;
				if (pRst->nRst == STC_GAMECMD_DELETE_MAIL_T::RST_OK)
				{
					for (whlist<PrivateMailUnit*>::iterator it=pPlayer->m_MailList.begin(); it!=pPlayer->m_MailList.end(); ++it)
					{
						PrivateMailUnit*	pMail		= *it;
						if (pMail->nMailID == pRst->nMailID)
						{
							if (pMail->nFlag != MAIL_FLAG_REWARD)
							{
								nMailType		= pMail->nType;
								pPlayer->m_MailList.erase(it);
							}
							break;
						}
					}
				}
				switch (nMailType)
				{
				case MAIL_TYPE_PRIVATE:
				case MAIL_TYPE_ALLIANCE:
					{
						nMailType	= MAIL_TYPE_PRIVATE|MAIL_TYPE_ALLIANCE;
					}
					break;
				case MAIL_TYPE_SYSTEM:
					{

					}
					break;
				case MAIL_TYPE_ALLIANCE_EVENT:
					{

					}
					break;
				case MAIL_TYPE_GOLDORE_EVENT:
					{

					}
					break;
				}
				STC_GAMECMD_DELETE_MAIL_T	Cmd;
				Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				Cmd.nGameCmd				= STC_GAMECMD_DELETE_MAIL;
				Cmd.nMailID					= pRst->nMailID;
				Cmd.nTotalMailNum			= 0;
				for (whlist<PrivateMailUnit*>::iterator it=pPlayer->m_MailList.begin(); it!=pPlayer->m_MailList.end(); ++it)
				{
					PrivateMailUnit*	pMail		= *it;
					if ((pMail->nType&nMailType) != 0)
					{
						Cmd.nTotalMailNum++;
					}
				}
				Cmd.nRst					= pRst->nRst;
				SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_NEW_PRIVATE_MAIL_RPL:
		{
			P_DBS4WEBUSER_NEW_PRIVATE_MAIL_T*	pRst	= (P_DBS4WEBUSER_NEW_PRIVATE_MAIL_T*)pBaseCmd;
			PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
			if (pPlayer != NULL)
			{
				int	nMsgUnitLen			= nDSize - sizeof(P_DBS4WEBUSER_NEW_PRIVATE_MAIL_T);
				m_vectrawbuf.resize(sizeof(STC_GAMECMD_NEW_PRIVATE_MAIL_T) + nMsgUnitLen);
				STC_GAMECMD_NEW_PRIVATE_MAIL_T*	pCmd	= (STC_GAMECMD_NEW_PRIVATE_MAIL_T*)m_vectrawbuf.getbuf();
				pCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				pCmd->nGameCmd			= STC_GAMECMD_NEW_PRIVATE_MAIL;
				pCmd->nTotalMailNum		= 0;
				PrivateMailUnit*	pMail	= (PrivateMailUnit*)wh_getptrnexttoptr(pCmd);
				memcpy(pMail, wh_getptrnexttoptr(pRst), nMsgUnitLen);

				{
					// new mail
					PlayerData*	pPlayerData			= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pPlayer->nID);
					if (pPlayerData != NULL)
					{
						PrivateMailUnit*	pNewMail	= (PrivateMailUnit*)(new char[nMsgUnitLen]);
						memcpy(pNewMail, pMail, nMsgUnitLen);
						pPlayerData->AddNewMail(pNewMail);

						unsigned char	nMailType	= 0;
						switch (pMail->nType)
						{
						case MAIL_TYPE_PRIVATE:
						case MAIL_TYPE_ALLIANCE:
							{
								nMailType			= MAIL_TYPE_PRIVATE|MAIL_TYPE_ALLIANCE;
							}
							break;
						default:
							{
								nMailType			= pMail->nType;
							}
							break;
						}
						whlist<PrivateMailUnit*>::iterator	it		= pPlayerData->m_MailList.begin();
						for (; it!=pPlayerData->m_MailList.end(); ++it)
						{
							if (((*it)->nType&nMailType) != 0)
							{
								pCmd->nTotalMailNum++;
							}
						}
					}
				}

				SendCmdToClient(pPlayer->nID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
		}
		break;
	case CMDID_GET_PRIVATE_MAIL_1_RPL:
		{
			P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T*	pRst	= (P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T*)pBaseCmd;
			int	nMailSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T);
			if (pRst->nRst == P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T::RST_OK)
			{
				PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					// 新邮件添加进去
					PrivateMailUnit*	pMail	= (PrivateMailUnit*)(new char[nMailSize]);
					memcpy(pMail, wh_getptrnexttoptr(pRst), nMailSize);
					pPlayer->AddNewMail(pMail);

					// 发送给客户端
					m_vectrawbuf.resize(sizeof(STC_GAMECMD_NEW_PRIVATE_MAIL_T) + nMailSize);
					STC_GAMECMD_NEW_PRIVATE_MAIL_T*	pNewMail	= (STC_GAMECMD_NEW_PRIVATE_MAIL_T*)m_vectrawbuf.getbuf();
					pNewMail->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					pNewMail->nGameCmd	= STC_GAMECMD_NEW_PRIVATE_MAIL;
					pNewMail->nTotalMailNum		= 0;
					memcpy(wh_getptrnexttoptr(pNewMail), wh_getptrnexttoptr(pRst), nMailSize);
					
					unsigned char	nMailType	= 0;
					switch (pMail->nType)
					{
					case MAIL_TYPE_PRIVATE:
					case MAIL_TYPE_ALLIANCE:
						{
							nMailType			= MAIL_TYPE_PRIVATE|MAIL_TYPE_ALLIANCE;
						}
						break;
					default:
						{
							nMailType			= pMail->nType;
						}
						break;
					}
					whlist<PrivateMailUnit*>::iterator	it		= pPlayer->m_MailList.begin();
					for (; it!=pPlayer->m_MailList.end(); ++it)
					{
						if (((*it)->nType&nMailType) != 0)
						{
							pNewMail->nTotalMailNum++;
						}
					}
					SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
			}
		}
		break;
	case CMDID_REFUSE_JOIN_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_REFUSE_JOIN_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_REFUSE_JOIN_ALLIANCE_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI;
			Cmd.nApplicantID= pRst->nApplicantID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance != NULL)
				{
					pAlliance->joinEvents.delvalue(pRst->nApplicantID);

					for (int i=0; i<pAlliance->members.size(); i++)
					{
						const AllianceMemberUnit&	member	= pAlliance->members[i];
						if (member.nAccountID == pRst->nManagerID)
						{
							continue;
						}
						switch (member.nPosition)
						{
						case alliance_position_leader:
						case alliance_position_vice_leader:
						case alliance_position_manager:
							{
								PlayerData*	pPlayerTmp	= GetPlayerData(member.nAccountID);
								if (pPlayerTmp != NULL)
								{
									SendCmdToClient(pPlayerTmp->m_nPlayerGID, &Cmd, sizeof(Cmd));
								}
							}
							break;
						}
					}
				}

				ExcelText*	pText		= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_apply_alliance_refused);
				if (pText != NULL)
				{
					char	szBuf[excel_text_max_length];
					int		nLength	= 0;
					const char*	szAllianceName	= "";
					if (pAlliance != NULL)
					{
						szAllianceName	= pAlliance->basicInfo.szAllianceName;
					}
					nLength	= sprintf(szBuf, pText->szText, szAllianceName);
					AddMail(0, "", pRst->nApplicantID, false, MAIL_TYPE_SYSTEM, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
				}
			}
		}
		break;
	case CMDID_ABDICATE_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_ABDICATE_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_ABDICATE_ALLIANCE_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ABDICATE_ALLI_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ABDICATE_ALLI;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (Cmd.nRst == STC_GAMECMD_OPERATE_ABDICATE_ALLI_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance != NULL)
				{
					PlayerCard*	pMemberCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nMemberID);
					// 发送联盟邮件
					ExcelText*	pText		= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_abdicate_alliance);
					bool	bSendMail		= false;

					char	szBuf[excel_text_max_length];
					int		nLength	= 0;
					if (pText != NULL)
					{
						
						const char*	szNewLeaderName	= "";
						if (pMemberCard != NULL)
						{
							szNewLeaderName	= pMemberCard->szName;
						}
						nLength			= sprintf(szBuf, pText->szText, pAlliance->basicInfo.szLeaderName, szNewLeaderName);
						bSendMail		= true;
					}
					// 修改联盟基本信息
					if (pMemberCard != NULL)
					{
						pAlliance->basicInfo.nLeaderID	= pMemberCard->nAccountID;
						WH_STRNCPY0(pAlliance->basicInfo.szLeaderName, pMemberCard->szName);
					}
					else
					{
						m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
					}

					for (int i=0; i<pAlliance->members.size(); i++)
					{
						AllianceMemberUnit&	m	= pAlliance->members[i];
						if (m.nPosition == alliance_position_leader)
						{
							m.nPosition			= alliance_position_member;
						}
						if (m.nAccountID == pRst->nMemberID)
						{
							m.nPosition			= alliance_position_leader;
						}
						if (bSendMail)
						{
							AddMail(0, "", pAlliance->members[i].nAccountID, false, MAIL_TYPE_ALLIANCE_EVENT, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
						}
					}
				}
			}
		}
		break;
	case CMDID_GET_MY_FRIEND_APPLY_RPL:
		{
			P_DBS4WEBUSER_GET_MY_FRIEND_APPLY_T*	pRst	= (P_DBS4WEBUSER_GET_MY_FRIEND_APPLY_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_MY_FRIEND_APPLY_T) + pRst->nNum*sizeof(FriendApplyUnit));
			STC_GAMECMD_GET_MY_FRIEND_APPLY_T*	pRpl		= (STC_GAMECMD_GET_MY_FRIEND_APPLY_T*)m_vectrawbuf.getbuf();
			pRpl->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRpl->nGameCmd	= STC_GAMECMD_GET_MY_FRIEND_APPLY;
			pRpl->nNum		= pRst->nNum;
			pRpl->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRpl), wh_getptrnexttoptr(pRst), pRst->nNum*sizeof(FriendApplyUnit));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_OTHERS_FRIEND_APPLY_RPL:
		{
			P_DBS4WEBUSER_GET_OTHERS_FRIEND_APPLY_T*	pRst	= (P_DBS4WEBUSER_GET_OTHERS_FRIEND_APPLY_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_OTHERS_FRIEND_APPLY_T) + pRst->nNum*sizeof(FriendApplyUnit));
			STC_GAMECMD_GET_OTHERS_FRIEND_APPLY_T*	pRpl		= (STC_GAMECMD_GET_OTHERS_FRIEND_APPLY_T*)m_vectrawbuf.getbuf();
			pRpl->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRpl->nGameCmd	= STC_GAMECMD_GET_OTHERS_FRIEND_APPLY;
			pRpl->nNum		= pRst->nNum;
			pRpl->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRpl), wh_getptrnexttoptr(pRst), pRst->nNum*sizeof(FriendApplyUnit));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_FRIEND_LIST_RPL:
		{
			P_DBS4WEBUSER_GET_FRIEND_LIST_T*	pRst	= (P_DBS4WEBUSER_GET_FRIEND_LIST_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_FRIEND_LIST_T) + pRst->nNum*sizeof(FriendUnit));
			STC_GAMECMD_GET_FRIEND_LIST_T*	pRpl		= (STC_GAMECMD_GET_FRIEND_LIST_T*)m_vectrawbuf.getbuf();
			pRpl->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRpl->nGameCmd	= STC_GAMECMD_GET_FRIEND_LIST;
			pRpl->nNum		= pRst->nNum;
			pRpl->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRpl), wh_getptrnexttoptr(pRst), pRst->nNum*sizeof(FriendUnit));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_APPROVE_FRIEND_RPL:
		{
			P_DBS4WEBUSER_APPROVE_FRIEND_T*	pRst	= (P_DBS4WEBUSER_APPROVE_FRIEND_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_APPROVE_FRIEND_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_APPROVE_FRIEND;
			PlayerCard*	pCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nPeerAccountID);
			if (pCard == NULL)
			{
				memset(&Cmd.friendUnit, 0, sizeof(Cmd.friendUnit));
				Cmd.friendUnit.nAccountID	= pRst->nPeerAccountID;
			}
			else
			{
				Cmd.friendUnit.nAccountID	= pCard->nAccountID;
				WH_STRNCPY0(Cmd.friendUnit.szName, pCard->szName);
				Cmd.friendUnit.nLevel		= pCard->nLevel;
				Cmd.friendUnit.nHeadID		= pCard->nHeadID;
			}
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			// 给对方也发送下
			{
				PlayerData*	pPeerPlayer	= GetPlayerData(pRst->nPeerAccountID);
				if (pPeerPlayer != NULL)
				{
					// 说明在线
					PlayerCard*	pCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
					if (pCard != NULL)
					{
						Cmd.friendUnit.nAccountID	= pCard->nAccountID;
						Cmd.friendUnit.nLevel		= pCard->nLevel;
						WH_STRNCPY0(Cmd.friendUnit.szName, pCard->szName);
						Cmd.friendUnit.nHeadID		= pCard->nHeadID;
						SendCmdToClient(pPeerPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
				}
			}
		}
		break;
	case CMDID_APPLY_FRIEND_RPL:
		{
			P_DBS4WEBUSER_APPLY_FRIEND_T*	pRst	= (P_DBS4WEBUSER_APPLY_FRIEND_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_APPLY_FRIEND_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_APPLY_FRIEND;
			Cmd.nPeerAccountID	= pRst->nPeerAccountID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_APPLY_FRIEND_T::RST_OK)
			{
				PlayerData*	pPeerPlayer	= GetPlayerData(pRst->nPeerAccountID);	// 这个能看是否在线
				if (pPeerPlayer != NULL)
				{
					PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
					if (pPlayerCard != NULL)
					{
						STC_GAMECMD_NEW_FRIEND_APPLY_T	Notify;
						Notify.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Notify.nGameCmd		= STC_GAMECMD_NEW_FRIEND_APPLY;
						Notify.unit.nAccountID	= pPlayerCard->nAccountID;
						Notify.unit.nLevel		= pPlayerCard->nLevel;
						WH_STRNCPY0(Notify.unit.szName, pPlayerCard->szName);
						Notify.unit.nHeadID		= pPlayerCard->nHeadID;
						SendCmdToClient(pPeerPlayer->m_nPlayerGID, &Notify, sizeof(Notify));
					}
				}
			}
		}
		break;
	case CMDID_REFUSE_FRIEND_APPLY_RPL:
		{
			P_DBS4WEBUSER_REFUSE_FRIEND_APPLY_T*	pRst	= (P_DBS4WEBUSER_REFUSE_FRIEND_APPLY_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY;
			Cmd.nPeerAccountID	= pRst->nPeerAccountID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_CANCEL_FRIEND_APPLY_RPL:
		{
			P_DBS4WEBUSER_CANCEL_FRIEND_APPLY_T*	pRst	= (P_DBS4WEBUSER_CANCEL_FRIEND_APPLY_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY;
			Cmd.nPeerAccountID	= pRst->nPeerAccountID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_DELETE_FRIEND_RPL:
		{
			P_DBS4WEBUSER_DELETE_FRIEND_T*	pRst	= (P_DBS4WEBUSER_DELETE_FRIEND_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_DELETE_FRIEND_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_DELETE_FRIEND;
			Cmd.nPeerAccountID	= pRst->nPeerAccountID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_DELETE_FRIEND_T::RST_OK)
			{
				Cmd.nPeerAccountID	= pRst->nAccountID;

				PlayerData*	pPeerPlayer	= GetPlayerData(pRst->nPeerAccountID);
				if (pPeerPlayer != NULL)
				{
					SendCmdToClient(pPeerPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
				}
			}
		}
		break;
	case CMDID_LOAD_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_LOAD_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_LOAD_ALLIANCE_T*)pBaseCmd;
			if (pRst->nRst != P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_OK)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"FATAL ERROR,load_alliance ERROR,%d,0x%"WHINT64PRFX"X", pRst->nRst, pRst->nAllianceID);
			}
			else
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					pAlliance	= m_pGameMngS->GetAllianceMng()->AddAlliance(pRst->nAllianceID);
					if (pAlliance == NULL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"Cannot alloc alliance,0x%"WHINT64PRFX"X", pRst->nAllianceID);
						return 0;
					}
				}
				
				int	nOffset	= sizeof(P_DBS4WEBUSER_LOAD_ALLIANCE_T);
				// 1.基本信息
				{
					AllianceUnit*	pBaseInfo	= (AllianceUnit*)wh_getoffsetaddr(pRst, nOffset);
					memcpy(&pAlliance->basicInfo, pBaseInfo, sizeof(AllianceUnit));
					pAlliance->basicInfo.nRank			= m_pGameMngS->GetAllianceMng()->GetAllianceRank(pBaseInfo->nAllianceID);
					nOffset	+= sizeof(AllianceUnit);
				}

				// 2.成员列表
				{
					int*	pNum	= (int*)wh_getoffsetaddr(pRst, nOffset);
					pAlliance->members.resize(*pNum);
					memcpy(pAlliance->members.getbuf(), wh_getptrnexttoptr(pNum), *pNum*sizeof(AllianceMemberUnit));
					nOffset	+= (sizeof(int) + *pNum*sizeof(AllianceMemberUnit));
				}
				
				// 3.建筑时间队列
				{
					int*	pNum	= (int*)wh_getoffsetaddr(pRst, nOffset);
					pAlliance->buildingTEs.resize(*pNum);
					memcpy(pAlliance->buildingTEs.getbuf(), wh_getptrnexttoptr(pNum), *pNum*sizeof(AllianceBuildingTimeEvent));
					nOffset	+= (sizeof(int) + *pNum*sizeof(AllianceBuildingTimeEvent));
				}

				// 4.建筑列表
				{
					int*	pNum	= (int*)wh_getoffsetaddr(pRst, nOffset);
					pAlliance->buildings.resize(*pNum);
					memcpy(pAlliance->buildings.getbuf(), wh_getptrnexttoptr(pNum), *pNum*sizeof(AllianceBuildingUnit));
					nOffset	+= (sizeof(int) + *pNum*sizeof(AllianceBuildingUnit));
				}

				// 5.申请加入联盟事件
				{
					int*	pNum	= (int*)wh_getoffsetaddr(pRst, nOffset);
					pAlliance->joinEvents.resize(*pNum);
					memcpy(pAlliance->joinEvents.getbuf(), wh_getptrnexttoptr(pNum), *pNum*sizeof(AllianceJoinEvent));
					nOffset	+= (sizeof(int) + *pNum*sizeof(AllianceJoinEvent));
				}

				// 申请载入联盟邮件(作废，联盟邮件都走私人邮件)
// 				{
// 					P_DBS4WEB_GET_ALLIANCE_MAIL_T	Cmd;
// 					Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
// 					Cmd.nSubCmd		= CMDID_GET_ALLIANCE_MAIL_REQ;
// 					Cmd.nAllianceID	= pRst->nAllianceID;
// 					Cmd.nExt[0]		= 0;
// 
// 					g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
// 				}
			}
		}
		break;
	case CMDID_LOAD_ALL_ALLIANCE_ID_RPL:
		{
			P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T*	pRst	= (P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T*)pBaseCmd;
			if (pRst->nRst != P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T::RST_OK)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GameMngS)"FATAL ERROR,load_all_alliance_id ERROR,%d", pRst->nRst);
			}
			else
			{
				
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GameMngS)"load_all_alliance_id OK,%d", pRst->nNum);

				tty_id_t*	pAllianceID	= (tty_id_t*)wh_getptrnexttoptr(pRst);
				if (pRst->nNum>0)
				{
					m_pGameMngS->GetAllianceMng()->LoadAllianceRank(pRst->nNum, pAllianceID);
				}
				for (int i=0; i<pRst->nNum; i++)
				{
					m_pGameMngS->LoadAllianceFromDB(pAllianceID[i]);
				}
			}
		}
		break;
	case CMDID_LOAD_EXCEL_TEXT_RPL:
		{
			P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T*	pRst	= (P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T*)pBaseCmd;
			if (pRst->nRst != P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T::RST_OK)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(,LP_RT)"Load Text Excel ERROR!!!%d", pRst->nRst);
				// 重新载入
				P_DBS4WEB_LOAD_EXCEL_TEXT_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_EXCEL_TEXT_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
			else
			{
				m_bLoadExcelTextOK		= true;
				m_pGameMngS->GetExcelTextMngS()->Load(pRst->nSize, pRst->nNum, (ExcelText*)wh_getptrnexttoptr(pRst));
			}
		}
		break;
	case CMDID_LOAD_EXCEL_ALLIANCE_CONGRESS_RPL:
		{
			P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T*	pRst	= (P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T*)pBaseCmd;
			if (pRst->nRst != P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T::RST_OK)
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(,LP_RT)"Load AllianceCongress Excel ERROR!!!%d", pRst->nRst);
				// 重新载入
				P_DBS4WEB_LOAD_EXCEL_ALLIANCE_CONGRESS_T	Cmd;
				Cmd.nCmd				= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd				= CMDID_LOAD_EXCEL_ALLIANCE_CONGRESS_REQ;
				CmdOutToLogic_AUTO(m_pLogicDB, PNGSPACKET_2DB4WEB_REQ, &Cmd, sizeof(Cmd));
			}
			else
			{
				m_bLoadExcelAllianceCongressOK		= true;
				m_pGameMngS->GetAllianceMng()->LoadExcelAllianceCongress(pRst->nNum, (ExcelAllianceCongress*)wh_getptrnexttoptr(pRst));
			}
		}
		break;
	case CMDID_ADD_ALLIANCE_MAIL_RPL:
		{
			P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T*	pRst	= (P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T*)pBaseCmd;
			if (pRst->nRst == STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance != NULL)
				{
					int	nMailSize			= nDSize-sizeof(P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T);
					AllianceMailUnit*	pNewMail	= (AllianceMailUnit*)(new char[nMailSize]);
					memcpy(pNewMail, wh_getptrnexttoptr(pRst), nMailSize);
					pAlliance->mailList.push_front(pNewMail);

					// 通知所有在线的联盟成员,有新的联盟邮件到来
					m_vectrawbuf.resize(sizeof(STC_GAMECMD_NEW_ALLIANCE_MAIL_T) + nMailSize);
					STC_GAMECMD_NEW_ALLIANCE_MAIL_T*	pNewMailNotify	= (STC_GAMECMD_NEW_ALLIANCE_MAIL_T*)m_vectrawbuf.getbuf();
					pNewMailNotify->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					pNewMailNotify->nGameCmd		= STC_GAMECMD_NEW_ALLIANCE_MAIL;
					memcpy(wh_getptrnexttoptr(pNewMailNotify), pNewMail, nMailSize);
					SendCmdToClientWithTag64(pRst->nAllianceID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
			}
			STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL;
			Cmd.nRst		= pRst->nRst;

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ADD_ALLIANCE_LOG_RPL:
		{
		}
		break;
	case CMDID_ADD_PRIVATE_LOG_RPL:
		{
		}
		break;
	case CMDID_GET_ALLIANCE_MAIL_RPL:
		{
			P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T*	pRst	= (P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T*)pBaseCmd;
// 			int	nTotalLogSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T);
// 			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_MAIL_T) + nTotalLogSize);
// 			STC_GAMECMD_GET_ALLIANCE_MAIL_T*	pRplCmd		= (STC_GAMECMD_GET_ALLIANCE_MAIL_T*)m_vectrawbuf.getbuf();
// 			pRplCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
// 			pRplCmd->nGameCmd	= STC_GAMECMD_GET_ALLIANCE_MAIL;
// 			pRplCmd->nNum		= pRst->nNum;
// 			pRplCmd->nRst		= pRst->nRst;
// 			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nTotalLogSize);
// 			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
			if (pAlliance != NULL)
			{
				pAlliance->ClearMailList();
				AllianceMailUnit*	pMail	= (AllianceMailUnit*)wh_getptrnexttoptr(pRst);
				for (unsigned int i=0; i<pRst->nNum; i++)
				{
					int		nMailSize		= wh_offsetof(AllianceMailUnit, szText) + pMail->nTextLen;
					AllianceMailUnit*	pNewMail	= (AllianceMailUnit*)(new char[nMailSize]);
					memcpy(pNewMail, pMail, nMailSize);
					pAlliance->mailList.push_back(pNewMail);
					pMail					= (AllianceMailUnit*)wh_getoffsetaddr(pMail, nMailSize);
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_LOG_RPL:
		{
			P_DBS4WEBUSER_GET_ALLIANCE_LOG_T*	pRst	= (P_DBS4WEBUSER_GET_ALLIANCE_LOG_T*)pBaseCmd;
			int	nTotalLogSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_ALLIANCE_LOG_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ALLIANCE_LOG_T) + nTotalLogSize);
			STC_GAMECMD_GET_ALLIANCE_LOG_T*	pRplCmd		= (STC_GAMECMD_GET_ALLIANCE_LOG_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd	= STC_GAMECMD_GET_ALLIANCE_LOG;
			pRplCmd->nNum		= pRst->nNum;
			pRplCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nTotalLogSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_PRIVATE_LOG_RPL:
		{
			P_DBS4WEBUSER_GET_PRIVATE_LOG_T*	pRst	= (P_DBS4WEBUSER_GET_PRIVATE_LOG_T*)pBaseCmd;
			int	nTotalLogSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_PRIVATE_LOG_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_PRIVATE_LOG_T) + nTotalLogSize);
			STC_GAMECMD_GET_PRIVATE_LOG_T*	pRplCmd		= (STC_GAMECMD_GET_PRIVATE_LOG_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd	= STC_GAMECMD_GET_PRIVATE_LOG;
			pRplCmd->nNum		= pRst->nNum;
			pRplCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nTotalLogSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_BUY_ITEM_RPL:
		{
			P_DBS4WEBUSER_BUY_ITEM_T*	pRst	= (P_DBS4WEBUSER_BUY_ITEM_T*)pBaseCmd;
			int	nItemIDSize		= nDSize - sizeof(P_DBS4WEBUSER_BUY_ITEM_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_BUY_ITEM_T) + nItemIDSize);
			STC_GAMECMD_OPERATE_BUY_ITEM_T*	pRplCmd	= (STC_GAMECMD_OPERATE_BUY_ITEM_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd	= STC_GAMECMD_OPERATE_BUY_ITEM;
			pRplCmd->nMoneyNum	= pRst->nMoneyNum;
			pRplCmd->nMoneyType	= pRst->nMoneyType;
			pRplCmd->nExcelID	= pRst->nExcelID;
			pRplCmd->nItemIDNum	= pRst->nItemIDNum;
			pRplCmd->nNum		= pRst->nNum;
			pRplCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nItemIDSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			if (pRplCmd->nRst == STC_GAMECMD_OPERATE_BUY_ITEM_T::RST_OK)
			{
				PlayerData*	pPlayerData	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayerData != NULL)
				{
					switch (pRst->nMoneyType)
					{
					case money_type_diamond:
						{
							if (pPlayerData->m_CharAtb.nDiamond>pRst->nMoneyNum)
							{
								pPlayerData->m_CharAtb.nDiamond	-= pRst->nMoneyNum;
							}
							else
							{
								pPlayerData->m_CharAtb.nDiamond	= 0;
							}
						}
						break;
					case money_type_crystal:
						{
							if (pPlayerData->m_CharAtb.nCrystal>pRst->nMoneyNum)
							{
								pPlayerData->m_CharAtb.nCrystal	-= pRst->nMoneyNum;
							}
							else
							{
								pPlayerData->m_CharAtb.nCrystal	= 0;
							}
						}
						break;
					}
				}

				AddMoneyCostLog(pRst->nAccountID, money_use_type_buy_item, pRst->nMoneyType, pRst->nMoneyNum, pRst->nExcelID, pRst->nNum);
			}
		}
		break;
	case CMDID_SELL_ITEM_RPL:
		{
			P_DBS4WEBUSER_SELL_ITEM_T*	pRst	= (P_DBS4WEBUSER_SELL_ITEM_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_SELL_ITEM_T	RplCmd;
			RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			RplCmd.nGameCmd		= STC_GAMECMD_OPERATE_SELL_ITEM;
			RplCmd.nExcelID		= pRst->nExcelID;
			RplCmd.nGold		= pRst->nGold;
			RplCmd.nItemID		= pRst->nItemID;
			RplCmd.nNum			= pRst->nNum;
			RplCmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &RplCmd, sizeof(RplCmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_SELL_ITEM_T::RST_OK)
			{
				PlayerData*	pPlayerData	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayerData != NULL)
				{
					pPlayerData->m_CharAtb.nGold	+= pRst->nGold;
					
					if (pPlayerData->m_CharAtb.nCup != pRst->nCup)
					{
						pPlayerData->m_CharAtb.nCup		= pRst->nCup;
						SendPlayerCharAtbToClient(pPlayerData);
					}
				}
				
				PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
				if (pPlayerCard != NULL)
				{
					pPlayerCard->nCup	= pRst->nCup;
				}
				m_pGameMngS->GetAllianceMng()->SetAllianceMemberCup(pPlayerCard->nAllianceID, pPlayerCard->nAccountID, pPlayerCard->nCup);

				AddGoldProduceLog(pRst->nAccountID, gold_produce_sell_item, pRst->nGold);
				GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1005,ITEM_TRACK)"%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d"
					, item_track_sell, pRst->nAccountID, pRst->nItemID, pRst->nExcelID, pRst->nNum, pRst->nGold);
			}
		}
		break;
	case CMDID_GET_RELATION_LOG_RPL:
		{
			P_DBS4WEBUSER_GET_RELATION_LOG_T*	pRst	= (P_DBS4WEBUSER_GET_RELATION_LOG_T*)pBaseCmd;
			int	nTotalLogSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_RELATION_LOG_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_PRIVATE_LOG_T) + nTotalLogSize);
			STC_GAMECMD_GET_RELATION_LOG_T*	pRplCmd		= (STC_GAMECMD_GET_RELATION_LOG_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd	= STC_GAMECMD_GET_RELATION_LOG;
			pRplCmd->nNum		= pRst->nNum;
			pRplCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nTotalLogSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_FETCH_CHRISTMAS_TREE_RPL:
		{
			P_DBS4WEBUSER_FETCH_CHRISTMAS_TREE_T*	pRst	= (P_DBS4WEBUSER_FETCH_CHRISTMAS_TREE_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T	RplCmd;
			RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			RplCmd.nGameCmd		= STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE;
			RplCmd.nType		= STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T::award_type_none;
			RplCmd.nCrystal		= pRst->nCrystal;
			RplCmd.nExcelID		= pRst->nExcelID;
			RplCmd.nItemID		= pRst->nItemID;
			RplCmd.nRst			= pRst->nRst;
			if (RplCmd.nRst == STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T::RST_OK)
			{
				if (RplCmd.nCrystal == 0)
				{
					RplCmd.nType	= STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T::award_type_item;
				}
				else
				{
					RplCmd.nType	= STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T::award_type_crystal;
					
					PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
					if (pPlayer != NULL)
					{
						pPlayer->m_CharAtb.nCrystal	+= pRst->nCrystal;
					}
				}
			}
			SendCmdToClient(pRst->nExt[0], &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_WATERING_CHRISTMAS_TREE_RPL:
		{
			P_DBS4WEBUSER_WATERING_CHRISTMAS_TREE_T*	pRst	= (P_DBS4WEBUSER_WATERING_CHRISTMAS_TREE_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T	RplCmd;
			RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			RplCmd.nGameCmd		= STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE;
			RplCmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &RplCmd, sizeof(RplCmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T::RST_OK)
			{
				if (pRst->nAccountID != pRst->nWateringAccountID)
				{
					PlayerCard*	pWateringPlayer	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nWateringAccountID);
					if (pWateringPlayer != NULL)
					{
						ExcelText*	pText		= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_rl_watering_christmas_tree);
						if (pText != NULL)
						{
							char	szBuf[excel_text_max_length];
							int		nLength	= 0;

							nLength	= sprintf(szBuf, pText->szText, pWateringPlayer->szName);
							AddMail(pRst->nWateringAccountID, pWateringPlayer->szName, pRst->nAccountID, false, MAIL_TYPE_GOLDORE_EVENT, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
						}
					}
				}

				if (pRst->bReadyRipe)
				{
					STC_GAMECMD_CHRISTMAS_TREE_READY_RIPE_T	Notify;
					Notify.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Notify.nGameCmd	= STC_GAMECMD_CHRISTMAS_TREE_READY_RIPE;
					Notify.nBeginTime	= pRst->nBeginTime;
					Notify.nEndTime	= pRst->nBeginTime + pRst->nRipeTimeInterval;
					SendCmdToClient(pRst->nExt[0], &Notify, sizeof(Notify));
				}
			}
		}
		break;
	case CMDID_STEAL_GOLD_RPL:
		{
			P_DBS4WEBUSER_STEAL_GOLD_T*	pRst	= (P_DBS4WEBUSER_STEAL_GOLD_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_STEAL_GOLD_T	RplCmd;
			RplCmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			RplCmd.nGameCmd		= STC_GAMECMD_OPERATE_STEAL_GOLD;
			RplCmd.nGold		= pRst->nGold;
			RplCmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &RplCmd, sizeof(RplCmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_STEAL_GOLD_T::RST_OK)
			{
				PlayerCard*	pThiefPlayer	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nThiefAccountID);
				if (pThiefPlayer != NULL)
				{
					ExcelText*	pText		= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_rl_steal_gold);
					if (pText != NULL)
					{
						char	szBuf[excel_text_max_length];
						int		nLength	= 0;

						nLength	= sprintf(szBuf, pText->szText, pThiefPlayer->szName, pRst->nGold);
						AddMail(pRst->nThiefAccountID, pThiefPlayer->szName, pRst->nAccountID, false, MAIL_TYPE_GOLDORE_EVENT, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
					}
				}

				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nGold		+= pRst->nGold;
				}

				AddGoldProduceLog(pRst->nThiefAccountID, gold_produce_steal, pRst->nGold);
			}
		}
		break;
	case CMDID_GET_TRAINING_TE_RPL:
		{
			P_DBS4WEBUSER_GET_TRAINING_TE_T*	pRst	= (P_DBS4WEBUSER_GET_TRAINING_TE_T*)pBaseCmd;
			int	nTrainingTESize	= nDSize - sizeof(P_DBS4WEBUSER_GET_TRAINING_TE_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_TRAINING_TE_T) + nTrainingTESize);
			STC_GAMECMD_GET_TRAINING_TE_T*	pCmd		= (STC_GAMECMD_GET_TRAINING_TE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_TRAINING_TE;
			pCmd->nNum		= pRst->nNum;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), nTrainingTESize);
			g_pLPNet->SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ADD_TRAINING_RPL:
		{
			P_DBS4WEBUSER_ADD_TRAINING_T*	pRst	= (P_DBS4WEBUSER_ADD_TRAINING_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ADD_TRAINING_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ADD_TRAINING;
			Cmd.nRst		= pRst->nRst;
			g_pLPNet->SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (Cmd.nRst == STC_GAMECMD_OPERATE_ADD_TRAINING_T::RST_OK)
			{
				AddGoldCostLog(pRst->nAccountID, gold_cost_hero_train, pRst->nGold);

				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					if (pPlayer->m_CharAtb.nGold >= pRst->nGold)
					{
						pPlayer->m_CharAtb.nGold	-= pRst->nGold;
					}
					else
					{
						pPlayer->m_CharAtb.nGold	= 0;
					}
				}
			}
		}
		break;
	case CMDID_EXIT_TRAINING_RPL:
		{
			P_DBS4WEBUSER_EXIT_TRAINING_T*	pRst	= (P_DBS4WEBUSER_EXIT_TRAINING_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_EXIT_TRAINING_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_EXIT_TRAINING;
			memcpy(&Cmd.hero, &pRst->hero, sizeof(Cmd.hero));
			Cmd.nRst		= pRst->nRst;
			g_pLPNet->SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_DEAL_TRAINING_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_TRAINING_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_TRAINING_TE_T*)pBaseCmd;
			PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
			if (pPlayer != NULL)
			{
				STC_GAMECMD_TRAINING_OVER_T	Cmd;
				Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				Cmd.nGameCmd	= STC_GAMECMD_TRAINING_OVER;
				memcpy(&Cmd.hero, &pRst->hero, sizeof(Cmd.hero));
				SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_CONTRIBUTE_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_CONTRIBUTE_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_CONTRIBUTE_ALLIANCE_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE;
			Cmd.nDevelopment	= pRst->nContributeNum;
			Cmd.nGold			= pRst->nGold;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T::RST_OK)
			{
				PlayerData*	pPlayerData	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayerData != NULL)
				{
					if (pPlayerData->m_CharAtb.nGold > pRst->nGold)
					{
						pPlayerData->m_CharAtb.nGold	-= pRst->nGold;
					}
					else
					{
						pPlayerData->m_CharAtb.nGold	= 0;
					}
				}

				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
				}
				else
				{
					pAlliance->basicInfo.nDevelopment		+= pRst->nContributeNum;
					pAlliance->basicInfo.nTotalDevelopment	+= pRst->nContributeNum;
					pAlliance->SetLevel(pRst->nCongressLevel);

					int		nTextLen		= 0;
					char	szBuf[excel_text_max_length];
					bool	bSendMail		= false;
					ExcelText*	pExcelText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_alliance_contribute);
					if (pExcelText != NULL)
					{
						bSendMail			= true;
						const char*	pCharName	= "";
						if (pPlayerData != NULL)
						{
							pCharName		= pPlayerData->m_CharAtb.szName;
						}
						else
						{
							PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
							if (pPlayerCard != NULL)
							{
								pCharName	= pPlayerCard->szName;
							}
						}
						nTextLen			= sprintf(szBuf, pExcelText->szText, pCharName, pRst->nGold, pRst->nContributeNum);
					}

					for (unsigned int i=0; i<pAlliance->members.size(); i++)
					{
						AllianceMemberUnit*	pMember	= &pAlliance->members[i];
						if (pMember->nAccountID == pRst->nAccountID)
						{
							pMember->nDevelopment			+= pRst->nContributeNum;
							pMember->nTotalDevelopment		+= pRst->nContributeNum;
						}
						if (bSendMail)
						{
							AddMail(0, "", pMember->nAccountID, false, MAIL_TYPE_ALLIANCE_EVENT, MAIL_FLAG_NORMAL, false, nTextLen, szBuf, 0, NULL, NULL);
						}
					}
				}
			}
		}
		break;
	case CMDID_CONVERT_DIAMOND_GOLD_RPL:
		{
			P_DBS4WEBUSER_CONVERT_DIAMOND_GOLD_T*	pRst	= (P_DBS4WEBUSER_CONVERT_DIAMOND_GOLD_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD;
			Cmd.nDiamond		= pRst->nDiamond;
			Cmd.nGold			= pRst->nGold;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (Cmd.nRst == STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T::RST_OK)
			{
				AddMoneyCostLog(pRst->nAccountID, money_use_type_store_buy_gold, money_type_diamond, pRst->nDiamond, pRst->nGold);

				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nGold	+= pRst->nGold;
					if (pPlayer->m_CharAtb.nDiamond >= pRst->nDiamond)
					{
						pPlayer->m_CharAtb.nDiamond	-= pRst->nDiamond;
					}
					else
					{
						pPlayer->m_CharAtb.nDiamond	= 0;
					}
				}
			}
		}
		break;
	case CMDID_JOIN_ALLIANCE_NAME_RPL:
		{
			P_DBS4WEBUSER_JOIN_ALLIANCE_NAME_T*	pRst	= (P_DBS4WEBUSER_JOIN_ALLIANCE_NAME_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_JOIN_ALLI_NAME_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_JOIN_ALLI_NAME;
			Cmd.nAllianceID		= pRst->nAllianceID;
			WH_STRNCPY0(Cmd.szAllianceName, pRst->szAllianceName);
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (Cmd.nRst == STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance == NULL)
				{
					m_pGameMngS->LoadAllianceFromDB(pRst->nAllianceID);
				}
				else
				{
					AllianceJoinEvent*	pEvent	= pAlliance->joinEvents.push_back();
					pEvent->nAccountID			= pRst->nAccountID;
					pEvent->szName[0]			= 0;
					pEvent->nLevel				= 0;
					pEvent->nHeadID				= 0;
					PlayerCard*	pPlayerCard		= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
					if (pPlayerCard != NULL)
					{
						WH_STRNCPY0(pEvent->szName, pPlayerCard->szName);
						pEvent->nLevel			= pPlayerCard->nLevel;
						pEvent->nHeadID			= pPlayerCard->nHeadID;
					}

					// 给管理员通知
					for (int i=0; i<pAlliance->members.size(); i++)
					{
						AllianceMemberUnit& member	= pAlliance->members[i];
						if (member.nPosition != alliance_position_member)
						{
							PlayerData*	pManager	= GetPlayerData(member.nAccountID);
							if (pManager != NULL)
							{
								TTY_LPGAMEPLAY_CLIENT_GAME_CMD_T	Cmd;
								Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
								Cmd.nGameCmd	= STC_GAMECMD_NEW_ALLIANCE_JOIN;
								g_pLPNet->SendCmdToClient(pManager->m_nPlayerGID, &Cmd, sizeof(Cmd));
							}
						}
					}
				}
			}
		}
		break;
	case CMDID_GET_MY_ALLIANCE_JOIN_RPL:
		{
			P_DBS4WEBUSER_GET_MY_ALLIANCE_JOIN_T*	pRst	= (P_DBS4WEBUSER_GET_MY_ALLIANCE_JOIN_T*)pBaseCmd;
			STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT;
			Cmd.nAllianceID		= pRst->nAllianceID;
			WH_STRNCPY0(Cmd.szAllianceName, pRst->szAllianceName);
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_SET_ALLI_INTRO_RPL:
		{
			P_DBS4WEBUSER_SET_ALLI_INTRO_T*	pRst	= (P_DBS4WEBUSER_SET_ALLI_INTRO_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_SET_ALLI_INTRO;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T::RST_OK)
			{
				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance != NULL)
				{
					WH_STRNCPY0(pAlliance->basicInfo.szIntroduction, pRst->szIntroduction);
				}
			}
		}
		break;
		/*
	case CMDID_GET_OTHER_GOLDORE_POSITION_RPL:
		{
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_POSITION_T*	pRst	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_POSITION_T*)pBaseCmd;
			int	nExtDataSize		= nDSize - sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_POSITION_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T) + nExtDataSize);
			STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T*	pRplCmd	= (STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd		= STC_GAMECMD_GET_OTHER_GOLDORE_POSITION;
			pRplCmd->nAccountID		= pRst->nAccountID;
			pRplCmd->nNum			= pRst->nNum;
			pRplCmd->nRst			= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nExtDataSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_OTHER_GOLDORE_PRODUCTION_EVENT_RPL:
		{
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*	pRst	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*)pBaseCmd;
			int	nExtDataSize		= nDSize - sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T) + nExtDataSize);
			STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*	pRplCmd	= (STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd		= STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT;
			pRplCmd->nAccountID		= pRst->nAccountID;
			pRplCmd->nNum			= pRst->nNum;
			pRplCmd->nRst			= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nExtDataSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_OTHER_GOLDORE_PRODUCTION_TE_RPL:
		{
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_TE_T*	pRst	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_TE_T*)pBaseCmd;
			int	nExtDataSize		= nDSize - sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_TE_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T) + nExtDataSize);
			STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T*	pRplCmd	= (STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd		= STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE;
			pRplCmd->nAccountID		= pRst->nAccountID;
			pRplCmd->nNum			= pRst->nNum;
			pRplCmd->nRst			= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nExtDataSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_OTHER_GOLDORE_FETCH_INFO_RPL:
		{
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_FETCH_INFO_T*	pRst	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_FETCH_INFO_T*)pBaseCmd;
			int	nExtDataSize		= nDSize - sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_FETCH_INFO_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T) + nExtDataSize);
			STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T*	pRplCmd	= (STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd		= STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO;
			pRplCmd->nAccountID		= pRst->nAccountID;
			pRplCmd->nNum			= pRst->nNum;
			pRplCmd->nRst			= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nExtDataSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_OTHER_CHRISTMAS_TREE_INFO_RPL:
		{
			P_DBS4WEBUSER_GET_OTHER_CHRISTMAS_TREE_INFO_T*	pRst	= (P_DBS4WEBUSER_GET_OTHER_CHRISTMAS_TREE_INFO_T*)pBaseCmd;
			STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T	Cmd;
			Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd			= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO;
			Cmd.nAccountID			= pRst->nAccountID;
			Cmd.bICanWater			= pRst->bICanWater;
			Cmd.nCountDownTime		= pRst->nCountDownTime;
			Cmd.nState				= pRst->nState;
			Cmd.nWateredNum			= pRst->nWateredNum;
			Cmd.nRst				= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
		*/
	case CMDID_CHECK_DRAW_LOTTERY_RPL:
		{
			P_DBS4WEBUSER_CHECK_DRAW_LOTTERY_T*	pRst	= (P_DBS4WEBUSER_CHECK_DRAW_LOTTERY_T*)pBaseCmd;
			if (pRst->nRst == STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T::RST_OK)
			{
				if (pRst->nType==money_type_crystal || pRst->nType==money_type_diamond)
				{
					AddMoneyCostLog(pRst->nAccountID, money_use_type_lottery, pRst->nType, pRst->nPrice);
				}
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					switch (pRst->nType)
					{
					case money_type_none:
						{
							if (pPlayer->m_CharAtb.nFreeDrawLotteryNum > 0)
							{
								pPlayer->m_CharAtb.nFreeDrawLotteryNum--;
							}
						}
						break;
					case money_type_diamond:
						{
							if (pPlayer->m_CharAtb.nDiamond >= pRst->nPrice)
							{
								pPlayer->m_CharAtb.nDiamond	-= pRst->nPrice;
							}
							else
							{
								pPlayer->m_CharAtb.nDiamond	= 0;
							}
						}
						break;
					case money_type_crystal:
						{
							if (pPlayer->m_CharAtb.nCrystal >= pRst->nPrice)
							{
								pPlayer->m_CharAtb.nCrystal	-= pRst->nPrice;
							}
							else
							{
								pPlayer->m_CharAtb.nCrystal	= 0;
							}
						}
						break;
					default:
						{

						}
						break;
					}

					// 抽一次奖
					switch (pRst->nType)
					{
					case money_type_none:
					case money_type_diamond:
					case money_type_crystal:
						{
							m_pGameMngS->GetLotteryMng()->DrawOnceLottery();
						}
						break;
					case money_type_alliance_contribute:
						{
							m_pGameMngS->GetLotteryMng()->DrawOnceLotteryAlliance();
						}
						break;
					}
					
					m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_DRAW_LOTTERY_T) + lottery_probability_max_choice*sizeof(LotteryEntry));
					STC_GAMECMD_OPERATE_DRAW_LOTTERY_T*	pRplCmd	= (STC_GAMECMD_OPERATE_DRAW_LOTTERY_T*)m_vectrawbuf.getbuf();
					pRplCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					pRplCmd->nGameCmd			= STC_GAMECMD_OPERATE_DRAW_LOTTERY;
					pRplCmd->nRst				= STC_GAMECMD_OPERATE_DRAW_LOTTERY_T::RST_OK;
					pRplCmd->nType				= pRst->nType;
					pRplCmd->nPrice				= pRst->nPrice;
					pRplCmd->nChoice			= m_pGameMngS->GetLotteryMng()->GetCurChoice();
					pRplCmd->nNum				= lottery_probability_max_choice;
					const whvector<LotteryEntry>&	vectLottery	= m_pGameMngS->GetLotteryMng()->GetCurLotteryEntries();
					memcpy(wh_getptrnexttoptr(pRplCmd), vectLottery.getbuf(), lottery_probability_max_choice*sizeof(LotteryEntry));

					{
						// 记录下来抽奖结果
						pPlayer->m_lottery.nType	= vectLottery[pRplCmd->nChoice].nType;
						pPlayer->m_lottery.nData	= vectLottery[pRplCmd->nChoice].nData;
						pPlayer->m_lottery.bTopReward	= vectLottery[pRplCmd->nChoice].bTopReward;

						// 在数据库中增加抽奖日志
						P_DBS4WEB_ADD_LOTTERY_LOG_T	DBCmd;
						DBCmd.nCmd					= P_DBS4WEB_REQ_CMD;
						DBCmd.nSubCmd				= CMDID_ADD_LOTTERY_LOG_REQ;
						DBCmd.nAccountID			= pPlayer->m_CharAtb.nAccountID;
						DBCmd.nType					= pPlayer->m_lottery.nType;
						DBCmd.nData					= pPlayer->m_lottery.nData;
						DBCmd.nTime					= wh_time();
						SendCmdToDB(&DBCmd, sizeof(DBCmd));
					}

					SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

					// 1->CHECK,账号ID,费用类型,费用额度,奖励类型,奖励数据,奖励选项
					GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1004,LOTTERY)"1,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,0"
						, pRst->nAccountID, pRst->nType, pRst->nPrice, pPlayer->m_lottery.nType, pPlayer->m_lottery.nData, pRplCmd->nChoice);
				}
				if (pRst->nType == money_type_alliance_contribute)
				{
					// 这个和其他不一样
					AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
					if (pAlliance != NULL)
					{
						for (int i=0; i<pAlliance->members.size(); i++)
						{
							if (pAlliance->members[i].nAccountID == pRst->nAccountID)
							{
								if (pAlliance->members[i].nDevelopment > pRst->nPrice)
								{
									pAlliance->members[i].nDevelopment	-= pRst->nPrice;
								}
								else
								{
									pAlliance->members[i].nDevelopment	= 0;
								}
								break;
							}
						}
					}

					if (pPlayer != NULL)
					{
						if (pPlayer->m_CharAtb.nAllianceDrawLotteryNum>0)
						{
							pPlayer->m_CharAtb.nAllianceDrawLotteryNum--;
						}
					}
				}
			}
			else
			{
				STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T	Cmd;
				Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				Cmd.nGameCmd			= STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY;
				Cmd.nPrice				= pRst->nPrice;
				Cmd.nType				= pRst->nType;
				Cmd.nRst				= pRst->nRst;
				SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_FETCH_LOTTERY_RPL:
		{
			P_DBS4WEBUSER_FETCH_LOTTERY_T*	pRst	= (P_DBS4WEBUSER_FETCH_LOTTERY_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_FETCH_LOTTERY_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_FETCH_LOTTERY;
			Cmd.nType			= pRst->nType;
			Cmd.nData			= pRst->nData;
			Cmd.nItemID			= pRst->nItemID;
			Cmd.nMoneyType		= pRst->nExt[1];
			Cmd.nRst			= pRst->nRst;
			g_pLPNet->SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
			if (pRst->nRst == STC_GAMECMD_OPERATE_FETCH_LOTTERY_T::RST_OK)
			{
				if (pRst->nType == lottery_type_gold)
				{
					AddGoldProduceLog(pRst->nAccountID, gold_produce_lottery, pRst->nData);
				}

				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					switch (pRst->nType)
					{
					case lottery_type_item:
						{

						}
						break;
					case lottery_type_crystal:
						{
							pPlayer->m_CharAtb.nCrystal	+= pRst->nData;
						}
						break;
					case lottery_type_gold:
						{
							pPlayer->m_CharAtb.nGold	+= pRst->nData;
						}
						break;
					}
					pPlayer->m_lottery.nType	= lottery_type_none;
					pPlayer->m_lottery.nData	= 0;
					pPlayer->m_lottery.bTopReward	= false;
				}

				// 2->FETCH,账号ID,费用类型,费用额度,奖励类型,奖励数据,奖励选项
				GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1004,LOTTERY)"2,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d"
					, pRst->nAccountID, pRst->nExt[1], 0, pRst->nType, pRst->nData, 0, pRst->nValue);

				// 高级奖励需要在世界聊天频道广播
				if (pRst->bNotifyWorld)
				{
					// 高级奖励需要在世界频道广播
					const char*	cszCharName	= "";
					tty_id_t	nCharID		= 0;
					if (pPlayer != NULL)
					{
						cszCharName			= pPlayer->m_CharAtb.szName;
						nCharID				= pPlayer->m_CharAtb.nAccountID;
					}
					else
					{
						PlayerCard*	pCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
						if (pCard != NULL)
						{
							cszCharName		= pCard->szName;
							nCharID			= pCard->nAccountID;
						}
					}

					char	szBuf[1024]		= "";
					int		nLength			= 0;
					switch (pRst->nType)
					{
					case lottery_type_item:
						{
							ExcelText*	pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_lottery_item);
							if (pText != NULL)
							{
								nLength			= sprintf(szBuf, pText->szText, cszCharName, pRst->szItemName);
							}
						}
						break;
					case lottery_type_crystal:
						{
							ExcelText*	pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_lottery_crystal);
							if (pText != NULL)
							{
								nLength			= sprintf(szBuf, pText->szText, cszCharName, pRst->nData);
							}
						}
						break;
					case lottery_type_gold:
						{
							ExcelText*	pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_lottery_gold);
							if (pText != NULL)
							{
								nLength			= sprintf(szBuf, pText->szText, cszCharName, pRst->nData);
							}
						}
						break;
					}
					if (nLength > 0)
					{
						AddNotifyInWorldChat(nLength, szBuf, nCharID);
					}
				}
			}
		}
		break;
	case CMDID_GET_CHRISTMAS_TREE_RPL:
		{
			P_DBS4WEBUSER_GET_CHRISTMAS_TREE_T*	pRst	= (P_DBS4WEBUSER_GET_CHRISTMAS_TREE_T*)pBaseCmd;
			STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO;
			Cmd.bICanWater		= pRst->bICanWater;
			Cmd.nCanWaterCountDownTime	= pRst->nCanWaterCountDownTime;
			Cmd.nCountDownTime	= pRst->nCountDownTime;
			Cmd.nState			= pRst->nState;
			Cmd.nWateredNum		= pRst->nWateredNum;
			Cmd.nRst			= pRst->nRst;
			g_pLPNet->SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_GET_OTHER_GOLDORE_INFO_RPL:
		{
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_INFO_T*	pRst	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_INFO_T*)pBaseCmd;
			unsigned int	nGoldoreInfoSize				= nDSize - sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_INFO_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T) + nGoldoreInfoSize);
			STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T*	pCmd	= (STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_OTHER_GOLDORE_INFO;
			pCmd->nAccountID= pRst->nAccountID;
			pCmd->nCharHeadID	= 0;
			pCmd->nCharLevel	= 0;
			pCmd->nNum		= pRst->nNum;
			pCmd->nRst		= pRst->nRst;
			pCmd->christmasTreeInfo.bICanWater		= pRst->bCTICanWater;
			pCmd->christmasTreeInfo.nCountDownTime	= pRst->nCTCountDownTime;
			pCmd->christmasTreeInfo.nState			= pRst->nCTState;
			pCmd->christmasTreeInfo.nWateredNum		= pRst->nCTWateredNum;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), nGoldoreInfoSize);

			PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
			if (pPlayerCard != NULL)
			{
				pCmd->nCharHeadID	= pPlayerCard->nHeadID;
				pCmd->nCharLevel	= pPlayerCard->nLevel;
			}
			g_pLPNet->SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_GOLDORE_SMP_INFO_ALL_RPL:
		{
			P_DBS4WEBUSER_GET_GOLDORE_SMP_INFO_ALL_T*	pRst	= (P_DBS4WEBUSER_GET_GOLDORE_SMP_INFO_ALL_T*)pBaseCmd;
			unsigned int	nSmpInfoSize			= nDSize - sizeof(P_DBS4WEBUSER_GET_GOLDORE_SMP_INFO_ALL_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T) + nSmpInfoSize);
			STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T*	pCmd		= (STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd			= STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL;
			pCmd->nNum				= pRst->nNum;
			pCmd->nRst				= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), nSmpInfoSize);
			g_pLPNet->SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_ALLIANCE_TRADE_INFO_RPL:
		{
			P_DBS4WEBUSER_GET_ALLIANCE_TRADE_INFO_T*	pRst	= (P_DBS4WEBUSER_GET_ALLIANCE_TRADE_INFO_T*)pBaseCmd;
			STC_GAMECMD_GET_ALLIANCE_TRADE_INFO_T	Cmd;
			Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd			= STC_GAMECMD_GET_ALLIANCE_TRADE_INFO;
			Cmd.nCountDownTime		= pRst->nCountDownTime;
			Cmd.nGold				= pRst->nGold;
			Cmd.nTradeNum			= pRst->nTradeNum;
			Cmd.nExcelTradeGold		= pRst->nExcelTradeGold;
			Cmd.nExcelTradeNum		= pRst->nExcelTradeNum;
			Cmd.nExcelTradeTime		= pRst->nExcelTradeTime;
			Cmd.nRst				= pRst->nRst;
			g_pLPNet->SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLIANCE_TRADE_RPL:
		{
			P_DBS4WEBUSER_ALLIANCE_TRADE_T*	pRst	= (P_DBS4WEBUSER_ALLIANCE_TRADE_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ALLIANCE_TRADE_T	Cmd;
			Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd			= STC_GAMECMD_OPERATE_ALLIANCE_TRADE;
			Cmd.nCountDownTime		= pRst->nCountDownTime;
			Cmd.nGold				= pRst->nGold;
			Cmd.nRst				= pRst->nRst;
			g_pLPNet->SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_ALLIANCE_TRADE_T::RST_OK)
			{
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					if (pPlayer->m_CharAtb.nTradeNum > 0)
					{
						pPlayer->m_CharAtb.nTradeNum--;
					}
				}
			}
		}
		break;
	case CMDID_DEAL_ALLIANCE_TREADE_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_ALLIANCE_TREADE_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_ALLIANCE_TREADE_TE_T*)pBaseCmd;
			PlayerData*	pPlayer		= GetPlayerData(pRst->nAccountID);
			if (pPlayer != NULL)
			{
				pPlayer->m_CharAtb.nGold	+= pRst->nGold;
				// 推送一份新的属性数据 
				g_pLPNet->SendPlayerCharAtbToClient(pPlayer);
			}
			// 添加一封新邮件
			ExcelText*	pText		= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_alliance_trade_end);
			if (pText != NULL)
			{
				char	szBuf[excel_text_max_length];
				int		nLength		= sprintf(szBuf, pText->szText, pRst->nGold);
				g_pLPNet->AddMail(0, "", pRst->nAccountID, false, MAIL_TYPE_SYSTEM, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
			}
		}
		break;
	case CMDID_GET_ENEMY_LIST_RPL:
		{
			P_DBS4WEBUSER_GET_ENEMY_LIST_T*	pRst	= (P_DBS4WEBUSER_GET_ENEMY_LIST_T*)pBaseCmd;
			int	nUnitSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_ENEMY_LIST_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ENEMY_LIST_T) + nUnitSize);
			STC_GAMECMD_GET_ENEMY_LIST_T*	pCmd	= (STC_GAMECMD_GET_ENEMY_LIST_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_ENEMY_LIST;
			pCmd->nNum		= pRst->nNum;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), nUnitSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_DELETE_ENEMY_RPL:
		{
			P_DBS4WEBUSER_DELETE_ENEMY_T*	pRst	= (P_DBS4WEBUSER_DELETE_ENEMY_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_DELETE_ENEMY_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_DELETE_ENEMY;
			Cmd.nEnemyID	= pRst->nEnemyID;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ADD_ENEMY_RPL:
		{
			P_DBS4WEBUSER_ADD_ENEMY_T*	pRst	= (P_DBS4WEBUSER_ADD_ENEMY_T*)pBaseCmd;
			if (pRst->nRst == 0)
			{
				PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					STC_GAMECMD_NEW_ENEMY_T	Cmd;
					Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd	= STC_GAMECMD_NEW_ENEMY;
					PlayerCard*	pCard		= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nEnemyID);
					if (pCard != NULL)
					{
						Cmd.enemy.nAccountID	= pCard->nAccountID;
						Cmd.enemy.nHeadID		= pCard->nHeadID;
						Cmd.enemy.nLevel		= pCard->nLevel;
						WH_STRNCPY0(Cmd.enemy.szName, pCard->szName);
						SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
				}

				pPlayer				= GetPlayerData(pRst->nEnemyID);
				if (pPlayer != NULL)
				{
					STC_GAMECMD_NEW_ENEMY_T	Cmd;
					Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd	= STC_GAMECMD_NEW_ENEMY;
					PlayerCard*	pCard		= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
					if (pCard != NULL)
					{
						Cmd.enemy.nAccountID	= pCard->nAccountID;
						Cmd.enemy.nHeadID		= pCard->nHeadID;
						Cmd.enemy.nLevel		= pCard->nLevel;
						WH_STRNCPY0(Cmd.enemy.szName, pCard->szName);
						SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
					}
				}
			}
		}
		break;
	case CMDID_REFRESH_CHAR_ATB_RPL:
		{
			P_DBS4WEBUSER_REFRESH_CHAR_ATB_T*	pRst	= (P_DBS4WEBUSER_REFRESH_CHAR_ATB_T*)pBaseCmd;
			CharAtb*	pCharAtb	= (CharAtb*)wh_getptrnexttoptr(pRst);
			PlayerData*	pPlayer		= GetPlayerData(pCharAtb->nAccountID);
			if (pPlayer != NULL)
			{
				memcpy(&pPlayer->m_CharAtb, pCharAtb, sizeof(CharAtb));
				SendPlayerCharAtbToClient(pPlayer, pRst->bSimple);
			}

			PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pCharAtb->nAccountID);
			if (pPlayerCard != NULL)
			{
				pPlayerCard->nLevel			= pCharAtb->nLevel;
				pPlayerCard->nDevelopment	= pCharAtb->nDevelopment;
				pPlayerCard->nVip			= pCharAtb->nVip;
				pPlayerCard->nProtectTimeEndTick	= pCharAtb->nProtectTimeEndTick;
				pPlayerCard->nInstanceWangzhe		= pCharAtb->nInstanceWangzhe;
				pPlayerCard->nInstanceZhengzhan		= pCharAtb->nInstanceZhengzhan;
				pPlayerCard->nCup			= pCharAtb->nCup;
				pPlayerCard->bVipDisplay	= pCharAtb->bVipDisplay;
				// 其他的玩家登陆的时候再刷新

				m_pGameMngS->GetAllianceMng()->SetAllianceMemberCup(pPlayerCard->nAllianceID, pPlayerCard->nAccountID, pPlayerCard->nCup);
			}
		}
		break;
	case CMDID_ADD_ALLIANCE_DEV_RPL:
		{
			P_DBS4WEBUSER_ADD_ALLIANCE_DEV_T*	pRst	= (P_DBS4WEBUSER_ADD_ALLIANCE_DEV_T*)pBaseCmd;
			AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
			if (pAlliance != NULL)
			{
				if (pRst->nDeltaDevelopment >= 0)
				{
					pAlliance->basicInfo.nDevelopment		+= pRst->nDeltaDevelopment;
					pAlliance->basicInfo.nTotalDevelopment	+= pRst->nDeltaDevelopment;
				}
				else
				{
					unsigned int	nAbsDeltaDev	= -pRst->nDeltaDevelopment;
					if (pAlliance->basicInfo.nDevelopment > nAbsDeltaDev)
					{
						pAlliance->basicInfo.nDevelopment	-= nAbsDeltaDev;
					}
					else
					{
						pAlliance->basicInfo.nDevelopment	= 0;
					}
					if (pAlliance->basicInfo.nTotalDevelopment > nAbsDeltaDev)
					{
						pAlliance->basicInfo.nTotalDevelopment	-= nAbsDeltaDev;
					}
					else
					{
						pAlliance->basicInfo.nTotalDevelopment	= 0;
					}
				}
			}
		}
		break;
	case CMDID_LOAD_RANK_LIST_RPL:
		{
			m_bAlreadyLoadRank						= true;
			P_DBS4WEBUSER_LOAD_RANK_LIST_T*	pRst	= (P_DBS4WEBUSER_LOAD_RANK_LIST_T*)pBaseCmd;
			m_pGameMngS->GetRankMng()->RefreshRank(pRst->nType, pRst->nNum, wh_getptrnexttoptr(pRst));
			if (pRst->nType == rank_type_char_level)
			{
				m_bLoadCharLevelRankOK				= true;
			}
		}
		break;
	case CMDID_RECHARGE_DIAMOND_RPL:
		{
			P_DBS4WEBUSER_RECHARGE_DIAMOND_T*	pRst	= (P_DBS4WEBUSER_RECHARGE_DIAMOND_T*)pBaseCmd;
			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1000,RECHARGE_DIAMOND)"%d,0x%"WHINT64PRFX"X,%d,%d,%d"
				, pRst->nRst, pRst->nAccountID, pRst->nAddedDiamond, pRst->nVip, (int)pRst->bFromGS);
			if (pRst->nRst == P_DBS4WEBUSER_RECHARGE_DIAMOND_T::RST_OK)
			{
				PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nDiamond	+= (pRst->nAddedDiamond + pRst->nGiftDiamond);
					pPlayer->m_CharAtb.nTotalDiamond	+= pRst->nAddedDiamond;
					pPlayer->m_CharAtb.nVip		= pRst->nVip;

					STC_GAMECMD_RECHARGE_DIAMOND_T	Cmd;
					Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd		= STC_GAMECMD_RECHARGE_DIAMOND;
					Cmd.nAddedDiamond	= pRst->nAddedDiamond;
					Cmd.nGiftDiamond	= pRst->nGiftDiamond;
					Cmd.nVip			= pRst->nVip;
					SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
				}
				PlayerCard*	pCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
				if (pCard != NULL)
				{
					pCard->nVip		= pRst->nVip;
				}
			}

			// 如果来自全局服,则返回
			if (pRst->bFromGS)
			{
				GS4WEB_GROUP_RECHARGE_DIAMOND_RPL_T	Cmd;
				Cmd.nCmd		= P_GS4WEB_GROUP_RPL_CMD;
				Cmd.nSubCmd		= GROUP_RECHARGE_DIAMOND_RPL;
				Cmd.nRst		= pRst->nRst;
				Cmd.nAccountID	= pRst->nAccountID;
				Cmd.nAddedDiamond	= pRst->nAddedDiamond;
				Cmd.nVip		= pRst->nVip;
				memcpy(Cmd.nExt, pRst->nExt, sizeof(Cmd.nExt));
				m_msgerGS4Web.SendMsg(&Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_WRITE_CHAR_SIGNATURE_RPL:
		{
			P_DBS4WEBUSER_WRITE_CHAR_SIGNATURE_T*	pRst	= (P_DBS4WEBUSER_WRITE_CHAR_SIGNATURE_T*)pBaseCmd;
			// 1.发送返回指令
			{
				STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T	Cmd;
				Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				Cmd.nGameCmd		= STC_GAMECMD_OPERATE_WRITE_SIGNATURE;
				Cmd.nRst			= pRst->nRst;
				g_pLPNet->SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
			}
			// 2.修改PlayerData,PlayerCard
			if (pRst->nRst == STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T::RST_OK)
			{
				PlayerData*	pPlayerData	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayerData != NULL)
				{
					WH_STRNCPY0(pPlayerData->m_CharAtb.szSignature, pRst->szSignature);
				}

				PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
				if (pPlayerCard != NULL)
				{
					WH_STRNCPY0(pPlayerCard->szSignature, pRst->szSignature);
				}
			}
		}
		break;
	case CMDID_GET_INSTANCE_ID_RPL:
		{
			P_DBS4WEBUSER_GET_INSTANCE_ID_T*	pRst	= (P_DBS4WEBUSER_GET_INSTANCE_ID_T*)pBaseCmd;
			if (pRst->nInstanceID != 0)
			{
				PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					pPlayer->m_nGroupID		= pRst->nInstanceID;
					SetPlayerTag64(pPlayer->m_nPlayerGID, pRst->nInstanceID, false);
				}
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_INSTANCE_ID_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_INSTANCE_ID_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_INSTANCE_ID_T*)pBaseCmd;
			if (pRst->nNum != 0)
			{
				tty_id_t*	pInstanceID	= (tty_id_t*)wh_getptrnexttoptr(pRst);
				PlayerData*	pPlayer		= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					for (int i=0; i<pRst->nNum; i++)
					{
						m_pGameMngS->GetInstanceMng()->RefreshInstance(*pInstanceID, pRst->nAccountID);

						pInstanceID++;
					}
				}
			}
		}
		break;
	case CMDID_CHANGE_NAME_RPL:
		{
			P_DBS4WEBUSER_CHANGE_NAME_T*	pRst	= (P_DBS4WEBUSER_CHANGE_NAME_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CHANGE_NAME_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_CHANGE_NAME;
			Cmd.nRst		= pRst->nRst;
			Cmd.nDiamond	= pRst->nCostDiamond;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_CHANGE_NAME_T::RST_OK)
			{
				m_pGameMngS->GetPlayerCardMng()->ChangeName(pRst->szOldName, pRst->szNewName);

				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					WH_STRNCPY0(pPlayer->m_CharAtb.szName, pRst->szNewName);
					if (pPlayer->m_CharAtb.nDiamond > pRst->nCostDiamond)
					{
						pPlayer->m_CharAtb.nDiamond	-= pRst->nCostDiamond;
					}
					else
					{
						pPlayer->m_CharAtb.nDiamond	= 0;
					}
					pPlayer->m_CharAtb.nChangeNameNum++;
				}

				AddMoneyCostLog(pRst->nAccountID, money_use_type_change_char_name, money_type_diamond, pRst->nCostDiamond);
			}
		}
		break;
	case CMDID_MOVE_POSITION_RPL:
		{
			P_DBS4WEBUSER_MOVE_POSITION_T*	pRst	= (P_DBS4WEBUSER_MOVE_POSITION_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_MOVE_CITY_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_MOVE_CITY;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_MOVE_CITY_T::RST_OK)
			{
				// 位置变更(里面会unbook)
				m_pGameMngS->GetWorldMng()->MovePosition(pRst->nSrcPosX, pRst->nSrcPosY, pRst->nDstPosX, pRst->nDstPosY);

				// 修改卡片
				PlayerCard*	pCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
				if (pCard != NULL)
				{
					pCard->nPosX	= pRst->nDstPosX;
					pCard->nPosY	= pRst->nDstPosY;
				}

				// 修改CharAtb
				PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nPosX	= pRst->nDstPosX;
					pPlayer->m_CharAtb.nPosY	= pRst->nDstPosY;
				}
			}
			else
			{
				// 解除book状态
				m_pGameMngS->GetWorldMng()->UnbookPosition(pRst->nDstPosX, pRst->nDstPosY);
			}
		}
		break;
	case CMDID_GET_PAY_SERIAL_RPL:
		{
			P_DBS4WEBUSER_GET_PAY_SERIAL_T*	pRst	= (P_DBS4WEBUSER_GET_PAY_SERIAL_T*)pBaseCmd;
			STC_GAMECMD_GET_PAY_SERIAL_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_GET_PAY_SERIAL;
			Cmd.nRst		= pRst->nRst;
			memcpy(Cmd.szPaySerial, pRst->szPaySerial, sizeof(Cmd.szPaySerial));
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_GET_PAY_LIST_RPL:
		{
			P_DBS4WEBUSER_GET_PAY_LIST_T*	pRst	= (P_DBS4WEBUSER_GET_PAY_LIST_T*)pBaseCmd;
			char*	pSerial	= (char*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				P_DBS4WEB_DEAL_ONE_PAY_T	Cmd;
				Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd		= CMDID_DEAL_ONE_PAY_REQ;
				memcpy(Cmd.szPaySerial, pSerial, TTY_PAY_SERIAL_LEN);
				Cmd.szPaySerial[TTY_PAY_SERIAL_LEN]	= 0;
				SendCmdToDB(&Cmd, sizeof(Cmd));

				pSerial			= (char*)wh_getoffsetaddr(pSerial, TTY_PAY_SERIAL_LEN);
			}
		}
		break;
	case CMDID_DEAL_ONE_PAY_RPL:
		{
			// 实际上没有这个返回,统一走RechargeDiamond的返回
		}
		break;
	case CMDID_READ_NOTIFICATION_RPL:
		{
			P_DBS4WEBUSER_READ_NOTIFICATION_T*	pRst	= (P_DBS4WEBUSER_READ_NOTIFICATION_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_READ_NOTIFICATION_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_READ_NOTIFICATION;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_READ_NOTIFICATION_T::RST_OK)
			{
				PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nNotificationID	= pRst->nNotificationID;
				}
			}
		}
		break;
	case CMDID_CHANGE_HERO_NAME_RPL:
		{
			P_DBS4WEBUSER_CHANGE_HERO_NAME_T*	pRst	= (P_DBS4WEBUSER_CHANGE_HERO_NAME_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_CHANGE_HERO_NAME;
			Cmd.nRst			= pRst->nRst;
			Cmd.nDiamond		= pRst->nDiamond;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T::RST_OK)
			{
				AddMoneyCostLog(pRst->nAccountID, money_use_type_change_hero_name, money_type_diamond, pRst->nDiamond);
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					if (pPlayer->m_CharAtb.nDiamond >= pRst->nDiamond)
					{
						pPlayer->m_CharAtb.nDiamond	-= pRst->nDiamond;
					}
					else
					{
						pPlayer->m_CharAtb.nDiamond	= 0;
					}
				}
			}
		}
		break;
	case CMDID_READ_MAIL_ALL_RPL:
		{
			P_DBS4WEBUSER_READ_MAIL_ALL_T*	pRst	= (P_DBS4WEBUSER_READ_MAIL_ALL_T*)pBaseCmd;
			STC_GAMECMD_READ_MAIL_ALL_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_READ_MAIL_ALL;
			Cmd.nMailType		= pRst->nMailType;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_READ_MAIL_ALL_T::RST_OK)
			{
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					whlist<PrivateMailUnit*>::iterator	it	= pPlayer->m_MailList.begin();
					PrivateMailUnit*	pMail				= NULL;
					for (; it!=pPlayer->m_MailList.end(); ++it)
					{
						pMail			= *it;
						if ((pMail->nType&pRst->nMailType) && (pMail->nFlag!=MAIL_FLAG_REWARD))
						{
							pMail->bReaded	= true;
						}
					}
				}
			}
		}
		break;
	case CMDID_DELETE_MAIL_ALL_RPL:
		{
			P_DBS4WEBUSER_DELETE_MAIL_ALL_T*	pRst	= (P_DBS4WEBUSER_DELETE_MAIL_ALL_T*)pBaseCmd;
			STC_GAMECMD_DELETE_MAIL_ALL_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_DELETE_MAIL_ALL;
			Cmd.nMailType		= pRst->nMailType;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_DELETE_MAIL_ALL_T::RST_OK)
			{
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					whlist<PrivateMailUnit*>::iterator	it	= pPlayer->m_MailList.begin();
					whlist<PrivateMailUnit*>::iterator	itNext;
					PrivateMailUnit*	pMail				= NULL;
					for (; it!=pPlayer->m_MailList.end();)
					{
						itNext			= it;
						++itNext;

						pMail			= *it;
						if ((pMail->nType&pRst->nMailType) && (pMail->nFlag!=MAIL_FLAG_REWARD))
						{
							pPlayer->m_MailList.erase(it);
						}
						it				= itNext;
					}
				}
			}
		}
		break;
	case CMDID_ADD_GM_MAIL_RPL:
		{
			P_DBS4WEBUSER_ADD_GM_MAIL_T*	pRst	= (P_DBS4WEBUSER_ADD_GM_MAIL_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ADD_GM_MAIL_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_ADD_GM_MAIL;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_GET_PLAYERCARD_RPL:
		{
			// 这个指令用于处理玩家第一次创建角色成功,然后上线失败的处理(角色创建成功，从没上线，PlayerCard无法在内存中找到)
			P_DBS4WEBUSER_GET_PLAYERCARD_T*	pRst	= (P_DBS4WEBUSER_GET_PLAYERCARD_T*)pBaseCmd;
			if (pRst->nRst == P_DBS4WEBUSER_GET_PLAYERCARD_T::RST_OK)
			{
				PlayerCard*	pCard	= (PlayerCard*)wh_getptrnexttoptr(pRst);
				// 1.添加playercard
				PlayerCard*	pNewCard= m_pGameMngS->GetPlayerCardMng()->AddCard(pCard->nAccountID);
				memcpy(pNewCard, pCard, sizeof(PlayerCard));
				
				pNewCard->nLevelRank	= m_pGameMngS->GetRankMng()->GetRank(rank_type_char_level, pNewCard->nAccountID);
				pNewCard->nCityLevel	= 1; // 玩家登陆后会修改为正确的
				m_pGameMngS->GetPlayerCardMng()->AddCharName2AccountNameMap(pNewCard->szName, pNewCard->nAccountID);
				// 2.设置世界信息
				m_pGameMngS->GetWorldMng()->SetTileType(pNewCard->nPosX, pNewCard->nPosY, TileUnit::TILETYPE_TERRAIN, pNewCard->nAccountID);
			}
		}
		break;
	case CMDID_DEAL_EXCEL_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_EXCEL_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_EXCEL_TE_T*)pBaseCmd;
			P_DBS4WEBUSER_DEAL_EXCEL_TE_T::ExcelTERes*	pExcelTERes	= (P_DBS4WEBUSER_DEAL_EXCEL_TE_T::ExcelTERes*)wh_getptrnexttoptr(pRst);
			for (unsigned int nNum=0; nNum<pRst->nNum; nNum++)
			{
				switch (pExcelTERes->nExcelID)
				{
				case excel_te_id_refresh_char_day_te:
					{
						whunitallocatorFixed<PlayerUnit>::iterator	it	= m_Players.begin();
						for (; it != m_Players.end(); ++it)
						{
							// RefreshPlayerCharAtbFromDB((*it).nAccountID, false);
							P_DBS4WEB_DEAL_CHAR_DAY_TE_T	Cmd;
							Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
							Cmd.nSubCmd			= CMDID_DEAL_CHAR_DAY_TE_REQ;
							Cmd.nAccountID		= (*it).nAccountID;
							SendCmdToDB(&Cmd, sizeof(Cmd));
						}
					}
					break;
				case excel_te_id_refresh_world_famous_city:
					{
// 						if (m_bLoadTerrainOK && m_bLoadPlayerCardOK)
// 						{
							if (m_bAlreadyGetWorldFamousCity)
							{
								m_bCanGenWorldFamousCity			= true;
								m_bClearWorldFamousCityFirstWhenGen	= (bool)pExcelTERes->nData;
							}
							else
							{
								m_bNeedGenWorldFamousCity			= true;
								m_bClearWorldFamousCityFirstWhenGen	= (bool)pExcelTERes->nData;
							}
//						}
					}
					break;
// 				case excel_te_id_cup_activity_end:
// 					{
// 						P_DBS4WEB_CUP_ACTIVITY_END_T	Cmd;
// 						Cmd.nCmd						= P_DBS4WEB_REQ_CMD;
// 						Cmd.nSubCmd						= CMDID_CUP_ACTIVITY_END_REQ;
// 						SendCmdToDB(&Cmd, sizeof(Cmd));
// 					}
// 					break;
				case excel_te_id_refresh_world_res:
					{
						if (m_bAlreadyGetWorldRes)
						{
							m_bCanGenWorldRes						= true;
						}
						else
						{
							m_bNeedGenWorldRes						= true;
						}
					}
					break;
				case excel_te_id_arena_reward:
					{
						// 发送奖励
						P_DBS4WEB_ARENA_DEAL_REWARD_EVENT_T	Cmd;
						Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
						Cmd.nSubCmd			= CMDID_ARENA_DEAL_REWARD_EVENT_REQ;
						SendCmdToDB(&Cmd, sizeof(Cmd));
					}
					break;
				}
				pExcelTERes++;
			}
		}
		break;
	case CMDID_CHANGE_ALLIANCE_NAME_RPL:
		{
			P_DBS4WEBUSER_CHANGE_ALLIANCE_NAME_T*	pRst	= (P_DBS4WEBUSER_CHANGE_ALLIANCE_NAME_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_SET_ALLI_NAME_T		Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_SET_ALLI_NAME;
			Cmd.nDiamond		= pRst->nDiamond;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_SET_ALLI_NAME_T::RST_OK)
			{
				g_pLPNet->AddMoneyCostLog(pRst->nAccountID, money_use_type_change_alliance_name, money_type_diamond, pRst->nDiamond);

				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					if (pPlayer->m_CharAtb.nDiamond > pRst->nDiamond)
					{
						pPlayer->m_CharAtb.nDiamond -= pRst->nDiamond;
					}
					else
					{
						pPlayer->m_CharAtb.nDiamond	= 0;
					}
				}

				AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pRst->nAllianceID);
				if (pAlliance != NULL)
				{
					WH_STRNCPY0(pAlliance->basicInfo.szAllianceName, pRst->szAllianceName);

					for (int i=0; i<pAlliance->members.size(); i++)
					{
						AllianceMemberUnit&	member	= pAlliance->members[i];
						PlayerData*	pPlayerTmp		= GetPlayerData(member.nAccountID);
						if (pPlayerTmp != NULL)
						{
							WH_STRNCPY0(pPlayerTmp->m_CharAtb.szAllianceName, pRst->szAllianceName);
						}

						PlayerCard*	pCardTmp		= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(member.nAccountID);
						if (pCardTmp != NULL)
						{
							WH_STRNCPY0(pCardTmp->szAllianceName, pRst->szAllianceName);
						}
					}
				}
			}
		}
		break;
	case CMDID_GEN_WORLD_FAMOUS_CITY_RPL:
		{

		}
		break;
	case CMDID_GET_WORLD_FAMOUS_CITY_RPL:
		{
			P_DBS4WEBUSER_GET_WORLD_FAMOUS_CITY_T*	pRst	= (P_DBS4WEBUSER_GET_WORLD_FAMOUS_CITY_T*)pBaseCmd;
			WorldFamousCity*	pCity	= (WorldFamousCity*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				m_pGameMngS->GetWorldMng()->AddWorldFamousCity(pCity);

				pCity++;
			}
			m_bAlreadyGetWorldFamousCity	= true;
			if (m_bNeedGenWorldFamousCity)
			{
				m_bCanGenWorldFamousCity	= true;
				m_bNeedGenWorldFamousCity	= false;
			}
		}
		break;
	case CMDID_CUP_ACTIVITY_END_RPL:
		{
			m_pGameMngS->GetWorldMng()->ClearWorldFamousCityAll();
		}
		break;
	case CMDID_FETCH_MAIL_REWARD_RPL:
		{
			P_DBS4WEBUSER_FETCH_MAIL_REWARD_T*	pRst	= (P_DBS4WEBUSER_FETCH_MAIL_REWARD_T*)pBaseCmd;
			unsigned int	nDataSize	= nDSize - sizeof(P_DBS4WEBUSER_FETCH_MAIL_REWARD_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T) + nDataSize);
			STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T*	pCmd	= (STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd		= STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD;
			pCmd->nMailID		= pRst->nMailID;
			pCmd->nNum			= pRst->nNum;
			pCmd->nRst			= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), nDataSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			
			if (pRst->nRst == STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RST_OK)
			{
				PlayerData*	pPlayer	= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer == NULL)
				{
					return 0;
				}
				m_pGameMngS->LogMail(pPlayer, pRst->nMailID);
				pPlayer->DeleteMail(pRst->nMailID);
				STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit*	pUnit	= (STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit*)wh_getptrnexttoptr(pRst);
				for (int i=0; i<pRst->nNum; i++)
				{
					switch (pUnit->nType)
					{
					case mail_reward_type_diamond:
						{
							pPlayer->m_CharAtb.nDiamond	+= pUnit->nNum;
						}
						break;
					case mail_reward_type_crystal:
						{
							pPlayer->m_CharAtb.nCrystal	+= pUnit->nNum;
						}
						break;
					case mail_reward_type_gold:
						{
							pPlayer->m_CharAtb.nGold	+= pUnit->nNum;
						}
						break;
					}

					pUnit++;
				}
			}
		}
		break;
	case CMDID_UPLOAD_BILL_RPL:
		{
			P_DBS4WEB4USER_UPLOAD_BILL_T*	pRst	= (P_DBS4WEB4USER_UPLOAD_BILL_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_UPLOAD_BILL_T	Cmd;
			Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd		= STC_GAMECMD_OPERATE_UPLOAD_BILL;
			Cmd.nRst			= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1100,BILL)"1,0x%"WHINT64PRFX"X,%u,%u,%d,%s,%d"
				, pRst->nAccountID, pRst->nClientTime, pRst->nServerTime, 0, "", pRst->nRst);
		}
		break;

	case CMDID_TAVERN_REFRESH_RPL:
		{
			P_DBS4WEBUSER_TAVERN_REFRESH_T*	pRst	= (P_DBS4WEBUSER_TAVERN_REFRESH_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_TAVERN_REFRESH_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_TAVERN_REFRESH;
			Cmd.nRst		= pRst->nRst;
			Cmd.nCD			= pRst->nCD;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_HIRE_HERO_RPL:
		{
			P_DBS4WEBUSER_HIRE_HERO_T*	pRst	= (P_DBS4WEBUSER_HIRE_HERO_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_HIRE_HERO_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_HIRE_HERO;
			Cmd.nRst		= pRst->nRst;
			Cmd.nHeroID		= pRst->nHeroID;
			Cmd.nSlotID		= pRst->nSlotID;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_FIRE_HERO_RPL:
		{
			P_DBS4WEBUSER_FIRE_HERO_T*	pRst	= (P_DBS4WEBUSER_FIRE_HERO_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_FIRE_HERO_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_FIRE_HERO;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_LEVELUP_HERO_RPL:
		{
			P_DBS4WEBUSER_LEVELUP_HERO_T*	pRst	= (P_DBS4WEBUSER_LEVELUP_HERO_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_LEVELUP_HERO_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_LEVELUP_HERO;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ADD_GROW_RPL:
		{
			P_DBS4WEBUSER_ADD_GROW_T*	pRst	= (P_DBS4WEBUSER_ADD_GROW_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ADD_GROW_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_ADD_GROW;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_CONFIG_HERO_RPL:
		{
			P_DBS4WEBUSER_CONFIG_HERO_T*	pRst	= (P_DBS4WEBUSER_CONFIG_HERO_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CONFIG_HERO_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_CONFIG_HERO;
			Cmd.nRst		= pRst->nRst;
			Cmd.nProf		= pRst->nProf;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_GET_HERO_REFRESH_RPL:
		{
			P_DBS4WEBUSER_GET_HERO_REFRESH_T*	pRst	= (P_DBS4WEBUSER_GET_HERO_REFRESH_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_GET_HERO_REFRESH_T) + pRst->nNum*sizeof(RefreshHero));
			STC_GAMECMD_OPERATE_GET_HERO_REFRESH_T*	pCmd		= (STC_GAMECMD_OPERATE_GET_HERO_REFRESH_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_OPERATE_GET_HERO_REFRESH;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(RefreshHero));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_HERO_HIRE_RPL:
		{
			P_DBS4WEBUSER_GET_HERO_HIRE_T*	pRst	= (P_DBS4WEBUSER_GET_HERO_HIRE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_GET_HERO_HIRE_T) + pRst->nNum*sizeof(HireHero));
			STC_GAMECMD_OPERATE_GET_HERO_HIRE_T*	pCmd		= (STC_GAMECMD_OPERATE_GET_HERO_HIRE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_OPERATE_GET_HERO_HIRE;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(HireHero));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_START_COMBAT_RPL:
		{
			P_DBS4WEBUSER_START_COMBAT_T*	pRst	= (P_DBS4WEBUSER_START_COMBAT_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_START_COMBAT_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_START_COMBAT;
			Cmd.nRst		= pRst->nRst;
			Cmd.nCombatID	= pRst->nCombatID;
			Cmd.nCombatType	= pRst->nCombatType;
			Cmd.nGoTime		= pRst->nGoTime;
			Cmd.nBackTime	= pRst->nBackTime;
			Cmd.nAutoCombat	= pRst->nAutoCombat;
//			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_START_COMBAT_T::RST_OK &&
				pRst->nCombatType == 101)
			{
			// 成功就通知全队
			SendMsgToClient(pRst->nAccountID1, &Cmd, sizeof(Cmd));
			SendMsgToClient(pRst->nAccountID2, &Cmd, sizeof(Cmd));
			SendMsgToClient(pRst->nAccountID3, &Cmd, sizeof(Cmd));
			SendMsgToClient(pRst->nAccountID4, &Cmd, sizeof(Cmd));
			SendMsgToClient(pRst->nAccountID5, &Cmd, sizeof(Cmd));
			}
			else
			{
			// 失败就只给队长回执
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
			}

			// 玩家对战
			if (pRst->nRst == STC_GAMECMD_OPERATE_START_COMBAT_T::RST_OK &&
				(pRst->nCombatType == 2 || pRst->nCombatType == 20))
			{
			// 修改保护时间
			PlayerData*	pPlayer1		= GetPlayerData(pRst->nAccountID);
			if (pPlayer1 != NULL)
			{
				pPlayer1->m_CharAtb.nProtectTime = 0;
				pPlayer1->m_CharAtb.nProtectTimeEndTick = 0;
				SendPlayerCharAtbToClient(pPlayer1);
			}
			PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
			if (pPlayerCard != NULL)
			{
				pPlayerCard->nProtectTime = 0;
				pPlayerCard->nProtectTimeEndTick = 0;
			}

			// 通知对方
			PlayerUnit *pPlayer = GetPlayerByAccountID(pRst->nObjID);
			if (pPlayer)
			{
			P_DBS4WEB_LIST_COMBAT_T Cmd;
			Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd		= CMDID_LIST_COMBAT_REQ;
			Cmd.nAccountID	= pRst->nObjID;
			Cmd.nExt[0]		= pPlayer->nID;
			SendCmdToDB(&Cmd, sizeof(Cmd));
			}
			}
		}
		break;
	case CMDID_STOP_COMBAT_RPL:
		{
			P_DBS4WEBUSER_STOP_COMBAT_T*	pRst	= (P_DBS4WEBUSER_STOP_COMBAT_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_STOP_COMBAT_T) + pRst->nNum*sizeof(InstanceHeroDesc));
			STC_GAMECMD_OPERATE_STOP_COMBAT_T	*pCmd	= (STC_GAMECMD_OPERATE_STOP_COMBAT_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_OPERATE_STOP_COMBAT;
			pCmd->nRst		= pRst->nRst;
			pCmd->nCombatRst= pRst->nCombatRst;
			pCmd->nCombatID	= pRst->nCombatID;
			pCmd->nCombatType	= pRst->nCombatType;
			pCmd->n1Hero	= pRst->nHero1;
			pCmd->n2Hero	= pRst->nHero2;
			pCmd->n3Hero	= pRst->nHero3;
			pCmd->n4Hero	= pRst->nHero4;
			pCmd->n5Hero	= pRst->nHero5;
			pCmd->nDrug		= pRst->nDrug1;
			pCmd->nNewCombatID= pRst->nNewCombatID;
			pCmd->nGoTime		= pRst->nGoTime;
			pCmd->nBackTime	= pRst->nBackTime;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(InstanceHeroDesc));
			}

			// 通知client战斗结束
			switch (pRst->nCombatType)
			{
			case COMBAT_PVE_RAID:
				{
					// 普通战斗
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}break;
			case COMBAT_PVP_ATTACK:
				{
					// 玩家对战
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}break;
			case COMBAT_PVE_WORLDGOLDMINE:
			case COMBAT_PVP_WORLDGOLDMINE_ATTACK:
				{
					// 世界金矿
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}break;
			case COMBAT_INSTANCE_BAIZHANBUDAI:
			case COMBAT_INSTANCE_NANZHENGBEIZHAN:
				{
					// 百战不殆/南征北战
					pCmd->nDrug		= pRst->nDrug1;
					SendMsgToClient(pRst->nAccountID1, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					pCmd->nDrug		= pRst->nDrug2;
					SendMsgToClient(pRst->nAccountID2, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					pCmd->nDrug		= pRst->nDrug3;
					SendMsgToClient(pRst->nAccountID3, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					pCmd->nDrug		= pRst->nDrug4;
					SendMsgToClient(pRst->nAccountID4, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					pCmd->nDrug		= pRst->nDrug5;
					SendMsgToClient(pRst->nAccountID5, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}break;
			case COMBAT_WORLDCITY:
				{
					// 世界名城
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}break;
			case COMBAT_PVE_WORLD_RES:
				{
					// 世界资源
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}break;
			}
		}
		break;
	case	CMDID_GET_COMBAT_RPL:
		{
			P_DBS4WEBUSER_GET_COMBAT_T*	pRst	= (P_DBS4WEBUSER_GET_COMBAT_T*)pBaseCmd;
			CombatUnit *pUnit = (CombatUnit*)wh_getptrnexttoptr(pRst);
			if (pRst->nExt[1])
			{
				// 填好战斗返回包
				m_vectrawbuf.resize(sizeof(P_DBS4WEB_DEAL_COMBAT_BACK_T) + (pRst->nNumA+pRst->nNumD)*sizeof(CombatUnit4Log));
				P_DBS4WEB_DEAL_COMBAT_BACK_T *pReq	= (P_DBS4WEB_DEAL_COMBAT_BACK_T*)m_vectrawbuf.getbuf();
				pReq->nCmd			= P_DBS4WEB_REQ_CMD;
				pReq->nSubCmd		= CMDID_DEAL_COMBAT_BACK_REQ;
				pReq->nCombatID		= pRst->nCombatID;
				pReq->nCombatType	= pRst->nCombatType;
				pReq->nAccountID	= pRst->nAccountID;
				pReq->nObjID		= pRst->nObjID;
				pReq->nAccountID1	= pRst->nAccountID1;
				pReq->nAccountID2	= pRst->nAccountID2;
				pReq->nAccountID3	= pRst->nAccountID3;
				pReq->nAccountID4	= pRst->nAccountID4;
				pReq->nAccountID5	= pRst->nAccountID5;
				pReq->nNumA			= pRst->nNumA;
				pReq->nNumD			= pRst->nNumD;
				CombatUnit4Log *pLog;
				pLog	= (CombatUnit4Log*)wh_getptrnexttoptr(pReq);
				for (int i=0; i<pReq->nNumA; ++i)
				{
					pLog->nLevel		= pUnit->nLevel;
					strcpy(pLog->szName, pUnit->szName);
					pLog->nSlot			= pUnit->nSlot;
					pLog->nArmyType		= pUnit->nArmyType;
					pLog->nArmyLevel	= pUnit->nArmyLevel;
					pLog->nArmyNumBefore= pUnit->nArmyNum;
					pLog->nHeroID		= pUnit->nHeroID;
					pLog->nArmyAttack	= pUnit->nAttack;
					pLog->nArmyDefense	= pUnit->nDefense;
					pLog->nArmyHealth	= pUnit->nHealth;
					++ pLog;
					++ pUnit;
				}
				for (int i=0; i<pReq->nNumD; ++i)
				{
					pLog->nLevel		= pUnit->nLevel;
					strcpy(pLog->szName, pUnit->szName);
					pLog->nSlot			= pUnit->nSlot;
					pLog->nArmyType		= pUnit->nArmyType;
					pLog->nArmyLevel	= pUnit->nArmyLevel;
					pLog->nArmyNumBefore= pUnit->nArmyNum;
					pLog->nHeroID		= pUnit->nHeroID;
					pLog->nArmyAttack	= pUnit->nAttack;
					pLog->nArmyDefense	= pUnit->nDefense;
					pLog->nArmyHealth	= pUnit->nHealth;
					++ pLog;
					++ pUnit;
				}

				// 计算战斗并存盘
				CombatUnit aA[COMBATFIELD_CHANNEL], aD[COMBATFIELD_CHANNEL];
				int nRst = m_pGameMngS->CombatCalc_Raid(&aA[0], &aD[0], pRst);

				// 进入战斗返回状态
				pReq->nCombatRst	= nRst;
				pLog	= (CombatUnit4Log*)wh_getptrnexttoptr(pReq);
				for (int i=0; i<pReq->nNumA; ++i)
				{
					pLog->nArmyNumAfter	= aA[pLog->nSlot-1].nArmyNum<0? 0: aA[pLog->nSlot-1].nArmyNum;
					++ pLog;
				}
				for (int i=0; i<pReq->nNumD; ++i)
				{
					pLog->nArmyNumAfter	= aD[pLog->nSlot-1].nArmyNum<0? 0: aD[pLog->nSlot-1].nArmyNum;
					++ pLog;
				}
				g_pLPNet->SendCmdToDB(m_vectrawbuf.getbuf(), m_vectrawbuf.size());

				/*
				m_vectrawbuf.resize(sizeof(STC_GAMECMD_COMBAT_RST_T) + pRst->nNumA*sizeof(CombatUnit));
				STC_GAMECMD_COMBAT_RST_T*	pCmd		= (STC_GAMECMD_COMBAT_RST_T*)m_vectrawbuf.getbuf();
				pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				pCmd->nGameCmd	= STC_GAMECMD_COMBAT_RST;
				pCmd->nCombatID	= pRst->nCombatID;
				pCmd->nCombatType	= pRst->nCombatType;
				pCmd->nObjID	= pRst->nObjID;
				pCmd->nRst		= nRst;
				pCmd->nNum		= pRst->nNumA;
				CombatUnit *p = (CombatUnit*)wh_getptrnexttoptr(pCmd);
				for (int i=0; i<COMBATFIELD_CHANNEL; ++i)
				{
					if (aA[i].nSlot>0)
					{
						memcpy(p, &aA[i], sizeof(*p));
						++ p;
					}
				}

				// 通知client战斗结果
				switch (pRst->nCombatType)
				{
				case COMBAT_PVE_RAID:
				case COMBAT_PVE_WORLDGOLDMINE:
					{
						// 普通战斗
						SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}break;
				case COMBAT_PVP_ATTACK:
				case COMBAT_PVP_WORLDGOLDMINE_ATTACK:
					{
						// 玩家对战
						SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						// 包内容要改变一下
						m_vectrawbuf.resize(sizeof(STC_GAMECMD_COMBAT_RST_T) + pRst->nNumD*sizeof(CombatUnit));
						// 发给对方的话，战斗类型就+1
						pCmd->nCombatType	= pRst->nCombatType + 1;
						pCmd->nObjID	= pRst->nAccountID;
						pCmd->nRst	= pCmd->nRst==COMBAT_RST_VICTORY? COMBAT_RST_LOSE: COMBAT_RST_VICTORY;
						pCmd->nNum		= pRst->nNumD;
						p = (CombatUnit*)wh_getptrnexttoptr(pCmd);
						for (int i=0; i<COMBATFIELD_CHANNEL; ++i)
						{
							if (aD[i].nSlot>0)
							{
								memcpy(p, &aD[i], sizeof(*p));
								++ p;
							}
						}
						SendMsgToClient(pRst->nObjID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}break;
				case COMBAT_INSTANCE_BAIZHANBUDAI:
				case COMBAT_INSTANCE_NANZHENGBEIZHAN:
					{
						// 副本
						SendMsgToClient(pRst->nAccountID1, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						SendMsgToClient(pRst->nAccountID2, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						SendMsgToClient(pRst->nAccountID3, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						SendMsgToClient(pRst->nAccountID4, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						SendMsgToClient(pRst->nAccountID5, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}break;
				}
				*/

				// 返回时间为0的情况，就要把战斗数据发给client
				switch (pRst->nCombatType)
				{
				case COMBAT_PVE_WORLDGOLDMINE:
				case COMBAT_PVP_WORLDGOLDMINE_ATTACK:
				case COMBAT_PVP_WORLDGOLDMINE_DEFENSE:
					{
						m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_GET_COMBAT_T) + nDSize-sizeof(*pRst));
						// 发给client
						STC_GAMECMD_OPERATE_GET_COMBAT_T*	pCmd		= (STC_GAMECMD_OPERATE_GET_COMBAT_T*)m_vectrawbuf.getbuf();
						pCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						pCmd->nGameCmd		= STC_GAMECMD_OPERATE_GET_COMBAT;
						pCmd->nRst			= pRst->nRst;
						pCmd->nCombatID		= pRst->nCombatID;
						pCmd->nCombatType	= pRst->nCombatType;
						pCmd->nNumA			= pRst->nNumA;
						pCmd->nNumD			= pRst->nNumD;
						strcpy(pCmd->szObjName, pRst->szObjName);
						pCmd->nObjHeadID	= pRst->nObjHeadID;
						memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), nDSize-sizeof(*pRst));
						SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}break;
				}
			}
			else
			{
				m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_GET_COMBAT_T) + nDSize-sizeof(*pRst));
				// 发给client
				STC_GAMECMD_OPERATE_GET_COMBAT_T*	pCmd		= (STC_GAMECMD_OPERATE_GET_COMBAT_T*)m_vectrawbuf.getbuf();
				pCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				pCmd->nGameCmd		= STC_GAMECMD_OPERATE_GET_COMBAT;
				pCmd->nRst			= pRst->nRst;
				pCmd->nCombatID		= pRst->nCombatID;
				pCmd->nCombatType	= pRst->nCombatType;
				pCmd->nNumA			= pRst->nNumA;
				pCmd->nNumD			= pRst->nNumD;
				strcpy(pCmd->szObjName, pRst->szObjName);
				pCmd->nObjHeadID	= pRst->nObjHeadID;
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), nDSize-sizeof(*pRst));
				SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
		}
		break;
	case	CMDID_LIST_COMBAT_RPL:
		{
			P_DBS4WEBUSER_LIST_COMBAT_T*	pRst	= (P_DBS4WEBUSER_LIST_COMBAT_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_OPERATE_LIST_COMBAT_T) + pRst->nNum*sizeof(CombatDesc));
			STC_GAMECMD_OPERATE_LIST_COMBAT_T*	pCmd		= (STC_GAMECMD_OPERATE_LIST_COMBAT_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_OPERATE_LIST_COMBAT;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(CombatDesc));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_CONFIG_CITYDEFENSE_RPL:
		{
			P_DBS4WEBUSER_CONFIG_CITYDEFENSE_T*	pRst	= (P_DBS4WEBUSER_CONFIG_CITYDEFENSE_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_GET_ITEM_RPL:
		{
			P_DBS4WEBUSER_GET_ITEM_T*	pRst	= (P_DBS4WEBUSER_GET_ITEM_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ITEM_T) + pRst->nNum*sizeof(ItemDesc));
			STC_GAMECMD_GET_ITEM_T*	pCmd		= (STC_GAMECMD_GET_ITEM_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_ITEM;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(ItemDesc));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ADD_ITEM_RPL:
		{
			P_DBS4WEBUSER_ADD_ITEM_T*	pRst	= (P_DBS4WEBUSER_ADD_ITEM_T*)pBaseCmd;
			STC_GAMECMD_ADD_ITEM_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ADD_ITEM;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_DEL_ITEM_RPL:
		{
			P_DBS4WEBUSER_DEL_ITEM_T*	pRst	= (P_DBS4WEBUSER_DEL_ITEM_T*)pBaseCmd;
			STC_GAMECMD_DEL_ITEM_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_DEL_ITEM;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_EQUIP_ITEM_RPL:
		{
			P_DBS4WEBUSER_EQUIP_ITEM_T*	pRst	= (P_DBS4WEBUSER_EQUIP_ITEM_T*)pBaseCmd;
			STC_GAMECMD_EQUIP_ITEM_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_EQUIP_ITEM;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_DISEQUIP_ITEM_RPL:
		{
			P_DBS4WEBUSER_DISEQUIP_ITEM_T*	pRst	= (P_DBS4WEBUSER_DISEQUIP_ITEM_T*)pBaseCmd;
			STC_GAMECMD_DISEQUIP_ITEM_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_DISEQUIP_ITEM;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_MOUNT_ITEM_RPL:
		{
			P_DBS4WEBUSER_MOUNT_ITEM_T*	pRst	= (P_DBS4WEBUSER_MOUNT_ITEM_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_MOUNT_ITEM_T) + sizeof(HeroDesc));
			STC_GAMECMD_MOUNT_ITEM_T*	pCmd		= (STC_GAMECMD_MOUNT_ITEM_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_MOUNT_ITEM;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(HeroDesc));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_UNMOUNT_ITEM_RPL:
		{
			P_DBS4WEBUSER_UNMOUNT_ITEM_T*	pRst	= (P_DBS4WEBUSER_UNMOUNT_ITEM_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_UNMOUNT_ITEM_T) + sizeof(HeroDesc));
			STC_GAMECMD_UNMOUNT_ITEM_T*	pCmd		= (STC_GAMECMD_UNMOUNT_ITEM_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_UNMOUNT_ITEM;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(HeroDesc));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_COMPOS_ITEM_RPL:
		{
			P_DBS4WEBUSER_COMPOS_ITEM_T*	pRst	= (P_DBS4WEBUSER_COMPOS_ITEM_T*)pBaseCmd;
			STC_GAMECMD_COMPOS_ITEM_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_COMPOS_ITEM;
			Cmd.nRst		= pRst->nRst;
			Cmd.nItemID		= pRst->nItemID;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_DISCOMPOS_ITEM_RPL:
		{
			P_DBS4WEBUSER_DISCOMPOS_ITEM_T*	pRst	= (P_DBS4WEBUSER_DISCOMPOS_ITEM_T*)pBaseCmd;
			STC_GAMECMD_DISCOMPOS_ITEM_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_DISCOMPOS_ITEM;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_GET_QUEST_RPL:
		{
			P_DBS4WEBUSER_GET_QUEST_T*	pRst	= (P_DBS4WEBUSER_GET_QUEST_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_ITEM_T) + pRst->nNum*sizeof(QuestDesc));
			STC_GAMECMD_GET_QUEST_T*	pCmd		= (STC_GAMECMD_GET_QUEST_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_QUEST;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(QuestDesc));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_DONE_QUEST_RPL:
		{
			P_DBS4WEBUSER_DONE_QUEST_T*	pRst	= (P_DBS4WEBUSER_DONE_QUEST_T*)pBaseCmd;
			UseItemDesc *pDesc = (UseItemDesc*)wh_getptrnexttoptr(pRst);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_DONE_QUEST_T) + sizeof(UseItemDesc) + sizeof(AddItemDesc)*pDesc->nNum);

			STC_GAMECMD_DONE_QUEST_T	*pCmd	= (STC_GAMECMD_DONE_QUEST_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_DONE_QUEST;
			pCmd->nRst		= pRst->nRst;

			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(UseItemDesc));
			memcpy((char*)wh_getptrnexttoptr(pCmd)+sizeof(UseItemDesc), (char*)wh_getptrnexttoptr(pRst)+sizeof(UseItemDesc), sizeof(AddItemDesc)*pDesc->nNum);

			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			// 更新LP内存数据
			RefreshPlayerCharAtbFromDB(pRst->nAccountID);
		}
		break;
	case CMDID_GET_INSTANCESTATUS_RPL:
		{
			P_DBS4WEBUSER_GET_INSTANCESTATUS_T*	pRst	= (P_DBS4WEBUSER_GET_INSTANCESTATUS_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_INSTANCESTATUS_T) + pRst->nNum*sizeof(InstanceStatus));
			STC_GAMECMD_GET_INSTANCESTATUS_T*	pCmd		= (STC_GAMECMD_GET_INSTANCESTATUS_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_INSTANCESTATUS;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(InstanceStatus));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_INSTANCEDESC_RPL:
		{
			P_DBS4WEBUSER_GET_INSTANCEDESC_T*	pRst	= (P_DBS4WEBUSER_GET_INSTANCEDESC_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_INSTANCEDESC_T) + pRst->nNum*sizeof(InstanceDesc));
			STC_GAMECMD_GET_INSTANCEDESC_T*	pCmd		= (STC_GAMECMD_GET_INSTANCEDESC_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_INSTANCEDESC;
			pCmd->nRst		= pRst->nRst;
			pCmd->nClassID	= pRst->nClassID;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(InstanceDesc));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_CREATE_INSTANCE_RPL:
		{
			P_DBS4WEBUSER_CREATE_INSTANCE_T*	pRst	= (P_DBS4WEBUSER_CREATE_INSTANCE_T*)pBaseCmd;
			STC_GAMECMD_CREATE_INSTANCE_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_CREATE_INSTANCE;
			Cmd.nRst		= pRst->nRst;
			Cmd.nInstanceID	= pRst->nInstanceID;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
			if (pPlayer != NULL)
			{
				pPlayer->m_nGroupID		= pRst->nInstanceID;
				SetPlayerTag64(pPlayer->m_nPlayerGID, pRst->nInstanceID, false);
			}

			{
				// 获取一下副本状态
				P_DBS4WEB_GET_INSTANCESTATUS_T Cmd;
				Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd		= CMDID_GET_INSTANCESTATUS_REQ;
				Cmd.nAccountID	= pRst->nAccountID;
				Cmd.nExt[0]		= pRst->nExt[0];
				SendCmdToDB(&Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_JOIN_INSTANCE_RPL:
		{
			P_DBS4WEBUSER_JOIN_INSTANCE_T*	pRst	= (P_DBS4WEBUSER_JOIN_INSTANCE_T*)pBaseCmd;
			STC_GAMECMD_JOIN_INSTANCE_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_JOIN_INSTANCE;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			// 通知全队
			if (pRst->nRst == STC_GAMECMD_JOIN_INSTANCE_T::RST_OK)
			{
			P_DBS4WEB_GET_INSTANCEDATA_T Req;
			Req.nCmd		= P_DBS4WEB_REQ_CMD;
			Req.nSubCmd		= CMDID_GET_INSTANCEDATA_REQ;
			Req.nAccountID	= 0;
			Req.nExcelID	= pRst->nExcelID;
			Req.nInstanceID	= pRst->nInstanceID;
			Req.nExt[0]		= 0;
			Req.nExt[1]		= 1;
			g_pLPNet->SendCmdToDB(&Req, sizeof(Req));

			PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
			if (pPlayer != NULL)
			{
				pPlayer->m_nGroupID		= pRst->nInstanceID;
				SetPlayerTag64(pPlayer->m_nPlayerGID, pRst->nInstanceID, false);
			}
			}

			{
				// 获取一下副本状态
				P_DBS4WEB_GET_INSTANCESTATUS_T Cmd;
				Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd		= CMDID_GET_INSTANCESTATUS_REQ;
				Cmd.nAccountID	= pRst->nAccountID;
				Cmd.nExt[0]		= pRst->nExt[0];
				SendCmdToDB(&Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_QUIT_INSTANCE_RPL:
		{
			P_DBS4WEBUSER_QUIT_INSTANCE_T*	pRst	= (P_DBS4WEBUSER_QUIT_INSTANCE_T*)pBaseCmd;
			STC_GAMECMD_QUIT_INSTANCE_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_QUIT_INSTANCE;
			Cmd.nRst		= pRst->nRst;
			Cmd.nInstanceID	= pRst->nInstanceID;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
			if (pPlayer != NULL)
			{
				pPlayer->m_nGroupID		= 0;
				SetPlayerTag64(pPlayer->m_nPlayerGID, pRst->nInstanceID, true);
			}

			// 通知全队
			if (pRst->nRst == STC_GAMECMD_QUIT_INSTANCE_T::RST_OK)
			{
			P_DBS4WEB_GET_INSTANCEDATA_T Req;
			Req.nCmd		= P_DBS4WEB_REQ_CMD;
			Req.nSubCmd		= CMDID_GET_INSTANCEDATA_REQ;
			Req.nAccountID	= 0;
			Req.nExcelID	= pRst->nExcelID;
			Req.nInstanceID	= pRst->nInstanceID;
			Req.nExt[0]		= 0;
			Req.nExt[1]		= 1;
			g_pLPNet->SendCmdToDB(&Req, sizeof(Req));
			}
		}
		break;
	case CMDID_DESTROY_INSTANCE_RPL:
		{
			P_DBS4WEBUSER_DESTROY_INSTANCE_T*	pRst	= (P_DBS4WEBUSER_DESTROY_INSTANCE_T*)pBaseCmd;
			STC_GAMECMD_DESTROY_INSTANCE_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_DESTROY_INSTANCE;
			Cmd.nRst		= pRst->nRst;
			Cmd.nInstanceID	= pRst->nInstanceID;

			tty_id_t *pID	= &pRst->nID1;
			for (int i=0; i<5; ++i)
			{
				SendMsgToClient(*pID, &Cmd, sizeof(Cmd));
			PlayerData*	pPlayer	= GetPlayerData(*pID);
			if (pPlayer != NULL)
			{
				pPlayer->m_nGroupID		= 0;
				SetPlayerTag64(pPlayer->m_nPlayerGID, pRst->nInstanceID, true);
			}

				++ pID;
			}
		}
		break;
	case CMDID_GET_INSTANCEDATA_RPL:
		{
			P_DBS4WEBUSER_GET_INSTANCEDATA_T*	pRst	= (P_DBS4WEBUSER_GET_INSTANCEDATA_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_INSTANCEDATA_T) + pRst->nNum*sizeof(InstanceData) + pRst->nNumHero*sizeof(CombatUnit));
			STC_GAMECMD_GET_INSTANCEDATA_T*	pCmd		= (STC_GAMECMD_GET_INSTANCEDATA_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_INSTANCEDATA;
			pCmd->nRst		= pRst->nRst;
			pCmd->nCurLevel	= pRst->nCurLevel;
			pCmd->nAutoCombat	= pRst->nAutoCombat;
			pCmd->nAutoSupply	= pRst->nAutoSupply;
			pCmd->nRetryTimes	= pRst->nRetryTimes;
			pCmd->nInstanceID	= pRst->nInstanceID;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(InstanceData));
			}
			pCmd->nNumHero	= pRst->nNumHero;
			if (pCmd->nNumHero > 0)
			{
				memcpy((char*)wh_getptrnexttoptr(pCmd)+pCmd->nNum*sizeof(InstanceData), (char*)wh_getptrnexttoptr(pRst)+pCmd->nNum*sizeof(InstanceData), pCmd->nNumHero*sizeof(CombatUnit));
			}
			if (pRst->nExt[1] == 0)
			{
				SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
			else
			{
				// 通知全队
				InstanceData *pInstance	= (InstanceData*)wh_getptrnexttoptr(pRst);
				for (int i=0; i<pRst->nNum; ++i)
				{
					SendMsgToClient(pInstance->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					++ pInstance;
				}
			}
		}
		break;
	case CMDID_CONFIG_INSTANCEHERO_RPL:
		{
			P_DBS4WEBUSER_CONFIG_INSTANCEHERO_T*	pRst	= (P_DBS4WEBUSER_CONFIG_INSTANCEHERO_T*)pBaseCmd;
			STC_GAMECMD_CONFIG_INSTANCEHERO_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_CONFIG_INSTANCEHERO;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			// 通知全队
			if (pRst->nRst == STC_GAMECMD_CONFIG_INSTANCEHERO_T::RST_OK)
			{
			P_DBS4WEB_GET_INSTANCEDATA_T Req;
			Req.nCmd		= P_DBS4WEB_REQ_CMD;
			Req.nSubCmd		= CMDID_GET_INSTANCEDATA_REQ;
			Req.nAccountID	= 0;
			Req.nExcelID	= pRst->nExcelID;
			Req.nInstanceID	= pRst->nInstanceID;
			Req.nExt[0]		= 0;
			Req.nExt[1]		= 1;
			g_pLPNet->SendCmdToDB(&Req, sizeof(Req));
			}
		}
		break;
	case CMDID_GET_INSTANCELOOT_RPL:
		{
			P_DBS4WEBUSER_GET_INSTANCELOOT_T*	pRst	= (P_DBS4WEBUSER_GET_INSTANCELOOT_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_INSTANCELOOT_T) + pRst->nNum*sizeof(LootDesc));
			STC_GAMECMD_GET_INSTANCELOOT_T*	pCmd		= (STC_GAMECMD_GET_INSTANCELOOT_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_INSTANCELOOT;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(LootDesc));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_INSTANCELOOT_ADDITEM_RPL:
		{
			P_DBS4WEBUSER_GET_INSTANCELOOT_ADDITEM_RPL_T*	pRst	= (P_DBS4WEBUSER_GET_INSTANCELOOT_ADDITEM_RPL_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_INSTANCELOOT_ADDITEM_T) + pRst->nNum*sizeof(InstanceLootDesc));
			STC_GAMECMD_GET_INSTANCELOOT_ADDITEM_T*	pCmd		= (STC_GAMECMD_GET_INSTANCELOOT_ADDITEM_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_INSTANCELOOT_ADDITEM;
			pCmd->nCombatType	= pRst->nCombatType;
			pCmd->nObjID		= pRst->nObjID;
			pCmd->nNum		= pRst->nNum;
			pCmd->nClassID	= pRst->nClassID;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(InstanceLootDesc));
			}
			SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			if (pCmd->nCombatType==COMBAT_WORLDCITY)
			{
				PlayerData*	pPlayerData	= GetPlayerData(pRst->nAccountID);
				if (pPlayerData != NULL)
				{
					pPlayerData->m_CharAtb.nCup		= pRst->nCup;
				}
				PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
				if (pPlayerCard)
				{	
					pPlayerCard->nCup	= pRst->nCup;

					m_pGameMngS->GetAllianceMng()->SetAllianceMemberCup(pPlayerCard->nAllianceID, pPlayerCard->nAccountID, pPlayerCard->nCup);
				}
			}
		}
		break;
	case CMDID_AUTO_COMBAT_RPL:
		{
			P_DBS4WEBUSER_AUTO_COMBAT_T*	pRst	= (P_DBS4WEBUSER_AUTO_COMBAT_T*)pBaseCmd;
			STC_GAMECMD_AUTO_COMBAT_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_AUTO_COMBAT;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_DEAL_COMBAT_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_COMBAT_TE_T *pRst	= (P_DBS4WEBUSER_DEAL_COMBAT_TE_T*)pBaseCmd;
			if (pRst->nStatus == 0)
			{
				// 战斗前往超时
				// 计算结果并存盘
				P_DBS4WEB_GET_COMBAT_T Cmd;
				Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd		= CMDID_GET_COMBAT_REQ;
				Cmd.nAccountID	= 0;
				Cmd.nCombatID	= pRst->nCombatID;
				Cmd.nExt[0]		= 0;
				// 收到回执后会计算战斗并存盘
				Cmd.nExt[1]		= 1;
				// 生成战斗数据
				Cmd.nExt[2]		= 1;

				g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
			}
			else
			{
				// 战斗返回超时
				// 删除记录
				P_DBS4WEB_STOP_COMBAT_T Cmd;
				Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd		= CMDID_STOP_COMBAT_REQ;
				Cmd.nAccountID	= 0;
				Cmd.nCombatID	= pRst->nCombatID;
				Cmd.nCombatType	= pRst->nCombatType;
				Cmd.nExt[0]		= 0;

				g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
			}
		}
		break;
	case CMDID_EQUIP_ITEM_ALL_RPL:
		{
			P_DBS4WEBUSER_EQUIP_ITEM_ALL_T*	pRst	= (P_DBS4WEBUSER_EQUIP_ITEM_ALL_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_EQUIP_ITEM_ALL_T) + sizeof(HeroDesc));
			STC_GAMECMD_EQUIP_ITEM_ALL_T*	pCmd		= (STC_GAMECMD_EQUIP_ITEM_ALL_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_EQUIP_ITEM_ALL;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(HeroDesc));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_MOUNT_ITEM_ALL_RPL:
		{
			P_DBS4WEBUSER_MOUNT_ITEM_ALL_T*	pRst	= (P_DBS4WEBUSER_MOUNT_ITEM_ALL_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_MOUNT_ITEM_ALL_T) + sizeof(HeroDesc));
			STC_GAMECMD_MOUNT_ITEM_ALL_T*	pCmd		= (STC_GAMECMD_MOUNT_ITEM_ALL_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_MOUNT_ITEM_ALL;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(HeroDesc));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_USE_ITEM_RPL:
		{
			P_DBS4WEBUSER_USE_ITEM_T*	pRst	= (P_DBS4WEBUSER_USE_ITEM_T*)pBaseCmd;
			if (pRst->nType==SYNC_TYPE_HERO)
			{
				m_vectrawbuf.resize(sizeof(STC_GAMECMD_USE_ITEM_T) + sizeof(HeroDesc));
			}
			else if (pRst->nType==SYNC_TYPE_CHAR)
			{
				CharDesc *pDesc = (CharDesc*)wh_getptrnexttoptr(pRst);
				m_vectrawbuf.resize(sizeof(STC_GAMECMD_USE_ITEM_T) + sizeof(CharDesc));
				// 更新LP内存数据
				RefreshPlayerCharAtbFromDB(pRst->nAccountID);
			}
			else if (pRst->nType==SYNC_TYPE_ITEM)
			{
				UseItemDesc *pDesc = (UseItemDesc*)wh_getptrnexttoptr(pRst);
				m_vectrawbuf.resize(sizeof(STC_GAMECMD_USE_ITEM_T) + sizeof(UseItemDesc) + sizeof(AddItemDesc)*pDesc->nNum);
				// 更新LP内存数据
				RefreshPlayerCharAtbFromDB(pRst->nAccountID);
			}
			else if (pRst->nType==SYNC_TYPE_DRUG)
			{
				DrugDesc *pDrug = (DrugDesc*)wh_getptrnexttoptr(pRst);
				m_vectrawbuf.resize(sizeof(STC_GAMECMD_USE_ITEM_T) + sizeof(DrugDesc));
			}
			STC_GAMECMD_USE_ITEM_T	*pCmd	= (STC_GAMECMD_USE_ITEM_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_USE_ITEM;
			pCmd->nRst		= pRst->nRst;
			pCmd->nType		= pRst->nType;
			if (pRst->nType==SYNC_TYPE_HERO)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(HeroDesc));
			}
			else if (pRst->nType==SYNC_TYPE_CHAR)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(CharDesc));
			}
			else if (pRst->nType==SYNC_TYPE_ITEM)
			{
				UseItemDesc *pDesc = (UseItemDesc*)wh_getptrnexttoptr(pRst);
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(UseItemDesc));
				memcpy((char*)wh_getptrnexttoptr(pCmd)+sizeof(UseItemDesc), (char*)wh_getptrnexttoptr(pRst)+sizeof(UseItemDesc), sizeof(AddItemDesc)*pDesc->nNum);
			}
			else if (pRst->nType==SYNC_TYPE_DRUG)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(DrugDesc));
			}

			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_SYNC_CHAR_RPL:
		{
			P_DBS4WEBUSER_SYNC_CHAR_T*	pRst	= (P_DBS4WEBUSER_SYNC_CHAR_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_SYNC_CHAR_T) + sizeof(CharDesc));
			STC_GAMECMD_SYNC_CHAR_T*	pCmd		= (STC_GAMECMD_SYNC_CHAR_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_SYNC_CHAR;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(CharDesc));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_SYNC_HERO_RPL:
		{
			P_DBS4WEBUSER_SYNC_HERO_T*	pRst	= (P_DBS4WEBUSER_SYNC_HERO_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_SYNC_HERO_T) + sizeof(HeroDesc));
			STC_GAMECMD_SYNC_HERO_T*	pCmd		= (STC_GAMECMD_SYNC_HERO_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_SYNC_HERO;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(HeroDesc));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_AUTO_SUPPLY_RPL:
		{
			P_DBS4WEBUSER_AUTO_SUPPLY_T*	pRst	= (P_DBS4WEBUSER_AUTO_SUPPLY_T*)pBaseCmd;
			STC_GAMECMD_AUTO_SUPPLY_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_AUTO_SUPPLY;
			Cmd.nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_GET_EQUIP_RPL:
		{
			P_DBS4WEBUSER_GET_EQUIP_T*	pRst	= (P_DBS4WEBUSER_GET_EQUIP_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_EQUIP_T) + pRst->nNum*sizeof(EquipDesc));
			STC_GAMECMD_GET_EQUIP_T*	pCmd		= (STC_GAMECMD_GET_EQUIP_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_EQUIP;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(EquipDesc));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_GEM_RPL:
		{
			P_DBS4WEBUSER_GET_GEM_T*	pRst	= (P_DBS4WEBUSER_GET_GEM_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_GEM_T) + pRst->nNum*sizeof(GemDesc));
			STC_GAMECMD_GET_GEM_T*	pCmd		= (STC_GAMECMD_GET_GEM_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_GEM;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(GemDesc));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_MANUAL_SUPPLY_RPL:
		{
			P_DBS4WEBUSER_MANUAL_SUPPLY_T*	pRst	= (P_DBS4WEBUSER_MANUAL_SUPPLY_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_MANUAL_SUPPLY_T) + pRst->nNum*sizeof(InstanceHeroDesc));
			STC_GAMECMD_MANUAL_SUPPLY_T	*pCmd	= (STC_GAMECMD_MANUAL_SUPPLY_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_MANUAL_SUPPLY;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(InstanceHeroDesc));
			}
			SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			// 草药消耗
			tty_id_t *pID	= (tty_id_t*)&pRst->nAccountID1;
			int *pDrug		= (int*)&pRst->nDrug1;
			STC_GAMECMD_SUPPLY_DRUG_T Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_SUPPLY_DRUG;
			Cmd.nNum		= 0;
			if (pRst->nExcelID == 100 ||
				pRst->nExcelID == 2)
			{
				// 单人副本
				for (int i=0; i<5; ++i)
				{
					if (*pID>0)
					{
						Cmd.nNum	= *pDrug;
					}
					++ pID;
					++ pDrug;
				}

				SendMsgToClient(pRst->nAccountID1, &Cmd, sizeof(Cmd));
			}
			else
			{
				// 多人副本/单独ID
				for (int i=0; i<5; ++i)
				{
					if (*pID>0)
					{
						Cmd.nNum	= *pDrug;
						SendMsgToClient(*pID, &Cmd, sizeof(Cmd));
					}
					++ pID;
					++ pDrug;
				}
			}

			// 通知全队
			if (pRst->nRst == STC_GAMECMD_MANUAL_SUPPLY_T::RST_OK && 
				pRst->nExt[1] == 0 &&
				pRst->nExcelID >= 100)
			{
			P_DBS4WEB_GET_INSTANCEDATA_T Req;
			Req.nCmd		= P_DBS4WEB_REQ_CMD;
			Req.nSubCmd		= CMDID_GET_INSTANCEDATA_REQ;
			Req.nAccountID	= 0;
			Req.nExcelID	= pRst->nExcelID;
			Req.nInstanceID	= pRst->nInstanceID;
			Req.nExt[0]		= 0;
			Req.nExt[1]		= 1;
			g_pLPNet->SendCmdToDB(&Req, sizeof(Req));
			}
		}
		break;
	case CMDID_REPLACE_EQUIP_RPL:
		{
			P_DBS4WEBUSER_REPLACE_EQUIP_T*	pRst	= (P_DBS4WEBUSER_REPLACE_EQUIP_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_REPLACE_EQUIP_T) + sizeof(HeroDesc));
			STC_GAMECMD_REPLACE_EQUIP_T*	pCmd		= (STC_GAMECMD_REPLACE_EQUIP_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_REPLACE_EQUIP;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(HeroDesc));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_WORLDGOLDMINE_RPL:
		{
			P_DBS4WEBUSER_GET_WORLDGOLDMINE_T*	pRst	= (P_DBS4WEBUSER_GET_WORLDGOLDMINE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_WORLDGOLDMINE_T) + pRst->nNum*sizeof(WorldGoldMineDesc));
			STC_GAMECMD_GET_WORLDGOLDMINE_T	*pCmd	= (STC_GAMECMD_GET_WORLDGOLDMINE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_WORLDGOLDMINE;
			pCmd->nRst		= pRst->nRst;
			pCmd->nArea		= pRst->nArea;
			pCmd->nNum		= pRst->nNum;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(WorldGoldMineDesc));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_CONFIG_WORLDGOLDMINE_HERO_RPL:
		{
			P_DBS4WEBUSER_CONFIG_WORLDGOLDMINE_HERO_T*	pRst	= (P_DBS4WEBUSER_CONFIG_WORLDGOLDMINE_HERO_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T));
			STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T*	pCmd		= (STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO;
			pCmd->nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ROB_WORLDGOLDMINE_RPL:
		{
			P_DBS4WEBUSER_ROB_WORLDGOLDMINE_T*	pRst	= (P_DBS4WEBUSER_ROB_WORLDGOLDMINE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ROB_WORLDGOLDMINE_T));
			STC_GAMECMD_ROB_WORLDGOLDMINE_T*	pCmd		= (STC_GAMECMD_ROB_WORLDGOLDMINE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_ROB_WORLDGOLDMINE;
			pCmd->nRst		= pRst->nRst;
			pCmd->nCombatID	= pRst->nCombatID;
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_DROP_WORLDGOLDMINE_RPL:
		{
			P_DBS4WEBUSER_DROP_WORLDGOLDMINE_T*	pRst	= (P_DBS4WEBUSER_DROP_WORLDGOLDMINE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_DROP_WORLDGOLDMINE_T));
			STC_GAMECMD_DROP_WORLDGOLDMINE_T*	pCmd		= (STC_GAMECMD_DROP_WORLDGOLDMINE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_DROP_WORLDGOLDMINE;
			pCmd->nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_DEAL_WORLDGOLDMINE_GAIN_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_WORLDGOLDMINE_GAIN_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_WORLDGOLDMINE_GAIN_TE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GAIN_WORLDGOLDMINE_T));
			STC_GAMECMD_GAIN_WORLDGOLDMINE_T*	pCmd		= (STC_GAMECMD_GAIN_WORLDGOLDMINE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GAIN_WORLDGOLDMINE;
			pCmd->nRst		= pRst->nRst;
			pCmd->nArea		= pRst->nArea;
			pCmd->nClass	= pRst->nClass;
			pCmd->nIdx		= pRst->nIdx;
			pCmd->nGold		= pRst->nGold;
			SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			// 更新LP内存数据
			PlayerData*	pPlayer		= GetPlayerData(pRst->nAccountID);
			if (pPlayer)
			{
				pPlayer->m_CharAtb.nGold += pRst->nGold;
			}
		}
		break;
	case CMDID_MY_WORLDGOLDMINE_RPL:
		{
			P_DBS4WEBUSER_MY_WORLDGOLDMINE_T*	pRst	= (P_DBS4WEBUSER_MY_WORLDGOLDMINE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_MY_WORLDGOLDMINE_T));
			STC_GAMECMD_MY_WORLDGOLDMINE_T*	pCmd		= (STC_GAMECMD_MY_WORLDGOLDMINE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_MY_WORLDGOLDMINE;
			pCmd->nRst		= pRst->nRst;
			pCmd->nArea		= pRst->nArea;
			pCmd->nClass	= pRst->nClass;
			pCmd->nIdx		= pRst->nIdx;
			pCmd->nSafeTime	= pRst->nSafeTime;
			pCmd->n1Hero	= pRst->n1Hero;
			pCmd->n2Hero	= pRst->n2Hero;
			pCmd->n3Hero	= pRst->n3Hero;
			pCmd->n4Hero	= pRst->n4Hero;
			pCmd->n5Hero	= pRst->n5Hero;
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_COMBAT_PROF_RPL:
		{
			P_DBS4WEBUSER_COMBAT_PROF_T*	pRst	= (P_DBS4WEBUSER_COMBAT_PROF_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_COMBAT_PROF_T) + sizeof(CombatProf));
			STC_GAMECMD_COMBAT_PROF_T*	pCmd		= (STC_GAMECMD_COMBAT_PROF_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_COMBAT_PROF;
			pCmd->nRst		= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), sizeof(CombatProf));
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_LOAD_ARMY_RPL:
		{
			P_DBS4WEBUSER_LOAD_ARMY_T *pRst	= (P_DBS4WEBUSER_LOAD_ARMY_T*)pBaseCmd;
			ArmyDesc *pSrc	= (ArmyDesc*)wh_getptrnexttoptr(pRst);
			ArmyDesc *pDst	= &m_pGameMngS->m_army[1];
			memcpy(pDst, pSrc, sizeof(ArmyDesc)*5);
		}
		break;
	case CMDID_NEWMAIL_NOTIFY:
		{
			P_DBS4WEBUSER_NEWMAIL_NOTIFY_T *pRst	= (P_DBS4WEBUSER_NEWMAIL_NOTIFY_T*)pBaseCmd;
			P_DBS4WEB_GET_PRIVATE_MAIL_1_T	Cmd;
			Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
			Cmd.nSubCmd		= CMDID_GET_PRIVATE_MAIL_1_REQ;
			Cmd.nAccountID	= pRst->nAccountID;
			Cmd.nMailID		= pRst->nMailID;
			g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_KICK_INSTANCE_RPL:
		{
			P_DBS4WEBUSER_KICK_INSTANCE_T*	pRst	= (P_DBS4WEBUSER_KICK_INSTANCE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_KICK_INSTANCE_T));
			STC_GAMECMD_KICK_INSTANCE_T*	pCmd		= (STC_GAMECMD_KICK_INSTANCE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_KICK_INSTANCE;
			pCmd->nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			PlayerData*	pPlayer	= GetPlayerData(pRst->nObjID);
			if (pPlayer != NULL)
			{
				pPlayer->m_nGroupID		= 0;
				SetPlayerTag64(pPlayer->m_nPlayerGID, pRst->nInstanceID, true);
			}

			// 通知被踢玩家
			if (pRst->nRst == STC_GAMECMD_JOIN_INSTANCE_T::RST_OK)
			{
			STC_GAMECMD_QUIT_INSTANCE_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_QUIT_INSTANCE;
			Cmd.nRst		= STC_GAMECMD_QUIT_INSTANCE_T::RST_OK;
			Cmd.nInstanceID	= pRst->nInstanceID;
			SendMsgToClient(pRst->nObjID, &Cmd, sizeof(Cmd));
			}
			// 通知全队
			if (pRst->nRst == STC_GAMECMD_JOIN_INSTANCE_T::RST_OK)
			{
			P_DBS4WEB_GET_INSTANCEDATA_T Req;
			Req.nCmd		= P_DBS4WEB_REQ_CMD;
			Req.nSubCmd		= CMDID_GET_INSTANCEDATA_REQ;
			Req.nAccountID	= 0;
			Req.nExcelID	= pRst->nExcelID;
			Req.nInstanceID	= pRst->nInstanceID;
			Req.nExt[0]		= 0;
			Req.nExt[1]		= 1;
			g_pLPNet->SendCmdToDB(&Req, sizeof(Req));
			}
		}
		break;
	case CMDID_START_INSTANCE_RPL:
		{
			P_DBS4WEBUSER_START_INSTANCE_T*	pRst	= (P_DBS4WEBUSER_START_INSTANCE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_START_INSTANCE_T));
			STC_GAMECMD_START_INSTANCE_T*	pCmd		= (STC_GAMECMD_START_INSTANCE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_START_INSTANCE;
			pCmd->nRst		= pRst->nRst;
			pCmd->n1Hero	= pRst->n1Hero;
			pCmd->n2Hero	= pRst->n2Hero;
			pCmd->n3Hero	= pRst->n3Hero;
			pCmd->n4Hero	= pRst->n4Hero;
			pCmd->n5Hero	= pRst->n5Hero;
//			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			if (pRst->nRst == STC_GAMECMD_START_INSTANCE_T::RST_OK)
			{
			// 成功就通知全队
			SendMsgToClient(pCmd->n1Hero, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			SendMsgToClient(pCmd->n2Hero, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			SendMsgToClient(pCmd->n3Hero, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			SendMsgToClient(pCmd->n4Hero, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			SendMsgToClient(pCmd->n5Hero, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
			else
			{
			// 失败就只给队长回执
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
		}
		break;
	case CMDID_PREPARE_INSTANCE_RPL:
		{
			P_DBS4WEBUSER_PREPARE_INSTANCE_T*	pRst	= (P_DBS4WEBUSER_PREPARE_INSTANCE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_PREPARE_INSTANCE_T));
			STC_GAMECMD_PREPARE_INSTANCE_T*	pCmd		= (STC_GAMECMD_PREPARE_INSTANCE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_PREPARE_INSTANCE;
			pCmd->nRst		= pRst->nRst;
			pCmd->nStatus	= pRst->nStatus;
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			if (pRst->nRst == STC_GAMECMD_PREPARE_INSTANCE_T::RST_OK)
			{
			P_DBS4WEB_GET_INSTANCEDATA_T Req;
			Req.nCmd		= P_DBS4WEB_REQ_CMD;
			Req.nSubCmd		= CMDID_GET_INSTANCEDATA_REQ;
			Req.nAccountID	= 0;
			Req.nExcelID	= pRst->nExcelID;
			Req.nInstanceID	= pRst->nInstanceID;
			Req.nExt[0]		= 0;
			Req.nExt[1]		= 1;
			g_pLPNet->SendCmdToDB(&Req, sizeof(Req));
			}
		}
		break;
	case CMDID_SUPPLY_INSTANCE_RPL:
		{
			P_DBS4WEBUSER_SUPPLY_INSTANCE_T*	pRst	= (P_DBS4WEBUSER_SUPPLY_INSTANCE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_SUPPLY_INSTANCE_T));
			STC_GAMECMD_SUPPLY_INSTANCE_T*	pCmd		= (STC_GAMECMD_SUPPLY_INSTANCE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_SUPPLY_INSTANCE;
			pCmd->nRst		= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());

			// 草药消耗
			tty_id_t *pID	= (tty_id_t*)&pRst->nAccountID1;
			int *pDrug		= (int*)&pRst->nDrug1;
			STC_GAMECMD_SUPPLY_DRUG_T Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_SUPPLY_DRUG;
			Cmd.nNum		= 0;
			if (pRst->nExcelID == 100)
			{
				// 单人副本
				for (int i=0; i<5; ++i)
				{
					if (*pID>0)
					{
						Cmd.nNum	= *pDrug;
					}
					++ pID;
					++ pDrug;
				}

				SendMsgToClient(pRst->nAccountID1, &Cmd, sizeof(Cmd));
			}
			else
			{
				// 多人副本/单独ID
				for (int i=0; i<5; ++i)
				{
					if (*pID>0)
					{
						Cmd.nNum	= *pDrug;
						SendMsgToClient(*pID, &Cmd, sizeof(Cmd));
					}
					++ pID;
					++ pDrug;
				}
			}

			// 通知全队
//			if (pRst->nRst == STC_GAMECMD_SUPPLY_INSTANCE_T::RST_OK && 
//				pRst->nExt[1] == 0)
			if (pRst->nRst == STC_GAMECMD_SUPPLY_INSTANCE_T::RST_OK)
			{
			P_DBS4WEB_GET_INSTANCEDATA_T Req;
			Req.nCmd		= P_DBS4WEB_REQ_CMD;
			Req.nSubCmd		= CMDID_GET_INSTANCEDATA_REQ;
			Req.nAccountID	= 0;
			Req.nExcelID	= pRst->nExcelID;
			Req.nInstanceID	= pRst->nInstanceID;
			Req.nExt[0]		= 0;
			Req.nExt[1]		= 1;
			g_pLPNet->SendCmdToDB(&Req, sizeof(Req));
			}
		}
		break;
	case CMDID_GET_CITYDEFENSE_RPL:
		{
			P_DBS4WEBUSER_GET_CITYDEFENSE_T*	pRst	= (P_DBS4WEBUSER_GET_CITYDEFENSE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_CITYDEFENSE_T));
			STC_GAMECMD_GET_CITYDEFENSE_T*	pCmd		= (STC_GAMECMD_GET_CITYDEFENSE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_CITYDEFENSE;
			pCmd->nRst		= pRst->nRst;
			pCmd->n1Hero	= pRst->n1Hero;
			pCmd->n2Hero	= pRst->n2Hero;
			pCmd->n3Hero	= pRst->n3Hero;
			pCmd->n4Hero	= pRst->n4Hero;
			pCmd->n5Hero	= pRst->n5Hero;
			pCmd->nAutoSupply= pRst->nAutoSupply;
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_USE_DRUG_RPL:
		{
			P_DBS4WEBUSER_USE_DRUG_T*	pRst	= (P_DBS4WEBUSER_USE_DRUG_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_USE_DRUG_T));
			STC_GAMECMD_USE_DRUG_T*	pCmd		= (STC_GAMECMD_USE_DRUG_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_USE_DRUG;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			memcpy((char*)wh_getptrnexttoptr(pCmd), (char*)wh_getptrnexttoptr(pRst), sizeof(HeroDesc)*pRst->nNum);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), sizeof(*pCmd)+sizeof(HeroDesc)*pRst->nNum);

			RefreshPlayerCharAtbFromDB(pRst->nAccountID, false);
		}
		break;
	case CMDID_COMBAT_RST:
		{
			P_DBS4WEBUSER_COMBAT_RST_T*	pRst	= (P_DBS4WEBUSER_COMBAT_RST_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_COMBAT_RST_T) + pRst->nNumA*sizeof(CombatRst));
			STC_GAMECMD_COMBAT_RST_T*	pCmd		= (STC_GAMECMD_COMBAT_RST_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_COMBAT_RST;
			pCmd->nCombatID	= pRst->nCombatID;
			pCmd->nCombatType	= pRst->nCombatType;
			pCmd->nObjID	= pRst->nObjID;
			strcpy(pCmd->szObjName, pRst->szObjName);
			pCmd->nObjHeadID	= pRst->nObjHeadID;
			pCmd->nRst		= pRst->nCombatRst;
			pCmd->nNum		= pRst->nNumA;
			pCmd->nClassID	= pRst->nClassID;
			CombatRst *pD = (CombatRst*)wh_getptrnexttoptr(pCmd);
			CombatRst *pS = (CombatRst*)wh_getptrnexttoptr(pRst);
			memcpy(pD, pS, sizeof(CombatRst)*pRst->nNumA);

			// 通知client战斗结果
			switch (pRst->nCombatType)
			{
			case COMBAT_PVE_RAID:
			case COMBAT_PVE_WORLDGOLDMINE:
				{
					// 普通战斗
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

					// 更新LP内存数据
					RefreshPlayerCharAtbFromDB(pRst->nAccountID);
				}break;
			case COMBAT_PVP_ATTACK:
			case COMBAT_PVP_WORLDGOLDMINE_ATTACK:
				{
					// 玩家对战
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					// 包内容要改变一下
					m_vectrawbuf.resize(sizeof(STC_GAMECMD_COMBAT_RST_T) + pRst->nNumD*sizeof(CombatRst));
					// 发给对方的话，战斗类型就+1
					pCmd->nCombatType	= pRst->nCombatType + 1;
					pCmd->nObjID	= pRst->nAccountID;
					pCmd->nRst	= pCmd->nRst==COMBAT_RST_VICTORY? COMBAT_RST_LOSE: COMBAT_RST_VICTORY;
					pCmd->nNum		= pRst->nNumD;
					pD = (CombatRst*)wh_getptrnexttoptr(pCmd);
					pS = (CombatRst*)wh_getptrnexttoptr(pRst)+pRst->nNumA;
					memcpy(pD, pS, sizeof(CombatRst)*pRst->nNumD);
					SendMsgToClient(pRst->nObjID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

					// 添加仇人关系
					if (pRst->nCombatType==COMBAT_PVP_ATTACK)
					{
						m_pGameMngS->AddEnemy(pRst->nAccountID, pRst->nObjID);

						// 给联盟添加日志
						PlayerCard*	pCard			= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
						PlayerCard*	pObjCard		= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nObjID);
						if (pCard!=NULL && pObjCard!=NULL && pObjCard->nAllianceID!=0)
						{
							AllianceData_T*	pAlliance	= m_pGameMngS->GetAllianceMng()->GetAlliance(pObjCard->nAllianceID);
							if (pAlliance != NULL)
							{
								char		szBuf[1024]	= "";
								int			nLength		= 0;
								ExcelText*	pText	= m_pGameMngS->GetExcelTextMngS()->GetText(text_id_attack_alliance);
								if (pText!=NULL)
								{
									nLength	= sprintf(szBuf, pText->szText, pCard->szName, pObjCard->szName);
									for (int i=0; i<pAlliance->members.size(); i++)
									{
										if (pAlliance->members[i].nAccountID != pObjCard->nAccountID)
										{
											AddMail(0, "", pAlliance->members[i].nAccountID, false, MAIL_TYPE_ALLIANCE_EVENT, MAIL_FLAG_NORMAL, false, nLength, szBuf, 0, NULL, NULL);
										}
									}
								}
							}
						}
					}

					// 更新LP内存数据
					RefreshPlayerCharAtbFromDB(pRst->nAccountID);
					RefreshPlayerCharAtbFromDB(pRst->nObjID);
				}break;
			case COMBAT_INSTANCE_BAIZHANBUDAI:
			case COMBAT_INSTANCE_NANZHENGBEIZHAN:
				{
					// 副本
					SendMsgToClient(pRst->nAccountID1, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					SendMsgToClient(pRst->nAccountID2, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					SendMsgToClient(pRst->nAccountID3, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					SendMsgToClient(pRst->nAccountID4, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					SendMsgToClient(pRst->nAccountID5, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

					// 更新LP内存数据
					RefreshPlayerCharAtbFromDB(pRst->nAccountID1);
					RefreshPlayerCharAtbFromDB(pRst->nAccountID2);
					RefreshPlayerCharAtbFromDB(pRst->nAccountID3);
					RefreshPlayerCharAtbFromDB(pRst->nAccountID4);
					RefreshPlayerCharAtbFromDB(pRst->nAccountID5);
				}break;
			case COMBAT_WORLDCITY:
				{
					// 世界名城
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

					// 更新LP内存数据
					RefreshPlayerCharAtbFromDB(pRst->nAccountID);

					// 战斗胜利时	pRst->nCombatRst==1
					// 世界名城ID	pRst->nWorldCityID
					// 世界名字剩余	pRst->nWorldCityRes
					if (pRst->nCombatRst==1 && pRst->nWorldCityRes==0)
					{
						unsigned int	nPosX	= (unsigned int)(pRst->nWorldCityID>>32);
						unsigned int	nPosY	= (unsigned int)(pRst->nWorldCityID&0xFFFFFFFF);
						int				nRst	= m_pGameMngS->GetWorldMng()->ClearWorldFamousCity(nPosX, nPosY);
						GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_CITY)"ClearOne,0x%"WHINT64PRFX"X,%u,%u,%d", pRst->nWorldCityID, nPosX, nPosY, nRst);
					}
				}break;
			case COMBAT_PVE_WORLD_RES:
				{
					// 普通战斗
					SendMsgToClient(pRst->nAccountID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());

					// 更新LP内存数据
					RefreshPlayerCharAtbFromDB(pRst->nAccountID);

					if (pRst->nCombatRst == 1)
					{
						unsigned int	nPosX	= (unsigned int)(pRst->nObjID>>32);
						unsigned int	nPosY	= (unsigned int)(pRst->nObjID&0xFFFFFFFF);
						int				nRst	= m_pGameMngS->GetWorldMng()->ClearWorldRes(nPosX, nPosY);
						GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_RES)"ClearOne,0x%"WHINT64PRFX"X,%u,%u,%d", pRst->nObjID, nPosX, nPosY, nRst);
					
						STC_GAMECMD_WORLD_RES_CLEAR_T	Cmd;
						Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
						Cmd.nGameCmd			= STC_GAMECMD_WORLD_RES_CLEAR;
						Cmd.nPosX				= nPosX;
						Cmd.nPosY				= nPosY;
						SendCmdToAllClient(&Cmd, sizeof(Cmd));
					}
				}break;
			}
		}
		break;
	case CMDID_FIXHOUR_RPL:
		{
			P_DBS4WEBUSER_FIXHOUR_RST_T*	pRst	= (P_DBS4WEBUSER_FIXHOUR_RST_T*)pBaseCmd;
			if (pRst->nType == FIXHOUR_TYPE_DRUG)
			{
				AcctDrugDesc *pDrug	= (AcctDrugDesc*)wh_getptrnexttoptr(pRst);
				STC_GAMECMD_SUPPLY_DRUG_T Cmd;
				Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				Cmd.nGameCmd	= STC_GAMECMD_SUPPLY_DRUG;
				for (int i=0; i<pRst->u.nNum; ++i)
				{
					Cmd.nNum	= pDrug->nNum;
					SendMsgToClient(pDrug->nID, &Cmd, sizeof(Cmd));
					++ pDrug;
				}
			}
		}
		break;
	case CMDID_PVP_RST:
		{
			P_DBS4WEBUSER_PVP_RST_T *pRst	= (P_DBS4WEBUSER_PVP_RST_T*)pBaseCmd;
			/*
			STC_GAMECMD_PVP_RST_T Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_PVP_RST;
			Cmd.nCombatID	= pRst->nCombatID;
			Cmd.nCombatRst	= pRst->nCombatRst;
			Cmd.nGold		= pRst->nGold;
			Cmd.nMan		= pRst->nMan;
			SendMsgToClient(pRst->nAccountID, &Cmd, sizeof(Cmd));
			// 
			Cmd.nCombatRst	= Cmd.nCombatRst==COMBAT_RST_VICTORY? COMBAT_RST_LOSE: COMBAT_RST_VICTORY;
			SendMsgToClient(pRst->nObjID, &Cmd, sizeof(Cmd));
			*/
			
			if (pRst->nGold>0)
			{
			AddGoldProduceLog(pRst->nAccountID, gold_produce_raid, pRst->nGold);
			AddGoldCostLog(pRst->nObjID, gold_cost_be_raided, pRst->nGold);
			}
		}break;
	case CMDID_ARMY_ACCELERATE_RPL:
		{
			P_DBS4WEBUSER_ARMY_ACCELERATE_T *pRst	= (P_DBS4WEBUSER_ARMY_ACCELERATE_T*)pBaseCmd;
			STC_GAMECMD_ARMY_ACCELERATE_T Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ARMY_ACCELERATE;
			Cmd.nCombatID	= pRst->nCombatID;
			Cmd.nRst		= pRst->nRst;
			Cmd.nCostCrystal= pRst->nCostCrystal;
			Cmd.byType		= pRst->byType;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			// 更新LP内存数据
			RefreshPlayerCharAtbFromDB(pRst->nAccountID);

			// 水晶消耗日志
			int nCostType = pRst->byType==2? money_type_crystal: money_type_diamond;
			if (pRst->nRst==STC_GAMECMD_ARMY_ACCELERATE_T::RST_OK)
			{
				AddMoneyCostLog(pRst->nAccountID, money_use_type_accelerate_march, nCostType, pRst->nCostCrystal, pRst->nFinTime);
			}
		}break;
	case CMDID_ARMY_BACK_RPL:
		{
			P_DBS4WEBUSER_ARMY_BACK_T *pRst	= (P_DBS4WEBUSER_ARMY_BACK_T*)pBaseCmd;
			STC_GAMECMD_ARMY_BACK_T Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ARMY_BACK;
			Cmd.nCombatID	= pRst->nCombatID;
			Cmd.nRst		= pRst->nRst;
			Cmd.nBackTime	= pRst->nBackTime;
//			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
			SendMsgToClient(pRst->nAccountID, &Cmd, sizeof(Cmd));
		}break;
	case CMDID_SENDALLCLIENT:
		{
			P_DBS4WEBUSER_SENDALLCLIENT_T *pRst	= (P_DBS4WEBUSER_SENDALLCLIENT_T*)pBaseCmd;
			m_vectrawbuf.resize(wh_offsetof(STC_GAMECMD_CHAT_WORLD_T, szText) + pRst->nTextLen);

			switch (pRst->nType)
			{
			case	STC_GAMECMD_WORLD_NOTIFY:
				{
				STC_GAMECMD_CHAT_WORLD_T*	pChatTo	= (STC_GAMECMD_CHAT_WORLD_T*)m_vectrawbuf.getbuf();
				/*
				pChatTo->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				pChatTo->nGameCmd	= pRst->nType;
				pChatTo->nChatID	= m_pGameMngS->GetChatMng()->NewChatID();
				pChatTo->nAccountID	= pRst->nAccountID;
				pChatTo->nTime		= wh_time();
				pChatTo->nVip		= 0;
				pChatTo->nTextLen	= pRst->nTextLen;
				WH_STRNCPY0(pChatTo->szFrom, "");
				char *p = (char*)wh_getptrnexttoptr(pRst);
				memcpy(pChatTo->szText, p, pRst->nTextLen);
				g_pLPNet->SendCmdToAllClient(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				m_pGameMngS->GetChatMng()->AddWorldChatEntry(m_vectrawbuf.size(), m_vectrawbuf.getbuf(), pChatTo->nTime);
				*/
				AddNotifyInWorldChat(pRst->nTextLen, (char*)wh_getptrnexttoptr(pRst), pRst->nAccountID);
				}break;
			case	STC_GAMECMD_CHAT_ALLIANCE:
				{
				STC_GAMECMD_CHAT_ALLIANCE_T*	pChatTo	= (STC_GAMECMD_CHAT_ALLIANCE_T*)m_vectrawbuf.getbuf();
				pChatTo->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				pChatTo->nGameCmd	= pRst->nType;
				pChatTo->nChatID	= m_pGameMngS->GetChatMng()->NewChatID();
				pChatTo->nAccountID	= pRst->nAccountID;
				pChatTo->nTime		= wh_time();
				pChatTo->nVip		= 0;
				pChatTo->nTextLen	= pRst->nTextLen;
				WH_STRNCPY0(pChatTo->szFrom, "");
				char *p = (char*)wh_getptrnexttoptr(pRst);
				memcpy(pChatTo->szText, p, pRst->nTextLen);
				g_pLPNet->SendCmdToClientWithTag64(pRst->nAllianceID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				m_pGameMngS->GetChatMng()->AddAllianceChatEntry(pRst->nAllianceID, m_vectrawbuf.size(), m_vectrawbuf.getbuf(), pChatTo->nTime);
				}break;
			}
		}break;
	case CMDID_WORLDCITY_GET_RPL:
		{
			P_DBS4WEBUSER_WORLDCITY_GET_T*	pRst	= (P_DBS4WEBUSER_WORLDCITY_GET_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_WORLDCITY_GET_T) + pRst->nNum*sizeof(WorldCity));
			STC_GAMECMD_WORLDCITY_GET_T*	pCmd		= (STC_GAMECMD_WORLDCITY_GET_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_WORLDCITY_GET;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			pCmd->nRefreshTime	= pRst->nRefreshTime;
			pCmd->nBonusTime	= pRst->nBonusTime;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(WorldCity));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_WORLDCITY_GETLOG_RPL:
		{
			P_DBS4WEBUSER_WORLDCITY_GETLOG_T*	pRst	= (P_DBS4WEBUSER_WORLDCITY_GETLOG_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_WORLDCITY_GETLOG_T) + pRst->nNum*sizeof(WorldCityLog));
			STC_GAMECMD_WORLDCITY_GETLOG_T*	pCmd		= (STC_GAMECMD_WORLDCITY_GETLOG_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_WORLDCITY_GETLOG;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			pCmd->nMaxPage	= pRst->nMaxPage;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(WorldCityLog));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_WORLDCITY_GETLOG_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_WORLDCITY_GETLOG_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_WORLDCITY_GETLOG_ALLIANCE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T) + pRst->nNum*sizeof(WorldCityLogAlliance));
			STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T*	pCmd		= (STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			pCmd->nMaxPage	= pRst->nMaxPage;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(WorldCityLogAlliance));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_WORLDCITY_RANK_MAN_RPL:
		{
			P_DBS4WEBUSER_WORLDCITY_RANK_MAN_T*	pRst	= (P_DBS4WEBUSER_WORLDCITY_RANK_MAN_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_WORLDCITY_RANK_MAN_T) + pRst->nNum*sizeof(WorldCityRankMan));
			STC_GAMECMD_WORLDCITY_RANK_MAN_T*	pCmd		= (STC_GAMECMD_WORLDCITY_RANK_MAN_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_WORLDCITY_RANK_MAN;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			pCmd->nMaxPage	= pRst->nMaxPage;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(WorldCityRankMan));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_WORLDCITY_RANK_ALLIANCE_RPL:
		{
			P_DBS4WEBUSER_WORLDCITY_RANK_ALLIANCE_T*	pRst	= (P_DBS4WEBUSER_WORLDCITY_RANK_ALLIANCE_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_WORLDCITY_RANK_ALLIANCE_T) + pRst->nNum*sizeof(WorldCityRankAlliance));
			STC_GAMECMD_WORLDCITY_RANK_ALLIANCE_T*	pCmd		= (STC_GAMECMD_WORLDCITY_RANK_ALLIANCE_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_WORLDCITY_RANK_ALLIANCE;
			pCmd->nRst		= pRst->nRst;
			pCmd->nNum		= pRst->nNum;
			pCmd->nMaxPage	= pRst->nMaxPage;
			if (pCmd->nNum > 0)
			{
				memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pCmd->nNum*sizeof(WorldCityRankAlliance));
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ATB:
		{
			P_DBS4WEBUSER_ATB_T*	pRst	= (P_DBS4WEBUSER_ATB_T*)pBaseCmd;
			RefreshPlayerCharAtbFromDB(pRst->nAccountID);
		}break;

	case CMDID_DEAL_CHAR_DAY_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_CHAR_DAY_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_CHAR_DAY_TE_T*)pBaseCmd;
			RefreshPlayerCharAtbFromDB(pRst->nAccountID);
		}
		break;
	case CMDID_DEAL_ADCOLONY_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T*)pBaseCmd;
			P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T::AdColonyAward*	pAward	= (P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T::AdColonyAward*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				PlayerData*	pPlayer	= GetPlayerData(pAward->nAccountID);
				if (pPlayer != NULL)
				{
					switch (pAward->nMoneyType)
					{
					case money_type_diamond:
						{
							pPlayer->m_CharAtb.nDiamond	+=	pAward->nNum;
						}
						break;
					}

					STC_GAMECMD_ADCOLONY_AWARD_T	Cmd;
					Cmd.nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
					Cmd.nGameCmd		= STC_GAMECMD_ADCOLONY_AWARD;
					Cmd.nMoneyType		= pAward->nMoneyType;
					Cmd.nMoneyNum		= pAward->nNum;
					SendCmdToClient(pPlayer->m_nPlayerGID, &Cmd, sizeof(Cmd));
				}

				pAward++;
			}
		}
		break;
	case CMDID_DISMISS_SOLDIER_RPL:
		{
			P_DBS4WEBUSER_DISMISS_SOLDIER_T*	pRst	= (P_DBS4WEBUSER_DISMISS_SOLDIER_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_DISMISS_SOLDIER_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_DISMISS_SOLDIER;
			Cmd.nExcelID				= pRst->nExcelID;
			Cmd.nLevel					= pRst->nLevel;
			Cmd.nNum					= pRst->nNum;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_DISMISS_SOLDIER_T::RST_OK)
			{
				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.nPopulation	+= pRst->nNum;
				}
			}
		}
		break;
	case CMDID_REFRESH_MAIL_RPL:
		{
			P_DBS4WEBUSER_REFRESH_MAIL_T*	pRst	= (P_DBS4WEBUSER_REFRESH_MAIL_T*)pBaseCmd;
			if (pRst->nRst==0 && pRst->nNum>0)
			{
				// 使用缓存了
				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					PrivateMailUnit*	pMailUnit	= (PrivateMailUnit*)wh_getptrnexttoptr(pRst);
					for (unsigned int i=0; i<pRst->nNum; i++)
					{
						int	nMailSize		= wh_offsetof(PrivateMailUnit, szText) + pMailUnit->nTextLen;
						int*	pExtDataLen	= (int*)wh_getoffsetaddr(pMailUnit, nMailSize);
						void*	pExtData	= (void*)wh_getptrnexttoptr(pExtDataLen);
						nMailSize			+= (sizeof(int) + *pExtDataLen);
						PrivateMailUnit*	pNewMail= (PrivateMailUnit*)(new char[nMailSize]);
						memcpy(pNewMail, pMailUnit, nMailSize);
						pPlayer->AddNewMail(pNewMail);

						// 通知客户端新邮件到来
						{
							m_vectrawbuf.resize(sizeof(STC_GAMECMD_NEW_PRIVATE_MAIL_T) + nMailSize);
							STC_GAMECMD_NEW_PRIVATE_MAIL_T*	pNewMailCmd	= (STC_GAMECMD_NEW_PRIVATE_MAIL_T*)m_vectrawbuf.getbuf();
							pNewMailCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
							pNewMailCmd->nGameCmd			= STC_GAMECMD_NEW_PRIVATE_MAIL;
							pNewMailCmd->nTotalMailNum		= 0;
							PrivateMailUnit*	pMail	= (PrivateMailUnit*)wh_getptrnexttoptr(pNewMailCmd);
							// 不能用pNewMail,pNewMail可能被AddNewMail删除
							memcpy(pMail, pMailUnit, nMailSize);
							unsigned char	nMailType	= 0;
							switch (pMail->nType)
							{
							case MAIL_TYPE_PRIVATE:
							case MAIL_TYPE_ALLIANCE:
								{
									nMailType			= MAIL_TYPE_PRIVATE|MAIL_TYPE_ALLIANCE;
								}
								break;
							default:
								{
									nMailType			= pMail->nType;
								}
								break;
							}
							whlist<PrivateMailUnit*>::iterator	it		= pPlayer->m_MailList.begin();
							for (; it!=pPlayer->m_MailList.end(); ++it)
							{
								if (((*it)->nType&nMailType) != 0)
								{
									pNewMailCmd->nTotalMailNum++;
								}
							}
							SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
						}

						pMailUnit			= (PrivateMailUnit*)wh_getoffsetaddr(pMailUnit, nMailSize);
					}
				}
			}
		}
		break;
	case CMDID_BAN_CHAR_RPL:
		{
			P_DBS4WEBUSER_BAN_CHAR_T*	pRst	= (P_DBS4WEBUSER_BAN_CHAR_T*)pBaseCmd;
			GS4WEB_GROUP_BAN_CHAR_RPL_T	Cmd;
			Cmd.nCmd		= P_GS4WEB_GROUP_RPL_CMD;
			Cmd.nSubCmd		= GROUP_BAN_CHAR_RPL;
			Cmd.nRst		= pRst->nRst;
			Cmd.nAccountID	= pRst->nAccountID;
			Cmd.nGroupID	= m_cfginfo.nSvrGrpID;
			memcpy(Cmd.nExt, pRst->nExt, sizeof(Cmd.nExt));
			m_msgerGS4Web.SendMsg(&Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_KICK_CLIENT_ALL_RPL:
		{
			P_DBS4WEBUSER_KICK_CLIENT_ALL_T*	pRst	= (P_DBS4WEBUSER_KICK_CLIENT_ALL_T*)pBaseCmd;
			GS4WEB_GROUP_KICK_CLIENT_ALL_RPL_T	Cmd;
			Cmd.nCmd		= P_GS4WEB_GROUP_RPL_CMD;
			Cmd.nSubCmd		= GROUP_KICK_CLIENT_ALL_RPL;
			Cmd.nRst		= pRst->nRst;
			Cmd.nGroupID	= m_cfginfo.nSvrGrpID;
			memcpy(Cmd.nExt, pRst->nExt, sizeof(Cmd.nExt));
			m_msgerGS4Web.SendMsg(&Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_NEW_ADCOLONY_RPL:
		{
			P_DBS4WEBUSER_NEW_ADCOLONY_T*	pRst	= (P_DBS4WEBUSER_NEW_ADCOLONY_T*)pBaseCmd;
			GS4WEB_GROUP_NEW_ADCOLONY_RPL_T	Cmd;
			Cmd.nCmd		= P_GS4WEB_GROUP_RPL_CMD;
			Cmd.nSubCmd		= GROUP_NEW_ADCOLONY_RPL;
			Cmd.nRst		= 0;
			Cmd.nTransactionID	= pRst->nTransactionID;
			m_msgerGS4Web.SendMsg(&Cmd, sizeof(Cmd));
		}
		break;

	case CMDID_ALLI_DONATE_SOLDIER_RPL:
		{
			P_DBS4WEBUSER_ALLI_DONATE_SOLDIER_T*	pRst	= (P_DBS4WEBUSER_ALLI_DONATE_SOLDIER_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER;
			Cmd.nRst					= pRst->nRst;
			Cmd.nEventID				= pRst->nEventID;
			Cmd.nExcelID				= pRst->nExcelID;
			Cmd.nLevel					= pRst->nLevel;
			Cmd.nNum					= pRst->nNum;
			Cmd.nObjID					= pRst->nObjID;
			Cmd.nTime					= pRst->nTime;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T::RST_OK)
			{
				PlayerData*	pPlayer			= GetPlayerData(pRst->nObjID);
				if (pPlayer != NULL)
				{
					P_DBS4WEB_GET_DONATE_SOLDIER_QUEUE_T	Cmd;
					Cmd.nCmd					= P_DBS4WEB_REQ_CMD;
					Cmd.nSubCmd					= CMDID_GET_DONATE_SOLDIER_QUEUE_REQ;
					Cmd.nAccountID				= pPlayer->m_CharAtb.nAccountID;
					Cmd.nExt[0]					= pPlayer->m_nPlayerGID;
					g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));
				}
			}
		}
		break;
	case CMDID_ALLI_RECALL_SOLDIER_RPL:
		{
			P_DBS4WEBUSER_ALLI_RECALL_SOLDIER_T*	pRst	= (P_DBS4WEBUSER_ALLI_RECALL_SOLDIER_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER;
			Cmd.nNewEventID				= pRst->nNewEventID;
			Cmd.nOldEventID				= pRst->nOldEventID;
			Cmd.nRst					= pRst->nRst;
			Cmd.nTime					= pRst->nTime;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T::RST_OK)
			{
				PlayerData*	pPlayer			= GetPlayerData(pRst->nObjID);
				if (pPlayer != NULL)
				{
					P_DBS4WEB_GET_DONATE_SOLDIER_QUEUE_T	DBCmd;
					DBCmd.nCmd				= P_DBS4WEB_REQ_CMD;
					DBCmd.nSubCmd			= CMDID_GET_DONATE_SOLDIER_QUEUE_REQ;
					DBCmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
					DBCmd.nExt[0]			= pPlayer->m_nPlayerGID;
					g_pLPNet->SendCmdToDB(&DBCmd, sizeof(DBCmd));
				}
			}
		}
		break;
	case CMDID_ALLI_ACCE_SOLDIER_RPL:
		{
			P_DBS4WEBUSER_ALLI_ACCE_SOLDIER_T*	pRst	= (P_DBS4WEBUSER_ALLI_ACCE_SOLDIER_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER;
			Cmd.nEventID				= pRst->nEventID;
			Cmd.nMoney					= pRst->nMoney;
			Cmd.nMoneyType				= pRst->nMoneyType;
			Cmd.nRst					= pRst->nRst;
			Cmd.nTime					= pRst->nTime;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (Cmd.nRst == STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T::RST_OK)
			{
				PlayerData*	pPlayer		= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					switch (pRst->nMoneyType)
					{
					case money_type_diamond:
						{
							if (pPlayer->m_CharAtb.nDiamond > pRst->nMoney)
							{
								pPlayer->m_CharAtb.nDiamond	-= pRst->nMoney;
							}
							else
							{
								pPlayer->m_CharAtb.nDiamond	= 0;
							}
						}
						break;
					case money_type_crystal:
						{
							if (pPlayer->m_CharAtb.nCrystal > pRst->nMoney)
							{
								pPlayer->m_CharAtb.nCrystal	-= pRst->nMoney;
							}
							else
							{
								pPlayer->m_CharAtb.nCrystal	= 0;
							}
						}
						break;
					}
				}

				PlayerData*	pObjPlayer			= GetPlayerData(pRst->nObjID);
				if (pObjPlayer != NULL)
				{
					P_DBS4WEB_GET_DONATE_SOLDIER_QUEUE_T	DBCmd;
					DBCmd.nCmd				= P_DBS4WEB_REQ_CMD;
					DBCmd.nSubCmd			= CMDID_GET_DONATE_SOLDIER_QUEUE_REQ;
					DBCmd.nAccountID		= pObjPlayer->m_CharAtb.nAccountID;
					DBCmd.nExt[0]			= pObjPlayer->m_nPlayerGID;
					g_pLPNet->SendCmdToDB(&DBCmd, sizeof(DBCmd));
				}
			}
		}
		break;
	case CMDID_GET_DONATE_SOLDIER_QUEUE_RPL:
		{
			P_DBS4WEBUSER_GET_DONATE_SOLDIER_QUEUE_T*	pRst	= (P_DBS4WEBUSER_GET_DONATE_SOLDIER_QUEUE_T*)pBaseCmd;
			unsigned int	nUnitsSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_DONATE_SOLDIER_QUEUE_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T) + nUnitsSize);
			STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T*	pRplCmd		= (STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T*)m_vectrawbuf.getbuf();
			pRplCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRplCmd->nGameCmd			= STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE;
			pRplCmd->nNum				= pRst->nNum;
			pRplCmd->nRst				= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRplCmd), wh_getptrnexttoptr(pRst), nUnitsSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_LEFT_DONATE_SOLDIER_RPL:
		{
			P_DBS4WEBUSER_GET_LEFT_DONATE_SOLDIER_T*	pRst	= (P_DBS4WEBUSER_GET_LEFT_DONATE_SOLDIER_T*)pBaseCmd;
			STC_GAMECMD_GET_LEFT_DONATE_SOLDIER_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_GET_LEFT_DONATE_SOLDIER;
			Cmd.nLeftNum				= pRst->nLeftNum;
			Cmd.nMaxNum					= pRst->nMaxNum;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_DEAL_DONATE_SOLDIER_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_DONATE_SOLDIER_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_DONATE_SOLDIER_TE_T*)pBaseCmd;
			STC_GAMECMD_DONATE_SOLDIER_TE_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_DONATE_SOLDIER_TE;
			Cmd.nEventID				= pRst->nEventID;
			Cmd.nExcelID				= pRst->nExcelID;
			Cmd.nLevel					= pRst->nLevel;
			Cmd.nNum					= pRst->nNum;
			Cmd.nType					= pRst->nType;
			switch (Cmd.nType)
			{
			case te_subtype_ads_send:
				{
					PlayerData*	pPlayer	= GetPlayerData(pRst->nObjID);
					if (pPlayer != NULL)
					{
						SendCmdToClient(pPlayer->m_nPlayerGID,&Cmd, sizeof(Cmd));
					}
				}
				break;
			case te_subtype_ads_recall:
				{
					PlayerData*	pPlayer	= GetPlayerData(pRst->nAccountID);
					if (pPlayer != NULL)
					{
						SendCmdToClient(pPlayer->m_nPlayerGID,&Cmd, sizeof(Cmd));
					}
				}
				break;
			}

			P_DBS4WEB_GET_DONATE_SOLDIER_QUEUE_T	DBCmd;
			DBCmd.nCmd					= P_DBS4WEB_REQ_CMD;
			DBCmd.nSubCmd				= CMDID_GET_DONATE_SOLDIER_QUEUE_REQ;
			DBCmd.nAccountID			= 0;
			DBCmd.nExt[0]				= 0;

			PlayerData*	pPlayer			= GetPlayerData(pRst->nAccountID);
			if (pPlayer != NULL)
			{
				DBCmd.nAccountID		= pPlayer->m_CharAtb.nAccountID;
				DBCmd.nExt[0]			= pPlayer->m_nPlayerGID;
				g_pLPNet->SendCmdToDB(&DBCmd, sizeof(DBCmd));
			}
			PlayerData*	pObjPlayer		= GetPlayerData(pRst->nObjID);
			if (pObjPlayer != NULL)
			{
				DBCmd.nAccountID		= pObjPlayer->m_CharAtb.nAccountID;
				DBCmd.nExt[0]			= pObjPlayer->m_nPlayerGID;
				g_pLPNet->SendCmdToDB(&DBCmd, sizeof(DBCmd));
			}
		}
		break;
	case CMDID_ADD_GOLD_DEAL_RPL:
		{
			P_DBS4WEBUSER_ADD_GOLD_DEAL_T*	pRst	= (P_DBS4WEBUSER_ADD_GOLD_DEAL_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_ADD_GOLD_DEAL_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_ADD_GOLD_DEAL;
			Cmd.nGold					= pRst->nGold;
			Cmd.nTransactionID			= pRst->nTransactionID;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_ADD_GOLD_DEAL_T::RST_OK)
			{
				PlayerData*	pPlayer		= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					if (pPlayer->m_CharAtb.nGold >= pRst->nGold)
					{
						pPlayer->m_CharAtb.nGold	-= pRst->nGold;
					}
					else
					{
						pPlayer->m_CharAtb.nGold	= 0;
					}
				}
			}
		}
		break;
	case CMDID_BUY_GOLD_IN_MARKET_RPL:
		{
			P_DBS4WEBUSER_BUY_GOLD_IN_MARKET_T*	pRst	= (P_DBS4WEBUSER_BUY_GOLD_IN_MARKET_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET;
			Cmd.nGold					= pRst->nGold;
			Cmd.nMoneyType				= pRst->nMoneyType;
			Cmd.nTotalPrice				= pRst->nTotalPrice;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (Cmd.nRst == STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T::RST_OK)
			{
				AddGoldProduceLog(pRst->nAccountID, gold_produce_gold_market, pRst->nGold);
				AddGoldCostLog(pRst->nSellerID, gold_cost_gold_market, pRst->nGold);
				AddMoneyCostLog(pRst->nAccountID, money_use_type_gold_market, pRst->nMoneyType, pRst->nTotalPrice, pRst->nGold);

				PlayerData*	pPlayer		= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					pPlayer->SubMoney(pRst->nMoneyType, pRst->nTotalPrice);
					pPlayer->AddMoney(money_type_gold, pRst->nGold);

					RefreshPlayerCharAtbFromDB(pRst->nAccountID);
				}

				PlayerData*	pSeller		= GetPlayerData(pRst->nSellerID);
				if (pSeller != NULL)
				{
					pSeller->AddMoney(money_type_diamond, pRst->nTotalPrice);
					pSeller->SubMoney(money_type_gold, pRst->nGold);

					RefreshPlayerCharAtbFromDB(pRst->nSellerID);
				}
			}
		}
		break;
	case CMDID_CANCEL_GOLD_DEAL_RPL:
		{
			P_DBS4WEBUSER_CANCEL_GOLD_DEAL_T*	pRst	= (P_DBS4WEBUSER_CANCEL_GOLD_DEAL_T*)pBaseCmd;
			STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL;
			Cmd.nRst					= pRst->nRst;
			Cmd.nGold					= pRst->nGold;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T::RST_OK)
			{
				PlayerData*	pPlayer			= GetPlayerData(pRst->nAccountID);
				if (pPlayer != NULL)
				{
					pPlayer->AddMoney(money_type_gold, pRst->nGold);
				}
			}
		}
		break;
	case CMDID_GET_GOLD_DEAL_RPL:
		{
			P_DBS4WEBUSER_GET_GOLD_DEAL_T*	pRst	= (P_DBS4WEBUSER_GET_GOLD_DEAL_T*)pBaseCmd;
			int	nUnitSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_GOLD_DEAL_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_GOLD_DEAL_T) + nUnitSize);
			STC_GAMECMD_GET_GOLD_DEAL_T*	pRstCmd	= (STC_GAMECMD_GET_GOLD_DEAL_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd			= STC_GAMECMD_GET_GOLD_DEAL;
			pRstCmd->nNum				= pRst->nNum;
			pRstCmd->nPage				= pRst->nPage;
			pRstCmd->nTotalNum			= pRst->nTotalNum;
			pRstCmd->nRst				= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nUnitSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_SELF_GOLD_DEAL_RPL:
		{
			P_DBS4WEBUSER_GET_SELF_GOLD_DEAL_T*	pRst	= (P_DBS4WEBUSER_GET_SELF_GOLD_DEAL_T*)pBaseCmd;
			int	nUnitSize	= nDSize - sizeof(P_DBS4WEBUSER_GET_SELF_GOLD_DEAL_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_SELF_GOLD_DEAL_T) + nUnitSize);
			STC_GAMECMD_GET_SELF_GOLD_DEAL_T*	pRstCmd	= (STC_GAMECMD_GET_SELF_GOLD_DEAL_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd			= STC_GAMECMD_GET_SELF_GOLD_DEAL;
			pRstCmd->nNum				= pRst->nNum;
			pRstCmd->nRst				= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nUnitSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_DEAL_GOLD_MARKET_TE_RPL:
		{
			P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T*	pRst	= (P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T*)pBaseCmd;
			P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T::UNIT_T*	pUnit	= (P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T::UNIT_T*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				PlayerData*	pPlayer		= GetPlayerData(pUnit->nAccountID);
				if (pPlayer != NULL)
				{
					RefreshPlayerCharAtbFromDB(pUnit->nAccountID);
				}

				pUnit++;
			}
		}
		break;
	case CMDID_GET_WORLD_RES_RPL:
		{
			P_DBS4WEBUSER_GET_WORLD_RES_T*	pRst	= (P_DBS4WEBUSER_GET_WORLD_RES_T*)pBaseCmd;
			WorldRes*	pRes				= (WorldRes*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				m_pGameMngS->GetWorldMng()->AddWorldRes(pRes);

				pRes++;
			}

			m_bAlreadyGetWorldRes			= true;
			if (m_bNeedGenWorldRes)
			{
				m_bCanGenWorldRes			= true;
				m_bNeedGenWorldRes			= false;
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_CREATE_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_CREATE_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_CREATE_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_CREATE_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_CREATE;
			Cmd.nRst		= pRst->nRst;
			Cmd.nInstanceID	= pRst->nInstanceID;

			if (pRst->nRst == STC_GAMECMD_ALLI_INSTANCE_CREATE_T::RST_OK)
			{
				m_pGameMngS->GetInstanceMng()->CreateInstance(pRst->nInstanceID, pRst->nAccountID);
			}

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_JOIN_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_JOIN_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_JOIN_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_JOIN_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_JOIN;
			Cmd.nRst		= pRst->nRst;

			if (pRst->nRst == STC_GAMECMD_ALLI_INSTANCE_JOIN_T::RST_OK)
			{
				SendAlliInstanceNotify(pRst->nInstanceID, pRst->nAccountID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_JOIN);
				m_pGameMngS->GetInstanceMng()->JoinInstance(pRst->nInstanceID, pRst->nAccountID);
			}

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_EXIT_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_EXIT_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_EXIT_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_EXIT_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_EXIT;
			Cmd.nRst		= pRst->nRst;

			if (pRst->nRst == STC_GAMECMD_ALLI_INSTANCE_EXIT_T::RST_OK)
			{
				SendAlliInstanceNotify(pRst->nInstanceID, pRst->nAccountID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_EXIT);
				m_pGameMngS->GetInstanceMng()->ExitInstance(pRst->nInstanceID, pRst->nAccountID);
			}

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_KICK_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_KICK_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_KICK_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_KICK_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_KICK;
			Cmd.nRst		= pRst->nRst;
			Cmd.nObjID		= pRst->nObjID;

			if (pRst->nRst == STC_GAMECMD_ALLI_INSTANCE_KICK_T::RST_OK)
			{
				SendAlliInstanceNotify(pRst->nInstanceID, pRst->nObjID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_KICK);
				m_pGameMngS->GetInstanceMng()->ExitInstance(pRst->nInstanceID, pRst->nObjID);
			}

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_DESTROY_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_DESTROY_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_DESTROY_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_DESTROY_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_DESTROY;
			Cmd.nRst		= pRst->nRst;

			if (pRst->nRst == STC_GAMECMD_ALLI_INSTANCE_DESTROY_T::RST_OK)
			{
				SendAlliInstanceNotify(pRst->nInstanceID, pRst->nAccountID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_DESTROY);
				m_pGameMngS->GetInstanceMng()->DestroyInstance(pRst->nInstanceID);
			}

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_CONFIG_HERO_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_CONFIG_HERO_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_CONFIG_HERO_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO;
			Cmd.nRst		= pRst->nRst;

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_START_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_START_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_START_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_START_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_START;
			Cmd.nRst		= pRst->nRst;

			if (pRst->nRst == STC_GAMECMD_ALLI_INSTANCE_START_T::RST_OK)
			{
				SendAlliInstanceNotify(pRst->nInstanceID, pRst->nAccountID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_START);
			}

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_READY_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_READY_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_READY_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_READY_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_READY;
			Cmd.nRst		= pRst->nRst;

			if (pRst->nRst == STC_GAMECMD_ALLI_INSTANCE_READY_T::RST_OK)
			{
				if (pRst->bReady)
				{
					SendAlliInstanceNotify(pRst->nInstanceID, pRst->nAccountID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_READY);
				}
				else
				{
					SendAlliInstanceNotify(pRst->nInstanceID, pRst->nAccountID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_UNREADY);
				}
			}

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_LIST_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_LIST_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_LIST_T*)pBaseCmd;
			int	nLeftDataSize	= nDSize - sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_LIST_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T) + nLeftDataSize);
			STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T*	pRstCmd	= (STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_GET_LIST;
			pRstCmd->nRst		= pRst->nRst;
			pRstCmd->nNum		= pRst->nNum;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nLeftDataSize);

			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_CHAR_DATA_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_CHAR_DATA_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_CHAR_DATA_T*)pBaseCmd;
			int	nLeftDataSize	= nDSize - sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_CHAR_DATA_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T) + nLeftDataSize);
			STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T*	pRstCmd	= (STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA;
			pRstCmd->nRst		= pRst->nRst;
			pRstCmd->nInstanceID	= pRst->nInstanceID;
			pRstCmd->nNum		= pRst->nNum;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nLeftDataSize);

			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_HERO_DATA_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_HERO_DATA_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_HERO_DATA_T*)pBaseCmd;
			int	nLeftDataSize	= nDSize - sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_HERO_DATA_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T) + nLeftDataSize);
			STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T*	pRstCmd	= (STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA;
			pRstCmd->nRst		= pRst->nRst;
			pRstCmd->nInstanceID	= pRst->nInstanceID;
			pRstCmd->nNum		= pRst->nNum;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nLeftDataSize);

			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_STATUS_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_STATUS_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_STATUS_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_GET_STATUS_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_GET_STATUS;
			Cmd.nRst		= pRst->nRst;
			Cmd.bAutoCombat	= pRst->bAutoCombat;
			Cmd.bAutoSupply	= pRst->bAutoSupply;
			Cmd.nCharStatus	= pRst->nCharStatus;
			Cmd.nCurLevel	= pRst->nCurLevel;
			Cmd.nInstanceID	= pRst->nInstanceID;
			Cmd.nInstanceStatus	= pRst->nInstanceStatus;
			Cmd.nRetryTimes		= pRst->nRetryTimes;
			Cmd.nTotalFreeTimes	= pRst->nTotalFreeTimes;
			Cmd.nUsedFreeTimes	= pRst->nUsedFreeTimes;
			Cmd.nCreatorID	= pRst->nCreatorID;
			Cmd.nStopLevel	= pRst->nStopLevel;
			Cmd.nCombatType	= pRst->nCombatType;
			Cmd.nMaxLevel	= pRst->nMaxLevel;
			Cmd.nLastCombatResult	= pRst->nLastCombatResult;

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_LOOT_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_LOOT_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_LOOT_T*)pBaseCmd;
			if (pRst->nRst == STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::RST_OK)
			{
				m_vectrawbuf.resize(sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T));
				STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*	pRstCmd	= (STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*)m_vectrawbuf.getbuf();
				pRstCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				pRstCmd->nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_GET_LOOT;
				pRstCmd->nRst		= pRst->nRst;
				pRstCmd->nNum		= 0;
				const char*	cszLoot	= (const char*)wh_getptrnexttoptr(pRst);
				if (cszLoot[0] != 0)
				{
					cszLoot			+= 1;
					int	nNum		= 0;
					int	nOffset		= 0;
					unsigned int	nExcelID	= 0;
					unsigned int	nItemNum	= 0;

					while (wh_strsplit(&nOffset, "dd", cszLoot, "*", &nExcelID, &nItemNum) == 2)
					{
						STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::LootUnit*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::LootUnit*)m_vectrawbuf.pushn_back(sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::LootUnit));
						pUnit->nExcelID			= nExcelID;
						pUnit->nNum				= nItemNum;

						nNum++;
					}

					STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*	pRstCmd	= (STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T*)m_vectrawbuf.getbuf();
					pRstCmd->nNum	= nNum;
					SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					return 0;
				}
			}

			STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_GET_LOOT;
			Cmd.nRst		= pRst->nRst;
			Cmd.nNum		= 0;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_SAVE_HERO_DEPLOY_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T		Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY;
			Cmd.nRst		= pRst->nRst;

			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_START_COMBAT_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_START_COMBAT_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_START_COMBAT_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_START_COMBAT_T	Cmd;
			Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_START_COMBAT;
			Cmd.nRst		= pRst->nRst;
			Cmd.nMarchTime	= pRst->nMarchTime;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst==STC_GAMECMD_ALLI_INSTANCE_START_COMBAT_T::RST_OK 
				&& pRst->nCombatType==COMBAT_ALLI_INSTANCE)
			{
				SendAlliInstanceNotify(pRst->nObjID, pRst->nAccountID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_START_COMBAT);
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T*)pBaseCmd;
			tty_id_t*	pID			= (tty_id_t*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				P_DBS4WEB_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T	Cmd;
				Cmd.nCmd			= P_DBS4WEB_REQ_CMD;
				Cmd.nSubCmd			= CMDID_ALLI_INSTANCE_DEAL_COMBAT_RESULT_REQ;
				Cmd.nEventID		= *pID;
				SendCmdToDB(&Cmd, sizeof(Cmd));

				pID++;
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_DEAL_MARCH_BACK_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_MARCH_BACK_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_MARCH_BACK_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_BACK_T	Cmd;
			Cmd.nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd			= STC_GAMECMD_ALLI_INSTANCE_BACK;
			Cmd.bInstanceDestroyed	= pRst->bInstanceDestroyed;
			Cmd.nCombatType			= pRst->nCombatType;
			Cmd.nObjID				= pRst->nObjID;
			Cmd.nEventID			= pRst->nEventID;
			Cmd.nInstanceLevel		= pRst->nInstanceLevel;
			switch (pRst->nCombatType)
			{
			case combat_type_alli_instance:
				{
					SendCmdToClientWithTag64(pRst->nObjID, &Cmd, sizeof(Cmd));
					if (pRst->bInstanceDestroyed)
					{
						SendAlliInstanceNotify(pRst->nObjID, pRst->nAccountID, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_SYS_DESTROY);
						m_pGameMngS->GetInstanceMng()->DestroyInstance(pRst->nObjID);
					}
				}
				break;
			case combat_type_arena:
				{
					PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
					if (pPlayer != NULL)
					{
						SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
					}
				}
				break;
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_DEAL_COMBAT_RESULT_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T*)pBaseCmd;
			// 战报推送,奖励通知推送
			int*	pNotifyLen	= (int*)wh_getptrnexttoptr(pRst);
			char*	pNotify		= (char*)wh_getptrnexttoptr(pNotifyLen);
			if (pNotify[0] != 0)
			{
				AddNotifyInWorldChat(*pNotifyLen, pNotify);
			}
			// 简单战报
			{
				STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T	Cmd;
				Cmd.nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
				Cmd.nGameCmd	= STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG;
				Cmd.nCombatResult	= pRst->nCombatResult;
				Cmd.nBackLeftTime	= pRst->nBackLeftTime;
				Cmd.nCombatType		= pRst->nCombatType;
				Cmd.nInstanceLevel	= pRst->nInstanceLevel;
				Cmd.nInstanceID		= pRst->nObjID;
				Cmd.nEventID	= pRst->nEventID;
				Cmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T::RST_OK;

				switch (pRst->nCombatType)
				{
				case combat_type_alli_instance:
					{
						SendCmdToClientWithTag64(pRst->nObjID, &Cmd, sizeof(Cmd));
					}
					break;
				case combat_type_arena:
					{
						PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nAccountID);
						if (pPlayer != NULL)
						{
							SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
						}
					}
					break;
				}
			}
		}
		break;

	case CMDID_GET_KICK_CLIENT_ALL_RPL:
		{
			P_DBS4WEBUSER_GET_KICK_CLIENT_ALL_T*	pRst	= (P_DBS4WEBUSER_GET_KICK_CLIENT_ALL_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_KICK_CLIENT_ALL_T) + pRst->nLength);
			STC_GAMECMD_GET_KICK_CLIENT_ALL_T*	pCmd		= (STC_GAMECMD_GET_KICK_CLIENT_ALL_T*)m_vectrawbuf.getbuf();
			pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pCmd->nGameCmd	= STC_GAMECMD_GET_KICK_CLIENT_ALL;
			pCmd->nRst		= pRst->nRst;
			pCmd->nLength	= pRst->nLength;
			memcpy(wh_getptrnexttoptr(pCmd), wh_getptrnexttoptr(pRst), pRst->nLength);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_GET_HERO_SIMPLE_DATA_RPL:
		{
			P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T*	pRst	= (P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T*)pBaseCmd;
			unsigned int	nHeroUnitsSize	= nDSize-sizeof(P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_HERO_SIMPLE_DATA_T)+nHeroUnitsSize);
			STC_GAMECMD_HERO_SIMPLE_DATA_T*	pRstCmd			= (STC_GAMECMD_HERO_SIMPLE_DATA_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd		= STC_GAMECMD_HERO_SIMPLE_DATA;
			pRstCmd->nReason		= pRst->nReason;
			pRstCmd->nCombatType	= pRst->nCombatType;
			pRstCmd->nNum			= pRst->nNum;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nHeroUnitsSize);
			switch (pRst->nCombatType)
			{
			case combat_type_alli_instance:
				{
					SendCmdToClientWithTag64(pRst->nID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
				break;
			case combat_type_arena:
				{
					PlayerUnit*	pPlayer	= GetPlayerByAccountID(pRst->nID);
					if (pPlayer != NULL)
					{
						SendCmdToClient(pPlayer->nID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
					}
				}
				break;
			}
		}
		break;
	case CMDID_GET_INSTANCE_SIMPLE_DATA_RPL:
		{
			P_DBS4WEBUSER_GET_INSTANCE_SIMPLE_DATA_T*	pRst	= (P_DBS4WEBUSER_GET_INSTANCE_SIMPLE_DATA_T*)pBaseCmd;
			GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,LOAD_INSTANCE_SIMPLE_DATA)"%d", pRst->nNum);
			m_pGameMngS->GetInstanceMng()->LoadInstances(pRst->nNum, (SimpleInstanceData*)wh_getptrnexttoptr(pRst));
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_COMBAT_LOG_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_LOG_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_LOG_T*)pBaseCmd;
			int	nCombatLogSize			= nDSize - sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_LOG_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T) + nCombatLogSize);
			STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T*	pRstCmd			= (STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd			= STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG;
			pRstCmd->nID				= pRst->nID;
			pRstCmd->nCombatType		= pRst->nCombatType;
			pRstCmd->nRst				= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nCombatLogSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG;
			Cmd.nCombatResult			= pRst->nCombatResult;
			Cmd.nCombatType				= pRst->nCombatType;
			Cmd.nInstanceLevel			= pRst->nInstanceLevel;
			Cmd.nBackLeftTime			= pRst->nBackLeftTime;
			Cmd.nInstanceID				= pRst->nInstanceID;
			Cmd.nEventID				= 0;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_STOP_COMBAT_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_STOP_COMBAT_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_STOP_COMBAT_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT;
			Cmd.nInstanceID				= pRst->nInstanceID;
			Cmd.nCombatType				= pRst->nCombatType;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_SUPPLY_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_SUPPLY_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_SUPPLY_T*)pBaseCmd;
			STC_GAMECMD_ALLI_INSTANCE_SUPPLY_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_ALLI_INSTANCE_SUPPLY;
			Cmd.nInstanceID				= pRst->nInstanceID;
			Cmd.nCombatType				= pRst->nCombatType;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_SYSTEM_DESTROY_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_SYSTEM_DESTROY_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_SYSTEM_DESTROY_T*)pBaseCmd;
			const char*	cszInstanceIDs		= (const char*)wh_getptrnexttoptr(pRst);
			if (cszInstanceIDs[0] != 0)
			{
				int			nOffset			= 0;
				tty_id_t	nInstanceID		= 0;
				while (wh_strsplit(&nOffset, "I", cszInstanceIDs+1, ",", &nInstanceID) == 1)
				{
					SendAlliInstanceNotify(nInstanceID, 0, STC_GAMECMD_ALLI_INSTANCE_ACTION_NOTIFY_T::AI_ACTION_SYS_DESTROY);

					m_pGameMngS->GetInstanceMng()->DestroyInstance(nInstanceID);
				}
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_RPL:
		{
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*	pRst	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*)pBaseCmd;
			unsigned int	nArmyUnitsSize					= nDSize - sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T) + nArmyUnitsSize);
			STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*	pRstCmd	= (STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd			= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd		= STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY;
			pRstCmd->nExcelID		= pRst->nExcelID;
			pRstCmd->nLevel			= pRst->nLevel;
			pRstCmd->nNum			= pRst->nNum;
			pRstCmd->nRst			= pRst->nRst;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nArmyUnitsSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;

	case CMDID_ARENA_UPLOAD_DATA_RPL:
		{
			P_DBS4WEBUSER_ARENA_UPLOAD_DATA_T*	pRst	= (P_DBS4WEBUSER_ARENA_UPLOAD_DATA_T*)pBaseCmd;
			STC_GAMECMD_ARENA_UPLOAD_DATA_T		Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_ARENA_UPLOAD_DATA;
			Cmd.nRst					= pRst->nRst;
			Cmd.nRank					= pRst->nRank;
			Cmd.bFirstUpload			= pRst->bFirstUpload;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_ARENA_GET_RANK_LIST_RPL:
		{
			P_DBS4WEBUSER_ARENA_GET_RANK_LIST_T*	pRst	= (P_DBS4WEBUSER_ARENA_GET_RANK_LIST_T*)pBaseCmd;
			unsigned int	nRankListSize			= nDSize-sizeof(P_DBS4WEBUSER_ARENA_GET_RANK_LIST_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ARENA_GET_RANK_LIST_T) + nRankListSize);
			STC_GAMECMD_ARENA_GET_RANK_LIST_T*	pRstCmd		= (STC_GAMECMD_ARENA_GET_RANK_LIST_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd			= STC_GAMECMD_ARENA_GET_RANK_LIST;
			pRstCmd->nRst				= pRst->nRst;
			pRstCmd->nNum				= pRst->nNum;
			pRstCmd->nPage				= pRst->nPage;
			pRstCmd->nTotalNum			= pRst->nTotalNum;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nRankListSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ARENA_GET_STATUS_RPL:
		{
			P_DBS4WEBUSER_ARENA_GET_STATUS_T*	pRst	= (P_DBS4WEBUSER_ARENA_GET_STATUS_T*)pBaseCmd;
			unsigned int	nChallengeListSize			= nDSize-sizeof(P_DBS4WEBUSER_ARENA_GET_STATUS_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ARENA_GET_STATUS_T) + nChallengeListSize);
			STC_GAMECMD_ARENA_GET_STATUS_T*	pRstCmd		= (STC_GAMECMD_ARENA_GET_STATUS_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd			= STC_GAMECMD_ARENA_GET_STATUS;
			pRstCmd->nRst				= pRst->nRst;
			pRstCmd->nNum				= pRst->nNum;
			pRstCmd->nChallengeTimesLeft= pRst->nChallengeTimesLeft;
			pRstCmd->nPayTimesLeft		= pRst->nPayTimesLeft;
			pRstCmd->nUploadTimeLeft	= pRst->nUploadTimeLeft;
			pRstCmd->nRank				= pRst->nRank;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nChallengeListSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_ARENA_PAY_RPL:
		{
			P_DBS4WEBUSER_ARENA_PAY_T*	pRst	= (P_DBS4WEBUSER_ARENA_PAY_T*)pBaseCmd;
			STC_GAMECMD_ARENA_PAY_T		Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_ARENA_PAY;
			Cmd.nRst					= pRst->nRst;
			Cmd.nMoneyType				= pRst->nMoneyType;
			Cmd.nPrice					= pRst->nPrice;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_ARENA_PAY_T::RST_OK)
			{
				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->SubMoney(pRst->nMoneyType, pRst->nPrice);
				}

				AddMoneyCostLog(pRst->nAccountID, money_use_type_arena_pay, pRst->nMoneyType, pRst->nPrice);
			}
		}
		break;
	case CMDID_ARENA_DEAL_REWARD_EVENT_RPL:
		{
			P_DBS4WEBUSER_ARENA_DEAL_REWARD_EVENT_T*	pRst	= (P_DBS4WEBUSER_ARENA_DEAL_REWARD_EVENT_T*)pBaseCmd;
			const char*	cszMailIDs		= (const char*)wh_getptrnexttoptr(pRst);
			if (cszMailIDs[0] != 0)
			{
				int			nOffset		= 0;
				tty_id_t	nAccountID	= 0;
				unsigned int	nMailID	= 0;
				cszMailIDs				+= 1;
				while (wh_strsplit(&nOffset, "Id", cszMailIDs, ",", &nAccountID, &nMailID) == 2)
				{
					P_DBS4WEB_GET_PRIVATE_MAIL_1_T	Cmd;
					Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
					Cmd.nSubCmd		= CMDID_GET_PRIVATE_MAIL_1_REQ;
					Cmd.nAccountID	= nAccountID;
					Cmd.nMailID		= nMailID;
					g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

					RefreshPlayerCharAtbFromDB(nAccountID);
				}
			}
		}
		break;
	case CMDID_ARENA_GET_DEPLOY_RPL:
		{
			P_DBS4WEBUSER_ARENA_GET_DEPLOY_T*	pRst	= (P_DBS4WEBUSER_ARENA_GET_DEPLOY_T*)pBaseCmd;
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_ARENA_GET_DEPLOY_T));
			STC_GAMECMD_ARENA_GET_DEPLOY_T*	pRstCmd	= (STC_GAMECMD_ARENA_GET_DEPLOY_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd			= STC_GAMECMD_ARENA_GET_DEPLOY;
			pRstCmd->nRst				= pRst->nRst;
			pRstCmd->nNum				= 0;

			const char*	cszHeroData		= (const char*)wh_getptrnexttoptr(pRst);
			if (pRst->nRst==STC_GAMECMD_ARENA_GET_DEPLOY_T::RST_OK && cszHeroData[0]!=0)
			{
				int		nOffset			= 0;
				unsigned int	nNum		= 0;
				// ,*row*col*char_name*hero_id*hero_level*hero_name*prof*army_type*army_level*army_num
				unsigned int	nRow		= 0;
				unsigned int	nCol		= 0;
				char	szCharName[TTY_CHARACTERNAME_LEN]	= "";
				tty_id_t		nHeroID		= 0;
				unsigned int	nHeroLevel	= 0;
				char	szHeroName[TTY_CHARACTERNAME_LEN]	= "";
				unsigned int	nProf		= 0;
				unsigned int	nArmyType	= 0;
				unsigned int	nArmyLevel	= 0;
				unsigned int	nArmyNum	= 0;
				char	szOne[256]			= "";
				cszHeroData					+= 1;
				while (wh_strsplit(&nOffset, "s", cszHeroData, ",", szOne) == 1)
				{
					wh_strsplit("ddsIdsdddd", szOne+1, "*"
						, &nRow, &nCol, szCharName, &nHeroID, &nHeroLevel, szHeroName, &nProf, &nArmyType, &nArmyLevel, &nArmyNum);

					ArenaHeroDeploy*	pUnit	= (ArenaHeroDeploy*)m_vectrawbuf.pushn_back(sizeof(ArenaHeroDeploy));
					pUnit->nRow				= nRow;
					pUnit->nCol				= nCol;
					pUnit->nHeroID			= nHeroID;
					pUnit->nProf			= nProf;
					pUnit->nArmyType		= nArmyType;
					pUnit->nArmyNum			= nArmyNum;
					pUnit->nArmyLevel		= nArmyLevel;

					nNum++;
				}

				pRstCmd						= (STC_GAMECMD_ARENA_GET_DEPLOY_T*)m_vectrawbuf.getbuf();
				pRstCmd->nNum				= nNum;
			}
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	case CMDID_HERO_SUPPLY_RPL:
		{
			P_DBS4WEBUSER_HERO_SUPPLY_T*	pRst	= (P_DBS4WEBUSER_HERO_SUPPLY_T*)pBaseCmd;
			STC_GAMECMD_HERO_SUPPLY_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_HERO_SUPPLY;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_REFRESH_DRUG_RPL:
		{
			P_DBS4WEBUSER_REFRESH_DRUG_T*	pRst	= (P_DBS4WEBUSER_REFRESH_DRUG_T*)pBaseCmd;

			STC_GAMECMD_SUPPLY_DRUG_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_SUPPLY_DRUG;
			Cmd.nNum					= 0;

			P_DBS4WEBUSER_REFRESH_DRUG_T::DrugInfo*	pUnit	= (P_DBS4WEBUSER_REFRESH_DRUG_T::DrugInfo*)wh_getptrnexttoptr(pRst);
			for (int i=0; i<pRst->nNum; i++)
			{
				PlayerUnit*	pPlayer		= GetPlayerByAccountID(pUnit->nAccountID);
				if (pPlayer != NULL)
				{
					Cmd.nNum			= pUnit->nDrug;
					SendCmdToClient(pPlayer->nID, &Cmd, sizeof(Cmd));
				}

				pUnit++;
			}
		}
		break;
	case CMDID_SET_VIP_DISPLAY_RPL:
		{
			P_DBS4WEBUSER_SET_VIP_DISPLAY_T*	pRst	= (P_DBS4WEBUSER_SET_VIP_DISPLAY_T*)pBaseCmd;

			STC_GAMECMD_OPERATE_SET_VIP_DISPLAY_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_OPERATE_SET_VIP_DISPLAY;
			Cmd.bVipDisplay				= pRst->bVipDisplay;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));

			if (pRst->nRst == STC_GAMECMD_OPERATE_SET_VIP_DISPLAY_T::RST_OK)
			{
				PlayerCard*	pPlayerCard	= m_pGameMngS->GetPlayerCardMng()->GetCardByAccountID(pRst->nAccountID);
				if (pPlayerCard != NULL)
				{
					pPlayerCard->bVipDisplay	= pRst->bVipDisplay;
				}

				PlayerData*	pPlayer		= m_pGameMngS->GetPlayerMng()->GetPlayerByGID(pRst->nExt[0]);
				if (pPlayer != NULL)
				{
					pPlayer->m_CharAtb.bVipDisplay	= pRst->bVipDisplay;
				}
			}
		}
		break;

	case CMDID_POSITION_MARK_ADD_RECORD_RPL:
		{
			P_DBS4WEBUSER_POSITION_MARK_ADD_RECORD_T*	pRst	= (P_DBS4WEBUSER_POSITION_MARK_ADD_RECORD_T*)pBaseCmd;
			STC_GAMECMD_POSITION_MARK_ADD_RECORD_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_POSITION_MARK_ADD_RECORD;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_POSITION_MARK_CHG_RECORD_RPL:
		{
			P_DBS4WEBUSER_POSITION_MARK_CHG_RECORD_T*	pRst	= (P_DBS4WEBUSER_POSITION_MARK_CHG_RECORD_T*)pBaseCmd;
			STC_GAMECMD_POSITION_MARK_CHG_RECORD_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_POSITION_MARK_CHG_RECORD;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_POSITION_MARK_DEL_RECORD_RPL:
		{
			P_DBS4WEBUSER_POSITION_MARK_DEL_RECORD_T*	pRst	= (P_DBS4WEBUSER_POSITION_MARK_DEL_RECORD_T*)pBaseCmd;
			STC_GAMECMD_POSITION_MARK_DEL_RECORD_T	Cmd;
			Cmd.nCmd					= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			Cmd.nGameCmd				= STC_GAMECMD_POSITION_MARK_DEL_RECORD;
			Cmd.nRst					= pRst->nRst;
			SendCmdToClient(pRst->nExt[0], &Cmd, sizeof(Cmd));
		}
		break;
	case CMDID_POSITION_MARK_GET_RECORD_RPL:
		{
			P_DBS4WEBUSER_POSITION_MARK_GET_RECORD_T*	pRst	= (P_DBS4WEBUSER_POSITION_MARK_GET_RECORD_T*)pBaseCmd;
			int	nRecordSize				= nDSize - sizeof(P_DBS4WEBUSER_POSITION_MARK_GET_RECORD_T);
			m_vectrawbuf.resize(sizeof(STC_GAMECMD_POSITION_MARK_GET_RECORD_T)+nRecordSize);
			STC_GAMECMD_POSITION_MARK_GET_RECORD_T*	pRstCmd	= (STC_GAMECMD_POSITION_MARK_GET_RECORD_T*)m_vectrawbuf.getbuf();
			pRstCmd->nCmd				= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
			pRstCmd->nGameCmd			= STC_GAMECMD_POSITION_MARK_GET_RECORD;
			pRstCmd->nRst				= pRst->nRst;
			pRstCmd->nNum				= pRst->nNum;
			memcpy(wh_getptrnexttoptr(pRstCmd), wh_getptrnexttoptr(pRst), nRecordSize);
			SendCmdToClient(pRst->nExt[0], m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		}
		break;
	}
	return 0;
}
