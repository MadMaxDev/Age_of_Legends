// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-26

#ifndef __CONFIG_H_G_SRC_SOURCE_PG_PSX__
#define __CONFIG_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <guic/config.h>
#include <guic/text/x_string.h>
#include <guic/function.h>
#include <guic/mem_fn.h>

#ifndef pg_psx_public
#   define pg_psx_public pg_dllimport
#endif


// include pixel script sdk

#ifndef PG_LINK_LIB
#   define PIXELSCRIPT_DLL_LIBRARY_IMPORT
#endif

#define PS_UNICODE

#include <psx/config/ps_disable_warnings.h>
#include <ScriptSDK/include/pixelscript.h>
#include <psx/config/ps_default_warnings.h>

#define PG_PSX_SPACE psx

#define __PG_PSX_SPACE_BEGIN__ __PG_BASE_SPACE_BEGIN__ namespace PG_PSX_SPACE {
#define __PG_PSX_SPACE_END__ } __PG_BASE_SPACE_END__

__PG_PSX_SPACE_BEGIN__

typedef text::string string;
typedef text::wstring wstring;
typedef text::cstring cstring;
typedef text::wcstring wcstring;

const int code_max = (1024*1024*1024);

__PG_PSX_SPACE_END__

#endif  // __CONFIG_H_G_SRC_SOURCE_PG_PSX__