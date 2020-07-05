// Boost.Geometry

// Copyright (c) 2017-2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DENSIFY_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DENSIFY_HPP


#include <pareto_front/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <pareto_front/geometry/algorithms/detail/signed_size_type.hpp>
#include <pareto_front/geometry/arithmetic/arithmetic.hpp>
#include <pareto_front/geometry/arithmetic/cross_product.hpp>
#include <pareto_front/geometry/arithmetic/dot_product.hpp>
#include <pareto_front/geometry/arithmetic/normalize.hpp>
#include <pareto_front/geometry/core/assert.hpp>
#include <pareto_front/geometry/core/coordinate_dimension.hpp>
#include <pareto_front/geometry/core/coordinate_type.hpp>
#include <pareto_front/geometry/core/radian_access.hpp>
#include <pareto_front/geometry/formulas/spherical.hpp>
#include <pareto_front/geometry/formulas/interpolate_point_spherical.hpp>
#include <pareto_front/geometry/geometries/point.hpp>
#include <pareto_front/geometry/srs/sphere.hpp>
#include <pareto_front/geometry/strategies/densify.hpp>
#include <pareto_front/geometry/strategies/spherical/get_radius.hpp>
#include <pareto_front/geometry/util/math.hpp>
#include <pareto_front/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace densify
{


/*!
\brief Densification of spherical segment.
\ingroup strategies
\tparam RadiusTypeOrSphere \tparam_radius_or_sphere
\tparam CalculationType \tparam_calculation

\qbk{
[heading See also]
[link geometry.reference.algorithms.densify.densify_4_with_strategy densify (with strategy)]
}
 */
template
<
    typename RadiusTypeOrSphere = double,
    typename CalculationType = void
>
class spherical
{
public:
    // For consistency with area strategy the radius is set to 1
    inline spherical()
        : m_radius(1.0)
    {}

    template <typename RadiusOrSphere>
    explicit inline spherical(RadiusOrSphere const& radius_or_sphere)
        : m_radius(strategy_detail::get_radius
                    <
                        RadiusOrSphere
                    >::apply(radius_or_sphere))
    {}

    template <typename Point, typename AssignPolicy, typename T>
    inline void apply(Point const& p0, Point const& p1, AssignPolicy & policy, T const& length_threshold) const
    {
        typedef typename AssignPolicy::point_type out_point_t;
        typedef typename select_most_precise
            <
                typename coordinate_type<Point>::type,
                typename coordinate_type<out_point_t>::type,
                CalculationType
            >::type calc_t;

        calc_t angle01;

        formula::interpolate_point_spherical<calc_t> formula;
        formula.compute_angle(p0, p1, angle01);

        BOOST_GEOMETRY_ASSERT(length_threshold > T(0));

        signed_size_type n = signed_size_type(angle01 * m_radius / length_threshold);
        if (n <= 0)
            return;

        formula.compute_axis(p0, angle01);

        calc_t step = angle01 / (n + 1);

        calc_t a = step;
        for (signed_size_type i = 0 ; i < n ; ++i, a += step)
        {
            out_point_t p;
            formula.compute_point(a, p);

            geometry::detail::conversion::point_to_point
                <
                    Point, out_point_t,
                    2, dimension<out_point_t>::value
                >::apply(p0, p);

            policy.apply(p);
        }
    }

private:
    typename strategy_detail::get_radius
        <
            RadiusTypeOrSphere
        >::type m_radius;
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct default_strategy<spherical_equatorial_tag>
{
    typedef strategy::densify::spherical<> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::densify


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DENSIFY_HPP
