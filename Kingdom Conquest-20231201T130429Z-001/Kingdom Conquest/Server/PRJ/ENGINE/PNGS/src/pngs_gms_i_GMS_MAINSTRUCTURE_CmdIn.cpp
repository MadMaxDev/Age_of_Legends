// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CmdIn.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE模块中CmdIn处理部分实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-09-02
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_logic.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	// 先处理一些不用发送到消息队列中的指令
	switch( nCmd )
	{
		case	PNGSPACKET_2LOGIC_DISPHISTORY:
		{
		}
		break;
		case	GMSPACKET_2MS_CONFIG:
		{
			GMSPACKET_2MS_CONFIG_T	*pConfig	= (GMSPACKET_2MS_CONFIG_T *)pData;
			switch( pConfig->nSubCmd )
			{
			case	GMSPACKET_2MS_CONFIG_T::SUBCMD_REG_SVRCMD:
				{
					if( !m_setRegGZSSvrCmd.put(pConfig->nParam) )
					{
						// 说明应该已经被注册过一次了
						assert(0 && "This cmd is registered!");
						return	-1;
					}
				}
				break;
			default:
				break;
			}
		}
		break;
		case	PNGSPACKET_2LOGIC_REGCMDDEAL:
		{
			PNGSPACKET_2LOGIC_REGCMDDEAL_T	*pReg	= (PNGSPACKET_2LOGIC_REGCMDDEAL_T *)pData;
			if( !m_mapCmdReg2Logic.put(pReg->nCmdReg, pRstAccepter) )
			{
				// 说明应该已经被注册过一次了
				assert(0 && "This cmd is registered!");
				return	-1;
			}
		}
		break;
		default:
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(34,PNGS_RT)"GMS_MAINSTRUCTURE::DealCmdIn_One_Instant,%d,unknown,%p", nCmd, pRstAccepter);
		}
		break;
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	switch(nCmd)
	{
	case	GMSPACKET_2MS_CONFIG:
		{
			GMSPACKET_2MS_CONFIG_T	*pConfig	= (GMSPACKET_2MS_CONFIG_T *)pData;
			switch( pConfig->nSubCmd )
			{
			case	GMSPACKET_2MS_CONFIG_T::SUBCMD_SET_PLAYERNUM:
				{
					if( pConfig->nParam>m_cfginfo.nMaxPlayer )
					{
						pConfig->nParam	= m_cfginfo.nMaxPlayer;
					}
					m_nGMSLogicMaxPlayer		= pConfig->nParam;
				}
				break;
			default:
				break;
			}
		}
		break;
	case	PNGSPACKET_2LOGIC_SETMOOD:
		{
			PNGSPACKET_2LOGIC_SETMOOD_T	*pCmdSetMood	= (PNGSPACKET_2LOGIC_SETMOOD_T *)pData;
			switch( pCmdSetMood->nMood )
			{
			case	CMN::ILogic::MOOD_STOPPING:
				{
					// 默认就可以直接停止了
					m_nMood	= CMN::ILogic::MOOD_STOPPED;
					// 踢出所有用户（直接放在后面的服务器断开的时候自动做了）
					//for(UAF_Player_T::iterator it=m_pSHMData->m_Players.begin(); it!=m_pSHMData->m_Players.end(); ++it)
					//{
					//	TellCLSToKickPlayer((*it).nID);
					//}
					// 告诉所有其他服务器该结束了CAAFS、CLS和GZS（其中CLS会开始踢出所有用户）
					TellAllConnecterToQuit();
				}
				break;
			}
		}
		break;
	case	PNGSPACKET_2LOGIC_GMCMD:
		{
			// GM指令
			DealCmdIn_PNGSPACKET_2LOGIC_GMCMD(pRstAccepter, (PNGSPACKET_2LOGIC_GMCMD_T *)pData);
		}
		break;
	case	PNGSPACKET_2LOGIC_GMCMD_RST:
		{
			PNGSPACKET_2LOGIC_GMCMD_RST_T		*pCmd	= (PNGSPACKET_2LOGIC_GMCMD_RST_T *)pData;
			m_vectrawbuf.resize(wh_offsetof(SVR_GMTOOL_CMD_RST_T, data) + pCmd->nDSize);
			SVR_GMTOOL_CMD_RST_T				*pRst	= (SVR_GMTOOL_CMD_RST_T *)m_vectrawbuf.getbuf();
			pRst->nCmd		= SVR_GMTOOL_CMD_RST;
			pRst->nExt		= pCmd->nExt;
			memcpy(pRst->data, pCmd->pData, pCmd->nDSize);
			if( pCmd->nCntrID==-1 )
			{
				// 发送给所有的GMTOOL
				SendCmdToAllGMTOOL(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
			else
			{
				MYCNTRSVR::MYCNTR	*pCntr	= (MYCNTRSVR::MYCNTR *)m_CntrSvr.GetConnecterPtr(pCmd->nCntrID)->QueryInterface();
				if( pCntr )
				{
					pCntr->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
			}
		}
		break;
	case	GMSPACKET_2MS_CMD2ONEPLAYER:
		{
			GMSPACKET_2MS_CMD2ONEPLAYER_T		*pCmd	= (GMSPACKET_2MS_CMD2ONEPLAYER_T *)pData;
			SendCmdToPlayer(pCmd->nClientID, pCmd->nChannel, pCmd->pData, pCmd->nDSize);
		}
		break;
	case	GMSPACKET_2MS_CMD2MULTIPLAYER:
		{
			GMSPACKET_2MS_CMD2MULTIPLAYER_T		*pCmd	= (GMSPACKET_2MS_CMD2MULTIPLAYER_T *)pData;
			SendCmdToMultiPlayer(pCmd->panClientID, pCmd->nClientNum, pCmd->nChannel, pCmd->pData, pCmd->nDSize);
		}
		break;
	case	GMSPACKET_2MS_CMD2PLAYERWITHTAG:
		{
			GMSPACKET_2MS_CMD2PLAYERWITHTAG_T	*pCmd	= (GMSPACKET_2MS_CMD2PLAYERWITHTAG_T *)pData;
			SendCmdToAllPlayerByTag(pCmd->nTagIdx, pCmd->nTagVal, pCmd->nChannel, pCmd->pData, pCmd->nDSize, pCmd->nSvrIdx);
		}
		break;
	case	GMSPACKET_2MS_CMD2ALLPLAYER:
		{
			GMSPACKET_2MS_CMD2ALLPLAYER_T		*pCmd	= (GMSPACKET_2MS_CMD2ALLPLAYER_T *)pData;
			SendCmdToAllPlayer(pCmd->nChannel, pCmd->pData, pCmd->nDSize, pCmd->nSvrIdx);
		}
		break;
	case	GMSPACKET_2MS_SETPLAYERTAG:
		{
			GMSPACKET_2MS_SETPLAYERTAG_T		*pCmd	= (GMSPACKET_2MS_SETPLAYERTAG_T *)pData;
			SetPlayerTag(pCmd->nClientID, pCmd->nTagIdx, pCmd->nTagVal);
		}
		break;
	case	GMSPACKET_2MS_SETCLIENTGZSROUTE:
		{
			GMSPACKET_2MS_SETCLIENTGZSROUTE_T	*pCmd	= (GMSPACKET_2MS_SETCLIENTGZSROUTE_T *)pData;
			PlayerUnit	*pPlayer	= m_pSHMData->m_Players.GetPtrByID(pCmd->nClientID);
			if( pPlayer )
			{
				GMS_CLS_ROUTECLIENTDATATOGZS_T	CmdRoute;
				CmdRoute.nCmd					= GMS_CLS_ROUTECLIENTDATATOGZS;
				CmdRoute.nClientID				= pCmd->nClientID;
				CmdRoute.nSvrIdx				= pCmd->nSvrIdx;
				pPlayer->nSvrIdx				= pCmd->nSvrIdx;
				SendCmdToPlayerCLS(pCmd->nClientID, &CmdRoute, sizeof(CmdRoute));
			}
		}
		break;
	case	GMSPACKET_2MS_KICKPLAYER:
		{
			GMSPACKET_2MS_KICKPLAYER_T			*pCmd	= (GMSPACKET_2MS_KICKPLAYER_T *)pData;
			TellCLSToKickPlayer(pCmd->nClientID, pCmd->nKickSubCmd);
		}
		break;
	case	GMSPACKET_2MS_GZS_SVRCMD:
		{
			GMSPACKET_2MS_GZS_SVRCMD_T			*pCmd	= (GMSPACKET_2MS_GZS_SVRCMD_T *)pData;
			SendSvrCmdToGZS(pCmd->nSvrIdx, pCmd->pData, pCmd->nDSize, PNGS_SVRIDX_GMS);
		}
		break;
	case	GMSPACKET_2MS_UNRESERVEPLAYER:
		{
			GMSPACKET_2MS_UNRESERVEPLAYER_T		*pCmd	= (GMSPACKET_2MS_UNRESERVEPLAYER_T *)pData;
			if( m_pSHMData->m_Players.GetPtrByID(pCmd->nClientID) )
			{
				TellCLSToKickPlayer(pCmd->nClientID);
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(834,PLAYER_OUT)"GMSPACKET_2MS_UNRESERVEPLAYER,0x%X,id exist,tell cls to kick", pCmd->nClientID);
			}
			else
			{
				m_pSHMData->m_Players.UnReserveUnitByID(pCmd->nClientID);
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(835,PLAYER_OUT)"GMSPACKET_2MS_UNRESERVEPLAYER,0x%X,%d,%d", pCmd->nClientID, m_pSHMData->m_Players.size(), m_pSHMData->m_Players.getsizeleft());
			}
		}
		break;
	case	GMSPACKET_2MS_CMD2CLS:
		{
			GMSPACKET_2MS_CMD2CLS_T				*pCmd	= (GMSPACKET_2MS_CMD2CLS_T *)pData;
			SendCmdToPlayerCLS(pCmd->nClientID, pCmd->pData, pCmd->nDSize);
		}
		break;
	case	GMSPACKET_2MS_RAWCMD2GZSMS:
		{
			GMSPACKET_2MS_RAWCMD2GZSMS_T		*pCmd	= (GMSPACKET_2MS_RAWCMD2GZSMS_T *)pData;
			// 这个也需要延迟发送的
			QueueRawCmdToGZS(pCmd->nSvrIdx, pCmd->pData, pCmd->nDSize);
		}
		break;
	case	GMSPACKET_2MS_CHANGECLIENTID:
		{
			GMSPACKET_2MS_CHANGECLIENTID_T		*pCmd	= (GMSPACKET_2MS_CHANGECLIENTID_T *)pData;
			// To的ID理论上应该是一个被保留的ID，所以不会能get出来的
			if( m_pSHMData->m_Players.GetPtrByID(pCmd->nToID)==NULL )
			{
				// 正确
				// 先恢复一下保留的序号
				m_pSHMData->m_Players.UnReserveUnitByID(pCmd->nToID);
				// 通知所有GZS和对象所在的CLS对象ID改变
				{
					GMS_GZS_PLAYER_CHANGEID_T	C;
					C.nCmd		= GMS_GZS_PLAYER_CHANGEID;
					C.nFromID	= pCmd->nFromID;
					C.nToID		= pCmd->nToID;
					SendCmdToAllGZS(&C, sizeof(C));
				}
				{
					GMS_CLS_PLAYER_CHANGEID_T	C;
					C.nCmd		= GMS_CLS_PLAYER_CHANGEID;
					C.nFromID	= pCmd->nFromID;
					C.nToID		= pCmd->nToID;
					C.nSvrIdx	= pCmd->nSvrIdx;
					SendCmdToPlayerCLS(pCmd->nFromID, &C, sizeof(C));
				}
				// 生成旧ID对象，并填充内容
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,PLAYER_IN)"GMSPACKET_2MS_CHANGECLIENTID,0x%X,%X", pCmd->nFromID, pCmd->nToID);
				PlayerUnit	*pPlayer		= m_pSHMData->m_Players.AllocUnitByID(pCmd->nToID);
				PlayerUnit	*pFromPlayer	= m_pSHMData->m_Players.GetPtrByID(pCmd->nFromID);
				assert(pPlayer);
				assert(pFromPlayer);
				pPlayer->ReadFromOther(pFromPlayer);
				pPlayer->nID				= pCmd->nToID;
				// 删除新对象
				pFromPlayer->clear();
				m_pSHMData->m_Players.FreeUnitByPtr(pFromPlayer);
			}
			else
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(,PLAYER_IN)"GMSPACKET_2MS_CHANGECLIENTID,old exist,0x%X,0x%X", pCmd->nToID, pCmd->nFromID);
				// 有问题，直接踢掉新用户吧
				TellCLSToKickPlayer(pCmd->nFromID);
			}
		}
		break;
		case	GMSPACKET_2MS_CTRL:
		{
			GMSPACKET_2MS_CTRL_T	*pCtrl	= (GMSPACKET_2MS_CTRL_T *)pData;
			switch( pCtrl->nSubCmd )
			{
			case	GMSPACKET_2MS_CTRL_T::SUBCMD_CLOSE_CONNECTER:
				{
					Connecter	*pCntr	= m_CntrSvr.GetConnecterPtr(pCtrl->nParam);
					if( pCntr )
					{
						closesocket(pCntr->GetSocket());
					}
				}
				break;
			case	GMSPACKET_2MS_CTRL_T::SUBCMD_CLOSEALLCAAFS:
				{
					// 关闭所有的CAAFS
					TellAllCAAFSQuit();
				}
				break;
			case	GMSPACKET_2MS_CTRL_T::SUBCMD_SETCLSMAXPLAYER:
				{
					CLSUnit	*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pCtrl->nParam);
					if( pCLS )
					{
						pCLS->nMaxPlayer	= pCtrl->nParam1;
					}
				}
				break;
			case	GMSPACKET_2MS_CTRL_T::SUBCMD_PLAYER_STAT_ON:
				{
					m_vectrawbuf.resize(sizeof(SVR_CLS_CTRL_T) + sizeof(int));
					SVR_CLS_CTRL_T		&C	= *(SVR_CLS_CTRL_T *)m_vectrawbuf.getbuf();
					C.nCmd				= SVR_CLS_CTRL;
					C.nSubCmd			= SVR_CLS_CTRL_T::SUBCMD_PLAYER_STAT_ON;
					C.nParam			= pCtrl->nParam;
					*(int *)wh_getptrnexttoptr(&C)	= pCtrl->nParam1;
					SendCmdToPlayerCLS(C.nParam, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
				break;
			case	GMSPACKET_2MS_CTRL_T::SUBCMD_PLAYER_STAT_OFF:
				{
					SVR_CLS_CTRL_T		C;
					C.nCmd				= SVR_CLS_CTRL;
					C.nSubCmd			= SVR_CLS_CTRL_T::SUBCMD_PLAYER_STAT_OFF;
					C.nParam			= pCtrl->nParam;
					SendCmdToPlayerCLS(C.nParam, &C, sizeof(C));
				}
				break;
			default:
				break;
			}
		}
		break;
	case	GMSPACKET_2MS_CAAFSCTRL:
		{
			// 直接发指令给CAAFS
			
					GMS_CAAFS_CTRL_T	*pCmd	= (GMS_CAAFS_CTRL_T *)pData;
					MYCNTRSVR::MYCNTR	*pCntr	= m_pSHMData->m_aCAAFSGroup[pCmd->nCAAFSIdx].pCntr;
					if( pCntr )
					{
						pCntr->SendMsg(pCmd, sizeof(*pCmd));
					}
			
		}
		break;
	case	GMSPACKET_2MS_CAAFSCTRL_QUEUE:
		{
			GMS_CAAFS_KEEP_QUEUE_POS_T * pCmd = (GMS_CAAFS_KEEP_QUEUE_POS_T *)pData;
			MYCNTRSVR::MYCNTR	*pCntr	= m_pSHMData->m_aCAAFSGroup[pCmd->nCAAFSIdx].pCntr;
			if( pCntr )
			{
				pCntr->SendMsg(pCmd, sizeof(*pCmd));
			}
		}break;
	default:
		{
			// pCmdIn->pRstAccepter可能为空，所以不能获取其logictype
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(35,PNGS_RT)"GMS_MAINSTRUCTURE::DealCmdIn_One,%d,unknown,%p", nCmd, pRstAccepter);
		}
		break;
	}

	return	0;
}
int		GMS_MAINSTRUCTURE::DealCmdIn_PNGSPACKET_2LOGIC_GMCMD(ILogicBase *pRstAccepter, PNGSPACKET_2LOGIC_GMCMD_T *pCmd)
{
	pngs_cmd_t	*pnCmd	= (pngs_cmd_t *)pCmd->pData;
	if( (*pnCmd) == 0 )
	{
		char			buf[4096];
		*(pngs_cmd_t *)buf	= 0;
		char			*pszRst	= buf+sizeof(pngs_cmd_t);
		size_t	nRstSize= sizeof(buf)-sizeof(pngs_cmd_t);
		DealTstStr((char *)wh_getptrnexttoptr(pnCmd), pszRst, &nRstSize);
		// 把结果发回给来源
		PNGSPACKET_2LOGIC_GMCMD_RST_T	Rst;
		Rst.nCntrID		= pCmd->nCntrID;
		Rst.nExt		= pCmd->nExt;
		Rst.pData		= buf;
		Rst.nDSize		= sizeof(pngs_cmd_t) + nRstSize;
		CMN_LOGIC_CMDIN_AUTO(this, pRstAccepter, PNGSPACKET_2LOGIC_GMCMD_RST, Rst);
	}
	return	0;
}
