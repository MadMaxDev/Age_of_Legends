#include "../inc/CAAFS4Web_i.h"

using namespace n_pngs;

int		CAAFS4Web_i::Tick_BeforeDealCmdIn()
{
	Tick_DoMyOwnWork();
	return 0;
}

int		CAAFS4Web_i::Tick_AfterDealCmdIn()
{
	return 0;
}

int		CAAFS4Web_i::Tick_DoMyOwnWork()
{
	m_tickNow			= wh_gettickcount();

	// ��ȡʱ����
	m_itv.tick();

	// �շ�����GMS����Ϣ
	m_msgerLogicProxy.Tick();
	
	// �����LP��������Ϣ
	if (Tick_DealLogicProxyMsg() < 0)
	{
		return -1;
	}

	// ���մ��û���������Ϣ
	m_pEpollServer->TickRecv();
	// ������û���������Ϣ
	if (Tick_DealClientMsg() < 0)
	{
		return -2;
	}

	// ������м��ת��
	if (Tick_DealQueue() < 0)
	{
		return -3;
	}

	// ����ʱ���¼�
	if (Tick_DealTE() < 0)
	{
		return -4;
	}

	// ���������߼�,����ͻ��˷���
	m_pEpollServer->TickLogic();
	m_pEpollServer->TickSend();

	return 0;
}
