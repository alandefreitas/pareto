#include <catch2/catch.hpp>
#include <iostream>
#include <map>
#include <pareto/common/default_allocator.h>
//#include <pareto/common/demangle.h>
#include <pareto/spatial_map.h>
//#include <pareto/front.h>
//#include <pareto/point.h>

TEST_CASE("PMR") {
    SECTION("Map") {
        std::map<int, int, std::less<>,
                 pareto::default_allocator_type<std::pair<const int, int>>>
            m;
        m.emplace(2, 3);
        for (const auto &[k, v] : m) {
            std::cout << "k: " << k << std::endl;
            std::cout << "v: " << v << std::endl;
        }
    }

    SECTION("pareto::spatial_map") {
        /// The spatial map will use an internal PMR if header is available
        /// We use a placeholder allocator as default
        /// If the user creates a container with the default PMR allocator
        /// (the placeholder) then this is replaced by an internal memory
        /// pool limited to the container
        using map_type = pareto::spatial_map<int, 2, int>;
        map_type m;
        m.emplace(map_type::key_type({2, 4}), 3);
        for (const auto &[k, v] : m) {
            std::cout << "k: " << k << std::endl;
            std::cout << "v: " << v << std::endl;
        }
    }

    SECTION("Move std::allocator") {
        // Std allocators always point to the same resource
        std::allocator<int> mr;
        std::vector<int> v({1, 2, 3}, mr);
        std::vector<int> w;
        auto a1 = w.get_allocator();
        w = std::move(v);
        REQUIRE(w.get_allocator() == a1);
    }

#ifdef BUILD_PARETO_WITH_PMR
    SECTION("Move PMR") {
        // PMR allocators should assume the worst at compile time, no
        // propagation We can manually propagate during runtime though But we'd
        // have to keep track of the resources to make sure they are using the
        // same internal memory resource \see
        // https://stackoverflow.com/questions/45088131/why-does-stdpmrpolymorphic-allocator-not-propagate-on-container-move
        std::pmr::monotonic_buffer_resource mr(1000);
        std::pmr::polymorphic_allocator<int> alloc(&mr);
        std::pmr::vector<int> v({1, 2, 3}, alloc);
        std::pmr::vector<int> w;
        auto a1 = w.get_allocator();
        w = std::move(v);
        // The allocator is still the same (still point to the same
        // resource) and the only the internal items were moved
        // The move operation had a linear cost
        REQUIRE(w.get_allocator() == a1);
    }
#endif
}
