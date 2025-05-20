// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-22

template<typename T, typename S>
global_& BOOST_PP_CAT(op_, PG_PSX_OP_NAME)()
{
    bool (*f)(T, S) = &BOOST_PP_CAT(PG_PSX_OP_NAME, _wrap)<bool, T, S>;
    return BOOST_PP_CAT(op_, PG_PSX_OP_NAME)(f);
}
