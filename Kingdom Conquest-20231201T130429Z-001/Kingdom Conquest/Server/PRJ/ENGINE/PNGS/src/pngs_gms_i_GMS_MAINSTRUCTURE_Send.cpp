// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_Send.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE模块的所有发送操作函数功能实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
//                因为RemoveMYCNTR会导致BeforeDeleteConnecter的调用，所以把它们放在一个文件内
// CreationDate : 2005-08-24
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::SendGZSInfo0ToCLSAndChangeCLSStatus(GZSUnit *pGZS, CLSUnit *pCLS)
{
	if( pGZS->nSvrIdx<=0 )	// 说明是清空了的GZS信息
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(1119,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::SendGZSInfo0ToCLSAndChangeCLSStatus,BAD GZS,0x%X", pCLS->nID);
		return	0;
	}
	// 发送GZS信息
	GMS_CLS_GZSINFO_T		GZSInfo;
	GZSInfo.nCmd			= GMS_CLS_GZSINFO;
	GZSInfo.nSvrIdx			= pGZS->nSvrIdx;
	GZSInfo.nPort			= pGZS->nPort;
	GZSInfo.IP				= pGZS->IP;
	GZSInfo.nSvrIdx			= pGZS->nSvrIdx;
	pCLS->SendMsg(&GZSInfo, sizeof(GZSInfo));
	// 改变CLS的状态
	pCLS->nGZSNumToConnect	++;
	pCLS->nStatus			= CLSUnit::STATUS_CONNECTINGGZS;
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(560,PNGS_INT_RLS)"CLS2_CONNECT_GZS,0x%X,%d,%d", pCLS->nID, pGZS->nSvrIdx, pCLS->nGZSNumToConnect);
	return	0;
}
int		GMS_MAINSTRUCTURE::SendPlayerIDInCLSToGZS(CLSUnit *pCLS, GZSUnit *pGZS)
{
	assert(pGZS->pCntr);
	if( pCLS->GetCurPlayerNum()>0 )
	{
		m_vectrawbuf.resize( pCLS->GetCurPlayerNum()*wh_sizeinclass(GMS_GZS_PLAYER_ONLINE_T, anClientID[0]) + wh_offsetof(GMS_GZS_PLAYER_ONLINE_T, anClientID) );
		GMS_GZS_PLAYER_ONLINE_T		*pOL	= (GMS_GZS_PLAYER_ONLINE_T *)m_vectrawbuf.getbuf();
		pOL->nCmd	= GMS_GZS_PLAYER_ONLINE;
		pOL->nCLSID	= pCLS->nID;
		whDList<PlayerUnit *>::node	*pNode	= pCLS->dlPlayer.begin();
		int		i	= 0;
		int		j	= 0;
		while( pNode!=pCLS->dlPlayer.end() )
		{
			PlayerUnit	*pPlayer	= pNode->data;
			pNode	= pNode->next;
			if( pPlayer->nStatus == PlayerUnit::STATUS_WANTCLS )
			{
				j	++;
				// 还在连接CLS过程中的不需要发送给GZS
				continue;
			}
			pOL->anClientID[i++]	= pPlayer->nID;
		}
		assert((i+j) == pCLS->dlPlayer.size() );
		// 如果j不为0，则按照m_vectrawbuf.size()发送就会有问题
		if( i>0 )
		{
			pGZS->pCntr->SendMsg(m_vectrawbuf.getbuf(), i*wh_sizeinclass(GMS_GZS_PLAYER_ONLINE_T, anClientID[0]) + wh_offsetof(GMS_GZS_PLAYER_ONLINE_T, anClientID));
		}
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SendCmdToAllCLS(const void *pCmd, size_t nSize)
{
	for(UAF_CLS_T::iterator it=m_pSHMData->m_CLSs.begin(); it!=m_pSHMData->m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, pCmd, nSize);
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SendCmdToPlayerCLS(int nClientID, const void *pCmd, size_t nSize)
{
	return	SendCmdToPlayerCLS(m_pSHMData->m_Players.GetPtrByID(nClientID), pCmd, nSize);
}
int		GMS_MAINSTRUCTURE::SendCmdToPlayerCLS(PlayerUnit *pPlayer, const void *pCmd, size_t nSize)
{
	if( !pPlayer )
	{
		return	-1;
	}
	CLSUnit	*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pPlayer->nCLSID);
	if( pCLS )
	{
		return	pCLS->SendMsg(pCmd, nSize);
	}
	return	-2;
}
int		GMS_MAINSTRUCTURE::SendCmdToAllGZS(const void *pCmd, size_t nSize)
{
	for(int i=1;i<GZS_MAX_NUM;++i)
	{
		GZSUnit	*pGZS	= &m_pSHMData->m_GZSs[i];
		if( pGZS->pCntr )
		{
			pGZS->pCntr->SendMsg(pCmd, nSize);
		}
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::QueueRawCmdToGZS(unsigned char nSvrIdx, const void *pCmd, size_t nSize)
{
	unsigned char	*pnSvrIdx		= (unsigned char *)m_queueGZSSvrCmd.InAlloc(sizeof(*pnSvrIdx) + nSize);
	if(!pnSvrIdx)
	{
		assert(0);	// 不应该把内存耗尽的
		return	-1;
	}
	*pnSvrIdx				= nSvrIdx;
	memcpy(pnSvrIdx+1, pCmd, nSize);
	return	0;
}
int		GMS_MAINSTRUCTURE::SendSvrCmdToAllGZS(const void *pCmd, size_t nSize, unsigned char nFromSvrIdx, unsigned char nExceptSvrIdx)
{
	for(int i=1;i<GZS_MAX_NUM;i++)
	{
		if( nExceptSvrIdx==i )
		{
			continue;
		}
		GZSUnit	*pGZS	= &m_pSHMData->m_GZSs[i];
		if( pGZS->pCntr )
		{
			SendSvrCmdToGZS(pGZS, pCmd, nSize, nFromSvrIdx);
		}
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SendSvrCmdToGZS(unsigned char nSvrIdx, const void *pCmd, size_t nSize, unsigned char nFromSvrIdx)
{
	return	SendSvrCmdToGZS(m_pSHMData->m_GZSs+nSvrIdx, pCmd, nSize, nFromSvrIdx);
}
int		GMS_MAINSTRUCTURE::SendSvrCmdToGZS(GZSUnit *pGZS, const void *pCmd, size_t nSize, unsigned char nFromSvrIdx)
{
	assert( pGZS );
	// 合成指令
	pngs_cmd_t	*pnCmd	= (pngs_cmd_t *)pCmd;
	if( m_setRegGZSSvrCmd.has(*pnCmd) )
	{
		unsigned char	*pnSvrIdx		= (unsigned char *)m_queueGZSSvrCmd.InAlloc(sizeof(*pnSvrIdx) + wh_offsetof(GMS_GZS_GAMECMD_T, data) + nSize);
		if(!pnSvrIdx)
		{
			assert(0);	// 不应该把内存耗尽的
			return	-1;
		}
		*pnSvrIdx				= pGZS->nSvrIdx;
		GMS_GZS_GAMECMD_T	*pGameCmd	= (GMS_GZS_GAMECMD_T *)(pnSvrIdx+1);
		pGameCmd->nCmd			= GMS_GZS_GAMECMD;
		pGameCmd->nFromSvrIdx	= nFromSvrIdx;
		memcpy(pGameCmd->data, pCmd, nSize);
	}
	else
	{
		// 直接发送
		m_vectrawbuf.resize( wh_offsetof(GMS_GZS_GAMECMD_T, data) + nSize );
		GMS_GZS_GAMECMD_T	*pGameCmd	= (GMS_GZS_GAMECMD_T *)m_vectrawbuf.getbuf();
		pGameCmd->nCmd			= GMS_GZS_GAMECMD;
		pGameCmd->nFromSvrIdx	= nFromSvrIdx;
		memcpy(pGameCmd->data, pCmd, nSize);
		if( pGZS->pCntr )
		{
			if( pGZS->pCntr->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size())<0 )
			{
				return	-2;
			}
		}
		else
		{
			return	-3;
		}
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SendCmdToAllGMTOOL(const void *pCmd, size_t nSize)
{
	for(UAF_GMTOOL_T::iterator it=m_pSHMData->m_GMTOOLs.begin(); it!=m_pSHMData->m_GMTOOLs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, pCmd, nSize);
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SendCmdToPlayer(int nPlayerID, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	return	SendCmdToPlayer(m_pSHMData->m_Players.GetPtrByID(nPlayerID), nChannel, pCmd, nSize);
}
int		GMS_MAINSTRUCTURE::SendCmdToPlayer(GMS_MAINSTRUCTURE::PlayerUnit *pPlayer, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	if( !pPlayer )
	{
		return	-1;
	}
	// 找到它的CLS
	CLSUnit	*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pPlayer->nCLSID);
	// CLS对象应该不会在玩家之前消失的
	assert(pCLS);
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS_CLIENT_DATA_T, data) + nSize);
	SVR_CLS_CLIENT_DATA_T	&ClientData	= *(SVR_CLS_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	ClientData.nCmd			= SVR_CLS_CLIENT_DATA;
	ClientData.nChannel		= nChannel;
	ClientData.nClientID	= pPlayer->nID;
	memcpy(ClientData.data, pCmd, nSize);
	if( pCLS->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size())<0 )
	{
		return	-2;
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	// 分别整理出来这些玩家分别属于哪些CLS，然后分CLS处理
	whDList<CLSUnit *>			dlCLS;
	int	i;
	for(i=0;i<nPlayerNum;i++)
	{
		PlayerUnit	*pPlayer	= m_pSHMData->m_Players.GetPtrByID(paPlayerID[i]);
		if( pPlayer )
		{
			CLSUnit	*pCLS		= m_pSHMData->m_CLSs.GetPtrByID(pPlayer->nCLSID);
			if( pCLS )
			{
				pCLS->pvectClientID->push_back(paPlayerID[i]);
				if( !pCLS->dlnodeInSCTMP.isinlist() )
				{
					dlCLS.AddToTail(&pCLS->dlnodeInSCTMP);
				}
			}
		}
	}

	whDList<CLSUnit *>::node	*pNode	= dlCLS.begin();
	while( pNode!=dlCLS.end() )
	{
		CLSUnit	*pCLS	= pNode->data;
		SendCmdToMultiPlayerInOneCLS(pCLS->nID, pCLS->pvectClientID->getbuf(), pCLS->pvectClientID->size(), nChannel, pCmd, nSize);
		pCLS->pvectClientID->clear();
		pNode->leave();
		pNode	= dlCLS.begin();
	}

	return	0;
}
int		GMS_MAINSTRUCTURE::SendCmdToMultiPlayerInOneCLS(int nCLSID, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	return	SendCmdToMultiPlayerInOneCLS(m_pSHMData->m_CLSs.GetPtrByID(nCLSID), paPlayerID, nPlayerNum, nChannel, pCmd, nSize);
}
int		GMS_MAINSTRUCTURE::SendCmdToMultiPlayerInOneCLS(CLSUnit *pCLS, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	if( !pCLS )
	{
		return	-1;
	}

	m_vectrawbuf.resize(SVR_CLS_MULTICLIENT_DATA_T::GetTotalSize(nPlayerNum, nSize));
	SVR_CLS_MULTICLIENT_DATA_T	&Cmd	= *(SVR_CLS_MULTICLIENT_DATA_T *)m_vectrawbuf.getbuf();
	Cmd.nCmd			= SVR_CLS_MULTICLIENT_DATA;
	Cmd.nChannel		= nChannel;
	Cmd.nClientNum		= nPlayerNum;
	memcpy(Cmd.anClientID, paPlayerID, sizeof(int)*nPlayerNum);
	memcpy(Cmd.GetDataPtr(), pCmd, nSize);

	if( pCLS->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size())<0 )
	{
		return	-2;
	}

	return	0;
}
int		GMS_MAINSTRUCTURE::SendCmdToAllPlayer(unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx)
{
	// 直接发送给所有CLS告诉它们广播
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS_ALL_CLIENT_DATA_T, data) + nSize);
	SVR_CLS_ALL_CLIENT_DATA_T	&Cmd	= *(SVR_CLS_ALL_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	Cmd.nCmd		= SVR_CLS_ALL_CLIENT_DATA;
	Cmd.nChannel	= nChannel;
	Cmd.nSvrIdx		= nSvrIdx;
	memcpy(Cmd.data, pCmd, nSize);
	for(UAF_CLS_T::iterator it=m_pSHMData->m_CLSs.begin(); it!=m_pSHMData->m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx)
{
	// 直接发送给所有CLS告诉它们按tag发送
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS_TAGGED_CLIENT_DATA_T, data) + nSize);
	SVR_CLS_TAGGED_CLIENT_DATA_T	&Cmd	= *(SVR_CLS_TAGGED_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	Cmd.nCmd		= SVR_CLS_TAGGED_CLIENT_DATA;
	Cmd.nChannel	= nChannel;
	Cmd.nTagIdx		= nTagIdx;
	Cmd.nTagVal		= nTagVal;
	Cmd.nSvrIdx		= nSvrIdx;
	memcpy(Cmd.data, pCmd, nSize);
	for(UAF_CLS_T::iterator it=m_pSHMData->m_CLSs.begin(); it!=m_pSHMData->m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal)
{
	SVR_CLS_SET_TAG_TO_CLIENT_T	Cmd;
	Cmd.nCmd		= SVR_CLS_SET_TAG_TO_CLIENT;
	Cmd.nTagIdx		= nTagIdx;
	Cmd.nTagVal		= nTagVal;
	Cmd.nClientID	= nClientID;
	return	SendCmdToPlayerCLS(nClientID, &Cmd, sizeof(Cmd));
}
