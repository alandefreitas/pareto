
#ifndef BOOST_MPL_COUNT_IF_HPP_INCLUDED
#define BOOST_MPL_COUNT_IF_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2002
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <pareto_front/boost_subset/mpl/fold.hpp>
#include <pareto_front/boost_subset/mpl/next.hpp>
#include <pareto_front/boost_subset/mpl/integral_c.hpp>
#include <pareto_front/boost_subset/mpl/identity.hpp>
#include <pareto_front/boost_subset/mpl/eval_if.hpp>
#include <pareto_front/boost_subset/mpl/apply.hpp>
#include <pareto_front/boost_subset/mpl/aux_/msvc_eti_base.hpp>
#include <pareto_front/boost_subset/mpl/aux_/na_spec.hpp>
#include <pareto_front/boost_subset/mpl/aux_/lambda_support.hpp>
#include <pareto_front/boost_subset/mpl/aux_/config/forwarding.hpp>

namespace boost { namespace mpl {

namespace aux {

template< typename Predicate >
struct next_if
{
    template<
          typename N
        , typename T
        >
    struct apply
#if !defined(BOOST_MPL_CFG_NO_NESTED_FORWARDING)
        : eval_if<
              typename apply1<Predicate,T>::type
            , next<N>
            , identity<N>
            >
    {
#else
    {
        typedef typename eval_if<
              typename apply1<Predicate,T>::type
            , next<N>
            , identity<N>
            >::type type;
#endif
    };
};

} // namespace aux


template<
      typename BOOST_MPL_AUX_NA_PARAM(Sequence)
    , typename BOOST_MPL_AUX_NA_PARAM(Predicate)
    >
struct count_if
    : aux::msvc_eti_base< typename fold<
          Sequence
        , integral_c<unsigned long,0>
        , protect< aux::next_if<Predicate> >
        >::type >
{
    BOOST_MPL_AUX_LAMBDA_SUPPORT(2,count_if,(Sequence,Predicate))
};

BOOST_MPL_AUX_NA_SPEC(2, count_if)

}}

#endif // BOOST_MPL_COUNT_IF_HPP_INCLUDED
