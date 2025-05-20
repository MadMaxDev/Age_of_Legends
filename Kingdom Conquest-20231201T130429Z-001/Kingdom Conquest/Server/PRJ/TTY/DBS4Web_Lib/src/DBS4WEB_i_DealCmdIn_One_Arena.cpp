#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;
int		DBS4Web_i::ThreadDealCmd_Arena(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_ARENA_UPLOAD_DATA_REQ:
		{
			P_DBS4WEB_ARENA_UPLOAD_DATA_T*		pReq	= (P_DBS4WEB_ARENA_UPLOAD_DATA_T*)pCmd;
			P_DBS4WEBUSER_ARENA_UPLOAD_DATA_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ARENA_UPLOAD_DATA_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nRank		= 0;
			RplCmd.bFirstUpload	= false;
			RplCmd.nRst			= STC_GAMECMD_ARENA_UPLOAD_DATA_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_ARENA_UPLOAD_DATA_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"upload_arena_data,DBERR");
			}
			else
			{
				const char*	pHeroDeploy	= (const char*)wh_getptrnexttoptr(pReq);
				q.SpawnQuery("CALL upload_arena_data(0x%"WHINT64PRFX"X,'%s')"
					, pReq->nAccountID, pHeroDeploy);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_ARENA_UPLOAD_DATA_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"upload_arena_data,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_rank,_new
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nRank		= q.GetVal_32();
						RplCmd.bFirstUpload	= (bool)q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ARENA_UPLOAD_DATA_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"upload_arena_data,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ARENA_GET_RANK_LIST_REQ:
		{
			P_DBS4WEB_ARENA_GET_RANK_LIST_T*		pReq	= (P_DBS4WEB_ARENA_GET_RANK_LIST_T*)pCmd;
			P_DBS4WEBUSER_ARENA_GET_RANK_LIST_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ARENA_GET_RANK_LIST_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nTotalNum	= 0;
			RplCmd.nPage		= pReq->nPage;
			RplCmd.nRst			= STC_GAMECMD_ARENA_GET_RANK_LIST_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_ARENA_GET_RANK_LIST_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_arena_total_num,DBERR");
			}
			else
			{
				q.SpawnQuery("SELECT COUNT(*) FROM arena");
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_ARENA_GET_RANK_LIST_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_arena_total_num,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nTotalNum		= q.GetVal_32();

						q.FreeResult();
						q.SpawnQuery("CALL get_arena_rank_list(%d,%d)", pReq->nPage, pReq->nPageSize);
						nPreRet			= 0;
						q.ExecuteSPWithResult(nPreRet, false);
						if (nPreRet != MYSQL_QUERY_NORMAL)
						{
							RplCmd.nRst	= STC_GAMECMD_ARENA_GET_RANK_LIST_T::RST_SQL_ERR;
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_arena_rank_list,SQLERR");
						}
						else
						{
							RplCmd.nNum	= q.NumRows();
							if (RplCmd.nNum > 0)
							{
								vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ARENA_GET_RANK_LIST_T) + RplCmd.nNum*sizeof(STC_GAMECMD_ARENA_GET_RANK_LIST_T::RankListUnit));
								P_DBS4WEBUSER_ARENA_GET_RANK_LIST_T*	pRplCmd	= (P_DBS4WEBUSER_ARENA_GET_RANK_LIST_T*)vectRawBuf.getbuf();
								memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
								STC_GAMECMD_ARENA_GET_RANK_LIST_T::RankListUnit*	pUnit	= (STC_GAMECMD_ARENA_GET_RANK_LIST_T::RankListUnit*)wh_getptrnexttoptr(pRplCmd);
								while (q.FetchRow())
								{
									// a.rank,a.account_id,a.total_force,c.name,c.level,c.head_id,IFNULL(g.name,\'\')
									pUnit->nRank		= q.GetVal_32();
									pUnit->nAccountID	= q.GetVal_64();
									pUnit->nTotalForce	= q.GetVal_32();
									WH_STRNCPY0(pUnit->szName, q.GetStr());
									pUnit->nLevel		= q.GetVal_32();
									pUnit->nHeadID		= q.GetVal_32();
									WH_STRNCPY0(pUnit->szAllianceName, q.GetStr());

									pUnit++;
								}
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
								return 0;
							}
						}
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ARENA_GET_RANK_LIST_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_arena_total_num,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ARENA_GET_STATUS_REQ:
		{
			P_DBS4WEB_ARENA_GET_STATUS_T*		pReq	= (P_DBS4WEB_ARENA_GET_STATUS_T*)pCmd;
			P_DBS4WEBUSER_ARENA_GET_STATUS_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ARENA_GET_STATUS_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nChallengeTimesLeft	= 0;
			RplCmd.nUploadTimeLeft		= 0;
			RplCmd.nPayTimesLeft		= 0;
			RplCmd.nRank		= 0;
			RplCmd.nRst			= STC_GAMECMD_ARENA_GET_STATUS_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_ARENA_GET_STATUS_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_arena_status,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL get_arena_status(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_ARENA_GET_STATUS_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_arena_status,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_cd_upload_left_time,@_left_times,@_pay_times_left,_rank
						RplCmd.nRst					= q.GetVal_32();
						RplCmd.nUploadTimeLeft		= q.GetVal_32();
						RplCmd.nChallengeTimesLeft	= q.GetVal_32();
						RplCmd.nPayTimesLeft		= q.GetVal_32();
						RplCmd.nRank				= q.GetVal_32();
						
						q.FreeResult();
						q.SpawnQuery("CALL get_arena_challenge_list(0x%"WHINT64PRFX"X,%d)", pReq->nAccountID, pReq->nNum);
						nPreRet			= 0;
						q.ExecuteSPWithResult(nPreRet);
						if (nPreRet != MYSQL_QUERY_NORMAL)
						{
							RplCmd.nRst	= STC_GAMECMD_ARENA_GET_STATUS_T::RST_SQL_ERR;
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_arena_challenge_list,SQLERR");
						}
						else
						{
							RplCmd.nNum	= q.NumRows();
							if (RplCmd.nNum > 0)
							{
								vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ARENA_GET_STATUS_T) + RplCmd.nNum*sizeof(STC_GAMECMD_ARENA_GET_STATUS_T::ChallengeListUnit));
								P_DBS4WEBUSER_ARENA_GET_STATUS_T*	pRplCmd	= (P_DBS4WEBUSER_ARENA_GET_STATUS_T*)vectRawBuf.getbuf();
								memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
								STC_GAMECMD_ARENA_GET_STATUS_T::ChallengeListUnit*	pUnit	= (STC_GAMECMD_ARENA_GET_STATUS_T::ChallengeListUnit*)wh_getptrnexttoptr(pRplCmd);
								while (q.FetchRow())
								{
									// a.rank,a.account_id,a.total_force,c.name,c.level,c.head_id
									pUnit->nRank		= q.GetVal_32();
									pUnit->nAccountID	= q.GetVal_64();
									pUnit->nTotalForce	= q.GetVal_32();
									WH_STRNCPY0(pUnit->szName, q.GetStr());
									pUnit->nLevel		= q.GetVal_32();
									pUnit->nHeadID		= q.GetVal_32();

									pUnit++;
								}
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
								return 0;
							}
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_arena_status,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ARENA_PAY_REQ:
		{
			P_DBS4WEB_ARENA_PAY_T*	pReq	= (P_DBS4WEB_ARENA_PAY_T*)pCmd;
			P_DBS4WEBUSER_ARENA_PAY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ARENA_PAY_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nMoneyType	= pReq->nMoneyType;
			RplCmd.nPrice		= 0;
			RplCmd.nRst			= STC_GAMECMD_ARENA_PAY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_ARENA_PAY_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"pay_for_arena,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL pay_for_arena(0x%"WHINT64PRFX"X,%d,%d)"
							, pReq->nAccountID, pReq->nMoneyType, pReq->nNum);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_ARENA_PAY_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"pay_for_arena,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_price
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nPrice	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ARENA_PAY_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"pay_for_arena,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ARENA_DEAL_REWARD_EVENT_REQ:
		{
			P_DBS4WEB_ARENA_DEAL_REWARD_EVENT_T*	pReq	= (P_DBS4WEB_ARENA_DEAL_REWARD_EVENT_T*)pCmd;
			P_DBS4WEBUSER_ARENA_DEAL_REWARD_EVENT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ARENA_DEAL_REWARD_EVENT_RPL;

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_arena_reward_event,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL deal_arena_reward_event()");
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_arena_reward_event,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						int*	pLengths	= q.FetchLengths();
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ARENA_DEAL_REWARD_EVENT_T) + pLengths[0]+1);
						P_DBS4WEBUSER_ARENA_DEAL_REWARD_EVENT_T*	pRplCmd	= (P_DBS4WEBUSER_ARENA_DEAL_REWARD_EVENT_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						memcpy(wh_getptrnexttoptr(pRplCmd), q.GetStr(), pLengths[0]+1);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_ARENA_GET_DEPLOY_REQ:
		{
			P_DBS4WEB_ARENA_GET_DEPLOY_T*	pReq	= (P_DBS4WEB_ARENA_GET_DEPLOY_T*)pCmd;
			P_DBS4WEBUSER_ARENA_GET_DEPLOY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ARENA_GET_DEPLOY_RPL;
			RplCmd.nRst			= STC_GAMECMD_ARENA_GET_DEPLOY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst		= STC_GAMECMD_ARENA_GET_DEPLOY_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"arena_get_deploy,DBERR");
			}
			else
			{
				q.SpawnQuery("SELECT combat_data FROM arena WHERE account_id=0x%"WHINT64PRFX"X", pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (q.FetchRow())
				{
					int*	pLengths	= q.FetchLengths();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ARENA_GET_DEPLOY_T) + pLengths[0]+1);
					P_DBS4WEBUSER_ARENA_GET_DEPLOY_T*	pRplCmd	= (P_DBS4WEBUSER_ARENA_GET_DEPLOY_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					memcpy(wh_getptrnexttoptr(pRplCmd), q.GetStr(), pLengths[0]+1);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
				else
				{
					RplCmd.nRst	= STC_GAMECMD_ARENA_GET_DEPLOY_T::RST_NOT_UPLOAD;
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	}
	
	return 0;
}