// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : GSMEM_i.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �����ڴ�����߼�ģ���DLLʵ�ֲ���
//              : ���������DLLӦ�ý�GSDB.so��GSDB_d.so
//              : ��������ĺ���Ӧ�ý�GSDB_Create
// CreationDate : 2007-09-26
// ChangeLog    : 2008-02-19 �����Ľܵİ����£������˴��������ڴ浽�̶���ַ�Ĺ��ܣ�ԭ��ֻ����㳢���˼�����ַ����ʧ�ܾ�û�м������ԣ�û�з���ԭ������Ϊ���Եĵ�ַ̫�ͣ�����ʹ���˸ߵ�ַ�ͳɹ���0x60000000��

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
	// bQuick��ʽһ�����ڳ�ʼ�������г����µ�
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
	// ��ʼ�������ڴ�
	if( m_cfginfo.nSHMKey==0 )
	{
		if( InitSHMAndRelatedObjs(malloc(m_cfginfo.nSHMSize))<0 )
		{
			return	-11;
		}
	}
	else
	{
		// ��鹲���ڴ��Ƿ����
		m_pSHM			= whshm_open(m_cfginfo.nSHMKey, (void *)m_cfginfo.nFixedAddr);
		if( !m_pSHM )
		{
			// ��������ֱ�Ӵӹ����ڴ�������
			m_pSHM		= whshm_create(m_cfginfo.nSHMKey, m_cfginfo.nSHMSize, (void *)m_cfginfo.nFixedAddr);
			if( !m_pSHM )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(141,GSMEM_INT_RLS)"GSMEM::Init,NULL,whshm_create,%d,%d", m_cfginfo.nSHMKey, m_cfginfo.nSHMSize);
				return	-12;
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(711,GSMEM_INT_RLS)"GSMEM::Init,,whshm_create,0x%08X,%d,%d", m_pSHM->GetBuf(), m_cfginfo.nSHMKey, m_cfginfo.nSHMSize);
			// ֱ�ӳ�ʼ�������ڴ�
			if( InitSHMAndRelatedObjs(m_pSHM->GetBuf())<0 )
			{
				return	-13;
			}
		}
		else
		{
			m_bInherited	= true;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(711,GSMEM_INT_RLS)"GSMEM::Init,,whshm_open,0x%08X,%d,%d", m_pSHM->GetBuf(), m_cfginfo.nSHMKey, m_cfginfo.nSHMSize);
			// ˵��ԭ�������ڴ��Ǵ��ڵ�
			// �����Ϸ���
			// ���ڴ��С�Ƿ���ȷ
			if( m_pSHM->GetSize() != (size_t)m_cfginfo.nSHMSize )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(147,GSMEM_INT_RLS)"GSMEM::Init,,inherit SHM,%d,%d", m_pSHM->GetSize(), m_cfginfo.nSHMSize);
				return	-16;
			}
			// ����ƫ�Ƶ�ָ��
			if( AdjustSHMPtrAndSetMgr()<0 )
			{
				return	-18;
			}
		}
	}
	// ��ʼ��ʱ�����
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= m_cfginfo.nTQChunkSize;
	if( (rst=m_TQ.Init(&tqinfo))<0 )
	{
		return	-20;
	}
	// ������ʱ���ͳ��
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
			// �����������(����������ݺͷ���������)���ɹ������ˣ����ͷŹ����ڴ棬�����������ڴ�
			// ���Ҳ������ԭ�����ǷǷ�崻���Ȼ������Ҳû�������ɹ������
			if( CanFreeMem() )
			{
				int	rst	= whshm_destroy(m_cfginfo.nSHMKey);
				GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(288,GSMEM_INT_RLS)"GSMEM::Release,whshm_destroy,%d", rst);
			}
			else
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(838,GSMEM_INT_RLS)"GSMEM::Release,can not free mem,%d", m_am->m_nUseCount);
			}
			// ���������ҲҪ��SHM����ɾ����������й¶������ű��˵��ж�
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
	// m_aoone->Release();	Ӧ�ò����ˣ���Ϊ���ڴ汻�ͷ��˾Ϳ�����
	return	0;
}
int		GSMEM::AdjustSHMPtrAndSetMgr()
{
	long	nOffset	= m_pSHM->GetMemDiff() + m_cfginfo.nTstOffset;
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(817,GSMEM_INT_RLS)"GSMEM::AdjustSHMPtrAndSetMgr,%ld,memdiff", nOffset);

	m_nIsInherited	= 1;
	m_nMemDiff		= nOffset;

	// �Ȼ���Լ���Ҫ����Ϣ
	// ����ĵ���������Ϊ�˵��Կ����ڴ�ָ��仯��Ӱ��
	m_pmyinfo		= (MYINFO_T *)m_pSHM->GetBuf();
	if( m_cfginfo.nTstOffset!=0 )
	{
		// �ƶ�һ��
		void	*pOldMem	= m_pmyinfo;
		m_pmyinfo			= (MYINFO_T *)wh_getoffsetaddr(pOldMem, m_cfginfo.nTstOffset);
		memmove(m_pmyinfo, pOldMem, m_pSHM->GetSize()-abs(m_cfginfo.nTstOffset)-m_cfginfo.nTstOffset1);
	}
	if( m_cfginfo.nTstOffset1>0 )
	{
		m_pmyinfo			= (MYINFO_T *)wh_getoffsetaddr(m_pmyinfo, m_cfginfo.nTstOffset1);
	}
	// ���������ڲ�����
	m_aoone			= (AO_T *)wh_getptrnexttoptr(m_pmyinfo);
	m_am			= (AM_T *)wh_getptrnexttoptr(m_aoone);
	m_pMem			= wh_getptrnexttoptr(m_am);
	// �������������Ƿ�Ϊ0
	if( m_am->m_nAdjustCount>0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_FATAL, GLGR_STD_HDR(829,GSMEM_INT_RLS)"GSMEM::Init,%d,bad SHM AdjustCount,last inherit not complete,data discarded!", m_am->m_nAdjustCount);
		// ɾ�������ڴ棬�´������ľͰ�����ͨ������
		whshm_destroy(m_cfginfo.nSHMKey);
		return	-1;
	}

	m_am->m_nAdjustCount	= 1;

	// �������ڲ�����
	m_aoone->AdjustInnerPtr(nOffset);
	m_am->SetAO(m_aoone);

	// �����Լ����������
	m_pmyinfo->AdjustInnerPtr(nOffset);

	// ��ո�������
	m_am->m_nAdjustCount	= 0;
	m_am->m_nUseCount		= 0;

	// �������֮����Ҫ���浱ǰ�Ĺ����ڴ�ͷλ�ã������������ģ��ĵ���û�н�������ôm_nAdjustCount��������ģ����������ȱ���ָ��û��ʲô���⣩
	m_pSHM->SaveCurMemHdrPtr();

	return	0;
}

int		GSMEM::Tick_BeforeDealCmdIn()
{
	m_tickNow	= wh_gettickcount();

	// ʱ����д���
	Tick_DealTE();

	return	0;
}
