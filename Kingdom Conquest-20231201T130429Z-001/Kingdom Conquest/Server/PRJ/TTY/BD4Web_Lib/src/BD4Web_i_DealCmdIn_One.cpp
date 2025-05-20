#include "../inc/BD4Web_i.h"

using namespace n_pngs;

int		BD4Web_i::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	return 0;
}
int		BD4Web_i::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch (nCmd)
	{
	case PNGSPACKET_2LOGIC_SETMOOD:
		{
			m_nMood		= CMN::ILogic::MOOD_STOPPED;
		}
		break;
	case PNGSPACKET_BD4WEB_CMD_RPL:
		{
			DealCmdIn_One_BD4WEB_CMD_RPL((PNGSPACKET_BD4WEB_CMD_RPL_T*)pData, nDSize);
		}
		break;
	}
	return 0;
}
int		BD4Web_i::DealCmdIn_One_BD4WEB_CMD_RPL(PNGSPACKET_BD4WEB_CMD_RPL_T* pCmd, size_t nSize)
{
//	const BD_CMD_BASE_T*	pBaseCmd	= pCmd->pData;
	SendCmdToConnecter(pCmd->nExt[0], pCmd->pData, pCmd->nDSize);
	return 0;
}
