// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i_TEDeal.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CNL2的内部实现
// CreationDate : 2005-04-21
// ChangeLog    : 2008-07-29 把重新发送的等待时间多乘1倍，这样可以减少错误的重发量

#include "../inc/whnetcnl2_i.h"

using namespace	n_whnet;

namespace
{
	struct	AUTH_CFGINFO_T		: public whdataini::obj
	{
		char	szName[64];		// 运营商的名字
		char	szIP[64];		// 绑定的地址

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
	// 看看时间队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_tq.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		assert(pUnit->tefunc!=NULL);
		(this->*pUnit->tefunc)(pUnit);	// 这里面可以申请别的时间事件，反正和这个不干扰的。
		// 这个get出来的只能通过Del删除，不能自己删
		m_tq.Del(id);
	}
	return	0;
}
void	CNL2SlotMan_I_UDP::TEDeal_Connect(TQUNIT_T *pTE)
{
	// 连接超时
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_CONNECTING );
	// 通知上层并移除Slot
	ControlOutCloseAndRemoveSlot(pSlot, SLOTCLOSE_REASON_CONNECTTimeout);
}
void	CNL2SlotMan_I_UDP::TEDeal_Connect_RS(TQUNIT_T *pTE)
{
	// 重发连接请求
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_CONNECTING );
	SendConnectAndRegTQ(pSlot);
}
void	CNL2SlotMan_I_UDP::TEDeal_Accept(TQUNIT_T *pTE)
{
	// 接受超时
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_ACCEPTING );
	// 通知上层并移除Slot
	ControlOutCloseAndRemoveSlot(pSlot, SLOTCLOSE_REASON_ACCEPTTimeout);
}
void	CNL2SlotMan_I_UDP::TEDeal_Accept_RS(TQUNIT_T *pTE)
{
	// 重发Agree结果
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	assert( pSlot->slotinfo.nStatus == CNL2SLOTINFO_T::STATUS_ACCEPTING );
	SendConnectAgreeAndRegTQ(pSlot);
}
void	CNL2SlotMan_I_UDP::TEDeal_Data_RS(TQUNIT_T *pTE)
{
	// 发送idx对应的Out数据到对方
	CNL2SLOT_I							*pSlot		= pTE->pSlot;
	CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit	= pTE->un.resend.pOutUnit;
	slot_send(pSlot, pOutUnit->pCmd, pOutUnit->nSize);

	// 重发次数增加
	int	nNextRS	= pTE->pSlot->slotinfo.nRTT<<(pOutUnit->nRSCount+1);
	if( nNextRS>=m_nNoopTimeout )
	{
		nNextRS	= m_nNoopTimeout;
	}
	else
	{
		pOutUnit->nRSCount	++;
	}
	// 然后注册下一个事件
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
	// 检测上次的发送时刻是否距离现在超过了
	//if( pSlot->bNoopMustBeSend
	//||  wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastSend) >= m_nNoopTimeout )
	if( wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastSend) >= m_nNoopTimeout )
	{
		// 设定本次tick结束后需要发送NOOP
		SendNoopThisTick(pSlot);
		pSlot->bNoopMustBeSend	= false;
		pSlot->nNoopCount		= 0;
		// 因为SendNoopThisTick并没有导致发送动作，nLastSend还没有改变，所以时间要这么写
		RegNoopTQ(pSlot, m_tickNow+m_nNoopTimeout);
	}
	else
	{
		// 注册下一个
		RegNoopTQ(pSlot, pSlot->slotinfo.nLastSend+m_nNoopTimeout);
	}
	// 判断是否需要输出统计数据
	if( m_info.bLogNetStat )
	{
		char	buf[1024];
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,CNL_RT)"CNL2 net stat,%s", pSlot->slotinfo.infostr(buf));
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_Drop(TQUNIT_T *pTE)
{
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	// 检测上次的接收数据的时刻是否距离现在超过了
	if( wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastRecv) >= m_info.nDropTimeout )
	{
		// 移除slot，并通知上层
		ControlOutCloseAndRemoveSlot(pSlot, SLOTCLOSE_REASON_DROP);
	}
	else
	{
		// 注册下一个
		RegDropTQ(pSlot);
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_MightDrop(TQUNIT_T *pTE)
{
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	// 检测上次的接收数据的时刻是否距离现在超过了
	if( wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastRecv) >= m_nMightDropTimeout )
	{
		if( pSlot->bIsClient )
		{
			if( SwitchAddr(pSlot)<=0 )
			{
				return;
			}
			// 注册发送SwitchAddr请求的时刻
			RegSwitchAddrReq(pSlot, m_tickNow);
		}
		else
		{
			// 注册发送SwitchAddr请求的时刻
			RegSwitchAddrReq(pSlot, m_tickNow);
			// 服务器只需要通知客户端改变地址即可
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(808,CNL_RT)"CNL2SlotMan_I_UDP,order peer switch addr,0x%X,%d,%s", pSlot->slotinfo.nSlot, pSlot->slotinfo.nSpecialSeed, cmn_get_ipnportstr_by_saaddr(&pSlot->slotinfo.peerAddr));
		}
		// 注册下一个检查时刻
		RegMightDropTQ(pSlot, m_tickNow+m_nMightDropTimeout);
	}
	else
	{
		// 注册下一个
		RegMightDropTQ(pSlot, pSlot->slotinfo.nLastRecv+m_nMightDropTimeout);
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_SwitchAddrReq(TQUNIT_T *pTE)
{
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	// 检测上次的接收数据的时刻是否距离现在仍然超过了规定
	if( wh_tickcount_diff(m_tickNow, pSlot->slotinfo.nLastRecv) >= m_nMightDropTimeout )
	{
		SendSwitchAddrReq(pSlot);
		// 注册下一个事件
		RegSwitchAddrReq(pSlot, m_tickNow + pSlot->slotinfo.nRTT*8);
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_Close(TQUNIT_T *pTE)
{
	// 移除slot，并通知上层
	ControlOutCloseAndRemoveSlot(pTE->pSlot, SLOTCLOSE_REASON_CLOSETimeout);
}
void	CNL2SlotMan_I_UDP::TEDeal_Close_RS(TQUNIT_T *pTE)
{
	// 重发CLOSE
	CNL2SLOT_I	*pSlot	= pTE->pSlot;
	SendClose(pSlot);
	// 注册下一个
	RegCloseRSTQ(pSlot);
}
void	CNL2SlotMan_I_UDP::TEDeal_StatLOG(TQUNIT_T *pTE)
{
	// 输出一下日志统计
}
void	CNL2SlotMan_I_UDP::TEDeal_AllowDenyFileChangeCheck(TQUNIT_T *pTE)
{
	// 检查一下文件是否改变
	if( m_fcdAllowDeny.IsChanged() )
	{
		m_ConnectAllowDeny.initfromfile(m_info.szConnectAllowDeny);
	}
	// 设置下一个
	Set_TEDeal_AllowDenyFileChangeCheck();
}
void	CNL2SlotMan_I_UDP::Set_TEDeal_AllowDenyFileChangeCheck()
{
	TQUNIT_T	*pTQUnit;
	whtimequeue::ID_T	tqid;
	if( m_tq.AddGetRef(m_tickNow + m_info.nConnectAllowDenyRefreshInterval, (void **)&pTQUnit, &tqid)<0 )
	{
		// 加入时间队列失败
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
	// 输出日志
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(1007,CNL_RT)"STAT,%d,%d,%d,%d,%d", m_Slots.size(), (int)m_statUpByte.curstat(), (int)m_statDownByte.curstat(), (int)m_statUpCount.curstat(), (int)m_statDownCount.curstat());
	//
	m_statUpByte.reset();
	m_statDownByte.reset();
	m_statUpCount.reset();
	m_statDownCount.reset();

	// 设置下次计算
	Set_TEDeal_StatInterval();
}
void	CNL2SlotMan_I_UDP::Set_TEDeal_StatInterval()
{
	TQUNIT_T	*pTQUnit;
	whtimequeue::ID_T	tqid;
	if( m_tq.AddGetRef(m_tickNow + m_info.nStatInterval, (void **)&pTQUnit, &tqid)<0 )
	{
		// 加入时间队列失败
		assert(0);
	}
	else
	{
		pTQUnit->tefunc		= &CNL2SlotMan_I_UDP::TEDeal_StatInterval;
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_CA0(TQUNIT_T *pTQUnit)
{
	// 把授权文件载入到内存
	if( whfile_readfile(m_info.szAuthFile, m_vectAuth)==0 )
	{
		whfile_crc_readfile(m_info.szRSAPriKeyFile, m_vectRSA);
	}
}
void	CNL2SlotMan_I_UDP::TEDeal_CA1(TQUNIT_T *pTQUnit)
{
	if( m_vectAuth.size()>0 && m_vectRSA.size()>0 )
	{
		// 解开的缓冲
		whvector<char>	vectAuth;
		if( GetAuthInfo(m_vectRSA.getbuf(), m_vectRSA.size(), m_info.szRSAPass, m_vectAuth.getbuf(), m_vectAuth.size(), vectAuth)==0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,AUTH_LOG_TMP)"GetAuthInfo OK");

			// 分析解出的内容
			whfile_i_mem	f(vectAuth.getbuf(), vectAuth.size());
			WHDATAINI_CMN	ini;
			AUTH_CFGINFO_T	cfg;
			ini.addobj("Auth", &cfg);
			if( ini.analyzefile(&f,true)==0 )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,AUTH_LOG_TMP)"ini.analyzefile OK");

				// 判断一下
				if( cfg.szName[0] )
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,AUTH_LOG_TMP)"auth.name %s,auth.IP %s, cfg.IP %s", cfg.szName, cfg.szIP, m_info.szBindAddr);

					// 获得绑定的IP地址（也可以给后面加上端口）
					int	nLen	= strlen(cfg.szIP);
					if( memcmp(m_info.szBindAddr, cfg.szIP, nLen)==0 )
					{
						// 判断成功
						return;
					}
				}
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,AUTH_LOG_TMP)"auth.name NULL");
			}
		}
	}
	// 条件不满足，开始退
	SetTE_CA(&CNL2SlotMan_I_UDP::TEDeal_CA2, 1974);
	SetTE_CA(&CNL2SlotMan_I_UDP::TEDeal_CA3, 8*1024);
}
void	CNL2SlotMan_I_UDP::TEDeal_CA2(TQUNIT_T *pTQUnit)
{
	// 输出一句LOG
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(0,PNGS_RT)"@_@, bad luck, hacking detected, data being destoried ...");
}
void	CNL2SlotMan_I_UDP::TEDeal_CA3(TQUNIT_T *pTQUnit)
{
	// 结束
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
	// 开启连接超时定时器
	TQUNIT_T	*pTQUnit;
	if( m_tq.AddGetRef(m_tickNow+nMS, (void **)&pTQUnit, &teid)<0 )
	{
		// 怎么会无法加入时间事件涅？
		// 不过这个地方就不要assert了，否则会被找到的
		return;
	}
	pTQUnit->tefunc			= pFunc;
}
