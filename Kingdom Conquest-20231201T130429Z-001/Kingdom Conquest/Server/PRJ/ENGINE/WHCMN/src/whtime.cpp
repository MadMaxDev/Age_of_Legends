// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtime.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ��ʱ����غ���
// CreationDate : 2003-08-08
// ChangeLog    :
//                2004-11-03 ���ӳ�ʼ�ο�ʱ�䣬�������������ڲ���tick��0ʱ���������
//                2004-11-03 ��calibrate��ʱ�����Ϊ1000ms
//                2005-08-24 ʹ��timeGetTime�滻��GetTickCount
//                2006-07-25 ������VS2003����ʹ��timeGetTime����֡�First-chance exception at 0x77f8f281 in Tst_d.exe: 0xC0000005: Access violation writing location 0x00000010.��

#include "../inc/wh_platform.h"
#include "../inc/whtime.h"
#include "../inc/whstring.h"

#if defined( WIN32 )					// Windows�µ�ͷ		{
#include <Mmsystem.h>
#pragma comment(lib, "Winmm")
#endif									// EOF Windows�µ�ͷ	}
#if defined( __GNUC__ )					// Linux�µ�ͷ			{
#include <sys/time.h>
#include <unistd.h>
#endif									// EOF Linux�µ�ͷ		}
#include <stdio.h>

namespace n_whcmn
{

#pragma pack(1)
struct	WHCMN_TIME_STATIC_INFO_T
{
	// time��ʼ�ο�ʱ��
	int					g_timeref;
	// tick��ʼ�ο�ʱ��
	whtick_t			g_starttickref;
	// һ�����Ӧ��cpuʱ��Ƭ��(���linux�������)
#ifdef	__GNUC__
	unsigned long long	g_nCPUCountPerMilisecond;
#endif
	WHCMN_TIME_STATIC_INFO_T()
	: g_timeref(0)
	, g_starttickref(0)
#ifdef	__GNUC__
	, g_nCPUCountPerMilisecond(0)
#endif
	{
	}
};
#pragma pack()
static WHCMN_TIME_STATIC_INFO_T		l_si;
static WHCMN_TIME_STATIC_INFO_T		*l_psi	= &l_si;
#define	G_TIMEREF					l_psi->g_timeref
#define	G_STARTTICKREF				l_psi->g_starttickref
#ifdef	__GNUC__
#define	G_NCPUCOUNTPERMILISECOND	l_psi->g_nCPUCountPerMilisecond
#endif

void	wh_setstarttickref(whtick_t t)
{
	G_STARTTICKREF	= t;
}
void	wh_setstarttickref_tomakeoverflow(int beforeoverflow)
{
	whtick_t	nNow	= wh_gettickcount();
	nNow		= 0 - beforeoverflow - nNow;
	wh_setstarttickref(nNow);
}

#if defined (WIN32)
void	wh_gettickcount_calibrate(int nMS, bool bPrintRst)
{
	// ����windowsʲôҲ��������
	wh_setstarttickref(0);
}
whtick_t	wh_gettickcount()
{
	//return	GetTickCount() + G_STARTTICKREF;	
	// �����Ҫ����ȷһЩ�ľ���ҪtimeGetTime()
	// GetTickCount�ľ�����ż�Ļ�������16����
	return	timeGetTime();
}
void	wh_sleep(int __ms)
{
	Sleep(__ms);
}

#endif

#if defined (__GNUC__)
static struct timeval	g_starttime	= {0,0};
// ע�⣺ʱ�����������е�һ����Ҫ�ɷ֣�����Ҫ��ȡ����ƽ̨�ĺ���
// �����Ǹ���ȡcpuʱ���������ģ���������ͱ���Ҫ�г�ʼ�����м��tick
///////////////////////////////
// Returns the 64-bit cycle register
///////////////////////////////
/*
unsigned long long get_cpu_cyclecount()
{
	unsigned int		lo_cycles, hi_cycles;
	unsigned long long	cycles;
	asm volatile (
			".byte 0x0f, 0xa2" : : : "eax", "ebx", "ecx", "edx"
		);
	asm volatile (
			".byte 0x0f, 0x31\r\n"
			"movl %%eax, %0\r\n"
			"movl %%edx, %1" 
			: "=r" (lo_cycles), "=r" (hi_cycles)
			: 
			: "eax", "edx"
		);
	cycles	= hi_cycles; 
	cycles	= (cycles << 32) | lo_cycles;
	return	cycles;
}
*/
// ֱ��ʹ��TSC��
// When in protected or virtual 8086 mode, the time stamp disable (TSD) flag in register CR4 restricts the use of the RDTSC instruction as follows.
// When the TSD flag is clear, the RDTSC instruction can be executed at any privilege level; when the flag is set, the instruction can only be executed at privilege level 0.
// (When in real-address mode, the RDTSC instruction is always enabled.)
// The time-stamp counter can also be read with the RDMSR instruction, when executing at privilege level 0.
unsigned long long get_cpu_cyclecount()
{
	#define rdtsc(low,high)      __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
	unsigned int	l,h;
	rdtsc(l,h);

	unsigned long long	cycles = h;
	cycles	= (cycles << 32) | l;
	return	cycles;
}

// ��һ���汾��
/*
inline void get_cpu_cyclecount_2(unsigned long long * ticks)
{
	__asm
	{
		push edx;
		push ecx;
		mov ecx,ticks;
		_emit 0Fh
		_emit 31h
		mov [ecx],eax;
		mov [ecx+4],edx;
		pop ecx;
		pop edx;
	}
}
*/
void	wh_gettickcount_calibrate(int nMS, bool bPrintRst)
{
	if( G_NCPUCOUNTPERMILISECOND>0 )
	{
		// ��Ӧ�ó�ʼ������
		return;
	}

	wh_setstarttickref(0);

	struct timeval		ts0, ts1;
	unsigned long long	c0, c1;

	gettimeofday(&ts0, NULL);
	c0	= get_cpu_cyclecount();
	wh_sleep(nMS);
	gettimeofday(&ts1, NULL);
	c1	= get_cpu_cyclecount();

	// ����һ��CPU��Ƶ��ʱ��ı���
	G_NCPUCOUNTPERMILISECOND
		= (c1 - c0) * 1000
		/ ((ts1.tv_usec - ts0.tv_usec) + 1000000*(ts1.tv_sec - ts0.tv_sec))
		;

#ifdef	_DEBUG
	if( bPrintRst )
	{
		printf("whtick CPU Frequence:%u MHz%s", (unsigned int)(G_NCPUCOUNTPERMILISECOND/1000), WHLINEEND);
	}
#endif
}
whtick_t	wh_gettickcount()
{
	// get_cpu_cyclecount() / G_NCPUCOUNTPERMILISECOND �ܿ��ܻ�õ�һ������4�ֽڱ�ʾ��Χ��������ֱ��ȡ����Ľ������
	return	whtick_t(get_cpu_cyclecount() / G_NCPUCOUNTPERMILISECOND) + G_STARTTICKREF;
}

////////////////////////////////////////////////////////////////////
// ������Ǹ���gettimeofday���ġ�gettimeofdayʵ��ֻ��10����ľ��ȡ�
////////////////////////////////////////////////////////////////////
void	wh_gettickcount_bytod_reset()
{
	// �Խ����0����Ϊ��ʼʱ��(86400��һ���������)
	g_starttime.tv_sec	= wh_time();
	g_starttime.tv_sec	-= (g_starttime.tv_sec%86400);
	g_starttime.tv_usec	= 0;
}
// ���ϵͳ�ĺ�����
whtick_t	wh_gettickcount_bytod()
{
	struct timeval	t;
	gettimeofday(&t, NULL);
	return	(t.tv_sec - g_starttime.tv_sec)*1000
			+ (t.tv_usec) / 1000	// ��Ϊ�Ǵ�0�㿪ʼ���Բ��ü���
			//+ (t.tv_usec - g_starttime.tv_usec) / 1000
			;
}
// �����߳�һ��ʱ��(����)����linux��������ܻ���20����
void	wh_sleep(int __ms)
{
	// usleep(__ms*1000L); ��˵usleep���signal���
	struct timespec	req;
	req.tv_sec		= __ms / 1000;
	req.tv_nsec		= __ms % 1000 * 1000000L;
	// ����nanosleep�ľ�����linux�ϲ��Ժ���ֻ��20����
	nanosleep(&req, NULL);
}
#endif

time_t	wh_time()
{
	return	time(NULL) + G_TIMEREF;
}
void	wh_settime(time_t t)
{
	G_TIMEREF	= t - time(NULL);
}
void	wh_setdtime(int dt)
{
	G_TIMEREF	+= dt;
}
void	wh_settimeref(int dt)
{
	G_TIMEREF	= dt;
}


// �������ʱ�䣬��ʽYYYY-MM-DD HH:MM:SS
const char *	wh_gettimestr(time_t t, char *szStr)
{
	static char	stszStr[256];
	if( !szStr )
	{
		szStr	= stszStr;
	}

	struct tm	*ptm = localtime(&t);
	// ptm����Ϊ�յģ����t�Ǹ��Ƿ���ֵ�Ļ�
	if( ptm )
	{
		sprintf(szStr, "%04d-%02d-%02d %02d:%02d:%02d"
			, 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday
			, ptm->tm_hour, ptm->tm_min, ptm->tm_sec
			);
	}
	else
	{
		szStr[0]	= 0;
	}

	return	szStr;
}

// ���Сʱ�����ʱ�䴮����ʽHH:MM:SS
const char *	wh_gettimestr_HMS(time_t t, char *szStr, char cSep)
{
	static char	stszStr[256];
	if( !szStr )
	{
		szStr	= stszStr;
	}

	struct tm	*ptm = localtime(&t);
	// ptm����Ϊ�յģ����t�Ǹ��Ƿ���ֵ�Ļ�
	if( ptm )
	{
		sprintf(szStr, "%02d%c%02d%c%02d"
			, ptm->tm_hour, cSep, ptm->tm_min, cSep, ptm->tm_sec
			);
	}
	else
	{
		szStr[0]	= 0;
	}

	return	szStr;
}

time_t	wh_gettimefromstr(const char *szStr)
{
	// ���ִ����ʱ�䣬��ʽYYYY-MM-DD[ HH:MM:SS]
	char		szDate[128]="", szTime[128]="";
	struct tm	tm1;
	memset(&tm1, 0, sizeof(tm1));

	wh_strsplit("ss", szStr, " ", szDate, szTime);
	if( szTime[0]==0 && strchr(szDate,':') )	// �����ֻ��HH:MM:SS�Ĳ��֣�����Ϊ�ǽ����ĳ��ʱ��
	{
		strcpy(szTime, szDate);
		wh_getdaystr(wh_time(), szDate);
	}
	wh_strsplit("ddd", szDate, "-", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday);
	wh_strsplit("ddd", szTime, ":", &tm1.tm_hour, &tm1.tm_min, &tm1.tm_sec);
	// �����жϿ��Ա�֤���ֱ������0�����������0ʱ��
	if( tm1.tm_year==0 && tm1.tm_mon==0 )
	{
		return	0;
	}
	//
	tm1.tm_year	-= 1900;
	tm1.tm_mon	--;
	tm1.tm_isdst = -1;
	return	mktime(&tm1);
}
// ��ü򵥵�ʱ�䴮��sohuҪ��ģ�YYYYMMDD-HH:MM:SS��
const char *	wh_getsmptimestr(time_t t, char *szStr)
{
	static char	stszStr[256];
	if( !szStr )
	{
		szStr	= stszStr;
	}

	struct tm	*ptm = localtime(&t);
	if( ptm )
	{
		// ptm����Ϊ���ģ����t�Ǹ��Ƿ���ֵ�Ļ�
		sprintf(szStr, "%04d%02d%02d-%02d:%02d:%02d"
			, 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday
			, ptm->tm_hour, ptm->tm_min, ptm->tm_sec
			);
	}
	else
	{
		szStr[0]	= 0;
	}

	return	szStr;
}
// ��ü򵥵�ʱ�䴮���������ļ����ģ�YYYYMMDD-HH.MM.SS��
const char *	wh_getsmptimestr_for_file(time_t t, char *szStr)
{
	static char	stszStr[256];
	if( !szStr )
	{
		szStr	= stszStr;
	}

	struct tm	*ptm = localtime(&t);
	if( ptm )
	{
		// ptm����Ϊ���ģ����t�Ǹ��Ƿ���ֵ�Ļ�
		sprintf(szStr, "%04d%02d%02d-%02d.%02d.%02d"
			, 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday
			, ptm->tm_hour, ptm->tm_min, ptm->tm_sec
			);
	}
	else
	{
		szStr[0]	= 0;
	}

	return	szStr;
}
const char *	wh_gethourstr(time_t t, char *szStr)
{
	static char	stszStr[256];
	if( !szStr )
	{
		szStr	= stszStr;
	}
	struct tm	*ptm = localtime(&t);
	if( !ptm )
	{
		return	NULL;
	}
	sprintf(szStr, "%02d%02d%02d%02d"
		, (1900+ptm->tm_year)%100
		, 1+ptm->tm_mon
		, ptm->tm_mday
		, ptm->tm_hour
		);
	return	szStr;
}
const char *	wh_getdaystr(time_t t, char *szStr)
{
	static char	stszStr[256];
	if( !szStr )
	{
		szStr	= stszStr;
	}
	struct tm	*ptm = localtime(&t);
	if( !ptm )
	{
		return	NULL;
	}
	sprintf(szStr, "%04d-%02d-%02d"
		, (1900+ptm->tm_year)
		, 1+ptm->tm_mon
		, ptm->tm_mday
		);
	return	szStr;
}
const char *	wh_getsmpdaystr(time_t t, char *szStr)
{
	static char	stszStr[256];
	if( !szStr )
	{
		szStr	= stszStr;
	}
	struct tm	*ptm = localtime(&t);
	if( !ptm )
	{
		return	NULL;
	}
	sprintf(szStr, "%02d%02d%02d"
		, (1900+ptm->tm_year)%100
		, 1+ptm->tm_mon
		, ptm->tm_mday
		);
	return	szStr;
}

// ��ȡ����ʱ�̣����100���룩
int		wh_getontime()
{
#ifdef	WIN32
	FILETIME	ft;
	GetSystemTimeAsFileTime(&ft);
	whtick_t	dwT	= wh_gettickcount();
	whllsc_t	w;
	w.l[0]			= ft.dwLowDateTime;
	w.l[1]			= ft.dwHighDateTime;
	w.I				/= 10000;
	w.I				-= dwT;
	w.I				/= 100;
	return	w.l[0];
#else
	time_t	now		= time(NULL);
	whtick_t	t	= wh_gettickcount()/1000;
	return	now - t;
#endif
}

time_t	wh_timeinday(time_t t)
{
	struct tm	*ptm1 = localtime(&t);
	struct tm	tm1, tm2;
	memset(&tm1, 0, sizeof(tm1));
	tm1.tm_hour	= ptm1->tm_hour;
	tm1.tm_min	= ptm1->tm_min;
	tm1.tm_sec	= ptm1->tm_sec;
	tm1.tm_year	= 100;
	tm1.tm_mday	= 1;
	memset(&tm1, 0, sizeof(tm2));
	tm1.tm_isdst = -1;
	tm2.tm_isdst = -1;
	return	mktime(&tm1) - mktime(&tm2);
}
void	wh_timeinday(time_t &t1, time_t &t2)
{
	t1	= wh_timeinday(t1);
	t2	= wh_timeinday(t2);
	if( t2<=t1 )
	{
		// ����t1��t2֮�䲻����쵽24Сʱ��
		t1	-= 3600*24;
	}
}
int		wh_timecmpinday(time_t t1, time_t t2)
{
	t1	= wh_timeinday(t1);
	t2	= wh_timeinday(t2);
	return	int(t1-t2);
}
bool	wh_timeinrangeinday(time_t t, time_t t1, time_t t2)
{
	t	= wh_timeinday(t);
	wh_timeinday(t1, t2);

	return	t>=t1 && t<=t2;
}
bool	wh_timeinrangeinday(time_t try1, time_t try2, time_t t1, time_t t2)
{
	wh_timeinday(try1, try2);
	wh_timeinday(t1, t2);
	return	( try1>=t1 && try1<=t2 )
		||  ( try2>=t1 && try2<=t2 );
}
time_t	wh_makeindaytime(time_t t0, int sec)
{
	struct tm	*ptm1 = localtime(&t0);
	struct tm	tm1;
	memcpy(&tm1, ptm1, sizeof(tm1));
	tm1.tm_hour	= 0;
	tm1.tm_min	= 0;
	tm1.tm_sec	= 0;
	tm1.tm_isdst = -1;
	return		mktime(&tm1) + sec;
}

int		wh_gettoday(time_t t)
{
	if( t==0 )	t = wh_time();
	struct tm	*ptm = localtime(&t);
	if( !ptm )
	{
		return	0;
	}
	return	ptm->tm_mday;
}
time_t		wh_getday_begintime(time_t t)
{
	if( t==0 )	t = wh_time();
	return	wh_makeindaytime(t, 0);
}
time_t		wh_getweek_begintime(time_t t)
{
	if( t==0 )	t = wh_time();
	struct tm	*ptm = localtime(&t);
	if( !ptm )
	{
		return	0;
	}
	struct tm	tm1	= *ptm;
	tm1.tm_hour	= 0;
	tm1.tm_min	= 0;
	tm1.tm_sec	= 0;
	tm1.tm_isdst = -1;
	return		mktime(&tm1) - 24*3600*(ptm->tm_wday);
}
time_t		wh_getmonth_begintime(time_t t)
{
	if( t==0 )	t = wh_time();
	struct tm	*ptm = localtime(&t);
	if( !ptm )
	{
		return	0;
	}
	struct tm	tm1	= *ptm;
	tm1.tm_hour	= 0;
	tm1.tm_min	= 0;
	tm1.tm_sec	= 0;
	tm1.tm_isdst = -1;
	return		mktime(&tm1) - 24*3600*(ptm->tm_mday-1);
}

////////////////////////////////////////////////////////////////////
// whframecontrol
////////////////////////////////////////////////////////////////////
whframecontrol::whframecontrol()
: m_nMSPerTick(0), m_nStatPeriod(10000)
{
	Reset();
}
void	whframecontrol::SetFR(float fFR)
{
	m_nMSPerTick	= int(1000 / fFR);
	Reset();
}
void	whframecontrol::SetStatPeriod(int nMS)
{
	m_nStatPeriod	= nMS;
	Reset();
}
void	whframecontrol::Tick()
{
	// �Ƚϵ�ǰʱ�̺������ﵽ��ʱ�̵Ĳ���
	whtick_t	nNow	= wh_gettickcount();
	int			nDiff	= wh_tickcount_diff(m_nExpected, nNow);
	int			nToEnd	= wh_tickcount_diff(nNow, m_nStatEnd);
	if( nDiff<=0 )
	{
		if( nToEnd>=0 )
		{
			m_nExpected	= nNow;
		}
	}
	else
	{
		// ��Ϣ��ô��ʱ��
		wh_sleep(nDiff);
	}
	// �����Ѿ�������ͳ������
	if( nToEnd>=0 )
	{
		m_nStatEnd		= m_nExpected + m_nStatPeriod;
	}
	m_nExpected			+= m_nMSPerTick;
}
void	whframecontrol::Reset()
{
	m_nExpected	= wh_gettickcount();
	m_nStatEnd	= m_nExpected + m_nStatPeriod;
}

////////////////////////////////////////////////////////////////////
// whdaytimelooper ��һ��ʱ������㴥���¼�
////////////////////////////////////////////////////////////////////
whdaytimelooper::whdaytimelooper()
: m_nInterval(3600)
{
	m_nNextEventTime	= wh_time();
}
void	whdaytimelooper::setinterval(unsigned int nSec)
{
	m_nInterval	= nSec;
}
bool	whdaytimelooper::check() const
{
	return	wh_time() >= m_nNextEventTime;
}
void	whdaytimelooper::gonext()
{
	m_nNextEventTime	= ( (wh_time() / m_nInterval) + 1 ) * m_nInterval;
}

////////////////////////////////////////////////////////////////////
// whweektimelooper ��ÿ�ܵĹ̶�ʱ�䴥������
////////////////////////////////////////////////////////////////////
whweektimelooper::whweektimelooper()
{
	m_nNextEventTime	= wh_time();
}
void	whweektimelooper::settrigger(int nWeekDay, unsigned int nSec)
{
	time_t	now		= wh_time();
	struct tm	*ptm1 = localtime(&now);
	struct tm	tm1;
	memcpy(&tm1, ptm1, sizeof(tm1));
	tm1.tm_hour	= 0;
	tm1.tm_min	= 0;
	tm1.tm_sec	= nSec;
	// ������ڵ���������
	int		nAfter	= nWeekDay-tm1.tm_wday;
	if( nAfter<0 )
	{
		// ����Ѿ���ȥ�˾������һ����
		nAfter	+= 7;
	}
	tm1.tm_isdst = -1;
	m_nNextEventTime	= mktime(&tm1) + nAfter*24*3600;
}
bool	whweektimelooper::check() const
{
	return	wh_time() >= m_nNextEventTime;
}
void	whweektimelooper::gonext()
{
	// �����һ����
	m_nNextEventTime	+= 7*24*3600;
}
void	whweektimelooper::makesurenotnow()
{
	time_t	now	= wh_time();
	while( now>=m_nNextEventTime )
	{
		gonext();
	}
}

////////////////////////////////////////////////////////////////////
// ��ÿ��Ķ�ʱ��һ������
////////////////////////////////////////////////////////////////////
whdaytimecrontab::whdaytimecrontab()
: m_nTimeOfDay(0)
, m_fDays(1)
{
	m_nNextEventTime	= wh_time();
}
void	whdaytimecrontab::settimeofday(int nTOD, bool bNotRightNow)
{
	m_nTimeOfDay		= nTOD;
	// ������쵽���ʱ��
	m_nNextEventTime	= wh_makeindaytime(wh_time(), nTOD);
	if( bNotRightNow && check() )
	{
		gonext();
	}
}
bool	whdaytimecrontab::check() const
{
	return	wh_time() >= m_nNextEventTime;
}
void	whdaytimecrontab::gonext()
{
	m_nNextEventTime	+= (time_t)(m_fDays*(24*3600));
}
void	whdaytimecrontab::makesurenotnow()
{
	time_t	now	= wh_time();
	while( now>=m_nNextEventTime )
	{
		gonext();
	}
}

////////////////////////////////////////////////////////////////////
// ΪDLL֮����Ϣͬ��
////////////////////////////////////////////////////////////////////
void *	WHCMN_TIME_STATIC_INFO_Out()
{
	return	&l_si;
}
void	WHCMN_TIME_STATIC_INFO_In(void *pInfo)
{
#ifdef	_DEBUG
	printf("WHCMN_TIME_STATIC_INFO_In %p %p%s", l_psi, pInfo, WHLINEEND);
#endif
	l_psi	= (WHCMN_TIME_STATIC_INFO_T *)pInfo;
}

}		// EOF namespace n_whcmn
