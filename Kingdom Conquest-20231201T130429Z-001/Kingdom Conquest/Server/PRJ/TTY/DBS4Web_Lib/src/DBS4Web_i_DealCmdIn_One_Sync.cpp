#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_Sync(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_SYNC_CHAR_REQ: 
		{
			P_DBS4WEB_SYNC_CHAR_T*	pReq		= (P_DBS4WEB_SYNC_CHAR_T*)pCmd;
			P_DBS4WEBUSER_SYNC_CHAR_T*	pRpl	= (P_DBS4WEBUSER_SYNC_CHAR_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_SYNC_CHAR_RPL;
			pRpl->nRst			= STC_GAMECMD_SYNC_CHAR_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sync char,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_SYNC_CHAR_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_sync_char(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sync char,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_SYNC_CHAR_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						CharDesc *pChar	= (CharDesc*)wh_getptrnexttoptr(pRpl);
						pChar->nExp		= q.GetVal_32();
						pChar->nDiamond	= q.GetVal_32();
						pChar->nCrystal	= q.GetVal_32();
						pChar->nGold	= q.GetVal_32();
						pChar->nVIP		= q.GetVal_32();
						++ pChar;
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"sync char,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pChar-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sync char,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						pRpl->nRst	= STC_GAMECMD_SYNC_CHAR_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_SYNC_HERO_REQ: 
		{
			P_DBS4WEB_SYNC_HERO_T*	pReq		= (P_DBS4WEB_SYNC_HERO_T*)pCmd;
			P_DBS4WEBUSER_SYNC_HERO_T*	pRpl	= (P_DBS4WEBUSER_SYNC_HERO_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_SYNC_HERO_RPL;
			pRpl->nRst			= STC_GAMECMD_SYNC_HERO_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sync hero,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
				pRpl->nRst		= STC_GAMECMD_SYNC_HERO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_sync_hero(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nHeroID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sync hero,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
					pRpl->nRst	= STC_GAMECMD_SYNC_HERO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRpl);
						pHero->nAttackBase	= q.GetVal_32();
						pHero->nAttackAdd	= q.GetVal_32();
						pHero->nDefenseBase	= q.GetVal_32();
						pHero->nDefenseAdd	= q.GetVal_32();
						pHero->nHealthBase	= q.GetVal_32();
						pHero->nHealthAdd	= q.GetVal_32();
						pHero->nLeaderBase	= q.GetVal_32();
						pHero->nLeaderAdd	= q.GetVal_32();
						pHero->fGrow		= (float)q.GetVal_float();
						pHero->nHeroID		= pReq->nHeroID;
						pHero->nHealthState	= q.GetVal_32();
						++ pHero;
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"sync hero,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nHeroID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sync hero,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
						pRpl->nRst	= STC_GAMECMD_SYNC_HERO_T::RST_SQL_ERR;
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
