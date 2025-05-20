// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-2

#ifndef __OBJECT_H_G_SRC_SOURCE_PG_PSX__
#define __OBJECT_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/s_engine.h>
#include <psx/scope.h>
#include <boost/preprocessor/seq.hpp>

__PG_PSX_SPACE_BEGIN__

class pg_psx_public object_base : public scope
{
protected:
    bool m_reg_ctor;
    bool m_reg_dtor;
    bool m_reg_assign;
    uint_t m_flag;

    void inter_property_d(const wchar_t* decl, size_t offset);
    void inter_def_d(const wchar_t* decl, const psUPtr& ptr, uint_t callConv);
    void inter_behav_d(uint_t behav, const wchar_t* decl, const psUPtr& ptr, uint_t callConv);
public:
    object_base(s_engine& ng, const std::type_info& ti, const std::type_info& bti, int off, size_t s, const wstring& n, uint_t flag);

    template<bool is_base, typename T, typename B>struct test_cast;

    template<typename T, typename B>
    struct test_cast<true, T, B>
    {
        static int get()
        {
            return (int)(size_t)static_cast<B*>((T*)0);
        }
    };
    template<typename T>
    struct test_cast<false, T, int>
    {
        static int get()
        {
            return 0x7fffffff;
        }
    };

};

#include <guic/push_new.h>

template<typename T, typename B = int, typename Builder = decl_text::builder>
class object_ : public object_base
{
public:
    typedef object_base base_t;
    typedef Builder builder_t;
protected:
public:
    object_(s_engine& ng, const wstring& n, uint_t flag = psOBJ_CLASS_CDA)
        : base_t(ng, typeid(T), typeid(B),
        test_cast<boost::is_base_of<B, T>::value, T, B>::get(),
        sizeof(T), n, flag)
    {
    }
    ~object_()
    {
        //pg_assert((m_flag&psOBJ_CLASS_CONSTRUCTOR) == 0 || m_reg_ctor);
        //pg_assert((m_flag&psOBJ_CLASS_DESTRUCTOR) == 0 || m_reg_dtor);
        //pg_assert((m_flag&psOBJ_CLASS_ASSIGNMENT) == 0 || m_reg_assign);
    }

    object_& property_d(const wchar_t* decl, size_t offset)
    {
        inter_property_d(decl, offset);
        return *this;
    }
    template<typename U>
    object_& property(const wchar_t* name, U T::* offset)
    {
        wstring::scoped_string_t buf;
        builder_t::build<U>(*m_engine, buf);
        buf.append(L" ");
        buf.append(name);
        return property_d(buf.c_str(), *(size_t*)&offset);
    }

    template<typename F>
    object_& def_d(const wchar_t* decl, F f)
    {
        // only for 'this' pointer check.
        builder_t::decl_mf<T, F> check;check;

        psUPtr fnPtr = psSMethodPtr<sizeof(f)>::Convert(f);
        inter_def_d(decl, fnPtr, pg::functions::calling_convention<F>::value);
        return *this;
    }
    template<typename F>
    object_& def_fn_d(const wchar_t* decl, const function<F>& f)
    {
        return def_d(decl, redirect_code(f));
    }
    template<typename F>
    object_& def(const wchar_t* name, F f)
    {
        wstring::scoped_string_t buf;
        builder_t::build_mf<T, F>(*m_engine, buf, name);
        return def_d(buf.c_str(), f);
    }
    template<typename F>
    object_& def_fn(const wchar_t* name, const function<F>& f)
    {
        return def(name, redirect_code(f));
    }

    // destructor
    template<typename F>
    object_& dtor_d(const wchar_t* decl, F f)
    {
        m_reg_dtor = true;
        psUPtr fnPtr = psSMethodPtr<sizeof(f)>::Convert(f);
        inter_behav_d(psBEHAVE_DESTRUCT, decl, fnPtr, pg::functions::calling_convention<F>::value);
        return *this;
    }
    object_& dtor()
    {
        return dtor_d(L"void f()", &destruct<T>);
    }

    // constructor
    template<typename F>
    object_& ctor_d(const wchar_t* decl, F f)
    {
        m_reg_ctor = true;
        psUPtr fnPtr = psSMethodPtr<sizeof(f)>::Convert(f);
        inter_behav_d(psBEHAVE_CONSTRUCT, decl, fnPtr, pg::functions::calling_convention<F>::value);
        return *this;
    }
    object_& ctor()
    {
        return ctor_d(L"void f()", &construct<T>);
    }
    template<typename F>
    object_& ctor(F f)
    {
        wstring::scoped_string_t buf;
        builder_t::build_mf<T, F>(*m_engine, buf, L"f");
        return ctor_d(buf.c_str(), f);
    }
    template<typename F>
    object_& ctor_fn(const function<F>& f)
    {
        return ctor(redirect_code(f));
    }
#   define BOOST_PP_ITERATION_PARAMS_1 (3, (1, PG_FUNCTIONS_PARAM_MAX, <psx/detail/ctor_i.h>))
#   include BOOST_PP_ITERATE()
    
    template<typename F>
    object_& op_index_d(const wchar_t* decl, F f)
    {
        psUPtr fnPtr = psSMethodPtr<sizeof(f)>::Convert(f);
        inter_behav_d(psBEHAVE_INDEX, decl, fnPtr, pg::functions::calling_convention<F>::value);
        return *this;
    }
    template<typename F>
    object_& op_index_d(const wchar_t* decl, const function<F>& f)
    {
        op_index_d(decl, redirect_code(f));
        return *this;
    }
    template<typename F>
    object_& op_index(F f)
    {
        wstring::scoped_string_t buf;
        builder_t::build_mf<T, F>(*m_engine, buf, L"f");
        return op_index_d(buf.c_str(), f);
    }
    template<typename F>
    object_& op_index_fn(const function<F>& f)
    {
        return op_index(redirect_code(f));
    }
    template<typename R, typename S>
    static R index_wrap(T& t, S s)
    {
        return t[s];
    }
    template<typename R, typename S>
    object_& op_index()
    {
        R (*f)(T&, S) = &index_wrap<R, S>;
        return op_index(f);
    }
    template<typename R, typename S>
    static R index_c_wrap(const T& t, S s)
    {
        return t[s];
    }
    template<typename R, typename S>
    object_& op_index_c()
    {
        R (*f)(const T&, S) = &index_c_wrap<S>;
        return op_index(f);
    }

    // assignment operators

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_ASSIGNMENT
#   define PG_PSX_ASSIGN_NAME assign
#   define PG_PSX_ASSIGN_OP =
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_ADD_ASSIGN
#   define PG_PSX_ASSIGN_NAME add_a
#   define PG_PSX_ASSIGN_OP +=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_SUB_ASSIGN
#   define PG_PSX_ASSIGN_NAME sub_a
#   define PG_PSX_ASSIGN_OP -=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_MUL_ASSIGN
#   define PG_PSX_ASSIGN_NAME mul_a
#   define PG_PSX_ASSIGN_OP *=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_DIV_ASSIGN
#   define PG_PSX_ASSIGN_NAME div_a
#   define PG_PSX_ASSIGN_OP /=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_MOD_ASSIGN
#   define PG_PSX_ASSIGN_NAME mod_a
#   define PG_PSX_ASSIGN_OP %=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_OR_ASSIGN
#   define PG_PSX_ASSIGN_NAME or_a
#   define PG_PSX_ASSIGN_OP |=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_AND_ASSIGN
#   define PG_PSX_ASSIGN_NAME and_a
#   define PG_PSX_ASSIGN_OP &=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_XOR_ASSIGN
#   define PG_PSX_ASSIGN_NAME xor_a
#   define PG_PSX_ASSIGN_OP ^=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_SLL_ASSIGN
#   define PG_PSX_ASSIGN_NAME shl_a
#   define PG_PSX_ASSIGN_OP <<=
#   include "detail/assign_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_SRL_ASSIGN
#   define PG_PSX_ASSIGN_NAME shr_a
#   define PG_PSX_ASSIGN_OP >>=
#   include "detail/assign_i.h"

};

#include <guic/pop_new.h>

template<typename T>
inline object_<T, int, decl_text::builder> s_engine::object(const wstring& name, uint_t flag)
{
    return object_<T, int, decl_text::builder>(*this, name, flag);
}
template<typename T, typename B>
inline object_<T, B, decl_text::builder> s_engine::object(const wstring& name, uint_t flag)
{
    return object_<T, B, decl_text::builder>(*this, name, flag);
}
#if 0
template<typename T, typename Builder>
inline object_<T, int, Builder> s_engine::object(const wstring& name, uint_t flag)
{
    return object_<T, int, Builder>(*this, name, flag);
}
template<typename T, typename B, typename Builder>
inline object_<T, B, Builder> s_engine::object(const wstring& name, uint_t flag)
{
    return object_<T, B, decl_text::builder>(*this, name, flag);
}
#endif

__PG_PSX_SPACE_END__

#endif  // __OBJECT_H_G_SRC_SOURCE_PG_PSX__