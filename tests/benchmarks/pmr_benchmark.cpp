#include <benchmark/benchmark.h>
#include <pareto/common/default_allocator.h>
#include <pareto/point.h>
#include <pareto/spatial_map.h>
#include "../test_helpers.h"
#include <map>

pareto::point<int, 3> random_int_point() {
    pareto::point<int, 3> p(3);
    std::generate(p.begin(), p.end(), randn);
    return p;
}

void std_allocator_map(benchmark::State& state) {
    std::map<int, int, std::less<>, std::allocator<std::pair<const int, int>>> m;
    for (auto _ : state) {
        m.emplace(randi(),randi());
    }
}

BENCHMARK(std_allocator_map);

#ifdef BUILD_PARETO_WITH_PMR
void default_pmr_allocator_map(benchmark::State& state) {
    std::map<int, int, std::less<>, std::pmr::polymorphic_allocator<std::pair<const int, int>>> m;
    for (auto _ : state) {
        m.emplace(randi(),randi());
    }
}
BENCHMARK(default_pmr_allocator_map);


void unsyncronized_allocator_map(benchmark::State& state) {
    std::pmr::unsynchronized_pool_resource pool;
    std::pmr::polymorphic_allocator<std::pair<const int, int>> alloc(&pool);
    std::map<int, int, std::less<>, std::pmr::polymorphic_allocator<std::pair<const int, int>>> m(alloc);
    for (auto _ : state) {
        m.emplace(randi(),randi());
    }
}
BENCHMARK(unsyncronized_allocator_map);


void monotonic_allocator_map(benchmark::State& state) {
    std::pmr::monotonic_buffer_resource pool;
    std::pmr::polymorphic_allocator<std::pair<const int, int>> alloc(&pool);
    std::map<int, int, std::less<>, std::pmr::polymorphic_allocator<std::pair<const int, int>>> m(alloc);
    for (auto _ : state) {
        m.emplace(randi(),randi());
    }
}
BENCHMARK(monotonic_allocator_map);

#endif

void std_allocator_spatial_map(benchmark::State& state) {
    pareto::spatial_map<int, 3, int, std::less<>, std::allocator<std::pair<const pareto::point<int,3>, int>>> m;
    for (auto _ : state) {
        m.emplace(random_int_point(),randi());
    }
}
BENCHMARK(std_allocator_spatial_map);


#ifdef BUILD_PARETO_WITH_PMR
void default_pmr_allocator_spatial_map(benchmark::State& state) {
    pareto::spatial_map<int, 3, int, std::less<>, std::pmr::polymorphic_allocator<std::pair<const pareto::point<int,3>, int>>> m;
    for (auto _ : state) {
        m.emplace(random_int_point(),randi());
    }
}
BENCHMARK(default_pmr_allocator_spatial_map);

void default_pareto_allocator_spatial_map(benchmark::State& state) {
    pareto::spatial_map<int, 3, int, std::less<>> m;
    for (auto _ : state) {
        m.emplace(random_int_point(),randi());
    }
}
BENCHMARK(default_pareto_allocator_spatial_map);

void unsyncronized_allocator_spatial_map(benchmark::State& state) {
    std::pmr::unsynchronized_pool_resource pool;
    std::pmr::polymorphic_allocator<std::pair<const int, int>> alloc(&pool);
    pareto::spatial_map<int, 3, int, std::less<>, std::pmr::polymorphic_allocator<std::pair<const pareto::point<int,3>, int>>> m(alloc);
    for (auto _ : state) {
        m.emplace(random_int_point(),randi());
    }
}
BENCHMARK(unsyncronized_allocator_spatial_map);

void monotonic_allocator_spatial_map(benchmark::State& state) {
    std::pmr::monotonic_buffer_resource pool(100000);
    std::pmr::polymorphic_allocator<std::pair<const int, int>> alloc(&pool);
    pareto::spatial_map<int, 3, int, std::less<>, std::pmr::polymorphic_allocator<std::pair<const pareto::point<int,3>, int>>> m(alloc);
    for (auto _ : state) {
        m.emplace(random_int_point(),randi());
    }
}
BENCHMARK(monotonic_allocator_spatial_map);
#endif

BENCHMARK_MAIN();