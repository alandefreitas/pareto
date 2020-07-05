
#ifndef BOOST_MPL_ADVANCE_HPP_INCLUDED
#define BOOST_MPL_ADVANCE_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <pareto_front/boost_subset/mpl/advance_fwd.hpp>
#include <pareto_front/boost_subset/mpl/less.hpp>
#include <pareto_front/boost_subset/mpl/negate.hpp>
#include <pareto_front/boost_subset/mpl/long.hpp>
#include <pareto_front/boost_subset/mpl/if.hpp>
#include <pareto_front/boost_subset/mpl/tag.hpp>
#include <pareto_front/boost_subset/mpl/apply_wrap.hpp>
#include <pareto_front/boost_subset/mpl/aux_/advance_forward.hpp>
#include <pareto_front/boost_subset/mpl/aux_/advance_backward.hpp>
#include <pareto_front/boost_subset/mpl/aux_/value_wknd.hpp>
#include <pareto_front/boost_subset/mpl/aux_/na_spec.hpp>
#include <pareto_front/boost_subset/mpl/aux_/nttp_decl.hpp>

namespace boost { namespace mpl {

// default implementation for forward/bidirectional iterators
template< typename Tag >
struct advance_impl
{
    template< typename Iterator, typename N > struct apply
    {
        typedef typename less< N,long_<0> >::type backward_;
        typedef typename if_< backward_, negate<N>, N >::type offset_;

        typedef typename if_<
              backward_
            , aux::advance_backward< BOOST_MPL_AUX_VALUE_WKND(offset_)::value >
            , aux::advance_forward< BOOST_MPL_AUX_VALUE_WKND(offset_)::value >
            >::type f_;

        typedef typename apply_wrap1<f_,Iterator>::type type;
    };
};


template<
      typename BOOST_MPL_AUX_NA_PARAM(Iterator)
    , typename BOOST_MPL_AUX_NA_PARAM(N)
    >
struct advance
    : advance_impl< typename tag<Iterator>::type >
        ::template apply<Iterator,N>
{
};

template<
      typename Iterator
    , BOOST_MPL_AUX_NTTP_DECL(long, N)
    >
struct advance_c
    : advance_impl< typename tag<Iterator>::type >
        ::template apply<Iterator,long_<N> >
{
};

BOOST_MPL_AUX_NA_SPEC(2, advance)

}}

#endif // BOOST_MPL_ADVANCE_HPP_INCLUDED
