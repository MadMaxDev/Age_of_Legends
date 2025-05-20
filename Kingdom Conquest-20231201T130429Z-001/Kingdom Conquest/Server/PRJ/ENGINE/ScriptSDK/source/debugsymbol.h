#ifndef __PS_DEBUGSYMBOL_H__
#define __PS_DEBUGSYMBOL_H__

#include "datatype.h"
#include "string.h"

//-------------------------------------------------------------------
// 类名: psSDebugSymbol
// 说明: 
//-------------------------------------------------------------------
class psSDebugSymbol
{
public:
	psCString	name;
	psCDataType type;
	int			offset;
	int			start;
	int			end;
};

#endif // __PS_DEBUGSYMBOL_H__
