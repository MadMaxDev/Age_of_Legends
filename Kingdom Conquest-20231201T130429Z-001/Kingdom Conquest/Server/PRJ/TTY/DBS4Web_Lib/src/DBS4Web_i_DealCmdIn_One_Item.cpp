#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_Item(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_GET_ITEM_REQ:
		{
			P_DBS4WEB_GET_ITEM_T*	pReq		= (P_DBS4WEB_GET_ITEM_T*)pCmd;
			P_DBS4WEBUSER_GET_ITEM_T*	pRpl	= (P_DBS4WEBUSER_GET_ITEM_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_ITEM_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_ITEM_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get item,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_GET_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("select item_id,excel_id,num from items where account_id=0x%"WHINT64PRFX"X order by excel_id asc", pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get item,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_GET_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					ItemDesc *pItem = (ItemDesc*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pItem->nItemID		= q.GetVal_64();
						pItem->nExcelID		= q.GetVal_32();
						pItem->nNum			= q.GetVal_32();

						++pItem;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get item,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pItem-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_ITEM_REQ:
		{
			P_DBS4WEB_ADD_ITEM_T*	pReq	= (P_DBS4WEB_ADD_ITEM_T*)pCmd;
			P_DBS4WEBUSER_ADD_ITEM_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_ADD_ITEM_RPL;
			Rpl.nRst			= STC_GAMECMD_ADD_ITEM_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add item,DBERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
				Rpl.nRst			= STC_GAMECMD_ADD_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_add_item(0x%"WHINT64PRFX"X,0,%d,%d,1,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nNum, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add item,SQLERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
					Rpl.nRst	= STC_GAMECMD_ADD_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add item,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
						Rpl.nRst	= STC_GAMECMD_ADD_ITEM_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_DEL_ITEM_REQ:
		{
			P_DBS4WEB_DEL_ITEM_T*	pReq	= (P_DBS4WEB_DEL_ITEM_T*)pCmd;
			P_DBS4WEBUSER_DEL_ITEM_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_DEL_ITEM_RPL;
			Rpl.nRst			= STC_GAMECMD_DEL_ITEM_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"del item,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nItemID, pReq->nNum);
				Rpl.nRst			= STC_GAMECMD_DEL_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_del_item(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,1,%s)", pReq->nAccountID, pReq->nItemID, pReq->nNum, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"del item,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nItemID, pReq->nNum);
					Rpl.nRst	= STC_GAMECMD_DEL_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"del item,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nItemID, pReq->nNum, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"del item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nItemID, pReq->nNum);
						Rpl.nRst	= STC_GAMECMD_DEL_ITEM_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_EQUIP_ITEM_REQ:
		{
			P_DBS4WEB_EQUIP_ITEM_T*	pReq	= (P_DBS4WEB_EQUIP_ITEM_T*)pCmd;
			P_DBS4WEBUSER_EQUIP_ITEM_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_EQUIP_ITEM_RPL;
			Rpl.nRst			= STC_GAMECMD_EQUIP_ITEM_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"equip item,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID, pReq->nItemID);
				Rpl.nRst			= STC_GAMECMD_EQUIP_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_equip_item(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nHeroID, pReq->nItemID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"equip item,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID, pReq->nItemID);
					Rpl.nRst	= STC_GAMECMD_EQUIP_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"equip item,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nHeroID, pReq->nItemID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"equip item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID, pReq->nItemID);
						Rpl.nRst	= STC_GAMECMD_EQUIP_ITEM_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_DISEQUIP_ITEM_REQ:
		{
			P_DBS4WEB_DISEQUIP_ITEM_T*	pReq	= (P_DBS4WEB_DISEQUIP_ITEM_T*)pCmd;
			P_DBS4WEBUSER_DISEQUIP_ITEM_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_DISEQUIP_ITEM_RPL;
			Rpl.nRst			= STC_GAMECMD_DISEQUIP_ITEM_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"disequip item,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID, pReq->nItemID);
				Rpl.nRst			= STC_GAMECMD_DISEQUIP_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_disequip_item(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nHeroID, pReq->nItemID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"disequip item,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID, pReq->nItemID);
					Rpl.nRst	= STC_GAMECMD_DISEQUIP_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"disequip item,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nHeroID, pReq->nItemID, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"disequip item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID, pReq->nItemID);
						Rpl.nRst	= STC_GAMECMD_EQUIP_ITEM_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_MOUNT_ITEM_REQ:
		{
			P_DBS4WEB_MOUNT_ITEM_T*	pReq	= (P_DBS4WEB_MOUNT_ITEM_T*)pCmd;
			P_DBS4WEBUSER_MOUNT_ITEM_T	*pRpl	= (P_DBS4WEBUSER_MOUNT_ITEM_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd			= CMDID_MOUNT_ITEM_RPL;
			pRpl->nRst			= STC_GAMECMD_MOUNT_ITEM_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"mount item,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx, pReq->nExcelID);
				pRpl->nRst			= STC_GAMECMD_MOUNT_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_mount_item(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,1,%s)", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx, pReq->nExcelID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"mount item,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx, pReq->nExcelID);
					pRpl->nRst	= STC_GAMECMD_MOUNT_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRpl);
						if (pReq->nHeroID>0)
						{
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
						pHero->nArmyType	= q.GetVal_32();
						pHero->nArmyLevel	= q.GetVal_32();
						pHero->nArmyNum		= q.GetVal_32();
						pHero->nExp			= q.GetVal_32();
						pHero->nLevel		= q.GetVal_32();
						pHero->nProf		= q.GetVal_32();
						}
						++ pHero;
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"mount item,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx, pReq->nExcelID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"mount item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx, pReq->nExcelID);
						pRpl->nRst	= STC_GAMECMD_MOUNT_ITEM_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_UNMOUNT_ITEM_REQ:
		{
			P_DBS4WEB_UNMOUNT_ITEM_T*	pReq	= (P_DBS4WEB_UNMOUNT_ITEM_T*)pCmd;
			P_DBS4WEBUSER_UNMOUNT_ITEM_T	*pRpl	= (P_DBS4WEBUSER_UNMOUNT_ITEM_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd			= CMDID_UNMOUNT_ITEM_RPL;
			pRpl->nRst			= STC_GAMECMD_UNMOUNT_ITEM_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"unmount item,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx);
				pRpl->nRst			= STC_GAMECMD_UNMOUNT_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_unmount_item(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,1,%s)", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"unmount item,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx);
					pRpl->nRst	= STC_GAMECMD_UNMOUNT_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRpl);
						if (pReq->nHeroID>0)
						{
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
						pHero->nArmyType	= q.GetVal_32();
						pHero->nArmyLevel	= q.GetVal_32();
						pHero->nArmyNum		= q.GetVal_32();
						pHero->nExp			= q.GetVal_32();
						pHero->nLevel		= q.GetVal_32();
						pHero->nProf		= q.GetVal_32();
						}
						++ pHero;
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"unmount item,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"unmount item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->nSlotIdx);
						pRpl->nRst	= STC_GAMECMD_UNMOUNT_ITEM_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_COMPOS_ITEM_REQ:
		{
			P_DBS4WEB_COMPOS_ITEM_T*	pReq	= (P_DBS4WEB_COMPOS_ITEM_T*)pCmd;
			P_DBS4WEBUSER_COMPOS_ITEM_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_COMPOS_ITEM_RPL;
			Rpl.nRst			= STC_GAMECMD_COMPOS_ITEM_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"compos item,DBERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
				Rpl.nRst			= STC_GAMECMD_COMPOS_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_compos_item(0x%"WHINT64PRFX"X,%d,%d,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nNum, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"compos item,SQLERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
					Rpl.nRst	= STC_GAMECMD_COMPOS_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						Rpl.nItemID		= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"compos item,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"compos item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
						Rpl.nRst	= STC_GAMECMD_COMPOS_ITEM_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_DISCOMPOS_ITEM_REQ:
		{
			P_DBS4WEB_DISCOMPOS_ITEM_T*	pReq	= (P_DBS4WEB_DISCOMPOS_ITEM_T*)pCmd;
			P_DBS4WEBUSER_DISCOMPOS_ITEM_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_DISCOMPOS_ITEM_RPL;
			Rpl.nRst			= STC_GAMECMD_DISCOMPOS_ITEM_T::RST_OK;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"discompos item,DBERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
				Rpl.nRst			= STC_GAMECMD_DISCOMPOS_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_discompos_item(0x%"WHINT64PRFX"X,%d,%d,%s)", pReq->nAccountID, pReq->nExcelID, pReq->nNum, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"discompos item,SQLERR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
					Rpl.nRst	= STC_GAMECMD_DISCOMPOS_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"discompos item,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum, Rpl.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"discompos item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nExcelID, pReq->nNum);
						Rpl.nRst	= STC_GAMECMD_DISCOMPOS_ITEM_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_EQUIP_ITEM_ALL_REQ:
		{
			P_DBS4WEB_EQUIP_ITEM_ALL_T*	pReq	= (P_DBS4WEB_EQUIP_ITEM_ALL_T*)pCmd;
			P_DBS4WEBUSER_EQUIP_ITEM_ALL_T*	pRpl	= (P_DBS4WEBUSER_EQUIP_ITEM_ALL_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_EQUIP_ITEM_ALL_RPL;
			pRpl->nRst			= STC_GAMECMD_EQUIP_ITEM_ALL_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"equip item all,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nHeroID, pReq->n1HeadID, pReq->n2ChestID, pReq->n3ShoeID, pReq->n4WeaponID, pReq->n5TrinketID);
				pRpl->nRst			= STC_GAMECMD_EQUIP_ITEM_ALL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_equip_item_all(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", 
					pReq->nAccountID, pReq->nHeroID, pReq->n1HeadID, pReq->n2ChestID, pReq->n3ShoeID, pReq->n4WeaponID, pReq->n5TrinketID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"equip item all,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nHeroID, pReq->n1HeadID, pReq->n2ChestID, pReq->n3ShoeID, pReq->n4WeaponID, pReq->n5TrinketID);
					pRpl->nRst	= STC_GAMECMD_EQUIP_ITEM_ALL_T::RST_SQL_ERR;
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
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"equip item all,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nHeroID, pReq->n1HeadID, pReq->n2ChestID, pReq->n3ShoeID, pReq->n4WeaponID, pReq->n5TrinketID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"equip item all,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nHeroID, pReq->n1HeadID, pReq->n2ChestID, pReq->n3ShoeID, pReq->n4WeaponID, pReq->n5TrinketID);
						pRpl->nRst	= STC_GAMECMD_EQUIP_ITEM_ALL_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_MOUNT_ITEM_ALL_REQ:
		{
			P_DBS4WEB_MOUNT_ITEM_ALL_T*	pReq	= (P_DBS4WEB_MOUNT_ITEM_ALL_T*)pCmd;
			P_DBS4WEBUSER_MOUNT_ITEM_ALL_T*	pRpl	= (P_DBS4WEBUSER_MOUNT_ITEM_ALL_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_MOUNT_ITEM_ALL_RPL;
			pRpl->nRst			= STC_GAMECMD_MOUNT_ITEM_ALL_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"mount item all,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->n1ExcelID, pReq->n2ExcelID, pReq->n3ExcelID, pReq->n4ExcelID);
				pRpl->nRst			= STC_GAMECMD_MOUNT_ITEM_ALL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_mount_item_all(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%s)", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->n1ExcelID, pReq->n2ExcelID, pReq->n3ExcelID, pReq->n4ExcelID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"mount item all,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->n1ExcelID, pReq->n2ExcelID, pReq->n3ExcelID, pReq->n4ExcelID);
					pRpl->nRst	= STC_GAMECMD_MOUNT_ITEM_ALL_T::RST_SQL_ERR;
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
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"mount item all,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->n1ExcelID, pReq->n2ExcelID, pReq->n3ExcelID, pReq->n4ExcelID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"mount item all,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nEquipID, pReq->n1ExcelID, pReq->n2ExcelID, pReq->n3ExcelID, pReq->n4ExcelID);
						pRpl->nRst	= STC_GAMECMD_MOUNT_ITEM_ALL_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_USE_ITEM_REQ:
		{
			P_DBS4WEB_USE_ITEM_T*	pReq	= (P_DBS4WEB_USE_ITEM_T*)pCmd;
			P_DBS4WEBUSER_USE_ITEM_T	*pRpl	= (P_DBS4WEBUSER_USE_ITEM_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd			= CMDID_USE_ITEM_RPL;
			pRpl->nRst			= STC_GAMECMD_USE_ITEM_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			pRpl->nAccountID	= pReq->nAccountID;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"use item,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nExcelID, pReq->nNum);
				pRpl->nRst			= STC_GAMECMD_USE_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_use_item(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,1,%s)", pReq->nAccountID, pReq->nHeroID, pReq->nExcelID, pReq->nNum, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"use item,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nExcelID, pReq->nNum);
					pRpl->nRst	= STC_GAMECMD_USE_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nType		= q.GetVal_32();
						switch (pRpl->nType)
						{
						case SYNC_TYPE_HERO:
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
								pHero->nArmyType	= q.GetVal_32();
								pHero->nArmyLevel	= q.GetVal_32();
								pHero->nArmyNum		= q.GetVal_32();
								pHero->nExp			= q.GetVal_32();
								pHero->nLevel		= q.GetVal_32();
								pHero->nProf		= q.GetVal_32();
								++ pHero;
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
							}break;
						case SYNC_TYPE_CHAR:
							{
								pRpl->nRst		= q.GetVal_32();
								CharDesc *pChar	= (CharDesc*)wh_getptrnexttoptr(pRpl);
								pChar->nLevel	= q.GetVal_32();
								pChar->nExp		= q.GetVal_32();
								pChar->nDiamond	= q.GetVal_32();
								pChar->nCrystal	= q.GetVal_32();
								pChar->nGold	= q.GetVal_32();
								pChar->nVIP		= q.GetVal_32();
								pChar->nPopulation	= q.GetVal_32();
								pChar->nProtectTime	= q.GetVal_32();
								++ pChar;
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pChar-vectRawBuf.getbuf());
							}break;
						case SYNC_TYPE_ITEM:
							{
								pRpl->nRst		= q.GetVal_32();
								UseItemDesc *pDesc = (UseItemDesc*)wh_getptrnexttoptr(pRpl);
								AddItemDesc *pItem = (AddItemDesc*)wh_getptrnexttoptr(pDesc);
								// 返回字符串 "1*1*1*2*2*2,...,itemid*excelid*num"
								pDesc->nNum = dbs_wc(q.GetStr(2), '*');
								pDesc->nNum = pDesc->nNum==2? 1: (pDesc->nNum+1)/3;
								if (pDesc->nNum>0)
								{
									int idx = 0;
									char tmp[64];
									while (dbs_splitbychar(q.GetStr(2), '*', idx, tmp) > 0)
									{
										pItem->nItemID	= whstr2int64(tmp);
										dbs_splitbychar(q.GetStr(2), '*', idx, tmp);
										pItem->nExcelID	= atoi(tmp);
										dbs_splitbychar(q.GetStr(2), '*', idx, tmp);
										pItem->nNum		= atoi(tmp);
										++ pItem;
									}
								}
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pItem-vectRawBuf.getbuf());
							}break;
						case SYNC_TYPE_DRUG:
							{
								pRpl->nRst		= q.GetVal_32();
								DrugDesc *pDrug = (DrugDesc*)wh_getptrnexttoptr(pRpl);
								pDrug->nNum		= q.GetVal_32();
								++ pDrug;
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pDrug-vectRawBuf.getbuf());
							}break;
						default:
							{
								pRpl->nRst	= STC_GAMECMD_USE_ITEM_T::RST_SQL_ERR;
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
							}break;
						}
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"use item,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nExcelID, pReq->nNum, pRpl->nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"use item,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nHeroID, pReq->nExcelID, pReq->nNum);
						pRpl->nRst	= STC_GAMECMD_USE_ITEM_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_EQUIP_REQ:
		{
			P_DBS4WEB_GET_EQUIP_T*	pReq		= (P_DBS4WEB_GET_EQUIP_T*)pCmd;
			P_DBS4WEBUSER_GET_EQUIP_T*	pRpl	= (P_DBS4WEBUSER_GET_EQUIP_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_EQUIP_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_EQUIP_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get equip,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_GET_EQUIP_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_equip(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get equip,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_GET_EQUIP_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					EquipDesc *pEquip = (EquipDesc*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pEquip->nHeroID		= q.GetVal_64();
						pEquip->nEquipType	= q.GetVal_32();
						pEquip->nExcelID	= q.GetVal_32();
						pEquip->nEquipID	= q.GetVal_64();

						++pEquip;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get equip,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pEquip-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_GEM_REQ:
		{
			P_DBS4WEB_GET_GEM_T*	pReq		= (P_DBS4WEB_GET_GEM_T*)pCmd;
			P_DBS4WEBUSER_GET_GEM_T*	pRpl	= (P_DBS4WEBUSER_GET_GEM_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_GEM_RPL;
			pRpl->nRst			= STC_GAMECMD_GET_GEM_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get gem,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_GET_GEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_gem(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get gem,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_GET_GEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					GemDesc *pGem = (GemDesc*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pGem->nEquipID		= q.GetVal_64();
						pGem->nSlotIdx		= q.GetVal_32();
						pGem->nSlotType		= q.GetVal_32();
						pGem->nExcelID		= q.GetVal_32();
						pGem->nGemID		= q.GetVal_64();

						++pGem;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get gem,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pGem-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_REPLACE_EQUIP_REQ:
		{
			P_DBS4WEB_REPLACE_EQUIP_T*	pReq	= (P_DBS4WEB_REPLACE_EQUIP_T*)pCmd;
			P_DBS4WEBUSER_REPLACE_EQUIP_T*	pRpl	= (P_DBS4WEBUSER_REPLACE_EQUIP_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_REPLACE_EQUIP_RPL;
			pRpl->nRst			= STC_GAMECMD_REPLACE_EQUIP_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"replace equip,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nHeroID, pReq->nEquipType, pReq->nItemID);
				pRpl->nRst			= STC_GAMECMD_REPLACE_EQUIP_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_replace_equip(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,1,%s)", 
					pReq->nAccountID, pReq->nHeroID, pReq->nEquipType, pReq->nItemID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"replace equip,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nHeroID, pReq->nEquipType, pReq->nItemID);
					pRpl->nRst	= STC_GAMECMD_REPLACE_EQUIP_T::RST_SQL_ERR;
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
						pHero->nArmyType	= q.GetVal_32();
						pHero->nArmyLevel	= q.GetVal_32();
						pHero->nArmyNum		= q.GetVal_32();
						pHero->nExp			= q.GetVal_32();
						pHero->nLevel		= q.GetVal_32();
						pHero->nProf		= q.GetVal_32();
						++ pHero;
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"replace equip,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nHeroID, pReq->nEquipType, pReq->nItemID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"replace equip,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nHeroID, pReq->nEquipType, pReq->nItemID);
						pRpl->nRst	= STC_GAMECMD_REPLACE_EQUIP_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_USE_DRUG_REQ:
		{
			P_DBS4WEB_USE_DRUG_T*	pReq	= (P_DBS4WEB_USE_DRUG_T*)pCmd;
			P_DBS4WEBUSER_USE_DRUG_T	*pRpl	= (P_DBS4WEBUSER_USE_DRUG_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd			= CMDID_USE_DRUG_RPL;
			pRpl->nRst			= STC_GAMECMD_USE_DRUG_T::RST_OK;
			pRpl->nAccountID	= pReq->nAccountID;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"use drug,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nNum);
				pRpl->nRst			= STC_GAMECMD_USE_DRUG_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				tty_id_t *pID	= (tty_id_t*)wh_getptrnexttoptr(pReq);
				HeroDesc *pHero	= (HeroDesc*)wh_getptrnexttoptr(pRpl);
				for (int i=0; i<pReq->nNum; ++i)
				{
					q.FreeResult();
					q.SpawnQuery("call game_use_item(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,1,%s)", pReq->nAccountID, *pID, 2014, 0, SP_RESULT);
					int	nPreRet		= 0;
					q.ExecuteSPWithResult(nPreRet);
					if (q.FetchRow())
					{
						int nType		= q.GetVal_32();
						switch (nType)
						{
						case SYNC_TYPE_HERO:
							{
								pRpl->nRst			= q.GetVal_32();
								if (pRpl->nRst == 0)
								{
								pHero->nAttackBase	= q.GetVal_32();
								pHero->nAttackAdd	= q.GetVal_32();
								pHero->nDefenseBase	= q.GetVal_32();
								pHero->nDefenseAdd	= q.GetVal_32();
								pHero->nHealthBase	= q.GetVal_32();
								pHero->nHealthAdd	= q.GetVal_32();
								pHero->nLeaderBase	= q.GetVal_32();
								pHero->nLeaderAdd	= q.GetVal_32();
								pHero->fGrow		= (float)q.GetVal_float();
								pHero->nHeroID		= *pID;
								pHero->nHealthState	= q.GetVal_32();
								pHero->nArmyType	= q.GetVal_32();
								pHero->nArmyLevel	= q.GetVal_32();
								pHero->nArmyNum		= q.GetVal_32();
								pHero->nExp			= q.GetVal_32();
								pHero->nLevel		= q.GetVal_32();
								pHero->nProf		= q.GetVal_32();

								++ pRpl->nNum;
								++ pHero;
								}
							}break;
						}
					}
					++ pID;
				}
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
				GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"use drug,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nNum, pRpl->nRst);
				return 0;
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
