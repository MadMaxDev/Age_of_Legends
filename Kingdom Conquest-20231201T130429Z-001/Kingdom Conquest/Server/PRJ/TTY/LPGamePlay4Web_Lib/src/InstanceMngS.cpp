#include "../inc/InstanceMngS.h"

using namespace n_pngs;

CInstanceMngS::CInstanceMngS()
	: m_pHost(NULL)
{

}
CInstanceMngS::~CInstanceMngS()
{

}
int		CInstanceMngS::Init(CGameMngS* pHost)
{
	m_pHost		= pHost;
	return 0;
}
InstanceData_T*	CInstanceMngS::GetInstance(tty_id_t nInstanceID)
{
	return m_hashInstances.getptr(nInstanceID, false);
}
InstanceData_T*	CInstanceMngS::AddInstance(tty_id_t nInstanceID)
{
	return m_hashInstances.getptr(nInstanceID, true);
}
int		CInstanceMngS::RemoveInstance(tty_id_t nInstanceID)
{
	m_hashInstances.erase(nInstanceID);
	return 0;
}
int		CInstanceMngS::RemoveInstanceAll()
{
	m_hashInstances.clear();
	return 0;
}
int		CInstanceMngS::LoadInstances(unsigned int nNum, SimpleInstanceData* pUnit)
{
	RemoveInstanceAll();
	for (int i=0; i<nNum; i++)
	{
		InstanceData_T*	pInstance	= m_hashInstances.getptr(pUnit->nInstanceID, true);
		if (pInstance != NULL)
		{
			pInstance->AddAccount(pUnit->nAccountID);

			PlayerData*	pPlayer		= g_pLPNet->GetPlayerData(pUnit->nAccountID);
			if (pPlayer != NULL)
			{
				g_pLPNet->SetPlayerTag64(pPlayer->m_nPlayerGID, pUnit->nInstanceID, false);
			}
		}
		pUnit++;
	}
	return 0;
}
int		CInstanceMngS::CreateInstance(tty_id_t nInstanceID, tty_id_t nAccountID)
{
	InstanceData_T*	pInstance	= AddInstance(nInstanceID);
	if (pInstance != NULL)
	{
		pInstance->AddAccount(nAccountID);

		PlayerData*	pPlayer		= g_pLPNet->GetPlayerData(nAccountID);
		if (pPlayer != NULL)
		{
			g_pLPNet->SetPlayerTag64(pPlayer->m_nPlayerGID, nInstanceID, false);
		}
	}
	return 0;
}
int		CInstanceMngS::JoinInstance(tty_id_t nInstanceID, tty_id_t nAccountID)
{
	InstanceData_T*	pInstance	= GetInstance(nInstanceID);
	if (pInstance != NULL)
	{
		pInstance->AddAccount(nAccountID);

		PlayerData*	pPlayer		= g_pLPNet->GetPlayerData(nAccountID);
		if (pPlayer != NULL)
		{
			g_pLPNet->SetPlayerTag64(pPlayer->m_nPlayerGID, nInstanceID, false);
		}
	}
	return 0;
}
int		CInstanceMngS::ExitInstance(tty_id_t nInstanceID, tty_id_t nAccountID)
{
	InstanceData_T*	pInstance	= GetInstance(nInstanceID);
	if (pInstance != NULL)
	{
		PlayerData*	pPlayer		= g_pLPNet->GetPlayerData(nAccountID);
		if (pPlayer != NULL)
		{
			g_pLPNet->SetPlayerTag64(pPlayer->m_nPlayerGID, nInstanceID, true);
		}

		pInstance->DelAccount(nAccountID);
		if (pInstance->listAccountIDs.size() == 0)
		{
			DestroyInstance(nInstanceID);
		}
	}
	return 0;
}
int		CInstanceMngS::DestroyInstance(tty_id_t nInstanceID)
{
	InstanceData_T*	pInstance	= m_hashInstances.getptr(nInstanceID);
	if (pInstance != NULL)
	{
		whlist<tty_id_t>::iterator	it	= pInstance->listAccountIDs.begin();
		for (; pInstance->listAccountIDs.end()!=it; ++it)
		{
			PlayerData*	pPlayer	= g_pLPNet->GetPlayerData(*it);
			if (pPlayer != NULL)
			{
				g_pLPNet->SetPlayerTag64(pPlayer->m_nPlayerGID, nInstanceID, true);
			}
		}
	}
	m_hashInstances.erase(nInstanceID);
	return 0;
}
int		CInstanceMngS::RefreshInstance(tty_id_t nInstanceID, tty_id_t nAccountID)
{
	InstanceData_T*	pInstance	= GetInstance(nInstanceID);
	if (pInstance == NULL)
	{
		CreateInstance(nInstanceID, nAccountID);
	}
	else
	{
		JoinInstance(nInstanceID, nAccountID);
	}
	return 0;
}
