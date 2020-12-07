#include <benchmark/benchmark.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include <pareto/front.h>
#include <pareto/memory/memory_pool.h>
#include <pareto/point.h>
#include <pareto/tree/r_tree.h>
#ifdef BUILD_BOOST_TREE
#include <pareto/tree/boost_tree.h>
#endif

#include "monotonic_resource.h"
#include "../test_helpers.h"


// Rationale of these experiments
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
    std::set<std::array<double,3>, std::less<>, pareto::fast_memory_pool<std::array<double,3>>> s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_set_free_pool(benchmark::State &state) {
    std::set<std::array<double,3>, std::less<>, pareto::free_memory_pool<std::array<double,3>>> s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_set_new(benchmark::State &state) {
    std::set<std::array<double,3>> s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_set_monotonic([[maybe_unused]] benchmark::State &state) {
    std::pmr::monotonic_buffer_resource rsrc;
    std::pmr::polymorphic_allocator<std::array<double,3>> alloc(&rsrc);
    std::set<std::array<double,3>, std::less<>, std::pmr::polymorphic_allocator<std::array<double,3>>> s(alloc);
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_set_contiguous_pool([[maybe_unused]] benchmark::State &state) {
    std::set<std::array<double,3>, std::less<>, pareto::contiguous_memory_pool<std::array<double,3>>> s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.emplace();
        }
        benchmark::DoNotOptimize(s);
    }
}

void create_small_vector_contiguous_pool([[maybe_unused]] benchmark::State &state) {
    using allocator_type = pareto::contiguous_memory_pool<double>;
    allocator_type alloc;
    using small_vector_type = std::vector<double, allocator_type>;
    std::vector<small_vector_type> v;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            v.emplace_back(alloc);
            v.back().resize(3);
            v.back()[0] = 0;
            v.back()[1] = 1;
            v.back()[2] = 2;
        }
        benchmark::DoNotOptimize(v);
    }
}

void create_shared_pointer_vector_fast_pool([[maybe_unused]] benchmark::State &state) {
    pareto::fast_memory_pool<std::array<double,3>> alloc;
    std::vector<std::shared_ptr<std::array<double,3>>> v;
    v.reserve(state.range(0));
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            auto p = alloc.allocate(1);
            new(p) std::array<double, 3>();
            v.emplace(v.end(), p, [&alloc](auto p) {
                alloc.destroy(p);
                alloc.deallocate(p, 1);
            });
            v.back()->operator[](0) = 0;
            v.back()->operator[](1) = 1;
            v.back()->operator[](2) = 2;
        }
        benchmark::DoNotOptimize(v);
        state.PauseTiming();
        v.clear();
        alloc = pareto::fast_memory_pool<std::array<double,3>>();
        state.ResumeTiming();
    }
}

void create_shared_pointer_vector_new([[maybe_unused]] benchmark::State &state) {
    std::vector<std::shared_ptr<std::array<double,3>>> v;
    v.reserve(state.range(0));
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            v.emplace(v.end(), std::make_shared<std::array<double,3>>());
            v.back()->operator[](0) = 0;
            v.back()->operator[](1) = 1;
            v.back()->operator[](2) = 2;
        }
        benchmark::DoNotOptimize(v);
        state.PauseTiming();
        v.clear();
        state.ResumeTiming();
    }
}

void create_pointer_vector_fast_pool([[maybe_unused]] benchmark::State &state) {
    pareto::fast_memory_pool<std::array<double,3>> alloc;
    std::vector<std::array<double,3>*> v;
    v.reserve(state.range(0));
    for (auto _ : state) {
        long long i = 0;
        for (i = 0; i < state.range(0); ++i) {
            auto p = alloc.allocate(1);
            new(p) std::array<double, 3>();
            v.emplace(v.end(), p);
            v.back()->operator[](0) = i;
            v.back()->operator[](1) = i+1;
            v.back()->operator[](2) = i+2;
        }
        benchmark::DoNotOptimize(v);
        state.PauseTiming();
        for (auto &p : v) {
            alloc.destroy(p);
            alloc.deallocate(p,1);
        }
        v.clear();
        alloc = pareto::fast_memory_pool<std::array<double,3>>();
        state.ResumeTiming();
    }
}

void create_pointer_vector_new([[maybe_unused]] benchmark::State &state) {
    std::allocator<std::array<double,3>> alloc;
    std::vector<std::array<double,3>*> v;
    v.reserve(state.range(0));
    for (auto _ : state) {
        long long i = 0;
        for (i = 0; i < state.range(0); ++i) {
            auto p = alloc.allocate(1);
            new(p) std::array<double, 3>();
            v.emplace(v.end(), p);
            v.back()->operator[](0) = i;
            v.back()->operator[](1) = i+1;
            v.back()->operator[](2) = i+2;
        }
        benchmark::DoNotOptimize(v);
        state.PauseTiming();
        for (auto &p : v) {
            alloc.destroy(p);
            alloc.deallocate(p,1);
        }
        v.clear();
        state.ResumeTiming();
    }
}

void create_small_vector_free_pool([[maybe_unused]] benchmark::State &state) {
    pareto::free_memory_pool<double> alloc;
    using small_vector_type = std::vector<double, pareto::free_memory_pool<double>>;
    std::vector<small_vector_type> v;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            v.emplace_back(alloc);
            v.back().resize(3);
            v.back()[0] = 0;
            v.back()[1] = 1;
            v.back()[2] = 2;
        }
        benchmark::DoNotOptimize(v);
        state.PauseTiming();
        v.clear();
        state.ResumeTiming();
    }
}

void create_small_vector_new([[maybe_unused]] benchmark::State &state) {
    std::allocator<double> alloc;
    using small_vector_type = std::vector<double, std::allocator<double>>;
    std::vector<small_vector_type> v;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            v.emplace_back(alloc);
            v.back().resize(3);
            v.back()[0] = 0;
            v.back()[1] = 1;
            v.back()[2] = 2;
        }
        benchmark::DoNotOptimize(v);
        state.PauseTiming();
        v.clear();
        state.ResumeTiming();
    }
}

void create_small_vector_monotonic([[maybe_unused]] benchmark::State &state) {
    std::pmr::monotonic_buffer_resource rsrc;
    std::pmr::polymorphic_allocator<double> alloc(&rsrc);
    using small_vector_type = std::vector<double, std::pmr::polymorphic_allocator<double>>;
    std::vector<small_vector_type> v;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            v.emplace_back(alloc);
            v.back().resize(3);
            v.back()[0] = 0;
            v.back()[1] = 1;
            v.back()[2] = 2;
        }
        benchmark::DoNotOptimize(v);
        state.PauseTiming();
        v.clear();
        state.ResumeTiming();
    }
}

void create_large_vector_contiguous_interleaved_pool([[maybe_unused]] benchmark::State &state) {
    std::vector<double, pareto::contiguous_memory_pool<double>> s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.emplace_back(i);
        }
        benchmark::DoNotOptimize(s);
        state.PauseTiming();
        s.clear();
        state.ResumeTiming();
    }
}

void create_large_vector_free_pool([[maybe_unused]] benchmark::State &state) {
    std::vector<double, pareto::free_memory_pool<double>> s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.emplace_back(i);
        }
        benchmark::DoNotOptimize(s);
        state.PauseTiming();
        s.clear();
        state.ResumeTiming();
    }
}

void create_large_vector_new([[maybe_unused]] benchmark::State &state) {
    std::vector<double> s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.emplace_back(i);
        }
        benchmark::DoNotOptimize(s);
        state.PauseTiming();
        s.clear();
        state.ResumeTiming();
    }
}


void create_rtree_fast_pool([[maybe_unused]] benchmark::State &state) {
    using tree_type = pareto::r_tree<double, 3, unsigned, pareto::fast_memory_pool>;
    tree_type s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.insert(std::make_pair(tree_type::point_type({static_cast<double>(i),static_cast<double>(i),static_cast<double>(i)}), static_cast<unsigned>(i)));
        }
        benchmark::DoNotOptimize(s);
        state.PauseTiming();
        s.clear();
        state.ResumeTiming();
    }
}

void create_rtree_std_allocator([[maybe_unused]] benchmark::State &state) {
    using tree_type = pareto::r_tree<double, 3, unsigned, std::allocator>;
    tree_type s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.insert(std::make_pair(tree_type::point_type({static_cast<double>(i),static_cast<double>(i),static_cast<double>(i)}), static_cast<unsigned>(i)));
        }
        benchmark::DoNotOptimize(s);
        state.PauseTiming();
        s.clear();
        state.ResumeTiming();
    }
}

void create_rtree_runtime_fast_pool([[maybe_unused]] benchmark::State &state) {
    using tree_type = pareto::r_tree<double, 0, unsigned, pareto::fast_memory_pool>;
    tree_type s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.insert(std::make_pair(tree_type::point_type({static_cast<double>(i),static_cast<double>(i),static_cast<double>(i)}), static_cast<unsigned>(i)));
        }
        benchmark::DoNotOptimize(s);
        state.PauseTiming();
        s.clear();
        state.ResumeTiming();
    }
}

void create_rtree_runtime_std_allocator([[maybe_unused]] benchmark::State &state) {
    using tree_type = pareto::r_tree<double, 0, unsigned, std::allocator>;
    tree_type s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.insert(std::make_pair(tree_type::point_type({static_cast<double>(i),static_cast<double>(i),static_cast<double>(i)}), static_cast<unsigned>(i)));
        }
        benchmark::DoNotOptimize(s);
        state.PauseTiming();
        s.clear();
        state.ResumeTiming();
    }
}

#ifdef BUILD_BOOST_TREE
void create_rtree_boost([[maybe_unused]] benchmark::State &state) {
    using tree_type = pareto::boost_tree<double, 3, unsigned>;
    tree_type s;
    for (auto _ : state) {
        for (long long i = 0; i < state.range(0); ++i) {
            s.insert(std::make_pair(tree_type::point_type({static_cast<double>(i),static_cast<double>(i),static_cast<double>(i)}), static_cast<unsigned>(i)));
        }
        benchmark::DoNotOptimize(s);
        state.PauseTiming();
        s.clear();
        state.ResumeTiming();
    }
}
#endif

constexpr size_t max_container_size = 5000000;


template <size_t MAX_SIZE = max_container_size>
void container_sizes(benchmark::internal::Benchmark* b) {
#ifdef BUILD_LONG_TESTS
    constexpr size_t max_size_adjusted = MAX_SIZE;
#else
    constexpr size_t max_size_adjusted = 500;
#endif
    for (long long i = 50; i <= static_cast<long long>(max_size_adjusted); i *= 10) {
        b->Args({i});
    }
}

// R-Trees
BENCHMARK(create_rtree_fast_pool)->Apply(container_sizes<500000>);
BENCHMARK(create_rtree_std_allocator)->Apply(container_sizes<500000>);
BENCHMARK(create_rtree_runtime_fast_pool)->Apply(container_sizes<500000>);
BENCHMARK(create_rtree_runtime_std_allocator)->Apply(container_sizes<500000>);
#ifdef BUILD_BOOST_TREE
BENCHMARK(create_rtree_boost)->Apply(container_sizes<500000>);
#endif

//// Sets: fast > free > new > monotonic > contiguous
//BENCHMARK(create_set_fast_pool)->Apply(container_sizes);
//BENCHMARK(create_set_free_pool)->Apply(container_sizes);
//BENCHMARK(create_set_new)->Apply(container_sizes);
//BENCHMARK(create_set_monotonic)->Apply(container_sizes);
//BENCHMARK(create_set_contiguous_pool)->Apply(container_sizes<50000>);
//
//// Small vectors: fast > new > monotonic > contiguous > free
//BENCHMARK(create_small_vector_contiguous_pool)->Apply(container_sizes<50000>);
//BENCHMARK(create_small_vector_free_pool)->Apply(container_sizes<500000>);
//BENCHMARK(create_small_vector_new)->Apply(container_sizes);
//BENCHMARK(create_small_vector_monotonic)->Apply(container_sizes);

// Allocating pointer on a vector
BENCHMARK(create_shared_pointer_vector_fast_pool)->Apply(container_sizes<500000>);
BENCHMARK(create_shared_pointer_vector_new)->Apply(container_sizes<500000>);
BENCHMARK(create_pointer_vector_fast_pool)->Apply(container_sizes<500000>);
BENCHMARK(create_pointer_vector_new)->Apply(container_sizes<500000>);

// Large vector: new > interleaved > free
BENCHMARK(create_large_vector_free_pool)->Apply(container_sizes<5000>);
BENCHMARK(create_large_vector_contiguous_interleaved_pool)->Apply(container_sizes<5000>);
BENCHMARK(create_large_vector_new)->Apply(container_sizes<5000>);

BENCHMARK_MAIN();
