// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_CNTR_DealMsg_CLS.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTUREģ���е�CNTR_DealMsg_CLS��ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Zone Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-09-22
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gzs_logic.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::CNTR_DealMsg_CLS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
//	CLSUnit	*pCLS	= (CLSUnit *)pCntr->m_pExtPtr;
//	assert(pCLS);
	// ���ж�������������ת�ٶ�
	if( CLS_SVR_CLIENT_DATA == *pCmd )
	{
		// �ڲ�Ӧ����ת������Ӧ���߼�����Ԫ
		InnerRouteClientGameCmd((CLS_SVR_CLIENT_DATA_T *)pCmd, nSize);
	}
	else
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(654,PNGS_RT)"GZS_MAINSTRUCTURE::CNTR_DealMsg_CLS,%d,unknown", *pCmd);
	}
	return	0;
}
