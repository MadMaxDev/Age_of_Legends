// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-31

#include "commoninc.h"
#include <psx/s_module.h>
#include <guic/error.h>
#include <guic/diag/cpu.h>

__PG_PSX_SPACE_BEGIN__

#define COUNTER_GUARD_ENABLE 0

#if COUNTER_GUARD_ENABLE

#define COUNTER_GUARD counter_guard grd

struct counter_data
{
    uint64_t m_tick;
    counter_data()
    {
        SetProcessAffinityMask(GetCurrentProcess(), 1);
        m_tick = 0;
    }
    ~counter_data()
    {
        uint64_t t = m_tick*1000;
        t /= diag::cpu::freq();
        wchar_t buf[32];
        swprintf(buf, L"script time %d.\n", (uint_t)t);
        OutputDebugStringW(buf);
        GetTickCount();
    }
}g_counter_data;

struct counter_guard
{
    uint64_t m_tick;
    counter_guard()
    {
        m_tick = diag::cpu::counter();
    }
    ~counter_guard()
    {
        m_tick = diag::cpu::counter()-m_tick;
        g_counter_data.m_tick += m_tick;
    }
};

#else

#define COUNTER_GUARD

#endif

s_module::s_module(s_engine& ng, const wchar_t* name, const wchar_t* file)
{
    m_engine = &ng;
    m_handle = 0;
    int err = ng.handle()->CreateModule(name, file, &m_handle);
    if(err != psSUCCESS && err != psMODULE_EXIST)
    {
        throw error_info(L"cannot create script module <%s>.", name);
    }
    pg_assert(m_handle);
}

bool s_module::add_script(const wcstring& code, const wchar_t* filename)
{
    filename;
    int r = m_handle->SetCode(code.c_str(), (int)code.size());
    return r >= 0;
}

bool s_module::compile(psITextStream *txt, psIOutputStream* out)
{
    COUNTER_GUARD;
    int r = m_engine->m_handle->Compile(m_handle, txt, out);
    return r >= 0;
}

bool s_module::compile()
{
    COUNTER_GUARD;
    int r = m_engine->m_handle->Compile(m_handle, 0, 0);
    return r >= 0;
}

int s_module::function_id_by_decl(const wchar_t* decl)
{
    if(m_handle == 0)
    {
        return psNO_MODULE;
    }
    return m_handle->GetFunctionIDByDecl(decl);
}

int s_module::function_id_by_name(const wchar_t* name)
{
    if(m_handle == 0)
    {
        return psNO_MODULE;
    }
    return m_handle->GetFunctionIDByName(name);
}

int s_module::load_bytecode(psIBinaryStream* in)
{
	if(m_handle == 0)
    {
        return psNO_MODULE;
    }
	return m_handle->LoadByteCode(in);
}

__PG_PSX_SPACE_END__