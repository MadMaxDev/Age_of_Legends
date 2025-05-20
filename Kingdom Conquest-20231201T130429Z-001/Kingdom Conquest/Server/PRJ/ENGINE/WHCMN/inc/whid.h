// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whid.h
// Creator      : Wei Hua (魏华)
// Comment      : id分配、与指定数据对应的相关的功能
//              : 使用范例参考whid-tst.cpp
// CreationDate : 2003-05-15
// ChangeLOG    : 2005-01-12 取消原来的list而采用DList代替

#ifndef	__WHID_H__
#define	__WHID_H__

#include "whcmn_def.h"
#include "whDList.h"

namespace n_whcmn
{

// EXTNUM为附加整数数组的长度（可以用整数来存储指针）
template<int EXTNUM = 1>
class	whid
{
public:
	enum
	{
		MAXNUM			= 0x10000,
		MAXRAND			= 0x7FFF,
	};
	// 这个hashfunc是给外部使用本id的人用的
	struct	hashfunc
	{
		hashfunc() {}
		hashfunc(const hashfunc & hf) {}

		inline size_t	operator()(int id)	const
		{return id & 0xFFFF;}
	};
private:
	struct	UNIT_T
	{
		int	nID;						// 如果分配出去了，nID>0xFFFF；如果没有分配出去，nID<=0xFFFF，就是m_aIDNode的数组下标
		int	anExt[EXTNUM];				// 附加数据(这样可以实现简单的对应)
		inline void	clear()
		{
			nID	= -1;
			memset(anExt, 0, sizeof(anExt));
		}
	};
	typedef	whDList<UNIT_T>		DL_T;
	typedef	typename DL_T::node	node;	// 这个node包含了UNIT的数据部分
	int		m_nMaxNum;					// 最多的ID个数
	node	*m_aIDNode;					// ID数组
	int		m_nRand;					// 随机数
	DL_T	m_dlAvail;					// 可用的列表
	DL_T	m_dlAlloc;					// 已经分配的列表
public:
	whid();
	~whid();

	// 初始化
	// nMaxNum为最多的ID个数(不能超过65536个)
	int		Init(int nMaxNum = MAXNUM);
	// 终结
	int		Release();
	// 申请一个ID，返回附加指针（附加指针是一个int数组，长度为EXTNUM），可以在附加指针中填写需要关联的数据
	int *	AllocID(int *pnID);
	// 根据已有ID申请，返回附加指针（这个主要用于有一个中心服务器分配ID，个从属服务器根据中心服务器提供的ID在本机做ID到具体对应的对应）
	int *	AllocByID(int nID);
	// 释放ID
	int		FreeID(int nID);
	// 获取ID关联的附加数据(如果ID错误就返回NULL)
	int *	GetExtPtr(int nID) const;
};

template<int EXTNUM>
whid<EXTNUM>::whid()
: m_nMaxNum(0)
, m_aIDNode(NULL)
, m_nRand(0)
{
}
template<int EXTNUM>
whid<EXTNUM>::~whid()
{
	Release();
}

template<int EXTNUM>
int		whid<EXTNUM>::Init(int nMaxNum)
{
	if( nMaxNum>MAXNUM )
	{
		return	-1;
	}

	m_aIDNode	= new node [nMaxNum];	// 每个元素在构造的时候就已经初始化了
	assert(m_aIDNode);
	m_nMaxNum	= nMaxNum;

	// 初始化所有元素，并把它们加入avail列表
	for(int i=0;i<nMaxNum;i++)
	{
		m_aIDNode[i].data.nID	= i;
		m_dlAvail.AddToTail(&m_aIDNode[i]);
	}

	return	0;
}

template<int EXTNUM>
int		whid<EXTNUM>::Release()
{
	if( m_aIDNode==NULL )
	{
		// 应该已经Release过了，或者还没有初始化
		return	0;
	}
	m_dlAvail.reset();
	m_dlAlloc.reset();
	delete []	m_aIDNode;
	m_aIDNode	= NULL;
	return	0;
}

template<int EXTNUM>
int *	whid<EXTNUM>::AllocID(int *pnID)
{
	if( m_dlAvail.size() == 0 )
	{
		// 没有可分配的单元了
		return	NULL;
	}

	int		nIdx;
	node	*pNode	= m_dlAvail.begin();
	pNode->leave();
	m_dlAlloc.AddToHead(pNode);
	nIdx	= pNode->data.nID;

	m_nRand	++;
	if( m_nRand > MAXRAND )
	{
		m_nRand	= 1;
	}

	pNode->data.clear();
	pNode->data.nID	= nIdx | (m_nRand<<16);
	*pnID		= pNode->data.nID;

	return	pNode->data.anExt;
}

template<int EXTNUM>
int *	whid<EXTNUM>::AllocByID(int nID)
{
	int		nIdx	= nID & 0xFFFF;
	if( nIdx>=m_nMaxNum )
	{
		return	NULL;
	}
	node	*pNode	= &m_aIDNode[nIdx];
	if( pNode->nID > 0xFFFF )	// 如果超过0xFFFF就说明已经被分配了
	{
		// 没有这个ID
		return	NULL;
	}
	pNode->leave();				// 从avail队列中退出
	pNode->data.clear();		// 清理数据
	pNode->data.nID	= nID;		// 设置ID
	m_dlAlloc.AddToHead(pNode);	// 加入alloc队列

	return	pNode->data.anExt;
}
template<int EXTNUM>
int		whid<EXTNUM>::FreeID(int nID)
{
	if( nID<=0xFFFF )
	{
		return	-1;
	}
	int		nIdx	= nID & 0xFFFF;
	if( nIdx>=m_nMaxNum
	||  m_aIDNode[nIdx].data.nID != nID )
	{
		// 没有这个ID
		return	-1;
	}

	// 从alloc队列中退出
	m_aIDNode[nIdx].leave();
	// 设置ID，重新放入avail队列
	m_aIDNode[nIdx].data.nID	&= 0xFFFF;
	m_dlAvail.AddToHead(&m_aIDNode[nIdx]);

	return	0;
}
template<int EXTNUM>
int *	whid<EXTNUM>::GetExtPtr(int nID) const
{
	if( nID<=0xFFFF )
	{
		return	NULL;
	}
	int		nIdx	= nID & 0xFFFF;
	if( nIdx>=m_nMaxNum
	||  m_aIDNode[nIdx].data.nID != nID )
	{
		// 没有这个ID
		return	NULL;
	}

	return	m_aIDNode[nIdx].data.anExt;
}

}		// EOF namespace n_whcmn

#endif	// EOF __WHID_H__
