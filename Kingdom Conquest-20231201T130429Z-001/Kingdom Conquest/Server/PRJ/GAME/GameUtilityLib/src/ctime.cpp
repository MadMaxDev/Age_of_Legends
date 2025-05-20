//***************************************************************//
//  模块名：CPGTime								                 //
//                                                               //
//  目的：功能接近windows下的可以用在linux下的时间类			 //
//                                                               //
//  作者：swolf(王威)                                            //
//                                                               //
//  修改者：                                                     //
//                                                               //
//  编译器：MinGW Developer Studio 2.05                          //
//                                                               //
//  历史：2006-1-18 14:15 创建                                   //
//                                                               //
//                                                               //
//  备注：                                                       //
//                                                               //
//***************************************************************//

#ifndef __CPGTime_CPP__
#define __CPGTime_CPP__

#include "../inc/ctime.h"
#include "assert.h"

#include <WHCMN/inc/whtime.h>
using namespace n_whcmn;
/////////////////////////////////////////////////////////////////////////////
// CPGTimeSpan
/////////////////////////////////////////////////////////////////////////////

CPGTimeSpan::CPGTimeSpan() throw() :
	m_timeSpan(0)
{
}

CPGTimeSpan::CPGTimeSpan( time_t time ) throw() :
	m_timeSpan( time )
{
}

CPGTimeSpan::CPGTimeSpan(long lDays, int nHours, int nMins, int nSecs) throw()
{
 	m_timeSpan = nSecs + 60* (nMins + 60* (nHours + 24 * lDays));
}

string CPGTimeSpan::Format( const char *pszFormat ) const
//  * the only valid formats:
//      %D - # of days
//      %H - hour in 24 hour format
//      %M - minute (0-59)
//      %S - seconds (0-59)
//      %% - percent sign
{
	assert( pszFormat != NULL );
	if( pszFormat == NULL )
		return NULL;

	string szReturn = "";

	char strTmp[3];

	char ch;

	while ((ch = *pszFormat++) != '\0')
	{
		if (ch == '%')
		{
			switch (ch = *pszFormat++)
			{
			case '%':
				szReturn += ch;
				break;
			case 'D':
				{
					sprintf(strTmp,"%ld", GetDays());
					szReturn.append(strTmp);
				}
				break;
			case 'H':
				{
					sprintf(strTmp,"%02ld", GetHours());
					szReturn.append(strTmp);
				}
				break;
			case 'M':
				{
					sprintf(strTmp,"%02ld", GetMinutes());
					szReturn.append(strTmp);
				}
				break;
			case 'S':
				{
					sprintf(strTmp,"%02ld", GetSeconds());
					szReturn.append(strTmp);
				}
				break;
			default:
				assert(0);      // probably a bad format character
				break;
			}
		}
		else
		{
			sprintf(strTmp,"%c", ch);
			szReturn.append(strTmp);
		}
	}

	return strTmp;
}
char* CPGTimeSpan::Format( const char* pszFormat,char* pszReturn,int nSize ) const
{
	assert( pszFormat != NULL );
	if( pszFormat == NULL )
		return NULL;

	string szTmp = Format(pszFormat);

	if ( (int)szTmp.length() >= nSize )
	{
		return NULL;
	}

	strcpy(pszReturn,szTmp.c_str());

	return pszReturn;
}

long CPGTimeSpan::GetDays() const throw()
{
	return( m_timeSpan/(24*3600) );
}
long CPGTimeSpan::GetTotalHours() const throw()
{
	return( m_timeSpan/3600 );
}

long CPGTimeSpan::GetHours() const throw()
{
	return( long( GetTotalHours()-(GetDays()*24) ) );
}

long CPGTimeSpan::GetTotalMinutes() const throw()
{
	return( m_timeSpan/60 );
}

long CPGTimeSpan::GetMinutes() const throw()
{
	return( long( GetTotalMinutes()-(GetTotalHours()*60) ) );
}

long CPGTimeSpan::GetTotalSeconds() const throw()
{
	return( m_timeSpan );
}

long CPGTimeSpan::GetSeconds() const throw()
{
	return( long( GetTotalSeconds()-(GetTotalMinutes()*60) ) );
}

time_t CPGTimeSpan::GetTimeSpan() const throw()
{
	return( m_timeSpan );
}

CPGTimeSpan CPGTimeSpan::operator+( CPGTimeSpan span ) const throw()
{
	return( CPGTimeSpan( m_timeSpan+span.m_timeSpan ) );
}

CPGTimeSpan CPGTimeSpan::operator-( CPGTimeSpan span ) const throw()
{
	return( CPGTimeSpan( m_timeSpan-span.m_timeSpan ) );
}

CPGTimeSpan& CPGTimeSpan::operator+=( CPGTimeSpan span ) throw()
{
	m_timeSpan += span.m_timeSpan;
	return( *this );
}

CPGTimeSpan& CPGTimeSpan::operator-=( CPGTimeSpan span ) throw()
{
	m_timeSpan -= span.m_timeSpan;
	return( *this );
}

bool CPGTimeSpan::operator==( CPGTimeSpan span ) const throw()
{
	return( m_timeSpan == span.m_timeSpan );
}

bool CPGTimeSpan::operator!=( CPGTimeSpan span ) const throw()
{
	return( m_timeSpan != span.m_timeSpan );
}

bool CPGTimeSpan::operator<( CPGTimeSpan span ) const throw()
{
	return( m_timeSpan < span.m_timeSpan );
}

bool CPGTimeSpan::operator>( CPGTimeSpan span ) const throw()
{
	return( m_timeSpan > span.m_timeSpan );
}

bool CPGTimeSpan::operator<=( CPGTimeSpan span ) const throw()
{
	return( m_timeSpan <= span.m_timeSpan );
}

bool CPGTimeSpan::operator>=( CPGTimeSpan span ) const throw()
{
	return( m_timeSpan >= span.m_timeSpan );
}

/////////////////////////////////////////////////////////////////////////////
// CPGTime
/////////////////////////////////////////////////////////////////////////////

CPGTime CPGTime::GetCurTime() throw()
{
	return( CPGTime( wh_time() ) );
}

CPGTime::CPGTime() throw() :
	m_time(0)
{
}

CPGTime::CPGTime( time_t time )  throw():
	m_time( time )
{
}

CPGTime::CPGTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
	int nDST)
{
	SetTime(  nYear,nMonth,nDay,nHour,nMin,nSec );
}

string CPGTime::Format(const char* pszFormat) const
{
	assert( pszFormat != NULL );
	if(pszFormat == NULL)
		return NULL;

	char szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = localtime(&m_time);
	if (ptmTemp == NULL ||
		!strftime(szBuffer, maxTimeBufferSize, pszFormat, ptmTemp))
		szBuffer[0] = '\0';
	
	string szTmp = "";
	szTmp.append(szBuffer);

	return szTmp;
}

char *CPGTime::Format( const char* pszFormat,char* pszReturn,int nSize ) const
{
	assert( pszFormat != NULL );
	if( pszFormat == NULL )
		return NULL;

	char szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = localtime(&m_time);
	if (ptmTemp == NULL ||
		!strftime(szBuffer, maxTimeBufferSize, pszFormat, ptmTemp))
		szBuffer[0] = '\0';

	if( (int)strlen(szBuffer) >= nSize )
		return NULL;

	strcpy(pszReturn,szBuffer);

	return pszReturn;
}

string CPGTime::FormatGmt(const char* pszFormat) const
{
	assert( pszFormat != NULL );
	if(pszFormat == NULL)
		return NULL;

	char szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = gmtime(&m_time);
	if (ptmTemp == NULL ||
		!strftime(szBuffer, maxTimeBufferSize, pszFormat, ptmTemp))
		szBuffer[0] = '\0';

	string szTmp = "";
	szTmp.append(szBuffer);

	return szTmp;
}

char *CPGTime::FormatGmt( const char* pszFormat,char* pszReturn,int nSize ) const
{
	assert( pszFormat != NULL );
	if( pszFormat == NULL )
		return NULL;

	char szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = gmtime(&m_time);
	if (ptmTemp == NULL ||
		!strftime(szBuffer, maxTimeBufferSize, pszFormat, ptmTemp))
		szBuffer[0] = '\0';

	if( (int)strlen(szBuffer) >= nSize )
		return NULL;

	strcpy(pszReturn,szBuffer);

	return pszReturn;
}

CPGTime& CPGTime::operator=( time_t time ) throw()
{
	m_time = time;

	return( *this );
}

CPGTime& CPGTime::operator+=( CPGTimeSpan span ) throw()
{
	m_time += span.GetTimeSpan();

	return( *this );
}

CPGTime& CPGTime::operator-=( CPGTimeSpan span ) throw()
{
	m_time -= span.GetTimeSpan();

	return( *this );
}

/*
CPGTime CPGTime::operator-( CPGTimeSpan span ) const throw()
{
	return( CPGTime( m_time-span.GetTimeSpan() ) );
}*/

CPGTime CPGTime::operator+( CPGTimeSpan span ) const throw()
{
	return( CPGTime( m_time+span.GetTimeSpan() ) );
}
/*
CPGTimeSpan CPGTime::operator-( CPGTime time ) const throw()
{
	return( CPGTimeSpan( m_time-time.m_time ) );
}

bool CPGTime::operator==( CPGTime time ) const throw()
{
	return( m_time == time.m_time );
}

bool CPGTime::operator!=( CPGTime time ) const throw()
{
	return( m_time != time.m_time );
}

bool CPGTime::operator<( CPGTime time ) const throw()
{
	return( m_time < time.m_time );
}

bool CPGTime::operator>( CPGTime time ) const throw()
{
	return( m_time > time.m_time );
}

bool CPGTime::operator<=( CPGTime time ) const throw()
{
	return( m_time <= time.m_time );
}

bool CPGTime::operator>=( CPGTime time ) const throw()
{
	return( m_time >= time.m_time );
}
*/

struct tm* CPGTime::GetGmtTm(struct tm* ptm) const throw()
{
	if (ptm != NULL)
	{
		struct tm * ptmTemp;
		ptmTemp = gmtime(&m_time);
		
		if(ptmTemp == NULL)
			return NULL;
		
		*ptm = *ptmTemp;
		
		return ptm;
	}
	else
		return gmtime(&m_time);
}

struct tm* CPGTime::GetLocalTm(struct tm* ptm) const throw()
{
	if (ptm != NULL)
	{
		struct tm* ptmTemp = localtime(&m_time);
		if (ptmTemp == NULL)
			return NULL;    // indicates the m_time was not initialized!

		*ptm = *ptmTemp;
		return ptm;
	}
	else
		return localtime(&m_time);
}

void  CPGTime::SetTime(  int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,int nDST )
{
	struct tm atm;
	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	assert(nDay >= 1 && nDay <= 31);
	atm.tm_mday = nDay;
	assert(nMonth >= 1 && nMonth <= 12);
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	assert(nYear >= 1900);
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = nDST;
	m_time = mktime(&atm);
	assert(m_time != -1);       // indicates an illegal input time

	if(m_time == -1)//构造失败
		m_time = 0;
}

void  CPGTime::SetData( time_t iData )
{
	m_time = iData;
}


int CPGTime::GetYear() const throw()
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? (ptm->tm_year) + 1900 : 0 ; 
}

int CPGTime::GetMonth() const throw()
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_mon + 1 : 0;
}

int CPGTime::GetDay() const throw()
{
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_mday : 0 ; 
}

int CPGTime::GetHour() const throw()
{
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_hour : -1 ; 
}

int CPGTime::GetMinute() const throw()
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_min : -1 ; 
}

int CPGTime::GetSecond() const throw()
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_sec : -1 ;
}

int CPGTime::GetDayOfWeek() const throw()
{ 
	struct tm * ptm;
	ptm = GetLocalTm();
	return ptm ? ptm->tm_wday : -1 ; /* days since Sunday - [0,6] */
}

#endif //__CPGTime_CPP__
