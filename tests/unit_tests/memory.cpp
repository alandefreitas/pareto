#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <pareto/memory/memory_pool.h>
#include <set>

TEST_CASE("Memory Pool") {
    using namespace pareto;

    SECTION("Manually Allocate POD") {
        fast_memory_pool<int> alloc;
        int* i = alloc.allocate();
        *i = 2;
        REQUIRE(i != nullptr);
        REQUIRE(*i == 2);
        alloc.destroy(i);
        alloc.deallocate(i);
        REQUIRE(i != nullptr);
        REQUIRE(*i == 0);
        REQUIRE(alloc.current_block_size() == 2048);

        // Create analogous allocator for doubles
        decltype(alloc)::rebind<double>::other alloc2;
        double* d = alloc2.allocate();
        *d = 2.2;
        REQUIRE(d != nullptr);
        REQUIRE(*d == 2.2);
        alloc2.destroy(d);
        alloc2.deallocate(d);
        REQUIRE(d != nullptr);
        REQUIRE(*d == 0);

        // Create analogous allocator with conversion
        fast_memory_pool<float> alloc3(alloc);
        float* f = alloc3.allocate();
        *f = 2.3f;
        REQUIRE(f != nullptr);
        REQUIRE(*f == 2.3f);
    }

    SECTION("Set") {
        // Allocators are full of limitations in containers
        // * They are not stateful, so we cannot reuse the allocators
        //   with std containers.
        // * The allocator type and allocator you give to the container
        //   is basically thrown away and the container rebinds it to
        //   an allocator of a new type
        // * To make things worse, get_allocator() returns a new allocator
        //   of the original type and not the allocator after rebind
        // Nonetheless, the allocators still work for a single container
        // We can mitigate that stuff with polymorphic allocators
        std::set<int, std::less<>, fast_memory_pool<>> s;
        s.insert(4);
        s.insert(2);
        s.insert(3);
        s.insert(6);
        auto it = s.begin();
        REQUIRE(*it == 2);
        ++it;
        REQUIRE(*it == 3);
        ++it;
        REQUIRE(*it == 4);
        ++it;
        REQUIRE(*it == 6);
    }

    SECTION("Contiguous") {
        using allocator_type = pareto::contiguous_memory_pool<double>;
        allocator_type alloc;
        using small_vector_type = std::vector<double, allocator_type>;
        small_vector_type s{alloc};
        s.resize(3);
        s[0] = 1;
        s[1] = 2;
        s[2] = 3;
        REQUIRE(s[0] == 1);
        REQUIRE(s[1] == 2);
        REQUIRE(s[2] == 3);
        REQUIRE(s.get_allocator().current_block_size() == 0);

        std::vector<small_vector_type> v;
        v.emplace_back();
        v.emplace_back(alloc);
    }
}