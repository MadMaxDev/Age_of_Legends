/********************************************************
	Copyright 1995-2004, Pixel Software Corporation, 
	All rights reserved.
	
	Module name: geo

	Purpose: 一些矩形，点的数学方法

	Author: PersyZhang

	Compiler: Microsoft Visual C++ 6.0

	History: 6/7/95.

	Remarks:

*********************************************************/

#ifndef   __GEO_H
#define   __GEO_H

#include "Mat.h"
#include "DirectionDef.h"
#include <assert.h>

#ifdef WIN32
	#include <wTypes.h>
#else
	struct RECT
	{
		long left;
		long top;
		long right;
		long bottom;
	};
	typedef float FLOAT;
#endif

//Point
class   GPOINT {
			public :
				 int x;
				 int y;
			public :
				 GPOINT(){x=0;y=0;}
				 GPOINT(int x0,int y0)
						 {x =x0;y = y0;}
				 ~GPOINT(){;}
				inline  GPOINT& operator = (const GPOINT& gp);
				inline GPOINT& operator += ( const GPOINT& gp);
				inline GPOINT& operator -= ( const GPOINT& gp);
				inline GPOINT& operator *= ( const GPOINT& gp);
				inline GPOINT& operator /= ( const GPOINT& gp);
				inline GPOINT operator + ( const GPOINT& gp);
				inline GPOINT operator - ( const GPOINT& gp);
				inline BOOL operator == ( const GPOINT& gp) const;
				inline BOOL operator != ( const GPOINT& gp) const;
				inline GPOINT& Set(int x0,int y0);
				int	    X(){return  x;}
				int	    Y(){return  y;}
				int&    RefX(){return x;}
				int&    RefY(){return y;}

  //Gao**
				int	    GetX()			{   return  x;  }
				int	    GetY()			{   return  y;  }

				void    SetX(int x0)	{   x = x0;  }
				void    SetY(int y0)	{   y = y0;  }
  //Is Close to a Gpoint
				inline  BOOL    IsCloseTo(const GPOINT& gp) const;

  };

  inline GPOINT& GPOINT::Set(int x0,int y0)
  {
		 x = x0;
		 y = y0;
		 return (*this);
  }
  inline BOOL GPOINT::IsCloseTo(const GPOINT& gp) const 
  {
	  if ((abs(x - gp.x) < 2) && (abs(y - gp.y) < 2))
		  return TRUE;
	  else
		  return FALSE;
  }

  inline BOOL GPOINT::operator != ( const GPOINT& gp) const
  {  //operator  " != "
	 if ((this->x == gp.x)&&(this->y == gp.y))
		 return FALSE;
	 else
		return TRUE;
  }

  inline BOOL GPOINT::operator == ( const GPOINT& gp) const
  {   //operator  " == "
	 if ((this->x == gp.x)&&(this->y == gp.y))
		return TRUE;
	 else
		return FALSE;
  }

  inline GPOINT GPOINT::operator - ( const GPOINT& gp)
  {   //operator  " - "
	 GPOINT gpTmp;
	 gpTmp.x = this->x - gp.x;
	 gpTmp.y = this->y - gp.y;
  	 return gpTmp;
  } 

  inline GPOINT GPOINT::operator + ( const GPOINT& gp)
  {    //operator  " + "
 	 GPOINT gpTmp;
	 gpTmp.x = this->x + gp.x;
	 gpTmp.y = this->y + gp.y;
	 return gpTmp;
  }

  inline GPOINT& GPOINT::operator -= ( const GPOINT& gp)
  {   //operator  " -= "
	 this->x -= gp.x;
	 this->y -= gp.y;
	 return (*this);
 }

  inline GPOINT& GPOINT::operator *= ( const GPOINT& gp)
  {   
	 this->x *= gp.x;
	 this->y *= gp.y;
	 return (*this);
 }

  inline GPOINT& GPOINT::operator /= ( const GPOINT& gp)
  {   
	 this->x /= gp.x;
	 this->y /= gp.y;
	 return (*this);
 }

  
  inline GPOINT& GPOINT::operator += ( const GPOINT& gp)
  {   //operator  " += "
  	 this->x += gp.x;
	 this->y += gp.y;
	 return (*this);
  }

  inline GPOINT& GPOINT::operator = (const GPOINT& gp)
  {   //operator  " = "
	  this->x = gp.x;
	  this->y = gp.y;
	  return (*this);
  }

		


//	modify by Gao.
class    GPOINT3D : public GPOINT
{
public:
	    GPOINT3D(){;}
        GPOINT3D(int x,int y,int z) : GPOINT(x,y) { GPOINT3D::z = z;    }
        int	z;

        GPOINT3D operator / (int factor)
        {
            GPOINT3D pos;
            pos.x = x / factor;
            pos.y = y / factor;
            pos.z = z / factor;
            return pos;
        }

        GPOINT3D operator % (int factor)
        {
            GPOINT3D pos;
            pos.x = x % factor;
            pos.y = y % factor;
            pos.z = z % factor;
            return	pos;
        }

        GPOINT3D	operator + (const GPOINT3D& coor)
        {
            GPOINT3D pos;
            pos.x = x + coor.x;
            pos.y = y + coor.y;
            pos.z = z + coor.z;
            return pos;
        }

        BOOL	operator == (const GPOINT3D& pos)
        {
            return	( x == pos.x && y == pos.y && z == pos.z );
        }
};

//	modify by Gao.
class    GLINE : public GPOINT
{
	private:
		void	CalLength();
   public:
		GPOINT  m_gpEnd;       
		int		m_iLength;

		GLINE():GPOINT (0,0){
			m_iLength = 0;
		}
		GLINE(const GPOINT& gp) : GPOINT (gp.x,gp.y)
		{
			CalLength();
		}
		GLINE(int x,int y) : GPOINT (x,y)
		{
			CalLength();
		}
		GLINE(const GPOINT& gpStart, const GPOINT& gpEnd) : GPOINT(gpStart.x, gpStart.y)
		{
			m_gpEnd = gpEnd;
			CalLength();
		}
        GLINE(int x0,int y0,int x1,int y1) : GPOINT(x0, y0)
        {
            m_gpEnd.x = x1;
            m_gpEnd.y = y1;
			CalLength();
        }
		void	Set(const GPOINT& gp0,const GPOINT& gp1)
		{
			Set(gp0.x,gp0.y,gp1.x,gp1.y);
		}
		void	Set(int x0,int y0,int x1,int y1);
		inline	GPOINT	GetMiddle();	
		int		GetLength() const;

};

inline	GPOINT	GLINE::GetMiddle(){
	return GPOINT((x + m_gpEnd.x)/2, (y + m_gpEnd.y)/2);
}

 //Rectangle
  class  GRECT : public GPOINT 
  {
			public :
						int  cx;
						int  cy;
						int  size;
			public :
				GRECT():GPOINT(){cx=0;
									  cy=0;
									  size=0;}

				GRECT(const GPOINT& gp):GPOINT(gp.x,gp.y){cx=0;cy=0;size=0;}
				GRECT(const GRECT& gr):GPOINT(gr.x,gr.y){cx=gr.cx;cy=gr.cy;size=cx*cy;}
				
				
				GRECT(int  cx0,int cy0):GPOINT(0,0){
									  cx=cx0;
									  cy=cy0;
									  size=cx*cy;}
				GRECT(int x0,int y0,int cx0,int cy0):GPOINT(x0,y0){
									  cx=cx0;
									  cy=cy0;
									  size=cx*cy;}
				
				~GRECT(){;}
				inline GRECT&	operator = ( const GRECT& gr);
				inline GRECT&	operator = ( const GPOINT& gp);
				
				inline BOOL operator == ( const GRECT& gp) const;
				inline BOOL operator != ( const GRECT& gp) const;
				
				inline GRECT& operator += ( const GRECT& gr);
				inline GRECT& operator -= ( const GRECT& gr);
				GRECT& operator += ( const GPOINT& gp) {
					(*((GPOINT*)this))+=(gp);
					return *this;
				}
				GRECT& operator -= ( const GPOINT& gp){
					(*((GPOINT*)this))-=(gp);
					return *this;
				}

				inline BOOL operator >= ( const GRECT& gp) const;
				inline BOOL operator <= ( const GRECT& gp) const;

				inline GRECT&	Assign(const GRECT& gr);
				inline GRECT& Set(int x0,int y0,int width,int height);
				inline BOOL	 IsContain( const GPOINT& gp) const;
				inline BOOL	 IsContain(int x0,int y0) const;
				 
//				inline  BOOL Clip(  const GRECT * pgr);
				int    GetCX() const {return cx;}
				int    GetCY() const {return cy;}
				void    SetCX(int w){SetW(w);}
				void    SetCY(int h){SetH(h);}

				int	    GetW()	const	{   return  cx;	}
				int	    GetH()	const	{   return  cy; }

				void    SetW(int w)	{   cx = w;	size = cx * cy;}
				void    SetH(int h)	{   cy = h;	size = cx * cy;}

                inline  BOOL    Intersect(const GRECT& rect1,const GRECT& rect2);
				inline  BOOL    Intersect(const GRECT& rDest);

                inline  BOOL    IsContain(const GRECT& tested) const;
				inline  BOOL    IsIntersect   (const GRECT& rDest) const;
				inline  BOOL	IsTrueIntersect   (const GRECT& rDest) const;

				// modified by Jerry, 2001.2.9, 给 GRECT 扩展了 RECT 的相互转化函数
				GRECT(const RECT& rc){
					x = rc.left;	y = rc.top;
					cx = rc.right - rc.left;	cy = rc.bottom - rc.top;	size = cx * cy;
				}
				GRECT&	operator = ( const RECT& rc){
					x = rc.left;	y = rc.top;
					cx = rc.right - rc.left;	cy = rc.bottom - rc.top;	size = cx * cy;
					return  (*this);
				}
				BOOL operator == ( const RECT& rc) const{   //operator  " == "
					 return ( (x == rc.left) && (y == rc.top) && ( x + cx == rc.right) && (y + cy == rc.bottom) );
				}
				BOOL operator != ( const RECT& rc) const{
					return ( (x != rc.left) || (y != rc.top) || ( x + cx != rc.right) || (y + cy != rc.bottom) );
				}
				inline GRECT& operator += ( const RECT& rc);
				inline GRECT& operator -= ( const RECT& rc);
				operator RECT(){
					RECT rc = {x,y,x+cx,y+cy};
					return rc;
				}
				inline  BOOL IsContain(const RECT& tested) const;
				inline  BOOL IsContainBy(const RECT& tested) const;

  };


 inline  GRECT&	GRECT::operator = ( const GRECT& gr)
 {
		 this->x = gr.x;
		 this->y = gr.y;
		 this->cx = gr.cx;
		 this->cy = gr.cy;
		 this->size = gr.size;
		 return  (*this);
}

 inline  GRECT&	GRECT::operator = ( const GPOINT& gp)
 {
		 this->x = gp.x;
		 this->y = gp.y;
		 return  (*this);
}


inline GRECT& GRECT::Assign(const GRECT& gr)
{
		 this->x = gr.x;
		 this->y = gr.y;
		 this->cx = gr.cx;
		 this->cy = gr.cy;
		 this->size = gr.size;
		 return  (*this);
}
				
inline void Adjust(const GPOINT * pgpDest,const GPOINT * pgpSour,GPOINT& gp){
	gp.x += pgpSour->x - pgpDest->x;
	gp.y += pgpSour->y - pgpDest->y;
}


inline  BOOL  GRECT:: Intersect   (const GRECT& rect1,const GRECT& rect2)
{
		int	x0  =   MAX (rect1.x,rect2.x),
		    y0  =   MAX (rect1.y,rect2.y),
			x1  =   MIN (rect1.x + rect1 .cx -1, rect2 .x + rect2 .cx - 1 ) ,
			y1  =   MIN (rect1.y + rect1 .cy -1, rect2 .y + rect2 .cy - 1 ) ;

		if	(x0 > x1 || y0 > y1) 	
			return  FALSE;

		x = x0    ;
		y = y0    ;
		cx = x1 - x0 + 1 ;
		cy = y1 - y0 + 1 ;

			return  TRUE ;
}


inline  BOOL  GRECT:: IsTrueIntersect   (const GRECT& rDest) const
{
		int	xLeftUp  =   MAX (x,rDest.x),
		    yLeftUp  =   MAX (y,rDest.y),
			xRightDown  =   MIN (x + cx -1, rDest.x + rDest .cx - 1 ) ,
			yRightDown  =   MIN (y + cy -1, rDest.y + rDest .cy - 1 ) ;

		if	(xLeftUp >= xRightDown || yLeftUp >= yRightDown) 	
			return  FALSE;

		else
			return  TRUE ;
}

inline  BOOL  GRECT:: IsIntersect   (const GRECT& rDest) const
{
		int	xLeftUp  =   MAX (x,rDest.x),
		    yLeftUp  =   MAX (y,rDest.y),
			xRightDown  =   MIN (x + cx -1, rDest.x + rDest .cx - 1 ) ,
			yRightDown  =   MIN (y + cy -1, rDest.y + rDest .cy - 1 ) ;

		if	(xLeftUp > xRightDown || yLeftUp > yRightDown) 	
			return  FALSE;

		else
			return  TRUE ;
}


inline  BOOL  GRECT:: Intersect   (const GRECT& rDest)
{
		int	xLeftUp  =   MAX (x,rDest.x),
		    yLeftUp  =   MAX (y,rDest.y),
			xRightDown  =   MIN (x + cx -1, rDest.x + rDest .cx - 1 ) ,
			yRightDown  =   MIN (y + cy -1, rDest.y + rDest .cy - 1 ) ;

		if	(xLeftUp > xRightDown || yLeftUp > yRightDown) 	
			return  FALSE;

		x = xLeftUp    ;
		y = yLeftUp    ;
		cx = LENGTH(xLeftUp,xRightDown) ;
		cy = LENGTH(yLeftUp,yRightDown) ;

			return  TRUE ;
}

inline GRECT& GRECT::Set(int x0,int y0,int width,int height)
{
        x = x0;
	    y = y0;
	    cx = width;
	    cy = height;
		size = cx * cy;
	    return (*this);
}


inline  BOOL GRECT::  IsContain(const GRECT& tested) const
{
      	return	(
   	  			(tested.x >= x)   &&
   	  			(tested.y >= y)   &&
   	  			(tested.x  + tested.cx -1 <= x + cx -1) &&
   	  			(tested.y  + tested.cy -1 <= y + cy -1)
      			);
}


inline BOOL	 GRECT::IsContain( const GPOINT& gp) const
 {
   	 if ( !ISIN(gp.x,x,x + cx - 1) || !ISIN(gp.y,y,y + cy - 1))
   				  return FALSE;
     return TRUE;
}
				
inline BOOL	 GRECT::IsContain(int x0,int y0) const
{
	 if ( !ISIN(x0,x,x + cx - 1 ) || !ISIN(y0,y,y + cy -1))
				  return FALSE;
	 return TRUE;
}

inline BOOL GRECT::operator != ( const GRECT& gr) const
{  //operator  " != "
	 if ((this->x != gr.x) || (this->y != gr.y) || (this->cx != gr.cx) || (this->cy != gr.cy))
		 return true;
	 else
		return false;
}

inline BOOL GRECT::operator == ( const GRECT& gr) const
{   //operator  " == "
	 if ((this->x != gr.x) || (this->y != gr.y) || (this->cx != gr.cx) || (this->cy != gr.cy))
		return false;
	 else
		return true;
}

inline GRECT& GRECT::operator += ( const GRECT& gr)
{
	GPOINT	gpRightDown = *(GPOINT*)this;
	gpRightDown.x += cx;
	gpRightDown.y += cy;

	if ( gpRightDown.x < gr.x + gr.cx)
		gpRightDown.x = gr.x + gr.cx;
	if ( gpRightDown.y < gr.y + gr.cy)
		gpRightDown.y = gr.y + gr.cy;

	if ( x > gr.x )
		x = gr.x;
	if ( y > gr.y )
		y = gr.y;

	cx = gpRightDown.x - x;
	cy = gpRightDown.y - y;
	return (*this);
}

inline GRECT& GRECT::operator -= ( const GRECT& gr)
{
	if ( IsContain( gr ))
	{
		cx = x - gr.x + 1;
		cy = y - gr.y + 1;
	}
	return (*this);
}



/*
inline BOOL GRECT::Clip(  const GRECT * pgr)
 {
	 GPOINT   gpRightDown;
	 gpRightDown.x = x + cx;
	 gpRightDown.y = y + cy;
	 
	 if ((!pgr->IsContain(*(GPOINT*)this)) &&  (!pgr->IsContain(gpRightDown)))
		return FALSE;
	  
	 x = MAX(x,pgr->x);
	 y = MAX(y,pgr->y);
	 gpRightDown.x = MIN(gpRightDown.x,pgr->x + pgr->cx);
	 gpRightDown.y = MIN(gpRightDown.y,pgr->y + pgr->cy);
	 cx = gpRightDown.x - x - 1;
	 cy = gpRightDown.y - y - 1;
	
	 return TRUE;
}*/



//---------- modified by Jerry, 2001.2.9, 给 GRECT 扩展了 RECT 的相互转化函数 ----------//
// 合并
inline GRECT& GRECT::operator += ( const RECT& rc)
{
	cx = MAX(x+cx, rc.right);
	cy = MAX(y+cy, rc.bottom);
	x = MIN(x, rc.left);
	y = MIN(y, rc.top);
	size = cx * cy;
	return (*this);
}

inline GRECT& GRECT::operator -= ( const RECT& rc)
{
	if ( IsContain( GRECT(rc) ))
	{
		cx = x - rc.left + 1;
		cy = y - rc.top + 1;
	}
	return (*this);
}

inline  BOOL GRECT::IsContain(const RECT& tested) const
{
      	return	(
   	  			(tested.left >= x)   &&
   	  			(tested.top >= y)   &&
   	  			(tested.right <= x + cx -1) &&
   	  			(tested.bottom <= y + cy -1)
      			);
}

inline  BOOL GRECT::IsContainBy(const RECT& tested) const
{
      	return	(
   	  			(x >= tested.left )   &&
   	  			(y >= tested.top )   &&
   	  			(x + cx -1 <= tested.right) &&
   	  			(y + cy -1 <= tested.bottom)
      			);
}


inline BOOL GRECT::operator >= ( const GRECT& gr) const
{   //operator  " >= "
	 if ((this->x >= gr.x) && (this->y >= gr.y) && (this->cx >= gr.cx) && (this->cy >= gr.cy))
		return true;
	 else
		return false;
}

inline BOOL GRECT::operator <= ( const GRECT& gr) const
{   //operator  " <= "
	 if ((this->x <= gr.x) && (this->y <= gr.y) && (this->cx <= gr.cx) && (this->cy <= gr.cy))
		return true;
	 else
		return false;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// notes: u,v 系统


struct  FPOINT
{
	FLOAT u,v;

	FPOINT(){
		u = v = 0.0f;
	}
	FPOINT(FLOAT fu, FLOAT fv){
		u = fu; v = fv;
	}
	void Set(FLOAT fu, FLOAT fv){
		u = fu; v = fv;
	}

	inline FPOINT operator + ( const FPOINT& pos);
	inline FPOINT operator - ( const FPOINT& pos);
	inline FPOINT operator * ( const FPOINT& pos);
	inline FPOINT operator / ( const FPOINT& pos);

	inline FPOINT operator + ( const FLOAT fR);
	inline FPOINT operator - ( const FLOAT fR);
	inline FPOINT operator * ( const FLOAT fR);
	inline FPOINT operator / ( const FLOAT fR);
};
typedef FPOINT* LPFPOINT;

inline FPOINT FPOINT::operator + ( const FPOINT& pos)
{
	FPOINT posRes(u+pos.u,v+pos.v);
	return posRes;
}
inline FPOINT FPOINT::operator - ( const FPOINT& pos)
{
	FPOINT posRes(u-pos.u,v-pos.v);
	return posRes;
}
inline FPOINT FPOINT::operator * ( const FPOINT& pos)
{
	FPOINT posRes(u*pos.u,v*pos.v);
	return posRes;
}
inline FPOINT FPOINT::operator / ( const FPOINT& pos)
{
	FPOINT posRes(u/pos.u,v/pos.v);
	return posRes;
}

inline FPOINT FPOINT::operator + ( const FLOAT fR)
{
	FPOINT posRes(u+fR,v+fR);
	return posRes;
}
inline FPOINT FPOINT::operator - ( const FLOAT fR)
{
	FPOINT posRes(u-fR,v-fR);
	return posRes;
}
inline FPOINT FPOINT::operator * ( const FLOAT fR)
{
	FPOINT posRes(u*fR,v*fR);
	return posRes;
}
inline FPOINT FPOINT::operator / ( const FLOAT fR)
{
	FPOINT posRes(u/fR,v/fR);
	return posRes;
}

// modified by Jerry Mao
class  FSIZE
{
public:
	FLOAT	fWidth;
	FLOAT	fHeight;

	FSIZE(){
		fWidth = fHeight = 1.0f;
	}
	FSIZE(FLOAT fW,FLOAT fH){
		fWidth = fW;
		fHeight = fH;
	}

	BOOL IsValid(){
		return fWidth >0.0f && fHeight > 0.0f;
	}
};
typedef FSIZE* PFSIZE;
typedef FSIZE* LPFSIZE;

class  FRECT
{
public:
	FLOAT	fLeft;
	FLOAT	fTop;
	FLOAT	fRight;
	FLOAT	fBottom;


	FRECT(){
		fLeft = fBottom = 0.0f;
		fRight = fTop = 1.0f;
	}
	FRECT(FLOAT fL,FLOAT fT,FLOAT fR,FLOAT fB){
		fLeft = fL; fTop = fT; fRight = fR; fBottom = fB;
	}
	FLOAT GetWidth(){
		return fRight - fLeft;
	}
	FLOAT GetHeight(){
		return fBottom - fTop;
	}

	BOOL IsValid(){
		return GetWidth() > 0.0f && GetHeight() > 0.0f;
	}

	inline  BOOL IsContain(const FLOAT fU,const FLOAT fV) const;
	inline  BOOL IsTrueContain(const FLOAT fU,const FLOAT fV) const;
	inline  BOOL IsContain(const FRECT& frTest) const;
	inline  BOOL IsTrueContain(const FRECT& frTest) const;
	inline  BOOL IsIntersect(const FRECT& rDest) const;
	inline  BOOL IsTrueIntersect(const FRECT& rDest) const;
};
typedef FRECT* LPFRECT;
typedef FRECT* PFRECT;

inline  BOOL FRECT::IsContain(const FLOAT fU,const FLOAT fV) const
{
    return	(
   	  		(fU >= fLeft)   &&
   	  		(fV >= fTop)  &&
   	  		(fU <= fRight) &&
   	  		(fV <= fBottom)
 			);
}
inline  BOOL FRECT::IsTrueContain(const FLOAT fU,const FLOAT fV) const
{
    return	(
   	  		(fU > fLeft)   &&
   	  		(fV > fTop)   &&
   	  		(fU < fRight) &&
   	  		(fV < fBottom)
 			);
}

inline  BOOL FRECT::IsContain(const FRECT& frTest) const
{
    return	(
   	  		(frTest.fLeft >= fLeft)   &&
   	  		(frTest.fTop >= fTop)   &&
   	  		(frTest.fRight <= fRight) &&
   	  		(frTest.fBottom <= fBottom)
 			);
}

inline  BOOL FRECT::IsTrueContain(const FRECT& frTest) const
{
    return	(
   	  		(frTest.fLeft > fLeft)   &&
   	  		(frTest.fTop > fTop)   &&
   	  		(frTest.fRight < fRight) &&
   	  		(frTest.fBottom < fBottom)
 			);
}

inline  BOOL FRECT::IsIntersect(const FRECT& rDest) const
{
	FLOAT xLeftUp		=   MAX(fLeft,rDest.fLeft);
	FLOAT yLeftUp		=   MAX(fTop,rDest.fTop);
	FLOAT xRightDown	=   MIN(fRight, rDest.fRight);
	FLOAT yRightDown	=   MIN(fBottom, rDest.fBottom);

	if	(xLeftUp >= xRightDown || yLeftUp >= yRightDown) 	
		return  FALSE;
	else
		return  TRUE ;
}

inline  BOOL FRECT::IsTrueIntersect(const FRECT& rDest) const
{
	FLOAT xLeftUp		=   MAX(fLeft,rDest.fLeft);
	FLOAT yLeftUp		=   MAX(fTop,rDest.fTop);
	FLOAT xRightDown	=   MIN(fRight, rDest.fRight);
	FLOAT yRightDown	=   MIN(fBottom, rDest.fBottom);

	if	(xLeftUp > xRightDown || yLeftUp > yRightDown) 	
		return  FALSE;
	else
		return  TRUE ;
}

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//  x, z 系统

struct  GFX3DPOINT2
{
	FLOAT x,z;

	GFX3DPOINT2(){
		x = z = 0.0f;
	}
	GFX3DPOINT2(FLOAT fx, FLOAT fz){
		x = fx; z = fz;
	}
	void Set(FLOAT fx, FLOAT fz){
		x = fx; z = fz;
	}

	inline GFX3DPOINT2 operator + ( const GFX3DPOINT2& pos);
	inline GFX3DPOINT2 operator - ( const GFX3DPOINT2& pos);
	inline GFX3DPOINT2 operator * ( const GFX3DPOINT2& pos);
	inline GFX3DPOINT2 operator / ( const GFX3DPOINT2& pos);

	inline GFX3DPOINT2 operator + ( const FLOAT fR);
	inline GFX3DPOINT2 operator - ( const FLOAT fR);
	inline GFX3DPOINT2 operator * ( const FLOAT fR);
	inline GFX3DPOINT2 operator / ( const FLOAT fR);
};
typedef GFX3DPOINT2* LPGFX3DPOINT2;
typedef GFX3DPOINT2* PGFX3DPOINT2;

inline GFX3DPOINT2 GFX3DPOINT2::operator + ( const GFX3DPOINT2& pos)
{
	GFX3DPOINT2 posRes(x+pos.x,z+pos.z);
	return posRes;
}
inline GFX3DPOINT2 GFX3DPOINT2::operator - ( const GFX3DPOINT2& pos)
{
	GFX3DPOINT2 posRes(x-pos.x,z-pos.z);
	return posRes;
}
inline GFX3DPOINT2 GFX3DPOINT2::operator * ( const GFX3DPOINT2& pos)
{
	GFX3DPOINT2 posRes(x*pos.x,z*pos.z);
	return posRes;
}
inline GFX3DPOINT2 GFX3DPOINT2::operator / ( const GFX3DPOINT2& pos)
{
	GFX3DPOINT2 posRes(x/pos.x,z/pos.z);
	return posRes;
}

inline GFX3DPOINT2 GFX3DPOINT2::operator + ( const FLOAT fR)
{
	GFX3DPOINT2 posRes(x+fR,z+fR);
	return posRes;
}
inline GFX3DPOINT2 GFX3DPOINT2::operator - ( const FLOAT fR)
{
	GFX3DPOINT2 posRes(x-fR,z-fR);
	return posRes;
}
inline GFX3DPOINT2 GFX3DPOINT2::operator * ( const FLOAT fR)
{
	GFX3DPOINT2 posRes(x*fR,z*fR);
	return posRes;
}
inline GFX3DPOINT2 GFX3DPOINT2::operator / ( const FLOAT fR)
{
	GFX3DPOINT2 posRes(x/fR,z/fR);
	return posRes;
}




class  GFX3DRECT2
{
public:
	FLOAT	fLeft;
	FLOAT	fTop;
	FLOAT	fRight;
	FLOAT	fBottom;


	GFX3DRECT2(){
		fLeft = fBottom = 0.0f;
		fRight = fTop = 1.0f;
	}
	GFX3DRECT2(FLOAT fL,FLOAT fT,FLOAT fR,FLOAT fB){
		fLeft = fL; fTop = fT; fRight = fR; fBottom = fB;
	}
	FLOAT GetWidth(){
		return fRight - fLeft;
	}
	FLOAT GetHeight(){
		return fTop - fBottom;
	}

	BOOL IsValid(){
		return GetWidth() > 0.0f && GetHeight() > 0.0f;
	}

	inline  BOOL IsContain(const FLOAT fX,const FLOAT fZ) const;
	inline  BOOL IsTrueContain(const FLOAT fX,const FLOAT fZ) const;
	inline  BOOL IsContain(const GFX3DRECT2& frTest) const;
	inline  BOOL IsTrueContain(const GFX3DRECT2& frTest) const;
	inline  BOOL IsIntersect(const GFX3DRECT2& rDest) const;
	inline  BOOL IsTrueIntersect(const GFX3DRECT2& rDest) const;
};
typedef GFX3DRECT2* LPGFX3DRECT2;
typedef GFX3DRECT2* PGFX3DRECT2;

inline  BOOL GFX3DRECT2::IsContain(const FLOAT fX,const FLOAT fZ) const
{
    return	(
   	  		(fX >= fLeft)   &&
   	  		(fZ >= fBottom)  &&
   	  		(fX < fRight) &&
   	  		(fZ < fTop)
 			);
}
inline  BOOL GFX3DRECT2::IsTrueContain(const FLOAT fX,const FLOAT fZ) const
{
    return	(
   	  		(fX > fLeft)   &&
   	  		(fZ > fBottom)   &&
   	  		(fX < fRight) &&
   	  		(fZ < fTop)
 			);
}

inline  BOOL GFX3DRECT2::IsContain(const GFX3DRECT2& frTest) const
{
    return	(
   	  		(frTest.fLeft >= fLeft)   &&
   	  		(frTest.fBottom >= fBottom)   &&
   	  		(frTest.fRight < fRight) &&
   	  		(frTest.fTop < fTop)
 			);
}

inline  BOOL GFX3DRECT2::IsTrueContain(const GFX3DRECT2& frTest) const
{
    return	(
   	  		(frTest.fLeft > fLeft)   &&
   	  		(frTest.fBottom > fBottom)   &&
   	  		(frTest.fRight < fRight) &&
   	  		(frTest.fTop < fTop)
 			);
}

inline  BOOL GFX3DRECT2::IsIntersect(const GFX3DRECT2& rDest) const
{
	FLOAT xLeftUp		=   MAX(fLeft,rDest.fLeft);
	FLOAT yLeftUp		=   MIN(fTop,rDest.fTop);
	FLOAT xRightDown	=   MIN(fRight, rDest.fRight);
	FLOAT yRightDown	=   MAX(fBottom, rDest.fBottom);

	if	(xLeftUp >= xRightDown || yLeftUp <= yRightDown) 	
		return  FALSE;
	else
		return  TRUE ;
}

inline  BOOL GFX3DRECT2::IsTrueIntersect(const GFX3DRECT2& rDest) const
{
	FLOAT xLeftUp		=   MAX(fLeft,rDest.fLeft);
	FLOAT yLeftUp		=   MIN(fTop,rDest.fTop);
	FLOAT xRightDown	=   MIN(fRight, rDest.fRight);
	FLOAT yRightDown	=   MAX(fBottom, rDest.fBottom);

	if	(xLeftUp > xRightDown || yLeftUp < yRightDown) 	
		return  FALSE;
	else
		return  TRUE ;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

//	GFX3DLINE2						....... Describes a 2d line
// a * x + b * z + c = 0.0f
struct  GFX3DLINE2
{
	FLOAT a,b,c;

	GFX3DLINE2(){
		a = b = c =0.0f;
	}

	// construct a line with the begin point and end point
	GFX3DLINE2(FLOAT x0, FLOAT z0, FLOAT x1, FLOAT z1){
		Build(x0,z0,x1,z1);
	}

	GFX3DLINE2(FLOAT fA, FLOAT fB, FLOAT fC){
		Set(fA,fB,fC);
	}

	void Set(FLOAT fA, FLOAT fB, FLOAT fC){
		a = fA; b = fB; c = fC;
	}

	// build a line with the begin point and end point
	inline void Build(FLOAT x0, FLOAT z0, FLOAT x1, FLOAT z1);

	// normalize
	BOOL IsNormalized(){
		return a*a + b*b > 0.999f &&
			a*a + b*b < 1.001f;
	}
	inline void Normalize();

	inline FLOAT DotPoint(FLOAT x,FLOAT z);
	inline FLOAT DotPoint(FLOAT x,FLOAT z,FLOAT w);

	// whether is Intersect with a part of line
	inline  BOOL IsIntersect(const GFX3DPOINT2* pfp0, const GFX3DPOINT2* pfp1) const;
	inline  BOOL IsTrueIntersect(const GFX3DPOINT2* pfp0, const GFX3DPOINT2* pfp1) const;
};
typedef GFX3DLINE2* LPGFX3DLINE2;
typedef GFX3DLINE2* PGFX3DLINE2;

inline void GFX3DLINE2::Build(FLOAT x0, FLOAT z0, FLOAT x1, FLOAT z1)
{
	if ( x1==x0 )
	{
		assert(z1 != z0);
		a = z1-z0;
		b = 0.0f;
		c = -x0*a;
	}
	else
	{
		a = (z1-z0)/(x1-x0);
		b = -1.0f;
		c = z0 - a*x0;
		if (x1<x0)
		{
			a = -a; b = -b; c = -c;
		}
	}
	Normalize();
}

inline void GFX3DLINE2::Normalize()
{
	FLOAT fT = sqrtf(1/(a*a + b*b));
	a *= fT;
	b *= fT;
	c *= fT;
}

inline FLOAT GFX3DLINE2::DotPoint(FLOAT x,FLOAT z)
{
	return a*x + b*z + c;
}
inline FLOAT GFX3DLINE2::DotPoint(FLOAT x,FLOAT z,FLOAT w)
{
	return a*x + b*z + c*w;
}


inline  BOOL GFX3DLINE2::IsIntersect(const GFX3DPOINT2* pfp0, const GFX3DPOINT2* pfp1) const
{
	float fDot0 = a*pfp0->x + b*pfp0->z + c;
	if ( fDot0 == 0.0f )
		return TRUE;
	else if ( fDot0 < 0.0f && a*pfp1->x + b*pfp1->z + c >= 0.0f )
		return TRUE;
	else if ( fDot0 > 0.0f && a*pfp1->x + b*pfp1->z + c <= 0.0f )
		return TRUE;
	else
		return FALSE;
}

inline  BOOL GFX3DLINE2::IsTrueIntersect(const GFX3DPOINT2* pfp0, const GFX3DPOINT2* pfp1) const
{
	float fDot0 = a*pfp0->x + b*pfp0->z + c;
	if ( fDot0 < 0.0f && a*pfp1->x + b*pfp1->z + c > 0.0f )
		return TRUE;
	else if ( fDot0 > 0.0f && a*pfp1->x + b*pfp1->z + c < 0.0f )
		return TRUE;
	else
		return FALSE;
}


inline GFX3DPOINT2* Gfx3DLineIntersectLine(GFX3DPOINT2* pOut,GFX3DLINE2* pLine0,GFX3DLINE2* pLine1)
{
	float fT = (pLine0->a * pLine1->b) - (pLine1->a * pLine0->b);
	if ( fT == 0.0f )
		return NULL;
	pOut->z = (pLine1->a * pLine0->c) - (pLine0->a * pLine1->c);
	pOut->z /= fT;
	if ( pLine0->a != 0.0f )
		pOut->x = (- pLine0->b * pOut->z - pLine0->c) / pLine0->a;
	else
		pOut->x = (- pLine1->b * pOut->z - pLine1->c) / pLine1->a;
//	assert(pLine0->DotPoint(pOut->x,pOut->z) == 0.0f);
//	assert(pLine1->DotPoint(pOut->x,pOut->z) == 0.0f);
	return pOut;
}


///////////////////////////////////////////////////////////////////
// dist functions
// copy from global.h in qinshang 

inline BOOL   IsDistIn( const GPOINT& gp0, const GPOINT& gp1,const int iDist )
{	
	return	( abs(gp0.x - gp1.x) < iDist ) &&  
			( abs(gp0.y - gp1.y) < iDist );
}

inline	int FastDist(const GPOINT& gp0,const GPOINT& gp1)
{
	return abs(gp0.x - gp1.x) +  abs(gp0.y - gp1.y);
}

inline int	FastDist(int x0,int y0,int x1,int y1)
{
	  return abs(x1 - x0) + abs(y1 - y0);

}

inline int  SquareAdd(int x0, int y0, int x1, int y1)
{
	return (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
}

inline int  SquareAdd(const GPOINT& gp0, const GPOINT& gp1)
{
	return (gp0.x - gp1.x) * (gp0.x - gp1.x) + (gp0.y - gp1.y) * (gp0.y - gp1.y);
}

///////////////////////////////
//	Distance to 0,0  
//	Call math function only if large than MAX_X
inline  int     Dist ( int x, int y)
{
    assert ( x >= 0 && y >= 0 ) ;
    return (int)(sqrt((double)( x*x + y*y))) ; 
}

inline int     Dist (int x0,int y0,int x1,int y1 )
{
    return Dist(MAX(x0,x1) - MIN(x0,x1),MAX(y0,y1) - MIN(y0,y1));
}

inline int     Dist ( const GPOINT& gp0, const GPOINT& gp1 )
{
    return Dist(MAX(gp0.x,gp1.x) - MIN(gp0.x,gp1.x),MAX(gp0.y,gp1.y) - MIN(gp0.y,gp1.y));
}

//3舍4入
inline  int     Dist34( int x, int y)
{
    assert ( x >= 0 && y >= 0 ) ;
    return (int)(sqrt((double)( x*x + y*y)) + 0.6f) ; 
}

inline int     Dist34( const GPOINT& gp0, const GPOINT& gp1 )
{
    return Dist34(MAX(gp0.x,gp1.x) - MIN(gp0.x,gp1.x),MAX(gp0.y,gp1.y) - MIN(gp0.y,gp1.y));
}



/////////////////////////////////////////////////////////////////////////////////////
// 计算方向

 DIRECTION WhatDirection(const GPOINT& gpDest,const GPOINT& gpSour);

//precision精确计算8方向
 int WhatDir8(const GPOINT& gpDest,const GPOINT& gpSour);
//precision精确计算16方向
 int WhatDir16(const GPOINT& gpDest,const GPOINT& gpSour);


/* 是否越过目的点
	gpDest	目的坐标	
	gpCurPos当前坐标
	dir		方向		
*/
 BOOL IsExceedDest(  const GPOINT & gpDest,const GPOINT & gpCurPos,int iDir );

/*	按速度，方向移动坐标
	iSpeed	速度		
	dir		方向		
	gp		坐标		
*/
 void   MoveDirForSpeed(GPOINT& gp,const int &dir,const int &iSpeed);


 void	ClockEllipsePathMove( GPOINT & gpWorldPos,GPOINT gpCenterPos,int iRadius,int iSpeed );

 ///////////////////////////
 // a new mathod to calculate direction
 // I havn't replace WhatDirection with this one,
 // because 
 // 1: time cost
 // 2: the path_finding will be chged
 //
 DIRECTION CalDirection(const GPOINT& gpDest,const GPOINT& gpSour);

//gpBegin	起始点
//gpEnd		目标点
//iMax		最大距离
//iMin		最小距离
 BOOL	  IsRange( const GPOINT & gpBegin,const GPOINT & gpEnd,int iMax,int iMin );
//gpBegin	起始点
//gpEnd		目标点
//iMax		最大距离
 BOOL	  IsMaxRange( const GPOINT & gpBegin,const GPOINT & gpEnd,int iMax);
//gpBegin	起始点
//gpEnd		目标点
//iMin		最小距离
 BOOL	  IsMinRange( const GPOINT & gpBegin,const GPOINT & gpEnd,int iMin );

//gpEnemy	敌人位置
//gpSelf	自己位置
//iDis		安全距离
//iDirStep  方向偏差
 GPOINT	  GetFleePos( const GPOINT & gpEnemy,const GPOINT & gpSelf,int iDis,int iDirStep = 0 );
 GPOINT	  GetZhuiPos( const GPOINT & gpEnemy,const GPOINT & gpSelf,int iDis,int iDirStep = 0 );





inline float	GetAngle( const GPOINT & gpStart,const GPOINT & gpEnd )
{
	GPOINT	gDir;	
	gDir.x = gpStart.x - gpEnd.x;
	gDir.y = gpStart.y - gpEnd.y;
	float fAngle;

	if( gDir.y > 0.0f )			fAngle = -atanf((float)gDir.x/(float)gDir.y)+PI_DIV_2;
	else if( gDir.y < 0.0f )	fAngle = -atanf((float)gDir.x/(float)gDir.y)-PI_DIV_2;
	else
	{
		if( gDir.x > 0  )		fAngle = 0;
		else					fAngle = -PI;//-PI/2
	}
	return fAngle;
}

inline void	MoveForAngle( GPOINT & gpOut,const GPOINT & gpCur,float fAngle,float fSpeed )
{
	float fData;
	fData = cosf(fAngle +PI)*fSpeed;
	if( fData < 0 )
		fData -= 0.5f;
	else if( fData > 0 )
		fData += 0.5f;
	gpOut.x = gpCur.x + (int)fData;
	fData = sinf(fAngle +PI)*fSpeed;
	if( fData < 0 )
		fData -= 0.5f;
	else if( fData > 0 )
		fData += 0.5f;
	gpOut.y = gpCur.y + (int)fData;
}


BYTE	AngleToDir16( float fAngle);
float	Dir16ToAngle( BYTE bDir );

void	SetDestInRange( GPOINT & gpDest,const GPOINT & gpStart,int iMin,int iMax );
void	SetDestInRange( GPOINT & gpDest,const GPOINT & gpStart,int iMin,int iMax,float fAngle );

void	PointMakeRect( GRECT & grOut,GPOINT gpBegin,GPOINT gpEnd );
#endif
