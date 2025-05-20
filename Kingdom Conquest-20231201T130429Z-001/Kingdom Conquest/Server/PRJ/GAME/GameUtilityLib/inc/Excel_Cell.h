#ifndef __EXCEL_CELL_H__
#define __EXCEL_CELL_H__

#include "MemUtil.h"

enum EXCEL_TYPE
{
	EXCEL_TYPE_INT_SCALE		=	0,
	EXCEL_TYPE_FLOAT_SCALE		=	1,
	EXCEL_TYPE_STRING_SCALE		=	2,
	EXCEL_TYPE_INT_VECTOR		=	3,
	EXCEL_TYPE_FLOAT_VECTOR		=	4,
	EXCEL_TYPE_STRING_VECTOR	=	5
};

#pragma pack(4)

struct Excel_Cell : quick_plex_obj<Excel_Cell>
{
public:
	union
	{
		float		theFloat;
		long		theInt;
		const char*	theStr;	
		float*		theFloatVec;
		long*		theIntVec;
		const char**theStrVec;	
	};
	
	inline unsigned long GetVecCnt() const
	{
		return *( theIntVec - 1 );
	}

public:
	
	Excel_Cell() : theInt( 0 )
	{
	}
	
	Excel_Cell( const Excel_Cell& _Cell ) : theInt( _Cell.theInt )
	{
	}

	Excel_Cell& operator = ( const Excel_Cell& _Cell )
	{
		theInt = _Cell.theInt;
		return *this;
	}

public:

	Excel_Cell( float _f ) : theFloat( _f )
	{
	}

	Excel_Cell( long _l ) : theInt( _l )
	{
	}

	Excel_Cell( const char* _sz );

	Excel_Cell( float _fvec[] , unsigned long _cnt );

	Excel_Cell( long _lvec[] , unsigned long _cnt );

	Excel_Cell( const char* _fvec[] , unsigned long _cnt );
};

#pragma pack()

#endif//__EXCEL_CELL_H__
