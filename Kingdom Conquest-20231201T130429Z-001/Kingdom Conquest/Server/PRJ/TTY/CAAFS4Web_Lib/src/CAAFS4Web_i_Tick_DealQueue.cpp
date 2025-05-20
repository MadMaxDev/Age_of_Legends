#include "../inc/CAAFS4Web_i.h"

using namespace n_pngs;

int		CAAFS4Web_i::Tick_DealQueue()
{
	// 如果connecting队列中有空位就把后面的补上
	int		nAvail	= m_cfginfo.nClientConnectingNumMax - m_nConnectingNum;
	if (nAvail <= 0)
	{
		// connecting的人太多了
		return 0;
	}

	// 获得可以加入多少个
	int		nNum	= m_generWaitTransToConnect.MakeDecision(m_itv.getfloatInterval());
	if (nNum < nAvail)
	{
		if (nNum==0 && m_cfginfo.bAtLeastOnTransToConnectPerTick) // 在tick过快的时候至少可以检测一次是否有玩家需要进入
		{
			nNum	= 1;
		}
		nAvail		= nNum;
	}
	// 从需要大的队列开始,从waiting队列的头开始给后面分配nAvail个放入connecting队列中
	for (int i=m_vectWaitChannel.size()-1; i>=0; i--)
	{
		WAITCHANNEL_T*	pC	= m_vectWaitChannel.getptr(i);
		if (pC->pdlWait->size() > 0)
		{
			whDList<PlayerUnit*>::node*	pNode			= pC->pdlWait->begin();
			while (m_nLogicProxyAvailPlayerNum>0 && nAvail>0
				&& pNode!=pC->pdlWait->end())
			{
				whDList<PlayerUnit*>::node*	pCurNode	= pNode;
				// 先算出下一个(后面的操作会导致用户离开队列)
				nAvail--;
				pNode				= pNode->next;
				m_nLogicProxyAvailPlayerNum--;
				// 玩家进入connecting队列
				TransPlayerFromWaitToConnectQueue(pCurNode->data);
				// 将等待队头变成下一个序号
				pC->nWaitingSeq0	= pCurNode->data->nSeq;
			}
		}
	}

	return 0;
}
int		CAAFS4Web_i::TransPlayerFromWaitToConnectQueue(PlayerUnit* pPlayer)
{
	// 应该不可能,在waitqueue中就不能是其他状态
	assert(pPlayer->nStatus == PlayerUnit::STATUS_WAITING);

	// 计数++
	m_nConnectingNum++;

	// 清楚原来的时间事件
	pPlayer->teid.quit();
	// 从waiting队列中出来
	pPlayer->nodeWC.leave();
	// 加入到connecting队列中
	m_dlPlayerConnecting.AddToTail(&pPlayer->nodeWC);

	// 生成用户连接密码
	pPlayer->nPasswordToConnectCLS4Web	= GenPasswordToConnectCLS4Web();
	
	// 状态转换
	pPlayer->nStatus	= PlayerUnit::STATUS_CONNECTINGCLS4Web;

	// 向LP提出请求
	CAAFS4Web_LP_CLIENTWANTCLS4Web_T	ClientWantCLS4Web;
	ClientWantCLS4Web.nCmd					= CAAFS4Web_LP_CLIENTWANTCLS4Web;
	ClientWantCLS4Web.nClientIDInCAAFS4Web	= pPlayer->nID;
	ClientWantCLS4Web.IP					= pPlayer->IP;
	ClientWantCLS4Web.nPassword				= pPlayer->nPasswordToConnectCLS4Web;
	ClientWantCLS4Web.nTermType				= pPlayer->nTermType;
	m_msgerLogicProxy.SendMsg(&ClientWantCLS4Web, sizeof(ClientWantCLS4Web));

	GLOGGER2_WRITEFMT(GLOGGER_ID_BILLING, GLGR_STD_HDR(1393,WEB_LOGIN)"TransPlayerFromWaitToConnectQueue,0x%X,%s,%u", pPlayer->nID, cmn_get_IP(pPlayer->IP), pPlayer->nPasswordToConnectCLS4Web);

	return 0;
}
