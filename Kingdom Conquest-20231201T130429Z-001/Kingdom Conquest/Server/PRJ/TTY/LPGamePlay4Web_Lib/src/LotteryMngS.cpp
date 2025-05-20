#include "../inc/LotteryMngS.h"

using namespace n_pngs;

int		CLotteryMngS::Init(CGameMngS* pHost)
{
	_LoadExcelTable();

	m_vectTmpRandomIndex.resize(lottery_probability_max_choice);
	m_vectCurLotteryEntries.resize(lottery_probability_max_choice);
	return 0;
}
int		CLotteryMngS::ReloadExcelTable()
{
	// 1.清理已经读入的数据
	m_vectProbabilities.clear();
	for (int i=0; i<lottery_probability_max_choice; i++)
	{
		m_vectLotteryEntries[i].clear();
		m_vectLotteryEntriesAlliance[i].clear();
	}

	// 2.重新载入表格
	g_ExcelTableMng.ReloadTable(table_id_lottery_probability);
	g_ExcelTableMng.ReloadTable(table_id_lottery_content);
	g_ExcelTableMng.ReloadTable(table_id_lottery_content_alliance);
	
	// 3.重新导入数据
	return _LoadExcelTable();
}
int		CLotteryMngS::_LoadExcelTable()
{
	// 1.载入概率配置
	CExcelTable*	pTableProbability	= g_ExcelTableMng.GetTable(table_id_lottery_probability);
	if (pTableProbability == NULL)
	{
		return -1;
	}
	m_vectProbabilities.resize(lottery_probability_max_choice);
	for (int i=0; i<m_vectProbabilities.size(); i++)
	{
		m_vectProbabilities[i]		= 0;
	}
	for (int i=0; i<pTableProbability->GetCount(); i++)
	{
		SExcelLine*	pLine			= pTableProbability->GetLineFromList(i);
		if (pLine != NULL)
		{
			int		nChoice			= pLine->SafeGetInt(lp_col_choice, true);
			int		nProbability	= pLine->SafeGetInt(lp_col_probability, true);
			m_vectProbabilities[nChoice-1]	= nProbability;
		}
	}

	// 2.载入抽奖内容配置
	CExcelTable*	pTableContent	= g_ExcelTableMng.GetTable(table_id_lottery_content);
	if (pTableContent == NULL)
	{
		return -2;
	}
	for (int i=0; i<pTableContent->GetCount(); i++)
	{
		SExcelLine*	pLine			= pTableContent->GetLineFromList(i);
		if (pLine != NULL)
		{
			LotteryEntry	entry;
			entry.nType				= pLine->SafeGetInt(lc_col_type, true);
			entry.nData				= pLine->SafeGetInt(lc_col_data, true);
			entry.bTopReward		= (bool)pLine->SafeGetInt(lc_col_is_top_reward);
			int		nChoice			= pLine->SafeGetInt(lc_col_probability_choice, true);
			m_vectLotteryEntries[nChoice-1].push_back(entry);
		}
	}

	// 3.载入联盟抽奖内容配置
	CExcelTable*	pTableContentAlliance	= g_ExcelTableMng.GetTable(table_id_lottery_content_alliance);
	if (pTableContentAlliance == NULL)
	{
		return -3;
	}
	for (int i=0; i<pTableContentAlliance->GetCount(); i++)
	{
		SExcelLine*	pLine			= pTableContentAlliance->GetLineFromList(i);
		if (pLine != NULL)
		{
			LotteryEntry	entry;
			entry.nType				= pLine->SafeGetInt(lc_col_type, true);
			entry.nData				= pLine->SafeGetInt(lc_col_data, true);
			entry.bTopReward		= (bool)pLine->SafeGetInt(lc_col_is_top_reward);
			int		nChoice			= pLine->SafeGetInt(lc_col_probability_choice, true);
			m_vectLotteryEntriesAlliance[nChoice-1].push_back(entry);
		}
	}

	return 0;
}
int		CLotteryMngS::DrawOnceLottery()
{
	m_pCurVectLotteryEntriesArray	= m_vectLotteryEntries;
	return _RealDrawLottery();
}
int		CLotteryMngS::DrawOnceLotteryAlliance()
{
	m_pCurVectLotteryEntriesArray	= m_vectLotteryEntriesAlliance;
	return _RealDrawLottery();
}
int		CLotteryMngS::_RealDrawLottery()
{
	_GenRandomIndex();
	int	nRandom			= rand()%lottery_probability_base;
	int	nCurProbability	= 0;
	m_nChoice			= 0;
	// 生成抽中选项
	for (int i=0; i<lottery_probability_max_choice; i++)
	{
		nCurProbability	+= m_vectProbabilities[m_vectTmpRandomIndex[i]-1];
		if (nCurProbability>nRandom)
		{
			m_nChoice	= i;
			break;
		}
	}
	// 生成所有的奖励项
	for (int i=0; i<lottery_probability_max_choice; i++)
	{
		whvector<LotteryEntry>&	tmpLotteryVect	= m_pCurVectLotteryEntriesArray[m_vectTmpRandomIndex[i]-1];
		int	nRandomIdx	= rand()%tmpLotteryVect.size();
		m_vectCurLotteryEntries[i]				= tmpLotteryVect[nRandomIdx];
	}
	//_PrintDetailInfo(nRandom);
	return 0;
}
void	CLotteryMngS::_GenRandomIndex()
{
	for (int i=0; i<lottery_probability_max_choice; i++)
	{
		m_vectTmpRandomIndex[i]	= i+1;
	}
	for (int i=0; i<lottery_probability_max_choice/2; i++)
	{
		int	i1	= rand()%lottery_probability_max_choice;
		int	i2	= rand()%lottery_probability_max_choice;
		int	nTmp					= m_vectTmpRandomIndex[i1];
		m_vectTmpRandomIndex[i1]	= m_vectTmpRandomIndex[i2];
		m_vectTmpRandomIndex[i2]	= nTmp;
	}
}
void	CLotteryMngS::_PrintDetailInfo(int nRandom)
{
	int	nTotalProbability			= 0;
	for (int i=0; i<lottery_probability_max_choice; i++)
	{
		int	nRealIndex				= m_vectTmpRandomIndex[i];
		int	nProbability			= m_vectProbabilities[nRealIndex-1];
		nTotalProbability			+= nProbability;
		printf("index:%d,real_index:%d,probability:%d,total_probability:%d,type:%d,data:%d\n"
			, i, nRealIndex, nProbability, nTotalProbability, m_vectCurLotteryEntries[i].nType, m_vectCurLotteryEntries[i].nData);
	}
	printf("random:%d,choice:%d\n", nRandom, m_nChoice);
}
