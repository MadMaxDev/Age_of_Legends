#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_WorldGoldMine(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_GET_WORLDGOLDMINE_REQ:
		{
			P_DBS4WEB_GET_WORLDGOLDMINE_T*	pReq		= (P_DBS4WEB_GET_WORLDGOLDMINE_T*)pCmd;
			P_DBS4WEBUSER_GET_WORLDGOLDMINE_T*	pRpl	= (P_DBS4WEBUSER_GET_WORLDGOLDMINE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_WORLDGOLDMINE_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_WORLDGOLDMINE_T::RST_OK;
			pRpl->nArea			= pReq->nArea;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get worldgoldmine,DBERR,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nArea, pReq->nClass, pReq->nIdx);
				pRpl->nRst		= STC_GAMECMD_GET_WORLDGOLDMINE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_worldgoldmine(%d,%d,%d,%s)", pReq->nArea, pReq->nClass, pReq->nIdx, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get worldgoldmine,DBERR,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nArea, pReq->nClass, pReq->nIdx);
					pRpl->nRst		= STC_GAMECMD_GET_WORLDGOLDMINE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					WorldGoldMineDesc *pMine = (WorldGoldMineDesc*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pMine->nClass		= q.GetVal_32();
						pMine->nIdx			= q.GetVal_32();
						pMine->nAccountID	= q.GetVal_64();
						strcpy(pMine->szName, q.GetStr());
						pMine->nSafeTime	= q.GetVal_32();
						++pMine;
					}

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get worldgoldmine,0x%"WHINT64PRFX"X,%d,%d,%d,%d", pReq->nAccountID, pReq->nArea, pReq->nClass, pReq->nIdx, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pMine-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_CONFIG_WORLDGOLDMINE_HERO_REQ: 
		{
			P_DBS4WEB_CONFIG_WORLDGOLDMINE_HERO_T*	pReq		= (P_DBS4WEB_CONFIG_WORLDGOLDMINE_HERO_T*)pCmd;
			P_DBS4WEBUSER_CONFIG_WORLDGOLDMINE_HERO_T*	pRpl	= (P_DBS4WEBUSER_CONFIG_WORLDGOLDMINE_HERO_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_CONFIG_WORLDGOLDMINE_HERO_RPL;
			pRpl->nRst			= STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config worldgoldmine hero,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
				pRpl->nRst		= STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_config_worldgoldmine_hero(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,1,%s)", 
					pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config worldgoldmine hero,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
					pRpl->nRst		= STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"config worldgoldmine hero,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config worldgoldmine hero,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
						pRpl->nRst		= STC_GAMECMD_CONFIG_WORLDGOLDMINE_HERO_T::RST_DB_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_ROB_WORLDGOLDMINE_REQ: 
		{
			P_DBS4WEB_ROB_WORLDGOLDMINE_T*	pReq		= (P_DBS4WEB_ROB_WORLDGOLDMINE_T*)pCmd;
			P_DBS4WEBUSER_ROB_WORLDGOLDMINE_T*	pRpl	= (P_DBS4WEBUSER_ROB_WORLDGOLDMINE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_ROB_WORLDGOLDMINE_RPL;
			pRpl->nRst			= STC_GAMECMD_ROB_WORLDGOLDMINE_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"rob worldgoldmine,DBERR,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nArea, pReq->nClass, pReq->nIdx, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
				pRpl->nRst		= STC_GAMECMD_ROB_WORLDGOLDMINE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_rob_worldgoldmine(%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", 
					pReq->nArea, pReq->nClass, pReq->nIdx, pReq->nAccountID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"rob worldgoldmine,DBERR,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nArea, pReq->nClass, pReq->nIdx, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
					pRpl->nRst		= STC_GAMECMD_ROB_WORLDGOLDMINE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->nCombatID	= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"rob worldgoldmine,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nArea, pReq->nClass, pReq->nIdx, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"rob worldgoldmine,DBERR,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nArea, pReq->nClass, pReq->nIdx, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
						pRpl->nRst		= STC_GAMECMD_ROB_WORLDGOLDMINE_T::RST_DB_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_DROP_WORLDGOLDMINE_REQ: 
		{
			P_DBS4WEB_DROP_WORLDGOLDMINE_T*	pReq		= (P_DBS4WEB_DROP_WORLDGOLDMINE_T*)pCmd;
			P_DBS4WEBUSER_DROP_WORLDGOLDMINE_T*	pRpl	= (P_DBS4WEBUSER_DROP_WORLDGOLDMINE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_DROP_WORLDGOLDMINE_RPL;
			pRpl->nRst			= STC_GAMECMD_DROP_WORLDGOLDMINE_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"drop worldgoldmine,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_DROP_WORLDGOLDMINE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_drop_worldgoldmine(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"drop worldgoldmine,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst		= STC_GAMECMD_DROP_WORLDGOLDMINE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"drop worldgoldmine,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"drop worldgoldmine,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
						pRpl->nRst		= STC_GAMECMD_DROP_WORLDGOLDMINE_T::RST_DB_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_DEAL_WORLDGOLDMINE_GAIN_TE_REQ:
		{
			P_DBS4WEB_DEAL_WORLDGOLDMINE_GAIN_TE_T*	pReq	= (P_DBS4WEB_DEAL_WORLDGOLDMINE_GAIN_TE_T*)pCmd;
			P_DBS4WEBUSER_DEAL_WORLDGOLDMINE_GAIN_TE_T	*pRpl	= (P_DBS4WEBUSER_DEAL_WORLDGOLDMINE_GAIN_TE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_DEAL_WORLDGOLDMINE_GAIN_TE_RPL;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal worldgoldmine gain te,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nEventID);
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_deal_te_worldgoldmine_gain(0x%"WHINT64PRFX"X,%s)", pReq->nEventID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal worldgoldmine gain te,SQLERR,0x%"WHINT64PRFX"X,%d", pReq->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->nAccountID	= q.GetVal_64();
						pRpl->nArea		= q.GetVal_32();
						pRpl->nClass	= q.GetVal_32();
						pRpl->nIdx		= q.GetVal_32();
						pRpl->nGold		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal worldgoldmine gain te,0x%"WHINT64PRFX"X", pReq->nEventID);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal worldgoldmine gain te,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nEventID);
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_MY_WORLDGOLDMINE_REQ: 
		{
			P_DBS4WEB_MY_WORLDGOLDMINE_T*	pReq		= (P_DBS4WEB_MY_WORLDGOLDMINE_T*)pCmd;
			P_DBS4WEBUSER_MY_WORLDGOLDMINE_T*	pRpl	= (P_DBS4WEBUSER_MY_WORLDGOLDMINE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_MY_WORLDGOLDMINE_RPL;
			pRpl->nRst			= STC_GAMECMD_MY_WORLDGOLDMINE_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"my worldgoldmine,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_MY_WORLDGOLDMINE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_my_worldgoldmine(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"my worldgoldmine,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst		= STC_GAMECMD_MY_WORLDGOLDMINE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->nArea		= q.GetVal_32();
						pRpl->nClass	= q.GetVal_32();
						pRpl->nIdx		= q.GetVal_32();
						pRpl->nSafeTime	= q.GetVal_32();
						pRpl->n1Hero	= q.GetVal_64();
						pRpl->n2Hero	= q.GetVal_64();
						pRpl->n3Hero	= q.GetVal_64();
						pRpl->n4Hero	= q.GetVal_64();
						pRpl->n5Hero	= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"my worldgoldmine,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"my worldgoldmine,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
						pRpl->nRst		= STC_GAMECMD_MY_WORLDGOLDMINE_T::RST_DB_ERR;
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
