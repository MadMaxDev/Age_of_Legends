// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-1

#include <guic/functions/iteration_head.h>


#define PG_PP_FNR_CALL_I(J,I,D) po.set_arg(I, BOOST_PP_CAT(a,I))
#define PG_PP_FNR_CALL          BOOST_PP_ENUM(BOOST_PP_ITERATION(),PG_PP_FNR_CALL_I,BOOST_PP_EMPTY)


template<typename Policy, typename R
    PG_PP_FN_COMMA
    PG_PP_FN_TEMPLATE_PARAMS>
class functor<R (PG_PP_FN_PARAMS), Policy> : public functor_base
{
public:
    static void build_decl_s(s_engine& ng, wstring::scoped_string_t& buf, const wchar_t* fnName)
    {
        Policy::builder_t::build_fn<R (PG_PP_FN_PARAMS)>(ng, buf, fnName);
    }

    void build_decl(wstring::scoped_string_t& buf, const wchar_t* fnName)
    {
        Policy::builder_t::build_fn<R (PG_PP_FN_PARAMS)>(*m_engine, buf, fnName);
    }
    
    bool load(s_engine& ng, const wchar_t* modName, const wchar_t* fnName)
    {
        wstring::scoped_string_t buf;
        build_decl(buf, fnName);
        return load_d(ng, modName, buf.c_str());
    }
    bool load(s_engine& ng, psIModule* mod, const wchar_t* fnName)
    {
        wstring::scoped_string_t buf;
        build_decl(buf, fnName);
        return load_d(ng, mod, buf.c_str());
    }

    R operator () (PG_PP_FN_PARAMS) const
    {
        pg_assert(!empty());

        Policy po(*m_engine);

        prepare(po.m_context);
        PG_PP_FNR_CALL;
        execute(po.m_context);
        return po.get_ret<R>();
    }
};



#undef PG_PP_FNR_CALL_I
#undef PG_PP_FNR_CALL

#include <guic/functions/iteration_foot.h>
