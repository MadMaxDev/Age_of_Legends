#ifndef __PS_STDSTRING_H__
#define __PS_STDSTRING_H__

#include <assert.h>
#include "pixelscript.h"
extern "C"
{
	void psRegisterStdStringA( psIScriptEngine* engine );
	void psRegisterStdStringW( psIScriptEngine* engine );
}
#endif // __PS_STDSTRING_H__
