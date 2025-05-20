// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs_i_GZS_MAINSTRUCTURE_Send.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ���ڲ���GZS_MAINSTRUCTUREģ������з��Ͳ�����������ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Master Server����д�����߼��������е��ܿط�����
//                ��ΪRemoveMYCNTR�ᵼ��BeforeDeleteConnecter�ĵ��ã����԰����Ƿ���һ���ļ���
// CreationDate : 2005-09-22
// Change LOG   :

#include "../inc/pngs_gzs_i_GZS_MAINSTRUCTURE.h"

using namespace n_pngs;

int		GZS_MAINSTRUCTURE::SendCmdToAllCLS(const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	for(whunitallocatorFixed<CLSUnit>::iterator it=m_CLSs.begin(); it!=m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, pCmd, nSize);
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::SendCmdToPlayerCLS(int nClientID, const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	return	SendCmdToPlayerCLS(m_Players.getptr(nClientID), pCmd, nSize);
}
int		GZS_MAINSTRUCTURE::SendCmdToPlayerCLS(PlayerUnit *pPlayer, const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	if( !pPlayer )
	{
		return	-1;
	}
	CLSUnit	*pCLS	= m_CLSs.getptr(pPlayer->nCLSID);
	if( pCLS )
	{
		pCLS->pCntr->SendMsg(pCmd, nSize);
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::SendCmdToPlayer(int nPlayerID, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	return	SendCmdToPlayer(m_Players.getptr(nPlayerID), nChannel, pCmd, nSize);
}
int		GZS_MAINSTRUCTURE::SendCmdToPlayer(GZS_MAINSTRUCTURE::PlayerUnit *pPlayer, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	if( !pPlayer )
	{
		return	-1;
	}

	pngs_cmd_t		nCmd	= SVR_CLS_CLIENT_DATA;
	if( nSize==0 )
	{
		// ˵��Ӧ�÷���PACK��ָ��
		m_wcsCmdPack.WriteEnd();
		nSize				= m_wcsCmdPack.GetShrinkSize();
		pCmd				= m_wcsCmdPack.GetShrinkBuf();
		nCmd				= SVR_CLS_CLIENT_DATA_PACK;

		// ����
		CmdPackReset();
	}
	else if( m_cpb.nSendCmd == GZSPACKET_2MS_CMD2ONEPLAYER )
	{
		// ѹ��������
		if( AddPackData(pCmd, nSize)<0 )
		{
			return	-1;
		}
	}
	else
	{
		// ȷ��������Ҫ�����
		assert( m_cpb.nSendCmd == GZSPACKET_2MS_0 );
	}

	// �ҵ�����CLS
	CLSUnit	*pCLS	= m_CLSs.getptr(pPlayer->nCLSID);
	// CLS����Ӧ�ò��������֮ǰ��ʧ�ģ�������CLS��GZS���ˣ����Ǻ�GMSû�жϣ�Ȼ��GMS����֪ͨGZS�û������ˣ�
	if( pCLS == NULL )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(810,PNGS_LOST)"GZS_MAINSTRUCTURE::SendCmdToPlayer,CLS NULL,0x%X,0x%X,%d,%d", pPlayer->nCLSID, pPlayer->nID, m_cfginfo.nSvrIdx, nSize);
		TellGMSToKickPlayerAndRemovePlayer(pPlayer->nID);
		return	-100;
	}
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS_CLIENT_DATA_T, data) + nSize);
	SVR_CLS_CLIENT_DATA_T	&ClientData	= *(SVR_CLS_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	ClientData.nCmd			= nCmd;
	ClientData.nChannel		= nChannel;
	ClientData.nClientID	= pPlayer->nID;
	memcpy(ClientData.data, pCmd, nSize);
	if( pCLS->pCntr->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size())<0 )
	{
		return	-2;
	}
	return	0;
}

namespace
{

struct PlayerCollect
{
    PlayerCollect*  m_next;
    size_t  m_size;
    int m_cls;
    int m_ids[1];
};

} // namespace

int		GZS_MAINSTRUCTURE::SendCmdToMultiPlayer(int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
#if 1
    PlayerCollect* pc = NULL;
    for(int i = 0; i < nPlayerNum; ++ i)
    {
		PlayerUnit* pPlayer = m_Players.getptr(paPlayerID[i]);
        if(pPlayer == 0) continue;
        // ͨ��һ��ѭ���ҵ����ʵ�cls����Ϊ���࣬ʹ��ѭ����������
        PlayerCollect* p = pc;
        for(;;)
        {
            if(p == 0)
            {
                // û���ҵ��������µ�
                p = (PlayerCollect*)::malloc(sizeof(PlayerCollect)+sizeof(int)*(nPlayerNum-i-1));
                p->m_next = pc;
                pc = p;
                p->m_size = 0;
                p->m_cls = pPlayer->nCLSID;
            }
            else if(p->m_cls != pPlayer->nCLSID)
            {
                // ������ǣ�����һ��
                p = p->m_next;
                continue;
            }
            // �ҵ��ˣ������ڲ�ѭ���������¸���ɫ
            p->m_ids[p->m_size++] = pPlayer->nID;
            break;
        }
    }

    while(pc)
    {
		SendCmdToMultiPlayerInOneCLS(pc->m_cls, pc->m_ids, pc->m_size, nChannel, pCmd, nSize);
        PlayerCollect* t = pc;
        pc = pc->m_next;
        ::free(t);
    }
#else
	// �ֱ����������Щ��ҷֱ�������ЩCLS��Ȼ���CLS����
	std::multimap<int, int>	mmCLS2Player;
	int	i;
	for(i=0;i<nPlayerNum;i++)
	{
		PlayerUnit	*pPlayer	= m_Players.getptr(paPlayerID[i]);
		if( pPlayer )
		{
			mmCLS2Player.insert( std::make_pair(pPlayer->nCLSID, pPlayer->nID) );
		}
	}

	// Ԥ���㹻�ĳ���
	whvector<int>		vect;
	vect.reserve(nPlayerNum);

	int	nCLSID			= 0;
	std::multimap<int, int>::iterator	it	= mmCLS2Player.begin();
	while( it!=mmCLS2Player.end() )
	{
		if( (*it).first != nCLSID )
		{
			// ���ǰ���ж�������
			if( nCLSID>0 )
			{
				SendCmdToMultiPlayerInOneCLS(nCLSID, vect.getbuf(), vect.size(), nChannel, pCmd, nSize);
			}
			// ��ʼ�µ�һ��
			nCLSID	= (*it).first;
			vect.clear();
		}
		vect.push_back((*it).second);
		++	it;
	}
	if( nCLSID>0 )
	{
		// ˵�������һ���黹û�з���
		SendCmdToMultiPlayerInOneCLS(nCLSID, vect.getbuf(), vect.size(), nChannel, pCmd, nSize);
	}
#endif
	return	0;
}
int		GZS_MAINSTRUCTURE::SendCmdToMultiPlayerInOneCLS(int nCLSID, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	return	SendCmdToMultiPlayerInOneCLS(m_CLSs.getptr(nCLSID), paPlayerID, nPlayerNum, nChannel, pCmd, nSize);
}
int		GZS_MAINSTRUCTURE::SendCmdToMultiPlayerInOneCLS(CLSUnit *pCLS, int *paPlayerID, int nPlayerNum, unsigned char nChannel, const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	if( !pCLS )
	{
		return	-1;
	}

	pngs_cmd_t		nCmd	= SVR_CLS_MULTICLIENT_DATA;
	if( nSize==0 )
	{
		// ˵��Ӧ�÷���PACK��ָ��
		m_wcsCmdPack.WriteEnd();
		nSize				= m_wcsCmdPack.GetShrinkSize();
		pCmd				= m_wcsCmdPack.GetShrinkBuf();
		nCmd				= SVR_CLS_MULTICLIENT_DATA_PACK;

		// ����
		CmdPackReset();
	}
	else if( m_cpb.nSendCmd == GZSPACKET_2MS_CMD2MULTIPLAYER )
	{
		// ѹ��������
		if( AddPackData(pCmd, nSize)<0 )
		{
			return	-1;
		}
	}
	else
	{
		// ȷ��������Ҫ�����
		assert( m_cpb.nSendCmd == GZSPACKET_2MS_0 );
	}

	m_vectrawbuf.resize(SVR_CLS_MULTICLIENT_DATA_T::GetTotalSize(nPlayerNum, nSize));
	SVR_CLS_MULTICLIENT_DATA_T	&Cmd	= *(SVR_CLS_MULTICLIENT_DATA_T *)m_vectrawbuf.getbuf();
	Cmd.nCmd			= nCmd;
	Cmd.nChannel		= nChannel;
	Cmd.nClientNum		= nPlayerNum;
	memcpy(Cmd.anClientID, paPlayerID, sizeof(int)*nPlayerNum);
	memcpy(Cmd.GetDataPtr(), pCmd, nSize);

	if( pCLS->pCntr->SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size())<0 )
	{
		return	-2;
	}

	return	0;
}
int		GZS_MAINSTRUCTURE::SendCmdToAllPlayer(unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	pngs_cmd_t		nCmd	= SVR_CLS_ALL_CLIENT_DATA;
	if( nSize==0 )
	{
		// ˵��Ӧ�÷���PACK��ָ��
		m_wcsCmdPack.WriteEnd();
		nSize				= m_wcsCmdPack.GetShrinkSize();
		pCmd				= m_wcsCmdPack.GetShrinkBuf();
		nCmd				= SVR_CLS_ALL_CLIENT_DATA_PACK;

		// ����
		CmdPackReset();
	}
	else if( m_cpb.nSendCmd == GZSPACKET_2MS_CMD2ALLPLAYER )
	{
		// ѹ��������
		if( AddPackData(pCmd, nSize)<0 )
		{
			return	-1;
		}
	}
	else
	{
		// ȷ��������Ҫ�����
		assert( m_cpb.nSendCmd == GZSPACKET_2MS_0 );
	}

	// ֱ�ӷ��͸�����CLS�������ǹ㲥
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS_ALL_CLIENT_DATA_T, data) + nSize);
	SVR_CLS_ALL_CLIENT_DATA_T	&Cmd	= *(SVR_CLS_ALL_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	Cmd.nCmd		= nCmd;
	Cmd.nChannel	= nChannel;
	Cmd.nSvrIdx		= nSvrIdx;
	memcpy(Cmd.data, pCmd, nSize);
	for(whunitallocatorFixed<CLSUnit>::iterator it=m_CLSs.begin(); it!=m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::SendCmdToAllPlayerByTag(unsigned char nTagIdx, short nTagVal, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	pngs_cmd_t		nCmd	= SVR_CLS_TAGGED_CLIENT_DATA;
	if( nSize==0 )
	{
		// ˵��Ӧ�÷���PACK��ָ��
		m_wcsCmdPack.WriteEnd();
		nSize				= m_wcsCmdPack.GetShrinkSize();
		pCmd				= m_wcsCmdPack.GetShrinkBuf();
		nCmd				= SVR_CLS_TAGGED_CLIENT_DATA_PACK;

		// ����
		CmdPackReset();
	}
	else if( m_cpb.nSendCmd == GZSPACKET_2MS_CMD2PLAYERWITHTAG )
	{
		// ѹ��������
		if( AddPackData(pCmd, nSize)<0 )
		{
			return	-1;
		}
	}
	else
	{
		// ȷ��������Ҫ�����
		assert( m_cpb.nSendCmd == GZSPACKET_2MS_0 );
	}

	// ֱ�ӷ��͸�����CLS�������ǰ�tag����
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS_TAGGED_CLIENT_DATA_T, data) + nSize);
	SVR_CLS_TAGGED_CLIENT_DATA_T	&Cmd	= *(SVR_CLS_TAGGED_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	Cmd.nCmd		= nCmd;
	Cmd.nSvrIdx		= nSvrIdx;
	Cmd.nChannel	= nChannel;
	Cmd.nTagIdx		= nTagIdx;
	Cmd.nTagVal		= nTagVal;
	memcpy(Cmd.data, pCmd, nSize);
	for(whunitallocatorFixed<CLSUnit>::iterator it=m_CLSs.begin(); it!=m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::SendCmdToAllPlayerByTag64(whuint64 nTag, unsigned char nChannel, const void *pCmd, size_t nSize, unsigned char nSvrIdx)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	pngs_cmd_t		nCmd	= SVR_CLS_TAGGED64_CLIENT_DATA;
	if( nSize==0 )
	{
		// ˵��Ӧ�÷���PACK��ָ��
		m_wcsCmdPack.WriteEnd();
		nSize				= m_wcsCmdPack.GetShrinkSize();
		pCmd				= m_wcsCmdPack.GetShrinkBuf();
		nCmd				= SVR_CLS_TAGGED64_CLIENT_DATA_PACK;

		// ����
		CmdPackReset();
	}
	else if( m_cpb.nSendCmd == GZSPACKET_2MS_CMD2PLAYERWITHTAG64 )
	{
		// ѹ��������
		if( AddPackData(pCmd, nSize)<0 )
		{
			return	-1;
		}
	}
	else
	{
		// ȷ��������Ҫ�����
		assert( m_cpb.nSendCmd == GZSPACKET_2MS_0 );
	}

	// ֱ�ӷ��͸�����CLS�������ǰ�tag����
	m_vectrawbuf.resize(wh_offsetof(SVR_CLS_TAGGED64_CLIENT_DATA_T, data) + nSize);
	SVR_CLS_TAGGED64_CLIENT_DATA_T	&Cmd	= *(SVR_CLS_TAGGED64_CLIENT_DATA_T *)m_vectrawbuf.getbuf();
	Cmd.nCmd		= nCmd;
	Cmd.nSvrIdx		= nSvrIdx;
	Cmd.nChannel	= nChannel;
	Cmd.nTag		= nTag;
	memcpy(Cmd.data, pCmd, nSize);
	for(whunitallocatorFixed<CLSUnit>::iterator it=m_CLSs.begin(); it!=m_CLSs.end(); ++it)
	{
		SafeSendMsg((*it).pCntr, m_vectrawbuf.getbuf(), m_vectrawbuf.size());
	}
	return	0;
}
int		GZS_MAINSTRUCTURE::SetPlayerTag(int nClientID, unsigned char nTagIdx, short nTagVal)
{
	SVR_CLS_SET_TAG_TO_CLIENT_T	Cmd;
	Cmd.nCmd		= SVR_CLS_SET_TAG_TO_CLIENT;
	Cmd.nTagIdx		= nTagIdx;
	Cmd.nTagVal		= nTagVal;
	Cmd.nClientID	= nClientID;
	return	SendCmdToPlayerCLS(nClientID, &Cmd, sizeof(Cmd));
}
int		GZS_MAINSTRUCTURE::SetPlayerTag64(int nClientID, whuint64 nTag, bool bDel)
{
	SVR_CLS_SET_TAG64_TO_CLIENT_T	Cmd;
	Cmd.nCmd		= SVR_CLS_SET_TAG64_TO_CLIENT;
	Cmd.nTag		= nTag;
	Cmd.bDel		= bDel;
	Cmd.nClientID	= nClientID;
	return	SendCmdToPlayerCLS(nClientID, &Cmd, sizeof(Cmd));
}

int		GZS_MAINSTRUCTURE::SendSvrCmdToSvrBySvrIdx(unsigned char nSvrIdx, const void *pCmd, size_t nSize)
{
	assert(nSize<PNGS_RAWBUF_SIZE);
	// ��װһ��ָ��͸�GMS
	m_vectrawbuf.resize( wh_offsetof(GZS_GMS_GAMECMD_T, data) + nSize );
	GZS_GMS_GAMECMD_T	*pGameCmd	= (GZS_GMS_GAMECMD_T *)m_vectrawbuf.getbuf();
	pGameCmd->nCmd		= GZS_GMS_GAMECMD;
	pGameCmd->nToSvrIdx	= nSvrIdx;
	memcpy(pGameCmd->data, pCmd, nSize);
	return	m_msgerGMS.SendMsg(m_vectrawbuf.getbuf(), m_vectrawbuf.size());
}

int		GZS_MAINSTRUCTURE::AddPackData(const void *pData, size_t nDSize)
{
	try
	{
		m_wcsCmdPack.WriteVData(pData, nDSize);
	}
	catch( const char *cszErr )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(676,PNGS_RT)"GZS_MAINSTRUCTURE::AddPackData,%s,WriteVData,%d", cszErr, nDSize);
		return	-1;
	}
	return	0;
}
