#ifndef __PS_TYPES_H__
#define __PS_TYPES_H__

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

//-------------------------------------------------------------------
// 类型定义
//-------------------------------------------------------------------
typedef unsigned long   psDWORD;
typedef unsigned short  psWORD;
typedef unsigned char   psBYTE;
typedef unsigned int    psUINT;

#ifdef WIN32
#ifdef _MSC_VER
#define ps_vsnwprintf   _vsnwprintf
#define ps_snwprintf    _snwprintf
#else
#define ps_vsnwprintf   vsnwprintf
#define ps_snwprintf    snwprintf
#endif
#endif
#ifdef __GNUC__
#define ps_vsnwprintf   vswprintf
#define ps_snwprintf    swprintf
#endif

//#define PS_UNICODE
#ifdef PS_UNICODE
	#include <wchar.h>      // wchar
	typedef wchar_t			psCHAR;
	#define PS_T(x)			L##x
	#define ps_strlen	   wcslen
	#define ps_strcpy	   wcscpy
	#define ps_strcmp	   wcscmp
	#define ps_strtod	   wcstod
	#define ps_strtol	   wcstol	
	#define	ps_sscanf	   swscanf
	#define ps_vsnprintf   ps_vsnwprintf
	#define ps_snprintf    ps_snwprintf
#else
	typedef char			psCHAR;
	#define PS_T(x)			x
	#define ps_strlen	   strlen
	#define ps_strcpy	   strcpy
	#define ps_strcmp	   strcmp
	#define ps_strtod	   strtod
	#define ps_strtol	   strtol
	#define ps_sscanf	   sscanf
#ifdef _MSC_VER
	#define ps_vsnprintf   _vsnprintf
	#define ps_snprintf    _snprintf
#else
	#define ps_vsnprintf   vsnprintf
	#define ps_snprintf    snprintf
#endif
#endif

#if defined(_MSC_VER)
	typedef __int64			psQWORD;
	#define PS_STDCALL		__stdcall
	#define PS_CDECL		__cdecl
#else
	typedef long long		psQWORD;
	#define PS_STDCALL		
	#define PS_CDECL
#endif

#ifndef NULL
	#define NULL		0
#endif

#include <string>

#endif // __PS_TYPE_H__
