// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CNTR_DealMsg_GZS.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE模块中的CNTR_DealMsg_GZS的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-08-23
// Change LOG   : 2007-01-17 在让所有CLS连接GZS的时候，通知CAAFS可用的人数为0。这样可以让用户在队列中等待。

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::CNTR_DealMsg_GZS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	GZSUnit		*pGZS	= (GZSUnit *)(pCntr->m_pExtPtr);
	switch( *pCmd )
	{
		case	GZS_GMS_MYINFO:
		{
			// 设置GZS信息
			GZS_GMS_MYINFO_T	*pInfo	= (GZS_GMS_MYINFO_T *)pCmd;
			pGZS->nPort			= pInfo->nPort;
			pGZS->IP			= pInfo->IP;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(562,PNGS_INT_RLS)"GMS_MAINSTRUCTURE get GZS_GMS_MYINFO,%d,%s:%d", pGZS->nSvrIdx, cmn_get_IP(pInfo->IP), pInfo->nPort);
			// 轮询所有CLS
			for(UAF_CLS_T::iterator it=m_pSHMData->m_CLSs.begin(); it!=m_pSHMData->m_CLSs.end(); ++it)
			{
				CLSUnit	*pCLS	= &(*it);
				// 发送这个的GZS信息给CLS
				SendGZSInfo0ToCLSAndChangeCLSStatus(pGZS, pCLS);
				// 发送这个CLS的所有在线用户的信息给GZS
				SendPlayerIDInCLSToGZS(pCLS, pGZS);
			}
			// 因为这会导致所有CLS开始链接GZS，所以需要重新向CAAFS发送可用人数的信息
			for(int i=0;i<GMS_MAX_CAAFS_NUM;i++)
			{
				CAAFSGroup	*pGroup	= m_pSHMData->m_aCAAFSGroup + i;
				if( !pGroup->pCntr )
				{
					// 没有CAAFS就不用继续统计了
					continue;
				}
				// 发送通知给CAAFS
				GMS_CAAFS_GMSINFO_T		GMSInfo;
				GMSInfo.nCmd			= GMS_CAAFS_GMSINFO;
				GMSInfo.nGMSAvailPlayer	= 0;
				pGroup->pCntr->SendMsg(&GMSInfo, sizeof(GMSInfo));
			}
		}
		break;
		case	GZS_GMS_GAMECMD:
		{
			InnerRouteGZSGameCmd(pCntr, (GZS_GMS_GAMECMD_T *)pCmd, nSize);
		}
		break;
		case	GZS_GMS_KICKPALYERBYID:
		{
			GZS_GMS_KICKPALYERBYID_T	*pCmdKick	= (GZS_GMS_KICKPALYERBYID_T *)pCmd;
			PlayerUnit	*pPlayer	= m_pSHMData->m_Players.GetPtrByID(pCmdKick->nClientID);
			if( pPlayer )
			{
				if( pPlayer->nSvrIdx == pGZS->nSvrIdx )
				{
					TellCLSToKickPlayer(pCmdKick->nClientID);
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(811,PLAYER_OUT)"GZS_GMS_KICKPALYERBYID,%d,0x%X", pGZS->nSvrIdx, pCmdKick->nClientID);
				}
				else
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(813,PLAYER_OUT)"GZS_GMS_KICKPALYERBYID STRANGE,%d,0x%X,%d", pGZS->nSvrIdx, pCmdKick->nClientID, pPlayer->nSvrIdx);
				}
			}
		}
		break;
		case	GZS_GMS_CTRL:
		{
			GZS_GMS_CTRL_T	*pCmdCtrl	= (GZS_GMS_CTRL_T *)pCmd;
			switch( pCmdCtrl->nSubCmd )
			{
			case	GZS_GMS_CTRL_T::SUBCMD_EXIT:
				{
					// 设置相应的GZS的正常退出标志
					pGZS->nProp	|= GZSUnit::PROP_GOODEXIT;
					// 通知上层GZS处于退出状态，不能继续接收用户了（就是说用户不能切入或者进入）
					GMSPACKET_2GP_GZSOFFLINE_T	GZSOL;
					GZSOL.nSvrIdx	= pGZS->nSvrIdx;
					GZSOL.nMode		= GMSPACKET_2GP_GZSOFFLINE_T::MODE_STOPPING;
					CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSOFFLINE, &GZSOL, sizeof(GZSOL));
				}
				break;
			case	GZS_GMS_CTRL_T::SUBCMD_CLSDOWN:
				{
					// 通知该CLS去重新连接GZS
					CLSUnit	*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pCmdCtrl->nParam);
					if( pCLS )
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(812,PNGS_RT)"GZS_GMS_CLSDROP,%d,0x%X", pGZS->nSvrIdx, pCmdCtrl->nParam);
						// 发送这个的GZS信息给CLS
						SendGZSInfo0ToCLSAndChangeCLSStatus(pGZS, pCLS);
						// 发送这个CLS的所有在线用户的信息给GZS
						SendPlayerIDInCLSToGZS(pCLS, pGZS);
					}
				}
				break;
			default:
				{
				}
				break;
			}
		}
		break;
		default:
		{
			// 其余的都不处理
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(563,PNGS_RT)"GMS_MAINSTRUCTURE::CNTR_DealMsg_GZS,%d,unknown", *pCmd);
		}
		break;
	}
	return	0;
}
