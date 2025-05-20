// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-5

#ifndef __SCOPE_H_G_SRC_SOURCE_PG_PSX__
#define __SCOPE_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/s_engine.h>
#include <psx/decl_text.h>
#include <guic/functions/calling_convention.h>
#include <guic/asm_x/redirect.h>
#include <guic/mpl/remove_pointer.h>

__PG_PSX_SPACE_BEGIN__

class pg_psx_public scope
{
protected:
    s_engine*           m_engine;
    psIScriptEngine*    m_sEngine;
    wstring  m_name;

    template<typename F>
    typename asm_x::redirect_fn< function<F> >::function_ptr_t redirect_code(const function<F>& f)
    {
        typedef asm_x::redirect_fn< function<F> >  redirect_t;
        boost::shared_ptr<redirect_t> rd(new redirect_t(f));
        m_engine->submit_object(rd);
        return rd->code();
    }
public:
    scope(s_engine& ng, const wstring& name);
};

__PG_PSX_SPACE_END__

#endif  // __SCOPE_H_G_SRC_SOURCE_PG_PSX__