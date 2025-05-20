// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_CNTR_DealMsg_CLS.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS模块内部的GZS_MAINSTRUCTURE模块中的CNTR_DealMsg_CLS的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GZS是Game Zone Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-09-22
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gzs_logic.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::CNTR_DealMsg_CLS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
//	CLSUnit	*pCLS	= (CLSUnit *)pCntr->m_pExtPtr;
//	assert(pCLS);
	// 先判断这个可以提高跳转速度
	if( CLS_SVR_CLIENT_DATA == *pCmd )
	{
		// 内部应该是转发给相应的逻辑处理单元
		InnerRouteClientGameCmd((CLS_SVR_CLIENT_DATA_T *)pCmd, nSize);
	}
	else
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(654,PNGS_RT)"GZS_MAINSTRUCTURE::CNTR_DealMsg_CLS,%d,unknown", *pCmd);
	}
	return	0;
}
