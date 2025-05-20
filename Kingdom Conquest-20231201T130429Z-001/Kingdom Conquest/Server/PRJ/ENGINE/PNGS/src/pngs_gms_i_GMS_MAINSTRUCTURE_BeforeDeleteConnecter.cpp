// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_BeforeDeleteConnecter.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ���BeforeDeleteConnecter��������ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
//                ��ΪRemoveMYCNTR�ᵼ��BeforeDeleteConnecter�ĵ��ã����԰����Ƿ���һ���ļ���
// CreationDate : 2005-08-24
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::RemoveMYCNTR(int nCntrID)
{
	return	RemoveMYCNTR( (MYCNTRSVR::MYCNTR *)m_CntrSvr.GetConnecterPtr(nCntrID)->QueryInterface() );
}
int		GMS_MAINSTRUCTURE::RemoveMYCNTR(MYCNTRSVR::MYCNTR *pCntr)
{
	if( !pCntr )
	{
		return	-1;
	}
	// ��Ϊ�������һ���Ǻ�������صģ�����ֱ�ӹر����Ӽ���
	// �����ϵ�Ĳ�����BeforeDeleteConnecter
	closesocket( pCntr->GetSocket() );
	return	0;
}
void	GMS_MAINSTRUCTURE::BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo)
{
	// ȷ�����ĸ�Connecter
	MYCNTRSVR::MYCNTR	*pCntr	= (MYCNTRSVR::MYCNTR *)pCntrInfo->pCntr->QueryInterface();
	if(!pCntr)
	{
		// �п�����accepter
		return;
	}
	//
	if( !pCntr->m_pExtPtr )
	{
		// û�й�����Ӧ��Server��Ӧ����û��ͨ����һ��У�飩
		// ����������ζ�Ӧ�����һ��
		// ��Ϊ����ָ����delete pCntr��ʱ���˳����У���Ϊ��whtimequeue::ID_T�������в��������������
		pCntr->clear();
		return;
	}
	// ������صĹ�ϵ��������(�����CLS��Ҫ�Ȱ�CLS��ص�����Player������)
	switch( pCntr->m_nSvrType )
	{
		case	APPTYPE_CAAFS:
		{
			CAAFSGroup	*pCAAFS	= (CAAFSGroup *)pCntr->m_pExtPtr;
			// �����Group�е��������������������Ϳ������½���Group��
			pCAAFS->pCntr	= NULL;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(30,PNGS_RT)"CAAFS removed,0x%X,%d", pCntr->GetIDInMan(), m_pSHMData->GetCAAFSGroupID(pCAAFS));
		}
		break;
		case	APPTYPE_CLS:
		{
			CLSUnit	*pCLS	= (CLSUnit *)pCntr->m_pExtPtr;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(31,PNGS_RT)"CLS CntrID removed,0x%X,0x%X", pCntr->GetIDInMan(), pCLS->nID);
			if( pCLS->nProp & CLSUnit::PROP_GOODEXIT )
			{
				DealCLSDisconnection(pCLS);
			}
			else
			{
				// ���������˳��ģ��򴴽�ʱ���¼��ȴ�һ�ᣬ˵�����Է�����ʱ������ϣ����ܻ�����
				// ֻ��Ҫ��Connectָ����գ������Ͳ���������Ͷ���������������ʱ��Ҳ��֪��
				pCLS->pCntr			= NULL;
				SetTE_CLS_DropWait_TimeOut(pCLS);
			}
		}
		break;
		case	APPTYPE_GZS:
		{
			GZSUnit	*pGZS	= (GZSUnit *)pCntr->m_pExtPtr;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(32,PNGS_RT)"GZS CntrID removed,0x%X,%d", pCntr->GetIDInMan(), pGZS->nSvrIdx);
			unsigned char	nProp	= pGZS->nProp;
			if( (nProp & GZSUnit::PROP_GOODEXIT) )
			{
				DealGZSDisconnection(pGZS);
			}
			else
			{
				// ���������˳��ģ��򴴽�ʱ���¼��ȴ�һ�ᣬ˵�����Է�����ʱ������ϣ����ܻ�����
				// ֻ��Ҫ��Connectָ����գ������Ͳ���������Ͷ���������������ʱ��Ҳ��֪��
				pGZS->pCntr			= NULL;
				SetTE_GZS_DropWait_TimeOut(pGZS);
				// ��û�Ҫ֪ͨһ���ϲ㣬GZS��ʱ�Ͽ�״̬
				GMSPACKET_2GP_GZSTEMPOFF_T	Off;
				Off.nSvrIdx			= pGZS->nSvrIdx;
				Off.bOff			= true;
				CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSTEMPOFF, &Off, sizeof(Off));
			}
		}
		break;
		case	APPTYPE_GMTOOL:
		{
			GMTOOLUnit	*pGMTOOL	= (GMTOOLUnit *)pCntr->m_pExtPtr;
			// ɾ������
			pGMTOOL->clear();
			m_pSHMData->m_GMTOOLs.FreeUnitByPtr(pGMTOOL);
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(33,PNGS_RT)"GMTOOL CntrID removed,0x%X", pCntr->GetIDInMan());
		}
		break;
		default:
		{
			// �������͵�Ӧ��������û��ͨ����֤��ɾ���ģ����Բ��ᵽ����
			assert(0);
		}
		break;
	}
	// ���һ��Connecter�������߼���صĲ���
	pCntr->clear();
}
