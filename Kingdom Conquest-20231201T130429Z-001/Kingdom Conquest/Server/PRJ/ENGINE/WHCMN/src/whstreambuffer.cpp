// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whstreambuffer.cpp
// Creator: Wei Hua (魏华)
// Comment: 纯数据流缓冲
// CreationDate: 2003-10-06

#include "../inc/whstreambuffer.h"
#include <string.h>
#include <assert.h>

using namespace n_whcmn;

whstreambuffer::whstreambuffer()
: m_pBuf(NULL)
, m_nMaxSize(0)
, m_nSize(0)
, m_nBegin(0)
, m_nScrollSize(0)
{
}
whstreambuffer::~whstreambuffer()
{
	Release();
}
int		whstreambuffer::Init(size_t nMaxSize, size_t nTailPadding, size_t nScrollSize)
{
	m_nMaxSize		= nMaxSize;
	m_nScrollSize	= nScrollSize;
	assert(m_pBuf==NULL);
	m_pBuf			= new char [nMaxSize + nTailPadding];
	Clean();
	return	0;
}
int		whstreambuffer::Release()
{
	if( m_pBuf )
	{
		delete []	m_pBuf;
		m_pBuf		= NULL;
		m_nMaxSize	= 0;
		m_nSize		= 0;
		m_nBegin	= 0;
		m_nScrollSize	= 0;
	}
	return	0;
}
void	whstreambuffer::Clean()
{
	m_nSize		= 0;
	m_nBegin	= 0;
}
void *	whstreambuffer::InAlloc(size_t nSize)
{
	if( GetSizeLeft()<nSize )
	{
		return	NULL;
	}
	assert(m_pBuf);

	char		*pBuf = m_pBuf + m_nBegin + m_nSize;
	m_nSize		+= nSize;
	return		pBuf;
}
int		whstreambuffer::In(const void *pData, size_t nSize)
{
	void	*pBuf = InAlloc(nSize);
	if( !pBuf )
	{
		return	-1;
	}
	memcpy(pBuf, pData, nSize);
	return		0;
}
int		whstreambuffer::Out(void *pData, size_t *pnSize)
{
	if( m_nSize==0 )
	{
		*pnSize	= 0;
		return	-1;
	}

	// 最大不能超过已有的数据长度
	if( (*pnSize)>=m_nSize )
	{
		(*pnSize)	= m_nSize;
	}
	memcpy(pData, GetBuf(), *pnSize);
	
	Out(*pnSize);

	return		0;
}
void	whstreambuffer::Out(size_t nSize)
{
	if( nSize>=m_nSize )
	{
		Clean();
		return;
	}
	m_nSize		-= nSize;
	m_nBegin	+= nSize;
	if( m_nBegin>=m_nScrollSize )
	{
		// 超过规定长度
		memmove(m_pBuf, m_pBuf+m_nBegin, m_nSize);
		m_nBegin	= 0;
	}
}

