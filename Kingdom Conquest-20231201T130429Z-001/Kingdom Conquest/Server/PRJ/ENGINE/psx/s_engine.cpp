// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-26

#include "commoninc.h"
#include <psx/s_engine.h>
#include <psx/s_ostream.h>
#include <guic/error.h>

__PG_PSX_SPACE_BEGIN__

s_ref_module::~s_ref_module()
{
    if(m_module)
    {
        const wchar_t* name = m_module->GetName();
        m_engine->m_handle->Discard(name);
        m_engine->m_refModules.erase(m_module);
    }
}

// s_engine

s_engine::s_engine()
{
    m_handle = NULL;
    m_reporter = 0;
}

s_engine::~s_engine()
{
    destroy();
}

bool s_engine::create(psIScriptEngine* pEng)
{
    pg_assert(m_handle == NULL);

    if(pEng == NULL)
    {
        m_handle = ::psCreateScriptEngine(PIXELSCRIPT_VERSION);
    }
    else
    {
        m_handle = pEng;
        pEng->AddRef();
    }
    if(m_handle == NULL)
    {
        return false;
    }

//    int r = m_handle->CreateContext(&m_context.m_handle);
//    if(r < 0)
//    {
//        return false;
//    }

    return true;
}

void s_engine::destroy()
{
    if(m_handle == NULL)
    {
        return;
    }

    m_holdModules.clear();

    pg_assert(m_refModules.empty());
    m_refModules.clear();

    //if(m_context.m_handle)
    //{
    //    m_context.m_handle->Release();
    //    m_context.m_handle = NULL;
    //}

    while(!m_ctx.empty())
    {
        psIScriptContext* c = m_ctx.back();
        m_ctx.pop_back();
        c->Release();
    }

    m_handle->Release();

    m_handle = NULL;
}

psIScriptContext* s_engine::get_ctx()
{
    if(m_ctx.empty())
    {
        psIScriptContext* c;
        int r = m_handle->CreateContext(&c);
        if(r < 0)
        {
            return 0;
        }
        return c;
    }
    psIScriptContext* c = m_ctx.back();
    m_ctx.pop_back();
    return c;
}

void s_engine::put_ctx(psIScriptContext* c)
{
    c->ReleaseModuleRef();
    m_ctx.push_back(c);
}

s_ref_module* s_engine::create_ref_module(psIModule* mod)
{
    s_ref_module* rmod = NULL;
    ref_modules::iterator i = m_refModules.find(mod);
    if(i != m_refModules.end())
    {
        rmod = i->second;
        rmod->add_ref();
    }
    else
    {
        rmod = new s_ref_module(*this, mod);
        m_refModules[mod] = rmod;
    }
    return rmod;
}

void s_engine::hold_ref_module(s_ref_module* m)
{
    m_holdModules.push_back(m);
}

void s_engine::reg_decl(const std::type_info& i, const wstring& decl, bool& existing)
{
    pg_assert(!decl.empty());

    existing = false;

    // check existing
    if(m_declInfos.find(&i) != m_declInfos.end())
    {
        existing = true;
        pg_assert(m_declInfos[&i] == decl);
        return;
    }
    m_declInfos[&i] = decl;

    string iname(i.name());

    // check existing by name
    {
        wstring& d = m_declTypes[iname];
        if(d.empty())
        {
            d = decl;
        }
        else
        {
            existing = true;
            if(d != decl)
            {
                throw error_info(L"register type <%s> with different object name<%s:%s>.",
                    iname.c_str(), d.c_str(), decl.c_str());
            }
        }
    }

#ifdef PG_DEBUG
    //pg_assert(m_declChecks.find(decl) == m_declChecks.end());
    //m_declChecks[decl] = iname;
#endif
}

const wstring& s_engine::find_decl(const std::type_info& i)
{
    {
        decl_infos::iterator it = m_declInfos.find(&i);
        if(it != m_declInfos.end())
        {
            return it->second;
        }
    }

    string iname(i.name());
    decl_types::iterator it = m_declTypes.find(iname);
    if(it == m_declTypes.end())
    {
        static wstring strEmpty;
        return strEmpty;
    }
    m_declInfos[&i] = it->second;
    return it->second;
}

bool s_engine::execute(const wcstring& str)
{
    return execute(str, NULL);
}

bool s_engine::execute(const wcstring& str, psIOutputStream *out)
{
    if(str.empty())
    {
        return true;
    }

    // must be a null terminate string
    pg_assert(str[str.size()] == 0);

    int r = m_handle->ExecuteString(0, str.c_str(), out);
    return r >= 0;
}


__PG_PSX_SPACE_END__