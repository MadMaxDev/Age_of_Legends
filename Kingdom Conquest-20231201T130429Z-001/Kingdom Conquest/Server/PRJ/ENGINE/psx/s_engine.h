// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-26

#ifndef __S_ENGINE_H_G_SRC_SOURCE_PG_PSX__
#define __S_ENGINE_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/config.h>
#include <guic/ref_tier.h>
#include <guic/error.h>
#include <boost/any.hpp>
#include <boost/smart_ptr.hpp>

__PG_PSX_SPACE_BEGIN__

namespace decl_text
{
    class builder;
};

template<typename Builder> class global_;
template<typename T, typename B, typename Builder> class object_;

class s_engine;

class s_ref_module : public ref_base
{
    pg_noncopyable_decl(s_ref_module)
public:
    psIModule*  m_module;
    s_engine*   m_engine;
    wstring     m_file;

    s_ref_module(s_engine& ng, psIModule* mod)
        : m_engine(&ng)
        , m_module(mod)
    {
    }
    virtual ~s_ref_module();
};

class pg_psx_public s_engine
{
    pg_noncopyable_decl(s_engine)
public:
    friend class s_ref_module;

    typedef std::map< string, wstring, std::less<string>, memory::pool_allocator<byte_t> > decl_types;
    typedef std::map< const std::type_info*, wstring, std::less<const std::type_info*>, memory::pool_allocator<byte_t> > decl_infos;
    typedef std::map< wstring, string, std::less<wstring>, memory::pool_allocator<byte_t> > decl_checks;
    typedef std::map<psIModule*, s_ref_module*, std::less<psIModule*>, memory::pool_allocator<byte_t> > ref_modules;
    typedef std::list< ref_ptr<s_ref_module> > hold_modules;
    typedef std::vector< psIScriptContext*, memory::pool_allocator<byte_t> > ctx_set;

    //s_context           m_context;
    std::list<boost::any>   m_objects;
    ctx_set   m_ctx;
    
    decl_types  m_declTypes;
    decl_infos  m_declInfos;
    decl_checks m_declChecks;

    ref_modules m_refModules;
    hold_modules    m_holdModules;
    
public:
    psIScriptEngine*    m_handle;
    i_error_reporter*   m_reporter;
public:
    s_engine();
    virtual ~s_engine();

    bool create(psIScriptEngine* pEng = NULL);
    void destroy();

    psIScriptEngine* handle() const
    {
        return m_handle;
    }
    psIScriptContext* get_ctx();
    void put_ctx(psIScriptContext*);
    //s_context& context()
    //{
    //    return m_context;
    //}

    global_<decl_text::builder> global();
    template<typename Builder> global_<Builder> global();

    template<typename T>
    object_<T, int, decl_text::builder> object(const wstring& name, uint_t flag = psOBJ_CLASS_CDA);
    template<typename T, typename B>
    object_<T, B, decl_text::builder> object(const wstring& name, uint_t flag = psOBJ_CLASS_CDA);

    void submit_object(const boost::any& a)
    {
        m_objects.push_back(a);
    }


    s_ref_module* create_ref_module(psIModule* mod);
    void hold_ref_module(s_ref_module*);

    void reg_decl(const std::type_info& i, const wstring& decl, bool& existing);
    const wstring& find_decl(const std::type_info& i);

    // execute a script string
    bool execute(const wcstring& str);
    bool execute(const wcstring& str, psIOutputStream *out);
};

__PG_PSX_SPACE_END__

#endif  // __S_ENGINE_H_G_SRC_SOURCE_PG_PSX__