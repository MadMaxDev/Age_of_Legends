// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-2

#include <guic/functions/iteration_head.h>

#define PG_PP_FN_BUILD_I(J,I,D) \
    builder::build<BOOST_PP_CAT(T,I)>(ng, buf),\
    append_comma<I>::append(buf)
#define PG_PP_FN_BUILD      BOOST_PP_ENUM(BOOST_PP_ITERATION(),PG_PP_FN_BUILD_I,BOOST_PP_EMPTY)


template<typename R PG_PP_FN_COMMA
    PG_PP_FN_TEMPLATE_PARAMS>
class decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
public:
    template<size_t ArgIdx>
    class append_comma
    {
    public:
        template<typename Buf>
        static void append(Buf& buf)
        {
            buf.append(L", ");
        }
    };
    template<>
    class append_comma<BOOST_PP_ITERATION()-1>
    {
    public:
        template<typename Buf>
        static void append(Buf& )
        {
        }
    };
    template<typename Buf>
    static void build(s_engine& ng, Buf& buf, const wchar_t* name)
    {
        builder::build<R>(ng, buf);
        buf.append(L" ");
        buf.append(name);
        buf.append(L"(");
        PG_PP_FN_BUILD;
        buf.append(L")");
    }
};

template<typename R PG_PP_FN_COMMA
    PG_PP_FN_TEMPLATE_PARAMS>
class decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
public:
    template<typename Buf>
    static void build(s_engine& ng, Buf& buf, const wchar_t* name)
    {
        builder::build<R>(ng, buf);
        buf.append(L" ");
        buf.append(name);
        buf.append(L"(");
#if BOOST_PP_ITERATION() > 0
        PG_PP_FN_BUILD;
        buf.append(L", ...");
#else
        buf.append(L"...");
#endif
        buf.append(L")");
    }
};

// functions
template<typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_fn<R (__cdecl *)(PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_fn<R __cdecl (PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_fn<R (__cdecl *)(PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_fn<R __cdecl (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_fn<R (__stdcall *)(PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_fn<R __stdcall (PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};

// methods
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl C::*)(PG_PP_FN_PARAMS) const> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl C::*)(PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl *)(C* PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __cdecl (C* PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl *)(const C* PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __cdecl (const C* PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl *)(C& PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __cdecl (C& PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl *)(const C& PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __cdecl (const C& PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};

// variable parameter
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl C::*)(PG_PP_FN_PARAMS PG_PP_FN_COMMA ...) const> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl C::*)(PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl *)(C* PG_PP_FN_COMMA PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __cdecl (C* PG_PP_FN_COMMA PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl *)(const C* PG_PP_FN_COMMA PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __cdecl (const C* PG_PP_FN_COMMA PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl *)(C& PG_PP_FN_COMMA PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __cdecl (C& PG_PP_FN_COMMA PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__cdecl *)(const C& PG_PP_FN_COMMA PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __cdecl (const C& PG_PP_FN_COMMA PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)> : public decl_fn_mf<R (PG_PP_FN_PARAMS PG_PP_FN_COMMA ...)>
{
};
// variable parameter end

template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__stdcall C::*)(PG_PP_FN_PARAMS) const> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__stdcall C::*)(PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__stdcall *)(C* PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __stdcall (C* PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__stdcall *)(const C* PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __stdcall (const C* PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__stdcall *)(C& PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __stdcall (C& PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (__stdcall *)(const C& PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R __stdcall (const C& PG_PP_FN_COMMA PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};

template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (C::*)(PG_PP_FN_PARAMS) const> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};
template<typename C, typename R PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_PARAMS>
class decl_mf<C, R (C::*)(PG_PP_FN_PARAMS)> : public decl_fn_mf<R (PG_PP_FN_PARAMS)>
{
};

#undef PG_PP_FN_BUILD_I
#undef PG_PP_FN_BUILD

#include <guic/functions/iteration_foot.h>
