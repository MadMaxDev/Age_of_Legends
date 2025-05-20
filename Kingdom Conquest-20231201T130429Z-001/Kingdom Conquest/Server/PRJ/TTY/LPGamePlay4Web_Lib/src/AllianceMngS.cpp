#include "../inc/AllianceMngS.h"

using namespace n_pngs;

CAllianceMngS::CAllianceMngS()
: m_pHost(NULL)
{
}
CAllianceMngS::~CAllianceMngS()
{
}
int		CAllianceMngS::Init(CGameMngS* pHost)
{
	m_pHost		= pHost;
	m_hashAlliances.resizebucket(m_pHost->GetCfgInfo()->nAllianceNumExpect);
	return 0;
}
AllianceData_T*	CAllianceMngS::GetAlliance(tty_id_t nAllianceID)
{
	AllianceData_T*	pAlliance	= NULL;
	if (m_hashAlliances.get(nAllianceID, pAlliance))
	{
		return pAlliance;
	}
	return NULL;
}
AllianceData_T*	CAllianceMngS::AddAlliance(tty_id_t nAllianceID)
{
	AllianceData_T*	pAlliance	= new AllianceData_T();
	if (m_hashAlliances.put(nAllianceID, pAlliance))
	{
		return pAlliance;
	}
	delete pAlliance;
	return NULL;
}
int		CAllianceMngS::RemoveAlliance(tty_id_t nAllianceID)
{
	AllianceData_T*	pAlliance	= NULL;
	unsigned int	nRank		= 0;
	if (m_hashAlliances.get(nAllianceID, pAlliance))
	{
		nRank		= pAlliance->basicInfo.nRank;
		WHSafeDelete(pAlliance);
		m_hashAlliances.erase(nAllianceID);
	}
	if (nRank != 0)
	{
		whhash<tty_id_t, AllianceData_T*>::kv_iterator	it	= m_hashAlliances.begin();
		for (; it != m_hashAlliances.end(); ++it)
		{
			AllianceData_T* pAllianceTmp	= it.getvalue();
			if (pAllianceTmp->basicInfo.nRank > nRank)
			{
				pAllianceTmp->basicInfo.nRank--;
			}
		}
	}
	m_vectAllianceRank.erasevalue(nAllianceID);
	return 0;
}
