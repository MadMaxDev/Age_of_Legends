#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_Instance(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_GET_INSTANCESTATUS_REQ:
		{
			P_DBS4WEB_GET_INSTANCESTATUS_T*	pReq		= (P_DBS4WEB_GET_INSTANCESTATUS_T*)pCmd;
			P_DBS4WEBUSER_GET_INSTANCESTATUS_T*	pRpl	= (P_DBS4WEBUSER_GET_INSTANCESTATUS_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_INSTANCESTATUS_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_INSTANCESTATUS_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instance status,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_GET_INSTANCESTATUS_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_instancestatus(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instance status,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_GET_INSTANCESTATUS_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum	= q.NumRows();
					InstanceStatus *pInstance = (InstanceStatus*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pInstance->nExcelID			= q.GetVal_32();
						pInstance->nResDayTimesFree	= q.GetVal_32();
						pInstance->nDayTimesFree	= q.GetVal_32();
						pInstance->nResDayTimesFee	= q.GetVal_32();
						pInstance->nDayTimesFee		= q.GetVal_32();
						pInstance->nInstanceID		= q.GetVal_64();
						pInstance->nStatus			= q.GetVal_32();
						pInstance->nClassID			= q.GetVal_32();
						pInstance->nCreatorID		= q.GetVal_64();
						pInstance->nCurLevel		= q.GetVal_32();
						++ pInstance;
					}

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get instance status,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pInstance-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_INSTANCEDESC_REQ:
		{
			P_DBS4WEB_GET_INSTANCEDESC_T*	pReq		= (P_DBS4WEB_GET_INSTANCEDESC_T*)pCmd;
			P_DBS4WEBUSER_GET_INSTANCEDESC_T*	pRpl	= (P_DBS4WEBUSER_GET_INSTANCEDESC_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_INSTANCEDESC_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_INSTANCEDESC_T::RST_OK;
			pRpl->nClassID		= pReq->nClassID;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instance desc,DBERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nClassID);
				pRpl->nRst		= STC_GAMECMD_GET_INSTANCEDESC_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_instancedesc(0x%"WHINT64PRFX"X,%d,%d,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nClassID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instance desc,SQLERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nClassID);
					pRpl->nRst	= STC_GAMECMD_GET_INSTANCEDESC_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum	= q.NumRows();
					InstanceDesc *pInstance = (InstanceDesc*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pInstance->nInstanceID		= q.GetVal_64();
						strcpy(pInstance->szName, q.GetStr());
						pInstance->nCurLevel		= q.GetVal_32();
						pInstance->nLeaderLevel		= q.GetVal_32();
						pInstance->nCurPlayer		= q.GetVal_32();
						pInstance->nCreatorID		= q.GetVal_64();
						++ pInstance;
					}

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get instance desc,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nClassID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pInstance-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_CREATE_INSTANCE_REQ:
		{
			P_DBS4WEB_CREATE_INSTANCE_T*	pReq	= (P_DBS4WEB_CREATE_INSTANCE_T*)pCmd;
			P_DBS4WEBUSER_CREATE_INSTANCE_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_CREATE_INSTANCE_RPL;
			Rpl.nRst			= STC_GAMECMD_CREATE_INSTANCE_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			Rpl.nAccountID		= pReq->nAccountID;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"create instance,DBERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nClassID);
				Rpl.nRst			= STC_GAMECMD_CREATE_INSTANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_create_instance(0x%"WHINT64PRFX"X,%d,%d,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nClassID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"create instance,SQLERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nClassID);
					Rpl.nRst	= STC_GAMECMD_CREATE_INSTANCE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						Rpl.nInstanceID	= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"create instance,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nClassID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"create instance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nClassID);
						Rpl.nRst	= STC_GAMECMD_CREATE_INSTANCE_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_JOIN_INSTANCE_REQ:
		{
			P_DBS4WEB_JOIN_INSTANCE_T*	pReq	= (P_DBS4WEB_JOIN_INSTANCE_T*)pCmd;
			P_DBS4WEBUSER_JOIN_INSTANCE_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_JOIN_INSTANCE_RPL;
			Rpl.nRst			= STC_GAMECMD_JOIN_INSTANCE_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			Rpl.nExcelID		= pReq->nExcelID;
			Rpl.nInstanceID		= pReq->nInstanceID;
			Rpl.nAccountID		= pReq->nAccountID;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
				Rpl.nRst			= STC_GAMECMD_JOIN_INSTANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_join_instance(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join instance,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
					Rpl.nRst	= STC_GAMECMD_JOIN_INSTANCE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"join instance,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join instance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
						Rpl.nRst	= STC_GAMECMD_JOIN_INSTANCE_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_QUIT_INSTANCE_REQ:
		{
			P_DBS4WEB_QUIT_INSTANCE_T*	pReq	= (P_DBS4WEB_QUIT_INSTANCE_T*)pCmd;
			P_DBS4WEBUSER_QUIT_INSTANCE_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_QUIT_INSTANCE_RPL;
			Rpl.nRst			= STC_GAMECMD_QUIT_INSTANCE_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			Rpl.nExcelID		= pReq->nExcelID;
			Rpl.nInstanceID		= pReq->nInstanceID;
			Rpl.nAccountID		= pReq->nAccountID;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"quit instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
				Rpl.nRst			= STC_GAMECMD_QUIT_INSTANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_quit_instance(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0,1,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"quit instance,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
					Rpl.nRst	= STC_GAMECMD_QUIT_INSTANCE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"quit instance,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"quit instance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
						Rpl.nRst	= STC_GAMECMD_QUIT_INSTANCE_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_DESTROY_INSTANCE_REQ:
		{
			P_DBS4WEB_DESTROY_INSTANCE_T*	pReq	= (P_DBS4WEB_DESTROY_INSTANCE_T*)pCmd;
			P_DBS4WEBUSER_DESTROY_INSTANCE_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_DESTROY_INSTANCE_RPL;
			Rpl.nRst			= STC_GAMECMD_DESTROY_INSTANCE_T::RST_OK;
			Rpl.nInstanceID		= pReq->nInstanceID;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"destroy instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
				Rpl.nRst			= STC_GAMECMD_DESTROY_INSTANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_destroy_instance(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0,1,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"destroy instance,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
					Rpl.nRst	= STC_GAMECMD_DESTROY_INSTANCE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						Rpl.nID1		= q.GetVal_64();
						Rpl.nID2		= q.GetVal_64();
						Rpl.nID3		= q.GetVal_64();
						Rpl.nID4		= q.GetVal_64();
						Rpl.nID5		= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"destroy instance,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"destroy instance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
						Rpl.nRst	= STC_GAMECMD_DESTROY_INSTANCE_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_INSTANCEDATA_REQ:
		{
			P_DBS4WEB_GET_INSTANCEDATA_T*	pReq		= (P_DBS4WEB_GET_INSTANCEDATA_T*)pCmd;
			P_DBS4WEBUSER_GET_INSTANCEDATA_T*	pRpl	= (P_DBS4WEBUSER_GET_INSTANCEDATA_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_INSTANCEDATA_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_INSTANCEDATA_T::RST_OK;
			pRpl->nNum			= 0;
			pRpl->nNumHero		= 0;
			pRpl->nInstanceID	= pReq->nInstanceID;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instance data,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
				pRpl->nRst		= STC_GAMECMD_GET_INSTANCEDATA_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("select cur_level,auto_combat,auto_supply,retry_times from instance where instance_id=0x%"WHINT64PRFX"X", pReq->nInstanceID);
				int nPreRet		= 0;
				q.GetResult(nPreRet);
				q.FetchRow();
				pRpl->nCurLevel	= q.GetVal_32();
				pRpl->nAutoCombat	= q.GetVal_32();
				pRpl->nAutoSupply	= q.GetVal_32();
				pRpl->nRetryTimes	= q.GetVal_32();
				q.FreeResult();

				q.SpawnQuery("call game_get_instancedata(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, SP_RESULT);
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instance data,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
					pRpl->nRst	= STC_GAMECMD_GET_INSTANCEDATA_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum	= q.NumRows();
					tty_id_t aHeroID[5];
					aHeroID[0]=aHeroID[1]=aHeroID[2]=aHeroID[3]=aHeroID[4]=0;
					tty_id_t *pHeroID	= &aHeroID[0];
					InstanceData *pInstance = (InstanceData*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pInstance->nAccountID		= q.GetVal_64();
						strcpy(pInstance->szName, q.GetStr());
						pInstance->nStatus			= q.GetVal_32(3);
						pInstance->n1Hero	= 0;
						pInstance->n2Hero	= 0;
						pInstance->n3Hero	= 0;
						pInstance->n4Hero	= 0;
						pInstance->n5Hero	= 0;
						// 返回字符串 ",1,2,...,n"
						int nNum = dbs_wc(q.GetStr(2), ',');
						if (nNum>0)
						{
							int idx = 0;
							char tmp[32];
							tty_id_t *pID	= (tty_id_t*)&(pInstance->n1Hero);
							while (dbs_splitbychar(q.GetStr(2)+1, ',', idx, tmp) > 0)
							{
								*pID	= whstr2int64(tmp);
								*pHeroID= *pID;
								++ pRpl->nNumHero;
								++ pHeroID;
								++ pID;
							}
						}
						++ pInstance;
					}
					// 如果是多人副本，还需要获取武将数据
					if (pReq->nExcelID==101)
					{
						CombatUnit *pUnit	= (CombatUnit*)pInstance;
						q.FreeResult();
						q.SpawnQuery("call game_get_herocombat(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", aHeroID[0], aHeroID[1], aHeroID[2], aHeroID[3], aHeroID[4], SP_RESULT);
						q.ExecuteSPWithResult(nPreRet);
						if (nPreRet != MYSQL_QUERY_NORMAL)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get hero combat,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", aHeroID[0], aHeroID[1], aHeroID[2], aHeroID[3], aHeroID[4]);
							pRpl->nRst	= STC_GAMECMD_GET_INSTANCEDATA_T::RST_SQL_ERR;
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
						}
						else
						{
							while (q.FetchRow())
							{
								pUnit->nHeroID		= q.GetVal_64();
								strcpy(pUnit->szName, q.GetStr());
								pUnit->nArmyType	= q.GetVal_32();
								pUnit->nArmyLevel	= q.GetVal_32();
								pUnit->nArmyNum		= q.GetVal_32();
								pUnit->nAttack		= q.GetVal_32();
								pUnit->nDefense		= q.GetVal_32();
								pUnit->nHealth		= q.GetVal_32();
								pUnit->nModel		= q.GetVal_32();
								pUnit->nLevel		= q.GetVal_32();
								pUnit->nHealthState	= q.GetVal_32();

								++ pUnit;
							}
						}

						// 移动一下尾部指针
						pInstance	= (InstanceData*)pUnit;
					}
					else
					{
						// 单人副本就不用传后面一段数据了
						pRpl->nNumHero	= 0;
					}

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get instance data,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pInstance-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_CONFIG_INSTANCEHERO_REQ:
		{
			P_DBS4WEB_CONFIG_INSTANCEHERO_T*	pReq	= (P_DBS4WEB_CONFIG_INSTANCEHERO_T*)pCmd;
			P_DBS4WEBUSER_CONFIG_INSTANCEHERO_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_CONFIG_INSTANCEHERO_RPL;
			Rpl.nRst			= STC_GAMECMD_CONFIG_INSTANCEHERO_T::RST_OK;
			Rpl.nExcelID		= pReq->nExcelID;
			Rpl.nInstanceID		= pReq->nInstanceID;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config instance hero,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
				Rpl.nRst			= STC_GAMECMD_CONFIG_INSTANCEHERO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_config_instancehero(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,1,%s)", 
					pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config instance hero,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
					Rpl.nRst	= STC_GAMECMD_CONFIG_INSTANCEHERO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"config instance hero,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"config instance hero,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
						Rpl.nRst	= STC_GAMECMD_CONFIG_INSTANCEHERO_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_INSTANCELOOT_REQ:
		{
			P_DBS4WEB_GET_INSTANCELOOT_T*	pReq		= (P_DBS4WEB_GET_INSTANCELOOT_T*)pCmd;
			P_DBS4WEBUSER_GET_INSTANCELOOT_T*	pRpl	= (P_DBS4WEBUSER_GET_INSTANCELOOT_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_INSTANCELOOT_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_INSTANCELOOT_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instance loot,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
				pRpl->nRst		= STC_GAMECMD_GET_INSTANCELOOT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_instanceloot(0x%"WHINT64PRFX"X,%s)", pReq->nInstanceID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instance loot,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
					pRpl->nRst	= STC_GAMECMD_GET_INSTANCELOOT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					LootDesc *pLoot = (LootDesc*)wh_getptrnexttoptr(pRpl);
					if (q.FetchRow())
					{
						// 返回字符串 "1*1*2*1,...,id*num"
						pRpl->nNum = dbs_wc(q.GetStr(0), '*');
						pRpl->nNum = pRpl->nNum==1? 1: (pRpl->nNum+1)/2;
						if (pRpl->nNum>0)
						{
							int idx = 0;
							char tmp[32];
							while (dbs_splitbychar(q.GetStr(0), '*', idx, tmp) > 0)
							{
								pLoot->nExcelID	= atoi(tmp);
								++ pLoot;
								// 跳过数量
								dbs_splitbychar(q.GetStr(0), '*', idx, tmp);
							}
						}
					}

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get instance loot,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pLoot-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_AUTO_COMBAT_REQ:
		{
			P_DBS4WEB_AUTO_COMBAT_T*	pReq	= (P_DBS4WEB_AUTO_COMBAT_T*)pCmd;
			P_DBS4WEBUSER_AUTO_COMBAT_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_AUTO_COMBAT_RPL;
			Rpl.nRst			= STC_GAMECMD_AUTO_COMBAT_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"auto combat,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nInstanceID, pReq->nVal);
				Rpl.nRst			= STC_GAMECMD_AUTO_COMBAT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_switch_autocombat(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,1,%s)", pReq->nAccountID, pReq->nInstanceID, pReq->nVal, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"auto combat,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nInstanceID, pReq->nVal);
					Rpl.nRst	= STC_GAMECMD_AUTO_COMBAT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"auto combat,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nInstanceID, pReq->nVal, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"auto combat,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nInstanceID, pReq->nVal);
						Rpl.nRst	= STC_GAMECMD_AUTO_COMBAT_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_AUTO_SUPPLY_REQ:
		{
			P_DBS4WEB_AUTO_SUPPLY_T*	pReq	= (P_DBS4WEB_AUTO_SUPPLY_T*)pCmd;
			P_DBS4WEBUSER_AUTO_SUPPLY_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_AUTO_SUPPLY_RPL;
			Rpl.nRst			= STC_GAMECMD_AUTO_SUPPLY_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"auto supply,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nInstanceID, pReq->nVal);
				Rpl.nRst			= STC_GAMECMD_AUTO_SUPPLY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_switch_autosupply(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,1,%s)", pReq->nAccountID, pReq->nInstanceID, pReq->nVal, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"auto supply,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nInstanceID, pReq->nVal);
					Rpl.nRst	= STC_GAMECMD_AUTO_SUPPLY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"auto supply,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nInstanceID, pReq->nVal, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"auto supply,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nInstanceID, pReq->nVal);
						Rpl.nRst	= STC_GAMECMD_AUTO_SUPPLY_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_MANUAL_SUPPLY_REQ:
		{
			P_DBS4WEB_MANUAL_SUPPLY_T*	pReq	= (P_DBS4WEB_MANUAL_SUPPLY_T*)pCmd;
			P_DBS4WEBUSER_MANUAL_SUPPLY_T*	pRpl	= (P_DBS4WEBUSER_MANUAL_SUPPLY_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd			= CMDID_MANUAL_SUPPLY_RPL;
			pRpl->nRst			= STC_GAMECMD_MANUAL_SUPPLY_T::RST_OK;
			pRpl->nNum			= 0;
			pRpl->nExcelID		= pReq->nCombatType;
			pRpl->nInstanceID	= pReq->nInstanceID;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			pRpl->nAccountID	= pReq->nAccountID;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"manual supply,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nInstanceID, pReq->nCombatType, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
				pRpl->nRst			= STC_GAMECMD_MANUAL_SUPPLY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_manual_supply(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", 
					pReq->nAccountID, pReq->nInstanceID, pReq->nCombatType, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"manual supply,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nInstanceID, pReq->nCombatType, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
					pRpl->nRst	= STC_GAMECMD_MANUAL_SUPPLY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->nExt[1]	= q.GetVal_32();
						tty_id_t *pID	= (tty_id_t*)&pRpl->nAccountID1;
						int *pDrug		= (int*)&pRpl->nDrug1;
						// 返回字符串 ",1,0,2,0,...,n,0"
						int idx = 0;
						char tmp[32];
						while (dbs_splitbychar(q.GetStr(2)+1, ',', idx, tmp) > 0)
						{
							*pID	= whstr2int64(tmp);
							dbs_splitbychar(q.GetStr(2)+1, ',', idx, tmp);
							*pDrug	= atoi(tmp);

							++ pID;
							++ pDrug;
						}

						InstanceHeroDesc *pHero = (InstanceHeroDesc*)wh_getptrnexttoptr(pRpl);
						// 获取副本武将摘要
						q.FreeResult();
						q.SpawnQuery("call game_get_instancehero(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0,%s)", 
							pReq->nAccountID, pReq->nInstanceID, pReq->nCombatType, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, SP_RESULT);
						q.ExecuteSPWithResult(nPreRet);
						pRpl->nNum		= q.NumRows();
						while (q.FetchRow())
						{
							pHero->nHeroID	= q.GetVal_64();
							pHero->nArmyNum	= q.GetVal_32();
							pHero->nArmyProf= q.GetVal_32();
							pHero->nHealthState	= q.GetVal_32();
							pHero->nLevel	= q.GetVal_32();
							pHero->nMaxArmyNum	= q.GetVal_32();

							++ pHero;
						}

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"manual supply,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nInstanceID, pReq->nCombatType, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"manual supply,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nInstanceID, pReq->nCombatType, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
						pRpl->nRst	= STC_GAMECMD_MANUAL_SUPPLY_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_KICK_INSTANCE_REQ:
		{
			P_DBS4WEB_KICK_INSTANCE_T*	pReq	= (P_DBS4WEB_KICK_INSTANCE_T*)pCmd;
			P_DBS4WEBUSER_KICK_INSTANCE_T*	pRpl	= (P_DBS4WEBUSER_KICK_INSTANCE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_KICK_INSTANCE_RPL;
			pRpl->nRst			= STC_GAMECMD_KICK_INSTANCE_T::RST_OK;
			pRpl->nExcelID		= pReq->nExcelID;
			pRpl->nInstanceID	= pReq->nInstanceID;
			pRpl->nObjID		= pReq->nObjID;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"kick instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nObjID);
				pRpl->nRst			= STC_GAMECMD_KICK_INSTANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_kick_instance(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nObjID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"kick instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nObjID);
					pRpl->nRst			= STC_GAMECMD_KICK_INSTANCE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"kick instance,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nObjID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"kick instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nObjID);
						pRpl->nRst			= STC_GAMECMD_KICK_INSTANCE_T::RST_DB_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_START_INSTANCE_REQ:
		{
			P_DBS4WEB_START_INSTANCE_T*	pReq	= (P_DBS4WEB_START_INSTANCE_T*)pCmd;
			P_DBS4WEBUSER_START_INSTANCE_T*	pRpl	= (P_DBS4WEBUSER_START_INSTANCE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_START_INSTANCE_RPL;
			pRpl->nRst			= STC_GAMECMD_START_INSTANCE_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"start instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
				pRpl->nRst			= STC_GAMECMD_START_INSTANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_start_instance(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"kick instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
					pRpl->nRst			= STC_GAMECMD_START_INSTANCE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->n1Hero=pRpl->n2Hero=pRpl->n3Hero=pRpl->n4Hero=pRpl->n5Hero=0;
						int idx = 0;
						char tmp[32];
						tty_id_t *pID	= (tty_id_t*)&(pRpl->n1Hero);
						while (dbs_splitbychar(q.GetStr(1), ',', idx, tmp) > 0)
						{
							*pID	= whstr2int64(tmp);
							++ pID;
						}
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"start instance,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"start instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
						pRpl->nRst			= STC_GAMECMD_START_INSTANCE_T::RST_DB_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_PREPARE_INSTANCE_REQ:
		{
			P_DBS4WEB_PREPARE_INSTANCE_T*	pReq	= (P_DBS4WEB_PREPARE_INSTANCE_T*)pCmd;
			P_DBS4WEBUSER_PREPARE_INSTANCE_T*	pRpl	= (P_DBS4WEBUSER_PREPARE_INSTANCE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_PREPARE_INSTANCE_RPL;
			pRpl->nRst			= STC_GAMECMD_PREPARE_INSTANCE_T::RST_OK;
			pRpl->nExcelID		= pReq->nExcelID;
			pRpl->nInstanceID	= pReq->nInstanceID;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"prepare instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", 
					pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nVal);
				pRpl->nRst			= STC_GAMECMD_PREPARE_INSTANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_prepare_instance(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,1,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nVal, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"prepare instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", 
						pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nVal);
					pRpl->nRst			= STC_GAMECMD_PREPARE_INSTANCE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->nStatus	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"prepare instance,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nVal, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"prepare instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pReq->nVal);
						pRpl->nRst			= STC_GAMECMD_PREPARE_INSTANCE_T::RST_DB_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_SUPPLY_INSTANCE_REQ:
		{
			P_DBS4WEB_SUPPLY_INSTANCE_T*	pReq	= (P_DBS4WEB_SUPPLY_INSTANCE_T*)pCmd;
			P_DBS4WEBUSER_SUPPLY_INSTANCE_T*	pRpl	= (P_DBS4WEBUSER_SUPPLY_INSTANCE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_SUPPLY_INSTANCE_RPL;
			pRpl->nRst			= STC_GAMECMD_SUPPLY_INSTANCE_T::RST_OK;
			pRpl->nExcelID		= pReq->nExcelID;
			pRpl->nInstanceID	= pReq->nInstanceID;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"supply instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
				pRpl->nRst			= STC_GAMECMD_SUPPLY_INSTANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_supply_instance(0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"supply instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
					pRpl->nRst			= STC_GAMECMD_SUPPLY_INSTANCE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->nExt[1]	= q.GetVal_32();
						tty_id_t *pID	= (tty_id_t*)&pRpl->nAccountID1;
						int *pDrug		= (int*)&pRpl->nDrug1;
						// 返回字符串 ",1,0,2,0,...,n,0"
						int idx = 0;
						char tmp[32];
						while (dbs_splitbychar(q.GetStr(2)+1, ',', idx, tmp) > 0)
						{
							*pID	= whstr2int64(tmp);
							dbs_splitbychar(q.GetStr(2)+1, ',', idx, tmp);
							*pDrug	= atoi(tmp);

							++ pID;
							++ pDrug;
						}

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"supply instance,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"supply instance,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nExcelID, pReq->nInstanceID);
						pRpl->nRst			= STC_GAMECMD_SUPPLY_INSTANCE_T::RST_DB_ERR;
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
