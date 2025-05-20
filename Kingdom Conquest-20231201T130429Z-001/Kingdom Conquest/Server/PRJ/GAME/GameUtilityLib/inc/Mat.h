/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: mat

	Purpose: 一些数学方法

	Author: soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 2004-11-27 

	Remarks: 2006-4-19 Erica - 加入对Bit标志位的处理
	         2006-9-24 魏华将ISIN_CC、ISIN_CO、ISIN_OO改为模板函数，以避免在gcc下因为类型匹配和变量取值范围造成的warning。

*********************************************************/

#ifndef   __MAT_H
#define   __MAT_H
#include  <stdlib.h>
#include  <math.h>
#include  <limits.h>
#include  "DMtype.h"	

// the consts about pi
const float OU_PI	= 3.14159265f;

const float OU_4PI	= OU_PI * 4.0f;
const float OU_3PI	= OU_PI * 3.0f;
const float OU_2PI	= OU_PI * 2.0f;
const float OU_3PI2	= OU_PI * 1.5f;
const float OU_3PI4	= OU_PI * 3.0f / 4.0f;
const float OU_2PI3	= OU_PI * 2.0f / 3.0f;
const float OU_PI2	= OU_PI / 2.0f;
const float OU_PI3	= OU_PI / 3.0f;
const float OU_PI4	= OU_PI / 4.0f;
const float OU_PI6	= OU_PI / 6.0f;
const float OU_PI8	= OU_PI / 8.0f;
const float OU_PI16	= OU_PI / 16.0f;

// for old project
const float PAI       = OU_PI;

const float PI       = (const float)3.1415926; 
const float PI_DIV_2 = (const float)1.5707963;
const float PI2      = (const float)6.2831852;

const float PI_801  = (const float)PI / 8 * 1;
const float PI_802  = (const float)PI / 8 * 2;
const float PI_803  = (const float)PI / 8 * 3;
const float PI_804  = (const float)PI / 8 * 4;
const float PI_805  = (const float)PI / 8 * 5;
const float PI_806  = (const float)PI / 8 * 6;
const float PI_807  = (const float)PI / 8 * 7;
const float PI_808  = (const float)PI / 8 * 8;
const float PI_809  = (const float)PI / 8 * 9;
const float PI_810  = (const float)PI / 8 * 10;
const float PI_811  = (const float)PI / 8 * 11;
const float PI_812  = (const float)PI / 8 * 12;
const float PI_813  = (const float)PI / 8 * 13;
const float PI_814  = (const float)PI / 8 * 14;
const float PI_815  = (const float)PI / 8 * 15;
const float PI_816  = (const float)PI / 8 * 16;


const float PI_1601  = (const float)PI / 16 * 1;
const float PI_1603  = (const float)PI / 16 * 3;
const float PI_1605  = (const float)PI / 16 * 5;
const float PI_1607  = (const float)PI / 16 * 7;
const float PI_1609  = (const float)PI / 16 * 9;
const float PI_1611  = (const float)PI / 16 * 11;
const float PI_1613  = (const float)PI / 16 * 13;
const float PI_1615  = (const float)PI / 16 * 15;
const float PI_1617  = (const float)PI / 16 * 17;
const float PI_1619  = (const float)PI / 16 * 19;
const float PI_1621  = (const float)PI / 16 * 21;
const float PI_1623  = (const float)PI / 16 * 23;
const float PI_1625  = (const float)PI / 16 * 25;
const float PI_1627  = (const float)PI / 16 * 27;
const float PI_1629  = (const float)PI / 16 * 29;
const float PI_1631  = (const float)PI / 16 * 31;

inline BOOL  IsFloatDist( float fSrc,float fDest,float fStep )
{
	return fabs( fSrc - fDest ) <= fStep;
}

inline BOOL  IsAngleDist( float fSrc,float fDest,float fStep )
{
	float fAngle = (float)fabs( fSrc - fDest );
	if( fAngle > PI2 )
	{
		fAngle -= PI2;
	}
	return fAngle <= fStep;
}

inline int		IsOdd(int i)	{return i & 0x00000001;}
inline DWORD	IsOdd(DWORD i)	{return i & 0x00000001;}

#define   MIN_INT      1
#define   MIN_FLOAT    0.0001

#define   LENGTH(a,b)  ((b) - (a) + 1)

#define   MIN(x,y) ( ((x)>(y))? (y):(x) )
#define   MAX(x,y) ( ((x)>(y))? (x):(y) )
#define   MAX_ABS(x,y) ( (abs(x)>abs(y))? abs(x):abs(y) )
#define   MIN_ABS(x,y) ( (abs(x)>abs(y))? abs(y):abs(x) )

#define   ADDC(a,b,a_max)( (( (a) + (b)) >= (a_max)) ?  0 : (a) + (b))

#define   INCC(a,a_max)( (( (a) + 1) >= (a_max)) ?  0 : (a) + 1)
#define   DECC(a,a_max)( (( (a) - 1) < 0) ?  (a_max)-1 : (a) - 1)
#define   SUBCC(a,b,a_max) ( ((b) > (a)) ? (a_max) + (a) - (b) : (a) - (b))
#define   ADDCC(a,b,a_max) ( ((b) + (a) >= (a_max)) ?  (a) + (b) - (a_max) : (a) + (b))

template<typename _Ty>
inline BOOL   ISIN(const _Ty &a,const _Ty &b,const _Ty &c)
{
	return	((a>=b)&&(a<=c)) ? TRUE : FALSE;
}

template<typename _Ty>
inline BOOL   ISIN_CC(const _Ty &a,const _Ty &b,const _Ty &c)
{
	return	((b>=a)&&(b<=c)) ? TRUE : FALSE;
}
template<typename _Ty>
inline BOOL   ISIN_CO(const _Ty &a,const _Ty &b,const _Ty &c)
{
	return	((b>=a)&&(b<c)) ? TRUE : FALSE;
}
template<typename _Ty>
inline BOOL   ISIN_OO(const _Ty &a,const _Ty &b,const _Ty &c)
{
	return	((b>a)&&(b<c)) ? TRUE : FALSE;
}

template<typename t>
inline void SetVauleMin( t& v, const t min )
{
	if(v < min)
	{
		v = min;
	}
}

template<typename t>
inline void SetVauleMax( t& v, const t max )
{
	if(v > max)
	{
		v = max;
	}
}

template<typename t>
inline void SetVauleMinMax( t& v, const t min, const t max )
{
	if(v < min)
	{
		v = min;
	}
	else if(v > max)
	{
		v = max;
	}
}

#define	  Math_Distance(x1,y1,x0,y0) ( sqrt( ((x1) - (x0))*((x1) - (x0)) + ((y1) - (y0))*((y1) - (y0)) ) )

/* tell what quadrant it is */
/* 0 , 1 , 2 , 3 quadrant   */ 
#define   WHAT_QUADRANT(x,y)( ((x)>=0) ? (((y)>=0) ? 0 : 3) : (((y)>=0)? 1 : 2))

//get  sign
#define  SIGN(a)(((a) >= 0 ) ? 1 : -1 )

/* get  direction  if a==0 then 0 */
#define  DIRECT(a)(((a) >= 0 ) ? (((a) == 0) ? 0 : 1) : -1 )

//if 0 then replace with a min number
#define  INT_NOT_0(a)(((a) == 0)? MIN_INT : (a))
#define  FLOAT_NOT_0(a)(((a) == 0)? MIN_FLOAT : (a))



////////////////////////////////////////////////////////////////////
// 

inline int RandInt( int low, int high ) //not include high
{
    int range = high - low;
    int num = rand() % range;
    return( num + low );
}
inline double RandDouble( double low, double high )
{
    double range = high - low;
    double num = range * (double)rand()/(double)RAND_MAX;
    return( num + low );
}


const int   seprate_sin  = 1000;
const float step_sin = PI / seprate_sin;

#define QBIT_MAX	0x0f				/* maximum 4 bits value */
#define BYTE_MAX	0xff				/* maximum unsigned char value */
#define WORD_MAX	0xffff				/* maximum unsigned short value */
#define DWORD_MAX	0xffffffff			/* maximum unsigned int value */
//#define MYINT_MIN	(-2147483647 - 1)	/* minimum (signed) int value */
//#define MYINT_MAX	2147483647			/* maximum (signed) int value */

BYTE Int2QBit( int iData );
BYTE Int2Byte( int iData );
WORD Int2Word( int iData );
int SQWord2Int( SQWORD qwData );
DWORD SQWord2DWord( SQWORD qwData );

//////////////////////////////////////////////////////////////////////////Bit标志位
#define   GetBitFromByte(idx,byData)	( ( byData>>( idx&0x07   ) ) & 0x01 )
#define   GetDBitFromByte(idx,byData)	( ( byData>>((idx&0x03)*2) ) & 0x03 )
#define   GetQBitFromByte(idx,byData)	( ( byData>>((idx&0x01)*4) ) & 0x0F )

bool SetBitToByte( int idx, BYTE & byNewData, BYTE byOldData );
bool SetDBitToByte( int idx, BYTE & byNewData, BYTE byOldData );
bool SetQBitToByte( int idx, BYTE & byNewData, BYTE byOldData );
bool ChgQBitToByte( int idx, int & iNewData, BYTE byOldData );


inline float	GetAngle( float fStartx,float fStarty,float fEndx,float fEndy )
{
	//D3DXVECTOR3 vDir = *pStart - *pEnd;
	float fDirX = fStartx - fEndx;
	float fDirY = fStarty - fEndy;

	float fAngle;

	if( fDirY > 0.0f )			fAngle = -atanf((float)fDirX/(float)fDirY)+PI_DIV_2;
	else if( fDirY < 0.0f )	fAngle = -atanf((float)fDirX/(float)fDirY)-PI_DIV_2;
	else
	{
		if( fDirX > 0  )		fAngle = 0;
		else					fAngle = -PI;//-PI/2
	}
	return fAngle;
}

inline int	GetDegree( float fStartx,float fStarty,float fEndx,float fEndy )
{
	float fDirX = fStartx - fEndx;
	float fDirY = fStarty - fEndy;

	float fAngle;

	if( fDirY > 0.0f )			fAngle = -atanf((float)fDirX/(float)fDirY)+PI_DIV_2;
	else if( fDirY < 0.0f )	fAngle = -atanf((float)fDirX/(float)fDirY)-PI_DIV_2;
	else
	{
		if( fDirX > 0  )		fAngle = 0;
		else					fAngle = -PI;//-PI/2
	}
	int iDegree = 270-(int)((fAngle+PI)*360/PI2 + 0.05f);	// 浮点转整形可能数据丢失，比如11.9999会变成11而不是12

	if( iDegree < 0 )
		iDegree += 360;
	return iDegree;
}

inline void	MoveForDegree( float & fOutx,float & fOuty,int iDegree,float fSpeed )
{
	float fAngle = (float)(270-iDegree)*PI2/360;
	fOutx = fOutx + cosf(fAngle)*fSpeed;
	fOuty = fOuty + sinf(fAngle)*fSpeed;
}

inline void	MoveForAngle( float & fOutx,float & fOuty,float fAngle,float fSpeed )
{
	fOutx = fOutx + cosf(fAngle +PI)*fSpeed;
	fOuty = fOuty + sinf(fAngle +PI)*fSpeed;
}


inline float   Dist (float x,float y)
{
	return (float)sqrt((double)( x*x + y*y)) ; 
}

inline float   Dist (float x,float y,float z)
{
	return (float)sqrt((double)( x*x + y*y + z*z));
}

inline float    Dist (float    x0,float    y0,float    x1,float    y1 )
{
	return Dist(MAX(x0,x1) - MIN(x0,x1),MAX(y0,y1) - MIN(y0,y1));
}

inline float    Dist (float    x0,float    y0,float    z0,float    x1,float    y1,float    z1 )
{
	return Dist(MAX(x0,x1) - MIN(x0,x1),MAX(y0,y1) - MIN(y0,y1),MAX(z0,z1) - MIN(z0,z1));
}

//--------------------------------------------------------------------
// Name: DistPointToLine
// Desc: 取一个点到一条线段的垂直距离（如果和线段没交点则距离无穷大）
//-------------------------------------------------------------------
inline float DistPointToLineSegment( float lineX0,float lineY0,float lineX1,float lineY1,float pointX,float pointY)
{
	// 坐标差
	float	Lx = lineX1 - lineX0;
	float	Ly = lineY1 - lineY0;

	// 点到直线的交点坐标
	float x = 0.0f;
	float y = 0.0f;

	float D = Lx * Lx + Ly * Ly;

	if ( D < 1e-9f )
	{
		// 距离太近，直接返回给定点到起点的距离
		return sqrt( (lineX0-pointX) * (lineX0-pointX) + (lineY0-pointY) * (lineY0-pointY) );
	}

	float a = ( Lx * ( pointX - lineX0 ) +
		Ly * ( pointY - lineY0 ) );

	if( a < 0.0f )
	{
		x = lineX0;
		y = lineY0;
		//return float(INT_MAX);
	}
	else if( a > D )
	{
		x = lineX1;
		y = lineY1;
		//return float(INT_MAX);
	}
	else
	{
		a /= D;
		x = lineX0 + a * Lx;
		y = lineY0 + a * Ly;
	}
	
	return sqrt( (x-pointX) * (x-pointX) + (y-pointY) * (y-pointY) );
}

///////////////////////////////////////////////////最低级的随机数生成函数
#define IA		16807
#define IM		2147483647
#define AM		(1.0 / IM)
#define IQ		127773
#define IR		2836
#define MASK	123459876

// 原来是 static int g_iRandF = 7654321; 放到头文件里是错误的，会导致每个cpp都有一份
extern int g_iRandF;

inline void SRandF(int iSeed)
{
	g_iRandF = iSeed;
}

inline int RandF()
{
	int k;
	//float ans;
	g_iRandF ^= MASK;
	k = g_iRandF / IQ;
	g_iRandF = IA * (g_iRandF - k * IQ) - IR * k;
	if( g_iRandF < 0 )	g_iRandF += IM;
	//ans = float(AM * g_iRandF);
	g_iRandF ^= MASK;
	return g_iRandF;
}

#endif
