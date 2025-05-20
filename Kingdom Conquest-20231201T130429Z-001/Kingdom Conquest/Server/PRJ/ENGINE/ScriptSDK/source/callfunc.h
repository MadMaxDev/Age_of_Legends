#ifndef __PS_CALLFUNC_H__
#define __PS_CALLFUNC_H__

#include "context.h"

enum psEInternalCallConv
{
	ICC_CDECL,
	ICC_CDECL_RETURNINMEM,
	ICC_STDCALL,
	ICC_STDCALL_RETURNINMEM,
	ICC_THISCALL,
	ICC_THISCALL_RETURNINMEM,
	ICC_THIS_STDCALL,
	ICC_THIS_STDCALL_RETURNINEME,
	ICC_VIRTUAL_THISCALL,
	ICC_VIRTUAL_THISCALL_RETURNINMEM,
	ICC_CDECL_OBJFIRST,
	ICC_CDECL_OBJFIRST_RETURNINMEM,
};

enum HOST_RETURN_TYPE
{
	HOST_RETURN_VOID,
	HOST_RETURN_BYTE,
	HOST_RETURN_WORD, 
	HOST_RETURN_DWORD,
	HOST_RETURN_QWORD, 
	HOST_RETURN_FLOAT, 
	HOST_RETURN_DOUBLE
};

struct psSSystemFunctionInterface
{
	void				*func;
	int                  baseOffset;
	psEInternalCallConv  callConv;
	int                  scriptReturnSize;
	int                  paramSize;
	bool                 hostReturnInMemory;
	bool				 takesObjByVal;
	HOST_RETURN_TYPE     hostReturnType;
};

namespace pse 
{
	int DetectCallingConvention(bool bObject, psUPtr &ptr, psDWORD callConv, psSSystemFunctionInterface *internal);

	int PrepareSystemFunction(psCScriptFunction *func, psSSystemFunctionInterface *internal, psCScriptEngine *engine);

	int CallSystemFunction(int id, int paramSize, psCContext *context);
}

#endif // __PS_CALLFUNC_H__

