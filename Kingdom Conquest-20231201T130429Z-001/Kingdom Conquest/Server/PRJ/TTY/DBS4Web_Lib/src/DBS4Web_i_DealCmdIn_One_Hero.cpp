#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_Hero(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_TAVERN_REFRESH_REQ:
		{
			P_DBS4WEB_TAVERN_REFRESH_T*	pReq	= (P_DBS4WEB_TAVERN_REFRESH_T*)pCmd;
			P_DBS4WEBUSER_TAVERN_REFRESH_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd		= CMDID_TAVERN_REFRESH_RPL;
			Rpl.nRst			= STC_GAMECMD_OPERATE_TAVERN_REFRESH_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"tavern_refresh,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				Rpl.nRst			= STC_GAMECMD_OPERATE_TAVERN_REFRESH_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_tavern_refresh(0x%"WHINT64PRFX"X,%d,%d,%s)", pReq->nAccountID, TAVERN_REFRESH_NUM, pReq->nUseItem, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"tavern_refresh,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					Rpl.nRst	= STC_GAMECMD_OPERATE_TAVERN_REFRESH_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						Rpl.nCD			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"tavern_refresh,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"tavern_refresh,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						Rpl.nRst	= STC_GAMECMD_OPERATE_UPGRADE_SOLDIER_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_HIRE_HERO_REQ:
		{
			P_DBS4WEB_HIRE_HERO_T*	pReq	= (P_DBS4WEB_HIRE_HERO_T*)pCmd;
			P_DBS4WEBUSER_HIRE_HERO_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_HIRE_HERO_RPL;
			Rpl.nRst			= STC_GAMECMD_OPERATE_HIRE_HERO_T::RST_OK;
			Rpl.nHeroID			= pReq->nHeroID;
			Rpl.nSlotID			= pReq->nSlotID;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"hire hero,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nSlotID, pReq->nHeroID);
				Rpl.nRst			= STC_GAMECMD_OPERATE_HIRE_HERO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_hire_hero(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nSlotID, pReq->nHeroID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"hire hero,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nSlotID, pReq->nHeroID);
					Rpl.nRst	= STC_GAMECMD_OPERATE_HIRE_HERO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"hire hero,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nSlotID, pReq->nHeroID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"hire hero,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nSlotID, pReq->nHeroID);
						Rpl.nRst	= STC_GAMECMD_OPERATE_HIRE_HERO_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_FIRE_HERO_REQ:
		{
			P_DBS4WEB_FIRE_HERO_T*	pReq	= (P_DBS4WEB_FIRE_HERO_T*)pCmd;
			P_DBS4WEBUSER_FIRE_HERO_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_FIRE_HERO_RPL;
			Rpl.nRst			= STC_GAMECMD_OPERATE_FIRE_HERO_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fire hero,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
				Rpl.nRst			= STC_GAMECMD_OPERATE_FIRE_HERO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_fire_hero(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nHeroID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fire hero,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
					Rpl.nRst	= STC_GAMECMD_OPERATE_FIRE_HERO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"fire hero,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nHeroID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fire hero,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
						Rpl.nRst	= STC_GAMECMD_OPERATE_FIRE_HERO_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_LEVELUP_HERO_REQ:
		{
			P_DBS4WEB_LEVELUP_HERO_T*	pReq	= (P_DBS4WEB_LEVELUP_HERO_T*)pCmd;
			P_DBS4WEBUSER_LEVELUP_HERO_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_LEVELUP_HERO_RPL;
			Rpl.nRst			= STC_GAMECMD_OPERATE_LEVELUP_HERO_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"levelup hero,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
				Rpl.nRst			= STC_GAMECMD_OPERATE_LEVELUP_HERO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_levelup_hero(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,1,%s)", pReq->nAccountID, pReq->nHeroID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"levelup hero,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
					Rpl.nRst	= STC_GAMECMD_OPERATE_LEVELUP_HERO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"levelup hero,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nHeroID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"levelup hero,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
						Rpl.nRst	= STC_GAMECMD_OPERATE_LEVELUP_HERO_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_GROW_REQ:
		{
			P_DBS4WEB_ADD_GROW_T*	pReq	= (P_DBS4WEB_ADD_GROW_T*)pCmd;
			P_DBS4WEBUSER_ADD_GROW_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_ADD_GROW_RPL;
			Rpl.nRst			= STC_GAMECMD_OPERATE_ADD_GROW_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add grow,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%.2f", pReq->nAccountID, pReq->nHeroID, pReq->fGrow);
				Rpl.nRst			= STC_GAMECMD_OPERATE_ADD_GROW_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_add_grow(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%.2f,1,%s)", pReq->nAccountID, pReq->nHeroID, pReq->fGrow, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add grow,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%.2f", pReq->nAccountID, pReq->nHeroID, pReq->fGrow);
					Rpl.nRst	= STC_GAMECMD_OPERATE_ADD_GROW_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add grow,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%.2f,%d", pReq->nAccountID, pReq->nHeroID, pReq->fGrow, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add grow,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%.2f", pReq->nAccountID, pReq->nHeroID, pReq->fGrow);
						Rpl.nRst	= STC_GAMECMD_OPERATE_ADD_GROW_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_CONFIG_HERO_REQ:
		{
			P_DBS4WEB_CONFIG_HERO_T*	pReq	= (P_DBS4WEB_CONFIG_HERO_T*)pCmd;
			P_DBS4WEBUSER_CONFIG_HERO_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_CONFIG_HERO_RPL;
			Rpl.nRst			= STC_GAMECMD_OPERATE_CONFIG_HERO_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config hero,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nArmyType, pReq->nArmyLevel, pReq->nArmyNum);
				Rpl.nRst			= STC_GAMECMD_OPERATE_CONFIG_HERO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_config_hero(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,1,%s)", pReq->nAccountID, pReq->nHeroID, pReq->nArmyType, pReq->nArmyLevel, pReq->nArmyNum, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config hero,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nArmyType, pReq->nArmyLevel, pReq->nArmyNum);
					Rpl.nRst	= STC_GAMECMD_OPERATE_CONFIG_HERO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						Rpl.nProf		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"config hero,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nArmyType, pReq->nArmyLevel, pReq->nArmyNum, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config hero,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nArmyType, pReq->nArmyLevel, pReq->nArmyNum);
						Rpl.nRst	= STC_GAMECMD_OPERATE_CONFIG_HERO_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_HERO_REFRESH_REQ:
		{
			P_DBS4WEB_GET_HERO_REFRESH_T*	pReq		= (P_DBS4WEB_GET_HERO_REFRESH_T*)pCmd;
			P_DBS4WEBUSER_GET_HERO_REFRESH_T*	pRpl	= (P_DBS4WEBUSER_GET_HERO_REFRESH_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_HERO_REFRESH_RPL;
			pRpl->nRst			= STC_GAMECMD_OPERATE_GET_HERO_REFRESH_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get hero refresh,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_OPERATE_GET_HERO_REFRESH_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("select slot_idx,hero_id,name,profession,model,attack,defense,health,grow,grow_max from refresh_heros where account_id=0x%"WHINT64PRFX"X", pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get hero refresh,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_OPERATE_GET_HERO_REFRESH_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					RefreshHero *pHero = (RefreshHero*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pHero->nSlotID		= q.GetVal_32();
						pHero->nHeroID		= q.GetVal_64();
						strcpy(pHero->szName, q.GetStr());
						pHero->nProfession	= q.GetVal_32();
						pHero->nModel		= q.GetVal_32();
						pHero->nAttack		= q.GetVal_32();
						pHero->nDefense		= q.GetVal_32();
						pHero->nHealth		= q.GetVal_32();
						pHero->fGrow		= q.GetVal_float();
						pHero->fGrowMax		= q.GetVal_float();

						++pHero;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get hero refresh,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_HERO_HIRE_REQ:
		{
			P_DBS4WEB_GET_HERO_HIRE_T*	pReq		= (P_DBS4WEB_GET_HERO_HIRE_T*)pCmd;
			P_DBS4WEBUSER_GET_HERO_HIRE_T*	pRpl	= (P_DBS4WEBUSER_GET_HERO_HIRE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_HERO_HIRE_RPL;
			pRpl->nRst			= STC_GAMECMD_OPERATE_GET_HERO_HIRE_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get hero hire,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_OPERATE_GET_HERO_HIRE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_hero_hire(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get hero hire,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_OPERATE_GET_HERO_HIRE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					HireHero *pHero = (HireHero*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pHero->nHeroID		= q.GetVal_64();
						strcpy(pHero->szName, q.GetStr());
						pHero->nProfession	= q.GetVal_32();
						pHero->nModel		= q.GetVal_32();
						pHero->nAttack		= q.GetVal_32();
						pHero->nDefense		= q.GetVal_32();
						pHero->nHealth		= q.GetVal_32();
						pHero->nLeader		= q.GetVal_32();
						pHero->fGrow		= q.GetVal_float();
						pHero->nArmyType	= q.GetVal_32();
						pHero->nArmyLevel	= q.GetVal_32();
						pHero->nArmyNum		= q.GetVal_32();
						pHero->nStatus		= q.GetVal_32();
						pHero->nAttackAdd		= q.GetVal_32();
						pHero->nDefenseAdd		= q.GetVal_32();
						pHero->nHealthAdd		= q.GetVal_32();
						pHero->nLeaderAdd		= q.GetVal_32();
						pHero->nLevel		= q.GetVal_32();
						pHero->nProf		= q.GetVal_32();
						pHero->nHealthState	= q.GetVal_32();
						pHero->nExp			= q.GetVal_32();
						pHero->fGrowMax		= q.GetVal_float();

						++pHero;
					}

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get hero hire,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_UPDATE_HERO_REQ:
		{
			P_DBS4WEB_UPDATE_HERO_T*	pReq	= (P_DBS4WEB_UPDATE_HERO_T*)pCmd;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"update hero,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nHeroID, pReq->nArmyNum);
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_update_hero(0x%"WHINT64PRFX"X,%d,1,%s)", pReq->nHeroID, pReq->nArmyNum, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"update hero,SQLERR,0x%"WHINT64PRFX"X,%d", pReq->nHeroID, pReq->nArmyNum);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						int nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"update hero,0x%"WHINT64PRFX"X,%d,%d", pReq->nHeroID, pReq->nArmyNum, nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"update hero,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nHeroID, pReq->nArmyNum);
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_CONFIG_CITYDEFENSE_REQ:
		{
			P_DBS4WEB_CONFIG_CITYDEFENSE_T*	pReq	= (P_DBS4WEB_CONFIG_CITYDEFENSE_T*)pCmd;
			P_DBS4WEBUSER_CONFIG_CITYDEFENSE_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_CONFIG_CITYDEFENSE_RPL;
			Rpl.nRst			= STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config citydefense,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
				Rpl.nRst			= STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_config_citydefense(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%s)", 
					pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, pReq->nAutoSupply, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config citydefense,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
					Rpl.nRst	= STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"config citydefense,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config citydefense,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
						Rpl.nRst	= STC_GAMECMD_OPERATE_CONFIG_CITYDEFENSE_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_CITYDEFENSE_REQ:
		{
			P_DBS4WEB_GET_CITYDEFENSE_T*	pReq		= (P_DBS4WEB_GET_CITYDEFENSE_T*)pCmd;
			P_DBS4WEBUSER_GET_CITYDEFENSE_T*	pRpl	= (P_DBS4WEBUSER_GET_CITYDEFENSE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_CITYDEFENSE_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_CITYDEFENSE_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get citydefense,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_GET_CITYDEFENSE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_citydefense(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get citydefense,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_GET_CITYDEFENSE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->n1Hero		= q.GetVal_64();
						pRpl->n2Hero		= q.GetVal_64();
						pRpl->n3Hero		= q.GetVal_64();
						pRpl->n4Hero		= q.GetVal_64();
						pRpl->n5Hero		= q.GetVal_64();
						pRpl->nAutoSupply	= q.GetVal_32();

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get citydefense,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get citydefense,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
						pRpl->nRst	= STC_GAMECMD_GET_CITYDEFENSE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	default:
		{

		}
		break;
	}

	return 0;
}
