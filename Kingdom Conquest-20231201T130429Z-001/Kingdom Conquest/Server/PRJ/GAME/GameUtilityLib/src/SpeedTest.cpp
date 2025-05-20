/********************************************************
	Copyright 1996-2005, Pixel Software Corporation, 
	All rights reserved.

	Module name: 速度测试

	Purpose: 优化程序

	Author: Solo

	Compiler: Microsoft Visual C++ 6.0
	
	History:	2005-Mar-11		Solo	Created
				2005-Apr~May	Erica	Modified
				2007-Mar-29		Erica	Add To 寻仙

	Remarks:

*********************************************************/


#include "../inc/SpeedTest.h"
#include "../inc/LogMngInf.h"
#include "../inc/misc.h"
#include "../inc/DMtype.h"

#if CPU_GUARD_ENABLED

#include <vector>
#include <WHNET/inc/whnetudp.h>

typedef unsigned char byte_t;
typedef unsigned int uint_t;

struct cpu_guard_data
{
    uint64_t    m_tick;
    size_t      m_count;
    size_t      m_size;
    char*       m_name;

    cpu_guard_data(const char* n)
    {
        m_size = strlen(n);
        m_name = (char*)malloc(m_size+1);
        strcpy(m_name, n);
        m_tick = 0;
        m_count= 0;
    }
    ~cpu_guard_data()
    {
        if(m_name)
        {
            free(m_name);
        }
    }
};

namespace
{

// 获取CPU时钟周期的函数，跨平台。
#ifdef _WIN32
__declspec(naked) uint64_t cpu_guard_counter()
{
    __asm
    {
        rdtsc
        ret
    }
}
uint64_t cpu_guard_freq(int test_times)
{
    double fac = 0.0;
    uint64_t count = 0;
    uint64_t pcount = 0;

    for(int i = 0; i < test_times; ++ i)
    {
        uint64_t start = cpu_guard_counter();
        uint64_t pcstart;
        ::QueryPerformanceCounter((LARGE_INTEGER*)&pcstart);
        ::Sleep(40);
        uint64_t pcstop;
        ::QueryPerformanceCounter((LARGE_INTEGER*)&pcstop);
        uint64_t stop = cpu_guard_counter();
        
        uint64_t c = stop-start;
        uint64_t pc = pcstop-pcstart;
        double f = (double)pc/(double)c;
        if(f > fac)
        {
            fac = f;
            count = c;
            pcount = pc;
        }
    }

    uint64_t pfreq;
    ::QueryPerformanceFrequency((LARGE_INTEGER*)&pfreq);
    
    uint64_t cpufreq = count*pfreq/pcount;
    return cpufreq;
}
#else
// 从魏华那里抄来的
static uint64_t cpu_guard_counter()
{
	unsigned int	l,h;
    __asm__ __volatile__("rdtsc" : "=a" (l), "=d" (h));

	unsigned long long	cycles = h;
	cycles	= (cycles << 32) | l;
	return	cycles;
}

uint64_t cpu_guard_freq(int test_times)
{
	struct timeval		ts0, ts1;
	uint64_t c0, c1;
    uint64_t freq = 0xFFFFFFFFffffffffLL;

    for(int i = 0; i < test_times; ++ i)
    {
	    c0	= cpu_guard_counter();
	    gettimeofday(&ts0, NULL);
	    wh_sleep(200);
	    gettimeofday(&ts1, NULL);
	    c1	= cpu_guard_counter();

        uint64_t q = ((c1-c0)*1000000)/((ts1.tv_sec - ts0.tv_sec)*1000000 + ts1.tv_usec - ts0.tv_usec);
        if(q < freq)
        {
            freq = q;
        }
    }
    return freq;
}

#endif

uint64_t cpu_guard_build_freq()
{
    static uint64_t f = cpu_guard_freq(8);
    return f;
}

struct cpu_guard_vec
{
    typedef std::vector<cpu_guard_data*>  data_vec;

    data_vec    m_datas;
    size_t      m_data_size;
    SOCKET      m_sock;
    sockaddr_in m_host_addr;
    bool        m_debug;
    bool        m_should_clear;
    bool        m_should_debug_out;

    cpu_guard_vec()
    {
        m_datas.reserve(128);
        m_data_size = 0;
        m_debug = false;
        m_should_clear = false;
        m_should_debug_out = false;
        m_sock  = INVALID_SOCKET;
        memset(&m_host_addr, 0, sizeof(m_host_addr));
#ifdef _WIN32
        // 采用丢弃不合理数据的方法，不再绑定处理器了。
        // SetProcessAffinityMask(GetCurrentProcess(), 1);
#endif
    }
    ~cpu_guard_vec()
    {
        if(m_debug)
        {
            debug_out();
        }
        data_vec::iterator i = m_datas.begin();
        data_vec::iterator e = m_datas.end();
        for(; i != e; ++ i)
        {
            cpu_guard_data* d = *i;
            delete d;
        }
        m_datas.clear();
        if(m_sock != INVALID_SOCKET)
        {
            closesocket(m_sock);
        }
    }

    void debug_out()
    {
        double fac = 1.0/(double)cpu_guard_freq(4);
        data_vec::iterator i = m_datas.begin();
        data_vec::iterator e = m_datas.end();
        for(; i != e; ++ i)
        {
            cpu_guard_data* d = *i;
            char buf[4096];
            sprintf(buf, "%s|%gs\n", d->m_name, (double)d->m_tick*fac);
#ifdef _WIN32
            OutputDebugString(buf);
#endif
        }
    }
    void clear()
    {
        data_vec::iterator i = m_datas.begin();
        data_vec::iterator e = m_datas.end();
        for(; i != e; ++ i)
        {
            cpu_guard_data* d = *i;
            d->m_tick   = 0;
            d->m_count  = 0;
        }
    }

    static bool str_cmp(const char* a, const char* b)
    {
        return strcmp(a, b) < 0;
    }

    cpu_guard_data* get(const char* n)
    {
        data_vec::iterator i = m_datas.begin();
        data_vec::iterator e = m_datas.end();
        for(; i != e; ++ i)
        {
            cpu_guard_data* d = *i;
            if(strcmp(d->m_name, n) == 0)
            {
                return d;
            }
        }
        cpu_guard_data* d = new cpu_guard_data(n);
        m_data_size += d->m_size+(8+4+1);
        m_datas.push_back(d);
        return d;
    }
};

static cpu_guard_vec g_cpu_guard_vec;

} // namespace

cpu_guard_data* cpu_guard_data_get(const char* n)
{
    return g_cpu_guard_vec.get(n);
}

cpu_guard_data* cpu_guard_data_get(const char* n1, const char* n2)
{
    size_t s1 = strlen(n1);
    size_t s2 = strlen(n2);
    char* n = (char*)malloc(s1+s2+1);
    memcpy(n, n1, s1);
    strcpy(n+s1, n2);
    cpu_guard_data* d = g_cpu_guard_vec.get(n);
    free(n);
    return d;
}

void cpu_guard_clear()
{
    g_cpu_guard_vec.m_should_clear = true;
}

void cpu_guard_debug(bool v)
{
    g_cpu_guard_vec.m_debug = true;
}

void cpu_guard_debug_out()
{
    g_cpu_guard_vec.m_should_debug_out = true;
}

void cpu_guard_host(const char* addr)
{
    if(addr == 0)
    {
        g_cpu_guard_vec.m_sock = INVALID_SOCKET;
        return;
    }
    unsigned a, b, c, d, p;
    sscanf(addr, "%d.%d.%d.%d:%d", &a, &b, &c, &d, &p);
    char ipstr[32];
    sprintf(ipstr, "%d.%d.%d.%d", a, b, c, d);
    sockaddr_in n;
    memset(&n, 0, sizeof(n));
    n_whnet::cmn_get_saaddr_by_ipnport(&n, ipstr, p);
    g_cpu_guard_vec.m_host_addr = n;
    g_cpu_guard_vec.m_sock = n_whnet::udp_create_socket(0);
}

void cpu_guard_send()
{
    if(g_cpu_guard_vec.m_should_clear)
    {
        g_cpu_guard_vec.m_should_clear = false;
        g_cpu_guard_vec.clear();
    }
    if(g_cpu_guard_vec.m_should_debug_out)
    {
        g_cpu_guard_vec.m_should_debug_out = false;
        g_cpu_guard_vec.debug_out();
    }

    if(g_cpu_guard_vec.m_sock == INVALID_SOCKET)
    {
        return;
    }
    if(g_cpu_guard_vec.m_data_size == 0)
    {
        return;
    }
    size_t len = g_cpu_guard_vec.m_data_size+8;
    byte_t* buf = (byte_t*)malloc(len);
    byte_t* p = buf;
    *(uint64_t*)p = cpu_guard_build_freq();
    p += 8;

    cpu_guard_vec::data_vec::iterator i = g_cpu_guard_vec.m_datas.begin();
    cpu_guard_vec::data_vec::iterator e = g_cpu_guard_vec.m_datas.end();
    for(; i != e; ++ i)
    {
        cpu_guard_data* d = *i;
        *(uint64_t*)p = d->m_tick;
        p += 8;
        *(uint_t*)p = (uint_t)d->m_count;
        p += 4;
        memcpy(p, d->m_name, d->m_size);
        p[d->m_size] = 0;
        p += d->m_size+1;
    }

    n_whnet::udp_sendto(g_cpu_guard_vec.m_sock, buf, len, &g_cpu_guard_vec.m_host_addr);
    free(buf);
}

cpu_guard_auto::cpu_guard_auto(cpu_guard_data* d)
{
    m_data = d;
    m_tick = cpu_guard_counter();
}

cpu_guard_auto::~cpu_guard_auto()
{
    uint64_t t = cpu_guard_counter();
    if(t <= m_tick)
    {
        return;
    }
    m_tick = t - m_tick;
    m_data->m_tick += m_tick;
    ++ m_data->m_count;
}

#endif // CPU_GUARD_ENABLED

CSpeedTest::CSpeedTest()
{
#ifdef	WIN32
	m_iNum	 	 = 0;
	m_iCount 	 = 0;

	m_llBegCount = NULL;
	m_llEndCount = NULL;

	m_pData		 = NULL;
	m_pMaxData	 = NULL;
	m_pSumData	 = NULL;
	
	m_pTimes	 = NULL;
	m_pMaxTimes  = NULL;
	m_pSumTimes  = NULL;
#endif
}

CSpeedTest::~CSpeedTest()
{
#ifdef	WIN32
	MyDeleteArray( m_llBegCount );
	MyDeleteArray( m_llEndCount );

	MyDeleteArray( m_pData );
	MyDeleteArray( m_pMaxData );
	MyDeleteArray( m_pSumData );

	MyDeleteArray( m_pTimes );
	MyDeleteArray( m_pMaxTimes );
	MyDeleteArray( m_pSumTimes );
#endif
}

void CSpeedTest::Build( int iNum )
{
#ifdef	WIN32
	m_iCount = 0;

	if ( iNum < 1 )
	{
		return;
	}

	m_llBegCount = new LARGE_INTEGER[iNum];
	if ( m_llBegCount == NULL )
	{
		return;
	}
	memset( m_llBegCount,0,sizeof(LARGE_INTEGER)*iNum );

	m_llEndCount = new LARGE_INTEGER[iNum];
	if ( m_llEndCount == NULL )
	{
		return;
	}
	memset( m_llEndCount,0,sizeof(LARGE_INTEGER)*iNum );

	m_pData = new LONGLONG[iNum];
	if ( m_pData == NULL )
	{
		return;
	}
	memset( m_pData,0,sizeof(LONGLONG)*iNum );

	m_pMaxData = new int[iNum];
	if ( m_pMaxData == NULL )
	{
		return;
	}
	memset( m_pMaxData,0,sizeof(int)*iNum );

	m_pSumData = new int[iNum];
	if ( m_pSumData == NULL )
	{
		return;
	}
	memset( m_pSumData,0,sizeof(int)*iNum );

	m_pTimes = new int[iNum];
	if ( m_pTimes == NULL )
	{
		return;
	}
	memset( m_pTimes,0,sizeof(int)*iNum );

	m_pMaxTimes = new int[iNum];
	if ( m_pMaxTimes == NULL )
	{
		return;
	}
	memset( m_pMaxTimes,0,sizeof(int)*iNum );

	m_pSumTimes = new int[iNum];
	if ( m_pSumTimes == NULL )
	{
		return;
	}
	memset( m_pSumTimes,0,sizeof(int)*iNum );

	LARGE_INTEGER pliFreq;
	QueryPerformanceFrequency(&pliFreq);
	m_llCountFrq = pliFreq.QuadPart;

	m_iNum = iNum;
#endif
}

void CSpeedTest::Reset()
{
#ifdef	WIN32
	if ( m_iNum < 1 )
	{
		return;
	}

	m_iCount = 1;
	
	memset( m_llBegCount,0,sizeof(LARGE_INTEGER)*m_iNum );
	memset( m_llEndCount,0,sizeof(LARGE_INTEGER)*m_iNum );
	
	memset( m_pData,0,sizeof(LONGLONG)*m_iNum );
	memset( m_pMaxData,0,sizeof(int)*m_iNum );
	memset( m_pSumData,0,sizeof(int)*m_iNum );
	
	memset( m_pTimes,0,sizeof(int)*m_iNum );
	memset( m_pMaxTimes,0,sizeof(int)*m_iNum );
	memset( m_pSumTimes,0,sizeof(int)*m_iNum );
#endif
}

void CSpeedTest::Tick()
{
#ifdef	WIN32
	m_iCount++;

	for ( int i=0; i<m_iNum; i++ )
	{	
		m_iTemp = GetMilliSec(m_pData[i]);

		m_pSumData[i] += m_iTemp; // 记录总调用时间

		if ( m_iTemp > m_pMaxData[i] )
		{
			m_pMaxData[i] = m_iTemp;
		}

		if ( m_pTimes[i] > m_pMaxTimes[i] )
		{
			m_pMaxTimes[i] = m_pTimes[i];
		}
		
		m_pData[i]	= 0;
		m_pTimes[i]	= 0;
	}
#endif
}

void CSpeedTest::Begin( int idx )
{
#ifdef	WIN32
	if ( idx < m_iNum )
	{
		QueryPerformanceCounter(&m_llBegCount[idx]);
	}
#endif
}

void CSpeedTest::End( int idx )
{
#ifdef	WIN32
	if ( idx < m_iNum )
	{
		QueryPerformanceCounter(&m_llEndCount[idx]);

		// 由于是多处理器运行, EndCount有可能小于BegCount
		m_llTemp = m_llEndCount[idx].QuadPart - m_llBegCount[idx].QuadPart;

		m_pData[idx] += m_llTemp; // 一 tick 之内调用 count
		m_pTimes[idx]++;	// 一 tick 之内调用次数
		m_pSumTimes[idx]++;	// 记录总调用次数
	}
#endif
}

void CSpeedTest::OutputLog()
{
#ifdef	WIN32
	if ( m_iCount == 0 )
	{
		return;
	}
	string szInfo;
	szInfo = "SpeedTest";
	for ( int i=0; i<m_iNum; i++ )
	{
		char szTemp[10240];
		sprintf( szTemp, "	%d	%d	%d	%d	%d	%d	||"
			,GetMilliSec(m_pData[i])//curT:
			,m_pMaxData[i]//maxT:
			,m_pSumData[i]/m_iCount//avgT:
			,m_pTimes[i]//curC:
			,m_pMaxTimes[i]//MaxC:
			,m_pSumTimes[i]/m_iCount);//AvgC:
		szInfo += szTemp;

	}
	OutputSysLog( szInfo.c_str() );
#endif
}

