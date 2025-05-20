// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i_TEDeal.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CNL2���ڲ�ʵ��
// CreationDate : 2005-04-21
// ChangeLog    : 2008-07-29 �����·��͵ĵȴ�ʱ����1�����������Լ��ٴ�����ط���

#include "../inc/whnetcnl2_i.h"

using namespace	n_whnet;

namespace
{
	struct	AUTH_CFGINFO_T		: public whdataini::obj
	{
		char	szName[64];		// ��Ӫ�̵�����
		char	szIP[64];		// �󶨵ĵ�ַ

		AUTH_CFGINFO_T()
		{
			szName[0]	= 0;
			szIP[0]		= 0;
		}

		WHDATAPROP_DECLARE_MAP(AUTH_CFGINFO_T);
	};

	WHDATAPROP_MAP_BEGIN_AT_ROOT(AUTH_CFGINFO_T)
		WHDATAPROP_ON_SETVALUE_smp(charptr, szName, 0)
		WHDATAPROP_ON_SETVALUE_smp(charptr, szIP, 0)
	WHDATAPROP_MAP_END()
}

int		CNL2SlotMan_I_UDP::TickLogic_TE()
{
	maketicknow();
	// ����ʱ��������Ƿ��ж���
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_tq.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// ���������������ʱ���¼�����������������ŵġ�
		// ���get������ֻ��ͨ��Delɾ���������Լ�ɾ
		m_tq.Del(id);
	}
	return	0;
}
void	CNL2SlotMan_I_UDP::TEDeal_Connect(TQUNIT_T *pTE)
{
	// ���ӳ�ʱ
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_CONNECTING );
	// ֪ͨ�ϲ㲢�Ƴ�Slot
	ControlOutCloseAndRemoveSlot(pSlot, SLOTCLOSE_REASON_CONNECTTimeout);
}
void	CNL2SlotMan_I_UDP::TEDeal_Connect_RS(TQUNIT_T *pTE)
{
	// �ط���������
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_CONNECTING );
	SendConnectAndRegTQ(pSlot);
}
void	CNL2SlotMan_I_UDP::TEDeal_Accept(TQUNIT_T *pTE)
{
	// ���ܳ�ʱ
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_ACCEPTING );
	// ֪ͨ�ϲ㲢�Ƴ�Slot
	ControlOutCloseAndRemoveSlot(pSlot, SLOTCLOSE_REASON_ACCEPTTimeout);
}
void	CNL2SlotMan_I_UDP::TEDeal_Accept_RS(TQUNIT_T *pTE)
{
	// �ط�Agree���
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_ACCEPTING );
	SendConnectAgreeAndRegTQ(pSlot);
}
void	CNL2SlotMan_I_UDP::TEDeal_Data_RS(TQUNIT_T *pTE)
{
	// ����idx��Ӧ��Out���ݵ��Է�
	CNL2SLOT_I							*pSlot		= pTE->pSlot;
	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= pTE->un.resend.pOutUnit;
	slot_send(pSlot, pOutUnit->pCmd, pOutUnit->nSize);

	// �ط���������
	int	nNextRS	= pTE->pSlot->slotinfo.nRTT<<(pOutUnit->nRSCount+1);
	if( nNextRS>=m_nNoopTimeout )
	{
		nNextRS	= m_nNoopTimeout;
	}
	else
	{
		pOutUnit->nRSCount	++;
	}
	// Ȼ��ע����һ���¼�
	TQUNIT_T				*pTQUnit;
	if( m_tq.AddGetRef(m_tickNow + nNextRS, (void **)&pTQUnit, &pOutUnit->nTimeEventID)<0 )
	{
		assert(0);
		return;
	}
	pTQUnit->pSlot			= pSlot;
	pTQUnit->tefunc			= &CNL2SlotMan_I_UDP::TEDeal_Data_RS;
	pTQUnit->un.resend.pOutUnit	= pOutUnit;
}
void	CNL2SlotMan_I_UDP::TEDeal_Noop(TQUNIT_T *pTE)
{
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	// ����ϴεķ���ʱ���Ƿ�������ڳ�����
	//if( pSlot->bNoopMustBeSend
	//||  wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastSend) >= m_nNoopTimeout )
	if( wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastSend) >= m_nNoopTimeout )
	{
		// �趨����tick��������Ҫ����NOOP
		SendNoopThisTick(pSlot);
		pSlot->bNoopMustBeSend	= false;
		pSlot->nNoopCount		= 0;
		// ��ΪSendNoopThisTick��û�е��·��Ͷ�����nLastSend��û�иı䣬����ʱ��Ҫ��ôд
		RegNoopTQ(pSlot, m_tickNow+m_nNoopTimeout);
	}
	else
	{
		// ע����һ��
		RegNoopTQ(pSlot, pSlot->slotinfo.nLastSend+m_nNoopTimeout);
	}
	// �ж��Ƿ���Ҫ���ͳ������
	if( m_info.bLogNetStat )
	{
		char	buf[1024];
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,CNL_RT)"CNL2 net stat,%s", pSlot->slotinfo.infostr(buf));
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_Drop(TQUNIT_T *pTE)
{
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	// ����ϴεĽ������ݵ�ʱ���Ƿ�������ڳ�����
	if( wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastRecv) >= m_info.nDropTimeout )
	{
		// �Ƴ�slot����֪ͨ�ϲ�
		ControlOutCloseAndRemoveSlot(pSlot, SLOTCLOSE_REASON_DROP);
	}
	else
	{
		// ע����һ��
		RegDropTQ(pSlot);
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_MightDrop(TQUNIT_T *pTE)
{
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	// ����ϴεĽ������ݵ�ʱ���Ƿ�������ڳ�����
	if( wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastRecv) >= m_nMightDropTimeout )
	{
		if( pSlot->bIsClient )
		{
			if( SwitchAddr(pSlot)<=0 )
			{
				return;
			}
			// ע�ᷢ��SwitchAddr�����ʱ��
			RegSwitchAddrReq(pSlot, m_tickNow);
		}
		else
		{
			// ע�ᷢ��SwitchAddr�����ʱ��
			RegSwitchAddrReq(pSlot, m_tickNow);
			// ������ֻ��Ҫ֪ͨ�ͻ��˸ı��ַ����
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(808,CNL_RT)"CNL2SlotMan_I_UDP,order peer switch addr,0x%X,%d,%s", pSlot->slotinfo.nSlot, pSlot->slotinfo.nSpecialSeed, cmn_get_ipnportstr_by_saaddr(&pSlot->slotinfo.peerAddr));
		}
		// ע����һ�����ʱ��
		RegMightDropTQ(pSlot, m_tickNow+m_nMightDropTimeout);
	}
	else
	{
		// ע����һ��
		RegMightDropTQ(pSlot, pSlot->slotinfo.nLastRecv+m_nMightDropTimeout);
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_SwitchAddrReq(TQUNIT_T *pTE)
{
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	// ����ϴεĽ������ݵ�ʱ���Ƿ����������Ȼ�����˹涨
	if( wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastRecv) >= m_nMightDropTimeout )
	{
		SendSwitchAddrReq(pSlot);
		// ע����һ���¼�
		RegSwitchAddrReq(pSlot, m_tickNow + pSlot->slotinfo.nRTT*8);
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_Close(TQUNIT_T *pTE)
{
	// �Ƴ�slot����֪ͨ�ϲ�
	ControlOutCloseAndRemoveSlot(pTE->pSlot, SLOTCLOSE_REASON_CLOSETimeout);
}
void	CNL2SlotMan_I_UDP::TEDeal_Close_RS(TQUNIT_T *pTE)
{
	// �ط�CLOSE
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	SendClose(pSlot);
	// ע����һ��
	RegCloseRSTQ(pSlot);
}
void	CNL2SlotMan_I_UDP::TEDeal_StatLOG(TQUNIT_T *pTE)
{
	// ���һ����־ͳ��
}
void	CNL2SlotMan_I_UDP::TEDeal_AllowDenyFileChangeCheck(TQUNIT_T *pTE)
{
	// ���һ���ļ��Ƿ�ı�
	if( m_fcdAllowDeny.IsChanged() )
	{
		m_ConnectAllowDeny.initfromfile(m_info.szConnectAllowDeny);
	}
	// ������һ��
	Set_TEDeal_AllowDenyFileChangeCheck();
}
void	CNL2SlotMan_I_UDP::Set_TEDeal_AllowDenyFileChangeCheck()
{
	TQUNIT_T	*pTQUnit;
	whtimequeue::ID_T	tqid;
	if( m_tq.AddGetRef(m_tickNow + m_info.nConnectAllowDenyRefreshInterval, (void **)&pTQUnit, &tqid)<0 )
	{
		// ����ʱ�����ʧ��
		assert(0);
	}
	else
	{
		pTQUnit->tefunc		= &CNL2SlotMan_I_UDP::TEDeal_AllowDenyFileChangeCheck;
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_StatInterval(TQUNIT_T *pTE)
{
	//
	m_statUpByte.calcstat();
	m_statDownByte.calcstat();
	m_statUpCount.calcstat();
	m_statDownCount.calcstat();
	// �����־
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1007,CNL_RT)"STAT,%d,%d,%d,%d,%d", m_Slots.size(), (int)m_statUpByte.curstat(), (int)m_statDownByte.curstat(), (int)m_statUpCount.curstat(), (int)m_statDownCount.curstat());
	//
	m_statUpByte.reset();
	m_statDownByte.reset();
	m_statUpCount.reset();
	m_statDownCount.reset();

	// �����´μ���
	Set_TEDeal_StatInterval();
}
void	CNL2SlotMan_I_UDP::Set_TEDeal_StatInterval()
{
	TQUNIT_T	*pTQUnit;
	whtimequeue::ID_T	tqid;
	if( m_tq.AddGetRef(m_tickNow + m_info.nStatInterval, (void **)&pTQUnit, &tqid)<0 )
	{
		// ����ʱ�����ʧ��
		assert(0);
	}
	else
	{
		pTQUnit->tefunc		= &CNL2SlotMan_I_UDP::TEDeal_StatInterval;
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_CA0(TQUNIT_T *pTQUnit)
{
	// ����Ȩ�ļ����뵽�ڴ�
	if( whfile_readfile(m_info.szAuthFile, m_vectAuth)==0 )
	{
		whfile_crc_readfile(m_info.szRSAPriKeyFile, m_vectRSA);
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_CA1(TQUNIT_T *pTQUnit)
{
	if( m_vectAuth.size()>0 && m_vectRSA.size()>0 )
	{
		// �⿪�Ļ���
		whvector<char>	vectAuth;
		if( GetAuthInfo(m_vectRSA.getbuf(), m_vectRSA.size(), m_info.szRSAPass, m_vectAuth.getbuf(), m_vectAuth.size(), vectAuth)==0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,AUTH_LOG_TMP)"GetAuthInfo OK");

			// �������������
			whfile_i_mem	f(vectAuth.getbuf(), vectAuth.size());
			WHDATAINI_CMN	ini;
			AUTH_CFGINFO_T	cfg;
			ini.addobj("Auth", &cfg);
			if( ini.analyzefile(&f,true)==0 )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,AUTH_LOG_TMP)"ini.analyzefile OK");

				// �ж�һ��
				if( cfg.szName[0] )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,AUTH_LOG_TMP)"auth.name %s,auth.IP %s, cfg.IP %s", cfg.szName, cfg.szIP, m_info.szBindAddr);

					// ��ð󶨵�IP��ַ��Ҳ���Ը�������϶˿ڣ�
					int	nLen	= strlen(cfg.szIP);
					if( memcmp(m_info.szBindAddr, cfg.szIP, nLen)==0 )
					{
						// �жϳɹ�
						return;
					}
				}
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,AUTH_LOG_TMP)"auth.name NULL");
			}
		}
	}
	// ���������㣬��ʼ��
	SetTE_CA(&CNL2SlotMan_I_UDP::TEDeal_CA2, 1974);
	SetTE_CA(&CNL2SlotMan_I_UDP::TEDeal_CA3, 8*1024);
}
void	CNL2SlotMan_I_UDP::TEDeal_CA2(TQUNIT_T *pTQUnit)
{
	// ���һ��LOG
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,PNGS_RT)"@_@, bad luck, hacking detected, data being destoried ...");
}
void	CNL2SlotMan_I_UDP::TEDeal_CA3(TQUNIT_T *pTQUnit)
{
	// ����
	CONTROL_T	*pCO;
	ControlOutAlloc(pCO);
	if( pCO )
	{
		pCO->nCmd	= CONTROL_OUT_SHOULD_STOP;
	}
}
void	CNL2SlotMan_I_UDP::SetTE_CA(TEDEAL_T pFunc, int nMS)
{
	whtimequeue::ID_T	teid;
	// �������ӳ�ʱ��ʱ��
	TQUNIT_T	*pTQUnit;
	if( m_tq.AddGetRef(m_tickNow+nMS, (void **)&pTQUnit, &teid)<0 )
	{
		// ��ô���޷�����ʱ���¼�����
		// ��������ط��Ͳ�Ҫassert�ˣ�����ᱻ�ҵ���
		return;
	}
	pTQUnit->tefunc			= pFunc;
}
