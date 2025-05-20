// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whid.h
// Creator      : Wei Hua (κ��)
// Comment      : id���䡢��ָ�����ݶ�Ӧ����صĹ���
//              : ʹ�÷����ο�whid-tst.cpp
// CreationDate : 2003-05-15
// ChangeLOG    : 2005-01-12 ȡ��ԭ����list������DList����

#ifndef	__WHID_H__
#define	__WHID_H__

#include "whcmn_def.h"
#include "whDList.h"

namespace n_whcmn
{

// EXTNUMΪ������������ĳ��ȣ��������������洢ָ�룩
template<int EXTNUM = 1>
class	whid
{
public:
	enum
	{
		MAXNUM			= 0x10000,
		MAXRAND			= 0x7FFF,
	};
	// ���hashfunc�Ǹ��ⲿʹ�ñ�id�����õ�
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
		int	nID;						// ��������ȥ�ˣ�nID>0xFFFF�����û�з����ȥ��nID<=0xFFFF������m_aIDNode�������±�
		int	anExt[EXTNUM];				// ��������(��������ʵ�ּ򵥵Ķ�Ӧ)
		inline void	clear()
		{
			nID	= -1;
			memset(anExt, 0, sizeof(anExt));
		}
	};
	typedef	whDList<UNIT_T>		DL_T;
	typedef	typename DL_T::node	node;	// ���node������UNIT�����ݲ���
	int		m_nMaxNum;					// ����ID����
	node	*m_aIDNode;					// ID����
	int		m_nRand;					// �����
	DL_T	m_dlAvail;					// ���õ��б�
	DL_T	m_dlAlloc;					// �Ѿ�������б�
public:
	whid();
	~whid();

	// ��ʼ��
	// nMaxNumΪ����ID����(���ܳ���65536��)
	int		Init(int nMaxNum = MAXNUM);
	// �ս�
	int		Release();
	// ����һ��ID�����ظ���ָ�루����ָ����һ��int���飬����ΪEXTNUM���������ڸ���ָ������д��Ҫ����������
	int *	AllocID(int *pnID);
	// ��������ID���룬���ظ���ָ�루�����Ҫ������һ�����ķ���������ID���������������������ķ������ṩ��ID�ڱ�����ID�������Ӧ�Ķ�Ӧ��
	int *	AllocByID(int nID);
	// �ͷ�ID
	int		FreeID(int nID);
	// ��ȡID�����ĸ�������(���ID����ͷ���NULL)
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

	m_aIDNode	= new node [nMaxNum];	// ÿ��Ԫ���ڹ����ʱ����Ѿ���ʼ����
	assert(m_aIDNode);
	m_nMaxNum	= nMaxNum;

	// ��ʼ������Ԫ�أ��������Ǽ���avail�б�
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
		// Ӧ���Ѿ�Release���ˣ����߻�û�г�ʼ��
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
		// û�пɷ���ĵ�Ԫ��
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
	if( pNode->nID > 0xFFFF )	// �������0xFFFF��˵���Ѿ���������
	{
		// û�����ID
		return	NULL;
	}
	pNode->leave();				// ��avail�������˳�
	pNode->data.clear();		// ��������
	pNode->data.nID	= nID;		// ����ID
	m_dlAlloc.AddToHead(pNode);	// ����alloc����

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
		// û�����ID
		return	-1;
	}

	// ��alloc�������˳�
	m_aIDNode[nIdx].leave();
	// ����ID�����·���avail����
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
		// û�����ID
		return	NULL;
	}

	return	m_aIDNode[nIdx].data.anExt;
}

}		// EOF namespace n_whcmn

#endif	// EOF __WHID_H__
