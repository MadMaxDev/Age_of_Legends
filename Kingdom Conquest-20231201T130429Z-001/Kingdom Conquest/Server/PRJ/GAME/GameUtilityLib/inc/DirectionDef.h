/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: DirectionDef.h

	Purpose:Definitions about Direction

	Author: Soloman

	Compiler: Microsoft Visual C++ 6.0

	History: 4/3/2003.

	Remarks:

*********************************************************/

#ifndef  __DIRECTIONDEF_H__
#define  __DIRECTIONDEF_H__

#include <math.h>
#include <assert.h>
#include "DMtype.h"


enum DIRECTION {
	NONE_DIR = -1,
	RIGHT=7,
	RIGHT_UP=0,
	UP=1,
	LEFT_UP=2,
	LEFT=3,
	LEFT_DOWN=4,
	DOWN=5,
	RIGHT_DOWN=6,
	TOTAL_DIRECTION_NUMBER=8
};

enum DIRECTION_16 {
	D16_R=0,
	D16_RRU,
	D16_RU,
	D16_RUU,
	D16_U,
	D16_LUU,
	D16_LU,
	D16_LLU,
	D16_L,
	D16_LLD,
	D16_LD,
	D16_LDD,
	D16_D,
	D16_RDD,
	D16_RD,
	D16_RRD,
	TOTAL_DIRECTION_16_NUMBER
};

enum DIR3D_16 {
	DIR3D_LU=0,
	DIR3D_LUU,
	DIR3D_U,
	DIR3D_RUU,
	DIR3D_RU,
	DIR3D_RRU,
	DIR3D_R,
	DIR3D_RRD,
	DIR3D_RD,
	DIR3D_RDD,
	DIR3D_D,
	DIR3D_LDD,
	DIR3D_LD,
	DIR3D_LLD,
	DIR3D_L,
	DIR3D_LLU,
	TOTAL_DIR3D_NUMBER
};


// corner order of a rectangle
enum ENUM_CORNER
{
	OU_DOWNLEFT	= 0,
	OU_DOWNRIGHT,
	OU_UPRIGHT,
	OU_UPLEFT,

	OU_NUMBER,
};


/////////////////////////////////
//	calculate distance between DestDirection & SourDirection
inline int	ClockDistance(int iDestDir, int iSourDir){
	return (8 - (iDestDir - iSourDir))%8;
}
inline int	ClockDistance_16(int iDestDir, int iSourDir){
	return (16 - (iDestDir - iSourDir))%16;
}
inline int	UnClockDistance(int iDestDir, int iSourDir){
	return (8 + (iDestDir - iSourDir))%8;
}
inline int	UnClockDistance_16(int iDestDir, int iSourDir){
	return (16 + (iDestDir - iSourDir))%16;
}


//求方向差
inline int	GetDir8Step( int dir1,int dir2 ){
	int iStep = abs(dir1 - dir2);

	if( iStep > 4 )
	{
		iStep = 8 - iStep;
	}
	return iStep;
}


//求方向差
inline int	GetDir16Step( int dir1,int dir2,BOOL &booLeft ){
	int iStep = abs(dir1 - dir2);

	
	if( iStep > 8 )
	{
		booLeft = true;
		iStep = 16 - iStep;
	}
	else
	{
		booLeft = false;
	}
	return iStep;
}


inline void	Direction16Bound( int &dir )
{
	if ( dir <  0)
		dir += TOTAL_DIRECTION_16_NUMBER;
	else if ( dir >=  TOTAL_DIRECTION_16_NUMBER)
		dir -= TOTAL_DIRECTION_16_NUMBER;
}

//相对方向
inline void	OppositeDir16( int &dir ){
	dir -= 8;
	Direction16Bound( dir );
}


inline void	TurnDirection16( int &dir,int iStep ){
	dir += iStep;
	Direction16Bound( dir );
}

//以目标方向旋转
inline void	TurnDestDirection16( const int &DestDir,int &CurDir ){
	if( CurDir != DestDir )
	{
		if( abs( CurDir - DestDir ) < 8 )
		{
			if( CurDir > DestDir )
			{
				CurDir--;
			}
			else 
			{
				CurDir++;
			}
		}
		else
		{
			if( CurDir < DestDir )
			{
				CurDir--;
			}
			else 
			{
				CurDir++;
			}
		}
		Direction16Bound( CurDir );
	}
}

//顺时针旋转
inline void	ClockTurnDirection( int &dir ){
	dir++;
	if ( dir >=  TOTAL_DIRECTION_NUMBER)
		dir = 0;
}

//逆时针旋转
inline void	UnClockTurnDirection( int &dir ){
	dir--;
	if ( dir <  0)
		dir = TOTAL_DIRECTION_NUMBER-1;
}

inline void	TurnDirection( int &dir,int iStep ){
	dir += iStep;
	if ( dir <  0)
		dir += TOTAL_DIRECTION_NUMBER;
	else if ( dir >=  TOTAL_DIRECTION_NUMBER)
		dir -= TOTAL_DIRECTION_NUMBER;
}

//以目标方向旋转
inline void	TurnDestDirection( const int &DestDir,int &CurDir ){
	if( CurDir != DestDir )
	{
		if( abs( CurDir - DestDir ) < 4 )
		{
			if( CurDir > DestDir )
					UnClockTurnDirection( CurDir );
			else	ClockTurnDirection( CurDir );
		}
		else
		{
			if( CurDir < DestDir )
					UnClockTurnDirection( CurDir );
			else	ClockTurnDirection( CurDir );
		}
	}
}

//相对方向
inline void	OppositeDir( int &dir ){
	if( dir == RIGHT)				dir =  LEFT;
	else if( dir == RIGHT_UP)		dir =  LEFT_DOWN ;
	else if( dir == UP)				dir =  DOWN ;
	else if( dir == LEFT_UP)		dir =  RIGHT_DOWN ;
	else if( dir == LEFT)			dir =  RIGHT ;
	else if( dir == LEFT_DOWN)		dir =  RIGHT_UP ;
	else if( dir == DOWN)			dir =  UP ;
	else if( dir == RIGHT_DOWN)		dir =  LEFT_UP ;
	else assert(0);			
}

///////////////////////////////////

#define  RRU RIGHT_RIGHT_UP
#define  RUU RIGHT_UP_UP
#define  LUU LEFT_UP_UP
#define  LLU LEFT_LEFT_UP
#define  LLD LEFT_LEFT_DOWN
#define  LDD LEFT_DOWN_DOWN
#define  RDD RIGHT_DOWN_DOWN
#define  RRD RIGHT_RIGHT_DOWN 	



#endif//#if !defined(__DIRECTIONDEF_H__)
