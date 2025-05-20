// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_Tick_DealQueue.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CAAFS中Tick_DealQueue部分的具体实现(处理等待队列和连接队列的变换)
// CreationDate : 2005-08-04
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::Tick_DealQueue()
{
	// 如果connecting队列中有空位就把后面的补上
	int		nAvail	= m_cfginfo.nClientConnectingNumMax - m_nConnectingNum;
	if( nAvail<=0 )
	{
		// Connecting的人太多了
		return	0;
	}

	// 获得可以加入多少个
	int		nNum	= m_generWaitTransToConnect.MakeDecision(m_itv.getfloatInterval());
	if( nNum<nAvail )
	{
		if( nNum==0 && m_cfginfo.bAtLeastOneTransToConnectPerTick )	// 这样保证在tick过快的时候至少可以检测一次是否有玩家需要进入
		{
			nNum	= 1;
		}
		nAvail		= nNum;
	}
	// 从需要大的队列开始，从Waiting队列的头开始给后面分配nAvail个放入Connecting队列中
	for(int i=m_vectWaitChannel.size()-1;i>=0;i--)
	{
		WAITCHANNEL_T	*pC	= m_vectWaitChannel.getptr(i);
		if( pC->pdlWait->size()>0 )
		{
			whDList<PlayerUnit *>::node	*pNode	= pC->pdlWait->begin();
#ifdef UNITTEST
			if(m_nGMSAvailPlayerNum==0)	printf("---deal tq : GMS : %d,  Avail : %d , Channel : %d\n",m_nConnectingNum,nAvail,i);
#endif
			while( m_nGMSAvailPlayerNum>0 && nAvail>0 && pNode != pC->pdlWait->end() )
			{
				whDList<PlayerUnit *>::node	*pCurNode	= pNode;
				// 先算出下一个(因为后面的操作中可能会导致用户离开Waiting队)
				nAvail					--;
				pNode					= pNode->next;
				m_nGMSAvailPlayerNum	--;
				// 玩家进入Connecting队列
				TransPlayerFromWaitToConnectQueue(pCurNode->data);
				// 等待队头变成下一个序号
				pC->nWaitingSeq0		= pCurNode->data->nSeq;
			}
		}
	}

	return	0;
}
int		CAAFS2_I::TransPlayerFromWaitToConnectQueue(PlayerUnit *pPlayer)
{
	// 应该不可能，在waitqueue中就不能是其他状态
	assert( pPlayer->nStatus == PlayerUnit::STATUS_WAITING );

	// 记数++
	m_nConnectingNum	++;

	// 清除原来的时间事件
	pPlayer->teid.quit();
	// 从Waiting队列中出来
	pPlayer->nodeWC.leave();
	// 加入到Connecting队列中
	m_dlPlayerConnecting.AddToTail(&pPlayer->nodeWC);

	// 生成用户连接密码
	pPlayer->nPasswordToConnectCLS	= GenPasswordToConnectCLS();

	// 状态转换
	pPlayer->nStatus	= PlayerUnit::STATUS_CONNECTINGCLS;	

	// 向GMS提出请求
	CAAFS_GMS_CLIENTWANTCLS_T		ClientWantCLS;
	ClientWantCLS.nCmd				= CAAFS_GMS_CLIENTWANTCLS;
	ClientWantCLS.nClientIDInCAAFS	= pPlayer->nID;
	ClientWantCLS.IP				= pPlayer->IP;
	ClientWantCLS.nPassword			= pPlayer->nPasswordToConnectCLS;
	ClientWantCLS.nNetworkType		= pPlayer->nNetworkType;
	m_msgerGMS.SendMsg(&ClientWantCLS, sizeof(ClientWantCLS));

	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(912,PNGS_RT)"TransPlayerFromWaitToConnectQueue,0x%X,%d,%d,%d,%d", pPlayer->nID, m_nConnectingNum, m_nGMSAvailPlayerNum, m_Players.size(),pPlayer->nSeq);

	// 不用设超时以免CLS没有反应（因为在GMS中一定会设定这个超时的，如果CLS一段时间内没反应或者CLS掉了则会有相应处理）
	// GMS一定会给用户传回一个结果GMS_CAAFS_CLIENTCANGOTOCLS（无论成功或失败）

	return	0;
}
