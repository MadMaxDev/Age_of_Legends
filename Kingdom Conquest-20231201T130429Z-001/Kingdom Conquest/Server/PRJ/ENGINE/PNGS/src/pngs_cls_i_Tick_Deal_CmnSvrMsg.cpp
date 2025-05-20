// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls_i_Tick_Deal_CmnSvrMsg.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CLSģ��Ĵ��������������ͨ��ָ��Ĳ���
//                PNGS��Pixel Network Game Structure����д
//                CLS��Connection Load Server����д�������ӷ��书���еĸ��ط�����
// CreationDate : 2005-08-11
// Change LOG   :

#include "../inc/pngs_cls_i.h"

using namespace n_pngs;

int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_CTRL(GZSUnit *pGZS, void *pCmd, int nSize)	// ���pGZSΪ���������GMS
{
	SVR_CLS_CTRL_T	*pCtrl	= (SVR_CLS_CTRL_T *)pCmd;
	switch( pCtrl->nSubCmd )
	{
	case	SVR_CLS_CTRL_T::SUBCMD_EXIT:
		{
			if( pGZS )
			{
				// ֱ�ӹرպ�GZS������
				closesocket(pGZS->sockGZS);
			}
			else
			{
				// ��Ȼ��GMS�����ģ��͸����ϲ�Ӧ�ý���������
				// ��������ܿ����˳���
				m_bShouldStop	= true;
			}
		}
		break;
	case	SVR_CLS_CTRL_T::SUBCMD_STRCMD:
		{
			// �����ִ�
			// ����ʱ����һ��
			char	cmd[64]		= "";
			char	param[1024]	= "";
			wh_strsplit("sa", (char*)&pCtrl->nParam, "", cmd, param);
			if( strcmp(cmd, "kickgzs")==0 )
			{
				// kickgzs gzsidx
				int	nIdx		= whstr2int(param);
				GZSUnit	*pGZS	= m_GZSs + nIdx;
				closesocket(pGZS->sockGZS);
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,TEMP)"GZS drop by cmd,%d", nIdx);
			}
			else if( strcmp(cmd, "set")==0 )
			{
				// key,val
				char	szKey[64]	= "";
				char	szVal[64]	= "";
				wh_strsplit("ss", param, ",", szKey, szVal);
				m_cfginfo.setvalue(szKey, szVal);
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,TEMP)"setvalue,%s=%s", szKey, szVal);
			}
			else
			{
				// ����ʶָ��˵�����������ƥ��������
				GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(,TEMP)"CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_CTRL,SVR_CLS_CTRL_T::SUBCMD_STRCMD,unknown cmd,%s", cmd);
			}
		}
		break;
	case	SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID:
	case	SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_AS_DROP:
	case	SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_AS_END:
		{
			int	nSvrIdx	= 0;
			if( pGZS )
			{
				nSvrIdx	= pGZS->nSvrIdx;
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1066,PLAYER_OUT)"KICK,%d,%d,0x%X", pCtrl->nSubCmd, nSvrIdx, pCtrl->nParam);
			// �߳��û�
			PlayerUnit	*pPlayer	= m_Players.getptr(pCtrl->nParam);
			if( pPlayer )
			{
				switch( pCtrl->nSubCmd )
				{
				case	SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_AS_DROP:
					{
						pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_AS_DROP;
					}
					break;
				case	SVR_CLS_CTRL_T::SUBCMD_KICKPALYERBYID_AS_END:
					{
						pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_END;
					}
					break;
				default:
					{
						pPlayer->nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_KICKED;
					}
					break;
				}
				RemovePlayerUnit(pPlayer);
			}
			else
			{
				// ֱ�Ӱ����û��Ѿ������ڷ��ͽ����GMS
				CLS_GMS_CLIENT_DROP_T	CD;
				CD.nCmd				= CLS_GMS_CLIENT_DROP;
				CD.nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTEXIST;
				CD.nClientID		= pCtrl->nParam;
				m_msgerGMS.SendMsg(&CD, sizeof(CD));
			}
		}
		break;
	case	SVR_CLS_CTRL_T::SUBCMD_DROPGZS:
		{
			// �رպ�GZS������
			GZSUnit	*pGZS	= m_GZSs + pCtrl->nParam;
			closesocket(pGZS->sockGZS);
		}
		break;
	case	SVR_CLS_CTRL_T::SUBCMD_KICKPLAYEROFGZS:
		{
			RemoveAllPlayerOfGZS(pCtrl->nParam);
		}
		break;
	case	SVR_CLS_CTRL_T::SUBCMD_CHECKPLAYEROFFLINE:
		{
			// �ж�һ����Щ�û�ID�Ѿ���������
			int	*panID	= (int *)wh_getptrnexttoptr(pCtrl);
			int i;
			// GMS�е���û�У������GMS�û��Ѿ�������
			for(i=0;i<pCtrl->nParam;i++)
			{
				int	nID	= panID[i];
				if( !m_Players.getptr(nID) )
				{
					CLS_GMS_CLIENT_DROP_T	CD;
					CD.nCmd				= CLS_GMS_CLIENT_DROP;
					CD.nRemoveReason	= CLS_GMS_CLIENT_DROP_T::REMOVEREASON_NOTEXIST;
					CD.nClientID		= nID;
					m_msgerGMS.SendMsg(&CD, sizeof(CD));
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(849,PNGS_RELOAD)"SVR_CLS_CTRL_T::SUBCMD_CHECKPLAYEROFFLINE Notify,0x%X", nID);
				}
			}
			// ���е�GMSû�У���Ҳ�߳�������Ժ���˵�����ܲ�����ֵģ�
		}
		break;
	case	SVR_CLS_CTRL_T::SUBCMD_SET_NAME:
		{
			PlayerUnit	*pPlayer	= m_Players.getptr(pCtrl->nParam);
			const char	*cszName	= (char *)wh_getptrnexttoptr(pCtrl);
			if( pPlayer )
			{
				WH_STRNCPY0(pPlayer->szName, cszName);
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1185,PLAYER_IN)"SVR_CLS_CTRL_T::SUBCMD_SET_NAME,0x%X,0x%X,%s", pPlayer, pCtrl->nParam, cszName);
		}
		break;
	case	SVR_CLS_CTRL_T::SUBCMD_PLAYER_STAT_ON:
		{
			CLS_CLIENT_NOTIFY_T	Notify;
			Notify.nCmd			= CLS_CLIENT_NOTIFY;
			Notify.nSubCmd		= CLS_CLIENT_NOTIFY_T::SUBCMD_STATON;
			Notify.data[0]		= (unsigned char)*(int *)wh_getptrnexttoptr(pCtrl);
			SendRawCmdToSinglePlayer(pCtrl->nParam, 0, &Notify, sizeof(Notify));
		}
		break;
	case	SVR_CLS_CTRL_T::SUBCMD_PLAYER_STAT_OFF:
		{
			CLS_CLIENT_NOTIFY_T	Notify;
			Notify.nCmd			= CLS_CLIENT_NOTIFY;
			Notify.nSubCmd		= CLS_CLIENT_NOTIFY_T::SUBCMD_STATOFF;
			SendRawCmdToSinglePlayer(pCtrl->nParam, 0, &Notify, sizeof(Notify));
		}
		break;
	}
	return	0;
}
int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG_TO_CLIENT(void *pCmd, int nSize)
{
	SVR_CLS_SET_TAG_TO_CLIENT_T	*pSetTag	= (SVR_CLS_SET_TAG_TO_CLIENT_T *)pCmd;
	// �ҵ����
	PlayerUnit	*pPlayer	= TryGetPlayerAndSendNotifyToGMSIsNotExisted(pSetTag->nClientID);
	if( !pPlayer )
	{
		// ��Ҳ�����
		return	0;
	}
	assert(pSetTag->nTagIdx<PlayerUnit::MAX_TAG_NUM);
	// Ϊ�˱������Ƴ�
	DelPlayerFromTagList(pPlayer, pSetTag->nTagIdx);
	// ������
	AddPlayerToTagList(pPlayer, pSetTag->nTagIdx, pSetTag->nTagVal);
	// ��¼��־
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1179,DBG)"SET_TAG_TO_CLIENT,%d,0x%X,%d,%d,%s", m_nCurCmdFromSvrIdx, pPlayer->nID, pSetTag->nTagIdx, pSetTag->nTagVal, pPlayer->szName);

	return	0;
}
int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_SET_PROP_TO_CLIENT(void *pCmd, int nSize)
{
	SVR_CLS_SET_PROP_TO_CLIENT_T	*pSet	= (SVR_CLS_SET_PROP_TO_CLIENT_T *)pCmd;
	// �ҵ����
	PlayerUnit	*pPlayer	= TryGetPlayerAndSendNotifyToGMSIsNotExisted(pSet->nClientID);
	if( !pPlayer )
	{
		// ��Ҳ�����
		return	0;
	}
	pPlayer->nProp			= pSet->nProp;
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(779,DBG)"CLS SetPlayerProp,0x%X,0x%X", pPlayer->nID, pPlayer->nProp);
	return	0;
}
int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd)
{
	SVR_CLS_CLIENT_DATA_T	*pClientData	= (SVR_CLS_CLIENT_DATA_T *)pCmd;
#ifdef	_DEBUG
	if( m_cfginfo.bLogSvrCmd )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(878,CMDLOG)"GZS2CLS,One,%d,%d,,%X,%X,%X,%X", nSendCmd, nSize, (unsigned char)pClientData->data[0], (unsigned char)pClientData->data[1], (unsigned char)pClientData->data[2], (unsigned char)pClientData->data[3]);
	}
#endif
	// ����ҷ�������
	// ��Ҫ��¼������־
	return	SendDataToSinglePlayer(m_Players.getptr(pClientData->nClientID), pClientData->nChannel, pClientData->data, nSize - wh_offsetof(SVR_CLS_CLIENT_DATA_T, data), nSendCmd);
}
int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd)
{
	SVR_CLS_MULTICLIENT_DATA_T	*pMData		= (SVR_CLS_MULTICLIENT_DATA_T *)pCmd;
	// ������ݲ��ֵĳ���
	int		nDSize	= pMData->GetDSize(nSize);
	void	*pData	= pMData->GetDataPtr();

	// ������������GZS�����������ݵ���cls崵�
	if(nDSize<0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(1093,PNGS_RT)"CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA,bad size,%d,%d,%d,%d", nSize, nDSize, pMData->nChannel, pMData->nClientNum);
		return	-1;
	}
	if(pMData->nChannel>=CNL2_CHANNELNUM_MAX)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(1093,PNGS_RT)"CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_MULTICLIENT_DATA,bad channel,%d,%d,%d,%d", nSize, nDSize, pMData->nChannel, pMData->nClientNum);
		return	-2;
	}

	// �������û������ݶ���һ���ģ�����׼��һ�μ���
	m_vectrawbuf.resize(nDSize + wh_offsetof(CLS_CLIENT_DATA_T, data));
	CLS_CLIENT_DATA_T	*pDataToClient	= (CLS_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	pDataToClient->nCmd	= nSendCmd;
	memcpy(pDataToClient->data, pData, nDSize);

#ifdef	_DEBUG
	if( m_cfginfo.bLogSvrCmd )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(879,CMDLOG)"GZS2CLS,Multi,%d,%d,%d,%X,%X,%X,%X", nSendCmd, nSize, pMData->nClientNum, (unsigned char)pDataToClient->data[0], (unsigned char)pDataToClient->data[1], (unsigned char)pDataToClient->data[2], (unsigned char)pDataToClient->data[3]);
	}
#endif

	// �����б�����
	for(int i=0;i<pMData->nClientNum;i++)
	{
		SendRawCmdToSinglePlayer(m_Players.getptr(pMData->anClientID[i]), pMData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return	0;
}
int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd)
{
	SVR_CLS_TAGGED_CLIENT_DATA_T	*pTaggdClientData	= (SVR_CLS_TAGGED_CLIENT_DATA_T *)pCmd;
	assert(pTaggdClientData->nTagIdx<PlayerUnit::MAX_TAG_NUM);
	// �õ�hash��ֵ
	unsigned int	nHashKey	= CreateHashKey(pTaggdClientData->nTagIdx, pTaggdClientData->nTagVal);
	// ���Ҷ��У����û�оͷ���
	DLPUNIT_T		*pDLU;
	if( !m_mapTag2DL.get(nHashKey, &pDLU) )
	{
		// û��
		return	0;
	}
	// �������tag��ص��б�����
	int	nDSize	= nSize - wh_offsetof(SVR_CLS_TAGGED_CLIENT_DATA_T, data);
	assert(nDSize>=0);

	// �������û������ݶ���һ���ģ�����׼��һ�μ���
	m_vectrawbuf.resize(nDSize + wh_offsetof(CLS_CLIENT_DATA_T, data));
	CLS_CLIENT_DATA_T	*pDataToClient	= (CLS_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	pDataToClient->nCmd	= nSendCmd;
	memcpy(pDataToClient->data, pTaggdClientData->data, nDSize);

	// �ж�һ���Ƿ���svr���
	if( PNGS_SVRIDX_AUTOGZS == pTaggdClientData->nSvrIdx )
	{
		pTaggdClientData->nSvrIdx	= m_nCurCmdFromSvrIdx;
	}

	DLPLAYER_NODE_T	*pNode	= pDLU->pDLP->begin();
	while( pNode != pDLU->pDLP->end() )
	{
		// ����һ����һ�����ָ�룬������SendRawCmdToSinglePlayer�п��ܵ����û���ɾ����pNode����������
		DLPLAYER_NODE_T	*pNodeNext	= pNode->next;

		switch( pTaggdClientData->nSvrIdx )
		{
		case	PNGS_SVRIDX_GMS:
			{
				// ��������������͵�
				assert(0);
			}
			break;
		case	PNGS_SVRIDX_ALLGZS:
			{
				// ȫ��������
				SendRawCmdToSinglePlayer(pNode->data, pTaggdClientData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
			break;
		case	PNGS_SVRIDX_ALLGZSEXCEPTME:
			{
				// ֻ�Գ����Լ��ķ���
				if( pTaggdClientData->nSvrIdx != pNode->data->nSvrIdx )
				{
					// ����
					SendRawCmdToSinglePlayer(pNode->data, pTaggdClientData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
			}
			break;
		default:
			{
				// ֻ�Է��ϵķ���
				if( pTaggdClientData->nSvrIdx == pNode->data->nSvrIdx )
				{
					// ����
					SendRawCmdToSinglePlayer(pNode->data, pTaggdClientData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
			}
			break;
		}
		// ��һ��
		pNode	= pNodeNext;
	}
	return	0;
}
int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_ALL_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd)
{
	SVR_CLS_ALL_CLIENT_DATA_T	*pAllClientData	= (SVR_CLS_ALL_CLIENT_DATA_T *)pCmd;
	// ���������û�������
	int	nDSize	= nSize - wh_offsetof(SVR_CLS_ALL_CLIENT_DATA_T, data);
	assert(nDSize>=0);

	// �������û������ݶ���һ���ģ�����׼��һ�μ���
	m_vectrawbuf.resize(nDSize + wh_offsetof(CLS_CLIENT_DATA_T, data));
	CLS_CLIENT_DATA_T	*pDataToClient	= (CLS_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	pDataToClient->nCmd	= nSendCmd;
	memcpy(pDataToClient->data, pAllClientData->data, nDSize);

	if( PNGS_SVRIDX_AUTOGZS == pAllClientData->nSvrIdx )
	{
		pAllClientData->nSvrIdx	= m_nCurCmdFromSvrIdx;
	}
	if( pAllClientData->nSvrIdx>0 && pAllClientData->nSvrIdx<=GZS_MAX_NUM )
	{
		SendRawCmdToAllPlayerOfGZS(pAllClientData->nSvrIdx, pAllClientData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	else
	{
		SendRawCmdToAllPlayer(pAllClientData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return	0;
}
int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_SET_TAG64_TO_CLIENT(void *pCmd, int nSize)
{
	SVR_CLS_SET_TAG64_TO_CLIENT_T	*pSetTag	= (SVR_CLS_SET_TAG64_TO_CLIENT_T *)pCmd;
	// �ҵ����
	PlayerUnit	*pPlayer	= TryGetPlayerAndSendNotifyToGMSIsNotExisted(pSetTag->nClientID);
	if( !pPlayer )
	{
		// ��Ҳ�����
		return	0;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(1180,DBG)"SET_TAG64_TO_CLIENT,%d,0x%X,0x%"WHINT64PRFX"X,%d,%s", m_nCurCmdFromSvrIdx, pPlayer->nID, pSetTag->nTag, pSetTag->bDel, pPlayer->szName);
	// ֱ�����루��Ϊ�����ظ��Ļ�ֱ�ӷ��أ�
	if( pSetTag->bDel )
	{
		DelPlayerFrom64TagList(pPlayer, pSetTag->nTag);
	}
	else
	{
		AddPlayerToTag64List(pPlayer, pSetTag->nTag);
	}
	return	0;
}
int		CLS2_I::Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED64_CLIENT_DATA(void *pCmd, int nSize, pngs_cmd_t nSendCmd)	// �����Tick_Deal_CmnSvrMsg_SVR_CLS_TAGGED_CLIENT_DATA�����ݻ���һֱ
{
	SVR_CLS_TAGGED64_CLIENT_DATA_T	*pTaggdClientData	= (SVR_CLS_TAGGED64_CLIENT_DATA_T *)pCmd;
	// ���Ҷ��У����û�оͷ���
	DLPUNIT_T		*pDLU;
	if( !m_map64Tag2DL.get(pTaggdClientData->nTag, &pDLU) )
	{
		// û��
		return	0;
	}
	// �������tag��ص��б�����
	int	nDSize	= nSize - wh_offsetof(SVR_CLS_TAGGED64_CLIENT_DATA_T, data);
	assert(nDSize>=0);

	// �������û������ݶ���һ���ģ�����׼��һ�μ���
	m_vectrawbuf.resize(nDSize + wh_offsetof(CLS_CLIENT_DATA_T, data));
	CLS_CLIENT_DATA_T	*pDataToClient	= (CLS_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	pDataToClient->nCmd	= nSendCmd;
	memcpy(pDataToClient->data, pTaggdClientData->data, nDSize);

	// �ж�һ���Ƿ���svr���
	if( PNGS_SVRIDX_AUTOGZS == pTaggdClientData->nSvrIdx )
	{
		pTaggdClientData->nSvrIdx	= m_nCurCmdFromSvrIdx;
	}

	DLPLAYER_NODE_T	*pNode	= pDLU->pDLP->begin();
	while( pNode != pDLU->pDLP->end() )
	{
		// ����һ����һ�����ָ�룬������SendRawCmdToSinglePlayer�п��ܵ����û���ɾ����pNode����������
		DLPLAYER_NODE_T	*pNodeNext	= pNode->next;

		switch( pTaggdClientData->nSvrIdx )
		{
		case	PNGS_SVRIDX_GMS:
			{
				// ��������������͵�
				assert(0);
			}
			break;
		case	PNGS_SVRIDX_ALLGZS:
			{
				// ȫ��������
				SendRawCmdToSinglePlayer(pNode->data, pTaggdClientData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
			}
			break;
		case	PNGS_SVRIDX_ALLGZSEXCEPTME:
			{
				// ֻ�Գ����Լ��ķ���
				if( pTaggdClientData->nSvrIdx != pNode->data->nSvrIdx )
				{
					// ����
					SendRawCmdToSinglePlayer(pNode->data, pTaggdClientData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
			}
			break;
		default:
			{
				// ֻ�Է��ϵķ���
				if( pTaggdClientData->nSvrIdx == pNode->data->nSvrIdx )
				{
					// ����
					SendRawCmdToSinglePlayer(pNode->data, pTaggdClientData->nChannel, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
				}
			}
			break;
		}
		// ��һ��
		pNode	= pNodeNext;
	}
	return	0;
}
