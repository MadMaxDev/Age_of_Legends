#include "../inc/GameMngS.h"

using namespace n_pngs;

int		CGameMngS::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();

	whtimequeue::ID_T	id;
	TQUNIT_T*	pUnit	= NULL;
	while (m_TQ.GetUnitBeforeTime(m_tickNow, (void**)&pUnit, &id) == 0)
	{
		assert(pUnit->tefunc != NULL);
		(this->*pUnit->tefunc)(pUnit);
		m_TQ.Del(id);
	}

	return 0;
}
