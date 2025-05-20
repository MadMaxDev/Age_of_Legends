#include "../inc/DBS4Web_i.h"

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_PositionMark(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_POSITION_MARK_ADD_RECORD_REQ:
		{
			P_DBS4WEB_POSITION_MARK_ADD_RECORD_T*	pReq	= (P_DBS4WEB_POSITION_MARK_ADD_RECORD_T*)pCmd;
			PositionMark*	pMark	= (PositionMark*)wh_getptrnexttoptr(pReq);
			P_DBS4WEBUSER_POSITION_MARK_ADD_RECORD_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_POSITION_MARK_ADD_RECORD_RPL;
			RplCmd.nRst				= STC_GAMECMD_POSITION_MARK_ADD_RECORD_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_ADD_RECORD_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_add_record,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL position_mark_add_record(0x%"WHINT64PRFX"X,%d,%d,%d,'%s')"
					, pReq->nAccountID, pMark->nPosX, pMark->nPosY, pMark->nHeadID, (char*)wh_getptrnexttoptr(pMark));
				int	nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_ADD_RECORD_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_add_record,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_ADD_RECORD_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_add_record,NO RESULT,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_POSITION_MARK_CHG_RECORD_REQ:
		{
			P_DBS4WEB_POSITION_MARK_CHG_RECORD_T*	pReq	= (P_DBS4WEB_POSITION_MARK_CHG_RECORD_T*)pCmd;
			PositionMark*	pMark	= (PositionMark*)wh_getptrnexttoptr(pReq);
			P_DBS4WEBUSER_POSITION_MARK_CHG_RECORD_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_POSITION_MARK_CHG_RECORD_RPL;
			RplCmd.nRst				= STC_GAMECMD_POSITION_MARK_CHG_RECORD_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_CHG_RECORD_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_chg_record,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL position_mark_chg_record(0x%"WHINT64PRFX"X,%d,%d,%d,'%s')"
					, pReq->nAccountID, pMark->nPosX, pMark->nPosY, pMark->nHeadID, (char*)wh_getptrnexttoptr(pMark));
				int	nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_CHG_RECORD_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_chg_record,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_CHG_RECORD_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_chg_record,NO RESULT,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_POSITION_MARK_DEL_RECORD_REQ:
		{
			P_DBS4WEB_POSITION_MARK_DEL_RECORD_T*	pReq	= (P_DBS4WEB_POSITION_MARK_DEL_RECORD_T*)pCmd;
			PositionMark*	pMark	= (PositionMark*)wh_getptrnexttoptr(pReq);
			P_DBS4WEBUSER_POSITION_MARK_DEL_RECORD_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_POSITION_MARK_DEL_RECORD_RPL;
			RplCmd.nRst				= STC_GAMECMD_POSITION_MARK_DEL_RECORD_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_DEL_RECORD_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_del_record,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL position_mark_del_record(0x%"WHINT64PRFX"X,%d,%d)", pReq->nAccountID, pReq->nPosX, pReq->nPosY);
				int	nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_DEL_RECORD_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_del_record,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_DEL_RECORD_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_del_record,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_POSITION_MARK_GET_RECORD_REQ:
		{
			P_DBS4WEB_POSITION_MARK_GET_RECORD_T*	pReq	= (P_DBS4WEB_POSITION_MARK_GET_RECORD_T*)pCmd;
			PositionMark*	pMark	= (PositionMark*)wh_getptrnexttoptr(pReq);
			P_DBS4WEBUSER_POSITION_MARK_GET_RECORD_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_POSITION_MARK_GET_RECORD_RPL;
			RplCmd.nNum				= 0;
			RplCmd.nRst				= STC_GAMECMD_POSITION_MARK_GET_RECORD_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_DEL_RECORD_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_del_record,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL position_mark_get_record(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_POSITION_MARK_DEL_RECORD_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"position_mark_del_record,SQLERR");
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_POSITION_MARK_GET_RECORD_T));
						P_DBS4WEBUSER_POSITION_MARK_GET_RECORD_T*	pRplCmd	= (P_DBS4WEBUSER_POSITION_MARK_GET_RECORD_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						while (q.FetchRow())
						{
							int*	pLengths		= q.FetchLengths();
							PositionMark*	pUnit	= (PositionMark*)vectRawBuf.pushn_back(sizeof(PositionMark) + pLengths[3]+1);
							pUnit->nPosX			= q.GetVal_32();
							pUnit->nPosY			= q.GetVal_32();
							pUnit->nHeadID			= q.GetVal_32();
							pUnit->nLength			= pLengths[3]+1;
							memcpy(wh_getptrnexttoptr(pUnit), q.GetStr(), pUnit->nLength);

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	}
	return 0;
}
