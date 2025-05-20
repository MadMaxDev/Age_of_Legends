// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whqueue.h
// Creator      : Wei Hua (魏华)
// Comment      : whsmpqueue简单的变长元素数据队列，从GFSmpQueue改编而来
//                可插入的单个单位尺寸在.cpp中定义为unitsize_t
//                注意(!!!!): 插入队列的元素加上单元头的最大长度不能超过nTotalSize的一半
//                否则赶上Head和Tail刚好在中间的情况就在也不能插入了
// CreationDate : 2003-05-21
// ChangeLOG    : 

// 设计说明：
// 1. 所有单元在队列内首位相接，连续排放，FIFO。
// 2. 最多会有m_nTotalSize-1的内存被使用，一个byte的浪费可以使m_nHead和m_nTail的关系简单化
// 另外，允许插入长度为0的数据，如果不想插入在上层判断即可。（因为长度为0有时是有意义的，比如有序号的空包）
// 3. 所有的Out如果有pnSize非空，那么如果*pnSize非0，就要判断要接收的数据长度是否大于该长度
// 2004-10-21 smpqueue是用Remember来做FreeLast

#ifndef	__WHQUEUE_H__
#define	__WHQUEUE_H__

#include <stdlib.h>
#include <assert.h>
#include "whlock.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 简单队列（每个元素可以不等长）
////////////////////////////////////////////////////////////////////
class	whsmpqueue
{
protected:
	size_t	m_nHead;								// 指向第一个已分配的位置
	size_t	m_nTail;								// 指向第一个可分配的位置(在插入过程中Tail不会和Head重合，所以最后会浪费一点空间)
	size_t	m_nLEnd;								// 当发生回绕，即Tail<Head时，LEnd指向物理顺序最后一个数据块结束的下一个位置
													// 可理解为LogicEnd或LinkEnd
	char	*m_pBuf;								// 数据缓冲区
	size_t	m_nTotalSize;							// 缓冲区的总长度
	size_t	m_nLogicMaxSize;						// 逻辑上的最大长度
	size_t	m_nSize;								// 当前已分配的大小(含所有的数据单元头)
	size_t	m_nUnitNum;								// 当前数据单元总数
	bool	m_bAcceptZeroLen;						// 是否接受长度为0的数据
	struct	REMEMBER_T
	{
		size_t	nUnitNum	;
		size_t	nSize		;
		size_t	nHead		;
		size_t	nTail		;
		size_t	nLEnd		;
		REMEMBER_T()
		: nUnitNum(0)
		, nSize(0)
		, nHead(0)
		, nTail(0)
		, nLEnd(0)
		{ }
	}m_remember;
public:
	inline size_t	GetSize() const
	{ return m_nSize; }								// 获取队列已分配内存大小
	inline size_t	GetUnitNum() const
	{ return m_nUnitNum; }							// 获取队列单元个数
	inline void		SetAcceptZeroLen(bool bSet)
	{ m_bAcceptZeroLen = bSet; }
	inline bool		GetAcceptZeroLen() const
	{ return m_bAcceptZeroLen; }
	size_t			GetMaxInSizse() const;			// 获得可以加入的最大长度
	// 为调试
	inline void		GetHTLE(size_t *pnHead, size_t *pnTail, size_t *pnLEnd) const
	{
		*pnHead	= m_nHead;
		*pnTail	= m_nTail;
		*pnLEnd	= m_nLEnd;
	}
	// m_nLogicMaxSize可以动态控制队列可以容纳的总数据长度
	inline int		GetLogicMaxSize() const
	{
		return	m_nLogicMaxSize;
	}
	inline void		SetLogicMaxSize(int nSize)
	{
		m_nLogicMaxSize	= nSize;
	}
public:
	whsmpqueue();
	~whsmpqueue();									// 里面会自动调用Release
	int		Init(size_t nTotalSize);				// 初始化队列
	int		Release();								// 终结(会在析构中自动调用)
	void	Clean();								// 清除队列中的所有数据
	void *	InAlloc(size_t nSize);					// 申请入队内存，自行填写内容，避免多申请拷贝一次
													// 注意(!!!!): 调用者保证不会使用nSize以外的区域
													// 这个是唯一危险的public操作，慎用慎用！！！！
	int		In(const void *pData, size_t nSize);
													// 拷贝入队
													// 注意(!!!!): 插入队列的元素加上单元头的最大长度不能超过nTotalSize的一半
													// 否则赶上Head和Tail刚好在中间的情况就在也不能插入了
	size_t	PeekSize() const;
													// 获得将要出队的下一条数据的长度
	void *	OutPeek(size_t *pnSize) const;			// *pnSize为0或一个足够大的值
													// 申请出队不释放。需要调用OutFree正式释放
	void *	OutFree(size_t *pnSize);
													// 申请出队(返回数据块大小)。直接从队列内存中读取数据。不用自己释放。
													// 注意(!!!!): 调用者保证在下一个队列(I/O)操作前使用完数据
	int		Out(void *pData, size_t *pnSize);		// 返回-1表示没有数据，0表示有数据(数据长度在*pnSize中)
													// 拷贝出队
	int		FreeN(int nNum);						// 一次释放nNum个单元，返回真正释放的数目
	int		In2(const void *pData0, size_t nSize0, const void *pData, size_t nSize);
	int		Out2(void *pData0, size_t nSize0, void *pData, size_t *pnSize);
	void	Remember();								// 记忆当前的数据位置
	void	RollBack();								// 反悔到记忆的位置，在Remember之后的所有插入全部失效
													// 注意这期间的操作只要没有相互覆盖就不会发生问题
};

// With Lock
// 注意：OutFree无法保证加锁，因为在没有被使用前可能就被Alloc覆盖了
// OutPeek只要保证使用时还没有被别的线程释放就没有这样的问题
// 也就是如果一个线程写队列，一个线程读队列，那么用Out/OutPeek+FreeN对In/InAlloc就没问题
class	whsmpqueueWL	: public whsmpqueue
{
private:
	whlock	m_lock;
	bool	m_bUseLock;
public:
	whsmpqueueWL()
	: m_bUseLock(true)
	{
	}
	inline void	setUseLock(bool bSet=true)
	{
		m_bUseLock	= bSet;
	}
	inline bool	getUseLock()
	{
		return	m_bUseLock;
	}
	inline void	lock()
	{
		if( m_bUseLock )
		{
			m_lock.lock();
		}
	}
	inline void	unlock()
	{
		if( m_bUseLock )
		{
			m_lock.unlock();
		}
	}
	inline void	ReInitLock()
	{
		if( m_bUseLock )
		{
			m_lock.reinit();
		}
	}
	inline void	Clean()
	{
		lock();
		whsmpqueue::Clean();
		unlock();
	}
	inline void *	InAlloc(size_t nSize)							// InAlloc不能用于多线程中，除非外界手动Lock、Copy、Unlock
	{
		assert(0);
		return	NULL;
	}
	int		In(const void *pData, size_t nSize)
	{
		lock();
		int		rst = whsmpqueue::In(pData, nSize);
		unlock();
		return	rst;
	}
	inline size_t	PeekSize()
	{
		lock();
		size_t	rst = whsmpqueue::PeekSize();
		unlock();
		return	rst;
	}
	inline void *	OutPeek(size_t *pnSize)							// 如果永远只有一个读N个写那么OutPeek就会是安全的
	{
		lock();
		void *	ptr = whsmpqueue::OutPeek(pnSize);
		unlock();
		return	ptr;
	}
	inline void *	OutFree(size_t *pnSize)
	{
		assert(0);
		return	NULL;
	}
	inline int		Out(void *pData, size_t *pnSize)
	{
		lock();
		int		rst = whsmpqueue::Out(pData, pnSize);
		unlock();
		return	rst;
	}
	inline int		In2(const void *pData0, size_t nSize0, const void *pData, size_t nSize)
	{
		lock();
		int		rst = whsmpqueue::In2(pData0, nSize0, pData, nSize);
		unlock();
		return	rst;
	}
	inline int		Out2(void *pData0, size_t nSize0, void *pData, size_t *pnSize)
	{
		lock();
		int		rst = whsmpqueue::Out2(pData0, nSize0, pData, pnSize);
		unlock();
		return	rst;
	}
	inline int		FreeN(int nNum)
	{
		lock();
		int		rst = whsmpqueue::FreeN(nNum);
		unlock();
		return	rst;
	}
	inline void		Remember()
	{
		lock();
		whsmpqueue::Remember();
		unlock();
	}
	inline void		RollBack()
	{
		lock();
		whsmpqueue::RollBack();
		unlock();
	}
};

// Can Lock
// 它一般用于需要自己控制lock/unlock的情况
// 比如想直接通过AllocFree获取并处理命令，那么就需要在前后加上lock/unlock
class	whsmpqueueCL	: public whsmpqueue
{
private:
	whlock	m_lock;
public:
	inline void	lock()
	{
		m_lock.lock();
	}
	inline void	unlock()
	{
		m_lock.unlock();
	}
	inline void	ReInitLock()
	{
		m_lock.reinit();
	}
	inline void	Clean()
	{
		m_lock.lock();
		whsmpqueue::Clean();
		m_lock.unlock();
	}
	inline int	Init(size_t nTotalSize)
	{
		ReInitLock();
		return	whsmpqueue::Init(nTotalSize);
	}
};

////////////////////////////////////////////////////////////////////
// 快速队列（每个元素必须不大于一定长度，但是会按照最大长度存储）
// 而且内部默认长度是4的倍数
////////////////////////////////////////////////////////////////////
class	whquickqueue
{
protected:
	int	m_nHead;								// 指向第一个已分配的位置()
	int	m_nTail;								// 指向第一个可分配的位置
	int	m_nCurPos, m_nCount;					// 指向当前被检索的位置(BeginGet和GetNext用)
	int	m_nUnitNum;								// 当前已经入队的单元总数

	char	* m_pBuf;								// 数据缓冲区
	int	m_nUnitSize;							// 一个单元含头部(整形的长度)的长度
	int	m_nTotalSize;							// 缓冲区的总长度 = m_nUnitSize * m_nMaxUnit

	int	m_nUnitDataSize;						// 一个单元数据部分的长度
	int	m_nMaxUnit;								// 最多存储的单元数量
	int	m_nLogicMaxUnit;						// 罗机上最多存储的单元数量
public:
	inline int	GetMaxUnit() const
	{ return	m_nMaxUnit; }
	inline int	GetLogicMaxUnit() const
	{ return	m_nLogicMaxUnit; }
	inline void	SetLogicMaxUnit(int nNum)
	{ m_nLogicMaxUnit	= nNum; }
	inline int	GetUnitDataSize() const
	{ return m_nUnitDataSize; }
	inline int	GetUnitNum() const
	{ return	m_nUnitNum; }
public:
	whquickqueue();
	~whquickqueue();								// 里面会自动调用Release
	int		Init(int nMaxUnit, int nUnitDataSize);// 初始化队列
	int		Release();								// 终结(会在析构中自动调用)
	void	Clean();								// 清除队列中的所有数据
	void *	InAlloc(int nSize=0);					// 申请入队内存，自行填写内容，避免多申请拷贝一次
													// nSize如果大于m_nUnitDataSize会返回NULL
													// nSize==0一定会申请成功
													// 注意(!!!!): 调用者保证不会使用m_nUnitDataSize以外的区域
													// 这个是唯一危险的public操作，慎用慎用！！！！
	int		In(const void *pData, int nSize);		// 拷贝入队，如果nSize超过m_nUnitDataSize会返回失败
	void *	OutPeek(int *pnSize) const;			// 申请出队不释放。需要调用OutFree正式释放
	void *	OutFree(int *pnSize);					// 申请出队。直接从队列内存中读取数据。不用自己释放。
													// 注意(!!!!): 调用者保证在下一个队列(I/O)操作前使用完数据
	int		Out(void *pData, int *pnSize);			// 返回-1表示没有数据
													// 拷贝出队
	int		FreeN(int nNum);						// 一次释放nNum个单元，返回真正释放的数目
	int		FreeLastN(int nNum);					// 一次释放尾部nNum个单元，返回真正释放的数目

	// 顺序浏览
	// 注意：这两个操作一定!!!!不可以!!!!和In/Out操作混用
	int		BeginGet();
	void *	GetNext(int *pnSize);
};

////////////////////////////////////////////////////////////////////
// 对象队列，和QickQueue类似，不过是模板的，单元数据类型固定，不用记长度了
// Alloc和Free是通过移动m_nHead, m_nTail两个索引完成的，没有内存移动
////////////////////////////////////////////////////////////////////
template <typename _Ty>
class	whvectorqueue
{
protected:
	_Ty		* m_pBuf;
	int	m_nUnitNum;								// 已经有的单元数量
	int	m_nHead, m_nTail;						// 指向头尾单元的序号(注意，是序号哦，可不是偏移)
	int	m_nCurPos, m_nCount;					// 指向当前被检索的位置(BeginGet和GetNext用)，和还要检索的数量

	int	m_nMaxUnit;								// 最多存储的单元数量
public:
	inline int	GetMaxUnit() const
	{ return	m_nMaxUnit; }
	inline int	GetUnitNum() const
	{ return	m_nUnitNum; }
	inline int	GetAvailNum() const					// 还可以装下多少个
	{ return	m_nMaxUnit-m_nUnitNum; }
	inline int	size() const
	{ return	GetUnitNum(); }
	// 顺序浏览
	// 注意：这两个操作一定!!!!不可以!!!!和In/Out操作混用
	inline void		BeginGet()
	{
		m_nCurPos	= m_nHead;
		m_nCount	= m_nUnitNum;
	}
	// 获得顺次数的第nIdx个数据(起始为0)
	inline void		BeginGet(int nIdx)
	{
		if( m_nUnitNum<=nIdx || nIdx<0 )
		{
			// 序号超界
			m_nCount	= 0;
			return;
		}
		m_nCurPos	= (m_nHead + nIdx) % m_nMaxUnit;
		m_nCount	= m_nUnitNum - nIdx;
	}
	inline _Ty *	GetNext()
	{
		if( m_nCount == 0 )
		{
			// 到了尽头
			return	0;
		}
		m_nCount	--;

		// 获得这个
		_Ty	* pRst	= m_pBuf + m_nCurPos;

		// 获得下一个位置
		m_nCurPos	++;
		if( m_nCurPos == m_nMaxUnit )
		{
			if( m_nTail != m_nMaxUnit )
			{
				// 只要m_nTail不指向总缓冲区的下一个字节(即回绕位置)，m_nCurPos就要回绕
				m_nCurPos	= 0;
			}
		}

		return	pRst;
	}
	// 获得顺次数的第nIdx个数据(起始为0)
	inline _Ty *	Get(int nIdx)
	{
		if( m_nUnitNum<=nIdx || nIdx<0 )
		{
			// 序号超界
			return	0;
		}

		nIdx	= (m_nHead + nIdx) % m_nMaxUnit;

		return	m_pBuf + nIdx;
	}
	// 获得第一个
	inline _Ty *	GetFirst()
	{
		if( m_nUnitNum>0 )
		{
			return	m_pBuf + m_nHead;
		}
		else
		{
			return	NULL;
		}
	}
	inline bool	IsReleased() const
	{
		return	m_pBuf == NULL;
	}
public:
	whvectorqueue();
	~whvectorqueue();
	int		Init(int nMaxUnit);					// 初始化队列
	int		Release();								// 终结(会在析构中自动调用)
	void	Clean();								// 清除队列中的所有数据
	_Ty *	Alloc(int *pnIdx=NULL);					// 分配一个单元(这个指针是稳定的的!!!!)
	int		FreeN(int nNum);						// 释放连续的nNum个单元(这里的释放可不作单元的析构哦)
	int		FreeLastN(int nNum);					// 从后向前释放
	bool	OutFree(_Ty & Data);					// 释放并弹出
	_Ty *	OutPeek();								// 不弹出，但是只获得最顶的数据指针
	// 这两个由外界保证可用
	inline void	Push(const _Ty & Data)
	{
		_Ty	*pData	= Alloc();
		if( pData )
		{
			*pData	= Data;
		}
	}
	inline _Ty	Pop()
	{
		_Ty		Data;
		bool	bRst	= OutFree(Data);
		assert(bRst);
		return	Data;
	}
};
template <typename _Ty>
whvectorqueue<_Ty>::whvectorqueue()
: m_pBuf		(0)
, m_nUnitNum	(0)
, m_nHead		(0)
, m_nTail		(0)
, m_nCurPos		(0)
, m_nCount		(0)
, m_nMaxUnit	(0)
{
}
template <typename _Ty>
whvectorqueue<_Ty>::~whvectorqueue()
{
	Release();
}
template <typename _Ty>
int		whvectorqueue<_Ty>::Init(int nMaxUnit)
{
	assert( !m_pBuf );

	m_pBuf		= new _Ty[nMaxUnit];
	if( !m_pBuf )
	{
		assert(0);
		return	-1;
	}
	m_nMaxUnit	= nMaxUnit;
	Clean();

	return	0;
}
template <typename _Ty>
int		whvectorqueue<_Ty>::Release()
{
	if( m_pBuf )
	{
		delete []	m_pBuf;
		m_pBuf	= 0;
	}
	return	0;
}
template <typename _Ty>
void	whvectorqueue<_Ty>::Clean()
{
	m_nHead		= 0;
	m_nTail		= 0;
	m_nCurPos	= 0;
	m_nCount	= 0;
	m_nUnitNum	= 0;
}
template <typename _Ty>
_Ty *	whvectorqueue<_Ty>::Alloc(int *pnIdx)
{
	if( m_nUnitNum == m_nMaxUnit )
	{
		// 分配光了
		// 此时m_nTail==m_nHead
		return	NULL;
	}

	if( m_nTail >= m_nHead )
	{
		if( m_nTail < m_nMaxUnit )
		{
			goto	GoodEnd;
		}
		else if( 0 < m_nHead )
		{
			m_nTail	= 0;
			goto	GoodEnd;
		}
	}
	else
	{
		// m_nTail在前面一定有空间
		goto	GoodEnd;
	}

	return	NULL;

GoodEnd:
	_Ty		*pRst;
	pRst		= m_pBuf + m_nTail;
	if( pnIdx )
	{
		*pnIdx	= m_nTail;
	}
	m_nTail		++;
	m_nUnitNum	++;

	return	pRst;
}
template <typename _Ty>
int		whvectorqueue<_Ty>::FreeN(int nNum)
{
	if( nNum<=0 )
	{
		return	0;
	}
	if( m_nUnitNum <= nNum )
	{
		// 这样就全部释放了啦
		nNum	= m_nUnitNum;
		Clean();
		return	nNum;
	}

	m_nHead		= (m_nHead + nNum) % m_nMaxUnit;
	m_nUnitNum	-= nNum;

	return	nNum;
}
template <typename _Ty>
int		whvectorqueue<_Ty>::FreeLastN(int nNum)
{
	if( nNum<=0 )
	{
		return	0;
	}
	if( m_nUnitNum <= nNum )
	{
		// 这样就全部释放了啦
		nNum	= m_nUnitNum;
		Clean();
		return	nNum;
	}
	m_nTail		-= nNum;
	if( m_nTail<=0 )
	{
		m_nTail	+= m_nMaxUnit;
	}
	m_nUnitNum	-= nNum;

	return	nNum;
}
template <typename _Ty>
bool	whvectorqueue<_Ty>::OutFree(_Ty & Data)
{
	if( m_nUnitNum == 0 )
	{
		return	false;
	}
	Data	= m_pBuf[m_nHead];
	FreeN(1);
	return	true;
}
template <typename _Ty>
_Ty *	whvectorqueue<_Ty>::OutPeek()
{
	if( m_nUnitNum == 0 )
	{
		return	NULL;
	}
	return	&m_pBuf[m_nHead];
}

////////////////////////////////////////////////////////////////////
// 高级队列
// 相对于smpqueue说的，主要是用于线程间数据交换
// 锁定模式比smpqueueWL/CL要更高效一些
// 而且Peek出来的数据也会持续有效直到显式释放
// 索引区和数据区分开
// 2005-07-05 还没有想清楚。暂时搁置吧。:(
////////////////////////////////////////////////////////////////////
class	whadvqueue
{
protected:
	struct	IDX_T
	{
		int		nSize;								// 数据长度
		char	*pData;								// 数据指针
	};
protected:
	IDX_T	*m_paIdx;								// 索引数组存储区
	char	*m_pData;								// 数据存储区
	int		m_nMaxNum;								// 最多的数据个数
	int		m_nMaxSize;								// 缓冲区的最大长度
	int		m_nHead;								// 第一个已分配单元的下标
	int		m_nToRead;								// 第一个可读单元的下标
	int		m_nTail;								// 可读的结尾
	int		m_nToWrite;								// 第一个可分配单元的下标
													// 注意：
													// if ToRead==Tail，则说明队列没有东西可读
													// if Head==ToWrite+1，则说明队列已满，不可再写入
													// 一般ToRead>=Head，ToWrite>=Tail
	int		m_nTotalNum;							// 数据单元总数
public:
	inline int	idxpp(int nIdx)
	{
		if( (++nIdx) >= m_nMaxNum )
		{
			return	0;
		}
		return	nIdx;
	}
	inline int	idxpn(int nIdx, int n)
	{
		assert(n <= m_nMaxNum);						// 逻辑上应该保证n不会大于m_nMaxNum
		nIdx	+= n;
		if( nIdx >= m_nMaxNum )
		{
			nIdx	-= m_nMaxNum;
		}
		return	nIdx;
	}
public:
	whadvqueue();
	~whadvqueue();
	int		Init(int nMaxNum, int nMaxSize);		// 初始化队列
	int		Release();								// 终结它
	void	Clean();								// 清除队列中的所有数据
	void *	InAlloc(int nSize);						// 申请入队内存，自行填写内容，避免多申请拷贝一次
													// 注意(!!!!): 调用者保证不会使用nSize以外的区域
													// 这个是唯一危险的public操作，慎用慎用！！！！
	int		In(const void *pData, int nSize);
													// 拷贝入队
													// 注意(!!!!): 插入队列的元素加上单元头的最大长度不能超过nTotalSize的一半
													// 否则赶上Head和Tail刚好在中间的情况就在也不能插入了
	// 所有的出队操作都需要修改
	int		PeekSize() const;						// 获得将要出队的下一条数据的长度
	void *	OutPeek(int *pnSize) const;				// *pnSize为0或一个足够大的值
													// 申请出队不释放。需要调用OutFree正式释放
	void *	OutFree(int *pnSize);					// 申请出队(返回数据块大小)。直接从队列内存中读取数据。不用自己释放。
													// 注意(!!!!): 调用者保证在下一个队列(I/O)操作前使用完数据
	int		Out(void *pData, size_t *pnSize);		// 返回-1表示没有数据，0表示有数据(数据长度在*pnSize中)
													// 拷贝出队
	int		FreeN(int nNum);						// 一次释放nNum个单元，返回真正释放的数目
	// 附加的真正的释放函数
	// 只有这个函数将来才需要和各个In函数做互斥
	void	RealDoOut()
	{
		m_nHead	= m_nToRead;						// 这样就有空地方In了
	}
	void	RealDoIn()
	{
		m_nTail	= m_nToWrite;						// 这样就有东西Out了
	}
};


}		// EOF namespace n_whcmn

#endif	// EOF __WHQUEUE_H__
