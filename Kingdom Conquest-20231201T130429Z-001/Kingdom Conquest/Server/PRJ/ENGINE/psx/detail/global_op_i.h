// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-5

template<typename F>
global_& BOOST_PP_SEQ_CAT((op_)(PG_PSX_OP_NAME)(_d))(const wchar_t* decl, F f)
{
    return behav_d(PG_PSX_BEHAVE_TYPE, decl, f);
}
template<typename F>
global_& BOOST_PP_SEQ_CAT((opfn_)(PG_PSX_OP_NAME)(_d))(const wchar_t* decl, const function<F>& f)
{
    BOOST_PP_SEQ_CAT((op_)(PG_PSX_OP_NAME)(_d))(decl, redirect_code(f));
    return *this;
}
template<typename F>
global_& BOOST_PP_CAT(op_, PG_PSX_OP_NAME)(F f)
{
    wstring::scoped_string_t buf;
    builder_t::build_fn<F>(*m_engine, buf, L"f");
    return BOOST_PP_SEQ_CAT((op_)(PG_PSX_OP_NAME)(_d))(buf.c_str(), f);
}
template<typename F>
global_& BOOST_PP_CAT(opfn_, PG_PSX_OP_NAME)(const function<F>& f)
{
    return BOOST_PP_CAT(op_, PG_PSX_OP_NAME)(redirect_code(f));
}
template<typename R, typename T, typename S>
static R BOOST_PP_CAT(PG_PSX_OP_NAME, _wrap)(T t, S s)
{
    return t PG_PSX_GLOBAL_OP s;
}
template<typename R, typename T, typename S>
global_& BOOST_PP_CAT(op_, PG_PSX_OP_NAME)()
{
    R (*f)(T, S) = &BOOST_PP_CAT(PG_PSX_OP_NAME, _wrap)<R, T, S>;
    return BOOST_PP_CAT(op_, PG_PSX_OP_NAME)(f);
}

#undef PG_PSX_BEHAVE_TYPE
#undef PG_PSX_OP_NAME
#undef PG_PSX_GLOBAL_OP