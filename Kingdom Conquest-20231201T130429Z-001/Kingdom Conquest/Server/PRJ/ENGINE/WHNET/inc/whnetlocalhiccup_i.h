// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetlocalhiccup_i.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于UDP的局域网内的应用程序授权运行的实现部分主头文件
//              : hiccup是打嗝的意思，只是随便想到这个词就这么用了:P
// CreationDate : 2006-06-29
// ChangeLog    : 

#ifndef	__WHNETLOCALHICCUP_I_H__
#define	__WHNETLOCALHICCUP_I_H__

#include "whnetlocalhiccup.h"
#include "whnetcnl2.h"
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whtimequeue.h>

using namespace n_whcmn;

namespace n_whnet
{

class	whnetlocalhiccup_client_I	: public whnetlocalhiccup_client
{
public:
	// 构造总得有的吧
	whnetlocalhiccup_client_I();
	// 为接口实现的方法
	virtual	~whnetlocalhiccup_client_I();
	virtual	int	Init(INFO_T *pInfo);
	virtual	int	Release();
	virtual	int	Tick();
	virtual	SOCKET GetSocket() const
	{
		return	m_pSlotMan->GetSocket();
	}
// 自己用的
private:
	// <<数据>>
	// 初始化数据
	INFO_T					m_info;
	// 连接器
	CNL2SlotMan				*m_pSlotMan;
	// 加密工厂
	ICryptFactory			*m_pICryptFactory;
	// 连接的slot
	int						m_nSlot;
	// 地址列表（从初始化参数中分解出来）
	whsmpqueue				m_queueAddr;
	// 状态
	struct	STATUS_T
	{
		// STATUS定义
		enum	TYPE_T
		{
			NOTHING						= 0,						// 没有状态
			CONNECTING					= 1,						// 连接过程中
			WORKING						= 2,						// 正常工作过程中
			STOPPED						= 3,						// 停止工作了
			MAXNUM,													// 最多的个数
		};
		// 各种处理函数类型
		typedef	void (whnetlocalhiccup_client_I::*F_CMN_T)();			// 普通的不带参数的函数
		typedef	void (whnetlocalhiccup_client_I::*F_TRANS_T)(STATUS_T *pNextStatus);
																	// 从一个状态转换为另一个状态的时候

		TYPE_T							nStatus;
		F_CMN_T							pBegin;						// 在开始一个状态时需要的函数
		F_CMN_T							pWork;						// 工作过程中
		F_TRANS_T						pTrans;						// 从一个状态转换为另一个状态的时候
	};
	STATUS_T				*m_pStatus;
	STATUS_T				m_aSTATUS[STATUS_T::MAXNUM];
private:
	// <<方法>>
	// 初始化STATUS数据
	void	StatusObjInit();
	// 设置状态
	void	SetStatus(STATUS_T::TYPE_T nType);
	inline STATUS_T::TYPE_T	GetStatus() const
	{
		return	m_pStatus->nStatus;
	}
	// 状态处理函数
	void	_SF_CMN_T_DoNothing();
	void	_SF_TRANS_T_DoNothing(STATUS_T *pNextStatus);
	// CONNECTING的
	void	_SF_Begin_CONNECTING();
	void	_SF_Work_CONNECTING();
	// WORKING的
	void	_SF_Begin_WORKING();
	void	_SF_Work_WORKING();
	// STOPPED的
	void	_SF_Begin_STOPPED();
};

class	whnetlocalhiccup_server_I	: public whnetlocalhiccup_server
{
public:
	// 构造总得有的吧
	whnetlocalhiccup_server_I();
	// 为接口实现的方法
	virtual	~whnetlocalhiccup_server_I();
	virtual	int	Init(INFO_T *pInfo);
	virtual	int	Release();
	virtual	int	Tick();
	virtual	SOCKET GetSocket() const
	{
		return	m_pSlotMan->GetSocket();
	}
// 自己用的
private:
	// <<数据>>
	// 初始化数据
	INFO_T					m_info;
	// 连接器
	CNL2SlotMan				*m_pSlotMan;
	// 加密工厂
	ICryptFactory			*m_pICryptFactory;
	// 连接单元
	struct	ClientUnit
	{
		enum
		{
			PROP_AUTHOK		= 0x00000001,		// 已经授权校验完毕
			PROP_ROOT		= 0x00000002,		// 具有管理权限的连接
		};
		int					nSlot;				// 对应的slot号
		unsigned int		nProp;				// 属性，取值为PROP_XXX
		void	clear()
		{
			nSlot	= 0;
			nProp	= 0;
		}
	};
	whunitallocatorFixed<ClientUnit>	m_Clients;
	struct	TQUNIT_T
	{
		typedef	void (whnetlocalhiccup_server_I::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// 处理该事件的函数
		// 数据部分
		union
		{
			int	nClientID;											// 玩家ID
		}un;
	};
	whtimequeue							m_TQ;
	// 当前的时刻
	whtick_t							m_tickNow;
private:
	// <<方法>>
	// 时间事件处理
	void	TEDeal_ClientPass_TimeOut(TQUNIT_T *pTQUnit);
	// 通过slot号获得client对象指针
	ClientUnit *	GetClientBySlot(int nSlot);
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETLOCALHICCUP_I_H__
