// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whvector.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的动态数组whvector和简单的堆栈whstack
//                为了简单我这里不使用new和delete，所以不会调用类的构造和析构
//                所以希望只能给没有构造和析构的类或结构使用
//                2004-03-04：严重注意insertafter/before的使用，在其前后
//                如果需要whvector为函数参数，严重推荐使用引用作或者指针形式!!!!
//                resize后size()就等于resize的长度，但是注意：reserve后capacity()>=这个值，并不是等于
// CreationDate : 2003-05-10
// ChangeLog    :
//                2004-01-17 添加了分配器，这样它将来就可能使用共享内存了
//                2005-02-05 增加了whvector2和whvectorN（即二维和多维数组）
//                2005-03-21 增加了whvector的拷贝构造函数。
//                2006-10-19 增加了f_seek中未知origin的assert，并增加了bReadOnly参数（在非ReadOnly条件下，Seek可以扩展vector）
//                2007-03-11 增加了SetString
//                2007-07-17 给pushn_back增加了返回指针

#ifndef	__WHVECTOR_H__
#define	__WHVECTOR_H__

#ifdef	_WIN32
#pragma warning(disable: 4786)
#endif
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "whallocator.h"
#include "whbits.h"
#include "whcmn_def.h"

#define	N_WHCMN		n_whcmn
namespace n_whcmn
{

// 交换两个变量
template<typename T>
inline void	swap(T & lhs,T & rhs)
{
	T	tmp;
	tmp	= lhs;
	lhs	= rhs;
	rhs	= tmp;
}
template<class MYAND_T, bool _bCPP=false>
struct	MyNewDel
{
	MYAND_T	*m_pAND;
	MyNewDel(MYAND_T *pAND)
		: m_pAND(pAND)
	{
	}
	inline void	SetAND(MYAND_T *pAND)
	{
		m_pAND	= pAND;
	}
	template<typename _Ty>
	inline _Ty *	MyWHRenew(_Ty *ptr, size_t nOldSize, size_t nNewSize)
	{
		return	(_Ty *)m_pAND->Realloc( ptr, nNewSize*sizeof(_Ty) );
	}
	template<typename _Ty>
	inline void		MyWHDelete(_Ty *ptr, size_t nSize)
	{
		m_pAND->Free( ptr );
	}
	template<typename _Ty>
	inline void		MyWHMemCpy(_Ty *dst, const _Ty *src, size_t nSize)
	{
		memcpy(dst, src, sizeof(_Ty)*nSize);
	}
};
template<class MYAND_T>
struct	MyNewDel<MYAND_T, true>
{
	MYAND_T	*m_pAND;
	MyNewDel(MYAND_T *pAND)
		: m_pAND(pAND)
	{
	}
	template<typename _Ty>
	inline _Ty *	MyWHRenew(_Ty *ptr, size_t nOldSize, size_t nNewSize)
	{
		return	m_pAND->RenewArray( ptr, nOldSize, nNewSize );
	}
	template<typename _Ty>
	inline void		MyWHDelete(_Ty *ptr, size_t nSize)
	{
		m_pAND->DeleteArray( ptr, nSize );
	}
	template<typename _Ty>
	inline void		MyWHMemCpy(_Ty *dst, const _Ty *src, size_t nSize)
	{
		for(size_t i=0;i<nSize;i++)
		{
			*dst	= *src;
			++	dst;
			++	src;
		}
	}
};

template<typename _Ty, bool _bCPP=false, class _AllocationObj=whcmnallocationobj, int _nReserve=0>
class	whvector
{
public:
	typedef	whfakeallocationobj<_AllocationObj>				MYFAO_T;
	typedef	whAND<whfakeallocationobj<_AllocationObj> >		MYAND_T;
protected:
	// 分配器
	MYFAO_T	m_FAO;
	MYAND_T	m_AND;
	MyNewDel<MYAND_T, _bCPP>	m_MND;
	_Ty		*m_Buf;
	size_t	m_nSize;
	size_t	m_nCapacity;
	int		m_nOffset;							// 读写文件的指针
	bool	m_bNoResize;						// 最开始reserve之后大小就不能改变了（这个主要用于在DLL之间交换数据时避免跨DLL的内存分配）
public:
	bool	m_bResizeAfterWrite;				// 是否在写指令f_write后一定要resize一下（为了简单，外界可以直接修改这个值）
public:
	// 设置底层分配器
	inline void		SetAO(_AllocationObj *pAO)
	{
		m_FAO.SetAO(pAO);
	}
	inline _AllocationObj *	GetAO()
	{
		return	m_FAO.GetAO();
	}
	inline MYAND_T &	GetAND()
	{
		return	m_AND;
	}
	// 清空vector或者文件内容
	void		clear()
	{
		// 这个应该不会有别的操作的
		m_nSize		= 0;
		m_nOffset	= 0;
	}
	void		destroy()
	{
		if( m_Buf )
		{
			m_MND.MyWHDelete(m_Buf, m_nCapacity);
		}
		born0();
	}
	void		push_back(const _Ty& _X)
	{
		*push_back()	= _X;
	}
	_Ty *	push_back()
	{
		resize(size()+1);
		// 这时候size已经变成+1的了
		return	getptr(size()-1);
	}
	void		push_back(const _Ty *_aX, size_t nNum)
	{
		int	nIdx	= size();
		resize(nIdx+nNum);
		for(size_t i=0;i<nNum;i++)
		{
			*(getptr(nIdx+i))	= _aX[i];
		}
	}
	void		push_back(whvector<_Ty, _bCPP, _AllocationObj, _nReserve> &other)
	{
		push_back(other.getbuf(), other.size());
	}
	void	pop_front()
	{
		if( size()>0 )	delrange(0,1);
	}
	void	popn_front(int _Num)
	{
		if( size()>0 )	delrange(0,_Num);
	}
	void	pop_back()
	{
		if( size()>0 )	resize(size()-1);
	}
	void	popn_back(int _Num)
	{
		if( ((size_t)_Num)>size() )
		{
			clear();
		}
		else
		{
			resize(size()-_Num);
		}
	}
	_Ty *	pushn_back(const _Ty * _aX, int _Num)
	{
		_Ty *ptr	= pushn_back(_Num);
		m_MND.MyWHMemCpy(ptr, _aX, _Num);
		return		ptr;
	}
	_Ty *	pushn_back(int _Num)
	{
		int		nNow = size();
		resize(size()+_Num);
		// 这时候size已经变成+1的了
		return	getptr(nNow);
	}

	// 注意：下面的几个insert需要上层保证nIdx的正确性，否则会有灾难性后果!!!!
	void	insertbefore(int nIdx, const _Ty& _X)
	{
		// 在某个索引后面插入元素
		*insertbefore(nIdx)	= _X;
	}
	_Ty *	insertbefore(int nIdx)
	{
		assert(size_t(nIdx)<size());
		int	nOldSize	= size();
		resize(nOldSize+1);
		// 从nIdx开始向后错一位
		if( nIdx < nOldSize )
		{
			memmove(getptr(nIdx+1), getptr(nIdx), sizeof(_Ty)*(nOldSize-nIdx));
		}
		return	getptr(nIdx);
	}
	// insertafter一定要慎用啊，这个家伙一旦使用就可能导致vector长度改变，刚刚使用的指针就可能作废了
	void	insertafter(int nIdx, const _Ty& _X)
	{
		// 在某个索引后面插入元素
		*insertafter(nIdx)	= _X;
	}
	_Ty *	insertafter(int nIdx)
	{
		assert(size_t(nIdx)<size());
		resize(size()+1);
		// 从nIdx+1开始向后错一位
		if( size_t(nIdx+1)<size()-1 )
		{
			// 2004-02-07: 原来是酱紫的：memmove(getptr(nIdx+2), getptr(nIdx+1), sizeof(_Ty)*(size()-nIdx-1));
			// 我忽略了移动不能包括最后一个，所以在size刚好等于reserve的时候就会出现内存越界，又是刘毅发现的，真是bug杀手啊，hiahia。
			memmove(getptr(nIdx+2), getptr(nIdx+1), sizeof(_Ty)*(size()-1-nIdx-1));
		}
		return	getptr(nIdx+1);
	}
	int		delrange(int nIdx, int nNum)		// 删除一段区域，并用后面的填充空白
	{
		if( size()<size_t(nIdx+nNum) )
		{
			nNum	= int(size()) - nIdx;
		}
		if( nNum<0 )
		{
			return	-1;
		}
		int	nSizeToMove = size()-nIdx-nNum;
		if( nSizeToMove>0 )
		{
			memmove(getptr(nIdx), getptr(nIdx+nNum), sizeof(_Ty)*nSizeToMove);
		}
		// 这一步已经保证size()-nNum不会越界(为负数)了
		resize( size()-nNum );
		return	0;
	}
	int		delvalue(const _Ty & val)			// 删除值为这个的第一个
	{
		for(size_t i=0;i<size();i++)
		{
			if(m_Buf[i] == val)
			{
				delrange(i,1);
				return	0;
			}
		}
		return	-1;
	}
	inline int	erasevalue(const _Ty & val)		// 我有时候喜欢这个，呵呵
	{
		return	delvalue(val);
	}
	int		findvalue(const _Ty & val)			// 找到第一个值，返回下标，没找到返回-1
	{
		for(size_t i=0;i<size();i++)
		{
			if(m_Buf[i] == val)
			{
				return	i;
			}
		}
		return	-1;
	}
	bool	has(const _Ty & val)				// 判断是否有这个值
	{
		for(size_t i=0;i<size();i++)
		{
			if(m_Buf[i] == val)
			{
				return	true;
			}
		}
		return	false;
	}
	inline size_t	totalbytes() const			// 总字节数
	{
		return	size() * sizeof(_Ty);
	}
	inline size_t	size() const				// 元素个数
	{
		return	m_nSize;
	}
	inline size_t	capacity() const
	{
		return	m_nCapacity;
	}
	inline void		SetNoResize(bool bSet)
	{
		m_bNoResize	= bSet;
	}
	void		reserve(size_t nCap)
	{
		assert(nCap<0x7FFFFFFF);				// 尺寸不能太大的(2005-02-16加)
		if( nCap>m_nCapacity )
		{
			m_Buf		= m_MND.MyWHRenew(m_Buf, m_nCapacity, nCap);
			m_nCapacity	= nCap;
		}
	}
	// reserve圆整到2的n次幂
	inline void		reserve2n(size_t nCap)
	{
		reserve(whbit_clp2(nCap));
	}
	void		resize(size_t nSize)			// resize不会给新增的元素赋值（不会调用他们的默认构造函数）
	{
		assert(nSize<0x7FFFFFFF);				// 尺寸不能太大的
		// 增加一个没有内容的元素
		// 如果空间不够了就扩展
		bool	bAlloc	= false;
		int		nOldCap	= m_nCapacity;
		if( m_nCapacity==0 )
		{
			m_nCapacity	= nSize;
			bAlloc		= true;
		}
		else if( nSize>m_nCapacity )
		{
			assert( !m_bNoResize );				// 说明上层希望避免这样情况
			m_nCapacity	*= 2;
			if( nSize>m_nCapacity )
			{
				// 如果一次增加很多就直接变成该值
				m_nCapacity	= nSize;
			}
			bAlloc		= true;
		}
		// 因为如果碰到nSize==0的情况就不用realloc了
		if( bAlloc && m_nCapacity )
		{
			m_Buf	= m_MND.MyWHRenew(m_Buf, nOldCap, m_nCapacity);
		}
		m_nSize	= nSize;
	}
	inline void	exactresize(size_t nSize)		// 重置到确切地尺寸，相当于reserve+resize
	{
		reserve(nSize);
		resize(nSize);
	}
	inline void	extend(int nExtraSize)			// 扩大这么多(可以为负)
	{
		int	nSize	= size() + nExtraSize;
		if( nSize<=0 )
		{
			clear();
		}
		else
		{
			resize( nSize );
		}
	}
	inline void		swap(whvector<_Ty, _bCPP, _AllocationObj, _nReserve> & _V)
	{
		N_WHCMN::swap(m_Buf, _V.m_Buf);
		N_WHCMN::swap(m_nCapacity, _V.m_nCapacity);
		N_WHCMN::swap(m_nSize, _V.m_nSize);
	}
	inline _Ty&		operator[](int _P) const
	{
		assert(_P>=0 && _P<(int)size());
		return	*(m_Buf+_P);
	}
	inline _Ty *	getptr(int _P) const
	{
		assert(_P>=0 && _P<(int)size());
		return	(m_Buf+_P);
	}
	inline _Ty &	get(int _P) const
	{
		return	*getptr(_P);
	}
	// 从一个参考点开始获取(_P从0开始)
	// _P返回真正的位置
	inline int		getroundidx(int _Ref, int _P) const
	{
		_P	= _Ref+_P;
		if( _P>=(int)size() )
		{
			_P	= _P-size();
		}
		return	_P;
	}
	// 这个无论如何都会得到合法的_P
	inline int		getsaferoundidx(int _Ref, int _P) const
	{
		_P	= _Ref+_P;
		if( _P>=(int)size() )
		{
			_P	%= size();
		}
		return	_P;
	}
	inline _Ty *	getroundptr(int _Ref, int _P) const
	{
		assert(_P>=0 && _P<(int)size());
		return	(m_Buf+getroundidx(_Ref, _P));
	}
	inline _Ty *	getsaferoundptr(int _Ref, int _P) const
	{
		return	(m_Buf+getsaferoundidx(_Ref, _P));
	}
	inline _Ty *	getbuf() const				// 这个只是为了使用书写方面
	{
		if(capacity()>0)	return	m_Buf;
		return	NULL;							// 没有分配任何内存，当然返回空指针
	}
	inline _Ty &	getfromtail(int nIdx) const
	{
		assert(nIdx>=0 && nIdx<(int)size());
		return	*(m_Buf+size()-1-nIdx);
	}
	inline _Ty *	getlastptr() const
	{
		assert(size()>0);
		return	m_Buf+size()-1;
	}
	inline _Ty &	getfirst() const
	{
		assert(size()>0);
		return	m_Buf[0];
	}
	inline _Ty &	getlast() const
	{
		assert(size()>0);
		return	m_Buf[size()-1];
	}
	// 查找一个值是否存在，返回索引
	int	findvalue(int nFromIdx, const _Ty &Val)
	{
		for(int i=nFromIdx; i<size(); i++)
		{
			if( get(i) == Val )
			{
				return	i;
			}
		}
		return	-1;
	}
	// 下面两个是为了和stl统一做的
	inline _Ty *	begin() const
	{
		return	m_Buf;
	}
	inline _Ty *	end() const
	{
		return	m_Buf+m_nSize;
	}
	// 赋值
	inline void operator = (const whvector<_Ty, _bCPP, _AllocationObj, _nReserve> &other)
	{
		resize(other.size());
		if( _bCPP )
		{
			for(size_t i=0;i<size();i++)
			{
				get(i)	= other.get(i);
			}
		}
		else
		{
			m_MND.MyWHMemCpy(getbuf(), other.getbuf(), other.size());
		}
	}
	// 增加类似于文件写的功能
	// 注意：!!!!这个功能只能在_Ty为1字节类型时使用!!!!
	// 指针重置
	inline int	f_rewind()
	{
		assert(sizeof(_Ty)==1);
		m_nOffset	= 0;
		return		0;
	}
	int	f_seek(int nOffset, int nOrigin, bool bReadOnly)
	{
		assert(sizeof(_Ty)==1);
		switch( nOrigin )
		{
		case	SEEK_SET:
			m_nOffset	= nOffset;
			break;
		case	SEEK_CUR:
			m_nOffset	+= nOffset;
			break;
		case	SEEK_END:
			m_nOffset	= size() + nOffset;
			break;
		default:
			assert(0);
			return	-1;
			break;
		}

		if( m_nOffset<0 )
		{
			m_nOffset		= 0;
		}
		else if( m_nOffset>(int)m_nSize )
		{
			if( bReadOnly )
			{
				m_nOffset	= m_nSize;
			}
			else
			{
				resize(m_nOffset);
			}
		}

		return	0;
	}
	// 写入指定长度(rewind之后成功写入相当于清空原来的内容)
	int	f_write(const void *pData, int nSize)
	{
		assert(sizeof(_Ty)==1);
		if( nSize>0 )
		{
			size_t	nNewOffset	= m_nOffset+nSize;
			if( m_bResizeAfterWrite || nNewOffset>size() )
			{
				resize(nNewOffset);
			}
			memcpy(getptr(m_nOffset), pData, nSize);
			m_nOffset	= nNewOffset;
		}
		return		nSize;
	}
	// 写入特定类型的数据
	template<typename _TyVal>
	inline int	f_write(const _TyVal *pVal)
	{
		return	f_write(pVal, sizeof(_TyVal));
	}
	// 在尾部打印(打印完后size()等于字串长度(不含最后的\0))
	// nLen		表示打印内容字串的最大长度(注意这个一定要足够大)
	inline void	f_aprintf(int nLen, const char *cszFmt, ...)
	{
		int	nBegin	= size();
		resize(nBegin + nLen);
		char	*pToPrn	= (char *)getptr(nBegin);
		va_list	arglist;
		va_start(arglist, cszFmt);
		vsprintf(pToPrn, cszFmt, arglist);
		va_end(arglist);
		resize(nBegin + strlen(pToPrn));
	}
	// 读出指定长度
	int	f_read(void *pData, int nSize)
	{
		assert(sizeof(_Ty)==1);
		int	nDiff = m_nSize - m_nOffset;
		if( nSize>nDiff )
		{
			nSize	= nDiff;
		}
		if( nSize>0 )
		{
			memcpy(pData, getptr(m_nOffset), nSize);
			m_nOffset	+= nSize;
		}
		return	nSize;
	}
	// 读出一行，返回读到的长度
	// 如果到了文件结尾则返回0
	int	f_readline(char *pszLine, int nSize)
	{
		// 一个字节一个字节地读入，直到读到结束"\n"或者文件结尾
		char	c, *pszBuf = pszLine;
		int		i = 0;
		while( nSize>i && m_nOffset<(int)m_nSize )
		{
			c	= get(m_nOffset++);
			pszBuf[i++]	= c;
			if( c=='\n' )
			{
				break;
			}
		}
		pszBuf[i]	= 0;
		return	i;
	}
	// 读出特定类型的数据
	template<typename _TyVal>
	inline int	f_read(_TyVal *pVal)
	{
		return	f_read(pVal, sizeof(_TyVal));
	}
	// 判断是否文件结束
	inline bool	f_iseof() const
	{
		assert(sizeof(_Ty)==1);
		return	m_nOffset==(int)m_nSize;
	}
	// 返回文件指针
	inline int	f_tell() const
	{
		assert(sizeof(_Ty)==1);
		return	m_nOffset;
	}
	// 设置字串给它（应该只针对whvector<char>）
	void	SetString(const char *cszStr)
	{
		resize(strlen(cszStr)+1);
		memcpy(getbuf(), cszStr, size());
	}
private:
	inline void	born0()							// 什么也没有的初始状态
	{
		m_Buf		= 0;
		m_nSize		= 0;
		m_nCapacity	= 0;
		m_nOffset	= 0;
		m_bResizeAfterWrite	= false;
	}
public:
	whvector(size_t n = 0, _AllocationObj *pAO = NULL)
	: m_FAO(pAO), m_AND(&m_FAO), m_MND(&m_AND)
	, m_bNoResize(false)
	{
		born0();
		resize(n);
		if( n==0 )
		{
			reserve(_nReserve);
		}
	}
	whvector(size_t n, const _Ty & val, _AllocationObj *pAO = NULL)
	: m_FAO(pAO), m_AND(&m_FAO), m_MND(&m_AND)
	, m_bNoResize(false)
	{
		born0();
		resize(n);
		for(size_t i=0;i<n;i++)
		{
			m_Buf[i] = val;
		}
	}
	whvector(const whvector<_Ty, _bCPP, _AllocationObj, _nReserve> &other)
	: m_FAO(other.m_FAO), m_AND(&m_FAO), m_MND(&m_AND)
	, m_bNoResize(false)
	{
		born0();
		operator = (other);
	}
	whvector(const NULLCONSTRUCT_T &nc)
	{
	}
	~whvector()
	{
		// 释放所有空间
		destroy();
	}
	void	AdjustInnerPtr(int nOffset)
	{
		wh_adjustaddrbyoffset(m_Buf, nOffset);
		m_FAO.AdjustInnerPtr(nOffset);
		m_AND.SetAO(&m_FAO);
		m_MND.SetAND(&m_AND);
	}
};

// stack一般使用的方法有：
// push, pop, pops, top, clear, size, []
template<typename _Ty, bool _bCPP=false, class _AllocationObj=whcmnallocationobj>
class	whstack	: public whvector<_Ty,_bCPP,_AllocationObj>
{
private:
	typedef	whvector<_Ty,_bCPP,_AllocationObj>	FATHERCLASS;
public:
	// 这个和whvector不同，这里只预留，但是不真正填入数据
	whstack(int nReserve=0)
	{
		FATHERCLASS::reserve(nReserve);
	}
	inline void	push(const _Ty& _X)
	{
		FATHERCLASS::push_back(_X);
	}
	inline const _Ty&	pop()
	{
		FATHERCLASS::pop_back();
		return	FATHERCLASS::m_Buf[FATHERCLASS::size()];
	}
	inline void	pops(size_t nNum)
	{
		if( nNum>FATHERCLASS::size() )
		{
			nNum	= FATHERCLASS::size();
		}
		FATHERCLASS::resize(FATHERCLASS::size()-nNum);
	}
	inline _Ty&	top() const
	{
		return	FATHERCLASS::getlast();
	}
	// 序号从栈顶开始是0
	inline _Ty&		operator[](int _P) const
	{
		return	FATHERCLASS::operator[](FATHERCLASS::size()-1-_P);
	}
};

// 用于和脚本结合时的
// 参数栈
typedef	whstack<void *>				whparamstack_t;
// 参数类型栈
typedef	whstack<unsigned int>		whtypestack_t;


// 2维矩阵
// 不能带类的构造和析构的
template<typename _Ty, class _AllocationObj=whcmnallocationobj>
class	whvector2
{
public:
	whvector<_Ty, false, _AllocationObj>	m_ivect;
	int		m_nX, m_nY;
public:
	whvector2()
	: m_nX(0), m_nY(0)
	{
	}
	whvector2(int nX, int nY)
	{
		resize(nX, nY);
	}
	inline void		resize(int nX, int nY)
	{
		m_nX		= nX;
		m_nY		= nY;
		m_ivect.resize(nX*nY);
	}
	inline _Ty *	getptr(int nX, int nY)
	{
		assert(nX>=0 && nX<m_nX);
		assert(nY>=0 && nY<m_nY);
		return	m_ivect.getptr(nX + nY*m_nX);
	}
	inline _Ty &	get(int nX, int nY)
	{
		return	*getptr(nX, nY);
	}
};
// n维矩阵
template<typename _Ty, class _AllocationObj=whcmnallocationobj>
class	whvectorN
{
public:
	whvector<_Ty, false, _AllocationObj>	m_ivect;
	whvector<int, false, _AllocationObj>	m_S;				// 每维的宽度
	whvector<int, false, _AllocationObj>	m_SFactor;			// 变成一维每个坐标乘的参数
public:
	whvectorN()
	{
	}
	whvectorN(const whvector<int, false, _AllocationObj> &S)
	{
		resize(S);
	}
	inline void		resize(const whvector<int, false, _AllocationObj> &S)
	{
		m_S			= S;
		int			nSize		= 1;
		int			nSFactor	= 1;
		m_SFactor.resize(S.size());
		for(size_t i=0;i<S.size();i++)
		{
			nSize			*= S[i];
			m_SFactor[i]	= nSFactor;
			nSFactor		*= S[i];
		}
		m_ivect.resize(nSize);
	}
	inline _Ty *	getptr(const whvector<int, false, _AllocationObj> &P)
	{
		int		nPos	= P[0];
		for(size_t i=1;i<P.size();i++)
		{
			nPos		+= P[i]*m_SFactor[i];
		}
		return	m_ivect.getptr(nPos);
	}
	inline _Ty &	get(const whvector<int, false, _AllocationObj> &P)
	{
		return	*getptr(P);
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHVECTOR_H__
