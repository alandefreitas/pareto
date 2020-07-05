// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ITERATORS_DETAIL_POINT_ITERATOR_INNER_RANGE_TYPE_HPP
#define BOOST_GEOMETRY_ITERATORS_DETAIL_POINT_ITERATOR_INNER_RANGE_TYPE_HPP

#include <pareto_front/boost_subset/range.hpp>
#include <pareto_front/boost_subset/type_traits/is_const.hpp>
#include <pareto_front/boost_subset/mpl/if.hpp>

#include <pareto_front/geometry/core/ring_type.hpp>
#include <pareto_front/geometry/core/tag.hpp>
#include <pareto_front/geometry/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace point_iterator
{


template
<
    typename Geometry, 
    typename Tag = typename tag<Geometry>::type
>
struct inner_range_type
{
    typedef typename boost::mpl::if_c
        <
            !boost::is_const<Geometry>::type::value,
            typename boost::range_value<Geometry>::type,
            typename boost::range_value<Geometry>::type const
        >::type type;
};


template <typename Polygon>
struct inner_range_type<Polygon, polygon_tag>
{
    typedef typename boost::mpl::if_c
        <
            !boost::is_const<Polygon>::type::value,
            typename geometry::ring_type<Polygon>::type,
            typename geometry::ring_type<Polygon>::type const
        >::type type;
};


}} // namespace detail::point_iterator
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ITERATORS_DETAIL_POINT_ITERATOR_INNER_RANGE_TYPE_HPP
