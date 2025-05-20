// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtimeevent.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 时间事件
// CreationDate : 2004-04-07

#include "../inc/whtimeevent.h"
#include <string.h>

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whfixedtimeevent
////////////////////////////////////////////////////////////////////
whfixedtimeevent::whfixedtimeevent()
: m_pEventBuf(NULL)
, m_nEventUnitSize(0)
, m_nStartIdx(whalist::INVALIDIDX)
, m_nEndIdx(whalist::INVALIDIDX)
, m_nRand(0), m_nMaxRand(0x7FFF)
, m_bDisabled(false)
{
}
whfixedtimeevent::~whfixedtimeevent()
{
	Release();
}
int	whfixedtimeevent::Init(INFO_T *pInfo)
{
	int		rst;

	// 不能超过这么多
	if(pInfo->nMaxNum > 0xFFFF)
	{
		assert(0);
		return	-1;
	}

	m_info				= *pInfo;

	m_nMaxRand			= 0x7FFF;

	m_nEventUnitSize	= sizeof(EVENTUNIT_T)-1 + m_info.nDataSize;

	m_pEventBuf	= new char[whalist::CalcTotalSize(m_nEventUnitSize, m_info.nMaxNum)];
	assert(m_pEventBuf);

	// 初始化队列
	rst		= m_EventQueue.Init(m_pEventBuf, m_nEventUnitSize, m_info.nMaxNum);
	if( rst<0 )
	{
		return	-1;
	}
	m_nStartIdx	= whalist::INVALIDIDX;
	m_nEndIdx	= whalist::INVALIDIDX;

	return	0;
}
int	whfixedtimeevent::Release()
{
	if( m_pEventBuf )
	{
		delete []	m_pEventBuf;
		m_pEventBuf	= NULL;
	}
	return	0;
}
int	whfixedtimeevent::Tick()
{
	if( m_bDisabled )
	{
		return	0;
	}

	// 
	whtick_t	nNow	= wh_gettickcount();
	int			nCount	= 0;
	// 从头开始判断是否有到时间的
	while( m_nStartIdx != whalist::INVALIDIDX )
	{
		EVENTUNIT_T	*pUnit	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(m_nStartIdx);
		assert(pUnit);
		int	nDiff		= wh_tickcount_diff(nNow, pUnit->nTriggerTick);
		if( nDiff<0 )
		{
			// 说明还没到时间，结束
			break;
		}
		// 删除这个事件（m_nStartIdx内部会转到下一个，并且这个pUnit指针不会失效）
		_DelEvent(m_nStartIdx);
		// 处理事件(里面也许会再加一个到队列里去)
		DealEvent(pUnit->Data, pUnit->nSize);
		// 看是不是太多了
		if( (++nCount)>=m_info.nMaxDealPerTick )
		{
			break;
		}
	}
	return	0;
}
int	whfixedtimeevent::AddEvent(void *pData, int nSize)
{
	return	AddEvent(wh_gettickcount() + m_info.nInterval, pData, nSize);
}
int	whfixedtimeevent::AddEvent(whtick_t nTriggerTick, void *pData, int nSize)
{
	assert(nSize <= m_info.nDataSize);
	int	nIdx	= m_EventQueue.Alloc();
	if( nIdx<0 )
	{
		return	-1;
	}
	EVENTUNIT_T	*pUnit	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(nIdx);
	assert( pUnit );
	pUnit->nTriggerTick	= nTriggerTick;
	pUnit->nSize		= nSize;
	memcpy(pUnit->Data, pData, nSize);
	// 加入链表
	pUnit->nNextIdx		= whalist::INVALIDIDX;
	pUnit->nPrevIdx		= m_nEndIdx;
	EVENTUNIT_T	*pPrev	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(m_nEndIdx);
	if( pPrev )
	{
		pPrev->nNextIdx	= nIdx;
	}
	else
	{
		m_nStartIdx		= nIdx;
	}
	m_nEndIdx			= nIdx;

	if( (++m_nRand) > m_nMaxRand )
	{
		m_nRand			= 1;
	}
	pUnit->nIdx4Outside	= (m_nRand<<16) | nIdx;
	return				pUnit->nIdx4Outside;
}
int	whfixedtimeevent::Clear()
{
	m_EventQueue.Reset();
	m_nStartIdx	= whalist::INVALIDIDX;
	m_nEndIdx	= whalist::INVALIDIDX;
	return		0;
}
void	whfixedtimeevent::SetInterval(int nInterval)
{
	m_info.nInterval	= nInterval;
}
int	whfixedtimeevent::DelEvent(int nIdx4Out)
{
	int			nIdx;
	EVENTUNIT_T	*pUnit	= _GetUnit(nIdx4Out, &nIdx);
	if( !pUnit )
	{
		return	-1;
	}
	return		_DelEvent(nIdx);
}
int	whfixedtimeevent::_DelEvent(int nIdx)
{
	EVENTUNIT_T	*pUnit	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(nIdx);
	if( !pUnit )
	{
		return	-1;
	}

	EVENTUNIT_T	*pPrev	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(pUnit->nPrevIdx);
	EVENTUNIT_T	*pNext	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(pUnit->nNextIdx);

	if( pPrev )
	{
		pPrev->nNextIdx	= pUnit->nNextIdx;
	}
	else
	{
		m_nStartIdx		= pUnit->nNextIdx;
	}
	if( pNext )
	{
		pNext->nPrevIdx	= pUnit->nPrevIdx;
	}
	else
	{
		m_nEndIdx		= pUnit->nPrevIdx;
	}

	// 能到这里应该不会出错的
	if( m_EventQueue.Free(nIdx)<0 )
	{
		assert(0);
	}
	return	0;
}
bool	whfixedtimeevent::IsHasEvent(int nIdx4Out)
{
	return	_GetUnit(nIdx4Out, NULL) != NULL;
}

whfixedtimeevent::EVENTUNIT_T *	whfixedtimeevent::_GetUnit(int nIdx4Out, int *pnIdx)
{
	if( m_nStartIdx==whalist::INVALIDIDX )
	{
		// 没有东西了
		return	NULL;
	}

	int	nIdx			= nIdx4Out & 0xFFFF;

	EVENTUNIT_T	*pUnit	= (EVENTUNIT_T *)m_EventQueue.GetDataUnitPtr(nIdx);
	if( !pUnit )
	{
		return	NULL;
	}
	if( pUnit->nIdx4Outside != nIdx4Out )
	{
		return	NULL;
	}

	if( pnIdx )
	{
		*pnIdx	= nIdx;
	}
	return	pUnit;
}

////////////////////////////////////////////////////////////////////
// whsmallrangetimeevent
////////////////////////////////////////////////////////////////////
whsmallrangetimeevent::whsmallrangetimeevent()
: m_nStartTime(0)
, m_nNowIHostIdx(-1)
{
}
whsmallrangetimeevent::~whsmallrangetimeevent()
{
}
int		whsmallrangetimeevent::Init(INFO_T *pInfo)
{
	if( m_vectUnitHost.size()>0 )
	{
		assert(0);
		return	-1;
	}
	memcpy(&m_info, pInfo, sizeof(m_info));
	// 申请足够多的host
	m_vectUnitHost.resize(m_info.nTimeRange / m_info.nResolution);
	// 清空指针
	memset(m_vectUnitHost.getbuf(), 0, m_vectUnitHost.totalbytes());
	// 从头开始
	m_nStartTime	= 0;
	m_nNowIHostIdx	= -1;
	return	0;
}
int		whsmallrangetimeevent::Release()
{
	// 清空指针(为了保险起见)
	memset(m_vectUnitHost.getbuf(), 0, m_vectUnitHost.totalbytes());
	// 尺寸缩水
	m_vectUnitHost.clear();
	return	0;
}
whsmallrangetimeevent::UNIT_T *	whsmallrangetimeevent::SetNowTime(whtick_t nNow, whsmallrangetimeevent::UNIT_T **ppTail)
{
	if( ppTail )
	{
		*ppTail		= NULL;
	}

	// 从上次开始到nNow有几个
	int	nCount=0;
	if( m_nNowIHostIdx<0 )
	{
		// 第一次
		m_nNowIHostIdx	= 0;
		m_nStartTime	= nNow;
		return	NULL;
	}
	// if( nNow<=m_nStartTime )
	int	nDiff	= wh_tickcount_diff(nNow, m_nStartTime);
	if( nDiff<=0 )
	{
		// 这个可能是因为上次时间的取整造成的
		return	NULL;
	}
	nCount	= nDiff / m_info.nResolution;
	if( nCount==0 )
	{
		nCount	= 1;
	}
	else if( nCount>(int)m_vectUnitHost.size() )
	{
		nCount	= m_vectUnitHost.size();
	}
	// 从m_nNowIHostIdx开始处理nCount个Host
	UNIT_T	*pHead	= NULL;
	UNIT_T	*pTail	= NULL;
	for(int i=0;i<nCount;i++)
	{
		UNITHOST_T	*pHost	= m_vectUnitHost.getsaferoundptr(m_nNowIHostIdx, i);
		if( pHost->pHead )
		{
			if( !pHead )
			{
				pHead		= pHost->pHead;
				pTail		= pHost->pTail;
			}
			else
			{
				// 把这个和原来的连起来(注意可不能用InsertAfterMe那样就只有phead被插入了)
				pTail->TailLinkHead(pHost->pHead);
				// 原来的对尾指向这个
				pTail		= pHost->pTail;
			}
			pHost->clear();
		}
		// 将来视情况看是否要清除每个Unit的nHostIdx !!!!!!!!
	}

	// 参考时间移动到现在
	m_nNowIHostIdx	= m_vectUnitHost.getsaferoundidx(m_nNowIHostIdx, nCount);
	m_nStartTime	+= nCount*m_info.nResolution;
	if( ppTail )
	{
		*ppTail		= pTail;
	}

	// 清除每个Unit和Host的联系
	UNIT_T	*pTmp	= pHead;
	while( pTmp )
	{
		pTmp->leavehost();
		pTmp		= pTmp->pNext;
	}

	return	pHead;
}
int		whsmallrangetimeevent::AddUnit(whtick_t nTime, whsmallrangetimeevent::UNIT_T *pUnit)
{
	if( pUnit->pSRTE
	&&  pUnit->pSRTE != this)
	{
		// 不是我的
		assert(0);
		return	-1;
	}

	if( pUnit->nHostIdx>=0 )
	{
		assert(0);	// 不能把一个事件加入两次!!!!
		// 在Release版就先把原来的删除
		RemoveUnit(pUnit);
	}

	// 为了保险先看是否已经设置时间了
	if( m_nNowIHostIdx<0 )
	{
		// 第一次使用，需要手动设置一下现在为开始时刻
		SetNowTime(wh_gettickcount());
	}

	// 计算nTime在那个槽内
	// nTime==0表示加入到头部
	int	nDiff	= wh_tickcount_diff(nTime, m_nStartTime);
	if( nTime==0 || nDiff<0 )
	{
		// 不能小的，如果小可能减出来的是负数
		nTime	= m_nStartTime;
		nDiff	= 0;
	}
	int		nIdx	= nDiff / m_info.nResolution;
	nIdx			= m_vectUnitHost.getsaferoundidx(m_nNowIHostIdx, nIdx);
	UNITHOST_T		*pHost	= m_vectUnitHost.getptr(nIdx);
	if( !pHost->pTail )
	{
		// 第一次加入
		pUnit->pPrev	= NULL;
		pUnit->pNext	= NULL;
		pHost->pHead	= pUnit;
		pHost->pTail	= pUnit;
	}
	else
	{
		// 放到尾部
		pUnit->pPrev	= pHost->pTail;
		pUnit->pNext	= NULL;
		pHost->pTail->pNext	= pUnit;
		pHost->pTail	= pUnit;
	}
	pUnit->nHostIdx		= nIdx;
	pUnit->pSRTE		= this;
	return	0;
}
int		whsmallrangetimeevent::RemoveUnit(whsmallrangetimeevent::UNIT_T *pUnit)
{
	if( pUnit->pSRTE )
	{
		if( pUnit->pSRTE != this)
		{
			// 不是我的
			assert(0);
			return	-1;
		}
	}
	else
	{
		// 应该已经被Remove过了
		return	-1;
	}

	UNITHOST_T	*pHost	= NULL;
	if( pUnit->nHostIdx>=0 )
	{
		pHost			= m_vectUnitHost.getptr(pUnit->nHostIdx);
	}
	if( !pHost )
	{
		// 已经被删除了(多删除几次是可以地，不过不能多插入，所以那里会有assert)
		return			-1;
	}

	// 从对象的队伍中脱离
	if( pUnit->pPrev )
	{
		pUnit->pPrev->pNext	= pUnit->pNext;
	}
	else
	{
		pHost->pHead		= pUnit->pNext;
	}
	if( pUnit->pNext )
	{
		pUnit->pNext->pPrev	= pUnit->pPrev;
	}
	else
	{
		pHost->pTail		= pUnit->pPrev;
	}
	if( !pHost->pTail && pHost->pHead )
	{
		assert(0);
	}

	// 清空对象
	pUnit->clear();
	return	0;
}
int		whsmallrangetimeevent::GetUnitRelIdx(UNIT_T *pUnit)
{
	UNITHOST_T	*pHost	= NULL;
	if( pUnit->nHostIdx>=0 )
	{
		pHost			= m_vectUnitHost.getptr(pUnit->nHostIdx);
	}
	if( !pHost )
	{
		// 应该是Unit已经被删除了
		return			-1;
	}
	return	GetRoundRelIdx(m_nNowIHostIdx, pUnit->nHostIdx);
}
int		whsmallrangetimeevent::GetRoundRelIdx(int nFirstRef, int nRealIdx)
{
	if( nRealIdx>=nFirstRef )
	{
		return	nRealIdx - nFirstRef;
	}
	return		nRealIdx + m_vectUnitHost.size()-m_nNowIHostIdx;
}
