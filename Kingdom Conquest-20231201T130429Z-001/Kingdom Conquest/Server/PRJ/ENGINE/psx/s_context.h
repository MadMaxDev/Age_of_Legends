// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-31

#ifndef __S_CONTEXT_H_G_SRC_SOURCE_PG_PSX__
#define __S_CONTEXT_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/config.h>

__PG_PSX_SPACE_BEGIN__

class pg_psx_public s_context
{
protected:
    friend class s_engine;
    psIScriptContext*   m_handle;
public:
    s_context()
    {
        m_handle = 0;
    }
    ~s_context()
    {
        if(m_handle)
        {
            m_handle->Release();
        }
    }

    psIScriptContext* handle() const
    {
        return m_handle;
    }
};

__PG_PSX_SPACE_END__

#endif  // __S_CONTEXT_H_G_SRC_SOURCE_PG_PSX__