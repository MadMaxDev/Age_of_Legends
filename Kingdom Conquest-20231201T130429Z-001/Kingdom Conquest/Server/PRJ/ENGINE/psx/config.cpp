// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-26

#include "commoninc.h"
#include <psx/config.h>
#include <guic/lib_name.h>

__PG_PSX_SPACE_BEGIN__

#ifdef _WINDLL

#   pragma comment(lib, PG_LIB_NAME("pg_guic"))

#   ifdef PG_DEBUG
#       pragma comment(lib, "PixelScript_UD.lib")
#   else
#       pragma comment(lib, "PixelScript_U.lib")
#   endif

#endif

__PG_PSX_SPACE_END__