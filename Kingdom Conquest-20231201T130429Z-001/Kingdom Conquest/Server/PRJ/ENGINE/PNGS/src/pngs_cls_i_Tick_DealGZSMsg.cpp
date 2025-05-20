// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick_DealGZSMsg.cpp
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的CLS模块的Tick_DealGZSMsg内容
//                PNGS是Pixel Network Game Structure的缩写
//                CLS是Connection Load Server的缩写，是连接分配功能中的负载服务器
// CreationDate : 2005-08-09
// Change LOG   :

#include "../inc/pngs_cls_i.h"

using namespace n_pngs;

int		CLS2_I::Tick_DealGZSMsg()
{
	m_tickNow	= wh_gettickcount();

	m_vectIDToDestroy.clear();
	for(int i=1;i<GZS_MAX_NUM;++i)
	{
		GZSUnit	*pGZS	= &m_GZSs[i];
		switch( pGZS->nStatus )
		{
			case	GZSUnit::STATUS_CONNECTING:
			{
				// 检查连接是否完成
				switch( tcp_check_connecting_socket(pGZS->sockGZS) )
				{
					case	CONNECTING_SOCKET_CHKRST_OK:
					{
						// 连接完成
						// 初始化msger
						m_MSGER_INFO.sock	= pGZS->sockGZS;
						int	rst	= pGZS->msgerGZS.Init(m_MSGER_INFO.GetBase());
						if( rst<0 )
						{
							// 初始化失败
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(645,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,%d,pGZS->msgerGZS.Init,%d", rst, pGZS->nSvrIdx);
							m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
						}
						else
						{
							// 发送HELLO
							CLS_GZS_HELLO_T	Hello;
							Hello.nCmd		= CLS_GZS_HELLO;
							Hello.nCLSID	= m_nCLSID;
							Hello.nCLSVer	= CLS_VER;
							pGZS->msgerGZS.SendMsg(&Hello, sizeof(Hello));
							// 置握手状态
							pGZS->nStatus	= GZSUnit::STATUS_SHAKEHAND;
							// 设置握手超时
							pGZS->tickConnectGZSTimeOut	= m_tickNow + m_cfginfo.nHelloTimeOut;
							GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(646,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,GZS connected,%d", pGZS->nSvrIdx);
						}
					}
					break;
					case	CONNECTING_SOCKET_CHKRST_CONNTECTING:
					{
						// 判断是否超时
						if( wh_tickcount_diff(m_tickNow, pGZS->tickConnectGZSTimeOut) >= 0 )
						{
							// 超时了
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(647,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,,connecting timeout,%d", pGZS->nSvrIdx);
							m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
						}
					}
					break;
					case	CONNECTING_SOCKET_CHKRST_ERR:
					{
						// 出错了，删除这个GZS
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(648,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,,connecting socket ERROR,%d", pGZS->nSvrIdx);
						m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
					}
					break;
				}
			}
			break;
			case	GZSUnit::STATUS_SHAKEHAND:
			{
				pGZS->msgerGZS.ManualRecv();
				// 判断msger的网络是否有问题了
				if( pGZS->msgerGZS.GetStatus() != tcpmsger::STATUS_WORKING )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(649,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,,socket ERROR while SHAKEHAND,%d", pGZS->nSvrIdx);
					m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
				}
				else if( wh_tickcount_diff(m_tickNow, pGZS->tickConnectGZSTimeOut)>=0 )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(650,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,,timeout while SHAKEHAND,%d", pGZS->nSvrIdx);
					m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
				}
				else
				{
					// 则正常处理该GZS收到的指令
					pngs_cmd_t	*pCmd;
					size_t		nSize;
					if( (pCmd=(pngs_cmd_t *)pGZS->msgerGZS.PeekMsg(&nSize)) != NULL )
					{
						switch( *pCmd )
						{
							case	GZS_CLS_HI:
							{
								GZS_CLS_HI_T	*pHI	= (GZS_CLS_HI_T *)pCmd;
								if( pHI->nRst != GZS_CLS_HI_T::RST_OK
								||  GZS_VER != pHI->nGZSVer
								)
								{
									// 在函数结束的时候关闭和GZS的连接
									// RemoveGZSUnit内部会告诉GMS自己和这个GZS连接失败
									m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
								}
								else
								{
									// 置工作状态
									pGZS->nStatus	= GZSUnit::STATUS_WORKING;
									// 告诉GMS自己和这个GZS连接成功了
									CLS_GMS_CONNECTEDTOGZS_T	ConnectedToGZS;
									ConnectedToGZS.nCmd		= CLS_GMS_CONNECTEDTOGZS;
									ConnectedToGZS.nRst		= CLS_GMS_CONNECTEDTOGZS_T::RST_OK;
									ConnectedToGZS.nSvrIdx	= pGZS->nSvrIdx;
									m_msgerGMS.SendMsg(&ConnectedToGZS, sizeof(ConnectedToGZS));
									GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(651,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,OK,SHAKEHAND,%d", pGZS->nSvrIdx);
								}
							}
							break;
						}
						pGZS->msgerGZS.FreeMsg();
						// 这里最多处理HI这一条指令，其他的要留到后面的working中处理
					}
				}
			}
			break;
			case	GZSUnit::STATUS_WORKING:
			{
				pGZS->msgerGZS.ManualRecv();
				// 判断msger的网络是否有问题了
				if( pGZS->msgerGZS.GetStatus() != tcpmsger::STATUS_WORKING )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(652,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,,socket ERROR,%d", pGZS->nSvrIdx);
					m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
				}
				else
				{
					m_nCurCmdFromSvrIdx	= i;
					// 则正常处理该GZS收到的指令
					pngs_cmd_t	*pCmd;
					size_t		nSize;
					//BIGTICKDEBUG
					m_nGZScmdCount = 0;
					memset(m_GZStickFuncCount,0,sizeof(m_GZStickFuncCount));
					memset(m_GZStickFuncTime,0,sizeof(m_GZStickFuncTime));
					while( (pCmd=(pngs_cmd_t *)pGZS->msgerGZS.PeekMsg(&nSize)) != NULL )
					{
//#ifdef	_DEBUG
//						if( m_cfginfo.bLogSvrCmd )
//						{
//							GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(877,CMDLOG)"GZS2CLS,%d,%d,%d", pGZS->nSvrIdx, *pCmd, nSize);
//						}
//#endif
						m_nGZScmdCount++;
						whtick_t begin_wht = wh_gettickcount(); 
						switch( *pCmd )
						{
						case	SVR_CLS_CTRL:
							Tick_Deal_CmnSvrMsg_SVR_CLS_CTRL(pGZS, pCmd, nSize);
							break;
						case	SVR_CLS_SET_TAG_TO_CLIENT:
							Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG_TO_CLIENT(pCmd, nSize);
							break;
						case	SVR_CLS_CLIENT_DATA:
							Tick_Deal_CmnSvrMsg_SVR_CLS_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
							break;
						case	SVR_CLS_CLIENT_DATA_PACK:
							Tick_Deal_CmnSvrMsg_SVR_CLS_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
							break;
						case	SVR_CLS_MULTICLIENT_DATA:
							Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
							break;
						case	SVR_CLS_MULTICLIENT_DATA_PACK:
							Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
							break;
						case	SVR_CLS_TAGGED_CLIENT_DATA:
							Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
							break;
						case	SVR_CLS_TAGGED_CLIENT_DATA_PACK:
							Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
							break;
						case	SVR_CLS_ALL_CLIENT_DATA:
							Tick_Deal_CmnSvrMsg_SVR_CLS_ALL_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
							break;
						case	SVR_CLS_ALL_CLIENT_DATA_PACK:
							Tick_Deal_CmnSvrMsg_SVR_CLS_ALL_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
							break;
						case	SVR_CLS_SET_TAG64_TO_CLIENT:
							Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG64_TO_CLIENT(pCmd, nSize);
							break;
						case	SVR_CLS_TAGGED64_CLIENT_DATA:
							Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED64_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA);
							break;
						case	SVR_CLS_TAGGED64_CLIENT_DATA_PACK:
							Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED64_CLIENT_DATA(pCmd, nSize, CLS_CLIENT_DATA_PACK);
							break;
						}
						m_GZStickFuncCount[*pCmd-210]++;
						whtick_t end_wht = wh_gettickcount();
						m_GZStickFuncTime[*pCmd-210] += wh_tickcount_diff(end_wht,begin_wht);

						pGZS->msgerGZS.FreeMsg();
					}
				}
			}
			break;
		}
	}
	for(size_t	i=0;i<m_vectIDToDestroy.size();++i)
	{
		RemoveGZSUnit(m_vectIDToDestroy[i]);
	}

	return		0;
}

// 和GZS相关的其他操作
int		CLS2_I::GetGZSSockets(whvector<SOCKET> &vect)
{
	// 遍历所有GZS对象获取socket
	for(int i=1;i<GZS_MAX_NUM;++i)
	{
		GZSUnit	*pGZS	= &m_GZSs[i];
		if( pGZS->nStatus != GZSUnit::STATUS_NOTHING )
		{
			vect.push_back(pGZS->sockGZS);
		}
	}
	return		0;
}
int		CLS2_I::DoAllGZSSend()
{
	// 遍历所有GZS对象对其进行Send
	for(int i=1;i<GZS_MAX_NUM;++i)
	{
		GZSUnit	*pGZS	= &m_GZSs[i];
		switch( pGZS->nStatus )
		{
			// 这几种情况都是要发送的，否则怎么能从STATUS_SHAKEHAND变成STATUS_WORKING
			case	GZSUnit::STATUS_SHAKEHAND:
			case	GZSUnit::STATUS_WORKING:
				pGZS->msgerGZS.ManualSend();
			break;
		}
	}
	return		0;
}
