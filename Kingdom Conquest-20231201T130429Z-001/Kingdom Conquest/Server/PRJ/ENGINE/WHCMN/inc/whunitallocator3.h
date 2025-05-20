// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : whunitallocator3.h
// Creator     : Wei Hua (κ��)
// Comment     : �µĶ����޵Ķ�̬Ԫ�ط������������ʵ��ķ�������
//             : �տ�ʼֻ����һ��ָ������������Ԫ�صķ���ͨ������������������ԭ������Ӧ�ķ����������������Ϳ���ʹ�ù����ڴ��ˡ�
//             : ǿ�ҽ��飺���������Ҫ���ڹ����ڴ��У�����ò�Ҫ���Լ��ڲ���ʹ��ϵͳ���������ڴ棬��ò�Ҫ���麯��!!!!
// ChangeLOG   : 2007-09-29 ��whunitallocator2�޸Ķ�����ȡ��ԭ����chunk����������Ϊ��̨�ṩ�ķ�������ܸ�����
//             : 2007-11-06 ������whunitallocatorFixed3��iteratorֱ�ӴӸ���̳У��������캯������Ҳʹ�ø��࣬���²�ͬ����(_Ty)��whunitallocatorFixed3::iterator���Ի��ิ�ƣ����������������д���˶��󣬱�����ͨ�������������ڶ����ڲ����ݻ��ҳ���

#ifndef	__WHUNITALLOCATOR3_H__
#define	__WHUNITALLOCATOR3_H__

#include "./whcmn_def.h"
#include "./whDList.h"
#include "./whallocator2.h"

namespace	n_whcmn
{

// ��Թ̶�����buffer�Ĵ�ID�ķ�����
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
		int			nID;						// ��Ԫ��ID
		int			nOldID;						// �ϴε�ID
		void	clear()
		{
			nID		= -1;
			nOldID	= -1;
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
	DL_T			m_dlReserved;				// �����Ȳ�����Ķ����б�
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
	// _SIZEΪ���ݲ��ֵĳ���
	int	Init(_AllocationObj *pAllocationObj, int nMax, int _SIZE)
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
		// �ͷ���������
		if( m_paIdx == NULL )	// ���ָ�������������Ƿ��Ѿ��ͷŹ���
		{
			return	0;
		}
		// �ͷŸ����б��еģ����е�node
		_ReleaseNodeInDL(m_dlAlloc);
		_ReleaseNodeInDL(m_dlAvail);
		_ReleaseNodeInDL(m_dlReserved);
		// �ͷ��ܱ�
		m_pAllocationObj->Free(m_paIdx);
		m_paIdx	= NULL;
		return	0;
	}
	void *	Alloc(int *pnID)
	{
		*pnID	= -1;
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
			pNode	= (node *)m_pAllocationObj->Alloc(m_DATASIZE);
			pNode->reset();
			pNode->data.clear();
			pNode->data.nID	= m_nIdxToAlloc;
			// ��Idx�����
			m_paIdx[m_nIdxToAlloc]	= pNode;
			m_nIdxToAlloc++;
		}
		assert( pNode!=NULL );
		pNode->data.nID	= MakeID(pNode->data.nID, MakeRand());
		pNode->data.nOldID	= pNode->data.nID;
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
			pNode	= (node *)m_pAllocationObj->Alloc(m_DATASIZE);
			pNode->reset();
			pNode->data.clear();
			// ��Idx�����
			m_paIdx[nIdx]	= pNode;
			m_nIdxToAlloc++;
		}
		// ����alloc�б�
		m_dlAlloc.AddToHead(pNode);
		// ����IDΪָ��ֵ
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
			// ��ų���
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
		node	*pNode	= GetNodeByID(nID);
		if( pNode )
		{
			// �ͷ�
			FreeNode(pNode);
			return	0;
		}
		// �޷��ͷ�
		return	-2;
	}
	inline void	FreeByPtr(void *ptr)
	{
		// ���node��ָ��
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
	// ������ű���һ��Free�ĵ�Ԫ
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
	// ����IDȡ������һ����Ԫ
	int		UnReserveByID(int nID)
	{
		node	*pNode	= GetNodeByIdx(GetIdxByID(nID));
		if( !pNode )
		{
			return	-1;
		}
		if( pNode->data.nOldID != nID )
		{
			// ˵������unreserved��
			return	0;
		}
		UnReserveNode(pNode);
		return	0;
	}

	inline int	size() const			// ����Ѿ�����ĸ���
	{
		return	m_dlAlloc.size();
	}
	inline int	getsizeleft() const		// ��û����Է���ĸ�����������reserve�ģ�
	{
		return	(m_nMax-m_nIdxToAlloc) + m_dlAvail.size();
	}
	// �����ڲ�ָ�룬����ڼ̳й����ڴ淢��ָ��ƫ��ʱ��Ҫ
	void	AdjustInnerPtr(_AllocationObj *pAllocationObj, int nOffset)
	{
		// �����������Ҫ
		m_pAllocationObj	= pAllocationObj;
		// ������
		if( m_paIdx )
		{
			wh_adjustaddrbyoffset(m_paIdx, nOffset);
			for(int i=0;i<m_nMax;i++)
			{
				wh_adjustaddrbyoffset(m_paIdx[i], nOffset);	// ���ΪNULL��ָ���ǲ��ᱻ������
			}
		}
		// ���������ڲ�
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
			// ԭ��ֱ��д�ɡ�FATHERIT::operator *();���Ͳ���
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
		inline iterator	operator ++ (int)	// ��д����ᱨno postfix form of 'operator ++' found for type 'iterator'
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
	void	clear()	// �ͷ����еĶ���
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
