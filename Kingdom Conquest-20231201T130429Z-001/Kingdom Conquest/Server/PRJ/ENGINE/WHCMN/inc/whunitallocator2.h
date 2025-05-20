// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : whunitallocator2.h
// Creator     : Wei Hua (κ��)
// Comment     : �µĶ����޵Ķ�̬Ԫ�ط������������ԭ����whunitallocatorFixed��ʡ�ڴ�
//             : �տ�ʼֻ����һ��ָ������������Ԫ�صķ���ͨ��whchunkallocationobjonesize������ֻ����Ҫ��ʱ��ŷ���һ���ڴ档
// ChangeLOG   : 2006-03-26 �޸���Init�Ĳ����Ⱥ�˳��

#ifndef	__WHUNITALLOCATOR2_H__
#define	__WHUNITALLOCATOR2_H__

#include "./whcmn_def.h"
#include "./whDList.h"
#include "./whallocator2.h"

namespace	n_whcmn
{

// ��Թ̶�����buffer�Ĵ�ID�ķ�����
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
		int			nID;						// ��Ԫ��ID
		void	clear()
		{
			nID		= -1;
		}
	};
	typedef	whDList<UNITHDR_T>		DL_T;
	typedef	typename DL_T::node		node;		// ���node������UNIT�����ݲ���
	int				m_nMax;						// ����Ԫ������
	int				m_nIdxToAlloc;				// ���õ����������������С��m_nMax�˾Ͳ����ˡ�
	int				m_nRand;					// �����������ID��ǰ�벿�֡�
	node			**m_paIdx;					// ������
												// ���ʵ�ʷ����˵�Ԫ�����Ӧ��ָ���ָ����Ӧ��λ�á�
												// �����Ԫ��Free�ˣ�����ָ����Ȼָ���λ�ã���Ԫnode�ڲ����ID���IDX�Ա��Ϊ���ͷ�
	int				m_DATASIZE;					// ���ݲ��ֳߴ�
	DL_T			m_dlAvail;					// ���ͷŵ�Ԫ���б�
	DL_T			m_dlAlloc;					// �Ѿ����������Ԫ���б�
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
	// _CHUNKSIZEΪÿ�μ��з���ĵ�Ԫ����
	// _SIZEΪ���ݲ��ֵĳ���
	int	Init(_AllocationObj *pAllocationObj, int nMax, int _CHUNKSIZE, int _SIZE)
	{
		if( nMax>=MAXNUM )
		{
			// ���ܷ��䳬��65535����Ԫ
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
		// ���������б�����û��
		if( m_dlAvail.size()>0 )
		{
			// �У��Ƴ���
			pNode	= m_dlAvail.begin();
			pNode->leave();
		}
		else
		{
			if( m_nIdxToAlloc>=m_nMax )
			{
				// û�пɷ������
				return	NULL;
			}
			// û�������µ�
			pNode	= (node *)m_CAOS.Alloc();
			pNode->reset();
			pNode->data.clear();
			pNode->data.nID	= m_nIdxToAlloc;
			// ��Idx�����
			m_paIdx[m_nIdxToAlloc]	= pNode;
			m_nIdxToAlloc++;
		}
		assert( pNode!=NULL );
		pNode->data.nID	= MakeID(pNode->data.nID, MakeRand());
		*pnID			= pNode->data.nID;
		m_dlAlloc.AddToHead(pNode);
		// �������ݲ���
		return	pNode+1;
	}
	// ����ID���з��䣨�����Alloc����Ӧ�ò�����ͬһ����������ʹ�ã�
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
			// ԭ�����У�˵����avail������
			if(pNode->data.nID >= MAXNUM )
			{
				// ˵��ԭ���Ѿ���������
				// ����������ܻ���֣��Ͳ���assert(0)��
				return	NULL;
			}
			// ��avail�б����˳�
			pNode->leave();
		}
		else
		{
			// ԭ��û�У������µ�
			pNode	= (node *)m_CAOS.Alloc();
			pNode->reset();
			pNode->data.clear();
			// ��Idx�����
			m_paIdx[nIdx]	= pNode;
			m_nIdxToAlloc++;
		}
		// ����alloc�б�
		m_dlAlloc.AddToHead(pNode);
		// ����IDΪָ��ֵ
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
	void	FreeNode(node *pNode)	// ���������ò�Ҫ����
	{
		assert(pNode->data.nID>0xFFFF);
		// ���IDΪIDX��Ȼ�����avail�б�
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
				// �ͷ�
				FreeNode(pNode);
				return	0;
			}
		}
		// �޷��ͷ�
		return	-2;
	}
	inline void	FreeByPtr(void *ptr)
	{
		// ���node��ָ��
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
			// ԭ��ֱ��д�ɡ�FATHERIT::operator *();���Ͳ���
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
