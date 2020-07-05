
#ifndef BOOST_MPL_AUX_RANGE_C_SIZE_HPP_INCLUDED
#define BOOST_MPL_AUX_RANGE_C_SIZE_HPP_INCLUDED

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

#include <pareto_front/boost_subset/mpl/size_fwd.hpp>
#include <pareto_front/boost_subset/mpl/minus.hpp>
#include <pareto_front/boost_subset/mpl/aux_/range_c/tag.hpp>

namespace boost { namespace mpl {

template<>
struct size_impl< aux::half_open_range_tag >
{
    template< typename Range > struct apply
        : minus<
              typename Range::finish
            , typename Range::start
            >
    {
    };
};

}}

#endif // BOOST_MPL_AUX_RANGE_C_SIZE_HPP_INCLUDED
