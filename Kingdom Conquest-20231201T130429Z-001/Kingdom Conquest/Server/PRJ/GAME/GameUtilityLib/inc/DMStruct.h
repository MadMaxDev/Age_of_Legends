/////////////////////////////////////////////////
// DMStruct.h	:	V0010
// Writen by	:	Liu Gang
// V0010		:	Mar.16.2000
// V0011		:	July.28.2000
// V0012		:	May.23.2001
//					修改了静态链表的实现，因为有BUG，现在的结构与MFC的类似。
/////////////////////////////////////////////////
// 通用数据结构模板


#ifndef		__DMSTRUCT_H__
#define		__DMSTRUCT_H__
#include "DMtype.h"
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Container 包容器模板
// DataType	:	数据结构的定义
// nCount	:	该包容器的大小
template <class DataType, int nCount >
class CContainer
{
private:
	DataType	m_Data[nCount];		// 内容
	BOOL		m_bValid[nCount];	// 内容是否有效
	int			m_nCount;			// 成员计数
	DataType	m_Value;			// 初始值
	int			m_nMax;				// 最大ID

public:
	// 构造函数
	CContainer ();
	CContainer ( DataType Value );
	// 祈构函数
	~CContainer ();

	// 清除所有成员
	virtual void Clear();
	virtual void Clear( DataType Value );

	// 将成员加到指定索引处，如果该位置原来是空的，返回TRUE
	BOOL Add ( int nIndex, const DataType &Data );
	// 删除指定索引处的成员，如果该位置原来合法，返回TRUE
	BOOL Remove ( int nIndex );
	// 替换某个位置的内容，如果该位置有内容，则返回TRUE
	BOOL Set( int nIndex, const DataType &Data );
	// 得到指定索引处的成员，如果该位置合法，返回TRUE，否则仍然会得到数据内容
	BOOL Get ( int nIndex, DataType *pData );
	// 得到指定索引处的成员指针
	BOOL GetPtr ( int nIndex, DataType **ppData );
	// 查询成员，返回数组中的下标序号，-1为没有找到
	int Find ( const DataType &Data );
	// 得到数量
	int Count();
	// 得到最大数量
	int MaxCount();
	// 指定的位置是否空闲
	BOOL IsValid( int nIndex );
	// 结构内是否是空的
	BOOL IsEmpty();

	// 在空闲位置增加一个成员，如果成功，则返回下标
	int AddEx( const DataType &Data );
	// 删除具有该内容的成员，如果成功，则返回该成员原来的位置
	int RemoveEx( const DataType &Data );
	// 查找下一个空位置
	int FindRoom();
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Stack 堆栈模板
template <class DataType, int nCount>
class CStack : public CContainer <DataType, nCount>
{
	typedef CContainer <DataType, nCount> CContainerBase;

private:

	int	m_nTop;		// 栈顶指针，是下一个空位置

public:
	// 构造函数
	CStack ();
	CStack ( DataType Value );
	// 祈构函数
	~CStack ();

	// 清除所有成员
	virtual void Clear();
	virtual void Clear( DataType Value );

	// 压栈
	BOOL Push( const DataType &Data );
	// 出栈
	BOOL Pop( DataType *pData );
	// 得到栈顶信息
	BOOL Peek( DataType *pData );
	// 得到栈顶的数组下标
	int GetTopIndex();
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Queue 队列模板
template <class DataType, int nCount>
class CQueue : public CContainer <DataType, nCount>
{
	typedef CContainer <DataType, nCount> CContainerBase;

private:
	int m_nHead;	// 队列开始位置
	int m_nTail;	// 队列结束位置

public:
	// 构造函数
	CQueue ();
	CQueue ( DataType Value );
	// 祈构函数
	~CQueue ();

	// 清除所有成员
	virtual void Clear();
	virtual void Clear( DataType Value );

	// 进队
	BOOL In( const DataType &Data );
	// 出队
	BOOL Out( DataType *pData );
	// 得到队头信息
	BOOL PeekHead( DataType *pData );
	// 得到队尾信息
	BOOL PeekTail( DataType *pData );
	// 得到队头的数组下标
	int GetHeadIndex();
	// 得到队尾的数组下标
	int GetTailIndex();
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Static Link List 静态链表模板（双向循环）
template <class DataType, int nCount>
class CStaticLinkList : public CContainer <DataType, nCount>
{
	typedef CContainer <DataType, nCount> CContainerBase;

	int m_nHead;			// 指向头结点
	int m_nTail;			// 指向尾结点
	int m_nNext[nCount];	// 指向下一个结点，与每一个结点相配
	int m_nPrev[nCount];	// 指向上一个结点，与每一个结点相配

public:

	// 构造函数
	CStaticLinkList ();
	CStaticLinkList ( DataType Value );
	// 祈构函数
	~CStaticLinkList ();

	// 清除所有成员
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
//	CDMList	链表模版
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
	// 得到数量
	int GetCount() const;
	// 结构内是否是空的
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
	// 根据顺序查询成员，返回链表的下标，NULL为没有找到
	POSI FindIndex( int nIndex );
	// 根据取值查询成员，返回链表的下标，NULL为没有找到
	POSI Find( const DataType &Data, POSI after = NULL );
};

// BUG FIX	: Mar.28.2001
// 当进行模版嵌套时，必须重载该数据类（DataType）的构造函数和『＝』。
// 否则会出现严重BUG。
// 举例如下：
/*
class CMyData2
{
public:
	CDMList<CMyData> m_MyData;

	CMyData2();
	CMyData2(const CMyData2& md2);		// <== 必须

	CMyData2& operator=(const CMyData2& md2 );	// <== 必须
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
// 当单个参数的模版进行多层模版嵌套时，必须事先定义其参数的类型，
// 模版使用的原则：	两个『>』不能挨在一起
// 否则编译会失败
// 举例如下：
/*
template<class DataType>
class	TMP_A
{
};

typedef	TMP_A<int> TMP_C;		// <== 必须定义

void test()
{
	TMP_A<TMP_C> aa;			// <== 正确

	TMP_A<TMP_A<int>> aaa;		// <== 错误
}
*/

/////////////////////////////////////////////////


/////////////////////////////////////////////////
//
//	CDMPtrList	:	指针链表
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
//	CDMPtrList	:	指针链表
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
//	CDMPtrPool	:	内存分配后并不急于释放
//
/////////////////////////////////////////////////
template <class DataType>
class	CDMPtrPool	:	CDMPtrList
{
	int m_nMax;
	CDMPtrList m_ListReady;
public:
	CDMPtrPool();
	// 占用一个空位，没有则申请内存
	DataType* NewPtr( int nNum = 10 );
	// 释放一个空位，并不真正从内存中删除
	void DeletePtr( DataType* pData );

	// 释放所有空位，并不真正从内存中删除
	void DeleteAll();

	// 从内存中删除一定数量的空位
	void Free( int nNum = 10 );
	// 从内存中删除所有空位
	void FreeAll();
};
/////////////////////////////////////////////////


/////////////////////////////////////////////////
#include "DMStruct.inl"
/////////////////////////////////////////////////


/////////////////////////////////////////////////
#endif	// __DMSTRUCT_H__

