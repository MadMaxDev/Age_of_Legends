#include "../inc/WorldMngS.h"

using namespace n_pngs;

static unsigned int	_GetRandInRange(const char* cszRange)
{
	unsigned int	nBegin			= 0;
	unsigned int	nEnd			= 0;
	wh_strsplit("dd", cszRange, "*", &nBegin, &nEnd);
	if (nBegin>=nEnd)
	{
		return nBegin;
	}
	return nBegin + rand()%(nEnd-nBegin);
}
static unsigned int	_GetRandInRange(unsigned int nBegin, unsigned int nEnd)
{
	if (nBegin>=nEnd)
	{
		return nBegin;
	}
	return nBegin + rand()%(nEnd-nBegin);
}

CWorldMngS::CWorldMngS()
: m_pHost(NULL)
, m_pArrTiles(NULL)
{

}
CWorldMngS::~CWorldMngS()
{
	for (int i=0; i<m_pHost->GetCfgInfo()->nWorldXMax; i++)
	{
		delete[] m_pArrTiles[i];
		m_pArrTiles[i]	= NULL;
	}

	delete[] m_pArrTiles;
	m_pArrTiles			= NULL;
}
int		CWorldMngS::Init(CGameMngS* pHost)
{
	m_pHost				= pHost;
	m_pArrTiles			= new TileUnit*[pHost->GetCfgInfo()->nWorldXMax];
	for (int i=0; i<m_pHost->GetCfgInfo()->nWorldXMax; i++)
	{
		m_pArrTiles[i]	= new TileUnit[pHost->GetCfgInfo()->nWorldYMax];
	}
	
	// 读取出生点配置
	return _LoadExcelTable();
}
int		CWorldMngS::ReloadExcelTable()
{
	// 1.清理已经读入的数据
	m_vectPlayerBirthPlaceCfg.clear();
	m_vectWorldFamousCityCfg.clear();
	m_vectWorldResCfg.clear();
	m_vectSoldierCfg.clear();

	// 2.重新载入表格
	g_ExcelTableMng.ReloadTable(table_id_player_birthplace);
	g_ExcelTableMng.ReloadTable(table_id_world_famous_city);
	g_ExcelTableMng.ReloadTable(table_id_world_resource);
	g_ExcelTableMng.ReloadTable(table_id_soldier);

	// 3.重新导入数据
	return _LoadExcelTable();
}
int		CWorldMngS::_LoadExcelTable()
{
	CExcelTable*	pTable	= g_ExcelTableMng.GetTable(table_id_player_birthplace);
	if (pTable == NULL)
	{
		return -1;
	}
	for (int i=0; i<pTable->GetCount(); i++)
	{
		SExcelLine*	pLine	= pTable->GetLineFromList(i);
		ExcelPlayerBirthPlace*	pUnit	= m_vectPlayerBirthPlaceCfg.push_back();
		pUnit->nPlayerNum	= pLine->SafeGetInt(pbp_col_playernum);
		pUnit->nBeginX		= pLine->SafeGetInt(pbp_col_begin_x);
		pUnit->nBeginY		= pLine->SafeGetInt(pbp_col_begin_y);
		pUnit->nEndX		= pLine->SafeGetInt(pbp_col_end_x);
		pUnit->nEndY		= pLine->SafeGetInt(pbp_col_end_y);
	}

	pTable					= g_ExcelTableMng.GetTable(table_id_world_famous_city);
	if (pTable == NULL)
	{
		return -2;
	}
	for (int i=0; i<pTable->GetCount(); i++)
	{
		SExcelLine*	pLine	= pTable->GetLineFromList(i);
		ExcelWorldFamousCity*	pUnit	= m_vectWorldFamousCityCfg.push_back();
		pUnit->nType		= pLine->SafeGetInt(wfc_col_type);
		pUnit->nNum			= pLine->SafeGetInt(wfc_col_num);
		pUnit->nBeginX		= pLine->SafeGetInt(wfc_col_begin_x);
		pUnit->nBeginY		= pLine->SafeGetInt(wfc_col_begin_y);
		pUnit->nEndX		= pLine->SafeGetInt(wfc_col_end_x);
		pUnit->nEndY		= pLine->SafeGetInt(wfc_col_end_y);
	}

	pTable					= g_ExcelTableMng.GetTable(table_id_world_resource);
	if (pTable == NULL)
	{
		return -3;
	}
	for (int i=0; i<pTable->GetCount(); i++)
	{
		SExcelLine*	pLine	= pTable->GetLineFromList(i);
		ExcelWorldRes*	pUnit	= m_vectWorldResCfg.push_back();
		pUnit->nType		= pLine->SafeGetInt(wr_col_type);
		pUnit->nLevel		= pLine->SafeGetInt(wr_col_level);
		pUnit->nRefreshNum	= pLine->SafeGetInt(wr_col_refresh_num);
		pUnit->nTotalNum	= pLine->SafeGetInt(wr_col_total_num);
		pUnit->nBeginX		= pLine->SafeGetInt(wr_col_begin_x);
		pUnit->nBeginY		= pLine->SafeGetInt(wr_col_begin_y);
		pUnit->nEndX		= pLine->SafeGetInt(wr_col_end_x);
		pUnit->nEndY		= pLine->SafeGetInt(wr_col_end_y);
		WH_STRNCPY0(pUnit->szArmyData, pLine->FastGetStr(wr_col_army_data, true));
		WH_STRNCPY0(pUnit->szGoldLoot, pLine->FastGetStr(wr_col_gold_loot, true));
		WH_STRNCPY0(pUnit->szPopLoot, pLine->FastGetStr(wr_col_pop_loot, true));
		WH_STRNCPY0(pUnit->szCrystalLoot, pLine->FastGetStr(wr_col_crystal_loot, true));
	}

	pTable					= g_ExcelTableMng.GetTable(table_id_soldier);
	if (pTable == NULL)
	{
		return -4;
	}
	for (int i=0; i<pTable->GetCount(); i++)
	{
		SExcelLine*	pLine	= pTable->GetLineFromList(i);
		ExcelSoldier*	pUnit	= m_vectSoldierCfg.push_back();
		pUnit->nType		= pLine->SafeGetInt(s_col_type);
		pUnit->nLevel		= pLine->SafeGetInt(s_col_level);
		pUnit->nATK			= pLine->SafeGetInt(s_col_atk);
		pUnit->nDEF			= pLine->SafeGetInt(s_col_def);
		pUnit->nHP			= pLine->SafeGetInt(s_col_hp);
	}
	return 0;
}
int		CWorldMngS::LoadTerrainInfo(TerrainUnit* pUnits, int nNum)
{
	for (int i=0; i<nNum; i++)
	{
		TileUnit*	pTileUnit	= GetTile(pUnits[i].nPosX, pUnits[i].nPosY);
		if (pTileUnit == NULL)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,LP_RT)"TERRAIN ERROR,INVALID POSITION,%u,%u", pUnits[i].nPosX, pUnits[i].nPosY);
			continue;
		}
		pTileUnit->nType			= TileUnit::TILETYPE_TERRAIN;
		pTileUnit->terrainInfo		= pUnits[i].info;
	}
	return 0;
}
int		CWorldMngS::RoundFindAvailablePos(bool bNeedBook, int nMaxWidth, int nSrcX, int nSrcY, int nBeginX , int nBeginY, int nEndX, int nEndY, unsigned int& nPosX, unsigned int& nPosY)
{
	int	nMinX	= 0;
	int	nMinY	= 0;
	int	nMaxX	= 0;
	int	nMaxY	= 0;
	for (int nWidth=1; nWidth<=nMaxWidth; nWidth++)
	{
		nMinX	= (nSrcX-nWidth)>nBeginX?nSrcX-nWidth:nBeginX;
		nMaxX	= (nSrcX+nWidth)<nEndX?nSrcX+nWidth:nEndX;
		nMinY	= (nSrcY-nWidth)>nBeginY?nSrcY-nWidth:nBeginY;
		nMaxY	= (nSrcY+nWidth)<nEndY?nSrcY+nWidth:nEndY;

		// 1.
		for (int m=nMinY+1; m<nMaxY; m++)
		{
			if (CanBuildCity(nMinX, m))
			{
				nPosX		= nMinX;
				nPosY		= m;
				if (bNeedBook)
				{
					BookPosition(nMinX, m);
					return 0;
				}
			}
			if (CanBuildCity(nMaxX, m))
			{
				nPosX		= nMaxX;
				nPosY		= m;
				if (bNeedBook)
				{
					BookPosition(nMaxX, m);
					return 0;
				}
			}
		}

		// 2.
		for (int m=nMinX; m<=nMaxX; m++)
		{
			if (CanBuildCity(m, nMinY))
			{
				nPosX		= m;
				nPosY		= nMinY;
				if (bNeedBook)
				{
					BookPosition(m, nMinY);
					return 0;
				}
			}
			if (CanBuildCity(m, nMaxY))
			{
				nPosX		= m;
				nPosY		= nMaxY;
				if (bNeedBook)
				{
					BookPosition(m, nMaxY);
					return 0;
				}
			}
		}
	}

	return -1;
}
int		CWorldMngS::GetAvailablePos(unsigned int nCountry, unsigned int& nPosX, unsigned int& nPosY, bool bNeedBook/* = false*/)
{
// 	// 先简单化,后续需要通过读取MB表来确定位置生成规则
// 	for (int i=0; i<m_pHost->GetCfgInfo()->nWorldXMax; i++)
// 	{
// 		for (int j=0; j<m_pHost->GetCfgInfo()->nWorldYMax; j++)
// 		{
// 			if (CanBuildCity(i, j))
// 			{
// 				nPosX	= i;
// 				nPosY	= j;
// 				if (bNeedBook)
// 				{
// 					BookPosition(i, j);
// 				}
// 				return 0;
// 			}
// 		}
// 	}
	int		nPlayerNum		= m_pHost->GetPlayerCardMng()->GetTotalPlayerNum();
global_find_beign:
	int		nBeginX		= 0;
	int		nBeginY		= 0;
	int		nEndX		= 0;
	int		nEndY		= 0;
	int		nField		= -1;
	int		_nPosX		= 0;
	int		_nPosY		= 0;

	if (m_vectPlayerBirthPlaceCfg.size()<=1 || nPlayerNum < m_vectPlayerBirthPlaceCfg[1].nPlayerNum)
	{
		nField			= 0;

		ExcelPlayerBirthPlace&	unit	= m_vectPlayerBirthPlaceCfg[0];
		nBeginX			= unit.nBeginX;
		nBeginY			= unit.nBeginY;
		nEndX			= unit.nEndX;
		nEndY			= unit.nEndY;

		_nPosX			= _GetRandInRange(nBeginX, nEndX);
		_nPosY			= _GetRandInRange(nBeginY, nEndY);

		if (CanBuildCity(_nPosX, _nPosY))
		{
			nPosX		= _nPosX;
			nPosY		= _nPosY;
			if (bNeedBook)
			{
				BookPosition(nPosX, nPosY);
			}
			return 0;
		}
		else
		{
			if (RandFindPosition(true, _nPosX, _nPosY, nBeginX, nBeginY, nEndX, nEndY, nPosX, nPosY) == 0)
			{
				return 0;
			}
			// 在附近找可用位置(应该是向四周扩散搜索可用位置)
			int	nMaxWidth	= _GetMaxWidth(_nPosX, _nPosY, nBeginX, nBeginY, nEndX, nEndY);
			if (RoundFindAvailablePos(bNeedBook, nMaxWidth, _nPosX, _nPosY, nBeginX, nBeginY, nEndX, nEndY, nPosX, nPosY) == 0)
			{
				return 0;
			}
			else
			{
				if (m_vectPlayerBirthPlaceCfg.size()<=1)
				{
					return -100;
				}
				nPlayerNum	= m_vectPlayerBirthPlaceCfg[1].nPlayerNum;
				goto global_find_beign;
			}
		}
	}
	else
	{
		ExcelPlayerBirthPlace&	preUnit	= m_vectPlayerBirthPlaceCfg[0];
		int		nPreBeginX		= preUnit.nBeginX;
		int		nPreBeginY		= preUnit.nBeginY;
		int		nPreEndX		= preUnit.nEndX;
		int		nPreEndY		= preUnit.nEndY;

		for (int i=1; i<m_vectPlayerBirthPlaceCfg.size(); i++)
		{
			ExcelPlayerBirthPlace&	unit	= m_vectPlayerBirthPlaceCfg[i];
			if (nPlayerNum < unit.nPlayerNum)
			{
				nBeginX			= unit.nBeginX;
				nBeginY			= unit.nBeginY;
				nEndX			= unit.nEndX;
				nEndY			= unit.nEndY;
				nField			= i-1;
				break;
			}

			if (i != (m_vectPlayerBirthPlaceCfg.size()-1))
			{
				nPreBeginX		= unit.nBeginX;
				nPreBeginY		= unit.nBeginY;
				nPreEndX		= unit.nEndX;
				nPreEndY		= unit.nEndY;
			}
			else
			{
				nBeginX			= unit.nBeginX;
				nBeginY			= unit.nBeginY;
				nEndX			= unit.nEndX;
				nEndY			= unit.nEndY;
				nField			= i;
			}
		}

		// 可用区间被分为4个field
		// field0 [(nBeginX,nBeginY), (nPreBeginX,nEndY))
		// field1 [(nPreBeginX,nBeginY), (nPreEndX,nPreBeginY))
		// field2 [(nPreBeginX,nPreEndY), (nPreEndX,nEndY))
		// field3 [(nPreEndX,nBeginY), (nEndX,nEndY))
		int		_nBeginX		= 0;
		int		_nBeginY		= 0;
		int		_nEndX			= 0;
		int		_nEndY			= 0;
		int		_nLocalField	= 0;

		int		_nSquare0		= (nPreBeginX-nBeginX)*(nEndY-nBeginY);
		int		_nSquare1		= (nPreEndX-nPreBeginX)*(nPreBeginY-nBeginY);
		int		_nSquare2		= (nPreEndX-nPreBeginX)*(nEndY-nPreEndY);
		int		_nSquare3		= (nEndX-nPreEndX)*(nEndY-nBeginY);

		int		nFieldPercent	= rand()%(_nSquare0+_nSquare1+_nSquare2+_nSquare3);
		if (nFieldPercent < _nSquare0)
		{
			_nLocalField		= 0;
		}
		else if (nFieldPercent < (_nSquare0+_nSquare1))
		{
			_nLocalField		= 1;
		}
		else if (nFieldPercent < (_nSquare0+_nSquare1+_nSquare2))
		{
			_nLocalField		= 2;
		}
		else
		{
			_nLocalField		= 3;
		}
		int		_nLocalFindNum	= 1;
local_find_begin:
		switch (_nLocalField)
		{
		case 0:
			{
				_nBeginX		= nBeginX;
				_nBeginY		= nBeginY;
				_nEndX			= nPreBeginX;
				_nEndY			= nEndY;
			}
			break;
		case 1:
			{
				_nBeginX		= nPreBeginX;
				_nBeginY		= nBeginY;
				_nEndX			= nPreEndX;
				_nEndY			= nPreBeginY;
			}
			break;
		case 2:
			{
				_nBeginX		= nPreBeginX;
				_nBeginY		= nPreEndY;
				_nEndX			= nPreEndX;
				_nEndY			= nEndY;
			}
			break;
		default:
			{
				_nBeginX		= nPreEndX;
				_nBeginY		= nBeginY;
				_nEndX			= nEndX;
				_nEndY			= nEndY;
			}
			break;
		}

		_nPosX			= _GetRandInRange(_nBeginX, _nEndX);
		_nPosY			= _GetRandInRange(_nBeginY, _nEndY);

		if (CanBuildCity(_nPosX, _nPosY))
		{
			nPosX		= _nPosX;
			nPosY		= _nPosY;
			if (bNeedBook)
			{
				BookPosition(nPosX, nPosY);
			}
			return 0;
		}
		else
		{
			if (RandFindPosition(true, _nPosX, _nPosY, _nBeginX, _nBeginY, _nEndX, _nEndY, nPosX, nPosY) == 0)
			{
				return 0;
			}

			// 在附近找可用位置(应该是向四周扩散搜索可用位置)
			int	nMaxWidth	= _GetMaxWidth(_nPosX, _nPosY, _nBeginX, _nBeginY, _nEndX, _nEndY);
			if (RoundFindAvailablePos(bNeedBook, nMaxWidth, _nPosX, _nPosY, _nBeginX, _nBeginY, _nEndX, _nEndY, nPosX, nPosY) == 0)
			{
				return 0;
			}
			else
			{
				_nLocalField	= (_nLocalField+1)%4;
				if (_nLocalFindNum > 4)
				{
					if (nField == (m_vectPlayerBirthPlaceCfg.size()-1))
					{
						return -1;	// 最外层也分配不了位置了
					}
					else
					{
						// 去更外一层找
						nPlayerNum	= m_vectPlayerBirthPlaceCfg[nField+1].nPlayerNum;
					}
					goto global_find_beign;
				}
				else
				{
					_nLocalFindNum++;
					goto local_find_begin;
				}
			}
		}
	}

	return -1;
}
int		CWorldMngS::SetTileType(unsigned int nPosX, unsigned int nPosY, unsigned int nType, tty_id_t nAccountID/* =0 */)
{
	TileUnit*	pTileUnit	= GetTile(nPosX, nPosY);
	if (pTileUnit == NULL)
	{
		return -1;
	}
	pTileUnit->nAccountID		= nAccountID;
	if (nAccountID!=0 || (nType==TileUnit::TILETYPE_NONE && pTileUnit->terrainInfo.IsTerrain()))
	{
		pTileUnit->nType		= TileUnit::TILETYPE_TERRAIN;
	}
	else
	{
		pTileUnit->nType		= nType;
	}
	return 0;
}
bool	CWorldMngS::CanMovePosition(unsigned int nSrcPosX, unsigned int nSrcPosY, unsigned int nDstPosX, unsigned int nDstPosY)
{
	TileUnit*	pSrcTile	= GetTile(nSrcPosX, nSrcPosY);
	TileUnit*	pDstTile	= GetTile(nDstPosX, nDstPosY);
	if (pSrcTile==NULL || pDstTile==NULL)
	{
		return false;
	}
	return CanBuildCity(nDstPosX, nDstPosY);
}
int		CWorldMngS::BookPosition(unsigned int nPosX, unsigned int nPosY)
{
	TileUnit*	pUnit		= GetTile(nPosX, nPosY);
	if (pUnit == NULL)
	{
		return -1;
	}
	pUnit->bBooked			= true;
	return 0;
}
int		CWorldMngS::UnbookPosition(unsigned int nPosX, unsigned int nPosY)
{
	TileUnit*	pUnit		= GetTile(nPosX, nPosY);
	if (pUnit == NULL)
	{
		return -1;
	}
	pUnit->bBooked			= false;
	return 0;
}
int		CWorldMngS::MovePosition(unsigned int nSrcPosX, unsigned int nSrcPosY, unsigned int nDstPosX, unsigned int nDstPosY)
{
	TileUnit*	pSrcTile	= GetTile(nSrcPosX, nSrcPosY);
	TileUnit*	pDstTile	= GetTile(nDstPosX, nDstPosY);
	if (pSrcTile==NULL || pDstTile==NULL)
	{
		return -1;
	}
	pDstTile->nAccountID	= pSrcTile->nAccountID;
	pDstTile->nType			= TileUnit::TILETYPE_TERRAIN;
	pDstTile->bBooked		= false;
	pSrcTile->nAccountID	= 0;
	if (!pSrcTile->terrainInfo.IsTerrain())
	{
		pSrcTile->nType		= TileUnit::TILETYPE_NONE;
	}
	return 0;
}
TileUnit*	CWorldMngS::GetTile(unsigned int nPosX, unsigned int nPosY)
{
	if (IsValidPosition(nPosX, nPosY))
	{
		return &m_pArrTiles[nPosX][nPosY];
	}
	return NULL;
}
bool CWorldMngS::IsValidPosition(unsigned int nPosX, unsigned int nPosY)
{
	if (nPosX>=(unsigned int)(m_pHost->GetCfgInfo()->nWorldXMax) || nPosY>=(unsigned int)(m_pHost->GetCfgInfo()->nWorldYMax))
	{
		return false;
	}
	return true;
}
int		CWorldMngS::RandFindPosition(bool bNeedBook, int nSrcX, int nSrcY, int nBeginX , int nBeginY, int nEndX, int nEndY, unsigned int& nPosX, unsigned int& nPosY)
{
	whvector<int>	vectRandDir;
	for (int i=0; i<4; i++)
	{
		vectRandDir.push_back(i);
	}
	for (int i=0; i<2; i++)
	{
		int	nTmp1	= rand()%4;
		int	nTmp2	= rand()%4;
		int	nTmp	= vectRandDir[nTmp1];
		vectRandDir[nTmp1]	= vectRandDir[nTmp2];
		vectRandDir[nTmp2]	= nTmp;
	}
	whvector<Position_T>	vectAvailablePos;
	for (int i=0; i<4; i++)
	{
		vectAvailablePos.clear();
		switch (vectRandDir[i])
		{
		case 0:
			{
				for (int j=nBeginX; j<nSrcX; j++)
				{
					if (CanBuildCity(j, nSrcY))
					{
						vectAvailablePos.push_back(Position_T(j, nSrcY));
					}
				}
			}
			break;
		case 1:
			{
				for (int j=nSrcX+1; j<nEndX; j++)
				{
					if (CanBuildCity(j, nSrcY))
					{
						vectAvailablePos.push_back(Position_T(j, nSrcY));
					}
				}
			}
			break;
		case 2:
			{
				for (int j=nBeginY; j<nSrcY; j++)
				{
					if (CanBuildCity(nSrcX, j))
					{
						vectAvailablePos.push_back(Position_T(nSrcX, j));
					}
				}
			}
			break;
		case 3:
			{
				for (int j=nSrcY+1; j<nEndY; j++)
				{
					if (CanBuildCity(nSrcX, j))
					{
						vectAvailablePos.push_back(Position_T(nSrcX, j));
					}
				}
			}
			break;
		}
		if (vectAvailablePos.size() > 0)
		{
			int	nIndex	= rand()%vectAvailablePos.size();
			nPosX		= vectAvailablePos[nIndex].nPosX;
			nPosY		= vectAvailablePos[nIndex].nPosY;
			if (bNeedBook)
			{
				BookPosition(nPosX, nPosY);
			}

			return 0;
		}
	}
	// 没有找到合适的位置
	return -1;
}
static void	_ExchangeWorldCityHelper(unsigned int nPosX, unsigned int nPosY, TileUnit*	pTile, WorldFamousCity*	pCity, unsigned int nType)
{
	pCity->bCanBuild				= pTile->terrainInfo.bCanBuild;
	pCity->nFloor2					= pTile->terrainInfo.nFloor2;
	pCity->nPosX					= nPosX;
	pCity->nPosY					= nPosY;
	pCity->nType					= nType;
	pCity->nSrcTerrainType			= pTile->nType;

	pTile->terrainInfo.bCanBuild	= false;
	pTile->terrainInfo.nFloor2		= wfc_floor2_begin + nType;
	pTile->nType					= TileUnit::TILETYPE_TERRAIN;
}
int		CWorldMngS::GenWorldFamousCity(bool bClearFirst)
{
	// 
	if (bClearFirst)
	{
		ClearWorldFamousCityAll();
	}

	//
	m_vectWorldFamousCityTmp.clear();
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_CITY)"GenAll,%d", m_vectWorldFamousCity.size());

	for (int i=0; i<m_vectWorldFamousCityCfg.size(); i++)
	{
		ExcelWorldFamousCity&	unit	= m_vectWorldFamousCityCfg[i];
		int						nCurNum	= _GetWorldFamousCityNum(unit.nType);
		int						nNum	= unit.nNum>nCurNum?unit.nNum-nCurNum:0;
		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_CITY)"GenType,%d/%d,%d,%d", nNum, nCurNum, unit.nNum, unit.nType);
		for (int j=0; j<nNum; j++)
		{
			unsigned int	nSrcPosX	= _GetRandInRange(unit.nBeginX, unit.nEndX);
			unsigned int	nSrcPosY	= _GetRandInRange(unit.nBeginY, unit.nEndY);
			if (CanBuildCity(nSrcPosX, nSrcPosY))
			{
				TileUnit*	pTile				= GetTile(nSrcPosX, nSrcPosY);
				WorldFamousCity*	pCity		= m_vectWorldFamousCityTmp.push_back();
				_ExchangeWorldCityHelper(nSrcPosX, nSrcPosY, pTile, pCity, unit.nType);
			}
			else
			{
				unsigned int	nPosX			= 0;
				unsigned int	nPosY			= 0;
				if (RandFindPosition(false, nSrcPosX, nSrcPosY, unit.nBeginX, unit.nBeginY, unit.nEndX, unit.nEndY, nPosX, nPosY) == 0)
				{
					TileUnit*	pTile				= GetTile(nPosX, nPosY);
					WorldFamousCity*	pCity		= m_vectWorldFamousCityTmp.push_back();
					_ExchangeWorldCityHelper(nPosX, nPosY, pTile, pCity, unit.nType);
				}
				else
				{
					// 在附近找可用位置(应该是向四周扩散搜索可用位置)
					int	nMaxWidth	= _GetMaxWidth(nPosX, nPosY, unit.nBeginX, unit.nBeginY, unit.nEndX, unit.nEndY);
					if (RoundFindAvailablePos(false, nMaxWidth, nSrcPosX, nSrcPosY, unit.nBeginX, unit.nBeginY, unit.nEndX, unit.nEndY, nPosX, nPosY) == 0)
					{
						TileUnit*	pTile				= GetTile(nPosX, nPosY);
						WorldFamousCity*	pCity		= m_vectWorldFamousCityTmp.push_back();
						_ExchangeWorldCityHelper(nPosX, nPosY, pTile, pCity, unit.nType);
					}
				}
			}
		}
	}

	if (m_vectWorldFamousCityTmp.size() > 0)
	{
		m_vectWorldFamousCity.pushn_back(m_vectWorldFamousCityTmp.getbuf(), m_vectWorldFamousCityTmp.size());
		TellDBGenWorldFamousCity(bClearFirst);

		for (int i=0; i<m_vectWorldFamousCityTmp.size(); i++)
		{
			const WorldFamousCity&	city	= m_vectWorldFamousCityTmp[i];
			GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_CITY)"GenOne,%u,%u,%d", city.nPosX, city.nPosY, city.nType);
		}
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_CITY)"GenAllEnd,%d/%d", m_vectWorldFamousCityTmp.size(), m_vectWorldFamousCity.size());
	return 0;
}
int		CWorldMngS::ClearWorldFamousCity(unsigned int nPosX, unsigned int nPosY)
{
	WorldFamousCity*	pCity	= NULL;
	for (int i=0; i<m_vectWorldFamousCity.size(); i++)
	{
		if (m_vectWorldFamousCity[i].nPosX==nPosX && m_vectWorldFamousCity[i].nPosY==nPosY)
		{
			pCity				= &m_vectWorldFamousCity[i];
			break;
		}
	}
	if (pCity == NULL)
	{
		return -1; // 这个位置没有名城
	}
	TileUnit*	pTile			= GetTile(nPosX, nPosY);
	if (pTile == NULL)
	{
		return -2;
	}
	pTile->terrainInfo.bCanBuild	= pCity->bCanBuild;
	pTile->terrainInfo.nFloor2		= pCity->nFloor2;
	pTile->nType					= pCity->nSrcTerrainType;
	m_vectWorldFamousCity.erasevalue(*pCity);
	return 0;
}
int		CWorldMngS::ClearWorldFamousCityAll()
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_CITY)"ClearAll,%d", m_vectWorldFamousCity.size());
	while (m_vectWorldFamousCity.size() > 0)
	{
		ClearWorldFamousCity(m_vectWorldFamousCity[0].nPosX, m_vectWorldFamousCity[0].nPosY);
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_CITY)"ClearAllEnd,%d", m_vectWorldFamousCity.size());
	return 0;
}
int		CWorldMngS::TellDBGenWorldFamousCity(bool bClearFirst)
{
	whvector<char>	vectTmp;
	vectTmp.resize(sizeof(P_DBS4WEB_GEN_WORLD_FAMOUS_CITY_T) + m_vectWorldFamousCityTmp.size()*sizeof(WorldFamousCity));
	P_DBS4WEB_GEN_WORLD_FAMOUS_CITY_T*	pReqCmd	= (P_DBS4WEB_GEN_WORLD_FAMOUS_CITY_T*)vectTmp.getbuf();
	pReqCmd->nCmd								= P_DBS4WEB_REQ_CMD;
	pReqCmd->nSubCmd							= CMDID_GEN_WORLD_FAMOUS_CITY_REQ;
	pReqCmd->bClearFirst						= bClearFirst;
	pReqCmd->nNum								= m_vectWorldFamousCityTmp.size();
	memcpy(wh_getptrnexttoptr(pReqCmd), m_vectWorldFamousCityTmp.getbuf(), m_vectWorldFamousCityTmp.size()*sizeof(WorldFamousCity));
	g_pLPNet->SendCmdToDB(vectTmp.getbuf(), vectTmp.size());
	return 0;
}
int		CWorldMngS::AddWorldFamousCity(WorldFamousCity* pCity)
{
	TileUnit*	pTile	= GetTile(pCity->nPosX, pCity->nPosY);
	if (pTile == NULL)
	{
		return -1;
	}
	WorldFamousCity*	pNewCity	= m_vectWorldFamousCity.push_back();
	memcpy(pNewCity, pCity, sizeof(WorldFamousCity));
	pTile->terrainInfo.bCanBuild	= false;
	pTile->terrainInfo.nFloor2		= wfc_floor2_begin + pCity->nType;
	pTile->nType					= TileUnit::TILETYPE_TERRAIN;
	return 0;
}

void	CWorldMngS::_ExchangeWorldResHelper(unsigned int nPosX, unsigned int nPosY, TileUnit* pTile, WorldRes* pRes, ExcelWorldRes* pUnit)
{
	pRes->nID						= _GetWorldResID(nPosX, nPosY);
	pRes->bCanBuild					= pTile->terrainInfo.bCanBuild;
	pRes->nFloor2					= pTile->terrainInfo.nFloor2;
	pRes->nPosX						= nPosX;
	pRes->nPosY						= nPosY;
	pRes->nType						= pUnit->nType;
	pRes->nLevel					= pUnit->nLevel;
	pRes->nSrcTerrainType			= pTile->nType;
	pRes->nGold						= _GetRandInRange(pUnit->szGoldLoot);
	pRes->nPop						= _GetRandInRange(pUnit->szPopLoot);
	pRes->nCrystal					= _GetRandInRange(pUnit->szCrystalLoot);
	pRes->nForce					= 0;
	WH_STRNCPY0(pRes->szArmyData, pUnit->szArmyData);
	int	nLength						= sprintf(pRes->szArmyDeploy, "%d*%d*%d*%d*%d"
										, _GetRandInRange(1,5), _GetRandInRange(1,5), _GetRandInRange(1,5), _GetRandInRange(1,5), _GetRandInRange(1,5));
	pRes->szArmyDeploy[nLength]		= 0;

	pTile->terrainInfo.bCanBuild	= false;
	pTile->terrainInfo.nFloor2		= wr_floor2_begin + pUnit->nType;
	pTile->nType					= TileUnit::TILETYPE_TERRAIN;

	// 计算战力
	unsigned int	nLevel			= 0;
	unsigned int	nNum			= 0;
	wh_strsplit("dd", pRes->szArmyData, "*", &nLevel, &nNum);
	if (nLevel==0 || nNum==0)
	{
		return;
	}
	if (m_vectSoldierCfg[nLevel-1].nLevel == nLevel)
	{
		const ExcelSoldier&	unit	= m_vectSoldierCfg[nLevel-1];
		pRes->nForce				= (unit.nATK+unit.nDEF*0.9+unit.nHP*0.8)*nNum*5/3;
	}
	else
	{
		for (int i=0; i<m_vectSoldierCfg.size(); i++)
		{
			const ExcelSoldier&	unit	= m_vectSoldierCfg[i];
			if (unit.nLevel == nLevel)
			{
				pRes->nForce			= (unit.nATK+unit.nDEF*0.9+unit.nHP*0.8)*nNum*5/3;
				break;
			}
		}
	}
}
int		CWorldMngS::GenWorldRes()
{
	m_vectWorldResTmp.clear();
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_RES)"GenAll,%d", m_mapWorldRes.size());

	for (int i=0; i<m_vectWorldResCfg.size(); i++)
	{
		ExcelWorldRes&	unit	= m_vectWorldResCfg[i];
		int				nCurNum	= _GetWorldResNum(unit.nType, unit.nLevel);
		int				nNum	= nCurNum<unit.nTotalNum?unit.nTotalNum-nCurNum:0;
		nNum					= nNum>unit.nRefreshNum?unit.nRefreshNum:nNum;
		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_RES)"GenTypeLevel,%d/%d,%d,%d,%d,%d"
			, nNum, nCurNum, unit.nRefreshNum, unit.nTotalNum, unit.nType, unit.nLevel);
		for (int j=0; j<nNum; j++)
		{
			unsigned int	nSrcPosX	= _GetRandInRange(unit.nBeginX, unit.nEndX);
			unsigned int	nSrcPosY	= _GetRandInRange(unit.nBeginY, unit.nEndY);
			if (CanBuildCity(nSrcPosX, nSrcPosY))
			{
				TileUnit*	pTile		= GetTile(nSrcPosX, nSrcPosY);
				WorldRes*	pRes		= m_vectWorldResTmp.push_back();
				_ExchangeWorldResHelper(nSrcPosX, nSrcPosY, pTile, pRes, &unit);
			}
			else
			{
				unsigned int	nPosX			= 0;
				unsigned int	nPosY			= 0;
				if (RandFindPosition(false, nSrcPosX, nSrcPosY, unit.nBeginX, unit.nBeginY, unit.nEndX, unit.nEndY, nPosX, nPosY) == 0)
				{
					TileUnit*	pTile			= GetTile(nPosX, nPosY);
					WorldRes*	pRes			= m_vectWorldResTmp.push_back();
					_ExchangeWorldResHelper(nPosX, nPosY, pTile, pRes, &unit);
				}
				else
				{
					// 在附近找可用位置(应该是向四周扩散搜索可用位置)
					int	nMaxWidth	= _GetMaxWidth(nPosX, nPosY, unit.nBeginX, unit.nBeginY, unit.nEndX, unit.nEndY);
					if (RoundFindAvailablePos(false, nMaxWidth, nSrcPosX, nSrcPosY, unit.nBeginX, unit.nBeginY, unit.nEndX, unit.nEndY, nPosX, nPosY) == 0)
					{
						TileUnit*	pTile			= GetTile(nPosX, nPosY);
						WorldRes*	pRes			= m_vectWorldResTmp.push_back();
						_ExchangeWorldResHelper(nPosX, nPosY, pTile, pRes, &unit);
					}
				}
			}
		}
	}

	if (m_vectWorldResTmp.size() > 0)
	{
		for (int i=0; i<m_vectWorldResTmp.size(); i++)
		{
			const WorldRes&	res	= m_vectWorldResTmp[i];
			if (m_mapWorldRes.put(res.nID, res))
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_RES)"GenOne,%u,%u,%d", res.nPosX, res.nPosY, res.nType);
			}
		}
		TellDBGenWorldRes();
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_RES)"GenAllEnd,%d/%d", m_vectWorldResTmp.size(), m_mapWorldRes.size());
	return 0;
}
int		CWorldMngS::TellDBGenWorldRes()
{
	whvector<char>	vectTmp;
	vectTmp.resize(sizeof(P_DBS4WEB_GEN_WORLD_RES_T) + m_vectWorldResTmp.size()*sizeof(WorldRes));
	P_DBS4WEB_GEN_WORLD_RES_T*	pReqCmd			= (P_DBS4WEB_GEN_WORLD_RES_T*)vectTmp.getbuf();
	pReqCmd->nCmd								= P_DBS4WEB_REQ_CMD;
	pReqCmd->nSubCmd							= CMDID_GEN_WORLD_RES_REQ;
	pReqCmd->nNum								= m_vectWorldResTmp.size();
	memcpy(wh_getptrnexttoptr(pReqCmd), m_vectWorldResTmp.getbuf(), m_vectWorldResTmp.size()*sizeof(WorldRes));
	g_pLPNet->SendCmdToDB(vectTmp.getbuf(), vectTmp.size());
	return 0;
}
int		CWorldMngS::ClearWorldResAll()
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_RES)"ClearAll,%d", m_mapWorldRes.size());
	while (m_mapWorldRes.size() > 0)
	{
		whhash<tty_id_t,WorldRes>::kv_iterator	it	= m_mapWorldRes.begin();
		const WorldRes&	res		= it.getvalue();
		ClearWorldRes(res.nPosX, res.nPosY);
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,WORLD_RES)"ClearAllEnd,%d", m_mapWorldRes.size());
	return 0;
}
int		CWorldMngS::ClearWorldRes(unsigned int nPosX, unsigned int nPosY)
{
	tty_id_t	nID		= _GetWorldResID(nPosX, nPosY);
	WorldRes*	pRes	= m_mapWorldRes.getptr(nID);
	if (pRes == NULL)
	{
		return -1;
	}
	TileUnit*	pTile	= GetTile(nPosX, nPosY);
	if (pTile == NULL)
	{
		return -2;
	}
	pTile->terrainInfo.bCanBuild	= pRes->bCanBuild;
	pTile->terrainInfo.nFloor2		= pRes->nFloor2;
	pTile->nType					= pRes->nSrcTerrainType;
	m_mapWorldRes.erase(nID);
	return 0;
}
int		CWorldMngS::AddWorldRes(WorldRes* pRes)
{
	TileUnit*	pTile				= GetTile(pRes->nPosX, pRes->nPosY);
	if (pTile == NULL)
	{
		return -1;
	}
	m_mapWorldRes.put(pRes->nID, *pRes);
	pTile->terrainInfo.bCanBuild	= false;
	pTile->terrainInfo.nFloor2		= wr_floor2_begin + pRes->nType;
	pTile->nType					= TileUnit::TILETYPE_TERRAIN;
	return 0;
}
