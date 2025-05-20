// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtimequeue.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 时间队列(可以放置间隔达49.7天的事件)
// CreationDate : 2005-04-11
// ChangeLOG    : 2005-09-13 whtimequeue::GetUnitBeforeTime中对未超时的判断从原来的>=改为>，这样在本tick内设置为当前tick就一定会被调用
//              : 2008-07-14 增加了whtimequeue::Del中的保护措施，以便让set内容被破坏时也可以尽量删除。

#include "../inc/whtimequeue.h"

using namespace n_whcmn;

whtimequeue::whtimequeue()
: m_punits(NULL)
, m_nRotate(0)
{
}
whtimequeue::~whtimequeue()
{
	Release();
}
int		whtimequeue::Init(INFO_T *pInfo)
{
	if(m_punits!=NULL)
	{
		assert(0);
		return	-1;
	}
	m_punits	= new whsmpunitallocator(pInfo->nUnitLen, pInfo->nChunkSize);
	assert(m_punits);
	memcpy(&m_info, pInfo, sizeof(m_info));
	m_nRotate	= 0;
	return	0;
}
int		whtimequeue::Release()
{
	m_idset.clear();
	if( m_punits )
	{
		delete		m_punits;
		m_punits	= NULL;
	}
	return	0;
}
void	whtimequeue::Clear()
{
	m_punits->clear();
	m_idset.clear();
}
int 	whtimequeue::Add(whtick_t t, void *pUnit, ID_T *pID)
{
	void	*pUnitRef;
	int	rst	= AddGetRef(t, &pUnitRef, pID);
	if( rst<0 )
	{
		return	rst;
	}
	memcpy(pUnitRef, pUnit, m_info.nUnitLen);
	return	0;
}
int 	whtimequeue::AddGetRef(whtick_t t, void **ppUnit, ID_T *pID)
{
	assert(pID);
	ID_T	&id	= *pID;
	id.quit();	// 为了保险，quit一下
	char	*pszBuf;
	// 申请idx
	id.t	= t;
	id.idx	= m_punits->AllocUnit(pszBuf);
	id.pHost= NULL;
	if( id.idx<0 )
	{
		return	-1;
	}
	*ppUnit	= pszBuf;
	// 加入set
	idset_t::iterator	it	= m_idset.find(id);
	if( it != m_idset.end() )
	{
		m_punits->FreeUnit(id.idx);
		assert(0);
		return	-2;
	}
	id.nRotate	= m_nRotate++;
	// 先插入，pHost要在后面设置，这样通过检索出来的id就没有pHost，也就不会自己退出了
	m_idset.insert(id);
	// 给外面的要带上这个
	id.pHost	= this;

	return	0;
}
int		whtimequeue::TimeMove(whtimequeue::ID_T *pID, whtick_t newt)
{
	assert(pID->pHost);
	assert(pID->idx >= 0);
	// 查找
	idset_t::iterator	it	= m_idset.find(*pID);
	if( it == m_idset.end() )
	{
		// 没有这个
		return		-1;
	}
	// 删除旧的
	m_idset.erase(it);
	// 插入新的
	pID->t		= newt;
	pID->pHost	= NULL;
	m_idset.insert(*pID);
	pID->pHost	= this;
	return	0;
}
int		whtimequeue::Del(ID_T &id)
{
	if( id.idx<0 )
	{
		// 没有这个
		// 为了保险还是要清除host，以免它自己调用quit还会到这里来
		id.pHost	= NULL;
		return		-1;
	}
	idset_t::iterator	it	= m_idset.find(id);
	if( it == m_idset.end() )
	{
		// 没有这个
		if( id.pHost )
		{
			// 为了保险还是要清除host，以免它自己调用quit还会到这里来（这个可能是因为别的地方先删除了）
			id.pHost	= NULL;
		}
		// 判断是否是begin
		it	= m_idset.begin();
		if( m_idset.size()>0 && (*it)==id )
		{
			// 这样就直接到外面去删除了，这样可以缓解由于set内容被破坏导致的死循环
			// 记录一下日志
			WHCMN_LOG_WRITEFMT(WHCMN_LOG_ID_FATAL, WHCMN_LOG_STD_HDR(1019, TQ_ERR)"whtimequeue::Del Fail,%d,%d,%d", id.t, id.nRotate, id.idx);
		}
		else
		{
			return		-1;
		}
	}

	m_idset.erase(it);
	m_punits->FreeUnit(id.idx);
	// 为了保险直接清除
	id.clear();
	return	0;
}
int		whtimequeue::GetUnitBeforeTime(whtick_t t, void **ppUnit, ID_T *pID)
{
	idset_t::iterator	it	= m_idset.begin();
	if( it == m_idset.end() )
	{
		// 没有东西了
		return	-1;
	}
	if( wh_tickcount_diff( (*it).t, t) > 0 )
	{
		// 时间没到(期望到的时间比现在的时间晚)(只要相等就认为到时间了)
		return	-2;
	}
	if( pID )
	{
		*pID	= (*it);
		// (2005-07-12)查找出来的pID->pHost应该一定是空的
		assert(pID->pHost == NULL);
	}
	*ppUnit	= GetUnitPtr((*it).idx);
	assert( (*ppUnit)!=NULL );
	return	0;
}
