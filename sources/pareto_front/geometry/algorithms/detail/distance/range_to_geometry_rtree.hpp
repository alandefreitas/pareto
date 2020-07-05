// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_RANGE_TO_GEOMETRY_RTREE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_RANGE_TO_GEOMETRY_RTREE_HPP

#include <iterator>
#include <utility>

#include <pareto_front/geometry/core/assert.hpp>
#include <pareto_front/geometry/core/point_type.hpp>

#include <pareto_front/geometry/iterators/has_one_element.hpp>

#include <pareto_front/geometry/strategies/distance.hpp>

#include <pareto_front/geometry/algorithms/dispatch/distance.hpp>

#include <pareto_front/geometry/algorithms/detail/closest_feature/range_to_range.hpp>
#include <pareto_front/geometry/algorithms/detail/distance/is_comparable.hpp>
#include <pareto_front/geometry/algorithms/detail/distance/iterator_selector.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace distance
{


template
<
    typename PointOrSegmentIterator,
    typename Geometry,
    typename Strategy
>
class point_or_segment_range_to_geometry_rtree
{
private:
    typedef typename std::iterator_traits
        <
            PointOrSegmentIterator
        >::value_type point_or_segment_type;

    typedef iterator_selector<Geometry const> selector_type;

    typedef detail::closest_feature::range_to_range_rtree range_to_range;

public:
    typedef typename strategy::distance::services::return_type
        <
            Strategy,
            typename point_type<point_or_segment_type>::type,
            typename point_type<Geometry>::type
        >::type return_type;

    static inline return_type apply(PointOrSegmentIterator first,
                                    PointOrSegmentIterator last,
                                    Geometry const& geometry,
                                    Strategy const& strategy)
    {
        namespace sds = strategy::distance::services;

        BOOST_GEOMETRY_ASSERT( first != last );

        if ( geometry::has_one_element(first, last) )
        {
            return dispatch::distance
                <
                    point_or_segment_type, Geometry, Strategy
                >::apply(*first, geometry, strategy);
        }

        typename sds::return_type
            <
                typename sds::comparable_type<Strategy>::type,
                typename point_type<point_or_segment_type>::type,
                typename point_type<Geometry>::type
            >::type cd_min;

        std::pair
            <
                point_or_segment_type,
                typename selector_type::iterator_type
            > closest_features
            = range_to_range::apply(first,
                                    last,
                                    selector_type::begin(geometry),
                                    selector_type::end(geometry),
                                    sds::get_comparable
                                        <
                                            Strategy
                                        >::apply(strategy),
                                    cd_min);

        return
            is_comparable<Strategy>::value
            ?
            cd_min
            :
            dispatch::distance
                <
                    point_or_segment_type,                    
                    typename std::iterator_traits
                        <
                            typename selector_type::iterator_type
                        >::value_type,
                    Strategy
                >::apply(closest_features.first,
                         *closest_features.second,
                         strategy);
    }
};


}} // namespace detail::distance
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_RANGE_TO_GEOMETRY_RTREE_HPP
