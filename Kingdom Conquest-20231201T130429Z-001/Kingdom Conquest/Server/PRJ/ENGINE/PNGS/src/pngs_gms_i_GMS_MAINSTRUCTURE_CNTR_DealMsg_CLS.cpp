// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CNTR_DealMsg_CLS.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE模块中的CNTR_DealMsg_CLS的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-08-23
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::CNTR_DealMsg_CLS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	CLSUnit	*pCLS	= (CLSUnit *)pCntr->m_pExtPtr;
	assert(pCLS);
	// 先判断这个可以提高跳转速度
	if( CLS_SVR_CLIENT_DATA == *pCmd )
	{
		// 内部应该是转发给相应的逻辑处理单元
		InnerRouteClientGameCmd(GMSPACKET_2GP_PLAYERCMD, (CLS_SVR_CLIENT_DATA_T *)pCmd, nSize);
	}
	else if( CLS_SVR_CLIENT_STAT == *pCmd )
	{
		// 给逻辑层的原因是为了和帐号角色等信息关联
		InnerRouteClientGameCmd(GMSPACKET_2GP_PLAYER_STAT, (CLS_SVR_CLIENT_DATA_T *)pCmd, nSize);
	}
	else
	{
		switch( *pCmd )
		{
			case	CLS_GMS_HELLO1:
			{
				CLS_GMS_HELLO1_T	*pHello1	= (CLS_GMS_HELLO1_T *)pCmd;
				pCLS->nPort			= pHello1->nPort;
				pCLS->IP			= pHello1->IP;
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(58,PNGS_INT_RLS)"CLS_GMS_HELLO1,0x%X,%s:%d", pCLS->nID, cmn_get_IP(pCLS->IP), pCLS->nPort);
			}
			break;
			case	CLS_GMS_CONNECTEDTOGZS:
			{
				CLS_GMS_CONNECTEDTOGZS_T	*pConnectedToGZS	= (CLS_GMS_CONNECTEDTOGZS_T *)pCmd;
				// 无论结果如何都需要把记数减减
				pCLS->nGZSNumToConnect	--;
				// 上面的信息只用于来记录LOG
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(59,PNGS_INT_RLS)"CLS_GMS_CONNECTEDTOGZS,0x%X,%d,%d,%d"
					, pCLS->nID, pConnectedToGZS->nSvrIdx, pConnectedToGZS->nRst, pCLS->nGZSNumToConnect
					);
				if( pCLS->nGZSNumToConnect<=0 )
				{
					// 只要该连接的GZS都有了结果，那么CLS就可以正常工作了
					pCLS->nStatus	= CLSUnit::STATUS_WORKING;
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(60,PNGS_INT_RLS)"CLS CAN WORK,0x%X", pCLS->nID);
				}
				// 判断结果，如果连接失败则踢掉相应的GZS
				if( pConnectedToGZS->nRst != CLS_GMS_CONNECTEDTOGZS_T::RST_OK )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(61,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_CLS,%d,CLS connect GZS FAIL,0x%X,%d", pConnectedToGZS->nRst, pCLS->nID, pConnectedToGZS->nSvrIdx);
					GZSUnit	*pGZS	= m_pSHMData->m_GZSs + pConnectedToGZS->nSvrIdx;
					RemoveMYCNTR(pGZS->pCntr);
				}
			}
			break;
			case	CLS_GMS_CLIENTCANGOTOCLS:
			{
				CLS_GMS_CLIENTCANGOTOCLS_T		*pClientCanGo	= (CLS_GMS_CLIENTCANGOTOCLS_T *)pCmd;
				PlayerUnit	*pPlayer	= m_pSHMData->m_Players.GetPtrByID(pClientCanGo->nClientID);
				assert( pPlayer );		// Player一定存在，因为GMS中Player的离开都是以CLS作为发起的，哪怕是GMS要Kick一个Player，也是发指令给CLS。除非CLS宕了，GMS才会主动移除它相关的Player。
				// 收到CLS_GMS_CLIENTCANGOTOCLS就一定说明成功了
				// 玩家状态改变
				pPlayer->nStatus		= PlayerUnit::STATUS_GOINGTOCLS;
				// 玩家对应的CLS应该一定是发出这个指令的CLS
				assert( pCLS->nID == pPlayer->nCLSID );
				// 找到CAAFS对象
				CAAFSGroup	*pCAAFS		= GetCAAFSByGroupID(pCLS->nGroupID);
				if( pCAAFS )
				{
					// 发送给CAAFS
					GMS_CAAFS_CLIENTCANGOTOCLS_T	ClientCanGo;
					ClientCanGo.nCmd		= GMS_CAAFS_CLIENTCANGOTOCLS;
					ClientCanGo.nRst		= GMS_CAAFS_CLIENTCANGOTOCLS_T::RST_OK;
					ClientCanGo.nPort		= pCLS->nPort;
					ClientCanGo.IP			= pCLS->IP;
					ClientCanGo.nClientIDInCAAFS	= pPlayer->nClientIDInCAAFS;
					ClientCanGo.nClientID	= pPlayer->nID;
					pCAAFS->pCntr->SendMsg(&ClientCanGo, sizeof(ClientCanGo));
					// CAAFS收到这个消息后它上面的玩家就可以自然消亡了
				}
				else
				{
					// 如果找不到说明这时候CAAFS可能宕机了，或者这是停机过程，CAAFS先停了
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1015,PLAYER_IN)"CAAFS LOST,0x%X,0x%X,%d", pClientCanGo->nClientID, pPlayer->nCLSID,pCLS->nGroupID);
				}
			}
			break;
			case	CLS_GMS_CLIENTTOCLS_RST:
			{
				CLS_GMS_CLIENTTOCLS_RST_T	*pClientToCLSRst	= (CLS_GMS_CLIENTTOCLS_RST_T *)pCmd;
				if( pClientToCLSRst->nRst == CLS_GMS_CLIENTTOCLS_RST_T::RST_OK )
				{
					// 成功
					PlayerUnit	*pPlayer	= m_pSHMData->m_Players.GetPtrByID(pClientToCLSRst->nClientID);
					assert(pPlayer);
					// 玩家状态改变
					pPlayer->nStatus	= PlayerUnit::STATUS_GAMING;
					// 理论上现在玩家就和CAAFS无关了
					// 然后就可以开始游戏了
					// 还需要通知GamePlay
					GMSPACKET_2GP_PLAYERONLINE_T	CmdPlayerIn;
					CmdPlayerIn.nClientID		= pPlayer->nID;
					CmdPlayerIn.IP				= pPlayer->IP;
					CmdPlayerIn.nNetworkType	= pPlayer->nNetworkType;
					CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_PLAYERONLINE, &CmdPlayerIn, sizeof(CmdPlayerIn));
					// 还要通知所有的GZS的MS这个玩家上线
					GMS_GZS_PLAYER_ONLINE_T		PO;
					PO.nCmd						= GMS_GZS_PLAYER_ONLINE;
					PO.nCLSID					= pCLS->nID;
					PO.anClientID[0]			= pPlayer->nID;
					SendCmdToAllGZS(&PO, sizeof(PO));
				}
				else
				{
					// 失败
					// 删除玩家(如果玩家还是进入CLS的状态的话还需要给CAAFS重新发送CLS_GMS_CLIENTCANGOTOCLS_T)
					RemovePlayerUnit(pClientToCLSRst->nClientID);
				}
			}
			break;
			case	CLS_GMS_CLIENT_DROP:
			{
				CLS_GMS_CLIENT_DROP_T	*pDrop	= (CLS_GMS_CLIENT_DROP_T *)pCmd;
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(62,PLAYER_OUT)"CLIENT_DROP,0x%X,%d", pDrop->nClientID, pDrop->nRemoveReason);
				RemovePlayerUnit(pDrop->nClientID, pDrop->nRemoveReason);
			}
			break;
			case	CLS_GMS_CTRL:
			{
				CLS_GMS_CTRL_T	*pCtrl	= (CLS_GMS_CTRL_T *)pCmd;
				switch( pCtrl->nSubCmd )
				{
				case	CLS_GMS_CTRL_T::SUBCMD_EXIT:
					{
						pCLS->nProp	|= CLSUnit::PROP_GOODEXIT;
						// 关闭和CLS的连接
						closesocket(pCLS->pCntr->GetSocket());
					}
					break;
				default:
					break;
				}
			}
			break;
			default:
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(559,PNGS_RT)"GMS_MAINSTRUCTURE::CNTR_DealMsg_CLS,%d,unknown", *pCmd);
			}
			break;
		}
	}
	return	0;
}
