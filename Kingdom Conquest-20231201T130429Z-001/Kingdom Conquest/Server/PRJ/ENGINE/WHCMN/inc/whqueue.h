// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whqueue.h
// Creator      : Wei Hua (κ��)
// Comment      : whsmpqueue�򵥵ı䳤Ԫ�����ݶ��У���GFSmpQueue�ı����
//                �ɲ���ĵ�����λ�ߴ���.cpp�ж���Ϊunitsize_t
//                ע��(!!!!): ������е�Ԫ�ؼ��ϵ�Ԫͷ����󳤶Ȳ��ܳ���nTotalSize��һ��
//                �������Head��Tail�պ����м���������Ҳ���ܲ�����
// CreationDate : 2003-05-21
// ChangeLOG    : 

// ���˵����
// 1. ���е�Ԫ�ڶ�������λ��ӣ������ŷţ�FIFO��
// 2. ������m_nTotalSize-1���ڴ汻ʹ�ã�һ��byte���˷ѿ���ʹm_nHead��m_nTail�Ĺ�ϵ�򵥻�
// ���⣬������볤��Ϊ0�����ݣ��������������ϲ��жϼ��ɡ�����Ϊ����Ϊ0��ʱ��������ģ���������ŵĿհ���
// 3. ���е�Out�����pnSize�ǿգ���ô���*pnSize��0����Ҫ�ж�Ҫ���յ����ݳ����Ƿ���ڸó���
// 2004-10-21 smpqueue����Remember����FreeLast

#ifndef	__WHQUEUE_H__
#define	__WHQUEUE_H__

#include <stdlib.h>
#include <assert.h>
#include "whlock.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// �򵥶��У�ÿ��Ԫ�ؿ��Բ��ȳ���
////////////////////////////////////////////////////////////////////
class	whsmpqueue
{
protected:
	size_t	m_nHead;								// ָ���һ���ѷ����λ��
	size_t	m_nTail;								// ָ���һ���ɷ����λ��(�ڲ��������Tail�����Head�غϣ����������˷�һ��ռ�)
	size_t	m_nLEnd;								// ���������ƣ���Tail<Headʱ��LEndָ������˳�����һ�����ݿ��������һ��λ��
													// �����ΪLogicEnd��LinkEnd
	char	*m_pBuf;								// ���ݻ�����
	size_t	m_nTotalSize;							// ���������ܳ���
	size_t	m_nLogicMaxSize;						// �߼��ϵ���󳤶�
	size_t	m_nSize;								// ��ǰ�ѷ���Ĵ�С(�����е����ݵ�Ԫͷ)
	size_t	m_nUnitNum;								// ��ǰ���ݵ�Ԫ����
	bool	m_bAcceptZeroLen;						// �Ƿ���ܳ���Ϊ0������
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
	{ return m_nSize; }								// ��ȡ�����ѷ����ڴ��С
	inline size_t	GetUnitNum() const
	{ return m_nUnitNum; }							// ��ȡ���е�Ԫ����
	inline void		SetAcceptZeroLen(bool bSet)
	{ m_bAcceptZeroLen = bSet; }
	inline bool		GetAcceptZeroLen() const
	{ return m_bAcceptZeroLen; }
	size_t			GetMaxInSizse() const;			// ��ÿ��Լ������󳤶�
	// Ϊ����
	inline void		GetHTLE(size_t *pnHead, size_t *pnTail, size_t *pnLEnd) const
	{
		*pnHead	= m_nHead;
		*pnTail	= m_nTail;
		*pnLEnd	= m_nLEnd;
	}
	// m_nLogicMaxSize���Զ�̬���ƶ��п������ɵ������ݳ���
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
	~whsmpqueue();									// ������Զ�����Release
	int		Init(size_t nTotalSize);				// ��ʼ������
	int		Release();								// �ս�(�����������Զ�����)
	void	Clean();								// ��������е���������
	void *	InAlloc(size_t nSize);					// ��������ڴ棬������д���ݣ���������뿽��һ��
													// ע��(!!!!): �����߱�֤����ʹ��nSize���������
													// �����ΨһΣ�յ�public�������������ã�������
	int		In(const void *pData, size_t nSize);
													// �������
													// ע��(!!!!): ������е�Ԫ�ؼ��ϵ�Ԫͷ����󳤶Ȳ��ܳ���nTotalSize��һ��
													// �������Head��Tail�պ����м���������Ҳ���ܲ�����
	size_t	PeekSize() const;
													// ��ý�Ҫ���ӵ���һ�����ݵĳ���
	void *	OutPeek(size_t *pnSize) const;			// *pnSizeΪ0��һ���㹻���ֵ
													// ������Ӳ��ͷš���Ҫ����OutFree��ʽ�ͷ�
	void *	OutFree(size_t *pnSize);
													// �������(�������ݿ��С)��ֱ�ӴӶ����ڴ��ж�ȡ���ݡ������Լ��ͷš�
													// ע��(!!!!): �����߱�֤����һ������(I/O)����ǰʹ��������
	int		Out(void *pData, size_t *pnSize);		// ����-1��ʾû�����ݣ�0��ʾ������(���ݳ�����*pnSize��)
													// ��������
	int		FreeN(int nNum);						// һ���ͷ�nNum����Ԫ�����������ͷŵ���Ŀ
	int		In2(const void *pData0, size_t nSize0, const void *pData, size_t nSize);
	int		Out2(void *pData0, size_t nSize0, void *pData, size_t *pnSize);
	void	Remember();								// ���䵱ǰ������λ��
	void	RollBack();								// ���ڵ������λ�ã���Remember֮������в���ȫ��ʧЧ
													// ע�����ڼ�Ĳ���ֻҪû���໥���ǾͲ��ᷢ������
};

// With Lock
// ע�⣺OutFree�޷���֤��������Ϊ��û�б�ʹ��ǰ���ܾͱ�Alloc������
// OutPeekֻҪ��֤ʹ��ʱ��û�б�����߳��ͷž�û������������
// Ҳ�������һ���߳�д���У�һ���̶߳����У���ô��Out/OutPeek+FreeN��In/InAlloc��û����
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
	inline void *	InAlloc(size_t nSize)							// InAlloc�������ڶ��߳��У���������ֶ�Lock��Copy��Unlock
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
	inline void *	OutPeek(size_t *pnSize)							// �����Զֻ��һ����N��д��ôOutPeek�ͻ��ǰ�ȫ��
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
// ��һ��������Ҫ�Լ�����lock/unlock�����
// ������ֱ��ͨ��AllocFree��ȡ�����������ô����Ҫ��ǰ�����lock/unlock
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
// ���ٶ��У�ÿ��Ԫ�ر��벻����һ�����ȣ����ǻᰴ����󳤶ȴ洢��
// �����ڲ�Ĭ�ϳ�����4�ı���
////////////////////////////////////////////////////////////////////
class	whquickqueue
{
protected:
	int	m_nHead;								// ָ���һ���ѷ����λ��()
	int	m_nTail;								// ָ���һ���ɷ����λ��
	int	m_nCurPos, m_nCount;					// ָ��ǰ��������λ��(BeginGet��GetNext��)
	int	m_nUnitNum;								// ��ǰ�Ѿ���ӵĵ�Ԫ����

	char	* m_pBuf;								// ���ݻ�����
	int	m_nUnitSize;							// һ����Ԫ��ͷ��(���εĳ���)�ĳ���
	int	m_nTotalSize;							// ���������ܳ��� = m_nUnitSize * m_nMaxUnit

	int	m_nUnitDataSize;						// һ����Ԫ���ݲ��ֵĳ���
	int	m_nMaxUnit;								// ���洢�ĵ�Ԫ����
	int	m_nLogicMaxUnit;						// �޻������洢�ĵ�Ԫ����
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
	~whquickqueue();								// ������Զ�����Release
	int		Init(int nMaxUnit, int nUnitDataSize);// ��ʼ������
	int		Release();								// �ս�(�����������Զ�����)
	void	Clean();								// ��������е���������
	void *	InAlloc(int nSize=0);					// ��������ڴ棬������д���ݣ���������뿽��һ��
													// nSize�������m_nUnitDataSize�᷵��NULL
													// nSize==0һ��������ɹ�
													// ע��(!!!!): �����߱�֤����ʹ��m_nUnitDataSize���������
													// �����ΨһΣ�յ�public�������������ã�������
	int		In(const void *pData, int nSize);		// ������ӣ����nSize����m_nUnitDataSize�᷵��ʧ��
	void *	OutPeek(int *pnSize) const;			// ������Ӳ��ͷš���Ҫ����OutFree��ʽ�ͷ�
	void *	OutFree(int *pnSize);					// ������ӡ�ֱ�ӴӶ����ڴ��ж�ȡ���ݡ������Լ��ͷš�
													// ע��(!!!!): �����߱�֤����һ������(I/O)����ǰʹ��������
	int		Out(void *pData, int *pnSize);			// ����-1��ʾû������
													// ��������
	int		FreeN(int nNum);						// һ���ͷ�nNum����Ԫ�����������ͷŵ���Ŀ
	int		FreeLastN(int nNum);					// һ���ͷ�β��nNum����Ԫ�����������ͷŵ���Ŀ

	// ˳�����
	// ע�⣺����������һ��!!!!������!!!!��In/Out��������
	int		BeginGet();
	void *	GetNext(int *pnSize);
};

////////////////////////////////////////////////////////////////////
// ������У���QickQueue���ƣ�������ģ��ģ���Ԫ�������͹̶������üǳ�����
// Alloc��Free��ͨ���ƶ�m_nHead, m_nTail����������ɵģ�û���ڴ��ƶ�
////////////////////////////////////////////////////////////////////
template <typename _Ty>
class	whvectorqueue
{
protected:
	_Ty		* m_pBuf;
	int	m_nUnitNum;								// �Ѿ��еĵ�Ԫ����
	int	m_nHead, m_nTail;						// ָ��ͷβ��Ԫ�����(ע�⣬�����Ŷ���ɲ���ƫ��)
	int	m_nCurPos, m_nCount;					// ָ��ǰ��������λ��(BeginGet��GetNext��)���ͻ�Ҫ����������

	int	m_nMaxUnit;								// ���洢�ĵ�Ԫ����
public:
	inline int	GetMaxUnit() const
	{ return	m_nMaxUnit; }
	inline int	GetUnitNum() const
	{ return	m_nUnitNum; }
	inline int	GetAvailNum() const					// ������װ�¶��ٸ�
	{ return	m_nMaxUnit-m_nUnitNum; }
	inline int	size() const
	{ return	GetUnitNum(); }
	// ˳�����
	// ע�⣺����������һ��!!!!������!!!!��In/Out��������
	inline void		BeginGet()
	{
		m_nCurPos	= m_nHead;
		m_nCount	= m_nUnitNum;
	}
	// ���˳�����ĵ�nIdx������(��ʼΪ0)
	inline void		BeginGet(int nIdx)
	{
		if( m_nUnitNum<=nIdx || nIdx<0 )
		{
			// ��ų���
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
			// ���˾�ͷ
			return	0;
		}
		m_nCount	--;

		// ������
		_Ty	* pRst	= m_pBuf + m_nCurPos;

		// �����һ��λ��
		m_nCurPos	++;
		if( m_nCurPos == m_nMaxUnit )
		{
			if( m_nTail != m_nMaxUnit )
			{
				// ֻҪm_nTail��ָ���ܻ���������һ���ֽ�(������λ��)��m_nCurPos��Ҫ����
				m_nCurPos	= 0;
			}
		}

		return	pRst;
	}
	// ���˳�����ĵ�nIdx������(��ʼΪ0)
	inline _Ty *	Get(int nIdx)
	{
		if( m_nUnitNum<=nIdx || nIdx<0 )
		{
			// ��ų���
			return	0;
		}

		nIdx	= (m_nHead + nIdx) % m_nMaxUnit;

		return	m_pBuf + nIdx;
	}
	// ��õ�һ��
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
	int		Init(int nMaxUnit);					// ��ʼ������
	int		Release();								// �ս�(�����������Զ�����)
	void	Clean();								// ��������е���������
	_Ty *	Alloc(int *pnIdx=NULL);					// ����һ����Ԫ(���ָ�����ȶ��ĵ�!!!!)
	int		FreeN(int nNum);						// �ͷ�������nNum����Ԫ(������ͷſɲ�����Ԫ������Ŷ)
	int		FreeLastN(int nNum);					// �Ӻ���ǰ�ͷ�
	bool	OutFree(_Ty & Data);					// �ͷŲ�����
	_Ty *	OutPeek();								// ������������ֻ����������ָ��
	// ����������籣֤����
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
		// �������
		// ��ʱm_nTail==m_nHead
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
		// m_nTail��ǰ��һ���пռ�
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
		// ������ȫ���ͷ�����
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
		// ������ȫ���ͷ�����
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
// �߼�����
// �����smpqueue˵�ģ���Ҫ�������̼߳����ݽ���
// ����ģʽ��smpqueueWL/CLҪ����ЧһЩ
// ����Peek����������Ҳ�������Чֱ����ʽ�ͷ�
// ���������������ֿ�
// 2005-07-05 ��û�����������ʱ���ðɡ�:(
////////////////////////////////////////////////////////////////////
class	whadvqueue
{
protected:
	struct	IDX_T
	{
		int		nSize;								// ���ݳ���
		char	*pData;								// ����ָ��
	};
protected:
	IDX_T	*m_paIdx;								// ��������洢��
	char	*m_pData;								// ���ݴ洢��
	int		m_nMaxNum;								// �������ݸ���
	int		m_nMaxSize;								// ����������󳤶�
	int		m_nHead;								// ��һ���ѷ��䵥Ԫ���±�
	int		m_nToRead;								// ��һ���ɶ���Ԫ���±�
	int		m_nTail;								// �ɶ��Ľ�β
	int		m_nToWrite;								// ��һ���ɷ��䵥Ԫ���±�
													// ע�⣺
													// if ToRead==Tail����˵������û�ж����ɶ�
													// if Head==ToWrite+1����˵������������������д��
													// һ��ToRead>=Head��ToWrite>=Tail
	int		m_nTotalNum;							// ���ݵ�Ԫ����
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
		assert(n <= m_nMaxNum);						// �߼���Ӧ�ñ�֤n�������m_nMaxNum
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
	int		Init(int nMaxNum, int nMaxSize);		// ��ʼ������
	int		Release();								// �ս���
	void	Clean();								// ��������е���������
	void *	InAlloc(int nSize);						// ��������ڴ棬������д���ݣ���������뿽��һ��
													// ע��(!!!!): �����߱�֤����ʹ��nSize���������
													// �����ΨһΣ�յ�public�������������ã�������
	int		In(const void *pData, int nSize);
													// �������
													// ע��(!!!!): ������е�Ԫ�ؼ��ϵ�Ԫͷ����󳤶Ȳ��ܳ���nTotalSize��һ��
													// �������Head��Tail�պ����м���������Ҳ���ܲ�����
	// ���еĳ��Ӳ�������Ҫ�޸�
	int		PeekSize() const;						// ��ý�Ҫ���ӵ���һ�����ݵĳ���
	void *	OutPeek(int *pnSize) const;				// *pnSizeΪ0��һ���㹻���ֵ
													// ������Ӳ��ͷš���Ҫ����OutFree��ʽ�ͷ�
	void *	OutFree(int *pnSize);					// �������(�������ݿ��С)��ֱ�ӴӶ����ڴ��ж�ȡ���ݡ������Լ��ͷš�
													// ע��(!!!!): �����߱�֤����һ������(I/O)����ǰʹ��������
	int		Out(void *pData, size_t *pnSize);		// ����-1��ʾû�����ݣ�0��ʾ������(���ݳ�����*pnSize��)
													// ��������
	int		FreeN(int nNum);						// һ���ͷ�nNum����Ԫ�����������ͷŵ���Ŀ
	// ���ӵ��������ͷź���
	// ֻ�����������������Ҫ�͸���In����������
	void	RealDoOut()
	{
		m_nHead	= m_nToRead;						// �������пյط�In��
	}
	void	RealDoIn()
	{
		m_nTail	= m_nToWrite;						// �������ж���Out��
	}
};


}		// EOF namespace n_whcmn

#endif	// EOF __WHQUEUE_H__
