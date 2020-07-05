// Boost.Range library
//
//  Copyright Neil Groves 2009. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef BOOST_RANGE_ALGORITHM_EXT_ERASE_HPP_INCLUDED
#define BOOST_RANGE_ALGORITHM_EXT_ERASE_HPP_INCLUDED

#include <pareto_front/boost_subset/range/config.hpp>
#include <pareto_front/boost_subset/range/concepts.hpp>
#include <pareto_front/boost_subset/range/difference_type.hpp>
#include <pareto_front/boost_subset/range/iterator_range_core.hpp>
#include <pareto_front/boost_subset/range/begin.hpp>
#include <pareto_front/boost_subset/range/end.hpp>
#include <pareto_front/boost_subset/assert.hpp>

namespace boost
{
    namespace range
    {

template< class Container >
inline Container& erase( Container& on,
      iterator_range<BOOST_DEDUCED_TYPENAME Container::iterator> to_erase )
{
    BOOST_RANGE_CONCEPT_ASSERT(( ForwardRangeConcept<Container> ));
    on.erase( boost::begin(to_erase), boost::end(to_erase) );
    return on;
}

template< class Container, class T >
inline Container& remove_erase( Container& on, const T& val )
{
    BOOST_RANGE_CONCEPT_ASSERT(( ForwardRangeConcept<Container> ));
    on.erase(
        std::remove(boost::begin(on), boost::end(on), val),
        boost::end(on));
    return on;
}

template< class Container, class Pred >
inline Container& remove_erase_if( Container& on, Pred pred )
{
    BOOST_RANGE_CONCEPT_ASSERT(( ForwardRangeConcept<Container> ));
    on.erase(
        std::remove_if(boost::begin(on), boost::end(on), pred),
        boost::end(on));
    return on;
}

    } // namespace range
    using range::erase;
    using range::remove_erase;
    using range::remove_erase_if;
} // namespace boost

#endif // include guard
