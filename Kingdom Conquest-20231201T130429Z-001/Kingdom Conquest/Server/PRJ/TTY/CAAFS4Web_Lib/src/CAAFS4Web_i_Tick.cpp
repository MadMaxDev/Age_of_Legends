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

	// 获取时间间隔
	m_itv.tick();

	// 收发来自GMS的消息
	m_msgerLogicProxy.Tick();
	
	// 处理从LP发来的消息
	if (Tick_DealLogicProxyMsg() < 0)
	{
		return -1;
	}

	// 接收从用户发来的消息
	m_pEpollServer->TickRecv();
	// 处理从用户发来的消息
	if (Tick_DealClientMsg() < 0)
	{
		return -2;
	}

	// 处理队列间的转换
	if (Tick_DealQueue() < 0)
	{
		return -3;
	}

	// 处理时间事件
	if (Tick_DealTE() < 0)
	{
		return -4;
	}

	// 处理连接逻辑,并向客户端发包
	m_pEpollServer->TickLogic();
	m_pEpollServer->TickSend();

	return 0;
}
