#include "../inc/CLS4Web_i.h"

using namespace n_pngs;

int		CLS4Web_i::Tick_Deal_CmnSvrMsg_SVR_CLS4Web_CLIENT_DATA(void* pCmd, size_t nSize)
{
	SVR_CLS4Web_CLIENT_DATA_T*	pClientData	= (SVR_CLS4Web_CLIENT_DATA_T*)pCmd;
	return SendDataToSinglePlayer(m_Players.getptr(pClientData->nClientID), pClientData->data, nSize-wh_offsetof(SVR_CLS4Web_CLIENT_DATA_T,data), CLS4Web_CLIENT_DATA);
}
int		CLS4Web_i::Tick_Deal_CmnSvrMsg_SVR_CLS4Web_MULTICLIENT_DATA(void* pCmd, size_t nSize)
{
	SVR_CLS4Web_MULTICLIENT_DATA_T*	pMData	= (SVR_CLS4Web_MULTICLIENT_DATA_T*)pCmd;
	// 获取数据部分的指针和长度
	int		nDSize	= pMData->GetDSize(nSize);
	void*	pData	= pMData->GetDataPtr();

	if (nDSize < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,CLS4Web_RT_TMP)"SVR_CLS4Web_MULTICLIENT_DATA_T,bad size,%d,%d,%d", nSize, nDSize, pMData->nClientNum);
		return -1;
	}

	// 拼原始包,不用SendDataToSinglePlayer(这样会把同一个包拼nClientNum次)
	m_vectrawbuf.resize(nDSize + wh_offsetof(CLS4Web_CLIENT_DATA_T, data));
	CLS4Web_CLIENT_DATA_T*	pDataToClient	= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pDataToClient->nCmd		= CLS4Web_CLIENT_DATA;
	memcpy(pDataToClient->data, pData, nDSize);

	for (int i=0; i<pMData->nClientNum; i++)
	{
		SendRawCmdToSinglePlayer(m_Players.getptr(pMData->anClientID[i]), m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return 0;
}
int		CLS4Web_i::Tick_Deal_CmnSvrMsg_SVR_CLS4Web_SET_TAG_TO_CLIENT(void* pCmd, int nSize)
{
	SVR_CLS4Web_SET_TAG_TO_CLIENT_T*	pSetTag	= (SVR_CLS4Web_SET_TAG_TO_CLIENT_T*)pCmd;
	// 找到玩家
	PlayerUnit*	pPlayer		= TryGetPlayerAndSendNotifyToLPIsNotExisted(pSetTag->nClientID);
	if (pPlayer == NULL)
	{
		return 0;
	}
	if (pSetTag->nTagIdx >= PlayerUnit::MAX_TAG_NUM)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,CLS4Web_RT_TMP)"SVR_CLS4Web_SET_TAG_TO_CLIENT_T,err tag idx,%d,%d", (int)pSetTag->nTagIdx, PlayerUnit::MAX_TAG_NUM);
		return -1;
	}
	// 为了保险先移出
	DelPlayerFromTagList(pPlayer, pSetTag->nTagIdx);
	// 再移入
	AddPlayerToTagList(pPlayer, pSetTag->nTagIdx, pSetTag->nTagVal);
	// 记录日志
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,CLS4Web_RT)"SVR_CLS4Web_SET_TAG_TO_CLIENT_T,0x%X,%d,%d,%s", pPlayer->nID, pSetTag->nTagIdx, pSetTag->nTagVal, pPlayer->szName);
	return 0;
}
int		CLS4Web_i::Tick_Deal_CmnSvrMsg_SVR_CLS4Web_TAGGED_CLIENT_DATA(void* pCmd, size_t nSize)
{
	SVR_CLS4Web_TAGGED_CLIENT_DATA_T*	pTaggedClientData	= (SVR_CLS4Web_TAGGED_CLIENT_DATA_T*)pCmd;
	unsigned int	nHashKey	= CreateHashKey(pTaggedClientData->nTagIdx, pTaggedClientData->nTagVal);
	// 查找队列,如果没有就放弃
	DLPUNIT_T*		pDLU		= NULL;
	if (!m_mapTag2DL.get(nHashKey, &pDLU))
	{
		return 0;
	}
	
	// 拼包
	int	nDSize		= nSize - wh_offsetof(SVR_CLS4Web_TAGGED_CLIENT_DATA_T, data);
	if (nDSize < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,CLS4Web_RT_TMP)"SVR_CLS4Web_TAGGED_CLIENT_DATA_T,bad size,%d,%d", nSize, nDSize);
		return -1;
	}
	m_vectrawbuf.resize(nDSize + wh_offsetof(CLS4Web_CLIENT_DATA_T, data));
	// 遍历这个tag相关的列表
	CLS4Web_CLIENT_DATA_T*	pDataToClient	= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pDataToClient->nCmd			= CLS4Web_CLIENT_DATA;
	memcpy(pDataToClient->data, pTaggedClientData->data, nDSize);

	DLPLAYER_NODE_T*	pNode	= pDLU->pDLP->begin();
	while (pNode != pDLU->pDLP->end())
	{
		// 保存一下下一个结点指针,SendRawCmdToSinglePlayer可能导致pNode失效
		DLPLAYER_NODE_T*	pNextNode		= pNode->next;
		SendRawCmdToSinglePlayer(pNode->data, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
		pNode					= pNextNode;
	}

	return 0;
}
int		CLS4Web_i::Tick_Deal_CmnSvrMsg_SVR_CLS4Web_ALL_CLIENT_DATA(void* pCmd, size_t nSize)
{
	SVR_CLS4Web_ALL_CLIENT_DATA_T*	pAllClientData	= (SVR_CLS4Web_ALL_CLIENT_DATA_T*)pCmd;
	// 遍历所有用户,发送
	int	nDSize		= nSize - wh_offsetof(SVR_CLS4Web_ALL_CLIENT_DATA_T, data);
	if (nDSize < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,CLS4Web_RT_TMP)"SVR_CLS4Web_ALL_CLIENT_DATA_T,bad size,%d,%d", nSize, nDSize);
		return -1;
	}

	// 给所有用户的数据都是一样的,所以准备一次即可
	m_vectrawbuf.resize(nDSize + wh_offsetof(CLS4Web_CLIENT_DATA_T, data));
	CLS4Web_CLIENT_DATA_T*	pDataToClient	= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pDataToClient->nCmd		= CLS4Web_CLIENT_DATA;
	memcpy(pDataToClient->data, pAllClientData->data, nDSize);
	SendRawCmdToAllPlayer(m_vectrawbuf.getbuf(), m_vectrawbuf.size());

	return 0;
}
int		CLS4Web_i::Tick_Deal_CmnSvrMsg_SVR_CLS4Web_SET_TAG64_TO_CLIENT(void* pCmd, size_t nSize)
{
	SVR_CLS4Web_SET_TAG64_TO_CLIENT_T*	pSetTag	= (SVR_CLS4Web_SET_TAG64_TO_CLIENT_T*)pCmd;
	// 找到玩家
	PlayerUnit*	pPlayer		= TryGetPlayerAndSendNotifyToLPIsNotExisted(pSetTag->nClientID);
	if (pPlayer == NULL)
	{
		return 0;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,CLS4Web_RT)"SET_TAG64_TO_CLIENT,0x%X,0x%"WHINT64PRFX"X,%d,%s", pPlayer->nID, pSetTag->nTag, pSetTag->bDel, pPlayer->szName);
	// 直接移入
	if (pSetTag->bDel)
	{
		DelPlayerFromTag64List(pPlayer, pSetTag->nTag);
	}
	else
	{
		AddPlayerToTag64List(pPlayer, pSetTag->nTag);
	}
	return 0;
}
int		CLS4Web_i::Tick_Deal_CmnSvrMsg_SVR_CLS4Web_TAGGED64_CLIENT_DATA(void* pCmd, size_t nSize)
{
	SVR_CLS4Web_TAGGED64_CLIENT_DATA_T*	pTaggedClientData	= (SVR_CLS4Web_TAGGED64_CLIENT_DATA_T*)pCmd;
	// 查找队列,如果没有就放弃
	DLPUNIT_T*		pDLU	= NULL;
	if (!m_map64Tag2DL.get(pTaggedClientData->nTag, &pDLU))
	{
		return 0;
	}
	// 拼包
	int	nDSize	= nSize - wh_offsetof(SVR_CLS4Web_TAGGED64_CLIENT_DATA_T, data);
	if (nDSize < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,CLS4Web_RT_TMP)"SVR_CLS4Web_TAGGED64_CLIENT_DATA_T,bad size,%d,%d", nSize, nDSize);
		return -1;
	}
	m_vectrawbuf.resize(nDSize + wh_offsetof(CLS4Web_CLIENT_DATA_T, data));
	CLS4Web_CLIENT_DATA_T*	pDataToClient	= (CLS4Web_CLIENT_DATA_T*)m_vectrawbuf.getbuf();
	pDataToClient->nCmd			= CLS4Web_CLIENT_DATA;
	memcpy(pDataToClient->data, pTaggedClientData->data, nDSize);
	// 遍历这个tag相关的列表,发送
	DLPLAYER_NODE_T*	pNode	= pDLU->pDLP->begin();
	while (pNode != pDLU->pDLP->end())
	{
		// 保存一下下一个结点指针,SendRawCmdToSinglePlayer可能导致结点失效
		DLPLAYER_NODE_T*	pNodeNext	= pNode->next;
		SendRawCmdToSinglePlayer(pNode->data, m_vectrawbuf.getbuf(),  m_vectrawbuf.size());
		pNode	= pNodeNext;
	}
	return 0;
}
