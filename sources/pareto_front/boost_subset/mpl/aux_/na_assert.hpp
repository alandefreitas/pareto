
#ifndef BOOST_MPL_AUX_NA_ASSERT_HPP_INCLUDED
#define BOOST_MPL_AUX_NA_ASSERT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <pareto_front/boost_subset/mpl/aux_/na.hpp>
#include <pareto_front/boost_subset/mpl/aux_/config/msvc.hpp>
#include <pareto_front/boost_subset/mpl/aux_/config/workaround.hpp>

#if !BOOST_WORKAROUND(_MSC_FULL_VER, <= 140050601)    \
    && !BOOST_WORKAROUND(__EDG_VERSION__, <= 243)
#   include <pareto_front/boost_subset/mpl/assert.hpp>
#   define BOOST_MPL_AUX_ASSERT_NOT_NA(x) \
    BOOST_MPL_ASSERT_NOT((boost::mpl::is_na<type>)) \
/**/
#else
#   include <pareto_front/boost_subset/static_assert.hpp>
#   define BOOST_MPL_AUX_ASSERT_NOT_NA(x) \
    BOOST_STATIC_ASSERT(!boost::mpl::is_na<x>::value) \
/**/
#endif

#endif // BOOST_MPL_AUX_NA_ASSERT_HPP_INCLUDED
