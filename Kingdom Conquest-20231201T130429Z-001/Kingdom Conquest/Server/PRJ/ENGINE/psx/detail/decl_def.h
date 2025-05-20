// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-2

#ifndef PG_PSX_DECL_DEF
#   error must define PG_PSX_DECL_DEF before include "decl_def.h"
#endif // PG_PSX_DECL_DEF

#ifndef PG_PSX_DECL_PRIM_DEF
#   error must define PG_PSX_DECL_PRIM_DEF before include "decl_def.h"
#endif // PG_PSX_DECL_PRIM_DEF

#ifdef _NATIVE_WCHAR_T_DEFINED
    PG_PSX_DECL_DEF(wchar_t,        L"wchar_t")
    PG_PSX_DECL_DEF(unsigned short, L"short")
#else
    PG_PSX_DECL_DEF(wchar_t,        L"wchar_t")
#endif

PG_PSX_DECL_DEF(char,           L"char")
PG_PSX_DECL_DEF(short,          L"short")
PG_PSX_DECL_DEF(int,            L"int")
PG_PSX_DECL_DEF(__int64,        L"int64")
PG_PSX_DECL_DEF(unsigned char,  L"char")
//PG_PSX_DECL_DEF(unsigned short, L"short")
PG_PSX_DECL_DEF(unsigned int,   L"int")
PG_PSX_DECL_DEF(uint64_t,       L"int64")
PG_PSX_DECL_DEF(double,         L"double")
PG_PSX_DECL_DEF(float,          L"float")
PG_PSX_DECL_DEF(bool,           L"bool")

PG_PSX_DECL_PRIM_DEF(void,              L"void")
PG_PSX_DECL_PRIM_DEF(void*,              L"void*")

#undef PG_PSX_DECL_PRIM_DEF
#undef PG_PSX_DECL_DEF