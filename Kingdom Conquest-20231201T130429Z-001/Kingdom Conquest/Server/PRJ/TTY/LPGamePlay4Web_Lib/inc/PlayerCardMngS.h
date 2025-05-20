#ifndef __PlayerCardMngS_H__
#define __PlayerCardMngS_H__

#include "WHCMN/inc/whhash.h"

#include "GameMngS.h"

namespace n_pngs
{
class CGameMngS;
class CPlayerCardMngS
{
public:
	CPlayerCardMngS();
	~CPlayerCardMngS();
public:
	int			Init(CGameMngS* pHost);
	int			LoadCards(PlayerCard* pCards, int nNum);
	int			AddCharName2AccountNameMap(char* szCharName, tty_id_t nAccountID);
	PlayerCard*	AddCard(tty_id_t nAccountID);
	PlayerCard*	GetCardByAccountID(tty_id_t nAccountID, bool bLoadFromDBIfNotExist=true);
	PlayerCard*	GetCardByPosition(int nPosX, int nPosY, bool bLoadFromDBIfNotExist=true);
	PlayerCard*	GetCardByName(char* szCharName, bool bLoadFromDBIfNotExist=true);
	inline int	ChangeName(char* szOldCharName, char* szNewCharName)
	{
		PlayerCard*	pCard	= GetCardByName(szOldCharName);
		if (pCard == NULL)
		{
			return -100;
		}
		WH_STRNCPY0(pCard->szName, szNewCharName);
		return ChangeCharName2AccountNameMap(szOldCharName, szNewCharName);
	}
	int			UpdateLevelRankData();
	inline	int	GetTotalPlayerNum()
	{
		return m_cards.size();
	}
private:
	int			ChangeCharName2AccountNameMap(char* szOldCharName, char* szNewCharName);
	PlayerCard*	_GetCard(tty_id_t nAccountID, bool bLoadFromDBIfNotExist);
private:
	CGameMngS*	m_pHost;
	whhash<tty_id_t, PlayerCard>	m_cards;
	whhash<whstr4hash, tty_id_t>	m_charName2AccountID;		// 使用模糊查找,这个就没意义
};
}

#endif
