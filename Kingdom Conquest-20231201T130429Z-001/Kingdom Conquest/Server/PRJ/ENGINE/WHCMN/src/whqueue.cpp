// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whqueue.cpp
// Creator      : Wei Hua (κ��)
// Comment      : whsmpqueue�򵥵ı䳤Ԫ�����ݶ��У���whsmpqueue�ı����
//                �ɲ���ĵ�����λ�ߴ���.cpp�ж���Ϊunitsize_t��Ŀǰ��u_short
// ע��(!!!!)   : ������е�Ԫ�ؼ��ϵ�Ԫͷ����󳤶Ȳ��ܳ���nTotalSize��һ��
//                �������Head��Tail�պ����м���������Ҳ���ܲ�����
// CreationDate : 2003-05-21
// ChangeLOG    : 2003-06-27
//                ����QickQueue���������Out�ͷ���Ϻ��Զ�������ָ���嵽0��Ĺ��ܡ�
//                2005-02-24 unitsize_t��u_short��Ϊu_long
//                2005-07-01 whsmpqueue::Release��ȡ����Clean.��������Ӧ��û�б�Ҫ��ô���ɡ�m_pBuf��ɾ������Щ���ݻ���ɶ���塣
//                2005-07-15 whsmpqueue::OutFree������������ߴ�̫С���������ݵĴ�ʩ���������������������bug����ϲ㻺����������й�����

// ���˵����
// ���е�Ԫ�ڶ�������λ��ӣ������ŷţ�FIFO��
// ������m_nTotalSize-1���ڴ汻ʹ�ã�һ��byte���˷ѿ���ʹm_nHead��m_nTail�Ĺ�ϵ�򵥻�

#include "../inc/whqueue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

using namespace n_whcmn;

typedef	unsigned int	unitsize_t;
#pragma pack(1)
struct	SQ_UNIT
{
	unitsize_t	size;
	char		data[1];
};
#pragma pack()
#define	SQ_UNIT_HDR_SIZE	(sizeof(SQ_UNIT)-1)
#define	QQ_UNIT_LEN(dsize)	(sizeof(int) + dsize)

////////////////////////////////////////////////////////////////////
// �򵥶��У�ÿ��Ԫ�ؿ��Բ��ȳ���
////////////////////////////////////////////////////////////////////
whsmpqueue::whsmpqueue()
: m_nHead			(0)
, m_nTail			(0)
, m_nLEnd			(0)
, m_pBuf			(NULL)
, m_nTotalSize		(0)
, m_nLogicMaxSize	(0)
, m_nSize			(0)
, m_nUnitNum		(0)
, m_bAcceptZeroLen	(false)
{
}
whsmpqueue::~whsmpqueue()
{
	Release();
}
int		whsmpqueue::Init(size_t nTotalSize)
{
	assert( !m_pBuf );

	m_pBuf	= new char[nTotalSize];
	if( !m_pBuf )
	{
		return	-1;
	}

	m_nTotalSize	= nTotalSize;
	m_nLogicMaxSize	= nTotalSize;
	Clean();

	return	0;
}
int		whsmpqueue::Release()
{
	// ��ʱ������˵���Clean();
	// Clean(); 2005-07-01 ��������Ӧ��û�б�Ҫ��ô���ɡ�m_pBuf��ɾ������Щ���ݻ���ɶ���塣
	if( m_pBuf )
	{
		delete []	m_pBuf;
		m_pBuf		= NULL;
	}
	return	0;
}
void	whsmpqueue::Clean()
{
	m_nUnitNum	= 0;
	m_nSize		= 0;
	m_nHead		= 0;
	m_nTail		= 0;
	m_nLEnd		= 0;
	Remember();
}
size_t	whsmpqueue::GetMaxInSizse() const
{
	if( m_nTail>=m_nHead )
	{
		return	m_nTotalSize - 1 - m_nTail - SQ_UNIT_HDR_SIZE;
	}
	else
	{
		return	m_nHead - 1 - m_nTail - SQ_UNIT_HDR_SIZE;
	}
}
void *	whsmpqueue::InAlloc(size_t nSize)
{
	size_t	nUnitSize;
	nUnitSize	= SQ_UNIT_HDR_SIZE + nSize;

	if(nUnitSize >= m_nTotalSize/2-1)
	{
		assert(0);
		return	NULL;
	}
	if( nUnitSize+m_nSize >= m_nLogicMaxSize )
	{
		// �������߼����ֵ
		return	NULL;
	}

	if( m_nTail>=m_nHead )
	{
		if( m_nTail+nUnitSize < m_nTotalSize )
		{
			goto	GoodEnd;
		}
		else if( nUnitSize<m_nHead )
		{
			m_nLEnd	= m_nTail;
			m_nTail	= 0;
			// ����������Ϊ��ʱ��Head��Ҫ��Tailһ�����
			if( m_nHead==m_nLEnd )
			{
				m_nHead	= 0;
			}
			goto	GoodEnd;
		}
	}
	else
	{
		if( m_nTail+nUnitSize < m_nHead )
		{
			goto	GoodEnd;
		}
	}

	return	NULL;

GoodEnd:
	SQ_UNIT	*pRst;
	pRst		= (SQ_UNIT *)(m_pBuf + m_nTail);
	pRst->size	= (unitsize_t)nSize;
	m_nTail		+= nUnitSize;
	m_nSize		+= nUnitSize;
	m_nUnitNum	++;

	return	pRst->data;
}
int		whsmpqueue::In(const void *pData, size_t nSize)
{
	// �ղ����ʲôҲ����
	if( nSize==0 )
	{
		if( !m_bAcceptZeroLen )
		{
			return	0;
		}
	}

	void	*pRst;

	pRst	= InAlloc(nSize);
	if( !pRst )
	{
		return	-1;
	}
	// Ҳ����˵pData����Ϊ��
	if( nSize > 0 )
	{
		memcpy(pRst, pData, nSize);
	}

	return	0;
}
size_t	whsmpqueue::PeekSize() const
{
	if( m_nHead == m_nTail )
	{
		return	0;
	}

	SQ_UNIT	*pRst;
	pRst	= (SQ_UNIT *)(m_pBuf + m_nHead);
	return	pRst->size;
}
void *	whsmpqueue::OutPeek(size_t *pnSize) const
{
	if( m_nHead == m_nTail )
	{
		*pnSize	= 0;
		return	NULL;
	}

	SQ_UNIT	*pRst;
	pRst	= (SQ_UNIT *)(m_pBuf + m_nHead);
	if( pnSize )
	{
		*pnSize	= pRst->size;
	}

	return	pRst->data;
}
void *	whsmpqueue::OutFree(size_t *pnSize)
{
	if( m_nUnitNum == 0 )
	{
		// ������m_head == m_tail����Ϊ��������ʱ����д�
		if( pnSize )	*pnSize	= 0;
		return	NULL;
	}

	SQ_UNIT	*pRst;
	size_t		nUnitSize;
	bool		bSkip	= false;
	pRst		= (SQ_UNIT *)(m_pBuf + m_nHead);
	if( (*pnSize)>0 && (*pnSize)<pRst->size )
	{
		// 2005-07-15 ��
		assert(0);
		// �����������
		bSkip	= true;
	}
	*pnSize		= pRst->size;
	nUnitSize	= SQ_UNIT_HDR_SIZE + (*pnSize);

	m_nHead		+= nUnitSize;
	if( m_nTail<m_nHead )
	{
		if( m_nHead==m_nLEnd )
		{
			m_nHead	= 0;
		}
		else
		{
			assert(m_nHead<m_nLEnd);
		}
		// m_nHead>m_nLEnd������������������ǲ�����ֵģ�
		// �������˸ı��˶����е�Ԫ��ͷ
	}
	// m_nTail>=m_nHead������Ͳ��ñ�Ĳ����ˣ��ȴ�m_nHead�����ӽ�m_nTail

	m_nUnitNum	--;
	m_nSize		-= nUnitSize;
	assert(m_nSize>=0);
	// ���ȫû�˾ͻ�������������´ο��ԷŴ�һЩ������
	if( m_nUnitNum == 0 )
	{
		Clean();
	}

	if( bSkip )
	{
		return	NULL;
	}
	return	pRst->data;
}
int		whsmpqueue::Out(void *pData, size_t *pnSize)
{
	void	*pRst;

	pRst	= OutFree(pnSize);
	if( !pRst )
	{
		return	-1;
	}
	memcpy(pData, pRst, *pnSize);

	return	0;
}
int		whsmpqueue::FreeN(int nNum)
{
	if( m_nUnitNum <= (size_t)nNum )
	{
		// ������ȫ���ͷ�����
		nNum	= m_nUnitNum;
		Clean();
		return	nNum;
	}
	int			nCount = 0;
	size_t		nUnitSize;
	
	while( nCount<nNum )
	{
		SQ_UNIT	*pRst	= (SQ_UNIT *)(m_pBuf + m_nHead);
		nUnitSize	= SQ_UNIT_HDR_SIZE + pRst->size;
		m_nHead		+= nUnitSize;
		if( m_nTail<m_nHead )
		{
			if( m_nHead==m_nLEnd )
			{
				m_nHead	= 0;
			}
			else
			{
				assert(m_nHead<m_nLEnd);
			}
		}
		m_nUnitNum	--;
		m_nSize		-= nUnitSize;
		nCount		++;
	}

	return	nCount;
}
int		whsmpqueue::In2(const void *pData0, size_t nSize0, const void *pData, size_t nSize)
{
	char *ptr;

	ptr	= (char *)InAlloc(nSize0 + nSize);
	if( !ptr )
	{
		return	-1;
	}

	if( nSize0 > 0 )
	{
		memcpy(ptr, pData0, nSize0);
	}
	if( nSize > 0 )
	{
		memcpy(ptr+nSize0, pData, nSize);
	}
	return	0;
}
int		whsmpqueue::Out2(void *pData0, size_t nSize0, void *pData, size_t *pnSize)
{
	char *ptr;

	ptr	= (char *)OutFree(pnSize);
	if( !ptr )
	{
		return	-1;
	}
	(*pnSize)	-= nSize0;
	if( (*pnSize)<0 )
	{
		return	-1;
	}
	memcpy(pData0, ptr, nSize0);
	memcpy(pData, ptr+nSize0, (*pnSize));

	return	0;
}
void	whsmpqueue::Remember()
{
	m_remember.nUnitNum		= m_nUnitNum;
	m_remember.nSize		= m_nSize;
	m_remember.nHead		= m_nHead;
	m_remember.nTail		= m_nTail;
	m_remember.nLEnd		= m_nLEnd;
}
void	whsmpqueue::RollBack()
{
	m_nUnitNum		= m_remember.nUnitNum;
	m_nSize			= m_remember.nSize;
	m_nHead			= m_remember.nHead;
	m_nTail			= m_remember.nTail;
	m_nLEnd			= m_remember.nLEnd;
}

////////////////////////////////////////////////////////////////////
// ���ٶ��У�ÿ��Ԫ�ر��벻����һ�����ȣ����ǻᰴ����󳤶ȴ洢��
// �����ڲ�Ĭ�ϳ�����4�ı���
////////////////////////////////////////////////////////////////////
whquickqueue::whquickqueue()
: m_nHead			(0)				// ָ���һ���ѷ����λ��
, m_nTail			(0)				// ָ���һ���ɷ����λ��
, m_nCurPos			(0)				// ��ǰ�������ʵ�λ��
, m_nCount			(0)				// ����Ҫ���ʵĸ���
, m_nUnitNum		(0)				// ��ǰ��Ԫ����
, m_pBuf			(NULL)			// ���ݻ�����
, m_nUnitSize		(0)				// һ����Ԫ��ͷ��(���εĳ���)�ĳ���
, m_nTotalSize		(0)				// ���������ܳ��� (m_nUnitSize * m_nMaxUnit
, m_nUnitDataSize	(0)				// һ����Ԫ���ݲ��ֵĳ���
, m_nMaxUnit		(0)				// ���洢�ĵ�Ԫ����
, m_nLogicMaxUnit	(0)				// �߼������洢�ĵ�Ԫ����
{
}
whquickqueue::~whquickqueue()
{
	Release();
}
int		whquickqueue::Init(int nMaxUnit, int nUnitDataSize)
{
	assert( !m_pBuf );

	m_nUnitDataSize	= nUnitDataSize;
	m_nMaxUnit		= nMaxUnit;
	m_nLogicMaxUnit	= nMaxUnit;
	m_nUnitSize		= QQ_UNIT_LEN(nUnitDataSize);
	m_nTotalSize	= m_nUnitSize * m_nMaxUnit;

	m_pBuf	= new char[m_nTotalSize];
	if( !m_pBuf )
	{
		return	-1;
	}

	Clean();

	return	0;
}
int		whquickqueue::Release()
{
	if( m_pBuf )
	{
		delete []	m_pBuf;
		m_pBuf	= NULL;
	}
	return	0;
}
void	whquickqueue::Clean()
{
	m_nHead		= 0;
	m_nTail		= 0;
	m_nCurPos	= 0;
	m_nCount	= 0;
	m_nUnitNum	= 0;
}
void *	whquickqueue::InAlloc(int nSize)
{
	if( nSize > m_nUnitDataSize )
	{
		// �޷����������ڴ�
		return	NULL;
	}
	if( m_nUnitNum >= m_nLogicMaxUnit )
	{
		// �������߼�����
		return	NULL;
	}
	if( m_nUnitNum == m_nMaxUnit )
	{
		// �������
		// ��ʱm_nTail==m_nHead
		return	NULL;
	}

	if( m_nTail >= m_nHead )
	{
		if( m_nTail < m_nTotalSize )
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
	char	*pRst;
	pRst		= m_pBuf + m_nTail;
	m_nTail		+= m_nUnitSize;
	m_nUnitNum	++;

	*(int *)pRst	= nSize;
	pRst		+= sizeof(int);

	return	pRst;
}
int		whquickqueue::In(const void *pData, int nSize)
{
	void	*pRst;

	pRst	= InAlloc(nSize);
	if( !pRst )
	{
		return	-1;
	}
	if( nSize>0 )
	{
		memcpy(pRst, pData, nSize);
	}

	return	0;
}
void *	whquickqueue::OutPeek(int *pnSize) const
{
	if( m_nUnitNum == 0 )
	{
		*pnSize	= 0;
		return	NULL;
	}

	char	*pRst;
	pRst	= m_pBuf + m_nHead;
	if( pnSize )
	{
		*pnSize	= *(int *)pRst;
	}

	return	pRst + sizeof(int);
}
void *	whquickqueue::OutFree(int *pnSize)
{
	if( m_nUnitNum == 0 )
	{
		if( pnSize )	*pnSize	= 0;
		return	NULL;
	}

	char	*pRst;
	pRst	= m_pBuf + m_nHead;
	if( pnSize )
	{
		assert((*pnSize)==0 || (*pnSize)>=(*(int *)pRst));
		*pnSize	= *(int *)pRst;
	}
	pRst	+= sizeof(int);

	m_nHead	+= m_nUnitSize;
	if( m_nHead == m_nTotalSize )
	{
		// ����
		m_nHead	= 0;
		if( m_nTail == m_nTotalSize )
		{
			// һ�����
			m_nTail	= 0;
		}
	}
	m_nUnitNum	--;
	// ���ȫû�˾ͻ�������������´ο��ԷŴ�һЩ������
	if( m_nUnitNum == 0 )
	{
		Clean();
	}

	return	pRst;
}
int		whquickqueue::Out(void *pData, int *pnSize)
{
	const void	*pRst;

	pRst	= OutFree(pnSize);
	if( !pRst )
	{
		return	-1;
	}
	memcpy(pData, pRst, *pnSize);

	return	0;
}
int		whquickqueue::FreeN(int nNum)
{
	if( m_nUnitNum <= nNum )
	{
		// ������ȫ���ͷ�����
		nNum	= m_nUnitNum;
		Clean();
		return	nNum;
	}

	m_nHead		+= nNum * m_nUnitSize;
	// ע�⣺m_nHead��m_nTail����m_nTotalSize������������
	//       ��Ϊ������ȫ���ͷŵ��������ǰ�洦����
	if( m_nHead>=m_nTotalSize )
	{
		// �ۻ�ͷ
		m_nHead	-= m_nTotalSize;
	}

	m_nUnitNum	-= nNum;

	return	nNum;
}
int		whquickqueue::FreeLastN(int nNum)
{
	if( m_nUnitNum <= nNum )
	{
		// ������ȫ���ͷ�����
		nNum	= m_nUnitNum;
		Clean();
		return	nNum;
	}

	m_nTail		-= nNum * m_nUnitSize;
	if( m_nTail<=0 )
	{
		// �ۻ�ͷ
		m_nTail	+= m_nTotalSize;
	}
	m_nUnitNum	-= nNum;

	return	nNum;
}
int		whquickqueue::BeginGet()
{
	m_nCurPos	= m_nHead;
	m_nCount	= m_nUnitNum;
	return	0;
}
void *	whquickqueue::GetNext(int *pnSize)
{
	if( m_nCount == 0 )
	{
		// ���˾�ͷ
		return	NULL;
	}
	m_nCount	--;

	// ��ñ�λ�õ�����
	char	*pRst = m_pBuf + m_nCurPos;
	if( pnSize )
	{
		*pnSize	= *(int *)pRst;
	}
	pRst	+= sizeof(int);

	// �����һ��λ��
	m_nCurPos	+= m_nUnitSize;
	if( m_nCurPos == m_nTotalSize )
	{
		if( m_nTail != m_nTotalSize )
		{
			// ֻҪm_nTail��ָ���ܻ���������һ���ֽ�(������λ��)��m_nCurPos��Ҫ����
			m_nCurPos	= 0;
		}
	}

	return	pRst;
}

////////////////////////////////////////////////////////////////////
// �߼�����
////////////////////////////////////////////////////////////////////
whadvqueue::whadvqueue()
: m_paIdx(NULL)
, m_pData(NULL)
, m_nMaxNum(0)
, m_nMaxSize(0)
, m_nHead(0)
, m_nToRead(0)
, m_nTail(0)
, m_nToWrite(0)
, m_nTotalNum(0)
{
	// 2005-07-05 ��û�����������ʱ���ðɡ�:(
	assert(0);
}
whadvqueue::~whadvqueue()
{
	Release();
}
int		whadvqueue::Init(int nMaxNum, int nMaxSize)
{
	assert( m_paIdx == NULL );
	assert( m_pData == NULL );

	m_paIdx	= new IDX_T[nMaxNum];
	if( !m_paIdx )
	{
		assert(0);
		return	-1;
	}
	m_pData	= new char[nMaxSize];
	if( !m_pData )
	{
		assert(0);
		return	-2;
	}

	m_nMaxNum	= nMaxNum;
	m_nMaxSize	= nMaxSize;

	Clean();

	return	0;
}
int		whadvqueue::Release()
{
	if( m_paIdx )
	{
		delete []	m_paIdx;
		m_paIdx	= NULL;
	}
	if( m_pData )
	{
		delete []	m_pData;
		m_pData	= NULL;
	}
	return	0;
}
void	whadvqueue::Clean()
{
	m_nHead		= 0;
	m_nToRead	= 0;
	m_nTail		= 0;
	m_nToWrite	= 0;
	m_nTotalNum	= 0;
}
void *	whadvqueue::InAlloc(int nSize)
{
	// �ж��Ƿ���Բ���
	// �����Ƿ񳬹�
	int	nIdx	= idxpp(m_nToWrite);
	if( nIdx == m_nHead )
	{
		return	NULL;
	}
	// �Ƿ��пռ�������

	return	NULL;
}
