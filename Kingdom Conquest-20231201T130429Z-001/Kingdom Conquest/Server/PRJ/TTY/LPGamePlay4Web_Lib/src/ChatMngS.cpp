#include "../inc/ChatMngS.h"

using namespace n_pngs;

CChatMngS::CChatMngS()
: m_pHost(NULL)
, m_nChatID(0)
{
	ReleaseChatList(&m_WorldChatList);

	whhash<tty_id_t, chat_list>::kv_iterator	itHash	= m_AllianceChatMap.begin();
	for (; itHash != m_AllianceChatMap.end(); ++itHash)
	{
		ReleaseChatList(&itHash.getvalue());
	}
}
CChatMngS::~CChatMngS()
{

}
int		CChatMngS::Init(CGameMngS* pHost)
{
	m_pHost		= pHost;
	return 0;
}
int		CChatMngS::AddWorldChatEntry(unsigned int nSize, void* pData, time_t nTime)
{
	// 1.插入
	chat_entry_T*	pEntry	= m_WorldChatList.push_front();
	pEntry->nTime			= nTime;
	pEntry->nSize			= nSize;
	pEntry->pData			= new char[nSize];
	memcpy(pEntry->pData, pData, nSize);
	
	// 2.删除多余的(使用while要多做一次判断)
	if ((int)m_WorldChatList.size() > m_pHost->GetCfgInfo()->nStoredChatEntryNum)
	{
		chat_list::iterator	it	= --m_WorldChatList.end();
		delete[]	(*it).pData;
		m_WorldChatList.pop_back();
	}
	return 0;
}
int		CChatMngS::AddAllianceChatEntry(tty_id_t nAllianceID, unsigned int nSize, void* pData, time_t nTime)
{
	// 1.插入
	chat_list*	pList		= m_AllianceChatMap.getptr(nAllianceID, true);
	if (pList == NULL)
	{
		return -1;
	}
	chat_entry_T*	pEntry	= pList->push_front();
	pEntry->nTime			= nTime;
	pEntry->nSize			= nSize;
	pEntry->pData			= new char[nSize];
	memcpy(pEntry->pData, pData, nSize);

	// 2.删除多余的(使用while要多做一次判断)
	if ((int)pList->size() > m_pHost->GetCfgInfo()->nStoredChatEntryNum)
	{
		chat_list::iterator	it	= --pList->end();
		delete[]	(*it).pData;
		pList->pop_back();
	}
	return 0;
}
int		CChatMngS::RemoveAllianceChat(tty_id_t nAllianceID)
{
	chat_list*	pList		= m_AllianceChatMap.getptr(nAllianceID, false);
	if (pList == NULL)
	{
		return 0;
	}
	ReleaseChatList(pList);
	m_AllianceChatMap.erase(nAllianceID);
	return 0;
}
int		CChatMngS::GetRecentChat(PlayerData* pPlayer)
{
	// 链表初始化
	chat_list*	arrList[chat_channel_max];
	arrList[chat_channel_private]	= NULL;
	arrList[chat_channel_alliance]	= m_AllianceChatMap.getptr(pPlayer->m_CharAtb.nAllianceID, false);
	arrList[chat_channel_world]		= &m_WorldChatList;
	// 迭代器初始化
	chat_list::iterator	arrIt[chat_channel_max];
	if (arrList[chat_channel_alliance] != NULL)
	{
		arrIt[chat_channel_alliance]	= arrList[chat_channel_alliance]->begin();
	}
	arrIt[chat_channel_world]		= m_WorldChatList.begin();

	// 发送消息包
	whlist<chat_entry_T*>	tmpList;
	for (int i=0; i<m_pHost->GetCfgInfo()->nSendChatEntryNum; i++)
	{
		time_t			nTime		= 0;
		unsigned int	nChannel	= chat_channel_private;
		chat_entry_T*	pEntry		= NULL;
		for (int j=chat_channel_private; j<chat_channel_max; j++)
		{
			if (arrList[j]!=NULL && arrIt[j]!=arrList[j]->end())
			{
				if ((*arrIt[j]).nTime >= nTime)
				{
					nChannel		= j;
					pEntry			= (chat_entry_T*)&(*arrIt[j]);
					nTime			= pEntry->nTime;
				}
			}
		}
		if (pEntry != NULL)
		{
			tmpList.push_front(pEntry);

			++arrIt[nChannel];
			if (arrIt[nChannel] == arrList[nChannel]->end())
			{
				arrList[nChannel]	= NULL;
			}
		}
		else
		{
			// 没有记录了
			break;
		}
	}
	for (whlist<chat_entry_T*>::iterator it=tmpList.begin(); it!=tmpList.end(); ++it)
	{
		chat_entry_T*	pEntry		= (*it);
		g_pLPNet->SendCmdToClient(pPlayer->m_nPlayerGID, pEntry->pData, pEntry->nSize);
	}

	return 0;
}
