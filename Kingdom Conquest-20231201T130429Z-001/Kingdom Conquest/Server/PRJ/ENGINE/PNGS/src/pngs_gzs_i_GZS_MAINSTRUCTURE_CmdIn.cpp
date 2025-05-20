// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_CmdIn.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTUREģ����CmdIn������ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Zone Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-09-21
// Change LOG   : 2006-04-26 ������GZS�رջᵼ��������Ҷ��ߵ�bug����ԭ�������յ�STOPPING MOOD��ʱ���������Ҷ��ߣ�����������GMS�����������ʵ��GZS�ж���һ�����ݣ����Ը�Ϊ֪ͨCLS������GZS��ص�����ߵ���

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_logic.h"
#include "../inc/pngs_packet_gzs_logic.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	// �ȴ���һЩ���÷��͵���Ϣ�����е�ָ��
	switch( nCmd )
	{
		case	PNGSPACKET_2LOGIC_DISPHISTORY:
		{
		}
		break;
		case	GZSPACKET_2MS_CONFIG:
		{
			// ������⣬������ܻ�ı�ָ�����ݵ�������Ϊ����
			CmdIn_GZSPACKET_2MS_CONFIG(pRstAccepter, (GZSPACKET_2MS_CONFIG_T *)pData, nDSize);
		}
		break;
		case	PNGSPACKET_2LOGIC_REGCMDDEAL:
		{
			PNGSPACKET_2LOGIC_REGCMDDEAL_T	*pReg	= (PNGSPACKET_2LOGIC_REGCMDDEAL_T *)pData;
			if( !m_mapCmdReg2Logic.put(pReg->nCmdReg, pRstAccepter) )
			{
				// ˵��Ӧ���Ѿ���ע���һ����
				assert(0 && "This cmd is registered!");
				return	-1;
			}
		}
		break;
		default:
		{
			assert(0);
		}
		break;
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::CmdIn_GZSPACKET_2MS_CONFIG(ILogicBase *pRstAccepter, GZSPACKET_2MS_CONFIG_T *pCmd, int nSize)
{
	switch( pCmd->nSubCmd )
	{
	case	GZSPACKET_2MS_CONFIG_T::SUBCMD_GET_SVRIDX:
		{
			pCmd->nParam	= m_cfginfo.nSvrIdx;
		}
		break;
	case	GZSPACKET_2MS_CONFIG_T::SUBCMD_GET_CONNECTEDTOGMSPTR:
		{
			pCmd->nParam	= (int)&m_bConnectedToGMS;
		}
		break;
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	switch(nCmd)
	{
	case	PNGSPACKET_2LOGIC_SETMOOD:
		{
			PNGSPACKET_2LOGIC_SETMOOD_T	*pCmdSetMood	= (PNGSPACKET_2LOGIC_SETMOOD_T *)pData;
			switch( pCmdSetMood->nMood )
			{
			case	CMN::ILogic::MOOD_STOPPING:
				{
					// Ĭ�ϾͿ���ֱ��ֹͣ��
					m_nMood	= CMN::ILogic::MOOD_STOPPED;
					// ����GMS�Լ��������˳���
					TellGMSGoodExit();
					// ����CLS�߳������Լ���ص��û�
					TellCLSToKickAllPlayerOfMe();
				}
				break;
			}
		}
		break;
	case	GZSPACKET_2MS_CMD2ONEPLAYER:
		{
			GZSPACKET_2MS_CMD2ONEPLAYER_T	*pCmd	= (GZSPACKET_2MS_CMD2ONEPLAYER_T *)pData;
			SendCmdToPlayer(pCmd->nClientID, pCmd->nChannel, pCmd->pData, pCmd->nDSize);
		}
		break;
	case	GZSPACKET_2MS_CMD2MULTIPLAYER:
		{
			GZSPACKET_2MS_CMD2MULTIPLAYER_T	*pCmd	= (GZSPACKET_2MS_CMD2MULTIPLAYER_T *)pData;
#ifdef	_DEBUG
			if( m_cfginfo.bLogSendCmd )
			{
				int	nDSize	= pCmd->nDSize>8 ? 8 : pCmd->nDSize;
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(910,CMDLOG)"CMD2MULTIPLAYER,%d,%d,%d,%s", m_tickNow, pCmd->nClientNum, pCmd->nDSize, wh_hex2str((unsigned char *)pCmd->pData, nDSize, NULL, 1, true));
			}
#endif
			SendCmdToMultiPlayer(pCmd->panClientID, pCmd->nClientNum, pCmd->nChannel, pCmd->pData, pCmd->nDSize);
		}
		break;
	case	GZSPACKET_2MS_CMD2PLAYERWITHTAG:
		{
			GZSPACKET_2MS_CMD2PLAYERWITHTAG_T	*pCmd= (GZSPACKET_2MS_CMD2PLAYERWITHTAG_T *)pData;
			SendCmdToAllPlayerByTag(pCmd->nTagIdx, pCmd->nTagVal, pCmd->nChannel, pCmd->pData, pCmd->nDSize, pCmd->nSvrIdx);
		}
		break;
	case	GZSPACKET_2MS_CMD2ALLPLAYER:
		{
			GZSPACKET_2MS_CMD2ALLPLAYER_T	*pCmd	= (GZSPACKET_2MS_CMD2ALLPLAYER_T *)pData;
			SendCmdToAllPlayer(pCmd->nChannel, pCmd->pData, pCmd->nDSize, pCmd->nSvrIdx);
		}
		break;
	case	GZSPACKET_2MS_SETPLAYERTAG:
		{
			GZSPACKET_2MS_SETPLAYERTAG_T	*pCmd	= (GZSPACKET_2MS_SETPLAYERTAG_T *)pData;
			SetPlayerTag(pCmd->nClientID, pCmd->nTagIdx, pCmd->nTagVal);
		}
		break;
	case	GZSPACKET_2MS_KICKPLAYER:
		{
			GZSPACKET_2MS_KICKPLAYER_T		*pCmd	= (GZSPACKET_2MS_KICKPLAYER_T *)pData;
			TellCLSToKickPlayer(pCmd->nClientID);
		}
		break;
	case	GZSPACKET_2MS_SVRCMD:
		{
			GZSPACKET_2MS_SVRCMD_T			*pCmd	= (GZSPACKET_2MS_SVRCMD_T *)pData;
			SendSvrCmdToSvrBySvrIdx(pCmd->nSvrIdx, pCmd->pData, pCmd->nDSize);
		}
		break;
	case	GZSPACKET_2MS_CMD_PACK_ADD:
		{
			// �������
			GZSPACKET_2MS_CMD_PACK_ADD_T	*pCmd	= (GZSPACKET_2MS_CMD_PACK_ADD_T *)pData;
			AddPackData(pCmd->pData, pCmd->nDSize);
		}
		break;
	case	GZSPACKET_2MS_CMD_PACK_BEGIN:
		{
			m_cpb	= *(GZSPACKET_2MS_CMD_PACK_BEGIN_T *)pData;
			m_wcsCmdPack.Reset();
		}
		break;
	case	GZSPACKET_2MS_SETPLAYERTAG64:
		{
			GZSPACKET_2MS_SETPLAYERTAG64_T	*pCmd	= (GZSPACKET_2MS_SETPLAYERTAG64_T *)pData;
			SetPlayerTag64(pCmd->nClientID, pCmd->nTag, pCmd->bDel);
		}
		break;
	case	GZSPACKET_2MS_CMD2PLAYERWITHTAG64:
		{
			GZSPACKET_2MS_CMD2PLAYERWITHTAG64_T	*pCmd	= (GZSPACKET_2MS_CMD2PLAYERWITHTAG64_T *)pData;
			SendCmdToAllPlayerByTag64(pCmd->nTag, pCmd->nChannel, pCmd->pData, pCmd->nDSize, pCmd->nSvrIdx);
		}
		break;
	default:
		{
			// pCmdIn->pRstAccepter����Ϊ�գ����Բ��ܻ�ȡ��logictype
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(653,PNGS_RT)"GZS_MAINSTRUCTURE::DealCmdIn_One,%ld,unknown,%p", nCmd, pRstAccepter);
		}
		break;
	}

	return	0;
}
