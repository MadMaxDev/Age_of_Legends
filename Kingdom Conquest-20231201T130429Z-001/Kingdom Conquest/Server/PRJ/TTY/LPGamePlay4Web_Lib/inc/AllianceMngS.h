#ifndef __AllianceMngS_H__
#define __AllianceMngS_H__

#include "WHCMN/inc/whlist.h"
#include "WHCMN/inc/whhash.h"

#include "GameMngS.h"
#include "../../Common/inc/tty_common_excel_def.h"

using namespace n_whcmn;

namespace n_pngs
{
class CGameMngS;

// 最好隔一段时间就重新载入一次,主要是为了做一些纠正
// 比如:
// 1.建造成功,可是数据库断连,未返回成功信息,需要下次重新载入进行纠正(概率极低:可以使用侦测到断线就全部标记需要重新载入)
// 2.可能因为时间顺序关系,内存不能正确反应数据库信息,这个可能需要下次重新载入才能正确反应数据库信息(概率极低:可以配置为每过一段时间重新载入一次)
struct AllianceData_T 
{
	AllianceUnit					basicInfo;
	whvector<AllianceMemberUnit>	members;
	whvector<AllianceBuildingUnit>	buildings;
	whvector<AllianceJoinEvent>		joinEvents;
	whvector<AllianceBuildingTimeEvent>	buildingTEs;
	whlist<AllianceMailUnit*>		mailList;

	AllianceData_T()
	{
		Clear();
	}
	~AllianceData_T()
	{
		Clear();
	}
	void	Clear()
	{
		memset(&basicInfo, 0, sizeof(basicInfo));
		members.clear();
		buildings.clear();
		joinEvents.clear();
		buildingTEs.clear();

		ClearMailList();
	}
	void ClearMailList()
	{
		for (whlist<AllianceMailUnit*>::iterator it=mailList.begin(); it!=mailList.end(); ++it)
		{
			delete[] (char*)(*it);
		}
		mailList.clear();
	}
	inline int GetLevel()
	{
		for (unsigned int i=0; i<buildings.size(); i++)
		{
			if (buildings[i].nExcelID == 1)
			{
				return buildings[i].nLevel;
			}
		}
		return 0;
	}
	inline int	SetLevel(unsigned int nLevel)
	{
		for (unsigned int i=0; i<buildings.size(); i++)
		{
			if (buildings[i].nExcelID == 1)
			{
				buildings[i].nLevel	= nLevel;
			}
		}
		return 0;
	}
	inline	int	GetMemberPosition(tty_id_t nAccountID)
	{
		for (unsigned int i=0; i<members.size(); i++)
		{
			if (members[i].nAccountID == nAccountID)
			{
				return members[i].nPosition;
			}
		}
		return alliance_position_none;
	}
	inline int	SetMemberCup(tty_id_t nAccountID, unsigned int nCup)
	{
		for (unsigned int i=0; i<members.size(); i++)
		{
			if (members[i].nAccountID == nAccountID)
			{
				members[i].nCup		= nCup;
				return 0;
			}
		}
		return -1;
	}
};
class CAllianceMngS
{
public:
	CAllianceMngS();
	~CAllianceMngS();
public:
	int		Init(CGameMngS* pHost);
	AllianceData_T*	GetAlliance(tty_id_t nAllianceID);
	AllianceData_T*	AddAlliance(tty_id_t nAllianceID);
	int		RemoveAlliance(tty_id_t nAllianceID);
	inline int		GetMaxMemberByAllianceLevel(unsigned int nLevel)
	{
		if (nLevel > m_vectExcelAllianceCongress.size())
		{
			return 0;
		}
		return m_vectExcelAllianceCongress[nLevel-1].nMemberNum;
	}
	inline int		LoadAllianceRank(int nNum, tty_id_t* pnAllianceIDs)
	{
		m_vectAllianceRank.clear();
		m_vectAllianceRank.pushn_back(pnAllianceIDs, nNum);
		return 0;
	}
	inline int		GetAllianceRank(tty_id_t nAllianceID)
	{
		for (unsigned int i=0; i<m_vectAllianceRank.size(); i++)
		{
			if (m_vectAllianceRank[i] == nAllianceID)
			{
				return i+1;
			}
		}
		return 0;
	}
	inline int		LoadExcelAllianceCongress(int nNum, ExcelAllianceCongress* pExcels)
	{
		m_vectExcelAllianceCongress.clear();
		m_vectExcelAllianceCongress.pushn_back(pExcels, nNum);
		return 0;
	}
	inline int		SetAllianceMemberCup(tty_id_t nAllianceID, tty_id_t nAccountID, unsigned int nCup)
	{
		if (nAllianceID == 0)
		{
			return -1;
		}
		AllianceData_T*	pAlliance	= GetAlliance(nAllianceID);
		if (pAlliance == NULL)
		{
			return -2;
		}
		return pAlliance->SetMemberCup(nAccountID, nCup);
	}
public:
	CGameMngS*	m_pHost;
	whhash<tty_id_t, AllianceData_T*>	m_hashAlliances;
	whvector<ExcelAllianceCongress>		m_vectExcelAllianceCongress;
	whvector<tty_id_t>					m_vectAllianceRank;
	whvector<ExcelAlliancePosition>		m_vectExcelAlliancePosition;
};
}

#endif
