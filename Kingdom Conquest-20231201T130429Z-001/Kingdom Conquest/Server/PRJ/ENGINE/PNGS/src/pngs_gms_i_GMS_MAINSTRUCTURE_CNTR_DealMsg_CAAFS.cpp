// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CNTR_DealMsg_CAAFS.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ���е�CNTR_DealMsg_CAAFS��ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-23
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::CNTR_DealMsg_CAAFS(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	// ͨ��Connecter�ҵ����CAAFS
	CAAFSGroup	*pGroup				= (CAAFSGroup *)pCntr->m_pExtPtr;
	assert(pGroup);
	assert(pGroup->pCntr);
	int			nGroupID			= m_pSHMData->GetCAAFSGroupID(pGroup);

	switch( *pCmd )
	{
		case	CAAFS_GMS_CTRL:
		{
		}
		break;
		case	CAAFS_GMS_CLIENTWANTCLS:
		{
			CAAFS_GMS_CLIENTWANTCLS_T		*pClientWantCLS	= (CAAFS_GMS_CLIENTWANTCLS_T *)pCmd;
			GMS_CAAFS_CLIENTCANGOTOCLS_T	ClientCanGo;
			ClientCanGo.nCmd				= GMS_CAAFS_CLIENTCANGOTOCLS;
			ClientCanGo.nClientIDInCAAFS	= pClientWantCLS->nClientIDInCAAFS;
			// �������CAAFS��Group�е��ĸ�CLS��λ�����
			CLSUnit		*pCLS				= NULL;
			int			nMinPlayer			= m_cfginfo.anMaxPlayerNumInCLSOfCAAFSGroup[nGroupID];
			for(whDList<CLSUnit *>::node *pNode=pGroup->dlCLS.begin(); pNode!=pGroup->dlCLS.end(); pNode=pNode->next)
			{
				CLSUnit		*pTmpCLS		= pNode->data;
				if( !pTmpCLS->CanAcceptPlayer()						// ���ܽ����û�
				||  pTmpCLS->GetCurPlayerNum()>=pTmpCLS->nMaxPlayer	// �Ѿ�����
				||  pTmpCLS->nGroupID != nGroupID			// �������CAAFS���
				)
				{
					continue;
				}
				if( pTmpCLS->nNetworkType == NETWORKTYPE_UNKNOWN
				||  pTmpCLS->nNetworkType == pClientWantCLS->nNetworkType
				)
				{
					if( pTmpCLS->GetCurPlayerNum() < nMinPlayer )
					{
						nMinPlayer	= pTmpCLS->GetCurPlayerNum();
						pCLS		= pTmpCLS;
					}
				}
			}
			if( pCLS == NULL  )
			{
				// û��λ����
				ClientCanGo.nRst			= GMS_CAAFS_CLIENTCANGOTOCLS_T::RST_ERR_NOCLSCANACCEPT;
				pCntr->SendMsg(&ClientCanGo, sizeof(ClientCanGo));
				return	0;
			}

			// �������
			int			nID			= -1;
			PlayerUnit	*pPlayer	= m_pSHMData->m_Players.AllocUnit(&nID);
			if( nID<0 )
			{
				// ֪ͨCAAFS�޷����з���
				ClientCanGo.nRst			= GMS_CAAFS_CLIENTCANGOTOCLS_T::RST_ERR_MEM;
				pCntr->SendMsg(&ClientCanGo, sizeof(ClientCanGo));
				return	0;
			}
			// �������ú͹���
			pPlayer->clear();
			pPlayer->nID		= nID;
			pPlayer->nStatus	= PlayerUnit::STATUS_WANTCLS;
			pPlayer->nCLSID		= pCLS->nID;
			pPlayer->nClientIDInCAAFS		= pClientWantCLS->nClientIDInCAAFS;
			pPlayer->IP			= pClientWantCLS->IP;
			pPlayer->nNetworkType			= pClientWantCLS->nNetworkType;
			// ����������ݸ�CLS
			GMS_CLS_CLIENTWANTCLS_T			ToCLSClientWantCLS;
			ToCLSClientWantCLS.nCmd			= GMS_CLS_CLIENTWANTCLS;
			ToCLSClientWantCLS.nClientID	= nID;
			ToCLSClientWantCLS.nPassword	= pClientWantCLS->nPassword;
			pCLS->SendMsg(&ToCLSClientWantCLS, sizeof(ToCLSClientWantCLS));
			// ע��ֻҪCLS��崾�һ���з��صġ����CLS��˾���GMS���������������ص���ҡ�
			// ����CLS���б�
			pCLS->dlPlayer.AddToTail(&pPlayer->dlnodeInCLS);
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(832,PLAYER_IN)"CAAFS_GMS_CLIENTWANTCLS AllocUnit,0x%X,%d,0x%X,0x%X,%d", nID, m_pSHMData->m_Players.size(), pClientWantCLS->nClientIDInCAAFS, pCLS->nID, pCLS->dlPlayer.size());
		}
		break;
		case	CAAFS_GMS_CLIENT_CMD:
		{
			// �����ϲ�
			CAAFS_GMS_CLIENT_CMD_T	*pClientCmd	= (CAAFS_GMS_CLIENT_CMD_T *)pCmd;
			GMSPACKET_2GP_PLAYERCMD_FROM_CAAFS_T	Cmd2GP;
			Cmd2GP.nCAAFSIdx					= nGroupID;
			Cmd2GP.nClientIDInCAAFS				= pClientCmd->nClientIDInCAAFS;
			Cmd2GP.IP							= pClientCmd->IP;
			Cmd2GP.pData						= wh_getptrnexttoptr(pClientCmd);
			Cmd2GP.nDSize						= nSize - sizeof(*pClientCmd);
			CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, GMSPACKET_2GP_PLAYERCMD_FROM_CAAFS, Cmd2GP);
		}
		break;
	}
	return	0;
}
