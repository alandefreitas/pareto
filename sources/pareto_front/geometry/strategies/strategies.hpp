// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2014-2019.
// Modifications copyright (c) 2014-2019 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_STRATEGIES_HPP
#define BOOST_GEOMETRY_STRATEGIES_STRATEGIES_HPP


#include <pareto_front/geometry/strategies/tags.hpp>

#include <pareto_front/geometry/strategies/area.hpp>
#include <pareto_front/geometry/strategies/azimuth.hpp>
#include <pareto_front/geometry/strategies/buffer.hpp>
#include <pareto_front/geometry/strategies/centroid.hpp>
#include <pareto_front/geometry/strategies/compare.hpp>
#include <pareto_front/geometry/strategies/convex_hull.hpp>
#include <pareto_front/geometry/strategies/covered_by.hpp>
#include <pareto_front/geometry/strategies/densify.hpp>
#include <pareto_front/geometry/strategies/disjoint.hpp>
#include <pareto_front/geometry/strategies/distance.hpp>
#include <pareto_front/geometry/strategies/envelope.hpp>
#include <pareto_front/geometry/strategies/intersection.hpp>
#include <pareto_front/geometry/strategies/intersection_strategies.hpp> // for backward compatibility
#include <pareto_front/geometry/strategies/relate.hpp>
#include <pareto_front/geometry/strategies/side.hpp>
#include <pareto_front/geometry/strategies/transform.hpp>
#include <pareto_front/geometry/strategies/within.hpp>

#include <pareto_front/geometry/strategies/cartesian/area.hpp>
#include <pareto_front/geometry/strategies/cartesian/azimuth.hpp>
#include <pareto_front/geometry/strategies/cartesian/box_in_box.hpp>
#include <pareto_front/geometry/strategies/cartesian/buffer_end_flat.hpp>
#include <pareto_front/geometry/strategies/cartesian/buffer_end_round.hpp>
#include <pareto_front/geometry/strategies/cartesian/buffer_join_miter.hpp>
#include <pareto_front/geometry/strategies/cartesian/buffer_join_round.hpp>
#include <pareto_front/geometry/strategies/cartesian/buffer_join_round_by_divide.hpp>
#include <pareto_front/geometry/strategies/cartesian/buffer_point_circle.hpp>
#include <pareto_front/geometry/strategies/cartesian/buffer_point_square.hpp>
#include <pareto_front/geometry/strategies/cartesian/buffer_side_straight.hpp>
#include <pareto_front/geometry/strategies/cartesian/centroid_average.hpp>
#include <pareto_front/geometry/strategies/cartesian/centroid_bashein_detmer.hpp>
#include <pareto_front/geometry/strategies/cartesian/centroid_weighted_length.hpp>
#include <pareto_front/geometry/strategies/cartesian/densify.hpp>
#include <pareto_front/geometry/strategies/cartesian/disjoint_segment_box.hpp>
#include <pareto_front/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <pareto_front/geometry/strategies/cartesian/distance_pythagoras_point_box.hpp>
#include <pareto_front/geometry/strategies/cartesian/distance_pythagoras_box_box.hpp>
#include <pareto_front/geometry/strategies/cartesian/distance_projected_point.hpp>
#include <pareto_front/geometry/strategies/cartesian/distance_projected_point_ax.hpp>
#include <pareto_front/geometry/strategies/cartesian/distance_segment_box.hpp>
#include <pareto_front/geometry/strategies/cartesian/envelope_box.hpp>
#include <pareto_front/geometry/strategies/cartesian/envelope_point.hpp>
#include <pareto_front/geometry/strategies/cartesian/envelope_multipoint.hpp>
#include <pareto_front/geometry/strategies/cartesian/envelope_segment.hpp>
#include <pareto_front/geometry/strategies/cartesian/expand_box.hpp>
#include <pareto_front/geometry/strategies/cartesian/expand_point.hpp>
#include <pareto_front/geometry/strategies/cartesian/expand_segment.hpp>
#include <pareto_front/geometry/strategies/cartesian/index.hpp>
#include <pareto_front/geometry/strategies/cartesian/intersection.hpp>
#include <pareto_front/geometry/strategies/cartesian/point_in_box.hpp>
#include <pareto_front/geometry/strategies/cartesian/point_in_point.hpp>
#include <pareto_front/geometry/strategies/cartesian/point_in_poly_franklin.hpp>
#include <pareto_front/geometry/strategies/cartesian/point_in_poly_crossings_multiply.hpp>
#include <pareto_front/geometry/strategies/cartesian/point_in_poly_winding.hpp>
#include <pareto_front/geometry/strategies/cartesian/line_interpolate.hpp>
#include <pareto_front/geometry/strategies/cartesian/side_by_triangle.hpp>

#include <pareto_front/geometry/strategies/spherical/area.hpp>
#include <pareto_front/geometry/strategies/spherical/azimuth.hpp>
#include <pareto_front/geometry/strategies/spherical/densify.hpp>
#include <pareto_front/geometry/strategies/spherical/disjoint_segment_box.hpp>
#include <pareto_front/geometry/strategies/spherical/distance_haversine.hpp>
#include <pareto_front/geometry/strategies/spherical/distance_cross_track.hpp>
#include <pareto_front/geometry/strategies/spherical/distance_cross_track_box_box.hpp>
#include <pareto_front/geometry/strategies/spherical/distance_cross_track_point_box.hpp>
#include <pareto_front/geometry/strategies/spherical/distance_segment_box.hpp>
#include <pareto_front/geometry/strategies/spherical/compare.hpp>
#include <pareto_front/geometry/strategies/spherical/envelope_box.hpp>
#include <pareto_front/geometry/strategies/spherical/envelope_point.hpp>
#include <pareto_front/geometry/strategies/spherical/envelope_multipoint.hpp>
#include <pareto_front/geometry/strategies/spherical/envelope_segment.hpp>
#include <pareto_front/geometry/strategies/spherical/expand_box.hpp>
#include <pareto_front/geometry/strategies/spherical/expand_point.hpp>
#include <pareto_front/geometry/strategies/spherical/expand_segment.hpp>
#include <pareto_front/geometry/strategies/spherical/index.hpp>
#include <pareto_front/geometry/strategies/spherical/intersection.hpp>
#include <pareto_front/geometry/strategies/spherical/point_in_point.hpp>
#include <pareto_front/geometry/strategies/spherical/point_in_poly_winding.hpp>
#include <pareto_front/geometry/strategies/spherical/line_interpolate.hpp>
#include <pareto_front/geometry/strategies/spherical/ssf.hpp>

#include <pareto_front/geometry/strategies/geographic/area.hpp>
#include <pareto_front/geometry/strategies/geographic/azimuth.hpp>
#include <pareto_front/geometry/strategies/geographic/buffer_point_circle.hpp>
#include <pareto_front/geometry/strategies/geographic/densify.hpp>
#include <pareto_front/geometry/strategies/geographic/disjoint_segment_box.hpp>
#include <pareto_front/geometry/strategies/geographic/distance.hpp>
#include <pareto_front/geometry/strategies/geographic/distance_andoyer.hpp>
#include <pareto_front/geometry/strategies/geographic/distance_cross_track.hpp>
#include <pareto_front/geometry/strategies/geographic/distance_cross_track_box_box.hpp>
#include <pareto_front/geometry/strategies/geographic/distance_cross_track_point_box.hpp>
#include <pareto_front/geometry/strategies/geographic/distance_segment_box.hpp>
#include <pareto_front/geometry/strategies/geographic/distance_thomas.hpp>
#include <pareto_front/geometry/strategies/geographic/distance_vincenty.hpp>
#include <pareto_front/geometry/strategies/geographic/envelope_segment.hpp>
#include <pareto_front/geometry/strategies/geographic/expand_segment.hpp>
#include <pareto_front/geometry/strategies/geographic/index.hpp>
#include <pareto_front/geometry/strategies/geographic/intersection.hpp>
//#include <pareto_front/geometry/strategies/geographic/intersection_elliptic.hpp>
#include <pareto_front/geometry/strategies/geographic/point_in_poly_winding.hpp>
#include <pareto_front/geometry/strategies/geographic/line_interpolate.hpp>
#include <pareto_front/geometry/strategies/geographic/side.hpp>
#include <pareto_front/geometry/strategies/geographic/side_andoyer.hpp>
#include <pareto_front/geometry/strategies/geographic/side_thomas.hpp>
#include <pareto_front/geometry/strategies/geographic/side_vincenty.hpp>

#include <pareto_front/geometry/strategies/agnostic/buffer_distance_symmetric.hpp>
#include <pareto_front/geometry/strategies/agnostic/buffer_distance_asymmetric.hpp>
#include <pareto_front/geometry/strategies/agnostic/hull_graham_andrew.hpp>
#include <pareto_front/geometry/strategies/agnostic/point_in_box_by_side.hpp>
#include <pareto_front/geometry/strategies/agnostic/point_in_point.hpp>
#include <pareto_front/geometry/strategies/agnostic/point_in_poly_winding.hpp>
#include <pareto_front/geometry/strategies/agnostic/simplify_douglas_peucker.hpp>

#include <pareto_front/geometry/strategies/strategy_transform.hpp>

#include <pareto_front/geometry/strategies/transform/matrix_transformers.hpp>
#include <pareto_front/geometry/strategies/transform/map_transformer.hpp>
#include <pareto_front/geometry/strategies/transform/inverse_transformer.hpp>


#endif // BOOST_GEOMETRY_STRATEGIES_STRATEGIES_HPP
