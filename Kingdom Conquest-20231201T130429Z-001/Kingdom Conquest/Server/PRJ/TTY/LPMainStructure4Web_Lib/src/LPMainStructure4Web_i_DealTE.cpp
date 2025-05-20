#include "../inc/LPMainStructure4Web_i.h"

using namespace n_pngs;

int		LPMainStructure4Web_i::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// ����ʱ��������Ƿ��ж���
	whtimequeue::ID_T	id;
	TQUNIT_T*			pUnit	= NULL;
	while (m_TQ.GetUnitBeforeTime(m_tickNow, (void**)&pUnit, &id) == 0)
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);
		m_TQ.Del(id);
	}

	return 0;
}
void	LPMainStructure4Web_i::TEDeal_CalcAndSend_CAAFS4Web_AvailPlayerNum(TQUNIT_T* pTQUnit)
{
	for (int i=0; i<LP_MAX_CAAFS4Web_NUM; i++)
	{
		CAAFS4WebGroup*	pGroup	= m_aCAAFS4WebGroup+i;
		if (pGroup->nCntrID == 0)
		{
			// û���ϵĲ��ù�
			continue;
		}
		int		nCount			= 0;
		whDList<CLS4WebUnit*>::node*	pNode	= pGroup->dlCLS4Web.begin();
		for (; pNode!=pGroup->dlCLS4Web.end(); pNode=pNode->next)
		{
			CLS4WebUnit*	pCLS4Web			= pNode->data;
			if (pCLS4Web->CanAcceptPlayer())
			{
				nCount	+= pCLS4Web->GetAcceptablePlayerNum();
			}
		}
		// ����֪ͨ��CAAFS4Web
		LP_CAAFS4Web_LPINFO_T	LPInfo;
		LPInfo.nCmd					= LP_CAAFS4Web_LPINFO;
		if (nCount > m_cfginfo.nMaxPlayer)
		{
			int	nCurNum		= m_cfginfo.nMaxPlayer-m_Players.size();
			if (nCurNum > 0)
			{
				LPInfo.nLPAvailPlayer	= nCurNum;
			}
			else
			{
				LPInfo.nLPAvailPlayer	= 0;
			}
		}
		else
		{
			LPInfo.nLPAvailPlayer	= nCount;
		}
		if (LPInfo.nLPAvailPlayer > m_Players.availsize())
		{
			LPInfo.nLPAvailPlayer	= m_Players.availsize();
		}
		if (LPInfo.nLPAvailPlayer > m_cfginfo.nSafeReservePlayerNum)
		{
			LPInfo.nLPAvailPlayer	-= m_cfginfo.nSafeReservePlayerNum;
		}
		else
		{
			LPInfo.nLPAvailPlayer	= 0;
		}
		SendCmdToConnecter(pGroup->nCntrID,  &LPInfo, sizeof(LPInfo));
	}
	if (m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCalcAndSendCAAFS4WebAvailPlayerTimeOut, (void**)&pTQUnit, &m_teid_CalcAndSend_CAAFS4Web_AvailPlayerNum) < 0)
	{
		assert(0);
		return;
	}
	pTQUnit->tefunc	= &LPMainStructure4Web_i::TEDeal_CalcAndSend_CAAFS4Web_AvailPlayerNum;
}
void	LPMainStructure4Web_i::TEDeal_Hello_TimeOut(TQUNIT_T* pTQUnit)
{
	// ��ָ��ʱ����û���յ��Է�������hi��
	int	nCntrID	= pTQUnit->un.svr.nID;
	if (m_mapCntrID2AppInfo.find(nCntrID) == m_mapCntrID2AppInfo.end())
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1387,LPMS_RT)"%s,0x%X", __FUNCTION__, nCntrID);
		// ���HI�ɹ������m_mapCntrID2AppInfo���ҵ�������Ϣ
		RemoveConnecter(nCntrID);
	}
}
void	LPMainStructure4Web_i::SetTE_Hello_TimeOut(int nCntrID)
{
	TQUNIT_T*			pTQUnit;
	whtimequeue::ID_T	teid;
	int	nRst	= m_TQ.AddGetRef(m_tickNow+m_cfginfo.nHelloTimeOut, (void**)&pTQUnit, &teid);
	if (nRst < 0)
	{
		RemoveConnecter(nCntrID);
		// ��¼��־
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(29,PNGS_INT_RLS)"%s,%d,m_TQ.AddGetRef,Cntr Removed", __FUNCTION__, nRst);
		return;
	}
	pTQUnit->tefunc		= &LPMainStructure4Web_i::TEDeal_Hello_TimeOut;
	pTQUnit->un.svr.nID	= nCntrID;
}
void	LPMainStructure4Web_i::SetTE_CLS4Web_DropWait_TimeOut(CLS4WebUnit* pCLS4Web)
{
	TQUNIT_T*			pTQUnit;
	whtimequeue::ID_T	teid;
	int	nRst	= m_TQ.AddGetRef(m_tickNow+m_cfginfo.nSvrDropWaitTimeOut, (void**)&pTQUnit, &teid);
	if (nRst < 0)
	{
		// ����ע���ֱ��KO��
		DealCLS4WebDisconnection(pCLS4Web);
		// ��¼��־
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(29,PNGS_INT_RLS)"%s,%d,m_TQ.AddGetRef,DealCLS4WebDisconnection", __FUNCTION__, nRst);
		return;
	}
	pTQUnit->tefunc		= &LPMainStructure4Web_i::TEDeal_CLS4Web_DropWait_TimeOut;
	pTQUnit->un.ptr		= pCLS4Web;		// �����ӵ�ָ���ǲ���ʧЧ��
}
void	LPMainStructure4Web_i::TEDeal_CLS4Web_DropWait_TimeOut(TQUNIT_T* pTQUnit)
{
	CLS4WebUnit*	pCLS4WebUnit	= (CLS4WebUnit*)pTQUnit->un.ptr;
	if (pCLS4WebUnit->nCntrID == 0)
	{
		// ���ʱ����û����������
		DealCLS4WebDisconnection(pCLS4WebUnit);
	}
}
