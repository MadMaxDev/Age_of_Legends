/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: CIDStrTable

	Purpose: ���ַ���IDת��Ϊָ��

	Author: soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 6/10/2004.

	Remarks: ע��ID��Ҫ�ظ�

*********************************************************/



#ifndef __CIDStrTable_H
#define __CIDStrTable_H

#include "IDTableBase.h"



struct	SIDStrUnit
{
	int iID;
	void *	p;

	SIDStrUnit()
	{
		Empty();
	}


	void	Empty()
	{
		iID = -1;
		p = NULL;
	}

	BOOL IsEmpty()
	{
		return iID == -1 && p == NULL;
	}

};

struct	SStrList;

class  CIDStrTable : public CIDTableBase
{
public:
	CIDStrTable();
	~CIDStrTable();
	
	int GetNum()	{ return m_iNum; }
	int	GetCurMax() { return m_iCurMax; }
	
	void * GetObj( int i );
	SIDStrUnit *  GetUnit( int i );
	void * GetObj( LPCSTR sz );

	void Build( int iMax );
	void Remove( int i );

	int Add( void * p,LPCSTR sz );
	int AddInto( int i,void * p,LPCSTR sz );
	void Reset();
	void SetMin( int iMin );

	BOOL  Load( LPCSTR szName );
private:
	SIDStrUnit * m_pBuf;
	SStrList	*m_pStrList;
	
};

#endif
