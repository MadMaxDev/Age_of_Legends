#include "../inc/Stdafx.h"
#include "../inc/EmTools.h"
#include "../inc/Mat.h"
#include "../inc/IDCompressTable.h"
#include "../inc/misc.h"


CIDCompressTable::CIDCompressTable()
{
	m_pBuf = NULL;
	m_pBufExp = NULL;

	m_iMax = 0;
	m_iMin = 0;
	m_iCur = 0;
	m_iCurExp = 0;
}

CIDCompressTable::~CIDCompressTable()
{
	ClearAll();
}

void CIDCompressTable::ClearAll()
{
	SAFE_DELETE_ARRAY( m_pBuf );
	SAFE_DELETE_ARRAY( m_pBufExp );
}

void CIDCompressTable::Build( int iMax )
{
	ClearAll();

	m_iMax = iMax + 1;	//所以最大+1，与IDTable统一,反正不会出错;	
	m_pBuf = new SIDCompressUnit[m_iMax];
	m_pBufExp = new SIDCompressUnit[m_iMax];
	
	Reset();
}


void CIDCompressTable::Reset()
{
	m_iNum = 0;
	m_iCurMax = 0;
	if( m_pBuf )
	{
		int i;
		for(i=0;i<m_iMax;i++)
			m_pBuf[i].Clear();
		for(i=0;i<m_iMax;i++)
			m_pBufExp[i].Clear();
	}
}

void CIDCompressTable::SetMin( int iMin )
{
	m_iMin = iMin;
	m_iCur = iMin;
}

SIDCompressUnit	* CIDCompressTable::GetFree( DWORD i,SIDCompressUnit	* & pPrevNode )
{
	int idx = i%m_iMax;
	if( m_pBuf[idx].IsEmpty() )
	{
		pPrevNode = NULL;
		return &(m_pBuf[idx]);
	}
	else
	{
		pPrevNode = &(m_pBuf[idx]);

		int iNum = 0;
		while( pPrevNode->pNext )
		{
			if( ++iNum >= m_iMax )
			{
				goto Fail;
			}
			pPrevNode = pPrevNode->pNext;
		}

		iNum = 0;
		while( !m_pBufExp[m_iCurExp].IsEmpty() )
		{
			if( ++iNum >= m_iMax )
			{
				goto Fail;
			}

			m_iCurExp++;
			if( m_iCurExp >= m_iMax )
				m_iCurExp = 0;
		}
		return &(m_pBufExp[m_iCurExp]);
	}
Fail:
	char	szStr[4096];
	sprintf( szStr,"Error CIDCompressTable::Add  Full %d",m_iMax );
	OutputError( szStr );
	return NULL;
}

BOOL CIDCompressTable::AddInto( void * p,DWORD i )
{	
	
	SIDCompressUnit	*  pPrevNode = NULL;
	SIDCompressUnit	*  pFreeNode;
	pFreeNode = GetFree( i,pPrevNode );
	if( pFreeNode )
	{
		pFreeNode->dID = i;
		pFreeNode->p = p;
		if( pPrevNode )
		{
			pPrevNode->pNext = pFreeNode;
		}
		m_iNum++;
		return true;
	}
//GetFree里已经记录了， AddInto( void * p,DWORD i )查不出重复加入ID，Get时候会先Get出最早加入的指针
//	char	szStr[4096];
//	sprintf( szStr,"Error CIDCompressTable::AddInto overlay%d",i );
//	OutputError( szStr );
	return false;
}

void * CIDCompressTable::GetObj( DWORD i )
{
	int idx = i%m_iMax;
	SIDCompressUnit	*  pNode = &(m_pBuf[idx]);

	while( pNode->dID != i )
	{
		if( pNode->pNext )
		{
			pNode = pNode->pNext;
		}
		else
		{
			return NULL;
		}
	}
	return pNode->p;
}


void CIDCompressTable::Remove( DWORD i )
{
	int idx = i%m_iMax;	
	SIDCompressUnit	*  pNode = &(m_pBuf[idx]);
	SIDCompressUnit	*  pPrevNode = NULL;

	

	while( pNode->dID != i )
	{
		if( pNode->pNext )
		{
			pPrevNode = pNode;
			pNode = pNode->pNext;
		}
		else
		{
			return;
		}
	}

	if( pPrevNode )
	{
		pPrevNode->pNext = pNode->pNext;
	}
	else
	{

		if( pNode->pNext )
		{
			SIDCompressUnit	*  pNext = pNode->pNext;
			pNode->dID = pNext->dID;
			pNode->p = pNext->p;
			pNode->pNext = pNext->pNext;
			pNode = pNext;
		}
	}
	
	pNode->Clear();
	m_iNum--;
}
