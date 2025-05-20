// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_CmdIn.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GZS模块内部的GZS_MAINSTRUCTURE模块中CmdIn处理部分实现
//                PNGS是Pixel Network Game Structure的缩写
//                GZS是Game Zone Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-09-21
// Change LOG   : 2006-04-26 修正了GZS关闭会导致所有玩家断线的bug。（原理是在收到STOPPING MOOD的时候把所有玩家断线，但是忘记了GMS的所有玩家其实在GZS中都有一个备份，所以改为通知CLS把所有GZS相关的玩家踢掉）

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_logic.h"
#include "../inc/pngs_packet_gzs_logic.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	// 先处理一些不用发送到消息队列中的指令
	switch( nCmd )
	{
		case	PNGSPACKET_2LOGIC_DISPHISTORY:
		{
		}
		break;
		case	GZSPACKET_2MS_CONFIG:
		{
			// 这个特殊，里面可能会改变指令数据的内容作为返回
			CmdIn_GZSPACKET_2MS_CONFIG(pRstAccepter, (GZSPACKET_2MS_CONFIG_T *)pData, nDSize);
		}
		break;
		case	PNGSPACKET_2LOGIC_REGCMDDEAL:
		{
			PNGSPACKET_2LOGIC_REGCMDDEAL_T	*pReg	= (PNGSPACKET_2LOGIC_REGCMDDEAL_T *)pData;
			if( !m_mapCmdReg2Logic.put(pReg->nCmdReg, pRstAccepter) )
			{
				// 说明应该已经被注册过一次了
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
					// 默认就可以直接停止了
					m_nMood	= CMN::ILogic::MOOD_STOPPED;
					// 告诉GMS自己是正常退出的
					TellGMSGoodExit();
					// 告诉CLS踢出所有自己相关的用户
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
			// 添加数据
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
			// pCmdIn->pRstAccepter可能为空，所以不能获取其logictype
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(653,PNGS_RT)"GZS_MAINSTRUCTURE::DealCmdIn_One,%ld,unknown,%p", nCmd, pRstAccepter);
		}
		break;
	}

	return	0;
}
