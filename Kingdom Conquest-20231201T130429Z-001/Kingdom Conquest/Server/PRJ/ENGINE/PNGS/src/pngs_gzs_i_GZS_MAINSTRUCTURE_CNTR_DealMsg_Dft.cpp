// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_CNTR_DealMsg_Dft.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS模块内部的GZS_MAINSTRUCTURE模块中的CNTR_DealMsg_Dft的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GZS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-09-22
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"

using namespace n_pngs;

bool	GZS_MAINSTRUCTURE::CNTR_CanDealMsg() const
{
	return	true;
}
// 对应各个TCP连接的初始处理函数。主要是处理HELLO指令
int		GZS_MAINSTRUCTURE::CNTR_DealMsg_Dft(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	bool	bOK	= false;
	// 根据第一个包判断是那种Server
	switch( *pCmd )
	{
		case	CLS_GZS_HELLO:
		{
			// 注：CLS如果收到失败的HI则会关闭连接，如果它不关闭，则到了超时GZS会把它关闭
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
			// 创建CLS对象
			int				nOldID	= 0;
			CLSUnit			*pCLS	= m_CLSs.AllocUnitByID(pHello->nCLSID, &nOldID);
			if( !pCLS )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(656,PNGS_RT)"GZS_MAINSTRUCTURE::CNTR_DealMsg_Dft,,m_CLSs.AllocUnitByID,0x%X,0x%X", pHello->nCLSID, nOldID);
				HI.nRst				= GZS_CLS_HI_T::RST_ERR_MEMERR;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// 清空
			pCLS->clear();
			// 关联CLS对象
			pCntr->m_nSvrType		= APPTYPE_CLS;
			pCntr->m_pExtPtr		= pCLS;
			pCLS->nID				= pHello->nCLSID;
			pCLS->pCntr				= pCntr;
			pCLS->nStatus			= CLSUnit::STATUS_WORKING;
			// 发送消息给CLS
			pCntr->SendMsg(&HI, sizeof(HI));

			// 设置后面的处理函数
			pCntr->m_p_CNTR_DEALMSG	= &GZS_MAINSTRUCTURE::CNTR_DealMsg_CLS;
			// 表示成功
			bOK	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(657,PNGS_RT)"HELLO from CLS,0x%X,0x%X", pCntr->GetIDInMan(), pCLS->nID);
		}
		break;
		default:
			// 本阶段（刚刚开始的时候只能收到Hello指令）不能处理的指令，关闭连接
		break;
	}

	if( bOK )
	{
		// 对于成功的统一处理
		// 清空原来的Hello超时事件
		pCntr->teid.quit();
	}

	return	0;
}
