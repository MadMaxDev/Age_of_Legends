// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-5

template<typename F>
object_& BOOST_PP_SEQ_CAT((op_)(PG_PSX_ASSIGN_NAME)(_d))(const wchar_t* decl, F f)
{
    psUPtr fnPtr = psSMethodPtr<sizeof(f)>::Convert(f);
    inter_behav_d(PG_PSX_BEHAVE_TYPE, decl, fnPtr, pg::functions::calling_convention<F>::value);
    return *this;
}
template<typename F>
object_& BOOST_PP_SEQ_CAT((opfn_)(PG_PSX_ASSIGN_NAME)(_d))(const wchar_t* decl, const function<F>& f)
{
    BOOST_PP_SEQ_CAT((op_)(PG_PSX_ASSIGN_NAME)(_d))(decl, redirect_code(f));
    return *this;
}
template<typename F>
object_& BOOST_PP_CAT(op_, PG_PSX_ASSIGN_NAME)(F f)
{
    wstring::scoped_string_t buf;
    builder_t::build_mf<T, F>(*m_engine, buf, L"f");
    return BOOST_PP_SEQ_CAT((op_)(PG_PSX_ASSIGN_NAME)(_d))(buf.c_str(), f);
}
template<typename F>
object_& BOOST_PP_CAT(opfn_, PG_PSX_ASSIGN_NAME)(const function<F>& f)
{
    return BOOST_PP_CAT(op_, PG_PSX_ASSIGN_NAME)(redirect_code(f));
}
template<typename S>
static T& BOOST_PP_CAT(PG_PSX_ASSIGN_NAME, _wrap)(T& t, S s)
{
    t PG_PSX_ASSIGN_OP s;
    return t;
}
template<typename S>
object_& BOOST_PP_CAT(op_, PG_PSX_ASSIGN_NAME)()
{
    T& (*f)(T&, S) = &BOOST_PP_CAT(PG_PSX_ASSIGN_NAME, _wrap)<S>;
    return BOOST_PP_CAT(op_, PG_PSX_ASSIGN_NAME)(f);
}
object_& BOOST_PP_CAT(op_, PG_PSX_ASSIGN_NAME)()
{
    return BOOST_PP_CAT(op_, PG_PSX_ASSIGN_NAME)<const T&>();
}

#undef PG_PSX_BEHAVE_TYPE
#undef PG_PSX_ASSIGN_NAME
#undef PG_PSX_ASSIGN_OP