// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : GSMEM_i.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 公共内存管理逻辑模块的DLL实现部分
//              : 编译出来的DLL应该叫GSDB.so或GSDB_d.so
//              : 创建对象的函数应该叫GSDB_Create
// CreationDate : 2007-09-26
// ChangeLog    : 2008-02-19 在钟文杰的帮助下，增加了创建共享内存到固定地址的功能（原来只是随便尝试了几个地址发现失败就没有继续尝试，没有分析原因是因为测试的地址太低，现在使用了高地址就成功了0x60000000）

#include "../inc/GSMEM_i.h"

using namespace n_pngs;

////////////////////////////////////////////////////////////////////
// GSMEM
////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(GSMEM::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nSHMKey, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nSHMSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nFixedAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMAXSIZENUM, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nHASHSTEP1, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nHASHSTEP2, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nOutputStatInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTstOffset, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTstOffset1, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nMinMemWarn, 0)
	WHDATAPROP_ON_SETVALFUNC(Key2Idx)
WHDATAPROP_MAP_END()

GSMEM::GSMEM()
: m_nIsInherited(0)
, m_nMemDiff(0)
, m_pSHM(NULL)
, m_pmyinfo(NULL)
, m_pMem(NULL)
, m_aoone(NULL)
, m_am(NULL)
, m_tickNow(0)
, m_bInherited(false)
, m_bQuickDetach(false)
{
	m_cfginfo.pHost		= this;
	strcpy(FATHERCLASS::m_szLogicType, PNGS_DLL_NAME_GSMEM);
	FATHERCLASS::m_nVer	= GSMEM_VER;
}
GSMEM::~GSMEM()
{
}
int		GSMEM::Organize()
{
	return	0;
}
int		GSMEM::Detach(bool bQuick)
{
	// bQuick方式一般是在初始化过程中出错导致的
	m_bQuickDetach	= bQuick;
	return	0;
}
int		GSMEM::Init(const char *cszCFGName)
{
	m_tickNow	= wh_gettickcount();
	int	rst	= 0;
	if( (rst=Init_CFG(cszCFGName))<0 )
	{
		return	-1;
	}
	// 初始化共享内存
	if( m_cfginfo.nSHMKey==0 )
	{
		if( InitSHMAndRelatedObjs(malloc(m_cfginfo.nSHMSize))<0 )
		{
			return	-11;
		}
	}
	else
	{
		// 检查共享内存是否存在
		m_pSHM			= whshm_open(m_cfginfo.nSHMKey, (void *)m_cfginfo.nFixedAddr);
		if( !m_pSHM )
		{
			// 不存在则直接从共享内存中申请
			m_pSHM		= whshm_create(m_cfginfo.nSHMKey, m_cfginfo.nSHMSize, (void *)m_cfginfo.nFixedAddr);
			if( !m_pSHM )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(141,GSMEM_INT_RLS)"GSMEM::Init,NULL,whshm_create,%d,%d", m_cfginfo.nSHMKey, m_cfginfo.nSHMSize);
				return	-12;
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(711,GSMEM_INT_RLS)"GSMEM::Init,,whshm_create,0x%08X,%d,%d", m_pSHM->GetBuf(), m_cfginfo.nSHMKey, m_cfginfo.nSHMSize);
			// 直接初始化共享内存
			if( InitSHMAndRelatedObjs(m_pSHM->GetBuf())<0 )
			{
				return	-13;
			}
		}
		else
		{
			m_bInherited	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(711,GSMEM_INT_RLS)"GSMEM::Init,,whshm_open,0x%08X,%d,%d", m_pSHM->GetBuf(), m_cfginfo.nSHMKey, m_cfginfo.nSHMSize);
			// 说明原来共享内存是存在的
			// 检查其合法性
			// 看内存大小是否正确
			if( m_pSHM->GetSize() != (size_t)m_cfginfo.nSHMSize )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(147,GSMEM_INT_RLS)"GSMEM::Init,,inherit SHM,%d,%d", m_pSHM->GetSize(), m_cfginfo.nSHMSize);
				return	-16;
			}
			// 整理偏移的指针
			if( AdjustSHMPtrAndSetMgr()<0 )
			{
				return	-18;
			}
		}
	}
	// 初始化时间队列
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		return	-20;
	}
	// 启动定时输出统计
	SetTE_OutputStat();

	return	0;
}
int		GSMEM::Release()
{
	if( m_pmyinfo )
	{
		if( m_cfginfo.nSHMKey==0 )
		{
			free(m_pmyinfo);
		}
		else
		{
			// 如果所有数据(包括玩家数据和服务器数据)都成功存盘了，就释放共享内存，否则保留共享内存
			// 这个也可能是原来就是非法宕机，然后重启也没有重启成功的情况
			if( CanFreeMem() )
			{
				int	rst	= whshm_destroy(m_cfginfo.nSHMKey);
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(288,GSMEM_INT_RLS)"GSMEM::Release,whshm_destroy,%d", rst);
			}
			else
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(838,GSMEM_INT_RLS)"GSMEM::Release,can not free mem,%d", m_am->m_nUseCount);
			}
			// 但无论如何也要把SHM对象删除，否则有泄露，会干扰别人的判断
			if( m_pSHM )
			{
				delete	m_pSHM;
				m_pSHM	= NULL;
			}
		}
		m_pmyinfo		= NULL;
	}
	return	0;
}

int		GSMEM::Init_CFG(const char *cszCFGName)
{
	WHDATAINI_CMN	ini;
	ini.addobj("GSMEM", &m_cfginfo);
	
	int	rst = ini.analyzefile(cszCFGName);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(148,GSMEM_INT_RLS)"GSMEM::Init_CFG,%d,ini.analyzefile,%s,%s", rst, cszCFGName, ini.printerrreport());
		return	-1;
	}
	if( m_cfginfo.nTstOffset1<0 )
	{
		m_cfginfo.nTstOffset1	= 0;
	}

	return	0;
}
int		GSMEM::Init_CFG_Reload(const char *cszCFGName)
{
	WHDATAINI_CMN	ini;
	ini.addobj("GSMEM", &m_cfginfo);
	
	int	rst = ini.analyzefile(cszCFGName, false, 1);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(148,Init_CFG_Reload)"GSMEM::Init_CFG,%d,ini.analyzefile,%s,%s", rst, cszCFGName, ini.printerrreport());
		return	-1;
	}

	return	0;
}
int		GSMEM::AddKey2Idx(const char *cszParam)
{
	char	szKey[PNGS_LOGICNAME_LEN]	= "";
	int		nIdx						= 0;
	if( wh_strsplit("sd", cszParam, ",", szKey, &nIdx)!=2 )
	{
		return	-1;
	}
	m_mapKey2PtrIdx[szKey]	= nIdx;
	return	0;
}
int		GSMEM::InitSHMAndRelatedObjs(void *pBuf)
{
	if( !pBuf )
	{
		assert(0);
		return	-1;
	}
	m_pmyinfo		= (MYINFO_T*)pBuf;
	m_pmyinfo->clear();

	int	nTotalSize	= m_cfginfo.nSHMSize - sizeof(*m_pmyinfo) - sizeof(*m_aoone) - sizeof(*m_am);
	m_aoone			= (AO_T *)wh_getptrnexttoptr(m_pmyinfo);
	m_am			= (AM_T *)wh_getptrnexttoptr(m_aoone);
	m_pMem			= wh_getptrnexttoptr(m_am);
	new (m_aoone) AO_T;
	if( m_aoone->Init(m_pMem, nTotalSize, m_cfginfo.nMAXSIZENUM, m_cfginfo.nHASHSTEP1, m_cfginfo.nHASHSTEP2)<0 )
	{
		return	-2;
	}
	new (m_am) AM_T;
	m_am->SetAO(m_aoone);

	return	0;
}
int		GSMEM::ReleaseSHMAndRelatedObjs()
{
	// m_aoone->Release();	应该不用了，因为大内存被释放了就可以了
	return	0;
}
int		GSMEM::AdjustSHMPtrAndSetMgr()
{
	long	nOffset	= m_pSHM->GetMemDiff() + m_cfginfo.nTstOffset;
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(817,GSMEM_INT_RLS)"GSMEM::AdjustSHMPtrAndSetMgr,%ld,memdiff", nOffset);

	m_nIsInherited	= 1;
	m_nMemDiff		= nOffset;

	// 先获得自己需要的信息
	// 下面的调整纯粹是为了调试看看内存指针变化的影响
	m_pmyinfo		= (MYINFO_T *)m_pSHM->GetBuf();
	if( m_cfginfo.nTstOffset!=0 )
	{
		// 移动一下
		void	*pOldMem	= m_pmyinfo;
		m_pmyinfo			= (MYINFO_T *)wh_getoffsetaddr(pOldMem, m_cfginfo.nTstOffset);
		memmove(m_pmyinfo, pOldMem, m_pSHM->GetSize()-abs(m_cfginfo.nTstOffset)-m_cfginfo.nTstOffset1);
	}
	if( m_cfginfo.nTstOffset1>0 )
	{
		m_pmyinfo			= (MYINFO_T *)wh_getoffsetaddr(m_pmyinfo, m_cfginfo.nTstOffset1);
	}
	// 分配器的内部调整
	m_aoone			= (AO_T *)wh_getptrnexttoptr(m_pmyinfo);
	m_am			= (AM_T *)wh_getptrnexttoptr(m_aoone);
	m_pMem			= wh_getptrnexttoptr(m_am);
	// 看看调整计数是否为0
	if( m_am->m_nAdjustCount>0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(829,GSMEM_INT_RLS)"GSMEM::Init,%d,bad SHM AdjustCount,last inherit not complete,data discarded!", m_am->m_nAdjustCount);
		// 删除共享内存，下次启动的就按照普通启动了
		whshm_destroy(m_cfginfo.nSHMKey);
		return	-1;
	}

	m_am->m_nAdjustCount	= 1;

	// 分配器内部调整
	m_aoone->AdjustInnerPtr(nOffset);
	m_am->SetAO(m_aoone);

	// 调整自己保存的数据
	m_pmyinfo->AdjustInnerPtr(nOffset);

	// 清空各个计数
	m_am->m_nAdjustCount	= 0;
	m_am->m_nUseCount		= 0;

	// 整理完毕之后需要保存当前的共享内存头位置（如果后面其他模块的调整没有结束，那么m_nAdjustCount会有问题的，所以这里先保存指针没有什么问题）
	m_pSHM->SaveCurMemHdrPtr();

	return	0;
}

int		GSMEM::Tick_BeforeDealCmdIn()
{
	m_tickNow	= wh_gettickcount();

	// 时间队列处理
	Tick_DealTE();

	return	0;
}
