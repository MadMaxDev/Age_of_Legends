//***************************************************************//
//  ģ������CPGTime								                 //
//                                                               //
//  Ŀ�ģ����ܽӽ�windows�µĿ�������linux�µ�ʱ����			 //
//                                                               //
//  ���ߣ�swolf(����)                                            //
//                                                               //
//  �޸��ߣ�                                                     //
//                                                               //
//  ��������MinGW Developer Studio 2.05                          //
//                                                               //
//  ��ʷ��2006-1-18 14:15 ����                                   //
//                                                               //
//                                                               //
//  ��ע��                                                       //
//                                                               //
//***************************************************************//

#ifndef __CPGTime_H__
#define __CPGTime_H__

#include <time.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

class CPGTimeSpan
{
public:
	CPGTimeSpan() throw();
	CPGTimeSpan( time_t time ) throw();
	CPGTimeSpan( long lDays, int nHours, int nMins, int nSecs ) throw();

	long GetDays() const throw();
	long GetTotalHours() const throw();
	long GetHours() const throw();
	long GetTotalMinutes() const throw();
	long GetMinutes() const throw();
	long GetTotalSeconds() const throw();
	long GetSeconds() const throw();

	time_t GetTimeSpan() const throw();

	CPGTimeSpan operator+( CPGTimeSpan span ) const throw();
	CPGTimeSpan operator-( CPGTimeSpan span ) const throw();
	CPGTimeSpan& operator+=( CPGTimeSpan span ) throw();
	CPGTimeSpan& operator-=( CPGTimeSpan span ) throw();
	bool operator==( CPGTimeSpan span ) const throw();
	bool operator!=( CPGTimeSpan span ) const throw();
	bool operator<( CPGTimeSpan span ) const throw();
	bool operator>( CPGTimeSpan span ) const throw();
	bool operator<=( CPGTimeSpan span ) const throw();
	bool operator>=( CPGTimeSpan span ) const throw();

	//[����]pszFormat:��ʽ
	//����;��ʽ���������ַ���
	string Format( const char *pszFormat ) const;
	//[����]pszFormat:��ʽ [���]pszReturn:��ʽ�������ɵ��ַ���  [����]nSize:�����洢���������ַ����鳤��
	//����;ָ���ʽ���������ַ�����ָ��
	char *Format( const char* pszFormat,char* pszReturn,int nSize ) const;


private:
	time_t m_timeSpan;
};

class CPGTime
{
public:
	static CPGTime GetCurTime() throw();

	CPGTime() throw();
	CPGTime( time_t time ) throw();
	CPGTime( int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
		int nDST = -1 );

	CPGTime& operator=( time_t time ) throw();

	CPGTime& operator+=( CPGTimeSpan span ) throw();
	CPGTime& operator-=( CPGTimeSpan span ) throw();

//	CPGTime operator-( CPGTimeSpan span ) const throw();
	CPGTime operator+( CPGTimeSpan span ) const throw();
/*
	CPGTimeSpan operator-( CPGTime time ) const throw();
	bool operator==( CPGTime time ) const throw();
	bool operator!=( CPGTime time ) const throw();
	bool operator<( CPGTime time ) const throw();
	bool operator>( CPGTime time ) const throw();
	bool operator<=( CPGTime time ) const throw();
	bool operator>=( CPGTime time ) const throw();
*/
	struct tm* GetGmtTm( struct tm* ptm = NULL ) const throw();
	struct tm* GetLocalTm( struct tm* ptm = NULL ) const throw();
	
	inline time_t GetData() const throw();
	//�������ݣ�������time_t
	void  SetData( time_t iData );

	void  SetTime(  int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
		int nDST = -1 );

	int GetYear() const throw();
	int GetMonth() const throw();
	int GetDay() const throw();
	int GetHour() const throw();
	int GetMinute() const throw();
	int GetSecond() const throw();
	int GetDayOfWeek() const throw();


	//[����]pszFormat:��ʽ
	//         %a��������� %A���������� %b����·��� %B�����·���
    //         %y��(00-99) %Y��(�������) %m��(01-12) %d��(01-31) %j��(001-366)
    //         %Hʱ(00-23) %Iʱ(01-12) %p������(AM/PM) %M��(00-59) %S��(00-59) 
    //         %U������Ϊ׼��������(00-53) %W����һΪ׼��������(00-59) %wһ���е�������(0-6)
    //         %c���ں�ʱ��(��/��/�� ʱ���֣���) %x����(��/��/��) %Xʱ��(ʱ���֣���) 
    //         %z��%Zʱ������
	//����;��ʽ���������ַ���
	string Format( const char *pszFormat ) const;
	//[����]pszFormat:��ʽ [���]pszReturn:��ʽ�������ɵ��ַ���  [����]nSize:�����洢���������ַ����鳤��
	//����;ָ���ʽ���������ַ�����ָ��
	char *Format( const char* pszFormat,char* pszReturn,int nSize ) const;
	//[����]pszFormat:��ʽ
	//����;��ʽ���������ַ���
	string FormatGmt( const char *pszFormat ) const;
	//[����]pszFormat:��ʽ [���]pszReturn:��ʽ�������ɵ��ַ���  [����]nSize:�����洢���������ַ����鳤��
	//����;ָ���ʽ���������ַ�����ָ��
	char *FormatGmt( const char* pszFormat,char* pszReturn,int nSize ) const;

	
private:
	time_t m_time;

};

inline time_t CPGTime::GetData() const throw()
{
	return( m_time );
}


inline CPGTimeSpan operator-( CPGTime t0, CPGTime t1 )
{
	return( CPGTimeSpan( t0.GetData()-t1.GetData() ) );
}

inline bool operator==( CPGTime t0, CPGTime t1 )
{
	return t0.GetData() == t1.GetData();
}

inline bool operator!=( CPGTime t0, CPGTime t1 )
{
	return t0.GetData() != t1.GetData();
}

inline bool operator<( CPGTime t0, CPGTime t1 )
{
	return t0.GetData() < t1.GetData();
}

inline bool operator>( CPGTime t0, CPGTime t1 )
{
	return t0.GetData() > t1.GetData();
}

inline bool operator<=( CPGTime t0, CPGTime t1 )
{
	return t0.GetData() <= t1.GetData();
}

inline bool operator>=( CPGTime t0, CPGTime t1 )
{
	return t0.GetData() >= t1.GetData();
}


const int maxTimeBufferSize = 128;
const long maxDaysInSpan  =	3615897L;



#endif  // __CPGTime_H__
