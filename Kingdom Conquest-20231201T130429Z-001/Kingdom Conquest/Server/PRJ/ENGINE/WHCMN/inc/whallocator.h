// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whallocator.h
// Creator      : Wei Hua (魏华)
// Comment      : 分配器
// CreationDate : 2004-01-17
// ChangeLOG    : 2005-12-16 增加whcmnallocationobj
//              : 2006-04-04 给whcmnallocator::New函数增加了_Ty*参数，否则到g++下面不能直接使用模板参数的静态薄板方法。顺便庆祝我生日，hiahia。

#ifndef	__WHALLOCATOR_H__
#define	__WHALLOCATOR_H__

#include <new>
#include <stdlib.h>
#include <assert.h>

namespace	n_whcmn
{

// 这个是以对象方法的方式提供分配函数
// （主要是为了将来能扩展到chunk的分配）
class	whcmnallocationobj
{
public:
	inline void *	Alloc(size_t nSize)
	{
		return	::malloc( nSize );
	}
	inline void *	Realloc(void *pBuf, size_t nSize)
	{
		return	::realloc(pBuf, nSize);
	}
	inline void	Free(void *pBuf)
	{
		::free(pBuf);
	}
};
// 这个主要是为了在没有设置_AllocationObj的情况下也能进行普通的分配
template<class _AllocationObj>
class	whfakeallocationobj	: whcmnallocationobj
{
private:
	_AllocationObj	*m_pAO;
public:
	whfakeallocationobj(_AllocationObj *pAO=NULL)	: m_pAO(pAO)
	{
	}
	whfakeallocationobj(const whfakeallocationobj &other)	: m_pAO(other.m_pAO)
	{
	}
	inline void	SetAO(_AllocationObj *pAO)
	{
		m_pAO	= pAO;
	}
	inline _AllocationObj *	GetAO()
	{
		return	m_pAO;
	}
	inline void	AdjustInnerPtr(int nOffset)
	{
		if( m_pAO )
		{
			wh_adjustaddrbyoffset(m_pAO, nOffset);
		}
	}
	inline void *	Alloc(size_t nSize)
	{
		if( m_pAO )
		{
			return	m_pAO->Alloc( nSize );
		}
		else
		{
			return	whcmnallocationobj::Alloc( nSize );
		}
	}
	inline void *	Realloc(void *pBuf, size_t nSize)
	{
		if( m_pAO )
		{
			return	m_pAO->Realloc( pBuf, nSize );
		}
		else
		{
			return	whcmnallocationobj::Realloc( pBuf, nSize );
		}
	}
	inline void	Free(void *pBuf)
	{
		if( m_pAO )
		{
			return	m_pAO->Free( pBuf );
		}
		else
		{
			return	whcmnallocationobj::Free( pBuf );
		}
	}
};
// 基于allocationobj的new和delete
template<class _AllocationObj>
class	whAND
{
private:
	_AllocationObj	*m_pAllocator;
public:
	whAND(_AllocationObj *pAllocator=NULL)
	: m_pAllocator(pAllocator)
	{
	}
	inline void	SetAO(_AllocationObj *pAllocator)
	{
		m_pAllocator	= pAllocator;
	}
	inline _AllocationObj *	GetAO()
	{
		return	m_pAllocator;
	}
	template<typename _Ty>
	inline void	AllocByType(_Ty *&ptr)
	{
		ptr	= (_Ty *)Alloc(sizeof(_Ty));
	}
	inline void *	Alloc(size_t nSize)
	{
		return	m_pAllocator->Alloc(nSize);
	}
	inline void *	Realloc(void *pBuf, size_t nSize)
	{
		return	m_pAllocator->Realloc(pBuf, nSize);
	}
	inline void	Free(void *pBuf)
	{
		m_pAllocator->Free(pBuf);
	}
	// New后面的参数只是一个用来表示类型的空指针
	// 用法New((_Type*)NULL)
	template<typename _Ty>
	inline bool	New(_Ty *&ptr)
	{
		ptr	= new (Alloc(sizeof(_Ty))) _Ty;
		return	true;
	}
	template<typename _Ty>
	void		Delete(_Ty *ptr)
	{
		// 调用析构
		ptr->~_Ty();
		// 释放内存
		Free(ptr);
	}
	template<typename _Ty>
	_Ty *	NewArray(_Ty *, size_t nNum)
	{
		// 必须一个一个调用构造，因为new数组会多出四个字节来记录数组元素个数
		_Ty *pArr	= (_Ty *)Alloc(nNum*sizeof(_Ty));
		if( !pArr )	return	false;
		_Ty *pUnit	= pArr;
		for(size_t i=0;i<nNum;i++)
		{
			new (pUnit) _Ty;
			pUnit	++;
		}
		return	pArr;
	}
	template<typename _Ty>
	_Ty *	RenewArray(_Ty *ptr, size_t nOldNum, size_t nNewNum)
	{
		// 一般都应该是nNewSize>nOldSize
		int		nDiff	= nNewNum - nOldNum;
		if( nDiff<=0 )
		{
			return	ptr;
		}
		_Ty *pNew	= (_Ty *)Realloc(ptr, sizeof(_Ty)*nNewNum);
		if( !pNew )
		{
			return	NULL;
		}
		// 调用后面没有初始化的部分的构造函数
		_Ty *pUnit	= pNew+nOldNum;
		while(nDiff-- > 0)
		{
			new (pUnit) _Ty;
			pUnit	++;
		}
		return	pNew;
	}
	template<typename _Ty>
	void		DeleteArray(_Ty *pArr, size_t nNum)
	{
		// 调用所有东西的析构
		for(size_t i=0;i<nNum;i++)
		{
			pArr[i].~_Ty();
		}
		// 释放内存
		Free(pArr);
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHALLOCATOR_H__
