// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whlist.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵�����
// CreationDate : 2003-05-10
// ChangeLog    :
//                2004-01-16 �޸�list��node�����뷽ʽͨ��chunk����
//                2004-01-17 ����˷������������������Ϳ���ʹ�ù����ڴ���
//                2004-10-01 �޸���iterator�ĺ���++��--������ԭ�������塣�������:)
//                2005-12-28 ��whDList�ֳ���дһ���ļ�whDList.h

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
		// ����д�Ϳ��Ա�֤���۵��ö��ٴ�leave���ǰ�ȫ��
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
	// ���iterator���Ա�����ɾ���ڵ�Ĳ���
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
		inline iterator	operator ++ (int)	// ��д����ᱨno postfix form of 'operator ++' found for type 'iterator'
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
			// <0��Ч
			assert(n>=0);
			for(int i=0;i<n;i++)
			{
				pnode	= pnode->next;
			}
			return	*this;
		}
		inline iterator operator -- (int)	// ��д����ᱨno postfix form of 'operator --' found for type 'iterator'
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
	node	m_head, m_tail;				// ���õĵ�λ�б�
	node	m_availhead, m_availtail;	// ���õĵ�λ�б�
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

		// ���������б�����û��
		if( pnode == &m_availtail )
		{
			// û�У�����һ��
			pnode	= m_abc.alloc();
		}
		else
		{
			// �У��뿪���Զ���
			pnode->leave();
		}

		return	pnode;
	}
public:
	// ע���Ⱥ�˳��
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
		// ��_P֮ǰ����_X
		*insertbefore(_P)	= _X;
	}
	_Ty *	insertbefore(iterator _P)
	{
		// insertbefore�������ڿ��б���(before begin/end ������)
		node *	pnode;
		pnode	= getavailnode();

		// ���node���������б�
		pnode->insertbefore( _P.pnode );
		m_size	++;

		return	&pnode->data;
	}
	void	insertafter(iterator _P, const _Ty& _X)
	{
		// ��_P֮�����_X������Ҫע�⣬_P������end����������
		*insertafter(_P)	= _X;
	}
	_Ty *	insertafter(iterator _P)
	{
		// insertafter�������ڿ��б���
		assert(_P != end());
		node *	pnode;
		pnode	= getavailnode();

		// ���node���������б�
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

		// ���node���������б�
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

		// ���node���������б�
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
	// ɾ��ֵΪ����ĵ�һ��
	// �оͷ���true�����򷵻�false
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
		// ɾ�����нڵ㣬�������ڿ��ö���
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
		// ��m_abcȥdestroy����
		m_abc.destroy();
		resetht();
	}
	inline void		addreserve(int size)
	{
		// ���size������λ��
		for(int i=0; i<size; i++)
		{
			node	* pnode;
			pnode	= m_abc.alloc();
			pnode->insertafter(&m_availhead);
		}
	}
};

// ������ű����ǷǸ�����
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
	// ����0��nTotal-1����������
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
	// �黹���
	inline void	returnidx(int nIdx)
	{
		push_front(nIdx);
	}
	// ��ÿ��õ����(����-1��ʾû��)
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

// �ڲ�ά����һ��available���е��б��������붨���ڴ棩
// ע�⣺��һ��дӦ�ÿ����ֽڶ�������⣬���ڶ���1�ֽڵģ�Ӧ�û���4�ֽڶ���ĺ�Щ!!!!
class	whalist
{
public:
	enum
	{
		INVALIDIDX	= -1,
	};
	struct	HDR_T
	{
		size_t	nDataUnitSize;			// ÿ�����ݿ�ĳߴ�
		size_t	nMaxNum;				// �����÷�������
		size_t	nCurNum;				// ��ǰ�Ѿ����������
		int		AvailableHdr;			// ���Է����ͷ (Available��һ����������)
	};
	#pragma pack(1)
	struct	UNIT_T
	{
		// ��һ�������š�(INVALIDIDX��ʾû��)
		bool	bAllocated;
		int		Next;					// ��������Ѿ���������ϲ�ʹ��ʱ���ڱ��Ƶ��������õ�
										// ����û�з���֮ǰ�����γ��ڲ���available����
		char	Data[1];				// ���ݲ��ֿ�ͷ
	};
	#pragma pack()
protected:
	HDR_T	*m_pHdr;					// ��Ϣͷ
	char 	*m_pUnit;					// ���ݿ�ʼ
	size_t	m_nUnitSize;				// ÿ��UNIT_T��ĳߴ�
public:
	// ���������ҪnNum��Ԫ���ܹ���Ҫ���ڴ������Ƕ���bytes
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
	// ��ʼ����pBuf�ĳߴ�Ӧ��Ӧ�÷�����ˣ�����С��CalcTotalSize�õ��ĳߴ�
	int		Init(void *pBuf, size_t nDataUnitSize, size_t nMaxNum);
	// �����е��ڴ�����̳й���
	int		Inherit(void *pBuf);
	// �Ӿ��ڴ�̳й�����ͬʱ�����ڴ����Ѿ���չ����ڴ�(��ԭ�����ݵĺ����������ڴ�)��ϣ������������ΪnNewMaxNum
	int		InheritEnlarge(void *pBuf, size_t nNewMaxNum, size_t nNewDataUnitSize);
	// ����һ�£��ָ���ʲôҲû�������״̬
	int		Reset();
	// Alloc֮�����Ҫ��¼����idx������ͨ��Free�ͷ�
	// ����Alloc�����ĵ�ԪĬ��GetNextOf��õ�INVALIDIDX
	int		Alloc();
	// �ͷ�һ����
	int		Free(int nIdx);
	// �ͷ�һ������(��Ȼ������Ҫ��֤�����������ȷ��Ч��)
	int		FreeChain(int nHeadIdx);
	// ���ָ����Ŷ�Ӧ�����ݿ�ʼָ��
	void*	GetDataUnitPtr(int nIdx) const;
	// �ж�һ�����Ƿ����Ѿ������
	bool	IsUnitAllocated(int nIdx) const;
	// ����Ѿ�����ĸ���
	size_t	GetNum() const;
	// ��û���������ĸ���
	size_t	GetAvailNum() const;
	// (ע�⣬������������һ���Ƕ�����õĵ�Ԫʹ�õģ�������ͨ��������������Ŀ��Լ���������)
	// ���һ������һ��
	int		GetNextOf(int nIdx) const;
	// ����nIdx����һ��ΪnNext
	bool	SetNextOf(int nIdx, int nNext);
protected:
	UNIT_T*	GetUnitPtr(int nIdx) const;
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHLIST_H__
