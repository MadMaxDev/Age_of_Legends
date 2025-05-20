// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_CNTR_DealMsg_Dft.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTUREģ���е�CNTR_DealMsg_Dft��ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-09-22
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"

using namespace n_pngs;

bool	GZS_MAINSTRUCTURE::CNTR_CanDealMsg() const
{
	return	true;
}
// ��Ӧ����TCP���ӵĳ�ʼ����������Ҫ�Ǵ���HELLOָ��
int		GZS_MAINSTRUCTURE::CNTR_DealMsg_Dft(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	bool	bOK	= false;
	// ���ݵ�һ�����ж�������Server
	switch( *pCmd )
	{
		case	CLS_GZS_HELLO:
		{
			// ע��CLS����յ�ʧ�ܵ�HI���ر����ӣ���������رգ����˳�ʱGZS������ر�
			CLS_GZS_HELLO_T	*pHello	= (CLS_GZS_HELLO_T *)pCmd;
			GZS_CLS_HI_T			HI;
			HI.nCmd					= GZS_CLS_HI;
			HI.nRst					= GZS_CLS_HI_T::RST_OK;
			HI.nGZSVer				= GZS_VER;
			if( CLS_VER != pHello->nCLSVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(655,PNGS_RT)"GZS_MAINSTRUCTURE::CNTR_DealMsg_Dft,,bad CLS ver,0x%X,%d,%d", pHello->nCLSID, pHello->nCLSVer, CLS_VER);
				HI.nRst				= GZS_CLS_HI_T::RST_ERR_BAD_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// ����CLS����
			int				nOldID	= 0;
			CLSUnit			*pCLS	= m_CLSs.AllocUnitByID(pHello->nCLSID, &nOldID);
			if( !pCLS )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(656,PNGS_RT)"GZS_MAINSTRUCTURE::CNTR_DealMsg_Dft,,m_CLSs.AllocUnitByID,0x%X,0x%X", pHello->nCLSID, nOldID);
				HI.nRst				= GZS_CLS_HI_T::RST_ERR_MEMERR;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// ���
			pCLS->clear();
			// ����CLS����
			pCntr->m_nSvrType		= APPTYPE_CLS;
			pCntr->m_pExtPtr		= pCLS;
			pCLS->nID				= pHello->nCLSID;
			pCLS->pCntr				= pCntr;
			pCLS->nStatus			= CLSUnit::STATUS_WORKING;
			// ������Ϣ��CLS
			pCntr->SendMsg(&HI, sizeof(HI));

			// ���ú���Ĵ�����
			pCntr->m_p_CNTR_DEALMSG	= &GZS_MAINSTRUCTURE::CNTR_DealMsg_CLS;
			// ��ʾ�ɹ�
			bOK	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(657,PNGS_RT)"HELLO from CLS,0x%X,0x%X", pCntr->GetIDInMan(), pCLS->nID);
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
