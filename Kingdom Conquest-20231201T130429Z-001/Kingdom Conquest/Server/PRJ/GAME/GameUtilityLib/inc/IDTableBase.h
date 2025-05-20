/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: IDCompressTable

	Purpose: ID表的基类

	Author: soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 6/10/2004.

	Remarks:

*********************************************************/


#ifndef __CIDTableBase_H
#define __CIDTableBase_H

#include <string>
#include "DMtype.h"
class  CIDTableBase
{
public:
	CIDTableBase();
	~CIDTableBase();
	
	int GetNum()	{ return m_iNum; }
	void	SetLogName( const char * sz ){ 
		if( m_pLogName == NULL )
		{
			m_pLogName = new char[256];
			memset(m_pLogName,0,sizeof(char)*256);
		}
		strncpy( m_pLogName,sz,255 );
	}
	void	OutputError( const char * sz );
protected:
	int m_iMax;
	int m_iMin;
	int m_iCurMax;//当前最大值
	int m_iCur;
	int m_iNum;
	char *	m_pLogName;//记log的时候使用
};

#endif
