// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtransaction.h
// Creator      : Wei Hua (κ��)
// Comment      : ͨ�õ�transaction�������
// CreationDate : 2006-11-08
// ChangeLOG    : 2006-12-11 ��session��Ϊtransaction�����Ӧ�ø�����һЩ

#ifndef	__WHTRANSACTION_H__
#define	__WHTRANSACTION_H__

#include "whtimequeue.h"

namespace n_whcmn
{

// Ԥ�ȶ���
class	whtransactionbase;
// transaction���������������ӿڣ�
class	whtransactionman
{
public:
	static	whtransactionman *	Create(const char *cszName);
	virtual	~whtransactionman()	{};
public:
	struct	INFO_T
	{
		int			nMaxTransaction;								// ����transaction����
		int			nTQChunkSize;									// ʱ����еķ��䵥Ԫ����
	};
public:
	// ��ĳЩ����¿�����Ҫ��ȡһЩ�ڲ��ӿ�
	virtual void *	QueryInterface(const char *cszType)	{return NULL;}
	// ��ʼ��
	virtual int		Init(INFO_T *pInfo)								= 0;
	// �ս�
	virtual int		Release()										= 0;
	// ��õ�ǰ��Transaction����
	virtual int		GetNum() const									= 0;
	// ���һ��transaction����ͬʱ��transactionID���ý�transaction�����У����������transactionID
	virtual int		AddTransaction(whtransactionbase *pTransaction)	= 0;
	// ɾ��transaction
	virtual whtransactionbase *	DelTransaction(int nTransactionID)	= 0;
	// ����transactionID���transaction����
	virtual whtransactionbase *	GetTransaction(int nTransactionID)	= 0;
	// ע��ʱ���¼�
	virtual int		RegTimeEvent(int dt, int nTID, int nEvent, whtimequeue::ID_T *pTQID)
																	= 0;
	// ����һ���߼�����
	virtual int		Tick()											= 0;
};

// transaction����ӿ�
class	whtransactionbase
{
protected:
	whtransactionman	*m_pTMan;									// TransactionMan
	int					m_nTID;										// TransactionID
public:
	// ���ù�����ָ��
	inline	void	SetTransactionMan(whtransactionman	*pTMan)
	{
		m_pTMan		= pTMan;
	}
	// ��ȡtransactionID��һ��������������ʱ��Ӧ������ID�ģ�
	inline int		GetTransactionID() const
	{
		assert(m_nTID>0);
		return	m_nTID;
	}
	// ����transactionID
	inline void		SetTransactionID(int nTID)
	{
		m_nTID		= nTID;
	}
	// �ӹ�������ɾ���Լ���ͬʱ�Լ�Ҳ�����������Transaction�ж��Լ�������ʱ���Լ����õģ�
	inline void		IAmDone()
	{
		if( m_pTMan )	// �����Ϊ�˱����ظ�ɾ��
		{
			m_pTMan->DelTransaction(m_nTID);
			m_pTMan	= NULL;
			delete	this;
		}
	}
public:
	whtransactionbase()
		: m_pTMan(NULL)
		, m_nTID(0)
	{
	}
	virtual	~whtransactionbase()									{};
	// ״̬������һ�����������ڹ̶����ȵ�ָ�����ݣ�nDSize�����������sizeof���ָ���С����Ϊ��Ȼ���ڿ��ò��������ǽ�������ڲ���Ҫ��ͳһ�ķ����洢��ָ������еĻ�������Ҫ�ˡ����ң������Ҫ��ָ�����У��ָ��������Ƿ�ƥ��Ļ�Ҳ���ã�
	// һ��Ĭ����0������INITָ��
	virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize)	= 0;
	// ����ʱ�¼�
	virtual	int		DealTimeEvent(int nEvent)						= 0;
	// ����transaction�����ʹ�����Ҫ����log��
	virtual	const char *	GetTypeStr() const						= 0;
	// ����һ���߼�������һ�㶼����ʵ�����������
//	virtual int		Tick()	{return 0;}
};
// ��TQID��ʱ�����ID���ģ�����ض���Ҫһ��ʱ�����ID�ʹ��������
class	whtransactionbasewithTQID	: public whtransactionbase
{
protected:
	whtimequeue::ID_T	m_TQID;
public:
	~whtransactionbasewithTQID()
	{
		// Ϊ�˱��գ��ͷ�һ�����ID
		m_TQID.quit();
	}
	inline int		RegTimeEvent(int dt, int nEvent)
	{
		return	m_pTMan->RegTimeEvent(dt, GetTransactionID(), nEvent, &m_TQID);
	}
};

}		// EOF namespace n_whcmn

#define	WHTRANSACTION_DECLARE_GETTYPESTR(CName,TypeName)	\
	virtual	const char *	GetTypeStr() const		\
	{												\
	return	#CName"_"#TypeName;							\
	}												\
// EOF WHTRANSACTION_DECLARE_GETTYPESTR


#endif	// EOF __WHTRANSACTION_H__
