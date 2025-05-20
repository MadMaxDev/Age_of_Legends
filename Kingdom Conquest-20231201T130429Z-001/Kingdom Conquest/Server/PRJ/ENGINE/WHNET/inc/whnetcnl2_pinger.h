// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_pinger.h
// Creator      : Wei Hua (κ��)
// Comment      : ���ڲ��Ի���CNL2�ķ�������pingֵ�Ķ���
//                �÷�����һ��ֻʹ��CNL_PINGER_MAN���󼴿ɣ�
//                1. ����CNL_PINGER_MAN����
//                �����ֶ�����һ��m_pinger��nInterval��nTimeOut��nTotal���������������Ƿֱ��ǣ�����ping�ļ��(����)���հ��ȴ���ʱ�������ܹ���Ҫ���͵ĸ�����
//                ���Ĭ��ֵ�ֱ��ǣ�nInterval(1000)��nTimeOut(4000)��nTotal(10)
//                2. ��StartThread��ʼping�����߳�
//                3. ����AddUnit���Լ���һ��ping��������ɹ����ض���ID���������ɾ���������Զ���ڣ�
//                4. ����GetStatistic�������ping�����ͳ�����ݣ�Ȼ���Լ�����ʾ
//                5. �����Ҫɾ��ĳЩping��Ԫ�����Ե���DelUnit�������Ҫ�������Ping��Ԫ������ClearAllUnit��

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
	struct	ECHODATA_T										// ԭ�����ص����ݲ���
	{
		unsigned int		nID;							// ���ڱ�ʶ��ݵģ��������Բ�ͨ����ַ������ͬ��Ŀ�ĵأ�
		n_whcmn::whtick_t	nSendTime;						// ����ʱ��
	};
	#pragma pack(pop, old_pack_num)
	struct	DSTUNIT_T										// һ��ping��Ŀ��Ľṹ
	{
		struct sockaddr_in	dstaddr;						// Ŀ���ַ
		n_whcmn::whtick_t	nLastSendTime;					// �ϴη��������ʱ��
		int					nSendCount;						// �Ѿ����͵ĸ���
		int					nRecvCount;						// ���յ��ĸ���
		int					nTotalDelay;					// ���յ������ۼ���ʱ
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
				// ȫ���������
				return	true;
			}
			if( nLastSendTime>0
			&&  n_whcmn::wh_tickcount_diff(nNow, nLastSendTime)>=nTimeOut
			)
			{
				// ��û�յ��ģ������Ѿ���ʱ��
				return	true;
			}
			return		false;
		}
		// ���¿�ʼping
		void	Reset();
		// ���ͳ��
		// ������	*pfLossRate
		// ƽ����ʱ	*pnDelay
		// ��Ȩ��ʱ	*pnWeightedDelay = nDelay / (1-fLossRate*SendCount/TotalCount)
		void	GetStatistic(float *pfLossRate, int *pnDelay, int *pnWeightedDelay, int nTotalCountRef);
	};
private:
	SOCKET					sock;							// �����õ�socket
	n_whcmn::whunitallocatorFixed<DSTUNIT_T>	m_Units;	// ��Ԫ����
public:
	// ��������(��Щ������Ҫ���ֱ�����õģ���Ȼ��Ĭ��ֵ)
	int						nInterval;						// ����ping�ļ��(����)
	int						nTimeOut;						// �հ��ȴ���ʱ����
	int						nTotal;							// �ܹ���Ҫ���͵ĸ���
	// ��������
	CNL_PINGER(int nMaxDst);								// nMaxDst�������Լ����Ŀ���ַ����
	~CNL_PINGER();
	int		AddUnit(const struct sockaddr_in &addr);		// ���һ��ping�ĵ�ַ(����ɹ������ض����ID�ţ����򷵻�<0)
	int		AddUnit(const char *cszIPPort);					// ͨ���ִ����һ��ping�ĵ�ַ(���غ���һ������һ��)
	int		DelUnit(int nID);								// ɾ��һ��ping����
	int		ClearAllUnit();									// ������е�Ԫ
	int		Tick();											// һ���߼��˶�(���շ��ذ����ط�)
	int		ResetUnit(int nID);
	int		ResetAllUnit();
	bool	IsAllOver(int nID);
	// ������е�pingͳ��
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
	int		SendEchoReq(DSTUNIT_T *pUnit);					// ��ָ����ַ����echo����
};

// ���̵߳�ping������
class	CNL_PINGER_MAN
{
public:
	CNL_PINGER			m_pinger;							// ����ֱ������pinger�Ĳ������緢�ʹ�������ʱ�ȡ�
private:
	n_whcmn::whlock		m_lock;
	n_whcmn::whtid_t	m_tid;
	bool				m_stopthread;
	int					m_interval;
public:
	CNL_PINGER_MAN(int nMaxDst);
	~CNL_PINGER_MAN();
	int		StartThread(int nInterval=0);					// ��ʼ�����߳�(nInterval���ڲ�����select�ĳ�ʱ��Ϊ0��ʹ��Ĭ�ϵļ����50)
	int		EndThread();									// ���������߳�
	int		Tick();											// һ���߼��˶�(���շ��ذ����ط�)
															// �����thread���ã�������ط�0��ʾӦ���߳̽����ˣ�
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
