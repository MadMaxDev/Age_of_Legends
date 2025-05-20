#include "../inc/CLS4Web_i.h"

using namespace n_pngs;

int		CLS4Web_i::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	return 0;
}

int		CLS4Web_i::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch (nCmd)
	{
	//////////////////////////////////////////////////////////////////////////
	// ¹«¹²µÄ
	//////////////////////////////////////////////////////////////////////////
	case PNGSPACKET_2LOGIC_SETMOOD:
		{
			m_nMood		= CMN::ILogic::MOOD_STOPPED;
		}
		break;
	}
	return 0;
}
