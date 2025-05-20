#include "../inc/NotificationMngS.h"

using namespace n_pngs;

CNotificationMngS::CNotificationMngS(CGameMngS* pHost)
: m_pHost(pHost)
{
	m_vectrawbuf.reserve(32*1024);
}
CNotificationMngS::~CNotificationMngS()
{
	DelNotificationAll();
}
// 添加通知
int		CNotificationMngS::AddNotification(Web_Notification_T* pNotification)
{
	// 先生成数据
	Web_Notification_T*	pNewNotification	= (Web_Notification_T*)(new char[pNotification->GetTotalSize()]);
	memcpy(pNewNotification, pNotification, pNotification->GetTotalSize());

	list_notification_t::iterator	it	= m_listNotification.begin();
	if (it == m_listNotification.end())
	{
		m_listNotification.push_back(pNewNotification);
	}
	else
	{
		for (; it != m_listNotification.end(); ++it)
		{
			if ((*it)->nNotificationID < pNewNotification->nNotificationID)
			{
				m_listNotification.insertbefore(it, pNewNotification);
				break;
			}
		}
		if (it == m_listNotification.end())
		{
			m_listNotification.push_back(pNewNotification);
		}
	}

	return 0;
}
int		CNotificationMngS::DelNotificationByID(int nNotificationID)
{
	list_notification_t::iterator	it	= m_listNotification.begin();
	for (; it != m_listNotification.end(); ++it)
	{
		if ((*it)->nNotificationID == nNotificationID)
		{
			delete[] (char*)(*it);
			// 删除节点
			m_listNotification.erase(it);
			break;
		}
	}
	return 0;
}
int		CNotificationMngS::DelNotificationBeforeTime(time_t nTime)
{
	list_notification_t::iterator	it	= m_listNotification.begin();
	list_notification_t::iterator	itNext	= it;
	while (it != m_listNotification.end())
	{
		++itNext;
		if ((*it)->nTime <= nTime)
		{
			delete[] (char*)(*it);
			m_listNotification.erase(it);
		}
		it	= itNext;
	}
	return 0;
}
int		CNotificationMngS::DelNotificationAfterTime(time_t nTime)
{
	list_notification_t::iterator	it	= m_listNotification.begin();
	list_notification_t::iterator	itNext	= it;
	while (it != m_listNotification.end())
	{
		++itNext;
		if ((*it)->nTime >= nTime)
		{
			// 清理内存
			delete[] (char*)(*it);
			// 删除节点
			m_listNotification.erase(it);
		}
		it	= itNext;
	}
	return 0;
}
int		CNotificationMngS::DelNotificationByGroup(int nGroupID)
{
	list_notification_t::iterator	it	= m_listNotification.begin();
	list_notification_t::iterator	itNext	= it;
	while (it != m_listNotification.end())
	{
		++itNext;
		if ((*it)->nGroupID == nGroupID)
		{
			// 清理内存
			delete[] (char*)(*it);
			// 删除节点
			m_listNotification.erase(it);
		}
		it	= itNext;
	}
	return 0;
}
int		CNotificationMngS::DelNotificationAll()
{
	list_notification_t::iterator	it		= m_listNotification.begin();
	list_notification_t::iterator	itNext	= it;
	while (it != m_listNotification.end())
	{
		++itNext;
		// 清理内存
		delete[] (char*)(*it);
		// 删除节点
		m_listNotification.erase(it);
		it	= itNext;
	}
	return 0;
}
int		CNotificationMngS::SendNotificationToPlayer(PlayerData* pPlayer)
{
	int	nMaxNum	= m_listNotification.size();
	if (nMaxNum > m_pHost->GetCfgInfo()->nNotificationSendToClientNum)
	{
		nMaxNum	= m_pHost->GetCfgInfo()->nNotificationSendToClientNum;
	}
	// 预留够大,m_vectrawbuf的buf不会失效
	int	nTotalSize	= 0;
	int	nNum	= 0;
	list_notification_t::iterator it		= m_listNotification.begin();
	for (int i=0; i<nMaxNum; i++)
	{
		if ((*it)->nNotificationID > pPlayer->m_CharAtb.nNotificationID)
		{
			nNum++;
			nTotalSize	+= (*it)->GetTotalSize();
		}
		else
		{
			break;
		}
		++it;
	}
	m_vectrawbuf.resize(sizeof(STC_GAMECMD_GET_NOTIFICATION_T) + nTotalSize);
	STC_GAMECMD_GET_NOTIFICATION_T*	pCmd	= (STC_GAMECMD_GET_NOTIFICATION_T*)m_vectrawbuf.getbuf();
	pCmd->nCmd		= TTY_LPGAMEPLAY_CLIENT_GAME_CMD;
	pCmd->nGameCmd	= STC_GAMECMD_GET_NOTIFICATION;
	pCmd->nNum		= nNum;
	pCmd->nRst		= STC_GAMECMD_GET_NOTIFICATION_T::RST_OK;
	it				= m_listNotification.begin();
	Web_Notification_T*	pNotify	= (Web_Notification_T*)wh_getptrnexttoptr(pCmd);
	for (int i=0; i<nNum; i++)
	{
		memcpy(pNotify, *it, (*it)->GetTotalSize());

		pNotify		= (Web_Notification_T*)wh_getoffsetaddr(pNotify, pNotify->GetTotalSize());
		++it;
	}
	g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	return 0;
}
