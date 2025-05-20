/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: CIDTable

	Purpose: ��IDת��Ϊָ��

	Author: soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 6/10/2004.

	Remarks: ע��ID��Ҫ�ظ�
		

*********************************************************/

#ifndef __CIDTable_H
#define __CIDTable_H


#include "IDTableBase.h"

class  CIDTable : public CIDTableBase
{
public:
	CIDTable();
	~CIDTable();
	
	int GetNum()	{ return m_iNum; }
//	int	GetCurMax() { return m_iCurMax; }
	
	void * GetObj( int i );

	void Build( int iMax,int iPrivateMax = 10 );
	void BuildUseBuf( void ** pBuf,int iMax );
	void Remove( int i );

/////////////////////////////////////////////////////////����һ�����е�id��-1Ϊ�Ѿ�û�п���id��	
	int	GetFreeAutoID();
////////////////////////////////////////////////////////////////////////////////
	int Add( void * p );
	int AddInto( int i,void * p );
	void Reset();
	void SetMin( int iMin );
	void	Space( int iSpace = 1000 );//Ϊ�˼�������뽨��
private:
	void ** m_pBuf;
	bool m_booUseBuf;
};

#endif
