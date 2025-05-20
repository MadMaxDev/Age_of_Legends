#ifndef __ChatMngS_H__
#define __ChatMngS_H__

#include "GameMngS.h"

#include "WHCMN/inc/whhash.h"
#include "WHCMN/inc/whlist.h"

using namespace n_whcmn;

namespace n_pngs
{
class CGameMngS;

class CChatMngS
{
public:
	CChatMngS();
	~CChatMngS();
public:
	int		Init(CGameMngS*	pHost);
	int		AddWorldChatEntry(unsigned int nSize, void* pData, time_t nTime);
	int		AddAllianceChatEntry(tty_id_t nAllianceID, unsigned int nSize, void* pData, time_t nTime);
	int		RemoveAllianceChat(tty_id_t nAllianceID);
	
	int		GetRecentChat(PlayerData* pPlayer);
	inline unsigned int NewChatID()
	{
		return ++m_nChatID;
	}
public:
	CGameMngS*		m_pHost;
	unsigned int	m_nChatID;		// 聊天内容ID

	// 聊天数据实际内容保存位置
	typedef	whlist<chat_entry_T>	chat_list;
	chat_list	m_WorldChatList;
	whhash<tty_id_t, chat_list>		m_AllianceChatMap;

private:
	inline int ReleaseChatList(chat_list* pList)
	{
		chat_list::iterator	it	= pList->begin();
		for (; it != pList->end(); ++it)
		{
			delete[]	(*it).pData;
		}
		pList->clear();
		return 0;
	}
};
}

#endif
