// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_BeforeDeleteConnecter.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS模块内部的GZS_MAINSTRUCTURE模块的BeforeDeleteConnecter函数功能实现
//                PNGS是Pixel Network Game Structure的缩写
//                GZS是Game Zone Server的缩写，是逻辑服务器中的总控服务器
//                因为RemoveMYCNTR会导致BeforeDeleteConnecter的调用，所以把它们放在一个文件内
// CreationDate : 2005-09-20
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::RemoveMYCNTR(int nCntrID)
{
	return	RemoveMYCNTR( (MYCNTRSVR::MYCNTR *)m_CntrSvr.GetConnecterPtr(nCntrID)->QueryInterface() );
}
int		GZS_MAINSTRUCTURE::RemoveMYCNTR(MYCNTRSVR::MYCNTR *pCntr)
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
void	GZS_MAINSTRUCTURE::BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo)
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
		case	APPTYPE_CLS:
		{
			CLSUnit	*pCLS	= (CLSUnit *)pCntr->m_pExtPtr;
			// 清除所有和CLS相关的玩家
			int	nCount	= 0;
			whunitallocatorFixed<PlayerUnit>::iterator	it = m_Players.begin();
			while( it!=m_Players.end() )
			{
				PlayerUnit	*pPlayer	= &(*it);
				++	it;
				if( pPlayer->nCLSID == pCLS->nID )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(809,PNGS_RT)"Player removed on CLS,0x%X,0x%X,0x%X,%d", pCntr->GetIDInMan(), pCLS->nID, pPlayer->nID, m_Players.size());
					// 是和这个CLS相关的
					RemovePlayerUnit(pPlayer);
					nCount	++;
				}
			}

			// 通知GMS有CLS和自己断线了
			if( m_nMood == MOOD_WORKING )
			{
				// 只有在工作状态才需要通知GMS，停止过程就不用了
				GZS_GMS_CTRL_T	Cmd;
				Cmd.nCmd		= GZS_GMS_CTRL;
				Cmd.nSubCmd		= GZS_GMS_CTRL_T::SUBCMD_CLSDOWN;
				Cmd.nParam		= pCLS->nID;
				m_msgerGMS.SendMsg(&Cmd, sizeof(Cmd));
			}

			// 删除对象
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(682,PNGS_RT)"CLS removed,0x%X,0x%X,%d", pCntr->GetIDInMan(), pCLS->nID, nCount);
			pCLS->clear();
			m_CLSs.FreeUnitByPtr(pCLS);
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
