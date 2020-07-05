
#ifndef BOOST_MPL_IS_SEQUENCE_HPP_INCLUDED
#define BOOST_MPL_IS_SEQUENCE_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <pareto_front/boost_subset/mpl/not.hpp>
#include <pareto_front/boost_subset/mpl/and.hpp>
#include <pareto_front/boost_subset/mpl/begin_end.hpp>
#include <pareto_front/boost_subset/mpl/if.hpp>
#include <pareto_front/boost_subset/mpl/bool.hpp>
#include <pareto_front/boost_subset/mpl/sequence_tag_fwd.hpp>
#include <pareto_front/boost_subset/mpl/identity.hpp>
#include <pareto_front/boost_subset/mpl/void.hpp>
#include <pareto_front/boost_subset/mpl/aux_/has_tag.hpp>
#include <pareto_front/boost_subset/mpl/aux_/has_begin.hpp>
#include <pareto_front/boost_subset/mpl/aux_/na_spec.hpp>
#include <pareto_front/boost_subset/mpl/aux_/lambda_support.hpp>
#include <pareto_front/boost_subset/mpl/aux_/config/eti.hpp>
#include <pareto_front/boost_subset/mpl/aux_/config/msvc.hpp>
#include <pareto_front/boost_subset/mpl/aux_/config/workaround.hpp>
#if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
#   include <pareto_front/boost_subset/mpl/aux_/msvc_is_class.hpp>
#elif BOOST_WORKAROUND(BOOST_MSVC, == 1300)
#   include <pareto_front/boost_subset/type_traits/is_class.hpp>
#endif

#include <pareto_front/boost_subset/type_traits/is_same.hpp>

namespace boost { namespace mpl {

#if BOOST_WORKAROUND(BOOST_MSVC, <= 1300)

namespace aux {

// agurt, 11/jun/03: 
// MSVC 6.5/7.0 fails if 'has_begin' is instantiated on a class type that has a
// 'begin' member that doesn't name a type; e.g. 'has_begin< std::vector<int> >'
// would fail; requiring 'T' to have _both_ 'tag' and 'begin' members workarounds
// the issue for most real-world cases
template< typename T > struct is_sequence_impl
    : and_<
          identity< aux::has_tag<T> >
        , identity< aux::has_begin<T> >
        >
{
};

} // namespace aux
        
template<
      typename BOOST_MPL_AUX_NA_PARAM(T)
    >
struct is_sequence
    : if_<
#if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
          aux::msvc_is_class<T> 
#else
          boost::is_class<T> 
#endif
        , aux::is_sequence_impl<T>
        , bool_<false>
        >::type
{
    BOOST_MPL_AUX_LAMBDA_SUPPORT(1, is_sequence, (T))
};

#elif defined(BOOST_MPL_CFG_NO_HAS_XXX)

template<
      typename BOOST_MPL_AUX_NA_PARAM(T)
    >
struct is_sequence
    : bool_<false>
{
};

#else

template<
      typename BOOST_MPL_AUX_NA_PARAM(T)
    >
struct is_sequence
    : not_< is_same< typename begin<T>::type, void_ > >
{
    BOOST_MPL_AUX_LAMBDA_SUPPORT(1, is_sequence, (T))
};

#endif // BOOST_MSVC

#if defined(BOOST_MPL_CFG_MSVC_60_ETI_BUG)
template<> struct is_sequence<int>
    : bool_<false>
{
};
#endif

BOOST_MPL_AUX_NA_SPEC_NO_ETI(1, is_sequence)

}}

#endif // BOOST_MPL_IS_SEQUENCE_HPP_INCLUDED
