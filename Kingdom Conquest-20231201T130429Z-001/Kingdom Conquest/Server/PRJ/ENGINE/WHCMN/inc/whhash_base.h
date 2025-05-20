// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whhash_base.h
// Creator      : Wei Hua (魏华)
// Comment      : hash表的一些公共定义
//                参考了cui和stlport的实现
//                不过这里面不会自动调用类的构造函数，因为想避免delete重载后的嵌套(虽然不会出错)
//                注意：即使不使用，hashtable还是会占用一定的初始内存
//                节点的分配采用了whallocbychunk，所以value如果被分配则地址是不会变的。
// Function list: _whcmn_hashfunc	普通的hash仿函数
//
// CreationDate : 2007-09-07 从whhash.h中转移过来
// ChangeLog    :

#ifndef	__WHHASH_BASE_H__
#define	__WHHASH_BASE_H__

#include "whcmn_def.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 质数表
////////////////////////////////////////////////////////////////////
enum { NUM_PRIMES = 30};						// prime list from cui's implementation which is from stlport :)
static const unsigned int	g_prime_list[NUM_PRIMES]	=
{
  17ul,         29ul,
  53ul,         97ul,         193ul,       389ul,       769ul,
  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
  1610612741ul, 3221225473ul, 4294967291ul
};// 前两个是wh加的，因为在pnl的应用中可能经常小于10个节点(不过因为空间不减，所以可能最后还是达到很高，所以还是算了:()
inline unsigned int	_next_prime(unsigned int n)
{
	for(int i=0; i<NUM_PRIMES; i++)
	{
		if(g_prime_list[i] > n)
		{
			return g_prime_list[i];
		}
	}
	return 0xFFFFFFFF;
}

////////////////////////////////////////////////////////////////////
// 普通的hash仿函数
////////////////////////////////////////////////////////////////////
struct	_whcmn_hashfunc
{
	_whcmn_hashfunc() {}

	inline unsigned int	operator()(char data)	const	{return (unsigned int)data;}
	inline unsigned int	operator()(short data)	const	{return (unsigned int)data;}
	inline unsigned int	operator()(int data)	const	{return data;}
	inline unsigned int	operator()(long data)	const	{return data;}
	inline unsigned int	operator()(unsigned int data)	const	{return data;}
	inline unsigned int	operator()(unsigned long data)	const	{return (unsigned int)data;}
	inline unsigned int	operator()(wchar_t data) const	{return (unsigned int)data;}
	inline unsigned int	operator()(whint64 data) const	{return (unsigned int)data;}	// 这个是假定在低32位就最多的表现
	inline unsigned int	operator()(whuint64 data) const	{return (unsigned int)data;}	// 这个是假定在低32位就最多的表现
	inline unsigned int	operator()(void *data)	const
	{
		// 因为数组大小是质数，所以这个数是什么样子没有关系。
		return ((unsigned int)data);
	}
	inline unsigned int	operator()(char *s)		const
	{
		return	operator()((const char *)s);
	}
	inline unsigned int	operator()(const char *s)	const
	{
		unsigned int h = 0;
		for(;*s; s++)
		{
			h = h * 31 + *(unsigned char *)s;
		}
		return h;
	}
};

////////////////////////////////////////////////////////////////////
// hash节点
////////////////////////////////////////////////////////////////////
template <class _Key, class _Value>
struct whhash_node
{
	whhash_node	*_next;
	_Key		_key;
	_Value		_val;
	whhash_node()
	: _next(NULL)
	{
	}
	whhash_node(const whhash_node & other)
	{
		operator = (other);
	}
	// 这个等号主要是为了实现hash拷贝用的
	inline void operator = (const whhash_node & other)
	{
		_val	= other._val;
	}
};
template <class _Key>
struct whhashset_node
{
	whhashset_node	*_next;
	_Key			_key;
	whhashset_node()
	: _next(NULL)
	{
	}
	whhashset_node(const whhashset_node & other)
	{
		operator = (other);
	}
	// 这个等号主要是为了实现hash拷贝用的
	inline void operator = (const whhashset_node & other)
	{
		// do nothing
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHHASH_BASE_H__
