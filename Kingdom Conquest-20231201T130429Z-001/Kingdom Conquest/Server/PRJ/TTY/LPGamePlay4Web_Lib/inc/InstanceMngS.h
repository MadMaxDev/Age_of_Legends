#ifndef __InstanceMngS_H__
#define __InstanceMngS_H__

#include "WHCMN/inc/whlist.h"
#include "WHCMN/inc/whhash.h"

#include "GameMngS.h"

using namespace n_whcmn;

namespace n_pngs
{
class CGameMngS;

//////////////////////////////////////////////////////////////////////////
// 用于处理副本的tag64
//////////////////////////////////////////////////////////////////////////
struct InstanceData_T
{
	whlist<tty_id_t>		listAccountIDs;

	void	Clear()
	{
		listAccountIDs.clear();
	}

	bool	CanDelInstance()
	{
		return listAccountIDs.size()==0;
	}

	void	DelAccount(tty_id_t nAccountID)
	{
		whlist<tty_id_t>::iterator	it	= listAccountIDs.begin();
		for (; it!=listAccountIDs.end(); ++it)
		{
			if (*it == nAccountID)
			{
				listAccountIDs.erase(it);
				return;
			}
		}
	}

	void	AddAccount(tty_id_t nAccountID)
	{
		whlist<tty_id_t>::iterator	it	= listAccountIDs.begin();
		for (; it!=listAccountIDs.end(); ++it)
		{
			if (*it == nAccountID)
			{
				return;
			}
		}
		listAccountIDs.push_back(nAccountID);
	}
};
class CInstanceMngS
{
public:
	CInstanceMngS();
	~CInstanceMngS();
public:
	int		Init(CGameMngS* pHost);
	InstanceData_T*	GetInstance(tty_id_t nInstanceID);
	InstanceData_T*	AddInstance(tty_id_t nInstanceID);
	int		RemoveInstance(tty_id_t nInstanceID);
	int		RemoveInstanceAll();
	int		LoadInstances(unsigned int nNum, SimpleInstanceData* pUnit);

	// 功能函数
	int		CreateInstance(tty_id_t nInstanceID, tty_id_t nAccountID);
	int		JoinInstance(tty_id_t nInstanceID, tty_id_t nAccountID);
	int		ExitInstance(tty_id_t nInstanceID, tty_id_t nAccountID);
	int		DestroyInstance(tty_id_t nInstanceID);
	int		RefreshInstance(tty_id_t nInstanceID, tty_id_t nAccountID);
private:
	CGameMngS*	m_pHost;
	whhash<tty_id_t, InstanceData_T>	m_hashInstances;
};
}

#endif
