// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick_DealGZSMsg.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CLSģ���Tick_DealGZSMsg����
//                PNGS��Pixel Network Game Structure����д
//                CLS��Connection Load Server����д�������ӷ��书���еĸ��ط�����
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
				// ��������Ƿ����
				switch( tcp_check_connecting_socket(pGZS->sockGZS) )
				{
					case	CONNECTING_SOCKET_CHKRST_OK:
					{
						// �������
						// ��ʼ��msger
						m_MSGER_INFO.sock	= pGZS->sockGZS;
						int	rst	= pGZS->msgerGZS.Init(m_MSGER_INFO.GetBase());
						if( rst<0 )
						{
							// ��ʼ��ʧ��
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(645,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,%d,pGZS->msgerGZS.Init,%d", rst, pGZS->nSvrIdx);
							m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
						}
						else
						{
							// ����HELLO
							CLS_GZS_HELLO_T	Hello;
							Hello.nCmd		= CLS_GZS_HELLO;
							Hello.nCLSID	= m_nCLSID;
							Hello.nCLSVer	= CLS_VER;
							pGZS->msgerGZS.SendMsg(&Hello, sizeof(Hello));
							// ������״̬
							pGZS->nStatus	= GZSUnit::STATUS_SHAKEHAND;
							// �������ֳ�ʱ
							pGZS->tickConnectGZSTimeOut	= m_tickNow + m_cfginfo.nHelloTimeOut;
							GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(646,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,GZS connected,%d", pGZS->nSvrIdx);
						}
					}
					break;
					case	CONNECTING_SOCKET_CHKRST_CONNTECTING:
					{
						// �ж��Ƿ�ʱ
						if( wh_tickcount_diff(m_tickNow, pGZS->tickConnectGZSTimeOut) >= 0 )
						{
							// ��ʱ��
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(647,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,,connecting timeout,%d", pGZS->nSvrIdx);
							m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
						}
					}
					break;
					case	CONNECTING_SOCKET_CHKRST_ERR:
					{
						// �����ˣ�ɾ�����GZS
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
				// �ж�msger�������Ƿ���������
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
					// �����������GZS�յ���ָ��
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
									// �ں���������ʱ��رպ�GZS������
									// RemoveGZSUnit�ڲ������GMS�Լ������GZS����ʧ��
									m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
								}
								else
								{
									// �ù���״̬
									pGZS->nStatus	= GZSUnit::STATUS_WORKING;
									// ����GMS�Լ������GZS���ӳɹ���
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
						// ������ദ��HI��һ��ָ�������Ҫ���������working�д���
					}
				}
			}
			break;
			case	GZSUnit::STATUS_WORKING:
			{
				pGZS->msgerGZS.ManualRecv();
				// �ж�msger�������Ƿ���������
				if( pGZS->msgerGZS.GetStatus() != tcpmsger::STATUS_WORKING )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(652,PNGS_RT)"CLS2_I::Tick_DealGZSMsg,,socket ERROR,%d", pGZS->nSvrIdx);
					m_vectIDToDestroy.push_back(pGZS->nSvrIdx);
				}
				else
				{
					m_nCurCmdFromSvrIdx	= i;
					// �����������GZS�յ���ָ��
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

// ��GZS��ص���������
int		CLS2_I::GetGZSSockets(whvector<SOCKET> &vect)
{
	// ��������GZS�����ȡsocket
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
	// ��������GZS����������Send
	for(int i=1;i<GZS_MAX_NUM;++i)
	{
		GZSUnit	*pGZS	= &m_GZSs[i];
		switch( pGZS->nStatus )
		{
			// �⼸���������Ҫ���͵ģ�������ô�ܴ�STATUS_SHAKEHAND���STATUS_WORKING
			case	GZSUnit::STATUS_SHAKEHAND:
			case	GZSUnit::STATUS_WORKING:
				pGZS->msgerGZS.ManualSend();
			break;
		}
	}
	return		0;
}
