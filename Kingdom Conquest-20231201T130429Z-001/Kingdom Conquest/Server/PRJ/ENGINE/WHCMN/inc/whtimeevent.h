// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtimeevent.h
// Creator      : Wei Hua (魏华)
// Comment      : 时间事件
//                这个原来放在whtime.h中，但是在编译的时候出现了undefined reference to `non-virtual thunk的错误
//                当时里面是用#include <whqueue.h>。所以转移到单独的这个文件中。
// CreationDate : 2004-04-07
// ChangeLOG    : 2004-11-02 取消了NowDiff

#ifndef	__WHTIMEEVENT_H__
#define	__WHTIMEEVENT_H__

#include "whtime.h"
#include "whlist.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 固定间隔的时间Event
////////////////////////////////////////////////////////////////////
// 这个只能单线程用
class	whfixedtimeevent
{
public:
	struct	INFO_T
	{
		int	nMaxDealPerTick;								// 一个Tick最多可以连续处理的Event个数
		int	nInterval;										// 每个Event被加入后，隔这么长时间后被触发(毫秒)
		int	nMaxNum;										// 最多可以管理的Event数量
		int	nDataSize;										// 事件的附加数据的尺寸
		INFO_T()
		: nMaxDealPerTick(100)
		, nInterval(3600000)								// 默认一个小时吧
		, nMaxNum(100)
		, nDataSize(16)
		{
		}
	};
protected:
	#pragma pack(1)
	struct	EVENTUNIT_T
	{
		whtick_t	nTriggerTick;							// 应该被触发的时刻
		int			nIdx4Outside;							// 给外界看的idx
		int			nPrevIdx;								// 链表中的前一个序号
		int			nNextIdx;								// 链表中的下一个序号
		int			nSize;									// 附加数据的尺寸
		char		Data[1];								// 附加数据起始点
	};
	#pragma pack()
protected:
	INFO_T			m_info;
	char			*m_pEventBuf;							// 给whalist用的
	int				m_nEventUnitSize;						// 一个时间对象的总尺寸（EVENTUNIT_T加上后面的数据）
	int				m_nStartIdx;							// 起始事件序号
	int				m_nEndIdx;								// 终止事件序号
	int				m_nRand;								// 用于生成序号的前面的随机部分
	int				m_nMaxRand;								// nRand值1~m_nMaxRand
	whalist			m_EventQueue;							// 用于存放事件的队列
	bool			m_bDisabled;							// 查否已经被禁用
public:
	whfixedtimeevent();
	virtual ~whfixedtimeevent();
	int		Init(INFO_T *pInfo);
	int		Release();
	int		Tick();											// 工作一下（内部会检测到了时间的Event）
	int		AddEvent(void *pData, int nSize);				// 添加一个事件，nSize不能超过INFO_T::nDataSize
															// 如果成功：返回事件id（也就是idx4out），失败：返回-1
	int		AddEvent(whtick_t nTriggerTick, void *pData, int nSize);
															// 添加一个事件，并指定触发时刻（注意，需要上层需要从逻辑上保证后添加的时间靠后）
	int		DelEvent(int nIdx4Out);							// 删除相应的事件（这个一般是在事件的提交者消失后需要做的）
	bool	IsHasEvent(int nIdx4Out);						// 判断一个事件是否存在
	int		Clear();										// 清除所有的事件
	void	SetInterval(int nInterval);						// (为了调试)设置新的时间间隔
public:
	// 使用GetStartIdx、GetNextIdx、GetDataPtr可以浏览整个列表的内容
	// 注意：这里是idx而不是idx4out
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
	inline int	GetEventNum() const							// 获得目前存在的事件数量
	{
		return	m_EventQueue.GetNum();
	}
	inline void	SetDisable(bool bSet)
	{
		m_bDisabled	= bSet;
	}
private:
	EVENTUNIT_T *	_GetUnit(int nIdx4Out, int *pnIdx);		// 根据外界序号获得事件对象指针
	int	_DelEvent(int nIdx);								// 按序号删除相应的事件
	// 这个要被子类实现
	// 系统在这个被调用之前会删除相应的事件索引，不过该事件的实体还存在，可以使用
	// 一般在DealEvent中，会继续申请下一个事件
	virtual int	DealEvent(void *pData, int nSize)			= 0;
															// 处理一个事件（在处理完后该事件被删除）
};

////////////////////////////////////////////////////////////////////
// 在小范围内的定点时间Event
// 即，从某个时刻开始后在一段时间内的时间事件
// 时间单位为毫秒
////////////////////////////////////////////////////////////////////
class	whsmallrangetimeevent
{
public:
	// 初始化结构
	struct	INFO_T
	{
		int	nResolution;								// 时间分辨率
		int	nTimeRange;									// 范围总时长
		INFO_T()
		: nResolution(20)
		, nTimeRange(20000)
		{
		}
	};
	// 一个事件的结构（真正用到的地方可以从它继承）
	struct	UNIT_T
	{
		whsmallrangetimeevent	*pSRTE;
		int		nHostIdx;									// Host位置的序号（为-1表明没有附着在host上。比如在SetNowTime返回的链表中）
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
		// 尾巴连头
		void	TailLinkHead(UNIT_T *pOtherHead)
		{
			assert(pNext==NULL);
			assert(pOtherHead->pPrev==NULL);
			pNext				= pOtherHead;
			pOtherHead->pPrev	= this;
		}
		// 从所属的SRTE中移除
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
	INFO_T		m_info;										// 初始化信息
	whtick_t	m_nStartTime;								// 区域开始时间
	int			m_nNowIHostIdx;								// 逻辑当前时间对应单元host的序号
	whvector<UNITHOST_T>	m_vectUnitHost;					// 轮转的Host(逻辑序号第一个单元m_nNowIHostIdx对应m_nStartTime)
public:
	whsmallrangetimeevent();
	~whsmallrangetimeevent();
	int		Init(INFO_T *pInfo);
	int		Release();
	UNIT_T *	SetNowTime(whtick_t nNow, UNIT_T **ppTail=NULL);
															// 设置当前时间，并返回到当前时间的可用事件列表的第一个元素(没有就返回NULL，同时*ppTail也是NULL)
	int		AddUnit(whtick_t nTime, UNIT_T *pUnit);			// 添加一个在时间nTime发生的时间事件
	int		RemoveUnit(UNIT_T *pUnit);						// 删除一个事件(比如某个对象已经完成任务了，不需要再有时间事件了)
	int		GetUnitRelIdx(UNIT_T *pUnit);					// 获得单元的相对下标(即相对起点的下标)。如果没有则返回-1。
	int		GetRoundRelIdx(int nFirstRef, int nRealIdx);	// 获得nRealIdx相对nFirstRef的相对下标
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHTIMEEVENT_H__
