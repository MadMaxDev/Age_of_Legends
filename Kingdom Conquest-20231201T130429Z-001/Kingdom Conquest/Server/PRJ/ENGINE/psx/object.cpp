// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-2

#include "commoninc.h"
#include <psx/object.h>
#include <guic/error.h>
#include <psx/s_engine.h>

__PG_PSX_SPACE_BEGIN__

object_base::object_base(s_engine& ng, const std::type_info& ti, const std::type_info& bti, int off, size_t s, const wstring& n, uint_t flag)
    : scope(ng, n)
    , m_flag(flag)
{
    bool existing = false;
    m_engine->reg_decl(ti, n, existing);
    if(existing)
    {
        return;
    }

    int r = m_sEngine->RegisterObjectType(m_name.c_str(), (int)s, flag);
    if(r < 0)
    {
        throw error_info(L"failed register object <%s>, error <%d>.",
            m_name.c_str(), r);
    }
    if(off == 0x7fffffff)
    {
        return;
    }
    const wstring& d = m_engine->find_decl(bti);
    if(d.empty())
    {
        throw error_info(L"base class <%S> not declare", bti.name());
    }

    r = m_sEngine->DeclareObjectHierarchy(m_name.c_str(), d.c_str(), off);
    if(r < 0)
    {
        throw error_info(L"failed declare object hierarchy <%s,%s>, error <%d>.",
            m_name.c_str(), d.c_str(), r);
    }
};

void object_base::inter_property_d(const wchar_t* decl, size_t offset)
{
    int r = m_sEngine->RegisterObjectProperty(m_name.c_str(), decl, (int)offset);
    if(r < 0)
    {
        throw error_info(L"failed register object property <%s::%s>, offset <%d>, error <%d>.",
            m_name.c_str(), decl, offset, r);
    }
}

void object_base::inter_def_d(const wchar_t* decl, const psUPtr& ptr, uint_t callConv)
{
    switch(callConv)
    {
    case pg::functions::cc_cdecl:
        callConv = psCALL_CDECL_OBJFIRST;
        break;
    case pg::functions::cc_stdcall:
        callConv = psCALL_STDCALL;
        break;
    case pg::functions::cc_thiscall:
        callConv = psCALL_THISCALL;
        break;
    default:
        throw error_info(L"register object function <%s::%s> with invalid calling convention <%d>.",
            m_name.c_str(), decl, callConv);
        break;
    }

    int r = m_sEngine->RegisterObjectMethod(m_name.c_str(), decl, ptr, callConv);
    if(r < 0)
    {
        throw error_info(L"failed register object function <%s::%s>, address <%d>, error <%d>.",
            m_name.c_str(), decl, ptr, r);
    }
}

void object_base::inter_behav_d(uint_t behav, const wchar_t* decl, const psUPtr& ptr, uint_t callConv)
{
    switch(callConv)
    {
    case pg::functions::cc_cdecl:
        callConv = psCALL_CDECL_OBJFIRST;
        break;
    case pg::functions::cc_stdcall:
        callConv = psCALL_STDCALL;
        break;
    case pg::functions::cc_thiscall:
        callConv = psCALL_THISCALL;
        break;
    default:
        throw error_info(L"register object function <%s::%s> with invalid calling convention <%d>.",
            m_name.c_str(), decl, callConv);
        break;
    }

    int r = m_sEngine->RegisterObjectBehaviour(m_name.c_str(), behav, decl, ptr, callConv);
    if(r < 0)
    {
        throw error_info(L"failed register object function <%s::%s>, address <%d>, error <%d>.",
            m_name.c_str(), decl, ptr, r);
    }
}

__PG_PSX_SPACE_END__