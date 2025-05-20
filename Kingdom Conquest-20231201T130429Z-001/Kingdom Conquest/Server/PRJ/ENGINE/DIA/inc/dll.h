// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: dll.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_DLL_H__
#define __DIA_DLL_H__

#include "numeric_type.h"

//////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__
#include <dlfcn.h>
#define dia_export
#define dia_import
typedef void* dia_dll_handle;
#else
#define dia_export __declspec(dllexport)
#define dia_import __declspec(dllimport)
typedef HMODULE dia_dll_handle;
#endif

//////////////////////////////////////////////////////////////////////////
dia_dll_handle dd_open(const char *dll_name);
void dd_close(dia_dll_handle handle);

void* dd_get_func(dia_dll_handle handle, const char *func_name);


#endif

