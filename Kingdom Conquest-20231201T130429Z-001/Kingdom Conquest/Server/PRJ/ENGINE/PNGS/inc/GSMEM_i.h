// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : GSMEM_i.h
// Creator      : Wei Hua (魏华)
// Comment      : 公共内存管理逻辑模块的DLL实现部分
//              : 编译出来的DLL应该叫GSDB.so或GSDB_d.so
//              : 创建对象的函数应该叫GSDB_Create
// CreationDate : 2007-09-26
// ChangeLog    : 2008-02-19 在钟文杰的帮助下，增加了创建共享内存到固定地址的功能（原来只是随便尝试了几个地址发现失败就没有继续尝试，没有分析原因是因为测试的地址太低，现在使用了高地址就成功了0x60000000）

#ifndef	__GSMEM_I_H__
#define	__GSMEM_I_H__

#include <WHNET/inc/whnetudpGLogger.h>
#include <WHCMN/inc/whshm.h>
#include <WHCMN/inc/whallocmgr2.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whtimequeue.h>
#include "./pngs_cmn.h"
#include "./pngs_packet_logic.h"
#include "./pngs_packet_mem_logic.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{

////////////////////////////////////////////////////////////////////
// GSMEM
////////////////////////////////////////////////////////////////////
class	GSMEM	: public CMN::ILogic
{
// 为接口实现的
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	GSMEM();
	~GSMEM();
	virtual	void	SelfDestroy()									{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect)		{return 0;}
	virtual	int		SureSend()										{return 0;}
private:
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize);
	virtual int		Tick_BeforeDealCmdIn();
	virtual int		Tick_AfterDealCmdIn()							{return 0;}
// 自己用的
protected:
	enum
	{
		PTR_NUM	= 32
	};
	typedef	CMN::ILogic					FATHERCLASS;
	#pragma pack(push, old_pack_num, 4)
	struct	MYINFO_T
	{
		void *	aptr[PTR_NUM];

		void	clear()
		{
			WHMEMSET0THIS();
		}

		void	AdjustInnerPtr(int nOffset)
		{
			for(int i=0;i<PTR_NUM;i++)
			{
				wh_adjustaddrbyoffset(aptr[i], nOffset);
			}
		}
	};
	#pragma pack(pop, old_pack_num)
	struct	CFGINFO_T		: public whdataini::obj
	{
		GSMEM							*pHost;						// 用于调用GSMEM的一些方法
		int								nSHMKey;					// 共享内存KEY，如果为0则表示不使用共享内存(默认是0)
		int								nSHMSize;					// 需要的共享内存块大小
		unsigned int					nFixedAddr;					// 尝试放在固定的内存地址上
		int								nMAXSIZENUM;				// 最多分配的不同尺寸个数
		int								nHASHSTEP1;
		int								nHASHSTEP2;
		int								nOutputStatInterval;		// 定期输出统计日志的间隔
		int								nTQChunkSize;				// 时间队列的每次次性分配块大小
		int								nTstOffset;					// 用于测试的内存偏移（需要移动偏移的量）
		int								nTstOffset1;				// 用于测试的内存偏移（原来指针和开头的偏移量）
		int								nMinMemWarn;				// 当可用内存数量剩下这么多时，就notify上层

		CFGINFO_T()
			: pHost(NULL)
			, nSHMKey(0)
			, nSHMSize(1*1024*1024)
			, nFixedAddr(0)
			, nMAXSIZENUM(64)
			, nHASHSTEP1(193)
			, nHASHSTEP2(17)
			, nOutputStatInterval(60*1000)
			, nTQChunkSize(100)
			, nTstOffset(0), nTstOffset1(0)
			, nMinMemWarn(1024*1024)
		{
		}
		WHDATAPROP_SETVALFUNC_DECLARE(Key2Idx)
		{
			return	pHost->AddKey2Idx(cszVal);
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T)
	};
	friend struct	CFGINFO_T;

	struct	TQUNIT_T
	{
		typedef	void (GSMEM::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// 处理该事件的函数
		// 数据部分
		union
		{
			int		nDummy;
		}un;
	};

protected:
	CFGINFO_T							m_cfginfo;
	int									m_nIsInherited;				// 表明内存是继承来的
	int									m_nMemDiff;					// 本次继承的内存和上次之间的偏移
	whshm								*m_pSHM;					// 共享内存对象的指针
	MYINFO_T							*m_pmyinfo;					// 这个最终是放在共享内存的头部
	void								*m_pMem;					// 用于分配的部分（即给m_aoone用的部分）
	AO_T								*m_aoone;					// 吃进一块大内存，在里面做小分配
	AM_T								*m_am;						// 多尺寸分配管理器
	whhash<whstr4hash, int>				m_mapKey2PtrIdx;			// 字串对指针数组下标的映射（内部指针不在这个映射中）
	whtick_t							m_tickNow;					// 当前时刻
	// 时间事件处理器
	whtimequeue							m_TQ;
	// 是否是继承的内存
	bool								m_bInherited;
	// 是否是QuickDetach
	bool								m_bQuickDetach;
private:
	// 根据配置文件初始化参数
	int		Init_CFG(const char *cszCFGName);
	// 添加一个key到指针序号的映射
	int		AddKey2Idx(const char *cszParam);
	// 初始化共享内存和相关对象
	int		InitSHMAndRelatedObjs(void *pBuf);
	// 终结共享内存和相关对象
	int		ReleaseSHMAndRelatedObjs();
	// 调整内部指针
	int		AdjustSHMPtrAndSetMgr();
	// 判断是否可以释放共享内存
	inline bool	CanFreeMem() const
	{
		return	m_am->m_nUseCount == 0 && ((!m_bInherited) || (!m_bQuickDetach));
	}

	// 定时相关的
	int		Tick_DealTE();
	void	TEDeal_OutputStat(TQUNIT_T * pTQUnit);
	void	SetTE_OutputStat();

public:
	// 创建自己类型的对象
	static CMN::ILogic *	Create()
	{
		return	new	GSMEM;
	}
};

}		// EOF namespace n_pngs

#endif	// EOF __GSMEM_I_H__
