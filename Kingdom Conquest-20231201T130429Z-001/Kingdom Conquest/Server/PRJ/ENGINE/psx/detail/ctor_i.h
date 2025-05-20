// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-5

#include <guic/functions/iteration_head.h>


template<PG_PP_FN_TEMPLATE_PARAMS>
static void BOOST_PP_CAT(ctor_wrapper_, BOOST_PP_ITERATION())(T* p PG_PP_FN_COMMA PG_PP_FN_PARAMS)
{
    ::new(p) T(PG_PP_FN_ARGS);
}
template<PG_PP_FN_TEMPLATE_PARAMS>
object_& ctor()
{
    void(*f)(T* PG_PP_FN_COMMA PG_PP_FN_TEMPLATE_ARGS) =
        &BOOST_PP_CAT(ctor_wrapper_, BOOST_PP_ITERATION())<PG_PP_FN_TEMPLATE_ARGS>;
    return ctor(f);
}


#include <guic/functions/iteration_foot.h>
