#include "../inc/PlayerCardMngS.h"

using namespace n_pngs;

CPlayerCardMngS::CPlayerCardMngS()
: m_pHost(NULL)
{
}
CPlayerCardMngS::~CPlayerCardMngS()
{

}
int		CPlayerCardMngS::Init(CGameMngS* pHost)
{
	m_pHost		= pHost;
	m_cards.resizebucket(m_pHost->GetCfgInfo()->nGroupPlayerTotalNumExpect);
	return 0;
}
int		CPlayerCardMngS::LoadCards(PlayerCard* pCards, int nNum)
{
	for (int i=0; i<nNum; i++)
	{
		if (!m_cards.put(pCards[i].nAccountID, pCards[i]))
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,LP_RT)"LoadCards ERROR,0x%"WHINT64PRFX"X", pCards->nAccountID);
		}
		AddCharName2AccountNameMap(pCards[i].szName, pCards[i].nAccountID);
		// 在世界中设置地理信息
		m_pHost->GetWorldMng()->SetTileType(pCards[i].nPosX, pCards[i].nPosY, TileUnit::TILETYPE_TERRAIN, pCards[i].nAccountID);
	}
	return 0;
}
int		CPlayerCardMngS::AddCharName2AccountNameMap(char* szCharName, tty_id_t nAccountID)
{
	if (!m_charName2AccountID.put(szCharName, nAccountID))
	{
		return -1;
	}
	return 0;
}
int		CPlayerCardMngS::ChangeCharName2AccountNameMap(char* szOldCharName, char* szNewCharName)
{
	tty_id_t	nAccountID	= 0;
	if (!m_charName2AccountID.get(szOldCharName, nAccountID))
	{
		return -1;
	}
	m_charName2AccountID.erase(szOldCharName);
	if (!m_charName2AccountID.put(szNewCharName, nAccountID))
	{
		return -2;
	}
	return 0;
}
PlayerCard*		CPlayerCardMngS::AddCard(tty_id_t nAccountID)
{
	return m_cards.getptr(nAccountID, true);
}
PlayerCard*		CPlayerCardMngS::GetCardByAccountID(tty_id_t nAccountID, bool bLoadFromDBIfNotExist)
{
	return _GetCard(nAccountID, bLoadFromDBIfNotExist);
}
PlayerCard*		CPlayerCardMngS::GetCardByPosition(int nPosX, int nPosY, bool bLoadFromDBIfNotExist)
{
	TileUnit* pTile	= m_pHost->GetWorldMng()->GetTile(nPosX, nPosY);
	if (pTile==NULL || pTile->nType!=TileUnit::TILETYPE_TERRAIN || pTile->nAccountID==0)
	{
		return NULL;
	}
	return _GetCard(pTile->nAccountID, bLoadFromDBIfNotExist);
}
PlayerCard*		CPlayerCardMngS::GetCardByName(char* szCharName, bool bLoadFromDBIfNotExist)
{
	tty_id_t	nAccountID	= 0;
	if (!m_charName2AccountID.get(szCharName, nAccountID))
	{
		return NULL;
	}
	return _GetCard(nAccountID, bLoadFromDBIfNotExist);
}
PlayerCard*	CPlayerCardMngS::_GetCard(tty_id_t nAccountID, bool bLoadFromDBIfNotExist)
{
	PlayerCard*	pCard	= m_cards.getptr(nAccountID);
	if (pCard != NULL)
	{
		unsigned int	nTimeNow	= wh_time();
		pCard->nProtectTime	= nTimeNow>pCard->nProtectTimeEndTick?0:pCard->nProtectTimeEndTick-nTimeNow;
	}
	else if (bLoadFromDBIfNotExist)
	{
		g_pLPNet->GetPlayerCardFromDB(nAccountID);
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,NO_PLAYERCARD)"0x%"WHINT64PRFX"X", nAccountID);
	}
	return pCard;
}
int		CPlayerCardMngS::UpdateLevelRankData()
{
	CRankMngS*	pRankMng			= m_pHost->GetRankMng();
	whhash<tty_id_t, PlayerCard>::kv_iterator	it	= m_cards.begin();
	for (; it != m_cards.end(); ++it)
	{
		it.getvalue().nLevelRank	= pRankMng->GetRank(rank_type_char_level, it.getkey());
	}
	return 0;
}
