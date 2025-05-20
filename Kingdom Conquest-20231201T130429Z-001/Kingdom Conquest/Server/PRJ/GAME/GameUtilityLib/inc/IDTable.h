/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: CIDTable

	Purpose: 将ID转化为指针

	Author: soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 6/10/2004.

	Remarks: 注意ID不要重复
		

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

/////////////////////////////////////////////////////////返回一个空闲的id，-1为已经没有空闲id了	
	int	GetFreeAutoID();
////////////////////////////////////////////////////////////////////////////////
	int Add( void * p );
	int AddInto( int i,void * p );
	void Reset();
	void SetMin( int iMin );
	void	Space( int iSpace = 1000 );//为了间隔人物与建筑
private:
	void ** m_pBuf;
	bool m_booUseBuf;
};

#endif
