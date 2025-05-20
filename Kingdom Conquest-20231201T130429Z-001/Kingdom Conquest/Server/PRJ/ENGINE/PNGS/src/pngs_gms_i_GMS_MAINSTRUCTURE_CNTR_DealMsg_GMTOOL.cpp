// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CNTR_DealMsg_GMTOOL.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ���е�CNTR_DealMsg_GMTOOL��ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-23
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::CNTR_DealMsg_GMTOOL(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	if( (*pCmd)!=GMTOOL_SVR_CMD )
	{
		// ����Ӧ�����Ӷ���
		closesocket(pCntr->GetSocket());
		return	-1;
	}
	GMTOOL_SVR_CMD_T	*pGMCmd	= (GMTOOL_SVR_CMD_T *)pCmd;
	// ��ָ���GamePlay
	PNGSPACKET_2LOGIC_GMCMD_T	GMCmd;
	GMCmd.nCntrID		= pCntr->GetIDInMan();
	GMCmd.nExt			= pGMCmd->nExt;
	GMCmd.nDSize		= nSize - wh_offsetof(GMTOOL_SVR_CMD_T,data);
	GMCmd.pData			= pGMCmd->data;
	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, PNGSPACKET_2LOGIC_GMCMD, GMCmd);
	return	0;
}
