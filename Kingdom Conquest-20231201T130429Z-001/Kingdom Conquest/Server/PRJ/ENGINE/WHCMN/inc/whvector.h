// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whvector.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵Ķ�̬����whvector�ͼ򵥵Ķ�ջwhstack
//                Ϊ�˼������ﲻʹ��new��delete�����Բ��������Ĺ��������
//                ����ϣ��ֻ�ܸ�û�й�������������ṹʹ��
//                2004-03-04������ע��insertafter/before��ʹ�ã�����ǰ��
//                �����ҪwhvectorΪ���������������Ƽ�ʹ������������ָ����ʽ!!!!
//                resize��size()�͵���resize�ĳ��ȣ�����ע�⣺reserve��capacity()>=���ֵ�������ǵ���
// CreationDate : 2003-05-10
// ChangeLog    :
//                2004-01-17 ����˷������������������Ϳ���ʹ�ù����ڴ���
//                2005-02-05 ������whvector2��whvectorN������ά�Ͷ�ά���飩
//                2005-03-21 ������whvector�Ŀ������캯����
//                2006-10-19 ������f_seek��δ֪origin��assert����������bReadOnly�������ڷ�ReadOnly�����£�Seek������չvector��
//                2007-03-11 ������SetString
//                2007-07-17 ��pushn_back�����˷���ָ��

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

// ������������
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
	// ������
	MYFAO_T	m_FAO;
	MYAND_T	m_AND;
	MyNewDel<MYAND_T, _bCPP>	m_MND;
	_Ty		*m_Buf;
	size_t	m_nSize;
	size_t	m_nCapacity;
	int		m_nOffset;							// ��д�ļ���ָ��
	bool	m_bNoResize;						// �ʼreserve֮���С�Ͳ��ܸı��ˣ������Ҫ������DLL֮�佻������ʱ�����DLL���ڴ���䣩
public:
	bool	m_bResizeAfterWrite;				// �Ƿ���дָ��f_write��һ��Ҫresizeһ�£�Ϊ�˼򵥣�������ֱ���޸����ֵ��
public:
	// ���õײ������
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
	// ���vector�����ļ�����
	void		clear()
	{
		// ���Ӧ�ò����б�Ĳ�����
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
		// ��ʱ��size�Ѿ����+1����
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
		// ��ʱ��size�Ѿ����+1����
		return	getptr(nNow);
	}

	// ע�⣺����ļ���insert��Ҫ�ϲ㱣֤nIdx����ȷ�ԣ�������������Ժ��!!!!
	void	insertbefore(int nIdx, const _Ty& _X)
	{
		// ��ĳ�������������Ԫ��
		*insertbefore(nIdx)	= _X;
	}
	_Ty *	insertbefore(int nIdx)
	{
		assert(size_t(nIdx)<size());
		int	nOldSize	= size();
		resize(nOldSize+1);
		// ��nIdx��ʼ����һλ
		if( nIdx < nOldSize )
		{
			memmove(getptr(nIdx+1), getptr(nIdx), sizeof(_Ty)*(nOldSize-nIdx));
		}
		return	getptr(nIdx);
	}
	// insertafterһ��Ҫ���ð�������һ�һ��ʹ�þͿ��ܵ���vector���ȸı䣬�ո�ʹ�õ�ָ��Ϳ���������
	void	insertafter(int nIdx, const _Ty& _X)
	{
		// ��ĳ�������������Ԫ��
		*insertafter(nIdx)	= _X;
	}
	_Ty *	insertafter(int nIdx)
	{
		assert(size_t(nIdx)<size());
		resize(size()+1);
		// ��nIdx+1��ʼ����һλ
		if( size_t(nIdx+1)<size()-1 )
		{
			// 2004-02-07: ԭ���ǽ��ϵģ�memmove(getptr(nIdx+2), getptr(nIdx+1), sizeof(_Ty)*(size()-nIdx-1));
			// �Һ������ƶ����ܰ������һ����������size�պõ���reserve��ʱ��ͻ�����ڴ�Խ�磬�������㷢�ֵģ�����bugɱ�ְ���hiahia��
			memmove(getptr(nIdx+2), getptr(nIdx+1), sizeof(_Ty)*(size()-1-nIdx-1));
		}
		return	getptr(nIdx+1);
	}
	int		delrange(int nIdx, int nNum)		// ɾ��һ�����򣬲��ú�������հ�
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
		// ��һ���Ѿ���֤size()-nNum����Խ��(Ϊ����)��
		resize( size()-nNum );
		return	0;
	}
	int		delvalue(const _Ty & val)			// ɾ��ֵΪ����ĵ�һ��
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
	inline int	erasevalue(const _Ty & val)		// ����ʱ��ϲ��������Ǻ�
	{
		return	delvalue(val);
	}
	int		findvalue(const _Ty & val)			// �ҵ���һ��ֵ�������±꣬û�ҵ�����-1
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
	bool	has(const _Ty & val)				// �ж��Ƿ������ֵ
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
	inline size_t	totalbytes() const			// ���ֽ���
	{
		return	size() * sizeof(_Ty);
	}
	inline size_t	size() const				// Ԫ�ظ���
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
		assert(nCap<0x7FFFFFFF);				// �ߴ粻��̫���(2005-02-16��)
		if( nCap>m_nCapacity )
		{
			m_Buf		= m_MND.MyWHRenew(m_Buf, m_nCapacity, nCap);
			m_nCapacity	= nCap;
		}
	}
	// reserveԲ����2��n����
	inline void		reserve2n(size_t nCap)
	{
		reserve(whbit_clp2(nCap));
	}
	void		resize(size_t nSize)			// resize�����������Ԫ�ظ�ֵ������������ǵ�Ĭ�Ϲ��캯����
	{
		assert(nSize<0x7FFFFFFF);				// �ߴ粻��̫���
		// ����һ��û�����ݵ�Ԫ��
		// ����ռ䲻���˾���չ
		bool	bAlloc	= false;
		int		nOldCap	= m_nCapacity;
		if( m_nCapacity==0 )
		{
			m_nCapacity	= nSize;
			bAlloc		= true;
		}
		else if( nSize>m_nCapacity )
		{
			assert( !m_bNoResize );				// ˵���ϲ�ϣ�������������
			m_nCapacity	*= 2;
			if( nSize>m_nCapacity )
			{
				// ���һ�����Ӻܶ��ֱ�ӱ�ɸ�ֵ
				m_nCapacity	= nSize;
			}
			bAlloc		= true;
		}
		// ��Ϊ�������nSize==0������Ͳ���realloc��
		if( bAlloc && m_nCapacity )
		{
			m_Buf	= m_MND.MyWHRenew(m_Buf, nOldCap, m_nCapacity);
		}
		m_nSize	= nSize;
	}
	inline void	exactresize(size_t nSize)		// ���õ�ȷ�еسߴ磬�൱��reserve+resize
	{
		reserve(nSize);
		resize(nSize);
	}
	inline void	extend(int nExtraSize)			// ������ô��(����Ϊ��)
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
	// ��һ���ο��㿪ʼ��ȡ(_P��0��ʼ)
	// _P����������λ��
	inline int		getroundidx(int _Ref, int _P) const
	{
		_P	= _Ref+_P;
		if( _P>=(int)size() )
		{
			_P	= _P-size();
		}
		return	_P;
	}
	// ���������ζ���õ��Ϸ���_P
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
	inline _Ty *	getbuf() const				// ���ֻ��Ϊ��ʹ����д����
	{
		if(capacity()>0)	return	m_Buf;
		return	NULL;							// û�з����κ��ڴ棬��Ȼ���ؿ�ָ��
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
	// ����һ��ֵ�Ƿ���ڣ���������
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
	// ����������Ϊ�˺�stlͳһ����
	inline _Ty *	begin() const
	{
		return	m_Buf;
	}
	inline _Ty *	end() const
	{
		return	m_Buf+m_nSize;
	}
	// ��ֵ
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
	// �����������ļ�д�Ĺ���
	// ע�⣺!!!!�������ֻ����_TyΪ1�ֽ�����ʱʹ��!!!!
	// ָ������
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
	// д��ָ������(rewind֮��ɹ�д���൱�����ԭ��������)
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
	// д���ض����͵�����
	template<typename _TyVal>
	inline int	f_write(const _TyVal *pVal)
	{
		return	f_write(pVal, sizeof(_TyVal));
	}
	// ��β����ӡ(��ӡ���size()�����ִ�����(��������\0))
	// nLen		��ʾ��ӡ�����ִ�����󳤶�(ע�����һ��Ҫ�㹻��)
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
	// ����ָ������
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
	// ����һ�У����ض����ĳ���
	// ��������ļ���β�򷵻�0
	int	f_readline(char *pszLine, int nSize)
	{
		// һ���ֽ�һ���ֽڵض��룬ֱ����������"\n"�����ļ���β
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
	// �����ض����͵�����
	template<typename _TyVal>
	inline int	f_read(_TyVal *pVal)
	{
		return	f_read(pVal, sizeof(_TyVal));
	}
	// �ж��Ƿ��ļ�����
	inline bool	f_iseof() const
	{
		assert(sizeof(_Ty)==1);
		return	m_nOffset==(int)m_nSize;
	}
	// �����ļ�ָ��
	inline int	f_tell() const
	{
		assert(sizeof(_Ty)==1);
		return	m_nOffset;
	}
	// �����ִ�������Ӧ��ֻ���whvector<char>��
	void	SetString(const char *cszStr)
	{
		resize(strlen(cszStr)+1);
		memcpy(getbuf(), cszStr, size());
	}
private:
	inline void	born0()							// ʲôҲû�еĳ�ʼ״̬
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
		// �ͷ����пռ�
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

// stackһ��ʹ�õķ����У�
// push, pop, pops, top, clear, size, []
template<typename _Ty, bool _bCPP=false, class _AllocationObj=whcmnallocationobj>
class	whstack	: public whvector<_Ty,_bCPP,_AllocationObj>
{
private:
	typedef	whvector<_Ty,_bCPP,_AllocationObj>	FATHERCLASS;
public:
	// �����whvector��ͬ������ֻԤ�������ǲ�������������
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
	// ��Ŵ�ջ����ʼ��0
	inline _Ty&		operator[](int _P) const
	{
		return	FATHERCLASS::operator[](FATHERCLASS::size()-1-_P);
	}
};

// ���ںͽű����ʱ��
// ����ջ
typedef	whstack<void *>				whparamstack_t;
// ��������ջ
typedef	whstack<unsigned int>		whtypestack_t;


// 2ά����
// ���ܴ���Ĺ����������
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
// nά����
template<typename _Ty, class _AllocationObj=whcmnallocationobj>
class	whvectorN
{
public:
	whvector<_Ty, false, _AllocationObj>	m_ivect;
	whvector<int, false, _AllocationObj>	m_S;				// ÿά�Ŀ��
	whvector<int, false, _AllocationObj>	m_SFactor;			// ���һάÿ������˵Ĳ���
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
