#include "../inc/LotteryMngS.h"

using namespace n_pngs;

int		CLotteryMngS::Init()
{
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
			int		nChoice			= pLine->SafeGetInt(lc_col_probability_choice, true);
			m_vectLotteryEntries[nChoice-1].push_back(entry);
		}
	}
	m_vectTmpRandomIndex.resize(lottery_probability_max_choice);
	m_vectCurLotteryEntries.resize(lottery_probability_max_choice);
	return 0;
}
int		CLotteryMngS::DrawOnceLottery()
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
		whvector<LotteryEntry>&	tmpLotteryVect	= m_vectLotteryEntries[m_vectTmpRandomIndex[i]-1];
		int	nRandomIdx	= rand()%tmpLotteryVect.size();
		m_vectCurLotteryEntries[i]				= tmpLotteryVect[nRandomIdx];
	}
	_PrintDetailInfo(nRandom);
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
	printf("random:%d,index(choice):%d,real_index:%d\n", nRandom, m_nChoice, m_vectTmpRandomIndex[m_nChoice]);
}
