// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whhash_base.h
// Creator      : Wei Hua (κ��)
// Comment      : hash���һЩ��������
//                �ο���cui��stlport��ʵ��
//                ���������治���Զ�������Ĺ��캯������Ϊ�����delete���غ��Ƕ��(��Ȼ�������)
//                ע�⣺��ʹ��ʹ�ã�hashtable���ǻ�ռ��һ���ĳ�ʼ�ڴ�
//                �ڵ�ķ��������whallocbychunk������value������������ַ�ǲ����ġ�
// Function list: _whcmn_hashfunc	��ͨ��hash�º���
//
// CreationDate : 2007-09-07 ��whhash.h��ת�ƹ���
// ChangeLog    :

#ifndef	__WHHASH_BASE_H__
#define	__WHHASH_BASE_H__

#include "whcmn_def.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// ������
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
};// ǰ������wh�ӵģ���Ϊ��pnl��Ӧ���п��ܾ���С��10���ڵ�(������Ϊ�ռ䲻�������Կ�������Ǵﵽ�ܸߣ����Ի�������:()
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
// ��ͨ��hash�º���
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
	inline unsigned int	operator()(whint64 data) const	{return (unsigned int)data;}	// ����Ǽٶ��ڵ�32λ�����ı���
	inline unsigned int	operator()(whuint64 data) const	{return (unsigned int)data;}	// ����Ǽٶ��ڵ�32λ�����ı���
	inline unsigned int	operator()(void *data)	const
	{
		// ��Ϊ�����С�������������������ʲô����û�й�ϵ��
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
// hash�ڵ�
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
	// ����Ⱥ���Ҫ��Ϊ��ʵ��hash�����õ�
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
	// ����Ⱥ���Ҫ��Ϊ��ʵ��hash�����õ�
	inline void operator = (const whhashset_node & other)
	{
		// do nothing
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHHASH_BASE_H__
