// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_Tick_DealGMSMsg.cpp
// Creator      : Wei Hua (κ��)
// Comment      : GZS��Tick_DealGMSMsg���ֵľ���ʵ��
// CreationDate : 2005-09-22
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::Tick_DealGMSMsg()
{
	m_tickNow	= wh_gettickcount();

	m_msgerGMS.Tick();
	// �ܵ�����˵��GMS�����ӻ�����

	// �����GMS��������Ϣ
	pngs_cmd_t	*pCmd;
	size_t		nSize;
	while( (pCmd=(pngs_cmd_t *)m_msgerGMS.PeekMsg(&nSize)) != NULL )
	{
		if( m_cfginfo.bLogRecvCmd )
		{
			int		nTmpSize	= nSize>256 ? 256 : nSize;
			char	szBuf[4096];
			GLOGGER2_WRITEFMT(GLOGGER_ID_DATARECOVER, GLGR_STD_HDR(1031,CMDLOG)"GMS,%u,,%d,0x%X,%s", m_tickNow, nSize, *pCmd, wh_hex2str((unsigned char *)pCmd, nTmpSize, szBuf, 1, true));
		}
		switch( *pCmd )
		{
			case	GMS_GZS_CTRL:
			{
				Tick_DealGMSMsg_GMS_GZS_CTRL(pCmd, nSize);
			}
			break;
			case	GMS_GZS_PLAYER_ONLINE:
			{
				Tick_DealGMSMsg_GMS_GZS_PLAYER_ONLINE(pCmd, nSize);
			}
			break;
			case	GMS_GZS_PLAYER_OFFLINE:
			{
				Tick_DealGMSMsg_GMS_GZS_PLAYER_OFFLINE(pCmd, nSize);
			}
			break;
			case	GMS_GZS_GAMECMD:
			{
				Tick_DealGMSMsg_GMS_GZS_GAMECMD(pCmd, nSize);
			}
			break;
			case	GMS_GZS_PLAYER_CHANGEID:
			{
				Tick_DealGMSMsg_GMS_GZS_PLAYER_CHANGEID(pCmd, nSize);
			}
			break;
			default:
			{
				// ����Ķ�������
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(677,PNGS_RT)"GZS_MAINSTRUCTURE::Tick_DealGMSMsg,%d,unknown", *pCmd);
			}
			break;
		}
		// �ͷŸոմ������Ϣ
		m_msgerGMS.FreeMsg();
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::Tick_DealGMSMsg_GMS_GZS_CTRL(void *pCmd, int nSize)
{
	GMS_GZS_CTRL_T	*pCtrl	= (GMS_GZS_CTRL_T *)pCmd;
	switch( pCtrl->nSubCmd )
	{
		case	GMS_GZS_CTRL_T::SUBCMD_EXIT:
		{
			// ��������ܿ����˳���
			CmdOutToLogic_AUTO(m_pCMN, CMN::CMD2CMN_SHOULDSTOP, NULL, 0);
		}
		break;
		case	GMS_GZS_CTRL_T::SUBCMD_STRCMD:
		{
		}
		break;
		case	GMS_GZS_CTRL_T::SUBCMD_STRRST:
		{
		}
		break;
		case	GMS_GZS_CTRL_T::SUBCMD_CLSDOWN:
		{
			// ���CLS������ر���CLS������
			CLSUnit	*pCLS	= m_CLSs.getptr(pCtrl->nParam);
			if( pCLS )
			{
				// ֻҪ���߾Ϳ��԰��ö��ߵĹ��̴�����(�Զ���������û�֮���)
				RemoveMYCNTR(pCLS->pCntr);
			}
		}
		break;
		case	GMS_GZS_CTRL_T::SUBCMD_CHECKPLAYEROFFLINE:
		{
			int		*panID			= (int *)wh_getptrnexttoptr(pCtrl);
			whhashset<int>			myset;
			myset.resizebucket(pCtrl->nParam);
			for(int i=0;i<pCtrl->nParam;i++)
			{
				myset.put(panID[i]);
			}
			whunitallocatorFixed<PlayerUnit>::iterator	it = m_Players.begin();
			while( it!=m_Players.end() )
			{
				int	nID	= (*it++).nID;
				if( !myset.has( nID ) )
				{
					m_Players.FreeUnit(nID);
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(848,PNGS_RELOAD)"GMS_GZS_CTRL_T::SUBCMD_CHECKPLAYEROFFLINE Free,0x%X", nID);
				}
			}
		}
		break;
		case	GMS_GZS_CTRL_T::SUBCMD_ENABLE_CLIENT_RECV:
		{
			whbit_ulong_clr(&m_nNoCLIENT_RECV, pCtrl->nParam);
		}
		break;
		case	GMS_GZS_CTRL_T::SUBCMD_DISENABLE_CLIENT_RECV:
		{
			whbit_ulong_set(&m_nNoCLIENT_RECV, pCtrl->nParam);
		}
		break;
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::Tick_DealGMSMsg_GMS_GZS_PLAYER_ONLINE(void *pCmd, int nSize)
{
	GMS_GZS_PLAYER_ONLINE_T	*pPO	= (GMS_GZS_PLAYER_ONLINE_T *)pCmd;
	// �����ҵĸ���
	int		nNum	= ( nSize - wh_offsetof(GMS_GZS_PLAYER_ONLINE_T, anClientID) ) / wh_sizeinclass(GMS_GZS_PLAYER_ONLINE_T, anClientID[0]);
	assert(nNum>0);
	// �������
	for(int i=0;i<nNum;i++)
	{
		int			nID				= pPO->anClientID[i];
		int			nOldID			= 0;
		PlayerUnit	*pPlayer		= m_Players.AllocUnitByID(nID, &nOldID);
		if( pPlayer )
		{
			pPlayer->nID			= nID;
			pPlayer->nCLSID			= pPO->nCLSID;
			/* ��Ϊ������CLSҲ���GZS���������Բ������ϰ��û������ߣ�������Ϊ�û��ո����ӵ�ʱ��Ҳ�ᷢ�����GZS��������ʱ��������û��������ǲ��õ�
			   �����������ȷ��GMS����CLS��GZS�������֮���ٷ��͸����û���������Ϣ�Ͳ�������������
			// �����ж�һ��CLS�Ƿ���ڣ������������GMS�߳��û�
			if( m_CLSs.getptr(pPlayer->nCLSID) == NULL )
			{
				TellGMSToKickPlayerAndRemovePlayer(pPlayer->nID);
			}
			*/
		}
		else
		{
			// �����ϲ�Ӧ�÷���ʧ�ܵ�
			// ���ǿ�����ĳЩ�����GMS��һ�����ظ����͹���
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(825,PLAYER_IN)"GZS_MAINSTRUCTURE::Tick_DealGMSMsg_GMS_GZS_PLAYER_ONLINE,0x%X,m_Players.AllocUnitByID,0x%X", nID, nOldID);
		}
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::Tick_DealGMSMsg_GMS_GZS_PLAYER_OFFLINE(void *pCmd, int nSize)
{
	GMS_GZS_PLAYER_OFFLINE_T		*pPO	= (GMS_GZS_PLAYER_OFFLINE_T *)pCmd;
	// �����ҵĸ���
	int		nNum	= ( nSize - wh_offsetof(GMS_GZS_PLAYER_OFFLINE_T, anClientID) ) / wh_sizeinclass(GMS_GZS_PLAYER_OFFLINE_T, anClientID[0]);
	// ɾ�����
	for(int i=0;i<nNum;i++)
	{
		m_Players.FreeUnit(pPO->anClientID[i]);
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::Tick_DealGMSMsg_GMS_GZS_GAMECMD(void *pCmd, int nSize)
{
	GMS_GZS_GAMECMD_T	*pGameCmd	= (GMS_GZS_GAMECMD_T *)pCmd;
	nSize	-= wh_offsetof(GMS_GZS_GAMECMD_T, data);
	assert(nSize > 0);

	// �ȿ��Ƿ���Ҫ���ɸ����logic
	pngs_cmd_t	*pnCmd		= (pngs_cmd_t *)pGameCmd->data;
	ILogicBase	*pLogic;
	if( m_mapCmdReg2Logic.get(*pnCmd, pLogic) )
	{
		if( pLogic->CmdIn_NMT(this, PNGSPACKET_2LOGIC_DISPATCHCMD, pGameCmd->data, nSize)<0 )
		{
			return	-1;
		}
		return	0;
	}

	// ����GP	
	GZSPACKET_2GP_SVRCMD_T	Cmd2GP;
	Cmd2GP.nFromSvrIdx		= pGameCmd->nFromSvrIdx;
	Cmd2GP.nDSize			= nSize;
	Cmd2GP.pData			= pGameCmd->data;
	return	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, GZSPACKET_2GP_SVRCMD, Cmd2GP);
}
int		GZS_MAINSTRUCTURE::Tick_DealGMSMsg_GMS_GZS_PLAYER_CHANGEID(void *pCmd, int nSize)
{
	GMS_GZS_PLAYER_CHANGEID_T	*pC	= (GMS_GZS_PLAYER_CHANGEID_T *)pCmd;
	PlayerUnit	*pPlayerTo		= m_Players.AllocUnitByID(pC->nToID);
	if( pPlayerTo )
	{
		PlayerUnit	*pPlayerFrom	= m_Players.getptr(pC->nFromID);
		if( pPlayerFrom )
		{
			pPlayerTo->clear();
			pPlayerTo->nID			= pC->nToID;
			pPlayerTo->nCLSID		= pPlayerFrom->nCLSID;
			// ɾ����Դ
			m_Players.FreeUnitByPtr(pPlayerFrom);
		}
	}
	return	0;
}
