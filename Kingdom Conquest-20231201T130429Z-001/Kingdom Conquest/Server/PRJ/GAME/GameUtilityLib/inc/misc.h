/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: misc

	Purpose: 一些方法

	Author: soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 20004-11-27 

	Remarks:

*********************************************************/

#ifndef  __MISC_H
#define  __MISC_H
#include <math.h>
#include <stdio.h>
//#include "geo.h"
#include "DMtype.h"

template<class T>
void MyDelete(T& ptr)
{
	if( ptr )
	{
		delete ptr;
	}
	ptr = NULL;
}

template<class T>
void MyDeleteArray(T& ptr)
{
	if( ptr )
	{
		delete[] ptr;
	}
	ptr = NULL;
}

//#define MyDelete(ptr){ if (ptr) delete ptr; ptr = NULL; }
//#define MyDeleteArray(ptr){ if (ptr) delete [] ptr; ptr = NULL; }


#define SAFE_DELETE_GDI(p) if(p){::DeleteObject(p);p = NULL;}
#define RELEASE_CONTROL(p) if(p){p->Destroy( );p = NULL;}
#ifndef SAFE_DELETE
#define SAFE_DELETE(ptr){ if (ptr) delete ptr; ptr = NULL; }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(ptr){ if (ptr) delete [] ptr; ptr = NULL; }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#define  WINDOW_CX  (GetSystemMetrics(SM_CXSCREEN)) 
#define  WINDOW_CY  (GetSystemMetrics(SM_CYSCREEN))
#define  WINDOW_X   ((GetSystemMetrics(SM_CXSCREEN) - GAMEWINDOW_CX)/2)
#define  WINDOW_Y   ((GetSystemMetrics(SM_CYSCREEN) - GAMEWINDOW_CY)/2)
#define  MENU_CY   (GetSystemMetrics(SM_CYMENU)) 
#define  VSCROLL_CX   (GetSystemMetrics(SM_CXVSCROLL )) 
#define  HSCROLL_CY   (GetSystemMetrics(SM_CYHSCROLL )) 

#define	 ID_INVALID		(-1)

#define		_BOOL(a)	((a) ? 1 : 0)

#ifdef WIN32
#define p_vsnprintf _vsnprintf
#else
#define p_vsnprintf vsnprintf
#endif

// 输出debug信息
void OutputDebugStringF( const char* format, ... );


int		GetRank( int i,int j );//求阶程

inline int		FloatToInt( float f )//浮点数4舍5入
{
	if( f - floor( f ) >= 0.5 )
	{
		return ( int)(ceil( f ));
	}
	return ( int)(floor( f ));

}

inline BOOL	BoolChg(BOOL & flag)
{
	if( flag )
	{		
		flag = false;
		return true;
	}
	else 
	{
		flag = true;
		return false;
	}
}

inline BOOL	SetDataReturnChange( int * pData1,int iPar )
{
	if( *pData1 == iPar )
	{
		return false;
	}
	*pData1 = iPar;
	return true;
}


int GetMaxRand( int iMax );//可以取出超过10000的随机数
int RandIntCC( int iMin,int iMax );//获取整型随机数, 包含上限
float RandFloatCC( float fMin,float fMax );//获取浮点数随机数, 包含上限

//booIs true递减，false递加
//return   偏移量	修改iCurHigh
int	GetCurveUnit( int & iCurHigh,const int iCurStep,const BOOL booIs );


inline	BOOL	ByteOnBit( BYTE b,int iPos )
{
	return b  & ( 1 << iPos );
}

inline	BOOL	DwordOnBit( DWORD d,int iPos )
{
	return d  & ( 1 << iPos );
}

LPCSTR	DataToByteStr( BYTE * pData,int iSize );

inline void	F1NearToF2(  float * f1,const float * f2,float fSpeed )
{
	if( *f1 < *f2 )
	{
		*f1 += fSpeed;
		if( *f1 > *f2 )
		{
			*f1 = *f2;
		}
	}
	else if( *f1 > *f2 )
	{
		*f1 -= fSpeed;
		if( *f1 < *f2 )
		{
			*f1 = *f2;
		}
	}
}

BOOL	Angle1NearToAngle1(  float * A1,const float * A2,float fSpeed );

inline void	I1NearToI2(  int * i1,const int * i2,int iSpeed )
{
	if( *i1 < *i2 )
	{
		*i1 += iSpeed;
		if( *i1 > *i2 )
		{
			*i1 = *i2;
		}
	}
	else if( *i1 > *i2 )
	{
		*i1 -= iSpeed;
		if( *i1 < *i2 )
		{
			*i1 = *i2;
		}
	}
}

float GetCurveHigh( float & fCur,float fStep,float fMaxHigh );
float	GetRandAngle();

void	F1NearToF2Ex1(  float * f1,const float * f2,float fSpeed );//速度逐渐减慢

float	GetSphereHigh( float fDist,float fRadius );


#include	<string>
void G_MiscGetStackTrace(std::string& buf);

#endif
