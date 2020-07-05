
#ifndef BOOST_MPL_VECTOR_HPP_INCLUDED
#define BOOST_MPL_VECTOR_HPP_INCLUDED

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
#   include <pareto_front/boost_subset/mpl/limits/vector.hpp>
#   include <pareto_front/boost_subset/mpl/aux_/na.hpp>
#   include <pareto_front/boost_subset/mpl/aux_/config/preprocessor.hpp>

#   include <pareto_front/boost_subset/preprocessor/inc.hpp>
#   include <pareto_front/boost_subset/preprocessor/cat.hpp>
#   include <pareto_front/boost_subset/preprocessor/stringize.hpp>

#if !defined(BOOST_NEEDS_TOKEN_PASTING_OP_FOR_TOKENS_JUXTAPOSING)
#   define AUX778076_VECTOR_HEADER \
    BOOST_PP_CAT(vector, BOOST_MPL_LIMIT_VECTOR_SIZE).hpp \
    /**/
#else
#   define AUX778076_VECTOR_HEADER \
    BOOST_PP_CAT(vector, BOOST_MPL_LIMIT_VECTOR_SIZE)##.hpp \
    /**/
#endif

#   include BOOST_PP_STRINGIZE(pareto_front/boost_subset/mpl/vector/AUX778076_VECTOR_HEADER)
#   undef AUX778076_VECTOR_HEADER
#endif

#include <pareto_front/boost_subset/mpl/aux_/config/use_preprocessed.hpp>

#if !defined(BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS) \
    && !defined(BOOST_MPL_PREPROCESSING_MODE)

#   define BOOST_MPL_PREPROCESSED_HEADER vector.hpp
#   include <pareto_front/boost_subset/mpl/aux_/include_preprocessed.hpp>

#else

#   include <pareto_front/boost_subset/mpl/limits/vector.hpp>

#   define AUX778076_SEQUENCE_NAME vector
#   define AUX778076_SEQUENCE_LIMIT BOOST_MPL_LIMIT_VECTOR_SIZE
#   include <pareto_front/boost_subset/mpl/aux_/sequence_wrapper.hpp>

#endif // BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#endif // BOOST_MPL_VECTOR_HPP_INCLUDED
