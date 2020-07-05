// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2010-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_RANGE_UNIQUED_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_RANGE_UNIQUED_HPP


#include <pareto_front/boost_subset/range/adaptor/uniqued.hpp>

#include <pareto_front/geometry/core/tag.hpp>
#include <pareto_front/geometry/core/tags.hpp>


namespace boost { namespace geometry
{

namespace traits
{

template<typename Geometry>
#if BOOST_VERSION > 104500
struct tag<boost::uniqued_range<Geometry> >
#else
struct tag<boost::range_detail::unique_range<Geometry> >
#endif
{
    typedef typename geometry::tag<Geometry>::type type;
};

}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_RANGE_UNIQUED_HPP

