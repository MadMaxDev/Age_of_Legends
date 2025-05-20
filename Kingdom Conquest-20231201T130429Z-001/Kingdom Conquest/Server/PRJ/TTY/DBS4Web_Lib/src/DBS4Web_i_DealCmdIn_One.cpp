#include "../inc/DBS4Web_i.h"
#include <errno.h>

using namespace n_pngs;

int		DBS4Web_i::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	return 0;
}

int		DBS4Web_i::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch (nCmd)
	{
	case PNGSPACKET_2LOGIC_SETMOOD:
		{
			m_bStopping				= true;
			m_tickStoppingBegin		= wh_gettickcount();
		}
		break;
	case PNGS_TR2CD_NOTIFY:
		{
			PNGS_TR2CD_NOTIFY_T*	pCmd	= (PNGS_TR2CD_NOTIFY_T*)pData;
			switch (pCmd->nSubCmd)
			{
			case PNGS_TR2CD_NOTIFY_T::SUBCMD_DISCONNECT:
				{
					m_nLPCntrID				= -1;
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1359,DBS4Web_RT)"DBS4Web_i::DealCmdIn_One,disconnect,cntr id:0x%X", pCmd->nParam1);
				}
				break;
			case PNGS_TR2CD_NOTIFY_T::SUBCMD_CONNECT:
				{
					m_nLPCntrID				= pCmd->nParam1;
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1359,DBS4Web_RT)"DBS4Web_i::DealCmdIn_One,connect,cntr id:0x%X", pCmd->nParam1);
				}
				break;
			default:
				{

				}
				break;
			}
		}
		break;
	case PNGS_TR2CD_CMD:
		{
			return DealCmdIn_One_PNGS_TR2CD_CMD(pData, nDSize);
		}
		break;
	default:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1359,DBS4Web_RT)"DBS4Web_i::DealCmdIn_One,%d,unknown,%d", nCmd, nDSize);
		}
		break;
	}
	return 0;
}

int		DBS4Web_i::DealCmdIn_One_PNGS_TR2CD_CMD(const void* pData, int nDSize)
{
	PNGS_TR2CD_CMD_T*	pCDCmd	= (PNGS_TR2CD_CMD_T*)pData;
	P_DBS4WEB_CMD_T*	pCmd	= (P_DBS4WEB_CMD_T*)pCDCmd->pData;
	switch (pCmd->nCmd)
	{
	case P_DBS4WEB_REQ_CMD:
		{
			//处理的时候需要发回响应结果,所以PNGS_TR2CD_CMD_T也存储掉
			int	nTotalSize		= sizeof(PNGS_TR2CD_CMD_T) + pCDCmd->nDSize;

			m_queueCmdInMT.lock();
			void*	pBuf		 = m_queueCmdInMT.whsmpqueue::InAlloc(nTotalSize);
			if (pBuf != NULL)
			{
				PNGS_TR2CD_CMD_T*	pCDCmdBuf	= (PNGS_TR2CD_CMD_T*)pBuf;
				memcpy(pCDCmdBuf, pCDCmd, sizeof(*pCDCmd));
				pCDCmdBuf->pData				= wh_getptrnexttoptr(pCDCmdBuf);
				memcpy(pCDCmdBuf->pData, pCDCmd->pData, pCDCmd->nDSize);
			}
			m_queueCmdInMT.unlock();
		}
		break;
	default:
		{

		}
		break;
	}
	return 0;
}

int		DBS4Web_i::ThreadDealCmd(void* pData, int nDSize, whtid_t tid, whvector<char>& vectRawBuf)
{
	MYSQL*	pMySQL	= GetSelfMySQLHandle(tid);
	if (pMySQL == NULL)
	{
		pMySQL		= BorrowMySQLHandle(tid);
	}
	
	//!!!这个地方后续开发应该注意
	//1.是丢弃请求,还是把请求再次放入m_queueCmdInMT队列中
	//2.这个线程是否需要关闭,拿不到可用句柄的线程是无意义的
	//3.log是否需要把这个请求的详细数据都记录下来
	if (pMySQL == NULL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1359,DBS4Web_RT)"DBS4Web_i::ThreadDealCmd, no availabel mysql handle,tid 0x%X", tid);
		return -1;
	}

	PNGS_TR2CD_CMD_T*	pCDCmd		= (PNGS_TR2CD_CMD_T*)pData;
	P_DBS4WEB_CMD_T*	pCmd		= (P_DBS4WEB_CMD_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	
	switch (pCmd->nSubCmd)//上层已经过滤了,大指令为P_DBS4WEB_REQ_CMD
	{
	case CMDID_ACCOUNT_ONLINE_REQ:
		{
			P_DBS4WEB_ACCOUNT_ONLINE_T*	pAccountOnline	= (P_DBS4WEB_ACCOUNT_ONLINE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			int			nRst		= 0;
			P_DBS4WEBUSER_ACCOUNT_ONLINE_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_ACCOUNT_ONLINE_RPL;
			memcpy(RplCmd.nExt, pAccountOnline->nExt, sizeof(RplCmd.nExt));
			RplCmd.nRst				= TTY_LOGIN_RST_SUCCEED;
			if (!q.Connected())
			{
				RplCmd.nRst			= TTY_LOGIN_RST_DBERR;
			}
			else
			{
				q.SpawnQuery("CALL account_login(0x%"WHINT64PRFX"X, %u, %d, %s)", pAccountOnline->nAccountID, pAccountOnline->nIP, (int)pAccountOnline->nBinded, SP_RESULT);
				int	nPreRst	= 0;
				q.ExecuteSPWithResult(nPreRst);
				if (nPreRst != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= TTY_LOGIN_RST_DBERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"acccount_online,%d,0x%"WHINT64PRFX"X",nPreRst, pAccountOnline->nAccountID);
				}
				else
				{
					if (q.FetchRow())
					{
						nRst			= q.GetVal_32();
						switch (nRst)
						{
						case -100:
						case -101:
						case -102:
						case -103:
							{
								RplCmd.nRst	= TTY_LOGIN_RST_SQLERR;
							}
							break;
						case -1:
							{
								RplCmd.nRst	= TTY_LOGIN_RST_NOCHAR;
							}
							break;
						case -10:
							{
								RplCmd.nRst	= TTY_LOGIN_RST_CHAR_BANNED;
							}
							break;
						case 0:
							{
								RplCmd.nRst	= TTY_LOGIN_RST_SUCCEED;
							}
							break;
						default:
							{
								RplCmd.nRst	= TTY_LOGIN_RST_SQLERR;
							}
							break;
						}
					}
					else
					{
						RplCmd.nRst			= TTY_LOGIN_RST_SQLERR;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"acccount_online,%d,0x%"WHINT64PRFX"X,%d,%d",nPreRst, pAccountOnline->nAccountID, nRst, (int)RplCmd.nRst);
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_LOAD_CHAR_REQ:
		{
			P_DBS4WEB_LOAD_CHAR_T*	pLoadChar	= (P_DBS4WEB_LOAD_CHAR_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				P_DBS4WEBUSER_LOAD_CHAR_T	RplCmd;
				RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
				RplCmd.nSubCmd		= CMDID_LOAD_CHAR_RPL;
				RplCmd.nRst			= P_DBS4WEBUSER_LOAD_CHAR_T::RST_DBERR;
				memcpy(RplCmd.nExt, pLoadChar->nExt, sizeof(RplCmd.nExt));
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
			}
			else
			{
				vectRawBuf.resize(sizeof(P_DBS4WEBUSER_LOAD_CHAR_T) + sizeof(CharAtb));
				P_DBS4WEBUSER_LOAD_CHAR_T*	pRplCmd	= (P_DBS4WEBUSER_LOAD_CHAR_T*)vectRawBuf.getbuf();
				pRplCmd->nCmd		= P_DBS4WEB_RPL_CMD;
				pRplCmd->nSubCmd	= CMDID_LOAD_CHAR_RPL;
				pRplCmd->nRst		= P_DBS4WEBUSER_LOAD_CHAR_T::RST_OK;
				memcpy(pRplCmd->nExt, pLoadChar->nExt, sizeof(pRplCmd->nExt));

				void*	pPos		= wh_getptrnexttoptr(pRplCmd);
				try
				{
					// 1.获取角色属性
					q.SpawnQuery("CALL get_char_atb(0x%"WHINT64PRFX"X)", pLoadChar->nAccountID);
					int	nPreRst		= 0;
					q.ExecuteSPWithResult(nPreRst);
					if (nPreRst != MYSQL_QUERY_NORMAL)
					{
						pRplCmd->nRst	= P_DBS4WEBUSER_LOAD_CHAR_T::RST_SQLERR;
						throw -1;
					}
					else
					{
						if (q.FetchRow())
						{
							CharAtb*	pCharAtb		= (CharAtb*)pPos;
							pCharAtb->nAccountID		= pLoadChar->nAccountID;
							WH_STRNCPY0(pCharAtb->szName, q.GetStr());
							pCharAtb->nLastLoginTime	= q.GetVal_32();
							pCharAtb->nLastLogoutTime	= q.GetVal_32();
							pCharAtb->nExp				= q.GetVal_32();
							pCharAtb->nLevel			= q.GetVal_32();
							pCharAtb->nDiamond			= q.GetVal_32();
							pCharAtb->nCrystal			= q.GetVal_32();
							pCharAtb->nGold				= q.GetVal_32();
							pCharAtb->nVip				= q.GetVal_32();
							pCharAtb->nSex				= q.GetVal_32();
							pCharAtb->nHeadID			= q.GetVal_32();
							pCharAtb->nPosX				= q.GetVal_32();
							pCharAtb->nPosY				= q.GetVal_32();
							pCharAtb->nDevelopment		= q.GetVal_32();
							pCharAtb->nPopulation		= q.GetVal_32();
							pCharAtb->nBuildNum			= q.GetVal_32();
							pCharAtb->nAddedBuildNum	= q.GetVal_32();
							pCharAtb->nFreeDrawLotteryNum	= q.GetVal_32();

							pCharAtb->nAllianceID		= q.GetVal_64();
							WH_STRNCPY0(pCharAtb->szAllianceName, q.GetStr());

							pCharAtb->nDrug				= q.GetVal_32();
							pCharAtb->nTradeNum			= q.GetVal_32();

							WH_STRNCPY0(pCharAtb->szSignature, q.GetStr());
							pCharAtb->nChangeNameNum	= q.GetVal_32();

							pCharAtb->nProtectTimeEndTick		= q.GetVal_32();
							pCharAtb->nAddBuildNumTimeEndTick	= q.GetVal_32();
							unsigned int nTimeNow		= wh_time();
							pCharAtb->nProtectTime		= nTimeNow>pCharAtb->nProtectTimeEndTick?0:pCharAtb->nProtectTimeEndTick-nTimeNow;
							pCharAtb->nAddBuildNumTime	= nTimeNow>pCharAtb->nAddBuildNumTimeEndTick?0:pCharAtb->nAddBuildNumTimeEndTick-nTimeNow;
							pCharAtb->nTotalBuildNum	= q.GetVal_32();
							pCharAtb->nNotificationID	= q.GetVal_32();
							pCharAtb->nAllianceDrawLotteryNum	= q.GetVal_32();
							pCharAtb->nTotalDiamond		= q.GetVal_32();
							pCharAtb->nInstanceWangzhe	= q.GetVal_32();
							pCharAtb->nInstanceZhengzhan	= q.GetVal_32();
							pCharAtb->nCup				= q.GetVal_32();
							pCharAtb->bBinded			= (bool)q.GetVal_32();
							pCharAtb->bVipDisplay		= (bool)q.GetVal_32();
						}
						else
						{
							pRplCmd->nRst	= P_DBS4WEBUSER_LOAD_CHAR_T::RST_NOCHAR;
							throw -2;
						}
					}


					// n.数据发回
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
				catch (int nErrCode)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,LOAD_CHAR)"%d,0x%"WHINT64PRFX"X,%d", nErrCode, pLoadChar->nAccountID, (int)pRplCmd->nRst);
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_LOAD_CHAR_T));
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
			}
		}
		break;
	case CMDID_ACCOUNT_OFFLINE_REQ:
		{
			P_DBS4WEB_ACCOUNT_OFFLINE_T*	pAccountOffline	= (P_DBS4WEB_ACCOUNT_OFFLINE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				
			}
			else
			{
				q.SpawnQuery("CALL account_logout(0x%"WHINT64PRFX"X)", pAccountOffline->nAccountID);
				int	nPreRst	= 0;
				q.Execute(nPreRst);
			}
		}
		break;
	case CMDID_CHAR_CREATE_REQ:
		{
			P_DBS4WEB_CHAR_CREATE_T*	pCreateChar		= (P_DBS4WEB_CHAR_CREATE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			int			nRst		= 0;
			P_DBS4WEBUSER_CHAR_CREATE_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_CHAR_CREATE_RPL;
			memcpy(RplCmd.nExt, pCreateChar->nExt, sizeof(RplCmd.nExt));
			RplCmd.nRst				= TTY_CHAR_CREATE_RST_OK;
			if (!q.Connected())
			{
				RplCmd.nRst			= TTY_CHAR_CREATE_RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL create_char(0x%"WHINT64PRFX"X, \'%s\', %u, %u, %u, %u, %d, %u)"
					, pCreateChar->nAccountID, pCreateChar->szCharName, pCreateChar->nSex, pCreateChar->nHeadID
					, pCreateChar->nPosX, pCreateChar->nPosY, (int)pCreateChar->nBinded, pCreateChar->nGiftAppID);
				int	nPreRst	= 0;
				q.ExecuteSPWithResult(nPreRst);
				if (nPreRst != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= TTY_CHAR_CREATE_RST_DB_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"create_char,DBERROR,%d,0x%"WHINT64PRFX"X",nPreRst, pCreateChar->nAccountID);
				}
				else
				{
					if (q.FetchRow())
					{
						nRst			= q.GetVal_32();
						if (nRst != 0)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"create_char,SQLERROR,%d,0x%"WHINT64PRFX"X", nRst, pCreateChar->nAccountID);
						}
						switch (nRst)
						{
						case -1:
							{
								RplCmd.nRst	= TTY_CHAR_CREATE_RST_ALREADYEXISTCHAR;
							}
							break;
						case -2:
							{
								RplCmd.nRst	= TTY_CHAR_CREATE_RST_NAMEDUP;
							}
							break;
						case -3:
						case -4:
						case -100:
							{
								RplCmd.nRst	= TTY_CHAR_CREATE_RST_SQL_ERR;
							}
							break;
						case -8:
							{
								RplCmd.nRst	= TTY_CHAR_CREATE_RST_POSITION_DUP;
							}
							break;
						case 0:
							{
								RplCmd.nRst	= TTY_CHAR_CREATE_RST_OK;
							}
							break;
						default:
							{
								RplCmd.nRst	= TTY_CHAR_CREATE_RST_SQL_ERR;
							}
							break;
						}
					}
					else
					{
						RplCmd.nRst			= TTY_CHAR_CREATE_RST_SQL_ERR;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"create_char,%d,0x%"WHINT64PRFX"X,%d", nRst, pCreateChar->nAccountID, (int)RplCmd.nRst);
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_LOAD_PLAYERCARD_REQ:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"load_player_card");
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_player_card,DBERROR");
				P_DBS4WEBUSER_LOAD_PLAYERCARD_T	RplCmd;
				RplCmd.nCmd		= P_DBS4WEB_RPL_CMD;
				RplCmd.nSubCmd	= CMDID_LOAD_PLAYERCARD_RPL;
				RplCmd.nRst		= P_DBS4WEBUSER_LOAD_PLAYERCARD_T::RST_DBERR;
				RplCmd.nNum		= 0;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL load_player_card()");
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_player_card,SQLERROR,%d", nPreRet);
					P_DBS4WEBUSER_LOAD_PLAYERCARD_T	RplCmd;
					RplCmd.nCmd		= P_DBS4WEB_RPL_CMD;
					RplCmd.nSubCmd	= CMDID_LOAD_PLAYERCARD_RPL;
					RplCmd.nRst		= P_DBS4WEBUSER_LOAD_PLAYERCARD_T::RST_SQLERR;
					RplCmd.nNum		= 0;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					int	nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_LOAD_PLAYERCARD_T) + nNum*sizeof(PlayerCard));
					P_DBS4WEBUSER_LOAD_PLAYERCARD_T&	RplCmd	= *(P_DBS4WEBUSER_LOAD_PLAYERCARD_T*)vectRawBuf.getbuf();
					RplCmd.nCmd		= P_DBS4WEB_RPL_CMD;
					RplCmd.nSubCmd	= CMDID_LOAD_PLAYERCARD_RPL;
					RplCmd.nRst		= P_DBS4WEBUSER_LOAD_PLAYERCARD_T::RST_OK;
					RplCmd.nNum		= nNum;
					PlayerCard*	pCard	= (PlayerCard*)wh_getptrnexttoptr(&RplCmd);
					while (q.FetchRow())
					{
						pCard->nAccountID	= q.GetVal_64();
						WH_STRNCPY0(pCard->szName, q.GetStr());
						pCard->nLevel		= q.GetVal_32();
						pCard->nVip			= q.GetVal_32();
						pCard->nSex			= q.GetVal_32();
						pCard->nHeadID		= q.GetVal_32();
						pCard->nPosX		= q.GetVal_32();
						pCard->nPosY		= q.GetVal_32();
						pCard->nAllianceID	= q.GetVal_64();
						WH_STRNCPY0(pCard->szAllianceName, q.GetStr());
						pCard->nDevelopment	= q.GetVal_32();
						WH_STRNCPY0(pCard->szSignature, q.GetStr());
						pCard->nProtectTimeEndTick	= q.GetVal_32();
						pCard->nCityLevel	= q.GetVal_32();
						pCard->nLevelRank	= 0;
						unsigned int nTimeNow		= wh_time();
						pCard->nProtectTime		= nTimeNow>pCard->nProtectTimeEndTick?0:pCard->nProtectTimeEndTick-nTimeNow;
						pCard->nInstanceWangzhe	= q.GetVal_32();
						pCard->nInstanceZhengzhan	= q.GetVal_32();
						pCard->nCup				= q.GetVal_32();
						pCard->bVipDisplay		= (bool)q.GetVal_32();

						pCard++; 
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
			}
		}
		break;
	case CMDID_LOAD_TERRAIN_REQ:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"load_terrain");
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_terrain,DBERROR");
				P_DBS4WEBUSER_LOAD_TERRAIN_T	RplCmd;
				RplCmd.nCmd		= P_DBS4WEB_RPL_CMD;
				RplCmd.nSubCmd	= CMDID_LOAD_TERRAIN_RPL;
				RplCmd.nRst		= P_DBS4WEBUSER_LOAD_TERRAIN_T::RST_DBERR;
				RplCmd.nNum		= 0;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT pos_x,pos_y,can_build,floor_1,floor_2 FROM excel_terrain_info");
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_terrain,SQLERROR");
					P_DBS4WEBUSER_LOAD_TERRAIN_T	RplCmd;
					RplCmd.nCmd		= P_DBS4WEB_RPL_CMD;
					RplCmd.nSubCmd	= CMDID_LOAD_TERRAIN_RPL;
					RplCmd.nRst		= P_DBS4WEBUSER_LOAD_TERRAIN_T::RST_SQLERR;
					RplCmd.nNum		= 0;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					int	nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_LOAD_TERRAIN_T) + nNum*sizeof(TerrainUnit));
					P_DBS4WEBUSER_LOAD_TERRAIN_T&	RplCmd	= *(P_DBS4WEBUSER_LOAD_TERRAIN_T*)vectRawBuf.getbuf();
					RplCmd.nCmd		= P_DBS4WEB_RPL_CMD;
					RplCmd.nSubCmd	= CMDID_LOAD_TERRAIN_RPL;
					RplCmd.nRst		= P_DBS4WEBUSER_LOAD_PLAYERCARD_T::RST_OK;
					RplCmd.nNum		= nNum;
					TerrainUnit*	pUnit	= (TerrainUnit*)wh_getptrnexttoptr(&RplCmd);
					while (q.FetchRow())
					{
						pUnit->nPosX		= q.GetVal_32();
						pUnit->nPosY		= q.GetVal_32();
						pUnit->info.bCanBuild		= (bool)q.GetVal_32();
						pUnit->info.nFloor1			= (char)q.GetVal_32();
						pUnit->info.nFloor2			= (char)q.GetVal_32();

						pUnit++; 
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
			}
		}
		break;
	case CMDID_GET_TIMEEVENT_LIST_REQ:
		{
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				// 无所谓,下个时间事件到了还会继续发来请求的
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL get_te_before_now()");
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					return 0;
				}
				else
				{
					int	nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T) + nNum*sizeof(P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T::TE_UNIT_T));
					P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T&	RplCmd	= *(P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T*)vectRawBuf.getbuf();
					RplCmd.nCmd		= P_DBS4WEB_RPL_CMD;
					RplCmd.nSubCmd	= CMDID_GET_TIMEEVENT_LIST_RPL;
					RplCmd.nNum		= nNum;
					P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T::TE_UNIT_T*	pTEUnit	= (P_DBS4WEBUSER_GET_DB_TIMEEVENT_LIST_T::TE_UNIT_T*)wh_getptrnexttoptr(&RplCmd);
					while (q.FetchRow())
					{
						pTEUnit->nEventID	= q.GetVal_64();
						pTEUnit->nType		= q.GetVal_32();

						pTEUnit++; 
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
			}
		}
		break;
	case CMDID_DEAL_BUILDING_TE_REQ:
		{
			P_DBS4WEB_DEAL_BUILDING_TE_T*	pDealTE	= (P_DBS4WEB_DEAL_BUILDING_TE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				// 无所谓,下个时间事件到了还会继续发来请求的
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL deal_building_event(0x%"WHINT64PRFX"X)", pDealTE->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_building_event,SQLERROR,%d,0x%"WHINT64PRFX"X", nPreRet, pDealTE->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_account_id,_auto_id,_excel_id,_type;
						int			nResult		= q.GetVal_32();
						tty_id_t	nAccountID	= q.GetVal_64();
						int			nAutoID		= q.GetVal_32();
						int			nExcelID	= q.GetVal_32();
						int			nType		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal_building_event,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pDealTE->nEventID, nResult, nAccountID, nAutoID, nExcelID, nType);

						P_DBS4WEBUSER_DEAL_BUILDING_TE_T	RplCmd;
						RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
						RplCmd.nSubCmd			= CMDID_DEAL_BUILDING_TE_RPL;
						RplCmd.nAccountID		= nAccountID;
						RplCmd.nAutoID			= nAutoID;
						RplCmd.nExcelID			= nExcelID;
						RplCmd.nType			= nType;
						RplCmd.nRst				= nResult;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_building_event,FOUND_COUNT()=0,0x%"WHINT64PRFX"X"
							, pDealTE->nEventID);
					}
				}
			}
		}
		break;
	case CMDID_DEAL_ONCE_PERPERSON_TE_REQ:
		{
			P_DBS4WEB_DEAL_ONCE_PERPERSON_TE_T*	pDealTE	= (P_DBS4WEB_DEAL_ONCE_PERPERSON_TE_T*)pCmd;
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL deal_once_perperson_event(0x%"WHINT64PRFX"X)", pDealTE->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_once_perperson_event,SQLERROR,%d,0x%"WHINT64PRFX"X", nPreRet, pDealTE->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_account_id,_type,_production;
						int			nResult		= q.GetVal_32();
						tty_id_t	nAccountID	= q.GetVal_64();
						int			nType		= q.GetVal_32();
						int			nProduction	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal_once_perperson_event,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d"
							, pDealTE->nEventID, nResult, nAccountID, nType, nProduction);

						if (nResult == 0)
						{
							P_DBS4WEBUSER_DEAL_ONCE_PERPERSON_TE_T	Rst;
							Rst.nCmd		= P_DBS4WEB_RPL_CMD;
							Rst.nSubCmd		= CMDID_DEAL_ONCE_PERPERSON_TE_RPL;
							Rst.nResult		= nResult;
							Rst.nAccountID	= nAccountID;
							Rst.nType		= nType;
							Rst.nProduction	= nProduction;

							SendMsg_MT(pCDCmd->nConnecterID, &Rst, sizeof(Rst));
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_once_perperson_event,FOUND_COUNT()=0,0x%"WHINT64PRFX"X", pDealTE->nEventID);
					}
				}
			}
		}
		break;
	case CMDID_GET_BUILDING_LIST_REQ:
		{
			P_DBS4WEB_GET_BUILDING_LIST_T*	pGetBuildingList	= (P_DBS4WEB_GET_BUILDING_LIST_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_building_list,DBERROR,0x%"WHINT64PRFX"X", pGetBuildingList->nAccountID);

				P_DBS4WEBUSER_GET_BUILDING_LIST_T RplCmd;
				RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
				RplCmd.nSubCmd		= CMDID_GET_BUILDING_LIST_RPL;
				RplCmd.nAccountID	= pGetBuildingList->nAccountID;
				RplCmd.nNum			= 0;
				RplCmd.nRst			= STC_GAMECMD_GET_BUILDING_LIST_T::RST_DB_ERR;
				memcpy(RplCmd.nExt, pGetBuildingList->nExt, sizeof(RplCmd.nExt));
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT auto_id,excel_id,level FROM buildings WHERE account_id=0x%"WHINT64PRFX"X", pGetBuildingList->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_building_list,SQLERROR,%d,0x%"WHINT64PRFX"X", nPreRet, pGetBuildingList->nAccountID);

					P_DBS4WEBUSER_GET_BUILDING_LIST_T RplCmd;
					RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
					RplCmd.nSubCmd		= CMDID_GET_BUILDING_LIST_RPL;
					RplCmd.nAccountID	= pGetBuildingList->nAccountID;
					RplCmd.nNum			= 0;
					RplCmd.nRst			= STC_GAMECMD_GET_BUILDING_LIST_T::RST_SQL_ERR;
					memcpy(RplCmd.nExt, pGetBuildingList->nExt, sizeof(RplCmd.nExt));
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					int		nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_BUILDING_LIST_T) + nNum*sizeof(BuildingUnit));
					P_DBS4WEBUSER_GET_BUILDING_LIST_T*	pRplCmd	= (P_DBS4WEBUSER_GET_BUILDING_LIST_T*)vectRawBuf.getbuf();
					pRplCmd->nCmd		= P_DBS4WEB_RPL_CMD;
					pRplCmd->nSubCmd	= CMDID_GET_BUILDING_LIST_RPL;
					pRplCmd->nAccountID	= pGetBuildingList->nAccountID;
					pRplCmd->nNum		= nNum;
					pRplCmd->nRst		= STC_GAMECMD_GET_BUILDING_LIST_T::RST_OK;
					memcpy(pRplCmd->nExt, pGetBuildingList->nExt, sizeof(pRplCmd->nExt));
					BuildingUnit*	pBuildingUnit	= (BuildingUnit*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pBuildingUnit->nAutoID		= q.GetVal_32();
						pBuildingUnit->nExcelID		= q.GetVal_32();
						pBuildingUnit->nLevel		= q.GetVal_32();

						++pBuildingUnit;
					}

					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
			}
		}
		break;
	case CMDID_GET_BUILDING_TE_LIST_REQ:
		{
			P_DBS4WEB_GET_BUILDING_TE_LIST_T*	pGetBuildingTEList	= (P_DBS4WEB_GET_BUILDING_TE_LIST_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_building_te_list,DBERROR,0x%"WHINT64PRFX"X", pGetBuildingTEList->nAccountID);

				P_DBS4WEBUSER_GET_BUILDING_TE_LIST_T RplCmd;
				RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
				RplCmd.nSubCmd		= CMDID_GET_BUILDING_TE_LIST_RPL;
				RplCmd.nNum			= 0;
				RplCmd.nRst			= STC_GAMECMD_GET_BUILDING_TE_LIST_T::RST_DB_ERR;
				RplCmd.nTimeNow		= 0;
				memcpy(RplCmd.nExt, pGetBuildingTEList->nExt, sizeof(RplCmd.nExt));
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT b.auto_id,b.excel_id,b.type,t.begin_time,t.end_time FROM building_time_events b JOIN time_events t ON b.event_id=t.event_id WHERE b.account_id=0x%"WHINT64PRFX"X", pGetBuildingTEList->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_building_te_list,SQLERROR,%d,0x%"WHINT64PRFX"X", nPreRet,  pGetBuildingTEList->nAccountID);

					P_DBS4WEBUSER_GET_BUILDING_TE_LIST_T RplCmd;
					RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
					RplCmd.nSubCmd		= CMDID_GET_BUILDING_TE_LIST_RPL;
					RplCmd.nNum			= 0;
					RplCmd.nRst			= STC_GAMECMD_GET_BUILDING_TE_LIST_T::RST_SQL_ERR;
					RplCmd.nTimeNow		= 0;
					memcpy(RplCmd.nExt, pGetBuildingTEList->nExt, sizeof(RplCmd.nExt));
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					int		nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_BUILDING_TE_LIST_T) + nNum*sizeof(BuildingTEUnit));
					P_DBS4WEBUSER_GET_BUILDING_TE_LIST_T*	pRplCmd	= (P_DBS4WEBUSER_GET_BUILDING_TE_LIST_T*)vectRawBuf.getbuf();
					pRplCmd->nCmd		= P_DBS4WEB_RPL_CMD;
					pRplCmd->nSubCmd	= CMDID_GET_BUILDING_TE_LIST_RPL;
					pRplCmd->nNum		= nNum;
					pRplCmd->nRst		= STC_GAMECMD_GET_BUILDING_TE_LIST_T::RST_OK;
					pRplCmd->nTimeNow	= wh_time();
					memcpy(pRplCmd->nExt, pGetBuildingTEList->nExt, sizeof(pRplCmd->nExt));
					BuildingTEUnit*	pBuildingTEUnit	= (BuildingTEUnit*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pBuildingTEUnit->nAutoID	= q.GetVal_32();
						pBuildingTEUnit->nExcelID	= q.GetVal_32();
						pBuildingTEUnit->nType		= q.GetVal_32();
						pBuildingTEUnit->nBeginTime	= q.GetVal_32();
						pBuildingTEUnit->nEndTime	= q.GetVal_32();

						++pBuildingTEUnit;
					}

					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
			}
		}
		break;
	case CMDID_BUILD_BUILDING_REQ:
		{
			P_DBS4WEB_BUILD_BUILDING_T*	pBuildBuilding	= (P_DBS4WEB_BUILD_BUILDING_T*)pCmd;

			P_DBS4WEBUSER_BUILD_BUILDING_T RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_BUILD_BUILDING_RPL;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_BUILD_BUILDING_T::RST_OK;
			RplCmd.nAutoID		= pBuildBuilding->nAutoID;
			RplCmd.nExcelID		= pBuildBuilding->nExcelID;
			RplCmd.nAddedCharDev= 0;
			RplCmd.nAddedCharExp= 0;
			RplCmd.nCurCharExp	= 0;
			RplCmd.nCurCharLevel= 0;
			RplCmd.nAccountID	= pBuildBuilding->nAccountID;
			memcpy(RplCmd.nExt, pBuildBuilding->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"build_building,DBERROR,0x%"WHINT64PRFX"X,%d,%d", pBuildBuilding->nAccountID, pBuildBuilding->nExcelID, pBuildBuilding->nAutoID);

				RplCmd.nRst			= STC_GAMECMD_OPERATE_BUILD_BUILDING_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL build_building(0x%"WHINT64PRFX"X, %d, %d)", pBuildBuilding->nAccountID, pBuildBuilding->nExcelID, pBuildBuilding->nAutoID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"build_building,SQLERROR,0x%"WHINT64PRFX"X,%d,%d", pBuildBuilding->nAccountID, pBuildBuilding->nExcelID, pBuildBuilding->nAutoID);
					
					RplCmd.nRst			= STC_GAMECMD_OPERATE_BUILD_BUILDING_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nAddedCharExp= q.GetVal_32();
						RplCmd.nAddedCharDev= q.GetVal_32();
						RplCmd.nCurCharExp	= q.GetVal_32();
						RplCmd.nCurCharLevel= q.GetVal_32();
						RplCmd.nGoldCost	= q.GetVal_32();
						RplCmd.nPopulationCost	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst			= STC_GAMECMD_OPERATE_BUILD_BUILDING_T::RST_SQL_ERR;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"build_building,%d,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d,%d,%d"
						, (int)RplCmd.nRst, pBuildBuilding->nAccountID, pBuildBuilding->nExcelID, pBuildBuilding->nAutoID, RplCmd.nAddedCharExp, RplCmd.nAddedCharDev, RplCmd.nCurCharExp, RplCmd.nCurCharLevel, RplCmd.nGoldCost, RplCmd.nPopulationCost);
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_UPGRADE_BUILDING_REQ:
		{
			P_DBS4WEB_UPGRADE_BUILDING_T*	pUpgradeBuilding	= (P_DBS4WEB_UPGRADE_BUILDING_T*)pCmd;

			P_DBS4WEBUSER_UPGRADE_BUILDING_T RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_UPGRADE_BUILDING_RPL;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_UPGRADE_BUILDING_T::RST_OK;
			RplCmd.nAutoID		= pUpgradeBuilding->nAutoID;
			RplCmd.nExcelID		= 0;
			RplCmd.nAddedCharDev= 0;
			RplCmd.nAddedCharExp= 0;
			RplCmd.nCurCharExp	= 0;
			RplCmd.nCurCharLevel= 0;
			RplCmd.nAccountID	= pUpgradeBuilding->nAccountID;
			memcpy(RplCmd.nExt, pUpgradeBuilding->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"upgrade_building,DBERROR,0x%"WHINT64PRFX"X,%d", pUpgradeBuilding->nAccountID, pUpgradeBuilding->nAutoID);

				RplCmd.nRst			= STC_GAMECMD_OPERATE_UPGRADE_BUILDING_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL upgrade_building(0x%"WHINT64PRFX"X, %d)", pUpgradeBuilding->nAccountID, pUpgradeBuilding->nAutoID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"upgrade_building,SQLERROR,0x%"WHINT64PRFX"X,%d", pUpgradeBuilding->nAccountID, pUpgradeBuilding->nAutoID);

					RplCmd.nRst			= STC_GAMECMD_OPERATE_UPGRADE_BUILDING_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nExcelID		= q.GetVal_32();
						RplCmd.nAddedCharExp= q.GetVal_32();
						RplCmd.nAddedCharDev= q.GetVal_32();
						RplCmd.nCurCharExp	= q.GetVal_32();
						RplCmd.nCurCharLevel= q.GetVal_32();
						RplCmd.nGoldCost	= q.GetVal_32();
						RplCmd.nPopulationCost	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst			= STC_GAMECMD_OPERATE_UPGRADE_BUILDING_T::RST_SQL_ERR;
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"upgrade_building,%d,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d,%d,%d", 
						(int)RplCmd.nRst, pUpgradeBuilding->nAccountID, pUpgradeBuilding->nAutoID, RplCmd.nExcelID, RplCmd.nAddedCharExp, RplCmd.nAddedCharDev, RplCmd.nCurCharExp, RplCmd.nCurCharLevel, RplCmd.nGoldCost, RplCmd.nPopulationCost);
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_ONCE_PERPERSON_TE_REQ:
		{
			P_DBS4WEB_ADD_ONCE_PERPERSON_TE_T*	pOnceTE	= (P_DBS4WEB_ADD_ONCE_PERPERSON_TE_T*)pCmd;

			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_once_perperson_time_event,DBERROR,0x%"WHINT64PRFX"X,%d", pOnceTE->nAccountID, pOnceTE->nType);
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_once_perperson_time_event(0x%"WHINT64PRFX"X, %d)", pOnceTE->nAccountID, pOnceTE->nType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_once_perperson_time_event,SQLERROR,0x%"WHINT64PRFX"X,%d", pOnceTE->nAccountID, pOnceTE->nType);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_account_id,_event_id,_type
						int			nResult		= q.GetVal_32();
						tty_id_t	nAccountID	= q.GetVal_64();
						tty_id_t	nEventID	= q.GetVal_64();
						int			nType		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_once_perperson_time_event,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", nResult, nAccountID, nEventID, nType);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_once_perperson_time_event,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d", pOnceTE->nAccountID, pOnceTE->nType);
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_DEAL_PRODUCTION_TE_REQ:
		{
			P_DBS4WEB_DEAL_PRODUCTION_TE_T*	pDealTE	= (P_DBS4WEB_DEAL_PRODUCTION_TE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL deal_production_event(0x%"WHINT64PRFX"X)", pDealTE->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_production_event,SQLERROR,%d,0x%"WHINT64PRFX"X", nPreRet, pDealTE->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_account_id,_auto_id,_production,_type
						int			nResult		= q.GetVal_32();
						tty_id_t	nAccountID	= q.GetVal_64();
						int			nAutoID		= q.GetVal_32();
						int			nProduction	= q.GetVal_32();
						int			nType		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal_production_event,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pDealTE->nEventID, nResult, nAccountID, nAutoID, nProduction, nType);

						if (nResult == 0)
						{
							P_DBS4WEBUSER_DEAL_PRODUCTION_TE_T	Rst;
							Rst.nCmd		= P_DBS4WEB_RPL_CMD;
							Rst.nSubCmd		= CMDID_DEAL_PRODUCTION_TE_RPL;
							Rst.nResult		= nResult;
							Rst.nAccountID	= nAccountID;
							Rst.nAutoID		= nAutoID;
							Rst.nProduction	= nProduction;
							Rst.nType		= nType;

							SendMsg_MT(pCDCmd->nConnecterID, &Rst, sizeof(Rst));
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_production_event,FOUND_COUNT()=0,0x%"WHINT64PRFX"X", pDealTE->nEventID);
					}
				}
			}
		}
		break;
	case CMDID_ADD_PRODUCTION_TE_REQ:
		{
			P_DBS4WEB_ADD_PRODUCTION_TE_T*	pProductionTE	= (P_DBS4WEB_ADD_PRODUCTION_TE_T*)pCmd;
			P_DBS4WEBUSER_ADD_PRODUCTION_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_PRODUCTION_TE_RPL;
			RplCmd.nAutoID		= pProductionTE->nAutoID;
			RplCmd.nProduction	= 0;
			RplCmd.nType		= pProductionTE->nType;
			RplCmd.nResult		= STC_GAMECMD_OPERATE_PRODUCTION_T::RST_OK;
			memcpy(RplCmd.nExt, pProductionTE->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_production_time_event,DBERROR,0x%"WHINT64PRFX"X,%d,%d,%d", pProductionTE->nAccountID, pProductionTE->nType, pProductionTE->nAutoID, pProductionTE->nProductionChoice);
				RplCmd.nResult	= STC_GAMECMD_OPERATE_PRODUCTION_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_production_time_event(0x%"WHINT64PRFX"X, %d, %d, %d)", pProductionTE->nAccountID, pProductionTE->nAutoID, pProductionTE->nProductionChoice, pProductionTE->nType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_production_time_event,SQLERROR,0x%"WHINT64PRFX"X,%d,%d,%d", pProductionTE->nAccountID, pProductionTE->nType, pProductionTE->nAutoID, pProductionTE->nProductionChoice);
					RplCmd.nResult	= STC_GAMECMD_OPERATE_PRODUCTION_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_production,_event_id
						int			nResult		= q.GetVal_32();
						int			nProduction	= q.GetVal_32();
						tty_id_t	nEventID	= q.GetVal_64();
						RplCmd.nResult			= nResult;
						RplCmd.nProduction		= nProduction;
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_production_time_event,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,0x%"WHINT64PRFX"X", pProductionTE->nAccountID, pProductionTE->nType, pProductionTE->nAutoID, pProductionTE->nProductionChoice, nResult, nProduction, nEventID);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_production_time_event,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d,%d", pProductionTE->nAccountID, pProductionTE->nType, pProductionTE->nAccountID, pProductionTE->nProductionChoice);
						RplCmd.nResult	= STC_GAMECMD_OPERATE_PRODUCTION_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_FETCH_PRODUCTION_REQ:
		{
			P_DBS4WEB_FETCH_PRODUCTION_T*	pFetch	= (P_DBS4WEB_FETCH_PRODUCTION_T*)pCmd;
			P_DBS4WEBUSER_FETCH_PRODUCTION_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_FETCH_PRODUCTION_RPL;
			RplCmd.nAccountID	= pFetch->nAccountID;
			RplCmd.nAutoID		= pFetch->nAutoID;
			RplCmd.nProduction	= 0;
			RplCmd.nType		= pFetch->nType;
			RplCmd.nResult		= STC_GAMECMD_FETCH_PRODUCTION_T::RST_OK;
			memcpy(RplCmd.nExt, pFetch->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_production,DBERROR,0x%"WHINT64PRFX"X,%d,%d", pFetch->nAccountID, pFetch->nType, pFetch->nAutoID);
				RplCmd.nResult	= STC_GAMECMD_FETCH_PRODUCTION_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL fetch_production(0x%"WHINT64PRFX"X, %d, %d)", pFetch->nAccountID, pFetch->nAutoID, pFetch->nType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_production,SQLERROR,0x%"WHINT64PRFX"X,%d,%d", pFetch->nAccountID, pFetch->nType, pFetch->nAutoID);
					RplCmd.nResult	= STC_GAMECMD_FETCH_PRODUCTION_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_production
						int			nResult		= q.GetVal_32();
						int			nProduction	= q.GetVal_32();
						RplCmd.nResult			= nResult;
						RplCmd.nProduction		= nProduction;
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"fetch_production,0x%"WHINT64PRFX"X,%d,%d,%d,%d", pFetch->nAccountID, pFetch->nType, pFetch->nAutoID, nResult, nProduction);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_production,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d", pFetch->nAccountID, pFetch->nType, pFetch->nAutoID);
						RplCmd.nResult	= STC_GAMECMD_FETCH_PRODUCTION_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_PRODUCTION_EVENT_REQ:
		{
			P_DBS4WEB_GET_PRODUCTION_EVENT_T*	pGet	= (P_DBS4WEB_GET_PRODUCTION_EVENT_T*)pCmd;
			P_DBS4WEBUSER_GET_PRODUCTION_EVENT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_PRODUCTION_EVENT_RPL;
			RplCmd.nAccountID	= pGet->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nResult		= STC_GAMECMD_GET_PRODUCTION_EVENT_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_production_event,DBERROR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nResult	= STC_GAMECMD_GET_PRODUCTION_EVENT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT auto_id,production,type FROM production_events WHERE account_id=0x%"WHINT64PRFX"X", pGet->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_production_event,SQLERROR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nResult	= STC_GAMECMD_GET_PRODUCTION_EVENT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"get_production_event,0x%"WHINT64PRFX"X,%d", pGet->nAccountID, RplCmd.nNum);
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_PRODUCTION_EVENT_T) + RplCmd.nNum*sizeof(ProductionEvent));
						P_DBS4WEBUSER_GET_PRODUCTION_EVENT_T*	pRplCmd	= (P_DBS4WEBUSER_GET_PRODUCTION_EVENT_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						ProductionEvent*	pEvent	= (ProductionEvent*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pEvent->nAutoID			= q.GetVal_32();
							pEvent->nProduction		= q.GetVal_32();
							pEvent->nType			= q.GetVal_32();
							pEvent++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_PRODUCTION_TE_REQ:
		{
			P_DBS4WEB_GET_PRODUCTION_TE_T*	pGet	= (P_DBS4WEB_GET_PRODUCTION_TE_T*)pCmd;
			P_DBS4WEBUSER_GET_PRODUCTION_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_PRODUCTION_TE_RPL;
			RplCmd.nAccountID	= pGet->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nTimeNow		= wh_time();
			RplCmd.nResult		= STC_GAMECMD_GET_PRODUCTION_TE_LIST_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_production_te,DBERROR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nResult	= STC_GAMECMD_GET_PRODUCTION_TE_LIST_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT p.auto_id,p.production,p.type,t.begin_time,t.end_time FROM production_time_events p join time_events t on p.event_id=t.event_id WHERE account_id=0x%"WHINT64PRFX"X", pGet->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_production_te,SQLERROR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nResult	= STC_GAMECMD_GET_PRODUCTION_TE_LIST_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"get_production_te,0x%"WHINT64PRFX"X,%d", pGet->nAccountID, RplCmd.nNum);
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_PRODUCTION_TE_T) + RplCmd.nNum*sizeof(ProductionTimeEvent));
						P_DBS4WEBUSER_GET_PRODUCTION_TE_T*	pRplCmd	= (P_DBS4WEBUSER_GET_PRODUCTION_TE_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						ProductionTimeEvent*	pEvent	= (ProductionTimeEvent*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pEvent->nAutoID			= q.GetVal_32();
							pEvent->nProduction		= q.GetVal_32();
							pEvent->nType			= q.GetVal_32();
							pEvent->nBeginTime		= q.GetVal_32();
							pEvent->nEndTime		= q.GetVal_32();
							pEvent++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
					return 0;
				}
			}
		}
		break;
	case CMDID_ACCELERATE_REQ:
		{
			P_DBS4WEB_ACCELERATE_T*	pAcce	= (P_DBS4WEB_ACCELERATE_T*)pCmd;
			P_DBS4WEBUSER_ACCELERATE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ACCELERATE_RPL;
			RplCmd.nAccountID	= pAcce->nAccountID;
			RplCmd.nAutoID		= pAcce->nAutoID;
			RplCmd.nExcelID		= pAcce->nExcelID;
			RplCmd.nMoneyNum	= 0;
			RplCmd.nMoneyType	= pAcce->nMoneyType;
			RplCmd.nTime		= 0;
			RplCmd.nType		= pAcce->nType;
			RplCmd.nSubType		= pAcce->nSubType;
			RplCmd.nAcceTime	= 0;
			RplCmd.nResult		= STC_GAMECMD_OPERATE_ACCE_T::RST_OK;
			memcpy(RplCmd.nExt, pAcce->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"accelerate,DBERR,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d", pAcce->nAccountID, pAcce->nType, pAcce->nSubType, pAcce->nAutoID, pAcce->nExcelID, pAcce->nTime, pAcce->nMoneyType);
				RplCmd.nResult	= STC_GAMECMD_OPERATE_ACCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL accelerate(0x%"WHINT64PRFX"X, %d, %d, %d, %d, %d, %d)", pAcce->nAccountID, pAcce->nAutoID, pAcce->nExcelID, pAcce->nType, pAcce->nSubType, pAcce->nTime, pAcce->nMoneyType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"accelerate,SQLERR,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d", pAcce->nAccountID, pAcce->nType, pAcce->nSubType, pAcce->nAutoID, pAcce->nExcelID, pAcce->nTime, pAcce->nMoneyType);
					RplCmd.nResult	= STC_GAMECMD_OPERATE_ACCE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_time,_money_num,_excel_id,@_acce_time
						RplCmd.nResult		= q.GetVal_32();
						RplCmd.nTime		= q.GetVal_32();
						RplCmd.nMoneyNum	= q.GetVal_32();
						RplCmd.nExcelID		= q.GetVal_32();
						RplCmd.nAcceTime	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"accelerate,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d,%d,%d,%d", pAcce->nAccountID, pAcce->nType, pAcce->nSubType, pAcce->nAutoID, RplCmd.nExcelID, RplCmd.nAcceTime, RplCmd.nResult, RplCmd.nTime, RplCmd.nMoneyNum, pAcce->nMoneyType);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"accelerate,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d", pAcce->nAccountID, pAcce->nType, pAcce->nSubType, pAcce->nAutoID, pAcce->nExcelID, pAcce->nTime, pAcce->nMoneyType);
						RplCmd.nResult	= STC_GAMECMD_OPERATE_ACCE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_DEAL_RESEARCH_TE_REQ:
		{
			P_DBS4WEB_DEAL_RESEARCH_TE_T*	pDeal	= (P_DBS4WEB_DEAL_RESEARCH_TE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_research_event,DBERR,0x%"WHINT64PRFX"X", pDeal->nEventID);
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL deal_research_event(0x%"WHINT64PRFX"X)", pDeal->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_research_event,SQLERR,0x%"WHINT64PRFX"X", pDeal->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_account_id,_excel_id
						P_DBS4WEBUSER_DEAL_RESEARCH_TE_T	RplCmd;
						RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
						RplCmd.nSubCmd		= CMDID_DEAL_RESEARCH_TE_RPL;
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nAccountID	= q.GetVal_64();
						RplCmd.nExcelID		= q.GetVal_32();
						RplCmd.nLevel		= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal_research_event,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pDeal->nEventID, (int)RplCmd.nRst, RplCmd.nAccountID, RplCmd.nExcelID);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_research_event,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pDeal->nEventID);
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_ADD_RESEARCH_TE_REQ:
		{
			P_DBS4WEB_ADD_RESEARCH_TE_T*	pAdd	= (P_DBS4WEB_ADD_RESEARCH_TE_T*)pCmd;
			P_DBS4WEBUSER_ADD_RESEARCH_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_RESEARCH_TE_RPL;
			RplCmd.nAccountID	= pAdd->nAccountID;
			RplCmd.nResult		= STC_GAMECMD_OPERATE_RESEARCH_T::RST_OK;
			RplCmd.nExcelID		= pAdd->nExcelID;
			RplCmd.nGold		= 0;
			RplCmd.nLevel		= 0;
			RplCmd.nTime		= 0;
			memcpy(RplCmd.nExt, pAdd->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_research_time_event,DBERR,0x%"WHINT64PRFX"X,%d", pAdd->nAccountID, pAdd->nExcelID);
				RplCmd.nResult	= STC_GAMECMD_OPERATE_RESEARCH_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_research_time_event(0x%"WHINT64PRFX"X, %d)", pAdd->nAccountID, pAdd->nExcelID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_research_time_event,SQLERR,0x%"WHINT64PRFX"X,%d", pAdd->nAccountID, pAdd->nExcelID);
					RplCmd.nResult	= STC_GAMECMD_OPERATE_RESEARCH_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_level,_time_cost,_gold_cost,_event_id
						RplCmd.nResult	= q.GetVal_32();
						RplCmd.nLevel	= q.GetVal_32();
						RplCmd.nTime	= q.GetVal_32();
						RplCmd.nGold	= q.GetVal_32();
						tty_id_t	nEventID		= q.GetVal_64();

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_research_time_event,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,0x%"WHINT64PRFX"X"
							, pAdd->nAccountID, pAdd->nExcelID, (int)RplCmd.nResult, RplCmd.nLevel, RplCmd.nTime, RplCmd.nGold, nEventID);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_research_time_event,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d", pAdd->nAccountID, pAdd->nExcelID);
						RplCmd.nResult	= STC_GAMECMD_OPERATE_RESEARCH_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_TECHNOLOGY_REQ:
		{
			P_DBS4WEB_GET_TECHNOLOGY_T*	pGet	= (P_DBS4WEB_GET_TECHNOLOGY_T*)pCmd;
			P_DBS4WEBUSER_GET_TECHNOLOGY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_TECHNOLOGY_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_TECHNOLOGY_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_technology,DBERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nRst			= STC_GAMECMD_GET_TECHNOLOGY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT excel_id,level FROM technologys WHERE account_id=0x%"WHINT64PRFX"X", pGet->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_technology,DBERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nRst			= STC_GAMECMD_GET_TECHNOLOGY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get_technology,0x%"WHINT64PRFX"X,%d", pGet->nAccountID, RplCmd.nNum);
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_TECHNOLOGY_T) + RplCmd.nNum*sizeof(TechnologyUnit));
						P_DBS4WEBUSER_GET_TECHNOLOGY_T*	pRplCmd	= (P_DBS4WEBUSER_GET_TECHNOLOGY_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						TechnologyUnit*	pTech	= (TechnologyUnit*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pTech->nExcelID		= q.GetVal_32();
							pTech->nLevel		= q.GetVal_32();
							pTech++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_RESEARCH_TE_REQ:
		{
			P_DBS4WEB_GET_RESEARCH_TE_T*	pGet	= (P_DBS4WEB_GET_RESEARCH_TE_T*)pCmd;
			P_DBS4WEBUSER_GET_RESEARCH_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_RESEARCH_TE_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nTimeNow		= wh_time();
			RplCmd.nRst			= STC_GAMECMD_GET_TECHNOLOGY_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_research_te,DBERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nRst			= STC_GAMECMD_GET_TECHNOLOGY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT excel_id,begin_time,end_time FROM research_time_events r join time_events t on r.event_id=t.event_id WHERE r.account_id=0x%"WHINT64PRFX"X", pGet->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_research_te,SQLERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nRst			= STC_GAMECMD_GET_TECHNOLOGY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get_research_te,0x%"WHINT64PRFX"X,%d", pGet->nAccountID, RplCmd.nNum);
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_RESEARCH_TE_T) + RplCmd.nNum*sizeof(ResearchTimeEvent));
						P_DBS4WEBUSER_GET_RESEARCH_TE_T*	pRplCmd	= (P_DBS4WEBUSER_GET_RESEARCH_TE_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						ResearchTimeEvent*	pEvent	= (ResearchTimeEvent*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pEvent->nExcelID		= q.GetVal_32();
							pEvent->nBeginTime		= q.GetVal_32();
							pEvent->nEndTime		= q.GetVal_32();
							pEvent++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_CONSCRIPT_SOLDIER_REQ:
		{
			P_DBS4WEB_CONSCRIPT_SOLDIER_T*	pConscript	= (P_DBS4WEB_CONSCRIPT_SOLDIER_T*)pCmd;
			P_DBS4WEBUSER_CONSCRIPT_SOLDIER_T RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CONSCRIPT_SOLDIER_RPL;
			RplCmd.nAccountID	= pConscript->nAccountID;
			RplCmd.nExcelID		= pConscript->nExcelID;
			RplCmd.nLevel		= pConscript->nLevel;
			RplCmd.nNum			= pConscript->nNum;
			RplCmd.nGold		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T::RST_OK;
			memcpy(RplCmd.nExt, pConscript->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"conscript_soldier,DBERR,0x%"WHINT64PRFX"X,%d,%d,%d"
					, pConscript->nAccountID, pConscript->nExcelID, pConscript->nLevel, pConscript->nNum);
				RplCmd.nRst	= STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL conscript_soldier(0x%"WHINT64PRFX"X, %d, %d, %d)", pConscript->nAccountID, pConscript->nExcelID, pConscript->nLevel, pConscript->nNum);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"conscript_soldier,SQLERR,0x%"WHINT64PRFX"X,%d,%d,%d"
						, pConscript->nAccountID, pConscript->nExcelID, pConscript->nLevel, pConscript->nNum);
					RplCmd.nRst	= STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_gold;
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nGold	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"conscript_soldier,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d"
							, pConscript->nAccountID, pConscript->nExcelID, pConscript->nLevel, pConscript->nNum, RplCmd.nGold, RplCmd.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"conscript_soldier,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pConscript->nAccountID, pConscript->nExcelID, pConscript->nLevel, pConscript->nNum);
						RplCmd.nRst	= STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_UPGRADE_SOLDIER_REQ:
		{
			P_DBS4WEB_UPGRADE_SOLDIER_T*	pUpgrade	= (P_DBS4WEB_UPGRADE_SOLDIER_T*)pCmd;
			P_DBS4WEBUSER_UPGRADE_SOLDIER_T RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_UPGRADE_SOLDIER_RPL;
			RplCmd.nAccountID	= pUpgrade->nAccountID;
			RplCmd.nExcelID		= pUpgrade->nExcelID;
			RplCmd.nFromLevel	= pUpgrade->nFromLevel;
			RplCmd.nToLevel		= pUpgrade->nToLevel;
			RplCmd.nNum			= pUpgrade->nNum;
			RplCmd.nGold		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CONSCRIPT_SOLDIER_T::RST_OK;
			memcpy(RplCmd.nExt, pUpgrade->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"upgrade_soldier,DBERR,0x%"WHINT64PRFX"X,%d,%d,%d,%d"
					, pUpgrade->nAccountID, pUpgrade->nExcelID, pUpgrade->nFromLevel, pUpgrade->nToLevel, pUpgrade->nNum);
				RplCmd.nRst	= STC_GAMECMD_OPERATE_UPGRADE_SOLDIER_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL upgrade_soldier(0x%"WHINT64PRFX"X, %d, %d, %d, %d)", pUpgrade->nAccountID, pUpgrade->nExcelID, pUpgrade->nFromLevel, pUpgrade->nToLevel, pUpgrade->nNum);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"upgrade_soldier,SQLERR,0x%"WHINT64PRFX"X,%d,%d,%d,%d"
						, pUpgrade->nAccountID, pUpgrade->nExcelID, pUpgrade->nFromLevel, pUpgrade->nToLevel, pUpgrade->nNum);
					RplCmd.nRst	= STC_GAMECMD_OPERATE_UPGRADE_SOLDIER_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_gold;
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nGold	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"upgrade_soldier,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d,%d"
							, pUpgrade->nAccountID, pUpgrade->nExcelID, pUpgrade->nFromLevel, pUpgrade->nToLevel, pUpgrade->nNum, RplCmd.nGold, RplCmd.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"upgrade_soldier,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d,%d,%d"
							, pUpgrade->nAccountID, pUpgrade->nExcelID, pUpgrade->nFromLevel, pUpgrade->nToLevel, pUpgrade->nNum);
						RplCmd.nRst	= STC_GAMECMD_OPERATE_UPGRADE_SOLDIER_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_SOLDIER_REQ:
		{
			P_DBS4WEB_GET_SOLDIER_T*	pGet	= (P_DBS4WEB_GET_SOLDIER_T*)pCmd;
			P_DBS4WEBUSER_GET_SOLDIER_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_SOLDIER_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_SOLDIER_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_soldier,DBERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nRst			= STC_GAMECMD_GET_SOLDIER_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT excel_id,level,num FROM soldiers WHERE account_id=0x%"WHINT64PRFX"X", pGet->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_soldier,SQLERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nRst			= STC_GAMECMD_GET_SOLDIER_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get_soldier,0x%"WHINT64PRFX"X,%d", pGet->nAccountID, RplCmd.nNum);
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_SOLDIER_T) + RplCmd.nNum*sizeof(SoldierUnit));
						P_DBS4WEBUSER_GET_SOLDIER_T*	pRplCmd	= (P_DBS4WEBUSER_GET_SOLDIER_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						SoldierUnit*	pUnit	= (SoldierUnit*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nExcelID		= q.GetVal_32();
							pUnit->nLevel		= q.GetVal_32();
							pUnit->nNum			= q.GetVal_32();
							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_ACCEPT_ALLIANCE_MEMBER_REQ:
		{
			P_DBS4WEB_ACCEPT_ALLIANCE_MEMBER_T*	pAcce	= (P_DBS4WEB_ACCEPT_ALLIANCE_MEMBER_T*)pCmd;
			P_DBS4WEBUSER_ACCEPT_ALLIANCE_MEMBER_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ACCEPT_ALLIANCE_MEMBER_RPL;
			RplCmd.nNewMemberID	= pAcce->nNewMemberID;
			RplCmd.nAllianceID	= pAcce->nAllianceID;
			RplCmd.nAccountID	= pAcce->nManagerID;
			RplCmd.nDevelopment	= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T::RST_OK;
			memcpy(RplCmd.nExt, pAcce->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"accept_alliance_member,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pAcce->nManagerID, pAcce->nAllianceID, pAcce->nNewMemberID);
				RplCmd.nRst			= STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL accept_alliance_member(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pAcce->nManagerID, pAcce->nAllianceID, pAcce->nNewMemberID);
				int	nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"accept_alliance_member,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pAcce->nManagerID, pAcce->nAllianceID, pAcce->nNewMemberID);
					RplCmd.nRst			= STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nDevelopment	= q.GetVal_32();	// 增加的发展度
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"accept_alliance_member,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pAcce->nManagerID, pAcce->nAllianceID, pAcce->nNewMemberID, RplCmd.nRst, RplCmd.nDevelopment);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"accept_alliance_member,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pAcce->nManagerID, pAcce->nAllianceID, pAcce->nNewMemberID);
						RplCmd.nRst			= STC_GAMECMD_OPERATE_ACCEPT_ALLI_MEMBER_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_BUILD_ALLIANCE_BUILDING_REQ:
		{
			P_DBS4WEB_BUILD_ALLIANCE_BUILDING_T*	pBuild	= (P_DBS4WEB_BUILD_ALLIANCE_BUILDING_T*)pCmd;
			P_DBS4WEBUSER_BUILD_ALLIANCE_BUILDING_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_BUILD_ALLIANCE_BUILDING_RPL;
			RplCmd.nExcelID		= pBuild->nExcelID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T::RST_OK;
			RplCmd.nAllianceID	= 0;
			memcpy(RplCmd.nExt, pBuild->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"build_alliance_building,DBERR,0x%"WHINT64PRFX"X,%d", pBuild->nAccountID, pBuild->nExcelID);
				RplCmd.nRst			= STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL build_alliance_building(0x%"WHINT64PRFX"X, %d)", pBuild->nAccountID, pBuild->nExcelID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"build_alliance_building,SQLERR,0x%"WHINT64PRFX"X,%d", pBuild->nAccountID, pBuild->nExcelID);
					RplCmd.nRst			= STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nDevelopment	= q.GetVal_32();
						RplCmd.nTime		= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						RplCmd.nType		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"build_alliance_building,0x%"WHINT64PRFX"X,%d,%d,%d,%d,0x%"WHINT64PRFX"X,%d"
							, pBuild->nAccountID, pBuild->nExcelID, RplCmd.nRst, RplCmd.nDevelopment, RplCmd.nTime, RplCmd.nAllianceID, RplCmd.nType);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"build_alliance_building,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d", pBuild->nAccountID, pBuild->nExcelID);
						RplCmd.nRst			= STC_GAMECMD_OPERATE_BUILD_ALLI_BUILDING_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_CANCEL_JOIN_ALLIANCE_REQ:
		{
			P_DBS4WEB_CANCEL_JOIN_ALLIANCE_T*	pCancel	= (P_DBS4WEB_CANCEL_JOIN_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_CANCEL_JOIN_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CANCEL_JOIN_ALLIANCE_RPL;
			RplCmd.nAllianceID	= 0;
			RplCmd.nAccountID	= pCancel->nAccountID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T::RST_OK;
			memcpy(RplCmd.nExt, pCancel->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"cancel_join_alliance,DBERR,0x%"WHINT64PRFX"X", pCancel->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL cancel_join_alliance(0x%"WHINT64PRFX"X)", pCancel->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"cancel_join_alliance,SQLERR,0x%"WHINT64PRFX"X", pCancel->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst	= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"cancel_join_alliance,0x%"WHINT64PRFX"X,%d", pCancel->nAccountID, (int)RplCmd.nRst);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"cancel_join_alliance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pCancel->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_JOIN_ALLI_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_CREATE_ALLIANCE_REQ:
		{
			P_DBS4WEB_CREATE_ALLIANCE_T*	pCreate	= (P_DBS4WEB_CREATE_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_CREATE_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CREATE_ALLIANCE_RPL;
			RplCmd.nAllianceID	= 0;
			RplCmd.nGold		= 0;
			WH_STRNCPY0(RplCmd.szAllianceName, pCreate->szAllianceName);
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CREATE_ALLI_T::RST_OK;
			memcpy(RplCmd.nExt, pCreate->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"create_alliance,DBERR,0x%"WHINT64PRFX"X,%s", pCreate->nAccountID, pCreate->szAllianceName);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_CREATE_ALLI_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL create_alliance(0x%"WHINT64PRFX"X, '%s')", pCreate->nAccountID, pCreate->szAllianceName);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"create_alliance,SQLERR,0x%"WHINT64PRFX"X,%s", pCreate->nAccountID, pCreate->szAllianceName);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_CREATE_ALLI_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nGold	= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"create_alliance,0x%"WHINT64PRFX"X,%s,%d,%d,0x%"WHINT64PRFX"X"
							, pCreate->nAccountID, pCreate->szAllianceName, (int)RplCmd.nRst, RplCmd.nGold, RplCmd.nAllianceID);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"create_alliance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%s", pCreate->nAccountID, pCreate->szAllianceName);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_CREATE_ALLI_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_DEAL_ALLIANCE_BUILDING_TE_REQ:
		{
			P_DBS4WEB_DEAL_ALLIANCE_BUILDING_TE_T*	pDeal	= (P_DBS4WEB_DEAL_ALLIANCE_BUILDING_TE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				// 无所谓
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL deal_alliance_building_event(0x%"WHINT64PRFX"X)", pDeal->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_alliance_building_event,SQLERR,%d,0x%"WHINT64PRFX"X", nPreRet, pDeal->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						P_DBS4WEBUSER_DEAL_ALLIANCE_BUILDING_TE_T	RplCmd;
						RplCmd.nCmd		= P_DBS4WEB_RPL_CMD;
						RplCmd.nSubCmd	= CMDID_DEAL_ALLIANCE_BUILDING_TE_RPL;
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						RplCmd.nExcelID		= q.GetVal_32();
						RplCmd.nType		= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal_alliance_building_event,%d,0x%"WHINT64PRFX"X,%d,%d", (int)RplCmd.nRst, RplCmd.nAllianceID, RplCmd.nExcelID, RplCmd.nType);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_alliance_building_event,SQLERR,NO RESULT,%d,0x%"WHINT64PRFX"X", nPreRet, pDeal->nEventID);
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_DISMISS_ALLIANCE_REQ:
		{
			P_DBS4WEB_DISMISS_ALLIANCE_T*	pDismiss	= (P_DBS4WEB_DISMISS_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_DISMISS_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_DISMISS_ALLIANCE_RPL;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_DISMISS_ALLI_T::RST_OK;
			RplCmd.nAllianceID	= 0;
			memcpy(RplCmd.nExt, pDismiss->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"dismiss_alliance,DBERR,0x%"WHINT64PRFX"X", pDismiss->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_DISMISS_ALLI_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL dismiss_alliance(0x%"WHINT64PRFX"X)", pDismiss->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"dismiss_alliance,SQLERR,0x%"WHINT64PRFX"X", pDismiss->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_DISMISS_ALLI_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"dismiss_alliance,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X", pDismiss->nAccountID, (int)RplCmd.nRst, RplCmd.nAllianceID);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"dismiss_alliance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pDismiss->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_DISMISS_ALLI_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_EXIT_ALLIANCE_REQ:
		{
			P_DBS4WEB_EXIT_ALLIANCE_T*	pExit	= (P_DBS4WEB_EXIT_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_EXIT_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_EXIT_ALLIANCE_RPL;
			RplCmd.nAccountID	= pExit->nAccountID;
			RplCmd.nAllianceID	= 0;
			RplCmd.nDevelopment	= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_EXIT_ALLI_T::RST_OK;
			memcpy(RplCmd.nExt, pExit->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"exit_alliance,DBERR,0x%"WHINT64PRFX"X", pExit->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_EXIT_ALLI_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL exit_alliance(0x%"WHINT64PRFX"X)", pExit->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"exit_alliance,SQLERR,0x%"WHINT64PRFX"X", pExit->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_EXIT_ALLI_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						RplCmd.nDevelopment	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"exit_alliance,SQLERR,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pExit->nAccountID, (int)RplCmd.nRst, RplCmd.nAllianceID, RplCmd.nDevelopment);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"exit_alliance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pExit->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_EXIT_ALLI_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_EXPEL_ALLIANCE_MEMBER_REQ:
		{
			P_DBS4WEB_EXPEL_ALLIANCE_MEMBER_T*	pExpel	= (P_DBS4WEB_EXPEL_ALLIANCE_MEMBER_T*)pCmd;
			P_DBS4WEBUSER_EXPEL_ALLIANCE_MEMBER_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_EXPEL_ALLIANCE_MEMBER_RPL;
			RplCmd.nAllianceID	= pExpel->nAllianceID;
			RplCmd.nManagerID	= pExpel->nManagerID;
			RplCmd.nMemberID	= pExpel->nMemberID;
			RplCmd.nDevelopment	= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T::RST_OK;
			memcpy(RplCmd.nExt, pExpel->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"expel_alliance_member,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
					, pExpel->nAllianceID, pExpel->nManagerID, pExpel->nMemberID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL expel_alliance_member(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)"
					, pExpel->nManagerID, pExpel->nAllianceID, pExpel->nMemberID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"expel_alliance_member,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
						, pExpel->nAllianceID, pExpel->nManagerID, pExpel->nMemberID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nDevelopment	= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"expel_alliance_member,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d"
							, pExpel->nAllianceID, pExpel->nManagerID, pExpel->nMemberID, (int)RplCmd.nRst, RplCmd.nDevelopment);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"expel_alliance_member,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
							, pExpel->nAllianceID, pExpel->nManagerID, pExpel->nMemberID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_EXPEL_ALLI_MEMBER_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_JOIN_ALLIANCE_REQ:
		{
			P_DBS4WEB_JOIN_ALLIANCE_T*	pJoin	= (P_DBS4WEB_JOIN_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_JOIN_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_JOIN_ALLIANCE_RPL;
			RplCmd.nAllianceID	= pJoin->nAllianceID;
			RplCmd.nAccountID	= pJoin->nAccountID;
			RplCmd.nLevel		= pJoin->nLevel;
			RplCmd.nHeadID		= pJoin->nHeadID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_JOIN_ALLI_T::RST_OK;
			WH_STRNCPY0(RplCmd.szName, pJoin->szName);
			memcpy(RplCmd.nExt, pJoin->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join_alliance,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
					, pJoin->nAllianceID, pJoin->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_JOIN_ALLI_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL join_alliance(0x%"WHINT64PRFX"X, 0x%"WHINT64PRFX"X)", pJoin->nAccountID, pJoin->nAllianceID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join_alliance,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
						, pJoin->nAllianceID, pJoin->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_JOIN_ALLI_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst	= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"join_alliance,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
							, pJoin->nAllianceID, pJoin->nAccountID, (int)RplCmd.nRst);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join_alliance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
							, pJoin->nAllianceID, pJoin->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_JOIN_ALLI_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_SET_ALLIANCE_POSITION_REQ:
		{
			P_DBS4WEB_SET_ALLIANCE_POSITION_T*	pSet	= (P_DBS4WEB_SET_ALLIANCE_POSITION_T*)pCmd;
			P_DBS4WEBUSER_SET_ALLIANCE_POSITION_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_SET_ALLIANCE_POSITION_RPL;
			RplCmd.nAllianceID	= pSet->nAllianceID;
			RplCmd.nManagerID	= pSet->nManagerID;
			RplCmd.nMemberID	= pSet->nMemberID;
			RplCmd.nPosition	= pSet->nPosition;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_SET_ALLI_POSITION_T::RST_OK;
			memcpy(RplCmd.nExt, pSet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_alliance_position,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
					, pSet->nAllianceID, pSet->nManagerID, pSet->nMemberID, pSet->nPosition);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_POSITION_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL set_alliance_position(0x%"WHINT64PRFX"X, 0x%"WHINT64PRFX"X, 0x%"WHINT64PRFX"X, %d)", pSet->nManagerID, pSet->nAllianceID, pSet->nMemberID, pSet->nPosition);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_alliance_position,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
						, pSet->nAllianceID, pSet->nManagerID, pSet->nMemberID, pSet->nPosition);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_POSITION_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst	= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_alliance_position,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d"
							, pSet->nAllianceID, pSet->nManagerID, pSet->nMemberID, pSet->nPosition, (int)RplCmd.nRst);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_alliance_position,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
							, pSet->nAllianceID, pSet->nManagerID, pSet->nMemberID, pSet->nPosition);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_POSITION_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_INFO_REQ:
		{
			P_DBS4WEB_GET_ALLIANCE_INFO_T*	pGet	= (P_DBS4WEB_GET_ALLIANCE_INFO_T*)pCmd;
			P_DBS4WEBUSER_GET_ALLIANCE_INFO_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_ALLIANCE_INFO_RPL;
			RplCmd.nAllianceID	= pGet->nAllianceID;
			RplCmd.nRst			= STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_info,DBERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
				RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT a.name as alliance_name,c.name as leader_name,a.account_id,a.development,a.total_development,a.introduction FROM alliances a JOIN common_characters c ON a.account_id=c.account_id WHERE a.alliance_id=0x%"WHINT64PRFX"X", pGet->nAllianceID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_info,SQLERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
					RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_ALLIANCE_INFO_T) + sizeof(AllianceUnit));
						P_DBS4WEBUSER_GET_ALLIANCE_INFO_T*	pRplCmd	= (P_DBS4WEBUSER_GET_ALLIANCE_INFO_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						AllianceUnit*	pAlliance	= (AllianceUnit*)wh_getptrnexttoptr(pRplCmd);
						pAlliance->nAllianceID		= pGet->nAllianceID;
						WH_STRNCPY0(pAlliance->szAllianceName, q.GetStr());
						WH_STRNCPY0(pAlliance->szLeaderName, q.GetStr());
						pAlliance->nLeaderID		= q.GetVal_64();
						pAlliance->nDevelopment		= q.GetVal_32();
						pAlliance->nTotalDevelopment	= q.GetVal_32();
						WH_STRNCPY0(pAlliance->szIntroduction, q.GetStr());
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_INFO_T::RST_NO_ALLI;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_MEMBER_REQ:
		{
			P_DBS4WEB_GET_ALLIANCE_MEMBER_T*	pGet	= (P_DBS4WEB_GET_ALLIANCE_MEMBER_T*)pCmd;
			P_DBS4WEBUSER_GET_ALLIANCE_MEMBER_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_ALLIANCE_MEMBER_RPL;
			RplCmd.nAllianceID	= pGet->nAllianceID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_ALLIANCE_MEMBER_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_member,DBERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
				RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_MEMBER_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT m.account_id,m.position,m.development,m.total_development,c.name,c.level,c.cup FROM alliance_members m JOIN common_characters c ON m.account_id=c.account_id WHERE m.alliance_id=0x%"WHINT64PRFX"X", pGet->nAllianceID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_member,SQLERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
					RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_MEMBER_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_ALLIANCE_MEMBER_T) + RplCmd.nNum*sizeof(AllianceMemberUnit));
					P_DBS4WEBUSER_GET_ALLIANCE_MEMBER_T*	pRplCmd	= (P_DBS4WEBUSER_GET_ALLIANCE_MEMBER_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					AllianceMemberUnit*	pUnit	= (AllianceMemberUnit*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pUnit->nAccountID		= q.GetVal_64();
						pUnit->nPosition		= q.GetVal_32();
						pUnit->nDevelopment		= q.GetVal_32();
						pUnit->nTotalDevelopment= q.GetVal_32();
						WH_STRNCPY0(pUnit->szName, q.GetStr());
						pUnit->nLevel			= q.GetVal_32();
						pUnit->nCup				= q.GetVal_32();

						pUnit++;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_BUILDING_TE_REQ:
		{
			P_DBS4WEB_GET_ALLIANCE_BUILDING_TE_T*	pGet	= (P_DBS4WEB_GET_ALLIANCE_BUILDING_TE_T*)pCmd;
			P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_ALLIANCE_BUILDING_TE_RPL;
			RplCmd.nAllianceID	= pGet->nAllianceID;
			RplCmd.nTimeNow		= wh_time();
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_building_te,DBERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
				RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT d.excel_id,d.type,t.begin_time,t.end_time FROM alliance_building_time_events d JOIN time_events t ON d.event_id=t.event_id WHERE d.alliance_id=0x%"WHINT64PRFX"X", pGet->nAllianceID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_building_te,SQLERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
					RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_BUILDING_TE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_TE_T) + RplCmd.nNum*sizeof(AllianceBuildingTimeEvent));
					P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_TE_T*	pRplCmd	= (P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_TE_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					AllianceBuildingTimeEvent*	pEvent	= (AllianceBuildingTimeEvent*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pEvent->nExcelID	= q.GetVal_32();
						pEvent->nType		= q.GetVal_32();
						pEvent->nBeginTime	= q.GetVal_32();
						pEvent->nEndTime	= q.GetVal_32();

						pEvent++;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_BUILDING_REQ:
		{
			P_DBS4WEB_GET_ALLIANCE_BUILDING_T*	pGet	= (P_DBS4WEB_GET_ALLIANCE_BUILDING_T*)pCmd;
			P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_ALLIANCE_BUILDING_RPL;
			RplCmd.nAllianceID	= pGet->nAllianceID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_ALLIANCE_BUILDING_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_building,DBERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
				RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_BUILDING_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT excel_id,level FROM alliance_buildings WHERE alliance_id=0x%"WHINT64PRFX"X", pGet->nAllianceID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_building,SQLERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
					RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_BUILDING_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_T) + RplCmd.nNum*sizeof(AllianceBuildingUnit));
					P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_T*	pRplCmd	= (P_DBS4WEBUSER_GET_ALLIANCE_BUILDING_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					AllianceBuildingUnit*	pUnit	= (AllianceBuildingUnit*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pUnit->nExcelID	= q.GetVal_32();
						pUnit->nLevel	= q.GetVal_32();

						pUnit++;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_JOIN_EVENT_REQ:
		{
			P_DBS4WEB_GET_ALLIANCE_JOIN_EVENT_T*	pGet	= (P_DBS4WEB_GET_ALLIANCE_JOIN_EVENT_T*)pCmd;
			P_DBS4WEBUSER_GET_ALLIANCE_JOIN_EVENT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_ALLIANCE_JOIN_EVENT_RPL;
			RplCmd.nAllianceID	= pGet->nAllianceID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_join_event,DBERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
				RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT c.account_id,c.name,c.level,c.head_id FROM common_characters c JOIN alliance_join_events a ON c.account_id=a.account_id WHERE a.alliance_id=0x%"WHINT64PRFX"X", pGet->nAllianceID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_join_event,SQLERR,0x%"WHINT64PRFX"X", pGet->nAllianceID);
					RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_JOIN_EVENT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_ALLIANCE_JOIN_EVENT_T) + RplCmd.nNum*sizeof(AllianceJoinEvent));
					P_DBS4WEBUSER_GET_ALLIANCE_JOIN_EVENT_T*	pRplCmd	= (P_DBS4WEBUSER_GET_ALLIANCE_JOIN_EVENT_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					AllianceJoinEvent*	pEvent	= (AllianceJoinEvent*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pEvent->nAccountID		= q.GetVal_64();
						WH_STRNCPY0(pEvent->szName, q.GetStr());
						pEvent->nLevel			= q.GetVal_32();
						pEvent->nHeadID			= q.GetVal_32();

						pEvent++;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_PRIVATE_MAIL_REQ:
		{
			P_DBS4WEB_ADD_PRIVATE_MAIL_T*	pAdd	= (P_DBS4WEB_ADD_PRIVATE_MAIL_T*)pCmd;
			pAdd->szText[pAdd->nTextLen-1]			= 0;
			int*	pExtDataLen						= (int*)wh_getoffsetaddr(pAdd, wh_offsetof(P_DBS4WEB_ADD_PRIVATE_MAIL_T, szText) + pAdd->nTextLen);
			void*	pExtData						= wh_getptrnexttoptr(pExtDataLen);

			P_DBS4WEBUSER_ADD_PRIVATE_MAIL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_PRIVATE_MAIL_RPL;
			RplCmd.bNeedRst		= pAdd->bNeedRst;
			RplCmd.nRst			= STC_GAMECMD_ADD_MAIL_T::RST_OK;
			memcpy(RplCmd.nExt, pAdd->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_private_mail,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pAdd->nSenderID, pAdd->nReceiverID, (int)pAdd->nFlag);
				RplCmd.nRst		= STC_GAMECMD_ADD_MAIL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_private_mail(0x%"WHINT64PRFX"X, 0x%"WHINT64PRFX"X, %d, %d, %d, '", pAdd->nSenderID, pAdd->nReceiverID, (int)pAdd->nType, (int)pAdd->nFlag, (int)pAdd->bSendLimit);
				q.BinaryToString(pAdd->nTextLen, pAdd->szText);
				if (*pExtDataLen == 0)
				{
					q.StrMove("',NULL,");
				}
				else
				{
					q.StrMove("','");
					q.BinaryToString(*pExtDataLen, (const char*)pExtData);
					q.StrMove("',");
				}
				int		nEcho	= 1;
				q.StrMove("%d,%s,%s", nEcho, "@result", "@mail_id");
				q.StrMove(")");
				
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_private_mail,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pAdd->nSenderID, pAdd->nReceiverID, (int)pAdd->nFlag);
					RplCmd.nRst		= STC_GAMECMD_ADD_MAIL_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						int	nMailID		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_private_mail,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d", pAdd->nSenderID, pAdd->nReceiverID, (int)pAdd->nFlag, nMailID, (int)RplCmd.nRst);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						if (RplCmd.nRst == STC_GAMECMD_ADD_MAIL_T::RST_OK)
						{
							// 有新邮件到来,请注意查收！
							int	nMsgUnitLen			= wh_offsetof(PrivateMailUnit, szText) + pAdd->nTextLen + sizeof(int) + *pExtDataLen;
							vectRawBuf.resize(sizeof(P_DBS4WEBUSER_NEW_PRIVATE_MAIL_T) + nMsgUnitLen);
							P_DBS4WEBUSER_NEW_PRIVATE_MAIL_T*	pNewMail	= (P_DBS4WEBUSER_NEW_PRIVATE_MAIL_T*)vectRawBuf.getbuf();
							pNewMail->nCmd			= P_DBS4WEB_RPL_CMD;
							pNewMail->nSubCmd		= CMDID_NEW_PRIVATE_MAIL_RPL;
							pNewMail->nAccountID	= pAdd->nReceiverID;
							PrivateMailUnit*	pMsgUnit	= (PrivateMailUnit*)wh_getptrnexttoptr(pNewMail);
							pMsgUnit->bReaded		= false;
							pMsgUnit->nType			= pAdd->nType;
							pMsgUnit->nFlag			= pAdd->nFlag;
							pMsgUnit->nMailID		= nMailID;
							pMsgUnit->nSenderID		= pAdd->nSenderID;
							pMsgUnit->nTextLen		= pAdd->nTextLen;
							pMsgUnit->nTime			= wh_time();
							WH_STRNCPY0(pMsgUnit->szSender, pAdd->szSender);
							memcpy(pMsgUnit->szText, pAdd->szText, pAdd->nTextLen);
							int*	pNewExtDataLen	= (int*)wh_getoffsetaddr(pMsgUnit, wh_offsetof(PrivateMailUnit, szText) + pAdd->nTextLen);
							*pNewExtDataLen			= *pExtDataLen;
							memcpy(wh_getptrnexttoptr(pNewExtDataLen), pExtData, *pExtDataLen);
							
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());	
						}
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_private_mail,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pAdd->nSenderID, pAdd->nReceiverID, (int)pAdd->nFlag);
						RplCmd.nRst		= STC_GAMECMD_ADD_MAIL_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_PRIVATE_MAIL_REQ:
		{
			P_DBS4WEB_GET_PRIVATE_MAIL_T*	pGet	= (P_DBS4WEB_GET_PRIVATE_MAIL_T*)pCmd;
			P_DBS4WEBUSER_GET_PRIVATE_MAIL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_PRIVATE_MAIL_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_MAIL_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_mail,DBERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_MAIL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT m.mail_id,m.sender_id,m.flag,m.readed,m.type,uncompress(m.text),uncompress(m.ext_data),m.time,IFNULL(c.name,'') as sender_name FROM private_mails m LEFT JOIN common_characters c ON m.sender_id=c.account_id WHERE m.account_id=0x%"WHINT64PRFX"X ORDER BY m.mail_id DESC", pGet->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_mail,SQLERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_MAIL_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum == 0)
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_PRIVATE_MAIL_T));
						P_DBS4WEBUSER_GET_PRIVATE_MAIL_T*	pRpl	= (P_DBS4WEBUSER_GET_PRIVATE_MAIL_T*)vectRawBuf.getbuf();
						memcpy(pRpl, &RplCmd, sizeof(RplCmd));
						PrivateMailUnit*	pUnit		= NULL;
						while (q.FetchRow())
						{
							int*	pLengths		= q.FetchLengths();
							int		nTotalMsgLen	= wh_offsetof(PrivateMailUnit, szText) + pLengths[5] + (sizeof(int)+pLengths[6]);
							pUnit					= (PrivateMailUnit*)vectRawBuf.pushn_back(nTotalMsgLen);
							
							pUnit->nTextLen			= pLengths[5];
							pUnit->nMailID			= q.GetVal_32();
							pUnit->nSenderID		= q.GetVal_64();
							pUnit->nFlag			= (char)q.GetVal_32();
							pUnit->bReaded			= (bool)q.GetVal_32();
							pUnit->nType			= (char)q.GetVal_32();
							memcpy(pUnit->szText, q.GetStr(), pUnit->nTextLen);
							int		nMsgUnitLen		= wh_offsetof(PrivateMailUnit, szText) + pUnit->nTextLen;
							int*	pExtDataLen		= (int*)wh_getoffsetaddr(pUnit, nMsgUnitLen);
							*pExtDataLen			= pLengths[6];
							void*	pExtData		= (void*)wh_getptrnexttoptr(pExtDataLen);
							memcpy(pExtData, q.GetStr(), *pExtDataLen);
							pUnit->nTime			= q.GetVal_32();
							WH_STRNCPY0(pUnit->szSender, q.GetStr());
						}

						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;
	case CMDID_READ_PRIVATE_MAIL_REQ:
		{
			P_DBS4WEB_READ_PRIVATE_MAIL_T*	pRead	= (P_DBS4WEB_READ_PRIVATE_MAIL_T*)pCmd;
			P_DBS4WEBUSER_READ_PRIVATE_MAIL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_READ_PRIVATE_MAIL_RPL;
			RplCmd.nMailID		= pRead->nMailID;
			RplCmd.nRst			= STC_GAMECMD_READ_MAIL_T::RST_OK;
			memcpy(RplCmd.nExt, pRead->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"read_private_mail,DBERR,0x%"WHINT64PRFX"X,%d", pRead->nAccountID, pRead->nMailID);
				RplCmd.nRst		= STC_GAMECMD_READ_MAIL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("UPDATE private_mails SET readed=1 WHERE account_id=0x%"WHINT64PRFX"X AND mail_id=%d AND flag!=%d"
					, pRead->nAccountID, pRead->nMailID, MAIL_FLAG_REWARD);
				int	nPreRet		= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"read_private_mail,SQLERR,0x%"WHINT64PRFX"X,%d", pRead->nAccountID, pRead->nMailID);
					RplCmd.nRst		= STC_GAMECMD_READ_MAIL_T::RST_SQL_ERR;
				}
				else
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"read_private_mail,0x%"WHINT64PRFX"X,%d", pRead->nAccountID, pRead->nMailID);
					RplCmd.nRst		= STC_GAMECMD_READ_MAIL_T::RST_OK;
				}
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
		}
		break;
	case CMDID_DEL_PRIVATE_MAIL_REQ:
		{
			P_DBS4WEB_DEL_PRIVATE_MAIL_T*	pDel	= (P_DBS4WEB_DEL_PRIVATE_MAIL_T*)pCmd;
			P_DBS4WEBUSER_DEL_PRIVATE_MAIL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_DEL_PRIVATE_MAIL_RPL;
			RplCmd.nMailID		= pDel->nMailID;
			RplCmd.nRst			= STC_GAMECMD_DELETE_MAIL_T::RST_OK;
			memcpy(RplCmd.nExt, pDel->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"delete_private_mail,DBERR,0x%"WHINT64PRFX"X,%d", pDel->nAccountID, pDel->nMailID);
				RplCmd.nRst		= STC_GAMECMD_DELETE_MAIL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("DELETE FROM private_mails WHERE account_id=0x%"WHINT64PRFX"X AND mail_id=%d AND flag!=%d"
					, pDel->nAccountID, pDel->nMailID, MAIL_FLAG_REWARD);
				int	nPreRet		= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"delete_private_mail,SQLERR,0x%"WHINT64PRFX"X,%d", pDel->nAccountID, pDel->nMailID);
					RplCmd.nRst		= STC_GAMECMD_DELETE_MAIL_T::RST_SQL_ERR;
				}
				else
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"delete_private_mail,0x%"WHINT64PRFX"X,%d", pDel->nAccountID, pDel->nMailID);
					RplCmd.nRst		= STC_GAMECMD_DELETE_MAIL_T::RST_OK;
				}
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
		}
		break;
	case CMDID_GET_PRIVATE_MAIL_1_REQ:
		{
			P_DBS4WEB_GET_PRIVATE_MAIL_1_T*	pGet	= (P_DBS4WEB_GET_PRIVATE_MAIL_1_T*)pCmd;
			P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_PRIVATE_MAIL_1_RPL;
			RplCmd.nRst			= P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T::RST_OK;
			RplCmd.nAccountID	= pGet->nAccountID;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_mail_1,DBERR,0x%"WHINT64PRFX"X,%d", pGet->nAccountID, pGet->nMailID);
				RplCmd.nRst		= P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT m.mail_id,m.sender_id,m.flag,m.readed,m.type,uncompress(m.text),uncompress(m.ext_data),m.time,IFNULL(c.name,'') as sender_name FROM private_mails m LEFT JOIN common_characters c ON m.sender_id=c.account_id WHERE m.account_id=0x%"WHINT64PRFX"X AND mail_id=%d", pGet->nAccountID, pGet->nMailID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_mail_1,SQLERR,0x%"WHINT64PRFX"X,%d", pGet->nAccountID, pGet->nMailID);
					RplCmd.nRst		= P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T));
						P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T*	pRpl	= (P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T*)vectRawBuf.getbuf();
						memcpy(pRpl, &RplCmd, sizeof(RplCmd));

						int*	pLengths		= q.FetchLengths();
						int		nTotalMsgLen	=  wh_offsetof(PrivateMailUnit, szText) + pLengths[5] + (sizeof(int)+pLengths[6]);
						PrivateMailUnit*	pUnit		= (PrivateMailUnit*)vectRawBuf.pushn_back(nTotalMsgLen);

						pUnit->nTextLen			= pLengths[5];
						pUnit->nMailID			= q.GetVal_32();
						pUnit->nSenderID		= q.GetVal_64();
						pUnit->nFlag			= (char)q.GetVal_32();
						pUnit->bReaded			= (bool)q.GetVal_32();
						pUnit->nType			= (char)q.GetVal_32();
						memcpy(pUnit->szText, q.GetStr(), pUnit->nTextLen);
						int		nMsgUnitLen		= wh_offsetof(PrivateMailUnit, szText) + pUnit->nTextLen;
						int*	pExtDataLen		= (int*)wh_getoffsetaddr(pUnit, nMsgUnitLen);
						*pExtDataLen			= pLengths[6];
						void*	pExtData		= (void*)wh_getptrnexttoptr(pExtDataLen);
						memcpy(pExtData, q.GetStr(), *pExtDataLen);
						pUnit->nTime			= q.GetVal_32();
						WH_STRNCPY0(pUnit->szSender, q.GetStr());

						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_mail_1,NOT EXISTS,0x%"WHINT64PRFX"X,%d", pGet->nAccountID, pGet->nMailID);
						RplCmd.nRst		= P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T::RST_NOT_EXIST;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_REFUSE_JOIN_ALLIANCE_REQ:
		{
			P_DBS4WEB_REFUSE_JOIN_ALLIANCE_T*	pRefuse	= (P_DBS4WEB_REFUSE_JOIN_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_REFUSE_JOIN_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_REFUSE_JOIN_ALLIANCE_RPL;
			RplCmd.nAllianceID	= 0;
			RplCmd.nApplicantID	= pRefuse->nApplicantID;
			RplCmd.nManagerID	= pRefuse->nManagerID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T::RST_OK;
			memcpy(RplCmd.nExt, pRefuse->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refuse_join_alliance,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pRefuse->nManagerID, pRefuse->nApplicantID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL refuse_join_alliance(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pRefuse->nManagerID, pRefuse->nApplicantID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refuse_join_alliance,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pRefuse->nManagerID, pRefuse->nApplicantID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"refuse_join_alliance,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X"
							, pRefuse->nManagerID, pRefuse->nApplicantID, RplCmd.nRst, RplCmd.nAllianceID);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refuse_join_alliance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pRefuse->nManagerID, pRefuse->nApplicantID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_REFUSE_JOIN_ALLI_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_ABDICATE_ALLIANCE_REQ:
		{
			P_DBS4WEB_ABDICATE_ALLIANCE_T*	pAbdicate	= (P_DBS4WEB_ABDICATE_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_ABDICATE_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ABDICATE_ALLIANCE_RPL;
			RplCmd.nAllianceID	= 0;
			RplCmd.nMemberID	= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_ABDICATE_ALLI_T::RST_OK;
			memcpy(RplCmd.nExt, pAbdicate->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"abdicate_alliance,DBERR,0x%"WHINT64PRFX"X,%s", pAbdicate->nLeaderID, pAbdicate->szMemberName);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_ABDICATE_ALLI_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL abdicate_alliance(0x%"WHINT64PRFX"X,'%s')", pAbdicate->nLeaderID, pAbdicate->szMemberName);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"abdicate_alliance,SQLERR,0x%"WHINT64PRFX"X,%s", pAbdicate->nLeaderID, pAbdicate->szMemberName);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_ABDICATE_ALLI_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						RplCmd.nMemberID	= q.GetVal_64();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"abdicate_alliance,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X"
							, pAbdicate->nLeaderID, RplCmd.nMemberID, RplCmd.nRst, RplCmd.nAllianceID);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"abdicate_alliance,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%s", pAbdicate->nLeaderID, pAbdicate->szMemberName);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_ABDICATE_ALLI_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_MY_FRIEND_APPLY_REQ:
		{
			P_DBS4WEB_GET_MY_FRIEND_APPLY_T*	pGet	= (P_DBS4WEB_GET_MY_FRIEND_APPLY_T*)pCmd;
			P_DBS4WEBUSER_GET_MY_FRIEND_APPLY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_MY_FRIEND_APPLY_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_MY_FRIEND_APPLY_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_my_friend_apply,DBERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_MY_FRIEND_APPLY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				// 自己申请为别人好友的事件
				q.SpawnQuery("SELECT e.peer_account_id,IFNULL(c.name,''),c.level,c.head_id FROM common_characters c RIGHT JOIN friend_apply_events e ON c.account_id=e.peer_account_id WHERE e.account_id=0x%"WHINT64PRFX"X", pGet->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_my_friend_apply,SQLERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_MY_FRIEND_APPLY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_MY_FRIEND_APPLY_T) + RplCmd.nNum*sizeof(FriendApplyUnit));
						P_DBS4WEBUSER_GET_MY_FRIEND_APPLY_T*	pRpl	= (P_DBS4WEBUSER_GET_MY_FRIEND_APPLY_T*)vectRawBuf.getbuf();
						memcpy(pRpl, &RplCmd, sizeof(RplCmd));
						FriendApplyUnit*	pUnit	= (FriendApplyUnit*)wh_getptrnexttoptr(pRpl);
						while (q.FetchRow())
						{
							pUnit->nAccountID		= q.GetVal_64();
							WH_STRNCPY0(pUnit->szName, q.GetStr());
							pUnit->nLevel			= q.GetVal_32();
							pUnit->nHeadID			= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_OTHERS_FRIEND_APPLY_REQ:
		{
			P_DBS4WEB_GET_OTHERS_FRIEND_APPLY_T*	pGet	= (P_DBS4WEB_GET_OTHERS_FRIEND_APPLY_T*)pCmd;
			P_DBS4WEBUSER_GET_OTHERS_FRIEND_APPLY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_OTHERS_FRIEND_APPLY_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_OTHERS_FRIEND_APPLY_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_others_friend_apply,DBERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_OTHERS_FRIEND_APPLY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				// 别人申请为自己好友的事件
				q.SpawnQuery("SELECT e.account_id,IFNULL(c.name,''),c.level,c.head_id FROM common_characters c RIGHT JOIN friend_apply_events e ON c.account_id=e.account_id WHERE e.peer_account_id=0x%"WHINT64PRFX"X", pGet->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_others_friend_apply,SQLERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_OTHERS_FRIEND_APPLY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_OTHERS_FRIEND_APPLY_T) + RplCmd.nNum*sizeof(FriendApplyUnit));
						P_DBS4WEBUSER_GET_OTHERS_FRIEND_APPLY_T*	pRpl	= (P_DBS4WEBUSER_GET_OTHERS_FRIEND_APPLY_T*)vectRawBuf.getbuf();
						memcpy(pRpl, &RplCmd, sizeof(RplCmd));
						FriendApplyUnit*	pUnit	= (FriendApplyUnit*)wh_getptrnexttoptr(pRpl);
						while (q.FetchRow())
						{
							pUnit->nAccountID		= q.GetVal_64();
							WH_STRNCPY0(pUnit->szName, q.GetStr());
							pUnit->nLevel			= q.GetVal_32();
							pUnit->nHeadID			= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_FRIEND_LIST_REQ:
		{
			P_DBS4WEB_GET_FRIEND_LIST_T*	pGet	= (P_DBS4WEB_GET_FRIEND_LIST_T*)pCmd;
			P_DBS4WEBUSER_GET_FRIEND_LIST_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_FRIEND_LIST_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_FRIEND_LIST_T::RST_OK;
			memcpy(RplCmd.nExt, pGet->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_friend_list,DBERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_FRIEND_LIST_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT p.peer_account_id,IFNULL(c.name,''),c.level,c.head_id FROM common_characters c RIGHT JOIN personal_relations p ON c.account_id=p.peer_account_id WHERE p.account_id=0x%"WHINT64PRFX"X AND p.relation_type=%d"
					, pGet->nAccountID, pr_type_friend);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_friend_list,SQLERR,0x%"WHINT64PRFX"X", pGet->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_FRIEND_LIST_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_FRIEND_LIST_T) + RplCmd.nNum*sizeof(FriendUnit));
						P_DBS4WEBUSER_GET_FRIEND_LIST_T*	pRpl	= (P_DBS4WEBUSER_GET_FRIEND_LIST_T*)vectRawBuf.getbuf();
						memcpy(pRpl, &RplCmd, sizeof(RplCmd));
						FriendUnit*	pUnit		= (FriendUnit*)wh_getptrnexttoptr(pRpl);
						while (q.FetchRow())
						{
							pUnit->nAccountID	= q.GetVal_64();
							WH_STRNCPY0(pUnit->szName, q.GetStr());
							pUnit->nLevel		= q.GetVal_32();
							pUnit->nHeadID		= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_APPROVE_FRIEND_REQ:
		{
			P_DBS4WEB_APPROVE_FRIEND_T*	pApprove	= (P_DBS4WEB_APPROVE_FRIEND_T*)pCmd;
			P_DBS4WEBUSER_APPROVE_FRIEND_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_APPROVE_FRIEND_RPL;
			RplCmd.nAccountID	= pApprove->nAccountID;
			RplCmd.nPeerAccountID	= pApprove->nPeerAccountID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_APPROVE_FRIEND_T::RST_OK;
			memcpy(RplCmd.nExt, pApprove->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"approve_friend,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pApprove->nAccountID, pApprove->nPeerAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_APPROVE_FRIEND_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL approve_friend(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pApprove->nAccountID, pApprove->nPeerAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"approve_friend,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pApprove->nAccountID, pApprove->nPeerAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_APPROVE_FRIEND_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						int	nFriendNum		= q.GetVal_32();
						int	nPeerFriendNum	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"approve_friend,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pApprove->nAccountID, pApprove->nPeerAccountID, RplCmd.nRst, nFriendNum, nPeerFriendNum);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"approve_friend,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pApprove->nAccountID, pApprove->nPeerAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_APPROVE_FRIEND_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_APPLY_FRIEND_REQ:
		{
			P_DBS4WEB_APPLY_FRIEND_T*	pApply	= (P_DBS4WEB_APPLY_FRIEND_T*)pCmd;
			P_DBS4WEBUSER_APPLY_FRIEND_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_APPLY_FRIEND_RPL;
			RplCmd.nAccountID	= pApply->nAccountID;
			RplCmd.nPeerAccountID	= pApply->nPeerAccountID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_APPLY_FRIEND_T::RST_OK;
			memcpy(RplCmd.nExt, pApply->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"apply_friend,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pApply->nAccountID, pApply->nPeerAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_APPLY_FRIEND_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL apply_friend(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pApply->nAccountID, pApply->nPeerAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"apply_friend,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pApply->nAccountID, pApply->nPeerAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_APPLY_FRIEND_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"apply_friend,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pApply->nAccountID, pApply->nPeerAccountID, RplCmd.nRst);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"apply_friend,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pApply->nAccountID, pApply->nPeerAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_APPLY_FRIEND_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_REFUSE_FRIEND_APPLY_REQ:
		{
			P_DBS4WEB_REFUSE_FRIEND_APPLY_T*	pRefuse	= (P_DBS4WEB_REFUSE_FRIEND_APPLY_T*)pCmd;
			P_DBS4WEBUSER_REFUSE_FRIEND_APPLY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_REFUSE_FRIEND_APPLY_RPL;
			RplCmd.nAccountID	= pRefuse->nAccountID;
			RplCmd.nPeerAccountID	= pRefuse->nPeerAccountID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T::RST_OK;
			memcpy(RplCmd.nExt, pRefuse->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refuse_friend_apply,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pRefuse->nAccountID, pRefuse->nPeerAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL refuse_friend_apply(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pRefuse->nAccountID, pRefuse->nPeerAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refuse_friend_apply,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pRefuse->nAccountID, pRefuse->nPeerAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"refuse_friend_apply,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pRefuse->nAccountID, pRefuse->nPeerAccountID, RplCmd.nRst);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refuse_friend_apply,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pRefuse->nAccountID, pRefuse->nPeerAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_REFUSE_FRIEND_APPLY_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_CANCEL_FRIEND_APPLY_REQ:
		{
			P_DBS4WEB_CANCEL_FRIEND_APPLY_T*	pCancel	= (P_DBS4WEB_CANCEL_FRIEND_APPLY_T*)pCmd;
			P_DBS4WEBUSER_CANCEL_FRIEND_APPLY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CANCEL_FRIEND_APPLY_RPL;
			RplCmd.nAccountID	= pCancel->nAccountID;
			RplCmd.nPeerAccountID	= pCancel->nPeerAccountID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T::RST_OK;
			memcpy(RplCmd.nExt, pCancel->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"cancel_friend_apply,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pCancel->nAccountID, pCancel->nPeerAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL cancel_friend_apply(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pCancel->nAccountID, pCancel->nPeerAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"cancel_friend_apply,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pCancel->nAccountID, pCancel->nPeerAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"cancel_friend_apply,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pCancel->nAccountID, pCancel->nPeerAccountID, RplCmd.nRst);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"cancel_friend_apply,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pCancel->nAccountID, pCancel->nPeerAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_FRIEND_APPLY_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_DELETE_FRIEND_REQ:
		{
			P_DBS4WEB_DELETE_FRIEND_T*	pDelete	= (P_DBS4WEB_DELETE_FRIEND_T*)pCmd;
			P_DBS4WEBUSER_DELETE_FRIEND_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_DELETE_FRIEND_RPL;
			RplCmd.nAccountID	= pDelete->nAccountID;
			RplCmd.nPeerAccountID	= pDelete->nPeerAccountID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_DELETE_FRIEND_T::RST_OK;
			memcpy(RplCmd.nExt, pDelete->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"delete_friend,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pDelete->nAccountID, pDelete->nPeerAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_DELETE_FRIEND_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL delete_friend(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pDelete->nAccountID, pDelete->nPeerAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"delete_friend,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pDelete->nAccountID, pDelete->nPeerAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_DELETE_FRIEND_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst	= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"delete_friend,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pDelete->nAccountID, pDelete->nPeerAccountID, RplCmd.nRst);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"delete_friend,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pDelete->nAccountID, pDelete->nPeerAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_DELETE_FRIEND_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_LOAD_ALLIANCE_REQ:
		{
			P_DBS4WEB_LOAD_ALLIANCE_T*	pReq	= (P_DBS4WEB_LOAD_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_LOAD_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_LOAD_ALLIANCE_RPL;
			RplCmd.nRst			= P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_OK;
			RplCmd.nAllianceID	= pReq->nAllianceID;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_alliance,DBERR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
				RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				int	nTotalSize	= sizeof(P_DBS4WEBUSER_LOAD_ALLIANCE_T);
				P_DBS4WEBUSER_LOAD_ALLIANCE_T*	pRplCmd	= (P_DBS4WEBUSER_LOAD_ALLIANCE_T*)vectRawBuf.getbuf();
				memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
				// 1.基本信息
				{
					q.SpawnQuery("SELECT a.name as alliance_name,c.name as leader_name,a.account_id,a.development,a.total_development,a.introduction FROM alliances a JOIN common_characters c ON a.account_id=c.account_id WHERE a.alliance_id=0x%"WHINT64PRFX"X", pReq->nAllianceID);
					int	nPreRet		= 0;
					q.GetResult(nPreRet, false);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_alliance,get_alliance_info,SQLERR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
						RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						if (q.FetchRow())
						{
							AllianceUnit*	pAlliance	= (AllianceUnit*)wh_getptrnexttoptr(pRplCmd);
							pAlliance->nAllianceID		= pReq->nAllianceID;
							WH_STRNCPY0(pAlliance->szAllianceName, q.GetStr());
							WH_STRNCPY0(pAlliance->szLeaderName, q.GetStr());
							pAlliance->nLeaderID		= q.GetVal_64();
							pAlliance->nDevelopment		= q.GetVal_32();
							pAlliance->nTotalDevelopment	= q.GetVal_32();
							WH_STRNCPY0(pAlliance->szIntroduction, q.GetStr());
						}
						else
						{
							RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_NOT_EXIST;
							SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
							return 0;
						}
					}
					nTotalSize		+= sizeof(AllianceUnit);
					q.FreeResult();
				}

				// 2.成员列表
				{
					q.SpawnQuery("SELECT m.account_id,m.position,m.development,m.total_development,c.name,c.level,c.cup FROM alliance_members m JOIN common_characters c ON m.account_id=c.account_id WHERE m.alliance_id=0x%"WHINT64PRFX"X", pReq->nAllianceID);
					int	nPreRet		= 0;
					q.GetResult(nPreRet, false);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_alliance,get_alliance_member,SQLERR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
						RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						int*	pNum	= (int*)wh_getoffsetaddr(pRplCmd, nTotalSize);
						*pNum			= q.NumRows();
						AllianceMemberUnit*	pUnit	= (AllianceMemberUnit*)wh_getptrnexttoptr(pNum);
						while (q.FetchRow())
						{
							pUnit->nAccountID		= q.GetVal_64();
							pUnit->nPosition		= q.GetVal_32();
							pUnit->nDevelopment		= q.GetVal_32();
							pUnit->nTotalDevelopment= q.GetVal_32();
							WH_STRNCPY0(pUnit->szName, q.GetStr());
							pUnit->nLevel			= q.GetVal_32();
							pUnit->nCup				= q.GetVal_32();

							pUnit++;
						}
						nTotalSize		+= (sizeof(int) + *pNum*sizeof(AllianceMemberUnit));
						q.FreeResult();
					}
				}

				// 3.建筑时间队列
				{
					q.SpawnQuery("SELECT d.excel_id,d.type,t.begin_time,t.end_time FROM alliance_building_time_events d JOIN time_events t ON d.event_id=t.event_id WHERE d.alliance_id=0x%"WHINT64PRFX"X", pReq->nAllianceID);
					int	nPreRet		= 0;
					q.GetResult(nPreRet, false);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_alliance,get_alliance_building_te,SQLERR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
						RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						int*	pNum	= (int*)wh_getoffsetaddr(pRplCmd, nTotalSize);
						*pNum			= q.NumRows();
						AllianceBuildingTimeEvent*	pEvent	= (AllianceBuildingTimeEvent*)wh_getptrnexttoptr(pNum);
						while (q.FetchRow())
						{
							pEvent->nExcelID	= q.GetVal_32();
							pEvent->nType		= q.GetVal_32();
							pEvent->nBeginTime	= q.GetVal_32();
							pEvent->nEndTime	= q.GetVal_32();

							pEvent++;
						}
						nTotalSize		+= (sizeof(int) + *pNum*sizeof(AllianceBuildingTimeEvent));
						q.FreeResult();
					}
				}

				// 4.建筑列表
				{
					q.SpawnQuery("SELECT excel_id,level FROM alliance_buildings WHERE alliance_id=0x%"WHINT64PRFX"X", pReq->nAllianceID);
					int	nPreRet		= 0;
					q.GetResult(nPreRet, false);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_alliance,get_alliance_building,SQLERR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
						RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						int*	pNum	= (int*)wh_getoffsetaddr(pRplCmd, nTotalSize);
						*pNum			= q.NumRows();
						AllianceBuildingUnit*	pUnit	= (AllianceBuildingUnit*)wh_getptrnexttoptr(pNum);
						while (q.FetchRow())
						{
							pUnit->nExcelID	= q.GetVal_32();
							pUnit->nLevel	= q.GetVal_32();

							pUnit++;
						}
						nTotalSize		+= (sizeof(int) + *pNum*sizeof(AllianceBuildingUnit));
						q.FreeResult();
					}
				}

				// 5.申请加入联盟事件列表
				{
					q.SpawnQuery("SELECT c.account_id,c.name,c.level,c.head_id FROM common_characters c JOIN alliance_join_events a ON c.account_id=a.account_id WHERE a.alliance_id=0x%"WHINT64PRFX"X", pReq->nAllianceID);
					int	nPreRet		= 0;
					q.GetResult(nPreRet, false);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_alliance,get_alliance_join_event,SQLERR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
						RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALLIANCE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						int*	pNum	= (int*)wh_getoffsetaddr(pRplCmd, nTotalSize);
						*pNum			= q.NumRows();
						AllianceJoinEvent*	pEvent	= (AllianceJoinEvent*)wh_getptrnexttoptr(pNum);
						while (q.FetchRow())
						{
							pEvent->nAccountID		= q.GetVal_64();
							WH_STRNCPY0(pEvent->szName, q.GetStr());
							pEvent->nLevel			= q.GetVal_32();
							pEvent->nHeadID			= q.GetVal_32();

							pEvent++;
						}
						nTotalSize		+= (sizeof(int) + *pNum*sizeof(AllianceJoinEvent));
						q.FreeResult();
					}
				}

				vectRawBuf.resize(nTotalSize);
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
			}
		}
		break;
	case CMDID_LOAD_ALL_ALLIANCE_ID_REQ:
		{
			P_DBS4WEB_LOAD_ALL_ALLIANCE_ID_T*	pReq	= (P_DBS4WEB_LOAD_ALL_ALLIANCE_ID_T*)pCmd;
			P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_LOAD_ALL_ALLIANCE_ID_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T::RST_OK;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_all_alliance_id,DBERR");
				RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT alliance_id FROM alliances ORDER BY total_development DESC");
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_all_alliance_id,SQLERR");
					RplCmd.nRst		= P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T) + RplCmd.nNum*sizeof(tty_id_t));
						P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T*	pRplCmd	= (P_DBS4WEBUSER_LOAD_ALL_ALLIANCE_ID_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						tty_id_t*	pID	= (tty_id_t*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							*pID		= q.GetVal_64();

							++pID;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_LOAD_EXCEL_TEXT_REQ:
		{
			P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_LOAD_EXCEL_TEXT_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nSize		= 0;
			RplCmd.nRst			= P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T::RST_OK;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_excel_text,DBERROR");
				RplCmd.nRst		= P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT max(excel_id) FROM excel_text");
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_excel_text,SQLERROR,get size");
					RplCmd.nRst		= P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nSize	= q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_excel_text,SQLERROR,NO RESULT,get size");
						RplCmd.nRst		= P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}

				q.FreeResult();
				
				q.SpawnQuery("SELECT excel_id,content FROM excel_text");
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_excel_text,SQLERROR");
					RplCmd.nRst		= P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T) + RplCmd.nNum*sizeof(ExcelText));
					P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T*	pRplCmd	= (P_DBS4WEBUSER_LOAD_EXCEL_TEXT_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					ExcelText*	pExcel	= (ExcelText*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pExcel->nExcelID	= q.GetVal_32();
						WH_STRNCPY0(pExcel->szText, q.GetStr());

						pExcel++;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_ALLIANCE_MAIL_REQ:
		{
			P_DBS4WEB_ADD_ALLIANCE_MAIL_T*	pReq	= (P_DBS4WEB_ADD_ALLIANCE_MAIL_T*)pCmd;
			vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T) + wh_offsetof(AllianceMailUnit, szText) + TTY_ALLIANCE_MAIL_TEXT_LEN);
			P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T*	pRplCmd	= (P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T*)vectRawBuf.getbuf();
			pRplCmd->nCmd			= P_DBS4WEB_RPL_CMD;
			pRplCmd->nSubCmd		= CMDID_ADD_ALLIANCE_MAIL_RPL;
			pRplCmd->nAllianceID	= 0;
			pRplCmd->nRst			= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_OK;
			vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T));
			memcpy(pRplCmd->nExt, pReq->nExt, sizeof(pReq->nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_mail,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				pRplCmd->nRst		= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_alliance_mail(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, pReq->szText);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_mail,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					pRplCmd->nRst		= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						AllianceMailUnit*	pMail	= (AllianceMailUnit*)wh_getptrnexttoptr(pRplCmd);
						pRplCmd->nRst			= q.GetVal_32();
						pRplCmd->nAllianceID	= q.GetVal_64();
						pMail->nMailID			= q.GetVal_32();
						pMail->nAccountID		= pReq->nAccountID;
						pMail->nTime			= wh_time();
						WH_STRNCPY0(pMail->szName, pReq->szSender);
						pMail->nTextLen			= pReq->nTextLen;
						memcpy(pMail->szText, pReq->szText, pMail->nTextLen);
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_ADD_ALLIANCE_MAIL_T) + wh_offsetof(AllianceMailUnit, szText) + pMail->nTextLen);

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_alliance_mail,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pRplCmd->nRst, pRplCmd->nAllianceID, pMail->nMailID);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_mail,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						pRplCmd->nRst			= STC_GAMECMD_OPERATE_ADD_ALLIANCE_MAIL_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_ALLIANCE_LOG_REQ:
		{
			P_DBS4WEB_ADD_ALLIANCE_LOG_T*	pReq	= (P_DBS4WEB_ADD_ALLIANCE_LOG_T*)pCmd;
			P_DBS4WEBUSER_ADD_ALLIANCE_LOG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_ALLIANCE_LOG_RPL;
			RplCmd.nAllianceID	= pReq->nAllianceID;
			RplCmd.nLogID		= 0;
			RplCmd.nRst			= P_DBS4WEBUSER_ADD_ALLIANCE_LOG_T::RST_OK;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_log,DBERROR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
				RplCmd.nRst		= P_DBS4WEBUSER_ADD_ALLIANCE_LOG_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_alliance_log(0x%"WHINT64PRFX"X, '%s')", pReq->nAllianceID, pReq->szText);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_log,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
					RplCmd.nRst		= P_DBS4WEBUSER_ADD_ALLIANCE_LOG_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nLogID	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_alliance_log,0x%"WHINT64PRFX"X,%d", pReq->nAllianceID, RplCmd.nLogID);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_log,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAllianceID);
						RplCmd.nRst		= P_DBS4WEBUSER_ADD_ALLIANCE_LOG_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_PRIVATE_LOG_REQ:
		{
			P_DBS4WEB_ADD_PRIVATE_LOG_T*	pReq	= (P_DBS4WEB_ADD_PRIVATE_LOG_T*)pCmd;
			P_DBS4WEBUSER_ADD_PRIVATE_LOG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_PRIVATE_LOG_RPL;
			RplCmd.nLogID		= 0;
			RplCmd.nRst			= P_DBS4WEBUSER_ADD_PRIVATE_LOG_T::RST_OK;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_private_log,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= P_DBS4WEBUSER_ADD_PRIVATE_LOG_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_private_log(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, pReq->szText);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_private_log,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= P_DBS4WEBUSER_ADD_PRIVATE_LOG_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nLogID	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_private_log,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, RplCmd.nLogID);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_private_log,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst		= P_DBS4WEBUSER_ADD_PRIVATE_LOG_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_MAIL_REQ:
		{
			P_DBS4WEB_GET_ALLIANCE_MAIL_T*	pReq	= (P_DBS4WEB_GET_ALLIANCE_MAIL_T*)pCmd;
			P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_ALLIANCE_MAIL_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nAllianceID	= pReq->nAllianceID;
			RplCmd.nRst			= STC_GAMECMD_GET_ALLIANCE_MAIL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_mail,DBERROR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
				RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_MAIL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT m.mail_id,m.text,m.time,c.account_id,c.name FROM alliance_mails m JOIN common_characters c ON m.account_id=c.account_id WHERE m.alliance_id=0x%"WHINT64PRFX"X", pReq->nAllianceID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_mail,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
					RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_MAIL_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.reserve(sizeof(P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T) + RplCmd.nNum*(wh_offsetof(AllianceMailUnit,szText)+TTY_ALLIANCE_MAIL_TEXT_LEN));
						P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T*	pRplCmd	= (P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						AllianceMailUnit*	pAllianceMail			= (AllianceMailUnit*)wh_getptrnexttoptr(pRplCmd);
						int		nTotalSize			= sizeof(P_DBS4WEBUSER_GET_ALLIANCE_MAIL_T);
						while (q.FetchRow())
						{
							int*	pLengths		= q.FetchLengths();
							pAllianceMail->nMailID	= q.GetVal_32();
							pAllianceMail->nTextLen	= pLengths[1];
							memcpy(pAllianceMail->szText, q.GetStr(), pLengths[1]);
							pAllianceMail->nTime	= q.GetVal_32();
							pAllianceMail->nAccountID	= q.GetVal_64();
							WH_STRNCPY0(pAllianceMail->szName, q.GetStr());

							nTotalSize				+= (wh_offsetof(AllianceMailUnit,szText) + pAllianceMail->nTextLen);
							pAllianceMail			= (AllianceMailUnit*)wh_getoffsetaddr(pRplCmd, nTotalSize);
						}
						vectRawBuf.resize(nTotalSize);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_LOG_REQ:
		{
			P_DBS4WEB_GET_ALLIANCE_LOG_T*	pReq	= (P_DBS4WEB_GET_ALLIANCE_LOG_T*)pCmd;
			P_DBS4WEBUSER_GET_ALLIANCE_LOG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_ALLIANCE_LOG_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_ALLIANCE_LOG_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_log,DBERROR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
				RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_LOG_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT log_id,text,time FROM alliance_logs WHERE alliance_id=0x%"WHINT64PRFX"X", pReq->nAllianceID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_log,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAllianceID);
					RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_LOG_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.reserve(sizeof(P_DBS4WEBUSER_GET_ALLIANCE_LOG_T) + RplCmd.nNum*(wh_offsetof(AllianceLogUnit,szText)+TTY_ALLIANCE_LOG_TEXT_LEN));
						P_DBS4WEBUSER_GET_ALLIANCE_LOG_T*	pRplCmd	= (P_DBS4WEBUSER_GET_ALLIANCE_LOG_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						AllianceLogUnit*	pAllianceLog			= (AllianceLogUnit*)wh_getptrnexttoptr(pRplCmd);
						int		nTotalSize			= sizeof(P_DBS4WEBUSER_GET_ALLIANCE_LOG_T);
						while (q.FetchRow())
						{
							int*	pLengths		= q.FetchLengths();
							pAllianceLog->nLogID	= q.GetVal_32();
							pAllianceLog->nTextLen	= pLengths[1];
							memcpy(pAllianceLog->szText, q.GetStr(), pLengths[1]);
							pAllianceLog->nTime		= q.GetVal_32();

							nTotalSize				+= (wh_offsetof(AllianceLogUnit,szText) + pAllianceLog->nTextLen);
							pAllianceLog			= (AllianceLogUnit*)wh_getoffsetaddr(pRplCmd, nTotalSize);
						}
						vectRawBuf.resize(nTotalSize);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_PRIVATE_LOG_REQ:
		{
			P_DBS4WEB_GET_PRIVATE_LOG_T*	pReq	= (P_DBS4WEB_GET_PRIVATE_LOG_T*)pCmd;
			P_DBS4WEBUSER_GET_PRIVATE_LOG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_PRIVATE_LOG_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_PRIVATE_LOG_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_log,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_PRIVATE_LOG_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT log_id,text,time FROM private_logs WHERE account_id=0x%"WHINT64PRFX"X", pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_log,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_PRIVATE_LOG_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.reserve(sizeof(P_DBS4WEBUSER_GET_PRIVATE_LOG_T) + RplCmd.nNum*(wh_offsetof(PrivateLogUnit,szText)+TTY_PRIVATE_LOG_TEXT_LEN));
						P_DBS4WEBUSER_GET_PRIVATE_LOG_T*	pRplCmd	= (P_DBS4WEBUSER_GET_PRIVATE_LOG_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						PrivateLogUnit*	pPrivateLog	= (PrivateLogUnit*)wh_getptrnexttoptr(pRplCmd);
						int		nTotalSize			= sizeof(P_DBS4WEBUSER_GET_PRIVATE_LOG_T);
						while (q.FetchRow())
						{
							int*	pLengths		= q.FetchLengths();
							pPrivateLog->nLogID	= q.GetVal_32();
							pPrivateLog->nTextLen	= pLengths[1];
							memcpy(pPrivateLog->szText, q.GetStr(), pLengths[1]);
							pPrivateLog->nTime		= q.GetVal_32();

							nTotalSize				+= (wh_offsetof(PrivateLogUnit,szText) + pPrivateLog->nTextLen);
							pPrivateLog				= (PrivateLogUnit*)wh_getoffsetaddr(pRplCmd, nTotalSize);
						}
						vectRawBuf.resize(nTotalSize);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_BUY_ITEM_REQ:
		{
			P_DBS4WEB_BUY_ITEM_T*	pReq	= (P_DBS4WEB_BUY_ITEM_T*)pCmd;
			P_DBS4WEBUSER_BUY_ITEM_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_BUY_ITEM_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nMoneyNum	= 0;
			RplCmd.nMoneyType	= pReq->nMoneyType;
			RplCmd.nExcelID		= pReq->nExcelID;
			RplCmd.nItemIDNum	= 0;
			RplCmd.nNum			= pReq->nNum;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_BUY_ITEM_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"buy_item,DBERROR,0x%"WHINT64PRFX"X,%d,%d,%d"
					, pReq->nAccountID, pReq->nExcelID, pReq->nNum,pReq->nMoneyType);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_BUY_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL buy_item(0x%"WHINT64PRFX"X, %d, %d, %d)", pReq->nAccountID, pReq->nExcelID, pReq->nNum, pReq->nMoneyType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"buy_item,SQLERROR,0x%"WHINT64PRFX"X,%d,%d,%d"
						, pReq->nAccountID, pReq->nExcelID, pReq->nNum, pReq->nMoneyType);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_BUY_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nMoneyNum	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"buy_item,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d"
							, pReq->nAccountID, pReq->nExcelID, pReq->nNum, RplCmd.nRst, RplCmd.nMoneyNum, RplCmd.nMoneyType);
						if (RplCmd.nRst == STC_GAMECMD_OPERATE_BUY_ITEM_T::RST_OK)
						{
							int*	pLengths	= q.FetchLengths();
							char*	szBuf		= new char[pLengths[2]+3];
							memcpy(szBuf, q.GetStr(), pLengths[2]);
							szBuf[pLengths[2]]		= ',';
							szBuf[pLengths[2]+1]	= '0';
							szBuf[pLengths[2]+2]	= 0;
							tty_id_t	nItemID;
							whvector<tty_id_t>	vectItemIDs;
							while (wh_strsplit("Ia", szBuf, ",", &nItemID, szBuf) == 2)
							{
								if (nItemID != 0)
								{
									vectItemIDs.push_back(nItemID);
								}
							}
							delete[] szBuf;
							RplCmd.nItemIDNum	= vectItemIDs.size();

							vectRawBuf.resize(sizeof(P_DBS4WEBUSER_BUY_ITEM_T) + vectItemIDs.size()*sizeof(tty_id_t));
							P_DBS4WEBUSER_BUY_ITEM_T*	pRplCmd	= (P_DBS4WEBUSER_BUY_ITEM_T*)vectRawBuf.getbuf();
							memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
							memcpy(wh_getptrnexttoptr(pRplCmd), vectItemIDs.getbuf(), vectItemIDs.size()*sizeof(tty_id_t));
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
							return 0;
						}
						else
						{
							SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
							return 0;
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"buy_item_with_diamond,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d"
							, pReq->nAccountID, pReq->nExcelID, pReq->nNum);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_BUY_ITEM_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_SELL_ITEM_REQ:
		{
			P_DBS4WEB_SELL_ITEM_T*	pReq	= (P_DBS4WEB_SELL_ITEM_T*)pCmd;
			P_DBS4WEBUSER_SELL_ITEM_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_SELL_ITEM_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nExcelID		= 0;
			RplCmd.nGold		= 0;
			RplCmd.nItemID		= pReq->nItemID;
			RplCmd.nNum			= pReq->nNum;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_SELL_ITEM_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sell_item,DBERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
					, pReq->nAccountID, pReq->nItemID, pReq->nNum);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_SELL_ITEM_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL sell_item(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d)", pReq->nAccountID, pReq->nItemID, pReq->nNum);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sell_item,SQLERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
						, pReq->nAccountID, pReq->nItemID, pReq->nNum);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_SELL_ITEM_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nGold	= q.GetVal_32();
						RplCmd.nExcelID	= q.GetVal_32();
						RplCmd.nCup		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"sell_item,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d"
							, pReq->nAccountID, pReq->nItemID, pReq->nNum, RplCmd.nRst, RplCmd.nGold, RplCmd.nExcelID);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"sell_item,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
							, pReq->nAccountID, pReq->nItemID, pReq->nNum);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_SELL_ITEM_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_RELATION_LOG_REQ:
		{
			P_DBS4WEB_GET_RELATION_LOG_T*	pReq	= (P_DBS4WEB_GET_RELATION_LOG_T*)pCmd;
			P_DBS4WEBUSER_GET_RELATION_LOG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_RELATION_LOG_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_RELATION_LOG_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_relation_log,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_RELATION_LOG_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT log_id,type,p_account_id,time,text FROM relation_logs WHERE account_id=0x%"WHINT64PRFX"X", pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_relation_log,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_RELATION_LOG_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_RELATION_LOG_T) + RplCmd.nNum*(wh_offsetof(RelationLogUnit, szText)+TTY_RELATION_LOG_TEXT_LEN));
						P_DBS4WEBUSER_GET_RELATION_LOG_T*	pGet	= (P_DBS4WEBUSER_GET_RELATION_LOG_T*)vectRawBuf.getbuf();
						memcpy(pGet, &RplCmd, sizeof(RplCmd));
						RelationLogUnit*	pLog	= (RelationLogUnit*)wh_getptrnexttoptr(pGet);
						int	nTotalSize				= sizeof(P_DBS4WEBUSER_GET_RELATION_LOG_T);
						while (q.FetchRow())
						{
							int*	pLengths		= q.FetchLengths();
							pLog->nLogID			= q.GetVal_32();
							pLog->nType				= q.GetVal_32();
							pLog->nAccountID		= q.GetVal_64();
							pLog->nTime				= q.GetVal_32();
							pLog->nTextLen			= pLengths[4];
							memcpy(pLog->szText, q.GetStr(), pLog->nTextLen);

							int	nUnitLen			= wh_offsetof(RelationLogUnit, szText) + pLog->nTextLen;
							nTotalSize				+= nUnitLen;
							pLog					= (RelationLogUnit*)wh_getoffsetaddr(pLog, nUnitLen);
						}
						vectRawBuf.resize(nTotalSize);
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_FETCH_CHRISTMAS_TREE_REQ:
		{
			P_DBS4WEB_FETCH_CHRISTMAS_TREE_T*	pReq	= (P_DBS4WEB_FETCH_CHRISTMAS_TREE_T*)pCmd;
			P_DBS4WEBUSER_FETCH_CHRISTMAS_TREE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_FETCH_CHRISTMAS_TREE_RPL;
			RplCmd.nCrystal		= 0;
			RplCmd.nExcelID		= 0;
			RplCmd.nItemID		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_christmas_tree,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL fetch_christmas_tree(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_christmas_tree,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nItemID	= q.GetVal_64();
						RplCmd.nExcelID	= q.GetVal_32();
						RplCmd.nCrystal	= q.GetVal_32();
						float fPercent	= q.GetVal_float();
						float fTotalPer	= q.GetVal_float();
						float fRand		= q.GetVal_float();

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"fetch_christmas_tree,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X,%f,%f,%f"
							, pReq->nAccountID, RplCmd.nRst, RplCmd.nCrystal, RplCmd.nExcelID, RplCmd.nItemID, fPercent, fTotalPer, fRand);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_christmas_tree,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_FETCH_CHRISTMAS_TREE_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_WATERING_CHRISTMAS_TREE_REQ:
		{
			P_DBS4WEB_WATERING_CHRISTMAS_TREE_T*	pReq	= (P_DBS4WEB_WATERING_CHRISTMAS_TREE_T*)pCmd;
			P_DBS4WEBUSER_WATERING_CHRISTMAS_TREE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_WATERING_CHRISTMAS_TREE_RPL;
			RplCmd.bReadyRipe	= false;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nRipeTimeInterval	= 0;
			RplCmd.nBeginTime	= 0;
			RplCmd.nWateringAccountID	= pReq->nWateringAccountID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_christmas_tree,DBERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nWateringAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL watering_christmas_tree(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pReq->nAccountID, pReq->nWateringAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_christmas_tree,SQLERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nWateringAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_event_id,_ripe_time_interval,_ready_ripe
						RplCmd.nRst				= q.GetVal_32();
						tty_id_t	nEventID	= q.GetVal_64();
						RplCmd.nBeginTime		= q.GetVal_32();
						RplCmd.bReadyRipe		= (bool)q.GetVal_32();
						RplCmd.nRipeTimeInterval= q.GetVal_32();
						
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"fetch_christmas_tree,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pReq->nAccountID, pReq->nWateringAccountID, RplCmd.nRst, nEventID, RplCmd.nRipeTimeInterval, (int)RplCmd.bReadyRipe, RplCmd.nBeginTime);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_christmas_tree,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nWateringAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_WATERING_CHRISTMAS_TREE_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_STEAL_GOLD_REQ:
		{
			P_DBS4WEB_STEAL_GOLD_T*	pReq	= (P_DBS4WEB_STEAL_GOLD_T*)pCmd;
			P_DBS4WEBUSER_STEAL_GOLD_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_STEAL_GOLD_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nThiefAccountID	= pReq->nThiefAccountID;
			RplCmd.nAutoID		= pReq->nAutoID;
			RplCmd.nGold		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_STEAL_GOLD_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"steal_gold,DBERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nThiefAccountID, pReq->nAutoID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_STEAL_GOLD_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL steal_gold(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d)", pReq->nAccountID, pReq->nThiefAccountID, pReq->nAutoID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"steal_gold,SQLERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nThiefAccountID, pReq->nAutoID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_STEAL_GOLD_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// 	_result,_steal_gold,_steal_percent,_steal_num_max
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nGold	= q.GetVal_32();
						float fPercent	= q.GetVal_float();
						int nStealMax	= q.GetVal_32();

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"steal_gold,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%f,%d"
							, pReq->nAccountID, pReq->nThiefAccountID, pReq->nAutoID, RplCmd.nRst, RplCmd.nGold, fPercent, nStealMax);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"steal_gold,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nThiefAccountID, pReq->nAutoID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_STEAL_GOLD_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_RELATION_LOG_REQ:
		{
			P_DBS4WEB_ADD_RELATION_LOG_T*	pReq	= (P_DBS4WEB_ADD_RELATION_LOG_T*)pCmd;
			P_DBS4WEBUSER_ADD_RELATION_LOG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_RELATION_LOG_RPL;
			RplCmd.nLogID		= 0;
			RplCmd.nRst			= P_DBS4WEBUSER_ADD_RELATION_LOG_T::RST_OK;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_relation_log,DBERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nPeerAccountID, pReq->nType);
				RplCmd.nRst		= P_DBS4WEBUSER_ADD_RELATION_LOG_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_relation_log(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,'%s')", pReq->nAccountID, pReq->nPeerAccountID, pReq->nType, pReq->szText);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_relation_log,SQLERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nPeerAccountID, pReq->nType);
					RplCmd.nRst			= P_DBS4WEBUSER_ADD_RELATION_LOG_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _log_id,_time
						RplCmd.nLogID	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_relation_log,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nPeerAccountID, pReq->nType, RplCmd.nLogID);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_relation_log,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nPeerAccountID, pReq->nType);
						RplCmd.nRst		= P_DBS4WEBUSER_ADD_RELATION_LOG_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_TRAINING_TE_REQ:
		{
			P_DBS4WEB_GET_TRAINING_TE_T*	pReq	= (P_DBS4WEB_GET_TRAINING_TE_T*)pCmd;
			P_DBS4WEBUSER_GET_TRAINING_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_TRAINING_TE_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_TRAINING_TE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_training_te,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_TRAINING_TE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT h.hero_id,h.exp,t.begin_time,t.end_time FROM hero_training_time_events h JOIN time_events t ON h.event_id=t.event_id WHERE h.account_id=0x%"WHINT64PRFX"X", pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_training_te,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_TRAINING_TE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_TRAINING_TE_T) + RplCmd.nNum*sizeof(TrainingTimeEvent));
						P_DBS4WEBUSER_GET_TRAINING_TE_T*	pRplCmd	= (P_DBS4WEBUSER_GET_TRAINING_TE_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						TrainingTimeEvent*	pEvent	= (TrainingTimeEvent*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pEvent->nHeroID			= q.GetVal_64();
							pEvent->nExp			= q.GetVal_32();
							pEvent->nBeginTime		= q.GetVal_32();
							pEvent->nEndTime		= q.GetVal_32();

							pEvent					= (TrainingTimeEvent*)wh_getptrnexttoptr(pEvent);
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_ADD_TRAINING_REQ:
		{
			P_DBS4WEB_ADD_TRAINING_T*	pReq		= (P_DBS4WEB_ADD_TRAINING_T*)pCmd;
			P_DBS4WEBUSER_ADD_TRAINING_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_TRAINING_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nGold		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_ADD_TRAINING_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_training,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_ADD_TRAINING_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				tty_id_t*	pHeroID	= (tty_id_t*)wh_getptrnexttoptr(pReq);
				char	szBuf[1024];
				int		nLength	= 0;
				for (int i=0; i<(pReq->nNum-1); i++)
				{
					nLength	+= sprintf(szBuf+nLength, "0x%"WHINT64PRFX"X,", *pHeroID);
					pHeroID++;
				}
				nLength		+= sprintf(szBuf+nLength, "0x%"WHINT64PRFX"X", *pHeroID);
				szBuf[nLength]	= 0;
				q.SpawnQuery("CALL add_training_time_event(0x%"WHINT64PRFX"X,'%s',%d)", pReq->nAccountID, szBuf, pReq->nTimeUnitNum);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_training,SQLERROR,0x%"WHINT64PRFX"X,%s,%d", pReq->nAccountID, szBuf, pReq->nTimeUnitNum);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_ADD_TRAINING_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// SELECT _result,_event_id,@_gold,@_exp,@_time,@_time+_time;
						RplCmd.nRst				= q.GetVal_32();
						RplCmd.nGold			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_training,0x%"WHINT64PRFX"X,%s,%d,%d,%d", pReq->nAccountID, szBuf, RplCmd.nRst, RplCmd.nGold, pReq->nTimeUnitNum);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_training,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%s,%d", pReq->nAccountID, szBuf, pReq->nTimeUnitNum);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_ADD_TRAINING_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_EXIT_TRAINING_REQ:
		{
			P_DBS4WEB_EXIT_TRAINING_T*	pReq	= (P_DBS4WEB_EXIT_TRAINING_T*)pCmd;
			P_DBS4WEBUSER_EXIT_TRAINING_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_EXIT_TRAINING_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			memset(&RplCmd.hero, 0, sizeof(RplCmd.hero));
			RplCmd.nRst			= STC_GAMECMD_OPERATE_EXIT_TRAINING_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"exit_training,DBERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_EXIT_TRAINING_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL exit_training(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pReq->nAccountID, pReq->nHeroID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"exit_training,SQLERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_EXIT_TRAINING_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_training_percent,_exp;
						RplCmd.nRst					= q.GetVal_32();
						float	fTrainingPercent	= q.GetVal_float();
						int		nExp				= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"exit_training,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%f,%d"
							, pReq->nAccountID, pReq->nHeroID, RplCmd.nRst, fTrainingPercent, nExp);

						if (RplCmd.nRst == STC_GAMECMD_OPERATE_EXIT_TRAINING_T::RST_OK)
						{
							q.FreeResult();
							q.SpawnQuery("select T1.hero_id,T1.name,T1.profession,T1.model,T1.attack,T1.defense,T1.health,T1.leader,T1.grow,T1.army_type,T1.army_level,T1.army_num,T1.status,T1.attack_add,T1.defense_add,T1.health_add,T1.leader_add,T1.level,T1.exp,T1.army_prof,T1.healthstate,T1.grow_max from hire_heros T1 where T1.hero_id=0x%"WHINT64PRFX"X", pReq->nHeroID);
							q.GetResult(nPreRet, false);
							if (nPreRet == MYSQL_QUERY_NORMAL)
							{
								if (q.FetchRow())
								{
									HireHero*	pHero	= &RplCmd.hero;
									pHero->nHeroID		= q.GetVal_64();
									strcpy(pHero->szName, q.GetStr());
									pHero->nProfession	= q.GetVal_32();
									pHero->nModel		= q.GetVal_32();
									pHero->nAttack		= q.GetVal_32();
									pHero->nDefense		= q.GetVal_32();
									pHero->nHealth		= q.GetVal_32();
									pHero->nLeader		= q.GetVal_32();
									pHero->fGrow		= q.GetVal_float();
									pHero->nArmyType	= q.GetVal_32();
									pHero->nArmyLevel	= q.GetVal_32();
									pHero->nArmyNum		= q.GetVal_32();
									pHero->nStatus		= q.GetVal_32();
									pHero->nAttackAdd		= q.GetVal_32();
									pHero->nDefenseAdd		= q.GetVal_32();
									pHero->nHealthAdd		= q.GetVal_32();
									pHero->nLeaderAdd		= q.GetVal_32();
									pHero->nLevel		= q.GetVal_32();
									pHero->nExp			= q.GetVal_32();
									pHero->nProf		= q.GetVal_32();
									pHero->nHealthState	= q.GetVal_32();
									pHero->fGrowMax		= q.GetVal_float();
								}
							}
						}
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"exit_training,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_EXIT_TRAINING_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_DEAL_TRAINING_TE_REQ:
		{
			P_DBS4WEB_DEAL_TRAINING_TE_T*	pReq	= (P_DBS4WEB_DEAL_TRAINING_TE_T*)pCmd;
			P_DBS4WEBUSER_DEAL_TRAINING_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_DEAL_TRAINING_TE_RPL;
			RplCmd.nAccountID	= 0;
			RplCmd.nExp			= 0;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_training_te,DBERROR,0x%"WHINT64PRFX"X", pReq->nEventID);
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL deal_training_event(0x%"WHINT64PRFX"X)", pReq->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_training_te,SQLERROR,0x%"WHINT64PRFX"X", pReq->nEventID);
					//SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_account_id,_hero_id,_exp;
						int nRst			= q.GetVal_32();
						RplCmd.nAccountID	= q.GetVal_64();
						tty_id_t	nHeroID	= q.GetVal_64();
						RplCmd.nExp			= q.GetVal_32();
						memset(&RplCmd.hero, 0, sizeof(RplCmd.hero));
						if (nRst == 0)	// 成功了就发送
						{
							q.FreeResult();
							q.SpawnQuery("select T1.hero_id,T1.name,T1.profession,T1.model,T1.attack,T1.defense,T1.health,T1.leader,T1.grow,T1.army_type,T1.army_level,T1.army_num,T1.status,T1.attack_add,T1.defense_add,T1.health_add,T1.leader_add,T1.level,T1.exp,T1.army_prof,T1.healthstate,T1.grow_max from hire_heros T1 where T1.hero_id=0x%"WHINT64PRFX"X", nHeroID);
							q.GetResult(nPreRet, false);
							if (nPreRet == MYSQL_QUERY_NORMAL)
							{
								if (q.FetchRow())
								{
									HireHero*	pHero	= &RplCmd.hero;
									pHero->nHeroID		= q.GetVal_64();
									strcpy(pHero->szName, q.GetStr());
									pHero->nProfession	= q.GetVal_32();
									pHero->nModel		= q.GetVal_32();
									pHero->nAttack		= q.GetVal_32();
									pHero->nDefense		= q.GetVal_32();
									pHero->nHealth		= q.GetVal_32();
									pHero->nLeader		= q.GetVal_32();
									pHero->fGrow		= q.GetVal_float();
									pHero->nArmyType	= q.GetVal_32();
									pHero->nArmyLevel	= q.GetVal_32();
									pHero->nArmyNum		= q.GetVal_32();
									pHero->nStatus		= q.GetVal_32();
									pHero->nAttackAdd		= q.GetVal_32();
									pHero->nDefenseAdd		= q.GetVal_32();
									pHero->nHealthAdd		= q.GetVal_32();
									pHero->nLeaderAdd		= q.GetVal_32();
									pHero->nLevel		= q.GetVal_32();
									pHero->nExp			= q.GetVal_32();
									pHero->nProf		= q.GetVal_32();
									pHero->nHealthState	= q.GetVal_32();
									pHero->fGrowMax		= q.GetVal_float();
								}
							}
							// 查出英雄的数据
							SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						}
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal_training_te,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
							, pReq->nEventID, nRst, RplCmd.nAccountID, nHeroID, RplCmd.nExp);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_training_te,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nEventID);
						//SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_CONTRIBUTE_ALLIANCE_REQ:
		{
			P_DBS4WEB_CONTRIBUTE_ALLIANCE_T*	pReq	= (P_DBS4WEB_CONTRIBUTE_ALLIANCE_T*)pCmd;
			P_DBS4WEBUSER_CONTRIBUTE_ALLIANCE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CONTRIBUTE_ALLIANCE_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nContributeNum	= 0;
			RplCmd.nGold		= 0;
			RplCmd.nAllianceID	= 0;
			RplCmd.nCongressLevel	= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"contribute_alliance,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nGold);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL contribute_alliance(0x%"WHINT64PRFX"X, %d)", pReq->nAccountID, pReq->nGold);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"contribute_alliance,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nGold);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_contribute_num,_gold,_alliance_id,@_congress_level
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nContributeNum	= q.GetVal_32();
						RplCmd.nGold		= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						RplCmd.nCongressLevel	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"contribute_alliance,0x%"WHINT64PRFX"X,%d,%d,%d,%d,0x%"WHINT64PRFX"X"
							, pReq->nAccountID, pReq->nGold, RplCmd.nRst, RplCmd.nContributeNum, RplCmd.nGold, RplCmd.nAllianceID);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"contribute_alliance,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nGold);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_CONVERT_DIAMOND_GOLD_REQ:
		{
			P_DBS4WEB_CONVERT_DIAMOND_GOLD_T*	pReq	= (P_DBS4WEB_CONVERT_DIAMOND_GOLD_T*)pCmd;
			P_DBS4WEBUSER_CONVERT_DIAMOND_GOLD_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CONVERT_DIAMOND_GOLD_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nDiamond		= pReq->nDiamond;
			RplCmd.nGold		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"convert_diamond_gold,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nDiamond);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL convert_diamond_gold(0x%"WHINT64PRFX"X, %d)", pReq->nAccountID, pReq->nDiamond);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"convert_diamond_gold,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nDiamond);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_CONVERT_DIAMOND_GOLD_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_gold
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nGold		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"convert_diamond_gold,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pReq->nAccountID, pReq->nDiamond, RplCmd.nRst,  RplCmd.nGold);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"convert_diamond_gold,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nDiamond);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_CONTRIBUTE_ALLIANCE_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_JOIN_ALLIANCE_NAME_REQ:
		{
			P_DBS4WEB_JOIN_ALLIANCE_NAME_T*		pReq	= (P_DBS4WEB_JOIN_ALLIANCE_NAME_T*)pCmd;
			P_DBS4WEBUSER_JOIN_ALLIANCE_NAME_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_JOIN_ALLIANCE_NAME_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nAllianceID	= 0;
			WH_STRNCPY0(RplCmd.szAllianceName, pReq->szAllianceName);
			RplCmd.nRst			= STC_GAMECMD_OPERATE_JOIN_ALLI_NAME_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join_alliance_name,DBERROR,0x%"WHINT64PRFX"X,%s", pReq->nAccountID, pReq->szAllianceName);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_JOIN_ALLI_NAME_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL join_alliance_name(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, pReq->szAllianceName);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join_alliance_name,SQLERROR,0x%"WHINT64PRFX"X,%s", pReq->nAccountID, pReq->szAllianceName);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_JOIN_ALLI_NAME_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"join_alliance_name,0x%"WHINT64PRFX"X,%s,%d,0x%"WHINT64PRFX"X"
							, pReq->nAccountID, pReq->szAllianceName, RplCmd.nRst, RplCmd.nAllianceID);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"join_alliance_name,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%s", pReq->nAccountID, pReq->szAllianceName);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_JOIN_ALLI_NAME_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_MY_ALLIANCE_JOIN_REQ:
		{
			P_DBS4WEB_GET_MY_ALLIANCE_JOIN_T*		pReq	= (P_DBS4WEB_GET_MY_ALLIANCE_JOIN_T*)pCmd;
			P_DBS4WEBUSER_GET_MY_ALLIANCE_JOIN_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_MY_ALLIANCE_JOIN_RPL;
			RplCmd.nAllianceID	= 0;
			RplCmd.szAllianceName[0]	= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_my_alliance_join,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT a.alliance_id,a.name FROM alliances a JOIN alliance_join_events e ON a.alliance_id=e.alliance_id WHERE e.account_id=0x%"WHINT64PRFX"X"
					, pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_my_alliance_join,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nAllianceID	= q.GetVal_64();
						int*	pLengths	= q.FetchLengths();
						WH_STRNCPY0(RplCmd.szAllianceName, q.GetStr());
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get_my_alliance_join,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%s"
							, pReq->nAccountID, RplCmd.nAllianceID, RplCmd.szAllianceName);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"get_my_alliance_join,NO EVENT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst	= STC_GAMECMD_GET_MY_ALLIANCE_JOIN_EVENT_T::RST_NO_EVENT;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_LOAD_EXCEL_ALLIANCE_CONGRESS_REQ:
		{
			P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_LOAD_EXCEL_ALLIANCE_CONGRESS_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T::RST_OK;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_excel_text,DBERROR");
				RplCmd.nRst		= P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT level,development,member_num,vice_leader_num,manager_num FROM excel_alliance_congress ORDER BY level ASC");
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_excel_text,SQLERROR");
					RplCmd.nRst		= P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T) + RplCmd.nNum*sizeof(ExcelAllianceCongress));
					P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T*	pRplCmd	= (P_DBS4WEBUSER_LOAD_EXCEL_ALLIANCE_CONGRESS_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					ExcelAllianceCongress*	pExcel	= (ExcelAllianceCongress*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pExcel->nLevel			= q.GetVal_32();
						pExcel->nDevelopment	= q.GetVal_32();
						pExcel->nMemberNum		= q.GetVal_32();
						pExcel->nViceLeaderNum	= q.GetVal_32();
						pExcel->nManagerNum		= q.GetVal_32();

						pExcel++;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					return 0;
				}
			}
		}
		break;
	case CMDID_SET_ALLI_INTRO_REQ:
		{
			P_DBS4WEB_SET_ALLI_INTRO_T*		pReq	= (P_DBS4WEB_SET_ALLI_INTRO_T*)pCmd;
			P_DBS4WEBUSER_SET_ALLI_INTRO_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_SET_ALLI_INTRO_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nAllianceID	= pReq->nAllianceID;
			WH_STRNCPY0(RplCmd.szIntroduction, pReq->szIntroduction);
			RplCmd.nRst			= STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_alliance_introduction,DBERROR");
				RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
//				q.SpawnQuery("CALL set_alliance_introduction(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, pReq->szIntroduction);
				q.SpawnQuery("CALL set_alliance_introduction(0x%"WHINT64PRFX"X,", pReq->nAccountID);
				q.StrMove("'");
				q.BinaryToString(strlen(pReq->szIntroduction)+1, pReq->szIntroduction);
				q.StrMove("')");

				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_alliance_introduction,SQLERROR,%d,%s", q.GetErrno(), q.GetError());
					RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_alliance_introduction,SQLERROR,NO RESULT");
						RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_INTRO_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
		/*
	case CMDID_GET_OTHER_GOLDORE_POSITION_REQ:
		{
			P_DBS4WEB_GET_OTHER_GOLDORE_POSITION_T*	pReq	= (P_DBS4WEB_GET_OTHER_GOLDORE_POSITION_T*)pCmd;
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_POSITION_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_OTHER_GOLDORE_POSITION_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_position,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT auto_id,level FROM buildings WHERE excel_id=11 AND account_id=0x%"WHINT64PRFX"X", pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_position,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_POSITION_T) + RplCmd.nNum*sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T::GoldorePositionUnit));
						P_DBS4WEBUSER_GET_OTHER_GOLDORE_POSITION_T*	pRplCmd	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_POSITION_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T::GoldorePositionUnit*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_POSITION_T::GoldorePositionUnit*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nAutoID		= q.GetVal_32();
							pUnit->nLevel		= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_GET_OTHER_GOLDORE_PRODUCTION_EVENT_REQ:
		{
			P_DBS4WEB_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*	pReq	= (P_DBS4WEB_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*)pCmd;
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_OTHER_GOLDORE_PRODUCTION_EVENT_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_production_event,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT auto_id,production FROM production_events WHERE type=%d AND account_id=0x%"WHINT64PRFX"X", te_subtype_production_gold, pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_production_event,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T) + RplCmd.nNum*sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T::GoldoreProductionEvent));
						P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*	pRplCmd	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T::GoldoreProductionEvent*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_EVENT_T::GoldoreProductionEvent*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nAutoID		= q.GetVal_32();
							pUnit->nProduction	= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_GET_OTHER_GOLDORE_PRODUCTION_TE_REQ:
		{
			P_DBS4WEB_GET_OTHER_GOLDORE_PRODUCTION_TE_T*	pReq	= (P_DBS4WEB_GET_OTHER_GOLDORE_PRODUCTION_TE_T*)pCmd;
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_OTHER_GOLDORE_PRODUCTION_TE_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_production_te,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT p.auto_id,t.begin_time,t.end_time FROM production_time_events p JOIN time_events t ON p.event_id=t.event_id WHERE p.type=%d AND p.account_id=0x%"WHINT64PRFX"X", te_subtype_production_gold, pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_production_te,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_TE_T) + RplCmd.nNum*sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T::GoldoreProductionTimeEvent));
						P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_TE_T*	pRplCmd	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_PRODUCTION_TE_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T::GoldoreProductionTimeEvent*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_PRODUCTION_TE_T::GoldoreProductionTimeEvent*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nAutoID		= q.GetVal_32();
							pUnit->nBeginTime	= q.GetVal_32();
							pUnit->nEndTime		= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_GET_OTHER_GOLDORE_FETCH_INFO_REQ:
		{
			P_DBS4WEB_GET_OTHER_GOLDORE_FETCH_INFO_T*	pReq	= (P_DBS4WEB_GET_OTHER_GOLDORE_FETCH_INFO_T*)pCmd;
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_FETCH_INFO_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_OTHER_GOLDORE_FETCH_INFO_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_fetch_info,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL get_other_goldore_fetch_info(0x%"WHINT64PRFX"X, 0x%"WHINT64PRFX"X)", pReq->nMyAccountID, pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_fetch_info,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T::RST_DB_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_FETCH_INFO_T) + RplCmd.nNum*sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T::FetchUnit));
						P_DBS4WEBUSER_GET_OTHER_GOLDORE_FETCH_INFO_T*	pRplCmd	= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_FETCH_INFO_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T::FetchUnit*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_FETCH_INFO_T::FetchUnit*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nAutoID		= q.GetVal_32();
							pUnit->bCanFetch	= (bool)q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_GET_OTHER_CHRISTMAS_TREE_INFO_REQ:
		{
			P_DBS4WEB_GET_OTHER_CHRISTMAS_TREE_INFO_T*	pReq	= (P_DBS4WEB_GET_OTHER_CHRISTMAS_TREE_INFO_T*)pCmd;
			P_DBS4WEBUSER_GET_OTHER_CHRISTMAS_TREE_INFO_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_OTHER_CHRISTMAS_TREE_INFO_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nState		= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::state_none;
			RplCmd.nCountDownTime	= 0;
			RplCmd.nWateredNum		= 0;
			RplCmd.bICanWater	= false;
			RplCmd.nRst			= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_christmas_tree_info,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL get_other_christmas_tree_info(0x%"WHINT64PRFX"X, 0x%"WHINT64PRFX"X)", pReq->nMyAccountID, pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_christmas_tree_info,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _can_water,_ripe,_begin_time,_end_time,_watered_num
						RplCmd.bICanWater		= (bool)q.GetVal_32();
						bool bRipe				= (bool)q.GetVal_32();
						unsigned int	nBeginTime		= q.GetVal_32();
						RplCmd.nCountDownTime	= q.GetVal_32();
						RplCmd.nWateredNum		= q.GetVal_32();
						if (bRipe)
						{
							RplCmd.nState		= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::state_ripe;
						}
						else if (RplCmd.bICanWater)
						{
							RplCmd.nState		= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::state_can_watering;
						}
						else
						{
							if (nBeginTime != 0)
							{
								RplCmd.nState		= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::state_ready_ripe;
							}
							else
							{
								RplCmd.nState		= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::state_can_watering;
							}
						}
					}
					else
					{
						RplCmd.nRst				= STC_GAMECMD_GET_OTHER_CHRISTMAS_TREE_INFO_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
		*/
	case CMDID_CHECK_DRAW_LOTTERY_REQ:
		{
			P_DBS4WEB_CHECK_DRAW_LOTTERY_T*	pReq	= (P_DBS4WEB_CHECK_DRAW_LOTTERY_T*)pCmd;
			P_DBS4WEBUSER_CHECK_DRAW_LOTTERY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CHECK_DRAW_LOTTERY_RPL;
			RplCmd.nPrice		= 0;
			RplCmd.nType		= pReq->nType;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T::RST_OK;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nAllianceID	= pReq->nAllianceID;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"check_draw_lottery,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL check_draw_lottery(0x%"WHINT64PRFX"X, %d)", pReq->nAccountID, pReq->nType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"check_draw_lottery,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_price
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nPrice	= q.GetVal_32();

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"check_draw_lottery,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pReq->nAccountID, pReq->nType, RplCmd.nRst, RplCmd.nPrice);
					}
					else
					{
						RplCmd.nRst	= STC_GAMECMD_OPERATE_CHECK_DRAW_LOTTERY_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"check_draw_lottery,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_FETCH_LOTTERY_REQ:
		{
			P_DBS4WEB_FETCH_LOTTERY_T*	pReq	= (P_DBS4WEB_FETCH_LOTTERY_T*)pCmd;
			P_DBS4WEBUSER_FETCH_LOTTERY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_FETCH_LOTTERY_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nType		= pReq->nType;
			RplCmd.nData		= pReq->nData;
			RplCmd.nItemID		= 0;
			RplCmd.bTopReward	= pReq->bTopReward;
			RplCmd.bNotifyWorld	= false;
			RplCmd.szItemName[0]= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_FETCH_LOTTERY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_lottery,DBERROR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nType, pReq->nData);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_FETCH_LOTTERY_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL fetch_lottery(0x%"WHINT64PRFX"X, %d, %d)", pReq->nAccountID, pReq->nType, pReq->nData);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_lottery,SQLERROR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nType, pReq->nData);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_FETCH_LOTTERY_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_item_name,@_value,@_value>=@_top_log_value,@_value>=@_notify_world_value,@vItem
						int*	pLengths	= q.FetchLengths();
						char	szItemIDs[512];
						RplCmd.nRst			= q.GetVal_32();
						WH_STRNCPY0(RplCmd.szItemName, q.GetStr());
						RplCmd.nValue		= q.GetVal_32();
						RplCmd.bTopReward	= (bool)q.GetVal_32();
						RplCmd.bNotifyWorld	= (bool)q.GetVal_32();
						memcpy(szItemIDs, q.GetStr(), pLengths[5]);
						szItemIDs[pLengths[5]]	= 0;
						while (wh_strsplit("Ia", szItemIDs, ",", &RplCmd.nItemID, szItemIDs) == 2)
						{
							if (RplCmd.nItemID != 0)
							{
								break;
							}
						}
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"fetch_lottery,0x%"WHINT64PRFX"X,%d,%d,%d,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nType, pReq->nData, RplCmd.nRst, RplCmd.nItemID);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_lottery,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, pReq->nType, pReq->nData);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_FETCH_LOTTERY_T::RST_SQL_ERR;
					}
				}
			}

			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
			return 0;
		}
		break;
	case CMDID_GET_CHRISTMAS_TREE_REQ:
		{
			P_DBS4WEB_GET_CHRISTMAS_TREE_T*	pReq	= (P_DBS4WEB_GET_CHRISTMAS_TREE_T*)pCmd;
			P_DBS4WEBUSER_GET_CHRISTMAS_TREE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_CHRISTMAS_TREE_RPL;
			RplCmd.bICanWater	= false;
			RplCmd.nCanWaterCountDownTime	= 0;
			RplCmd.nCountDownTime			= 0;
			RplCmd.nState		= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::RST_OK;
			RplCmd.nWateredNum	= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_christmas_tree,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL get_christmas_tree_info(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_christmas_tree,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _can_water,_ripe,_begin_time,_end_time,_watered_num,_time(可以浇水的倒计时)
						RplCmd.bICanWater		= (bool)q.GetVal_32();
						bool bRipe				= (bool)q.GetVal_32();
						unsigned int	nBeginTime		= q.GetVal_32();
						RplCmd.nCountDownTime	= q.GetVal_32();
						RplCmd.nWateredNum		= q.GetVal_32();
						RplCmd.nCanWaterCountDownTime	= q.GetVal_32();
						if (bRipe)
						{
							RplCmd.nState		= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::state_ripe;
						}
						else if (RplCmd.bICanWater)
						{
							RplCmd.nState		= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::state_can_watering;
						}
						else
						{
							if (nBeginTime != 0)
							{
								RplCmd.nState		= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::state_ready_ripe;
							}
							else
							{
								RplCmd.nState		= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::state_can_watering;
							}
						}
					}
					else
					{
						RplCmd.nRst				= STC_GAMECMD_GET_CHRISTMAS_TREE_INFO_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_OTHER_GOLDORE_INFO_REQ:
		{
			P_DBS4WEB_GET_OTHER_GOLDORE_INFO_T*	pReq	= (P_DBS4WEB_GET_OTHER_GOLDORE_INFO_T*)pCmd;
			P_DBS4WEBUSER_GET_OTHER_GOLDORE_INFO_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_OTHER_GOLDORE_INFO_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nCTState		= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::ChristmasTreeInfo::state_none;
			RplCmd.nCTCountDownTime	= 0;		
			RplCmd.nCTWateredNum	= 0;		
			RplCmd.bCTICanWater		= false;
			RplCmd.nRst			= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_info,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL get_other_goldore_info(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pReq->nAccountID, pReq->nMyAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_goldore_info,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					P_DBS4WEBUSER_GET_OTHER_GOLDORE_INFO_T*	pRplCmd		= NULL;
					int	nRplCmdSize										= 0;
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_OTHER_GOLDORE_INFO_T) + RplCmd.nNum*sizeof(STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::GoldoreInfo));
						pRplCmd		= (P_DBS4WEBUSER_GET_OTHER_GOLDORE_INFO_T*)vectRawBuf.getbuf();
						nRplCmdSize	= vectRawBuf.size();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::GoldoreInfo*	pUnit	= (STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::GoldoreInfo*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// auto_id,level,count_down_time,state
							pUnit->nAutoID	= q.GetVal_32();
							pUnit->nLevel	= q.GetVal_32();
							pUnit->nCountDownTime	= q.GetVal_32();
							pUnit->nState	= q.GetVal_32();

							pUnit++;
						}
					}
					else
					{
						pRplCmd		= &RplCmd;
						nRplCmdSize	= sizeof(RplCmd);
					}

					// 圣诞树相关
					q.FreeResult();
					q.SpawnQuery("CALL get_other_christmas_tree_info(0x%"WHINT64PRFX"X, 0x%"WHINT64PRFX"X)", pReq->nMyAccountID, pReq->nAccountID);
					int	nPreRet		= 0;
					q.ExecuteSPWithResult(nPreRet, false);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_other_christmas_tree_info,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
						pRplCmd->nRst		= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, pRplCmd, nRplCmdSize);
						return 0;
					}
					else
					{
						if (q.FetchRow())
						{
							// _can_water,_ripe,_begin_time,_end_time,_watered_num
							pRplCmd->bCTICanWater		= (bool)q.GetVal_32();
							bool bRipe				= (bool)q.GetVal_32();
							unsigned int	nBeginTime		= q.GetVal_32();
							pRplCmd->nCTCountDownTime	= q.GetVal_32();
							pRplCmd->nCTWateredNum		= q.GetVal_32();
							if (bRipe)
							{
								pRplCmd->nCTState		= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::ChristmasTreeInfo::state_ripe;
							}
							else if (pRplCmd->bCTICanWater)
							{
								pRplCmd->nCTState		= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::ChristmasTreeInfo::state_can_watering;
							}
							else
							{
								if (nBeginTime != 0)
								{
									pRplCmd->nCTState		= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::ChristmasTreeInfo::state_ready_ripe;
								}
								else
								{
									pRplCmd->nCTState		= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::ChristmasTreeInfo::state_can_watering;
								}
							}
						}
						else
						{
							pRplCmd->nRst				= STC_GAMECMD_GET_OTHER_GOLDORE_INFO_T::RST_SQL_ERR;
						}
						SendMsg_MT(pCDCmd->nConnecterID, pRplCmd, nRplCmdSize);
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_GOLDORE_SMP_INFO_ALL_REQ:
		{
			P_DBS4WEB_GET_GOLDORE_SMP_INFO_ALL_T*	pReq	= (P_DBS4WEB_GET_GOLDORE_SMP_INFO_ALL_T*)pCmd;
			P_DBS4WEBUSER_GET_GOLDORE_SMP_INFO_ALL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_GOLDORE_SMP_INFO_ALL_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_goldore_smp_info_all,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				tty_id_t*	pID	= (tty_id_t*)wh_getptrnexttoptr(pReq);
				// 0xFFFFFFFFFFFFFFFF->18446744073709551615,长20个字符,使用十六进制数据库会出错
				// 一个tty_id_t转换成十进制字符串长度为(20+1)=21,1->*分隔符及结尾'\0'
				char*	pBuf	= new char[pReq->nNum*21];
				int		nLength	= 0;
				nLength			= sprintf(pBuf, "%"WHINT64PRFX"u", *pID);
				++pID;
				for (int i=1; i<pReq->nNum; i++)
				{
					nLength		+= sprintf(pBuf+nLength, "*%"WHINT64PRFX"u", *pID);
					++pID;
				}
				pBuf[nLength]	= 0;
				q.SpawnQuery("CALL get_others_goldore_smp_info(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, pBuf);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet, false);
				delete[] pBuf;
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_goldore_smp_info_all,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_GOLDORE_SMP_INFO_ALL_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_GOLDORE_SMP_INFO_ALL_T) + RplCmd.nNum*sizeof(GoldoreSmpInfo));
						P_DBS4WEBUSER_GET_GOLDORE_SMP_INFO_ALL_T*	pRplCmd	= (P_DBS4WEBUSER_GET_GOLDORE_SMP_INFO_ALL_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						GoldoreSmpInfo*	pUnit	= (GoldoreSmpInfo*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// account_id,can_water,can_fetch_gold
							pUnit->nAccountID				= q.GetVal_64();
							pUnit->bCanWaterChristmasTree	= (bool)q.GetVal_32();
							pUnit->bCanFetchGold			= (bool)q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_ALLIANCE_TRADE_INFO_REQ:
		{
			P_DBS4WEB_GET_ALLIANCE_TRADE_INFO_T*	pReq	= (P_DBS4WEB_GET_ALLIANCE_TRADE_INFO_T*)pCmd;
			P_DBS4WEBUSER_GET_ALLIANCE_TRADE_INFO_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_GET_ALLIANCE_TRADE_INFO_RPL;
			RplCmd.nCountDownTime	= 0;
			RplCmd.nGold			= 0;
			RplCmd.nTradeNum		= 0;
			RplCmd.nExcelTradeNum	= 0;
			RplCmd.nExcelTradeTime	= 0;
			RplCmd.nExcelTradeGold	= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_ALLIANCE_TRADE_INFO_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_trade_info,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_TRADE_INFO_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL get_alliance_trade_info(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_trade_info,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_ALLIANCE_TRADE_INFO_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_gold,IF(_time_now>=@_end_time,0,@_end_time-_time_now),@_trade_num,_excel_trade_num,_excel_trade_gold,_excel_trade_time
						RplCmd.nRst				= q.GetVal_32();
						RplCmd.nGold			= q.GetVal_32();
						RplCmd.nCountDownTime	= q.GetVal_32();
						RplCmd.nTradeNum		= q.GetVal_32();
						RplCmd.nExcelTradeNum	= q.GetVal_32();
						RplCmd.nExcelTradeGold	= q.GetVal_32();
						RplCmd.nExcelTradeTime	= q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_alliance_trade_info,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst	= STC_GAMECMD_GET_ALLIANCE_TRADE_INFO_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_ALLIANCE_TRADE_REQ:
		{
			P_DBS4WEB_ALLIANCE_TRADE_T*	pReq	= (P_DBS4WEB_ALLIANCE_TRADE_T*)pCmd;
			P_DBS4WEBUSER_ALLIANCE_TRADE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLIANCE_TRADE_RPL;
			RplCmd.nCountDownTime	= 0;
			RplCmd.nGold		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_ALLIANCE_TRADE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"alliance_trade,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLIANCE_TRADE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL alliance_trade(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"alliance_trade,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLIANCE_TRADE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_event_id,_gold,_alliance_id,_trade_time,_trade_num
						RplCmd.nRst	= q.GetVal_32();
						tty_id_t	nEventID	= q.GetVal_64();
						RplCmd.nGold= q.GetVal_32();
						tty_id_t	nAllianceID	= q.GetVal_64();
						RplCmd.nCountDownTime	= q.GetVal_32();
						int	nTradeNum			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"alliance_trade,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d,%d"
							, pReq->nAccountID, RplCmd.nRst, nEventID, RplCmd.nGold, nAllianceID, RplCmd.nCountDownTime, nTradeNum);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"alliance_trade,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLIANCE_TRADE_T::RST_SQL_ERR;
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_DEAL_ALLIANCE_TREADE_TE_REQ:
		{
			P_DBS4WEB_DEAL_ALLIANCE_TREADE_TE_T*	pReq	= (P_DBS4WEB_DEAL_ALLIANCE_TREADE_TE_T*)pCmd;
			P_DBS4WEBUSER_DEAL_ALLIANCE_TREADE_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_DEAL_ALLIANCE_TREADE_TE_RPL;
			RplCmd.nAccountID	= 0;
			RplCmd.nGold		= 0;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_alliance_trade_te,DBERROR,0x%"WHINT64PRFX"X", pReq->nEventID);
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL deal_alliance_trade_event(0x%"WHINT64PRFX"X)", pReq->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_alliance_trade_te,SQLERROR,0x%"WHINT64PRFX"X", pReq->nEventID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_account_id,@_gold
						int	nRst			= q.GetVal_32();
						RplCmd.nAccountID	= q.GetVal_64();
						RplCmd.nGold		= q.GetVal_32();
						if (nRst == 0)
						{
							SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						}
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal_alliance_trade_te,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,%d"
							, pReq->nEventID, nRst, RplCmd.nAccountID, RplCmd.nGold);
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_alliance_trade_te,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nEventID);
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_ENEMY_LIST_REQ:
		{
			P_DBS4WEB_GET_ENEMY_LIST_T*	pReq	= (P_DBS4WEB_GET_ENEMY_LIST_T*)pCmd;
			P_DBS4WEBUSER_GET_ENEMY_LIST_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_ENEMY_LIST_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_ENEMY_LIST_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_enemy_list,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_ENEMY_LIST_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT p.peer_account_id,IFNULL(c.name,''),c.level,c.head_id FROM common_characters c RIGHT JOIN personal_relations p ON c.account_id=p.peer_account_id WHERE p.account_id=0x%"WHINT64PRFX"X AND p.relation_type=%d"
					, pReq->nAccountID, pr_type_enemy);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_enemy_list,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_ENEMY_LIST_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_ENEMY_LIST_T) + RplCmd.nNum*sizeof(EnemyUnit));
						P_DBS4WEBUSER_GET_ENEMY_LIST_T*	pRplCmd	= (P_DBS4WEBUSER_GET_ENEMY_LIST_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						EnemyUnit*	pUnit	= (EnemyUnit*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nAccountID	= q.GetVal_64();
							WH_STRNCPY0(pUnit->szName, q.GetStr());
							pUnit->nLevel		= q.GetVal_32();
							pUnit->nHeadID		= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_DELETE_ENEMY_REQ:
		{
			P_DBS4WEB_DELETE_ENEMY_T*	pReq	= (P_DBS4WEB_DELETE_ENEMY_T*)pCmd;
			P_DBS4WEBUSER_DELETE_ENEMY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_DELETE_ENEMY_RPL;
			RplCmd.nEnemyID		= pReq->nEnemyID;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_DELETE_ENEMY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"delete_enemy,DBERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nEnemyID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_DELETE_ENEMY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("DELETE FROM personal_relations WHERE account_id=0x%"WHINT64PRFX"X AND peer_account_id=0x%"WHINT64PRFX"X AND relation_type=%d"
					, pReq->nAccountID, pReq->nEnemyID, pr_type_enemy);
				int	nPreRet		= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"delete_enemy,SQLERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nEnemyID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_DELETE_ENEMY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_ADD_ENEMY_REQ:
		{
			P_DBS4WEB_ADD_ENEMY_T*	pReq		= (P_DBS4WEB_ADD_ENEMY_T*)pCmd;
			P_DBS4WEBUSER_ADD_ENEMY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_ENEMY_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nEnemyID		= pReq->nEnemyID;
			RplCmd.nRst			= 0;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_enemy,DBERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nEnemyID);
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_enemy(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pReq->nAccountID, pReq->nEnemyID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_enemy,SQLERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nEnemyID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_name,_level,_head_id
						RplCmd.nRst			= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_enemy,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nEnemyID);
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_REFRESH_CHAR_ATB_REQ:
		{
			P_DBS4WEB_REFRESH_CHAR_ATB_T*	pReq	= (P_DBS4WEB_REFRESH_CHAR_ATB_T*)pCmd;
			vectRawBuf.resize(sizeof(P_DBS4WEBUSER_REFRESH_CHAR_ATB_T) + sizeof(CharAtb));
			P_DBS4WEBUSER_REFRESH_CHAR_ATB_T*	pRplCmd	= (P_DBS4WEBUSER_REFRESH_CHAR_ATB_T*)vectRawBuf.getbuf();
			pRplCmd->nCmd		= P_DBS4WEB_RPL_CMD;
			pRplCmd->nSubCmd	= CMDID_REFRESH_CHAR_ATB_RPL;
			pRplCmd->bSimple	= pReq->bSimple;
			CharAtb*	pCharAtb	= (CharAtb*)wh_getptrnexttoptr(pRplCmd);
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refresh_char_atb,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				return 0;
			}
			else
			{	
				q.SpawnQuery("CALL get_char_atb(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRst		= 0;
				q.ExecuteSPWithResult(nPreRst);
				if (nPreRst != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refresh_char_atb,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						pCharAtb->nAccountID		= pReq->nAccountID;
						WH_STRNCPY0(pCharAtb->szName, q.GetStr());
						pCharAtb->nLastLoginTime	= q.GetVal_32();
						pCharAtb->nLastLogoutTime	= q.GetVal_32();
						pCharAtb->nExp				= q.GetVal_32();
						pCharAtb->nLevel			= q.GetVal_32();
						pCharAtb->nDiamond			= q.GetVal_32();
						pCharAtb->nCrystal			= q.GetVal_32();
						pCharAtb->nGold				= q.GetVal_32();
						pCharAtb->nVip				= q.GetVal_32();
						pCharAtb->nSex				= q.GetVal_32();
						pCharAtb->nHeadID			= q.GetVal_32();
						pCharAtb->nPosX				= q.GetVal_32();
						pCharAtb->nPosY				= q.GetVal_32();
						pCharAtb->nDevelopment		= q.GetVal_32();
						pCharAtb->nPopulation		= q.GetVal_32();
						pCharAtb->nBuildNum			= q.GetVal_32();
						pCharAtb->nAddedBuildNum	= q.GetVal_32();
						pCharAtb->nFreeDrawLotteryNum	= q.GetVal_32();

						pCharAtb->nAllianceID		= q.GetVal_64();
						WH_STRNCPY0(pCharAtb->szAllianceName, q.GetStr());

						pCharAtb->nDrug				= q.GetVal_32();
						pCharAtb->nTradeNum			= q.GetVal_32();

						WH_STRNCPY0(pCharAtb->szSignature, q.GetStr());
						pCharAtb->nChangeNameNum	= q.GetVal_32();

						pCharAtb->nProtectTimeEndTick		= q.GetVal_32();
						pCharAtb->nAddBuildNumTimeEndTick	= q.GetVal_32();
						unsigned int nTimeNow		= wh_time();
						pCharAtb->nProtectTime		= nTimeNow>pCharAtb->nProtectTimeEndTick?0:pCharAtb->nProtectTimeEndTick-nTimeNow;
						pCharAtb->nAddBuildNumTime	= nTimeNow>pCharAtb->nAddBuildNumTimeEndTick?0:pCharAtb->nAddBuildNumTimeEndTick-nTimeNow;

						pCharAtb->nTotalBuildNum	= q.GetVal_32();
						pCharAtb->nNotificationID	= q.GetVal_32();
						pCharAtb->nAllianceDrawLotteryNum	= q.GetVal_32();
						pCharAtb->nTotalDiamond		= q.GetVal_32();
						pCharAtb->nInstanceWangzhe	= q.GetVal_32();
						pCharAtb->nInstanceZhengzhan	= q.GetVal_32();
						pCharAtb->nCup				= q.GetVal_32();
						pCharAtb->bBinded			= (bool)q.GetVal_32();
						pCharAtb->bVipDisplay			= (bool)q.GetVal_32();

						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refresh_char_atb,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_ADD_ALLIANCE_DEV_REQ:
		{
			P_DBS4WEB_ADD_ALLIANCE_DEV_T*	pReq	= (P_DBS4WEB_ADD_ALLIANCE_DEV_T*)pCmd;
			P_DBS4WEBUSER_ADD_ALLIANCE_DEV_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_ALLIANCE_DEV_RPL;
			RplCmd.nAllianceID			= pReq->nAllianceID;
			RplCmd.nDeltaDevelopment	= pReq->nDeltaDevelopment;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_dev,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAllianceID, pReq->nDeltaDevelopment);
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL add_alliance_development(0x%"WHINT64PRFX"X, %d)", pReq->nAllianceID, pReq->nDeltaDevelopment);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_dev,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAllianceID, pReq->nDeltaDevelopment);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						int	nRst	= q.GetVal_32();
						if (nRst == 0)
						{
							SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_alliance_dev,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nAllianceID, pReq->nDeltaDevelopment);
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_LOAD_RANK_LIST_REQ:
		{
			P_DBS4WEB_LOAD_RANK_LIST_T*	pReq		= (P_DBS4WEB_LOAD_RANK_LIST_T*)pCmd;
			P_DBS4WEBUSER_LOAD_RANK_LIST_T		RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_LOAD_RANK_LIST_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nType		= pReq->nType;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_rank_list,DBERROR,%d", pReq->nType);
				return 0;
			}
			else
			{
				q.SpawnQuery("call get_rank_data(%u)", pReq->nType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"load_rank_list,SQLERROR,%d", pReq->nType);
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_LOAD_RANK_LIST_T) + RplCmd.nNum*sizeof(RankData));
						P_DBS4WEBUSER_LOAD_RANK_LIST_T*	pRplCmd	= (P_DBS4WEBUSER_LOAD_RANK_LIST_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						RankData*	pUnit	= (RankData*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nOnlyID	= q.GetVal_64();
							pUnit->nData	= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_RECHARGE_DIAMOND_REQ:
		{
			P_DBS4WEB_RECHARGE_DIAMOND_T*	pReq	= (P_DBS4WEB_RECHARGE_DIAMOND_T*)pCmd;
			P_DBS4WEBUSER_RECHARGE_DIAMOND_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_RECHARGE_DIAMOND_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nAddedDiamond= pReq->nAddedDiamond;
			RplCmd.nGiftDiamond	= 0;
			RplCmd.nVip			= 0;
			RplCmd.bFromGS		= pReq->bFromGS;
			RplCmd.nRst			= P_DBS4WEBUSER_RECHARGE_DIAMOND_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"recharge_diamond,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nAddedDiamond);
				RplCmd.nRst		= P_DBS4WEBUSER_RECHARGE_DIAMOND_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL recharge_diamond(0x%"WHINT64PRFX"X,%d,1,@_vip)", pReq->nAccountID, pReq->nAddedDiamond);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"recharge_diamond,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nAddedDiamond);
					RplCmd.nRst		= P_DBS4WEBUSER_RECHARGE_DIAMOND_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_vip,_gift_diamond
						RplCmd.nRst	= q.GetVal_32();
						RplCmd.nVip	= q.GetVal_32();
						RplCmd.nGiftDiamond	= q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"recharge_diamond,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nAddedDiamond);
						RplCmd.nRst	= P_DBS4WEBUSER_RECHARGE_DIAMOND_T::RST_SQL_ERR;
					}
				}
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
		}
		break;
	case CMDID_WRITE_CHAR_SIGNATURE_REQ:
		{
			P_DBS4WEB_WRITE_CHAR_SIGNATURE_T*	pReq	= (P_DBS4WEB_WRITE_CHAR_SIGNATURE_T*)pCmd;
			P_DBS4WEBUSER_WRITE_CHAR_SIGNATURE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_WRITE_CHAR_SIGNATURE_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			WH_STRNCPY0(RplCmd.szSignature, pReq->szSignature);
			RplCmd.nRst			= STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"write_signature,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
//				q.SpawnQuery("CALL write_signature(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, pReq->szSignature);
				q.SpawnQuery("CALL write_signature(0x%"WHINT64PRFX"X,", pReq->nAccountID);
				q.StrMove("'");
				q.BinaryToString(strlen(pReq->szSignature)+1, pReq->szSignature);
				q.StrMove("')");

				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"write_signature,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_WRITE_SIGNATURE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_GET_INSTANCE_ID_REQ:
		{
			P_DBS4WEB_GET_INSTANCE_ID_T*	pReq	= (P_DBS4WEB_GET_INSTANCE_ID_T*)pCmd;
			P_DBS4WEBUSER_GET_INSTANCE_ID_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_INSTANCE_ID_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nExcelID		= pReq->nExcelID;
			RplCmd.nInstanceID	= 0;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_instance,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID);
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT instance_id FROM instance_player WHERE account_id=0x%"WHINT64PRFX"X AND excel_id=%d", pReq->nAccountID, pReq->nExcelID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_instance,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nExcelID);
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nInstanceID	= q.GetVal_64();
					}
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
			}
		}
		break;
	case CMDID_CHANGE_NAME_REQ:
		{
			P_DBS4WEB_CHANGE_NAME_T*	pReq	= (P_DBS4WEB_CHANGE_NAME_T*)pCmd;
			P_DBS4WEBUSER_CHANGE_NAME_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CHANGE_NAME_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nCostDiamond	= 0;
			WH_STRNCPY0(RplCmd.szNewName, pReq->szNewName);
			RplCmd.szOldName[0]	= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CHANGE_NAME_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_name,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_CHANGE_NAME_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL change_name(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, pReq->szNewName);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_name,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_CHANGE_NAME_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_old_name,@_cost_diamond;
						RplCmd.nRst			= q.GetVal_32();
						WH_STRNCPY0(RplCmd.szOldName, q.GetStr());
						RplCmd.nCostDiamond	= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_name,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_CHANGE_NAME_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_MOVE_POSITION_REQ:
		{
			P_DBS4WEB_MOVE_POSITION_T*	pReq	= (P_DBS4WEB_MOVE_POSITION_T*)pCmd;
			P_DBS4WEBUSER_MOVE_POSITION_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_MOVE_POSITION_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nSrcPosX		= pReq->nSrcPosX;
			RplCmd.nSrcPosY		= pReq->nSrcPosY;
			RplCmd.nDstPosX		= pReq->nDstPosX;
			RplCmd.nDstPosY		= pReq->nDstPosY;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_MOVE_CITY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"move_position,DBERROR,0x%"WHINT64PRFX"X,%d,%d,%d,%d"
					, pReq->nAccountID, pReq->nSrcPosX, pReq->nSrcPosY, pReq->nDstPosX, pReq->nDstPosY);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_MOVE_CITY_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL move_position(0x%"WHINT64PRFX"X, %d, %d)", pReq->nAccountID, pReq->nDstPosX, pReq->nDstPosY);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"move_position,SQLERROR,0x%"WHINT64PRFX"X,%d,%d,%d,%d"
						, pReq->nAccountID, pReq->nSrcPosX, pReq->nSrcPosY, pReq->nDstPosX, pReq->nDstPosY);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_MOVE_CITY_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result;
						RplCmd.nRst	= q.GetVal_32();
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"move_position,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d,%d,%d"
							, pReq->nAccountID, pReq->nSrcPosX, pReq->nSrcPosY, pReq->nDstPosX, pReq->nDstPosY);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_MOVE_CITY_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_PAY_SERIAL_REQ:
		{
			P_DBS4WEB_GET_PAY_SERIAL_T*	pReq	= (P_DBS4WEB_GET_PAY_SERIAL_T*)pCmd;
			P_DBS4WEBUSER_GET_PAY_SERIAL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_PAY_SERIAL_RPL;
			RplCmd.nRst			= STC_GAMECMD_GET_PAY_SERIAL_T::RST_OK;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.szPaySerial[0]	= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_pay_serial,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_PAY_SERIAL_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT gen91PaySerial(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_pay_serial,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_PAY_SERIAL_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						memcpy(RplCmd.szPaySerial, q.GetStr(), sizeof(RplCmd.szPaySerial));
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_pay_serial,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_GET_PAY_SERIAL_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_GET_PAY_LIST_REQ:
		{
			P_DBS4WEB_GET_PAY_LIST_T*	pReq	= (P_DBS4WEB_GET_PAY_LIST_T*)pCmd;
			P_DBS4WEBUSER_GET_PAY_LIST_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_PAY_LIST_RPL;
			RplCmd.nNum			= 0;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_pay_list,DBERROR");
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT serial FROM pay91_procqueue", false);
				int	nPreRet		= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_pay_list,SQLERROR");
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_PAY_LIST_T) + RplCmd.nNum*TTY_PAY_SERIAL_LEN);
						P_DBS4WEBUSER_GET_PAY_LIST_T*	pRplCmd	= (P_DBS4WEBUSER_GET_PAY_LIST_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						void*	pSerial		= wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							memcpy(pSerial, q.GetStr(), TTY_PAY_SERIAL_LEN);
							pSerial			= wh_getoffsetaddr(pSerial, TTY_PAY_SERIAL_LEN);
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;
	case CMDID_DEAL_ONE_PAY_REQ:
		{
			P_DBS4WEB_DEAL_ONE_PAY_T*	pReq	= (P_DBS4WEB_DEAL_ONE_PAY_T*)pCmd;
// 			P_DBS4WEBUSER_DEAL_ONE_PAY_T	RplCmd;
// 			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
// 			RplCmd.nSubCmd				= CMDID_DEAL_ONE_PAY_RPL;
// 			RplCmd.nAccountID			= 0;
// 			RplCmd.nAddedDiamond		= 0;
// 			RplCmd.nVip					= 0;
			P_DBS4WEBUSER_RECHARGE_DIAMOND_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_RECHARGE_DIAMOND_RPL;
			RplCmd.bFromGS				= false;
			RplCmd.nAccountID			= 0;
			RplCmd.nAddedDiamond		= 0;
			RplCmd.nGiftDiamond			= 0;
			RplCmd.nVip					= 0;
			RplCmd.nRst					= P_DBS4WEBUSER_RECHARGE_DIAMOND_T::RST_OK;
			memset(RplCmd.nExt, 0, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_one_pay,DBERROR");
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL deal_one_91pay('%s')", pReq->szPaySerial);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_one_91pay,DBERROR,%s", pReq->szPaySerial);
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_account_id,@_GoodsCount,@_vip,@_gift_diamond
						RplCmd.nRst				= q.GetVal_32();
						RplCmd.nAccountID		= q.GetVal_64();
						RplCmd.nAddedDiamond	= q.GetVal_32();
						RplCmd.nVip				= q.GetVal_32();
						RplCmd.nGiftDiamond		= q.GetVal_32();

						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"deal_one_91pay,%s,%d,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pReq->szPaySerial, RplCmd.nRst, RplCmd.nAccountID, RplCmd.nAddedDiamond, RplCmd.nVip, RplCmd.nGiftDiamond);
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_one_91pay,SQLERROR,NO RESULT,%s", pReq->szPaySerial);
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_READ_NOTIFICATION_REQ:
		{
			P_DBS4WEB_READ_NOTIFICATION_T*	pReq	= (P_DBS4WEB_READ_NOTIFICATION_T*)pCmd;
			P_DBS4WEBUSER_READ_NOTIFICATION_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_READ_NOTIFICATION_RPL;
			RplCmd.nAccountID			= pReq->nAccountID;
			RplCmd.nNotificationID		= pReq->nNotificationID;
			RplCmd.nRst					= STC_GAMECMD_OPERATE_READ_NOTIFICATION_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
		
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"read_notification,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nNotificationID);
				RplCmd.nRst				= STC_GAMECMD_OPERATE_READ_NOTIFICATION_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("UPDATE common_characters SET notification_id=%d WHERE account_id=0x%"WHINT64PRFX"X", pReq->nNotificationID, pReq->nAccountID);
				int	nPreRet				= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"read_notification,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nNotificationID);
					RplCmd.nRst			= STC_GAMECMD_OPERATE_READ_NOTIFICATION_T::RST_SQL_ERR;
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
			return 0;
		}
		break;
	case CMDID_CHANGE_HERO_NAME_REQ:
		{
			P_DBS4WEB_CHANGE_HERO_NAME_T*	pReq	= (P_DBS4WEB_CHANGE_HERO_NAME_T*)pCmd;
			P_DBS4WEBUSER_CHANGE_HERO_NAME_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_CHANGE_HERO_NAME_RPL;
			RplCmd.nAccountID			= pReq->nAccountID;
			RplCmd.nDiamond				= 0;
			RplCmd.nRst					= STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_hero_name,DBERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
				RplCmd.nRst				= STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL change_hero_name(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,'%s')", pReq->nAccountID, pReq->nHeroID, pReq->szName);
				int	nPreRet				= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_hero_name,SQLERROR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
					RplCmd.nRst			= STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nDiamond	= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"change_hero_name,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d"
							, pReq->nAccountID, pReq->nHeroID, RplCmd.nRst, RplCmd.nDiamond);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_hero_name,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X", pReq->nAccountID, pReq->nHeroID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_CHANGE_HERO_NAME_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
			return 0;
		}
		break;
	case CMDID_READ_MAIL_ALL_REQ:
		{
			P_DBS4WEB_READ_MAIL_ALL_T*	pReq	= (P_DBS4WEB_READ_MAIL_ALL_T*)pCmd;
			P_DBS4WEBUSER_READ_MAIL_ALL_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_READ_MAIL_ALL_RPL;
			RplCmd.nAccountID			= pReq->nAccountID;
			RplCmd.nMailType			= pReq->nMailType;
			RplCmd.nRst					= STC_GAMECMD_READ_MAIL_ALL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"read_mail_all,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nMailType);
				RplCmd.nRst				= STC_GAMECMD_READ_MAIL_ALL_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("UPDATE private_mails SET readed=1 WHERE account_id=0x%"WHINT64PRFX"X AND type IN (%s) AND flag!=%d"
					, pReq->nAccountID, pReq->szTypeStr, MAIL_FLAG_REWARD);
				int	nPreRet				= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"read_mail_all,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nMailType);
					RplCmd.nRst				= STC_GAMECMD_READ_MAIL_ALL_T::RST_SQL_ERR;
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
			return 0;
		}
		break;
	case CMDID_DELETE_MAIL_ALL_REQ:
		{
			P_DBS4WEB_DELETE_MAIL_ALL_T*	pReq	= (P_DBS4WEB_DELETE_MAIL_ALL_T*)pCmd;
			P_DBS4WEBUSER_DELETE_MAIL_ALL_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_DELETE_MAIL_ALL_RPL;
			RplCmd.nAccountID			= pReq->nAccountID;
			RplCmd.nMailType			= pReq->nMailType;
			RplCmd.nRst					= STC_GAMECMD_READ_MAIL_ALL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"delete_mail_all,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nMailType);
				RplCmd.nRst				= STC_GAMECMD_READ_MAIL_ALL_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("DELETE FROM private_mails WHERE account_id=0x%"WHINT64PRFX"X AND type IN (%s) AND flag!=%d"
					, pReq->nAccountID, pReq->szTypeStr, MAIL_FLAG_REWARD);
				int	nPreRet				= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"read_mail_all,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nMailType);
					RplCmd.nRst				= STC_GAMECMD_READ_MAIL_ALL_T::RST_SQL_ERR;
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
			return 0;
		}
		break;
	case CMDID_ADD_GM_MAIL_REQ:
		{
			P_DBS4WEB_ADD_GM_MAIL_T*	pReq	= (P_DBS4WEB_ADD_GM_MAIL_T*)pCmd;
			P_DBS4WEBUSER_ADD_GM_MAIL_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_ADD_GM_MAIL_RPL;
			RplCmd.nRst					= STC_GAMECMD_OPERATE_ADD_GM_MAIL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_gm_mail,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst				= STC_GAMECMD_READ_MAIL_ALL_T::RST_DB_ERR;
			}
			else
			{
//				q.SpawnQuery("CALL add_gm_mail(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, wh_getptrnexttoptr(pReq));
				q.SpawnQuery("CALL add_gm_mail(0x%"WHINT64PRFX"X,", pReq->nAccountID);
				q.StrMove("'");
				q.BinaryToString(strlen((char*)wh_getptrnexttoptr(pReq))+1, (char*)wh_getptrnexttoptr(pReq));
				q.StrMove("')");

				int	nPreRet				= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_gm_mail,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst				= STC_GAMECMD_READ_MAIL_ALL_T::RST_SQL_ERR;
				}
				else
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"add_gm_mail,0x%"WHINT64PRFX"X", pReq->nAccountID);
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
			return 0;
		}
		break;
	case CMDID_GET_PLAYERCARD_REQ:
		{
			P_DBS4WEB_GET_PLAYERCARD_T*	pReq	= (P_DBS4WEB_GET_PLAYERCARD_T*)pCmd;
			P_DBS4WEBUSER_GET_PLAYERCARD_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_GET_PLAYERCARD_RPL;
			RplCmd.nRst					= P_DBS4WEBUSER_GET_PLAYERCARD_T::RST_OK;

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_playercard,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst				= P_DBS4WEBUSER_GET_PLAYERCARD_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL get_player_card(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet				= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_playercard,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst				= P_DBS4WEBUSER_GET_PLAYERCARD_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_PLAYERCARD_T) + sizeof(PlayerCard));
						P_DBS4WEBUSER_GET_PLAYERCARD_T*	pRplCmd	= (P_DBS4WEBUSER_GET_PLAYERCARD_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						PlayerCard*	pCard	= (PlayerCard*)wh_getptrnexttoptr(pRplCmd);
						pCard->nAccountID	= q.GetVal_64();
						WH_STRNCPY0(pCard->szName, q.GetStr());
						pCard->nLevel		= q.GetVal_32();
						pCard->nVip			= q.GetVal_32();
						pCard->nSex			= q.GetVal_32();
						pCard->nHeadID		= q.GetVal_32();
						pCard->nPosX		= q.GetVal_32();
						pCard->nPosY		= q.GetVal_32();
						pCard->nAllianceID	= q.GetVal_64();
						WH_STRNCPY0(pCard->szAllianceName, q.GetStr());
						pCard->nDevelopment	= q.GetVal_32();
						WH_STRNCPY0(pCard->szSignature, q.GetStr());
						pCard->nProtectTimeEndTick	= q.GetVal_32();
						pCard->nCityLevel	= q.GetVal_32();
						pCard->nLevelRank	= 0;
						unsigned int nTimeNow		= wh_time();
						pCard->nProtectTime		= nTimeNow>pCard->nProtectTimeEndTick?0:pCard->nProtectTimeEndTick-nTimeNow;
						pCard->nInstanceWangzhe	= q.GetVal_32();
						pCard->nInstanceZhengzhan	= q.GetVal_32();
						pCard->nCup			= q.GetVal_32();
						pCard->bVipDisplay	= (bool)q.GetVal_32();

						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_playercard,NOCHAR,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst				= P_DBS4WEBUSER_GET_PLAYERCARD_T::RST_NOCHAR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_DEAL_EXCEL_TE_REQ:
		{
			P_DBS4WEB_DEAL_EXCEL_TE_T*	pReq	= (P_DBS4WEB_DEAL_EXCEL_TE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_excel_te,DBERROR");
			}
			else
			{
				q.SpawnQuery("CALL deal_excel_time_event()");
				int	nPreRet				= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_excel_te,SQLERROR");
				}
				else
				{
					int	nNum			= q.NumRows();
					if (nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_DEAL_EXCEL_TE_T) + nNum*sizeof(P_DBS4WEBUSER_DEAL_EXCEL_TE_T::ExcelTERes));
						P_DBS4WEBUSER_DEAL_EXCEL_TE_T*	pRplCmd	= (P_DBS4WEBUSER_DEAL_EXCEL_TE_T*)vectRawBuf.getbuf();
						pRplCmd->nCmd		= P_DBS4WEB_RPL_CMD;
						pRplCmd->nSubCmd	= CMDID_DEAL_EXCEL_TE_RPL;
						pRplCmd->nNum		= nNum;
						P_DBS4WEBUSER_DEAL_EXCEL_TE_T::ExcelTERes*	pExcelTERes	= (P_DBS4WEBUSER_DEAL_EXCEL_TE_T::ExcelTERes*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pExcelTERes->nExcelID		= q.GetVal_32();
							pExcelTERes->nData			= q.GetVal_32();

							pExcelTERes++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_CHANGE_ALLIANCE_NAME_REQ:
		{
			P_DBS4WEB_CHANGE_ALLIANCE_NAME_T*	pReq	= (P_DBS4WEB_CHANGE_ALLIANCE_NAME_T*)pCmd;
			P_DBS4WEBUSER_CHANGE_ALLIANCE_NAME_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CHANGE_ALLIANCE_NAME_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nAllianceID	= 0;
			RplCmd.nDiamond		= 0;
			WH_STRNCPY0(RplCmd.szAllianceName, pReq->szAllianceName);
			RplCmd.nRst			= STC_GAMECMD_OPERATE_SET_ALLI_NAME_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_alliance_name,DBERROR,0x%"WHINT64PRFX"X,%s", pReq->nAccountID, pReq->szAllianceName);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_NAME_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL change_alliance_name(0x%"WHINT64PRFX"X, '%s')", pReq->nAccountID, pReq->szAllianceName);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_alliance_name,SQLERROR,0x%"WHINT64PRFX"X,%s", pReq->nAccountID, pReq->szAllianceName);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_NAME_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_alliance_id,@_price
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nAllianceID	= q.GetVal_64();
						RplCmd.nDiamond		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"change_alliance_name,0x%"WHINT64PRFX"X,%s,%d,0x%"WHINT64PRFX"X,%d"
							, pReq->nAccountID, pReq->szAllianceName, RplCmd.nRst, RplCmd.nAccountID, RplCmd.nDiamond);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"change_alliance_name,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%s", pReq->nAccountID, pReq->szAllianceName);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_ALLI_NAME_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_GEN_WORLD_FAMOUS_CITY_REQ:
		{
			P_DBS4WEB_GEN_WORLD_FAMOUS_CITY_T*	pReq	= (P_DBS4WEB_GEN_WORLD_FAMOUS_CITY_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"gen_world_famous_city,DBERROR");
			}
			else
			{
				// 1.是否需要进行奖杯结算
				if (pReq->bClearFirst)
				{
					// (1)删除所有的世界名城
					q.SpawnQuery("CALL game_del_worldcity(0, 1, @result)");
					int	nPreRet								= 0;
					q.ExecuteSPWithResult(nPreRet);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_del_worldcity,0,SQLERROR");
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_del_worldcity,0");
					}
					q.FreeResult();

					// (2)奖杯结算
					q.SpawnQuery("CALL game_gen_worldcity_fini(1, @result)");
					nPreRet									= 0;
					q.ExecuteSPWithResult(nPreRet);
					if (nPreRet != MYSQL_QUERY_NORMAL || !q.FetchRow())
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity_fini,0,SQLERROR");
					}
					else
					{
						int nNum = dbs_wc(q.GetStr(1), ',');
						if (nNum>0)
						{
							int idx = 0;
							char tmp[32];
							while (dbs_splitbychar(q.GetStr(1)+1, ',', idx, tmp) > 0)
							{
								P_DBS4WEBUSER_ATB_T Cmd;
								Cmd.nCmd		= P_DBS4WEB_RPL_CMD;
								Cmd.nSubCmd		= CMDID_ATB;
								Cmd.nAccountID	= whstr2int64(tmp);
								SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));
							}
						}
						GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity_fini,0");
					}
					q.FreeResult();

					// (3)奖励邮件
					q.SpawnQuery("CALL game_worldcity_bonus(1, @result)");
					nPreRet									= 0;
					q.ExecuteSPWithResult(nPreRet);
					if (nPreRet != MYSQL_QUERY_NORMAL || !q.FetchRow())
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_worldcity_bonus,0,SQLERROR");
					}
					else
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
						GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_worldcity_bonus,%d", nRst);
					}
					q.FreeResult();

					// (4)初始化一下
					q.SpawnQuery("CALL game_gen_worldcity_init(1, @result)");
					nPreRet									= 0;
					q.ExecuteSPWithResult(nPreRet);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity_init,0,SQLERROR");
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity_init,0");
					}
					q.FreeResult();
				}

				// 2.生成所有的世界名城
				WorldFamousCity*	pCity				= (WorldFamousCity*)wh_getptrnexttoptr(pReq);
				for (int i=0; i<pReq->nNum; i++)
				{
					tty_id_t	nID						= (((tty_id_t)pCity->nPosX)<<32) | (pCity->nPosY);
					q.SpawnQuery("CALL game_gen_worldcity(0x%"WHINT64PRFX"X, %d, %d, %d, %d, %d, %d, 1, @result)"
						, nID, pCity->nType, pCity->nPosX, pCity->nPosY, (int)pCity->bCanBuild, (int)pCity->nFloor2, pCity->nSrcTerrainType);
					int			nPreRet					= 0;
					q.ExecuteSPWithResult(nPreRet);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity,SQLERROR");
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity,0x%"WHINT64PRFX"X,%d,%d,%d,%d,%d"
							, nID, pCity->nType, pCity->nPosX, pCity->nPosY, (int)pCity->bCanBuild, (int)pCity->nFloor2);
					}
					q.FreeResult();

					pCity++;
				}
			}
		}
		break;
	case CMDID_GET_WORLD_FAMOUS_CITY_REQ:
		{
			P_DBS4WEB_GET_WORLD_FAMOUS_CITY_T*	pReq	= (P_DBS4WEB_GET_WORLD_FAMOUS_CITY_T*)pCmd;
			P_DBS4WEBUSER_GET_WORLD_FAMOUS_CITY_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_GET_WORLD_FAMOUS_CITY_RPL;
			RplCmd.nNum					= 0;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_world_famous_city,DBERROR");
			}
			else
			{
				q.SpawnQuery("SELECT class,pos_x,pos_y,can_build,floor2,terrain_type FROM world_city");
				int	nPreRet				= 0;
				q.GetResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_world_famous_city,SQLERROR");
				}
				else
				{
					RplCmd.nNum			= q.NumRows();
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_WORLD_FAMOUS_CITY_T) + RplCmd.nNum*sizeof(WorldFamousCity));
					P_DBS4WEBUSER_GET_WORLD_FAMOUS_CITY_T*	pRplCmd	= (P_DBS4WEBUSER_GET_WORLD_FAMOUS_CITY_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					WorldFamousCity*	pCity				= (WorldFamousCity*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pCity->nType	= q.GetVal_32();
						pCity->nPosX	= q.GetVal_32();
						pCity->nPosY	= q.GetVal_32();
						pCity->bCanBuild= (bool)q.GetVal_32();
						pCity->nFloor2	= (unsigned char)q.GetVal_32();
						pCity->nSrcTerrainType	= q.GetVal_32();

						pCity++;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
			}
		}
		break;
	case CMDID_CUP_ACTIVITY_END_REQ:
		{
			P_DBS4WEB_CUP_ACTIVITY_END_T*		pReq	= (P_DBS4WEB_CUP_ACTIVITY_END_T*)pCmd;
			P_DBS4WEBUSER_CUP_ACTIVITY_END_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_CUP_ACTIVITY_END_RPL;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"cup_acitivity_end,DBERROR");
				return 0;
			}
			else
			{
				// 1.删除所有的世界名城
				q.SpawnQuery("CALL game_del_worldcity(0, 1, @result)");
				int	nPreRet								= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_del_worldcity,0,SQLERROR");
				}
				else
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_del_worldcity,0");
				}
				q.FreeResult();

				// 2.奖杯结算
				q.SpawnQuery("CALL game_gen_worldcity_fini(1, @result)");
				nPreRet									= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL || !q.FetchRow())
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity_fini,0,SQLERROR");
				}
				else
				{
					int nNum = dbs_wc(q.GetStr(1), ',');
					if (nNum>0)
					{
						int idx = 0;
						char tmp[32];
						while (dbs_splitbychar(q.GetStr(1)+1, ',', idx, tmp) > 0)
						{
							P_DBS4WEBUSER_ATB_T Cmd;
							Cmd.nCmd		= P_DBS4WEB_RPL_CMD;
							Cmd.nSubCmd		= CMDID_ATB;
							Cmd.nAccountID	= whstr2int64(tmp);
							SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));
						}
					}
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity_fini,0");
				}
				q.FreeResult();

				// 3.奖励邮件
				q.SpawnQuery("CALL game_worldcity_bonus(1, @result)");
				nPreRet									= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL || !q.FetchRow())
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_worldcity_bonus,0,SQLERROR");
				}
				else
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
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_worldcity_bonus,%d", nRst);
				}
				q.FreeResult();

				// 4.初始化一下
				q.SpawnQuery("CALL game_gen_worldcity_init(1, @result)");
				nPreRet									= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity_init,0,SQLERROR");
				}
				else
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,DBS4Web)"game_gen_worldcity_init,0");
				}
				q.FreeResult();

				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
			}
		}
		break;
	case CMDID_FETCH_MAIL_REWARD_REQ:
		{
			P_DBS4WEB_FETCH_MAIL_REWARD_T*		pReq	= (P_DBS4WEB_FETCH_MAIL_REWARD_T*)pCmd;
			P_DBS4WEBUSER_FETCH_MAIL_REWARD_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_FETCH_MAIL_REWARD_RPL;
			RplCmd.nAccountID			= pReq->nAccountID;
			RplCmd.nMailID				= pReq->nMailID;
			RplCmd.nNum					= 0;
			RplCmd.nRst					= STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_mail_reward,DBERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nMailID);
				RplCmd.nRst				= STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL fetch_mail_reward(0x%"WHINT64PRFX"X,%d)", pReq->nAccountID, pReq->nMailID);
				int	nPreRet								= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_mail_reward,SQLERROR,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nMailID);
					RplCmd.nRst				= STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					if (q.FetchRow())
					{
						// SELECT _result,@_rewards; # 格式:type*num[*excel_id*only_id数量,item_id0,item_id1...];type*num[*excel_id*only_id数量,item_id0,item_id1...];...
						RplCmd.nRst			= q.GetVal_32();
						if (RplCmd.nRst == STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RST_OK)
						{
							int*	pLengths	= q.FetchLengths();
							char*	pTmpBuf		= new char[pLengths[1]+1];
							char	szOneReward[1024];
							memcpy(pTmpBuf, q.GetStr(), pLengths[1]);
							pTmpBuf[pLengths[1]]= 0;
							GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"fetch_mail_reward,0x%"WHINT64PRFX"X,%d,%d,%s"
								, pReq->nAccountID, pReq->nMailID, RplCmd.nRst, pTmpBuf);

							char*	pPos		= pTmpBuf;
							whvector<STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit>	vectTmpRewards;
							// 分割字串发送奖励
							int		nSplitToNum	= 0;
							while ((nSplitToNum=wh_strsplit("sa", pPos, ";", szOneReward, pPos)) >= 1)
							{
								unsigned int	nType	= 0;
								unsigned int	nNum	= 0;
								wh_strsplit("dda", szOneReward, "*", &nType, &nNum, szOneReward);
								switch(nType)
								{
								case mail_reward_type_item:
									{
										unsigned int	nExcelID	= 0;
										unsigned int	nIDNum		= 0;
										char		szTmpBuf4ExcelIDnNum[256];
										wh_strsplit("sa", szOneReward, ",", szTmpBuf4ExcelIDnNum, szOneReward);
										wh_strsplit("dd", szTmpBuf4ExcelIDnNum, "*", &nExcelID, &nIDNum);
										for (int i=0; i<nIDNum; i++)
										{
											STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit*	pUnit	= vectTmpRewards.push_back();
											pUnit->nType	= nType;
											pUnit->nNum		= nNum;
											pUnit->nData	= nExcelID;
											wh_strsplit("Ia", szOneReward, ",", &pUnit->nID, szOneReward);
										}
									}
									break;
								case mail_reward_type_diamond:
								case mail_reward_type_crystal:
								case mail_reward_type_gold:
									{
										STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit*	pUnit	= vectTmpRewards.push_back();
										pUnit->nType	= nType;
										pUnit->nNum		= nNum;
										pUnit->nData	= 0;
										pUnit->nID		= 0;
									}
									break;
								}

								if (nSplitToNum <= 1)
								{
									break;
								}
							}
							delete[] pTmpBuf;
							RplCmd.nNum					= vectTmpRewards.size();
							vectRawBuf.resize(sizeof(P_DBS4WEBUSER_FETCH_MAIL_REWARD_T) + RplCmd.nNum*sizeof(STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit));
							P_DBS4WEBUSER_FETCH_MAIL_REWARD_T*	pRplCmd	= (P_DBS4WEBUSER_FETCH_MAIL_REWARD_T*)vectRawBuf.getbuf();
							memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
							memcpy(wh_getptrnexttoptr(pRplCmd), vectTmpRewards.getbuf(), RplCmd.nNum*sizeof(STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RewardUnit));
							SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						}
						else
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"fetch_maiL_reward,0x%"WHINT64PRFX"X,%d,%d,NULL"
								, pReq->nAccountID, pReq->nMailID, RplCmd.nRst);
							SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
							return 0;
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"fetch_maiL_reward,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%d", pReq->nAccountID, pReq->nMailID);
						RplCmd.nRst				= STC_GAMECMD_OPERATE_FETCH_MAIL_REWARD_T::RST_SQL_ERR;
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
						return 0;
					}
				}
			}
		}
		break;
	case CMDID_UPLOAD_BILL_REQ:
		{
			P_DBS4WEB_UPLOAD_BILL_T*		pReq	= (P_DBS4WEB_UPLOAD_BILL_T*)pCmd;
			P_DBS4WEB4USER_UPLOAD_BILL_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_UPLOAD_BILL_RPL;
			RplCmd.nAccountID			= pReq->nAccountID;
			RplCmd.nClientTime			= pReq->nClientTime;
			RplCmd.nServerTime			= pReq->nServerTime;
			RplCmd.nRst					= STC_GAMECMD_OPERATE_UPLOAD_BILL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"store_bill,DBERROR,0x%"WHINT64PRFX"X,%u,%u"
					, pReq->nAccountID, pReq->nClientTime, pReq->nServerTime);
				RplCmd.nRst				= STC_GAMECMD_OPERATE_UPLOAD_BILL_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL store_bill(0x%"WHINT64PRFX"X,%u,%u,%u,'", pReq->nAccountID, pReq->nClientTime, pReq->nServerTime, pReq->nAppID);
				q.BinaryToString(strlen(pReq->szText), pReq->szText);
				q.StrMove("')");
				int	nPreRet				= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"store_bill,SQLERROR,0x%"WHINT64PRFX"X,%u,%u"
						, pReq->nAccountID, pReq->nClientTime, pReq->nServerTime);
					RplCmd.nRst				= STC_GAMECMD_OPERATE_UPLOAD_BILL_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"store_bill,0x%"WHINT64PRFX"X,%u,%u,%d"
							, pReq->nAccountID, pReq->nClientTime, pReq->nServerTime, RplCmd.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"store_bill,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X,%u,%u"
							, pReq->nAccountID, pReq->nClientTime, pReq->nServerTime);
						RplCmd.nRst				= STC_GAMECMD_OPERATE_UPLOAD_BILL_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_SET_BINDED_REQ:
		{
			P_DBS4WEB_SET_BINDED_T*	pReq	= (P_DBS4WEB_SET_BINDED_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_binded,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
			}
			else
			{
				q.SpawnQuery("UPDATE common_characters SET binded=1 WHERE account_id=0x%"WHINT64PRFX"X", pReq->nAccountID);
				int	nPreRet				= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"set_binded,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				}
			}
		}
		break;
	case CMDID_ADD_LOTTERY_LOG_REQ:
		{
			P_DBS4WEB_ADD_LOTTERY_LOG_T*	pReq	= (P_DBS4WEB_ADD_LOTTERY_LOG_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_lottery_log,DBERROR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, (int)pReq->nType, pReq->nData);
			}
			else
			{
				q.SpawnQuery("CALL add_unfetched_lottery_log(0x%"WHINT64PRFX"X, %u, %u, %u)", pReq->nAccountID, (int)pReq->nType, pReq->nData, pReq->nTime);
				int	nPreRet				= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"add_lottery_log,SQLERROR,0x%"WHINT64PRFX"X,%d,%d", pReq->nAccountID, (int)pReq->nType, pReq->nData);
				}
				else
				{
				}
			}
		}
		break;

	case CMDID_TAVERN_REFRESH_REQ:
	case CMDID_HIRE_HERO_REQ:
	case CMDID_FIRE_HERO_REQ:
//	case CMDID_LEVELUP_HERO_REQ:
//	case CMDID_ADD_GROW_REQ:
	case CMDID_CONFIG_HERO_REQ:
	case CMDID_GET_HERO_REFRESH_REQ:
	case CMDID_GET_HERO_HIRE_REQ:
	case CMDID_UPDATE_HERO_REQ:
	case CMDID_CONFIG_CITYDEFENSE_REQ:
	case CMDID_GET_CITYDEFENSE_REQ:
		{
			ThreadDealCmd_Hero(pMySQL, pCDCmd, vectRawBuf);
		}
		break;
	case CMDID_START_COMBAT_REQ:
	case CMDID_STOP_COMBAT_REQ:
	case CMDID_GET_COMBAT_REQ:
	case CMDID_LIST_COMBAT_REQ:
	case CMDID_DEAL_COMBAT_TE_REQ:
	case CMDID_DEAL_COMBAT_BACK_REQ:
	case CMDID_COMBAT_PROF_REQ:
	case CMDID_LOAD_ARMY_REQ:
	case CMDID_ARMY_ACCELERATE_REQ:
	case CMDID_ARMY_BACK_REQ:
	case CMDID_DEAL_WORLDCITYRECOVER_TE_REQ:
	case CMDID_WORLDCITY_GET_REQ:
	case CMDID_WORLDCITY_GETLOG_REQ:
	case CMDID_WORLDCITY_GETLOG_ALLIANCE_REQ:
	case CMDID_WORLDCITY_RANK_MAN_REQ:
	case CMDID_WORLDCITY_RANK_ALLIANCE_REQ:
		{
			ThreadDealCmd_Combat(pMySQL, pCDCmd, vectRawBuf);
		}
		break;
	case CMDID_GET_ITEM_REQ:
//	case CMDID_ADD_ITEM_REQ:
	case CMDID_DEL_ITEM_REQ:
	case CMDID_EQUIP_ITEM_REQ:
	case CMDID_DISEQUIP_ITEM_REQ:
	case CMDID_MOUNT_ITEM_REQ:
	case CMDID_UNMOUNT_ITEM_REQ:
	case CMDID_COMPOS_ITEM_REQ:
	case CMDID_DISCOMPOS_ITEM_REQ:
	case CMDID_EQUIP_ITEM_ALL_REQ:
	case CMDID_MOUNT_ITEM_ALL_REQ:
	case CMDID_GET_EQUIP_REQ:
	case CMDID_GET_GEM_REQ:
	case CMDID_REPLACE_EQUIP_REQ:
	case CMDID_USE_ITEM_REQ:
	case CMDID_USE_DRUG_REQ:
		{
			ThreadDealCmd_Item(pMySQL, pCDCmd, vectRawBuf);
		}
		break;
	case CMDID_GET_QUEST_REQ:
	case CMDID_DONE_QUEST_REQ:
		{
			ThreadDealCmd_Quest(pMySQL, pCDCmd, vectRawBuf);
		}
		break;
	case CMDID_GET_INSTANCESTATUS_REQ:
	case CMDID_GET_INSTANCEDESC_REQ:
	case CMDID_CREATE_INSTANCE_REQ:
	case CMDID_JOIN_INSTANCE_REQ:
	case CMDID_QUIT_INSTANCE_REQ:
	case CMDID_DESTROY_INSTANCE_REQ:
	case CMDID_GET_INSTANCEDATA_REQ:
	case CMDID_CONFIG_INSTANCEHERO_REQ:
	case CMDID_GET_INSTANCELOOT_REQ:
	case CMDID_AUTO_COMBAT_REQ:
	case CMDID_AUTO_SUPPLY_REQ:
	case CMDID_MANUAL_SUPPLY_REQ:
	case CMDID_KICK_INSTANCE_REQ:
	case CMDID_START_INSTANCE_REQ:
	case CMDID_PREPARE_INSTANCE_REQ:
	case CMDID_SUPPLY_INSTANCE_REQ:
		{
			ThreadDealCmd_Instance(pMySQL, pCDCmd, vectRawBuf);
		}
		break;
	case CMDID_GET_WORLDGOLDMINE_REQ:
	case CMDID_CONFIG_WORLDGOLDMINE_HERO_REQ:
	case CMDID_ROB_WORLDGOLDMINE_REQ:
	case CMDID_DROP_WORLDGOLDMINE_REQ:
	case CMDID_DEAL_WORLDGOLDMINE_GAIN_TE_REQ:
	case CMDID_MY_WORLDGOLDMINE_REQ:
		{
			ThreadDealCmd_WorldGoldMine(pMySQL, pCDCmd, vectRawBuf);
		}
		break;
	case CMDID_DEAL_FIXHOUR_TE_REQ:
	case CMDID_RECHARGE_TRY_REQ:
		{
			ThreadDealCmd_FixHour(pMySQL, pCDCmd, vectRawBuf);
		}
		break;

	case CMDID_DEAL_CHAR_DAY_TE_REQ:
		{
			P_DBS4WEB_DEAL_CHAR_DAY_TE_T*	pReq	= (P_DBS4WEB_DEAL_CHAR_DAY_TE_T*)pCmd;
			P_DBS4WEBUSER_DEAL_CHAR_DAY_TE_T	RplCmd;
			RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd				= CMDID_DEAL_CHAR_DAY_TE_RPL;
			RplCmd.nAccountID			= pReq->nAccountID;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_char_day_te,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
			}
			else
			{
				q.SpawnQuery("CALL _deal_char_day_te(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet				= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_char_day_te,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				}
				else
				{
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				}
			}
		}
		break;
	case CMDID_DEAL_ADCOLONY_TE_REQ:
		{
			P_DBS4WEB_DEAL_ADCOLONY_TE_T*	pReq	= (P_DBS4WEB_DEAL_ADCOLONY_TE_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_adcolony,DBERROR");
			}
			else
			{
				q.SpawnQuery("CALL deal_adcolony()");
				int	nPreRet				= 0;
				q.ExecuteSPWithResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"deal_adcolony,SQLERROR,%d", nPreRet);
				}
				else
				{
					int	nNum	= q.NumRows();
					if (nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T) + nNum*sizeof(P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T::AdColonyAward));
						P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T*	pRplCmd	= (P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T*)vectRawBuf.getbuf();
						pRplCmd->nCmd			= P_DBS4WEB_RPL_CMD;
						pRplCmd->nSubCmd		= CMDID_DEAL_ADCOLONY_TE_RPL;
						pRplCmd->nNum			= nNum;
						P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T::AdColonyAward*	pAward	= (P_DBS4WEBUSER_DEAL_ADCOLONY_TE_T::AdColonyAward*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pAward->nAccountID	= q.GetVal_64();
							pAward->nMoneyType	= q.GetVal_32();
							pAward->nNum		= q.GetVal_32();

							pAward++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;
	case CMDID_DISMISS_SOLDIER_REQ:
		{
			P_DBS4WEB_DISMISS_SOLDIER_T*	pReq	= (P_DBS4WEB_DISMISS_SOLDIER_T*)pCmd;
			P_DBS4WEBUSER_DISMISS_SOLDIER_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_DISMISS_SOLDIER_RPL;
			RplCmd.nExcelID			= pReq->nExcelID;
			RplCmd.nLevel			= pReq->nLevel;
			RplCmd.nNum				= pReq->nNum;
			RplCmd.nRst				= STC_GAMECMD_OPERATE_DISMISS_SOLDIER_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"DBERR,0x%"WHINT64PRFX"X,%u,%u,%u", pReq->nAccountID, pReq->nExcelID, pReq->nLevel, pReq->nNum);
				RplCmd.nRst			= STC_GAMECMD_OPERATE_DISMISS_SOLDIER_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL dismiss_soldier(0x%"WHINT64PRFX"X, %u, %u, %u)", pReq->nAccountID, pReq->nExcelID, pReq->nLevel, pReq->nNum);
				int	nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"SQLERR,0x%"WHINT64PRFX"X,%u,%u,%u", pReq->nAccountID, pReq->nExcelID, pReq->nLevel, pReq->nNum);
					RplCmd.nRst			= STC_GAMECMD_OPERATE_DISMISS_SOLDIER_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst		= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4Web)"0x%"WHINT64PRFX"X,%u,%u,%u,%d", pReq->nAccountID, pReq->nExcelID, pReq->nLevel, pReq->nNum, RplCmd.nRst);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%u,%u,%u", pReq->nAccountID, pReq->nExcelID, pReq->nLevel, pReq->nNum);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_DISMISS_SOLDIER_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_REFRESH_MAIL_REQ:
		{
			P_DBS4WEB_REFRESH_MAIL_T*	pReq	= (P_DBS4WEB_REFRESH_MAIL_T*)pCmd;
			P_DBS4WEBUSER_REFRESH_MAIL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_REFRESH_MAIL_RPL;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refresh_private_mail,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				return 0;
			}
			else
			{
				q.SpawnQuery("SELECT m.mail_id,m.sender_id,m.flag,m.readed,m.type,uncompress(m.text),uncompress(m.ext_data),m.time,IFNULL(c.name,'') as sender_name FROM private_mails m LEFT JOIN common_characters c ON m.sender_id=c.account_id WHERE m.account_id=0x%"WHINT64PRFX"X AND m.mail_id>%d ORDER BY m.mail_id DESC", pReq->nAccountID, pReq->nMailID);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refresh_private_mail,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					return 0;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum == 0)
					{
						return 0;
					}
					else
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_REFRESH_MAIL_T));
						P_DBS4WEBUSER_REFRESH_MAIL_T*	pRpl	= (P_DBS4WEBUSER_REFRESH_MAIL_T*)vectRawBuf.getbuf();
						memcpy(pRpl, &RplCmd, sizeof(RplCmd));
						PrivateMailUnit*	pUnit		= NULL;
						while (q.FetchRow())
						{
							int*	pLengths		= q.FetchLengths();
							int		nTotalMsgLen	= wh_offsetof(PrivateMailUnit, szText) + pLengths[5] + (sizeof(int)+pLengths[6]);
							pUnit					= (PrivateMailUnit*)vectRawBuf.pushn_back(nTotalMsgLen);
							
							pUnit->nTextLen			= pLengths[5];
							pUnit->nMailID			= q.GetVal_32();
							pUnit->nSenderID		= q.GetVal_64();
							pUnit->nFlag			= (char)q.GetVal_32();
							pUnit->bReaded			= (bool)q.GetVal_32();
							pUnit->nType			= (char)q.GetVal_32();
							memcpy(pUnit->szText, q.GetStr(), pUnit->nTextLen);
							int		nMsgUnitLen		= wh_offsetof(PrivateMailUnit, szText) + pUnit->nTextLen;
							int*	pExtDataLen		= (int*)wh_getoffsetaddr(pUnit, nMsgUnitLen);
							*pExtDataLen			= pLengths[6];
							void*	pExtData		= (void*)wh_getptrnexttoptr(pExtDataLen);
							memcpy(pExtData, q.GetStr(), *pExtDataLen);

							pUnit->nTime			= q.GetVal_32();
							WH_STRNCPY0(pUnit->szSender, q.GetStr());
						}

						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;
	case CMDID_BAN_CHAR_REQ:
		{
			P_DBS4WEB_BAN_CHAR_T*	pReq	= (P_DBS4WEB_BAN_CHAR_T*)pCmd;
			P_DBS4WEBUSER_BAN_CHAR_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_BAN_CHAR_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nRst			= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"ban_char,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= -200;
			}
			else
			{
				q.SpawnQuery("CALL ban_char(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"ban_char,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= -201;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst	= q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"ban_char,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst	= -201;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_KICK_CLIENT_ALL_REQ:
		{
			P_DBS4WEB_KICK_CLIENT_ALL_T*	pReq	= (P_DBS4WEB_KICK_CLIENT_ALL_T*)pCmd;
			P_DBS4WEBUSER_KICK_CLIENT_ALL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_KICK_CLIENT_ALL_RPL;
			RplCmd.nRst			= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			char*	pStr		= (char*)wh_getptrnexttoptr(pReq);
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"kick_client_all,DBERR,%s", pStr);
				RplCmd.nRst		= -200;
			}
			else
			{
				q.SpawnQuery("CALL kick_client_all('%s')", pStr);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"kick_client_all,SQLERR,%s", pStr);
					RplCmd.nRst		= -201;
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_GET_KICK_CLIENT_ALL_REQ:
		{
			P_DBS4WEB_GET_KICK_CLIENT_ALL_T*	pReq	= (P_DBS4WEB_GET_KICK_CLIENT_ALL_T*)pCmd;
			P_DBS4WEBUSER_GET_KICK_CLIENT_ALL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_KICK_CLIENT_ALL_RPL;
			RplCmd.nRst			= 0;
			RplCmd.nLength		= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_kick_client_all,DBERR");
				RplCmd.nRst		= STC_GAMECMD_GET_KICK_CLIENT_ALL_T::RST_DB_ERR;;
			}
			else
			{
				q.SpawnQuery("SELECT version_appid FROM t_kick_client_all");
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_kick_client_all,SQLERR");
					RplCmd.nRst		= STC_GAMECMD_GET_KICK_CLIENT_ALL_T::RST_SQL_ERR;;
				}
				else
				{
					if (q.FetchRow())
					{
						int*	pLengths	= q.FetchLengths();
						RplCmd.nLength	= pLengths[0]+1;
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_KICK_CLIENT_ALL_T) + RplCmd.nLength);
						P_DBS4WEBUSER_GET_KICK_CLIENT_ALL_T*	pRplCmd	= (P_DBS4WEBUSER_GET_KICK_CLIENT_ALL_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						char*	pStr	= (char*)wh_getptrnexttoptr(pRplCmd);
						memcpy(pStr, q.GetStr(), pLengths[0]);
						pStr[pLengths[0]]= 0;
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
						return 0;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;

	case CMDID_NEW_ADCOLONY_REQ:
		{
			P_DBS4WEB_NEW_ADCOLONY_T*	pReq	= (P_DBS4WEB_NEW_ADCOLONY_T*)pCmd;
			P_DBS4WEBUSER_NEW_ADCOLONY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_NEW_ADCOLONY_RPL;
			RplCmd.nTransactionID	= pReq->nTransactionID;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
			}
			else
			{
				q.SpawnQuery("INSERT IGNORE INTO adcolony_transactions (transaction_id,time,name,account_id,amount) VALUES (0x%"WHINT64PRFX"X,%u,'%s',0x%"WHINT64PRFX"X,%d)"
					, pReq->nTransactionID, pReq->nTime, pReq->szName, pReq->nAccountID, pReq->nAmount);
				int	nPreRet		= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
				}
				else
				{
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				}
			}
		}
		break;

	case CMDID_ALLI_DONATE_SOLDIER_REQ:
		{
			P_DBS4WEB_ALLI_DONATE_SOLDIER_T*	pReq	= (P_DBS4WEB_ALLI_DONATE_SOLDIER_T*)pCmd;
			P_DBS4WEBUSER_ALLI_DONATE_SOLDIER_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_DONATE_SOLDIER_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nEventID		= 0;
			RplCmd.nExcelID		= pReq->nExcelID;
			RplCmd.nLevel		= pReq->nLevel;
			RplCmd.nNum			= pReq->nNum;
			RplCmd.nObjID		= pReq->nObjID;
			RplCmd.nTime		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alliance_donate_soldier,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d"
					, pReq->nAccountID, pReq->nObjID, pReq->nExcelID, pReq->nLevel, pReq->nNum);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL alliance_donate_soldier(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d)"
					, pReq->nAccountID, pReq->nObjID, pReq->nExcelID, pReq->nLevel, pReq->nNum);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alliance_donate_soldier,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d"
						, pReq->nAccountID, pReq->nObjID, pReq->nExcelID, pReq->nLevel, pReq->nNum);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_event_id,@_interval
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nEventID	= q.GetVal_64();
						RplCmd.nTime	= q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alliance_donate_soldier,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d"
							, pReq->nAccountID, pReq->nObjID, pReq->nExcelID, pReq->nLevel, pReq->nNum);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_DONATE_SOLDIER_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_RECALL_SOLDIER_REQ:
		{
			P_DBS4WEB_ALLI_RECALL_SOLDIER_T*	pReq	= (P_DBS4WEB_ALLI_RECALL_SOLDIER_T*)pCmd;
			P_DBS4WEBUSER_ALLI_RECALL_SOLDIER_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_RECALL_SOLDIER_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNewEventID	= 0;
			RplCmd.nOldEventID	= pReq->nEventID;
			RplCmd.nTime		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alliance_recall_soldier,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
					, pReq->nAccountID, pReq->nEventID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL alliance_recall_soldier(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)", pReq->nAccountID, pReq->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alliance_recall_soldier,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
						, pReq->nAccountID, pReq->nEventID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_event_id,@_interval,@_obj_id
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.nNewEventID	= q.GetVal_64();
						RplCmd.nTime		= q.GetVal_32();
						RplCmd.nObjID		= q.GetVal_64();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alliance_recall_soldier,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
							, pReq->nAccountID, pReq->nEventID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_RECALL_SOLDIER_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_ALLI_ACCE_SOLDIER_REQ:
		{
			P_DBS4WEB_ALLI_ACCE_SOLDIER_T*	pReq	= (P_DBS4WEB_ALLI_ACCE_SOLDIER_T*)pCmd;
			P_DBS4WEBUSER_ALLI_ACCE_SOLDIER_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ALLI_ACCE_SOLDIER_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nEventID		= pReq->nEventID;
			RplCmd.nMoney		= 0;
			RplCmd.nMoneyType	= pReq->nMoneyType;
			RplCmd.nTime		= pReq->nTime;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"accelerate_donate_soldier,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d"
					, pReq->nAccountID, pReq->nEventID, pReq->nTime, pReq->nMoneyType);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL accelerate_donate_soldier(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d)"
					, pReq->nAccountID, pReq->nEventID, pReq->nTime, pReq->nMoneyType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"accelerate_donate_soldier,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d"
						, pReq->nAccountID, pReq->nEventID, pReq->nTime, pReq->nMoneyType);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_money,_time,@_obj_id
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nMoney	= q.GetVal_32();
						RplCmd.nTime	= q.GetVal_32();
						RplCmd.nObjID	= q.GetVal_64();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"accelerate_donate_soldier,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d"
							, pReq->nAccountID, pReq->nEventID, pReq->nTime, pReq->nMoneyType);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_ALLI_ACCE_SOLDIER_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_GET_DONATE_SOLDIER_QUEUE_REQ:
		{
			P_DBS4WEB_GET_DONATE_SOLDIER_QUEUE_T*	pReq	= (P_DBS4WEB_GET_DONATE_SOLDIER_QUEUE_T*)pCmd;
			P_DBS4WEBUSER_GET_DONATE_SOLDIER_QUEUE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_DONATE_SOLDIER_QUEUE_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_donate_soldier_queue,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T::RST_DB_ERR;
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				return 0;
			}
			else
			{
				q.SpawnQuery("CALL get_donate_soldier_queue(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_donate_soldier_queue,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					return 0;
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_DONATE_SOLDIER_QUEUE_T) + RplCmd.nNum*sizeof(STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T::UNIT_T));
						P_DBS4WEBUSER_GET_DONATE_SOLDIER_QUEUE_T*	pRplCmd	= (P_DBS4WEBUSER_GET_DONATE_SOLDIER_QUEUE_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T::UNIT_T*	pUnit	= (STC_GAMECMD_GET_DONATE_SOLDIER_QUEUE_T::UNIT_T*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// 1,d.obj_id,d.excel_id,d.level,d.num,d.type,t.event_id,IF(t.end_time>@_time_now,t.end_time-@_time_now,0),c.name
							pUnit->nDstType		= q.GetVal_32();
							pUnit->nPeerID		= q.GetVal_64();
							pUnit->nExcelID		= q.GetVal_32();
							pUnit->nLevel		= q.GetVal_32();
							pUnit->nNum			= q.GetVal_32();
							pUnit->nType		= q.GetVal_32();
							pUnit->nEventID		= q.GetVal_64();
							pUnit->nTime		= q.GetVal_32();
							WH_STRNCPY0(pUnit->szCharName, q.GetStr());

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_GET_LEFT_DONATE_SOLDIER_REQ:
		{
			P_DBS4WEB_GET_LEFT_DONATE_SOLDIER_T*	pReq	= (P_DBS4WEB_GET_LEFT_DONATE_SOLDIER_T*)pCmd;
			P_DBS4WEBUSER_GET_LEFT_DONATE_SOLDIER_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_LEFT_DONATE_SOLDIER_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nLeftNum		= 0;
			RplCmd.nMaxNum		= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_LEFT_DONATE_SOLDIER_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_left_donate_soldier_num,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst			= STC_GAMECMD_GET_LEFT_DONATE_SOLDIER_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL get_left_donate_soldier_num(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_left_donate_soldier_num,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst			= STC_GAMECMD_GET_LEFT_DONATE_SOLDIER_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// @_result,@_left_num,@_max_num
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nLeftNum	= q.GetVal_32();
						RplCmd.nMaxNum	= q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_left_donate_soldier_num,SQLERR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst			= STC_GAMECMD_GET_LEFT_DONATE_SOLDIER_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_DEAL_DONATE_SOLDIER_TE_REQ:
		{
			P_DBS4WEB_DEAL_DONATE_SOLDIER_TE_T*	pReq	= (P_DBS4WEB_DEAL_DONATE_SOLDIER_TE_T*)pCmd;
			P_DBS4WEBUSER_DEAL_DONATE_SOLDIER_TE_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_DEAL_DONATE_SOLDIER_TE_RPL;
			RplCmd.nAccountID	= 0;
			RplCmd.nEventID		= pReq->nEventID;
			RplCmd.nExcelID		= 0;
			RplCmd.nLevel		= 0;
			RplCmd.nNum			= 0;
			RplCmd.nObjID		= 0;
			RplCmd.nType		= 0;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_donate_soldier_event,DBERR,0x%"WHINT64PRFX"X", pReq->nEventID);
			}
			else
			{
				q.SpawnQuery("CALL deal_donate_soldier_event(0x%"WHINT64PRFX"X)", pReq->nEventID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_donate_soldier_event,SQLERR,0x%"WHINT64PRFX"X", pReq->nEventID);
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_account_id,@_obj_id,@_excel_id,@_level,@_num,@_type,@_mail_id
						int	nRst			= q.GetVal_32();
						RplCmd.nAccountID	= q.GetVal_64();
						RplCmd.nObjID		= q.GetVal_64();
						RplCmd.nExcelID		= q.GetVal_32();
						RplCmd.nLevel		= q.GetVal_32();
						RplCmd.nNum			= q.GetVal_32();
						RplCmd.nType		= q.GetVal_32();
						int	nMailID			= q.GetVal_32();
						GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(,DBS4WEB)"deal_donate_soldier_event,0x%"WHINT64PRFX"X,%d,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d,%d,%d,%d"
							, pReq->nEventID, nRst, RplCmd.nAccountID, RplCmd.nObjID, RplCmd.nExcelID, RplCmd.nLevel, RplCmd.nNum, RplCmd.nType);
						if (nRst == 0)
						{
							SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

							if (nMailID != 0)
							{
								P_DBS4WEBUSER_NEWMAIL_NOTIFY_T Cmd;
								Cmd.nCmd		= P_DBS4WEB_RPL_CMD;
								Cmd.nSubCmd		= CMDID_NEWMAIL_NOTIFY;
								Cmd.nAccountID	= RplCmd.nObjID;
								Cmd.nMailID		= nMailID;
								SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));
							}
						}
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"deal_donate_soldier_event,SQLERR,0x%"WHINT64PRFX"X", pReq->nEventID);
					}
				}
			}
		}
		break;
	case CMDID_ADD_GOLD_DEAL_REQ:
		{
			P_DBS4WEB_ADD_GOLD_DEAL_T*	pReq	= (P_DBS4WEB_ADD_GOLD_DEAL_T*)pCmd;
			P_DBS4WEBUSER_ADD_GOLD_DEAL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_ADD_GOLD_DEAL_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nTransactionID	= 0;
			RplCmd.nGold		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_ADD_GOLD_DEAL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"add_gold_deal,DBERR,0x%"WHINT64PRFX"X,%d,%d"
					, pReq->nAccountID, pReq->nGold, pReq->nPrice);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_ADD_GOLD_DEAL_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL add_gold_deal(0x%"WHINT64PRFX"X,%d,%d)", pReq->nAccountID, pReq->nGold, pReq->nPrice);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"add_gold_deal,SQLERR,0x%"WHINT64PRFX"X,%d,%d"
						, pReq->nAccountID, pReq->nGold, pReq->nPrice);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_ADD_GOLD_DEAL_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_transaction_id,_num
						RplCmd.nRst				= q.GetVal_32();
						RplCmd.nTransactionID	= q.GetVal_64();
						RplCmd.nGold			= q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"add_gold_deal,SQLERR,NO RESULT,0x%"WHINT64PRFX"X,%d,%d"
							, pReq->nAccountID, pReq->nGold, pReq->nPrice);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_ADD_GOLD_DEAL_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_BUY_GOLD_IN_MARKET_REQ:
		{
			P_DBS4WEB_BUY_GOLD_IN_MARKET_T*	pReq	= (P_DBS4WEB_BUY_GOLD_IN_MARKET_T*)pCmd;
			P_DBS4WEBUSER_BUY_GOLD_IN_MARKET_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_BUY_GOLD_IN_MARKET_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nMoneyType	= pReq->nMoneyType;
			RplCmd.nBuyerMailID	= 0;
			RplCmd.nGold		= 0;
			RplCmd.nSellerID	= 0;
			RplCmd.nSellerMailID	= 0;
			RplCmd.nTotalPrice	= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"buy_gold_in_market,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
					, pReq->nAccountID, pReq->nTransactionID, pReq->nMoneyType);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL buy_gold_in_market(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d)"
					, pReq->nAccountID, pReq->nTransactionID, pReq->nMoneyType);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"buy_gold_in_market,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
						, pReq->nAccountID, pReq->nTransactionID, pReq->nMoneyType);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_seller_id,@_num,@_total_price,@_buyer_mail_id,@_seller_mail_id
						RplCmd.nRst				= q.GetVal_32();
						RplCmd.nSellerID		= q.GetVal_64();
						RplCmd.nGold			= q.GetVal_32();
						RplCmd.nTotalPrice		= q.GetVal_32();
						RplCmd.nBuyerMailID		= q.GetVal_32();
						RplCmd.nSellerMailID	= q.GetVal_32();

						P_DBS4WEBUSER_NEWMAIL_NOTIFY_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_RPL_CMD;
						Cmd.nSubCmd		= CMDID_NEWMAIL_NOTIFY;
						Cmd.nAccountID	= pReq->nAccountID;
						Cmd.nMailID		= RplCmd.nBuyerMailID;
						SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));

						Cmd.nAccountID	= RplCmd.nSellerID;
						Cmd.nMailID		= RplCmd.nSellerMailID;
						SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"buy_gold_in_market,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X,%d"
							, pReq->nAccountID, pReq->nTransactionID, pReq->nMoneyType);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_BUY_GOLD_IN_MARKET_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_CANCEL_GOLD_DEAL_REQ:
		{
			P_DBS4WEB_CANCEL_GOLD_DEAL_T*	pReq	= (P_DBS4WEB_CANCEL_GOLD_DEAL_T*)pCmd;
			P_DBS4WEBUSER_CANCEL_GOLD_DEAL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_CANCEL_GOLD_DEAL_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nGold		= 0;
			RplCmd.nMailID		= 0;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"cancel_gold_deal,DBERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
					, pReq->nAccountID, pReq->nTransactionID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL cancel_gold_deal(0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X)"
					, pReq->nAccountID, pReq->nTransactionID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"cancel_gold_deal,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
						, pReq->nAccountID, pReq->nTransactionID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,@_mail_id,@_gold
						RplCmd.nRst		= q.GetVal_32();
						RplCmd.nMailID	= q.GetVal_32();
						RplCmd.nGold	= q.GetVal_32();

						P_DBS4WEBUSER_NEWMAIL_NOTIFY_T Cmd;
						Cmd.nCmd		= P_DBS4WEB_RPL_CMD;
						Cmd.nSubCmd		= CMDID_NEWMAIL_NOTIFY;
						Cmd.nAccountID	= RplCmd.nAccountID;
						Cmd.nMailID		= RplCmd.nMailID;
						SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"cancel_gold_deal,SQLERR,0x%"WHINT64PRFX"X,0x%"WHINT64PRFX"X"
							, pReq->nAccountID, pReq->nTransactionID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_CANCEL_GOLD_DEAL_T::RST_SQL_ERR;
					}
				}
			}

			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_GET_GOLD_DEAL_REQ:
		{
			P_DBS4WEB_GET_GOLD_DEAL_T*	pReq	= (P_DBS4WEB_GET_GOLD_DEAL_T*)pCmd;
			P_DBS4WEBUSER_GET_GOLD_DEAL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_GOLD_DEAL_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nPage		= pReq->nPage;
			RplCmd.nTotalNum	= 0;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_GOLD_DEAL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_gold_deal,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_GOLD_DEAL_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL get_gold_deal_num(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_gold_deal_num,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_GOLD_DEAL_T::RST_SQL_ERR;
					SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
				}
				q.FetchRow();
				RplCmd.nTotalNum	= q.GetVal_32();
				q.FreeResult();

				q.SpawnQuery("CALL get_gold_deal(0x%"WHINT64PRFX"X,%d,%d)", pReq->nAccountID, pReq->nPage, pReq->nPageSize);
				nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_gold_deal,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_GOLD_DEAL_T::RST_SQL_ERR;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum>0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_GOLD_DEAL_T)+RplCmd.nNum*sizeof(GoldMarketDeal_T));
						P_DBS4WEBUSER_GET_GOLD_DEAL_T*	pRplCmd	= (P_DBS4WEBUSER_GET_GOLD_DEAL_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						GoldMarketDeal_T*	pGoldMarket			= (GoldMarketDeal_T*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// transaction_id,num,price,total_price,account_id
							pGoldMarket->nTransactionID			= q.GetVal_64();
							pGoldMarket->nGold					= q.GetVal_32();
							pGoldMarket->nPrice					= q.GetVal_32();
							pGoldMarket->nTotalPrice			= q.GetVal_32();
							pGoldMarket->nAccountID				= q.GetVal_64();

							pGoldMarket++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_GET_SELF_GOLD_DEAL_REQ:
		{
			P_DBS4WEB_GET_SELF_GOLD_DEAL_T*	pReq	= (P_DBS4WEB_GET_SELF_GOLD_DEAL_T*)pCmd;
			P_DBS4WEBUSER_GET_SELF_GOLD_DEAL_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_SELF_GOLD_DEAL_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.nNum			= 0;
			RplCmd.nRst			= STC_GAMECMD_GET_SELF_GOLD_DEAL_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_self_gold_deal,DBERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_GET_SELF_GOLD_DEAL_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL get_self_gold_deal(0x%"WHINT64PRFX"X)", pReq->nAccountID);
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_self_gold_deal,SQLERR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_GET_GOLD_DEAL_T::RST_SQL_ERR;
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum>0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_SELF_GOLD_DEAL_T)+RplCmd.nNum*sizeof(GoldMarketDeal_T));
						P_DBS4WEBUSER_GET_SELF_GOLD_DEAL_T*	pRplCmd	= (P_DBS4WEBUSER_GET_SELF_GOLD_DEAL_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						GoldMarketDeal_T*	pGoldMarket			= (GoldMarketDeal_T*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// transaction_id,num,price,total_price,account_id
							pGoldMarket->nTransactionID			= q.GetVal_64();
							pGoldMarket->nGold					= q.GetVal_32();
							pGoldMarket->nPrice					= q.GetVal_32();
							pGoldMarket->nTotalPrice			= q.GetVal_32();
							pGoldMarket->nAccountID				= q.GetVal_64();

							pGoldMarket++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_DEAL_GOLD_MARKET_TE_REQ:
		{
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
			}
			else
			{
				q.SpawnQuery("CALL deal_gold_market_te()");
				int	nPreRet		= 0;
				q.Execute(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
				}
				else
				{
					int	nNum	= q.NumRows();
					if (nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T) + nNum*sizeof(P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T::UNIT_T));
						P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T*	pRplCmd	= (P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T*)vectRawBuf.getbuf();
						pRplCmd->nCmd			= P_DBS4WEB_RPL_CMD;
						pRplCmd->nSubCmd		= CMDID_DEAL_GOLD_MARKET_TE_RPL;
						pRplCmd->nNum			= nNum;
						P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T::UNIT_T*	pUnit	= (P_DBS4WEBUSER_DEAL_GOLD_MARKET_TE_T::UNIT_T*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// account_id,mail_id,_gold
							pUnit->nAccountID	= q.GetVal_64();
							pUnit->nMailID		= q.GetVal_32();
							pUnit->nGold		= q.GetVal_32();

							P_DBS4WEBUSER_NEWMAIL_NOTIFY_T Cmd;
							Cmd.nCmd		= P_DBS4WEB_RPL_CMD;
							Cmd.nSubCmd		= CMDID_NEWMAIL_NOTIFY;
							Cmd.nAccountID	= pUnit->nAccountID;
							Cmd.nMailID		= pUnit->nMailID;
							SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;
	case CMDID_GEN_WORLD_RES_REQ:
		{
			P_DBS4WEB_GEN_WORLD_RES_T*		pReq	= (P_DBS4WEB_GEN_WORLD_RES_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"gen_world_res,DBERR");
			}
			else
			{
				WorldRes*	pRes	= (WorldRes*)wh_getptrnexttoptr(pReq);
				for (int i=0; i<pReq->nNum; i++)
				{
					q.SpawnQuery("CALL gen_world_res(0x%"WHINT64PRFX"X,%d,%d,%d,%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d)"
						, pRes->nID, pRes->nType, pRes->nLevel, pRes->nPosX, pRes->nPosY
						, pRes->szArmyDeploy, pRes->szArmyData, pRes->nGold, pRes->nPop, pRes->nCrystal
						, (int)pRes->bCanBuild, (int)pRes->nFloor2, (int)pRes->nSrcTerrainType, pRes->nForce);
					int		nPreRet	= 0;
					q.ExecuteSPWithResult(nPreRet);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"gen_world_res,SQLERR,%d,%d", pRes->nPosX, pRes->nPosY);
					}
					else
					{
						if (q.FetchRow())
						{
							int	nRst	= q.GetVal_32();
							if (nRst != 0)
							{
								GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"gen_world_res,%d,%d,%d"
									, nRst, pRes->nPosX, pRes->nPosY);
							}
						}
						else
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"gen_world_res,SQLERR,NO RESULT,%d,%d", pRes->nPosX, pRes->nPosY);
						}
					}
					q.FreeResult();

					pRes++;
				}
			}
		}
		break;
	case CMDID_GET_WORLD_RES_REQ:
		{
			P_DBS4WEB_GET_WORLD_RES_T*		pReq	= (P_DBS4WEB_GET_WORLD_RES_T*)pCmd;
			P_DBS4WEBUSER_GET_WORLD_RES_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_WORLD_RES_RPL;
			RplCmd.nNum			= 0;

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_world_res,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL get_world_res()");
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_world_res,SQLERR");
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_WORLD_RES_T) + RplCmd.nNum*sizeof(WorldRes));
						P_DBS4WEBUSER_GET_WORLD_RES_T*	pRplCmd		= (P_DBS4WEBUSER_GET_WORLD_RES_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						WorldRes*	pRes		= (WorldRes*)wh_getptrnexttoptr(pRplCmd);
						// id,type,level,pos_x,pos_y,army_deploy,army_data,gold,pop,crystal,can_build,floor2,terrain_type
						while (q.FetchRow())
						{
							pRes->nID			= q.GetVal_64();
							pRes->nType			= q.GetVal_32();
							pRes->nLevel		= q.GetVal_32();
							pRes->nPosX			= q.GetVal_32();
							pRes->nPosY			= q.GetVal_32();
							WH_STRNCPY0(pRes->szArmyDeploy, q.GetStr());
							WH_STRNCPY0(pRes->szArmyData, q.GetStr());
							pRes->nGold			= q.GetVal_32();
							pRes->nPop			= q.GetVal_32();
							pRes->nCrystal		= q.GetVal_32();
							pRes->bCanBuild		= (bool)q.GetVal_32();
							pRes->nFloor2		= (unsigned char)q.GetVal_32();
							pRes->nSrcTerrainType	= q.GetVal_32();
							pRes->nForce		= q.GetVal_32();

							pRes++;
						}

						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
					else
					{
						SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
					}
				}
			}
		}
		break;
	case CMDID_ALLI_INSTANCE_CREATE_REQ:
	case CMDID_ALLI_INSTANCE_JOIN_REQ:
	case CMDID_ALLI_INSTANCE_EXIT_REQ:
	case CMDID_ALLI_INSTANCE_KICK_REQ:
	case CMDID_ALLI_INSTANCE_DESTROY_REQ:
	case CMDID_ALLI_INSTANCE_CONFIG_HERO_REQ:
	case CMDID_ALLI_INSTANCE_START_REQ:
	case CMDID_ALLI_INSTANCE_READY_REQ:
	case CMDID_ALLI_INSTANCE_GET_LIST_REQ:
	case CMDID_ALLI_INSTANCE_GET_CHAR_DATA_REQ:
	case CMDID_ALLI_INSTANCE_GET_HERO_DATA_REQ:
	case CMDID_ALLI_INSTANCE_GET_STATUS_REQ:
	case CMDID_ALLI_INSTANCE_GET_LOOT_REQ:
	case CMDID_ALLI_INSTANCE_SAVE_HERO_DEPLOY_REQ:
	case CMDID_ALLI_INSTANCE_START_COMBAT_REQ:
	case CMDID_ALLI_INSTANCE_DEAL_MARCH_ADVACNE_REQ:
	case CMDID_ALLI_INSTANCE_DEAL_MARCH_BACK_REQ:
	case CMDID_ALLI_INSTANCE_DEAL_COMBAT_RESULT_REQ:
	case CMDID_ALLI_INSTANCE_GET_COMBAT_RESULT_EVENT_REQ:
	case CMDID_ALLI_INSTANCE_GET_INSTANCE_ID_REQ:
	case CMDID_ALLI_INSTANCE_GET_COMBAT_LOG_REQ:
	case CMDID_ALLI_INSTANCE_GET_SIMPLE_COMBAT_LOG_REQ:
	case CMDID_ALLI_INSTANCE_STOP_COMBAT_REQ:
	case CMDID_ALLI_INSTANCE_SUPPLY_REQ:
	case CMDID_ALLI_INSTANCE_SYSTEM_DESTROY_REQ:
	case CMDID_ALLI_INSTANCE_GET_DEFENDER_DEPLOY_REQ:
		{
			ThreadDealCmd_AlliInstance(pMySQL, pCDCmd, vectRawBuf);
		}
		break;

	case CMDID_GET_HERO_SIMPLE_DATA_REQ:
		{
			P_DBS4WEB_GET_HERO_SIMPLE_DATA_T*	pReq	= (P_DBS4WEB_GET_HERO_SIMPLE_DATA_T*)pCmd;
			P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_HERO_SIMPLE_DATA_RPL;
			RplCmd.nID			= pReq->nID;
			RplCmd.nReason		= pReq->nReason;
			RplCmd.nCombatType	= pReq->nCombatType;
			RplCmd.nNum			= 0;
			
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_hero_simple_data,DBERR");
			}
			else
			{
				char	szBuf[1024]	= "";
				tty_id_t*	pID		= (tty_id_t*)wh_getptrnexttoptr(pReq);
				int		nLength		= sprintf(szBuf, "0x%"WHINT64PRFX"X", *pID);
				pID++;
				for (int i=1; i<pReq->nNum; i++)
				{
					nLength			+= sprintf(szBuf+nLength, ",0x%"WHINT64PRFX"X", *pID);
					++pID;
				}
				szBuf[nLength]		= 0;
				q.SpawnQuery("SELECT hero_id,army_type,army_level,army_num,army_prof,healthstate,level,exp,status"
					" FROM hire_heros WHERE hero_id in (%s)", szBuf);
				int	nPreRet			= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_hero_simple_data,SQLERR");
				}
				else
				{
					RplCmd.nNum		= q.NumRows();
					if (RplCmd.nNum == 0)
					{
						return 0;
					}
					vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T)+sizeof(HeroSimpleData)*RplCmd.nNum);
					P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T*	pRplCmd	= (P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T*)vectRawBuf.getbuf();
					memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
					HeroSimpleData*	pHero		= (HeroSimpleData*)wh_getptrnexttoptr(pRplCmd);
					while (q.FetchRow())
					{
						pHero->nHeroID			= q.GetVal_64();
						pHero->nArmyType		= q.GetVal_32();
						pHero->nArmyLevel		= q.GetVal_32();
						pHero->nArmyNum			= q.GetVal_32();
						pHero->nProf			= q.GetVal_32();
						pHero->nHealthState		= q.GetVal_32();
						pHero->nLevel			= q.GetVal_32();
						pHero->nExp				= q.GetVal_32();
						pHero->nStatus			= q.GetVal_32();
						pHero++;
					}
					SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
				}
			}
		}
		break;
	case CMDID_GET_INSTANCE_SIMPLE_DATA_REQ:
		{
			P_DBS4WEB_GET_INSTANCE_SIMPLE_DATA_T*	pReq	= (P_DBS4WEB_GET_INSTANCE_SIMPLE_DATA_T*)pCmd;
			P_DBS4WEBUSER_GET_INSTANCE_SIMPLE_DATA_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_GET_INSTANCE_SIMPLE_DATA_RPL;
			RplCmd.nNum			= 0;

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_all_instance,DBERR");
			}
			else
			{
				q.SpawnQuery("CALL alli_instance_get_all_instance()");
				int	nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"alli_instance_get_all_instance,SQLERR");
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_GET_INSTANCE_SIMPLE_DATA_T) + RplCmd.nNum*sizeof(SimpleInstanceData));
						P_DBS4WEBUSER_GET_INSTANCE_SIMPLE_DATA_T*	pRplCmd	= (P_DBS4WEBUSER_GET_INSTANCE_SIMPLE_DATA_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						SimpleInstanceData*	pUnit	= (SimpleInstanceData*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nInstanceID		= q.GetVal_64();
							pUnit->nAccountID		= q.GetVal_64();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;

	case CMDID_ARENA_UPLOAD_DATA_REQ:
	case CMDID_ARENA_GET_RANK_LIST_REQ:
	case CMDID_ARENA_GET_STATUS_REQ:
	case CMDID_ARENA_PAY_REQ:
	case CMDID_ARENA_DEAL_REWARD_EVENT_REQ:
	case CMDID_ARENA_GET_DEPLOY_REQ:
		{
			ThreadDealCmd_Arena(pMySQL, pCDCmd, vectRawBuf);
		}
		break;
	case CMDID_HERO_SUPPLY_REQ:
		{
			P_DBS4WEB_HERO_SUPPLY_T*	pReq	= (P_DBS4WEB_HERO_SUPPLY_T*)pCmd;
			P_DBS4WEBUSER_HERO_SUPPLY_T	RplCmd;
			RplCmd.nCmd				= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd			= CMDID_HERO_SUPPLY_RPL;
			RplCmd.nAccountID		= pReq->nAccountID;
			RplCmd.nCombatType		= pReq->nCombatType;
			RplCmd.nRst				= STC_GAMECMD_HERO_SUPPLY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= STC_GAMECMD_HERO_SUPPLY_T::RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"hero_supply,DBERR");
			}
			else
			{
				const char*	cszHeroIDs	= (const char*)wh_getptrnexttoptr(pReq);
				q.SpawnQuery("CALL hero_supply(0x%"WHINT64PRFX"X,%d,'%s')"
							, pReq->nAccountID, pReq->nCombatType, cszHeroIDs);
				int	nPreRet			= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst			= STC_GAMECMD_HERO_SUPPLY_T::RST_SQL_ERR;
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"hero_supply,SQLERR");
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_back_ids
						int*	pLengths	= q.FetchLengths();
						RplCmd.nRst			= q.GetVal_32();
						if (RplCmd.nRst == STC_GAMECMD_HERO_SUPPLY_T::RST_OK)
						{
							char	szBuf[1024]	= "";
							int		nLength		= pLengths[1]+1;
							memcpy(szBuf, q.GetStr(), nLength);
							PushHeroSimpleData(pCDCmd->nConnecterID, q, pReq->nCombatType, pReq->nAccountID, szBuf, STC_GAMECMD_HERO_SIMPLE_DATA_T::reason_self_supply);
						
							nLength				= sprintf(szBuf, "0x%"WHINT64PRFX"X", pReq->nAccountID);
							szBuf[nLength]		= 0;
							PushDrug(pCDCmd->nConnecterID, q, szBuf);
						}
					}
					else
					{
						RplCmd.nRst			= STC_GAMECMD_HERO_SUPPLY_T::RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"hero_supply,SQLERR,NO RESULT");
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case CMDID_REFRESH_DRUG_REQ:
		{
			P_DBS4WEB_REFRESH_DRUG_T*	pReq	= (P_DBS4WEB_REFRESH_DRUG_T*)pCmd;
			P_DBS4WEBUSER_REFRESH_DRUG_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_REFRESH_DRUG_RPL;
			RplCmd.nNum			= 0;

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
			}
			else
			{
				const char*	cszAccountIDs	= (const char*)wh_getptrnexttoptr(pReq);
				q.SpawnQuery("SELECT account_id,drug FROM common_characters WHERE account_id IN (%s)", cszAccountIDs);
				int	nPreRet		= 0;
				q.GetResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
				}
				else
				{
					RplCmd.nNum	= q.NumRows();
					if (RplCmd.nNum > 0)
					{
						vectRawBuf.resize(sizeof(P_DBS4WEBUSER_REFRESH_DRUG_T) + RplCmd.nNum*sizeof(P_DBS4WEBUSER_REFRESH_DRUG_T::DrugInfo));
						P_DBS4WEBUSER_REFRESH_DRUG_T*	pRplCmd	= (P_DBS4WEBUSER_REFRESH_DRUG_T*)vectRawBuf.getbuf();
						memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
						P_DBS4WEBUSER_REFRESH_DRUG_T::DrugInfo*	pUnit	= (P_DBS4WEBUSER_REFRESH_DRUG_T::DrugInfo*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							pUnit->nAccountID	= q.GetVal_64();
							pUnit->nDrug		= q.GetVal_32();

							pUnit++;
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;
	case CMDID_SET_VIP_DISPLAY_REQ:
		{
			P_DBS4WEB_SET_VIP_DISPLAY_T*	pReq	= (P_DBS4WEB_SET_VIP_DISPLAY_T*)pCmd;
			P_DBS4WEBUSER_SET_VIP_DISPLAY_T	RplCmd;
			RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
			RplCmd.nSubCmd		= CMDID_SET_VIP_DISPLAY_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.bVipDisplay	= pReq->bVipDisplay;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_SET_VIP_DISPLAY_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst		= STC_GAMECMD_OPERATE_SET_VIP_DISPLAY_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("UPDATE common_characters SET vip_display=%d WHERE account_id=0x%"WHINT64PRFX"X", (int)pReq->bVipDisplay, pReq->nAccountID);
				int	nPreRet		= 0;
				q.Execute(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst	= STC_GAMECMD_OPERATE_SET_VIP_DISPLAY_T::RST_SQL_ERR;
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;

	case CMDID_POSITION_MARK_ADD_RECORD_REQ:
	case CMDID_POSITION_MARK_CHG_RECORD_REQ:
	case CMDID_POSITION_MARK_DEL_RECORD_REQ:
	case CMDID_POSITION_MARK_GET_RECORD_REQ:
		{
			ThreadDealCmd_PositionMark(pMySQL, pCDCmd, vectRawBuf);
		}
		break;
	default:
		{

		}
		break;
	}

	return 0;
}
