#ifndef __RankMngS_H__
#define __RankMngS_H__

#include <map>
#include "WHCMN/inc/whvector.h"
#include "WHCMN/inc/whhash.h"

#include "GameMngS.h"

using namespace n_whcmn;
using namespace std;

namespace n_pngs
{
class CGameMngS;

class CRankMngS
{
public:
	CRankMngS();
	~CRankMngS();

	typedef whhash<tty_id_t, unsigned int>		onlyid2rank_map_T;
	typedef	whvector<RankData>					rankdata_vector_T;
public:
	int		Init(CGameMngS* pHost);
	int		RefreshRank(unsigned int nRankType, int nNum, void* pRankDataArray);
	int		LoadRankFromDB();
	int		GetRank(unsigned int nType, tty_id_t nAccountID);
	inline	rankdata_vector_T*	GetRankDataVector(unsigned int nType)
	{
		return _GetRankDataVector(nType, false);
	}
private:
	int		_InitAllRankTypes();
	int		_LoadRankDataFromFile();
	int		_QueryRankFromDB(unsigned int nRankType);
	const char*	_MakeRankFileName(unsigned int nRankType);
	onlyid2rank_map_T*	_GetOnlyID2RankMap(unsigned int nType, bool bCreateIfNotExist=true);
	rankdata_vector_T*	_GetRankDataVector(unsigned int nType, bool bCreateIfNotExist=true);
public:
	CGameMngS*					m_pHost;
	whvector<unsigned int>		m_vectRankTypes;

	map<unsigned int, onlyid2rank_map_T*>		m_mapRankType2RankMap;
	map<unsigned int, rankdata_vector_T*>		m_mapRankDataVect;
};
}

#endif
