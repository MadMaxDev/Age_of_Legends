// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whlist.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的链表
// CreationDate : 2003-05-10
// ChangeLog    :
//                2004-01-16 修改list中node的申请方式通过chunk进行
//                2004-01-17 添加了分配器，这样它将来就可能使用共享内存了
//                2004-10-01 修改了iterator的后置++和--，保持原来的语义。国庆快乐:)
//                2005-12-28 把whDList分出单写一个文件whDList.h

#ifndef	__WHLIST_H__
#define	__WHLIST_H__

#include <stdlib.h>
#include <string.h>
#include "whchunk.h"

namespace n_whcmn
{

template<typename _Ty>
struct	whlistnode
{
private:
	typedef	whlistnode	node;
public:
	node	*prev;
	node	*next;
	_Ty		data;
	inline void	insertafter(node *pnode)
	{
		prev		= pnode;
		next		= pnode->next;
		prev->next	= this;
		next->prev	= this;
	}
	inline void	insertbefore(node *pnode)
	{
		next		= pnode;
		prev		= pnode->prev;
		prev->next	= this;
		next->prev	= this;
	}
	inline void	leave()
	{
		// 这样写就可以保证无论调用多少次leave都是安全的
		if( next )
		{
			next->prev	= prev;
		}
		if( prev )
		{
			prev->next	= next;
		}
		prev		= NULL;
		next		= NULL;
	}
};

template<typename _Ty, class _Allocator=whcmnallocationobj>
class	whlist
{
protected:
	typedef whlistnode<_Ty>				node;
	whallocbychunk<node, _Allocator>	m_abc;
public:
	// 这个iterator可以保留做删除节点的参数
	class	iterator
	{
	public:
		node	*pnode;
	public:
		iterator()
		: pnode(NULL)
		{
		}
		iterator(node *_pnode)
		: pnode(_pnode)
		{
		}
		iterator(const iterator &_other)
		: pnode(_other.pnode)
		{
		}
	public:
		inline bool operator == (const iterator & it) const
		{
			return	it.pnode == pnode;
		}
		inline bool operator != (const iterator & it) const
		{
			return	it.pnode != pnode;
		}
		inline iterator	operator ++ (int)	// 不写这个会报no postfix form of 'operator ++' found for type 'iterator'
		{
			iterator	old(*this);
			pnode		= pnode->next;
			return		old;
		}
		inline iterator & operator ++ ()
		{
			pnode	= pnode->next;
			return	*this;
		}
		inline iterator & operator += (int n)
		{
			// <0无效
			assert(n>=0);
			for(int i=0;i<n;i++)
			{
				pnode	= pnode->next;
			}
			return	*this;
		}
		inline iterator operator -- (int)	// 不写这个会报no postfix form of 'operator --' found for type 'iterator'
		{
			iterator	old(*this);
			pnode		= pnode->prev;
			return		old;
		}
		inline iterator & operator -- ()
		{
			pnode	= pnode->prev;
			return	*this;
		}
		inline iterator & operator -= (int n)
		{
			for(int i=0;i<n;i++)
			{
				pnode	= pnode->prev;
			}
			return	*this;
		}
		inline const _Ty & operator * ()
		{
			return	pnode->data;
		}
	};
private:
	node	m_head, m_tail;				// 已用的单位列表
	node	m_availhead, m_availtail;	// 可用的单位列表
	size_t	m_size;
private:
	// reset header and tail
	inline void	resetht()
	{
		m_head.prev			= 0;
		m_head.next			= &m_tail;
		m_tail.prev			= &m_head;
		m_tail.next			= 0;
		m_availhead.prev	= 0;
		m_availhead.next	= &m_availtail;
		m_availtail.prev	= &m_availhead;
		m_availtail.next	= 0;

		m_size				= 0;
	}
	node *	getavailnode()
	{
		node	* pnode;
		pnode	= m_availhead.next;

		// 看看可用列表中有没有
		if( pnode == &m_availtail )
		{
			// 没有，申请一个
			pnode	= m_abc.alloc();
		}
		else
		{
			// 有，离开可以队列
			pnode->leave();
		}

		return	pnode;
	}
public:
	// 注意先后顺序
	whlist()
	: m_abc()
	, m_size(0)
	{
		resetht();
	}
	whlist(int nChunkSize)
	: m_abc(nChunkSize)
	, m_size(0)
	{
		resetht();
	}
	~whlist()
	{
		destroy();
	}
	inline void		setChunkSize(size_t nSize)
	{
		m_abc.setChunkSize(nSize);
	}
	inline size_t	size() const
	{
		return	m_size;
	}
	void	insertbefore(iterator _P, const _Ty& _X)
	{
		// 在_P之前插入_X
		*insertbefore(_P)	= _X;
	}
	_Ty *	insertbefore(iterator _P)
	{
		// insertbefore可以用在空列表中(before begin/end 都可以)
		node *	pnode;
		pnode	= getavailnode();

		// 这个node进入已用列表
		pnode->insertbefore( _P.pnode );
		m_size	++;

		return	&pnode->data;
	}
	void	insertafter(iterator _P, const _Ty& _X)
	{
		// 在_P之后插入_X，不过要注意，_P不能是end，否则错大了
		*insertafter(_P)	= _X;
	}
	_Ty *	insertafter(iterator _P)
	{
		// insertafter不能用在空列表中
		assert(_P != end());
		node *	pnode;
		pnode	= getavailnode();

		// 这个node进入已用列表
		pnode->insertafter( _P.pnode );
		m_size	++;

		return	&pnode->data;
	}
	void	push_front(const _Ty& _X)
	{
		*push_front()	= _X;
	}
	_Ty *	push_front()
	{
		node *	pnode;
		pnode	= getavailnode();

		// 这个node进入已用列表
		pnode->insertafter( &m_head );
		m_size	++;

		return	&pnode->data;
	}
	inline void	push_back(const _Ty& _X)
	{
		*push_back()	= _X;
	}
	inline _Ty *	push_back()
	{
		node *	pnode;
		pnode	= getavailnode();

		// 这个node进入已用列表
		pnode->insertbefore( &m_tail );
		m_size	++;

		return	&pnode->data;
	}
	inline void	pop_front()
	{
		if( m_size==0 )
		{
			return;
		}
		node *	pnode;
		pnode	= m_head.next;
		pnode->leave();
		pnode->insertafter(&m_availhead);
		m_size	--;
	}
	inline void	pop_back()
	{
		if( m_size==0 )
		{
			return;
		}
		node *	pnode;
		pnode	= m_tail.prev;
		pnode->leave();
		pnode->insertafter(&m_availhead);
		m_size	--;
	}
	inline iterator	begin()
	{
		iterator	it;
		it.pnode	= m_head.next;
		return		it;
	}
	inline iterator	end()
	{
		iterator	it;
		it.pnode	= &m_tail;
		return		it;
	}
	inline iterator	find(const _Ty & val)
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
	inline iterator	erase(iterator _P)
	{
		node	* pnode;
		pnode	= _P.pnode;
		_P		++;
		pnode->leave();
		pnode->insertafter(&m_availhead);
		m_size	--;
		return	_P;
	}
	// 删除值为这个的第一个
	// 有就返回true，否则返回false
	inline bool		erasevalue(const _Ty & val)
	{
		iterator	it = find(val);
		if( it==end() )
		{
			return	false;
		}
		erase(it);
		return		true;
	}
	inline void		clear()
	{
		// 删除所有节点，并插入在可用队列
		node	* pnode, * pnext;
		pnext	= m_head.next;
		while( pnext != &m_tail )
		{
			pnode	= pnext;
			pnext	= pnext->next;
			pnode->leave();
			pnode->insertafter(&m_availhead);
		}
		m_size		= 0;
	}
	inline void		destroy()
	{
		// 让m_abc去destroy即可
		m_abc.destroy();
		resetht();
	}
	inline void		addreserve(int size)
	{
		// 添加size个保留位置
		for(int i=0; i<size; i++)
		{
			node	* pnode;
			pnode	= m_abc.alloc();
			pnode->insertafter(&m_availhead);
		}
	}
};

// 所有序号必须是非负整数
class	whidxlist	: public whlist<int>
{
public:
	whidxlist()
	: whlist<int>()
	{
	}
	whidxlist(int nChunkSize)
	: whlist<int>(nChunkSize)
	{
	}
	// 将从0到nTotal-1的整数存入
	inline int		init(int nTotal)
	{
		int		i;
		for(i=0;i<nTotal;i++)
		{
			push_back(i);
		}
		return	0;
	}
	inline int		release()
	{
		destroy();
		return	0;
	}
	// 归还序号
	inline void	returnidx(int nIdx)
	{
		push_front(nIdx);
	}
	// 获得可用的序号(返回-1表示没有)
	inline int	borrowidx()
	{
		if( size()>0 )
		{
			int	nIdx = *(begin());
			pop_front();
			return	nIdx;
		}
		else
		{
			return	-1;
		}
	}
};

// 内部维护了一条available队列的列表（用于申请定长内存）
// 注意：下一次写应该考虑字节对齐的问题，现在都是1字节的，应该换成4字节对齐的好些!!!!
class	whalist
{
public:
	enum
	{
		INVALIDIDX	= -1,
	};
	struct	HDR_T
	{
		size_t	nDataUnitSize;			// 每个数据快的尺寸
		size_t	nMaxNum;				// 最多可用分配总数
		size_t	nCurNum;				// 当前已经分配的数量
		int		AvailableHdr;			// 可以分配的头 (Available是一个单向链表)
	};
	#pragma pack(1)
	struct	UNIT_T
	{
		// 下一个块的序号。(INVALIDIDX表示没有)
		bool	bAllocated;
		int		Next;					// 这个是在已经被分配后被上层使用时用于编制单向链表用的
										// 而在没有分配之前用于形成内部的available队列
		char	Data[1];				// 数据部分开头
	};
	#pragma pack()
protected:
	HDR_T	*m_pHdr;					// 信息头
	char 	*m_pUnit;					// 数据开始
	size_t	m_nUnitSize;				// 每个UNIT_T块的尺寸
public:
	// 返回如果需要nNum个元素总共需要的内存数量是多少bytes
	static size_t	CalcTotalSize(size_t nDataUnitSize, size_t nNum)
	{
		return	sizeof(HDR_T) + (sizeof(UNIT_T)-1+nDataUnitSize) * nNum;
	}
	inline size_t	CalcTotalSize() const
	{
		return	CalcTotalSize(m_pHdr->nDataUnitSize, m_pHdr->nMaxNum);
	}
	inline const HDR_T *	GetHdr() const
	{
		return	m_pHdr;
	}
public:
	whalist();
	// 初始化，pBuf的尺寸应该应该分配好了，不会小于CalcTotalSize得到的尺寸
	int		Init(void *pBuf, size_t nDataUnitSize, size_t nMaxNum);
	// 从已有的内存区域继承过来
	int		Inherit(void *pBuf);
	// 从旧内存继承过来，同时，旧内存是已经扩展大的内存(在原来数据的后面增加了内存)，希望容纳新数量为nNewMaxNum
	int		InheritEnlarge(void *pBuf, size_t nNewMaxNum, size_t nNewDataUnitSize);
	// 重置一下，恢复成什么也没有申请的状态
	int		Reset();
	// Alloc之后外界要记录下来idx，主动通过Free释放
	// 并且Alloc出来的单元默认GetNextOf会得到INVALIDIDX
	int		Alloc();
	// 释放一个块
	int		Free(int nIdx);
	// 释放一个链表(当然调用者要保证这个链表是正确有效的)
	int		FreeChain(int nHeadIdx);
	// 获得指定序号对应的数据开始指针
	void*	GetDataUnitPtr(int nIdx) const;
	// 判断一个块是否是已经分配的
	bool	IsUnitAllocated(int nIdx) const;
	// 获得已经申请的个数
	size_t	GetNum() const;
	// 获得还可以申请的个数
	size_t	GetAvailNum() const;
	// (注意，下面两个操作一定是对申请好的单元使用的，外界可以通过这两个用申请的块自己构成链表)
	// 获得一个的下一个
	int		GetNextOf(int nIdx) const;
	// 设置nIdx的下一个为nNext
	bool	SetNextOf(int nIdx, int nNext);
protected:
	UNIT_T*	GetUnitPtr(int nIdx) const;
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHLIST_H__
