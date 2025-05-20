// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_RemovePlayerUnit.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE模块的RemovePlayerUnit函数功能实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-08-24
// Change LOG   : 2007-09-19 修正了用户在连接CLS过程中被移除，其idx还是被reserve的问题

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::RemovePlayerUnit(int nID, unsigned char nRemoveReason)
{
	return	RemovePlayerUnit( m_pSHMData->m_Players.GetPtrByID(nID), nRemoveReason );
}
int		GMS_MAINSTRUCTURE::RemovePlayerUnit(PlayerUnit *pPlayer, unsigned char nRemoveReason)
{
	if( !pPlayer )
	{
		return	-1;
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(833,PLAYER_OUT)"GMS_MAINSTRUCTURE::RemovePlayerUnit,0x%X,0x%X,%d,%d,%d", pPlayer->nID, pPlayer->nClientIDInCAAFS, pPlayer->nStatus, nRemoveReason, m_pSHMData->m_Players.size());

	// 根据玩家状态先发送一些消息给其他服务器
	// 发送通知所有的GZS这个玩家下线了
	GMS_GZS_PLAYER_OFFLINE_T	PO;
	PO.nCmd						= GMS_GZS_PLAYER_OFFLINE;
	PO.anClientID[0]			= pPlayer->nID;
	SendCmdToAllGZS(&PO, sizeof(PO));

	// 判断用户是否是通知GP在线了
	bool	bGPOnline	= true;
	switch( pPlayer->nStatus )
	{
		case	PlayerUnit::STATUS_WANTCLS:
		case	PlayerUnit::STATUS_GOINGTOCLS:
		{
			bGPOnline	= false;
		}
		break;
		default:
		break;
	}

	// 通过Player找到CLS再找到CAAFS（这时候CLS不可能删除的）
	CLSUnit		*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pPlayer->nCLSID);
	assert(pCLS);
	CAAFSGroup	*pCAAFS	= GetCAAFSByGroupID(pCLS->nGroupID);
	// 这时候倒是可能CAAFS关闭了
	if(pCAAFS)
	{
		// 准备数据
		GMS_CAAFS_CLIENTCANGOTOCLS_T	ClientCanGo;
		ClientCanGo.nCmd				= GMS_CAAFS_CLIENTCANGOTOCLS;
		ClientCanGo.nClientIDInCAAFS	= pPlayer->nClientIDInCAAFS;
		if( !bGPOnline )
		{
			ClientCanGo.nRst		= GMS_CAAFS_CLIENTCANGOTOCLS_T::RST_ERR_OTHERKNOWN;
			pCAAFS->pCntr->SendMsg(&ClientCanGo, sizeof(ClientCanGo));
		}
		else
		{
			// 其他的状态就不用发送什么东西了
		}
	}

	// 还需要通知其他的逻辑模块(比如GamePlay)
	GMSPACKET_2GP_PLAYEROFFLINE_T	CmdPlayerOut;
	CmdPlayerOut.nClientID	= pPlayer->nID;
	CmdPlayerOut.nReason	= nRemoveReason;

	// 移除玩家
	// LOG得写在clear前面，否则数据就被清了，写不出来了
	pPlayer->clear();
	m_pSHMData->m_Players.FreeUnitByPtr(pPlayer);
	// 但是先保留原来的序号（等到上层逻辑真正删除了相应的玩家才能把序号放回来）
	if( bGPOnline )
	{
		m_pSHMData->m_Players.ReserveUnitByPtr(pPlayer);
	}

	// 最后发送指令（因为对于单线程程序来说可能会立即产生后果，里面会告诉MS说可以把保留的PlayerUnit释放为Free的了）
	CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_PLAYEROFFLINE, &CmdPlayerOut, sizeof(CmdPlayerOut));

	return	0;
}
