// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : GSMEM_i.h
// Creator      : Wei Hua (κ��)
// Comment      : �����ڴ�����߼�ģ���DLLʵ�ֲ���
//              : ���������DLLӦ�ý�GSDB.so��GSDB_d.so
//              : ��������ĺ���Ӧ�ý�GSDB_Create
// CreationDate : 2007-09-26
// ChangeLog    : 2008-02-19 �����Ľܵİ����£������˴��������ڴ浽�̶���ַ�Ĺ��ܣ�ԭ��ֻ����㳢���˼�����ַ����ʧ�ܾ�û�м������ԣ�û�з���ԭ������Ϊ���Եĵ�ַ̫�ͣ�����ʹ���˸ߵ�ַ�ͳɹ���0x60000000��

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
// Ϊ�ӿ�ʵ�ֵ�
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
// �Լ��õ�
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
		GSMEM							*pHost;						// ���ڵ���GSMEM��һЩ����
		int								nSHMKey;					// �����ڴ�KEY�����Ϊ0���ʾ��ʹ�ù����ڴ�(Ĭ����0)
		int								nSHMSize;					// ��Ҫ�Ĺ����ڴ���С
		unsigned int					nFixedAddr;					// ���Է��ڹ̶����ڴ��ַ��
		int								nMAXSIZENUM;				// ������Ĳ�ͬ�ߴ����
		int								nHASHSTEP1;
		int								nHASHSTEP2;
		int								nOutputStatInterval;		// �������ͳ����־�ļ��
		int								nTQChunkSize;				// ʱ����е�ÿ�δ��Է�����С
		int								nTstOffset;					// ���ڲ��Ե��ڴ�ƫ�ƣ���Ҫ�ƶ�ƫ�Ƶ�����
		int								nTstOffset1;				// ���ڲ��Ե��ڴ�ƫ�ƣ�ԭ��ָ��Ϳ�ͷ��ƫ������
		int								nMinMemWarn;				// �������ڴ�����ʣ����ô��ʱ����notify�ϲ�

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
		TEDEAL_T	tefunc;											// ������¼��ĺ���
		// ���ݲ���
		union
		{
			int		nDummy;
		}un;
	};

protected:
	CFGINFO_T							m_cfginfo;
	int									m_nIsInherited;				// �����ڴ��Ǽ̳�����
	int									m_nMemDiff;					// ���μ̳е��ڴ���ϴ�֮���ƫ��
	whshm								*m_pSHM;					// �����ڴ�����ָ��
	MYINFO_T							*m_pmyinfo;					// ��������Ƿ��ڹ����ڴ��ͷ��
	void								*m_pMem;					// ���ڷ���Ĳ��֣�����m_aoone�õĲ��֣�
	AO_T								*m_aoone;					// �Խ�һ����ڴ棬��������С����
	AM_T								*m_am;						// ��ߴ���������
	whhash<whstr4hash, int>				m_mapKey2PtrIdx;			// �ִ���ָ�������±��ӳ�䣨�ڲ�ָ�벻�����ӳ���У�
	whtick_t							m_tickNow;					// ��ǰʱ��
	// ʱ���¼�������
	whtimequeue							m_TQ;
	// �Ƿ��Ǽ̳е��ڴ�
	bool								m_bInherited;
	// �Ƿ���QuickDetach
	bool								m_bQuickDetach;
private:
	// ���������ļ���ʼ������
	int		Init_CFG(const char *cszCFGName);
	// ���һ��key��ָ����ŵ�ӳ��
	int		AddKey2Idx(const char *cszParam);
	// ��ʼ�������ڴ����ض���
	int		InitSHMAndRelatedObjs(void *pBuf);
	// �սṲ���ڴ����ض���
	int		ReleaseSHMAndRelatedObjs();
	// �����ڲ�ָ��
	int		AdjustSHMPtrAndSetMgr();
	// �ж��Ƿ�����ͷŹ����ڴ�
	inline bool	CanFreeMem() const
	{
		return	m_am->m_nUseCount == 0 && ((!m_bInherited) || (!m_bQuickDetach));
	}

	// ��ʱ��ص�
	int		Tick_DealTE();
	void	TEDeal_OutputStat(TQUNIT_T * pTQUnit);
	void	SetTE_OutputStat();

public:
	// �����Լ����͵Ķ���
	static CMN::ILogic *	Create()
	{
		return	new	GSMEM;
	}
};

}		// EOF namespace n_pngs

#endif	// EOF __GSMEM_I_H__
