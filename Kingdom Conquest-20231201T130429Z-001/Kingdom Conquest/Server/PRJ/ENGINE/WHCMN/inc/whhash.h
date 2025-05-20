// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whhash.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵�hashtable
//                �ο���cui��stlport��ʵ��
//                ���������治���Զ�������Ĺ��캯������Ϊ�����delete���غ��Ƕ��(��Ȼ�������)
//                ע�⣺��ʹ��ʹ�ã�hashtable���ǻ�ռ��һ���ĳ�ʼ�ڴ�
// Function list: _whcmn_hashfunc	��ͨ��hash�º���
//              : whhashbase		hash������
//              : whhash			hashtable ��/ֵ��Ӧ
//              : whhashset			ֻ������ʾһ�����Ƿ����
//              : whhashmapset		����ӳ������������������б� (һ����ӳ�������ظ���ֵ)
//              : whhashstore		һ�����洢��������ظ���ֵ
//              : whhash2			�򵥵�С��ģ˫������hashtableʵ��
//              : whhashidmapstr��whhashstrptrmapstrptr		id��string��ӳ��
//              : whhashidmapid		ͨ��whhash�ķ������������������Ķ�Ӧ��
//
// CreationDate : 2003-05-15
// ChangeLog    :
//                2004-01-16 ���뿼��whmem�ˣ������涼��new��delete��
//                2004-01-17 ��hash��hashset�ϲ�������󲿷ֵĴ�����ʵ������ͬ��
//                           ��whpairŲ���ˣ��������chunk������new/delete��
//                2005-09-26 ������hash�����Ĺ���
//                2006-07-04 ������put���ص�iterator��ppnodeָ�����һ����ʱ�����ĵ�ַ�Ĵ���
//                2007-06-20 �����˵���������ŵ�ͬһ��bucket�У�ɾ��bucket�����еķǵ�һ���ڶ�Ԫ��ʱ����������յĴ��󣨱�����Ѱ����Ƶ���ڱ������Ӻ�ɾ�����ߵ��µ��߶�ʧ����
//                2007-06-28 ������getorput�������������list����ͬһ�����Ķ�ֵ���롣
//                2007-07-11 ������whhash2::has����
//                2007-07-17 whhash2��iterator��UNIT_T & operator * ()������Ϊ��const����
//                2007-09-07 �ѹ����Ķ����ŵ�whhash_base.h�У�whhash2�ŵ����whhash2.h��
//                2007-09-10 ȡ����chunk�ķ���

// ע�⣺2005-01-27
// ������whstr4hash�ĺ��Դ�Сд������һЩ���ҡ���Ȼ��whstring.h���Ҷ�����m_bIC������ʵ����whstr4hash���Ǵ�Сд���еġ�
// ��������Ҫ��������һ�¡��������������õĻ����ȶ����Ȳ����ˡ�
//                2005-12-29 ��hashfunc�ж�ָ��Ĵ���ԭ����>>4����ȥ���ˣ���Ϊ�����С��������һ�����κη�1�����Լ��������ʡ�
//                2005-12-29 ������whhash2��
//                2006-02-16 ������g_prime_list�е�17��29����Ϊ����������ϵ���Ʒ���ܻ���hash������������Ʒ���ܻ�Ƚ��١�
//                2006-05-16 ��size_t -> unsigned int -> unsigned int
// ע�⣺2007-06-10
// ��hash����ܶ�̬���󣨼�resizebucket�����ã�������£�Ӧ�ñ����¼�������ڽ��ʹ�ã���Ϊbuckets�������ᵼ�¾ɵ������е�bucketidxʧЧ��
// ���ʵ��Ҫ��ôʹ�ã������ʼ�͵���resizebucket��hash��ĳߴ����õ��㹻��
// ע�⣺2007-06-28
// clear����ֻ��������еĶ�Ӧ��ϵ������ֻ�ǰ��ͷŵĽڵ����freenode�б��У���������ɾ�������������·���ĳЩ����֮�������е��ö���ĳ�ʼ����պ���
// clearclear������������������е�ֵ

#ifndef	__WHHASH_H__
#define	__WHHASH_H__

#include "whhash_base.h"
#include "whstring.h"
#include <assert.h>

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// hash����
////////////////////////////////////////////////////////////////////
template <typename _Key, typename _Node, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhashbase
{
public:											// publicΪ�˸�iterator��
	typedef	whfakeallocationobj<_AllocationObj>				MYFAO_T;
	typedef	whAND<whfakeallocationobj<_AllocationObj> >		MYAND_T;
	typedef	_Node			MYNODE;				// �����ҪΪ�˸�����ʹ��
	typedef	whvector<_Node *, false, _AllocationObj>		MYVECT_T;

	MYFAO_T					m_FAO;
	MYAND_T					m_AND;
	_HashFunc				_hash;
	unsigned int			_elementnum;
	MYVECT_T				_buckets;			// ��Ӧһ��hashֵ��Ͱ��ÿ��Ͱ�ﶼ��һ��_Node����
	bool					_bNeverResizeAgain;	// �Ƿ�����Ҳ����resize��
public:
	struct	stat_t
	{
		int		usedbucketnum;					// �ж�����bucket����
		int		maxnum, maxidx;
		int		minnum, minidx;
	};
	// ���iterator���Ա�����ɾ���ڵ�Ĳ�����ע�⣺��hash������֮������Ӿͻ�ʵЧ��
	// Ϊ�˼��Ͳ�ʵ��"--"������
	class	key_iterator
	{
	public:
		unsigned int	bucketidx;				// ����Լ������Ǹ�Ͱ
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
		// ��д����ᱨno postfix form of 'operator ++' found for type 'key_iterator'
		// ��ʵ������� x++�����������++x��
		inline key_iterator	operator ++ (int)
		{
			key_iterator	old(*this);
			operator ++ ();
			return			old;
		}
		// ��Ҫ���ڵ����ӵ�++��������֤�ҵ���һ��
		inline void	suregoodnext()
		{
			// �ҵ���һ���ǿյ�cur
			if( !cur )
			{
				while(!cur)
				{
					if( ++bucketidx==base->_buckets.size() )	// ����Ͳ����ǵ��˽�β������suregoodnext�������
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
			// <0��Ч
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
		// ���������Ƿ���ͬһ��bucket��
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
	// Ĭ��Ϊ��С��С(���������еĵ�һ��53)��n��ϣ���Ĵ�С
	// _M_next_size�������ֵӦ��>=��
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
	// ���(��������put_noresize)
	// ���ԭ��key�ʹ������޷����루������false��
	bool	put(const _Key &key, _Node *&pNode, unsigned int &nBucketIdx);
	bool	put_noresize(const _Key &key, _Node *&pNode, unsigned int &nBucketIdx);
	// ����iterator��put
	key_iterator	put(const _Key &key);
	// ��ȡָ����ֵ�ĵ�Ԫ
	bool	get(const _Key &key, _Node *&pNode) const;
	key_iterator	find(const _Key & key);
	// �ж�ָ����ֵ�Ƿ����
	bool	has(const _Key &key) const;
	// ��ȡָ����ֵ�ĵ�Ԫ��ͬʱɾ��֮
	bool	geterase(const _Key &key, _Node &Node);
	// �Ƴ�ָ����ֵ�ĵ�Ԫ
	bool	erase(const _Key &key);
	// ����iteratorɾ����ע�⣬����ڱ�������ʹ����Ҫ������
	bool	erase(key_iterator &it);
	// ͳ�����ݵķֲ���������ڼ����������Ƿ����
	bool	statistic(stat_t *pstat);
	// ������key������һ��vector��
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
	// ����ⲿ���Ե��ÿ�����һ��ʼ�ͽ�buckets�������Ŵ�һ���̶�
	// ע��resizebucket�����Ǹı�Ԫ�ظ���������bucket��������
	// _elementnum�������ǲ����ܱ�resize�ģ�ֻ��ͨ��put������
	void	resizebucket(unsigned int n, bool bNeverResizeAgain=false);
	// ������Ϣ����
	inline unsigned int	size() const		{ return _elementnum; }
	inline bool		empty() const		{ return size() == 0; }
	// ���Ʊ��hash���ݵ��Լ�
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
		// �����еĵ�Ԫָ��ȫ������һ��
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
			// �϶����ϲ���Ҫ��������hash�������ϲ���Ҫ����������Ա����õġ�
			assert(0);
			return;
		}
		_bNeverResizeAgain	= bNeverResizeAgain;

		// ����n��һ������old_n��
		n	= _M_next_size(n);
		// ��ԭ��������ȫ���Ƴ���tmp��
		MYVECT_T	tmp(n, (_Node*)(0), _buckets.GetAO());
		for(unsigned int i=0; i<old_n; ++i)
		{
			_Node* first	= _buckets[i];
			while(first)
			{
				unsigned int idx	= _hash(first->_key) % n;
				_buckets[i] = first->_next;
				// �ڵ����tmp
				first->_next	= tmp[idx];
				tmp[idx]	= first;
				// ����һ��
				first		= _buckets[i];
			}
			// ת�ƽ�����_buckets[i]ָ���
		}
		// �����ڴ�(ָ�뽻��)���ں�������ʱtmp���ͷŻ���ȥ���ڴ�ռ�
		_buckets.swap(tmp);
	}
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
void	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
copyfrom(whhashbase &other)
{
	if( this == &other )
	{
		// �Լ����ÿ������Լ���
		return;
	}
	// ����Լ�
	clear();
	// ����
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
			// �����ܵ�
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
		// �Ѿ����ˣ����������
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
	// ��ʱӦ����0��
	assert(_elementnum == 0);
	_elementnum = 0;
}
template <typename _Key, typename _Node, class _AllocationObj, class _HashFunc>
void	whhashbase<_Key, _Node, _AllocationObj, _HashFunc>::
clearclear()
{
	if( _elementnum==0 )
	{
		// �Ѿ����ˣ����������
		return;
	}
	for(unsigned int i=0; i<_buckets.size(); ++i)
	{
		_Node *	cur			= _buckets[i];
		while (cur != NULL)
		{
			_Node *	next	= cur->_next;
			cur->_val.clear();	// ���ֵ��ֵ���ͱ�����clear������
			FreeNode(cur);
			cur				= next;
		}
		_buckets[i]			= NULL;
	}
	// ��ʱӦ����0��
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

	// �ж��Ƿ�key�Ѿ�����
	for(_Node* cur = first; cur; cur=cur->_next) 
		if (cur->_key == key)
			return false;

	// �����ڵ㲢����nBucketIdxλ�õ�����
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
		// ɾ��һ�������ڵĵ����ӣ������Ǽ�¼���ϵĵ����ӣ�����hash������
		assert(0);
		return	false;
	}
	if( it.cur == pBucketNode )
	{
		// ���ǵ�һ����ֱ�ӻ�Ͳ�ĸ�ָ��
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
				// û�ҵ������Ҳ�Ƚ����
				assert(0);
				return	false;
			}
		}
		pCur->_next	= it.cur->_next;
	}

	// ����ͷ�node���ڴ�
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
// hashtable ��/ֵ��Ӧ
////////////////////////////////////////////////////////////////////
template <typename _Key, typename _Value, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhash	: public whhashbase<_Key, whhash_node<_Key, _Value>, _AllocationObj, _HashFunc>
{
private:
	typedef	whhashbase<_Key, whhash_node<_Key, _Value>, _AllocationObj, _HashFunc>	FATHERHASH;
public:
	whhash(unsigned int n = 0, _AllocationObj *pAO = NULL)						// Ĭ��Ϊ��С��С(���������еĵ�һ��53)
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
	// �Ȳ��ң�����о�ֱ�ӷ��أ�û�о�put
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
	// ����iterator��put
	inline kv_iterator	put(const _Key &key)
	{
		return	FATHERHASH::put(key);
	}
	// get����ò�������Ͳ��ı�*ppval����val��ֵ�����Կ���������Ԥ������һ����ʾ�����ֵ
	// ��ָ���ԭ���ǿ�����Ϊ�գ��Ͳ�������
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
	// ���������Ƚϳ��������ඨ��������
	// �Ƴ�����ֵΪval����Ŀ
	bool	eraseval(const _Value &val);
	// ������val������һ��vector��
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
				// ɾ��
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
// hashset ֻ������ʾһ�����Ƿ����
////////////////////////////////////////////////////////////////////
template <typename _Key, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhashset	: public whhashbase<_Key, whhashset_node<_Key>, _AllocationObj, _HashFunc>
{
private:
	typedef	whhashbase<_Key, whhashset_node<_Key>, _AllocationObj, _HashFunc>		FATHERHASH;
public:
	whhashset(unsigned int n = 0, _AllocationObj *pAO = NULL)	// Ĭ��Ϊ��С��С(���������еĵ�һ��53)
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
// ����ӳ������������������б� (һ����ӳ�������ظ���ֵ)
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
	// ���һ��
	bool	Add(const _TyKey & key, const _TyVal & val)
	{
		// �жϼ��Ƿ����
		VSET	*pSet;
		if( !m_kv.get(key, &pSet) )
		{
			// �����ֵ
			if( !m_kv.putalloc(key, pSet) )
			{
				return	false;
			}
			// ���ԭ��������
			pSet->clear();
		}
		// ���ԭ����ֵ�򷵻�false
		return	pSet->put(val);
	}
	// ɾ��key��Ӧ�����м�¼
	bool	Remove(const _TyKey & key)
	{
		return	m_kv.erase(key);
	}
	// ɾ��key/val��
	bool	Remove(const _TyKey & key, const _TyVal & val)
	{
		// �жϼ��Ƿ����
		VSET	*pSet;
		if( !m_kv.get(key, &pSet) )
		{
			return	false;
		}
		bool	bVal = pSet->erase(val);
		if( pSet->size()==0 )
		{
			// �Ѿ�û�������ˣ���ɾ��key
			m_kv.erase(key);
		}
		return	bVal;
	}
	// ɾ��vect��ʾ�Ķ�key/һval��
	void	Remove(const whvector<_TyKey> & vectkey, const _TyVal & val)
	{
		for(unsigned int i=0;i<vectkey.size();i++)
		{
			Remove(vectkey[i], val);
		}
	}
	// �������
	void	Clear()
	{
		m_kv.clear();
	}
	// ���key/val���Ƿ����
	bool	Has(const _TyKey & key, const _TyVal & val)
	{
		VSET	*pSet;
		if( !m_kv.get(key, &pSet) )
		{
			return	false;
		}
		return	pSet->has(val);
	}
	// ��ȡĳ��key��Ӧ��Ԫ�ظ���
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
	// ������е�key
	bool	GetAllKey(whvector<_TyKey> &vectKey)
	{
		m_kv.exporttovector(vectKey);
		return	true;
	}
	// ���key��Ӧ������val
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
// ע��˫��ľͲ�Ҫ�ˡ��������ǿ���ͨ����������ģ������ġ�

////////////////////////////////////////////////////////////////////
// һ�����洢��������ظ���ֵ
// �����������е�key��value�������ֱ�����m_kv��Ȼ���ٶ�ÿ��VVECT�������
////////////////////////////////////////////////////////////////////
template<typename _TyKey, typename _TyVal, class _AllocationObj=whcmnallocationobj, class _HashFunc = _whcmn_hashfunc>
class	whhashstore
{
public:
	typedef	whvector<_TyVal, false, _AllocationObj>				VVECT;
	typedef	whhash<_TyKey, VVECT, _AllocationObj, _HashFunc>	KV;
	KV	m_kv;
public:
	// ���һ��
	_TyVal *	Add(const _TyKey & key)
	{
		// �жϼ��Ƿ����
		VVECT	*pVect;
		if( !m_kv.get(key, &pVect) )
		{
			// �����ֵ
			if( !m_kv.putalloc(key, pVect) )
			{
				return	NULL;
			}
			// ���ԭ��������
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
	// ɾ��key��Ӧ�����м�¼
	bool	Remove(const _TyKey & key)
	{
		return	m_kv.erase(key);
	}
	// �������
	void	Clear()
	{
		m_kv.clear();
	}
	// ������е�key
	bool	GetAllKey(whvector<_TyKey> &vectkey)
	{
		m_kv.exporttovector(vectkey);
		return	true;
	}
	// ���key��Ӧ������val
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
// ͨ��whhash�ķ������������������Ķ�Ӧ��
////////////////////////////////////////////////////////////////////
class	whhashidmapid		: public whhash<int, int>
{
public:
	// ������Ҳ����򷵻����
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
// id��string��ӳ��
////////////////////////////////////////////////////////////////////
class	whhashidmapstrptr	: public whhash<int, whstrptr4hash>
{
public:
	// ������Ҳ����򷵻����
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
	// ������Ҳ����򷵻����
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
// �ִ����ִ���ӳ��
////////////////////////////////////////////////////////////////////
class	whhashstrptrmapstrptr	: public whhash<whstrptr4hash, whstrptr4hash>
{
public:
	// ������Ҳ����򷵻����
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
	// �������ʼ��(NULL�ִ���ʾ��β)
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

// ����vector�洢��str��ϣ��
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
	// ���أ�	1	����ɹ�
	//			0	ԭ������
	//			-1	����ʧ��
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
	// �ж��Ƿ����ִ�
	inline bool	Has(const char *cszStr)
	{
		return	m_set.has(cszStr);
	}
};
typedef	whstrinvectorhashset_base<whstrinvector4hash, _whstr_hashfunc>				whstrinvectorhashset;
typedef	whstrinvectorhashset_base<whstrinvector4hashNocase, _whstr_hashfuncNocase>	whstrinvectorhashsetNocase;

}		// EOF namespace n_whcmn

#endif	// EOF __WHHASH_H__
