// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whfile_util.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 文件辅助功能
// CreationDate : 2004-09-16

#include "../inc/whfile_fcd.h"
#include "../inc/whfile.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whfilechangedetector
////////////////////////////////////////////////////////////////////
whfilechangedetector::whfilechangedetector()
: m_nLastTime(0)
{
}
whfilechangedetector::~whfilechangedetector()
{
}
int		whfilechangedetector::SetFile(const char *szFileName)
{
	WH_STRNCPY0(m_szFileName, szFileName);
	// 记录一下文件时间
	m_nLastTime	= whfile_getmtime(m_szFileName);
	return	0;
}
int		whfilechangedetector::Release()
{
	return	0;
}
bool	whfilechangedetector::IsChanged()
{
	time_t	t	= whfile_getmtime(m_szFileName);
	if( t>0 && t!=m_nLastTime )
	{
		m_nLastTime	= t;
		return	true;
	}
	return		false;
}

////////////////////////////////////////////////////////////////////
// 	whmultifilechangedetector
////////////////////////////////////////////////////////////////////
whmultifilechangedetector::whmultifilechangedetector()
{
	m_units.reserve(32);
	m_vectRst.reserve(m_units.capacity());
}
whmultifilechangedetector::~whmultifilechangedetector()
{
}
int		whmultifilechangedetector::AddFile(const char *szFileName, int nExt)
{
	if( m_setFNames.has(szFileName) )
	{
		// 已经有了
		return	1;
	}

	// 文件名加入列表
	UNIT_T	*pUnit	= m_units.push_back();
	pUnit->clear();
	pUnit->nLastTime	= whfile_getmtime(szFileName);
	pUnit->nExt			= nExt;
	WH_STRNCPY0(pUnit->szFileName, szFileName);
	m_setFNames.put(szFileName);

	return	0;
}
int		whmultifilechangedetector::Release()
{
	m_units.destroy();
	m_setFNames.clear();

	return	0;
}
whvector<whmultifilechangedetector::RST_T> *	whmultifilechangedetector::CheckChanged()
{
	m_vectRst.clear();

	for(size_t i=0;i<m_units.size();++i)
	{
		UNIT_T	&u	= m_units[i];
		time_t	t	= whfile_getmtime(u.szFileName);
		if( t!=0 && t!=u.nLastTime )
		{
			u.nLastTime			= t;
			RST_T	*pRst		= m_vectRst.push_back();
			pRst->pszFileName	= u.szFileName;
			pRst->nExt			= u.nExt;
		}
	}
	if( m_vectRst.size()>0 )
	{
		return	&m_vectRst;
	}
	return	NULL;
}
