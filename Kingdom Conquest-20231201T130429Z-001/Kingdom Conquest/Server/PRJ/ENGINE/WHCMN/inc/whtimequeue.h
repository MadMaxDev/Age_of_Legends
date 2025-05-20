// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtimequeue.h
// Creator      : Wei Hua (魏华)
// Comment      : 时间队列(按毫秒算可以放置间隔达49.7/2天的事件)
// CreationDate : 2005-04-07
// ChangeLOG    : 2006-08-15 按刘岩要求增加了时间队列的clear方法
//              : 2007-07-31 保证同一时刻内先注册的事件先发生

#ifndef	__WHTIMEQUEUE_H__
#define	__WHTIMEQUEUE_H__

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif

#include <set>
#include "./whtime.h"
#include "./whunitallocator.h"

namespace n_whcmn
{

class	whtimequeue
{
public:
	// 定义
	// 初始化参数
	struct	INFO_T
	{
		// 这两个参数用于初始化whsmpunitallocator
		// nChunkSize表示每一批分配多少单元
		int				nUnitLen;
		int				nChunkSize;
		INFO_T()
			: nUnitLen(4)
			, nChunkSize(128)
		{
		}
	};
	// ID结构
	// 注意：可以在ID没有被释放的时候重新给这个ID赋予新值，但是它原来指向的ID就不能自动释放了。所以外面应该有地方先记录了ID，给予释放。例如：一般的TEDeal函数。
	struct	ID_T
	{
		whtick_t		t;
		int				nRotate;		// 轮转参数，用它来保证同一时刻的先后顺序
		int				idx;
		whtimequeue		*pHost;
		ID_T()
		: t(0)
		, nRotate(0)
		, idx(-1)
		, pHost(NULL)
		{
		}
		~ID_T()
		{
			// !!!!不要在这里调用DelFromQueue()，因为ID_T也许被赋值，交换，所以可能导致多次被删除的现象
			// 所以需要拥有者在适当的时候自己调用quit
		}
		inline void	clear()
		{
			t		= 0;
			// nRotate	= 0;	这个可以不用管，反正将来会重新填写的
			idx		= -1;
			pHost	= NULL;
		}
		// 主动从队列中退出
		// 因为有了这个所以要保证所有和timequeue相关的对象都必须在timequeue终结之前调用quit，否则就干脆不要调用
		inline int	quit()
		{
			if( pHost )
			{
				// 这个Del其实也就导致ID_T的clear了
				return	pHost->Del(*this);
			}
			return	0;
		}
		inline bool	IsValid() const
		{
			return	idx>=0;
		}
		inline bool operator <	(const ID_T &other) const
		{
			int	d	= wh_tickcount_diff(t, other.t);
			// 时间优先，ID辅助
			if( d!=0 )	return d<0;
			// 时间相等才比较rotate（同一个tick内的rotate一定不会一样的）
			if( nRotate!=other.nRotate)	return nRotate<other.nRotate;
			// rotate相等才比较idx（按理来说rotate不会不一样的，除非在同一时刻加入了整数那么多事件）
			return	idx < other.idx;
		}
		inline bool	operator == (const ID_T &other) const
		{
			return	t==other.t && idx==other.idx && nRotate==other.nRotate;
		}
	};
private:
	typedef	std::set<ID_T>	idset_t;
private:
	// 成员变量
	INFO_T				m_info;
	idset_t				m_idset;
	whsmpunitallocator	*m_punits;
	int					m_nRotate;
public:
	whtimequeue();
	~whtimequeue();
	int		Init(INFO_T *pInfo);
	int		Release();
	// 清空队列
	void	Clear();
	// 添加事件
	int 	Add(whtick_t t, void *pUnit, whtimequeue::ID_T *pID);
	int 	AddGetRef(whtick_t t, void **ppUnit, whtimequeue::ID_T *pID);
	// 删除事件
	int		Del(ID_T &id);
	// 修改时间
	int		TimeMove(whtimequeue::ID_T *pID, whtick_t newt);
	// 根据时间获得在这之前的最老的一个事件（id和数据）
	// 如果返回成功0，则使用完毕后自己记得删除
	int		GetUnitBeforeTime(whtick_t t, void **ppUnit, ID_T *pID);
	// 这个nIdx就是ID_T中的idx
	inline char *	GetUnitPtr(int nIdx)
	{
		return	m_punits->getptr(nIdx);
	}
	// 获得事件数量
	inline int		size() const
	{
		assert((int)m_idset.size() == m_punits->size());
		return	m_idset.size();
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHTIMEQUEUE_H__
