// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : whunitallocator.h
// Creator     : Wei Hua (魏华)
// Comment     : 基于简单的动态数组的容量只增的元素数组
//               这里使用标准stl是为了可以使用对象对应的构造函数功能
//
//               WHCMN_USECYCLE_ID如果被定义，则被释放的ID会派到候选队列的最后，
//               也就是说一般来说，使用一次被释放后，要等很久才会重新被申请。
//               这样做可以回避一些错误，比如有些ID来不及被删除等等。不过在目前
//               不定义WHCMN_USECYCLE_ID，这样可以及时暴露错误。
//
// CreationDate: 2003-05-22
// ChangeLog   : 
// 2003-06-13  : inline _Ty *	getptr(int nIdx) 原来用const后缀，就必须在return时强制转换。
// 2003-12-26  : 加上ID的范围设置：setrange()函数，这样可以控制生成的ID的长度。
//               另外注意：0和RANDMAX都不会被取到，附加部分在达到RANDMAX时就会变成1。
// 2004-01-17  : 为Fixed添加了分配器，这样它将来就可能使用共享内存了。而且分配器提供New操作，所以这些Allocator也可以分配类对象了。
//               不过类对象只是在最开始被调用了构在，在Alloc时并不会被调用的，所以需要手动clear
// 2005-05-26  : whsmpunitallocator为数据部分少分配了一个字节，导致填写数据的时候可能导致更改了后面的bUsed变量。
//               这个在雪峰的指导下发现的。
// 2005-08-05  : 取消了whunitallocatorFixed的GetByRefIdx、MultiGet。增加了iterator。
//               取消了它内部原来的listavail，而使用whDList取代
//               取消了setmax

#ifndef	__WHUNITALLOCATOR_H__
#define	__WHUNITALLOCATOR_H__

#include "./whlock.h"
#include "./whcmn_def.h"
#include "./whallocator.h"
#include "./whDList.h"
#include "./whlist.h"

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif

namespace	n_whcmn
{

////////////////////////////////////////////////////////////////////
// 固定最大数目的的分配器
// 它的好处是第一次申请的指针可以永远使用
// 而且_Ty可以有构造函数，不过并不是在Alloc的时候调用，而且在预先分配的时候调用的
////////////////////////////////////////////////////////////////////
template < typename _Ty, class _AllocationObj=whcmnallocationobj >
class	whunitallocatorFixed
{
protected:
	typedef	whfakeallocationobj<_AllocationObj>				MYFAO_T;
	typedef	whAND<whfakeallocationobj<_AllocationObj> >		MYAND_T;
	struct	MyTy;
	typedef	whDList<MyTy *>				DL_T;
	typedef	typename DL_T::node			node;
	struct	MyTy
	{
		int			id;
		int			idx;				// 在数组中的序号
		node		dlnode;
		_Ty			data;
		MyTy() : id(-1)
		{
		}
	};
	// 分配器
	MYFAO_T				m_FAO;
	MYAND_T				m_AND;
	MyTy *				m_pMyUnit;		// 数组头指针
	_Ty					m_foo;			// 用于找不到的情况
	int					m_nMax;			// 最多可分配的单元数量
	int					m_nAvailNum;	// 可用的数量
	DL_T				m_dlFree;		// 可用的对象列表
	DL_T				m_dlAllocated;	// 已经分配的对象列表
	DL_T				m_dlReserved;	// 保留先不分配的对象列表
	int					RANDMAX;		// 随机到了这个值就回绕
	int					RANDSHIFT;		// 随机数需要向左移位这么多再和idx组合
	int					REALIDXMASK;	// 真正idx部分的掩码
	int					m_nRand;		// 随机部分
	node				*m_pCurNode;	// 用于BeginGet和GetNext
public:
	class	iterator
	{
	public:
		typedef	typename DL_T::node	node;
	private:
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
		inline _Ty & operator * ()
		{
			return	pnode->data->data;
		}
		inline int	getid() const
		{
			return	pnode->data->id;
		}
		inline int	get_idx() const
		{
			return	pnode->data->idx;
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
	};
	inline iterator	begin()
	{
		iterator	it(m_dlAllocated.begin());
		return		it;
	}
	inline iterator	end()
	{
		iterator	it(m_dlAllocated.end());
		return		it;
	}
	// 0是第一个，以后依次向后
	iterator		getitbynum(int nNum)
	{
		iterator	it	= begin();
		for( ; it!=end() && nNum>0; --nNum)
		{
			++it;
		}
		return		it;
	}
public:
	inline MyTy *	GetMyTyPtr(_Ty *pUnit) const
	{
		return	(MyTy *)(((char *)pUnit) - wh_offsetof(MyTy, data));
	}
	inline int	GetIDByPtr(_Ty *pUnit)
	{
		return	GetMyTyPtr(pUnit)->id;
	}
	inline int		getmax() const
	{
		return	m_nMax;
	}
	inline int		getrealidx(int nID)  const
	{
		return	REALIDXMASK & nID;
	}
	inline int		getrealidx(int nID, MyTy * & pData)  const
	{
		int		nIdx;
		nIdx	= REALIDXMASK & nID;
		if( nIdx<0 || nIdx>=m_nMax )
		{
			return	-1;
		}
		if( m_pMyUnit[nIdx].id == nID )
		{
			pData	= &(m_pMyUnit[nIdx]);
			return	nIdx;
		}
		return	-1;
	}
	// 为了快就不作检查了
	inline _Ty *	getptr(int nID) const
	{
		MyTy	*pMyUnit;
		if( getrealidx(nID, pMyUnit) >= 0 )
		{
			return	&pMyUnit->data;
		}
		return	NULL;
	}
	// 为了快就不作检查了
	inline _Ty &	get(int nID)
	{
		MyTy	*pMyUnit;
		if( getrealidx(nID, pMyUnit) >= 0 )
		{
			return	pMyUnit->data;
		}
		return	m_foo;
	}
	// 检查一个单元是否被使用
	inline bool		IsUnitUsed(int nID) const
	{
		MyTy	*pMyUnit;
		if( getrealidx(nID, pMyUnit) >= 0 )
		{
			return	true;
		}
		return	false;
	}
	// 获得已经分配的单元总数
	inline int		size() const
	{
		return	m_nMax - m_nAvailNum;
	}
	// 获得还可用的单元空间数
	inline int		availsize() const
	{
		return	m_nAvailNum;
	}
	void	FreeToAllocated(MyTy *pMyUnit)
	{
		assert(pMyUnit->dlnode.isinlist(&m_dlFree));
		pMyUnit->dlnode.leave();
		m_dlAllocated.AddToTail(&pMyUnit->dlnode);
		m_nAvailNum	--;
		assert(m_nAvailNum>=0);
	}
	void	AllocatedToFree(MyTy *pMyUnit)
	{
		assert(pMyUnit->dlnode.isinlist(&m_dlAllocated));
		pMyUnit->id	= -1;
		pMyUnit->dlnode.leave();
#ifdef	WHCMN_USECYCLE_ID
		m_dlFree.AddToTail(&pMyUnit->dlnode);
#else
		// 放在前面可以让被释放的尽早使用
		m_dlFree.AddToHead(&pMyUnit->dlnode);
#endif
		m_nAvailNum	++;
		assert(m_nAvailNum<=m_nMax);
	}
	void	FreeToReserved(MyTy *pMyUnit)
	{
		assert(pMyUnit->dlnode.isinlist(&m_dlFree));
		pMyUnit->dlnode.leave();
		m_dlReserved.AddToTail(&pMyUnit->dlnode);
		m_nAvailNum	--;
		assert(m_nAvailNum>=0);
	}
	void	ReservedToFree(MyTy *pMyUnit)
	{
		if(pMyUnit->dlnode.isinlist(&m_dlReserved))	// 2007-11-27 不用assert了，因为可能会上层调用的时候是已经被unreserve过的
		{
			pMyUnit->dlnode.leave();
#ifdef	WHCMN_USECYCLE_ID
			m_dlFree.AddToTail(&pMyUnit->dlnode);
#else
			// 放在前面可以让被释放的尽早使用
			m_dlFree.AddToHead(&pMyUnit->dlnode);
#endif
			m_nAvailNum	++;
			assert(m_nAvailNum<=m_nMax);
		}
	}
public:
	whunitallocatorFixed(_AllocationObj *pAO=NULL)
	: m_FAO(pAO), m_AND(&m_FAO)
	, m_pMyUnit(NULL), m_nMax(0)
	, m_nAvailNum(0)
	, RANDMAX(0x7FFF), RANDSHIFT(16), REALIDXMASK(0xFFFF)
	, m_nRand(0)
	, m_pCurNode(NULL)
	{
	}
	whunitallocatorFixed(int nMax, _AllocationObj *pAO=NULL)
	: m_FAO(pAO), m_AND(&m_FAO)
	{
		Init(nMax);
	}
	~whunitallocatorFixed()
	{
		Release();
	}
	// 设置底层分配器
	inline void		SetAO(_AllocationObj *pAO)
	{
		m_FAO.SetAO(pAO);
	}
	inline _AllocationObj *	GetAO()
	{
		return	m_FAO.GetAO();
	}
	void	setrange(int _RANDMAX, int _RANDSHIFT, int _REALIDXMASK)
	{
		RANDMAX		= _RANDMAX;
		RANDSHIFT	= _RANDSHIFT;
		REALIDXMASK	= _REALIDXMASK;
	}
	int		Init(int nMax)
	{
		assert(nMax>0 && nMax<0xFFFFFF);

		Release();

		m_nMax		= nMax;
		m_pMyUnit	= m_AND.NewArray((MyTy *)0, nMax);

		clear();

		return	0;
	}
	int		Release()
	{
		if( m_pMyUnit )
		{
			m_AND.DeleteArray(m_pMyUnit, m_nMax);
			m_pMyUnit	= NULL;
		}
		// 如果不设这个，则size()会返回nMax
		m_nMax	= 0;
		return	0;
	}
	int		MakeNewID(int nIdx)
	{
		if( (++m_nRand) >= RANDMAX )
		{
			// 这样序号就不会为0了。不过还是按照<0非法来判断吧
			m_nRand	= 1;
		}
		return	(m_nRand<<RANDSHIFT) | nIdx;
	}
	int		AllocUnit(_Ty * & pUnit)
	{
		pUnit	= NULL;
		if( availsize()==0 )
		{
			return	-1;
		}
		node	*pNode	= m_dlFree.begin();
		MyTy	*pMyUnit	= pNode->data;
		int		nIdx		= pMyUnit->idx;
		pMyUnit->id		= MakeNewID(nIdx);
		pUnit			= &pMyUnit->data;
		FreeToAllocated(pMyUnit);
		return	pMyUnit->id;
	}
	// 判断一个ID是否可以AllocUnitByID
	bool			CanAllocUnitByID(int nID)
	{
		int		nIdx;
		nIdx	= REALIDXMASK & nID;
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// 序号超界
			return	false;
		}
		if( m_pMyUnit[nIdx].id >=0 )
		{
			// 已经被分配
			return	false;
		}
		return		true;
	}
	// 根据预先知道的ID号分配一个单元，这个用于在结构相同的各个网络对象间同步ID
	_Ty *			AllocUnitByID(int nID, int *pnOldID=NULL)
	{
		int		nIdx;
		nIdx	= REALIDXMASK & nID;
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// 序号超界
			return	NULL;
		}
		MyTy	*pMyUnit	= &m_pMyUnit[nIdx];
		if( pMyUnit->id >=0 )
		{
			// 已经被分配
			if( pnOldID )
			{
				*pnOldID	= pMyUnit->id;
			}
			return	NULL;
		}
		// 设置ID
		pMyUnit->id		= nID;
		FreeToAllocated(pMyUnit);
		return	&pMyUnit->data;
	}
	// 判断一个ID是否可以AllocUnitByIdx
	bool			CanAllocUnitByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// 序号超界
			return	false;
		}
		if( m_pMyUnit[nIdx].id >=0 )
		{
			// 已经被分配
			return	false;
		}
		return		true;
	}
	// 根据预先知道的Idx号分配一个单元，这个用于纯根据序号决定分配的地方
	_Ty *			AllocUnitByIdx(int nIdx, int *pnNewID, int *pnOldID=NULL)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// 序号超界
			return	NULL;
		}
		MyTy	*pMyUnit	= &m_pMyUnit[nIdx];
		if( pMyUnit->id >=0 )
		{
			// 已经被分配
			if( pnOldID )
			{
				*pnOldID	= pMyUnit->id;
			}
			return	NULL;
		}
		// 设置ID
		*pnNewID = pMyUnit->id = MakeNewID(nIdx);
		FreeToAllocated(pMyUnit);
		return	&pMyUnit->data;
	}
	// 释放一个单元
	int		FreeUnit(int nID)
	{
		MyTy	*pMyUnit;
		int		nIdx;
		if( (nIdx=getrealidx(nID, pMyUnit))<0 )
		{
			// 已经释放了(序号超界我不管)
			return	-1;
		}
		AllocatedToFree(pMyUnit);
		return	0;
	}
	// 根据序号释放一个单元
	int		FreeUnitByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// 序号超界
			return	-1;
		}
		AllocatedToFree(&m_pMyUnit[nIdx]);
		return	0;
	}
	// 根据指针释放单元(这样就不在乎pUnit的内容是否已经被清空了。如果用FreeUnit可能会用到pUnit中存储的ID)
	int		FreeUnitByPtr(_Ty *pUnit)
	{
		// 获得pUnit对应的MyTy
		AllocatedToFree(GetMyTyPtr(pUnit));
		return	0;
	}
	// 根据序号保留一个Free的单元
	int		ReserveUnitByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// 序号超界
			return	-1;
		}
		FreeToReserved(&m_pMyUnit[nIdx]);
		return	0;
	}
	// 根据指针保留一个Free的单元
	int		ReserveUnitByPtr(_Ty *pUnit)
	{
		FreeToReserved(GetMyTyPtr(pUnit));
		return	0;
	}
	// 根据序号取消保留一个单元
	int		UnReserveUnitByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// 序号超界
			return	-1;
		}
		ReservedToFree(&m_pMyUnit[nIdx]);
		return	0;
	}
	// 根据指针取消保留一个单元
	int		UnReserveUnitByPtr(_Ty *pUnit)
	{
		ReservedToFree(GetMyTyPtr(pUnit));
		return	0;
	}
	// 把保留单元重新放入Free队列
	void		clear()
	{
		if( !m_pMyUnit )
		{
			// 已经被release了
			return;
		}
		int		i;
		MyTy	*pMyUnit = &m_pMyUnit[0];
		for(i=0;i<m_nMax;i++)
		{
			pMyUnit->id		= -1;
			pMyUnit->idx	= i;
			// 为了保险leave一下
			pMyUnit->dlnode.leave();
			pMyUnit->dlnode.data	= pMyUnit;
			// 这个确保是要按顺序添加的
			m_dlFree.AddToTail(&pMyUnit->dlnode);
			pMyUnit	++;
		}
		m_nAvailNum	= m_nMax;
	}
	// 开始顺序获取
	inline void		BeginGet()
	{
		m_pCurNode	= m_dlAllocated.begin();
	}
	// 获取下一个
	_Ty *	GetNext()
	{
		if( m_pCurNode == m_dlAllocated.end() )
		{
			return	NULL;
		}
		_Ty	*pData	= &m_pCurNode->data->data;
		m_pCurNode	= m_pCurNode->next;
		return	pData;
	}
	_Ty *	GetNext(int *pnID)
	{
		if( m_pCurNode == m_dlAllocated.end() )
		{
			return	NULL;
		}
		_Ty	*pData	= &m_pCurNode->data->data;
		*pnID		= m_pCurNode->data->id;
		m_pCurNode	= m_pCurNode->next;
		return	pData;
	}
	// 根据下标得到对象指针
	_Ty *	GetByIdx(int nIdx, int *pnID=0)
	{
		if( nIdx<0 || nIdx>m_nMax )	return	NULL;
		MyTy	*pMyUnit = &m_pMyUnit[nIdx];
		if( pMyUnit->id < 0 )		return NULL;
		if( pnID )					*pnID = pMyUnit->id;
		return	&pMyUnit->data;
	}
};

// 调用所有元素的clear方法(这个当然要求所有原素都有clear方法)
template < typename _Ty, class _AllocationObj >
void	whunitallocatorFixed_AllClear(whunitallocatorFixed<_Ty, _AllocationObj> &uaf)
{
	// 注：如果不写typename则在linux下编译会出warning: implicit typename is deprecated ...
	for(typename whunitallocatorFixed<_Ty, _AllocationObj>::iterator it=uaf.begin(); it!=uaf.end(); ++it)
	{
		(*it).clear();
	}
}

////////////////////////////////////////////////////////////////////
// 定长数组
////////////////////////////////////////////////////////////////////
template <typename _Ty>
class	whunitarray
{
protected:
	int		m_nSize;					// 元素总数
	_Ty		*m_pBuf;
public:
	whunitarray()
	: m_nSize(0), m_pBuf(0)
	{
	}
	~whunitarray()
	{
		Release();
	}
	int	Init(int nSize)
	{
		assert(nSize>0 && nSize<0xFFFFFF);

		Release();
		m_pBuf	= new _Ty[nSize];
		m_nSize	= nSize;
		if( !m_pBuf )
		{
			return	-1;
		}
		return	0;
	}
	int	Release()
	{
		if( m_pBuf )
		{
			delete []	m_pBuf;
			m_pBuf		= 0;
		}
		return	0;
	}
	inline int		size() const
	{
		return	m_nSize;
	}
	inline _Ty *	getptr(int nIdx) const
	{
		if( nIdx<0 || nIdx>=m_nSize )
		{
			return	NULL;
		}
		return	m_pBuf + nIdx;
	}
	inline _Ty &	get(int nIdx)
	{
		assert(nIdx>=0 && nIdx<m_nSize);
		return	m_pBuf[nIdx];
	}
	inline _Ty &	operator[](int nIdx)
	{
		return	get(nIdx);
	}
	inline void		clearwithvalue(_Ty val)
	{
		int	i;
		for(i=0;i<m_nSize;i++)
		{
			m_pBuf[i]	= val;
		}
	}
};

////////////////////////////////////////////////////////////////////
// 多固定长度缓冲的数组
////////////////////////////////////////////////////////////////////
class	whbufarray
{
protected:
	char	*m_pBuf;
	int		m_nUnitSize;
	int		m_nTotal;
public:
	whbufarray();
	~whbufarray();
	int		Init(int nTotal, int nUnitSize);
	int		Release();
	char *	GetPtr(int nIdx) const;
	inline char *	operator[](int nIdx) const
	{
		return	GetPtr(nIdx);
	}
};

////////////////////////////////////////////////////////////////////
// 基于chunk的固定长度单元的分配
////////////////////////////////////////////////////////////////////
class	whsmpunitallocator
{
public:
	// 定义
	#pragma pack(1)
	struct	UNIT_T
	{
		bool				bUsed;
		char				data[1];
	};
	#pragma pack()
public:
	// 成员
	whallocbysmpchunk		m_chunkUnit;
	whidxlist				m_listAvail;
	int						m_nUnitSize;
public:
	whsmpunitallocator(int nUnitSize, int nChunkSize=100)
	: m_listAvail(nChunkSize)
	, m_nUnitSize(nUnitSize)
	{
		m_chunkUnit.m_info.nUnitSize	= nUnitSize + wh_offsetof(UNIT_T, data);
		m_chunkUnit.m_info.nChunkSize	= nChunkSize;
	}
	// 获得内部
	inline UNIT_T *	getunit(int nIdx)
	{
		if( nIdx<0 || nIdx>=(int)m_chunkUnit.size() )
		{
			return	NULL;
		}
		return	(UNIT_T *)m_chunkUnit[nIdx];
	}
	// 获得指定序号对应的单元指针(不检查nIdx超界，慎用)
	inline char *	getptr(int nIdx)
	{
		UNIT_T	*pUnit	= getunit(nIdx);
		if( pUnit && pUnit->bUsed )
		{
			return	pUnit->data;
		}
		else
		{
			return	NULL;
		}
	}
	// 检查一个单元是否被使用(上层保证序号有效)
	inline bool		IsUnitUsed(int nIdx) const
	{
		return	((UNIT_T *)m_chunkUnit[nIdx])->bUsed;
	}
	// 获得最大的尺寸
	inline int		maxsize() const
	{
		return	m_chunkUnit.size();
	}
	// 获得已经分配的单元总数
	inline int		size() const
	{
		return	m_chunkUnit.size() - m_listAvail.size();
	}
public:
	// 分配一个单元，返回分配的序号
	int		AllocUnit(char *&pBuf);
	// 释放一个单元
	int		FreeUnit(int nIdx);
	// 清除(释放)所有单元
	void	clear();
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHUNITALLOCATOR_H__
