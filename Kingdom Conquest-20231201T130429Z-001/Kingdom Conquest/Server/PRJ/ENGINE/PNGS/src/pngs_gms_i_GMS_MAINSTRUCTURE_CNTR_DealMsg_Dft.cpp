// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CNTR_DealMsg_Dft.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ���е�CNTR_DealMsg_Dft��ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-23
// Change LOG   : 2006-08-23 �������߳��ϵ���idx��GZS�ĳ�ʱ

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

bool	GMS_MAINSTRUCTURE::CNTR_CanDealMsg() const
{
	return	true;
}
// ��Ӧ����TCP���ӵĳ�ʼ����������Ҫ�Ǵ���HELLOָ��
int		GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	bool	bOK	= false;
	// ���ݵ�һ�����ж�������Server
	switch( *pCmd )
	{
		case	CAAFS_GMS_HELLO:
		{
			CAAFS_GMS_HELLO_T	*pHello	= (CAAFS_GMS_HELLO_T *)pCmd;
			GMS_CAAFS_HI_T		HI;
			HI.nCmd				= GMS_CAAFS_HI;
			HI.nRst				= GMS_CAAFS_HI_T::RST_OK;
			HI.nGMSVer			= GMS_VER;
			if( CAAFS_VER != pHello->nCAAFSVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(564,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CAAFS_GMS_HELLO,,bad ver,%d,%d,%d", pHello->nGroupID, pHello->nCAAFSVer, CAAFS_VER);
				// ����ʧ��HI
				HI.nRst			= GMS_CAAFS_HI_T::RST_ERR_BAD_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// ע��CAAFS����յ�ʧ�ܵ�HI���ر����ӣ���������رգ����˳�ʱGMS������ر�
			// �ж�Group����Ч��
			if( pHello->nGroupID<0 || pHello->nGroupID>=GMS_MAX_CAAFS_NUM )
			{
				// Group��Ҫ������Χ
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(565,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CAAFS_GMS_HELLO,,nGroupID out of range,%d,%d", pHello->nGroupID, GMS_MAX_CAAFS_NUM);
				// ����ʧ��HI
				HI.nRst			= GMS_CAAFS_HI_T::RST_ERR_BAD_GROUPID;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			CAAFSGroup	*pCAAFS	= &m_pSHMData->m_aCAAFSGroup[pHello->nGroupID];
			if( pCAAFS->pCntr!=NULL )
			{
				// �Ѿ��������Group��CAAFS�ˣ�����������
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(566,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CAAFS_GMS_HELLO,,ID DUP,%d,%s", pHello->nGroupID, cmn_get_ipnportstr_by_saaddr(&pCntr->m_addr));
				// ����ʧ��HI
				HI.nRst			= GMS_CAAFS_HI_T::RST_ERR_BAD_GROUPID;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			else
			{
				// ��������������
				pCAAFS->pCntr			= pCntr;
				// ����CAAFS����
				pCntr->m_pExtPtr		= pCAAFS;
				// ���öԷ�����
				pCntr->m_nSvrType		= APPTYPE_CAAFS;
				// ���ú���Ĵ�����
				pCntr->m_p_CNTR_DEALMSG	= &GMS_MAINSTRUCTURE::CNTR_DealMsg_CAAFS;
				// ��ʾ�ɹ�
				bOK	= true;
				char	szReConnect[64]	= "";
				if( pHello->bReConnect )
				{
					strcpy(szReConnect, ",RECONNECT");
					// ���������ľͲ��÷���HI��
				}
				else
				{
					// ����HI
					pCntr->SendMsg(&HI, sizeof(HI));
				}
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(567,PNGS_INT_RLS)"HELLO from CAAFS,0x%X,%s%s", pCntr->GetIDInMan(), cmn_get_ipnportstr_by_saaddr(&pCntr->m_addr), szReConnect);
			}
		}
		break;
		case	CLS_GMS_HELLO:
		{
			// ע��CLS����յ�ʧ�ܵ�HI���ر����ӣ���������رգ����˳�ʱGMS������ر�
			CLS_GMS_HELLO_T	*pHello	= (CLS_GMS_HELLO_T *)pCmd;
			GMS_CLS_HI_T			HI;
			HI.nCmd					= GMS_CLS_HI;
			HI.nRst					= GMS_CLS_HI_T::RST_OK;
			HI.nGMSVer				= GMS_VER;
			if( CLS_VER != pHello->nCLSVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(568,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CLS_GMS_HELLO,,bad ver,%d,%d,%d", pHello->nGroupID, pHello->nCLSVer, CLS_VER);
				// ����ʧ��HI
				HI.nRst			= GMS_CLS_HI_T::RST_ERR_BAD_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// �ж�Group����Ч��
			if( pHello->nGroupID<0 || pHello->nGroupID>=GMS_MAX_CAAFS_NUM )
			{
				// GroupID������Χ
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(569,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CLS_GMS_HELLO,,nGroupID out of range,%d,%d", pHello->nGroupID,GMS_MAX_CAAFS_NUM);
				// ����ʧ��HI
				HI.nRst			= GMS_CLS_HI_T::RST_ERR_BAD_GROUPID;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			CLSUnit				*pCLS	= NULL;
			char		szReConnect[64]	= "";
			if( pHello->nOldID>0 )
			{
				// ˵����������
				// ����һ�¾ɵ�
				pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pHello->nOldID);
				// Ӧ���ǿ����ҵ���
				if( !pCLS )
				{
					// ������CLS�˳�
					SVR_CLS_CTRL_T		Ctrl;
					Ctrl.nCmd			= SVR_CLS_CTRL;
					Ctrl.nSubCmd		= SVR_CLS_CTRL_T::SUBCMD_EXIT;
					pCntr->SendMsg(&Ctrl, sizeof(Ctrl));
					GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(851,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CLS_GMS_HELLO,,old id not found,0x%X", pHello->nOldID);
					return	0;
				}
				strcpy(szReConnect, ",RECONNECT");
			}
			else
			{
				// ����Ӧ��Group�е�CLS�����Ƿ��Ѿ�����
				if( m_pSHMData->m_aCAAFSGroup[pHello->nGroupID].dlCLS.size() >= m_cfginfo.anMaxCLSNumInCAAFSGroup[pHello->nGroupID] )
				{
					// ���group��CLS�Ѿ�̫����
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(570,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CLS_GMS_HELLO,,too many CLS,%d,%d", pHello->nGroupID, m_pSHMData->m_aCAAFSGroup[pHello->nGroupID].dlCLS.size());
					// ����ʧ��HI
					HI.nRst			= GMS_CLS_HI_T::RST_ERR_TOOMANY_CLS;
					pCntr->SendMsg(&HI, sizeof(HI));
					return	0;
				}
				// ����CLS����
				int		nID			= -1;
				pCLS	= m_pSHMData->m_CLSs.AllocUnit(&nID);
				if( nID<0 )
				{
					assert(0);
					HI.nRst			= GMS_CLS_HI_T::RST_ERR_MEMERR;
					pCntr->SendMsg(&HI, sizeof(HI));
					return	0;
				}
				// ���
				pCLS->clear();
				pCLS->pCntr			= pCntr;	// �����������ȹ���һ�£���ΪSendGZSInfo0ToCLSAndChangeCLSStatusҪ��
				pCLS->nID			= nID;
				pCLS->nGroupID		= pHello->nGroupID;
				pCLS->nNetworkType	= pHello->nNetworkType;
				pCLS->bCanAcceptPlayerWhenCLSConnectingGZS	= pHello->bCanAcceptPlayerWhenCLSConnectingGZS;
				pCLS->nMaxPlayer	= m_cfginfo.anMaxPlayerNumInCLSOfCAAFSGroup[pCLS->nGroupID];
				// ����CAAFS/CLS��Group
				m_pSHMData->m_aCAAFSGroup[pHello->nGroupID].dlCLS.AddToTail(&pCLS->dlnode);
				// �ȼٶ�������ʽ��ʼ������
				pCLS->nStatus		= CLSUnit::STATUS_WORKING;
				// ����HI (������Щ�����ǳɹ�����Ҫ��)
				HI.nCLSID				= nID;
				HI.nGMSMaxPlayer		= m_cfginfo.nMaxPlayer;
				HI.nCLSMaxConnection	= pCLS->nMaxPlayer;
				pCntr->SendMsg(&HI, sizeof(HI));
				// ���͵�ǰ������Hello�ɹ����ɹ�����IP��Ϣ��GZS��Ϣ����CLS
				for(int i=1;i<GZS_MAX_NUM;++i)
				{
					GZSUnit	*pGZS	= &m_pSHMData->m_GZSs[i];
					if( pGZS->pCntr && pGZS->IP )
					{
						// ��������趨״̬Ϊ����GZS״̬������Ҫ�ȴ����е�GZS�����ӵ��˲��ܿ�ʼ����
						SendGZSInfo0ToCLSAndChangeCLSStatus(pGZS, pCLS);
					}
				}
			}
			// ����CLS����
			pCntr->m_nSvrType		= APPTYPE_CLS;
			pCntr->m_pExtPtr		= pCLS;
			pCLS->pCntr				= pCntr;
			// ���ú���Ĵ�����
			pCntr->m_p_CNTR_DEALMSG	= &GMS_MAINSTRUCTURE::CNTR_DealMsg_CLS;
			// ��ʾ�ɹ�
			bOK	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(571,PNGS_INT_RLS)"HELLO from CLS,0x%X,0x%X%s", pCntr->GetIDInMan(), pCLS->nID, szReConnect);
			if( szReConnect[0] )
			{
				// ��CLS�����Լ��Ƿ������û�����
				TellCLSToCheckPlayerOffline(pCLS);
			}
		}
		break;
		case	GZS_GMS_HELLO:
		{
			GZS_GMS_HELLO_T	*pHello	= (GZS_GMS_HELLO_T *)pCmd;
			GMS_GZS_HI_T	HI;
			memset(&HI, 0, sizeof(HI));
			HI.nCmd			= GMS_GZS_HI;
			if( GZS_VER != pHello->nGZSVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(572,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft GZS_GMS_HELLO,bad ver,%d,%d,%d", pHello->nSvrIdx, pHello->nGZSVer, GZS_VER);
				// ����ʧ��HI
				HI.nRst			= GMS_CAAFS_HI_T::RST_ERR_BAD_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// SvrIdx����Ϊ0��
			if( pHello->nSvrIdx<=0 || pHello->nSvrIdx>GZS_MAX_NUM )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(573,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft GZS_GMS_HELLO,,nSvrIdx is out of range,%d,%d", pHello->nSvrIdx,GZS_MAX_NUM);
				// ɾ��������
				RemoveMYCNTR(pCntr);
				return	0;
			}
			// ԭ����������ͬSvrIdx�Ķ���
			GZSUnit				*pOldGZS	= m_pSHMData->m_GZSs + pHello->nSvrIdx;
			if( pOldGZS->pCntr )
			{
				// Ϊ�˲���㣬���߶�Ҫ���
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(574,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft,,GZS nSvrIdx DUP,%d,%s:%d", pHello->nSvrIdx, cmn_get_IP(pOldGZS->IP), pOldGZS->nPort);
				// ���Ƿ���Ҫǿ���߳��ɵ�GZS (���һ��ֻ���������߳����������е�GZS�ˣ���Ϊ��TCP���Բ������ԭ���ĵ�����ʱ��������GZS�����ڵ�����)
				if( pHello->nForceKickOldGZS )
				{
					switch( pHello->nForceKickOldGZS )
					{
					case	1:
						{
							// ����һ���
							GMS_GZS_CTRL_T		Ctrl;
							Ctrl.nCmd			= GMS_GZS_CTRL;
							Ctrl.nSubCmd		= GMS_GZS_CTRL_T::SUBCMD_EXIT;
							pOldGZS->pCntr->SendMsg(&Ctrl, sizeof(Ctrl));
							GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(575,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft,Old GZS will be told to quit by nSvrIdx DUP,%d,%s:%d", pHello->nSvrIdx, cmn_get_IP(pOldGZS->IP), pOldGZS->nPort);
						}
						break;
					case	2:
						{
							// ǿ��ɾ����Ӧ��Connecter
							RemoveMYCNTR(pOldGZS->pCntr);
						}
						break;
					}
				}
				// ���ͽ����������֪ͨ������ͻ��Ȼ��Է�Ӧ���Թ�
				HI.nRst				= GMS_GZS_HI_T::RST_IDXDUP;
				pCntr->SendMsg(&HI, sizeof(HI));
				// ����������ʱ��ʱ�䵽����ʱ����Ƴ�����
				return	0;
			}
			char	szReConnect[64]	= "";
			GZSUnit			*pGZS	= pOldGZS;
			GMSPACKET_2GP_GZSONLINE_T	GZSOL;
			GZSOL.nSvrIdx		= pHello->nSvrIdx;
			if( pHello->bReConnect )
			{
				// �Ƿ���������
				strcpy(szReConnect, ",RECONNECT");
				// ���������ľͲ��÷���HI��
				GZSOL.bReconnect	= true;
				// ��Ҫ�˳�һ��ԭ����ʱ���¼�
				pGZS->teid.quit();
			}
			else
			{
				// ���ڿ��������
				pGZS->clear();
				pGZS->nSvrIdx		= pHello->nSvrIdx;
				// ����HI
				HI.nRst				= GMS_GZS_HI_T::RST_OK;
				HI.nGMSMaxPlayer	= m_cfginfo.nMaxPlayer;
				HI.nGMSMaxCLS		= m_cfginfo.nMaxCLS;
				HI.nGMSVer			= GMS_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				GZSOL.bReconnect	= false;
			}
			// ����GZS����
			pGZS->pCntr				= pCntr;
			//
			pCntr->m_nSvrType		= APPTYPE_GZS;
			pCntr->m_pExtPtr		= pGZS;
			// ���ú���Ĵ�����
			pCntr->m_p_CNTR_DEALMSG	= &GMS_MAINSTRUCTURE::CNTR_DealMsg_GZS;
			// ��ʾ�ɹ�
			bOK	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(576,PNGS_INT_RLS)"HELLO from GZS,%d,0x%X,%s,%s", pGZS->nSvrIdx, pCntr->GetIDInMan(), szReConnect, cmn_get_IP(pGZS->IP));
			// ֪ͨGP��GZS���ߣ�ע�⣬���������pGZS->pCntr����֮����ã�
			CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSONLINE, &GZSOL, sizeof(GZSOL));
			if( szReConnect[0] )
			{
				// ��GZS�����Լ��Ƿ������û�����
				TellGZSToCheckPlayerOffline(pGZS);
			}
		}
		break;
		case	GMTOOL_SVR_HELLO:
		{
			GMTOOL_SVR_HELLO_T	*pHello	= (GMTOOL_SVR_HELLO_T *)pCmd;
			if( GMTOOL_VER != pHello->nGMTOOLVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(755,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft GMTOOL_SVR_HELLO,bad ver,%d,%d", pHello->nGMTOOLVer, GMTOOL_VER);
				// ɾ��������
				RemoveMYCNTR(pCntr);
				return	0;
			}
			// ��������
			int			nID			= -1;
			GMTOOLUnit	*pGMTOOL	= m_pSHMData->m_GMTOOLs.AllocUnit(&nID);
			if( nID<0 )
			{
				assert(0);
				// ɾ��������
				RemoveMYCNTR(pCntr);
				return	0;
			}
			pGMTOOL->clear();
			// ����GMTOOL����(��ΪGMTOOLֻ������/Ӧ��ʽ�����Բ���Ҫ���߼������Ӧ)
			pCntr->m_nSvrType		= APPTYPE_GMTOOL;
			pCntr->m_pExtPtr		= pGMTOOL;
			pGMTOOL->nID			= nID;
			pGMTOOL->pCntr			= pCntr;
			// ����HI
			SVR_GMTOOL_HI_T			HI;
			HI.nCmd					= SVR_GMTOOL_HI;
			HI.nSvrType				= APPTYPE_GMS;
			HI.nVer					= GMS_VER;
			pCntr->SendMsg(&HI, sizeof(HI));
			// ���ú���Ĵ�����
			pCntr->m_p_CNTR_DEALMSG	= &GMS_MAINSTRUCTURE::CNTR_DealMsg_GMTOOL;
			// ��ʾ�ɹ�
			bOK	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(756,PNGS_INT_RLS)"HELLO from GMTOOL,0x%X,0x%X", pCntr->GetIDInMan(), nID);
		}
		break;
		default:
			// ���׶Σ��ոտ�ʼ��ʱ��ֻ���յ�Helloָ����ܴ����ָ��ر�����
		break;
	}

	if( bOK )
	{
		// ���ڳɹ���ͳһ����
		// ���ԭ����Hello��ʱ�¼�
		pCntr->teid.quit();
	}

	return	0;
}
