#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_Quest(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_GET_QUEST_REQ:
		{
			P_DBS4WEB_GET_QUEST_T*	pReq		= (P_DBS4WEB_GET_QUEST_T*)pCmd;
			P_DBS4WEBUSER_GET_QUEST_T*	pRpl	= (P_DBS4WEBUSER_GET_QUEST_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_QUEST_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_QUEST_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get quest,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_GET_QUEST_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_quest(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get quest,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_GET_QUEST_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					QuestDesc *pQuest = (QuestDesc*)wh_getptrnexttoptr(pRpl);
					if (q.FetchRow())
					{
						// 返回字符串 ",1,0,2,0,...,n,0"
						pRpl->nNum = dbs_wc(q.GetStr(0), ',')/2;
						if (pRpl->nNum>0)
						{
							int idx = 0;
							char tmp[32];
							while (dbs_splitbychar(q.GetStr(0)+1, ',', idx, tmp) > 0)
							{
								pQuest->nExcelID	= atoi(tmp);
								dbs_splitbychar(q.GetStr(0)+1, ',', idx, tmp);
								pQuest->nDone		= atoi(tmp);
								++ pQuest;
							}
						}
					}

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get quest,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pQuest-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_DONE_QUEST_REQ:
		{
			P_DBS4WEB_DONE_QUEST_T*	pReq	= (P_DBS4WEB_DONE_QUEST_T*)pCmd;
			P_DBS4WEBUSER_DONE_QUEST_T	*pRpl	= (P_DBS4WEBUSER_DONE_QUEST_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_DONE_QUEST_RPL;
			pRpl->nRst			= STC_GAMECMD_DONE_QUEST_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			pRpl->nAccountID		= pReq->nAccountID;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"done quest,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID);
				pRpl->nRst		= STC_GAMECMD_DONE_QUEST_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_done_quest(0x%"WHINT64PRFX"X,%d,%s)", pReq->nAccountID, pReq->nExcelID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"done quest,SQLERR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID);
					pRpl->nRst	= STC_GAMECMD_DONE_QUEST_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						UseItemDesc *pDesc = (UseItemDesc*)wh_getptrnexttoptr(pRpl);
						AddItemDesc *pItem = (AddItemDesc*)wh_getptrnexttoptr(pDesc);
						// 返回字符串 "1*1*1*2*2*2,...,itemid*excelid*num"
						pDesc->nNum = dbs_wc(q.GetStr(1), '*');
						pDesc->nNum = pDesc->nNum==2? 1: (pDesc->nNum+1)/3;
						if (pDesc->nNum>0)
						{
							int idx = 0;
							char tmp[64];
							while (dbs_splitbychar(q.GetStr(1), '*', idx, tmp) > 0)
							{
								pItem->nItemID	= whstr2int64(tmp);
								dbs_splitbychar(q.GetStr(1), '*', idx, tmp);
								pItem->nExcelID	= atoi(tmp);
								dbs_splitbychar(q.GetStr(1), '*', idx, tmp);
								pItem->nNum		= atoi(tmp);
								++ pItem;
							}
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pItem-vectRawBuf.getbuf());
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"done quest,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pRpl->nRst);
					}
					else
					{
						pRpl->nRst	= STC_GAMECMD_DONE_QUEST_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"done quest,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID);
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
