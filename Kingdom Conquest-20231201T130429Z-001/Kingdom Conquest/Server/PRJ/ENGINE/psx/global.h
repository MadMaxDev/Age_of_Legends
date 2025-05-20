// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-2

#ifndef __GLOBAL_H_G_SRC_SOURCE_PG_PSX__
#define __GLOBAL_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/s_engine.h>
#include <psx/scope.h>
#include <boost/preprocessor/seq.hpp>

__PG_PSX_SPACE_BEGIN__

class pg_psx_public global_base : public scope
{
protected:
    void inter_property_d(const wchar_t* decl, void* p);
    void inter_property_d(const wchar_t* decl, const void* p);
    void inter_def_d(const wchar_t* decl, void* func, uint_t callConv);
    void inter_behav_d(uint_t behav, const wchar_t* decl, void* ptr, uint_t callConv);
    void inter_const_int(const wchar_t* name, int val);
public:
    global_base(s_engine& ng)
        : scope(ng, wstring())
    {
    }
};

template<typename Builder>
class pg_psx_public global_ : public global_base
{
public:
    typedef Builder builder_t;
public:
    global_(s_engine& ng)
        : global_base(ng)
    {
    }

    global_& const_int(const wchar_t* name, int val)
    {
        inter_const_int(name, val);
        return *this;
    }

    // register property with declaration.
    global_& property_d(const wchar_t* decl, void* p)
    {
        inter_property_d(decl, p);
        return *this;
    }
    global_& property_d(const wchar_t* decl, const void* p)
    {
        inter_property_d(decl, p);
        return *this;
    }

    // register property with name,
    // generate declaration automaticly.
    template<typename T>
    global_& property(const wchar_t* name, T& p)
    {
        wstring::scoped_string_t buf;
        builder_t::build<T>(*m_engine, buf);
        buf.append(L" ");
        buf.append(name);
        return property_d(buf.c_str(), &p);
    }

    // register function with declaration.
    template<typename F>
    global_& def_d(const wchar_t* decl, F f)
    {
        inter_def_d(decl, (void*)f, pg::functions::calling_convention<F>::value);
        return *this;
    }
    template<typename F>
    global_& def_fn_d(const wchar_t* decl, const function<F>& f)
    {
        return def_d(decl, redirect_code(f));
    }

    // register function with name,
    // generate declaration automaticly.
    template<typename F>
    global_& def(const wchar_t* name, F f)
    {
        wstring::scoped_string_t buf;
        builder_t::build_fn<F>(*m_engine, buf, name);
        return def_d(buf.c_str(), f);
    }
    template<typename F>
    global_& def_fn(const wchar_t* name, const function<F>& f)
    {
        return def(name, redirect_code(f));
    }

    template<typename F>
    global_& behav_d(uint_t behav, const wchar_t* decl, F f)
    {
        inter_behav_d(behav, decl, (void*)f, pg::functions::calling_convention<F>::value);
        return *this;
    }

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_ADD
#   define PG_PSX_OP_NAME add
#   define PG_PSX_GLOBAL_OP +
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_SUBTRACT
#   define PG_PSX_OP_NAME sub
#   define PG_PSX_GLOBAL_OP -
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_MULTIPLY
#   define PG_PSX_OP_NAME mul
#   define PG_PSX_GLOBAL_OP *
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_DIVIDE
#   define PG_PSX_OP_NAME div
#   define PG_PSX_GLOBAL_OP /
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_MODULO
#   define PG_PSX_OP_NAME mod
#   define PG_PSX_GLOBAL_OP %
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_EQUAL
#   define PG_PSX_OP_NAME eq
#   define PG_PSX_GLOBAL_OP ==
#   include "detail/global_cmp_i.h"
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_NOTEQUAL
#   define PG_PSX_OP_NAME ne
#   define PG_PSX_GLOBAL_OP !=
#   include "detail/global_cmp_i.h"
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_LESSTHAN
#   define PG_PSX_OP_NAME lt
#   define PG_PSX_GLOBAL_OP <
#   include "detail/global_cmp_i.h"
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_GREATERTHAN
#   define PG_PSX_OP_NAME gt
#   define PG_PSX_GLOBAL_OP >
#   include "detail/global_cmp_i.h"
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_LEQUAL
#   define PG_PSX_OP_NAME le
#   define PG_PSX_GLOBAL_OP <=
#   include "detail/global_cmp_i.h"
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_GEQUAL
#   define PG_PSX_OP_NAME ge
#   define PG_PSX_GLOBAL_OP >=
#   include "detail/global_cmp_i.h"
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_LOGIC_OR
#   define PG_PSX_OP_NAME logic_or
#   define PG_PSX_GLOBAL_OP ||
#   include "detail/global_cmp_i.h"
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_LOGIC_AND
#   define PG_PSX_OP_NAME logic_and
#   define PG_PSX_GLOBAL_OP &&
#   include "detail/global_cmp_i.h"
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_BIT_OR
#   define PG_PSX_OP_NAME or
#   define PG_PSX_GLOBAL_OP |
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_BIT_AND
#   define PG_PSX_OP_NAME and
#   define PG_PSX_GLOBAL_OP &
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_BIT_XOR
#   define PG_PSX_OP_NAME xor
#   define PG_PSX_GLOBAL_OP ^
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_BIT_SLL
#   define PG_PSX_OP_NAME shl
#   define PG_PSX_GLOBAL_OP <<
#   include "detail/global_op_i.h"

#   define PG_PSX_BEHAVE_TYPE psBEHAVE_BIT_SRL
#   define PG_PSX_OP_NAME shr
#   define PG_PSX_GLOBAL_OP >>
#   include "detail/global_op_i.h"

};

inline global_<decl_text::builder> s_engine::global()
{
    return global_<decl_text::builder>(*this);
}
template<typename Builder>
inline global_<Builder> s_engine::global()
{
    return global_<Builder>(*this);
}


__PG_PSX_SPACE_END__

#endif  // __GLOBAL_H_G_SRC_SOURCE_PG_PSX__