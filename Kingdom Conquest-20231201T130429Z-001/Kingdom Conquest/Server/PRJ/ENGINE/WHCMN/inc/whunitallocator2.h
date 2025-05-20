// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : whunitallocator2.h
// Creator     : Wei Hua (魏华)
// Comment     : 新的定上限的动态元素分配器，将会比原来的whunitallocatorFixed更省内存
//             : 刚开始只分配一个指针索引表，具体元素的分配通过whchunkallocationobjonesize来做，只在需要的时候才分配一批内存。
// ChangeLOG   : 2006-03-26 修改了Init的参数先后顺序

#ifndef	__WHUNITALLOCATOR2_H__
#define	__WHUNITALLOCATOR2_H__

#include "./whcmn_def.h"
#include "./whDList.h"
#include "./whallocator2.h"

namespace	n_whcmn
{

// 针对固定长度buffer的带ID的分配器
template <class _AllocationObj>
class	whbufferallocatorFixed
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
		void	clear()
		{
			nID		= -1;
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
	whchunkallocationobjonesize<_AllocationObj>	m_CAOS;
	_AllocationObj	*m_pAllocationObj;
public:
	whbufferallocatorFixed()
	: m_nMax(0)
	, m_nIdxToAlloc(0), m_nRand(0)
	, m_paIdx(NULL)
	, m_DATASIZE(0)
	{
	}
	~whbufferallocatorFixed()
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
	// _CHUNKSIZE为每次集中分配的单元个数
	// _SIZE为数据部分的长度
	int	Init(_AllocationObj *pAllocationObj, int nMax, int _CHUNKSIZE, int _SIZE)
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
		m_DATASIZE		= (_SIZE/4)*4 + ((_SIZE%4)==0 ? 0 : 4);
		if( m_CAOS.Init(m_pAllocationObj, sizeof(node) + m_DATASIZE, _CHUNKSIZE)<0 )
		{
			return	-3;
		}
		return	0;
	}
	int	Release()
	{
		if( m_paIdx == NULL )
		{
			return	0;
		}
		m_pAllocationObj->Free(m_paIdx);
		m_CAOS.Release();
		m_paIdx	= NULL;
		return	0;
	}
	void *	Alloc(int *pnID)
	{
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
			pNode	= (node *)m_CAOS.Alloc();
			pNode->reset();
			pNode->data.clear();
			pNode->data.nID	= m_nIdxToAlloc;
			// 和Idx相关联
			m_paIdx[m_nIdxToAlloc]	= pNode;
			m_nIdxToAlloc++;
		}
		assert( pNode!=NULL );
		pNode->data.nID	= MakeID(pNode->data.nID, MakeRand());
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
			pNode	= (node *)m_CAOS.Alloc();
			pNode->reset();
			pNode->data.clear();
			// 和Idx相关联
			m_paIdx[nIdx]	= pNode;
			m_nIdxToAlloc++;
		}
		// 加入alloc列表
		m_dlAlloc.AddToHead(pNode);
		// 设置ID为指定值
		pNode->data.nID	= nID;
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
	void *	GetByID(int nID)
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
				return	pNode+1;
			}
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
		node	*pNode	= m_paIdx[GetIdxByID(nID)];
		if( pNode )
		{
			if( pNode->data.nID == nID )
			{
				// 释放
				FreeNode(pNode);
				return	0;
			}
		}
		// 无法释放
		return	-2;
	}
	inline void	FreeByPtr(void *ptr)
	{
		// 获得node的指针
		FreeNode(GetNodeByPtr(ptr));
	}
	inline int	size() const
	{
		return	m_dlAlloc.size();
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
class	whunitallocatorFixed2	: public whbufferallocatorFixed<_AllocationObj>
{
private:
	typedef	whbufferallocatorFixed<_AllocationObj>	FATHERCLASS;
	typedef typename FATHERCLASS::iterator			FATHERIT;
public:
	class	iterator	: public FATHERIT
	{
		typedef	FATHERIT	_FCLASS;
	public:
		iterator()
		{
		}
		iterator(const FATHERIT &_other)
			: FATHERIT(_other)
		{
		}
		inline _Ty & operator * ()
		{
			// 原来直接写成“FATHERIT::operator *();”就不行
			return	*(_Ty *)_FCLASS::operator *();
		}
	};
	inline int	Init(_AllocationObj *pAllocationObj, int nMax, int _CHUNKSIZE)
	{
		return	FATHERCLASS::Init(pAllocationObj, nMax, _CHUNKSIZE, sizeof(_Ty));
	}
	_Ty *	Alloc(int *pnID)
	{
		_Ty	*pObj	= (_Ty *)FATHERCLASS::Alloc(pnID);
		if( pObj )
		{
			pObj	= new (pObj) _Ty;
		}
		return	pObj;
	}
	_Ty *	AllocByID(int nID)
	{
		_Ty	*pObj	= (_Ty *)FATHERCLASS::AllocByID(nID);
		if( pObj )
		{
			pObj	= new (pObj) _Ty;
		}
		return	pObj;
	}
	int		Free(int nID)
	{
		_Ty	*pObj	= (_Ty *)GetByID(nID);
		if( !pObj )
		{
			return	-1;
		}
		pObj->~_Ty();
		return	FATHERCLASS::Free(nID);
	}
	void	FreeByPtr(_Ty *pObj)
	{
		pObj->~_Ty();
		FATHERCLASS::FreeByPtr(pObj);
	}
	inline _Ty *	GetByID(int nID)
	{
		return	(_Ty *)FATHERCLASS::GetByID(nID);
	}
};

}			// EOF namespace n_whcmn

#endif
