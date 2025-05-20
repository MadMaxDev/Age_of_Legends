#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::ThreadDealCmd_Combat(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf)
{
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	switch (pCmd->nSubCmd)
	{
	case CMDID_START_COMBAT_REQ:
		{
			P_DBS4WEB_START_COMBAT_T*	pReq	= (P_DBS4WEB_START_COMBAT_T*)pCmd;
			P_DBS4WEBUSER_START_COMBAT_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_START_COMBAT_RPL;
			Rpl.nRst			= STC_GAMECMD_OPERATE_START_COMBAT_T::RST_OK;
			Rpl.nCombatType		= pReq->nCombatType;
			memcpy(Rpl.nExt, pReq->nExt, sizeof(Rpl.nExt));
			Rpl.nObjID			= pReq->nObjID;
			Rpl.nAutoCombat		= pReq->nAutoCombat;
			Rpl.nAccountID		= pReq->nAccountID;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"start combat,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
					pReq->nAccountID, pReq->nObjID, pReq->nCombatType, pReq->nAutoCombat, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
				Rpl.nRst			= STC_GAMECMD_OPERATE_START_COMBAT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_start_combat(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,1,%s)", 
					pReq->nAccountID, pReq->nObjID, pReq->nCombatType, pReq->nAutoCombat, pReq->nAutoSupply, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, pReq->nStopLevel, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"start combat,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
						pReq->nAccountID, pReq->nObjID, pReq->nCombatType, pReq->nAutoCombat, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
					Rpl.nRst	= STC_GAMECMD_OPERATE_START_COMBAT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nRst		= q.GetVal_32();
						Rpl.nCombatID	= q.GetVal_64();
						Rpl.nGoTime		= q.GetVal_32();
						Rpl.nBackTime	= q.GetVal_32();
						Rpl.nAccountID1=Rpl.nAccountID2=Rpl.nAccountID3=Rpl.nAccountID4=Rpl.nAccountID5=0;
						int idx = 0;
						char tmp[32];
						tty_id_t *pID	= (tty_id_t*)&(Rpl.nAccountID1);
						while (dbs_splitbychar(q.GetStr(4), ',', idx, tmp) > 0)
						{
							*pID	= whstr2int64(tmp);
							++ pID;
						}
						Rpl.nAllianceID	= q.GetVal_64(5);
						if (Rpl.nRst==STC_GAMECMD_OPERATE_START_COMBAT_T::RST_OK && Rpl.nAllianceID>0)
						{
							P_DBS4WEBUSER_SENDALLCLIENT_T *pCmd = (P_DBS4WEBUSER_SENDALLCLIENT_T*)vectRawBuf.getbuf();
							pCmd->nCmd		= P_DBS4WEB_RPL_CMD;
							pCmd->nSubCmd	= CMDID_SENDALLCLIENT;
							pCmd->nType		= STC_GAMECMD_CHAT_ALLIANCE;
							pCmd->nAllianceID	= Rpl.nAllianceID;
							pCmd->nAccountID	= pReq->nAccountID;

							pCmd->nTextLen	= strlen(q.GetStr(6));
							char *p = (char*)wh_getptrnexttoptr(pCmd);
							strcpy(p, q.GetStr(6));
							p += strlen(q.GetStr(6));
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), p-vectRawBuf.getbuf());
						}
						tty_id_t	nAtkAllianceID	= q.GetVal_64(7);
						if (nAtkAllianceID > 0)
						{
							P_DBS4WEBUSER_SENDALLCLIENT_T *pCmd = (P_DBS4WEBUSER_SENDALLCLIENT_T*)vectRawBuf.getbuf();
							pCmd->nCmd		= P_DBS4WEB_RPL_CMD;
							pCmd->nSubCmd	= CMDID_SENDALLCLIENT;
							pCmd->nType		= STC_GAMECMD_CHAT_ALLIANCE;
							pCmd->nAllianceID	= nAtkAllianceID;
							pCmd->nAccountID	= pReq->nObjID;

							pCmd->nTextLen	= strlen(q.GetStr(8));
							char *p = (char*)wh_getptrnexttoptr(pCmd);
							strcpy(p, q.GetStr(8));
							p += strlen(q.GetStr(8));
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), p-vectRawBuf.getbuf());
						}
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"start combat,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", 
							pReq->nAccountID, pReq->nObjID, pReq->nCombatType, pReq->nAutoCombat, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero, Rpl.nRst, Rpl.nCombatID, Rpl.nGoTime);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"start combat,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
							pReq->nAccountID, pReq->nObjID, pReq->nCombatType, pReq->nAutoCombat, pReq->n1Hero, pReq->n2Hero, pReq->n3Hero, pReq->n4Hero, pReq->n5Hero);
						Rpl.nRst	= STC_GAMECMD_OPERATE_START_COMBAT_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_STOP_COMBAT_REQ:
		{
			P_DBS4WEB_STOP_COMBAT_T*	pReq	= (P_DBS4WEB_STOP_COMBAT_T*)pCmd;
			P_DBS4WEBUSER_STOP_COMBAT_T	*pRpl	= (P_DBS4WEBUSER_STOP_COMBAT_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd			= CMDID_STOP_COMBAT_RPL;
			pRpl->nRst			= STC_GAMECMD_OPERATE_STOP_COMBAT_T::RST_OK;
			pRpl->nCombatID		= pReq->nCombatID;
			pRpl->nCombatType		= pReq->nCombatType;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"stop combat,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
				pRpl->nRst			= STC_GAMECMD_OPERATE_STOP_COMBAT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				// 是副本战斗的话，先收集在里面的所有账号ID
				pRpl->nAccountID1	= 0;
				pRpl->nAccountID2	= 0;
				pRpl->nAccountID3	= 0;
				pRpl->nAccountID4	= 0;
				pRpl->nAccountID5	= 0;
				int	nPreRet		= 0;
				
				if (pRpl->nCombatType==100 || pRpl->nCombatType==101)
				{
					q.FreeResult();
					q.SpawnQuery("select T1.account_id,T1.drug from instance_player T2 inner join common_characters T1 on T2.account_id=T1.account_id where T2.instance_id=0x%"WHINT64PRFX"X", pReq->nCombatID);
					q.GetResult(nPreRet);
					tty_id_t *pID	= (tty_id_t*)&(pRpl->nAccountID1);
					int *pDrug		= (int*)&(pRpl->nDrug1);
					while (q.FetchRow())
					{
						*pID	= q.GetVal_64();
						*pDrug	= q.GetVal_32();
						++ pID;
						++ pDrug;
					}
				}

				q.SpawnQuery("call game_stop_combat(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nCombatID, SP_RESULT);
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"stop combat,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
					pRpl->nRst	= STC_GAMECMD_OPERATE_STOP_COMBAT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						pRpl->nCombatRst	= q.GetVal_32();
						pRpl->nAccountID	= q.GetVal_64();
						pRpl->nHero1	= q.GetVal_64();
						pRpl->nHero2	= q.GetVal_64();
						pRpl->nHero3	= q.GetVal_64();
						pRpl->nHero4	= q.GetVal_64();
						pRpl->nHero5	= q.GetVal_64();
						pRpl->nDrug1	= q.GetVal_32();
						pRpl->nDrug2	= q.GetVal_32();
						pRpl->nDrug3	= q.GetVal_32();
						pRpl->nDrug4	= q.GetVal_32();
						pRpl->nDrug5	= q.GetVal_32();
						switch (pRpl->nRst)
						{
						case 2:
							{
								pRpl->nNewCombatID	= q.GetVal_64();
								pRpl->nGoTime		= q.GetVal_32();
								pRpl->nBackTime		= q.GetVal_32();

								InstanceHeroDesc *pHero = (InstanceHeroDesc*)wh_getptrnexttoptr(pRpl);
								// 获取副本武将摘要
								q.FreeResult();
								q.SpawnQuery("call game_get_instancehero(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0,0,0,0,0,1,%s)", pRpl->nAccountID, pReq->nCombatID, pReq->nCombatType, SP_RESULT);
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
								if (pRpl->nCombatType==100 || pRpl->nCombatType==101)
								{
									q.FreeResult();
									q.SpawnQuery("select T1.account_id,T1.drug from instance_player T2 inner join common_characters T1 on T2.account_id=T1.account_id where T2.instance_id=0x%"WHINT64PRFX"X", pReq->nCombatID);
									q.GetResult(nPreRet);
									tty_id_t *pID	= (tty_id_t*)&(pRpl->nAccountID1);
									int *pDrug		= (int*)&(pRpl->nDrug1);
									while (q.FetchRow())
									{
										*pID	= q.GetVal_64();
										*pDrug	= q.GetVal_32();
										++ pID;
										++ pDrug;
									}
								}
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
							}break;
						case 0:
						case 1:
						case 3:
						case 4:
						default:
							{
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
							}break;
						}

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"stop combat,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nCombatID, pRpl->nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"stop combat,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
						pRpl->nRst	= STC_GAMECMD_OPERATE_STOP_COMBAT_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_COMBAT_REQ:
		{
			P_DBS4WEB_GET_COMBAT_T*	pReq		= (P_DBS4WEB_GET_COMBAT_T*)pCmd;
			P_DBS4WEBUSER_GET_COMBAT_T*	pRpl	= (P_DBS4WEBUSER_GET_COMBAT_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_GET_COMBAT_RPL;
			pRpl->nRst			= STC_GAMECMD_OPERATE_GET_COMBAT_T::RST_OK;
			pRpl->nNumA			= 0;
			pRpl->nNumD			= 0;
			vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_COMBAT_T));
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get combat,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
				pRpl->nRst		= STC_GAMECMD_OPERATE_GET_COMBAT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_combat(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%s)", pReq->nAccountID, pReq->nCombatID, pReq->nExt[2], SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get combat,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
					pRpl->nRst	= STC_GAMECMD_OPERATE_GET_COMBAT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nCombatID		= pReq->nCombatID;
						pRpl->nCombatType	= q.GetVal_32();
						pRpl->nAccountID	= q.GetVal_64();
						pRpl->nObjID		= q.GetVal_64();
						strcpy(pRpl->szObjName, q.GetStr());
						pRpl->nObjHeadID	= q.GetVal_32();

						CombatUnit *p = (CombatUnit*)wh_getptrnexttoptr(pRpl);
						char szArmy[5][128], szName[32];
						tty_id_t nHeroID;
						int nSlot, nArmyType, nArmyLevel, nArmyNum, nAttack, nDefense, nHealth, nModel, nLevel, nHealthState;
						// 攻方部队数据
						szArmy[0][0] = szArmy[1][0] = szArmy[2][0] = szArmy[3][0] = szArmy[4][0] = 0;
						wh_strsplit("sssss", q.GetStr(), ",", szArmy[0], szArmy[1], szArmy[2], szArmy[3], szArmy[4]);
						for (int i=0; i<COMBATFIELD_CHANNEL; ++i)
						{
							nSlot = 0;
							wh_strsplit("dIsddddddddd", szArmy[i], "*", &nSlot, &nHeroID, szName, &nArmyType, &nArmyLevel, &nArmyNum, &nAttack, &nDefense, &nHealth, &nModel, &nLevel, &nHealthState);
							if (nSlot > 0)
							{
								++ pRpl->nNumA;
								p->nSlot		= nSlot;
								p->nHeroID		= nHeroID;
								strcpy(p->szName, szName);
								p->nArmyType	= nArmyType;
								p->nArmyLevel	= nArmyLevel;
								p->nArmyNum		= nArmyNum;
								p->nAttack		= nAttack;
								p->nDefense		= nDefense;
								p->nHealth		= nHealth;
								p->nModel		= nModel;
								p->nLevel		= nLevel;
								p->nHealthState	= nHealthState;
								++ p;
							}
						}
						// 守方部队数据
						szArmy[0][0] = szArmy[1][0] = szArmy[2][0] = szArmy[3][0] = szArmy[4][0] = 0;
						wh_strsplit("sssss", q.GetStr(), ",", szArmy[0], szArmy[1], szArmy[2], szArmy[3], szArmy[4]);
						for (int i=0; i<COMBATFIELD_CHANNEL; ++i)
						{
							nSlot = 0;
							wh_strsplit("dIsddddddddd", szArmy[i], "*", &nSlot, &nHeroID, szName, &nArmyType, &nArmyLevel, &nArmyNum, &nAttack, &nDefense, &nHealth, &nModel, &nLevel, &nHealthState);
							if (nSlot > 0)
							{
								++ pRpl->nNumD;
								p->nSlot		= nSlot;
								p->nHeroID		= nHeroID;
								strcpy(p->szName, szName);
								p->nArmyType	= nArmyType;
								p->nArmyLevel	= nArmyLevel;
								p->nArmyNum		= nArmyNum;
								p->nAttack		= nAttack;
								p->nDefense		= nDefense;
								p->nHealth		= nHealth;
								p->nModel		= nModel;
								p->nLevel		= nLevel;
								p->nHealthState	= nHealthState;
								++ p;
							}
						}

						// 是副本战斗的话，还得收集在里面的所有账号ID
						pRpl->nAccountID1	= 0;
						pRpl->nAccountID2	= 0;
						pRpl->nAccountID3	= 0;
						pRpl->nAccountID4	= 0;
						pRpl->nAccountID5	= 0;
						if (pRpl->nCombatType >= 100)
						{
							pRpl->nAccountID	= 0;

							q.ClearSPResult();
							q.SpawnQuery("select account_id from instance_player where instance_id=0x%"WHINT64PRFX"X", pReq->nCombatID);
							q.GetResult(nPreRet);
							tty_id_t *pID	= (tty_id_t*)&(pRpl->nAccountID1);
							while (q.FetchRow())
							{
								*pID	= q.GetVal_64();
								++ pID;
							}
						}

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get combat,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nCombatID, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)p-vectRawBuf.getbuf());

						// PVP时，VIP玩家会自动补给城防
						if (pReq->nExt[2] == 1 &&
							pRpl->nCombatType == 2)
						{
							q.ClearSPResult();
							q.SpawnQuery("select @vVIP,@vHero1,@vHero2,@vHero3,@vHero4,@vHero5,@vDrugStr");
							q.GetResult(nPreRet);
							q.FetchRow();
							int nVIP = q.GetVal_32();
							tty_id_t nHero1 = q.GetVal_64();
							tty_id_t nHero2 = q.GetVal_64();
							tty_id_t nHero3 = q.GetVal_64();
							tty_id_t nHero4 = q.GetVal_64();
							tty_id_t nHero5 = q.GetVal_64();
							if (nVIP > 0)
							{
								P_DBS4WEBUSER_MANUAL_SUPPLY_T*	pCmd	= (P_DBS4WEBUSER_MANUAL_SUPPLY_T*)vectRawBuf.getbuf();
								pCmd->nCmd			= P_DBS4WEB_RPL_CMD;
								pCmd->nSubCmd			= CMDID_MANUAL_SUPPLY_RPL;
								pCmd->nRst			= STC_GAMECMD_MANUAL_SUPPLY_T::RST_OK;
								pCmd->nNum			= 0;
								pCmd->nExcelID		= pRpl->nCombatType;
								pCmd->nInstanceID	= 0;
								memset(pCmd->nExt, 0, sizeof(pCmd->nExt));
								pCmd->nAccountID	= pRpl->nObjID;

								tty_id_t *pID	= (tty_id_t*)&pCmd->nAccountID1;
								int *pDrug		= (int*)&pCmd->nDrug1;
								// 返回字符串 ",1,0,2,0,...,n,0"
								int idx = 0;
								char tmp[32];
								while (dbs_splitbychar(q.GetStr(6)+1, ',', idx, tmp) > 0)
								{
									*pID	= whstr2int64(tmp);
									dbs_splitbychar(q.GetStr(6)+1, ',', idx, tmp);
									*pDrug	= atoi(tmp);

									++ pID;
									++ pDrug;
								}

								InstanceHeroDesc *pHero = (InstanceHeroDesc*)wh_getptrnexttoptr(pCmd);
								// 获取副本武将摘要
								q.FreeResult();
								q.SpawnQuery("call game_get_instancehero(0,0,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0,%s)", 
									pRpl->nCombatType, nHero1, nHero2, nHero3, nHero4, nHero5, SP_RESULT);
								q.ExecuteSPWithResult(nPreRet);
								pCmd->nNum		= q.NumRows();
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

								GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"citydefense autosupply,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", 
									pRpl->nObjID, 0, pRpl->nCombatType, nHero1, nHero2, nHero3, nHero4, nHero5);
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pHero-vectRawBuf.getbuf());
							}
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get combat,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
						pRpl->nRst	= STC_GAMECMD_OPERATE_GET_COMBAT_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_LIST_COMBAT_REQ:
		{
			P_DBS4WEB_LIST_COMBAT_T*	pReq		= (P_DBS4WEB_LIST_COMBAT_T*)pCmd;
			P_DBS4WEBUSER_LIST_COMBAT_T*	pRpl	= (P_DBS4WEBUSER_LIST_COMBAT_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_LIST_COMBAT_RPL;
			pRpl->nRst			= STC_GAMECMD_OPERATE_LIST_COMBAT_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"list combat,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRpl->nRst		= STC_GAMECMD_OPERATE_LIST_COMBAT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_list_combat(0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"list combat,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRpl->nRst	= STC_GAMECMD_OPERATE_LIST_COMBAT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					CombatDesc *pCombat = (CombatDesc*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						pCombat->nCombatID		= q.GetVal_64();
						pCombat->nCombatType	= q.GetVal_32();
						pCombat->nObjID			= q.GetVal_64();
						strcpy(pCombat->szName, q.GetStr());
						pCombat->nResTime		= q.GetVal_32();
						pCombat->nStatus		= q.GetVal_32();

						++pCombat;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"list combat,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pCombat-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_DEAL_COMBAT_TE_REQ:
		{
			P_DBS4WEB_DEAL_COMBAT_TE_T*	pReq	= (P_DBS4WEB_DEAL_COMBAT_TE_T*)pCmd;
			P_DBS4WEBUSER_DEAL_COMBAT_TE_T	Rpl;
			Rpl.nCmd			= P_DBS4WEB_RPL_CMD;
			Rpl.nSubCmd			= CMDID_DEAL_COMBAT_TE_RPL;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal combat te,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nEventID);
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_te_combat(0x%"WHINT64PRFX"X,%s)", pReq->nEventID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal combat te,SQLERR,0x%"WHINT64PRFX"X,%d", pReq->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						Rpl.nCombatID		= q.GetVal_64();
						Rpl.nCombatType		= q.GetVal_32();
						Rpl.nStatus			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal combat te,0x%"WHINT64PRFX"X", pReq->nEventID);
						SendMsg_MT(pCDCmd->nConnecterID, &Rpl, sizeof(Rpl));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal combat te,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nEventID);
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_DEAL_COMBAT_BACK_REQ:
		{
			P_DBS4WEB_DEAL_COMBAT_BACK_T*	pReq	= (P_DBS4WEB_DEAL_COMBAT_BACK_T*)pCmd;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal combat back,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nCombatID, pReq->nCombatRst);
				return 0;
			}
			else
			{
				int	nPreRet		= 0;

				// 更新武将数据
				P_DBS4WEBUSER_COMBAT_RST_T *pRst	= (P_DBS4WEBUSER_COMBAT_RST_T*)vectRawBuf.getbuf();
				pRst->nCmd			= P_DBS4WEB_RPL_CMD;
				pRst->nCombatRst	= pReq->nCombatRst;
				pRst->nSubCmd		= CMDID_COMBAT_RST;
				pRst->nCombatID		= pReq->nCombatID;
				pRst->nCombatType	= pReq->nCombatType;
				pRst->nAccountID	= pReq->nAccountID;
				pRst->nObjID		= pReq->nObjID;
				pRst->nClassID		= 0;

				int nClassID		= 0;

				int nGold=0;
				int nMan=0;
				int nCup=0;

				char buf1[256];buf1[0]=0;

				// 对方名字和头像
				q.SpawnQuery("select name,head_id from common_characters where account_id=%"WHINT64PRFX"d", pReq->nObjID);
				q.GetResult(nPreRet);
				if (q.FetchRow())
				{
					strcpy(pRst->szObjName, q.GetStr());
					pRst->nObjHeadID	= q.GetVal_32();
				}
				q.FreeResult();

				// 副本的class
				if (pReq->nCombatType==100 || pReq->nCombatType==101)
				{
				q.SpawnQuery("select class_id from instance where instance_id=%"WHINT64PRFX"d", pReq->nCombatID);
				q.GetResult(nPreRet);
				if (q.FetchRow())
				{
					pRst->nClassID	= q.GetVal_32();
					nClassID	= pRst->nClassID;
				}
				q.FreeResult();
				}

				pRst->nAccountID1	= pReq->nAccountID1;
				pRst->nAccountID2	= pReq->nAccountID2;
				pRst->nAccountID3	= pReq->nAccountID3;
				pRst->nAccountID4	= pReq->nAccountID4;
				pRst->nAccountID5	= pReq->nAccountID5;
				pRst->nNumA			= pReq->nNumA;
				pRst->nNumD			= pReq->nNumD;
				CombatRst *pR		= (CombatRst*)wh_getptrnexttoptr(pRst);
				CombatUnit4Log *pHero	= (CombatUnit4Log*)wh_getptrnexttoptr(pReq);

				// 计算双方的总战力
				int nProfA	= 0;
				for (int i=0; i<pReq->nNumA; ++i)
				{
					int n = (int)((pHero->nArmyAttack+pHero->nArmyDefense*0.9+pHero->nArmyHealth*0.8)/3);
					if (n==0 && pHero->nArmyNumBefore>0) n=1;
					nProfA += n;
					++ pHero;
				}
				int nProfD	= 0;
				for (int i=0; i<pReq->nNumD; ++i)
				{
					int n = (int)((pHero->nArmyAttack+pHero->nArmyDefense*0.9+pHero->nArmyHealth*0.8)/3);
					if (n==0 && pHero->nArmyNumBefore>0) n=1;
					nProfD += n;
					++ pHero;
				}
				
				pR		= (CombatRst*)wh_getptrnexttoptr(pRst);
				pHero	= (CombatUnit4Log*)wh_getptrnexttoptr(pReq);
				int nNum	= pReq->nNumA;
				for (int i=0; i<pReq->nNumA; ++i)
				{
					int nP = (int)((pHero->nArmyAttack+pHero->nArmyDefense*0.9+pHero->nArmyHealth*0.8)/3);
					q.FreeResult();
					q.SpawnQuery("call game_update_hero(%d,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,1,%s)", pReq->nCombatType, pHero->nHeroID, pHero->nArmyNumAfter-pHero->nArmyNumBefore, pReq->nObjID, pHero->nSlot, nP, nProfA, nProfD, pReq->nNumA, SP_RESULT);
					q.ExecuteSPWithResult(nPreRet);
					if (q.FetchRow())
					{
						q.SkipRow();
						pR->nHeroID		= pHero->nHeroID;
						pR->nArmyNum	= q.GetVal_32();
						pR->nArmyAttack	= q.GetVal_32();
						pR->nArmyDefense= q.GetVal_32();
						pR->nArmyHealth	= q.GetVal_32();
						pR->nHealthState= q.GetVal_32();
						pR->nArmyProf	= q.GetVal_32();
						pR->nArmyNumRecover	= q.GetVal_32();
						pR->nHeroLevel	= q.GetVal_32();
						pR->nHeroExp	= q.GetVal_32();
						pHero->nArmyNumRecover	= pR->nArmyNumRecover;
					}
					++ pR;
					++ pHero;
				}
				if (pReq->nCombatType == COMBAT_PVP_ATTACK ||
					pReq->nCombatType == COMBAT_PVP_WORLDGOLDMINE_ATTACK ||
					pReq->nCombatType == COMBAT_WORLDCITY ||
					pReq->nCombatType == COMBAT_PVE_WORLD_RES)
				{
					nNum += pReq->nNumD;
				for (int i=0; i<pReq->nNumD; ++i)
				{
					int nP = (int)((pHero->nArmyAttack+pHero->nArmyDefense*0.9+pHero->nArmyHealth*0.8)/3);
					q.FreeResult();
					q.SpawnQuery("call game_update_hero(%d,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,1,%s)", pReq->nCombatType, pHero->nHeroID, pHero->nArmyNumAfter-pHero->nArmyNumBefore, pReq->nObjID, pHero->nSlot, nP, nProfD, nProfA, pReq->nNumD, SP_RESULT);
					q.ExecuteSPWithResult(nPreRet);
					if (q.FetchRow())
					{
						q.SkipRow();
						pR->nHeroID		= pHero->nHeroID;
						pR->nArmyNum	= q.GetVal_32();
						pR->nArmyAttack	= q.GetVal_32();
						pR->nArmyDefense= q.GetVal_32();
						pR->nArmyHealth	= q.GetVal_32();
						pR->nHealthState= q.GetVal_32();
						pR->nArmyProf	= q.GetVal_32();
						pR->nArmyNumRecover	= q.GetVal_32();
						pR->nHeroLevel	= q.GetVal_32();
						pR->nHeroExp	= q.GetVal_32();
						pHero->nArmyNumRecover	= pR->nArmyNumRecover;
					}
					++ pR;
					++ pHero;
				}
				}
				// 通知client战斗结果
			//	SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pR-vectRawBuf.getbuf());
				int size = (char*)pR-vectRawBuf.getbuf();
				char *buf = new char[size];
				memcpy(buf, vectRawBuf.getbuf(), size);

				q.FreeResult();
				q.SpawnQuery("call game_back_combat(0x%"WHINT64PRFX"X,%d,%s)", pReq->nCombatID, pReq->nCombatRst, SP_RESULT);
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal combat back,SQLERR,0x%"WHINT64PRFX"X,%d", pReq->nCombatID, pReq->nCombatRst);
				}
				else
				{
					if (q.FetchRow())
					{
						int nRst	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal combat back,0x%"WHINT64PRFX"X,%d,%d", pReq->nCombatID, pReq->nCombatRst, nRst);

						int nGold,nMan;

						// 记录日志，邮件形式
						CombatLog *pLog	= (CombatLog*)vectRawBuf.getbuf();
						pLog->nCombatType	= q.GetVal_32();
						pLog->nCombatRst	= pReq->nCombatRst;
						strcpy(pLog->szAttackName, q.GetStr());
						strcpy(pLog->szDefenseName, q.GetStr());
						pLog->nNumA			= pReq->nNumA;
						pLog->nNumD			= pReq->nNumD;
						pLog->nObjID		= q.GetVal_64();
						pLog->nPosX			= q.GetVal_32();
						pLog->nPosY			= q.GetVal_32();
						void *pUnit			= wh_getptrnexttoptr(pLog);
						memcpy(pUnit, wh_getptrnexttoptr(pReq), sizeof(CombatUnit4Log)*(pLog->nNumA+pLog->nNumD));
						void *pAddon		= (char*)pUnit + sizeof(CombatUnit4Log)*(pLog->nNumA+pLog->nNumD);
						void *pEnd			= pAddon;
						switch (pLog->nCombatType)
						{
						case COMBAT_PVE_RAID:
							{
								// 军事任务
								CombatLog_PVE_Raid *pPVE	= (CombatLog_PVE_Raid*)pAddon;
								pPVE->nExcelID		= 0;
								pPVE->nNum			= 0;
								if (pReq->nCombatRst == 1)
								{
								pPVE->nExcelID		= q.GetVal_32();
								pPVE->nNum			= q.GetVal_32();
								}

								pEnd	= wh_getptrnexttoptr(pPVE);
							}break;
						case COMBAT_PVP_ATTACK:
							{
								pLog->nAttackHeadID	= q.GetVal_32();
								pLog->nDefenseHeadID= q.GetVal_32();
								// 玩家对战
								CombatLog_PVP *pPVP	= (CombatLog_PVP*)pAddon;
								pPVP->nGold		= 0;
								pPVP->nPopulation=0;
								pPVP->nCup		= 0;
								if (pReq->nCombatRst==1)
								{
								pPVP->nGold		= q.GetVal_32();
								nGold	= pPVP->nGold;
								pPVP->nPopulation	= q.GetVal_32();
								nMan	= pPVP->nPopulation;
								pPVP->nAddPopulation	= q.GetVal_32();
								pPVP->nCup			= q.GetVal_32();
								}

								nGold	= pPVP->nGold;
								nMan	= pPVP->nPopulation;
								nCup	= pPVP->nCup;

								pEnd	= wh_getptrnexttoptr(pPVP);
							}break;
						case COMBAT_PVE_WORLDGOLDMINE:
						case COMBAT_PVP_WORLDGOLDMINE_ATTACK:
							{
								// 世界金矿
								CombatLog_WorldGoldMine *pWGM	= (CombatLog_WorldGoldMine*)pAddon;
								pWGM->nArea		= q.GetVal_32();
								pWGM->nClass	= q.GetVal_32();
								pWGM->nIdx		= q.GetVal_32();

								pEnd	= wh_getptrnexttoptr(pWGM);
							}break;
						case COMBAT_INSTANCE_BAIZHANBUDAI:
						case COMBAT_INSTANCE_NANZHENGBEIZHAN:
							{
								// 副本掉落
								CombatLog_InstanceLoot *pIL	= (CombatLog_InstanceLoot*)pAddon;
								LootDesc *pLoot = (LootDesc*)wh_getptrnexttoptr(pIL);
								pIL->nNum	= 0;
								if (pReq->nCombatRst==1)
								{
								// 返回字符串 "1*1*1*2*2*2,...,itemid*excelid*num"
								pIL->nNum = dbs_wc(q.GetStr(7), '*');
								pIL->nNum = pIL->nNum==2? 1: (pIL->nNum+1)/3;
								if (pIL->nNum>0)
								{
									int idx = 0;
									char tmp[64];
									while (dbs_splitbychar(q.GetStr(7), '*', idx, tmp) > 0)
									{
										dbs_splitbychar(q.GetStr(7), '*', idx, tmp);
										pLoot->nExcelID	= atoi(tmp);
										dbs_splitbychar(q.GetStr(7), '*', idx, tmp);
										pLoot->nNum		= atoi(tmp);
										++ pLoot;
									}
								}
								pEnd	= wh_getptrnexttoptr(pLoot);

								// 获得道具的全服通告
								strcpy(buf1, q.GetStr(8));
								}
							}break;
						case COMBAT_WORLDCITY:
							{
								// 世界名城
								pRst	= (P_DBS4WEBUSER_COMBAT_RST_T*)buf;
								CombatLog_WorldCity *pWC	= (CombatLog_WorldCity*)pAddon;
								pWC->nID	= pLog->nObjID;
								pRst->nWorldCityID = pWC->nID;
								if (pReq->nCombatRst==1)
								{
								pWC->nCup	= q.GetVal_32();
								pWC->nRes	= q.GetVal_32();
								pRst->nWorldCityRes = pWC->nRes;

								pEnd	= wh_getptrnexttoptr(pWC);

								// 获取奖杯的世界通告
								strcpy(buf1, q.GetStr(9));
								}
							}break;
						case COMBAT_PVE_WORLD_RES:
							{
								pRst	= (P_DBS4WEBUSER_COMBAT_RST_T*)buf;
								pRst->nAccountID			= q.GetVal_64();
							}break;
						}
						q.FreeResult();
						q.SpawnQuery("call game_combat_log(0x%"WHINT64PRFX"X,'%s','%s',%d,", pReq->nCombatID, pLog->szAttackName, pLog->szDefenseName, pLog->nCombatRst);
						q.StrMove("'");
						q.BinaryToString((char*)pEnd-(char*)pLog, (const char*)pLog);
						q.StrMove("',%s)", SP_RESULT);
						q.ExecuteSPWithResult(nPreRet);
						if (nPreRet != MYSQL_QUERY_NORMAL)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"combat log,SQLERR,0x%"WHINT64PRFX"X", pReq->nCombatID);
						}
						else
						{
							if (q.FetchRow())
							{
								int nRst = q.GetVal_32();

								P_DBS4WEBUSER_NEWMAIL_NOTIFY_T Cmd;
								Cmd.nCmd		= P_DBS4WEB_RPL_CMD;
								Cmd.nSubCmd		= CMDID_NEWMAIL_NOTIFY;

								int nNum = dbs_wc(q.GetStr(1), ',');
								nNum = nNum==1? 1: (nNum+1)/2;
								if (nNum>0)
								{
									int idx = 0;
									char tmp[32];
									while (dbs_splitbychar(q.GetStr(1)+1, ',', idx, tmp) > 0)
									{
										Cmd.nAccountID	= whstr2int64(tmp);
										dbs_splitbychar(q.GetStr(1)+1, ',', idx, tmp);
										Cmd.nMailID		= atoi(tmp);
										SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));
									}
								}
								GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"combat log,0x%"WHINT64PRFX"X,%d", pReq->nCombatID, nRst);
							}
							else
							{
								GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"combat log,SQLERR,0x%"WHINT64PRFX"X", pReq->nCombatID);
							}
						}

						
						// 玩家对战，通知双方金币/人口的掠夺
						// 记录金币日志
						if (pLog->nCombatType==2 && pReq->nCombatRst==1)
						{
						P_DBS4WEBUSER_PVP_RST_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_RPL_CMD;
						Cmd.nSubCmd		= CMDID_PVP_RST;
						Cmd.nCombatID	= pReq->nCombatID;
						Cmd.nCombatRst	= pReq->nCombatRst;
						Cmd.nGold		= nGold;
						Cmd.nMan		= nMan;
						Cmd.nCup		= nCup;
						Cmd.nAccountID	= pReq->nAccountID;
						Cmd.nObjID		= pReq->nObjID;
						SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));
						}
						

						pHero	= (CombatUnit4Log*)wh_getptrnexttoptr(pReq);
						nNum	= pReq->nNumA;
						if (pReq->nCombatType == COMBAT_PVP_ATTACK ||
							pReq->nCombatType == COMBAT_PVP_WORLDGOLDMINE_ATTACK)
						{
							nNum += pReq->nNumD;
						}
						for (int i=0; i<nNum; ++i)
						{
							++ pHero;
						}

						// 有掉落的，还要通知client掉落内容
						if (pLog->nCombatType==1 ||
							pLog->nCombatType==100 ||
							pLog->nCombatType==101 ||
							pLog->nCombatType==20)
						{
						q.FreeResult();
						q.SpawnQuery("call game_get_instanceloot_additem(%d,0x%"WHINT64PRFX"X,%s)", pReq->nCombatType, pReq->nCombatID, SP_RESULT);
						q.ExecuteSPWithResult(nPreRet);
						if (nPreRet != MYSQL_QUERY_NORMAL)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get instanceloot,SQLERR,0x%"WHINT64PRFX"X", pReq->nCombatID);
							return 0;
						}
						P_DBS4WEBUSER_GET_INSTANCELOOT_ADDITEM_RPL_T *pCmd	= (P_DBS4WEBUSER_GET_INSTANCELOOT_ADDITEM_RPL_T*)vectRawBuf.getbuf();
						pCmd->nCmd			= P_DBS4WEB_RPL_CMD;
						pCmd->nSubCmd		= CMDID_GET_INSTANCELOOT_ADDITEM_RPL;
						pCmd->nInstanceID	= pReq->nCombatID;
						pCmd->nCombatType	= pReq->nCombatType;
						pCmd->nObjID		= pReq->nObjID;
						pCmd->nNum			= 0;
						pCmd->nClassID		= nClassID;
						InstanceLootDesc *pLoot = (InstanceLootDesc*)wh_getptrnexttoptr(pCmd);
						while (q.FetchRow())
						{
							pCmd->nAccountID	= q.GetVal_64();
							// 返回字符串 "1*1*1*2*2*2,...,itemid*excelid*num"
							pCmd->nNum = dbs_wc(q.GetStr(1), '*');
							pCmd->nNum = pCmd->nNum==2? 1: (pCmd->nNum+1)/3;
							if (pCmd->nNum>0)
							{
								int idx = 0;
								char tmp[64];
								while (dbs_splitbychar(q.GetStr(1), '*', idx, tmp) > 0)
								{
									pLoot->nItemID	= whstr2int64(tmp);
									dbs_splitbychar(q.GetStr(1), '*', idx, tmp);
									pLoot->nExcelID	= atoi(tmp);
									dbs_splitbychar(q.GetStr(1), '*', idx, tmp);
									pLoot->nNum		= atoi(tmp);
									++ pLoot;
								}
								if (pCmd->nCombatType==20)
								{
									pCmd->nCup	= q.GetVal_32(2);
								}
								SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pLoot-vectRawBuf.getbuf());
							}
						}
						}

						// 获得道具的通告
						// 获得奖杯的通告
						if (buf1[0])
						{
							P_DBS4WEBUSER_SENDALLCLIENT_T *pCmd = (P_DBS4WEBUSER_SENDALLCLIENT_T*)vectRawBuf.getbuf();
							pCmd->nCmd		= P_DBS4WEB_RPL_CMD;
							pCmd->nSubCmd	= CMDID_SENDALLCLIENT;
							pCmd->nType		= STC_GAMECMD_WORLD_NOTIFY;
							if (pReq->nCombatType==100)
								pCmd->nAccountID	= pReq->nAccountID1;
							else if (pReq->nCombatType==20)
								pCmd->nAccountID	= pReq->nAccountID;
							else
								pCmd->nAccountID	= 0;

							pCmd->nTextLen	= strlen(buf1);
							char *p = (char*)wh_getptrnexttoptr(pCmd);
							strcpy(p, buf1);
							p += strlen(buf1);
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), p-vectRawBuf.getbuf());
						}

						// 世界名城攻打队列
						if (pReq->nCombatType==20 && pReq->nCombatRst==1)
						{
							q.FreeResult();
							q.SpawnQuery("call game_rob_end(0x%"WHINT64PRFX"X,%s)", pReq->nObjID, SP_RESULT);
							q.ExecuteSPWithResult(nPreRet);
							char *BackBuf=NULL;
							if (nPreRet==MYSQL_QUERY_NORMAL && q.FetchRow())
							{
							int *pLengths = q.FetchLengths();
							BackBuf = new char[pLengths[0]];
							memcpy(BackBuf, q.GetStr(0), pLengths[0]);
							P_DBS4WEBUSER_ARMY_BACK_T Back;
							Back.nCmd		= P_DBS4WEB_RPL_CMD;
							Back.nSubCmd	= CMDID_ARMY_BACK_RPL;
							Back.nRst		= STC_GAMECMD_ARMY_BACK_T::RST_SQL_ERR;
							Back.nCombatID	= pReq->nCombatID;
							Back.nAccountID	= pReq->nAccountID;

							// 返回字符串 ",1,1,2,...,acct,combat"
							int idx = 0;
							char tmp[64];
							while (dbs_splitbychar(BackBuf+1, ',', idx, tmp) > 0)
							{
								Back.nAccountID	= whstr2int64(tmp);
								dbs_splitbychar(BackBuf+1, ',', idx, tmp);
								Back.nCombatID	= whstr2int64(tmp);
								if (Back.nAccountID != pReq->nAccountID)
								{
								q.FreeResult();
								q.SpawnQuery("call game_army_back(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", Back.nAccountID, Back.nCombatID, SP_RESULT);
								q.ExecuteSPWithResult(nPreRet);
								if (q.FetchRow())
								{
									Back.nRst	= q.GetVal_32();
									if (Back.nRst==0)
									{
									Back.nBackTime	= q.GetVal_32();
									GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"army back,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", Back.nAccountID, Back.nCombatID, Back.nRst);
									SendMsg_MT(pCDCmd->nConnecterID, &Back, sizeof(Back));
									}
								}
								}
							}
							if (BackBuf)
							{
								delete[] BackBuf;
							}
							}
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal combat back,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nCombatID, pReq->nCombatRst);
					}
				}

				pRst	= (P_DBS4WEBUSER_COMBAT_RST_T*)buf;
				pR		= (CombatRst*)wh_getptrnexttoptr(pRst);
				pHero	= (CombatUnit4Log*)wh_getptrnexttoptr(pReq);
				nNum	= pReq->nNumA;
				if (pReq->nCombatType == COMBAT_PVP_ATTACK ||
					pReq->nCombatType == COMBAT_PVP_WORLDGOLDMINE_ATTACK)
				{
					nNum += pReq->nNumD;
				}
				for (int i=0; i<nNum; ++i)
				{
					q.FreeResult();
					q.SpawnQuery("select level,exp from hire_heros where hero_id=%"WHINT64PRFX"d", pHero->nHeroID);
					q.GetResult(nPreRet);
					if (q.FetchRow())
					{
						pR->nHeroLevel	= q.GetVal_32();
						pR->nHeroExp	= q.GetVal_32();
					}
					++ pR;
					++ pHero;
				}
				// 通知client战斗结果
				SendMsg_MT(pCDCmd->nConnecterID, pRst, (char*)pR-(char*)pRst);
				delete[] buf;
				return 0;
			}
		}
		break;
	case CMDID_COMBAT_PROF_REQ:
		{
			P_DBS4WEB_COMBAT_PROF_T*	pReq	= (P_DBS4WEB_COMBAT_PROF_T*)pCmd;
			P_DBS4WEBUSER_COMBAT_PROF_T*	pRpl	= (P_DBS4WEBUSER_COMBAT_PROF_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_COMBAT_PROF_RPL;
			pRpl->nRst			= STC_GAMECMD_COMBAT_PROF_T::RST_OK;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"combat prof,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", 
					pReq->nAccountID, pReq->nClassID, pReq->nObjID, pReq->nCombatType);
				pRpl->nRst			= STC_GAMECMD_COMBAT_PROF_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_combat_prof(%d,0x%"WHINT64PRFX"X,%d,%s)", pReq->nClassID, pReq->nObjID, pReq->nCombatType, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"combat prof,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nClassID, pReq->nObjID, pReq->nCombatType);
					pRpl->nRst	= STC_GAMECMD_COMBAT_PROF_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst		= q.GetVal_32();
						CombatProf *pProf	= (CombatProf*)wh_getptrnexttoptr(pRpl);
						pProf->nProf	= q.GetVal_32();
						pProf->n1ArmyType	= q.GetVal_32();
						pProf->n2ArmyType	= q.GetVal_32();
						pProf->n3ArmyType	= q.GetVal_32();
						pProf->n4ArmyType	= q.GetVal_32();
						pProf->n5ArmyType	= q.GetVal_32();
						++ pProf;
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"combat prof,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nClassID, pReq->nObjID, pReq->nCombatType, pRpl->nRst);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pProf-vectRawBuf.getbuf());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"combat prof,DBERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nClassID, pReq->nObjID, pReq->nCombatType);
						pRpl->nRst	= STC_GAMECMD_COMBAT_PROF_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_LOAD_ARMY_REQ:
		{
			P_DBS4WEB_LOAD_ARMY_T*	pReq	= (P_DBS4WEB_LOAD_ARMY_T*)pCmd;
			P_DBS4WEBUSER_LOAD_ARMY_T*	pRpl	= (P_DBS4WEBUSER_LOAD_ARMY_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_LOAD_ARMY_RPL;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load army,DBERR");
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_load_army(%s)", SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load army,DBERR");
					return 0;
				}
				else
				{
					ArmyDesc *pArmy	= (ArmyDesc*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						wh_strsplit("dd", q.GetStr(), "*", &pArmy->aCrush[0], &pArmy->aCrush[1]);
						pArmy->fBonus	= (float)q.GetVal_float();
						++ pArmy;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"load army,OK");
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)pArmy-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_ARMY_ACCELERATE_REQ:
		{
			P_DBS4WEB_ARMY_ACCELERATE_T*	pReq	= (P_DBS4WEB_ARMY_ACCELERATE_T*)pCmd;
			P_DBS4WEBUSER_ARMY_ACCELERATE_T*	pRpl	= (P_DBS4WEBUSER_ARMY_ACCELERATE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_ARMY_ACCELERATE_RPL;
			pRpl->nCombatID		= pReq->nCombatID;
			pRpl->nRst			= STC_GAMECMD_ARMY_ACCELERATE_T::RST_SQL_ERR;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			pRpl->nAccountID	= pReq->nAccountID;
			pRpl->byType		= pReq->byType;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"army accelerate,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_army_accelerate(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%s)", pReq->nAccountID, pReq->nCombatID, pReq->byType, pReq->nTime, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"army accelerate,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst	= q.GetVal_32();
						pRpl->nCostCrystal	= q.GetVal_32();
						pRpl->nFinTime	= q.GetVal_32();
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"army accelerate,OK,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nCombatID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_ARMY_BACK_REQ:
		{
			P_DBS4WEB_ARMY_BACK_T*	pReq	= (P_DBS4WEB_ARMY_BACK_T*)pCmd;
			P_DBS4WEBUSER_ARMY_BACK_T*	pRpl	= (P_DBS4WEBUSER_ARMY_BACK_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_ARMY_BACK_RPL;
			pRpl->nCombatID		= pReq->nCombatID;
			pRpl->nRst			= STC_GAMECMD_ARMY_BACK_T::RST_SQL_ERR;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			pRpl->nAccountID	= pReq->nAccountID;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"army back,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_army_back(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s)", pReq->nAccountID, pReq->nCombatID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"army back,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nCombatID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pRpl->nRst	= q.GetVal_32();
						pRpl->nBackTime	= q.GetVal_32();
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"army back,OK,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nCombatID, pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
			}
		}
		break;
	case CMDID_DEAL_WORLDCITYRECOVER_TE_REQ:
		{
			P_DBS4WEB_DEAL_WORLDCITYRECOVER_TE_T*	pReq	= (P_DBS4WEB_DEAL_WORLDCITYRECOVER_TE_T*)pCmd;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal worldcityrecover te,DBERR,0x%"WHINT64PRFX"X,%d", pReq->nEventID);
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_deal_te_worldcityrecover(0x%"WHINT64PRFX"X,%s)", pReq->nEventID, SP_RESULT);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal worldcityrecover te,SQLERR,0x%"WHINT64PRFX"X,%d", pReq->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						int nRst			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal worldcityrecover te,0x%"WHINT64PRFX"X", pReq->nEventID);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal worldcityrecover te,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nEventID);
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_WORLDCITY_GET_REQ:
		{
			P_DBS4WEB_WORLDCITY_GET_T*	pReq		= (P_DBS4WEB_WORLDCITY_GET_T*)pCmd;
			P_DBS4WEBUSER_WORLDCITY_GET_T*	pRpl	= (P_DBS4WEBUSER_WORLDCITY_GET_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_WORLDCITY_GET_RPL;
			pRpl->nRst			= STC_GAMECMD_WORLDCITY_GET_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity,DBERR");
				pRpl->nRst		= STC_GAMECMD_WORLDCITY_GET_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_get_worldcity(%s)", SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity,SQLERR");
					pRpl->nRst	= STC_GAMECMD_WORLDCITY_GET_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					WorldCity *p = (WorldCity*)wh_getptrnexttoptr(pRpl);
					while (q.FetchRow())
					{
						p->nID		= q.GetVal_64();
						p->nClass	= q.GetVal_32();
						p->nPosX	= q.GetVal_32();
						p->nPosY	= q.GetVal_32();
						p->nRecover	= q.GetVal_32();
						p->nCup		= q.GetVal_32();
						strcpy(p->szName, q.GetStr());
						p->nInitProf	= q.GetVal_32();
						p->nInitRecover	= q.GetVal_32();
						p->nCurProf		= q.GetVal_32();

						++p;
					}

					q.FreeResult();
					q.SpawnQuery("call game_get_worldcity_endtime(%s)", SP_RESULT);
					q.GetResult(nPreRet);
					if (q.FetchRow())
					{
						pRpl->nRefreshTime	= q.GetVal_32();
						pRpl->nBonusTime	= q.GetVal_32();
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"worldcity,%d", pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)p-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_WORLDCITY_GETLOG_REQ:
		{
			P_DBS4WEB_WORLDCITY_GETLOG_T*	pReq		= (P_DBS4WEB_WORLDCITY_GETLOG_T*)pCmd;
			P_DBS4WEBUSER_WORLDCITY_GETLOG_T*	pRpl	= (P_DBS4WEBUSER_WORLDCITY_GETLOG_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_WORLDCITY_GETLOG_RPL;
			pRpl->nRst			= STC_GAMECMD_WORLDCITY_GETLOG_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity_log,DBERR");
				pRpl->nRst		= STC_GAMECMD_WORLDCITY_GETLOG_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_worldcity_log_man(%d,%d,%s)", pReq->nPageNum, pReq->nPageIdx, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity_log,SQLERR");
					pRpl->nRst	= STC_GAMECMD_WORLDCITY_GETLOG_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					WorldCityLog *p = (WorldCityLog*)wh_getptrnexttoptr(pRpl);
					int	nRank		= pReq->nPageIdx*pReq->nPageNum;
					while (q.FetchRow())
					{
						//p->nID = q.GetVal_32();
						p->nID		= ++nRank;
						p->nAccountID = q.GetVal_64();
						strcpy(p->szName, q.GetStr());
						p->nHeadID = q.GetVal_32();
						p->nDiamond = q.GetVal_32();
						p->nCup		= q.GetVal_32();

						++p;
					}
					q.FreeResult();
					q.SpawnQuery("select @vMaxPageNum");
					q.GetResult(nPreRet);
					q.FetchRow();
					pRpl->nMaxPage = q.GetVal_32();

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"worldcity_log,%d", pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)p-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_WORLDCITY_GETLOG_ALLIANCE_REQ:
		{
			P_DBS4WEB_WORLDCITY_GETLOG_ALLIANCE_T*	pReq		= (P_DBS4WEB_WORLDCITY_GETLOG_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_WORLDCITY_GETLOG_ALLIANCE_T*	pRpl	= (P_DBS4WEBUSER_WORLDCITY_GETLOG_ALLIANCE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_WORLDCITY_GETLOG_ALLIANCE_RPL;
			pRpl->nRst			= STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity_log_alliance,DBERR");
				pRpl->nRst		= STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_worldcity_log_alliance(%d,%d,%s)", pReq->nPageNum, pReq->nPageIdx, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity_log_alliance,SQLERR");
					pRpl->nRst	= STC_GAMECMD_WORLDCITY_GETLOG_ALLIANCE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					WorldCityLogAlliance *p = (WorldCityLogAlliance*)wh_getptrnexttoptr(pRpl);
					int	nRank	= pReq->nPageIdx*pReq->nPageNum;
					while (q.FetchRow())
					{
						//p->nID = q.GetVal_32();
						p->nID	= ++nRank;
						p->nAllianceID = q.GetVal_64();
						strcpy(p->szName, q.GetStr());
						p->nDiamond = q.GetVal_32();
						p->nCup		= q.GetVal_32();

						++p;
					}
					q.FreeResult();
					q.SpawnQuery("select @vMaxPageNum");
					q.GetResult(nPreRet);
					q.FetchRow();
					pRpl->nMaxPage = q.GetVal_32();

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"worldcity_log_alliance,%d", pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)p-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_WORLDCITY_RANK_MAN_REQ:
		{
			P_DBS4WEB_WORLDCITY_RANK_MAN_T*	pReq		= (P_DBS4WEB_WORLDCITY_RANK_MAN_T*)pCmd;
			P_DBS4WEBUSER_WORLDCITY_RANK_MAN_T*	pRpl	= (P_DBS4WEBUSER_WORLDCITY_RANK_MAN_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_WORLDCITY_RANK_MAN_RPL;
			pRpl->nRst			= STC_GAMECMD_WORLDCITY_RANK_MAN_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity_rank_man,DBERR");
				pRpl->nRst		= STC_GAMECMD_WORLDCITY_RANK_MAN_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_worldcity_rank_man(%d,%d,%s)", pReq->nPageNum, pReq->nPageIdx, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity_rank_man,SQLERR");
					pRpl->nRst	= STC_GAMECMD_WORLDCITY_RANK_MAN_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum	= q.NumRows();
					WorldCityRankMan *p = (WorldCityRankMan*)wh_getptrnexttoptr(pRpl);
					int	nRank	= pReq->nPageIdx*pReq->nPageNum;
					while (q.FetchRow())
					{
						//p->nID = q.GetVal_32();
						p->nID			= ++nRank;
						p->nAccountID	= q.GetVal_64();
						strcpy(p->szName, q.GetStr());
						p->nHeadID		= q.GetVal_32();
						p->nLevel		= q.GetVal_32();
						p->nCup			= q.GetVal_32();

						++p;
					}
					q.FreeResult();
					q.SpawnQuery("select @vMaxPageNum");
					q.GetResult(nPreRet);
					q.FetchRow();
					pRpl->nMaxPage = q.GetVal_32();

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"worldcity_rank_man,%d", pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)p-vectRawBuf.getbuf());
					return 0;
				}
			}
		}
		break;
	case CMDID_WORLDCITY_RANK_ALLIANCE_REQ:
		{
			P_DBS4WEB_WORLDCITY_RANK_ALLIANCE_T*	pReq		= (P_DBS4WEB_WORLDCITY_RANK_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_WORLDCITY_RANK_ALLIANCE_T*	pRpl	= (P_DBS4WEBUSER_WORLDCITY_RANK_ALLIANCE_T*)vectRawBuf.getbuf();
			pRpl->nCmd			= P_DBS4WEB_RPL_CMD;
			pRpl->nSubCmd		= CMDID_WORLDCITY_RANK_ALLIANCE_RPL;
			pRpl->nRst			= STC_GAMECMD_WORLDCITY_RANK_ALLIANCE_T::RST_OK;
			pRpl->nNum			= 0;
			memcpy(pRpl->nExt, pReq->nExt, sizeof(pRpl->nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity_rank_alliance,DBERR");
				pRpl->nRst		= STC_GAMECMD_WORLDCITY_RANK_ALLIANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
				return 0;
			}
			else
			{
				q.SpawnQuery("call game_worldcity_rank_alliance(%d,%d,%s)", pReq->nPageNum, pReq->nPageIdx, SP_RESULT);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"worldcity_rank_alliance,SQLERR");
					pRpl->nRst	= STC_GAMECMD_WORLDCITY_RANK_ALLIANCE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), sizeof(*pRpl));
					return 0;
				}
				else
				{
					pRpl->nNum = q.NumRows();
					WorldCityRankAlliance *p = (WorldCityRankAlliance*)wh_getptrnexttoptr(pRpl);
					int	nRank	= pReq->nPageIdx*pReq->nPageNum;
					while (q.FetchRow())
					{
						//p->nID = q.GetVal_32();
						p->nID	= ++nRank;
						p->nAllianceID = q.GetVal_64();
						strcpy(p->szName, q.GetStr());
						p->nLevel = q.GetVal_32();
						p->nCup = q.GetVal_32();

						++p;
					}

					q.FreeResult();
					q.SpawnQuery("select @vMaxPageNum");
					q.GetResult(nPreRet);
					q.FetchRow();
					pRpl->nMaxPage = q.GetVal_32();

					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"worldcity_rank_alliance,%d", pRpl->nRst);
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), (char*)p-vectRawBuf.getbuf());
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
