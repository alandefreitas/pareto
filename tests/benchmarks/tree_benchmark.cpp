#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include "monotonic_resource.h"
#include <benchmark/benchmark.h>
#include <pareto/front.h>
#include <pareto/memory_pool.h>

// Rationale
// The "unsynchronized_pool_resource" should give us performance
// without sacrificing robustness. It seems like compilers have
// not implemented this C++17 feature yet, and finding reliable
// allocator classes hasn't been easy. It's also hard to find
// r-trees in a STL-container format that use the allocators
// properly. In some of my previous benchmarks, Boost r-trees
// had no performance difference with memory pool allocators
// even when the same allocators when making set 4x faster.
// We implement our own allocator that gives us performance
// but it's not production safe for very sensitive tasks.
// It might be better to use std::allocator for production code.
// https://github.com/gcc-mirror/gcc/blob/4a3895903c29ed85da6fcb886f31ff23d4c6e935/libstdc%2B%2B-v3/include/std/memory_resource#L300
// https://github.com/llvm-mirror/libcxx/blob/78d6a7767ed57b50122a161b91f59f19c9bd0d19/include/experimental/memory_resource#L57

void create_set_fast_pool(benchmark::State &state) {
    std::set<std::array<double,3>, std::less<std::array<double,3>>, pareto::fast_memory_pool<std::array<double,3>>> s;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_set_free_pool(benchmark::State &state) {
    std::set<std::array<double,3>, std::less<std::array<double,3>>, pareto::free_memory_pool<std::array<double,3>>> s;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_set_new(benchmark::State &state) {
    std::set<std::array<double,3>> s;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_set_monotonic(benchmark::State &state) {
    std::pmr::monotonic_buffer_resource rsrc;
    std::pmr::polymorphic_allocator<std::array<double,3>> alloc(&rsrc);
    std::set<std::array<double,3>, std::less<std::array<double,3>>, std::pmr::polymorphic_allocator<std::array<double,3>>> s(alloc);
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_set_contiguous_pool(benchmark::State &state) {
    std::set<std::array<double,3>, std::less<std::array<double,3>>, pareto::contiguous_memory_pool<std::array<double,3>>> s;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_small_vector_contiguous_pool(benchmark::State &state) {
    pareto::contiguous_memory_pool<double> alloc;
    using small_vector_type = std::vector<double, pareto::contiguous_memory_pool<double>>;
    std::vector<small_vector_type> v;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            v.emplace(v.end(), alloc);
            v.back().resize(3);
            v.back()[0] = 0;
            v.back()[1] = 1;
            v.back()[2] = 2;
        }
        benchmark::DoNotOptimize(v);
    }
}

void create_small_vector_fast_pool_shared_ptr(benchmark::State &state) {
    pareto::fast_memory_pool<std::array<double,3>> alloc;
    std::vector<std::shared_ptr<std::array<double,3>>> v;
    v.reserve(state.range(0));
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            v.emplace(v.end(), std::shared_ptr<std::array<double,3>>(new std::array<double,3>(), std::default_delete<std::array<double,3>>(), alloc));
            v.back()->operator[](0) = 0;
            v.back()->operator[](1) = 1;
            v.back()->operator[](2) = 2;
        }
        benchmark::DoNotOptimize(v);
    }
}

void create_small_vector_fast_pool_new(benchmark::State &state) {
    pareto::fast_memory_pool<std::array<double,3>> alloc;
    std::vector<std::shared_ptr<std::array<double,3>>> v;
    v.reserve(state.range(0));
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            v.emplace(v.end(), std::shared_ptr<std::array<double,3>>(new std::array<double,3>(), std::default_delete<std::array<double,3>>(), alloc));
            v.back()->operator[](0) = 0;
            v.back()->operator[](1) = 1;
            v.back()->operator[](2) = 2;
        }
        benchmark::DoNotOptimize(v);
    }
}

void create_small_vector_free_pool(benchmark::State &state) {
    pareto::free_memory_pool<double> alloc;
    using small_vector_type = std::vector<double, pareto::free_memory_pool<double>>;
    std::vector<small_vector_type> v;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            v.emplace(v.end(), alloc);
            v.back().resize(3);
            v.back()[0] = 0;
            v.back()[1] = 1;
            v.back()[2] = 2;
        }
        benchmark::DoNotOptimize(v);
    }
}

void create_small_vector_new(benchmark::State &state) {
    std::allocator<double> alloc;
    using small_vector_type = std::vector<double, std::allocator<double>>;
    std::vector<small_vector_type> v;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            v.emplace(v.end(), alloc);
            v.back().resize(3);
            v.back()[0] = 0;
            v.back()[1] = 1;
            v.back()[2] = 2;
        }
        benchmark::DoNotOptimize(v);
    }
}

void create_small_vector_monotonic(benchmark::State &state) {

    std::pmr::monotonic_buffer_resource rsrc;
    std::pmr::polymorphic_allocator<double> alloc(&rsrc);
    using small_vector_type = std::vector<double, std::pmr::polymorphic_allocator<double>>;
    std::vector<small_vector_type> v;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            v.emplace(v.end(), alloc);
            v.back().resize(3);
            v.back()[0] = 0;
            v.back()[1] = 1;
            v.back()[2] = 2;
        }
        benchmark::DoNotOptimize(v);
    }
}

void create_large_vector_contiguous_interleaved_pool(benchmark::State &state) {
    std::vector<double, pareto::contiguous_memory_pool<double>> s;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            s.emplace_back(i);
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_large_vector_free_pool(benchmark::State &state) {
    std::vector<double, pareto::free_memory_pool<double>> s;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            s.emplace_back(i);
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_large_vector_new(benchmark::State &state) {
    std::vector<double> s;
    for (auto _ : state) {
        for (size_t i = 0; i < state.range(0); ++i) {
            s.emplace_back(i);
        }
        benchmark::DoNotOptimize(s);
    }
}

constexpr size_t max_container_size = 5000000;
template <size_t MAX_SIZE = max_container_size>
void pareto_sizes_and_samples(benchmark::internal::Benchmark* b) {
    for (long long i = 50; i <= MAX_SIZE; i *= 10) {
        b->Args({i});
    }
}

// Sets: fast > free > new > monotonic > contiguous
BENCHMARK(create_set_fast_pool)->Apply(pareto_sizes_and_samples);
BENCHMARK(create_set_free_pool)->Apply(pareto_sizes_and_samples);
BENCHMARK(create_set_new)->Apply(pareto_sizes_and_samples);
BENCHMARK(create_set_monotonic)->Apply(pareto_sizes_and_samples);
BENCHMARK(create_set_contiguous_pool)->Apply(pareto_sizes_and_samples<50000>);

// Small vectors: fast > new > monotonic > contiguous > free
BENCHMARK(create_small_vector_fast_pool_shared_ptr)->Apply(pareto_sizes_and_samples<50000>);
BENCHMARK(create_small_vector_fast_pool_new)->Apply(pareto_sizes_and_samples<50000>);
BENCHMARK(create_small_vector_contiguous_pool)->Apply(pareto_sizes_and_samples<50000>);
BENCHMARK(create_small_vector_free_pool)->Apply(pareto_sizes_and_samples);
BENCHMARK(create_small_vector_new)->Apply(pareto_sizes_and_samples);
BENCHMARK(create_small_vector_monotonic)->Apply(pareto_sizes_and_samples);

// Large vector: new > interleaved > free
BENCHMARK(create_large_vector_free_pool)->Apply(pareto_sizes_and_samples<5000>);
BENCHMARK(create_large_vector_contiguous_interleaved_pool)->Apply(pareto_sizes_and_samples<5000>);
BENCHMARK(create_large_vector_new)->Apply(pareto_sizes_and_samples<5000>);

BENCHMARK_MAIN();
