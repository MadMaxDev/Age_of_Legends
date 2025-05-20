// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-2

#include "commoninc.h"
#include <psx/decl_text.h>
#include <psx/s_engine.h>
#include <guic/error.h>

__PG_PSX_SPACE_BEGIN__

namespace decl_text
{

#   define PG_PSX_DECL_DEF(t, d) PG_PSX_DECL_MAPI(t, d)
#   define PG_PSX_DECL_PRIM_DEF(t, d) PG_PSX_DECL_MAPI_PRIM(t, d)
#   include "detail/decl_def.h"

    const wstring& builder::find(s_engine& ng, const std::type_info& i)
    {
        const wstring& s = ng.find_decl(i);
        if(s.empty())
        {
            throw error_info_a("cannot find mapping for type <%s>", i.name());
        }
        return s;
    }
}

__PG_PSX_SPACE_END__