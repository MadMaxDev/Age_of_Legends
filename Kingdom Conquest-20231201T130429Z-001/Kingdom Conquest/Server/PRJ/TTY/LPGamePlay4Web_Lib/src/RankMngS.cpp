#include "../inc/RankMngS.h"
#include "WHCMN/inc/whdir.h"

using namespace n_pngs;
using namespace n_whcmn;

CRankMngS::CRankMngS()
: m_pHost(NULL)
{

}
CRankMngS::~CRankMngS()
{
	{
		map<unsigned int, onlyid2rank_map_T*>::iterator	it	= m_mapRankType2RankMap.begin();
		for (; it!=m_mapRankType2RankMap.end(); ++it)
		{
			delete it->second;
			it->second	= NULL;
		}
	}

	{
		map<unsigned int, rankdata_vector_T*>::iterator	it	= m_mapRankDataVect.begin();
		for (; it!=m_mapRankDataVect.end(); ++it)
		{
			delete it->second;
			it->second	= NULL;
		}
	}
}
int		CRankMngS::Init(CGameMngS* pHost)
{
	m_pHost	= pHost;

	if (_InitAllRankTypes() < 0)
	{
		return -1;
	}
	
	if (_LoadRankDataFromFile() < 0)
	{
		return -2;
	}

	whdir_SureMakeDirForFile(m_pHost->GetCfgInfo()->szRankFileDir);

	return 0;
}
int		CRankMngS::RefreshRank(unsigned int nRankType, int nNum, void* pRankDataArray)
{
	rankdata_vector_T*	pVect		= _GetRankDataVector(nRankType);
	if (pVect != NULL)
	{
		pVect->resize(nNum);
		memcpy(pVect->getbuf(), pRankDataArray, nNum*sizeof(RankData));

		onlyid2rank_map_T*	pMap	= _GetOnlyID2RankMap(nRankType);
		if (pMap != NULL)
		{
			pMap->clear();

			RankData*	pRankData	= NULL;
			for (int i=0; i<nNum; i++)
			{
				pRankData			= pVect->getptr(i);
				pMap->put(pRankData->nOnlyID, i+1);
			}
		}
	}

	whfile*	pFile					= whfile_OpenCmnFile(_MakeRankFileName(nRankType), "wb");
	if (pFile != NULL)
	{
		pFile->Write(pRankDataArray, nNum*sizeof(RankData));
		whfile_CloseCmnFile(pFile);
	}
	
	return 0;
}
int		CRankMngS::LoadRankFromDB()
{
	int		nCurHour				= wh_time()/3600;
	for (int i=0; i<m_vectRankTypes.size(); i++)
	{
		whfile*	pFile				= whfile_OpenCmnFile(_MakeRankFileName(m_vectRankTypes[i]), "rb");
		if (pFile != NULL)
		{
			int	nChgHour			= pFile->FileTime()/3600;
			if (nChgHour != nCurHour)
			{
				_QueryRankFromDB(m_vectRankTypes[i]);
			}
			whfile_CloseCmnFile(pFile);
		}
		else
		{
			_QueryRankFromDB(m_vectRankTypes[i]);
		}
	}

	return 0;
}
int		CRankMngS::GetRank(unsigned int nType, tty_id_t nAccountID)
{
	onlyid2rank_map_T*	pMap	= _GetOnlyID2RankMap(nType, false);
	if (pMap == NULL)
	{
		return 0;
	}
	unsigned int		nRank	= 0;
	if (pMap->get(nAccountID, nRank))
	{
		return nRank;
	}
	else
	{
		rankdata_vector_T*	pVect	= GetRankDataVector(rank_type_char_level);
		if (pVect==NULL || pVect->size()==0)
		{
			return 0;
		}
		else
		{
			return pVect->size()+1;
		}
	}
}

int		CRankMngS::_InitAllRankTypes()
{
	m_vectRankTypes.clear();

	m_vectRankTypes.push_back(rank_type_char_level);
	m_vectRankTypes.push_back(rank_type_char_gold);
	m_vectRankTypes.push_back(rank_type_char_diamond);
	m_vectRankTypes.push_back(rank_type_instance_wangzhe);

	return 0;
}
int		CRankMngS::_LoadRankDataFromFile()
{
	unsigned int	nType		= 0;
	int				nCurHour	= wh_time()/3600;
	for (int i=0; i<m_vectRankTypes.size(); i++)
	{
		nType			= m_vectRankTypes[i];
		whfile*	pFile	= whfile_OpenCmnFile(_MakeRankFileName(nType), "rb");
		if (pFile == NULL)
		{
			_QueryRankFromDB(nType);
		}
		else
		{
			int	nLastChgHour	= pFile->FileTime()/3600;
			if (nLastChgHour != nCurHour)
			{
				// 需要载入了,但是老的先用着
				_QueryRankFromDB(nType);
			}
			else if (nType == rank_type_char_level)
			{
				g_pLPNet->NotifyMeRefreshLevelRankOfPlayerCard(true);
			}
			int	nNum	= pFile->FileSize()/sizeof(RankData);
			
			rankdata_vector_T*	pVect	= _GetRankDataVector(nType);
			onlyid2rank_map_T*	pMap	= _GetOnlyID2RankMap(nType);
			
			if (pVect!=NULL && pMap!=NULL)
			{
				pVect->clear();
				pVect->resize(nNum);

				pMap->clear();

				if (nNum > 0)
				{
					pFile->Read(pVect->getbuf(), nNum*sizeof(RankData));

					for (int j=0; j<nNum; j++)
					{
						RankData* pRankData		= pVect->getptr(j);
						pMap->put(pRankData->nOnlyID, j+1);
					}
				}
			}

			whfile_CloseCmnFile(pFile);
		}
	}

	return 0;
}
const char*	CRankMngS::_MakeRankFileName(unsigned int nRankType)
{
	static	char	szBuf[WH_MAX_PATH];
	int		nLength		= sprintf(szBuf, "%s%d", m_pHost->GetCfgInfo()->szRankFileDir, nRankType);
	szBuf[nLength]		= 0;

	return	szBuf;
}
CRankMngS::onlyid2rank_map_T*	CRankMngS::_GetOnlyID2RankMap(unsigned int nType, bool bCreateIfNotExist/*=true*/)
{
	onlyid2rank_map_T*	pMap	= NULL;
	map<unsigned int, onlyid2rank_map_T*>::iterator	it	= m_mapRankType2RankMap.find(nType);
	if (it != m_mapRankType2RankMap.end())
	{
		pMap	= it->second;
	}
	else if (bCreateIfNotExist)
	{
		pMap	= new onlyid2rank_map_T();
		m_mapRankType2RankMap.insert(map<unsigned int, onlyid2rank_map_T*>::value_type(nType, pMap));
	}

	return pMap;
}
CRankMngS::rankdata_vector_T*	CRankMngS::_GetRankDataVector(unsigned int nType, bool bCreateIfNotExist/*=true*/)
{
	rankdata_vector_T*	pVect	= NULL;
	map<unsigned int, rankdata_vector_T*>::iterator	it	= m_mapRankDataVect.find(nType);
	if (it != m_mapRankDataVect.end())
	{
		pVect	= it->second;
	}
	else if (bCreateIfNotExist)
	{
		pVect	= new rankdata_vector_T();
		m_mapRankDataVect.insert(map<unsigned int, rankdata_vector_T*>::value_type(nType, pVect));
	}

	return pVect;
}
int		CRankMngS::_QueryRankFromDB(unsigned int nRankType)
{
	P_DBS4WEB_LOAD_RANK_LIST_T	Cmd;
	Cmd.nCmd		= P_DBS4WEB_REQ_CMD;
	Cmd.nSubCmd		= CMDID_LOAD_RANK_LIST_REQ;
	Cmd.nType		= nRankType;

	g_pLPNet->SendCmdToDB(&Cmd, sizeof(Cmd));

	if (nRankType == rank_type_char_level)
	{
		g_pLPNet->NotifyMeRefreshLevelRankOfPlayerCard(false);
	}
	return 0;
}
