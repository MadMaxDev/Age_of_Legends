// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-2

#include "commoninc.h"
#include <psx/global.h>
#include <guic/error.h>

__PG_PSX_SPACE_BEGIN__

void global_base::inter_const_int(const wchar_t* name, int val)
{
    int r = m_sEngine->RegisterIntConstant(name, val);
    if(r < 0)
    {
        throw error_info(L"failed register int constant <%s>, val <%d>, error <%d>.",
            name, val, r);
    }
}

void global_base::inter_property_d(const wchar_t* decl, void* p)
{
    int r = m_sEngine->RegisterGlobalProperty(decl, p);
    if(r < 0)
    {
        throw error_info(L"failed register global property <%s>, data <%.8X>, error <%d>.",
            decl, p, r);
    }
}

void global_base::inter_property_d(const wchar_t* decl, const void* p)
{
    int r = m_sEngine->RegisterGlobalProperty(decl, const_cast<void*>(p));
    if(r < 0)
    {
        throw error_info(L"failed register global const property <%s>, data <%.8X>, error <%d>.",
            decl, p, r);
    }
}

void global_base::inter_def_d(const wchar_t* decl, void* func, uint_t callConv)
{
    switch(callConv)
    {
    case pg::functions::cc_cdecl:
        callConv = psCALL_CDECL;
        break;
    case pg::functions::cc_stdcall:
        callConv = psCALL_STDCALL;
        break;
    case pg::functions::cc_thiscall:
        callConv = psCALL_THISCALL;
        break;
    default:
        throw error_info(L"register global function <%s> with invalid calling convention <%d>.", decl, callConv);
        break;
    }

    psUPtr funcPointer;
    memset(&funcPointer, 0, sizeof(funcPointer));
    funcPointer.func = (psFUNCTION_t)func;

    int r = m_sEngine->RegisterGlobalFunction(decl, funcPointer, callConv);
    if(r < 0)
    {
        throw error_info(L"failed register global function <%s>.", decl);
    }
}

void global_base::inter_behav_d(uint_t behav, const wchar_t* decl, void* ptr, uint_t callConv)
{
    switch(callConv)
    {
    case pg::functions::cc_cdecl:
        callConv = psCALL_CDECL;
        break;
    case pg::functions::cc_stdcall:
        callConv = psCALL_STDCALL;
        break;
    case pg::functions::cc_thiscall:
        callConv = psCALL_THISCALL;
        break;
    default:
        throw error_info(L"register global behaviour <%s> with invalid calling convention <%d>.",
            decl, callConv);
        break;
    }

    psUPtr psFunc;
    memset(&psFunc, 0, sizeof(psFunc));
    psFunc.func = (psFUNCTION_t)ptr;

    int r = m_sEngine->RegisterGlobalBehaviour(behav, decl, psFunc, callConv);
    if(r < 0)
    {
        throw error_info(L"failed register global behaviour <%s>, address <%d>, error <%d>.",
            decl, ptr, r);
    }
}

__PG_PSX_SPACE_END__