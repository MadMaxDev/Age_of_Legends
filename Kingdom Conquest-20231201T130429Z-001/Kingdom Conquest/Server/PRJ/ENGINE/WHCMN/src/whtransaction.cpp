// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtransaction.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ͨ�õ�transaction�������
// CreationDate : 2006-11-08
// ChangeLOG    : 2007-11-13 ������whtransactionman_I::Tick()�п��ܵ����ӱ�ɾ�����������ò����Ĵ���

#include "../inc/whtransaction.h"
#include "../inc/whunitallocator.h"
#include "../inc/whstring.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// ��ʵ�ඨ��
////////////////////////////////////////////////////////////////////
class	whtransactionman_I	: public whtransactionman
{
public:
	whtransactionman_I(const char *cszName);
	~whtransactionman_I();
// Ϊ�ӿ�ʵ�ֵĺ���
public:
	virtual int		Init(INFO_T *pInfo);
	// �ս�
	virtual int		Release();
	// ��õ�ǰ��Transaction����
	virtual int		GetNum() const;
	// ���һ��transaction����ͬʱ��transactionID���ý�transaction�����У����������transactionID
	virtual int		AddTransaction(whtransactionbase *pTransaction);
	// ɾ��transaction�������ض���ָ�룬�ϲ�����Ƿ�Ӧ��ɾ����
	virtual whtransactionbase *	DelTransaction(int nTransactionID);
	// ����transactionID���transaction����
	virtual whtransactionbase *	GetTransaction(int nTransactionID);
	// ����һ���߼�����
	virtual int		Tick();
	// ע��ʱ���¼�
	virtual int		RegTimeEvent(int dt, int nTID, int nEvent, whtimequeue::ID_T *pTQID);
// �Լ�Ҫ�õĶ���
private:
	char			m_szName[64];
	// ��¼transactionָ������
	whunitallocatorFixed<whtransactionbase *>	m_TransactionPtrs;
	// ʱ�����
	whtimequeue		m_tq;									// ���ֳ�ʱ�����ӡ��������ط���
	// ��ǰʱ��
	whtick_t		m_tickNow;
private:
	struct	TQUNIT_T 
	{
		int			nTID;									// �����Ľ��׶���ID����Ϊ��������Ѿ���ɾ���ˣ����Ի�����ID�Ƚ����ף�
		int			nEvent;									// ��Ҫ���¼�����
	};
private:
	// ��ʼ������
	int		Init_TransactionPtrs(int nMaxTransaction);
	// �ͷ������Լ������Ԫ��
	int		Release_TransactionPtrs();
	// ʱ����д���
	int		Tick_TE();
};
////////////////////////////////////////////////////////////////////
// ��������
////////////////////////////////////////////////////////////////////
whtransactionman *	whtransactionman::Create(const char *cszName)
{
	return	new whtransactionman_I(cszName);
}
////////////////////////////////////////////////////////////////////
// ������ʵ��
////////////////////////////////////////////////////////////////////
whtransactionman_I::whtransactionman_I(const char *cszName)
: m_tickNow(0)
{
	WH_STRNCPY0(m_szName, cszName);
}
whtransactionman_I::~whtransactionman_I()
{
	Release();
}
int		whtransactionman_I::Init(INFO_T *pInfo)
{
	if( Init_TransactionPtrs(pInfo->nMaxTransaction)<0 )
	{
		return	-1;
	}
	whtimequeue::INFO_T	tqinfo;
	tqinfo.nUnitLen		= sizeof(TQUNIT_T);
	tqinfo.nChunkSize	= pInfo->nTQChunkSize;
	if( m_tq.Init(&tqinfo)<0 )
	{
		return	-4;
	}
	return	0;
}
// �ս�
int		whtransactionman_I::Release()
{
	Release_TransactionPtrs();
	// ���ʱ�����(������������)
	m_tq.Release();
	return	0;
}
int		whtransactionman_I::GetNum() const
{
	return	m_TransactionPtrs.size();
}
// ��ʼ������
int		whtransactionman_I::Init_TransactionPtrs(int nMaxTransaction)
{
	if( m_TransactionPtrs.Init(nMaxTransaction)<0 )
	{
		return	-1;
	}
	return	0;
}
// �ͷ������Լ������Ԫ��
int		whtransactionman_I::Release_TransactionPtrs()
{
	// �ͷ������е�ÿ��Ԫ��
	for(whunitallocatorFixed<whtransactionbase *>::iterator it=m_TransactionPtrs.begin(); it!=m_TransactionPtrs.end(); ++it)
	{
		delete	(*it);
	}
	// �ͷŴ�����
	m_TransactionPtrs.Release();
	return	0;
}
int		whtransactionman_I::AddTransaction(whtransactionbase *pTransaction)
{
	whtransactionbase	**pptr;
	int	nID;
	if( (nID=m_TransactionPtrs.AllocUnit(pptr))<0 )
	{
		return	-1;
	}
	*pptr	= pTransaction;
	// ����������ָ���Լ�sessionID
	pTransaction->SetTransactionMan(this);
	pTransaction->SetTransactionID(nID);

	WHCMN_LOG_WRITEFMT(WHCMN_LOG_ID_STORY, WHCMN_LOG_STD_HDR(1060, TRANSACTION)"AddTransaction,%s,%p,%s,0x%X,%d/%d", m_szName, this, pTransaction->GetTypeStr(), nID, m_TransactionPtrs.size(), m_TransactionPtrs.getmax());
	return	nID;
}
whtransactionbase *	whtransactionman_I::DelTransaction(int nTransactionID)
{
	whtransactionbase *ptr	= GetTransaction(nTransactionID);
	if( !ptr )
	{
		return	NULL;
	}
	// WHCMN_LOG_ID_STORY
	WHCMN_LOG_WRITEFMT(WHCMN_LOG_ID_STORY, WHCMN_LOG_STD_HDR(1061, TRANSACTION)"DelTransaction,%s,%p,%s,0x%X,%d/%d", m_szName, this, ptr->GetTypeStr(), nTransactionID, m_TransactionPtrs.size()-1, m_TransactionPtrs.getmax());
	m_TransactionPtrs.FreeUnit(nTransactionID);
	return	ptr;
}
whtransactionbase *	whtransactionman_I::GetTransaction(int nTransactionID)
{
	whtransactionbase	**pptr	= m_TransactionPtrs.getptr(nTransactionID);
	if( !pptr )
	{
		return	NULL;
	}
	return	*pptr;
}
int		whtransactionman_I::Tick()
{
	m_tickNow	= wh_gettickcount();
/*	// ����ÿ��transaction��tick
	whunitallocatorFixed<whtransactionbase *>::iterator it=m_TransactionPtrs.begin();
	while( it!=m_TransactionPtrs.end() )
	{
		whtransactionbase *pT	= *(it++);
		pT->Tick();				// ��ΪpT->Tick();�п��ܵ���ɾ��transaction��������Ҫ�������ķ�ʽ
	}
	*/	// ����tick�ˣ�Ӧ��û�ã�ʱ�����Ӧ�ÿ��Խ������tick��Ҫ���������
	// ʱ�����
	Tick_TE();
	return	0;
}
int		whtransactionman_I::Tick_TE()
{
	m_tickNow			= wh_gettickcount();
	// ����ʱ��������Ƿ��ж���
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_tq.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		whtransactionbase	*pT	= GetTransaction(pUnit->nTID);
		if( pT )			// ����˵�����ܸ�Transaction�Ѿ�ʵЧ�ˣ�����һ��Transaction�����˶����ʱ�¼���
		{
			pT->DealTimeEvent(pUnit->nEvent);
		}
		// ���get������ֻ��ͨ��Delɾ���������Լ�ɾ
		m_tq.Del(id);
	}
	return	0;
}
int		whtransactionman_I::RegTimeEvent(int dt, int nTID, int nEvent, whtimequeue::ID_T *pTQID)
{
	TQUNIT_T			*pTQUnit;
	if( m_tq.AddGetRef(m_tickNow+dt, (void **)&pTQUnit, pTQID)<0 )
	{
		assert(0);
		return	-1;
	}
	pTQUnit->nTID		= nTID;
	pTQUnit->nEvent		= nEvent;
	return	0;
}
