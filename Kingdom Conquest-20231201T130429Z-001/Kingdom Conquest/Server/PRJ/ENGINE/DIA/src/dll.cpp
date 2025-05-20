// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: dll.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include "../inc/dll.h"

#ifdef __GNUC__

dia_dll_handle dd_open(const char *dll_name)
{
	return ::dlopen(dll_name, RTLD_LAZY);
}
void dd_close(dia_dll_handle handle)
{
	::dlclose(handle);
}

void* dd_get_func(dia_dll_handle handle, const char *func_name)
{
	return ::dlsym(handle, func_name);
}

#else

dia_dll_handle dd_open(const char *dll_name)
{
	return LoadLibrary(dll_name);
}
void dd_close(dia_dll_handle handle)
{
	FreeLibrary(handle);
}

void* dd_get_func(dia_dll_handle handle, const char *func_name)
{
	return GetProcAddress(handle, func_name);
}

#endif

