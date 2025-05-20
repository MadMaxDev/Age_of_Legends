#ifndef __LotteryMngS_H__
#define __LotteryMngS_H__

#include "WHCMN/inc/whvector.h"
#include "GameUtilityLib/inc/SoloFileFunc.h"
#include "../../../TTY/Common/inc/tty_common_excel_def.h"
#include "../../../TTY/Common/inc/tty_common_PlayerData4Web.h"

using namespace n_whcmn;

namespace n_pngs
{
class CGameMngS;
class CLotteryMngS
{
public:
	int		Init();
	int		DrawOnceLottery();
	inline const whvector<LotteryEntry>&	GetCurLotteryEntries()
	{
		return m_vectCurLotteryEntries;
	}
	inline int	GetCurChoice()
	{
		return m_nChoice;
	}
private:
	void	_GenRandomIndex();
	void	_PrintDetailInfo(int nRandom);
private:
	whvector<int>							m_vectProbabilities;
	whvector<LotteryEntry>					m_vectLotteryEntries[lottery_probability_max_choice];
	whvector<int>							m_vectTmpRandomIndex;
	whvector<LotteryEntry>					m_vectCurLotteryEntries;
	int										m_nChoice;
};
}
#endif
