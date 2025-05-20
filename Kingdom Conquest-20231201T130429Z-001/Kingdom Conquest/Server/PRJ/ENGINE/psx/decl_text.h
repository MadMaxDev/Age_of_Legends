// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-2

#ifndef __DECL_TEXT_H_G_SRC_SOURCE_PG_PSX__
#define __DECL_TEXT_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/config.h>
#include <guic/mpl/is_base_of.h>
#include <guic/mpl/is_class.h>
#include <guic/mpl/is_pod.h>
#include <guic/functions/calling_convention.h>

__PG_PSX_SPACE_BEGIN__

class s_engine;

namespace decl_text
{
    class no_map
    {
    };
    template<typename T>
    class map : public no_map
    {
    };

    template<typename T>
    class is_mapped
    {
    public:
        static const bool value = !mpl::is_base_of< no_map, map<T> >::value;
    };


#define PG_PSX_DECL_MAP_PRIM(t, d) \
    template<>\
    class pg_psx_public map<t>\
    {\
    public:\
        static const wstring decl;\
    };
#define PG_PSX_DECL_MAPI_PRIM(t, d) const wstring map<t>::decl(d);

#define PG_PSX_DECL_MAP_CONST(t, d) \
    template<>\
    class pg_psx_public map<const t>\
    {\
    public:\
        static const wstring decl;\
    };
#define PG_PSX_DECL_MAPI_CONST(t, d) const wstring map<const t>::decl(L"const " d);

#define PG_PSX_DECL_MAP_REF(t, d) \
    template<>\
    class pg_psx_public map<t&>\
    {\
    public:\
        static const wstring decl;\
    };
#define PG_PSX_DECL_MAPI_REF(t, d) const wstring map<t&>::decl(d L"&");

#define PG_PSX_DECL_MAP_CREF(t, d) \
    template<>\
    class pg_psx_public map<const t&>\
    {\
    public:\
        static const wstring decl;\
    };
#define PG_PSX_DECL_MAPI_CREF(t, d) const wstring map<const t&>::decl(L"const " d L"&");

#define PG_PSX_DECL_MAP_POINTER(t, d) \
    template<>\
    class pg_psx_public map<t*>\
    {\
    public:\
        static const wstring decl;\
    };
#define PG_PSX_DECL_MAPI_POINTER(t, d) const wstring map<t*>::decl(d L"*");

#define PG_PSX_DECL_MAP_CPOINTER(t, d) \
    template<>\
    class pg_psx_public map<const t*>\
    {\
    public:\
        static const wstring decl;\
    };
#define PG_PSX_DECL_MAPI_CPOINTER(t, d) const wstring map<const t*>::decl(L"const " d L"*");

#define PG_PSX_DECL_MAP(t, d) \
    PG_PSX_DECL_MAP_PRIM(t, d)\
    PG_PSX_DECL_MAP_CONST(t, d)\
    PG_PSX_DECL_MAP_REF(t, d)\
    PG_PSX_DECL_MAP_CREF(t, d)\
    PG_PSX_DECL_MAP_POINTER(t, d)\
    PG_PSX_DECL_MAP_CPOINTER(t, d)

#define PG_PSX_DECL_MAPI(t, d) \
    PG_PSX_DECL_MAPI_PRIM(t, d)\
    PG_PSX_DECL_MAPI_CONST(t, d)\
    PG_PSX_DECL_MAPI_REF(t, d)\
    PG_PSX_DECL_MAPI_CREF(t, d)\
    PG_PSX_DECL_MAPI_POINTER(t, d)\
    PG_PSX_DECL_MAPI_CPOINTER(t, d)

#   define PG_PSX_DECL_DEF(t, d) PG_PSX_DECL_MAP(t, d)
#   define PG_PSX_DECL_PRIM_DEF(t, d) PG_PSX_DECL_MAP_PRIM(t, d)
#   include "detail/decl_def.h"

    class pg_psx_public builder
    {
    protected:
        const static wstring& find(s_engine& ng, const std::type_info& i);

        class build_by_map
        {
        public:
            template<typename T, typename Buf>
            static void build(s_engine& , Buf& buf)
            {
                buf.append(map<T>::decl);
            }
        };
        
        class build_by_engine
        {
        public:
            template<typename T>
            struct tool
            {
                template<typename Buf>
                static void build(s_engine& ng, Buf& buf)
                {
                    buf.append(find(ng, typeid(T)));
                }
            };
            template<typename T>
            struct tool<const T>
            {
                template<typename Buf>
                static void build(s_engine& ng, Buf& buf)
                {
                    buf.append(L"const ");
                    buf.append(find(ng, typeid(T)));
                }
            };
            template<typename T>
            struct tool<T&>
            {
                template<typename Buf>
                static void build(s_engine& ng, Buf& buf)
                {
                    buf.append(find(ng, typeid(T)));
                    buf.append(L"&");
                }
            };
            template<typename T>
            struct tool<const T&>
            {
                template<typename Buf>
                static void build(s_engine& ng, Buf& buf)
                {
                    buf.append(L"const ");
                    buf.append(find(ng, typeid(T)));
                    buf.append(L"&");
                }
            };
            template<typename T>
            struct tool<T*>
            {
                template<typename Buf>
                static void build(s_engine& ng, Buf& buf)
                {
                    buf.append(find(ng, typeid(T)));
                    buf.append(L"*");
                }
            };
            template<typename T>
            struct tool<const T*>
            {
                template<typename Buf>
                static void build(s_engine& ng, Buf& buf)
                {
                    buf.append(L"const ");
                    buf.append(find(ng, typeid(T)));
                    buf.append(L"*");
                }
            };
        public:
            template<typename T, typename Buf>
            static void build(s_engine& ng, Buf& buf)
            {
                tool<T>::build(ng, buf);
            }
        };
    public:
        template<typename T, typename Buf>
        static void build(s_engine& ng, Buf& buf)
        {
            typedef typename mpl::if_c<
                is_mapped<T>::value,
                build_by_map,
                build_by_engine
            >::type build_t;
            
            build_t::build<T>(ng, buf);
        }

        template<typename F>
        class decl_fn_mf;
        template<typename F>
        class decl_fn;
        template<typename C, typename F>
        class decl_mf;

#       define BOOST_PP_ITERATION_PARAMS_1 (3, (0, PG_FUNCTIONS_PARAM_MAX, <psx/detail/decl_fn_mf_i.h>))
#       include BOOST_PP_ITERATE()

        template<typename F, typename Buf>
        static void build_fn(s_engine& ng, Buf& buf, const wchar_t* name)
        {
            decl_fn<F>::build(ng, buf, name);
        }
        template<typename C, typename F, typename Buf>
        static void build_mf(s_engine& ng, Buf& buf, const wchar_t* name)
        {
            decl_mf<C, F>::build(ng, buf, name);
        }
    };

};

__PG_PSX_SPACE_END__

#endif  // __DECL_TEXT_H_G_SRC_SOURCE_PG_PSX__