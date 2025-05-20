#include "../inc/BD4Web_i.h"

using namespace n_pngs;

bool	BD4Web_i::Cntr_CanDealMsg() const
{
	return true;
}
int		BD4Web_i::Cntr_DealMsg(MyCntrSvr::MyCntr* pCntr, const BD_CMD_BASE_T* pBaseCmd, size_t nSize)
{
	if (!m_cfginfo.bBDSupport)
	{
		BD_CMD_BASE_T	RplCmd;
		RplCmd.nCmd		= BD_CMD_BD_NOT_OPEN_RPL;
		RplCmd.nSize	= sizeof(RplCmd);
		SendCmdToConnecter(pCntr, &RplCmd, sizeof(RplCmd));
		return -1;
	}

	PNGSPACKET_BD4WEB_CMD_REQ_T		Cmd;
	memset(Cmd.nExt, 0, sizeof(Cmd.nExt));
	Cmd.pData						= pBaseCmd;
	Cmd.nDSize						= nSize;
	Cmd.nExt[0]						= pCntr->GetIDInMan();
	Cmd.nExt[1]						= m_cfginfo.bNBO;
	CmdOutToLogic_AUTO(m_pLogicGS4Web, PNGSPACKET_BD4WEB_CMD_REQ, (const void*)&Cmd, sizeof(Cmd));
	return 0;
}
