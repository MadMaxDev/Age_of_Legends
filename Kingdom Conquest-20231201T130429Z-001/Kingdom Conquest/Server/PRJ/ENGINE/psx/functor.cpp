// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-1

#include "commoninc.h"
#include <psx/functor.h>
#include <psx/dbg_cfg.h>
#include <guic/error.h>

__PG_PSX_SPACE_BEGIN__

// functor_base

bool functor_base::load_d(s_engine& ng, const wchar_t* mod, const wchar_t* decl)
{
    psIModule* pMod = ng.m_handle->GetModule(mod);
    if(pMod == NULL)
    {
        return false;
    }
    return load_d(ng, pMod, decl);
}

bool functor_base::load_d(s_engine& ng, psIModule* mod, const wchar_t* decl)
{
    int fid = mod->GetFunctionIDByDecl(decl);
    if(fid < 0)
    {
        return false;
    }
    load_id(ng, mod->GetID(), fid);
    return true;
}

void functor_base::prepare(psIScriptContext* ctx) const
{
    int r;
    //psIScriptContext* ctx = m_engine->context().handle();
    r = ctx->Prepare(m_module, m_function, 0);
    if(r < 0)
    {
        throw error_info(L"failed prepare functor module<%d> function<%d> error<%d>.",
            m_module, m_function, r);
    }
    else if(r == psEXECUTION_EXCEPTION)
    {
        if(m_engine->m_reporter)
        {
            psIModule* mod = m_engine->m_handle->GetModule(m_module);
            if(mod)
            {
                const psIScriptFunction* fun = mod->GetFunctionByID(m_function);
                if(fun)
                {
                    m_engine->m_reporter->report_error(
                        L"failed execute functor module<%s> function<%s> with exception.",
                        mod->GetFileName(), fun->GetName());
                    return;
                }
            }
            m_engine->m_reporter->report_error(
                L"failed prepare functor module<%d> function<%d> error<%d>.",
                m_module, m_function, r);
        }
    }
}

void functor_base::execute(psIScriptContext* ctx) const
{
    int r;
    //psIScriptContext* ctx = m_engine->context().handle();
#if SCRIPT_DEBUGGER
    r = ctx->Execute(psEXEC_DEBUG);
#else
    r = ctx->Execute(0);
#endif
    if(r < 0)
    {
        throw error_info(L"failed execute functor module<%d> function<%d> error<%d>.",
            m_module, m_function, r);
    }
}

// functor_pollicy

void functor_policy::throw_error(int r)
{
    throw error_info(L"failed with error <%d>.", r);
}



__PG_PSX_SPACE_END__