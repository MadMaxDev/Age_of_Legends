// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_BeforeDeleteConnecter.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE模块的BeforeDeleteConnecter函数功能实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
//                因为RemoveMYCNTR会导致BeforeDeleteConnecter的调用，所以把它们放在一个文件内
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
	// 因为这个对象一定是和连接相关的，所以直接关闭连接即可
	// 清除关系的操作在BeforeDeleteConnecter
	closesocket( pCntr->GetSocket() );
	return	0;
}
void	GMS_MAINSTRUCTURE::BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo)
{
	// 确定是哪个Connecter
	MYCNTRSVR::MYCNTR	*pCntr	= (MYCNTRSVR::MYCNTR *)pCntrInfo->pCntr->QueryInterface();
	if(!pCntr)
	{
		// 有可能是accepter
		return;
	}
	//
	if( !pCntr->m_pExtPtr )
	{
		// 没有关联相应的Server（应该是没有通过第一步校验）
		// 但是无论如何都应该清除一下
		// 因为不能指望在delete pCntr的时候退出队列，因为在whtimequeue::ID_T的析构中并不会作这个工作
		pCntr->clear();
		return;
	}
	// 把他相关的关系都清理了(如果是CLS还要先把CLS相关的所有Player都断线)
	switch( pCntr->m_nSvrType )
	{
		case	APPTYPE_CAAFS:
		{
			CAAFSGroup	*pCAAFS	= (CAAFSGroup *)pCntr->m_pExtPtr;
			// 清除在Group中的索引，这样重新启动就可以重新进入Group了
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
				// 不是正常退出的，则创建时间事件等待一会，说不定对方是临时网络故障，可能会重连
				// 只是要把Connect指针清空，这样就不会继续发送东西，并且重连的时候也会知道
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
				// 不是正常退出的，则创建时间事件等待一会，说不定对方是临时网络故障，可能会重连
				// 只是要把Connect指针清空，这样就不会继续发送东西，并且重连的时候也会知道
				pGZS->pCntr			= NULL;
				SetTE_GZS_DropWait_TimeOut(pGZS);
				// 最好还要通知一下上层，GZS临时断开状态
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
			// 删除对象
			pGMTOOL->clear();
			m_pSHMData->m_GMTOOLs.FreeUnitByPtr(pGMTOOL);
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(33,PNGS_RT)"GMTOOL CntrID removed,0x%X", pCntr->GetIDInMan());
		}
		break;
		default:
		{
			// 其他类型的应该是连接没有通过验证就删除的，所以不会到这里
			assert(0);
		}
		break;
	}
	// 清空一下Connecter和这里逻辑相关的部分
	pCntr->clear();
}
