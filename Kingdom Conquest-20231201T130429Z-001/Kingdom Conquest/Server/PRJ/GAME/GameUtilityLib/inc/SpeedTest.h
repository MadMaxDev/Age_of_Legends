/********************************************************
	Copyright 1996-2005, Pixel Software Corporation, 
	All rights reserved.

	Module name: �ٶȲ���

	Purpose: Ϊ���Ż�����

	Author: Solo

	Compiler: Microsoft Visual C++ 6.0

	History:	2005-Mar-11		Solo	Created
				2005-Apr~May	Erica	Modified
				2007-Mar-29		Erica	Add To Ѱ��
                2007-12-3       Jiangli ����ʹ��rdtscָ����ٶȼ�鷽����

	Remarks:

*********************************************************/


#ifndef __SpeedTest_H
#define __SpeedTest_H

#ifdef	WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// ���Դ򿪻��߹ر��ٶȲ��ԡ�
#define CPU_GUARD_ENABLED 1


#if CPU_GUARD_ENABLED

#define CPU_GUARD(name) \
    static cpu_guard_data* s_cpu_guard_data##name = cpu_guard_data_get(#name);\
    cpu_guard_auto cpu_guard_auto_##name(s_cpu_guard_data##name);

struct cpu_guard_data;
cpu_guard_data* cpu_guard_data_get(const char*);

void cpu_guard_debug_out();
void cpu_guard_host(const char* addr);
void cpu_guard_send();
void cpu_guard_clear();

struct cpu_guard_auto
{
    cpu_guard_data*     m_data;
    unsigned long long  m_tick;
    cpu_guard_auto(cpu_guard_data*);
    ~cpu_guard_auto();
};

#else // CPU_GUARD_ENABLED

#define CPU_GUARD(name)
#define cpu_guard_debug_out()
#define cpu_guard_host(addr)
#define cpu_guard_send()
#define cpu_guard_clear()

#endif // CPU_GUARD_ENABLED


class CSpeedTest
{
	// �����ӿ�
public:
	void	Build( int iNum );
	void	Reset();
	void	OutputLog();

	void	Tick();

	void	Begin( int idx );
	void	End( int idx );
#ifdef	WIN32
	inline int GetMilliSec( LONGLONG llCount ) 
	{
		return (int)( (llCount*1000)/m_llCountFrq );
	}
#endif
	// ��������
public:
	CSpeedTest();
	~CSpeedTest();

private:
#ifdef	WIN32
	int				m_iNum;
	int				m_iCount;
	LONGLONG		m_llCountFrq;

	LARGE_INTEGER	*m_llBegCount;
	LARGE_INTEGER	*m_llEndCount;

	LONGLONG	   *m_pData;

	int			   *m_pMaxData;
	int		       *m_pSumData;
	
	int			   *m_pTimes;
	int			   *m_pMaxTimes;
	int		       *m_pSumTimes;

	LONGLONG		m_llTemp;
	int				m_iTemp;
#endif
};

#endif
