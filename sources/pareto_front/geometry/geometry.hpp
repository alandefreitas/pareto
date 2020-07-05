// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014-2018.
// Modifications copyright (c) 2014-2018 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRY_HPP
#define BOOST_GEOMETRY_GEOMETRY_HPP

// Shortcut to include all header files

#include <pareto_front/geometry/core/closure.hpp>
#include <pareto_front/geometry/core/coordinate_dimension.hpp>
#include <pareto_front/geometry/core/coordinate_system.hpp>
#include <pareto_front/geometry/core/coordinate_type.hpp>
#include <pareto_front/geometry/core/cs.hpp>
#include <pareto_front/geometry/core/interior_type.hpp>
#include <pareto_front/geometry/core/point_order.hpp>
#include <pareto_front/geometry/core/point_type.hpp>
#include <pareto_front/geometry/core/ring_type.hpp>
#include <pareto_front/geometry/core/tag.hpp>
#include <pareto_front/geometry/core/tag_cast.hpp>
#include <pareto_front/geometry/core/tags.hpp>

// Core algorithms
#include <pareto_front/geometry/core/access.hpp>
#include <pareto_front/geometry/core/exterior_ring.hpp>
#include <pareto_front/geometry/core/interior_rings.hpp>
#include <pareto_front/geometry/core/radian_access.hpp>
#include <pareto_front/geometry/core/radius.hpp>
#include <pareto_front/geometry/core/topological_dimension.hpp>

#include <pareto_front/geometry/arithmetic/arithmetic.hpp>
#include <pareto_front/geometry/arithmetic/dot_product.hpp>

#include <pareto_front/geometry/strategies/strategies.hpp>

#include <pareto_front/geometry/algorithms/append.hpp>
#include <pareto_front/geometry/algorithms/area.hpp>
#include <pareto_front/geometry/algorithms/assign.hpp>
#include <pareto_front/geometry/algorithms/buffer.hpp>
#include <pareto_front/geometry/algorithms/centroid.hpp>
#include <pareto_front/geometry/algorithms/clear.hpp>
#include <pareto_front/geometry/algorithms/comparable_distance.hpp>
#include <pareto_front/geometry/algorithms/convert.hpp>
#include <pareto_front/geometry/algorithms/convex_hull.hpp>
#include <pareto_front/geometry/algorithms/correct.hpp>
#include <pareto_front/geometry/algorithms/covered_by.hpp>
#include <pareto_front/geometry/algorithms/crosses.hpp>
#include <pareto_front/geometry/algorithms/densify.hpp>
#include <pareto_front/geometry/algorithms/difference.hpp>
#include <pareto_front/geometry/algorithms/discrete_frechet_distance.hpp>
#include <pareto_front/geometry/algorithms/discrete_hausdorff_distance.hpp>
#include <pareto_front/geometry/algorithms/disjoint.hpp>
#include <pareto_front/geometry/algorithms/distance.hpp>
#include <pareto_front/geometry/algorithms/envelope.hpp>
#include <pareto_front/geometry/algorithms/equals.hpp>
#include <pareto_front/geometry/algorithms/expand.hpp>
#include <pareto_front/geometry/algorithms/for_each.hpp>
#include <pareto_front/geometry/algorithms/intersection.hpp>
#include <pareto_front/geometry/algorithms/intersects.hpp>
#include <pareto_front/geometry/algorithms/is_empty.hpp>
#include <pareto_front/geometry/algorithms/is_simple.hpp>
#include <pareto_front/geometry/algorithms/is_valid.hpp>
#include <pareto_front/geometry/algorithms/length.hpp>
#include <pareto_front/geometry/algorithms/line_interpolate.hpp>
#include <pareto_front/geometry/algorithms/make.hpp>
#include <pareto_front/geometry/algorithms/num_geometries.hpp>
#include <pareto_front/geometry/algorithms/num_interior_rings.hpp>
#include <pareto_front/geometry/algorithms/num_points.hpp>
#include <pareto_front/geometry/algorithms/num_segments.hpp>
#include <pareto_front/geometry/algorithms/overlaps.hpp>
#include <pareto_front/geometry/algorithms/perimeter.hpp>
#include <pareto_front/geometry/algorithms/relate.hpp>
#include <pareto_front/geometry/algorithms/relation.hpp>
#include <pareto_front/geometry/algorithms/remove_spikes.hpp>
#include <pareto_front/geometry/algorithms/reverse.hpp>
#include <pareto_front/geometry/algorithms/simplify.hpp>
#include <pareto_front/geometry/algorithms/sym_difference.hpp>
#include <pareto_front/geometry/algorithms/touches.hpp>
#include <pareto_front/geometry/algorithms/transform.hpp>
#include <pareto_front/geometry/algorithms/union.hpp>
#include <pareto_front/geometry/algorithms/unique.hpp>
#include <pareto_front/geometry/algorithms/within.hpp>

// check includes all concepts
#include <pareto_front/geometry/geometries/concepts/check.hpp>

#include <pareto_front/geometry/srs/srs.hpp>

#include <pareto_front/geometry/util/for_each_coordinate.hpp>
#include <pareto_front/geometry/util/math.hpp>
#include <pareto_front/geometry/util/select_coordinate_type.hpp>
#include <pareto_front/geometry/util/select_most_precise.hpp>

#include <pareto_front/geometry/views/box_view.hpp>
#include <pareto_front/geometry/views/closeable_view.hpp>
#include <pareto_front/geometry/views/identity_view.hpp>
#include <pareto_front/geometry/views/reversible_view.hpp>
#include <pareto_front/geometry/views/segment_view.hpp>

#include <pareto_front/geometry/io/io.hpp>
#include <pareto_front/geometry/io/dsv/write.hpp>
#include <pareto_front/geometry/io/svg/svg_mapper.hpp>
#include <pareto_front/geometry/io/svg/write.hpp>
#include <pareto_front/geometry/io/wkt/read.hpp>
#include <pareto_front/geometry/io/wkt/write.hpp>

#endif // BOOST_GEOMETRY_GEOMETRY_HPP
