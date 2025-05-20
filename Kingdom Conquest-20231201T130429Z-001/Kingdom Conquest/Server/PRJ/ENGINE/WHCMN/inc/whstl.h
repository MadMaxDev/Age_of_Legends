// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstl.h
// Creator      : Wei Hua (魏华)
// Comment      : 用于辅助stl的函数和方法
// CreationDate : 2003-06-13

#ifndef	__WHSTL_H__
#define	__WHSTL_H__

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif
#include <map>
#include <set>
#include "whvector.h"

namespace n_whcmn
{

// 这个为了实现迅速查值
template<typename _TyKey, typename _TyVal>
class	whmultimap
{
public:
	typedef	std::set<_TyVal>							VALSET;
	typedef	std::map<_TyKey, VALSET>					MAPSET;
	typedef typename VALSET::iterator					VSIT;
	typedef typename MAPSET::iterator					MSIT;
	struct	iterator
	{
		MSIT				itmap;		// map位置
		VSIT				itset;		// set位置
	};
protected:
	MAPSET					m_map;
public:
	whmultimap()
	{
	}
	~whmultimap()
	{
	}
	// 添加__key/__val对
	bool	add(const _TyKey & __key, const _TyVal & __val)
	{
		// 判断这对是否存在
		MSIT	itSet;
		bool	bFound;
		if( !(bFound=searchKey(__key, itSet)) )
		{
			// 创建新key
			m_map.insert(std::make_pair(__key, VALSET()));
			bFound	= searchKey(__key, itSet);
		}
		if( !bFound )
		{
			// 无法添加
			return	false;
		}
		// 检查值是否已经存在
		if( itSet->second.find(__val) != itSet->second.end() )
		{
			// 该数据已经存在
			return	false;
		}

		itSet->second.insert(__val);
		return		true;
	}
	// 删除__key/__val对
	bool	remove(const _TyKey & __key, const _TyVal & __val)
	{
		iterator	it;
		if( searchKeyVal(__key, __val, it) )
		{
			it.itmap->second.erase((*it.itset));
			if( it.itmap->second.size()==0 )
			{
				m_map.erase(it.itmap);
			}
			return	true;
		}
		return		false;
	}
	// 删除__key对应的所有记录
	bool	remove(const _TyKey & __key)
	{
		// 判断这key是否存在
		MSIT	itSet;
		if( !searchKey(__key, itSet) )
		{
			// 不存在
			return	false;
		}
		// 删除该Key和所有set
		m_map.erase(itSet);
		return		true;
	}
	// 检测__key/__val对是否存在，如果为真将位置放在__it中
	bool	searchKeyVal(const _TyKey & __key, const _TyVal & __val, iterator & __it)
	{
		if( searchKey(__key, __it.itmap) )
		{
			__it.itset	= __it.itmap->second.find(__val);
			if( __it.itset != __it.itmap->second.end() )
			{
				return	true;
			}
		}
		return	false;
	}
	// 检测__key是否存在，如果存在返回set
	bool	searchKey(const _TyKey __key, MSIT &__it)
	{
		__it	= m_map.find(__key);
		if( __it==m_map.end() )
		{
			return	false;
		}
		return		true;
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHSTL_H__
