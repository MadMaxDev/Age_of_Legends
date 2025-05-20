// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtime.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单时间相关函数
// CreationDate : 2003-08-08
// ChangeLOG    : 2004-07-15 修改了linux的获得tickcount的方法，增加了wh_gettickcount_tickme函数，不过为了和原来的程序兼容，先直接在每次gettickcount中计算，这样会慢一些。
//                2004-09-25 whlooper中直接比较下次的时刻，减少了一次判断的减法
//                2005-09-27 增加wh_gethourstr
//                2006-01-19 增加了whlooper::makefirst()功能
//                2008-05-27 wh_gettimefromstr增加了如果只有HHMMSS部分则默认为是今天的时刻的功能

#ifndef	__WHTIME_H__
#define	__WHTIME_H__

#include <time.h>

namespace n_whcmn
{

typedef unsigned int	whtick_t;
enum
{
	WHTIME_20080101		= 1199116800,						// 2008-01-01 00:00:00的时刻
															// 可能有很多日子以这个为参照
};

////////////////////////////////////////////////////////////////////
// 系统时间相关函数
////////////////////////////////////////////////////////////////////
// 注意：时间是网络编程中的一个重要成分，所以要提取出跨平台的函数
// 这个在n_whnet::cmn_init中放了，如果要编写和网络无关的又要用到tickcount的，就需要直接调用这个了
// 这个在linux下会多占用一些时间(具体见wh_gettickcount_calibrate的实现)
void	wh_gettickcount_calibrate(int nMS=1000, bool bPrintRst=true);
// 设置起始参考tick时间（这个主要用于做tick过0的测试）
// 这个必须在wh_gettickcount_calibrate之后调用
void	wh_setstarttickref(whtick_t t);
void	wh_setstarttickref_tomakeoverflow(int beforeoverflow);
// 获得系统的毫秒数
whtick_t	wh_gettickcount();
// 获得两个时刻的差，一般来说，只要在时间差较小很小的范围内
// 这个差能够正确表示两个时刻的大小关系，哪怕有时刻回绕了也没问题
inline int wh_tickcount_diff(whtick_t t1, whtick_t t2)
{
	return	int(t1 - t2);
}
inline whtick_t	wh_tick_faraway(whtick_t t)
{
	return	t + 0x7F000000;
}

// 获取时间
time_t	wh_time();
// 设置时间
void	wh_settime(time_t t);
// 设置时间从现在开始变化这么多
void	wh_setdtime(int dt);
// 设置时间起始偏移
void	wh_settimeref(int dt);

// 获得日期时间，格式YYYY-MM-DD HH:MM:SS
const char *	wh_gettimestr(time_t t, char *szStr = NULL);
inline const char *	wh_gettimestr()
{
	return	wh_gettimestr(wh_time());
}
// 从字串获得时间，格式YYYY-MM-DD[ HH:MM:SS]
time_t	wh_gettimefromstr(const char *szStr);

// 获得简单的时间串（sohu要求的：YYYYMMDD-HH:MM:SS）
const char *		wh_getsmptimestr(time_t t, char *szStr = NULL);
inline const char *	wh_getsmptimestr()
{
	return	wh_getsmptimestr(wh_time());
}

// 获得小时分秒的时间串，格式HH:MM:SS
const char *	wh_gettimestr_HMS(time_t t, char *szStr = NULL, char cSep = ':');
inline const char *	wh_gettimestr_HMS()
{
	return	wh_gettimestr_HMS(wh_time());
}

// 获得简单的时间串（可用作文件名的：YYYYMMDD-HH.MM.SS）
const char *		wh_getsmptimestr_for_file(time_t t, char *szStr = NULL);
inline const char *	wh_getsmptimestr_for_file()
{
	return	wh_getsmptimestr_for_file(wh_time());
}

// 获得以小时为基础字串名，如：05092709表示05年7月27日9点
const char *		wh_gethourstr(time_t t, char *szStr = NULL);
inline const char *	wh_gethourstr()
{
	return	wh_gethourstr(wh_time());
}

// 获得以天为基础字串名，如：2005-09-27表示2005年7月27日
const char *		wh_getdaystr(time_t t, char *szStr = NULL);
inline const char *	wh_getdaystr()
{
	return	wh_getdaystr(wh_time());
}
// 获得以天为基础字串名（简单版），如：050927表示05年7月27日
const char *		wh_getsmpdaystr(time_t t, char *szStr = NULL);
inline const char *	wh_getsmpdaystr()
{
	return	wh_getsmpdaystr(wh_time());
}

// 将时间转化到第一天的时间（即距离当天0点的秒数）
time_t	wh_timeinday(time_t t);
// 比较在一天内的时间先后，比如昨天的14点大于今天的13点
int		wh_timecmpinday(time_t t1, time_t t2);
// 判断时间是否在一天的一个时段内
bool	wh_timeinrangeinday(time_t t, time_t t1, time_t t2);
bool	wh_timeinrangeinday(time_t try1, time_t try2, time_t t1, time_t t2);
// 获得以时间t0的当天的0点为基准的，过了sec秒的时间
time_t	wh_makeindaytime(time_t t0, int sec);

// 获得今天的日子(这个多用于写日志名，一天一个日志。如果t不为0则获得t对应的天)
int		wh_gettoday(time_t t=0);
// 获得今天0点的时刻
time_t	wh_getday_begintime(time_t t=0);
// 获得本周的第一天0点的时刻
time_t	wh_getweek_begintime(time_t t=0);
// 获得本月的第一天0点的时刻
time_t	wh_getmonth_begintime(time_t t=0);

// 挂起线程一定时间(毫秒)
void	wh_sleep(int __ms);

// 获取机器开启毫秒时刻（精度100毫秒）
int		wh_getontime();

////////////////////////////////////////////////////////////////////
// 循环计数器，即只在一定数值范围内循环
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
// 统计，比如统计网络发送/接收速率
////////////////////////////////////////////////////////////////////
template<typename _Ty>
class	whtimestat
{
private:
	whtick_t		m_nLastTime;							// 上次统计的时间
	_Ty				m_TotalVal;								// 值的总和
	_Ty				m_LastStat;								// 上次的统计值(单位/秒)
public:
	whtimestat()
	: m_nLastTime(0)
	, m_TotalVal(0), m_LastStat(0)
	{ reset(); }
	void	addval(_Ty val)									// 增加一次统计
	{
		m_TotalVal		+= val;
	}
	void	calcstat()
	{
		whtick_t	t	= wh_gettickcount();
		int	nDiff = wh_tickcount_diff(t, m_nLastTime);
		if( nDiff<=0 )
		{
			// 不计算
			return;
		}
		m_LastStat	= (_Ty)(((double)m_TotalVal * 1000) / nDiff);
	}
	inline _Ty		gettotal() const
	{
		return	m_TotalVal;
	}
	inline _Ty		curstat() const							// 得到当前统计值
	{
		return	m_LastStat;
	}
	void	reset()											// 重新开始统计
	{
		m_TotalVal		= 0;
		m_nLastTime		= wh_gettickcount();
	}
};

////////////////////////////////////////////////////////////////////
// 如果是非数值，那么他们可能要重载"="号和"()"
////////////////////////////////////////////////////////////////////
template<typename _Ty>
class	whcountstat
{
private:
	_Ty				m_TotalVal;								// 值的总和
	int				m_TotalCount;							// 总计数
	_Ty				m_LastStat;								// 上次的统计值(单位/秒)
	_Ty				m_MaxStat;								// 最大的统计值(单位/秒)
	_Ty				m_MaxValue;								// 最大值(单位)
public:
	whcountstat()
	: m_TotalVal(0), m_TotalCount(0)
	, m_LastStat(0), m_MaxStat(0), m_MaxValue(0)
	{
	}
	void	addval(_Ty val)									// 增加一次统计
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
	inline _Ty		curstat() const							// 得到当前统计值
	{
		return	m_LastStat;
	}
	inline _Ty		maxstat() const							// 得到最大统计值
	{
		return	m_MaxStat;
	}
	inline _Ty		maxval() const
	{
		return	m_MaxValue;
	}
	inline void	reset()										// 重新开始统计
	{
		m_TotalVal		= 0;
		m_TotalCount	= 0;
		m_MaxValue		= 0;
		// m_MaxStat需要载resetmax中重置
	}
	inline void	resetmax()
	{
		m_MaxStat		= 0;
	}
};

////////////////////////////////////////////////////////////////////
// 获得时间间隔
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
	// 获得毫秒数
	inline whtick_t	getInterval() const
	{
		return	m_nInterval;
	}
	// 获得浮点的秒数
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
// 每隔一段时间做一件事情，或者用来帧测超时
////////////////////////////////////////////////////////////////////
class	whlooper
{
protected:
	whtick_t	m_nNextTick;								// 下次触发的时刻
	int			m_nInterval;								// 做事的间隔(毫秒)
public:
	whlooper() : m_nNextTick(0), m_nInterval(1000)
	{}
	whlooper(int nInterval) : m_nNextTick(0), m_nInterval(nInterval)
	{}
	inline void	setinterval(int nInterval)					// 设置间隔
	{
		m_nInterval	= nInterval;
	}
	inline void	makefirst()									// 让第一次调用一定被触发
	{
		m_nNextTick	= 0;
	}
	inline void	reset()										// 重新开始
	{
		m_nNextTick	= wh_gettickcount() + m_nInterval;
	}
	inline bool	check() const								// 判断是否到了时候
	{
		if( m_nNextTick==0 )
		{
			// 也就是说最开始至少做一次
			return	true;
		}
		return	wh_tickcount_diff(wh_gettickcount(), m_nNextTick) >= 0;
	}
};

////////////////////////////////////////////////////////////////////
// 帧率控制器
////////////////////////////////////////////////////////////////////
class	whframecontrol
{
protected:
	int			m_nMSPerTick;								// 每个Tick需要的毫秒数
	int			m_nStatPeriod;								// 一次统计的周期(毫秒)
	whtick_t	m_nExpected;								// 期望在这个Tick达到的时刻
	whtick_t	m_nStatEnd;									// 本次统计结束的时刻
public:
	whframecontrol();
	void	SetFR(float fFR);								// 设置帧率(帧/秒)，注意最后产生的帧率不一定和这个一致，可能会高一些。
															// 因为内部是通过每帧的时间来计算的。
	void	SetStatPeriod(int nMS);							// 设置统计周期(毫秒)
	void	Tick();											// 放在需要帧率控制的主循环中，里面会调用sleep
	void	Reset();										// 避免调节时间造成的大时间差
};

////////////////////////////////////////////////////////////////////
// 流量控制器
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 按一天时间的整点触发事件
// 比如一天的每分钟或者每个小时的开始作一件事情
////////////////////////////////////////////////////////////////////
class	whdaytimelooper
{
protected:
	unsigned int	m_nInterval;							// 间隔的秒数
	time_t			m_nNextEventTime;						// 下次预定发生事件的时刻
public:
	whdaytimelooper();
	void	setinterval(unsigned int nSec);					// 设置间隔的秒数
	bool	check() const;									// 判断是否到了时候
	void	gonext();										// 开始下次等待
};

////////////////////////////////////////////////////////////////////
// 按每周的固定时间触发事情
////////////////////////////////////////////////////////////////////
class	whweektimelooper
{
protected:
	time_t			m_nNextEventTime;						// 下次预定发生事件的时刻
public:
	whweektimelooper();
	void	settrigger(int nWeekDay, unsigned int nSec);	// 星期几(0~6)和某个秒数
	bool	check() const;									// 判断是否到了时候
	void	gonext();										// 开始下次等待
	void	makesurenotnow();
	inline time_t	GetNextEventTime() const
	{
		return	m_nNextEventTime;
	}
};

////////////////////////////////////////////////////////////////////
// 在每天的定时做一件事情
////////////////////////////////////////////////////////////////////
class	whdaytimecrontab
{
protected:
	int				m_nTimeOfDay;							// 从0点开始的秒数(每天就在这个时刻做事)
	time_t			m_nNextEventTime;						// 下次预定发生事件的时刻
	float			m_fDays;								// 其实也可以几天做一次(默认是1)
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
	void	settimeofday(int nTOD, bool bNotRightNow=false);// 设置m_nTimeOfDay，如果bNotRightNow为真则表示计算的今天的时间如果超过了当前时间就自动跳到明天
	bool	check() const;									// 判断是否到了时候
	void	gonext();										// 开始下次等待
	void	makesurenotnow();								// 保证m_nNextEventTime的时间比现在要迟后（免得刚启动就发生）
};


////////////////////////////////////////////////////////////////////
// 峰值统计
////////////////////////////////////////////////////////////////////
template<typename _Ty>
struct	whtimepeakvalstat_t
{
	int		nTimeInterval;									// 一段时间的长度
	int		nLastPeakTick;									// 上次时间段的开始
	_Ty		val;											// 当前值
	_Ty		peakval;										// 总峰值
	_Ty		peakvalintime;									// 一段时间内的峰值
	whtimepeakvalstat_t()
	: nTimeInterval(60000)									// 默认60秒
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
	void	setval(_Ty __val)								// 设置新值
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
// 为DLL之间信息同步
////////////////////////////////////////////////////////////////////
void *	WHCMN_TIME_STATIC_INFO_Out();
void	WHCMN_TIME_STATIC_INFO_In(void *pInfo);

}		// EOF namespace n_whcmn

#endif	// EOF __WHTIME_H__
