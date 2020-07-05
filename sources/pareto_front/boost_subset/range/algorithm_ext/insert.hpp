// Boost.Range library
//
//  Copyright Neil Groves 2009. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef BOOST_RANGE_ALGORITHM_EXT_INSERT_HPP_INCLUDED
#define BOOST_RANGE_ALGORITHM_EXT_INSERT_HPP_INCLUDED

#include <pareto_front/boost_subset/range/config.hpp>
#include <pareto_front/boost_subset/range/concepts.hpp>
#include <pareto_front/boost_subset/range/difference_type.hpp>
#include <pareto_front/boost_subset/range/begin.hpp>
#include <pareto_front/boost_subset/range/end.hpp>
#include <pareto_front/boost_subset/assert.hpp>

namespace boost
{
    namespace range
    {

template< class Container, class Range >
inline Container& insert( Container& on,
                          BOOST_DEDUCED_TYPENAME Container::iterator before,
                          const Range& from )
{
    BOOST_RANGE_CONCEPT_ASSERT(( ForwardRangeConcept<Container> ));
    BOOST_RANGE_CONCEPT_ASSERT(( SinglePassRangeConcept<Range> ));
    on.insert( before, boost::begin(from), boost::end(from) );
    return on;
}

template< class Container, class Range >
inline Container& insert( Container& on, const Range& from )
{
    BOOST_RANGE_CONCEPT_ASSERT(( ForwardRangeConcept<Container> ));
    BOOST_RANGE_CONCEPT_ASSERT(( SinglePassRangeConcept<Range> ));
    on.insert(boost::begin(from), boost::end(from));
    return on;
}

    } // namespace range
    using range::insert;
} // namespace boost

#endif // include guard
