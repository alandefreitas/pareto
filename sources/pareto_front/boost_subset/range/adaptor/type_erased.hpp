// Boost.Range library
//
//  Copyright Neil Groves 2010. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef BOOST_RANGE_ADAPTOR_TYPE_ERASED_HPP_INCLUDED
#define BOOST_RANGE_ADAPTOR_TYPE_ERASED_HPP_INCLUDED

#include <pareto_front/boost_subset/range/reference.hpp>
#include <pareto_front/boost_subset/range/value_type.hpp>
#include <pareto_front/boost_subset/range/iterator_range_core.hpp>
#include <pareto_front/boost_subset/range/any_range.hpp>
#include <pareto_front/boost_subset/range/concepts.hpp>

namespace boost
{
    namespace adaptors
    {
        template<
            class Value = use_default
          , class Traversal = use_default
          , class Reference = use_default
          , class Difference = use_default
          , class Buffer = use_default
        >
        struct type_erased
        {
        };

        template<
            class SinglePassRange
          , class Value
          , class Traversal
          , class Reference
          , class Difference
          , class Buffer
        >
        typename any_range_type_generator<
            SinglePassRange
          , Value
          , Traversal
          , Reference
          , Difference
          , Buffer
        >::type
        operator|(SinglePassRange& rng,
                  type_erased<
                        Value
                      , Traversal
                      , Reference
                      , Difference
                      , Buffer
                    >)
        {
            BOOST_RANGE_CONCEPT_ASSERT((
                SinglePassRangeConcept<SinglePassRange>));

            typedef typename any_range_type_generator<
                SinglePassRange
              , Value
              , Traversal
              , Reference
              , Difference
              , Buffer
            >::type range_type;
            return range_type(boost::begin(rng), boost::end(rng));
        }

        template<
            class SinglePassRange
          , class Value
          , class Traversal
          , class Reference
          , class Difference
          , class Buffer
        >
        typename any_range_type_generator<
            const SinglePassRange
          , Value
          , Traversal
          , Reference
          , Difference
          , Buffer
        >::type
        operator|(const SinglePassRange& rng,
                  type_erased<
                            Value
                          , Traversal
                          , Reference
                          , Difference
                          , Buffer
                    >)
        {
            BOOST_RANGE_CONCEPT_ASSERT((
                SinglePassRangeConcept<const SinglePassRange>));

            typedef typename any_range_type_generator<
                const SinglePassRange
              , Value
              , Traversal
              , Reference
              , Difference
              , Buffer
            >::type range_type;
            return range_type(boost::begin(rng), boost::end(rng));
        }

        template<
            class SinglePassRange
          , class Value
          , class Traversal
          , class Reference
          , class Difference
          , class Buffer
        >
        typename any_range_type_generator<
            SinglePassRange
          , Value
          , Traversal
          , Reference
          , Difference
          , Buffer
        >::type
        type_erase(SinglePassRange& rng
                 , type_erased<
                            Value
                          , Traversal
                          , Reference
                          , Difference
                          , Buffer
                    > = type_erased<>()
                )
        {
            BOOST_RANGE_CONCEPT_ASSERT((
                SinglePassRangeConcept<SinglePassRange>));

            typedef typename any_range_type_generator<
                SinglePassRange
              , Value
              , Traversal
              , Reference
              , Difference
              , Buffer
            >::type range_type;

            return range_type(boost::begin(rng), boost::end(rng));
        }

        template<
            class SinglePassRange
          , class Value
          , class Traversal
          , class Reference
          , class Difference
          , class Buffer
        >
        typename any_range_type_generator<
            const SinglePassRange
          , Value
          , Traversal
          , Reference
          , Difference
          , Buffer
        >::type
        type_erase(const SinglePassRange& rng
                 , type_erased<
                            Value
                          , Traversal
                          , Reference
                          , Difference
                          , Buffer
                    > = type_erased<>()
                )
        {
            BOOST_RANGE_CONCEPT_ASSERT((
                SinglePassRangeConcept<const SinglePassRange>));

            typedef typename any_range_type_generator<
                const SinglePassRange
              , Value
              , Traversal
              , Reference
              , Difference
              , Buffer
            >::type range_type;

            return range_type(boost::begin(rng), boost::end(rng));
        }
    }
} // namespace boost

#endif // include guard
