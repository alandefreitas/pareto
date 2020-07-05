//  Copyright Neil Groves 2009. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef BOOST_RANGE_ALGORITHM_COPY_N_HPP_INCLUDED
#define BOOST_RANGE_ALGORITHM_COPY_N_HPP_INCLUDED

#include <pareto_front/boost_subset/assert.hpp>
#include <pareto_front/boost_subset/concept_check.hpp>
#include <pareto_front/boost_subset/range/begin.hpp>
#include <pareto_front/boost_subset/range/end.hpp>
#include <pareto_front/boost_subset/range/concepts.hpp>
#include <pareto_front/boost_subset/range/distance.hpp>
#include <pareto_front/boost_subset/range/iterator.hpp>
#include <pareto_front/boost_subset/range/iterator_range.hpp>
#include <algorithm>

namespace boost
{
    namespace range
    {

/// \brief template function copy
///
/// range-based version of the copy std algorithm
///
/// \pre SinglePassRange is a model of the SinglePassRangeConcept
/// \pre OutputIterator is a model of the OutputIteratorConcept
/// \pre 0 <= n <= distance(rng)
template< class SinglePassRange, class Size, class OutputIterator >
inline OutputIterator copy_n(const SinglePassRange& rng, Size n, OutputIterator out)
{
    BOOST_RANGE_CONCEPT_ASSERT(( SinglePassRangeConcept<const SinglePassRange> ));
    BOOST_ASSERT( n <= static_cast<Size>(::boost::distance(rng)) );
    BOOST_ASSERT( n >= static_cast<Size>(0) );

    BOOST_DEDUCED_TYPENAME range_iterator<const SinglePassRange>::type source = ::boost::begin(rng);

    for (Size i = 0; i < n; ++i, ++out, ++source)
        *out = *source;

    return out;
}

    } // namespace range
    using ::boost::range::copy_n;
} // namespace boost

#endif // include guard
