#include "../inc/CLS4Web_i.h"
#include <set>

using namespace n_pngs;

int		CLS4Web_i::Tick_BeforeDealCmdIn()
{
	Tick_DoMyOwnWork();
	return 0;
}

int		CLS4Web_i::Tick_AfterDealCmdIn()
{
	if (m_nStatus == STATUS_KICK_ALL_PLAYER)
	{
		if (m_pEpollServer->GetCntrNum() == 0)
		{
			// ��ȡ�����Ƶ�ַ
			sockaddr_in				addr;
			cmn_get_saaddr_by_ip(&addr, m_epollServerInfo.szIP);
			// ���Ͷ˿ں͵�ַ
			CLS4Web_LP_HI1_T		Cmd1;
			Cmd1.nCmd				= CLS4Web_LP_HI1;
			Cmd1.nPort				= m_epollServerInfo.iPort;
			Cmd1.IP					= addr.sin_addr.s_addr;
			m_msgerLP.SendMsg(&Cmd1, sizeof(Cmd1));

			// ״̬����Ϊ����
			m_nStatus				= STATUS_WORKING;
		}
	}
	return 0;
}

int		CLS4Web_i::Tick_DoMyOwnWork()
{
	m_tickNow		= wh_gettickcount();
	
	if (m_nStatus!=STATUS_WORKING && m_nStatus!=STATUS_KICK_ALL_PLAYER)
	{
		return -1;
	}

	dbgtimeuse	dtu;
	dtu.BeginMark();
	dtu.Mark();

	// �շ�����LP����Ϣ
	m_msgerLP.Tick();

	dtu.Mark();

	// ��������LP��������Ϣ
	if (Tick_DealLogicProxyMsg() < 0)
	{
		return -2;
	}

	dtu.Mark();

	// ���մ��û���������Ϣ
	m_pEpollServer->TickRecv();
	
	dtu.Mark();

	// ������û���������Ϣ
	if (Tick_DealClientMsg() < 0)
	{
		return -3;
	}

	dtu.Mark();

	// ����ʱ�¼�
	if (Tick_DealTE() < 0)
	{
		return -4;
	}
	
	dtu.Mark();

	// ���������߼�
	m_pEpollServer->TickLogic();

	dtu.Mark();

	// ��ͻ��˷���
	m_pEpollServer->TickSend();

	dtu.Mark();

#ifdef _DEBUG
	if (m_cfginfo.nCmdStatInterval && m_loopCmdStat.check())
	{
		m_loopCmdStat.reset();
		// ���һ����־
		m_vectrawbuf.clear();
		*m_vectrawbuf.getbuf()	= 0;
		int		nSec			= m_cfginfo.nCmdStatInterval/1000;
		std::multimap<CMDSTAT_T, unsigned short>		mapSort;

		for (size_t i=0; i<m_vectCmdStat.size(); i++)
		{
			CMDSTAT_T&	cs		= m_vectCmdStat[i];
			if (cs.nCount > 0)
			{
				mapSort.insert(std::make_pair(cs, i));
			}
		}
		int		nCount	= 0;
		std::multimap<CMDSTAT_T, unsigned short>::iterator it	= mapSort.begin();
		for (;it!=mapSort.end() && nCount++<=50; ++it)
		{
			m_vectrawbuf.f_aprintf(32, ",0x%04X %d %d", (*it).second, (*it).first.nByte, (*it).first.nCount);
		}
		if (m_vectrawbuf.size() > 512)
		{
			m_vectrawbuf[512]	= 0;
		}
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1380,CLS4Web_RT)"iostat,%d,%d,%s", nSec, m_pEpollServer->GetCntrNum(), m_vectrawbuf.getbuf());
		// ���ͳ�Ƽ�¼
		memset(m_vectCmdStat.getbuf(), 0, m_vectCmdStat.totalbytes());
	}
#endif

	if (dtu.GetDiff() >= m_cfginfo.nBigTickFilter)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BIGTICK)"CLS4Web,%s", dtu.Disp());
	}

	return 0;
}
