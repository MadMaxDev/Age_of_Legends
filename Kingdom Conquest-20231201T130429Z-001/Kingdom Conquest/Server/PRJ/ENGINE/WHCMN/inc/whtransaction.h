// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtransaction.h
// Creator      : Wei Hua (魏华)
// Comment      : 通用的transaction对象机制
// CreationDate : 2006-11-08
// ChangeLOG    : 2006-12-11 把session改为transaction，这个应该更贴切一些

#ifndef	__WHTRANSACTION_H__
#define	__WHTRANSACTION_H__

#include "whtimequeue.h"

namespace n_whcmn
{

// 预先定义
class	whtransactionbase;
// transaction对象管理器（对外接口）
class	whtransactionman
{
public:
	static	whtransactionman *	Create(const char *cszName);
	virtual	~whtransactionman()	{};
public:
	struct	INFO_T
	{
		int			nMaxTransaction;								// 最多的transaction个数
		int			nTQChunkSize;									// 时间队列的分配单元长度
	};
public:
	// 在某些情况下可能需要获取一些内部接口
	virtual void *	QueryInterface(const char *cszType)	{return NULL;}
	// 初始化
	virtual int		Init(INFO_T *pInfo)								= 0;
	// 终结
	virtual int		Release()										= 0;
	// 获得当前的Transaction个数
	virtual int		GetNum() const									= 0;
	// 添加一个transaction对象，同时把transactionID设置进transaction对象中，并返回这个transactionID
	virtual int		AddTransaction(whtransactionbase *pTransaction)	= 0;
	// 删除transaction
	virtual whtransactionbase *	DelTransaction(int nTransactionID)	= 0;
	// 根据transactionID获得transaction对象
	virtual whtransactionbase *	GetTransaction(int nTransactionID)	= 0;
	// 注册时间事件
	virtual int		RegTimeEvent(int dt, int nTID, int nEvent, whtimequeue::ID_T *pTQID)
																	= 0;
	// 进行一次逻辑跳动
	virtual int		Tick()											= 0;
};

// transaction对象接口
class	whtransactionbase
{
protected:
	whtransactionman	*m_pTMan;									// TransactionMan
	int					m_nTID;										// TransactionID
public:
	// 设置管理器指针
	inline	void	SetTransactionMan(whtransactionman	*pTMan)
	{
		m_pTMan		= pTMan;
	}
	// 获取transactionID（一般调用这个函数的时候都应该是有ID的）
	inline int		GetTransactionID() const
	{
		assert(m_nTID>0);
		return	m_nTID;
	}
	// 设置transactionID
	inline void		SetTransactionID(int nTID)
	{
		m_nTID		= nTID;
	}
	// 从管理器中删除自己，同时自己也毁灭（这个是在Transaction判断自己结束的时候自己调用的）
	inline void		IAmDone()
	{
		if( m_pTMan )	// 这个是为了避免重复删除
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
	// 状态机的下一步操作（对于固定长度的指令数据，nDSize部分填最好用sizeof获得指令大小，因为虽然现在看用不到，但是将来如果内部需要用统一的方法存储在指令队列中的话，就需要了。而且，如果需要用指令长度来校验指令和数据是否匹配的话也有用）
	// 一般默认用0来代表INIT指令
	virtual	int		DoNext(int nCmd, const void *pData, size_t nDSize)	= 0;
	// 处理超时事件
	virtual	int		DealTimeEvent(int nEvent)						= 0;
	// 返回transaction的类型串（主要用于log）
	virtual	const char *	GetTypeStr() const						= 0;
	// 进行一次逻辑跳动（一般都不用实现这个函数）
//	virtual int		Tick()	{return 0;}
};
// 带TQID（时间队列ID）的，如果必定需要一个时间队列ID就从这个派生
class	whtransactionbasewithTQID	: public whtransactionbase
{
protected:
	whtimequeue::ID_T	m_TQID;
public:
	~whtransactionbasewithTQID()
	{
		// 为了保险，释放一下这个ID
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
