#include "../inc/Mat.h"
#include "../inc/IDStrTable.h"
#include "../inc/misc.h"

#include <WHCMN/inc/whstring.h>

#include <iostream>
#include <string>
#include <map>


using namespace std;

struct my_cmp
{
	inline bool operator () (const string& _a,const string& _b) const
	{
		return stricmp(_a.c_str(),_b.c_str()) < 0;
	}
};
typedef map< string,int,my_cmp  > MAPSTRLIST;

struct	SStrList
{
	MAPSTRLIST	 List;
};

CIDStrTable::CIDStrTable()
{
	m_pStrList = new SStrList;
	m_pBuf = NULL;
	m_iMax = 0;
	m_iMin = 0;
	m_iCur = 0;
}

CIDStrTable::~CIDStrTable()
{
	SAFE_DELETE( m_pStrList );
	SAFE_DELETE_ARRAY( m_pBuf );
}

void CIDStrTable::Build( int iMax )
{
	SAFE_DELETE_ARRAY( m_pBuf );
	m_iMax = iMax + 1;	//所以最大+1，与IDTable统一,反正不会出错;	
	m_pBuf = new SIDStrUnit[m_iMax];
	
	Reset();
}

void CIDStrTable::SetMin( int iMin )
{
	m_iMin = iMin;
	m_iCur = iMin;
}

int CIDStrTable::Add( void * p,LPCSTR sz )
{	
	int iID;
	int i;
	for(i=m_iCur;i<m_iMax;i++)
	{
		if( m_pBuf[i].IsEmpty() )
		{
			iID = AddInto( i,p,sz );
			return iID;
		}
	}	

	for( i=m_iMin;i<m_iCur;i++)
	{
		if( m_pBuf[i].IsEmpty() )
		{
			iID = AddInto( i,p,sz );
			return iID;
		}
	}
	char	szStr[4096];
	sprintf( szStr,"Error CIDStrTable::Add  Full %d",m_iMax );
	OutputError( szStr );

	return 0;	
}

int CIDStrTable::AddInto( int i,void * p,LPCSTR sz )
{	
	if( !ISIN( i,0,m_iMax-1) )
	{
		char	szStr[4096];
		sprintf( szStr,"Error CIDStrTable::AddInto beyond %d,%d,%s",i,m_iMax,sz );
		OutputError( szStr );
		return -1;
	}

	if( m_pBuf[i].IsEmpty() )
	{

		m_pStrList->List.insert (MAPSTRLIST::value_type(sz,i) );//Name.LockBuffer()
		m_pBuf[i].p = p;
		m_pBuf[i].iID = i;
		m_iNum++;
		if( i > m_iCurMax-1 )
			m_iCurMax = i+1;
		return i;
	}

	char	szStr[4096];
	sprintf( szStr,"Error CIDStrTable::AddInto overlay %d",i );
	OutputError( szStr );

	return -1;
}

SIDStrUnit *  CIDStrTable::GetUnit( int i )
{
	if( ISIN( i,0,m_iMax-1) )
	{
		return &(m_pBuf[i]);
	}
	else
	{
		return NULL;
	}
}

void * CIDStrTable::GetObj( int i )
{
	if( ISIN( i,0,m_iMax-1) )
	{
		return m_pBuf[i].p;
	}
	else
	{
		return NULL;
	}
}

void * CIDStrTable::GetObj( LPCSTR sz )
{
	MAPSTRLIST::iterator theIterator;
	theIterator = m_pStrList->List.find(sz);//Name.LockBuffer()
	if(theIterator != m_pStrList->List.end() )    // is 0 - 9
	{
		return GetObj( (*theIterator).second  );
	}
	return NULL;
}


BOOL  CIDStrTable::Load( LPCSTR szName )
{
	return true;
}


void CIDStrTable::Remove( int i )
{
	if( !ISIN( i,0,m_iMax-1) )
	{
		char	szStr[4096];
		sprintf( szStr,"Error CIDStrTable::Remove beyond %d,%d",i,m_iMax );
		OutputError( szStr );
		return ;
	}

	m_pBuf[i].Empty();
	m_iNum--;
}

void CIDStrTable::Reset()
{
	m_iNum = 0;
	m_iCurMax = 0;
	if( m_pBuf )
	{
		for(int i=0;i<m_iMax;i++)
			m_pBuf[i].Empty();
	}
	SAFE_DELETE( m_pStrList );
	m_pStrList = new SStrList;
}

