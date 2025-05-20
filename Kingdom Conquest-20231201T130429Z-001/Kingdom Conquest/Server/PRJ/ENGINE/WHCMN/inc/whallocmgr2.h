// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whallocmgr2.h
// Creator      : Wei Hua (κ��)
// Comment      : �ڴ�����������������ǰ��whallocmgr�Ľӿ�ʵ�֡�ʹ�����µĻ���hash�ĳߴ��Ӧ����
//              : ��Ҫ��Ի���Ԥ�ȷ���Ĵ���ڴ棨�繲���ڴ棩��С���䡣
// << whallocmgr2ʹ��˵�� >>
// 1. ����һ��_AllocationObj���������Ķ�����ʵ����Alloc��Free�����Ķ����������������������ڴ档
//    �磺whallocator.h�е�whcmnallocationobj��whcmnallocationobjwithonemem��
//    ���ڷ�����: ��Ҫͨ��whcmnallocationobjwithonememһ��ʹ��һ����ڴ���ʹ�ã�nChunkSize���Ǹÿ���ڴ������ߴ磨һ����ܻ���100��M����
//    ���ڿͻ���: ���Ҳ��Ҫ�����Ľӿڣ������ʹ��whcmnallocationobj��Ȼ���nChunkSize���õıȽ�С������Ϊ�˼򵥿ͻ��˿���ֱ����ϵͳ��new��delete�Ϳ����ˡ�
// 2. ��ʼ��whallocmgr2���󣬵���Init������
// 3. ����ʹ�ã�����New��Delete�������ж���ķ�����ͷţ�
//    ����NewList���Է���һ���������Զ������������Ա����ӡ�ɾ�������ҡ���������������ڲ��ڴ�Ĺ�����������ɾ������ʹ��Delete�������ɣ�
//    ����Alloc��Free���л������ķ�����ͷš�ע���������Free�ͷŶ�����ô��������������Ͳ��ᱻ�����ˡ�
//
// << whallocmgr2::listʹ��˵�� >>
// 1. ʹ��whallocmgr2::NewList��������list����ɾ��list������ʹ��whallocmgr2::Delete������
// 2. ��ͨ������
//    ����ʹ��list��push_back/push_front/insertafter/insertbefore���������ݲ����б�
//    ����ʹ��find��������ָ��ֵ�ĵ�һ��ԭ�أ����������Ǳ�����ʽ��Ч�ʺܵͣ����Ƽ�����ʹ�ã���
//    ����ʹ��erase����ɾ��ָ��λ�õ�Ԫ�ء�
// 3. ��������
//    ����ʹ��begin/end/++iterator�����������
//    ����ʹ��rbegin/rend/--iterator���з������
// ���������ʹ�ÿ��Բο����ӳ���allocmgf2-tst.cpp
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
	// ������
	MYFAO_T	m_FAO;
public:
	int		m_nAdjustCount;									// ���Ϊ0��֤���ϴε�adjust����ȫ�����˵ģ��ϴ�ÿ����ģ��adjust����ʱ����Ҫ���������adjust���˼��������
	int		m_nUseCount;									// ���Ϊ0��֤�����е�ʹ���߶��Ѿ�ʹ������
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
		FATHERCLASS::SetAO(&m_FAO);							// ��Ϊ�ڴ�ı�ʱm_FAO�ĵ�ַʵ����Ҳ�ı���
	}
	inline _AllocationObj *	GetAO()
	{
		return	m_FAO.GetAO();
	}
	inline MYFAO_T &	GetFAO()
	{
		return	m_FAO;
	}
	// �������������������Ϊ�Լ�
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
	// ָ���Լ��ڲ�������ֻ�ڼ̳оɵĹ����ڴ��ʱ���ã�ע�⣺��Ҫ�Լ̳е��ڴ��еĸ��ֶ������κ�ɾ���򴴽������������ݸ�����������ˣ�
	// nOffset�����ɸ�
	template<typename _Ty>
	void	AdjustPtrAndInner(_Ty &ptr, int nOffset)
	{
		// �Ⱦ���ָ�뱾��
		wh_adjustaddrbyoffset(ptr, nOffset);
		// Ȼ��ѹ������Լ����������ö�������ڲ�����
		ptr->AdjustInnerPtr(this, nOffset);
	}
public:
	// �����Ķ���ģ��
	// list
	template<typename _Ty>
	class	list
	{
	public:
		typedef	whDList<_Ty>			DL_T;
		typedef	typename DL_T::node		node;	// ע�⣬��������дtypename�Ļ���VC7��gcc�º���������:P
	protected:
		friend	class whallocmgr2;
		whallocmgr2						*m_pMgr;
		DL_T							m_dl;
	public:
		// iterator����
		class	const_iterator;
		class	iterator
		{
		public:
			node	*pNode;				// ��Ҫ����ΪҪ�����ĵط�̫���ˡ����һ��еط���Ҫֱ�ӷ�����������ĵ�ַ��
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
			// ++����������������Ӳ���
			inline iterator	operator ++ (int)	// ��д����ᱨno postfix form of 'operator ++' found for type 'iterator'
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
			// --�������ڷ�������Ӳ���
			inline iterator	operator -- (int)	// ��д����ᱨno postfix form of 'operator --' found for type 'iterator'
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
		// const_iterator����
		class	const_iterator
		{
		public:
			node	*pNode;				// ��Ҫ����ΪҪ�����ĵط�̫���ˡ����һ��еط���Ҫֱ�ӷ�����������ĵ�ַ��
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
			// ++����������������Ӳ���
			inline const_iterator	operator ++ (int)	// ��д����ᱨno postfix form of 'operator ++' found for type 'const_iterator'
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
			// --�������ڷ�������Ӳ���
			inline const_iterator	operator -- (int)	// ��д����ᱨno postfix form of 'operator --' found for type 'const_iterator'
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
			// ɾ�����е�Ԫ
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
		// ע�⣺�����rxxx��������ѭ����Ҫ��--������������++����
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
		// ����һ����Ԫ
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
		void	pop_front()	// ���Ҫ��΢ʡһ�����
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
			// ���node���������б�
			pNode->insertbefore( _P.pNode );
			if( pIt )
			{
				pIt->pNode	= pNode;
			}
			return	pNode->data;
		}
		void	insertbefore(iterator _P, const _Ty& _X, iterator *pIt=NULL)
		{
			// ��_P֮ǰ����_X
			insertbefore(_P, pIt)	= _X;
		}
		_Ty &	insertafter(iterator _P, iterator *pIt=NULL)
		{
			node	*pNode;
			m_pMgr->New(pNode);
			assert(pNode);
			// ���node���������б�
			pNode->insertafter( _P.pNode );
			if( pIt )
			{
				pIt->pNode	= pNode;
			}
			return	pNode->data;
		}
		void	insertafter(iterator _P, const _Ty& _X, iterator *pIt=NULL)
		{
			// ��_P֮�����_X
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
		// ������һ����Ԫ��iterator����λ�ã�
		iterator	erase(const iterator &_P)
		{
			if( _P == end() )
			{
				return	end();
			}
			assert(size()>=1);
			node	*pNode	= _P.pNode;
			iterator	Next(pNode->next);
			// ��m_dl���뿪
			pNode->leave();
			// �ͷ�ָ��
			m_pMgr->Delete(pNode);
			// ������һ��
			return	Next;
		}
		// ɾ��ֵΪ����ĵ�һ��
		// �оͷ���true�����򷵻�false
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
		// ֻ��Ԫ�ش��б����Ƴ������ǲ�ɾ���ڴ棨�����Ҫ���ڴӹ����ڴ��лָ�����֮��ɾ���Ѿ��ɹ����������ʱ�õģ�
		void	fakeerase(node *pNode)
		{
			pNode->leave();
		}
		// ɾ�����е�Ԫ
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
			// �����µ��ڴ������
			SetMgr(pAM);
			// У��m_dl�ڲ�
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
			// ɾ�����е�Ԫ
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
			assert(nSize<0x7FFFFFFF);				// �ߴ粻��̫���
			// ����һ��û�����ݵ�Ԫ��
			// ����ռ䲻���˾���չ
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
					// ���һ�����Ӻܶ��ֱ�ӱ�ɸ�ֵ
					m_nCapacity	= nSize;
				}
				bAlloc		= true;
			}
			// ��Ϊ�������nSize==0������Ͳ���realloc��
			if( bAlloc && m_nCapacity )
			{
				m_Buf	= (_Ty *)m_pMgr->Realloc(m_Buf, m_nCapacity*sizeof(_Ty));
			}
			m_nSize	= nSize;
		}
		void	reserve(size_t nCap)
		{
			assert(nCap<0x7FFFFFFF);				// �ߴ粻��̫���
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
