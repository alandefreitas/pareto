// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2016, 2017.
// Modifications copyright (c) 2016-2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_INTERSECTION_HPP


#include <pareto_front/geometry/core/point_type.hpp>
#include <pareto_front/geometry/geometries/segment.hpp>

#include <pareto_front/geometry/policies/relate/intersection_points.hpp>
#include <pareto_front/geometry/policies/relate/direction.hpp>
#include <pareto_front/geometry/policies/relate/tupled.hpp>

#include <pareto_front/geometry/strategies/intersection.hpp>
#include <pareto_front/geometry/strategies/intersection_result.hpp>
#include <pareto_front/geometry/strategies/side.hpp>

#include <pareto_front/geometry/strategies/cartesian/intersection.hpp>
#include <pareto_front/geometry/strategies/cartesian/side_by_triangle.hpp>
#include <pareto_front/geometry/strategies/spherical/intersection.hpp>
#include <pareto_front/geometry/strategies/spherical/ssf.hpp>

#include <pareto_front/geometry/policies/robustness/segment_ratio_type.hpp>


namespace boost { namespace geometry
{


/*!
\brief "compound strategy", containing a segment-intersection-strategy
       and a side-strategy
 */
template
<
    typename Tag,
    typename Geometry1,
    typename Geometry2,
    typename IntersectionPoint,
    typename RobustPolicy,
    typename CalculationType = void
>
struct intersection_strategies
{
private :
    // for development BOOST_STATIC_ASSERT((! boost::is_same<RobustPolicy, void>::type::value));

    typedef typename geometry::point_type<Geometry1>::type point1_type;
    typedef typename geometry::point_type<Geometry2>::type point2_type;
    typedef typename model::referring_segment<point1_type const> segment1_type;
    typedef typename model::referring_segment<point2_type const> segment2_type;

    typedef segment_intersection_points
    <
        IntersectionPoint,
        typename geometry::segment_ratio_type
        <
            IntersectionPoint, RobustPolicy
        >::type
    > ip_type;

public:
    typedef policies::relate::segments_tupled
        <
            policies::relate::segments_intersection_points
                <
                    ip_type
                > ,
            policies::relate::segments_direction
        > intersection_policy_type;

    typedef typename strategy::intersection::services::default_strategy
            <
                Tag,
                CalculationType
            >::type segment_intersection_strategy_type;

    typedef typename strategy::side::services::default_strategy
        <
            Tag,
            CalculationType
        >::type side_strategy_type;

    typedef RobustPolicy rescale_policy_type;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_INTERSECTION_HPP
