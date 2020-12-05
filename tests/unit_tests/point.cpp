#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#ifdef BUILD_UNIT_TEST_EXTERN_INSTANTIATION
#include "instantiation/test_instantiations.h"
#endif

#include <pareto/front.h>
#include <pareto/point.h>


/// The point object has a better distance function
/// This one is for comparisons only
template<class POINT_TYPE>
double point_distance(const POINT_TYPE &p1, const POINT_TYPE &p2) {
    double dist = 0.;
    for (size_t i = 0; i < p1.dimensions(); ++i) {
        dist += pow(p1[i] - p2[i], 2);
    }
    return sqrt(dist);
}

TEST_CASE("Point dominance") {
    using namespace pareto;
    // Compile size is 0, so the dimension is inferred from
    // the point_type
    using pareto_front_t = front<double, 0, unsigned>;
    using point_t = pareto_front_t::point_type;

    // Points
    point_t p1 = {6.5, 4.6};
    point_t p2 = {3.4, 2.4};
    point_t p3 = {3.4, 6.5};

    REQUIRE_FALSE(p1.dominates(p2));
    REQUIRE_FALSE(p1.dominates(p3));

    REQUIRE(p2.dominates(p1));
    REQUIRE(p2.dominates(p3));

    REQUIRE_FALSE(p3.dominates(p1));
    REQUIRE_FALSE(p3.dominates(p2));

    REQUIRE_FALSE(p1.strongly_dominates(p2));
    REQUIRE_FALSE(p1.strongly_dominates(p3));

    REQUIRE(p2.strongly_dominates(p1));

    REQUIRE_FALSE(p2.strongly_dominates(p3));
    REQUIRE_FALSE(p3.strongly_dominates(p1));
    REQUIRE_FALSE(p3.strongly_dominates(p2));

    REQUIRE_FALSE(p1.non_dominates(p2));

    REQUIRE(p1.non_dominates(p3));
    REQUIRE_FALSE(p2.non_dominates(p1));
    REQUIRE_FALSE(p2.non_dominates(p3));

    REQUIRE(p3.non_dominates(p1));
    REQUIRE_FALSE(p3.non_dominates(p2));
}
