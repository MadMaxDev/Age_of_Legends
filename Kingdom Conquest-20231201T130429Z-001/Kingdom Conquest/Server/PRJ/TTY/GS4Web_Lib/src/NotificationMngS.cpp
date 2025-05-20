#include "../inc/NotificationMngS.h"

using namespace n_pngs;

CNotificationMngS::CNotificationMngS()
{
}
CNotificationMngS::~CNotificationMngS()
{
	DelNotificationAll();
}
int		CNotificationMngS::AddNotification(Web_Notification_T* pNotification)
{
	map_id2notification_t::iterator	itMap	= m_mapID2Notification.find(pNotification->nNotificationID);
	if (itMap != m_mapID2Notification.end())
	{
		return -1;
	}
	int		nNotifyTotalSize				= pNotification->GetTotalSize();
	Web_Notification_T*		pNewNotify		= (Web_Notification_T*)(new char[nNotifyTotalSize]);
	memcpy(pNewNotify, pNotification, nNotifyTotalSize);
	// 插入映射
	m_mapID2Notification.insert(map_id2notification_t::value_type(pNewNotify->nNotificationID, pNewNotify));

	// 插入链表(按时间降序)
	list_notification_t::iterator it	= m_listNotification.begin();
	if (it == m_listNotification.end())
	{
		m_listNotification.push_back(pNewNotify);
	}
	else
	{
		for (; it != m_listNotification.end(); ++it)
		{
			if ((*it)->nNotificationID < pNotification->nNotificationID)
			{
				m_listNotification.insertbefore(it, pNewNotify);
				break;
			}
		}
		if (it == m_listNotification.end())
		{
			m_listNotification.push_back(pNewNotify);
		}
	}
	
	return 0;
}
int		CNotificationMngS::DelNotificationByID(int nNotificationID)
{
	map_id2notification_t::iterator	it	= m_mapID2Notification.find(nNotificationID);
	if (it != m_mapID2Notification.end())
	{
		DelNotification(it->second);
	}
	return 0;
}
int		CNotificationMngS::DelNotificationBeforeTime(time_t nTime)
{
	list_notification_t::iterator	it		= m_listNotification.begin();
	list_notification_t::iterator	itNext	= it;
	while (it != m_listNotification.end())
	{
		Web_Notification_T*	pNotification	= *it;
		itNext								= ++it;
		if (pNotification->nTime <= nTime)
		{
			DelNotification(pNotification);
		}
		it								= itNext;
	}
	return 0;
}
int		CNotificationMngS::DelNotificationAfterTime(time_t nTime)
{
	list_notification_t::iterator	it		= m_listNotification.begin();
	list_notification_t::iterator	itNext	= it;
	while (it != m_listNotification.end())
	{
		Web_Notification_T*	pNotification	= *it;
		itNext								= ++it;
		if (pNotification->nTime >= nTime)
		{
			DelNotification(pNotification);
		}
		it								= itNext;
	}
	return 0;
}
int		CNotificationMngS::DelNotificationByGroup(int nGroupID)
{
	list_notification_t::iterator	it		= m_listNotification.begin();
	list_notification_t::iterator	itNext	= it;
	while (it != m_listNotification.end())
	{
		Web_Notification_T*	pNotification	= *it;
		itNext								= ++it;
		if (pNotification->nGroupID == nGroupID)
		{
			DelNotification(pNotification);
		}
		it								= itNext;
	}
	return 0;
}
int		CNotificationMngS::DelNotificationAll()
{
	list_notification_t::iterator	it		= m_listNotification.begin();
	list_notification_t::iterator	itNext	= it;
	while (it != m_listNotification.end())
	{
		Web_Notification_T*	pNotification	= *it;
		itNext							= ++it;
		DelNotification(pNotification);
		it								= itNext;
	}
	return 0;
}
int		CNotificationMngS::DelNotification(Web_Notification_T* pNotification)
{
	if (pNotification == NULL)
	{
		return -1;
	}

	// 从链表中删除
	list_notification_t::iterator	it	= m_listNotification.begin();
	for (; it != m_listNotification.end(); ++it)
	{
		if ((*it)->nNotificationID == pNotification->nNotificationID)
		{
			m_listNotification.erase(it);
			break;
		}
	}

	// 从映射中删除
	m_mapID2Notification.erase(pNotification->nNotificationID);

	// 清理数据,从缓冲中删除
	delete[]	(char*)pNotification;

	return 0;
}
Web_Notification_T*	CNotificationMngS::GetNotificationByID(int nNotificationID)
{
	map_id2notification_t::iterator	it	= m_mapID2Notification.find(nNotificationID);
	if (it != m_mapID2Notification.end())
	{
		return it->second;
	}
	return NULL;
}
int		CNotificationMngS::GetNotificationByGroup(int nGroupID, whvector<Web_Notification_T*>& vectNotifications)
{
	list_notification_t::iterator	it	= m_listNotification.begin();
	for (; it != m_listNotification.end(); ++it)
	{
		Web_Notification_T*	pNotification	= *it;
		if (pNotification->nGroupID==nGroupID)
		{
			vectNotifications.push_back(pNotification);
		}
	}
	return 0;
}
int		CNotificationMngS::GetNotificationBeforeTime(time_t nTime, whvector<Web_Notification_T*>& vectNotifications)
{
	list_notification_t::iterator	it	= m_listNotification.begin();
	for (; it != m_listNotification.end(); ++it)
	{
		Web_Notification_T*	pNotification	= *it;
		if (pNotification->nTime < nTime)
		{
			vectNotifications.push_back(pNotification);
		}
	}
	return 0;
}
int		CNotificationMngS::GetNotificationAfterTime(time_t nTime, whvector<Web_Notification_T*>& vectNotifications)
{
	list_notification_t::iterator	it	= m_listNotification.begin();
	for (; it != m_listNotification.end(); ++it)
	{
		Web_Notification_T*	pNotification	= *it;
		if (pNotification->nTime > nTime)
		{
			vectNotifications.push_back(pNotification);
		}
	}
	return 0;
}
int		CNotificationMngS::GetNotificationAll(whvector<Web_Notification_T*>& vectNotifications)
{
	list_notification_t::iterator	it	= m_listNotification.begin();
	for (; it != m_listNotification.end(); ++it)
	{
		vectNotifications.push_back(*it);
	}
	return 0;
}
