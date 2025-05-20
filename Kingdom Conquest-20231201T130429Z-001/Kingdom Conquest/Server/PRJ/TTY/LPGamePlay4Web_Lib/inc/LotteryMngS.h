#ifndef __LotteryMngS_H__
#define __LotteryMngS_H__

#include "WHCMN/inc/whvector.h"
#include "GameUtilityLib/inc/SoloFileFunc.h"
#include "../../Common/inc/tty_common_excel_def.h"
#include "../../Common/inc/tty_common_PlayerData4Web.h"

using namespace n_whcmn;

namespace n_pngs
{
class CGameMngS;
class CLotteryMngS
{
public:
	int		Init(CGameMngS* pHost);
	int		DrawOnceLottery();
	int		DrawOnceLotteryAlliance();
	inline const whvector<LotteryEntry>&	GetCurLotteryEntries()
	{
		return m_vectCurLotteryEntries;
	}
	inline int	GetCurChoice()
	{
		return m_nChoice;
	}
	int		ReloadExcelTable();
private:
	void	_GenRandomIndex();
	int		_RealDrawLottery();
	void	_PrintDetailInfo(int nRandom);
	int		_LoadExcelTable();
private:
	whvector<int>							m_vectProbabilities;
	whvector<LotteryEntry>					m_vectLotteryEntries[lottery_probability_max_choice];
	whvector<LotteryEntry>					m_vectLotteryEntriesAlliance[lottery_probability_max_choice];
	whvector<LotteryEntry>*					m_pCurVectLotteryEntriesArray;
	whvector<int>							m_vectTmpRandomIndex;
	whvector<LotteryEntry>					m_vectCurLotteryEntries;
	int										m_nChoice;
};
}
#endif
