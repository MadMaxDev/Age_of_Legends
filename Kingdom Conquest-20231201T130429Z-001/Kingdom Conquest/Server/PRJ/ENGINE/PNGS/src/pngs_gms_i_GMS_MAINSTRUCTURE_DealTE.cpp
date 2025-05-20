// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_DealTE.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ����ʱ�䴦���ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-23
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::Tick_DealTE()
{
	m_tickNow	= wh_gettickcount();
	// ����ʱ��������Ƿ��ж���
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_TQ.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// ���������������ʱ���¼�����������������ŵġ�
		// ���get������ֻ��ͨ��Delɾ���������Լ�ɾ
		m_TQ.Del(id);
	}
	return	0;
}

void	GMS_MAINSTRUCTURE::TEDeal_Hello_TimeOut(TQUNIT_T *pTQUnit)
{
	// ��ָ��ʱ��û���յ��Է�������Hello��
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(561,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::TEDeal_Hello_TimeOut,0x%X", pTQUnit->un.svr.nID);
	// ɾ����Ӧ��Connecter
	RemoveMYCNTR(pTQUnit->un.svr.nID);
}
void	GMS_MAINSTRUCTURE::SetTE_Hello_TimeOut(Connecter * pCntr)
{
	MYCNTRSVR::MYCNTR	*pMyCntr	= (MYCNTRSVR::MYCNTR *)pCntr->QueryInterface();
	TQUNIT_T	*pTQUnit;
	int	rst		= m_TQ.AddGetRef(m_tickNow+m_cfginfo.nHelloTimeOut, (void **)&pTQUnit, &pMyCntr->teid);
	if( rst<0 )
	{
		// ��ô���޷�����ʱ���¼�����
		// �Ƴ��û�
		RemoveMYCNTR(pMyCntr);
		// ��¼��־
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(29,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::SetTE_Hello_TimeOut,%d,m_TQ.AddGetRef,Cntr Removed", rst);
		return;
	}
	pTQUnit->tefunc		= &GMS_MAINSTRUCTURE::TEDeal_Hello_TimeOut;
	pTQUnit->un.svr.nID	= pMyCntr->GetIDInMan();
}
void	GMS_MAINSTRUCTURE::TEDeal_CalcAndSend_CAAFS_AvailPlayerNum(TQUNIT_T *pTQUnit)
{
	for(int i=0;i<GMS_MAX_CAAFS_NUM;i++)
	{
		CAAFSGroup	*pGroup	= m_pSHMData->m_aCAAFSGroup + i;
		if( !pGroup->pCntr )
		{
			// û��CAAFS�Ͳ��ü���ͳ����
			continue;
		}
		int			nCount	= 0;
		for(whDList<CLSUnit *>::node *pNode=pGroup->dlCLS.begin(); pNode!=pGroup->dlCLS.end(); pNode=pNode->next)
		{
			CLSUnit	*pCLS	= pNode->data;
			if( pCLS->CanAcceptPlayer() )
			{
				nCount	+= pCLS->GetAcceptablePlayerNum();
			}
		}
		// ����֪ͨ��CAAFS
		GMS_CAAFS_GMSINFO_T		GMSInfo;
		GMSInfo.nCmd			= GMS_CAAFS_GMSINFO;
		if( nCount>m_nGMSLogicMaxPlayer )
		{
			int	nCurNum			= m_nGMSLogicMaxPlayer - m_pSHMData->m_Players.size();
			if( nCurNum > 0 )
			{
				GMSInfo.nGMSAvailPlayer	= nCurNum;
			}
			else
			{
				GMSInfo.nGMSAvailPlayer	= 0;
			}
		}
		else
		{
			GMSInfo.nGMSAvailPlayer	= nCount;
		}
		if( GMSInfo.nGMSAvailPlayer>m_pSHMData->m_Players.getsizeleft() )
		{
			GMSInfo.nGMSAvailPlayer	= m_pSHMData->m_Players.getsizeleft();
		}
		if( GMSInfo.nGMSAvailPlayer>m_cfginfo.nSafeReservePlayerNum )
		{
			GMSInfo.nGMSAvailPlayer	-= m_cfginfo.nSafeReservePlayerNum;
		}
		else
		{
			GMSInfo.nGMSAvailPlayer	= 0;
		}
		pGroup->pCntr->SendMsg(&GMSInfo, sizeof(GMSInfo));
	}
	// �����µ�ʱ���¼�
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nCalcAndSendCAAFSAvailPlayerTimeOut, (void **)&pTQUnit, &m_teid_CalcAndSend_CAAFS_AvailPlayerNum)<0 )
	{
		// �����������ʱ����޷�����ʱ���¼�
		assert(0);
		return;
	}
	pTQUnit->tefunc		= &GMS_MAINSTRUCTURE::TEDeal_CalcAndSend_CAAFS_AvailPlayerNum;
}
void	GMS_MAINSTRUCTURE::TEDeal_GZS_DropWait_TimeOut(TQUNIT_T *pTQUnit)
{
	GZSUnit	*pGZS	= m_pSHMData->m_GZSs + pTQUnit->un.gzs.nIdx;
	if( pGZS->nSvrIdx>0 )
	{
		DealGZSDisconnection(pGZS);
	}
}
void	GMS_MAINSTRUCTURE::SetTE_GZS_DropWait_TimeOut(GZSUnit *pGZS)
{
	TQUNIT_T	*pTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nSvrDropWaitTimeOut, (void **)&pTQUnit, &pGZS->teid)<0 )
	{
		// �����������ʱ����޷�����ʱ���¼�
		assert(0);
		return;
	}
	pTQUnit->tefunc			= &GMS_MAINSTRUCTURE::TEDeal_GZS_DropWait_TimeOut;
	pTQUnit->un.gzs.nIdx	= pGZS->nSvrIdx;
}
void	GMS_MAINSTRUCTURE::TEDeal_CLS_DropWait_TimeOut(TQUNIT_T *pTQUnit)
{
	CLSUnit	*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pTQUnit->un.svr.nID);
	if( pCLS )
	{
		DealCLSDisconnection(pCLS);
	}
}
void	GMS_MAINSTRUCTURE::SetTE_CLS_DropWait_TimeOut(CLSUnit *pCLS)
{
	TQUNIT_T	*pTQUnit;
	if( m_TQ.AddGetRef(m_tickNow+m_cfginfo.nSvrDropWaitTimeOut, (void **)&pTQUnit, &pCLS->teid)<0 )
	{
		// �����������ʱ����޷�����ʱ���¼�
		assert(0);
		return;
	}
	pTQUnit->tefunc			= &GMS_MAINSTRUCTURE::TEDeal_CLS_DropWait_TimeOut;
	pTQUnit->un.svr.nID		= pCLS->nID;
}
