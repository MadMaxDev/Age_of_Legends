#include "stdafx.h"
#include "BinStream.h"
#include "io.h"
#include <assert.h>

BinStream::BinStream( const wchar_t* _Path , const wchar_t* _Mode )
{
	m_pFile = _wfopen( _Path , _Mode );
}

BinStream::~BinStream()
{
	if( m_pFile != NULL )
	{
		fclose( m_pFile );
		m_pFile = NULL;
	}
}

int BinStream::Size()
{
	if( m_pFile != NULL )
	{
		return filelength( fileno( (FILE*)(void*)m_pFile ) );
	}
	return 0;
}

void BinStream::Read( void* _pBuffer , int _Size )
{
	if( m_pFile != NULL )
	{
		int s = (int)fread( _pBuffer , 1 , _Size , m_pFile );
		assert( s == _Size );
	}
}

void BinStream::Skip( int _Size , int _Mode )
{
	if( m_pFile != NULL )
	{
		fseek( m_pFile , _Size , _Mode );
	}		
}

void BinStream::Write( const void* _pBuffer , int _Size )
{
	if( m_pFile != NULL )
	{
		int s = (int)fwrite( _pBuffer , 1 , _Size , m_pFile );
		assert( s == _Size );
	}	
}
