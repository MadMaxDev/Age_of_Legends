#ifndef __WorldMngS_H__
#define __WorldMngS_H__

#include "GameMngS.h"

#include "WHCMN/inc/whvector.h"
#include "../../Common/inc/tty_common_excel_def.h"

using namespace n_whcmn;

namespace n_pngs
{
class CGameMngS;
enum
{
	wfc_floor2_begin		= 100,		// ��������Floor2������ʼֵ
	wr_floor2_begin			= 200,		// ������ԴFloor2������ʼֵ
};
struct TileUnit
{
	enum
	{
		TILETYPE_NONE		= 0,		// ʲô��û��
		TILETYPE_TERRAIN	= 1,		// ����
	};
	unsigned int	nType;
	tty_id_t		nAccountID;			
	TerrainInfo		terrainInfo;
	bool			bBooked;			// �Ƿ�Ԥ��(�����ƶ��ǳع�����)

	TileUnit()
	: nType(TILETYPE_NONE)
	, nAccountID(0)
	, bBooked(false)
	{
		
	}
};
class CWorldMngS
{
public:
	CWorldMngS();
	~CWorldMngS();
public:
	int				Init(CGameMngS* pHost);
	// ���������Ϣ
	int				LoadTerrainInfo(TerrainUnit* pUnits, int nNum);
	// Ŀǰ���ǹ���tile
	int				GetAvailablePos(unsigned int nCountry, unsigned int& nPosX, unsigned int& nPosY, bool bNeedBook = false);
	int				SetTileType(unsigned int nPosX, unsigned int nPosY, unsigned int nType, tty_id_t nAccountID=0);
	bool			CanMovePosition(unsigned int nSrcPosX, unsigned int nSrcPosY, unsigned int nDstPosX, unsigned int nDstPosY);
	int				BookPosition(unsigned int nPosX, unsigned int nPosY);
	int				UnbookPosition(unsigned int nPosX, unsigned int nPosY);
	int				MovePosition(unsigned int nSrcPosX, unsigned int nSrcPosY, unsigned int nDstPosX, unsigned int nDstPosY);
	TileUnit*		GetTile(unsigned int nPosX, unsigned int nPosY);
	inline TileUnit**		GetAllTile()
	{
		return m_pArrTiles;
	}
	inline bool CanBuildCity(unsigned int nPosX, unsigned int nPosY)
	{
		return m_pArrTiles[nPosX][nPosY].nType==TileUnit::TILETYPE_NONE
				|| (m_pArrTiles[nPosX][nPosY].nType==TileUnit::TILETYPE_TERRAIN && !m_pArrTiles[nPosX][nPosY].bBooked && m_pArrTiles[nPosX][nPosY].terrainInfo.bCanBuild && m_pArrTiles[nPosX][nPosY].nAccountID==0);
	}
	bool IsValidPosition(unsigned int nPosX, unsigned int nPosY);
	inline bool IsBooked(unsigned int nPosX, unsigned int nPosY)
	{
		return m_pArrTiles[nPosX][nPosY].bBooked;
	}
	inline bool	IsPlayer(unsigned int nPosX, unsigned int nPosY)
	{
		return m_pArrTiles[nPosX][nPosY].nAccountID!=0;
	}
	int		ReloadExcelTable();
	int		GenWorldFamousCity(bool bClearFirst);
	int		ClearWorldFamousCityAll();
	int		ClearWorldFamousCity(unsigned int nPosX, unsigned int nPosY);
	int		TellDBGenWorldFamousCity(bool bClearFirst);
	int		AddWorldFamousCity(WorldFamousCity* pCity);
	inline const whvector<WorldFamousCity>&	GetWorldFamousCity() const
	{
		return m_vectWorldFamousCity;
	}

	int		GenWorldRes();
	int		ClearWorldResAll();
	int		ClearWorldRes(unsigned int nPosX, unsigned int nPosY);
	int		TellDBGenWorldRes();
	int		AddWorldRes(WorldRes* pRes);
	inline const WorldRes*	GetWorldRes(unsigned int nPosX, unsigned int nPosY)
	{
		tty_id_t	nID		= _GetWorldResID(nPosX, nPosY);
		return m_mapWorldRes.getptr(nID);
	}
	inline const WorldRes*	GetWorldRes(tty_id_t nID)
	{
		return m_mapWorldRes.getptr(nID);
	}
private:
	inline int _GetMaxWidth(int nSrcX, int nSrcY, int nBeginX, int nBeginY, int nEndX, int nEndY)
	{
		int		nMaxWidth1	= (nSrcX-nBeginX)>(nEndX-nSrcX)?(nSrcX-nBeginX):(nEndX-nSrcX);
		int		nMaxWidth2	= (nSrcY-nBeginY)>(nEndY-nSrcY)?(nSrcY-nBeginY):(nEndY-nSrcY);
		return nMaxWidth1>nMaxWidth2?nMaxWidth1:nMaxWidth2;
	}
	// ѭ����������Ŀ���λ��
	int		RoundFindAvailablePos(bool bNeedBook, int nMaxWidth, int nSrcX, int nSrcY, int nBeginX , int nBeginY, int nEndX, int nEndY, unsigned int& nPosX, unsigned int& nPosY);
	// ���Ѱ��һ��λ��
	int		RandFindPosition(bool bNeedBook, int nSrcX, int nSrcY, int nBeginX , int nBeginY, int nEndX, int nEndY, unsigned int& nPosX, unsigned int& nPosY);
	int		_LoadExcelTable();
	int		_GetWorldFamousCityNum(unsigned int nType)
	{
		int	nNum	= 0;
		for (int i=0; i<m_vectWorldFamousCity.size(); i++)
		{
			if (m_vectWorldFamousCity[i].nType == nType)
			{
				nNum++;
			}
		}
		return nNum;
	}
	inline tty_id_t	_GetWorldResID(unsigned int nPosX, unsigned int nPosY)
	{
		return ((tty_id_t)nPosX<<32) | nPosY;
	}
	int		_GetWorldResNum(unsigned int nType, unsigned int nLevel)
	{
		int	nNum	= 0;
		whhash<tty_id_t,WorldRes>::kv_iterator	it	= m_mapWorldRes.begin();
		for (; it!=m_mapWorldRes.end(); ++it)
		{
			const WorldRes&	unit	= it.getvalue();
			if (unit.nType==nType && unit.nLevel==nLevel)
			{
				nNum++;
			}
		}
		return nNum;
	}
	void	_ExchangeWorldResHelper(unsigned int nPosX, unsigned int nPosY, TileUnit* pTile, WorldRes* pRes, ExcelWorldRes* pUnit);
private:
	CGameMngS*		m_pHost;
	TileUnit**		m_pArrTiles;			// Tile����
	whvector<ExcelPlayerBirthPlace>	m_vectPlayerBirthPlaceCfg;
	whvector<ExcelWorldFamousCity>	m_vectWorldFamousCityCfg;
	whvector<WorldFamousCity>		m_vectWorldFamousCity;
	whvector<WorldFamousCity>		m_vectWorldFamousCityTmp;

	whvector<ExcelWorldRes>			m_vectWorldResCfg;
	whhash<tty_id_t,WorldRes>		m_mapWorldRes;
	whvector<WorldRes>				m_vectWorldResTmp;

	whvector<ExcelSoldier>			m_vectSoldierCfg;
};
}

#endif
