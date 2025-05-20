// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CNTR_DealMsg_Dft.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的GMS模块内部的GMS_MAINSTRUCTURE模块中的CNTR_DealMsg_Dft的实现
//                PNGS是Pixel Network Game Structure的缩写
//                GMS是Game Master Server的缩写，是逻辑服务器中的总控服务器
// CreationDate : 2005-08-23
// Change LOG   : 2006-08-23 增加了踢出老的重idx的GZS的超时

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

bool	GMS_MAINSTRUCTURE::CNTR_CanDealMsg() const
{
	return	true;
}
// 对应各个TCP连接的初始处理函数。主要是处理HELLO指令
int		GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft(MYCNTRSVR::MYCNTR *pCntr, const pngs_cmd_t *pCmd, size_t nSize)
{
	bool	bOK	= false;
	// 根据第一个包判断是那种Server
	switch( *pCmd )
	{
		case	CAAFS_GMS_HELLO:
		{
			CAAFS_GMS_HELLO_T	*pHello	= (CAAFS_GMS_HELLO_T *)pCmd;
			GMS_CAAFS_HI_T		HI;
			HI.nCmd				= GMS_CAAFS_HI;
			HI.nRst				= GMS_CAAFS_HI_T::RST_OK;
			HI.nGMSVer			= GMS_VER;
			if( CAAFS_VER != pHello->nCAAFSVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(564,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CAAFS_GMS_HELLO,,bad ver,%d,%d,%d", pHello->nGroupID, pHello->nCAAFSVer, CAAFS_VER);
				// 发送失败HI
				HI.nRst			= GMS_CAAFS_HI_T::RST_ERR_BAD_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// 注：CAAFS如果收到失败的HI则会关闭连接，如果它不关闭，则到了超时GMS会把它关闭
			// 判断Group的有效性
			if( pHello->nGroupID<0 || pHello->nGroupID>=GMS_MAX_CAAFS_NUM )
			{
				// Group需要超出范围
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(565,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CAAFS_GMS_HELLO,,nGroupID out of range,%d,%d", pHello->nGroupID, GMS_MAX_CAAFS_NUM);
				// 发送失败HI
				HI.nRst			= GMS_CAAFS_HI_T::RST_ERR_BAD_GROUPID;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			CAAFSGroup	*pCAAFS	= &m_pSHMData->m_aCAAFSGroup[pHello->nGroupID];
			if( pCAAFS->pCntr!=NULL )
			{
				// 已经有了这个Group的CAAFS了，不能再连入
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(566,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CAAFS_GMS_HELLO,,ID DUP,%d,%s", pHello->nGroupID, cmn_get_ipnportstr_by_saaddr(&pCntr->m_addr));
				// 发送失败HI
				HI.nRst			= GMS_CAAFS_HI_T::RST_ERR_BAD_GROUPID;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			else
			{
				// 关联连接器对象
				pCAAFS->pCntr			= pCntr;
				// 关联CAAFS对象
				pCntr->m_pExtPtr		= pCAAFS;
				// 设置对方类型
				pCntr->m_nSvrType		= APPTYPE_CAAFS;
				// 设置后面的处理函数
				pCntr->m_p_CNTR_DEALMSG	= &GMS_MAINSTRUCTURE::CNTR_DealMsg_CAAFS;
				// 表示成功
				bOK	= true;
				char	szReConnect[64]	= "";
				if( pHello->bReConnect )
				{
					strcpy(szReConnect, ",RECONNECT");
					// 对于重连的就不用发送HI了
				}
				else
				{
					// 发送HI
					pCntr->SendMsg(&HI, sizeof(HI));
				}
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(567,PNGS_INT_RLS)"HELLO from CAAFS,0x%X,%s%s", pCntr->GetIDInMan(), cmn_get_ipnportstr_by_saaddr(&pCntr->m_addr), szReConnect);
			}
		}
		break;
		case	CLS_GMS_HELLO:
		{
			// 注：CLS如果收到失败的HI则会关闭连接，如果它不关闭，则到了超时GMS会把它关闭
			CLS_GMS_HELLO_T	*pHello	= (CLS_GMS_HELLO_T *)pCmd;
			GMS_CLS_HI_T			HI;
			HI.nCmd					= GMS_CLS_HI;
			HI.nRst					= GMS_CLS_HI_T::RST_OK;
			HI.nGMSVer				= GMS_VER;
			if( CLS_VER != pHello->nCLSVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(568,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CLS_GMS_HELLO,,bad ver,%d,%d,%d", pHello->nGroupID, pHello->nCLSVer, CLS_VER);
				// 发送失败HI
				HI.nRst			= GMS_CLS_HI_T::RST_ERR_BAD_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// 判断Group的有效性
			if( pHello->nGroupID<0 || pHello->nGroupID>=GMS_MAX_CAAFS_NUM )
			{
				// GroupID超出范围
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(569,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CLS_GMS_HELLO,,nGroupID out of range,%d,%d", pHello->nGroupID,GMS_MAX_CAAFS_NUM);
				// 发送失败HI
				HI.nRst			= GMS_CLS_HI_T::RST_ERR_BAD_GROUPID;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			CLSUnit				*pCLS	= NULL;
			char		szReConnect[64]	= "";
			if( pHello->nOldID>0 )
			{
				// 说明是重连的
				// 查找一下旧的
				pCLS	= m_pSHMData->m_CLSs.GetPtrByID(pHello->nOldID);
				// 应该是可以找到的
				if( !pCLS )
				{
					// 发送让CLS退出
					SVR_CLS_CTRL_T		Ctrl;
					Ctrl.nCmd			= SVR_CLS_CTRL;
					Ctrl.nSubCmd		= SVR_CLS_CTRL_T::SUBCMD_EXIT;
					pCntr->SendMsg(&Ctrl, sizeof(Ctrl));
					GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(851,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CLS_GMS_HELLO,,old id not found,0x%X", pHello->nOldID);
					return	0;
				}
				strcpy(szReConnect, ",RECONNECT");
			}
			else
			{
				// 看对应的Group中的CLS数量是否已经够多
				if( m_pSHMData->m_aCAAFSGroup[pHello->nGroupID].dlCLS.size() >= m_cfginfo.anMaxCLSNumInCAAFSGroup[pHello->nGroupID] )
				{
					// 这个group的CLS已经太多了
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(570,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft CLS_GMS_HELLO,,too many CLS,%d,%d", pHello->nGroupID, m_pSHMData->m_aCAAFSGroup[pHello->nGroupID].dlCLS.size());
					// 发送失败HI
					HI.nRst			= GMS_CLS_HI_T::RST_ERR_TOOMANY_CLS;
					pCntr->SendMsg(&HI, sizeof(HI));
					return	0;
				}
				// 创建CLS对象
				int		nID			= -1;
				pCLS	= m_pSHMData->m_CLSs.AllocUnit(&nID);
				if( nID<0 )
				{
					assert(0);
					HI.nRst			= GMS_CLS_HI_T::RST_ERR_MEMERR;
					pCntr->SendMsg(&HI, sizeof(HI));
					return	0;
				}
				// 清空
				pCLS->clear();
				pCLS->pCntr			= pCntr;	// 必须在这里先关联一下，因为SendGZSInfo0ToCLSAndChangeCLSStatus要用
				pCLS->nID			= nID;
				pCLS->nGroupID		= pHello->nGroupID;
				pCLS->nNetworkType	= pHello->nNetworkType;
				pCLS->bCanAcceptPlayerWhenCLSConnectingGZS	= pHello->bCanAcceptPlayerWhenCLSConnectingGZS;
				pCLS->nMaxPlayer	= m_cfginfo.anMaxPlayerNumInCLSOfCAAFSGroup[pCLS->nGroupID];
				// 加入CAAFS/CLS的Group
				m_pSHMData->m_aCAAFSGroup[pHello->nGroupID].dlCLS.AddToTail(&pCLS->dlnode);
				// 先假定可以正式开始工作了
				pCLS->nStatus		= CLSUnit::STATUS_WORKING;
				// 发送HI (后面这些参数是成功才需要的)
				HI.nCLSID				= nID;
				HI.nGMSMaxPlayer		= m_cfginfo.nMaxPlayer;
				HI.nCLSMaxConnection	= pCLS->nMaxPlayer;
				pCntr->SendMsg(&HI, sizeof(HI));
				// 发送当前的所有Hello成功并成功发送IP信息的GZS信息给该CLS
				for(int i=1;i<GZS_MAX_NUM;++i)
				{
					GZSUnit	*pGZS	= &m_pSHMData->m_GZSs[i];
					if( pGZS->pCntr && pGZS->IP )
					{
						// 这里面会设定状态为连接GZS状态，还需要等待所有的GZS都连接到了才能开始工作
						SendGZSInfo0ToCLSAndChangeCLSStatus(pGZS, pCLS);
					}
				}
			}
			// 关联CLS对象
			pCntr->m_nSvrType		= APPTYPE_CLS;
			pCntr->m_pExtPtr		= pCLS;
			pCLS->pCntr				= pCntr;
			// 设置后面的处理函数
			pCntr->m_p_CNTR_DEALMSG	= &GMS_MAINSTRUCTURE::CNTR_DealMsg_CLS;
			// 表示成功
			bOK	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(571,PNGS_INT_RLS)"HELLO from CLS,0x%X,0x%X%s", pCntr->GetIDInMan(), pCLS->nID, szReConnect);
			if( szReConnect[0] )
			{
				// 让CLS告诉自己是否所有用户离线
				TellCLSToCheckPlayerOffline(pCLS);
			}
		}
		break;
		case	GZS_GMS_HELLO:
		{
			GZS_GMS_HELLO_T	*pHello	= (GZS_GMS_HELLO_T *)pCmd;
			GMS_GZS_HI_T	HI;
			memset(&HI, 0, sizeof(HI));
			HI.nCmd			= GMS_GZS_HI;
			if( GZS_VER != pHello->nGZSVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(572,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft GZS_GMS_HELLO,bad ver,%d,%d,%d", pHello->nSvrIdx, pHello->nGZSVer, GZS_VER);
				// 发送失败HI
				HI.nRst			= GMS_CAAFS_HI_T::RST_ERR_BAD_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				return	0;
			}
			// SvrIdx不能为0的
			if( pHello->nSvrIdx<=0 || pHello->nSvrIdx>GZS_MAX_NUM )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(573,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft GZS_GMS_HELLO,,nSvrIdx is out of range,%d,%d", pHello->nSvrIdx,GZS_MAX_NUM);
				// 删除该连接
				RemoveMYCNTR(pCntr);
				return	0;
			}
			// 原来不能有相同SvrIdx的对象
			GZSUnit				*pOldGZS	= m_pSHMData->m_GZSs + pHello->nSvrIdx;
			if( pOldGZS->pCntr )
			{
				// 为了查错方便，两边都要输出
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(574,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft,,GZS nSvrIdx DUP,%d,%s:%d", pHello->nSvrIdx, cmn_get_IP(pOldGZS->IP), pOldGZS->nPort);
				// 看是否需要强制踢出旧的GZS (这个一般只能是用来踢出正在运行中的GZS了，因为是TCP所以不会出现原来的调整超时过长导致GZS还存在的现象)
				if( pHello->nForceKickOldGZS )
				{
					switch( pHello->nForceKickOldGZS )
					{
					case	1:
						{
							// 温柔一点的
							GMS_GZS_CTRL_T		Ctrl;
							Ctrl.nCmd			= GMS_GZS_CTRL;
							Ctrl.nSubCmd		= GMS_GZS_CTRL_T::SUBCMD_EXIT;
							pOldGZS->pCntr->SendMsg(&Ctrl, sizeof(Ctrl));
							GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(575,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft,Old GZS will be told to quit by nSvrIdx DUP,%d,%s:%d", pHello->nSvrIdx, cmn_get_IP(pOldGZS->IP), pOldGZS->nPort);
						}
						break;
					case	2:
						{
							// 强制删除相应的Connecter
							RemoveMYCNTR(pOldGZS->pCntr);
						}
						break;
					}
				}
				// 发送结果，不过是通知索引冲突，然后对方应该自宫
				HI.nRst				= GMS_GZS_HI_T::RST_IDXDUP;
				pCntr->SendMsg(&HI, sizeof(HI));
				// 否则就在这个时间时间到来的时候就移除连接
				return	0;
			}
			char	szReConnect[64]	= "";
			GZSUnit			*pGZS	= pOldGZS;
			GMSPACKET_2GP_GZSONLINE_T	GZSOL;
			GZSOL.nSvrIdx		= pHello->nSvrIdx;
			if( pHello->bReConnect )
			{
				// 是否是重连的
				strcpy(szReConnect, ",RECONNECT");
				// 对于重连的就不用发送HI了
				GZSOL.bReconnect	= true;
				// 需要退出一下原来的时间事件
				pGZS->teid.quit();
			}
			else
			{
				// 现在可以清空了
				pGZS->clear();
				pGZS->nSvrIdx		= pHello->nSvrIdx;
				// 发送HI
				HI.nRst				= GMS_GZS_HI_T::RST_OK;
				HI.nGMSMaxPlayer	= m_cfginfo.nMaxPlayer;
				HI.nGMSMaxCLS		= m_cfginfo.nMaxCLS;
				HI.nGMSVer			= GMS_VER;
				pCntr->SendMsg(&HI, sizeof(HI));
				GZSOL.bReconnect	= false;
			}
			// 关联GZS对象
			pGZS->pCntr				= pCntr;
			//
			pCntr->m_nSvrType		= APPTYPE_GZS;
			pCntr->m_pExtPtr		= pGZS;
			// 设置后面的处理函数
			pCntr->m_p_CNTR_DEALMSG	= &GMS_MAINSTRUCTURE::CNTR_DealMsg_GZS;
			// 表示成功
			bOK	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(576,PNGS_INT_RLS)"HELLO from GZS,%d,0x%X,%s,%s", pGZS->nSvrIdx, pCntr->GetIDInMan(), szReConnect, cmn_get_IP(pGZS->IP));
			// 通知GP（GZS上线，注意，这个必须在pGZS->pCntr设置之后调用）
			CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSONLINE, &GZSOL, sizeof(GZSOL));
			if( szReConnect[0] )
			{
				// 让GZS告诉自己是否所有用户离线
				TellGZSToCheckPlayerOffline(pGZS);
			}
		}
		break;
		case	GMTOOL_SVR_HELLO:
		{
			GMTOOL_SVR_HELLO_T	*pHello	= (GMTOOL_SVR_HELLO_T *)pCmd;
			if( GMTOOL_VER != pHello->nGMTOOLVer )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(755,PNGS_INT_RLS)"GMS_MAINSTRUCTURE::CNTR_DealMsg_Dft GMTOOL_SVR_HELLO,bad ver,%d,%d", pHello->nGMTOOLVer, GMTOOL_VER);
				// 删除该连接
				RemoveMYCNTR(pCntr);
				return	0;
			}
			// 创建对象
			int			nID			= -1;
			GMTOOLUnit	*pGMTOOL	= m_pSHMData->m_GMTOOLs.AllocUnit(&nID);
			if( nID<0 )
			{
				assert(0);
				// 删除该连接
				RemoveMYCNTR(pCntr);
				return	0;
			}
			pGMTOOL->clear();
			// 关联GMTOOL对象(因为GMTOOL只是请求/应答方式，所以不需要有逻辑对象对应)
			pCntr->m_nSvrType		= APPTYPE_GMTOOL;
			pCntr->m_pExtPtr		= pGMTOOL;
			pGMTOOL->nID			= nID;
			pGMTOOL->pCntr			= pCntr;
			// 发送HI
			SVR_GMTOOL_HI_T			HI;
			HI.nCmd					= SVR_GMTOOL_HI;
			HI.nSvrType				= APPTYPE_GMS;
			HI.nVer					= GMS_VER;
			pCntr->SendMsg(&HI, sizeof(HI));
			// 设置后面的处理函数
			pCntr->m_p_CNTR_DEALMSG	= &GMS_MAINSTRUCTURE::CNTR_DealMsg_GMTOOL;
			// 表示成功
			bOK	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(756,PNGS_INT_RLS)"HELLO from GMTOOL,0x%X,0x%X", pCntr->GetIDInMan(), nID);
		}
		break;
		default:
			// 本阶段（刚刚开始的时候只能收到Hello指令）不能处理的指令，关闭连接
		break;
	}

	if( bOK )
	{
		// 对于成功的统一处理
		// 清空原来的Hello超时事件
		pCntr->teid.quit();
	}

	return	0;
}
