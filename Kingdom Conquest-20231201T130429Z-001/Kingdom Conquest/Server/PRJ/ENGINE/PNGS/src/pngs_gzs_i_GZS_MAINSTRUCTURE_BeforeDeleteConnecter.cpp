// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_BeforeDeleteConnecter.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTUREģ���BeforeDeleteConnecter��������ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Zone Server����д�����߼��������е��ܿط�����
//                ��ΪRemoveMYCNTR�ᵼ��BeforeDeleteConnecter�ĵ��ã����԰����Ƿ���һ���ļ���
// CreationDate : 2005-09-20
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::RemoveMYCNTR(int nCntrID)
{
	return	RemoveMYCNTR( (MYCNTRSVR::MYCNTR *)m_CntrSvr.GetConnecterPtr(nCntrID)->QueryInterface() );
}
int		GZS_MAINSTRUCTURE::RemoveMYCNTR(MYCNTRSVR::MYCNTR *pCntr)
{
	if( !pCntr )
	{
		return	-1;
	}
	// ��Ϊ�������һ���Ǻ�������صģ�����ֱ�ӹر����Ӽ���
	// �����ϵ�Ĳ�����BeforeDeleteConnecter
	closesocket( pCntr->GetSocket() );
	return	0;
}
void	GZS_MAINSTRUCTURE::BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T *pCntrInfo)
{
	// ȷ�����ĸ�Connecter
	MYCNTRSVR::MYCNTR	*pCntr	= (MYCNTRSVR::MYCNTR *)pCntrInfo->pCntr->QueryInterface();
	if(!pCntr)
	{
		// �п�����accepter
		return;
	}
	//
	if( !pCntr->m_pExtPtr )
	{
		// û�й�����Ӧ��Server��Ӧ����û��ͨ����һ��У�飩
		// ����������ζ�Ӧ�����һ��
		// ��Ϊ����ָ����delete pCntr��ʱ���˳����У���Ϊ��whtimequeue::ID_T�������в��������������
		pCntr->clear();
		return;
	}
	// ������صĹ�ϵ��������(�����CLS��Ҫ�Ȱ�CLS��ص�����Player������)
	switch( pCntr->m_nSvrType )
	{
		case	APPTYPE_CLS:
		{
			CLSUnit	*pCLS	= (CLSUnit *)pCntr->m_pExtPtr;
			// ������к�CLS��ص����
			int	nCount	= 0;
			whunitallocatorFixed<PlayerUnit>::iterator	it = m_Players.begin();
			while( it!=m_Players.end() )
			{
				PlayerUnit	*pPlayer	= &(*it);
				++	it;
				if( pPlayer->nCLSID == pCLS->nID )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(809,PNGS_RT)"Player removed on CLS,0x%X,0x%X,0x%X,%d", pCntr->GetIDInMan(), pCLS->nID, pPlayer->nID, m_Players.size());
					// �Ǻ����CLS��ص�
					RemovePlayerUnit(pPlayer);
					nCount	++;
				}
			}

			// ֪ͨGMS��CLS���Լ�������
			if( m_nMood == MOOD_WORKING )
			{
				// ֻ���ڹ���״̬����Ҫ֪ͨGMS��ֹͣ���̾Ͳ�����
				GZS_GMS_CTRL_T	Cmd;
				Cmd.nCmd		= GZS_GMS_CTRL;
				Cmd.nSubCmd		= GZS_GMS_CTRL_T::SUBCMD_CLSDOWN;
				Cmd.nParam		= pCLS->nID;
				m_msgerGMS.SendMsg(&Cmd, sizeof(Cmd));
			}

			// ɾ������
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(682,PNGS_RT)"CLS removed,0x%X,0x%X,%d", pCntr->GetIDInMan(), pCLS->nID, nCount);
			pCLS->clear();
			m_CLSs.FreeUnitByPtr(pCLS);
		}
		break;
		default:
		{
			// �������͵�Ӧ��������û��ͨ����֤��ɾ���ģ����Բ��ᵽ����
			assert(0);
		}
		break;
	}
	// ���һ��Connecter�������߼���صĲ���
	pCntr->clear();
}
