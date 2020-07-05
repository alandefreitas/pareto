
#ifndef BOOST_MPL_AUX_ITER_FOLD_BACKWARD_IMPL_HPP_INCLUDED
#define BOOST_MPL_AUX_ITER_FOLD_BACKWARD_IMPL_HPP_INCLUDED

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

#if !defined(BOOST_MPL_PREPROCESSING_MODE)
#   include <pareto_front/boost_subset/mpl/next_prior.hpp>
#   include <pareto_front/boost_subset/mpl/apply.hpp>
#   include <pareto_front/boost_subset/mpl/aux_/config/ctps.hpp>
#   if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
    || defined(BOOST_MPL_CFG_NO_NONTYPE_TEMPLATE_PARTIAL_SPEC)
#       include <pareto_front/boost_subset/mpl/if.hpp>
#       include <pareto_front/boost_subset/type_traits/is_same.hpp>
#   endif
#endif

#include <pareto_front/boost_subset/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(BOOST_MPL_PREPROCESSING_MODE)

#   define BOOST_MPL_PREPROCESSED_HEADER reverse_iter_fold_impl.hpp
#   include <pareto_front/boost_subset/mpl/aux_/include_preprocessed.hpp>

#else

#   define AUX778076_FOLD_IMPL_OP(iter) iter
#   define AUX778076_FOLD_IMPL_NAME_PREFIX reverse_iter_fold
#   include <pareto_front/boost_subset/mpl/aux_/reverse_fold_impl_body.hpp>

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_AUX_ITER_FOLD_BACKWARD_IMPL_HPP_INCLUDED
