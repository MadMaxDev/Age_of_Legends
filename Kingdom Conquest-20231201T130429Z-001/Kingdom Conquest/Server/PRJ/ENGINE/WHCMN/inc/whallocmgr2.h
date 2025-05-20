// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whallocmgr2.h
// Creator      : Wei Hua (魏华)
// Comment      : 内存分配管理器，参照以前的whallocmgr的接口实现。使用了新的基于hash的尺寸对应器。
//              : 主要针对基于预先分配的大块内存（如共享内存）的小分配。
// << whallocmgr2使用说明 >>
// 1. 定义一个_AllocationObj对象，这样的对象是实现了Alloc和Free方法的对象。这个对象被用来分配大块的内存。
//    如：whallocator.h中的whcmnallocationobj、whcmnallocationobjwithonemem。
//    对于服务器: 需要通过whcmnallocationobjwithonemem一次使用一大块内存来使用，nChunkSize即是该块大内存的总体尺寸（一般可能会有100多M）。
//    对于客户端: 如果也需要这样的接口，则可以使用whcmnallocationobj，然后把nChunkSize设置的比较小。不过为了简单客户端可以直接用系统的new和delete就可以了。
// 2. 初始化whallocmgr2对象，调用Init方法。
// 3. 正常使用：调用New和Delete方法进行对象的分配和释放；
//    调用NewList可以分配一个链表，可以对它进行链表成员的添加、删除、查找、浏览操作，链表内部内存的管理由链表负责，删除链表使用Delete方法即可；
//    调用Alloc和Free进行缓冲区的分配和释放。注意如果调用Free释放对象那么对象的析构函数就不会被调到了。
//
// << whallocmgr2::list使用说明 >>
// 1. 使用whallocmgr2::NewList方法创建list对象。删除list对象请使用whallocmgr2::Delete方法。
// 2. 普通操作：
//    可以使用list的push_back/push_front/insertafter/insertbefore方法将数据插入列表。
//    可以使用find方法查找指定值的第一个原素（不过查找是遍历方式，效率很低，不推荐经常使用）。
//    可以使用erase方法删除指定位置的元素。
// 3. 遍历操作
//    可以使用begin/end/++iterator进行正向遍历
//    可以使用rbegin/rend/--iterator进行反向遍历
// 关于上面的使用可以参考例子程序：allocmgf2-tst.cpp
// CreationDate : 2006-01-10
// ChangeLog    :

#ifndef	__WHALLOCMGR2_H__
#define	__WHALLOCMGR2_H__

#include "whallocator2.h"
#include "whDList.h"

namespace n_whcmn
{

template<class _AllocationObj>
class	whallocmgr2	: public whAND<whfakeallocationobj<_AllocationObj> >
{
public:
	typedef	whAND<whfakeallocationobj<_AllocationObj> >		FATHERCLASS;
	typedef	whfakeallocationobj<_AllocationObj>				MYFAO_T;
protected:
	// 分配器
	MYFAO_T	m_FAO;
public:
	int		m_nAdjustCount;									// 如果为0则证明上次的adjust是完全结束了的（上次每个个模块adjust它的时候都需要增加这个，adjust完了减少这个）
	int		m_nUseCount;									// 如果为0则证明所有的使用者都已经使用完了
public:
	whallocmgr2(_AllocationObj *pAO = NULL)
		: FATHERCLASS(&m_FAO)
		, m_FAO(pAO)
		, m_nAdjustCount(0)
		, m_nUseCount(0)
	{
	}
public:
	inline void	SetAO(_AllocationObj *pAO)
	{
		m_FAO.SetAO(pAO);
		FATHERCLASS::SetAO(&m_FAO);							// 因为内存改变时m_FAO的地址实际上也改变了
	}
	inline _AllocationObj *	GetAO()
	{
		return	m_FAO.GetAO();
	}
	inline MYFAO_T &	GetFAO()
	{
		return	m_FAO;
	}
	// 创建对象并设置其管理器为自己
	template<typename _Ty>
	bool	NewObjAndSetMgr(_Ty *&pObj)
	{
		if( !New(pObj) )
		{
			return	false;
		}
		pObj->SetMgr(this);
		return	true;
	}
	// 指针以及内部调整（只在继承旧的共享内存的时候用，注意：不要对继承的内存中的各种对象做任何删除或创建操作，把数据搞出来就完事了）
	// nOffset可正可负
	template<typename _Ty>
	void	AdjustPtrAndInner(_Ty &ptr, int nOffset)
	{
		// 先纠正指针本身
		wh_adjustaddrbyoffset(ptr, nOffset);
		// 然后把管理器自己传给对象，让对象进行内部纠正
		ptr->AdjustInnerPtr(this, nOffset);
	}
public:
	// 其他的对象模板
	// list
	template<typename _Ty>
	class	list
	{
	public:
		typedef	whDList<_Ty>			DL_T;
		typedef	typename DL_T::node		node;	// 注意，这个如果不写typename的话在VC7和gcc下后果会很严重:P
	protected:
		friend	class whallocmgr2;
		whallocmgr2						*m_pMgr;
		DL_T							m_dl;
	public:
		// iterator定义
		class	const_iterator;
		class	iterator
		{
		public:
			node	*pNode;				// 主要是因为要公开的地方太多了。而且还有地方需要直接访问这个变量的地址。
		public:
			iterator()
				: pNode(NULL)
			{
			}
			iterator(node *_pnode)
				: pNode(_pnode)
			{
			}
			iterator(const iterator &_other)
				: pNode(_other.pNode)
			{
			}
		public:
			inline bool operator == (const iterator & it) const
			{
				return	it.pNode == pNode;
			}
			inline bool operator != (const iterator & it) const
			{
				return	it.pNode != pNode;
			}
			inline bool operator == (const const_iterator & it) const
			{
				return	it.pNode == pNode;
			}
			inline bool operator != (const const_iterator & it) const
			{
				return	it.pNode != pNode;
			}
			// ++操作用于正向迭代子操作
			inline iterator	operator ++ (int)	// 不写这个会报no postfix form of 'operator ++' found for type 'iterator'
			{
				iterator	old(*this);
				pNode		= pNode->next;
				return		old;
			}
			inline iterator & operator ++ ()
			{
				pNode	= pNode->next;
				return	*this;
			}
			// --操作用于反向迭代子操作
			inline iterator	operator -- (int)	// 不写这个会报no postfix form of 'operator --' found for type 'iterator'
			{
				iterator	old(*this);
				pNode		= pNode->prev;
				return		old;
			}
			inline iterator & operator -- ()
			{
				pNode	= pNode->prev;
				return	*this;
			}
			inline _Ty & operator * ()
			{
				return	pNode->data;
			}
		};
		// const_iterator定义
		class	const_iterator
		{
		public:
			node	*pNode;				// 主要是因为要公开的地方太多了。而且还有地方需要直接访问这个变量的地址。
		public:
			const_iterator()
				: pNode(NULL)
			{
			}
			const_iterator(node *_pnode)
				: pNode(_pnode)
			{
			}
			const_iterator(const const_iterator &_other)
				: pNode(_other.pNode)
			{
			}
		public:
			inline bool operator == (const const_iterator & it) const
			{
				return	it.pNode == pNode;
			}
			inline bool operator != (const const_iterator & it) const
			{
				return	it.pNode != pNode;
			}
			inline bool operator == (const iterator & it) const
			{
				return	it.pNode == pNode;
			}
			inline bool operator != (const iterator & it) const
			{
				return	it.pNode != pNode;
			}
			// ++操作用于正向迭代子操作
			inline const_iterator	operator ++ (int)	// 不写这个会报no postfix form of 'operator ++' found for type 'const_iterator'
			{
				const_iterator	old(*this);
				pNode		= pNode->next;
				return		old;
			}
			inline const_iterator & operator ++ ()
			{
				pNode	= pNode->next;
				return	*this;
			}
			// --操作用于反向迭代子操作
			inline const_iterator	operator -- (int)	// 不写这个会报no postfix form of 'operator --' found for type 'const_iterator'
			{
				const_iterator	old(*this);
				pNode		= pNode->prev;
				return		old;
			}
			inline const_iterator & operator -- ()
			{
				pNode	= pNode->prev;
				return	*this;
			}
			inline const _Ty & operator * ()
			{
				return	pNode->data;
			}
		};
	public:
		list(whallocmgr2 *pMgr=NULL)
			: m_pMgr(pMgr)
		{
		}
		~list()
		{
			// 删除所有单元
			clear();
		}
		inline void	SetMgr(whallocmgr2 *pMgr)
		{
			m_pMgr	= pMgr;
		}
		inline whallocmgr2 *	GetMgr()
		{
			return	m_pMgr;
		}
		inline size_t	size() const
		{
			return	m_dl.size();
		}
		inline iterator	begin()
		{
			return	iterator(m_dl.begin());
		}
		inline const_iterator begin() const
		{
			return	const_iterator(m_dl.begin());
		}
		inline iterator	end()
		{
			return	iterator(m_dl.end());
		}
		inline const_iterator	end() const
		{
			return	const_iterator(m_dl.end());
		}
		// 注意：如果用rxxx函数，则循环需要用--操作而不能用++操作
		inline iterator	rbegin()
		{
			return	iterator(m_dl.rbegin());
		}
		inline const_iterator	rbegin() const
		{
			return	const_iterator(m_dl.rbegin());
		}
		inline iterator	rend()
		{
			return	iterator(m_dl.rend());
		}
		inline const_iterator	rend() const
		{
			return	const_iterator(m_dl.rend());
		}
		// 插入一个单元
		_Ty &	push_back(iterator *pIt=NULL)
		{
			node	*pNode;
			m_pMgr->New(pNode);
			assert(pNode);
			m_dl.AddToTail(pNode);
			if( pIt )
			{
				pIt->pNode	= pNode;
			}
			return	pNode->data;
		}
		void	push_back(const _Ty& _X, iterator *pIt=NULL)
		{
			push_back(pIt)	= _X;
		}
		_Ty &	push_front(iterator *pIt=NULL)
		{
			node	*pNode;
			m_pMgr->New(pNode);
			assert(pNode);
			m_dl.AddToHead(pNode);
			if( pIt )
			{
				pIt->pNode	= pNode;
			}
			return	pNode->data;
		}
		void	push_front(const _Ty& _X, iterator *pIt=NULL)
		{
			push_front(pIt)	= _X;
		}
		void	pop_front()	// 这个要稍微省一点计算
		{
			iterator	it=begin();
			if( it!=end() )
			{
				erase(it);
			}
		}
		void	pop_back()
		{
			iterator	it=end();
			if( it!=begin() )
			{
				erase(--it);
			}
		}
		_Ty &	insertbefore(iterator _P, iterator *pIt=NULL)
		{
			node	*pNode;
			m_pMgr->New(pNode);
			assert(pNode);
			// 这个node进入已用列表
			pNode->insertbefore( _P.pNode );
			if( pIt )
			{
				pIt->pNode	= pNode;
			}
			return	pNode->data;
		}
		void	insertbefore(iterator _P, const _Ty& _X, iterator *pIt=NULL)
		{
			// 在_P之前插入_X
			insertbefore(_P, pIt)	= _X;
		}
		_Ty &	insertafter(iterator _P, iterator *pIt=NULL)
		{
			node	*pNode;
			m_pMgr->New(pNode);
			assert(pNode);
			// 这个node进入已用列表
			pNode->insertafter( _P.pNode );
			if( pIt )
			{
				pIt->pNode	= pNode;
			}
			return	pNode->data;
		}
		void	insertafter(iterator _P, const _Ty& _X, iterator *pIt=NULL)
		{
			// 在_P之后插入_X
			insertafter(_P, pIt)	= _X;
		}
		iterator	find(const _Ty & val)
		{
			iterator	it;
			for(it=begin();it!=end();++it)
			{
				if( (*it)==val )
				{
					break;
				}
			}
			return	it;
		}
		// 返回下一个单元的iterator（即位置）
		iterator	erase(const iterator &_P)
		{
			if( _P == end() )
			{
				return	end();
			}
			assert(size()>=1);
			node	*pNode	= _P.pNode;
			iterator	Next(pNode->next);
			// 从m_dl中离开
			pNode->leave();
			// 释放指针
			m_pMgr->Delete(pNode);
			// 返回下一个
			return	Next;
		}
		// 删除值为这个的第一个
		// 有就返回true，否则返回false
		bool		erasevalue(const _Ty & val)
		{
			iterator	it = find(val);
			if( it==end() )
			{
				return	end();
			}
			erase(it);
			return		true;
		}
		// 只把元素从列表中移除，但是不删除内存（这个主要是在从共享内存中恢复数据之后删除已经成功保存的数据时用的）
		void	fakeerase(node *pNode)
		{
			pNode->leave();
		}
		// 删除所有单元
		void	clear()
		{
			iterator	it = begin();
			while( it!=end() )
			{
				iterator	itthis	= it;
				++	it;
				erase(itthis);
			}
		}
		void	AdjustInnerPtr(whallocmgr2 *pAM, int nOffset)
		{
			// 设置新的内存管理器
			SetMgr(pAM);
			// 校正m_dl内部
			m_dl.AdjustInnerPtr(nOffset);
		}
	};
	template<typename _Ty>
	class	vector
	{
	protected:
		friend	class whallocmgr2;
		whallocmgr2		*m_pMgr;
		_Ty				*m_Buf;
		size_t			m_nSize;
		size_t			m_nCapacity;
	public:
		vector(whallocmgr2 *pMgr=NULL)
			: m_pMgr(pMgr)
			, m_Buf(NULL)
			, m_nSize(0)
			, m_nCapacity(0)
		{
		}
		vector(const NULLCONSTRUCT_T &nc)
		{
		}
		~vector()
		{
			// 删除所有单元
			destroy();
		}
		inline void	SetMgr(whallocmgr2 *pMgr)
		{
			m_pMgr	= pMgr;
		}
		inline whallocmgr2 *	GetMgr()
		{
			return	m_pMgr;
		}
		inline void	destroy()
		{
			if( m_Buf )
			{
				m_pMgr->Free(m_Buf);
				m_Buf	= NULL;
			}
		}
		inline void	clear()
		{
			resize(0);
		}
		inline size_t	size() const
		{
			return	m_nSize;
		}
		inline size_t	capacity() const
		{
			return	m_nCapacity;
		}
		void	resize(size_t nSize)
		{
			assert(nSize<0x7FFFFFFF);				// 尺寸不能太大的
			// 增加一个没有内容的元素
			// 如果空间不够了就扩展
			bool	bAlloc	= false;
			if( m_nCapacity==0 )
			{
				m_nCapacity	= nSize;
				bAlloc		= true;
			}
			else if( nSize>m_nCapacity )
			{
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
				m_Buf	= (_Ty *)m_pMgr->Realloc(m_Buf, m_nCapacity*sizeof(_Ty));
			}
			m_nSize	= nSize;
		}
		void	reserve(size_t nCap)
		{
			assert(nCap<0x7FFFFFFF);				// 尺寸不能太大的
			m_Buf		= (_Ty *)m_pMgr->Realloc(m_Buf, nCap*sizeof(_Ty));
			m_nCapacity	= nCap;
		}
		inline bool		IsPosValid(int _P) const
		{
			return	m_Buf && _P>=0 && _P<(int)size();
		}
		inline _Ty&		operator[](int _P) const
		{
			assert(IsPosValid(_P));
			return	*(m_Buf+_P);
		}
		inline _Ty *	getptr(int _P) const
		{
			assert(IsPosValid(_P));
			return	(m_Buf+_P);
		}
		inline _Ty *	getbuf()
		{
			return	m_Buf;
		}
		inline _Ty &	get(int _P) const
		{
			assert(IsPosValid(_P));
			return	*getptr(_P);
		}
		void	AdjustInnerPtr(whallocmgr2 *pAM, int nOffset)
		{
			SetMgr(pAM);
			wh_adjustaddrbyoffset(m_Buf, nOffset);
		}
	};
};

}		// EOF namespace n_whcmn

#endif
