#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../test_helpers.h"

TEST_CASE("Front interface") {
    using namespace pareto;
    front<double,2> pf;
    pf(0.32,0.75) = 3;
    auto it = pf.begin({satisfies<double,2,unsigned>([](const front<double,2>::value_type& v) { return true; })});
    for (const auto &[p, v] : pf) {
        std::cout << "p: " << p << std::endl;
    }
}