// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtransaction.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 通用的transaction对象机制
// CreationDate : 2006-11-08
// ChangeLOG    : 2007-11-13 修正了whtransactionman_I::Tick()中可能迭代子被删除而继续调用产生的错误

#include "../inc/whtransaction.h"
#include "../inc/whunitallocator.h"
#include "../inc/whstring.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// 真实类定义
////////////////////////////////////////////////////////////////////
class	whtransactionman_I	: public whtransactionman
{
public:
	whtransactionman_I(const char *cszName);
	~whtransactionman_I();
// 为接口实现的函数
public:
	virtual int		Init(INFO_T *pInfo);
	// 终结
	virtual int		Release();
	// 获得当前的Transaction个数
	virtual int		GetNum() const;
	// 添加一个transaction对象，同时把transactionID设置进transaction对象中，并返回这个transactionID
	virtual int		AddTransaction(whtransactionbase *pTransaction);
	// 删除transaction（并返回对象指针，上层决定是否应该删除）
	virtual whtransactionbase *	DelTransaction(int nTransactionID);
	// 根据transactionID获得transaction对象
	virtual whtransactionbase *	GetTransaction(int nTransactionID);
	// 进行一次逻辑跳动
	virtual int		Tick();
	// 注册时间事件
	virtual int		RegTimeEvent(int dt, int nTID, int nEvent, whtimequeue::ID_T *pTQID);
// 自己要用的东东
private:
	char			m_szName[64];
	// 记录transaction指针数组
	whunitallocatorFixed<whtransactionbase *>	m_TransactionPtrs;
	// 时间队列
	whtimequeue		m_tq;									// 各种超时（连接、断连、重发）
	// 当前时间
	whtick_t		m_tickNow;
private:
	struct	TQUNIT_T 
	{
		int			nTID;									// 关联的交易对象ID（因为对象可能已经被删除了，所以还是用ID比较稳妥）
		int			nEvent;									// 需要的事件类型
	};
private:
	// 初始化数组
	int		Init_TransactionPtrs(int nMaxTransaction);
	// 释放数据以及里面的元素
	int		Release_TransactionPtrs();
	// 时间队列处理
	int		Tick_TE();
};
////////////////////////////////////////////////////////////////////
// 创建函数
////////////////////////////////////////////////////////////////////
whtransactionman *	whtransactionman::Create(const char *cszName)
{
	return	new whtransactionman_I(cszName);
}
////////////////////////////////////////////////////////////////////
// 函数体实现
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
// 终结
int		whtransactionman_I::Release()
{
	Release_TransactionPtrs();
	// 清空时间队列(这个放在最后做)
	m_tq.Release();
	return	0;
}
int		whtransactionman_I::GetNum() const
{
	return	m_TransactionPtrs.size();
}
// 初始化数组
int		whtransactionman_I::Init_TransactionPtrs(int nMaxTransaction)
{
	if( m_TransactionPtrs.Init(nMaxTransaction)<0 )
	{
		return	-1;
	}
	return	0;
}
// 释放数据以及里面的元素
int		whtransactionman_I::Release_TransactionPtrs()
{
	// 释放数组中的每个元素
	for(whunitallocatorFixed<whtransactionbase *>::iterator it=m_TransactionPtrs.begin(); it!=m_TransactionPtrs.end(); ++it)
	{
		delete	(*it);
	}
	// 释放大数组
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
	// 关联管理器指针以及sessionID
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
/*	// 调用每个transaction的tick
	whunitallocatorFixed<whtransactionbase *>::iterator it=m_TransactionPtrs.begin();
	while( it!=m_TransactionPtrs.end() )
	{
		whtransactionbase *pT	= *(it++);
		pT->Tick();				// 因为pT->Tick();中可能导致删除transaction，所以需要用这样的方式
	}
	*/	// 不用tick了，应该没用，时间队列应该可以解决所有tick需要解决的问题
	// 时间队列
	Tick_TE();
	return	0;
}
int		whtransactionman_I::Tick_TE()
{
	m_tickNow			= wh_gettickcount();
	// 看看时间队列中是否有东西
	whtimequeue::ID_T	id;
	TQUNIT_T			*pUnit;
	while( m_tq.GetUnitBeforeTime(m_tickNow, (void **)&pUnit, &id)==0 )
	{
		whtransactionbase	*pT	= GetTransaction(pUnit->nTID);
		if( pT )			// 否则说明可能该Transaction已经实效了（比如一个Transaction生成了多个超时事件）
		{
			pT->DealTimeEvent(pUnit->nEvent);
		}
		// 这个get出来的只能通过Del删除，不能自己删
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
