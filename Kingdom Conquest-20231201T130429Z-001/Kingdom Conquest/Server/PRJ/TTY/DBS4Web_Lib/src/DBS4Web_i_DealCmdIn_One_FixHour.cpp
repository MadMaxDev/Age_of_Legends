#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_FixHour(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_DEAL_FIXHOUR_TE_REQ:
		{
			P_DBS4WEB_DEAL_FIXHOUR_TE_T*	pReq	= (P_DBS4WEB_DEAL_FIXHOUR_TE_T*)pCmd;
			P_DBS4WEBUSER_FIXHOUR_RST_T	*pRpl	= (P_DBS4WEBUSER_FIXHOUR_RST_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd			= CMDID_FIXHOUR_RPL;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal fixhour te,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nEventID);
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_deal_te_fixhour(0x%"WHINT64PRFX"X,%s)", pReq->nEventID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal fixhour te,SQLERR,0x%"WHINT64PRFX"X,%d", pReq->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->nType		= q.GetVal_32();
						switch (pRpl->nType)
						{
						case FIXHOUR_TYPE_DRUG:
							{
								// 返回字符串 ",1,0,2,0,...,n,0"
								pRpl->u.nNum	= dbs_wc(q.GetStr(2), ',')/2;
								AcctDrugDesc *pDrug	= (AcctDrugDesc*)wh_getptrnexttoptr(pRpl);
								if (pRpl->u.nNum>0)
								{
									int idx = 0;
									char tmp[32];
									while (dbs_splitbychar(q.GetStr(2)+1, ',', idx, tmp) > 0)
									{
										pDrug->nID	= whstr2int64(tmp);
										dbs_splitbychar(q.GetStr(2)+1, ',', idx, tmp);
										pDrug->nNum	= atoi(tmp);
										++ pDrug;
									}
								}
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pDrug-vectRawBuf.getbuf());
							}break;
						}
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal fixhour te,0x%"WHINT64PRFX"X", pReq->nEventID);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal fixhour te,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nEventID);
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_RECHARGE_TRY_REQ:
		{
			P_DBS4WEB_RECHARGE_TRY_T*	pReq	= (P_DBS4WEB_RECHARGE_TRY_T*)pCmd;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"recharge try,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nDiamond);
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_recharge_try(0x%"WHINT64PRFX"X,%d,%s)", pReq->nAccountID, pReq->nDiamond, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"recharge try,SQLERR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nDiamond);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"recharge try,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nDiamond);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"recharge try,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nDiamond);
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
