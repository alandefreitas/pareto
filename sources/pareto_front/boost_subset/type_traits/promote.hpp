// Copyright 2005 Alexander Nasonov.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef FILE_boost_type_traits_promote_hpp_INCLUDED
#define FILE_boost_type_traits_promote_hpp_INCLUDED

#include <pareto_front/boost_subset/config.hpp>
#include <pareto_front/boost_subset/type_traits/integral_promotion.hpp>
#include <pareto_front/boost_subset/type_traits/floating_point_promotion.hpp>

namespace boost {

template<class T> struct promote : public integral_promotion<typename floating_point_promotion<T>::type>{};

#if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

   template <class T> using promote_t = typename promote<T>::type;

#endif

}

#endif // #ifndef FILE_boost_type_traits_promote_hpp_INCLUDED

