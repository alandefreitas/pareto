#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../test_helpers.h"

TEST_CASE("Front interface") {
    using namespace pareto;
    front<double,2> pf;
    pf(0.32,0.75) = 3;
}