// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtime.h
// Creator      : Wei Hua (κ��)
// Comment      : ��ʱ����غ���
// CreationDate : 2003-08-08
// ChangeLOG    : 2004-07-15 �޸���linux�Ļ��tickcount�ķ�����������wh_gettickcount_tickme����������Ϊ�˺�ԭ���ĳ�����ݣ���ֱ����ÿ��gettickcount�м��㣬��������һЩ��
//                2004-09-25 whlooper��ֱ�ӱȽ��´ε�ʱ�̣�������һ���жϵļ���
//                2005-09-27 ����wh_gethourstr
//                2006-01-19 ������whlooper::makefirst()����
//                2008-05-27 wh_gettimefromstr���������ֻ��HHMMSS������Ĭ��Ϊ�ǽ����ʱ�̵Ĺ���

#ifndef	__WHTIME_H__
#define	__WHTIME_H__

#include <time.h>

namespace n_whcmn
{

typedef unsigned int	whtick_t;
enum
{
	WHTIME_20080101		= 1199116800,						// 2008-01-01 00:00:00��ʱ��
															// �����кܶ����������Ϊ����
};

////////////////////////////////////////////////////////////////////
// ϵͳʱ����غ���
////////////////////////////////////////////////////////////////////
// ע�⣺ʱ�����������е�һ����Ҫ�ɷ֣�����Ҫ��ȡ����ƽ̨�ĺ���
// �����n_whnet::cmn_init�з��ˣ����Ҫ��д�������޹ص���Ҫ�õ�tickcount�ģ�����Ҫֱ�ӵ��������
// �����linux�»��ռ��һЩʱ��(�����wh_gettickcount_calibrate��ʵ��)
void	wh_gettickcount_calibrate(int nMS=1000, bool bPrintRst=true);
// ������ʼ�ο�tickʱ�䣨�����Ҫ������tick��0�Ĳ��ԣ�
// ���������wh_gettickcount_calibrate֮�����
void	wh_setstarttickref(whtick_t t);
void	wh_setstarttickref_tomakeoverflow(int beforeoverflow);
// ���ϵͳ�ĺ�����
whtick_t	wh_gettickcount();
// �������ʱ�̵Ĳһ����˵��ֻҪ��ʱ����С��С�ķ�Χ��
// ������ܹ���ȷ��ʾ����ʱ�̵Ĵ�С��ϵ��������ʱ�̻�����Ҳû����
inline int wh_tickcount_diff(whtick_t t1, whtick_t t2)
{
	return	int(t1 - t2);
}
inline whtick_t	wh_tick_faraway(whtick_t t)
{
	return	t + 0x7F000000;
}

// ��ȡʱ��
time_t	wh_time();
// ����ʱ��
void	wh_settime(time_t t);
// ����ʱ������ڿ�ʼ�仯��ô��
void	wh_setdtime(int dt);
// ����ʱ����ʼƫ��
void	wh_settimeref(int dt);

// �������ʱ�䣬��ʽYYYY-MM-DD HH:MM:SS
const char *	wh_gettimestr(time_t t, char *szStr = NULL);
inline const char *	wh_gettimestr()
{
	return	wh_gettimestr(wh_time());
}
// ���ִ����ʱ�䣬��ʽYYYY-MM-DD[ HH:MM:SS]
time_t	wh_gettimefromstr(const char *szStr);

// ��ü򵥵�ʱ�䴮��sohuҪ��ģ�YYYYMMDD-HH:MM:SS��
const char *		wh_getsmptimestr(time_t t, char *szStr = NULL);
inline const char *	wh_getsmptimestr()
{
	return	wh_getsmptimestr(wh_time());
}

// ���Сʱ�����ʱ�䴮����ʽHH:MM:SS
const char *	wh_gettimestr_HMS(time_t t, char *szStr = NULL, char cSep = ':');
inline const char *	wh_gettimestr_HMS()
{
	return	wh_gettimestr_HMS(wh_time());
}

// ��ü򵥵�ʱ�䴮���������ļ����ģ�YYYYMMDD-HH.MM.SS��
const char *		wh_getsmptimestr_for_file(time_t t, char *szStr = NULL);
inline const char *	wh_getsmptimestr_for_file()
{
	return	wh_getsmptimestr_for_file(wh_time());
}

// �����СʱΪ�����ִ������磺05092709��ʾ05��7��27��9��
const char *		wh_gethourstr(time_t t, char *szStr = NULL);
inline const char *	wh_gethourstr()
{
	return	wh_gethourstr(wh_time());
}

// �������Ϊ�����ִ������磺2005-09-27��ʾ2005��7��27��
const char *		wh_getdaystr(time_t t, char *szStr = NULL);
inline const char *	wh_getdaystr()
{
	return	wh_getdaystr(wh_time());
}
// �������Ϊ�����ִ������򵥰棩���磺050927��ʾ05��7��27��
const char *		wh_getsmpdaystr(time_t t, char *szStr = NULL);
inline const char *	wh_getsmpdaystr()
{
	return	wh_getsmpdaystr(wh_time());
}

// ��ʱ��ת������һ���ʱ�䣨�����뵱��0���������
time_t	wh_timeinday(time_t t);
// �Ƚ���һ���ڵ�ʱ���Ⱥ󣬱��������14����ڽ����13��
int		wh_timecmpinday(time_t t1, time_t t2);
// �ж�ʱ���Ƿ���һ���һ��ʱ����
bool	wh_timeinrangeinday(time_t t, time_t t1, time_t t2);
bool	wh_timeinrangeinday(time_t try1, time_t try2, time_t t1, time_t t2);
// �����ʱ��t0�ĵ����0��Ϊ��׼�ģ�����sec���ʱ��
time_t	wh_makeindaytime(time_t t0, int sec);

// ��ý��������(���������д��־����һ��һ����־�����t��Ϊ0����t��Ӧ����)
int		wh_gettoday(time_t t=0);
// ��ý���0���ʱ��
time_t	wh_getday_begintime(time_t t=0);
// ��ñ��ܵĵ�һ��0���ʱ��
time_t	wh_getweek_begintime(time_t t=0);
// ��ñ��µĵ�һ��0���ʱ��
time_t	wh_getmonth_begintime(time_t t=0);

// �����߳�һ��ʱ��(����)
void	wh_sleep(int __ms);

// ��ȡ������������ʱ�̣�����100���룩
int		wh_getontime();

////////////////////////////////////////////////////////////////////
// ѭ������������ֻ��һ����ֵ��Χ��ѭ��
////////////////////////////////////////////////////////////////////
class	whcyclecounter
{
protected:
	int	m_nMin, m_nMax, m_nRange;
	int	m_nNow;
public:
	whcyclecounter() : m_nMin(0), m_nMax(1), m_nRange(1), m_nNow(0)
	{}
	~whcyclecounter()								{}
	inline void	reset(int nMin, int nMax)
	{
		m_nMin		= nMin;
		m_nMax		= nMax;
		m_nRange	= nMax - nMin;
		m_nNow		= nMin;
	}
	inline void reset()
	{
		m_nNow		= m_nMin;
	}
	inline void	set(int nNow)
	{
		m_nNow	= nNow;
	}
	inline int	add(int nMore)
	{
		m_nNow	+= nMore;
		if( m_nNow>=m_nMax )
		{
			m_nNow	= (m_nNow - m_nMin)%m_nRange + m_nMin;
		}
		else if( m_nNow<m_nMin )
		{
			m_nNow	= m_nMax - (m_nMin - m_nNow)%m_nRange;
			if( m_nNow==m_nMax )
			{
				m_nNow	= m_nMin;
			}
		}
		return	m_nNow;
	}
	inline int	add(int nFrom, int nMore)
	{
		set(nFrom);
		return	add(nMore);
	}
	int		get() const
	{
		return	m_nNow;
	}
};

////////////////////////////////////////////////////////////////////
// ͳ�ƣ�����ͳ�����緢��/��������
////////////////////////////////////////////////////////////////////
template<typename _Ty>
class	whtimestat
{
private:
	whtick_t		m_nLastTime;							// �ϴ�ͳ�Ƶ�ʱ��
	_Ty				m_TotalVal;								// ֵ���ܺ�
	_Ty				m_LastStat;								// �ϴε�ͳ��ֵ(��λ/��)
public:
	whtimestat()
	: m_nLastTime(0)
	, m_TotalVal(0), m_LastStat(0)
	{ reset(); }
	void	addval(_Ty val)									// ����һ��ͳ��
	{
		m_TotalVal		+= val;
	}
	void	calcstat()
	{
		whtick_t	t	= wh_gettickcount();
		int	nDiff = wh_tickcount_diff(t, m_nLastTime);
		if( nDiff<=0 )
		{
			// ������
			return;
		}
		m_LastStat	= (_Ty)(((double)m_TotalVal * 1000) / nDiff);
	}
	inline _Ty		gettotal() const
	{
		return	m_TotalVal;
	}
	inline _Ty		curstat() const							// �õ���ǰͳ��ֵ
	{
		return	m_LastStat;
	}
	void	reset()											// ���¿�ʼͳ��
	{
		m_TotalVal		= 0;
		m_nLastTime		= wh_gettickcount();
	}
};

////////////////////////////////////////////////////////////////////
// ����Ƿ���ֵ����ô���ǿ���Ҫ����"="�ź�"()"
////////////////////////////////////////////////////////////////////
template<typename _Ty>
class	whcountstat
{
private:
	_Ty				m_TotalVal;								// ֵ���ܺ�
	int				m_TotalCount;							// �ܼ���
	_Ty				m_LastStat;								// �ϴε�ͳ��ֵ(��λ/��)
	_Ty				m_MaxStat;								// ����ͳ��ֵ(��λ/��)
	_Ty				m_MaxValue;								// ���ֵ(��λ)
public:
	whcountstat()
	: m_TotalVal(0), m_TotalCount(0)
	, m_LastStat(0), m_MaxStat(0), m_MaxValue(0)
	{
	}
	void	addval(_Ty val)									// ����һ��ͳ��
	{
		m_TotalVal		+= val;
		m_TotalCount	++;
		if( m_MaxValue<val )
		{
			m_MaxValue	= val;
		}
	}
	void	calc()
	{
		if( m_TotalCount==0 )
		{
			m_LastStat	= 0;
		}
		else
		{
			m_LastStat	= m_TotalVal / m_TotalCount;
			if( m_MaxStat<m_LastStat )
			{
				m_MaxStat	= m_LastStat;
			}
		}
	}
	inline _Ty		curstat() const							// �õ���ǰͳ��ֵ
	{
		return	m_LastStat;
	}
	inline _Ty		maxstat() const							// �õ����ͳ��ֵ
	{
		return	m_MaxStat;
	}
	inline _Ty		maxval() const
	{
		return	m_MaxValue;
	}
	inline void	reset()										// ���¿�ʼͳ��
	{
		m_TotalVal		= 0;
		m_TotalCount	= 0;
		m_MaxValue		= 0;
		// m_MaxStat��Ҫ��resetmax������
	}
	inline void	resetmax()
	{
		m_MaxStat		= 0;
	}
};

////////////////////////////////////////////////////////////////////
// ���ʱ����
////////////////////////////////////////////////////////////////////
class	whinterval
{
protected:
	whtick_t	m_nLast;
	whtick_t	m_nInterval;
	time_t		m_nLastTime, m_nThisTime;
public:
	whinterval()
	{
		reset();
	}
	inline void	reset()
	{
		m_nLastTime	= 0;
		m_nThisTime	= 0;
		m_nLast		= wh_gettickcount();
	}
	inline void	tick()
	{
		whtick_t	nThis = wh_gettickcount();
		m_nInterval	= nThis - m_nLast;
		m_nLast		= nThis;
		m_nLastTime	= m_nThisTime;
		m_nThisTime	= wh_time();
	}
	// ��ú�����
	inline whtick_t	getInterval() const
	{
		return	m_nInterval;
	}
	// ��ø��������
	inline float	getfloatInterval() const
	{
		return	float(m_nInterval) / 1000.f;
	}
	inline time_t	getlasttime() const
	{
		return	m_nLastTime;
	}
	inline time_t	getthistime() const
	{
		return	m_nThisTime;
	}
};

////////////////////////////////////////////////////////////////////
// ÿ��һ��ʱ����һ�����飬��������֡�ⳬʱ
////////////////////////////////////////////////////////////////////
class	whlooper
{
protected:
	whtick_t	m_nNextTick;								// �´δ�����ʱ��
	int			m_nInterval;								// ���µļ��(����)
public:
	whlooper() : m_nNextTick(0), m_nInterval(1000)
	{}
	whlooper(int nInterval) : m_nNextTick(0), m_nInterval(nInterval)
	{}
	inline void	setinterval(int nInterval)					// ���ü��
	{
		m_nInterval	= nInterval;
	}
	inline void	makefirst()									// �õ�һ�ε���һ��������
	{
		m_nNextTick	= 0;
	}
	inline void	reset()										// ���¿�ʼ
	{
		m_nNextTick	= wh_gettickcount() + m_nInterval;
	}
	inline bool	check() const								// �ж��Ƿ���ʱ��
	{
		if( m_nNextTick==0 )
		{
			// Ҳ����˵�ʼ������һ��
			return	true;
		}
		return	wh_tickcount_diff(wh_gettickcount(), m_nNextTick) >= 0;
	}
};

////////////////////////////////////////////////////////////////////
// ֡�ʿ�����
////////////////////////////////////////////////////////////////////
class	whframecontrol
{
protected:
	int			m_nMSPerTick;								// ÿ��Tick��Ҫ�ĺ�����
	int			m_nStatPeriod;								// һ��ͳ�Ƶ�����(����)
	whtick_t	m_nExpected;								// ���������Tick�ﵽ��ʱ��
	whtick_t	m_nStatEnd;									// ����ͳ�ƽ�����ʱ��
public:
	whframecontrol();
	void	SetFR(float fFR);								// ����֡��(֡/��)��ע����������֡�ʲ�һ�������һ�£����ܻ��һЩ��
															// ��Ϊ�ڲ���ͨ��ÿ֡��ʱ��������ġ�
	void	SetStatPeriod(int nMS);							// ����ͳ������(����)
	void	Tick();											// ������Ҫ֡�ʿ��Ƶ���ѭ���У���������sleep
	void	Reset();										// �������ʱ����ɵĴ�ʱ���
};

////////////////////////////////////////////////////////////////////
// ����������
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// ��һ��ʱ������㴥���¼�
// ����һ���ÿ���ӻ���ÿ��Сʱ�Ŀ�ʼ��һ������
////////////////////////////////////////////////////////////////////
class	whdaytimelooper
{
protected:
	unsigned int	m_nInterval;							// ���������
	time_t			m_nNextEventTime;						// �´�Ԥ�������¼���ʱ��
public:
	whdaytimelooper();
	void	setinterval(unsigned int nSec);					// ���ü��������
	bool	check() const;									// �ж��Ƿ���ʱ��
	void	gonext();										// ��ʼ�´εȴ�
};

////////////////////////////////////////////////////////////////////
// ��ÿ�ܵĹ̶�ʱ�䴥������
////////////////////////////////////////////////////////////////////
class	whweektimelooper
{
protected:
	time_t			m_nNextEventTime;						// �´�Ԥ�������¼���ʱ��
public:
	whweektimelooper();
	void	settrigger(int nWeekDay, unsigned int nSec);	// ���ڼ�(0~6)��ĳ������
	bool	check() const;									// �ж��Ƿ���ʱ��
	void	gonext();										// ��ʼ�´εȴ�
	void	makesurenotnow();
	inline time_t	GetNextEventTime() const
	{
		return	m_nNextEventTime;
	}
};

////////////////////////////////////////////////////////////////////
// ��ÿ��Ķ�ʱ��һ������
////////////////////////////////////////////////////////////////////
class	whdaytimecrontab
{
protected:
	int				m_nTimeOfDay;							// ��0�㿪ʼ������(ÿ��������ʱ������)
	time_t			m_nNextEventTime;						// �´�Ԥ�������¼���ʱ��
	float			m_fDays;								// ��ʵҲ���Լ�����һ��(Ĭ����1)
public:
	inline void	SetDays(float fDays)
	{
		m_fDays	= fDays;
	}
	inline time_t	GetNextEventTime() const
	{
		return	m_nNextEventTime;
	}
public:
	whdaytimecrontab();
	void	settimeofday(int nTOD, bool bNotRightNow=false);// ����m_nTimeOfDay�����bNotRightNowΪ�����ʾ����Ľ����ʱ����������˵�ǰʱ����Զ���������
	bool	check() const;									// �ж��Ƿ���ʱ��
	void	gonext();										// ��ʼ�´εȴ�
	void	makesurenotnow();								// ��֤m_nNextEventTime��ʱ�������Ҫ�ٺ���ø������ͷ�����
};


////////////////////////////////////////////////////////////////////
// ��ֵͳ��
////////////////////////////////////////////////////////////////////
template<typename _Ty>
struct	whtimepeakvalstat_t
{
	int		nTimeInterval;									// һ��ʱ��ĳ���
	int		nLastPeakTick;									// �ϴ�ʱ��εĿ�ʼ
	_Ty		val;											// ��ǰֵ
	_Ty		peakval;										// �ܷ�ֵ
	_Ty		peakvalintime;									// һ��ʱ���ڵķ�ֵ
	whtimepeakvalstat_t()
	: nTimeInterval(60000)									// Ĭ��60��
	, nLastPeakTick(0)
	, val(0), peakval(0), peakvalintime(0)
	{
	}
	void	clear()
	{
		nLastPeakTick	= 0;
		val				= 0;
		peakval			= 0;
		peakvalintime	= 0;
	}
	void	setval(_Ty __val)								// ������ֵ
	{
		val					= __val;
		if( peakval<__val )
		{
			peakval			= __val;
		}
		if( peakvalintime<__val )
		{
			peakvalintime	= __val;
		}
		if( nLastPeakTick==0
		||  wh_tickcount_diff(wh_gettickcount(), nLastPeakTick)>=nTimeInterval
		)
		{
			peakvalintime	= 0;
			nLastPeakTick	= wh_gettickcount();
		}
	}
};

////////////////////////////////////////////////////////////////////
// ΪDLL֮����Ϣͬ��
////////////////////////////////////////////////////////////////////
void *	WHCMN_TIME_STATIC_INFO_Out();
void	WHCMN_TIME_STATIC_INFO_In(void *pInfo);

}		// EOF namespace n_whcmn

#endif	// EOF __WHTIME_H__
