/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: IDCompressTable

	Purpose: 将ID转化为指针

	Author: soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 6/10/2004.

	Remarks: 注意ID不要重复
		ID可以任意大小，只要ID数量不超过上限，比CIDTable效率低，适合ID不连续的表

*********************************************************/


#ifndef __CIDCompressTable_H
#define __CIDCompressTable_H

#include "DMtype.h"
#include "IDTableBase.h"

struct	SIDCompressUnit
{
	SIDCompressUnit()
	{
		Clear();
	}

	void	Clear()
	{
		dID = 0;
		p = NULL;
		pNext = NULL;
	}
	BOOL	IsEmpty()
	{
		return p == NULL;
	}

	DWORD	dID;
	void *  p;
	SIDCompressUnit*	pNext;
};

class  CIDCompressTable : public CIDTableBase
{
public:
	CIDCompressTable();
	~CIDCompressTable();
	
	int GetNum()	{ return m_iNum; }
	int	GetCurMax() { return m_iCurMax; }
	
	
	void Build( int iMax );

	void * GetObj( DWORD i );
	void Remove( DWORD i );

	BOOL AddInto( void * p,DWORD i );


	void Reset();
	void SetMin( int iMin );

private:
	void ClearAll();
	SIDCompressUnit	* GetFree( DWORD i,SIDCompressUnit	* & pPrevNode );

private:
	int m_iCurExp;
	SIDCompressUnit	*  m_pBuf;
	SIDCompressUnit	*  m_pBufExp;
};

#endif
