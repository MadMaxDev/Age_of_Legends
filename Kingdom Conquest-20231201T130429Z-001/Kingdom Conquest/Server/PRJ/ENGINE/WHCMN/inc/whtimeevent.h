// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtimeevent.h
// Creator      : Wei Hua (κ��)
// Comment      : ʱ���¼�
//                ���ԭ������whtime.h�У������ڱ����ʱ�������undefined reference to `non-virtual thunk�Ĵ���
//                ��ʱ��������#include <whqueue.h>������ת�Ƶ�����������ļ��С�
// CreationDate : 2004-04-07
// ChangeLOG    : 2004-11-02 ȡ����NowDiff

#ifndef	__WHTIMEEVENT_H__
#define	__WHTIMEEVENT_H__

#include "whtime.h"
#include "whlist.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// �̶������ʱ��Event
////////////////////////////////////////////////////////////////////
// ���ֻ�ܵ��߳���
class	whfixedtimeevent
{
public:
	struct	INFO_T
	{
		int	nMaxDealPerTick;								// һ��Tick���������������Event����
		int	nInterval;										// ÿ��Event������󣬸���ô��ʱ��󱻴���(����)
		int	nMaxNum;										// �����Թ����Event����
		int	nDataSize;										// �¼��ĸ������ݵĳߴ�
		INFO_T()
		: nMaxDealPerTick(100)
		, nInterval(3600000)								// Ĭ��һ��Сʱ��
		, nMaxNum(100)
		, nDataSize(16)
		{
		}
	};
protected:
	#pragma pack(1)
	struct	EVENTUNIT_T
	{
		whtick_t	nTriggerTick;							// Ӧ�ñ�������ʱ��
		int			nIdx4Outside;							// ����翴��idx
		int			nPrevIdx;								// �����е�ǰһ�����
		int			nNextIdx;								// �����е���һ�����
		int			nSize;									// �������ݵĳߴ�
		char		Data[1];								// ����������ʼ��
	};
	#pragma pack()
protected:
	INFO_T			m_info;
	char			*m_pEventBuf;							// ��whalist�õ�
	int				m_nEventUnitSize;						// һ��ʱ�������ܳߴ磨EVENTUNIT_T���Ϻ�������ݣ�
	int				m_nStartIdx;							// ��ʼ�¼����
	int				m_nEndIdx;								// ��ֹ�¼����
	int				m_nRand;								// ����������ŵ�ǰ����������
	int				m_nMaxRand;								// nRandֵ1~m_nMaxRand
	whalist			m_EventQueue;							// ���ڴ���¼��Ķ���
	bool			m_bDisabled;							// ����Ѿ�������
public:
	whfixedtimeevent();
	virtual ~whfixedtimeevent();
	int		Init(INFO_T *pInfo);
	int		Release();
	int		Tick();											// ����һ�£��ڲ����⵽��ʱ���Event��
	int		AddEvent(void *pData, int nSize);				// ���һ���¼���nSize���ܳ���INFO_T::nDataSize
															// ����ɹ��������¼�id��Ҳ����idx4out����ʧ�ܣ�����-1
	int		AddEvent(whtick_t nTriggerTick, void *pData, int nSize);
															// ���һ���¼�����ָ������ʱ�̣�ע�⣬��Ҫ�ϲ���Ҫ���߼��ϱ�֤����ӵ�ʱ�俿��
	int		DelEvent(int nIdx4Out);							// ɾ����Ӧ���¼������һ�������¼����ύ����ʧ����Ҫ���ģ�
	bool	IsHasEvent(int nIdx4Out);						// �ж�һ���¼��Ƿ����
	int		Clear();										// ������е��¼�
	void	SetInterval(int nInterval);						// (Ϊ�˵���)�����µ�ʱ����
public:
	// ʹ��GetStartIdx��GetNextIdx��GetDataPtr������������б������
	// ע�⣺������idx������idx4out
	inline int	GetStartIdx() const
	{
		return	m_nStartIdx;
	}
	inline int	GetNextIdx(int nIdx) const
	{
		EVENTUNIT_T	*pUnit	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(nIdx);
		if( pUnit )
		{
			return	pUnit->nNextIdx;
		}
		return	whalist::INVALIDIDX;
	}
	inline void	*	GetDataPtr(int nIdx) const
	{
		EVENTUNIT_T	*pUnit	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(nIdx);
		if( pUnit )
		{
			return	pUnit->Data;
		}
		return	NULL;
	}
	inline int	GetEventNum() const							// ���Ŀǰ���ڵ��¼�����
	{
		return	m_EventQueue.GetNum();
	}
	inline void	SetDisable(bool bSet)
	{
		m_bDisabled	= bSet;
	}
private:
	EVENTUNIT_T *	_GetUnit(int nIdx4Out, int *pnIdx);		// ���������Ż���¼�����ָ��
	int	_DelEvent(int nIdx);								// �����ɾ����Ӧ���¼�
	// ���Ҫ������ʵ��
	// ϵͳ�����������֮ǰ��ɾ����Ӧ���¼��������������¼���ʵ�廹���ڣ�����ʹ��
	// һ����DealEvent�У������������һ���¼�
	virtual int	DealEvent(void *pData, int nSize)			= 0;
															// ����һ���¼����ڴ��������¼���ɾ����
};

////////////////////////////////////////////////////////////////////
// ��С��Χ�ڵĶ���ʱ��Event
// ������ĳ��ʱ�̿�ʼ����һ��ʱ���ڵ�ʱ���¼�
// ʱ�䵥λΪ����
////////////////////////////////////////////////////////////////////
class	whsmallrangetimeevent
{
public:
	// ��ʼ���ṹ
	struct	INFO_T
	{
		int	nResolution;								// ʱ��ֱ���
		int	nTimeRange;									// ��Χ��ʱ��
		INFO_T()
		: nResolution(20)
		, nTimeRange(20000)
		{
		}
	};
	// һ���¼��Ľṹ�������õ��ĵط����Դ����̳У�
	struct	UNIT_T
	{
		whsmallrangetimeevent	*pSRTE;
		int		nHostIdx;									// Hostλ�õ���ţ�Ϊ-1����û�и�����host�ϡ�������SetNowTime���ص������У�
		UNIT_T	*pPrev;
		UNIT_T	*pNext;
		UNIT_T()
		: pSRTE(NULL)
		, nHostIdx(-1)
		, pPrev(NULL)
		, pNext(NULL)
		{
		}
		inline void	leavehost()
		{
			pSRTE		= NULL;
			nHostIdx	= -1;
		}
		inline void	clear()
		{
			pSRTE		= NULL;
			nHostIdx	= -1;
			pPrev		= NULL;
			pNext		= NULL;
		}
		void	InsertAfterMe(UNIT_T *pOther)
		{
			pOther->pPrev	= this;
			pOther->pNext	= pNext;
			pNext			= pOther;
			if( pOther->pNext )
			{
				pOther->pNext->pPrev	= pOther;
			}
		}
		void	InsertBeforeMe(UNIT_T *pOther)
		{
			pOther->pPrev	= pPrev;
			pOther->pNext	= this;
			pPrev			= pOther;
			if( pOther->pPrev )
			{
				pOther->pPrev->pNext	= pOther;
			}
		}
		// β����ͷ
		void	TailLinkHead(UNIT_T *pOtherHead)
		{
			assert(pNext==NULL);
			assert(pOtherHead->pPrev==NULL);
			pNext				= pOtherHead;
			pOtherHead->pPrev	= this;
		}
		// ��������SRTE���Ƴ�
		int		RemoveSelfFromSRTE()
		{
			if( !pSRTE )
			{
				return	-1;
			}
			return	pSRTE->RemoveUnit(this);
		}
	};
	struct	UNITHOST_T
	{
		UNIT_T	*pHead;
		UNIT_T	*pTail;
		UNITHOST_T()
		: pHead(NULL)
		, pTail(NULL)
		{
		}
		inline void	clear()
		{
			pHead	= NULL;
			pTail	= NULL;
		}
	};
private:
	INFO_T		m_info;										// ��ʼ����Ϣ
	whtick_t	m_nStartTime;								// ����ʼʱ��
	int			m_nNowIHostIdx;								// �߼���ǰʱ���Ӧ��Ԫhost�����
	whvector<UNITHOST_T>	m_vectUnitHost;					// ��ת��Host(�߼���ŵ�һ����Ԫm_nNowIHostIdx��Ӧm_nStartTime)
public:
	whsmallrangetimeevent();
	~whsmallrangetimeevent();
	int		Init(INFO_T *pInfo);
	int		Release();
	UNIT_T *	SetNowTime(whtick_t nNow, UNIT_T **ppTail=NULL);
															// ���õ�ǰʱ�䣬�����ص���ǰʱ��Ŀ����¼��б�ĵ�һ��Ԫ��(û�оͷ���NULL��ͬʱ*ppTailҲ��NULL)
	int		AddUnit(whtick_t nTime, UNIT_T *pUnit);			// ���һ����ʱ��nTime������ʱ���¼�
	int		RemoveUnit(UNIT_T *pUnit);						// ɾ��һ���¼�(����ĳ�������Ѿ���������ˣ�����Ҫ����ʱ���¼���)
	int		GetUnitRelIdx(UNIT_T *pUnit);					// ��õ�Ԫ������±�(����������±�)�����û���򷵻�-1��
	int		GetRoundRelIdx(int nFirstRef, int nRealIdx);	// ���nRealIdx���nFirstRef������±�
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHTIMEEVENT_H__
