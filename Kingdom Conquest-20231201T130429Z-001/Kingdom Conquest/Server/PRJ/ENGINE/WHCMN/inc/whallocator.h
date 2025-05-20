// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whallocator.h
// Creator      : Wei Hua (κ��)
// Comment      : ������
// CreationDate : 2004-01-17
// ChangeLOG    : 2005-12-16 ����whcmnallocationobj
//              : 2006-04-04 ��whcmnallocator::New����������_Ty*����������g++���治��ֱ��ʹ��ģ������ľ�̬���巽����˳����ף�����գ�hiahia��

#ifndef	__WHALLOCATOR_H__
#define	__WHALLOCATOR_H__

#include <new>
#include <stdlib.h>
#include <assert.h>

namespace	n_whcmn
{

// ������Զ��󷽷��ķ�ʽ�ṩ���亯��
// ����Ҫ��Ϊ�˽�������չ��chunk�ķ��䣩
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
// �����Ҫ��Ϊ����û������_AllocationObj�������Ҳ�ܽ�����ͨ�ķ���
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
// ����allocationobj��new��delete
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
	// New����Ĳ���ֻ��һ��������ʾ���͵Ŀ�ָ��
	// �÷�New((_Type*)NULL)
	template<typename _Ty>
	inline bool	New(_Ty *&ptr)
	{
		ptr	= new (Alloc(sizeof(_Ty))) _Ty;
		return	true;
	}
	template<typename _Ty>
	void		Delete(_Ty *ptr)
	{
		// ��������
		ptr->~_Ty();
		// �ͷ��ڴ�
		Free(ptr);
	}
	template<typename _Ty>
	_Ty *	NewArray(_Ty *, size_t nNum)
	{
		// ����һ��һ�����ù��죬��Ϊnew��������ĸ��ֽ�����¼����Ԫ�ظ���
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
		// һ�㶼Ӧ����nNewSize>nOldSize
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
		// ���ú���û�г�ʼ���Ĳ��ֵĹ��캯��
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
		// �������ж���������
		for(size_t i=0;i<nNum;i++)
		{
			pArr[i].~_Ty();
		}
		// �ͷ��ڴ�
		Free(pArr);
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHALLOCATOR_H__
