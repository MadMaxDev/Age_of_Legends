// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whhash.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的hashtable
//                参考了cui和stlport的实现
//                不过这里面不会自动调用类的构造函数，因为想避免delete重载后的嵌套(虽然不会出错)
//                注意：即使不使用，hashtable还是会占用一定的初始内存
// Function list: _whcmn_hashfunc	普通的hash仿函数
//              : whhashbase		hash基础类
//              : whhash			hashtable 键/值对应
//              : whhashset			只用来表示一个键是否存在
//              : whhashmapset		单向映射表，比如用来做好友列表 (一个键映射多个不重复的值)
//              : whhashstore		一个键存储多个可能重复的值
//              : whhash2			简单的小规模双级检测的hashtable实现
//              : whhashidmapstr、whhashstrptrmapstrptr		id到string的映射
//              : whhashidmapid		通过whhash的方法生成整数对整数的对应表
//
// CreationDate : 2003-05-15
// ChangeLog    :
//                2004-01-16 不想考虑whmem了，这里面都用new和delete了
//                2004-01-17 把hash和hashset合并，里面大部分的代码其实都是相同的
//                           把whpair挪走了，分配采用chunk而不是new/delete了
//                2005-09-26 增加了hash表拷贝的功能
//                2006-07-04 修正了put返回的iterator中ppnode指向的是一个临时变量的地址的错误。
//                2007-06-20 修正了当多个键被放到同一个bucket中，删除bucket链表中的非第一、第二元素时导致链表被清空的错误（表现在寻仙中频繁在表中增加和删除道具导致道具丢失）。
//                2007-06-28 增加了getorput方法，用于配合list对于同一个键的多值插入。
//                2007-07-11 增加了whhash2::has方法
//                2007-07-17 whhash2的iterator的UNIT_T & operator * ()方法改为非const返回
//                2007-09-07 把公共的东西放到whhash_base.h中，whhash2放到别的whhash2.h中
//                2007-09-10 取消了chunk的分配

// 注意：2005-01-27
// 现在在whstr4hash的忽略大小写方面有一些混乱。虽然在whstring.h中我定义了m_bIC，但是实际上whstr4hash还是大小写敏感的。
// 这个今后需要重新整理一下。不过现在外面用的还算稳定就先不动了。
//                2005-12-29 把hashfunc中对指针的处理原来的>>4处理去掉了，因为数组大小是质数，一定和任何非1非他自己的数互质。
//                2005-12-29 增加了whhash2。
//                2006-02-16 开放了g_prime_list中的17和29，因为将来玩家身上的物品可能会用hash表来管理，而物品可能会比较少。
//                2006-05-16 把size_t -> unsigned int -> unsigned int
// 注意：2007-06-10
// 在hash表可能动态增大（即resizebucket被调用）的情况下，应该避免记录迭代子在今后使用，因为buckets的增长会导致旧迭代子中的bucketidx失效。
// 如果实在要这么使用，请在最开始就调用resizebucket把hash表的尺寸设置的足够大。
// 注意：2007-06-28
// clear方法只是清空所有的对应关系，但是只是把释放的节点放入freenode列表中，不会真正删除，所以在重新分配某些对象之后请自行调用对象的初始化清空函数
// clearclear方法可以用来清空所有的值

#ifndef	__WHHASH_H__
#define	__WHHASH_H__

#include "whhash_base.h"
#include "whstring.h"
#include <assert.h>

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// hash基础
////////////////////////////////////////////////////////////////////
template <typename _Key, typename _Node, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhashbase
{
public:											// public为了给iterator用
	typedef	whfakeallocationobj<_AllocationObj>				MYFAO_T;
	typedef	whAND<whfakeallocationobj<_AllocationObj> >		MYAND_T;
	typedef	_Node			MYNODE;				// 这个主要为了给子类使用
	typedef	whvector<_Node *, false, _AllocationObj>		MYVECT_T;

	MYFAO_T					m_FAO;
	MYAND_T					m_AND;
	_HashFunc				_hash;
	unsigned int			_elementnum;
	MYVECT_T				_buckets;			// 对应一个hash值的桶（每个桶里都是一个_Node链表）
	bool					_bNeverResizeAgain;	// 是否今后再也不能resize了
public:
	struct	stat_t
	{
		int		usedbucketnum;					// 有东西的bucket数量
		int		maxnum, maxidx;
		int		minnum, minidx;
	};
	// 这个iterator可以保留做删除节点的参数（注意：当hash表扩大之后迭代子就会实效）
	// 为了简便就不实现"--"操作了
	class	key_iterator
	{
	public:
		unsigned int	bucketidx;				// 标记自己属于那个桶
		_Node			*cur;
		whhashbase		*base;
	public:
		key_iterator()
		: bucketidx(0), cur(NULL), base(NULL)
		{
		}
		key_iterator(unsigned int _bucketidx, _Node *_cur, whhashbase *_base)
		: bucketidx(_bucketidx), cur(_cur), base(_base)
		{
		}
		key_iterator(const key_iterator &_other)
		: bucketidx(_other.bucketidx), cur(_other.cur), base(_other.base)
		{
		}
	public:
		inline bool operator == (const key_iterator & it) const
		{
			return	it.bucketidx==bucketidx && it.cur==cur;
		}
		inline bool operator != (const key_iterator & it) const
		{
			return	! operator == (it);
		}
		// 不写这个会报no postfix form of 'operator ++' found for type 'key_iterator'
		// 其实这个就是 x++，而下面就是++x。
		inline key_iterator	operator ++ (int)
		{
			key_iterator	old(*this);
			operator ++ ();
			return			old;
		}
		// 主要用于迭代子的++操作，保证找到下一个
		inline void	suregoodnext()
		{
			// 找到下一个非空的cur
			if( !cur )
			{
				while(!cur)
				{
					if( ++bucketidx==base->_buckets.size() )	// 这里就不考虑到了结尾还调用suregoodnext的情况了
					{
						return;
					}
					cur		= base->_buckets[bucketidx];
				}
			}
		}
		inline key_iterator & operator ++ ()
		{
			if( cur )
			{
				cur		= cur->_next;
			}
			suregoodnext();
			return		*this;
		}
		inline key_iterator & operator += (int n)
		{
			// <0无效
			assert(n>=0);
			for(int i=0;i<n;i++)
			{
				operator ++ ();
			}
			return	*this;
		}
		inline const _Key & operator * ()
		{
			return	cur->_key;
		}
		// 看看两个是否在同一个bucket中
		inline bool	like(const key_iterator &it) const
		{
			return	bucketidx == it.bucketidx;
		}
	};
	inline key_iterator	begin()
	{
		key_iterator	it;
		it.bucketidx	= 0;
		it.cur			= _buckets[0];
		it.base			= this;
		it.suregoodnext();
		return			it;
	}
	inline key_iterator	end()
	{
		key_iterator	it;
		it.bucketidx	= _buckets.size();
		it.cur			= NULL;
		it.base			= this;
		return			it;
	}
public:
	// 默认为最小大小(即质数表中的第一个53)，n是希望的大小
	// _M_next_size计算出的值应该>=它
	whhashbase(unsigned int n = 0, _AllocationObj *pAO = NULL)
	: m_FAO(pAO), m_AND(&m_FAO)
	, _hash()
	, _elementnum(0)
	, _buckets(_M_next_size(n),(_Node *)0, pAO)
	, _bNeverResizeAgain(false)
	{
	}
	whhashbase(const NULLCONSTRUCT_T &nc)
		: _buckets(nc)
	{
	}
	~whhashbase()
	{
		clear();
	}

public:
	void	clear();
	void	clearclear();
	// 添加(里面会调用put_noresize)
	// 如果原来key就存在则无法存入（并返回false）
	bool	put(const _Key &key, _Node *&pNode, unsigned int &nBucketIdx);
	bool	put_noresize(const _Key &key, _Node *&pNode, unsigned int &nBucketIdx);
	// 返回iterator的put
	key_iterator	put(const _Key &key);
	// 获取指定键值的单元
	bool	get(const _Key &key, _Node *&pNode) const;
	key_iterator	find(const _Key & key);
	// 判断指定键值是否存在
	bool	has(const _Key &key) const;
	// 获取指定键值的单元，同时删除之
	bool	geterase(const _Key &key, _Node &Node);
	// 移除指定键值的单元
	bool	erase(const _Key &key);
	// 根据iterator删除（注意，这个在遍历过程使用需要谨慎）
	bool	erase(key_iterator &it);
	// 统计数据的分布情况，用于检查哈西函数是否合理
	bool	statistic(stat_t *pstat);
	// 将所有key导出到一个vector中
	template<typename _VECTKEY>
	bool	exporttovector(_VECTKEY &vectKey) const
	{
		vectKey.clear();
		vectKey.reserve(_elementnum);

		for(_Node **ppNode=_buckets.begin(); ppNode!=_buckets.end(); ++ppNode)
		{
			_Node * cur	= *ppNode;
			while(cur != NULL)
			{
				vectKey.push_back(cur->_key);
				cur		= cur->_next;
			}
		}

		return	true;
	}
	// 这个外部可以调用可以在一开始就将buckets的数量放大到一定程度
	// 注意resizebucket，不是改变元素个数，而是bucket的数量，
	// _elementnum的数量是不可能被resize的，只能通过put来增加
	void	resizebucket(unsigned int n, bool bNeverResizeAgain=false);
	// 简便的信息访问
	inline unsigned int	size() const		{ return _elementnum; }
	inline bool		empty() const		{ return size() == 0; }
	// 复制别的hash内容到自己
	void	copyfrom(whhashbase &other);
	inline void operator = (whhashbase & other)
	{
		copyfrom(other);
	}
	void	AdjustInnerPtr(int nOffset)
	{
		_buckets.AdjustInnerPtr(nOffset);
		m_FAO.AdjustInnerPtr(nOffset);
		m_AND.SetAO(&m_FAO);
		// 把已有的单元指针全部调整一下
		for(size_t i=0;i<_buckets.size();++i)
		{
			_Node * &cur0	= _buckets[i];
			if( cur0==NULL )	continue;
			wh_adjustaddrbyoffset(cur0, nOffset);
			_Node *	cur		= cur0;
			while(cur->_next != NULL)
			{
				wh_adjustaddrbyoffset(cur->_next, nOffset);
				cur		= cur->_next;
			}
		}
	}
protected:
	unsigned int	_M_next_size(unsigned int n) const
	{
		return _next_prime(n);
	}
	inline _Node *	AllocNode()
	{
		_Node	*pNode = NULL;
		m_AND.New(pNode);
		++ _elementnum;
		return			pNode;
	}
	inline void		FreeNode(_Node *pNode)
	{
		m_AND.Delete(pNode);
		-- _elementnum;
	}
};
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
void	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
resizebucket(unsigned int n, bool bNeverResizeAgain)
{
	const unsigned int old_n	= _buckets.size();
	if(n > old_n)
	{
		if( _bNeverResizeAgain )
		{
			// 肯定是上层需要不能扩大hash表。比如上层需要保存迭代子以备后用的。
			assert(0);
			return;
		}
		_bNeverResizeAgain	= bNeverResizeAgain;

		// 这样n就一定大于old_n的
		n	= _M_next_size(n);
		// 把原来的数据全部移出到tmp中
		MYVECT_T	tmp(n, (_Node*)(0), _buckets.GetAO());
		for(unsigned int i=0; i<old_n; ++i)
		{
			_Node* first	= _buckets[i];
			while(first)
			{
				unsigned int idx	= _hash(first->_key) % n;
				_buckets[i] = first->_next;
				// 节点进入tmp
				first->_next	= tmp[idx];
				tmp[idx]	= first;
				// 看下一个
				first		= _buckets[i];
			}
			// 转移结束后_buckets[i]指向空
		}
		// 交换内存(指针交换)，在函数结束时tmp会释放换过去的内存空间
		_buckets.swap(tmp);
	}
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
void	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
copyfrom(whhashbase &other)
{
	if( this == &other )
	{
		// 自己不用拷贝给自己了
		return;
	}
	// 清空自己
	clear();
	// 插入
	for(key_iterator it=other.begin(); it!=other.end(); ++it)
	{
		_Node			*pNode;
		unsigned int	nBucketIdx;
		if( put((*it), pNode, nBucketIdx) )
		{
			*pNode	= *(it.cur);
		}
		else
		{
			// 不可能的
			assert(0);
		}
	}
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
void	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
clear()
{
	if( _elementnum==0 )
	{
		// 已经空了，不用清空了
		return;
	}
	for(unsigned int i=0; i<_buckets.size(); ++i)
	{
		_Node *	cur			= _buckets[i];
		while (cur != NULL)
		{
			_Node *	next	= cur->_next;
			FreeNode(cur);
			cur				= next;
		}
		_buckets[i]			= NULL;
	}
	// 此时应该是0了
	assert(_elementnum == 0);
	_elementnum = 0;
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
void	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
clearclear()
{
	if( _elementnum==0 )
	{
		// 已经空了，不用清空了
		return;
	}
	for(unsigned int i=0; i<_buckets.size(); ++i)
	{
		_Node *	cur			= _buckets[i];
		while (cur != NULL)
		{
			_Node *	next	= cur->_next;
			cur->_val.clear();	// 清空值（值类型必须有clear方法）
			FreeNode(cur);
			cur				= next;
		}
		_buckets[i]			= NULL;
	}
	// 此时应该是0了
	assert(_elementnum == 0);
	_elementnum = 0;
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
bool	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
put(const _Key &key, _Node *&pNode, unsigned int &nBucketIdx)
{
	resizebucket(_elementnum + 1);		
	return put_noresize(key, pNode, nBucketIdx);
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
bool	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
put_noresize(const _Key &key, _Node *&pNode, unsigned int &nBucketIdx)
{
	nBucketIdx		= _hash(key) % _buckets.size();
	_Node* first	= _buckets[nBucketIdx];

	// 判断是否key已经存在
	for(_Node* cur = first; cur; cur=cur->_next) 
		if (cur->_key == key)
			return false;

	// 创建节点并连入nBucketIdx位置的链表
	pNode			= AllocNode();
	pNode->_key		= key;
	pNode->_next	= first;
	_buckets[nBucketIdx]	= pNode;
	return			true;
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
typename whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::key_iterator	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
put(const _Key &key)
{
	resizebucket(_elementnum + 1);
	_Node			*pNode;
	unsigned int	nBucketIdx;
	if( !put_noresize(key, pNode, nBucketIdx) )
	{
		return	end();
	}
	return	key_iterator(nBucketIdx, pNode, this);
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
bool	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
get(const _Key & key, _Node *&pNode) const
{
	unsigned int n		= _hash(key) % _buckets.size();
	_Node *	first	= _buckets[n];

	for(_Node* cur=first; cur; cur=cur->_next) 
		if(cur->_key == key)
		{
			pNode	= cur;
			return	true;
		}
	return	false;
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
typename whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::key_iterator	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
find(const _Key & key)
{
	unsigned int n	= _hash(key) % _buckets.size();
	_Node *	first	= _buckets[n];

	for(_Node* cur=first; cur; cur=cur->_next) 
		if(cur->_key == key)
		{
			return	key_iterator(n, cur, this);
		}
	return	end();
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
bool	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
has(const _Key &key) const
{
	_Node	*pNode;
	return	get(key, pNode);
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
bool	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
geterase(const _Key &key, _Node &Node)
{
	unsigned int n		= _hash(key) % _buckets.size();
	_Node *	first	= _buckets[n];
	_Node * prev	= NULL;

	for(_Node* cur=first; cur; prev=cur,cur=cur->_next) 
		if(cur->_key == key)
		{
			Node	= *cur;
			if(cur == first)
			{
				_buckets[n]	= cur->_next;
			}
			else
			{
				prev->_next	= cur->_next;
			}
			FreeNode(cur);
			return	true;
		}
	return	false;
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
bool	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
erase(const _Key & key)
{
	unsigned int n		= _hash(key) % _buckets.size();
	_Node * first	= _buckets[n];
	_Node * prev	= NULL;

	for(_Node* cur=first; cur; prev=cur,cur=cur->_next)
	{
		if(cur->_key == key)
		{			
			if(cur == first)
			{
				_buckets[n]	= cur->_next;
			}
			else
			{
				prev->_next	= cur->_next;
			}
			FreeNode(cur);
			return true;
		}
	}
	return false;
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
bool	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
erase(key_iterator &it)
{
	if( !it.cur )
	{
		assert(0);
		return	false;
	}
	_Node	*&pBucketNode	= _buckets[it.bucketidx];
	if( pBucketNode == NULL )
	{
		// 删除一个不存在的迭代子，可能是记录了老的迭代子，但是hash表长大了
		assert(0);
		return	false;
	}
	if( it.cur == pBucketNode )
	{
		// 就是第一个，直接换筒的根指针
		pBucketNode	= it.cur->_next;
	}
	else
	{
		_Node	*pCur		= pBucketNode;
		while( pCur->_next != it.cur )
		{
			pCur	= pCur->_next;
			if( pCur->_next==NULL )
			{
				// 没找到，这个也比较奇怪
				assert(0);
				return	false;
			}
		}
		pCur->_next	= it.cur->_next;
	}

	// 最后释放node的内存
	FreeNode(it.cur);
	return	true;
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
bool	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
statistic(stat_t *pstat)
{
	pstat->usedbucketnum	= 0;
	pstat->maxidx	= -1;
	pstat->minidx	= -1;
	pstat->maxnum	= 0;
	pstat->minnum	= size();

	int	idx, num;
	idx				= 0;
	for(_Node **ppNode=_buckets.begin(); ppNode!=_buckets.end(); ++ppNode)
	{
		_Node * cur	= *ppNode;
		num			= 0;
		while(cur != NULL)
		{
			cur		= cur->_next;
			++ num;
		}
		if( num )
		{
			pstat->usedbucketnum	++;
			if( num>pstat->maxnum )
			{
				pstat->maxidx	= idx;
				pstat->maxnum	= num;
			}
			if( num<pstat->minnum )
			{
				pstat->minidx	= idx;
				pstat->minnum	= num;
			}
		}

		++ idx;
	}

	return	true;
}

////////////////////////////////////////////////////////////////////
// hashtable 键/值对应
////////////////////////////////////////////////////////////////////
template <typename _Key, typename _Value, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhash	: public whhashbase<_Key, whhash_node<_Key, _Value>, _AllocationObj, _HashFunc>
{
private:
	typedef	whhashbase<_Key, whhash_node<_Key, _Value>, _AllocationObj, _HashFunc>	FATHERHASH;
public:
	whhash(unsigned int n = 0, _AllocationObj *pAO = NULL)						// 默认为最小大小(即质数表中的第一个53)
	: FATHERHASH(n, pAO)
	{
	}
	class	kv_iterator	: public FATHERHASH::key_iterator
	{
	private:
		typedef	typename FATHERHASH::key_iterator	FATHER_IT;
	public:
		kv_iterator() 
		: FATHER_IT()
		{
		}
		kv_iterator(const FATHER_IT &_other) 
		: FATHER_IT(_other)
		{
		}
		inline _Key & getkey ()
		{
			return	FATHER_IT::cur->_key;
		}
		inline _Value & getvalue ()
		{
			return	FATHER_IT::cur->_val;
		}
	};
public:
	bool	put(const _Key &key, const _Value &val)
	{
		typename FATHERHASH::MYNODE	*pNode;
		unsigned int	nBucketIdx;
		bool	bRst = FATHERHASH::put(key, pNode, nBucketIdx);
		if( bRst )
		{
			pNode->_val	= val;
		}
		return	bRst;
	}
	bool	putalloc(const _Key &key, _Value *&pVal)
	{
		typename FATHERHASH::MYNODE	*pNode;
		unsigned int	nBucketIdx;
		bool	bRst = FATHERHASH::put(key, pNode, nBucketIdx);
		if( bRst )
		{
			pVal		= &pNode->_val;
		}
		return	bRst;
	}
	// 先查找，如果有就直接返回，没有就put
	kv_iterator	getorput(const _Key &key)
	{
		kv_iterator	it	= FATHERHASH::find(key);
		if( it!=FATHERHASH::end() )
		{
			return	it;
		}
		return	FATHERHASH::put(key);
	}
	_Value &	operator [] (const _Key &key)
	{
		kv_iterator	it	= getorput(key);
		return	it.getvalue();
	}
	// 返回iterator的put
	inline kv_iterator	put(const _Key &key)
	{
		return	FATHERHASH::put(key);
	}
	// get如果得不到结果就不改变*ppval或者val的值，所以可以在里面预先设置一个表示错误的值
	// 用指针的原因是可以置为空，就不用填了
	bool	get(const _Key &key, _Value **ppval = NULL) const
	{
		typename FATHERHASH::MYNODE	*pNode;
		bool	bRst = FATHERHASH::get(key, pNode);
		if( bRst && ppval )
		{
			*ppval	= &pNode->_val;
		}
		return	bRst;
	}
	bool	get(const _Key &key, _Value &val) const
	{
		typename FATHERHASH::MYNODE	*pNode;
		bool	bRst = FATHERHASH::get(key, pNode);
		if( bRst )
		{
			val		= pNode->_val;
		}
		return	bRst;
	}
	_Value *	getptr(const _Key &key, bool bCreateIfNotExisted=false)
	{
		typename FATHERHASH::MYNODE	*pNode;
		if( FATHERHASH::get(key, pNode) )
		{
			return	&pNode->_val;;
		}
		if( bCreateIfNotExisted )
		{
			unsigned int	nBucketIdx;
			if( FATHERHASH::put(key, pNode, nBucketIdx) )
			{
				return	&pNode->_val;
			}
		}
		return	NULL;
	}
	bool	geterase(const _Key &key, _Value &val)
	{
		typename FATHERHASH::MYNODE	Node;
		bool	bRst = FATHERHASH::geterase(key, Node);
		if( bRst )
		{
			val		= Node._val;
		}
		return	bRst;
	}
	// 下面两个比较长，放在类定义外面了
	// 移除所有值为val的条目
	bool	eraseval(const _Value &val);
	// 将所有val导出到一个vector中
	template<typename _VECTVAL>
	bool	exportvaltovector(_VECTVAL &vectVal) const
	{
		vectVal.clear();
		vectVal.reserve(FATHERHASH::_elementnum);

		for(typename FATHERHASH::MYNODE **ppNode=FATHERHASH::_buckets.begin(); ppNode!=FATHERHASH::_buckets.end(); ++ppNode)
		{
			typename FATHERHASH::MYNODE *cur	= *ppNode;
			while(cur != NULL)
			{
				vectVal.push_back(cur->_val);
				cur		= cur->_next;
			}
		}

		return	true;
	}
};
template <typename _Key, typename _Value, class _AllocationObj, class _HashFunc>
bool	whhash<_Key, _Value, _AllocationObj, _HashFunc>::
eraseval(const _Value & val)
{
	bool	bRst = false;
	for(typename FATHERHASH::MYNODE **ppnode=FATHERHASH::_buckets.begin(); ppnode!=FATHERHASH::_buckets.end(); ++ppnode)
	{
		typename FATHERHASH::MYNODE *cur	= *ppnode;
		typename FATHERHASH::MYNODE *tmp, *prev;
		while(cur != NULL)
		{
			tmp	= cur;
			cur	= cur->_next;
			if( tmp->_val == val )
			{
				if( tmp == *ppnode )
				{
					*ppnode		= cur;
				}
				else
				{
					prev->_next	= cur;
				}
				// 删除
				FATHERHASH::FreeNode(tmp);
				//
				tmp		= cur;
				bRst	= true;
			}
			prev		= tmp;
		}
	}

	return	bRst;
}

////////////////////////////////////////////////////////////////////
// hashset 只用来表示一个键是否存在
////////////////////////////////////////////////////////////////////
template <typename _Key, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhashset	: public whhashbase<_Key, whhashset_node<_Key>, _AllocationObj, _HashFunc>
{
private:
	typedef	whhashbase<_Key, whhashset_node<_Key>, _AllocationObj, _HashFunc>		FATHERHASH;
public:
	whhashset(unsigned int n = 0, _AllocationObj *pAO = NULL)	// 默认为最小大小(即质数表中的第一个53)
	: FATHERHASH(n, pAO)
	{
	}
public:
	bool	put(const _Key & key)
	{
		typename FATHERHASH::MYNODE	*pNode;
		unsigned int	nBucketIdx;
		return	FATHERHASH::put(key, pNode, nBucketIdx);
	}
	typename FATHERHASH::key_iterator	putgetit(const _Key & key)
	{
		return	FATHERHASH::put(key);
	}
};

////////////////////////////////////////////////////////////////////
// 单向映射表，比如用来做好友列表 (一个键映射多个不重复的值)
////////////////////////////////////////////////////////////////////
template<typename _TyKey, typename _TyVal, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhashmapset
{
public:
	typedef	whhashset<_TyVal, _AllocationObj, _HashFunc>		VSET;
	typedef	whhash<_TyKey, VSET, _AllocationObj, _HashFunc>		KV;
protected:
	KV	m_kv;
public:
	// 添加一个
	bool	Add(const _TyKey & key, const _TyVal & val)
	{
		// 判断键是否存在
		VSET	*pSet;
		if( !m_kv.get(key, &pSet) )
		{
			// 加入键值
			if( !m_kv.putalloc(key, pSet) )
			{
				return	false;
			}
			// 清空原来的数据
			pSet->clear();
		}
		// 如果原来有值则返回false
		return	pSet->put(val);
	}
	// 删除key对应的所有记录
	bool	Remove(const _TyKey & key)
	{
		return	m_kv.erase(key);
	}
	// 删除key/val对
	bool	Remove(const _TyKey & key, const _TyVal & val)
	{
		// 判断键是否存在
		VSET	*pSet;
		if( !m_kv.get(key, &pSet) )
		{
			return	false;
		}
		bool	bVal = pSet->erase(val);
		if( pSet->size()==0 )
		{
			// 已经没有内容了，则删除key
			m_kv.erase(key);
		}
		return	bVal;
	}
	// 删除vect表示的多key/一val对
	void	Remove(const whvector<_TyKey> & vectkey, const _TyVal & val)
	{
		for(unsigned int i=0;i<vectkey.size();i++)
		{
			Remove(vectkey[i], val);
		}
	}
	// 清除所有
	void	Clear()
	{
		m_kv.clear();
	}
	// 检测key/val对是否存在
	bool	Has(const _TyKey & key, const _TyVal & val)
	{
		VSET	*pSet;
		if( !m_kv.get(key, &pSet) )
		{
			return	false;
		}
		return	pSet->has(val);
	}
	// 获取某个key对应的元素个数
	int		GetValNum(const _TyKey & key)
	{
		VSET	*pSet;
		if( !m_kv.get(key, &pSet) )
		{
			return	0;
		}
		return	pSet->size();
	}
	int		GetAllKeyValNum(whvector<_TyKey> &vectKey, whvector<int> &vectValNum)
	{
		GetAllKey(vectKey);
		vectValNum.resize(vectKey.size());
		for(size_t i=0;i<vectKey.size();i++)
		{
			vectValNum[i]	= GetValNum(vectKey[i]);
		}
		return	vectKey.size();
	}
	// 获得所有的key
	bool	GetAllKey(whvector<_TyKey> &vectKey)
	{
		m_kv.exporttovector(vectKey);
		return	true;
	}
	// 获得key对应的所有val
	bool	GetAllValByKey(const _TyKey & key, whvector<_TyVal> & vectval)
	{
		VSET	*pSet;
		if( !m_kv.get(key, &pSet) )
		{
			return	false;
		}
		return	pSet->exporttovector(vectval);
	}
	void	Reserve(int nSize)
	{
		m_kv.resizebucket(nSize);
	}
};
// 注：双向的就不要了。理论上是可以通过两个单向模拟出来的。

////////////////////////////////////////////////////////////////////
// 一个键存储多个可能重复的值
// 如果想浏览所有的key的value，则可以直接浏览m_kv，然后再对每个VVECT进行浏览
////////////////////////////////////////////////////////////////////
template<typename _TyKey, typename _TyVal, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhashstore
{
public:
	typedef	whvector<_TyVal, false, _AllocationObj>				VVECT;
	typedef	whhash<_TyKey, VVECT, _AllocationObj, _HashFunc>	KV;
	KV	m_kv;
public:
	// 添加一个
	_TyVal *	Add(const _TyKey & key)
	{
		// 判断键是否存在
		VVECT	*pVect;
		if( !m_kv.get(key, &pVect) )
		{
			// 加入键值
			if( !m_kv.putalloc(key, pVect) )
			{
				return	NULL;
			}
			// 清空原来的数据
			pVect->clear();
		}
		return	pVect->push_back();
	}
	bool	Add(const _TyKey & key, const _TyVal & val)
	{
		_TyVal	*pVal	= Add(key);
		if( !pVal )		return	false;
		*pVal			= val;
		return			true;
	}
	// 删除key对应的所有记录
	bool	Remove(const _TyKey & key)
	{
		return	m_kv.erase(key);
	}
	// 清除所有
	void	Clear()
	{
		m_kv.clear();
	}
	// 获得所有的key
	bool	GetAllKey(whvector<_TyKey> &vectkey)
	{
		m_kv.exporttovector(vectkey);
		return	true;
	}
	// 获得key对应的所有val
	VVECT *	GetAllValByKey(const _TyKey & key)
	{
		VVECT	*pVect;
		if( !m_kv.get(key, &pVect) )
		{
			return	NULL;
		}
		return	pVect;
	}
};

////////////////////////////////////////////////////////////////////
// 通过whhash的方法生成整数对整数的对应表
////////////////////////////////////////////////////////////////////
class	whhashidmapid		: public whhash<int, int>
{
public:
	// 如果查找不到则返回这个
	int	m_nDftVal;
public:
	whhashidmapid(int nDftVal=0)
	: m_nDftVal(nDftVal)
	{
	}
	inline int	GetMap(int nKey) const
	{
		int	nVal;
		if( get(nKey, nVal) )
		{
			return	nVal;
		}
		return	m_nDftVal;
	}
};
////////////////////////////////////////////////////////////////////
// id到string的映射
////////////////////////////////////////////////////////////////////
class	whhashidmapstrptr	: public whhash<int, whstrptr4hash>
{
public:
	// 如果查找不到则返回这个
	const char *	m_szDftVal;
public:
	whhashidmapstrptr()
	{
		m_szDftVal	= "(NULL)";
	}
	const char *	GetMap(int nKey) const
	{
		whstrptr4hash	*pstr;
		if( get(nKey, &pstr) )
		{
			return	pstr->GetPtr();
		}
		return	m_szDftVal;
	}
};
class	whhashidmapstr		: public whhash<int, whstr4hash>
{
public:
	// 如果查找不到则返回这个
	const char *	m_szDftVal;
public:
	whhashidmapstr()
	{
		m_szDftVal	= "(NULL)";
	}
	const char *	GetMap(int nKey) const
	{
		whstr4hash	*pstr;
		if( get(nKey, &pstr) )
		{
			return	pstr->GetPtr();
		}
		return	m_szDftVal;
	}
};
////////////////////////////////////////////////////////////////////
// 字串到字串的映射
////////////////////////////////////////////////////////////////////
class	whhashstrptrmapstrptr	: public whhash<whstrptr4hash, whstrptr4hash>
{
public:
	// 如果查找不到则返回这个
	const char *	m_szDftVal;
public:
	whhashstrptrmapstrptr()
	{
		m_szDftVal	= "(NULL)";
	}
	const char *	GetMap(const char *cszKey) const
	{
		whstrptr4hash	*pstr;
		if( get(cszKey, &pstr) )
		{
			return	pstr->GetPtr();
		}
		return	m_szDftVal;
	}
	// 从数组初始化(NULL字串表示结尾)
	int	InitFromArray(const char *astr[][2])
	{
		int	i=0;
		while( astr[i][0] )
		{
			put(astr[i][0], astr[i][1]);
			i	++;
		}
		return	0;
	}
};

// 利用vector存储的str哈希表
template<class _strinvector, class _hashfunc>
class	whstrinvectorhashset_base
{
protected:
	whvector<char>													m_vect;
	whhashset<_strinvector, whcmnallocationobj, _hashfunc>			m_set;
public:
	void	clear()
	{
		m_vect.clear();
		m_set.clear();
	}
	// 返回：	1	加入成功
	//			0	原来就有
	//			-1	加入失败
	int		Add(const char *cszStr)
	{
		if( Has(cszStr) )
		{
			return	0;
		}
		int	nOffset	= m_vect.size();
		m_vect.pushn_back(cszStr, strlen(cszStr)+1);
		_strinvector	str(&m_vect, nOffset);
		if( !m_set.put(str) )
		{
			return	-1;
		}
		return	1;
	}
	// 判断是否含有字串
	inline bool	Has(const char *cszStr)
	{
		return	m_set.has(cszStr);
	}
};
typedef	whstrinvectorhashset_base<whstrinvector4hash, _whstr_hashfunc>				whstrinvectorhashset;
typedef	whstrinvectorhashset_base<whstrinvector4hashNocase, _whstr_hashfuncNocase>	whstrinvectorhashsetNocase;

}		// EOF namespace n_whcmn

#endif	// EOF __WHHASH_H__
