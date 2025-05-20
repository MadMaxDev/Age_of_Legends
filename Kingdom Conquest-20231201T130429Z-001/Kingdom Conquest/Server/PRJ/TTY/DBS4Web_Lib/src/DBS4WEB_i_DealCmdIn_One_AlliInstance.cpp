#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;
int		DBS4Web_i::ThreadDealCmd_AlliInstance(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_ALLI_INSTANCE_CREATE_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_CREATE_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_CREATE_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_CREATE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_CREATE_RPL;
			RplCmd.nInstanceID	= 0;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_CREATE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_CREATE_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_create,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_create(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_CREATE_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_create,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_instance_id
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nInstanceID	= q.GetVal_64();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_CREATE_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_create,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_JOIN_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_JOIN_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_JOIN_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_JOIN_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_JOIN_RPL;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_JOIN_T::RST_OK;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_JOIN_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_join,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_join(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pReq->nAccountID, pReq->nInstanceID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_JOIN_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_join,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						RplCmd.nRst	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_JOIN_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_join,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_EXIT_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_EXIT_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_EXIT_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_EXIT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_EXIT_RPL;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_EXIT_T::RST_OK;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_EXIT_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_exit,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_exit(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,1,1,%s)", pReq->nAccountID, pReq->nInstanceID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_EXIT_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_exit,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						RplCmd.nRst	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_EXIT_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_exit,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_KICK_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_KICK_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_KICK_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_KICK_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_KICK_RPL;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_KICK_T::RST_OK;
			RplCmd.nObjID		= pReq->nObjID;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_KICK_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_kick,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_kick(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pReq->nAccountID, pReq->nInstanceID, pReq->nObjID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_KICK_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_kick,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_KICK_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_kick,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_DESTROY_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_DESTROY_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_DESTROY_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_DESTROY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_DESTROY_RPL;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_DESTROY_T::RST_OK;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_DESTROY_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_destroy,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_destroy(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,1,0,1,%s)", pReq->nAccountID, pReq->nInstanceID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_DESTROY_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_destroy,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_ids
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_DESTROY_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_destroy,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_CONFIG_HERO_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_CONFIG_HERO_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_CONFIG_HERO_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_CONFIG_HERO_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_CONFIG_HERO_RPL;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_config_hero,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_config_hero(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)"
					, pReq->nAccountID, pReq->nInstanceID, pReq->n1HeroID, pReq->n2HeroID, pReq->n3HeroID, pReq->n4HeroID, pReq->n5HeroID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_config_hero,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_CONFIG_HERO_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_config_hero,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_START_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_START_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_START_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_START_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_START_RPL;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_START_T::RST_OK;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			RplCmd.nNum			= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_START_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_start,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_start(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pReq->nAccountID, pReq->nInstanceID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_START_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_start,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_ids
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_START_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_start,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_READY_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_READY_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_READY_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_READY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_READY_RPL;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_READY_T::RST_OK;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			RplCmd.bReady		= pReq->bReady;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_READY_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_ready,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_ready(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d)"
					, pReq->nAccountID, pReq->nInstanceID, (int)pReq->bReady);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_READY_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_ready,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_READY_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_ready,SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_LIST_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_LIST_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_LIST_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_LIST_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_LIST_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_list,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_list(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_list,SQLERR");
				}
				else
				{
					RplCmd.nNum			= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_LIST_T)+RplCmd.nNum*sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T::ROOM_T));
						P_DBS4WEBUSER_ALLI_INSTANCE_GET_LIST_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_LIST_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T::ROOM_T*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_LIST_T::ROOM_T*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// i.instance_id,i.player_num,i.creator_id,c.name,c.level
							pUnit->nInstanceID	= q.GetVal_64();
							pUnit->nPlayerNum	= q.GetVal_32();
							pUnit->nCreatorID	= q.GetVal_64();
							WH_STRNCPY0(pUnit->szCreatorName, q.GetStr());
							pUnit->nLevel		= q.GetVal_32();
							pUnit->nHeadID		= q.GetVal_32();

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
	case CMDID_ALLI_INSTANCE_GET_CHAR_DATA_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_CHAR_DATA_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_CHAR_DATA_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_CHAR_DATA_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_CHAR_DATA_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_char_data,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_char_data(0x%"WHINT64PRFX"X)", pReq->nInstanceID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_char_data,SQLERR");
				}
				else
				{
					RplCmd.nNum			= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_CHAR_DATA_T)+RplCmd.nNum*sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T::CharData));
						P_DBS4WEBUSER_ALLI_INSTANCE_GET_CHAR_DATA_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_CHAR_DATA_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T::CharData*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_CHAR_DATA_T::CharData*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// i.status,c.account_id,c.name,c.level
							pUnit->nStatus		= q.GetVal_32();
							pUnit->nAccountID	= q.GetVal_64();
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
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_HERO_DATA_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_HERO_DATA_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_HERO_DATA_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_HERO_DATA_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_HERO_DATA_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_hero_data,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_hero_data(0x%"WHINT64PRFX"X)", pReq->nInstanceID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_hero_data,SQLERR");
				}
				else
				{
					RplCmd.nNum			= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_HERO_DATA_T)+RplCmd.nNum*sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T::HeroData));
						P_DBS4WEBUSER_ALLI_INSTANCE_GET_HERO_DATA_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_HERO_DATA_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T::HeroData*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_HERO_DATA_T::HeroData*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// hero_id,name,account_id,level,health,prof,army_type,army_level,army_num,row,col
							pUnit->nHeroID		= q.GetVal_64();
							WH_STRNCPY0(pUnit->szHeroName, q.GetStr());
							pUnit->nAccountID	= q.GetVal_64();
							pUnit->nLevel		= q.GetVal_32();
							pUnit->nHealth		= q.GetVal_32();
							pUnit->nProf		= q.GetVal_32();
							pUnit->nArmyType	= q.GetVal_32();
							pUnit->nArmyLevel	= q.GetVal_32();
							pUnit->nArmyNum		= q.GetVal_32();
							pUnit->nRow			= q.GetVal_32();
							pUnit->nCol			= q.GetVal_32();
							pUnit->nHeadID		= q.GetVal_32();

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
	case CMDID_ALLI_INSTANCE_GET_STATUS_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_STATUS_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_STATUS_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_STATUS_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_STATUS_RPL;
			RplCmd.nInstanceID	= 0;
			RplCmd.bAutoCombat	= false;
			RplCmd.bAutoSupply	= false;
			RplCmd.nCharStatus	= instance_player_status_normal;
			RplCmd.nCurLevel	= 0;
			RplCmd.nInstanceStatus	= instance_status_normal;
			RplCmd.nRetryTimes		= 0;
			RplCmd.nTotalFreeTimes	= 0;
			RplCmd.nUsedFreeTimes	= 0;
			RplCmd.nCreatorID	= 0;
			RplCmd.nStopLevel	= 0;
			RplCmd.nMaxLevel	= 0;
			RplCmd.nCombatType	= pReq->nCombatType;
			RplCmd.nLastCombatResult	= 0;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_STATUS_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_STATUS_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_status,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_status(0x%"WHINT64PRFX"X,%d)", pReq->nAccountID, pReq->nCombatType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_STATUS_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_status,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// e.day_times_free,p.day_times_free,p.instance_id,p.status,i.status,i.level
						// ,i.auto_combat,i.auto_supply,i.retry_times,i.creaetor_id,@_max_level,i.combat_result
						RplCmd.nTotalFreeTimes	= q.GetVal_32();
						RplCmd.nUsedFreeTimes	= q.GetVal_32();
						RplCmd.nInstanceID		= q.GetVal_64();
						RplCmd.nCharStatus		= q.GetVal_32();
						RplCmd.nInstanceStatus	= q.GetVal_32();
						RplCmd.nCurLevel		= q.GetVal_32();
						RplCmd.bAutoCombat		= (bool)q.GetVal_32();
						RplCmd.bAutoSupply		= (bool)q.GetVal_32();
						RplCmd.nRetryTimes		= q.GetVal_32();
						RplCmd.nCreatorID		= q.GetVal_64();
						RplCmd.nStopLevel		= q.GetVal_32();
						RplCmd.nMaxLevel		= q.GetVal_32();
						RplCmd.nLastCombatResult	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_STATUS_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_status,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_LOOT_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_LOOT_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_LOOT_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_LOOT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_LOOT_RPL;
			RplCmd.nInstanceID	= pReq->nInstanceID;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)",DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_loot(0x%"WHINT64PRFX"X)", pReq->nInstanceID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_LOOT_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)",SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						int*	pLengths	= q.FetchLengths();
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_LOOT_T) + pLengths[0]+1);
						P_DBS4WEBUSER_ALLI_INSTANCE_GET_LOOT_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_LOOT_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						memcpy(wh_getptrnexttoptr(pRplCmd), q.GetStr(), pLengths[0]+1);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_SAVE_HERO_DEPLOY_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_SAVE_HERO_DEPLOY_RPL;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)",DBERR");
			}
			else
			{
				char*	pDeploy		= (char*)wh_getptrnexttoptr(pReq);
				q.SpawnQuery("CALL alli_instance_save_hero_deploy(0x%"WHINT64PRFX"X,'%s',1,1,%s)"
					, pReq->nAccountID, pDeploy, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)",SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						RplCmd.nRst		= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_SAVE_HERO_DEPLOY_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)",SQLERR");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_START_COMBAT_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_START_COMBAT_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_START_COMBAT_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_START_COMBAT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_START_COMBAT_RPL;
			RplCmd.nMarchTime	= 0;
			RplCmd.nEventID		= 0;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nObjID		= pReq->nObjID;
			RplCmd.nCombatType	= pReq->nCombatType;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_START_COMBAT_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(pReq->nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_START_COMBAT_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_start_combat,DBERR");
			}
			else
			{
				char*	pDeploy	= (char*)wh_getptrnexttoptr(pReq);
				q.SpawnQuery("CALL alli_instance_start_combat(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d,'%s',1,1,1,%s)", 
					pReq->nAccountID, pReq->nObjID, pReq->nCombatType, (int)pReq->bAutoCombat, (int)pReq->bAutoSupply, pReq->nStopLevel, pDeploy, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_START_COMBAT_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_start_combat,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nEventID	= q.GetVal_64();
						RplCmd.nMarchTime	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_START_COMBAT_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_start_combat,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_REQ;
			
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_march_advance_event,DBERR");
			}
			else
			{
				 q.SpawnQuery("CALL deal_alli_instance_march_advance_event(0x%"WHINT64PRFX"X)", pReq->nEventID);
				 int	nPreRet	= 0;
				 q.ExecuteSPWithResult(nPreRet);
				 if (nPreRet != MYSQL_QUERY_NORMAL)
				 {
					 GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_march_advance_event,SQLERR");
				 }
				 else
				 {
					 if (q.FetchRow())
					 {
						 RplCmd.nRst	= q.GetVal_32();
					 }
					 else
					 {
						  GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_march_advance_event,SQLERR,NO RESULT");
					 }
				 }
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_DEAL_MARCH_BACK_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_DEAL_MARCH_BACK_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_DEAL_MARCH_BACK_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_MARCH_BACK_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_DEAL_MARCH_BACK_RPL;
			RplCmd.bInstanceDestroyed	= false;
			RplCmd.nCombatType	= 0;
			RplCmd.nAccountID	= 0;
			RplCmd.nObjID		= 0;
			RplCmd.nEventID		= pReq->nEventID;
			RplCmd.nInstanceLevel		= 0;
			RplCmd.bAutoSupply	= false;

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_march_back_event,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL deal_alli_instance_march_back_event(0x%"WHINT64PRFX"X)", pReq->nEventID);
				int	nPreRet	= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_march_back_event,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_instance_destroyed,@_combat_type,@_account_id,@_obj_id,@_instance_level,@_auto_supply,@_hero_deploy,@_account_ids
						int*	pLengths			= q.FetchLengths();
						int	nRst	= q.GetVal_32();
						RplCmd.bInstanceDestroyed	= (bool)q.GetVal_32();
						RplCmd.nCombatType			= q.GetVal_32();
						RplCmd.nAccountID			= q.GetVal_64();
						RplCmd.nObjID				= q.GetVal_64();
						RplCmd.nInstanceLevel		= q.GetVal_32();
						RplCmd.bAutoSupply			= (bool)q.GetVal_32();
						
						if (nRst != 0)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_march_back_event,0x%"WHINT64PRFX"X,%d"
								, pReq->nEventID, nRst);
						}
						else
						{
							SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

							if (RplCmd.bAutoSupply)
							{
								whvector<char>		vectHeroDeploy;
								vectHeroDeploy.resize(pLengths[7] + 1);
								memcpy(vectHeroDeploy.getbuf(), q.GetStr(7), vectHeroDeploy.size());

								whvector<char>		vectAccountIDs;
								vectAccountIDs.resize(pLengths[8]+1);
								memcpy(vectAccountIDs.getbuf(), q.GetStr(8), vectAccountIDs.size());

								const char*	pHeroDeploy	= vectHeroDeploy.getbuf();
								if (pHeroDeploy[0] != 0)
								{
									char			szBuf[1024]	= "";
									int				nLength		= 0;
									int				nOffset		= 0;
									char			szOne[1024]	= "";
									while (wh_strsplit(&nOffset, "s", pHeroDeploy+1, ",", szOne) == 1)
									{
										int	nInnerOffset		= 0;
										unsigned int	nCol	= 0;
										unsigned int	nRow	= 0;
										tty_id_t		nHeroID	= 0;
										while(wh_strsplit(&nInnerOffset, "ddI", szOne+1, "*", &nCol, &nRow, &nHeroID) == 3)
										{
											if (nLength	== 0)
											{
												nLength				= sprintf(szBuf, "0x%"WHINT64PRFX"X", nHeroID);
											}
											else
											{
												nLength				+= sprintf(szBuf+nLength, ",0x%"WHINT64PRFX"X", nHeroID);
											}
										}
									}
									szBuf[nLength]				= 0;
									tty_id_t		nExtID		= 0;
									switch (RplCmd.nCombatType)
									{
									case combat_type_alli_instance:
										{
											nExtID				= RplCmd.nObjID;
										}
										break;
									case combat_type_arena:
										{

										}
										break;
									default:
										{
											nExtID				= RplCmd.nAccountID;
										}
										break;
									}
									PushHeroSimpleData(pCDCmd->nConnecterID, q, RplCmd.nCombatType, nExtID, szBuf, STC_GAMECMD_HERO_SIMPLE_DATA_T::reason_auto_supply);
								}

								PushDrug(pCDCmd->nConnecterID, q, vectAccountIDs.getbuf());
							}
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_march_back_event,SQLERR,NO RESULT");
					}
				}
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_DEAL_COMBAT_RESULT_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T*		pReq	= (P_DBS4WEB_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_DEAL_COMBAT_RESULT_RPL;
			RplCmd.nRst			= 0;
			RplCmd.nAccountID	= 0;
			RplCmd.nCombatResult= 0;
			RplCmd.nCombatType	= 0;
			RplCmd.nObjID		= 0;
			RplCmd.nEventID		= pReq->nEventID;
			RplCmd.nBackLeftTime		= 0;
			RplCmd.nInstanceLevel		= 0;
			RplCmd.bRefreshCharAtb		= false;

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_combat_result,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL deal_alli_instance_combat_result(0x%"WHINT64PRFX"X)", pReq->nEventID);
				int	nPreRet	= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_combat_result,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_use_time,@_cur_level,@_combat_result,@_combat_type,_account_id,@_obj_id,@_need_refresh_char_atb,@_mail_ids,@_notify,@_hero_ids,@_account_ids
						RplCmd.nRst				= q.GetVal_32();
						RplCmd.nBackLeftTime	= q.GetVal_32();
						RplCmd.nInstanceLevel	= q.GetVal_32();
						RplCmd.nCombatResult	= q.GetVal_32();
						RplCmd.nCombatType		= q.GetVal_32();
						RplCmd.nAccountID		= q.GetVal_64();
						RplCmd.nObjID			= q.GetVal_64();
						RplCmd.bRefreshCharAtb	= (bool)q.GetVal_32();
						if (RplCmd.nRst != 0)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_combat_result,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
								, pReq->nEventID, RplCmd.nRst, RplCmd.nCombatResult, RplCmd.nCombatType, RplCmd.nAccountID, RplCmd.nObjID);
						}
						else
						{
							int*	pLengths	= q.FetchLengths();
							// 结构体+邮件ID字串+通知字串+英雄ID串+君主ID串
							int		nTotalSize	= sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T) + (sizeof(int)+pLengths[9]+1);
							vectRawBuf.resize(nTotalSize);
							P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_DEAL_COMBAT_RESULT_T*)vectRawBuf.getbuf();
							memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
							int*	pLength		= (int*)wh_getptrnexttoptr(pRplCmd);
							*pLength			= pLengths[9]+1;
							void*	pPos		= wh_getptrnexttoptr(pLength);
							memcpy(pPos, q.GetStr(9), *pLength);
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());

							whvector<char>	vectTmpMailIDsBuf;
							vectTmpMailIDsBuf.resize(pLengths[8]+1);
							memcpy(vectTmpMailIDsBuf.getbuf(), q.GetStr(8), pLengths[8]+1);
							
							whvector<char>	vectTmpHeroIDsBuf;
							vectTmpHeroIDsBuf.resize(pLengths[10]+1);
							memcpy(vectTmpHeroIDsBuf.getbuf(), q.GetStr(10), pLengths[10]+1);
							
							whvector<char>	vectTmpAccountIDsBuf;
							vectTmpAccountIDsBuf.resize(pLengths[11]+1);
							memcpy(vectTmpAccountIDsBuf.getbuf(), q.GetStr(11), pLengths[11]+1);

							PushMail(pCDCmd->nConnecterID, q, vectTmpMailIDsBuf.getbuf());
							PushHeroSimpleData(pCDCmd->nConnecterID, q, RplCmd.nCombatType, RplCmd.nObjID, vectTmpHeroIDsBuf.getbuf(), STC_GAMECMD_HERO_SIMPLE_DATA_T::reason_fight_cost);
							PushCharAtb(pCDCmd->nConnecterID, q, vectTmpAccountIDsBuf.getbuf(), true);
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_alli_instance_march_advance_event,SQLERR,NO RESULT");
					}
				}
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_REQ:
		{
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_combat_result_event,DBERR");
			}
			else
			{
				dia_mysql_query	q(pMySQL, QUERY_LEN);
				if (!q.Connected())
				{
				}
				else
				{
					q.SpawnQuery("CALL alli_instance_get_combat_result_event()");
					int	nPreRet	= 0;
					q.ExecuteSPWithResult(nPreRet, false);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_combat_result_event,SQLERR");
					}
					else
					{
						int	nNum	= q.NumRows();
						if (nNum > 0)
						{
							vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T) + nNum*sizeof(tty_id_t));
							P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_T*)vectRawBuf.getbuf();
							pRplCmd->nCmd		= P_DBS4WEB_RPL_CMD;
							pRplCmd->nSubCmd	= CMDID_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_RPL;
							pRplCmd->nNum		= nNum;
							tty_id_t*	pID		= (tty_id_t*)wh_getptrnexttoptr(pRplCmd);
							while (q.FetchRow())
							{
								*pID			= q.GetVal_64();
								++pID;
							}
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						}
					}
				}
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_INSTANCE_ID_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_INSTANCE_ID_T*	pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_INSTANCE_ID_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_INSTANCE_ID_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_INSTANCE_ID_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_instance_id,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_instance_id(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_instance_id,SQLERR");
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_INSTANCE_ID_T) + RplCmd.nNum*sizeof(tty_id_t));
						P_DBS4WEBUSER_ALLI_INSTANCE_GET_INSTANCE_ID_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_INSTANCE_ID_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						tty_id_t*	pID	= (tty_id_t*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							*pID		= q.GetVal_64();
							pID++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_COMBAT_LOG_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_COMBAT_LOG_T*	pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_COMBAT_LOG_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_LOG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_COMBAT_LOG_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nCombatType	= pReq->nCombatType;
			RplCmd.nID			= pReq->nID;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_combat_log,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_combat_log(0x%"WHINT64PRFX"X,%d)", pReq->nID, pReq->nCombatType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_combat_log,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						int*	pLengths	= q.FetchLengths();
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_LOG_T)+pLengths[0]);
						P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_LOG_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_COMBAT_LOG_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						memcpy(wh_getptrnexttoptr(pRplCmd), q.GetStr(), pLengths[0]);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_GET_COMBAT_LOG_T::RST_NO_RECORD;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*	pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nBackLeftTime	= 0;
			RplCmd.nCombatResult	= 0;
			RplCmd.nCombatType		= pReq->nCombatType;
			RplCmd.nInstanceID		= pReq->nInstanceID;
			RplCmd.nInstanceLevel	= 0;
			RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_simple_combat_log,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_simple_combat_log(0x%"WHINT64PRFX"X,%d)", pReq->nInstanceID, pReq->nCombatType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_simple_combat_log,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// 剩余时间,战斗结果,副本当前关卡
						RplCmd.nBackLeftTime	= q.GetVal_32();
						RplCmd.nCombatResult	= q.GetVal_32();
						RplCmd.nInstanceLevel	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_T::RST_NO_RECORD;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_STOP_COMBAT_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_STOP_COMBAT_T*	pReq	= (P_DBS4WEB_ALLI_INSTANCE_STOP_COMBAT_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_STOP_COMBAT_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_ALLI_INSTANCE_STOP_COMBAT_RPL;
			RplCmd.nAccountID		= pReq->nAccountID;
			RplCmd.nCombatType		= pReq->nCombatType;
			RplCmd.nInstanceID		= pReq->nInstanceID;
			RplCmd.nRst				= STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_stop_combat,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_stop_combat(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X)"
					, pReq->nAccountID, pReq->nCombatType, pReq->nInstanceID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_stop_combat,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_STOP_COMBAT_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_stop_combat,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_SUPPLY_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_SUPPLY_T*	pReq	= (P_DBS4WEB_ALLI_INSTANCE_SUPPLY_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_SUPPLY_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_ALLI_INSTANCE_SUPPLY_RPL;
			RplCmd.nAccountID		= pReq->nAccountID;
			RplCmd.nCombatType		= pReq->nCombatType;
			RplCmd.nInstanceID		= pReq->nInstanceID;
			RplCmd.nRst				= STC_GAMECMD_ALLI_INSTANCE_SUPPLY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_SUPPLY_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_supply,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_supply(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X)"
					, pReq->nAccountID, pReq->nCombatType, pReq->nInstanceID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= STC_GAMECMD_ALLI_INSTANCE_SUPPLY_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_supply,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_hero_ids,@_account_ids
						int*	pLengths= q.FetchLengths();
						RplCmd.nRst		= q.GetVal_32();

						if (RplCmd.nRst == STC_GAMECMD_ALLI_INSTANCE_SUPPLY_T::RST_OK)
						{
							// 需要先缓存数据,因为Push函数中会将result清理掉(FreeResult)
							whvector<char>	vectHeroIDsBuf;
							vectHeroIDsBuf.resize(pLengths[1]+1);
							memcpy(vectHeroIDsBuf.getbuf(), q.GetStr(), vectHeroIDsBuf.size());

							whvector<char>	vectAccountIDsBuf;
							vectAccountIDsBuf.resize(pLengths[2]+1);
							memcpy(vectAccountIDsBuf.getbuf(), q.GetStr(), vectAccountIDsBuf.size());

							PushHeroSimpleData(pCDCmd->nConnecterID, q, pReq->nCombatType, pReq->nInstanceID, vectHeroIDsBuf.getbuf(), STC_GAMECMD_HERO_SIMPLE_DATA_T::reason_self_supply);
							PushDrug(pCDCmd->nConnecterID, q, vectAccountIDsBuf.getbuf());
						}
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_ALLI_INSTANCE_SUPPLY_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_supply,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_INSTANCE_SYSTEM_DESTROY_REQ:
		{
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_system_destroy,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_system_destroy()");
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_system_destroy,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						int*	pLengths	= q.FetchLengths();
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_SYSTEM_DESTROY_T) + pLengths[0]+1);
						P_DBS4WEBUSER_ALLI_INSTANCE_SYSTEM_DESTROY_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_SYSTEM_DESTROY_T*)vectRawBuf.getbuf();
						pRplCmd->nCmd		= P_DBS4WEB_RPL_CMD;
						pRplCmd->nSubCmd	= CMDID_ALLI_INSTANCE_SYSTEM_DESTROY_RPL;
						memcpy(wh_getptrnexttoptr(pRplCmd), q.GetStr(), pLengths[0]+1);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_system_destroy,SQLERR,NO RESULT");
					}
				}
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_REQ:
		{
			P_DBS4WEB_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*	pReq	= (P_DBS4WEB_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*)pCmd;
			P_DBS4WEBUSER_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_RPL;
			RplCmd.nExcelID			= pReq->nExcelID;
			RplCmd.nLevel			= pReq->nLevel;
			RplCmd.nNum				= 0;
			RplCmd.nRst				= STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_defender_deploy,DBERR");
				RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_defender_deploy(%d,%d)", pReq->nExcelID, pReq->nLevel);
				int	nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_defender_deploy,SQLERR");
					RplCmd.nRst			= STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T::RST_SQL_ERR;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T) + RplCmd.nNum*sizeof(STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T::ArmyUnit));
						P_DBS4WEBUSER_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*	pRplCmd	= (P_DBS4WEBUSER_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T::ArmyUnit*	pUnit	= (STC_GAMECMD_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_T::ArmyUnit*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// prof,army_type,row,col
							pUnit->nForce		= q.GetVal_32();
							pUnit->nArmyType	= q.GetVal_32();
							pUnit->nRow			= q.GetVal_32();
							pUnit->nCol			= q.GetVal_32();

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