// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-31

#ifndef __S_MODULE_H_G_SRC_SOURCE_PG_PSX__
#define __S_MODULE_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/s_engine.h>

__PG_PSX_SPACE_BEGIN__

class pg_psx_public s_module
{
protected:
    psIModule*  m_handle;
    s_engine*   m_engine;
public:
    s_module()
        : m_handle(NULL)
        , m_engine(NULL)
    {
    }
    s_module(s_engine& ng, psIModule* mod)
        : m_handle(mod)
        , m_engine(&ng)
    {
    }
    s_module(s_engine& ng, const wchar_t* name, const wchar_t* file);

    bool valid() const
    {
        return m_handle != NULL;
    }
    psIModule* handle() const
    {
        return m_handle;
    }
    operator psIModule* () const
    {
        return m_handle;
    }

    bool add_script(const wcstring& code, const wchar_t* filename = NULL);
    bool compile(psITextStream *txt, psIOutputStream* out);
    bool compile();
	int load_bytecode(psIBinaryStream* in);

    int function_id_by_decl(const wchar_t* decl);
    int function_id_by_name(const wchar_t* name);

};

__PG_PSX_SPACE_END__

#endif  // __S_MODULE_H_G_SRC_SOURCE_PG_PSX__