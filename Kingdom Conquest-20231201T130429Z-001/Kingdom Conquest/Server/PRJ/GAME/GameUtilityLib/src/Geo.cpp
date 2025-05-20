/*--------------------------------
  geo.cpp
----------------------------------*/
#include  <math.h>
#include  <memory.h>

#include  "../inc/Stdafx.h"
#include  "../inc/Geo.h"
#include  "../inc/misc.h"

void	GLINE::CalLength()
{
	m_iLength = (int)sqrt((double)((m_gpEnd.x - x + 1 )*(m_gpEnd.x - x + 1) + (m_gpEnd.y - y + 1)*(m_gpEnd.y - y + 1))) ; 
}

void	GLINE::Set(int x0,int y0,int x1,int y1)
{
	GPOINT::Set(x0,y0);
	m_gpEnd.Set(x1,y1);
	CalLength();
}

int		GLINE::GetLength() const
{
	return m_iLength;
}

int AmalgamationGrect(int nNumber,GRECT* pgrIn,GRECT* pgrOut)
{
	int nOut = 0;
	GRECT* pgrPut = pgrOut;
	int i,j;
	BOOL* aIntersect = new BOOL[nNumber];
	BOOL* abRemoved = new BOOL[nNumber];
	memset(aIntersect,0,sizeof(BOOL) * nNumber);
	memset(abRemoved,0,sizeof(BOOL) * nNumber);
	for ( i=nNumber-1; i>0; i-- )
	{
		if ( !abRemoved[i] )
		{
			for ( j=i-1 ; j>=0; j-- )
			{
				if ( !abRemoved[j] )
				{
					if ( pgrIn[i].IsIntersect(pgrIn[j]) )
					{
						if ( !aIntersect[i] )
							aIntersect[i] = TRUE;
						if ( !aIntersect[j] )
							aIntersect[j] = TRUE;

						if ( pgrIn[i].IsContain(pgrIn[j]) )
						{
							abRemoved[j] = TRUE;
						}
						else if ( pgrIn[j].IsContain(pgrIn[i]) )
						{
							abRemoved[i] = TRUE;
						}
					}
				}
			}
			if ( !aIntersect[i] )
			{
				abRemoved[i] = TRUE;
				*(pgrPut++) = pgrIn[i];
				nOut++;
			}
		}
	}

	delete[] abRemoved;
	delete[] aIntersect;

	return nOut;
}


/////////////////////////////////////////////////////////////////////////////////////
// 计算方向

// get the 相邻 map block direction
 DIRECTION WhatDirection(const GPOINT& gpDest,const GPOINT& gpSour)
{
   int x = gpDest.x - gpSour.x;
   int y = gpDest.y - gpSour.y;

   if (x > 0)
   {
	   if (y < 0)
          return RIGHT_UP;
 	   else if (y > 0)
	      return RIGHT_DOWN;
	   else //gp.y == 0
	      return RIGHT;
   }
 	else if (x < 0)
	{
	   if (y < 0)
          return LEFT_UP;
 	   else if (y > 0)
	      return LEFT_DOWN;
	   else //gp.y == 0
	      return LEFT;
	}
	else //gp.x == 0
	{
		if (y < 0)
		   return 	UP;
        else //gp.y <= 0
		   return 	DOWN;
		  
	}
	//unreachable code
	assert(FALSE);
}


 int WhatDir8(const GPOINT& gpDest,const GPOINT& gpSour)
{
	int iDir;
	int x = gpDest.x - gpSour.x;
    int y = gpDest.y - gpSour.y;

	float dAtan = atan2( (float)y, (float)x );
	if( dAtan < 0 )
		dAtan += PI2;

	if(		 ISIN( dAtan ,PI_801 ,PI_803 ) )		iDir = RIGHT_DOWN;
	else if( ISIN( dAtan ,PI_803 ,PI_805 ) )		iDir = DOWN; 		
	else if( ISIN( dAtan ,PI_805 ,PI_807 ) )		iDir = LEFT_DOWN;
	else if( ISIN( dAtan ,PI_807 ,PI_809 ) )		iDir = LEFT;		
	else if( ISIN( dAtan ,PI_809 ,PI_811 ) )		iDir = LEFT_UP;
	else if( ISIN( dAtan ,PI_811 ,PI_813 ) )		iDir = UP;
	else if( ISIN( dAtan ,PI_813 ,PI_815 ) )		iDir = RIGHT_UP;
	else											iDir = RIGHT;

	return iDir;
}

 int WhatDir16(const GPOINT& gpDest,const GPOINT& gpSour)
{
	int iDir;
	int x = gpDest.x - gpSour.x;
    int y = gpDest.y - gpSour.y;

	float dAtan = atan2( (float)y, (float)x );
	if( dAtan < 0 )
		dAtan += PI2;


	if(		 ISIN( dAtan ,PI_1601 ,PI_1603 ) )		iDir = D16_RDD;//
	else if( ISIN( dAtan ,PI_1603 ,PI_1605 ) )		iDir = D16_D;//
	else if( ISIN( dAtan ,PI_1605 ,PI_1607 ) )		iDir = D16_LDD;
	else if( ISIN( dAtan ,PI_1607 ,PI_1609 ) )		iDir = D16_LD;
	else if( ISIN( dAtan ,PI_1609 ,PI_1611 ) )		iDir = D16_LLD;
	else if( ISIN( dAtan ,PI_1611 ,PI_1613 ) )		iDir = D16_L;
	else if( ISIN( dAtan ,PI_1613 ,PI_1615 ) )		iDir = D16_LLU;
	else if( ISIN( dAtan ,PI_1615 ,PI_1617 ) )		iDir = D16_LU;
	else if( ISIN( dAtan ,PI_1617 ,PI_1619 ) )		iDir = D16_LUU;
	else if( ISIN( dAtan ,PI_1619 ,PI_1621 ) )		iDir = D16_U;
	else if( ISIN( dAtan ,PI_1621 ,PI_1623 ) )		iDir = D16_RUU;
	else if( ISIN( dAtan ,PI_1623 ,PI_1625 ) )		iDir = D16_RU;
	else if( ISIN( dAtan ,PI_1625 ,PI_1627 ) )		iDir = D16_RRU;
	else if( ISIN( dAtan ,PI_1627 ,PI_1629 ) )		iDir = D16_R;
	else if( ISIN( dAtan ,PI_1629 ,PI_1631 ) )		iDir = D16_RRD;
	else											iDir = D16_RD;

	
	return iDir;
}


 DIRECTION CalDirection(const GPOINT& gpDest,const GPOINT& gpSour)
{
	int x = gpDest.x - gpSour.x;
	int y = gpDest.y - gpSour.y;

   
	if ( abs(x) >= 2 * abs(y) )
	{
		if ( x > 0)
			return	RIGHT;
		else 
			return	LEFT;
	}
	else if ( abs(y) < 2 * abs(x) && abs(x) < 2 * abs(y) )
	{
		if ( x > 0 && y < 0 )
			return RIGHT_UP;
		else if ( x > 0 && y >= 0 )
			return	RIGHT_DOWN;
		else if ( x < 0 && y < 0 )
			return	LEFT_UP;
		else
			return LEFT_DOWN;
	}
	else// if ( abs(y) >= 2 *abs(x) )
	{
		if ( y < 0)
			return UP;
		else
			return DOWN;
		
	}
 	
	//unreachable code
	assert(FALSE);
}

 BOOL   IsExceedDest(  const GPOINT & gpDest,const GPOINT & gpCurPos,int iDir )
{
	switch( iDir )
	{
		case RIGHT:
			if( gpDest.x <= gpCurPos.x )
				return true;
			break;
		case LEFT:
			if( gpDest.x >= gpCurPos.x )
				return true;
			break;
		case UP:
			if( gpDest.y >= gpCurPos.y )
				return true;
			break;
		case DOWN:
			if( gpDest.y <= gpCurPos.y )
				return true;
			break;
		case LEFT_UP:
			if( gpDest.x >= gpCurPos.x )
				return true;
			if( gpDest.y >= gpCurPos.y )
				return true;
			break;
		case LEFT_DOWN:
			if( gpDest.y <= gpCurPos.y )
				return true;
			if( gpDest.x >= gpCurPos.x )
				return true;
			break;
		case RIGHT_UP:
			if( gpDest.x <= gpCurPos.x )
				return true;
			if( gpDest.y >= gpCurPos.y )
				return true;
			break;
		case RIGHT_DOWN:
			if( gpDest.x <= gpCurPos.x )
				return true;
			if( gpDest.y <= gpCurPos.y )
				return true;
			break;
		default:
			assert(0);
			break;
	}		
	return false;
}

//ly write
 void   MoveDirForSpeed(GPOINT& gp,const int &dir,const int &iSpeed)
{
	switch( dir )
	{
		case RIGHT:
			gp.x+=iSpeed;
			break;
		case LEFT:
			gp.x-=iSpeed;
			break;
		case UP:
			gp.y-=iSpeed;
			break;
		case DOWN:
			gp.y+=iSpeed;
			break;
		case LEFT_UP:
			gp.y-=iSpeed;
			gp.x-=iSpeed;
			break;
		case LEFT_DOWN:
			gp.y+=iSpeed;
			gp.x-=iSpeed;
			break;
		case RIGHT_UP:
			gp.y-=iSpeed;
			gp.x+=iSpeed;
			break;
		case RIGHT_DOWN:
			gp.y+=iSpeed;
			gp.x+=iSpeed;
			break;
		default:
			assert(0);
			break;
	}		
}




 BOOL	  IsRange( const GPOINT & gpBegin,const GPOINT & gpEnd,int iMax,int iMin )
{
	int cx = abs(gpBegin.x-gpEnd.x);
	int cy = abs(gpBegin.y-gpEnd.y);
	if( ( cx <= iMax && cy <= iMax ) &&
		( cx >= iMin || cy >= iMin ) )
	{
		return true;
	}
	return false;
}

 BOOL	  IsMaxRange( const GPOINT & gpBegin,const GPOINT & gpEnd,int iMax)
{
	int cx = abs(gpBegin.x-gpEnd.x);
	int cy = abs(gpBegin.y-gpEnd.y);
	if( cx <= iMax && cy <= iMax )
	{
		return true;
	}
	return false;
}

 BOOL	  IsMinRange( const GPOINT & gpBegin,const GPOINT & gpEnd,int iMin )
{
	int cx = abs(gpBegin.x-gpEnd.x);
	int cy = abs(gpBegin.y-gpEnd.y);
	if(	cx >= iMin || cy >= iMin )	
	{
		return true;
	}
	return false;
}

GPOINT	  GetFleePos( const GPOINT & gpEnemy,const GPOINT & gpSelf,int iDis,int iDirStep )
{
	GPOINT Pos;
	int iDir = WhatDir8(gpSelf,gpEnemy);
	Pos = gpSelf;
	int iDist = Dist(gpSelf,gpEnemy);
	iDis -= iDist;
	if( iDis < 1 )
		iDis = 1;
	TurnDirection( iDir,iDirStep );
	MoveDirForSpeed( Pos,iDir,iDis );
	return Pos;
}


GPOINT	  GetZhuiPos( const GPOINT & gpEnemy,const GPOINT & gpSelf,int iDis,int iDirStep )
{
	GPOINT Pos;
	int iDir = WhatDir8(gpEnemy,gpSelf);
	Pos = gpSelf;
	int iDist = Dist(gpSelf,gpEnemy);
	iDis -= iDist;
	if( iDis < 1 )
		iDis = 1;
	TurnDirection( iDir,iDirStep );
	MoveDirForSpeed( Pos,iDir,iDis );
	return Pos;
}


 void	ClockEllipsePathMove( GPOINT & gpWorldPos,GPOINT gpCenterPos,int iRadius,int iSpeed )
{

	int iDir = WhatDir8( gpWorldPos,gpCenterPos );
	int iDist = Dist( gpWorldPos,gpCenterPos );

	int iMoveiDir = -1;
	if( iDist < iRadius )
	{
		iMoveiDir = iDir;
	}
	else if( iDist > iRadius )
	{
		iMoveiDir = iDir-4;
		if( iMoveiDir < 0 )
			iMoveiDir += 8;
	}

	if( iMoveiDir != -1 )
		MoveDirForSpeed( gpWorldPos,iMoveiDir,iSpeed/2 );

	iMoveiDir = iDir-2;
	if( iMoveiDir < 0 )
		iMoveiDir += 8;

	MoveDirForSpeed( gpWorldPos,iMoveiDir,iSpeed );
	
	/*
	int iDist = Dist( gpWorldPos,gpCenterPos );

	if( iDist < iRadius )
	{
		iRadius = iDist + iSpeed;
	}
	else if( iDist > iRadius )
	{
		iRadius = iDist - iSpeed;
	}
	
	
	float cx  = (float)(gpWorldPos.x - gpCenterPos.x);
	float cy  = (float)(gpWorldPos.y - gpCenterPos.y);

	double dAtan = atan2( cy, cx );
	if( dAtan < 0 )
		dAtan += PI2;
	dAtan += PI_1601;
	if( dAtan < 0 )
		dAtan += PI2;

	double newx = sin( dAtan )*iRadius;
	double newy = cos( dAtan )*iRadius;
	
	gpWorldPos.x += (int)newx;
	gpWorldPos.y += (int)newy;
	*/
}

const float PI2Div16 = PI2/16;
BYTE	AngleToDir16( float fAngle)
{
	
	int iDir = FloatToInt( (fAngle/PI2Div16) );
	if( iDir < 0 )
		iDir += 16; 
	return (BYTE)iDir%16;
}


float	Dir16ToAngle( BYTE bDir )
{
	float fAngle;
	fAngle = PI2Div16*bDir;
	return fAngle;
}


void	SetDestInRange( GPOINT & gpDest,const GPOINT & gpStart,int iMin,int iMax )
{
	int iDist = Dist( gpStart,gpDest );
	if( iDist < iMin )
	{
		iDist = iMin;
	}
	else if( iDist > iMax )
	{
		iDist = iMax;
	}
	float fAngle = ::GetAngle( gpStart,gpDest );
	MoveForAngle( gpDest,gpStart,fAngle,(float)iDist );
}

void	SetDestInRange( GPOINT & gpDest,const GPOINT & gpStart,int iMin,int iMax,float fAngle )
{
	int iDist = Dist( gpStart,gpDest );
	if( iDist < iMin )
	{
		iDist = iMin;
	}
	else if( iDist > iMax )
	{
		iDist = iMax;
	}
	MoveForAngle( gpDest,gpStart,fAngle,(float)iDist );
}

void	PointMakeRect( GRECT & grOut,GPOINT gpBegin,GPOINT gpEnd )
{
	if( gpBegin.x < gpEnd.x )
	{
		grOut.x = gpBegin.x; 
	}
	else
	{
		grOut.x = gpEnd.x; 
	}

	if( gpBegin.y < gpEnd.y )
	{
		grOut.y = gpBegin.y; 
	}
	else
	{
		grOut.y = gpEnd.y; 
	}

	grOut.cx = abs( gpBegin.x-gpEnd.x ); 
	grOut.cy = abs( gpBegin.y-gpEnd.y ); 
}

