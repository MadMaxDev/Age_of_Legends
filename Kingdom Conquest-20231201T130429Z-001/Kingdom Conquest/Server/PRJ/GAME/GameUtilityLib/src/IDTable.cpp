#include "../inc/Stdafx.h"
#include "../inc/Mat.h"
#include "../inc/IDTable.h"
#include "../inc/EmTools.h"
#include "../inc/misc.h"

CIDTable::CIDTable()
{
	m_pBuf = NULL;
	m_iMax = 0;
	m_iMin = 0;
	m_iCur = 0;
	m_booUseBuf = false;
}

CIDTable::~CIDTable()
{
	if( m_booUseBuf == false )
	{
		SAFE_DELETE_ARRAY( m_pBuf );
	}
}

void CIDTable::BuildUseBuf( void ** pBuf,int iMax )
{
	m_booUseBuf = true;
	m_iMax = iMax + 1;	//因为最小是一，所以最大+1，以避免混淆
	m_pBuf = pBuf;
	SetMin( 1 );
	Reset();
}

void CIDTable::Build( int iMax,int iPrivateMax )
{
	m_booUseBuf = false;
	SAFE_DELETE_ARRAY( m_pBuf );
	m_iMax = iMax + 1;	//因为最小是一，所以最大+1，以避免混淆;	
	m_pBuf = (void **)new LPSTR[m_iMax];
	SetMin( 1 );
	Reset();
}

void CIDTable::SetMin( int iMin )
{
	m_iMin = iMin;
	m_iCur = iMin;
}

int	CIDTable::GetFreeAutoID()
{
	int i;
	for(i=m_iCur;i<m_iMax;i++)
	{
		if( m_pBuf[i] == NULL )
		{
			m_iCur = i+1;
			return i;
		}
	}	

	for( i=m_iMin;i<m_iCur;i++)
	{
		if( m_pBuf[i] == NULL )
		{
			m_iCur = i+1;
			return i;
		}
	}
	return -1;
}

int CIDTable::Add( void * p )
{	
	int iID;
	int i;
	for(i=m_iCur;i<m_iMax;i++)
	{
		if( m_pBuf[i] == NULL )
		{
			iID = AddInto( i,p );
			m_iCur = i+1;
			return iID;
		}
	}	

	for( i=m_iMin;i<m_iCur;i++)
	{
		if( m_pBuf[i] == NULL )
		{
			iID = AddInto( i,p );
			m_iCur = i+1;
			return iID;
		}
	}

	char	szStr[4096];
	sprintf( szStr,"Error CIDTable::Add  Full %d",m_iMax );
	OutputError( szStr );
	return 0;	
}

void CIDTable::Space( int iSpace )//为了间隔人物与建筑
{
	m_iCur += iSpace;
	if( m_iCur > m_iMax )
	{
		m_iCur -= m_iMax;
	}
}

int CIDTable::AddInto( int i,void * p )
{	
	if( !ISIN( i,0,m_iMax-1) )
	{
		char	szStr[4096];
		sprintf( szStr,"Error CIDTable::AddInto beyond %d,%d",i,m_iMax );
		OutputError( szStr );
		return -1;
	}

	if( m_pBuf[i] == NULL )
	{
		m_pBuf[i] = p;
		m_iNum++;
		return i;
	}
	else
	{
		char	szStr[4096];
		sprintf( szStr,"Error CIDTable::AddInto overlay %d",i );
		OutputError( szStr );

	}
	return -1;
}

void * CIDTable::GetObj( int i )
{
	if( ISIN( i,0,m_iMax-1) )
	{
		return m_pBuf[i];
	}
	else
	{
		return NULL;
	}
	
}


void CIDTable::Remove( int i )
{
	if( !ISIN( i,0,m_iMax-1) )
	{
		char	szStr[4096];
		sprintf( szStr,"Error CIDTable::Remove beyond %d,%d",i,m_iMax );
		OutputError( szStr );
		return ;
	}

	m_pBuf[i] = NULL;
	m_iNum--;
}

void CIDTable::Reset()
{
	m_iNum = 0;
	m_iCur = m_iMin;
	if( m_pBuf )
	{
		for(int i=0;i<m_iMax;i++)
			m_pBuf[i] = NULL;
	}
}
