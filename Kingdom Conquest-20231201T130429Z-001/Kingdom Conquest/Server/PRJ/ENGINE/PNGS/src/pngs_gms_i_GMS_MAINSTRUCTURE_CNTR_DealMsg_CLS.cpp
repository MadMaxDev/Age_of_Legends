// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CNTR_DealMsg_CLS.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ���е�CNTR_DealMsg_CLS��ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-23
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::CNTR_DealMsg_CLS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	CLSUnit	*pCLS	= (CLSUnit *)pCntr->m_pExtPtr;
	assert(pCLS);
	// ���ж�������������ת�ٶ�
	if( CLS_SVR_CLIENT_DATA == *pCmd )
	{
		// �ڲ�Ӧ����ת������Ӧ���߼�����Ԫ
		InnerRouteClientGameCmd(GMSPACKET_2GP_PLAYERCMD, (CLS_SVR_CLIENT_DATA_T *)pCmd, nSize);
	}
	else if( CLS_SVR_CLIENT_STAT == *pCmd )
	{
		// ���߼����ԭ����Ϊ�˺��ʺŽ�ɫ����Ϣ����
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
				// ���۽����ζ���Ҫ�Ѽ�������
				pCLS->nGZSNumToConnect	--;
				// �������Ϣֻ��������¼LOG
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(59,PNGS_INT_RLS)"CLS_GMS_CONNECTEDTOGZS,0x%X,%d,%d,%d"
					, pCLS->nID, pConnectedToGZS->nSvrIdx, pConnectedToGZS->nRst, pCLS->nGZSNumToConnect
					);
				if( pCLS->nGZSNumToConnect<=0 )
				{
					// ֻҪ�����ӵ�GZS�����˽������ôCLS�Ϳ�������������
					pCLS->nStatus	= CLSUnit::STATUS_WORKING;
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(60,PNGS_INT_RLS)"CLS CAN WORK,0x%X", pCLS->nID);
				}
				// �жϽ�����������ʧ�����ߵ���Ӧ��GZS
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
				assert( pPlayer );		// Playerһ�����ڣ���ΪGMS��Player���뿪������CLS��Ϊ����ģ�������GMSҪKickһ��Player��Ҳ�Ƿ�ָ���CLS������CLS��ˣ�GMS�Ż������Ƴ�����ص�Player��
				// �յ�CLS_GMS_CLIENTCANGOTOCLS��һ��˵���ɹ���
				// ���״̬�ı�
				pPlayer->nStatus		= PlayerUnit::STATUS_GOINGTOCLS;
				// ��Ҷ�Ӧ��CLSӦ��һ���Ƿ������ָ���CLS
				assert( pCLS->nID == pPlayer->nCLSID );
				// �ҵ�CAAFS����
				CAAFSGroup	*pCAAFS		= GetCAAFSByGroupID(pCLS->nGroupID);
				if( pCAAFS )
				{
					// ���͸�CAAFS
					GMS_CAAFS_CLIENTCANGOTOCLS_T	ClientCanGo;
					ClientCanGo.nCmd		= GMS_CAAFS_CLIENTCANGOTOCLS;
					ClientCanGo.nRst		= GMS_CAAFS_CLIENTCANGOTOCLS_T::RST_OK;
					ClientCanGo.nPort		= pCLS->nPort;
					ClientCanGo.IP			= pCLS->IP;
					ClientCanGo.nClientIDInCAAFS	= pPlayer->nClientIDInCAAFS;
					ClientCanGo.nClientID	= pPlayer->nID;
					pCAAFS->pCntr->SendMsg(&ClientCanGo, sizeof(ClientCanGo));
					// CAAFS�յ������Ϣ�����������ҾͿ�����Ȼ������
				}
				else
				{
					// ����Ҳ���˵����ʱ��CAAFS����崻��ˣ���������ͣ�����̣�CAAFS��ͣ��
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1015,PLAYER_IN)"CAAFS LOST,0x%X,0x%X,%d", pClientCanGo->nClientID, pPlayer->nCLSID,pCLS->nGroupID);
				}
			}
			break;
			case	CLS_GMS_CLIENTTOCLS_RST:
			{
				CLS_GMS_CLIENTTOCLS_RST_T	*pClientToCLSRst	= (CLS_GMS_CLIENTTOCLS_RST_T *)pCmd;
				if( pClientToCLSRst->nRst == CLS_GMS_CLIENTTOCLS_RST_T::RST_OK )
				{
					// �ɹ�
					PlayerUnit	*pPlayer	= m_pSHMData->m_Players.GetPtrByID(pClientToCLSRst->nClientID);
					assert(pPlayer);
					// ���״̬�ı�
					pPlayer->nStatus	= PlayerUnit::STATUS_GAMING;
					// ������������Ҿͺ�CAAFS�޹���
					// Ȼ��Ϳ��Կ�ʼ��Ϸ��
					// ����Ҫ֪ͨGamePlay
					GMSPACKET_2GP_PLAYERONLINE_T	CmdPlayerIn;
					CmdPlayerIn.nClientID		= pPlayer->nID;
					CmdPlayerIn.IP				= pPlayer->IP;
					CmdPlayerIn.nNetworkType	= pPlayer->nNetworkType;
					CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_PLAYERONLINE, &CmdPlayerIn, sizeof(CmdPlayerIn));
					// ��Ҫ֪ͨ���е�GZS��MS����������
					GMS_GZS_PLAYER_ONLINE_T		PO;
					PO.nCmd						= GMS_GZS_PLAYER_ONLINE;
					PO.nCLSID					= pCLS->nID;
					PO.anClientID[0]			= pPlayer->nID;
					SendCmdToAllGZS(&PO, sizeof(PO));
				}
				else
				{
					// ʧ��
					// ɾ�����(�����һ��ǽ���CLS��״̬�Ļ�����Ҫ��CAAFS���·���CLS_GMS_CLIENTCANGOTOCLS_T)
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
						// �رպ�CLS������
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
