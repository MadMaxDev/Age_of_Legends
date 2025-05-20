/////////////////////////////////////////////////
// DMStruct.inl	:	V0010
// Writen by	:	Liu Gang
// V0010		:	Mar.16.2000
// V0011		:	July.28.2000
// V0012		:	May.23.2001
//					修改了静态链表的实现，因为有BUG，现在的结构与MFC的类似。
/////////////////////////////////////////////////
// 通用数据结构模板

#include <assert.h>		// assert()
#include <stdio.h>
#include "DMStruct.h"

#ifdef WIN32
#define OutputDebugString_T ::OutputDebugString
#else
#define OutputDebugString_T ::printf
#endif

/////////////////////////////////////////////////
// Container 包容器模板
// DataType	:	数据结构的定义
// nCount	:	该包容器的大小

// 构造函数
template <class DataType, int nCount>
CContainer<DataType, nCount>::CContainer ()
{
	assert( nCount > 0 );
	Clear();
}

template <class DataType, int nCount>
CContainer<DataType, nCount>::CContainer ( DataType Value )
{
	assert( nCount > 0 );
	Clear( Value );
}
// 祈构函数
template <class DataType, int nCount>
CContainer<DataType, nCount>::~CContainer ()
{
	Clear();
}

// 清除所有成员
template <class DataType, int nCount>
void CContainer<DataType, nCount>::Clear()
{
	for ( int i=0; i<nCount; i++ )
	{
		m_bValid[i] = FALSE;
		m_Data[i] = m_Value;
	}
	m_nCount = 0;
	m_nMax = 0;
}

template <class DataType, int nCount>
void CContainer<DataType, nCount>::Clear( DataType Value )
{
	m_Value = Value;
	Clear();
}

// 将成员加到指定索引处，如果该位置原来是空的，返回TRUE
template <class DataType, int nCount>
BOOL CContainer<DataType, nCount>::Add ( int nIndex, const DataType &Data )
{
	assert( nIndex >= 0 && nIndex < nCount );
	if( m_bValid[nIndex] == TRUE )
		return FALSE;
	m_Data[nIndex] = Data;
	m_bValid[nIndex] = TRUE;
	m_nCount++;
	assert( m_nCount <= nCount );
	return TRUE;
}

// 删除指定索引处的成员，如果该位置原来合法，返回TRUE
template <class DataType, int nCount>
BOOL CContainer<DataType, nCount>::Remove ( int nIndex )
{
	assert( nIndex >= 0 && nIndex < nCount );
	if( m_bValid[nIndex] == FALSE )
		return FALSE;
	m_Data[nIndex] = m_Value;
	m_bValid[nIndex] = FALSE;
	m_nCount--;
	assert( m_nCount >= 0 );
	return TRUE;
}

// 替换某个位置的内容，如果该位置有内容，则返回TRUE
template <class DataType, int nCount>
BOOL CContainer<DataType, nCount>::Set( int nIndex, const DataType &Data )
{
	assert( nIndex >= 0 && nIndex < nCount );
	BOOL ret = FALSE;
	if( m_bValid[nIndex] == TRUE )
		ret = TRUE;
	m_Data[nIndex] = Data;
	m_bValid[nIndex] = TRUE;
	if( ret == FALSE )
	{
		m_nCount++;
		assert( m_nCount <= nCount );
	}
	return ret;
}
// 得到指定索引处的成员，如果该位置合法，返回TRUE，否则仍然会得到数据内容
template <class DataType, int nCount>
BOOL CContainer<DataType, nCount>::Get ( int nIndex, DataType *pData )
{
	assert( nIndex >= 0 && nIndex < nCount );
	if( m_bValid[nIndex] == FALSE )
		return FALSE;
	*pData = m_Data[nIndex];
	return TRUE;
}

// 得到指定索引处的成员指针
template <class DataType, int nCount>
BOOL CContainer<DataType, nCount>::GetPtr ( int nIndex, DataType **ppData )
{
	assert( nIndex >= 0 && nIndex < nCount );
	if( m_bValid[nIndex] == FALSE )
		return FALSE;
	*ppData = &m_Data[nIndex];
	return TRUE;
}

// 查询成员，返回数组中的下标序号，-1为没有找到
template <class DataType, int nCount>
int CContainer<DataType, nCount>::Find ( const DataType &Data )
{
	for ( int i=0; i<nCount; i++ )
	{
		if( m_bValid[i] == TRUE && m_Data[i] == Data )
		{
			return i;
		}
	}
	return -1;
}

// 得到数量
template <class DataType, int nCount>
int CContainer<DataType, nCount>::Count()
{
	return m_nCount;
}
// 得到最大数量
template <class DataType, int nCount>
int CContainer<DataType, nCount>::MaxCount()
{
	return nCount;
}

// 指定的位置是否空闲
template <class DataType, int nCount>
BOOL CContainer<DataType, nCount>::IsValid( int nIndex )
{
	assert( nIndex >= 0 && nIndex < nCount );
	return m_bValid[nIndex];
}

// 结构内是否是空的
template <class DataType, int nCount>
BOOL CContainer<DataType, nCount>::IsEmpty()
{
	return (m_nCount == 0);
}

// 在空闲位置增加一个成员，如果成功，则返回下标
template <class DataType, int nCount>
int CContainer<DataType, nCount>::AddEx( const DataType &Data )
{
	int ret = -1;
	int nFind = -1;
	nFind = FindRoom();
	if( nFind < 0 )
		return -1;
	m_nMax = (nFind+1)%nCount;
	ret = Add( nFind, Data );
	if( ret == FALSE )
		ret = -1;
	else
		ret = nFind;
	return ret;
}

// 删除具有该内容的成员，如果成功，则返回该成员原来的位置
template <class DataType, int nCount>
int CContainer<DataType, nCount>::RemoveEx( const DataType &Data )
{
	int ret = -1;
	for( int i=0; i<nCount; i++ )
	{
		// 得到指定索引处的成员，如果该位置合法，返回TRUE，否则仍然会得到数据内容
		DataType* pdata;
		if( GetPtr( i, &pdata ) == TRUE )
		{
			if( *pdata == Data )
			{
				Remove( i );
				ret = i;
				break;
			}
		}
	}
	return ret;
}

// 查找下一个空位置
template <class DataType, int nCount>
int CContainer<DataType, nCount>::FindRoom()
{
	int nFind = -1;
	for( int i=m_nMax; i<nCount; i++ )
	{
		// 得到指定索引处的成员，如果该位置合法，返回TRUE，否则仍然会得到数据内容
		if( IsValid( i ) == FALSE )
		{
			nFind = i;
			break;
		}
	}
	if( nFind < 0 )
	{
		for( int i=0; i<m_nMax; i++ )
		{
			// 得到指定索引处的成员，如果该位置合法，返回TRUE，否则仍然会得到数据内容
			if( IsValid( i ) == FALSE )
			{
				nFind = i;
				break;
			}
		}
	}
	if( nFind < 0 )
		return -1;
	return nFind;
}
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Stack 堆栈模板
// 构造函数
template <class DataType, int nCount>
CStack<DataType, nCount>::CStack ()
{
	Clear();
}
template <class DataType, int nCount>
CStack<DataType, nCount>::CStack ( DataType Value )
{
	Clear( Value );
}
// 祈构函数
template <class DataType, int nCount>
CStack<DataType, nCount>::~CStack ()
{
	Clear();
}

// 清除所有成员
template <class DataType, int nCount>
void CStack<DataType, nCount>::Clear()
{
	m_nTop = -1;
	CContainer<DataType, nCount>::Clear();
}

template <class DataType, int nCount>
void CStack<DataType, nCount>::Clear( DataType Value )
{
	m_nTop = -1;
	CContainer<DataType, nCount>::Clear( Value );
}

// 压栈
template <class DataType, int nCount>
BOOL CStack<DataType, nCount>::Push( const DataType &Data )
{
	char msg[256];
	if( CContainerBase::Count() == nCount )
	{
		assert( m_nTop == nCount-1 );
#ifdef	_DEBUG
		sprintf( msg, "CStack::Push() : Stack is overflow! Stack size is <%d>\n", CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return FALSE;
	}
	m_nTop++;
	assert( m_nTop >= 0 );
	BOOL ret = Add( m_nTop, Data );
	assert( ret == TRUE );
	return TRUE;
}

// 出栈
template <class DataType, int nCount>
BOOL CStack<DataType, nCount>::Pop( DataType *pData )
{
	char msg[256];
	if( CContainerBase::Count() == 0 )
	{
#ifdef	_DEBUG
		assert( m_nTop == -1 );
		sprintf( msg, "CStack::Push() : Stack is empty! Stack size is <%d>\n", CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return FALSE;
	}
	assert( m_nTop >= 0 );
	BOOL ret = Get( m_nTop, pData );
	assert( ret == TRUE );
	ret = CContainerBase::Remove( m_nTop );
	m_nTop--;
	assert( ret == TRUE );
	return TRUE;
}

// 得到栈顶信息
template <class DataType, int nCount>
BOOL CStack<DataType, nCount>::Peek( DataType *pData )
{
	char msg[256];
	if( CContainerBase::Count() == 0 )
	{
#ifdef	_DEBUG
		assert( m_nTop == -1 );
		sprintf( msg, "CStack::Peek() : Stack is empty! Stack size is <%d>\n", CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return FALSE;
	}
	return Get( m_nTop, pData );
}

// 得到栈顶的数组下标
template <class DataType, int nCount>
int CStack<DataType, nCount>::GetTopIndex()
{
	return m_nTop;	
}
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Queue 队列模板
// 构造函数
template <class DataType, int nCount>
CQueue<DataType, nCount>::CQueue ()
{
	Clear();
}
template <class DataType, int nCount>
CQueue<DataType, nCount>::CQueue ( DataType Value )
{
	Clear( Value );
}
// 祈构函数
template <class DataType, int nCount>
CQueue<DataType, nCount>::~CQueue ()
{
	Clear();
}

// 清除所有成员
template <class DataType, int nCount>
void CQueue<DataType, nCount>::Clear()
{
	m_nHead = 0;
	m_nTail = -1;
	CContainer<DataType, nCount>::Clear();
}

template <class DataType, int nCount>
void CQueue<DataType, nCount>::Clear( DataType Value )
{
	m_nHead = 0;
	m_nTail = -1;
	CContainer<DataType, nCount>::Clear( Value );
}

// 进队
template <class DataType, int nCount>
BOOL CQueue<DataType, nCount>::In( const DataType &Data )
{
	char msg[256];
	if( CContainerBase::Count() == nCount )
	{
#ifdef	_DEBUG
		assert( m_nHead == (m_nTail+1)%nCount );
		sprintf( msg, "CQueue::In() : Queue is overflow! Queue Info<%d,%d,%d>\n", m_nHead, m_nTail, CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return FALSE;
	}
	m_nTail = (m_nTail+1)%nCount;
	assert( m_nTail >= 0 );
	BOOL ret = Add( m_nTail, Data );
	assert( ret == TRUE );
	assert( nCount > 0 );
	return TRUE;
}

// 出队
template <class DataType, int nCount>
BOOL CQueue<DataType, nCount>::Out( DataType *pData )
{
	char msg[256];
	if( CContainerBase::Count() == 0 )
	{
#ifdef	_DEBUG
		assert( m_nHead == (m_nTail+1)%nCount );
		sprintf( msg, "CQueue::Out() : Queue is empty! Queue Info<%d,%d,%d>\n", m_nHead, m_nTail, CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return FALSE;
	}
	BOOL ret = Get( m_nHead, pData );
	assert( ret == TRUE );
	ret = CContainerBase::Remove( m_nHead );
	assert( ret == TRUE );
	assert( nCount > 0 );
	m_nHead = (m_nHead+1)%nCount;
	return TRUE;
}

// 得到队头信息
template <class DataType, int nCount>
BOOL CQueue<DataType, nCount>::PeekHead( DataType *pData )
{
	char msg[256];
	if( CContainerBase::Count() == 0 )
	{
#ifdef	_DEBUG
		assert( m_nHead == (m_nTail+1)%nCount );
		sprintf( msg, "CQueue::PeekHead() : Queue is empty! Queue Info<%d,%d,%d>\n", m_nHead, m_nTail, CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return FALSE;
	}
	return Get( m_nHead, pData );
}

// 得到队尾信息
template <class DataType, int nCount>
BOOL CQueue<DataType, nCount>::PeekTail( DataType *pData )
{
	char msg[256];
	if( CContainerBase::Count() == 0 )
	{
#ifdef	_DEBUG
		assert( m_nHead == (m_nTail+1)%nCount );
		sprintf( msg, "CQueue::PeekTail() : Queue is empty! Queue Info<%d,%d,%d>\n", m_nHead, m_nTail, CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return FALSE;
	}
	return Get( m_nTail, pData );
}

// 得到队头的数组下标
template <class DataType, int nCount>
int CQueue<DataType, nCount>::GetHeadIndex()
{
	return m_nHead;
}

// 得到队尾的数组下标
template <class DataType, int nCount>
int CQueue<DataType, nCount>::GetTailIndex()
{
	return m_nTail;
}
/////////////////////////////////////////////////


/////////////////////////////////////////////////
// Static Link List 静态链表模板（双向循环）
// 构造函数
template <class DataType, int nCount>
CStaticLinkList<DataType, nCount>::	CStaticLinkList ()
{
	Clear();
}

template <class DataType, int nCount>
CStaticLinkList<DataType, nCount>::	CStaticLinkList ( DataType Value )
{
	Clear( Value );
}

// 祈构函数
template <class DataType, int nCount>
CStaticLinkList<DataType, nCount>::	~CStaticLinkList ()
{
	Clear();
}

// 清除所有成员
template <class DataType, int nCount>
void CStaticLinkList<DataType, nCount>::Clear()
{
	m_nHead = -1;			// 指向头结点
	m_nTail = -1;			// 指向尾结点
	for ( int i=0; i<nCount; i++ )
	{
		m_nNext[i] = -1;	// 指向下一个结点，与每一个结点相配
		m_nPrev[i] = -1;	// 指向上一个结点，与每一个结点相配
	}
	CContainer<DataType, nCount>::Clear();
}

template <class DataType, int nCount>
void CStaticLinkList<DataType, nCount>::Clear( DataType Value )
{
	m_nHead = -1;			// 指向头结点
	m_nTail = -1;			// 指向尾结点
	for ( int i=0; i<nCount; i++ )
	{
		m_nNext[i] = -1;	// 指向下一个结点，与每一个结点相配
		m_nPrev[i] = -1;	// 指向上一个结点，与每一个结点相配
	}
	CContainer<DataType, nCount>::Clear( Value );
}

// head/tail access
template <class DataType, int nCount>
DataType CStaticLinkList<DataType, nCount>::GetHead()
{
	DataType Data;
	BOOL ret = Get( GetHeadPosition(), &Data );
	assert( ret == TRUE );
	return Data;
}
template <class DataType, int nCount>
DataType CStaticLinkList<DataType, nCount>::GetTail()
{
	DataType Data;
	BOOL ret = Get( GetTailPosition(), &Data );
	assert( ret == TRUE );
	return Data;
}
	// Operations
template <class DataType, int nCount>
DataType CStaticLinkList<DataType, nCount>::RemoveHead()
{
	DataType Data;
	Data = GetAt( GetHeadPosition() );
	RemoveAt( GetHeadPosition() );
	return Data;
}
template <class DataType, int nCount>
DataType CStaticLinkList<DataType, nCount>::RemoveTail()
{
	DataType Data;
	Data = GetAt( GetTailPosition() );
	RemoveAt( GetTailPosition() );
	return Data;
}
template <class DataType, int nCount>
int CStaticLinkList<DataType, nCount>::AddHead( const DataType &Data )
{
	return InsertBefore( GetHeadPosition(), Data );
}
template <class DataType, int nCount>
int CStaticLinkList<DataType, nCount>::AddTail( const DataType &Data )
{
	return InsertAfter( GetTailPosition(), Data );
}
template <class DataType, int nCount>
void CStaticLinkList<DataType, nCount>::RemoveAll()
{
	while( !CContainerBase::IsEmpty() )
	{
		RemoveHead();
	}
}

	// Iteration
template <class DataType, int nCount>
int CStaticLinkList<DataType, nCount>::GetHeadPosition()
{
	return m_nHead;
}
template <class DataType, int nCount>
int CStaticLinkList<DataType, nCount>::GetTailPosition()
{
	return m_nTail;
}
template <class DataType, int nCount>
DataType CStaticLinkList<DataType, nCount>::GetNext( int &nPos )
{
	assert( nPos >= 0 && nPos < nCount );
	DataType Data;
	Data = GetAt( nPos );
	nPos = m_nNext[nPos];
	return Data;
}
template <class DataType, int nCount>
DataType CStaticLinkList<DataType, nCount>::GetPrev( int &nPos )
{
	assert( nPos >= 0 && nPos < nCount );
	DataType Data;
	Data = GetAt( nPos );
	nPos = m_nPrev[nPos];
	return Data;
}
// Modification
template <class DataType, int nCount>
DataType CStaticLinkList<DataType, nCount>::GetAt( int nPos ) const
{
	assert( nPos >= 0 && nPos < nCount );
	DataType Data;
	BOOL ret = Get( nPos, &Data );
	assert( ret == TRUE );
	return Data;
}
template <class DataType, int nCount>
DataType& CStaticLinkList<DataType, nCount>::GetAt( int nPos )
{
	assert( nPos >= 0 && nPos < nCount );
	DataType* pData = NULL;
	BOOL ret = GetPtr( nPos, &pData );
	assert( ret == TRUE );
	return *pData;
}
template <class DataType, int nCount>
void CStaticLinkList<DataType, nCount>::SetAt( int nPos, const DataType &Data )
{
	assert( nPos >= 0 && nPos < nCount );
	DataType *pData;
	GetPtr( nPos, &pData );
	*pData = Data;
}
template <class DataType, int nCount>
void CStaticLinkList<DataType, nCount>::RemoveAt( int nPos )
{
#ifdef	_DEBUG
	char msg[256];
	if( CContainerBase::Count() == 0 )
	{
		sprintf( msg, "CStaticLinkList::RemoveAt() : List is empty! Head<%d>, Tail<%d>, Current<%d>, Total<%d>\n", m_nHead, m_nTail, nPos, CContainerBase::Count() );
		OutputDebugString_T( msg );
	}
#endif
	assert( nPos >= 0 && nPos < nCount );
	BOOL ret = CContainerBase::Remove( nPos );
	assert( ret == TRUE );

	// 改变连接关系
	int nCur, nNext, nPrev;
	nCur = nPos;
	nNext = m_nNext[nPos];
	nPrev = m_nPrev[nPos];
	if( nNext != -1 )
	{
		m_nPrev[nNext] = nPrev;
		if( m_nHead == nCur )
			m_nHead = nNext;
		if( m_nTail == nCur )
			m_nTail = nNext;
	}
	if( nPrev != -1 )
	{
		m_nNext[nPrev] = nNext;
		if( m_nHead == nCur )
			m_nHead = nPrev;
		if( m_nTail == nCur )
			m_nTail = nPrev;
	}
	m_nNext[nCur] = -1;	// 指向下一个结点，与每一个结点相配
	m_nPrev[nCur] = -1;	// 指向上一个结点，与每一个结点相配
	if( nNext == -1 && nPrev == -1 )
	{
		m_nHead = m_nTail = -1;
	}
}


// Insertion
template <class DataType, int nCount>
int CStaticLinkList<DataType, nCount>::InsertAfter( int nPos, const DataType &Data )
{
	if( CContainerBase::Count() == nCount )
	{
#ifdef	_DEBUG
		char msg[256];
		sprintf( msg, "CStaticLinkList::InsertAfter() : List is overflow! Head<%d>, Tail<%d>, Current<%d>, Total<%d>\n", m_nHead, m_nTail, nPos, CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return -1;
	}
	// 如果指向的是不合法的节点则添加到末尾
	if( nPos < 0 )
	{
		nPos = GetHeadPosition();
		return InsertBefore( nPos, Data );
	}
	else if( nPos >= nCount || CContainerBase::IsValid( nPos ) == FALSE )
		nPos = GetTailPosition();

	// 改变连接关系
	int nNew = CContainerBase::FindRoom();
	if( nPos == -1 )
	{	// 链表是空的
		assert( CContainerBase::IsEmpty() );
		m_nHead = nNew;
	}
	else
	{
		assert( nPos >= 0 && nPos < nCount && CContainerBase::IsValid( nPos ) );
		int nNext = m_nNext[nPos];
		m_nNext[nNew] = nNext;
		m_nPrev[nNew] = nPos;
		if( nNext != -1 )
			m_nPrev[nNext] = nNew;
		m_nNext[nPos] = nNew;
	}
	if( m_nTail == nPos )
		m_nTail = nNew;

	assert( nNew >= 0 && nNew < nCount );
	BOOL ret = Add( nNew, Data );
	assert( ret == TRUE );
	return nNew;
}

// 在当前指针前面插入内容
template <class DataType, int nCount>
int CStaticLinkList<DataType, nCount>::InsertBefore( int nPos, const DataType &Data )
{
	if( CContainerBase::Count() == nCount )
	{
#ifdef	_DEBUG
		char msg[256];
		sprintf( msg, "CStaticLinkList::InsertBefore() : List is overflow! Head<%d>, Tail<%d>, Current<%d>, Total<%d>\n", m_nHead, m_nTail, nPos, CContainerBase::Count() );
		OutputDebugString_T( msg );
#endif
		return -1;
	}
	// 如果指向的是不合法的节点则添加到末尾
	if( nPos < 0 )
		nPos = GetHeadPosition();
	else if( nPos >= nCount || CContainerBase::IsValid( nPos ) == FALSE )
	{
		nPos = GetTailPosition();
		return InsertAfter( nPos, Data );
	}

	// 改变连接关系
	int nNew = CContainerBase::FindRoom();
	if( nPos == -1 )
	{	// 链表是空的
		assert( CContainerBase::IsEmpty() );
		m_nTail = nNew;
	}
	else
	{
		assert( nPos >= 0 && nPos < nCount && CContainerBase::IsValid( nPos ) );
		int nPrev = m_nPrev[nPos];
		m_nNext[nNew] = nPos;
		m_nPrev[nNew] = nPrev;
		if( nPrev != -1 )
			m_nNext[nPrev] = nNew;
		m_nPrev[nPos] = nNew;
	}
	if( m_nHead == nPos )
		m_nHead = nNew;
	assert( nNew >= 0 && nNew < nCount );
	BOOL ret = Add( nNew, Data );
	assert( ret == TRUE );
	return nNew;
}

// 根据
// find by index
// returns :	Position
template <class DataType, int nCount>
int CStaticLinkList<DataType, nCount>::FindIndex( int nIndex )
{
	int nCounter = 0;
	int nPosLast;
	for( int nPos = GetHeadPosition(); nPos != -1; )
	{
		nPosLast = nPos;
		GetNext( nPos );
		if( nCounter == nIndex )
			return nPosLast;
		nCounter++;
	}
	return -1;
}
/////////////////////////////////////////////////


/////////////////////////////////////////////////
//
//	CDMList	链表模版
//
/////////////////////////////////////////////////

////////////
// 结点
template <class DataType>
CDMNode<DataType>::CDMNode()
{
	pNext = NULL;
	pPrev = NULL;
}
/*
template <class DataType>
CDMNode<DataType>::~CDMNode()
{
	pNext = NULL;
	pPrev = NULL;
}
*/
/*
template <class DataType>
CDMNode<DataType>& CDMNode<DataType>::operator=( const CDMNode& node )
{
	data = node.data;
	pNext = node.pNext;
	pPrev = node.pPrev;
	return *this;
}
*/
////////////

////////////
template <class DataType>
CDMList<DataType>::CDMList()
{
	m_nCount = 0;
	m_pNodeHead = NULL;
	m_pNodeTail = NULL;
}
template <class DataType>
CDMList<DataType>::~CDMList()
{
	// 不能在这里加本函数，否则容易造成重入
	if( m_nCount > 0 )
		RemoveAll();
	assert(m_nCount == 0);
}
////////////

////////////
template <class DataType>
CDMNode<DataType>* CDMList<DataType>::NewNode( CDMNode<DataType>*pPrev, CDMNode<DataType>*pNext )
{
	CDMNode<DataType>* pNode = new CDMNode<DataType>;
	if( !pNode )
	{
#ifdef	_DEBUG
	//	OutputDebugString_T( "CDMList::NewNode() Error(0) : Not enough memory!" );
#endif
		return NULL;
	}
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;

	m_nCount++;
	return pNode;
}
template <class DataType>
void CDMList<DataType>::FreeNode( CDMNode<DataType>* pNode )
{
	delete pNode;
	m_nCount--;
}
////////////

////////////
// count of elements
// 得到数量
template <class DataType>
int CDMList<DataType>::GetCount() const
	{ return m_nCount; }
// 结构内是否是空的
template <class DataType>
BOOL CDMList<DataType>::IsEmpty() const
	{ return m_nCount == 0; }
////////////

////////////
// head&tail
template <class DataType>
DataType& CDMList<DataType>::GetHead()
	{ assert(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
template <class DataType>
DataType CDMList<DataType>::GetHead() const
	{ assert(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
template <class DataType>
DataType& CDMList<DataType>::GetTail()
	{ assert(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
template <class DataType>
DataType CDMList<DataType>::GetTail() const
	{ assert(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
////////////

////////////
// get
template <class DataType>
POSI CDMList<DataType>::GetHeadPosition() const
	{ return (POSI) m_pNodeHead; }
template <class DataType>
POSI CDMList<DataType>::GetTailPosition() const
	{ return (POSI) m_pNodeTail; }

template <class DataType>
DataType& CDMList<DataType>::GetNext( POSI &rPosition )
	{ CDMNode<DataType>* pNode = (CDMNode<DataType>*) rPosition;
		rPosition = (POSI) pNode->pNext;
		return pNode->data; }
template <class DataType>
DataType CDMList<DataType>::GetNext( POSI &rPosition ) const
	{ CDMNode<DataType>* pNode = (CDMNode<DataType>*) rPosition;
		rPosition = (POSI) pNode->pNext;
		return pNode->data; }
template <class DataType>
DataType& CDMList<DataType>::GetPrev( POSI &rPosition )
	{ CDMNode<DataType>* pNode = (CDMNode<DataType>*) rPosition;
		rPosition = (POSI) pNode->pPrev;
		return pNode->data; }
template <class DataType>
DataType CDMList<DataType>::GetPrev( POSI &rPosition ) const
	{ CDMNode<DataType>* pNode = (CDMNode<DataType>*) rPosition;
		rPosition = (POSI) pNode->pPrev;
		return pNode->data; }
////////////

////////////
// Modification
template <class DataType>
DataType & CDMList<DataType>::GetAt( POSI position )
	{ CDMNode<DataType>* pNode = (CDMNode<DataType>*) position;
		return pNode->data; }
template <class DataType>
DataType CDMList<DataType>::GetAt( POSI position ) const
	{ CDMNode<DataType>* pNode = (CDMNode<DataType>*) position;
		return pNode->data; }
template <class DataType>
void CDMList<DataType>::SetAt( POSI pos, const DataType &newElement )
	{ CDMNode<DataType>* pNode = (CDMNode<DataType>*) pos;
		pNode->data = newElement; }
////////////


////////////
// Operations
template <class DataType>
void CDMList<DataType>::RemoveAll()
{
	CDMNode<DataType>* pNode = m_pNodeHead;
	while( pNode )
	{
		CDMNode<DataType>* pNext = pNode->pNext;
		FreeNode( pNode );
		pNode = pNext;
	}

	assert( m_nCount == 0 );
	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = NULL;
}

template <class DataType>
void CDMList<DataType>::RemoveAllData()
{
	CDMNode<DataType>* pNode = m_pNodeHead;
	while( pNode )
	{
		CDMNode<DataType>* pNext = pNode->pNext;
		if( pNode->data )
		{
			delete	pNode->data;
		}
		pNode = pNext;
	}
}

////////////

////////////
template <class DataType>
POSI CDMList<DataType>::AddHead( const DataType &newElement )
{
	CDMNode<DataType>* pNewNode = NewNode(NULL, m_pNodeHead);
	assert( pNewNode );
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (POSI) pNewNode;
}
template <class DataType>
POSI CDMList<DataType>::AddTail( const DataType &newElement )
{
	CDMNode<DataType>* pNewNode = NewNode(m_pNodeTail, NULL);
	assert( pNewNode );
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (POSI) pNewNode;
}
template <class DataType>
void CDMList<DataType>::AddHead( CDMList<DataType>* pNewList )
{
	assert(pNewList != NULL);

	// add a list of same elements to head (maintain order)
	POSI pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}
template <class DataType>
void CDMList<DataType>::AddTail( CDMList<DataType>* pNewList )
{
	assert(pNewList != NULL);

	// add a list of same elements
	POSI pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}
template <class DataType>
DataType& CDMList<DataType>::RemoveHead()
{
	assert(m_pNodeHead != NULL);  // don't call on empty list !!!

	CDMNode<DataType>* pOldNode = m_pNodeHead;
	m_FreeData = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return m_FreeData;
}
template <class DataType>
DataType& CDMList<DataType>::RemoveTail()
{
	assert(m_pNodeTail != NULL);  // don't call on empty list !!!

	CDMNode<DataType>* pOldNode = m_pNodeTail;
	m_FreeData = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return m_FreeData;
}
////////////

////////////
template <class DataType>
POSI CDMList<DataType>::InsertBefore( POSI position, const DataType &newElement )
{
	if (position == NULL)
		return AddHead(newElement); // insert before nothing -> head of the list

	CDMNode<DataType>* pOldNode = (CDMNode<DataType>*) position;
	CDMNode<DataType>* pNewNode = NewNode( pOldNode->pPrev, pOldNode );
	assert( pNewNode );
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		assert(pOldNode == m_pNodeHead);
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (POSI) pNewNode;
}
template <class DataType>
POSI CDMList<DataType>::InsertAfter( POSI position, const DataType &newElement )
{
	if (position == NULL)
		return AddTail(newElement); // insert after nothing -> tail of the list

	// Insert it before position
	CDMNode<DataType>* pOldNode = (CDMNode<DataType>*) position;
	CDMNode<DataType>* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (POSI) pNewNode;
}
template <class DataType>
void CDMList<DataType>::RemoveAt( POSI position )
{
	CDMNode<DataType>* pOldNode = (CDMNode<DataType>*) position;

	// remove pOldNode from list
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}
////////////

////////////
// slow operations
// 根据顺序查询成员，返回链表的下标，NULL为没有找到
template <class DataType>
POSI CDMList<DataType>::FindIndex( int nIndex )
{
	if (nIndex >= m_nCount || nIndex < 0)
		return NULL;  // went too far

	CDMNode<DataType>* pNode = m_pNodeHead;
	while (nIndex--)
	{
		pNode = pNode->pNext;
	}
	return (POSI) pNode;
}
// 根据取值查询成员，返回链表的下标，NULL为没有找到
template <class DataType>
POSI CDMList<DataType>::Find( const DataType &searchValue, POSI startAfter )
{
	CDMNode<DataType>* pNode = (CDMNode<DataType>*) startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;  // start at head
	}
	else
	{
		pNode = pNode->pNext;  // start after the one specified
	}

	for (; pNode != NULL; pNode = pNode->pNext)
		if (pNode->data == searchValue)
			return (POSI) pNode;
	return NULL;
}
////////////
/////////////////////////////////////////////////


/////////////////////////////////////////////////
//
//	CDMPtrList	:	指针链表
//
/////////////////////////////////////////////////
inline int CDMPtrList::GetCount() const
	{ return m_nCount; }
inline BOOL CDMPtrList::IsEmpty() const
	{ return m_nCount == 0; }
inline void*& CDMPtrList::GetHead()
	{ assert(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void* CDMPtrList::GetHead() const
	{ assert(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void*& CDMPtrList::GetTail()
	{ assert(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline void* CDMPtrList::GetTail() const
	{ assert(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline POSI CDMPtrList::GetHeadPosition() const
	{ return (POSI) m_pNodeHead; }
inline POSI CDMPtrList::GetTailPosition() const
	{ return (POSI) m_pNodeTail; }
inline void*& CDMPtrList::GetNext(POSI& rPosition) // return *Position++
	{ CDMPNode* pNode = (CDMPNode*) rPosition;
		rPosition = (POSI) pNode->pNext;
		return pNode->data; }
inline void* CDMPtrList::GetNext(POSI& rPosition) const // return *Position++
	{ CDMPNode* pNode = (CDMPNode*) rPosition;
		rPosition = (POSI) pNode->pNext;
		return pNode->data; }
inline void*& CDMPtrList::GetPrev(POSI& rPosition) // return *Position--
	{ CDMPNode* pNode = (CDMPNode*) rPosition;
		rPosition = (POSI) pNode->pPrev;
		return pNode->data; }
inline void* CDMPtrList::GetPrev(POSI& rPosition) const // return *Position--
	{ CDMPNode* pNode = (CDMPNode*) rPosition;
		rPosition = (POSI) pNode->pPrev;
		return pNode->data; }
inline void*& CDMPtrList::GetAt(POSI position)
	{ CDMPNode* pNode = (CDMPNode*) position;
		return pNode->data; }
inline void* CDMPtrList::GetAt(POSI position) const
	{ CDMPNode* pNode = (CDMPNode*) position;
		return pNode->data; }
inline void CDMPtrList::SetAt(POSI pos, void* newElement)
	{ CDMPNode* pNode = (CDMPNode*) pos;
		pNode->data = newElement; }
/////////////////////////////////////////////////


/////////////////////////////////////////////////
//
//	CDMPtrPool	:	内存分配后并不急于释放
//
/////////////////////////////////////////////////
template <class DataType>
CDMPtrPool<DataType>::CDMPtrPool(){ m_nMax = 0; }

// 得到一个空位，没有则申请内存
template <class DataType>
DataType* CDMPtrPool<DataType>::NewPtr( int nNum ) // = 10
{
	if( m_ListReady.IsEmpty() )
	{
		assert( nNum > 0 );
		for( int i=0; i<nNum; i++ )
		{
			DataType* pData = new DataType;
			if( !pData )
			{
#ifdef	_DEBUG
				OutputDebugString_T( "CDMPtrPool::NewPtr() Error(0) : Not enough memory!" );
#endif
				return NULL;
			}
			m_ListReady.AddTail( pData );
		}
#ifdef	_DEBUG
		if( m_nMax < GetCount() )
		{
			m_nMax = GetCount();
			char szMsg[256];
			::sprintf( szMsg, "CDMPtrPool::Max<%d>\n", m_nMax );
			//OutputDebugString_T( szMsg );
		}
#endif
	}
	if( !m_ListReady.IsEmpty() )
	{
		DataType* pData = (DataType*)m_ListReady.RemoveHead();
		AddTail( pData );
		return pData;
	}
	else
		return NULL;
}
// 释放一个空位，并不真正从内存中删除
// 注意，如果里面的内容过多，而且NewPtr()和DeletePtr()被调用的随机性过大，效率会比较低
template <class DataType>
void CDMPtrPool<DataType>::DeletePtr( DataType* pData )
{
	POSI posOld = NULL;
	for( POSI pos = GetHeadPosition(); pos!=NULL; )
	{
		posOld = pos;
		DataType* pData2 = (DataType*)GetNext( pos );
		if( pData2 && pData2 == pData )
		{
			RemoveAt( posOld );
			m_ListReady.AddTail( pData2 );
			break;
		}
	}
}

// 释放所有空位，并不真正从内存中删除
template <class DataType>
void CDMPtrPool<DataType>::DeleteAll()
{
	while( !IsEmpty() )
	{
		DataType* pData2 = (DataType*)RemoveHead();
		m_ListReady.AddTail( pData2 );
	}
}


// 从内存中删除一定数量的结点
template <class DataType>
void CDMPtrPool<DataType>::Free( int nNum ) // = 10 )
{
	int ncount = 0;
	while( !m_ListReady.IsEmpty() )
	{
		DataType* pData2 = (DataType*)m_ListReady.RemoveHead();
		delete pData2;
		ncount++;
		if( ncount == 10 )
			break;
	}
}
// 释放所有内存
template <class DataType>
void CDMPtrPool<DataType>::FreeAll()
{
	while( !m_ListReady.IsEmpty() )
	{
		DataType* pData2 = (DataType*)m_ListReady.RemoveHead();
		delete pData2;
	}
}
/////////////////////////////////////////////////
