#include "../inc/GDB4Web_i.h"
#include "../../Common/inc/tty_lp_client_gamecmd_packet.h"
#include <errno.h>

using namespace n_pngs;

int		GDB4Web_i::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	return 0;
}

int		GDB4Web_i::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
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
					m_vectConnecters.delvalue(pCmd->nParam1);
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_RT)"GDB4Web_i::DealCmdIn_One,disconnect,cntr id:0x%X", pCmd->nParam1);
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
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_RT)"GDB4Web_i::DealCmdIn_One,%d,unknown,%d", nCmd, nDSize);
		}
		break;
	}
	return 0;
}

int		GDB4Web_i::DealCmdIn_One_PNGS_TR2CD_CMD(const void* pData, int nDSize)
{
	PNGS_TR2CD_CMD_T*	pCDCmd	= (PNGS_TR2CD_CMD_T*)pData;
	GS4WEB_CMD_BASE_T*	pCmd	= (GS4WEB_CMD_BASE_T*)pCDCmd->pData;
	switch (pCmd->nCmd)
	{
	case P_GS4WEB_DB_REQ_CMD:
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

int		GDB4Web_i::ThreadDealCmd(void* pData, int nDSize, whtid_t tid, whvector<char>& vectRawBuf)
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
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_RT)"GDB4Web_i::ThreadDealCmd, no availabel mysql handle,tid 0x%X", tid);
		return -1;
	}

	PNGS_TR2CD_CMD_T*	pCDCmd		= (PNGS_TR2CD_CMD_T*)pData;
	GS4WEB_CMD_BASE_T*	pCmd		= (GS4WEB_CMD_BASE_T*)wh_getptrnexttoptr(pCDCmd);	// pCDCmd->pData无意义,已经失效
	
	switch (pCmd->nSubCmd)		// 上层已经过滤了,大指令为P_GS4WEB_DB_REQ_CMD
	{
	case DB_ACCOUNT_LOGIN_REQ:
		{
			GS4WEB_DB_ACCOUNT_LOGIN_REQ_T*	pLoginCmd	= (GS4WEB_DB_ACCOUNT_LOGIN_REQ_T*)pCmd;

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,LOGIN begin,%s,%d", __FUNCTION__, pLoginCmd->szAccount, (int)pLoginCmd->nGroupIdx);

			GS4WEB_DB_ACCOUNT_LOGIN_RPL_T	RplCmd;
			RplCmd.nCmd			= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd		= DB_ACCOUNT_LOGIN_RPL;
			RplCmd.nGroupIdx	= 0;
			RplCmd.nAccountID	= 0;
			WH_STRNCPY0(RplCmd.szAccountName, pLoginCmd->szAccount);
			RplCmd.nFromGroupIdx= pLoginCmd->nGroupIdx;
			RplCmd.nRst			= TTY_LOGIN_RST_SUCCEED;
			RplCmd.szDeviceID[0]		= 0;
			RplCmd.szPass[0]			= 0;
			RplCmd.nGiftAppID	= 0;
			memcpy(RplCmd.nExt, pLoginCmd->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query		q(pMySQL, QUERY_LEN);
			int					nRst	= 0;
			int					nStep	= 0;
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using,%s,%d", __FUNCTION__, pLoginCmd->szAccount, (int)pLoginCmd->nGroupIdx);
				RplCmd.nRst		= TTY_LOGIN_RST_DBERR;
				nStep			= 1;
			}
			else
			{
				q.SpawnQuery("call account_login('%s','%s','%s',%d,%u,%d,%s)"
					, pLoginCmd->szAccount, pLoginCmd->szPass, pLoginCmd->szDeviceID, (int)m_cfgInfo.bAccountLoginNeedCheckPass, pLoginCmd->nDeviceType, (int)pLoginCmd->nGroupIdx, SP_RESULT);
				int		pre_ret;
				q.ExecuteSPWithResult(pre_ret);
				if (pre_ret != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= TTY_LOGIN_RST_SQLERR;
					nStep			= 2;
				}
				else
				{
					if (q.FetchRow())
					{
						int	ret		= q.GetVal_32();
						switch (ret)
						{
						case -1:	// 数据库出错
							{
								RplCmd.nRst			= TTY_LOGIN_RST_BADACCOUNT;
								nStep				= 3;
							}
							break;
						case -10:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_ACCOUNT_BANNED;
								nStep				= 10;
							}
							break;
						case -11:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_DEVICE_BANNED;
								nStep				= 11;
							}
							break;
						case -12:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_GROUP_ERR;
								nStep				= 12;
							}
							break;
						case -100:	// 数据库出错
						case -101:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_SQLERR;
								nStep				= 4;
							}
							break;
						case -2:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_BADPASS;
								nStep				= 5;
							}
							break;
						case 0:		// 正常
							{
								RplCmd.nRst			= TTY_LOGIN_RST_SUCCEED;
								RplCmd.nAccountID	= q.GetVal_64();
								RplCmd.nBinded		= (unsigned char)q.GetVal_32();
								WH_STRNCPY0(RplCmd.szDeviceID, q.GetStr());
								WH_STRNCPY0(RplCmd.szPass, q.GetStr());
								RplCmd.nGiftAppID	= q.GetVal_32();
							}
							break;
						}
					}
					else
					{
						// 执行存储过程出错
						RplCmd.nRst		= TTY_LOGIN_RST_SQLERR;
						nStep			= 100;
					}
				}

				// 结果返回
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,LOGIN end,%s,%d,%d,%d,%d", __FUNCTION__, pLoginCmd->szAccount, (int)pLoginCmd->nGroupIdx, (int)RplCmd.nGroupIdx, (int)RplCmd.nRst, nStep);
			}
		}
		break;
	case DB_ACCOUNT_LOGOUT_REQ:
		{
			// 这个指令没必要返回
			GS4WEB_DB_ACCOUNT_LOGOUT_REQ_T*	pLogoutCmd	= (GS4WEB_DB_ACCOUNT_LOGOUT_REQ_T*)pCmd;

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,LOGOUT begin,0x%"WHINT64PRFX"X", __FUNCTION__, pLogoutCmd->nAccountID);

			dia_mysql_query		q(pMySQL, QUERY_LEN);
			int			nRst	= 0;		// 仅仅用来标记哪里出错了,没啥意思
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using,0x%"WHINT64PRFX"X", __FUNCTION__, pLogoutCmd->nAccountID);
				nRst			= 0xFF;
			}
			else
			{
				q.SpawnQuery("call account_logout(0x%"WHINT64PRFX"X)", pLogoutCmd->nAccountID);
				int	pre_ret		= 0;
				q.ExecuteSPWithResult(pre_ret);
			}

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,LOGOUT end,0x%"WHINT64PRFX"X", __FUNCTION__, pLogoutCmd->nAccountID);
		}
		break;
	case DB_ADD_NOTIFY_REQ:
		{
			GS4WEB_DB_ADD_NOTIFY_REQ_T*	pAddNotify	= (GS4WEB_DB_ADD_NOTIFY_REQ_T*)pCmd;
			char*	pTitle				= (char*)wh_getptrnexttoptr(pAddNotify);
			int*	pContentLen			= (int*)wh_getoffsetaddr(pTitle, pAddNotify->nTitleLen);
			char*	pContent			= (char*)wh_getptrnexttoptr(pContentLen);

			vectRawBuf.resize(sizeof(GS4WEB_DB_ADD_NOTIFY_RPL_T) + Web_Notification_T::GetTotalSize(pAddNotify->nTitleLen, *pContentLen));
			GS4WEB_DB_ADD_NOTIFY_RPL_T&	RplCmd	= *(GS4WEB_DB_ADD_NOTIFY_RPL_T*)vectRawBuf.getbuf();
			RplCmd.nCmd					= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd				= DB_ADD_NOTIFY_RPL;
			RplCmd.nRst					= BD_RST_OK;
			Web_Notification_T*	pNotify	= (Web_Notification_T*)wh_getptrnexttoptr(&RplCmd);
			pNotify->nNotificationID	= 0;
			pNotify->nGroupID			= pAddNotify->nGroupID;
			pNotify->nTime				= pAddNotify->nTime;
			pNotify->nTitleLen			= pAddNotify->nTitleLen;
			char*	pDstTitle			= (char*)wh_getptrnexttoptr(pNotify);
			memcpy(pDstTitle, pTitle, pNotify->nTitleLen);
			int*	pDstContentLen		= (int*)wh_getoffsetaddr(pDstTitle, pNotify->nTitleLen);
			*pDstContentLen				= *pContentLen;
			memcpy(wh_getptrnexttoptr(pDstContentLen), pContent, *pContentLen);
			memcpy(RplCmd.nExt, pAddNotify->nExt, sizeof(pAddNotify->nExt));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,AddNotify begin,%d,%u,%d", __FUNCTION__, pAddNotify->nGroupID, pAddNotify->nTime, pAddNotify->nExt[0]);

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using,%d,%u,%d", __FUNCTION__, pAddNotify->nGroupID, pAddNotify->nTime, pAddNotify->nExt[0]);
			}
			else
			{
				q.SpawnQuery("call add_notification(%d,%u,'", pAddNotify->nGroupID, pAddNotify->nTime);
				q.BinaryToString(strlen(pTitle), pTitle);
				q.StrMove("','");
				q.BinaryToString(strlen(pContent), pContent);
				q.StrMove("',%s)", SP_RESULT);

				int	pre_ret			= 0;
				q.ExecuteSPWithResult(pre_ret);
				if (pre_ret != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= BD_RST_DB_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						RplCmd.nRst					= q.GetVal_32();
						pNotify->nNotificationID	= q.GetVal_32();
					}
					else
					{
						RplCmd.nRst		= BD_RST_DB_ERR;
					}
				}
			}

			// 这个结果需要发送给主线程进行进一步处理
			SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,AddNotify end,%d,%u,%d,%d", __FUNCTION__, pAddNotify->nGroupID, pAddNotify->nTime, pAddNotify->nExt[0], RplCmd.nRst);
		}
		break;
	case DB_LOAD_NOTIFY_REQ:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,LoadNotify begin", __FUNCTION__);

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			int		nRst	= BD_RST_OK;
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using", __FUNCTION__);
			}
			else
			{
				q.SpawnQuery("select id,group_id,time,title,content from notification order by id desc");
				int	pre_ret			= 0;
				q.GetResult(pre_ret);
				if (pre_ret != MYSQL_QUERY_NORMAL)
				{
					nRst		= BD_RST_DB_ERR;
				}
				else
				{
					int	nNum	= q.NumRows();
					if (nNum == 0)
					{
						GS4WEB_DB_LOAD_NOTIFY_RPL_T	Cmd;
						Cmd.nCmd		= P_GS4WEB_DB_RPL_CMD;
						Cmd.nSubCmd		= DB_LOAD_NOTIFY_RPL;
						Cmd.nNum		= 0;
						Cmd.nRst		= BD_RST_OK;

						SendMsg_MT(pCDCmd->nConnecterID, &Cmd, sizeof(Cmd));
					}
					else
					{
						vectRawBuf.resize(sizeof(GS4WEB_DB_LOAD_NOTIFY_RPL_T));
						GS4WEB_DB_LOAD_NOTIFY_RPL_T*	pRplCmd	= (GS4WEB_DB_LOAD_NOTIFY_RPL_T*)vectRawBuf.getbuf();
						pRplCmd->nCmd	= P_GS4WEB_DB_RPL_CMD;
						pRplCmd->nSubCmd= DB_LOAD_NOTIFY_RPL;
						pRplCmd->nNum	= nNum;
						pRplCmd->nRst	= BD_RST_OK;
						int*	pLengths	= NULL;
						while (q.FetchRow())
						{
							pLengths		= q.FetchLengths();
							int	nTitleLen	= pLengths[3]+1;
							int	nContentLen	= pLengths[4]+1;
							// 通知内容长度下标为3
							Web_Notification_T*	pNotification	= (Web_Notification_T*)vectRawBuf.pushn_back(Web_Notification_T::GetTotalSize(nTitleLen, nContentLen));
							if (pNotification == NULL)
							{
								GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"cannot alloc memory");
								break;
							}
							pNotification->nNotificationID		= q.GetVal_32();
							pNotification->nGroupID				= q.GetVal_32();
							pNotification->nTime				= q.GetVal_32();
							pNotification->nTitleLen			= nTitleLen;
							char*	pTitle						= (char*)wh_getptrnexttoptr(pNotification);
							memcpy(pTitle, q.GetStr(), nTitleLen);
							int*	pContentLen					= (int*)wh_getoffsetaddr(pTitle, nTitleLen);
							*pContentLen						= nContentLen;
							memcpy(wh_getptrnexttoptr(pContentLen), q.GetStr(), *pContentLen);
						}
						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,LoadNotify end,%d", __FUNCTION__, nRst);
		}
		break;
	case DB_DEL_NOTIFY_BY_ID_REQ:
		{
			GS4WEB_DB_DEL_NOTIFY_BY_ID_REQ_T*	pDelNotify	= (GS4WEB_DB_DEL_NOTIFY_BY_ID_REQ_T*)pCmd;
			GS4WEB_DB_DEL_NOTIFY_BY_ID_RPL_T	RplCmd;
			RplCmd.nCmd				= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd			= DB_DEL_NOTIFY_BY_ID_RPL;
			RplCmd.nNotificationID	= pDelNotify->nNotificationID;
			RplCmd.nRst				= BD_RST_OK;
			memcpy(RplCmd.nExt, pDelNotify->nExt, sizeof(RplCmd.nExt));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyByID begin", __FUNCTION__);

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using", __FUNCTION__);
			}
			else
			{
				q.SpawnQuery("delete from notification where id=%d", pDelNotify->nNotificationID);
				int	pre_ret			= 0;
				q.Execute(pre_ret);
				if (pre_ret != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= BD_RST_DB_ERR;
				}
			}

			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyByID end,%d", __FUNCTION__, RplCmd.nRst);
		}
		break;
	case DB_DEL_NOTIFY_BT_REQ:
		{
			GS4WEB_DB_DEL_NOTIFY_BT_REQ_T*	pDelNotify	= (GS4WEB_DB_DEL_NOTIFY_BT_REQ_T*)pCmd;
			GS4WEB_DB_DEL_NOTIFY_BT_RPL_T	RplCmd;
			RplCmd.nCmd				= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd			= DB_DEL_NOTIFY_BT_RPL;
			RplCmd.nTime			= pDelNotify->nTime;
			RplCmd.nRst				= BD_RST_OK;
			memcpy(RplCmd.nExt, pDelNotify->nExt, sizeof(RplCmd.nExt));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyBeforeTime begin,%u", __FUNCTION__, pDelNotify->nTime);

			if (RplCmd.nTime == 0)
			{
				RplCmd.nTime		= BD_RST_TIME_FORMAT_ERR;
			}
			else
			{
				dia_mysql_query	q(pMySQL, QUERY_LEN);
				if (!q.Connected())
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using", __FUNCTION__);
				}
				else
				{
					q.SpawnQuery("delete from notification where time<=%u", pDelNotify->nTime);
					int	pre_ret			= 0;
					q.Execute(pre_ret);
					if (pre_ret != MYSQL_QUERY_NORMAL)
					{
						RplCmd.nRst		= BD_RST_DB_ERR;
					}
				}
			}

			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyBeforeTime end,%d,%u", __FUNCTION__, RplCmd.nRst, pDelNotify->nTime);

		}
		break;
	case DB_DEL_NOTIFY_AT_REQ:
		{
			GS4WEB_DB_DEL_NOTIFY_AT_REQ_T*	pDelNotify	= (GS4WEB_DB_DEL_NOTIFY_AT_REQ_T*)pCmd;
			GS4WEB_DB_DEL_NOTIFY_AT_RPL_T	RplCmd;
			RplCmd.nCmd				= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd			= DB_DEL_NOTIFY_AT_RPL;
			RplCmd.nTime			= pDelNotify->nTime;
			RplCmd.nRst				= BD_RST_OK;
			memcpy(RplCmd.nExt, pDelNotify->nExt, sizeof(RplCmd.nExt));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyAfterTime begin,%u", __FUNCTION__, pDelNotify->nTime);

			if (RplCmd.nTime == 0)
			{
				RplCmd.nTime		= BD_RST_TIME_FORMAT_ERR;
			}
			else
			{
				dia_mysql_query	q(pMySQL, QUERY_LEN);
				if (!q.Connected())
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using", __FUNCTION__);
				}
				else
				{
					q.SpawnQuery("delete from notification where time>=%u", pDelNotify->nTime);
					int	pre_ret			= 0;
					q.Execute(pre_ret);
					if (pre_ret != MYSQL_QUERY_NORMAL)
					{
						RplCmd.nRst		= BD_RST_DB_ERR;
					}
				}
			}

			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyAfterTime end,%d,%u", __FUNCTION__, RplCmd.nRst, pDelNotify->nTime);

		}
		break;
	case DB_DEL_NOTIFY_ALL_REQ:
		{
			GS4WEB_DB_DEL_NOTIFY_ALL_REQ_T*	pDelNotify	= (GS4WEB_DB_DEL_NOTIFY_ALL_REQ_T*)pCmd;
			GS4WEB_DB_DEL_NOTIFY_ALL_RPL_T	RplCmd;
			RplCmd.nCmd				= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd			= DB_DEL_NOTIFY_ALL_RPL;
			RplCmd.nRst				= BD_RST_OK;
			memcpy(RplCmd.nExt, pDelNotify->nExt, sizeof(RplCmd.nExt));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyAll begin", __FUNCTION__);


			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using", __FUNCTION__);
			}
			else
			{
				q.SpawnQuery("delete from notification");
				int	pre_ret			= 0;
				q.Execute(pre_ret);
				if (pre_ret != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= BD_RST_DB_ERR;
				}

			}

			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyAll end,%d", __FUNCTION__, RplCmd.nRst);

		}
		break;
	case DB_DEL_NOTIFY_BY_GROUP_REQ:
		{
			GS4WEB_DB_DEL_NOTIFY_BY_GROUP_REQ_T*	pDelNotify	= (GS4WEB_DB_DEL_NOTIFY_BY_GROUP_REQ_T*)pCmd;
			GS4WEB_DB_DEL_NOTIFY_BY_GROUP_RPL_T		RplCmd;
			RplCmd.nCmd				= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd			= DB_DEL_NOTIFY_BY_GROUP_RPL;
			RplCmd.nGroupID			= pDelNotify->nGroupID;
			RplCmd.nRst				= BD_RST_OK;
			memcpy(RplCmd.nExt, pDelNotify->nExt, sizeof(RplCmd.nExt));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyAll begin", __FUNCTION__);

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using", __FUNCTION__);
			}
			else
			{
				q.SpawnQuery("delete from notification where group_id=%d", pDelNotify->nGroupID);
				int	pre_ret			= 0;
				q.Execute(pre_ret);
				if (pre_ret != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= BD_RST_DB_ERR;
				}

			}

			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,DelNotifyAll end,%d", __FUNCTION__, RplCmd.nRst);

		}
		break;
	case DB_ACCOUNT_CREATE_REQ:
		{
			GS4WEB_DB_ACCOUNT_CREATE_REQ_T*		pCreateAccount	= (GS4WEB_DB_ACCOUNT_CREATE_REQ_T*)pCmd;
			GS4WEB_DB_ACCOUNT_CREATE_RPL_T		RplCmd;
			RplCmd.nCmd				= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd			= DB_ACCOUNT_CREATE_RPL;
			RplCmd.nRst				= TTY_ACCOUNT_CREATE_RST_OK;
			RplCmd.nAccountID		= 0;
			RplCmd.szAccount[0]		= 0;
			RplCmd.nFromGroupIdx	= pCreateAccount->nGroupIdx;
			RplCmd.nGiftAppID		= 0;
			WH_STRNCPY0(RplCmd.szPass, pCreateAccount->szPass);
			memcpy(RplCmd.nExt, pCreateAccount->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				RplCmd.nRst			= TTY_ACCOUNT_CREATE_RST_DB_ERR;
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using,CreateAccount,0x%X", __FUNCTION__, pCreateAccount->nExt[0]);
			}
			else
			{
				q.SpawnQuery("CALL account_create('%s', '%s', %d, '%s', %d, %u, %d)"
					, pCreateAccount->szAccountName, pCreateAccount->szPass, (int)pCreateAccount->bUseRandomAccountName
					, pCreateAccount->szDeviceID, pCreateAccount->nDeviceType, pCreateAccount->nAppID, (int)pCreateAccount->nGroupIdx);
				int	pre_ret			= 0;
				q.ExecuteSPWithResult(pre_ret);
				if (pre_ret != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= TTY_ACCOUNT_CREATE_RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						int	nRst		= q.GetVal_32();
						switch (nRst)
						{
						case 0:
							{
								RplCmd.nAccountID	= q.GetVal_64();
								WH_STRNCPY0(RplCmd.szAccount, q.GetStr());
								RplCmd.nGiftAppID	= q.GetVal_32();
								GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"CreateAccount,0x%"WHINT64PRFX"X,%s", RplCmd.nAccountID, RplCmd.szAccount);
							}
							break;
						case -1:
							{
								RplCmd.nRst			= TTY_ACCOUNT_CREATE_RST_NAME_DUP;
								GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"CreateAccount,NAMEDUP,0x%X,%d", pCreateAccount->nExt[0], nRst);
							}
							break;
						case -3:
							{
								RplCmd.nRst			= TTY_ACCOUNT_CREATE_RST_DEVICE_BINDED_GROUP;
								GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"CreateAccount,BINDED_GROUP,0x%X,%d", pCreateAccount->nExt[0], nRst);
							}
							break;
						case -10:
							{
								RplCmd.nRst			= TTY_ACCOUNT_CREATE_RST_DEVICE_BANNED;
								GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"CreateAccount,BAN_DEVICE,0x%X,%d", pCreateAccount->nExt[0], nRst);
							}
							break;
						default:
							{
								RplCmd.nRst			= TTY_ACCOUNT_CREATE_RST_SQL_ERR;
								GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"CreateAccount ERROR,0x%X,%d", pCreateAccount->nExt[0], nRst);
							}
							break;
						}
					}
					else
					{
						RplCmd.nRst			= TTY_ACCOUNT_CREATE_RST_SQL_ERR;
						GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"CreateAccount ERROR,0x%X", pCreateAccount->nExt[0]);
					}
				}
			}

			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case DB_ACCOUNT_LOGIN_DEVICEID_REQ:
		{
			GS4WEB_DB_ACCOUNT_LOGIN_DEVICEID_REQ_T*	pLoginCmd	= (GS4WEB_DB_ACCOUNT_LOGIN_DEVICEID_REQ_T*)pCmd;

			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,LOGIN_DEVICEID begin,%s,%d", __FUNCTION__, pLoginCmd->szDeviceID, (int)pLoginCmd->nGroupIdx);

			GS4WEB_DB_ACCOUNT_LOGIN_RPL_T	RplCmd;
			RplCmd.nCmd			= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd		= DB_ACCOUNT_LOGIN_RPL;
			RplCmd.nGroupIdx	= 0;
			RplCmd.nAccountID	= 0;
			RplCmd.szAccountName[0]		= 0;
			RplCmd.nFromGroupIdx= pLoginCmd->nGroupIdx;
			RplCmd.szDeviceID[0]		= 0;
			RplCmd.szPass[0]			= 0;
			RplCmd.nGiftAppID	= 0;
			RplCmd.nRst			= TTY_LOGIN_RST_SUCCEED;
			memcpy(RplCmd.nExt, pLoginCmd->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query		q(pMySQL, QUERY_LEN);
			int					nRst	= 0;
			int					nStep	= 0;
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,mysql handle disconnected when using,%s,%d", __FUNCTION__, pLoginCmd->szDeviceID, (int)pLoginCmd->nGroupIdx);
				RplCmd.nRst		= TTY_LOGIN_RST_DBERR;
				nStep			= 1;
			}
			else
			{
				q.SpawnQuery("call account_login_with_device_id('%s', %u, %d)", pLoginCmd->szDeviceID, pLoginCmd->nDeviceType, (int)pLoginCmd->nGroupIdx);
				int		pre_ret;
				q.ExecuteSPWithResult(pre_ret);
				if (pre_ret != MYSQL_QUERY_NORMAL)
				{
					RplCmd.nRst		= TTY_LOGIN_RST_SQLERR;
					nStep			= 2;
				}
				else
				{
					if (q.FetchRow())
					{
						int	ret		= q.GetVal_32();
						switch (ret)
						{
						case -1:	// 数据库出错
							{
								RplCmd.nRst			= TTY_LOGIN_RST_NO_DEVICEID;
								nStep				= 3;
							}
							break;
						case 0:		// 正常
							{
								RplCmd.nRst			= TTY_LOGIN_RST_SUCCEED;
								RplCmd.nAccountID	= q.GetVal_64();
								RplCmd.nBinded		= (unsigned char)q.GetVal_32();
								WH_STRNCPY0(RplCmd.szDeviceID, q.GetStr());
								WH_STRNCPY0(RplCmd.szAccountName, q.GetStr());
								WH_STRNCPY0(RplCmd.szPass, q.GetStr());
								RplCmd.nGiftAppID	= q.GetVal_32();
							}
							break;
						case -10:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_ACCOUNT_BANNED;
								nStep				= 10;
							}
							break;
						case -11:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_DEVICE_BANNED;
								nStep				= 11;
							}
							break;
						case -12:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_GROUP_ERR;
								nStep				= 12;
							}
							break;
						default:
							{
								RplCmd.nRst			= TTY_LOGIN_RST_UNKNOWNERR;
							}
							break;
						}
					}
					else
					{
						// 执行存储过程出错
						RplCmd.nRst		= TTY_LOGIN_RST_SQLERR;
						nStep			= 100;
					}
				}

				// 结果返回
				SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));

				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_i_RT)"%s,LOGIN_DEVICEID end,%s,%d,%d,%d,%d", __FUNCTION__, pLoginCmd->szDeviceID, (int)pLoginCmd->nGroupIdx, (int)RplCmd.nGroupIdx, (int)RplCmd.nRst, nStep);
			}
		}
		break;
	case DB_REGISTER_REQ:
		{
			GS4WEB_DB_REGISTER_REQ_T*	pRegister	= (GS4WEB_DB_REGISTER_REQ_T*)pCmd;

			GS4WEB_DB_REGISTER_RPL_T	RplCmd;
			RplCmd.nCmd			= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd		= DB_REGISTER_RPL;
			RplCmd.nAccountID	= pRegister->nAccountID;
			RplCmd.bIsRegister	= false;
			RplCmd.nFromGroupIdx= pRegister->nGroupIdx;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_OK;
			memcpy(RplCmd.nExt, pRegister->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"register,DBERROR,0x%"WHINT64PRFX"X", pRegister->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_DB_ERR;
			}
			else
			{
				char*	pPass		= (char*)wh_getptrnexttoptr(pRegister);
				char*	pOldPass	= (char*)wh_getoffsetaddr(pPass, pRegister->nPassLen+sizeof(int));
				q.SpawnQuery("CALL register(0x%"WHINT64PRFX"X,'%s','%s','%s')"
					, pRegister->nAccountID, pRegister->szAccount, pPass, pOldPass);
				int		nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"register,SQLERROR,0x%"WHINT64PRFX"X", pRegister->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result,_is_register
						RplCmd.nRst			= q.GetVal_32();
						RplCmd.bIsRegister	= (bool)q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"register,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pRegister->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_REGISTER_ACCOUNT_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case DB_BIND_DEVICE_REQ:
		{
			GS4WEB_DB_BIND_DEVICE_REQ_T*	pReq	= (GS4WEB_DB_BIND_DEVICE_REQ_T*)pCmd;

			GS4WEB_DB_BIND_DEVICE_RPL_T	RplCmd;
			RplCmd.nCmd			= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd		= DB_BIND_DEVICE_RPL;
			RplCmd.nFromGroupIdx= pReq->nGroupIdx;
			RplCmd.nRst			= STC_GAMECMD_OPERATE_BIND_DEVICE_T::RST_OK;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"bind_device,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= STC_GAMECMD_OPERATE_BIND_DEVICE_T::RST_DB_ERR;
			}
			else
			{
				q.SpawnQuery("CALL bind_device(0x%"WHINT64PRFX"X,'%s',%d)", pReq->nAccountID, pReq->szDeviceID, (int)pReq->nGroupIdx);
				int		nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"bind_device,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= STC_GAMECMD_OPERATE_BIND_DEVICE_T::RST_SQL_ERR;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						RplCmd.nRst			= q.GetVal_32();
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"bind_device,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst		= STC_GAMECMD_OPERATE_BIND_DEVICE_T::RST_SQL_ERR;
					}
				}
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case DB_BAN_ACCOUNT_REQ:
		{
			GS4WEB_DB_BAN_ACCOUNT_REQ_T*	pReq	= (GS4WEB_DB_BAN_ACCOUNT_REQ_T*)pCmd;

			GS4WEB_DB_BAN_ACCOUNT_RPL_T		RplCmd;
			RplCmd.nCmd			= P_GS4WEB_DB_RPL_CMD;
			RplCmd.nSubCmd		= DB_BAN_ACCOUNT_RPL;
			RplCmd.nAccountID	= pReq->nAccountID;
			RplCmd.bBanDevice	= pReq->bBanDevice;
			RplCmd.szDeviceID[0]= 0;
			RplCmd.nRst			= 0;
			memcpy(RplCmd.nExt, pReq->nExt, sizeof(RplCmd.nExt));

			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"ban_account,DBERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
				RplCmd.nRst		= -200;
			}
			else
			{
				q.SpawnQuery("CALL ban_account(0x%"WHINT64PRFX"X,%d)", pReq->nAccountID, (int)pReq->bBanDevice);
				int		nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"ban_account,SQLERROR,0x%"WHINT64PRFX"X", pReq->nAccountID);
					RplCmd.nRst		= -201;
				}
				else
				{
					if (q.FetchRow())
					{
						// _result
						RplCmd.nRst			= q.GetVal_32();
						WH_STRNCPY0(RplCmd.szDeviceID, q.GetStr());
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_i_RT)"ban_account,SQLERROR,NO RESULT,0x%"WHINT64PRFX"X", pReq->nAccountID);
						RplCmd.nRst		= -201;
					}
				}
		
			}
			SendMsg_MT(pCDCmd->nConnecterID, &RplCmd, sizeof(RplCmd));
		}
		break;
	case DB_GET_ADCOLONY_REQ:
		{
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
			}
			else
			{
				q.SpawnQuery("CALL get_adcolony_transaction()");
				int		nPreRet		= 0;
				q.ExecuteSPWithResult(nPreRet, false);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
				}
				else
				{
					int	nNum		= q.NumRows();
					if (nNum > 0)
					{
						vectRawBuf.resize(sizeof(GS4WEB_DB_GET_ADCOLONY_RPL_T) + nNum*sizeof(GS4WEB_DB_GET_ADCOLONY_RPL_T::AdColony_T));
						GS4WEB_DB_GET_ADCOLONY_RPL_T*	pRplCmd	= (GS4WEB_DB_GET_ADCOLONY_RPL_T*)vectRawBuf.getbuf();
						pRplCmd->nCmd		= P_GS4WEB_DB_RPL_CMD;
						pRplCmd->nSubCmd	= DB_GET_ADCOLONY_RPL;
						pRplCmd->nNum		= nNum;
						GS4WEB_DB_GET_ADCOLONY_RPL_T::AdColony_T*	pAdColony	= (GS4WEB_DB_GET_ADCOLONY_RPL_T::AdColony_T*)wh_getptrnexttoptr(pRplCmd);
						while (q.FetchRow())
						{
							// t.transaction_id,t.time,t.name,t.account_id,t.amount,a.group_id
							pAdColony->nTransactionID	= q.GetVal_64();
							pAdColony->nTime			= q.GetVal_32();
							WH_STRNCPY0(pAdColony->szName, q.GetStr());
							pAdColony->nAccountID		= q.GetVal_64();
							pAdColony->nAmount			= q.GetVal_32();
							pAdColony->nGroupID			= q.GetVal_32();

							pAdColony++;
						}

						SendMsg_MT(pCDCmd->nConnecterID, vectRawBuf.getbuf(), vectRawBuf.size());
					}
				}
			}
		}
		break;
	case DB_DEAL_ADCOLONY_REQ:
		{
			GS4WEB_DB_DEAL_ADCOLONY_REQ_T*	pReq	= (GS4WEB_DB_DEAL_ADCOLONY_REQ_T*)pCmd;
			dia_mysql_query	q(pMySQL, QUERY_LEN);
			if (!q.Connected())
			{
			}
			else
			{
				q.SpawnQuery("CALL deal_adcolony(0x%"WHINT64PRFX"X)", pReq->nTransactionID);
				int	nPreRet	= 0;
				q.Execute(nPreRet);
			}
		}
		break;
	}

	return 0;
}
