#ifndef __NotificationMngS_H__
#define __NotificationMngS_H__

#include <map>
using namespace std;

#include "WHCMN/inc/whvector.h"
#include "WHCMN/inc/whlist.h"
#include "WHCMN/inc/whunitallocator.h"

#include "GS4Web_i.h"

using namespace n_whcmn;

namespace n_pngs
{
class GS4Web_i;

class CNotificationMngS
{
public:
	CNotificationMngS();
	~CNotificationMngS();
public:
	// ���֪ͨ
	int		AddNotification(Web_Notification_T* pNotification);
	// ɾ��֪ͨ
	int		DelNotificationByID(int nNotificationID);
	int		DelNotificationBeforeTime(time_t nTime);
	int		DelNotificationAfterTime(time_t nTime);
	int		DelNotificationByGroup(int nGroupID);
	int		DelNotificationAll();
	int		DelNotification(Web_Notification_T* pNotification);
	// ��ȡ֪ͨ
	Web_Notification_T*	GetNotificationByID(int nNotificationID);
	int		GetNotificationByGroup(int nGroupID, whvector<Web_Notification_T*>& vectNotifications);
	int		GetNotificationBeforeTime(time_t nTime, whvector<Web_Notification_T*>& vectNotifications);
	int		GetNotificationAfterTime(time_t nTime, whvector<Web_Notification_T*>& vectNotifications);
	int		GetNotificationAll(whvector<Web_Notification_T*>& vectNotifications);
private:
	typedef	map<int, Web_Notification_T*>		map_id2notification_t;
	typedef	whlist<Web_Notification_T*>			list_notification_t;
	map_id2notification_t						m_mapID2Notification;
	list_notification_t							m_listNotification;		// ����ʱ�併������
};
}

#endif
