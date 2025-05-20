#ifndef __NotificationMngS_H__
#define __NotificationMngS_H__

#include "GameMngS.h"

#include "WHCMN/inc/whlist.h"

#include "../../Common/inc/tty_def_gs4web.h"
#include "../../Common/inc/tty_common_db_timeevent.h"

using namespace n_whcmn;

namespace n_pngs
{
class CGameMngS;
class CNotificationMngS
{
public:
	CNotificationMngS(CGameMngS* pHost);
	~CNotificationMngS();
public:
	// 添加通知
	int		AddNotification(Web_Notification_T* pNotification);
	// 删除通知
	int		DelNotificationByID(int nNotificationID);
	int		DelNotificationBeforeTime(time_t nTime);
	int		DelNotificationAfterTime(time_t nTime);
	int		DelNotificationByGroup(int nGroupID);
	int		DelNotificationAll();
	// 发送通知给客户端
	int		SendNotificationToPlayer(PlayerData* pPlayer);
private:
	typedef	whlist<Web_Notification_T*>		list_notification_t;
	list_notification_t		m_listNotification;

	CGameMngS*				m_pHost;

	whvector<char>			m_vectrawbuf;
};
}

#endif
