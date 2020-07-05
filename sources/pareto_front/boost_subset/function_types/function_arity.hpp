
// (C) Copyright Tobias Schwinger
//
// Use modification and distribution are subject to the boost Software License,
// Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).

//------------------------------------------------------------------------------

#ifndef BOOST_FT_FUNCTION_ARITY_HPP_INCLUDED
#define BOOST_FT_FUNCTION_ARITY_HPP_INCLUDED

#include <pareto_front/boost_subset/blank.hpp>
#include <pareto_front/boost_subset/mpl/if.hpp>

#include <pareto_front/boost_subset/mpl/aux_/lambda_support.hpp>

#include <pareto_front/boost_subset/function_types/is_callable_builtin.hpp>
#include <pareto_front/boost_subset/function_types/components.hpp>

namespace boost 
{ 
  namespace function_types 
  {
    template<typename T> struct function_arity
      : mpl::if_
        < function_types::is_callable_builtin<T>
        , typename components<T>::function_arity, boost::blank
        >::type
    {
      BOOST_MPL_AUX_LAMBDA_SUPPORT(1,function_arity,(T)) 
    };
  }
}

#endif

