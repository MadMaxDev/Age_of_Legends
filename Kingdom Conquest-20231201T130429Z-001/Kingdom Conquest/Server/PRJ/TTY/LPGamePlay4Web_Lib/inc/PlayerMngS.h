#ifndef __PlayerMngS_H__
#define __PlayerMngS_H__

#include "WHCMN/inc/whunitallocator.h"
#include "WHCMN/inc/whhash.h"
#include "WHCMN/inc/whlist.h"
#include "WHCMN/inc/whtimequeue.h"
#include "../../Common/inc/tty_common_PlayerData4Web.h"
#include "GameMngS.h"

#include <map>
using namespace std;

namespace n_pngs
{
class CGameMngS;
class CPlayerMngS
{
public:
	int				Init(int nMaxPlayer,CGameMngS *pHost);
	PlayerData*		AddPlayer(int nPlayerGID);
	void			DeletePlayer(int nPlayerGID);
	PlayerData*		GetPlayerByGID(int nPlayerGID);
public:
	// 时间队列
	struct TQUNIT_T 
	{
		typedef	void	(CPlayerMngS::*TEDEAL_T)(TQUNIT_T*);
		TEDEAL_T	tefunc;	
		int			nPlayerGID;
	};
	int				Tick_DealTE();
	void			TEDeal_RefreshMail_TimeOut(TQUNIT_T* pUnit);
	void			SetTE_RefreshMail(unsigned int nPlayerGID);
private:
	whunitallocatorFixed<PlayerData>		m_Players;
	CGameMngS*		m_pHost;
	whtick_t		m_nTickNow;

	whtimequeue		m_TQ;

	typedef map<unsigned int,whtimequeue::ID_T*>	map_playergid2teid;
	map_playergid2teid		m_mapPlayerGID2TEID;
};
}

#endif
