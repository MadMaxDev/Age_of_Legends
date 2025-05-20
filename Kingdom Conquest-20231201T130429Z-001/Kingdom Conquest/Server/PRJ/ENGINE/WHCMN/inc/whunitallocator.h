// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : whunitallocator.h
// Creator     : Wei Hua (κ��)
// Comment     : ���ڼ򵥵Ķ�̬���������ֻ����Ԫ������
//               ����ʹ�ñ�׼stl��Ϊ�˿���ʹ�ö����Ӧ�Ĺ��캯������
//
//               WHCMN_USECYCLE_ID��������壬���ͷŵ�ID���ɵ���ѡ���е����
//               Ҳ����˵һ����˵��ʹ��һ�α��ͷź�Ҫ�ȺܾòŻ����±����롣
//               ���������Իر�һЩ���󣬱�����ЩID��������ɾ���ȵȡ�������Ŀǰ
//               ������WHCMN_USECYCLE_ID���������Լ�ʱ��¶����
//
// CreationDate: 2003-05-22
// ChangeLog   : 
// 2003-06-13  : inline _Ty *	getptr(int nIdx) ԭ����const��׺���ͱ�����returnʱǿ��ת����
// 2003-12-26  : ����ID�ķ�Χ���ã�setrange()�������������Կ������ɵ�ID�ĳ��ȡ�
//               ����ע�⣺0��RANDMAX�����ᱻȡ�������Ӳ����ڴﵽRANDMAXʱ�ͻ���1��
// 2004-01-17  : ΪFixed����˷������������������Ϳ���ʹ�ù����ڴ��ˡ����ҷ������ṩNew������������ЩAllocatorҲ���Է���������ˡ�
//               ���������ֻ�����ʼ�������˹��ڣ���Allocʱ�����ᱻ���õģ�������Ҫ�ֶ�clear
// 2005-05-26  : whsmpunitallocatorΪ���ݲ����ٷ�����һ���ֽڣ�������д���ݵ�ʱ����ܵ��¸����˺����bUsed������
//               �����ѩ���ָ���·��ֵġ�
// 2005-08-05  : ȡ����whunitallocatorFixed��GetByRefIdx��MultiGet��������iterator��
//               ȡ�������ڲ�ԭ����listavail����ʹ��whDListȡ��
//               ȡ����setmax

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
// �̶������Ŀ�ĵķ�����
// ���ĺô��ǵ�һ�������ָ�������Զʹ��
// ����_Ty�����й��캯����������������Alloc��ʱ����ã�������Ԥ�ȷ����ʱ����õ�
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
		int			idx;				// �������е����
		node		dlnode;
		_Ty			data;
		MyTy() : id(-1)
		{
		}
	};
	// ������
	MYFAO_T				m_FAO;
	MYAND_T				m_AND;
	MyTy *				m_pMyUnit;		// ����ͷָ��
	_Ty					m_foo;			// �����Ҳ��������
	int					m_nMax;			// ���ɷ���ĵ�Ԫ����
	int					m_nAvailNum;	// ���õ�����
	DL_T				m_dlFree;		// ���õĶ����б�
	DL_T				m_dlAllocated;	// �Ѿ�����Ķ����б�
	DL_T				m_dlReserved;	// �����Ȳ�����Ķ����б�
	int					RANDMAX;		// ����������ֵ�ͻ���
	int					RANDSHIFT;		// �������Ҫ������λ��ô���ٺ�idx���
	int					REALIDXMASK;	// ����idx���ֵ�����
	int					m_nRand;		// �������
	node				*m_pCurNode;	// ����BeginGet��GetNext
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
	// 0�ǵ�һ�����Ժ��������
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
	// Ϊ�˿�Ͳ��������
	inline _Ty *	getptr(int nID) const
	{
		MyTy	*pMyUnit;
		if( getrealidx(nID, pMyUnit) >= 0 )
		{
			return	&pMyUnit->data;
		}
		return	NULL;
	}
	// Ϊ�˿�Ͳ��������
	inline _Ty &	get(int nID)
	{
		MyTy	*pMyUnit;
		if( getrealidx(nID, pMyUnit) >= 0 )
		{
			return	pMyUnit->data;
		}
		return	m_foo;
	}
	// ���һ����Ԫ�Ƿ�ʹ��
	inline bool		IsUnitUsed(int nID) const
	{
		MyTy	*pMyUnit;
		if( getrealidx(nID, pMyUnit) >= 0 )
		{
			return	true;
		}
		return	false;
	}
	// ����Ѿ�����ĵ�Ԫ����
	inline int		size() const
	{
		return	m_nMax - m_nAvailNum;
	}
	// ��û����õĵ�Ԫ�ռ���
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
		// ����ǰ������ñ��ͷŵľ���ʹ��
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
		if(pMyUnit->dlnode.isinlist(&m_dlReserved))	// 2007-11-27 ����assert�ˣ���Ϊ���ܻ��ϲ���õ�ʱ�����Ѿ���unreserve����
		{
			pMyUnit->dlnode.leave();
#ifdef	WHCMN_USECYCLE_ID
			m_dlFree.AddToTail(&pMyUnit->dlnode);
#else
			// ����ǰ������ñ��ͷŵľ���ʹ��
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
	// ���õײ������
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
		// ��������������size()�᷵��nMax
		m_nMax	= 0;
		return	0;
	}
	int		MakeNewID(int nIdx)
	{
		if( (++m_nRand) >= RANDMAX )
		{
			// ������žͲ���Ϊ0�ˡ��������ǰ���<0�Ƿ����жϰ�
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
	// �ж�һ��ID�Ƿ����AllocUnitByID
	bool			CanAllocUnitByID(int nID)
	{
		int		nIdx;
		nIdx	= REALIDXMASK & nID;
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// ��ų���
			return	false;
		}
		if( m_pMyUnit[nIdx].id >=0 )
		{
			// �Ѿ�������
			return	false;
		}
		return		true;
	}
	// ����Ԥ��֪����ID�ŷ���һ����Ԫ����������ڽṹ��ͬ�ĸ�����������ͬ��ID
	_Ty *			AllocUnitByID(int nID, int *pnOldID=NULL)
	{
		int		nIdx;
		nIdx	= REALIDXMASK & nID;
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// ��ų���
			return	NULL;
		}
		MyTy	*pMyUnit	= &m_pMyUnit[nIdx];
		if( pMyUnit->id >=0 )
		{
			// �Ѿ�������
			if( pnOldID )
			{
				*pnOldID	= pMyUnit->id;
			}
			return	NULL;
		}
		// ����ID
		pMyUnit->id		= nID;
		FreeToAllocated(pMyUnit);
		return	&pMyUnit->data;
	}
	// �ж�һ��ID�Ƿ����AllocUnitByIdx
	bool			CanAllocUnitByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// ��ų���
			return	false;
		}
		if( m_pMyUnit[nIdx].id >=0 )
		{
			// �Ѿ�������
			return	false;
		}
		return		true;
	}
	// ����Ԥ��֪����Idx�ŷ���һ����Ԫ��������ڴ�������ž�������ĵط�
	_Ty *			AllocUnitByIdx(int nIdx, int *pnNewID, int *pnOldID=NULL)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// ��ų���
			return	NULL;
		}
		MyTy	*pMyUnit	= &m_pMyUnit[nIdx];
		if( pMyUnit->id >=0 )
		{
			// �Ѿ�������
			if( pnOldID )
			{
				*pnOldID	= pMyUnit->id;
			}
			return	NULL;
		}
		// ����ID
		*pnNewID = pMyUnit->id = MakeNewID(nIdx);
		FreeToAllocated(pMyUnit);
		return	&pMyUnit->data;
	}
	// �ͷ�һ����Ԫ
	int		FreeUnit(int nID)
	{
		MyTy	*pMyUnit;
		int		nIdx;
		if( (nIdx=getrealidx(nID, pMyUnit))<0 )
		{
			// �Ѿ��ͷ���(��ų����Ҳ���)
			return	-1;
		}
		AllocatedToFree(pMyUnit);
		return	0;
	}
	// ��������ͷ�һ����Ԫ
	int		FreeUnitByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// ��ų���
			return	-1;
		}
		AllocatedToFree(&m_pMyUnit[nIdx]);
		return	0;
	}
	// ����ָ���ͷŵ�Ԫ(�����Ͳ��ں�pUnit�������Ƿ��Ѿ�������ˡ������FreeUnit���ܻ��õ�pUnit�д洢��ID)
	int		FreeUnitByPtr(_Ty *pUnit)
	{
		// ���pUnit��Ӧ��MyTy
		AllocatedToFree(GetMyTyPtr(pUnit));
		return	0;
	}
	// ������ű���һ��Free�ĵ�Ԫ
	int		ReserveUnitByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// ��ų���
			return	-1;
		}
		FreeToReserved(&m_pMyUnit[nIdx]);
		return	0;
	}
	// ����ָ�뱣��һ��Free�ĵ�Ԫ
	int		ReserveUnitByPtr(_Ty *pUnit)
	{
		FreeToReserved(GetMyTyPtr(pUnit));
		return	0;
	}
	// �������ȡ������һ����Ԫ
	int		UnReserveUnitByIdx(int nIdx)
	{
		if( nIdx<0 || nIdx>=m_nMax )
		{
			// ��ų���
			return	-1;
		}
		ReservedToFree(&m_pMyUnit[nIdx]);
		return	0;
	}
	// ����ָ��ȡ������һ����Ԫ
	int		UnReserveUnitByPtr(_Ty *pUnit)
	{
		ReservedToFree(GetMyTyPtr(pUnit));
		return	0;
	}
	// �ѱ�����Ԫ���·���Free����
	void		clear()
	{
		if( !m_pMyUnit )
		{
			// �Ѿ���release��
			return;
		}
		int		i;
		MyTy	*pMyUnit = &m_pMyUnit[0];
		for(i=0;i<m_nMax;i++)
		{
			pMyUnit->id		= -1;
			pMyUnit->idx	= i;
			// Ϊ�˱���leaveһ��
			pMyUnit->dlnode.leave();
			pMyUnit->dlnode.data	= pMyUnit;
			// ���ȷ����Ҫ��˳����ӵ�
			m_dlFree.AddToTail(&pMyUnit->dlnode);
			pMyUnit	++;
		}
		m_nAvailNum	= m_nMax;
	}
	// ��ʼ˳���ȡ
	inline void		BeginGet()
	{
		m_pCurNode	= m_dlAllocated.begin();
	}
	// ��ȡ��һ��
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
	// �����±�õ�����ָ��
	_Ty *	GetByIdx(int nIdx, int *pnID=0)
	{
		if( nIdx<0 || nIdx>m_nMax )	return	NULL;
		MyTy	*pMyUnit = &m_pMyUnit[nIdx];
		if( pMyUnit->id < 0 )		return NULL;
		if( pnID )					*pnID = pMyUnit->id;
		return	&pMyUnit->data;
	}
};

// ��������Ԫ�ص�clear����(�����ȻҪ������ԭ�ض���clear����)
template < typename _Ty, class _AllocationObj >
void	whunitallocatorFixed_AllClear(whunitallocatorFixed<_Ty, _AllocationObj> &uaf)
{
	// ע�������дtypename����linux�±�����warning: implicit typename is deprecated ...
	for(typename whunitallocatorFixed<_Ty, _AllocationObj>::iterator it=uaf.begin(); it!=uaf.end(); ++it)
	{
		(*it).clear();
	}
}

////////////////////////////////////////////////////////////////////
// ��������
////////////////////////////////////////////////////////////////////
template <typename _Ty>
class	whunitarray
{
protected:
	int		m_nSize;					// Ԫ������
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
// ��̶����Ȼ��������
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
// ����chunk�Ĺ̶����ȵ�Ԫ�ķ���
////////////////////////////////////////////////////////////////////
class	whsmpunitallocator
{
public:
	// ����
	#pragma pack(1)
	struct	UNIT_T
	{
		bool				bUsed;
		char				data[1];
	};
	#pragma pack()
public:
	// ��Ա
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
	// ����ڲ�
	inline UNIT_T *	getunit(int nIdx)
	{
		if( nIdx<0 || nIdx>=(int)m_chunkUnit.size() )
		{
			return	NULL;
		}
		return	(UNIT_T *)m_chunkUnit[nIdx];
	}
	// ���ָ����Ŷ�Ӧ�ĵ�Ԫָ��(�����nIdx���磬����)
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
	// ���һ����Ԫ�Ƿ�ʹ��(�ϲ㱣֤�����Ч)
	inline bool		IsUnitUsed(int nIdx) const
	{
		return	((UNIT_T *)m_chunkUnit[nIdx])->bUsed;
	}
	// ������ĳߴ�
	inline int		maxsize() const
	{
		return	m_chunkUnit.size();
	}
	// ����Ѿ�����ĵ�Ԫ����
	inline int		size() const
	{
		return	m_chunkUnit.size() - m_listAvail.size();
	}
public:
	// ����һ����Ԫ�����ط�������
	int		AllocUnit(char *&pBuf);
	// �ͷ�һ����Ԫ
	int		FreeUnit(int nIdx);
	// ���(�ͷ�)���е�Ԫ
	void	clear();
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHUNITALLOCATOR_H__
