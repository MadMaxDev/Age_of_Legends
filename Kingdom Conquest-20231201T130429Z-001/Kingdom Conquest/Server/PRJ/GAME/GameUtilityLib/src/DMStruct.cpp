/////////////////////////////////////////////////
// DMStruct.cpp	:	V0010
// Writen by	:	Liu Gang
// V0010		:	Mar.16.2000
// V0011		:	July.28.2000
// V0012		:	May.23.2001
//					修改了静态链表的实现，因为有BUG，现在的结构与MFC的类似。
/////////////////////////////////////////////////
// 通用数据结构模板

#include "../inc/DMStruct.h"
#include <assert.h>


/////////////////////////////////////////////////////////////////////////////
// CDMPlex

CDMPlex* CDMPlex::Create(CDMPlex*& pHead, UINT nMax, UINT cbElement)
{
	assert(nMax > 0 && cbElement > 0);
	CDMPlex* p = (CDMPlex*) new BYTE[sizeof(CDMPlex) + nMax * cbElement];
			// may throw exception
	p->pNext = pHead;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
}

void CDMPlex::FreeDataChain()     // free this one and links
{
	CDMPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		CDMPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CDMPtrList::CDMPtrList(int nBlockSize)
{
	assert(nBlockSize > 0);

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

void CDMPtrList::RemoveAll()
{
	// destroy elements

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}


CDMPtrList::~CDMPtrList()
{
	RemoveAll();
	assert(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Node helpers
/*
 * Implementation note: CDMPNode's are stored in CDMPlex blocks and
 *  chained together. Free blocks are maintained in a singly linked list
 *  using the 'pNext' member of CDMPNode with 'm_pNodeFree' as the head.
 *  Used blocks are maintained in a doubly linked list using both 'pNext'
 *  and 'pPrev' as links and 'm_pNodeHead' and 'm_pNodeTail'
 *   as the head/tail.
 *
 * We never free a CDMPlex block unless the List is destroyed or RemoveAll()
 *  is used - so the total number of CDMPlex blocks may grow large depending
 *  on the maximum past size of the list.
 */

CDMPtrList::CDMPNode*
CDMPtrList::NewNode(CDMPtrList::CDMPNode* pPrev, CDMPtrList::CDMPNode* pNext)
{
	if (m_pNodeFree == NULL)
	{
		// add another block
		CDMPlex* pNewBlock = CDMPlex::Create(m_pBlocks, m_nBlockSize,
				 sizeof(CDMPNode));

		// chain them into free list
		CDMPNode* pNode = (CDMPNode*) pNewBlock->data();
		// free in reverse order to make it easier to debug
		pNode += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	assert(m_pNodeFree != NULL);  // we must have something

	CDMPtrList::CDMPNode* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;
	assert(m_nCount > 0);  // make sure we don't overflow




	pNode->data = 0; // start with zero

	return pNode;
}

void CDMPtrList::FreeNode(CDMPtrList::CDMPNode* pNode)
{

	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;
	assert(m_nCount >= 0);  // make sure we don't underflow

	// if no more elements, cleanup completely
	if (m_nCount == 0)
		RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////

POSI CDMPtrList::AddHead(void* newElement,int   idx)
{
	CDMPNode* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	pNewNode->idx  = idx;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (POSI) pNewNode;

}



POSI CDMPtrList::AddTail(void* newElement,int   idx)
{
	CDMPNode* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	pNewNode->idx  = idx;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (POSI) pNewNode;

}



void CDMPtrList::AddHead(CDMPtrList* pNewList)
{
	assert(pNewList != NULL);
	// add a list of same elements to head (maintain order)
	POSI pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}

void CDMPtrList::AddTail(CDMPtrList* pNewList)
{
	assert(pNewList != NULL);
	// add a list of same elements
	POSI pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}

void* CDMPtrList::RemoveHead()
{
	assert(m_pNodeHead != NULL);  // don't call on empty list !!!
	CDMPNode* pOldNode = m_pNodeHead;
	void* returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

void* CDMPtrList::RemoveTail()
{
	assert(m_pNodeTail != NULL);  // don't call on empty list !!!

	CDMPNode* pOldNode = m_pNodeTail;
	void* returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

POSI CDMPtrList::InsertBefore(POSI position, void* newElement)
{
	if (position == NULL)
		return AddHead(newElement); // insert before nothing -> head of the list

	// Insert it before position
	CDMPNode* pOldNode = (CDMPNode*) position;
	CDMPNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
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



POSI CDMPtrList::InsertAfter(POSI position, void* newElement)
{
	if (position == NULL)
		return AddTail(newElement); // insert after nothing -> tail of the list

	// Insert it before position
	CDMPNode* pOldNode = (CDMPNode*) position;
	CDMPNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		assert(pOldNode == m_pNodeTail);
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (POSI) pNewNode;

}



void CDMPtrList::RemoveAt(POSI position)
{
	CDMPNode* pOldNode = (CDMPNode*) position;

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


/////////////////////////////////////////////////////////////////////////////
// slow operations

POSI CDMPtrList::FindIndex(int nIndex) const
{
	if (nIndex >= m_nCount || nIndex < 0)
		return NULL;  // went too far

	CDMPNode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		pNode = pNode->pNext;
	}
	return (POSI) pNode;
}

POSI CDMPtrList::Sort( BOOL booBig ) const
{
	CDMPNode* pNode = NULL;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;  // start at head
	}
	else
	{
		pNode = pNode->pNext;  // start after the one specified
	}

	CDMPNode* pSortNode = pNode;
	
	if( booBig )
	{
		for (; pNode != NULL; pNode = pNode->pNext)
		{
			if( pSortNode->idx < pNode->idx )
			{
				pSortNode = pNode;
			}
		}
	}
	else
	{
		for (; pNode != NULL; pNode = pNode->pNext)
		{
			if( pSortNode->idx > pNode->idx )
			{
				pSortNode = pNode;
			}
		}
	}
	return (POSI)pSortNode;
}

POSI CDMPtrList::Find(void* searchValue, POSI startAfter) const
{
	CDMPNode* pNode = (CDMPNode*) startAfter;
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
/////////////////////////////////////////////////////////////////////////////
