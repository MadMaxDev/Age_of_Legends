// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whset.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的集合(set/multiset)，所谓set就是可以sort的
// CreationDate : 2004-04-21
// ChangeLog    :

#ifndef	__WHSET_H__
#define	__WHSET_H__

#include "whlist.h"
#include <assert.h>

// 效率太差啦，:(((((((((((((
/*
namespace n_whcmn
{

template<typename _Ty, class _Allocator=whcmnallocator>
class	whset	: public whlist<_Ty, _Allocator>
{
protected:
	// 是否是降序
	bool	m_bDescent;
public:
	whset()
	: whlist<_Ty, _Allocator>()
	, m_bDescent(false)
	{}
	whset(int nChunkSize)
	: whlist<_Ty, _Allocator>(nChunkSize)
	, m_bDescent(false)
	{}
	inline void	SetDescent(bool bSet)
	{
		// 现在暂时不支持
		assert(0);
		m_bDescent	= bSet;
	}
	// 加入(注：erase可以用whlist的)
	bool	insert(const _Ty &val)
	{
		// 判断原来有没有
		iterator	it;
		if( find(val, it) )
		{
			// 原来有，无法加入
			return	false;
		}
		insertbefore(it, val);
		return	0;
	}
	// 获取(如果获取不到，则itref中存放的是恰好可以insertbefore的it)
	bool	find(const _Ty &val, iterator &itref)
	{
		if( size()==0 )
		{
			itref	= end();
			return	false;
		}
		// 用二分发查找
		int			nSLen	= size();
		iterator	itbegin	= begin();
		iterator	itend	= end();
		while( nSLen>0 )
		{
			if( nSLen == 1 )
			{
				itref	= itbegin;
				if( m_bDescent )
				{
					// 降序
				}
				else
				{
					// 升序
					// 判断和val的关系
					if( (*itref) < val )
					{
						// 这个位置下一个就是可以insertbefore的位置
						++	itref;
						return	false;
					}
					else if( val < (*itref) )
					{
						// 这个位置就是可以insertbefore的位置
						return	false;
					}
				}
				// 相等
				return	true;
			}
			// 计算查找范围
			int	nSplitIdx	= nSLen/2 - 1;
			// 查找到目标点
			itref			= itbegin;
			itref			+= nSplitIdx;
			// 看是否已经相等
			if( (*itref) == val )
			{
				return	true;
			}
			// 看排列顺序
			if( m_bDescent )
			{
				// 降序
			}
			else
			{
				// 升序
				if( (*itref) < val )
				{
					// 则在该点下一个到尾的范围内
					itbegin	= itref;
					++	itbegin;
					nSLen	-= (nSplitIdx+1);
				}
				else
				{
					// 则在头到该点的范围内
					itend	= itref;
					nSLen	= nSplitIdx+1;
				}
			}
		}
		// 没找到
		return	false;
	}
};
*/

}		// EOF namespace n_whcmn

#endif	// EOF __WHSET_H__