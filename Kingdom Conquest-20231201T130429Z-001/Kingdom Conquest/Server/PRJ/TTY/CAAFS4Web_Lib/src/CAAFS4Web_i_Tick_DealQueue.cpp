#include "../inc/CAAFS4Web_i.h"

using namespace n_pngs;

int		CAAFS4Web_i::Tick_DealQueue()
{
	// ���connecting�������п�λ�ͰѺ���Ĳ���
	int		nAvail	= m_cfginfo.nClientConnectingNumMax - m_nConnectingNum;
	if (nAvail <= 0)
	{
		// connecting����̫����
		return 0;
	}

	// ��ÿ��Լ�����ٸ�
	int		nNum	= m_generWaitTransToConnect.MakeDecision(m_itv.getfloatInterval());
	if (nNum < nAvail)
	{
		if (nNum==0 && m_cfginfo.bAtLeastOnTransToConnectPerTick) // ��tick�����ʱ�����ٿ��Լ��һ���Ƿ��������Ҫ����
		{
			nNum	= 1;
		}
		nAvail		= nNum;
	}
	// ����Ҫ��Ķ��п�ʼ,��waiting���е�ͷ��ʼ���������nAvail������connecting������
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
				// �������һ��(����Ĳ����ᵼ���û��뿪����)
				nAvail--;
				pNode				= pNode->next;
				m_nLogicProxyAvailPlayerNum--;
				// ��ҽ���connecting����
				TransPlayerFromWaitToConnectQueue(pCurNode->data);
				// ���ȴ���ͷ�����һ�����
				pC->nWaitingSeq0	= pCurNode->data->nSeq;
			}
		}
	}

	return 0;
}
int		CAAFS4Web_i::TransPlayerFromWaitToConnectQueue(PlayerUnit* pPlayer)
{
	// Ӧ�ò�����,��waitqueue�оͲ���������״̬
	assert(pPlayer->nStatus == PlayerUnit::STATUS_WAITING);

	// ����++
	m_nConnectingNum++;

	// ���ԭ����ʱ���¼�
	pPlayer->teid.quit();
	// ��waiting�����г���
	pPlayer->nodeWC.leave();
	// ���뵽connecting������
	m_dlPlayerConnecting.AddToTail(&pPlayer->nodeWC);

	// �����û���������
	pPlayer->nPasswordToConnectCLS4Web	= GenPasswordToConnectCLS4Web();
	
	// ״̬ת��
	pPlayer->nStatus	= PlayerUnit::STATUS_CONNECTINGCLS4Web;

	// ��LP�������
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
