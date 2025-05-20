#include "../inc/GSDB4Web_i.h"

using namespace n_pngs;

int		GSDB4Web_i::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch (nCmd)
	{
	case	PNGSPACKET_2LOGIC_DISPHISTORY:
		{
			//暂不提供
		}
		break;
	case PNGSPACKET_2DB4WEB_CONFIG:
		{
			return CmdIn_PNGSPACKET_2DB_CONFIG(pRstAccepter, (PNGSPACKET_2DB4WEB_CONFIG_T*)pData, nDSize);
		}
		break;
	default:
		{
			assert(0);
		}
		break;
	}
	return 0;
}

int		GSDB4Web_i::CmdIn_PNGSPACKET_2DB_CONFIG(ILogicBase *pRstAccepter, PNGSPACKET_2DB4WEB_CONFIG_T *pCmd, int nSize)
{
	switch (pCmd->nSubCmd)
	{
	case PNGSPACKET_2DB4WEB_CONFIG_T::SUBCMD_SET_DBUSER:
		{
			if (m_pLogicDBUser)
			{
				assert(0);
				return -1;
			}
			m_pLogicDBUser			= pRstAccepter;
		}
		break;
	case PNGSPACKET_2DB4WEB_CONFIG_T::SUBCMD_SET_DBUSERISWHAT:
		{
			m_nAppType				= pCmd->nParam;
			return ConnectToDBS4Web();
		}
		break;
	default:
		{

		}
		break;
	}
	return 0;
}

int		GSDB4Web_i::DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
{
	switch (nCmd)
	{
	//////////////////////////////////////////////////////////////////////////
	//公共的
	//////////////////////////////////////////////////////////////////////////
	case PNGSPACKET_2LOGIC_GMCMD:
		{
			//GM字串指令处理(用于调试,测试存活等等),预留
			return 0;
		}
		break;
	case PNGSPACKET_2LOGIC_SETMOOD:
		{
			//!!!后续开发要确定在存盘完结之后才能关闭
			m_nMood		= CMN::ILogic::MOOD_STOPPED;
		}
		break;
	case PNGSPACKET_2DB4WEB_REQ:
		{
			m_retrymsgerDBS4Web.SendMsg(pData, nDSize);
		}
		break;
	default:
		{
			return 0;
		}
		break;
	}
	return 0;
}

int		GSDB4Web_i::DealCmdFromDBS4Web(const void* pData, size_t nSize)
{
	CmdOutToLogic_AUTO(m_pLogicDBUser, PNGSPACKET_2DB4WEBUSER_RPL, pData, nSize);
	return 0;
}
