// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CNTR_DealMsg_GZS.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ���е�CNTR_DealMsg_GZS��ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-23
// Change LOG   : 2007-01-17 ��������CLS����GZS��ʱ��֪ͨCAAFS���õ�����Ϊ0�������������û��ڶ����еȴ���

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
			// ����GZS��Ϣ
			GZS_GMS_MYINFO_T	*pInfo	= (GZS_GMS_MYINFO_T *)pCmd;
			pGZS->nPort			= pInfo->nPort;
			pGZS->IP			= pInfo->IP;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(562,PNGS_INT_RLS)"GMS_MAINSTRUCTURE get GZS_GMS_MYINFO,%d,%s:%d", pGZS->nSvrIdx, cmn_get_IP(pInfo->IP), pInfo->nPort);
			// ��ѯ����CLS
			for(UAF_CLS_T::iterator it=m_pSHMData->m_CLSs.begin(); it!=m_pSHMData->m_CLSs.end(); ++it)
			{
				CLSUnit	*pCLS	= &(*it);
				// ���������GZS��Ϣ��CLS
				SendGZSInfo0ToCLSAndChangeCLSStatus(pGZS, pCLS);
				// �������CLS�����������û�����Ϣ��GZS
				SendPlayerIDInCLSToGZS(pCLS, pGZS);
			}
			// ��Ϊ��ᵼ������CLS��ʼ����GZS��������Ҫ������CAAFS���Ϳ�����������Ϣ
			for(int i=0;i<GMS_MAX_CAAFS_NUM;i++)
			{
				CAAFSGroup	*pGroup	= m_pSHMData->m_aCAAFSGroup + i;
				if( !pGroup->pCntr )
				{
					// û��CAAFS�Ͳ��ü���ͳ����
					continue;
				}
				// ����֪ͨ��CAAFS
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
					// ������Ӧ��GZS�������˳���־
					pGZS->nProp	|= GZSUnit::PROP_GOODEXIT;
					// ֪ͨ�ϲ�GZS�����˳�״̬�����ܼ��������û��ˣ�����˵�û�����������߽��룩
					GMSPACKET_2GP_GZSOFFLINE_T	GZSOL;
					GZSOL.nSvrIdx	= pGZS->nSvrIdx;
					GZSOL.nMode		= GMSPACKET_2GP_GZSOFFLINE_T::MODE_STOPPING;
					CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSOFFLINE, &GZSOL, sizeof(GZSOL));
				}
				break;
			case	GZS_GMS_CTRL_T::SUBCMD_CLSDOWN:
				{
					// ֪ͨ��CLSȥ��������GZS
					CLSUnit	*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pCmdCtrl->nParam);
					if( pCLS )
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(812,PNGS_RT)"GZS_GMS_CLSDROP,%d,0x%X", pGZS->nSvrIdx, pCmdCtrl->nParam);
						// ���������GZS��Ϣ��CLS
						SendGZSInfo0ToCLSAndChangeCLSStatus(pGZS, pCLS);
						// �������CLS�����������û�����Ϣ��GZS
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
			// ����Ķ�������
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(563,PNGS_RT)"GMS_MAINSTRUCTURE::CNTR_DealMsg_GZS,%d,unknown", *pCmd);
		}
		break;
	}
	return	0;
}
