/////////////////////////////////////////////////
// DMStruct.h	:	V0010
// Writen by	:	Liu Gang
// V0010		:	Mar.16.2000
// V0011		:	July.28.2000
// V0012		:	May.23.2001
//					�޸��˾�̬�����ʵ�֣���Ϊ��BUG�����ڵĽṹ��MFC�����ơ�
/////////////////////////////////////////////////
// ͨ�����ݽṹģ��


#ifndef		__DMSTRUCT_H__
#define		__DMSTRUCT_H__
#include "DMtype.h"
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Container ������ģ��
// DataType	:	���ݽṹ�Ķ���
// nCount	:	�ð������Ĵ�С
template <class DataType, int nCount >
class CContainer
{
private:
	DataType	m_Data[nCount];		// ����
	BOOL		m_bValid[nCount];	// �����Ƿ���Ч
	int			m_nCount;			// ��Ա����
	DataType	m_Value;			// ��ʼֵ
	int			m_nMax;				// ���ID

public:
	// ���캯��
	CContainer ();
	CContainer ( DataType Value );
	// ������
	~CContainer ();

	// ������г�Ա
	virtual void Clear();
	virtual void Clear( DataType Value );

	// ����Ա�ӵ�ָ���������������λ��ԭ���ǿյģ�����TRUE
	BOOL Add ( int nIndex, const DataType &Data );
	// ɾ��ָ���������ĳ�Ա�������λ��ԭ���Ϸ�������TRUE
	BOOL Remove ( int nIndex );
	// �滻ĳ��λ�õ����ݣ������λ�������ݣ��򷵻�TRUE
	BOOL Set( int nIndex, const DataType &Data );
	// �õ�ָ���������ĳ�Ա�������λ�úϷ�������TRUE��������Ȼ��õ���������
	BOOL Get ( int nIndex, DataType *pData );
	// �õ�ָ���������ĳ�Աָ��
	BOOL GetPtr ( int nIndex, DataType **ppData );
	// ��ѯ��Ա�����������е��±���ţ�-1Ϊû���ҵ�
	int Find ( const DataType &Data );
	// �õ�����
	int Count();
	// �õ��������
	int MaxCount();
	// ָ����λ���Ƿ����
	BOOL IsValid( int nIndex );
	// �ṹ���Ƿ��ǿյ�
	BOOL IsEmpty();

	// �ڿ���λ������һ����Ա������ɹ����򷵻��±�
	int AddEx( const DataType &Data );
	// ɾ�����и����ݵĳ�Ա������ɹ����򷵻ظó�Աԭ����λ��
	int RemoveEx( const DataType &Data );
	// ������һ����λ��
	int FindRoom();
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Stack ��ջģ��
template <class DataType, int nCount>
class CStack : public CContainer <DataType, nCount>
{
	typedef CContainer <DataType, nCount> CContainerBase;

private:

	int	m_nTop;		// ջ��ָ�룬����һ����λ��

public:
	// ���캯��
	CStack ();
	CStack ( DataType Value );
	// ������
	~CStack ();

	// ������г�Ա
	virtual void Clear();
	virtual void Clear( DataType Value );

	// ѹջ
	BOOL Push( const DataType &Data );
	// ��ջ
	BOOL Pop( DataType *pData );
	// �õ�ջ����Ϣ
	BOOL Peek( DataType *pData );
	// �õ�ջ���������±�
	int GetTopIndex();
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Queue ����ģ��
template <class DataType, int nCount>
class CQueue : public CContainer <DataType, nCount>
{
	typedef CContainer <DataType, nCount> CContainerBase;

private:
	int m_nHead;	// ���п�ʼλ��
	int m_nTail;	// ���н���λ��

public:
	// ���캯��
	CQueue ();
	CQueue ( DataType Value );
	// ������
	~CQueue ();

	// ������г�Ա
	virtual void Clear();
	virtual void Clear( DataType Value );

	// ����
	BOOL In( const DataType &Data );
	// ����
	BOOL Out( DataType *pData );
	// �õ���ͷ��Ϣ
	BOOL PeekHead( DataType *pData );
	// �õ���β��Ϣ
	BOOL PeekTail( DataType *pData );
	// �õ���ͷ�������±�
	int GetHeadIndex();
	// �õ���β�������±�
	int GetTailIndex();
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Static Link List ��̬����ģ�壨˫��ѭ����
template <class DataType, int nCount>
class CStaticLinkList : public CContainer <DataType, nCount>
{
	typedef CContainer <DataType, nCount> CContainerBase;

	int m_nHead;			// ָ��ͷ���
	int m_nTail;			// ָ��β���
	int m_nNext[nCount];	// ָ����һ����㣬��ÿһ���������
	int m_nPrev[nCount];	// ָ����һ����㣬��ÿһ���������

public:

	// ���캯��
	CStaticLinkList ();
	CStaticLinkList ( DataType Value );
	// ������
	~CStaticLinkList ();

	// ������г�Ա
	virtual void Clear();
	virtual void Clear( DataType Value );

	// head/tail access
	DataType GetHead();
	DataType GetTail();

	// Operations
	DataType RemoveHead();
	DataType RemoveTail();
	int AddHead( const DataType &Data );
	int AddTail( const DataType &Data );
	void RemoveAll();

	// Iteration
	int GetHeadPosition();
	int GetTailPosition();
	DataType GetNext( int &nPos );
	DataType GetPrev( int &nPos );

	// Modification
	DataType GetAt( int nPos ) const;
	DataType& GetAt( int nPos );
	void SetAt( int nPos, const DataType &Data );
	void RemoveAt( int nPos );

	// Insertion
	int InsertBefore( int nPos, const DataType &Data );
	int InsertAfter( int nPos, const DataType &Data );

	// find by index
	// returns :	Position
	int FindIndex( int nIndex );
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
//
//	CDMList	����ģ��
//
/////////////////////////////////////////////////
typedef void* POSI;

template <class DataType>
class	CDMNode
{
public:
	CDMNode<DataType>* pNext;
	CDMNode<DataType>* pPrev;
	DataType data;

	CDMNode();
	//~CDMNode();
	//CDMNode<DataType>& operator=( const CDMNode& node );
};

template <class DataType>
class	CDMList
{
protected:
	int m_nCount;

	CDMNode<DataType>* m_pNodeHead;
	CDMNode<DataType>* m_pNodeTail;

	DataType m_FreeData;

	CDMNode<DataType>* NewNode( CDMNode<DataType>*pPrev, CDMNode<DataType>*pNext );
	void FreeNode( CDMNode<DataType>* pNode );

public:
	CDMList();
	~CDMList();

	// count of elements
	// �õ�����
	int GetCount() const;
	// �ṹ���Ƿ��ǿյ�
	BOOL IsEmpty() const;

	// head&tail
	DataType& GetHead();
	DataType GetHead() const;
	DataType& GetTail();
	DataType GetTail() const;

	// iteration
	POSI GetHeadPosition() const;
	POSI GetTailPosition() const;
	DataType& GetNext( POSI &Pos );
	DataType GetNext( POSI &Pos ) const;
	DataType& GetPrev( POSI &Pos );
	DataType GetPrev( POSI &Pos ) const;

	// Modification
	DataType & GetAt( POSI Pos );
	DataType GetAt( POSI Pos ) const;
	void SetAt( POSI Pos, const DataType &Data );

	// Operations
	void RemoveAll();
	void RemoveAllData();

	POSI AddHead( const DataType &Data );
	POSI AddTail( const DataType &Data );
	void AddHead( CDMList<DataType>* pList );
	void AddTail( CDMList<DataType>* pList );
	DataType& RemoveHead();
	DataType& RemoveTail();

	POSI InsertBefore( POSI Pos, const DataType &Data );
	POSI InsertAfter( POSI Pos, const DataType &Data );
	void RemoveAt( POSI Pos );


	// slow operations
	// ����˳���ѯ��Ա������������±꣬NULLΪû���ҵ�
	POSI FindIndex( int nIndex );
	// ����ȡֵ��ѯ��Ա������������±꣬NULLΪû���ҵ�
	POSI Find( const DataType &Data, POSI after = NULL );
};

// BUG FIX	: Mar.28.2001
// ������ģ��Ƕ��ʱ���������ظ������ࣨDataType���Ĺ��캯���͡�������
// ������������BUG��
// �������£�
/*
class CMyData2
{
public:
	CDMList<CMyData> m_MyData;

	CMyData2();
	CMyData2(const CMyData2& md2);		// <== ����

	CMyData2& operator=(const CMyData2& md2 );	// <== ����
};
CMyData2::CMyData2()
{
}
CMyData2::CMyData2(const CMyData2& md2)
{
	*this = md2;
}
CMyData2& CMyData2::operator=(const CMyData2& md2 )
{
	m_MyData.RemoveAll();
	for( POSI pos = md2.m_MyData.GetHeadPosition(); pos!=NULL; )
	{
		CMyData md = md2.m_MyData.GetNext( pos );
		m_MyData.AddTail( md );
	}
	return *this;
}
*/

// BUG FIX	: Apr.6.2001
// ������������ģ����ж��ģ��Ƕ��ʱ���������ȶ�������������ͣ�
// ģ��ʹ�õ�ԭ��	������>�����ܰ���һ��
// ��������ʧ��
// �������£�
/*
template<class DataType>
class	TMP_A
{
};

typedef	TMP_A<int> TMP_C;		// <== ���붨��

void test()
{
	TMP_A<TMP_C> aa;			// <== ��ȷ

	TMP_A<TMP_A<int>> aaa;		// <== ����
}
*/

/////////////////////////////////////////////////


/////////////////////////////////////////////////
//
//	CDMPtrList	:	ָ������
//
/////////////////////////////////////////////////
struct CDMPlex     // warning variable length structure
{
	CDMPlex* pNext;
	void* data() { return this+1; }

	static CDMPlex* Create(CDMPlex*& head, UINT nMax, UINT cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions

	void FreeDataChain();       // free this one and links
};
/////////////////////////////////////////////////
//
//	CDMPtrList	:	ָ������
//
/////////////////////////////////////////////////
class CDMPtrList
{

protected:
	struct CDMPNode
	{
		CDMPNode* pNext;
		CDMPNode* pPrev;
		void* data;
		int   idx;
	};
public:

// Construction
	CDMPtrList(int nBlockSize = 10);

// Attributes (head and tail)
	// count of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	void*& GetHead();
	void* GetHead() const;
	void*& GetTail();
	void* GetTail() const;

// Operations
	// get head or tail (and remove it) - don't call on empty list!
	void* RemoveHead();
	void* RemoveTail();

	// add before head or after tail
	POSI AddHead(void* newElement,int   idx = -1);
	POSI AddTail(void* newElement,int   idx = -1);


	// add another list of elements before head or after tail
	void AddHead(CDMPtrList* pNewList);
	void AddTail(CDMPtrList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSI GetHeadPosition() const;
	POSI GetTailPosition() const;
	void*& GetNext(POSI& rPosition); // return *Position++
	void* GetNext(POSI& rPosition) const; // return *Position++
	void*& GetPrev(POSI& rPosition); // return *Position--
	void* GetPrev(POSI& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	void*& GetAt(POSI position);
	void* GetAt(POSI position) const;
	void SetAt(POSI pos, void* newElement);

	void RemoveAt(POSI position);

	// inserting before or after a given position
	POSI InsertBefore(POSI position, void* newElement);
	POSI InsertAfter(POSI position, void* newElement);


	POSI Sort( BOOL booBig ) const;
	// helper functions (note: O(n) speed)
	POSI Find(void* searchValue, POSI startAfter = NULL) const;
						// defaults to starting at the HEAD
						// return NULL if not found
	POSI FindIndex(int nIndex) const;
						// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CDMPNode* m_pNodeHead;
	CDMPNode* m_pNodeTail;
	int m_nCount;
	CDMPNode* m_pNodeFree;
	struct CDMPlex* m_pBlocks;
	int m_nBlockSize;

	CDMPNode* NewNode(CDMPNode*, CDMPNode*);
	void FreeNode(CDMPNode*);

public:
	~CDMPtrList();
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
//
//	CDMPtrPool	:	�ڴ����󲢲������ͷ�
//
/////////////////////////////////////////////////
template <class DataType>
class	CDMPtrPool	:	CDMPtrList
{
	int m_nMax;
	CDMPtrList m_ListReady;
public:
	CDMPtrPool();
	// ռ��һ����λ��û���������ڴ�
	DataType* NewPtr( int nNum = 10 );
	// �ͷ�һ����λ�������������ڴ���ɾ��
	void DeletePtr( DataType* pData );

	// �ͷ����п�λ�������������ڴ���ɾ��
	void DeleteAll();

	// ���ڴ���ɾ��һ�������Ŀ�λ
	void Free( int nNum = 10 );
	// ���ڴ���ɾ�����п�λ
	void FreeAll();
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
#include "DMStruct.inl"
/////////////////////////////////////////////////


/////////////////////////////////////////////////
#endif	// __DMSTRUCT_H__

