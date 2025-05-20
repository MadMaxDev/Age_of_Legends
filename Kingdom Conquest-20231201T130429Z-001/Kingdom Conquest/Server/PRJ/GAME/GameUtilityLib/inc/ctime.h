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

	//[输入]pszFormat:格式
	//返回;格式化后生成字符串
	string Format( const char *pszFormat ) const;
	//[输入]pszFormat:格式 [输出]pszReturn:格式化后生成的字符串  [输入]nSize:用来存储输出结果的字符数组长度
	//返回;指向格式化后生成字符串的指针
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
	//设置数据，参数是time_t
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


	//[输入]pszFormat:格式
	//         %a简短星期名 %A完整星期名 %b简短月份名 %B完整月份名
    //         %y年(00-99) %Y年(完整年份) %m月(01-12) %d日(01-31) %j日(001-366)
    //         %H时(00-23) %I时(01-12) %p上下午(AM/PM) %M分(00-59) %S秒(00-59) 
    //         %U以周日为准的星期数(00-53) %W以周一为准的星期数(00-59) %w一周中的星期数(0-6)
    //         %c日期和时间(日/月/年 时：分：秒) %x日期(日/月/年) %X时间(时：分：秒) 
    //         %z或%Z时区名字
	//返回;格式化后生成字符串
	string Format( const char *pszFormat ) const;
	//[输入]pszFormat:格式 [输出]pszReturn:格式化后生成的字符串  [输入]nSize:用来存储输出结果的字符数组长度
	//返回;指向格式化后生成字符串的指针
	char *Format( const char* pszFormat,char* pszReturn,int nSize ) const;
	//[输入]pszFormat:格式
	//返回;格式化后生成字符串
	string FormatGmt( const char *pszFormat ) const;
	//[输入]pszFormat:格式 [输出]pszReturn:格式化后生成的字符串  [输入]nSize:用来存储输出结果的字符数组长度
	//返回;指向格式化后生成字符串的指针
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
