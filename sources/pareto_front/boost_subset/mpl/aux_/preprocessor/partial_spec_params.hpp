
#ifndef BOOST_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED
#define BOOST_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED

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

#include <pareto_front/boost_subset/mpl/limits/arity.hpp>
#include <pareto_front/boost_subset/mpl/aux_/preprocessor/params.hpp>
#include <pareto_front/boost_subset/mpl/aux_/preprocessor/enum.hpp>
#include <pareto_front/boost_subset/mpl/aux_/preprocessor/sub.hpp>
#include <pareto_front/boost_subset/preprocessor/comma_if.hpp>

#define BOOST_MPL_PP_PARTIAL_SPEC_PARAMS(n, param, def) \
BOOST_MPL_PP_PARAMS(n, param) \
BOOST_PP_COMMA_IF(BOOST_MPL_PP_SUB(BOOST_MPL_LIMIT_METAFUNCTION_ARITY,n)) \
BOOST_MPL_PP_ENUM( \
      BOOST_MPL_PP_SUB(BOOST_MPL_LIMIT_METAFUNCTION_ARITY,n) \
    , def \
    ) \
/**/

#endif // BOOST_MPL_AUX_PREPROCESSOR_PARTIAL_SPEC_PARAMS_HPP_INCLUDED
