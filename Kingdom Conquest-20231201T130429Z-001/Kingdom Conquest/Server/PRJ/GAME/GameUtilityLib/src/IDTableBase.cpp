///////////////////////////
//	2000.6.1	Create by  soloman ;
///////////////////////////
#include <stdio.h>
#include <assert.h>
#include "../inc/IDTableBase.h"
#include "../inc/LogMngInf.h"
#include "../inc/misc.h"


CIDTableBase::CIDTableBase()
{
	m_iMax = 0;
	m_iMin = 0;
	m_iCurMax = 0;
	m_iCur = 0;
	m_iNum = 0;
	m_pLogName = NULL;
}

CIDTableBase::~CIDTableBase()
{
	MyDelete( m_pLogName );
}

void	CIDTableBase::OutputError( const char * sz )
{
	char	szStr[4096];
	

	if( m_pLogName )
	{
		sprintf( szStr,"Error,%s:%s",m_pLogName,sz );
	}
	else
	{
		sprintf( szStr,"Error,pure:%s",sz );
	}
	
	OutputErrorLog(  szStr );
}

