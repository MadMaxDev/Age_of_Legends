// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_pinger.h
// Creator      : Wei Hua (魏华)
// Comment      : 用于测试基于CNL2的服务器的ping值的对象
//                用法：（一般只使用CNL_PINGER_MAN对象即可）
//                1. 创建CNL_PINGER_MAN对象。
//                可以手动调整一下m_pinger的nInterval、nTimeOut、nTotal这三个参数（它们分别是：发送ping的间隔(毫秒)、收包等待的时间间隔、总共需要发送的个数）
//                其的默认值分别是：nInterval(1000)、nTimeOut(4000)、nTotal(10)
//                2. 用StartThread开始ping工作线程
//                3. 调用AddUnit可以加入一个ping对象，如果成功返回对象ID。（如果不删除则对象永远存在）
//                4. 调用GetStatistic获得所有ping对象的统计数据，然后自己做显示
//                5. 如果需要删除某些ping单元，可以调用DelUnit。如果需要清除所有Ping单元，调用ClearAllUnit。

#ifndef	__WHNETCNL2_PINGER_H__
#define __WHNETCNL2_PINGER_H__

#include "whnetcnl2.h"
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whtime.h>
#include <WHCMN/inc/whthread.h>

namespace n_whnet
{

class	CNL_PINGER
{
public:
	#pragma pack(push, old_pack_num, 1)
	struct	ECHODATA_T										// 原样返回的数据部分
	{
		unsigned int		nID;							// 用于标识身份的（这样可以不通过地址来区别不同的目的地）
		n_whcmn::whtick_t	nSendTime;						// 发送时间
	};
	#pragma pack(pop, old_pack_num)
	struct	DSTUNIT_T										// 一个ping的目标的结构
	{
		struct sockaddr_in	dstaddr;						// 目标地址
		n_whcmn::whtick_t	nLastSendTime;					// 上次发送请求的时间
		int					nSendCount;						// 已经发送的个数
		int					nRecvCount;						// 接收到的个数
		int					nTotalDelay;					// 接收到包的累计延时
		inline void	clear()
		{
			WHMEMSET0THIS();
		}
		inline bool	IsShouldSend(n_whcmn::whtick_t nNow, int nInterval) const
		{
			if( nLastSendTime==0
			||  n_whcmn::wh_tickcount_diff(nNow, nLastSendTime)>=nInterval
			)
			{
				return	true;
			}
			return		false;
		}
		inline bool	IsCanSend(int nTotal) const
		{
			return	nSendCount<nTotal;
		}
		inline bool	IsAllOver(int nTotalCount, n_whcmn::whtick_t nNow, int nTimeOut) const
		{
			if( nRecvCount == nTotalCount )
			{
				// 全部接收完毕
				return	true;
			}
			if( nLastSendTime>0
			&&  n_whcmn::wh_tickcount_diff(nNow, nLastSendTime)>=nTimeOut
			)
			{
				// 有没收到的，不过已经超时了
				return	true;
			}
			return		false;
		}
		// 重新开始ping
		void	Reset();
		// 获得统计
		// 丢包率	*pfLossRate
		// 平均延时	*pnDelay
		// 加权延时	*pnWeightedDelay = nDelay / (1-fLossRate*SendCount/TotalCount)
		void	GetStatistic(float *pfLossRate, int *pnDelay, int *pnWeightedDelay, int nTotalCountRef);
	};
private:
	SOCKET					sock;							// 发送用的socket
	n_whcmn::whunitallocatorFixed<DSTUNIT_T>	m_Units;	// 单元数组
public:
	// 公开数据(这些数据需要外界直接设置的，当然有默认值)
	int						nInterval;						// 发送ping的间隔(毫秒)
	int						nTimeOut;						// 收包等待的时间间隔
	int						nTotal;							// 总共需要发送的个数
	// 公开方法
	CNL_PINGER(int nMaxDst);								// nMaxDst是最多可以加入的目标地址个数
	~CNL_PINGER();
	int		AddUnit(const struct sockaddr_in &addr);		// 添加一个ping的地址(如果成功，返回对象的ID号，否则返回<0)
	int		AddUnit(const char *cszIPPort);					// 通过字串添加一个ping的地址(返回和上一个函数一样)
	int		DelUnit(int nID);								// 删除一个ping对象
	int		ClearAllUnit();									// 清除所有单元
	int		Tick();											// 一次逻辑运动(接收返回包和重发)
	int		ResetUnit(int nID);
	int		ResetAllUnit();
	bool	IsAllOver(int nID);
	// 获得所有的ping统计
	struct	PINGSTAT_T
	{
		int		nID;
		struct sockaddr_in	addr;
		float	fLossRate;
		int		nDelay;
		int		nWeightedDelay;
		bool	bIsAllOver;
	};
	void	GetStatistic(n_whcmn::whvector<PINGSTAT_T> &vectStat);
	int		GetStatistic(int nID, PINGSTAT_T *pStat);
public:
	inline SOCKET	GetSocket() const
	{
		return	sock;
	}
private:
	int		SendEchoReq(DSTUNIT_T *pUnit);					// 向指定地址发送echo请求
};

// 带线程的ping管理器
class	CNL_PINGER_MAN
{
public:
	CNL_PINGER			m_pinger;							// 可以直接设置pinger的参数，如发送次数，超时等。
private:
	n_whcmn::whlock		m_lock;
	n_whcmn::whtid_t	m_tid;
	bool				m_stopthread;
	int					m_interval;
public:
	CNL_PINGER_MAN(int nMaxDst);
	~CNL_PINGER_MAN();
	int		StartThread(int nInterval=0);					// 开始工作线程(nInterval是内部调用select的超时，为0则使用默认的间隔：50)
	int		EndThread();									// 结束工作线程
	int		Tick();											// 一次逻辑运动(接收返回包和重发)
															// 这个由thread调用（如果返回非0表示应该线程结束了）
public:
	inline int		AddUnit(const char *cszIPPort)
	{
		m_lock.lock();
		int	rst		= m_pinger.AddUnit(cszIPPort);
		m_lock.unlock();
		return		rst;
	}
	inline int		DelUnit(int nID)
	{
		m_lock.lock();
		int	rst		= m_pinger.DelUnit(nID);
		m_lock.unlock();
		return		rst;
	}
	inline int		ResetUnit(int nID)
	{
		m_lock.lock();
		int	rst		= m_pinger.ResetUnit(nID);
		m_lock.unlock();
		return		rst;
	}
	inline int		ResetAllUnit()
	{
		m_lock.lock();
		int	rst		= m_pinger.ResetAllUnit();
		m_lock.unlock();
		return		rst;
	}
	inline int		ClearAllUnit()
	{
		m_lock.lock();
		int	rst		= m_pinger.ClearAllUnit();
		m_lock.unlock();
		return		rst;
	}
	inline bool		IsAllOver(int nID)
	{
		m_lock.lock();
		bool	rst	= m_pinger.IsAllOver(nID);
		m_lock.unlock();
		return		rst;
	}
	inline 	void	GetStatistic(n_whcmn::whvector<CNL_PINGER::PINGSTAT_T> &vectStat)
	{
		m_lock.lock();
		m_pinger.GetStatistic(vectStat);
		m_lock.unlock();
	}
	inline int		GetStatistic(int nID, CNL_PINGER::PINGSTAT_T *pStat)
	{
		m_lock.lock();
		int	rst	= m_pinger.GetStatistic(nID, pStat);
		m_lock.unlock();
		return	rst;
	}
};



}	// EOF namespace n_whnet

#endif
