// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : whunitallocator3.h
// Creator     : Wei Hua (魏华)
// Comment     : 新的定上限的动态元素分配器，搭配适当的分配器。
//             : 刚开始只分配一个指针索引表，具体元素的分配通过分配器来做，分配原则由相应的分配器来定。这样就可以使用共享内存了。
//             : 强烈建议：如果对象需要放在共享内存中，就最好不要有自己内部又使用系统函数分配内存，最好不要有虚函数!!!!
// ChangeLOG   : 2007-09-29 从whunitallocator2修改而来。取消原来的chunk分配器，因为后台提供的分配起可能更好用
//             : 2007-11-06 修正了whunitallocatorFixed3的iterator直接从父类继承，拷贝构造函数参数也使用父类，导致不同类型(_Ty)的whunitallocatorFixed3::iterator可以互相复制，导致如果由于手误写错了对象，编译能通过，但是运行期对象内部数据混乱出错。

#ifndef	__WHUNITALLOCATOR3_H__
#define	__WHUNITALLOCATOR3_H__

#include "./whcmn_def.h"
#include "./whDList.h"
#include "./whallocator2.h"

namespace	n_whcmn
{

// 针对固定长度buffer的带ID的分配器
template <class _AllocationObj>
class	whbufferallocatorFixed3
{
public:
	enum
	{
		MAXNUM			= 0x10000,
		MAXRAND			= 0x7FFF,
	};
protected:
	struct	UNITHDR_T
	{
		int			nID;						// 单元的ID
		int			nOldID;						// 上次的ID
		void	clear()
		{
			nID		= -1;
			nOldID	= -1;
		}
	};
	typedef	whDList<UNITHDR_T>		DL_T;
	typedef	typename DL_T::node		node;		// 这个node包含了UNIT的数据部分
	int				m_nMax;						// 最多的元素数量
	int				m_nIdxToAlloc;				// 可用的最大的索引，如果不小于m_nMax了就不行了。
	int				m_nRand;					// 随机数，用作ID的前半部分。
	node			**m_paIdx;					// 索引表
												// 如果实际分配了单元，则对应的指针会指向相应的位置。
												// 如果单元被Free了，索引指针仍然指向该位置，单元node内部会把ID变成IDX以标记为被释放
	int				m_DATASIZE;					// 数据部分尺寸
	DL_T			m_dlAvail;					// 被释放的元素列表
	DL_T			m_dlAlloc;					// 已经申请出来的元素列表
	DL_T			m_dlReserved;				// 保留先不分配的对象列表
	_AllocationObj	*m_pAllocationObj;
public:
	whbufferallocatorFixed3()
	: m_nMax(0)
	, m_nIdxToAlloc(0), m_nRand(0)
	, m_paIdx(NULL)
	, m_DATASIZE(0)
	{
	}
	~whbufferallocatorFixed3()
	{
		Release();
	}
	inline int	MakeRand()
	{
		if( (++m_nRand) >= MAXRAND )
		{
			m_nRand	= 1;
		}
		return	m_nRand;
	}
	inline int	MakeID(int nIdx, int nRand) const
	{
		return	(nRand<<16) | nIdx;
	}
	inline int	GetIdxByID(int nID) const
	{
		return	nID & 0xFFFF;
	}
	// _SIZE为数据部分的长度
	int	Init(_AllocationObj *pAllocationObj, int nMax, int _SIZE)
	{
		if( nMax>=MAXNUM )
		{
			// 不能分配超过65535个单元
			assert(0);
			return	-1;
		}
		if( m_paIdx!=NULL )
		{
			assert(0);
			return	-2;
		}
		m_pAllocationObj	= pAllocationObj;
		m_nMax			= nMax;
		m_nIdxToAlloc	= 0;
		m_nRand			= 0;
		m_paIdx			= (node **)m_pAllocationObj->Alloc(sizeof(node *)*nMax);
		memset(m_paIdx, 0, sizeof(node *)*nMax);
		m_DATASIZE		= _SIZE + sizeof(node);
		return	0;
	}
	void	_ReleaseNodeInDL(DL_T &l)
	{
		node	*pNode	= l.begin();
		while( pNode!=l.end() )
		{
			node	*pCurNode	= pNode;
			pNode	= pNode->next;
			pCurNode->leave();
			m_pAllocationObj->Free(pCurNode);
		}
	}
	int	Release()
	{
		// 释放索引数组
		if( m_paIdx == NULL )	// 这个指针可以用来标记是否已经释放过了
		{
			return	0;
		}
		// 释放各个列表中的，所有的node
		_ReleaseNodeInDL(m_dlAlloc);
		_ReleaseNodeInDL(m_dlAvail);
		_ReleaseNodeInDL(m_dlReserved);
		// 释放总表
		m_pAllocationObj->Free(m_paIdx);
		m_paIdx	= NULL;
		return	0;
	}
	void *	Alloc(int *pnID)
	{
		*pnID	= -1;
		node	*pNode	= NULL;
		// 看看可用列表中有没有
		if( m_dlAvail.size()>0 )
		{
			// 有，移出来
			pNode	= m_dlAvail.begin();
			pNode->leave();
		}
		else
		{
			if( m_nIdxToAlloc>=m_nMax )
			{
				// 没有可分配的了
				return	NULL;
			}
			// 没有申请新的
			pNode	= (node *)m_pAllocationObj->Alloc(m_DATASIZE);
			pNode->reset();
			pNode->data.clear();
			pNode->data.nID	= m_nIdxToAlloc;
			// 和Idx相关联
			m_paIdx[m_nIdxToAlloc]	= pNode;
			m_nIdxToAlloc++;
		}
		assert( pNode!=NULL );
		pNode->data.nID	= MakeID(pNode->data.nID, MakeRand());
		pNode->data.nOldID	= pNode->data.nID;
		*pnID			= pNode->data.nID;
		m_dlAlloc.AddToHead(pNode);
		// 返回数据部分
		return	pNode+1;
	}
	// 根据ID进行分配（这个和Alloc方法应该不能在同一个对象身上使用）
	void *	AllocByID(int nID)
	{
		if( nID<=0xFFFF )
		{
			assert(0);
			return	NULL;
		}
		int		nIdx	= GetIdxByID(nID);
		node	*pNode	= m_paIdx[nIdx];
		if( pNode )
		{
			// 原来就有，说明在avail队列中
			if(pNode->data.nID >= MAXNUM )
			{
				// 说明原来已经被分配了
				// 这种情况可能会出现，就不用assert(0)了
				return	NULL;
			}
			// 从avail列表中退出
			pNode->leave();
		}
		else
		{
			// 原来没有，申请新的
			pNode	= (node *)m_pAllocationObj->Alloc(m_DATASIZE);
			pNode->reset();
			pNode->data.clear();
			// 和Idx相关联
			m_paIdx[nIdx]	= pNode;
			m_nIdxToAlloc++;
		}
		// 加入alloc列表
		m_dlAlloc.AddToHead(pNode);
		// 设置ID为指定值
		pNode->data.nID		= nID;
		pNode->data.nOldID	= nID;
		return	pNode+1;
	}
	node *	GetNodeByID(int nID)
	{
		if( nID<=0xFFFF )
		{
			return	NULL;
		}
		node	*pNode	= m_paIdx[GetIdxByID(nID)];
		if( pNode )
		{
			if( pNode->data.nID == nID )
			{
				return	pNode;
			}
		}
		return	NULL;
	}
	node *	GetNodeByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// 序号超界
			return	NULL;
		}
		return	m_paIdx[nIdx];
	}
	void *	GetByID(int nID)
	{
		node	*pNode	= GetNodeByID(nID);
		if( pNode )
		{
			return	pNode+1;
		}
		return	NULL;
	}
	inline node *	GetNodeByPtr(void *ptr)
	{
		return	(node *)( ((char *)ptr) - sizeof(node) );
	}
	inline int	GetIDByPtr(void *ptr)
	{
		return	GetNodeByPtr(ptr)->data.nID;
	}
	void	FreeNode(node *pNode)	// 这个外面最好不要调用
	{
		assert(pNode->data.nID>0xFFFF);
		// 清空ID为IDX，然后放入avail列表
		pNode->data.nID	= GetIdxByID(pNode->data.nID);
		pNode->leave();
		m_dlAvail.AddToHead(pNode);
	}
	int		Free(int nID)
	{
		if( nID<=0xFFFF )
		{
			return	-1;
		}
		node	*pNode	= GetNodeByID(nID);
		if( pNode )
		{
			// 释放
			FreeNode(pNode);
			return	0;
		}
		// 无法释放
		return	-2;
	}
	inline void	FreeByPtr(void *ptr)
	{
		// 获得node的指针
		FreeNode(GetNodeByPtr(ptr));
	}
	void	ReserveNode(node *pNode)
	{
		pNode->leave();
		m_dlReserved.AddToHead(pNode);
	}
	void	UnReserveNode(node *pNode)
	{
		pNode->leave();
		m_dlAvail.AddToHead(pNode);
	}
	inline void	ReserveByPtr(void *ptr)
	{
		ReserveNode(GetNodeByPtr(ptr));
	}
	inline void	UnReserveByPtr(void *ptr)
	{
		UnReserveNode(GetNodeByPtr(ptr));
	}
	// 根据序号保留一个Free的单元
	//int		ReserveByIdx(int nIdx)
	//{
	//	node	*pNode	= GetNodeByIdx(nIdx);
	//	if( !pNode )
	//	{
	//		return	-1;
	//	}
	//	ReserveNode(pNode);
	//	return	0;
	//}
	// 根据ID取消保留一个单元
	int		UnReserveByID(int nID)
	{
		node	*pNode	= GetNodeByIdx(GetIdxByID(nID));
		if( !pNode )
		{
			return	-1;
		}
		if( pNode->data.nOldID != nID )
		{
			// 说明不用unreserved了
			return	0;
		}
		UnReserveNode(pNode);
		return	0;
	}

	inline int	size() const			// 获得已经分配的个数
	{
		return	m_dlAlloc.size();
	}
	inline int	getsizeleft() const		// 获得还可以分配的个数（不包括reserve的）
	{
		return	(m_nMax-m_nIdxToAlloc) + m_dlAvail.size();
	}
	// 调整内部指针，这个在继承共享内存发生指针偏移时需要
	void	AdjustInnerPtr(_AllocationObj *pAllocationObj, int nOffset)
	{
		// 设置这个很重要
		m_pAllocationObj	= pAllocationObj;
		// 索引表
		if( m_paIdx )
		{
			wh_adjustaddrbyoffset(m_paIdx, nOffset);
			for(int i=0;i<m_nMax;i++)
			{
				wh_adjustaddrbyoffset(m_paIdx[i], nOffset);	// 如果为NULL的指针是不会被调整的
			}
		}
		// 各个链表内部
		m_dlAvail.AdjustInnerPtr(nOffset);
		m_dlAlloc.AdjustInnerPtr(nOffset);
		m_dlReserved.AdjustInnerPtr(nOffset);
	}
// iterator
public:
	class	iterator
	{
	protected:
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
		inline void * operator * ()
		{
			return	pnode+1;
		}
		inline int	getid() const
		{
			return	pnode->data.nID;
		}
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
	};
	inline iterator	begin()
	{
		iterator	it(m_dlAlloc.begin());
		return		it;
	}
	inline iterator	end()
	{
		iterator	it(m_dlAlloc.end());
		return		it;
	}
};

template <typename _Ty, class _AllocationObj>
class	whunitallocatorFixed3	: public whbufferallocatorFixed3<_AllocationObj>
{
private:
	typedef	whbufferallocatorFixed3<_AllocationObj>	FATHERCLASS;
	typedef typename FATHERCLASS::iterator			FATHERIT;
public:
	class	iterator	: public FATHERIT
	{
		typedef	FATHERIT	_FCLASS;
	public:
		iterator()
		{
		}
		explicit iterator(const FATHERIT &_other)
			: FATHERIT(_other)
		{
		}
		iterator(const iterator &_other)
			: FATHERIT(_other)
		{
		}
		inline _Ty & operator * ()
		{
			// 原来直接写成“FATHERIT::operator *();”就不行
			return	*(_Ty *)_FCLASS::operator *();
		}
		inline bool operator == (const iterator & it) const
		{
			return	it.pnode == _FCLASS::pnode;
		}
		inline bool operator != (const iterator & it) const
		{
			return	it.pnode != _FCLASS::pnode;
		}
		inline iterator	operator ++ (int)	// 不写这个会报no postfix form of 'operator ++' found for type 'iterator'
		{
			iterator	old(*this);
			_FCLASS::pnode		= _FCLASS::pnode->next;
			return		old;
		}
		inline iterator & operator ++ ()
		{
			_FCLASS::pnode	= _FCLASS::pnode->next;
			return	*this;
		}
	};
	inline iterator	begin()
	{
		iterator	it(FATHERCLASS::begin());
		return		it;
	}
	inline iterator	end()
	{
		iterator	it(FATHERCLASS::end());
		return		it;
	}
	inline int	Init(_AllocationObj *pAllocationObj, int nMax)
	{
		return	FATHERCLASS::Init(pAllocationObj, nMax, sizeof(_Ty));
	}
	_Ty *	AllocUnit(int *pnID)
	{
		_Ty	*pObj	= (_Ty *)FATHERCLASS::Alloc(pnID);
		if( pObj )
		{
			pObj	= new (pObj) _Ty;
		}
		return	pObj;
	}
	_Ty *	AllocUnitByID(int nID)
	{
		_Ty	*pObj	= (_Ty *)FATHERCLASS::AllocByID(nID);
		if( pObj )
		{
			pObj	= new (pObj) _Ty;
		}
		return	pObj;
	}
	int		FreeUnit(int nID)
	{
		_Ty	*pObj	= (_Ty *)FATHERCLASS::GetByID(nID);
		if( !pObj )
		{
			return	-1;
		}
		FreeUnitByPtr(pObj);
		return	0;
	}
	void	FreeUnitByPtr(_Ty *pObj)
	{
		pObj->~_Ty();
		FATHERCLASS::FreeByPtr(pObj);
	}
	void	clear()	// 释放所有的对象
	{
		iterator it	= begin();
		while( it!=end() )
		{
			iterator	tmp	= it++;
			FreeUnit(tmp.getid());
		}
	}
	inline void	ReserveUnitByPtr(_Ty *ptr)
	{
		FATHERCLASS::ReserveByPtr(ptr);
	}
	inline void	UnReserveUnitByPtr(_Ty *ptr)
	{
		FATHERCLASS::UnReserveByPtr(ptr);
	}
	//inline void	ReserveUnitByIdx(int nIdx)
	//{
	//	FATHERCLASS::ReserveByIdx(nIdx);
	//}
	inline void	UnReserveUnitByID(int nID)
	{
		FATHERCLASS::UnReserveByID(nID);
	}
	inline _Ty *	GetPtrByID(int nID)
	{
		return	(_Ty *)FATHERCLASS::GetByID(nID);
	}
};

}			// EOF namespace n_whcmn

#endif
