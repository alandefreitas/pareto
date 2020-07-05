// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef BOOST_RANGE_REVERSE_ITERATOR_HPP
#define BOOST_RANGE_REVERSE_ITERATOR_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include <pareto_front/boost_subset/range/config.hpp>
#include <pareto_front/boost_subset/range/iterator.hpp>
#include <pareto_front/boost_subset/type_traits/remove_reference.hpp>
#include <pareto_front/boost_subset/iterator/reverse_iterator.hpp>


namespace boost
{
    //////////////////////////////////////////////////////////////////////////
    // default
    //////////////////////////////////////////////////////////////////////////
    
    template< typename T >
    struct range_reverse_iterator
    {
        typedef reverse_iterator< 
            BOOST_DEDUCED_TYPENAME range_iterator<
                BOOST_DEDUCED_TYPENAME remove_reference<T>::type>::type > type;
    };
    

} // namespace boost


#endif
