// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-5

#include "commoninc.h"
#include <psx/scope.h>

__PG_PSX_SPACE_BEGIN__


scope::scope(s_engine& ng, const wstring& n)
    : m_engine(&ng)
    , m_sEngine(ng.handle())
    , m_name(n)
{
    
}

__PG_PSX_SPACE_END__