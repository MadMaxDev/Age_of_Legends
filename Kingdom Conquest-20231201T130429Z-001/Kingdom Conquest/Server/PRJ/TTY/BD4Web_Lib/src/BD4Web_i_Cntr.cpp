#include "../inc/BD4Web_i.h"

using namespace n_pngs;

Connecter*	BD4Web_i::NewConnecter(SOCKET sock, sockaddr_in* pAddr)
{
	if (m_nMood != MOOD_WORKING)	// 停止状态的时候就不能接受任何连接
	{
		return NULL;
	}

	MyCntrSvr::MyCntr*	pCntr	= new MyCntrSvr::MyCntr(this);
	assert(pCntr);
	// 初始化
	MyCntrSvr::MyCntr::INFO_T	info;
	memcpy(&info.msgerINFO, m_MSGER_INFO.GetBase(), sizeof(info.msgerINFO));
	info.msgerINFO.sock			= sock;
	int	nRst	= pCntr->Init(&info);
	if (nRst < 0)
	{
		WHSafeDelete(pCntr);
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BD4Web_i_RT)"%s,pCntr->Init,%d,%s", __FUNCTION__, nRst, cmn_get_ipnportstr_by_saaddr(pAddr));
	}
	else
	{
		memcpy(&pCntr->m_addr, pAddr, sizeof(pCntr->m_addr));
	}
	return pCntr;
}
void	BD4Web_i::AfterAddConnecter(Connecter* pCntr)
{
	MyCntrSvr::MyCntr*	pMyCntr	= (MyCntrSvr::MyCntr*)pCntr->QueryInterface();
	if (pMyCntr == NULL)
	{
		return;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,BD4Web_i_RT)"%s,new connecter,0x%X,%s", __FUNCTION__, pMyCntr->GetIDInMan(), cmn_get_ipnportstr_by_saaddr(&pMyCntr->m_addr));
}
void	BD4Web_i::BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T* pCntrInfo)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,BD4Web_i_RT)"%s,delete connecter,0x%X", __FUNCTION__, nCntrID);
}
int		BD4Web_i::RemoveMyCntr(int nCntrID)
{
	return RemoveMyCntr((MyCntrSvr::MyCntr*)m_CntrSvr.GetConnecterPtr(nCntrID));
}
int		BD4Web_i::RemoveMyCntr(MyCntrSvr::MyCntr* pCntr)
{
	if (pCntr == NULL)
	{
		return -1;
	}
	WHSafeDelete(pCntr);		// 使用自动关闭套接字,这里会进行关闭
	return 0;
}
int		BD4Web_i::SendCmdToConnecter(int nCntrID, const void* pData, size_t nSize)
{
	SendCmdToConnecter((MyCntrSvr::MyCntr*)m_CntrSvr.GetConnecterPtr(nCntrID), pData, nSize);
	return 0;
}
int		BD4Web_i::SendCmdToConnecter(MyCntrSvr::MyCntr* pCntr, const void* pData, size_t nSize)
{
	if (pCntr == NULL)
	{
		return -1;
	}
	return pCntr->SendMsg(pData, nSize);
}
