#include <benchmark/benchmark.h>
#include <pareto/front.h>
#include <pareto/implicit_tree.h>
#include <pareto/kd_tree.h>
#include <pareto/quad_tree.h>
#include <pareto/r_star_tree.h>
#include <pareto/r_tree.h>
#ifdef BUILD_BOOST_TREE
#include <pareto/boost_tree.h>
#endif
#include "../test_helpers.h"

/// \brief Construct a front
/// Functors allow us to pass functions as template template parameters
template<size_t COMPILE_DIMENSION, class Container>
struct construct {
    void operator()(benchmark::State &state) const {
        size_t n = state.range(0);
        for (auto _ : state) {
            state.PauseTiming();
            auto v = create_vector_with_values<COMPILE_DIMENSION, Container>(n);
            state.ResumeTiming();
            benchmark::DoNotOptimize(pareto::front<double, COMPILE_DIMENSION, unsigned, Container>(v.begin(), v.end()));
        }
    }
};

/// \brief Insert in a front
/// Functors allow us to pass functions as template template parameters
template<size_t COMPILE_DIMENSION, class Container>
struct insert {
    void operator()(benchmark::State &state) const {
        size_t n = state.range(0);
        for (auto _ : state) {
            state.PauseTiming();
            auto pf = create_test_pareto<COMPILE_DIMENSION, Container>(n);
            state.ResumeTiming();
            benchmark::DoNotOptimize(pf.insert(random_value<COMPILE_DIMENSION, Container>()));
        }
    }
};

/// \brief erase in a front
/// Functors allow us to pass functions as template template parameters
template<size_t COMPILE_DIMENSION, class Container>
struct erase {
    void operator()(benchmark::State &state) const {
        auto n = static_cast<size_t>(state.range(0));
        for (auto _ : state) {
            state.PauseTiming();
            auto pf = create_test_pareto<COMPILE_DIMENSION, Container>(n);
            auto reference_p = random_point<COMPILE_DIMENSION, Container>();
            auto p = pf.find_nearest(reference_p);
            state.ResumeTiming();
            benchmark::DoNotOptimize(pf.erase(p));
        }
    }
};

/// \brief Check point-front dominance in a front
/// Functors allow us to pass functions as template template parameters
template<size_t COMPILE_DIMENSION, class Container>
struct check_dominance {
    void operator()(benchmark::State &state) const {
        for (auto _ : state) {
            state.PauseTiming();
            auto pf = create_test_pareto<COMPILE_DIMENSION, Container>(state.range(0));
            auto p = random_point<COMPILE_DIMENSION, Container>();
            state.ResumeTiming();
            benchmark::DoNotOptimize(pf.dominates(p));
        }
    }
};

/// \brief query_intersection in a front
/// Functors allow us to pass functions as template template parameters
template<size_t COMPILE_DIMENSION, class Container>
struct query_intersection {
    void operator()(benchmark::State &state) const {
        for (auto _ : state) {
            state.PauseTiming();
            auto pf = create_test_pareto<COMPILE_DIMENSION, Container>(state.range(0));
            auto p1 = random_point<COMPILE_DIMENSION, Container>();
            state.ResumeTiming();
            auto it = pf.find_intersection(p1,p1);
            benchmark::DoNotOptimize(it != pf.end());
        }
    }
};

/// \brief Query nearest in a front
/// Functors allow us to pass functions as template template parameters
template<size_t COMPILE_DIMENSION, class Container>
struct query_nearest {
    void operator()(benchmark::State &state) const {
        for (auto _ : state) {
            state.PauseTiming();
            auto pf = create_test_pareto<COMPILE_DIMENSION, Container>(state.range(0));
            auto p = random_point<COMPILE_DIMENSION, Container>();
            state.ResumeTiming();
            auto it = pf.find_nearest(p);
            benchmark::DoNotOptimize(it != pf.end());
        }
    }
};

/// \brief Calculate front hypervolume
/// Functors allow us to pass functions as template template parameters
template<size_t COMPILE_DIMENSION, class Container>
struct hypervolume {
    void operator()(benchmark::State &state) const {
        for (auto _ : state) {
            state.PauseTiming();
            auto pf = create_test_pareto<COMPILE_DIMENSION, Container>(state.range(0));
            auto nadir = pf.nadir();
            // size_t c = 0;
            state.ResumeTiming();
            if (state.range(1) == 0) {
                benchmark::DoNotOptimize(pf.hypervolume(nadir));
            } else {
                benchmark::DoNotOptimize(pf.hypervolume(state.range(1), nadir));
            }
        }
    }
};

template <typename FRONT_T, size_t COMPILE_DIMENSION>
FRONT_T generate_reference_set() {
    if constexpr (COMPILE_DIMENSION == 0) {
        return FRONT_T(COMPILE_DIMENSION);
    } else {
        return FRONT_T();
    }
}

/// Functors allow us to pass functions as template template parameters
template<size_t COMPILE_DIMENSION, class Container>
struct igd {
    void operator()(benchmark::State &state) const {
        using pareto_front_t = pareto::front<double, COMPILE_DIMENSION, unsigned, Container>;
        for (auto _ : state) {
            state.PauseTiming();
            auto pf = create_test_pareto<COMPILE_DIMENSION, Container>(state.range(0));
            std::vector v(pf.begin(), pf.end());
            auto reference_set = generate_reference_set<pareto_front_t,COMPILE_DIMENSION>();
            for (auto &[k, v2] : v) {
                auto k2 = k - 2.0;
                reference_set.insert({k2, v2});
            }
            // size_t c = 0;
            state.ResumeTiming();
            benchmark::DoNotOptimize(pf.igd(reference_set));
        }
    }
};

/// Factors for benchmarks
constexpr size_t max_pareto_size = 5000;
void pareto_sizes(benchmark::internal::Benchmark* b) {
    for (size_t i = 50; i <= max_pareto_size; i *= 10) {
        b->Args({static_cast<long long>(i)});
    }
}

/// Factors for hypervolume benchmarks
constexpr size_t max_number_of_samples = 10000;
void pareto_sizes_and_samples(benchmark::internal::Benchmark* b) {
    for (long long i = 50; i <= static_cast<long long>(max_pareto_size); i *= 10) {
        for (long long j = 100; j <= static_cast<long long>(max_number_of_samples); j *= 10) {
            b->Args({i,j});
        }
    }
}

const size_t number_of_threads = std::thread::hardware_concurrency();

template <class F, class S>
auto register_bench(const std::string& name, F functor, S s) {
    auto r = benchmark::RegisterBenchmark(name.c_str(), functor);
    r->Apply(s)->Threads(static_cast<int>(number_of_threads))->Iterations(1);
}

template <size_t M, template <size_t,class> class F, bool is_boost_benchmark, class S>
auto register_all_containers(const std::string& name, S state_values) {
    if constexpr (!is_boost_benchmark) {
        register_bench(name + ",implicit_tree>", F<M,pareto::implicit_tree<double,M,unsigned>>(), state_values);
        register_bench(name + ",quad_tree>", F<M,pareto::quad_tree<double,M,unsigned>>(), state_values);
        register_bench(name + ",kd_tree>", F<M,pareto::kd_tree<double,M,unsigned>>(), state_values);
        register_bench(name + ",r_tree>", F<M,pareto::r_tree<double,M,unsigned>>(), state_values);
        register_bench(name + ",r_star_tree>", F<M,pareto::r_star_tree<double,M,unsigned>>(), state_values);
    }
#ifdef BUILD_BOOST_TREE
    else {
        // register_bench(name + ",boost_tree>", F<M,pareto::boost_tree<double,M,unsigned>>(), state_values);
    }
#endif
}

template <size_t M, bool is_hypervolume_benchmark, bool is_boost_benchmark>
void register_all_functions() {
    if constexpr (!is_hypervolume_benchmark) {
        register_all_containers<M, construct, is_boost_benchmark>("construct<m=" + std::to_string(M), pareto_sizes);
        register_all_containers<M, insert, is_boost_benchmark>("insert<m=" + std::to_string(M), pareto_sizes);
        register_all_containers<M, erase, is_boost_benchmark>("erase<m=" + std::to_string(M), pareto_sizes);
        register_all_containers<M, check_dominance, is_boost_benchmark>("check_dominance<m=" + std::to_string(M), pareto_sizes);
        register_all_containers<M, query_intersection, is_boost_benchmark>("query_intersection<m=" + std::to_string(M), pareto_sizes);
        register_all_containers<M, query_nearest, is_boost_benchmark>("query_nearest<m=" + std::to_string(M), pareto_sizes);
        register_all_containers<M, igd, is_boost_benchmark>("igd<m=" + std::to_string(M), pareto_sizes);
    } else {
        register_all_containers<M, hypervolume, is_boost_benchmark>("hypervolume<m=" + std::to_string(M), pareto_sizes_and_samples);
    }
}

template <bool is_hypervolume_benchmark, bool is_boost_benchmark>
auto register_all_dimensions() {
#ifdef BUILD_LONG_TESTS
    register_all_functions<1,is_hypervolume_benchmark,is_boost_benchmark>();
#endif
    register_all_functions<2,is_hypervolume_benchmark,is_boost_benchmark>();
    register_all_functions<3,is_hypervolume_benchmark,is_boost_benchmark>();
#ifdef BUILD_LONG_TESTS
    register_all_functions<5,is_hypervolume_benchmark,is_boost_benchmark>();
    register_all_functions<7,is_hypervolume_benchmark,is_boost_benchmark>();
    register_all_functions<9,is_hypervolume_benchmark,is_boost_benchmark>();
    register_all_functions<13,is_hypervolume_benchmark,is_boost_benchmark>();
#endif
}

int main(int argc, char** argv) {
    /*
     * We use metaprogramming to register the tests programmatically
     * because we are varying the dimensions, the functions and the
     * data structures passed as template arguments.
     *
     * We register benchmarks 4 times with different flags to
     * run the experiments in the following order:
     * * All functions except hypervolume
     * * Hypervolume function
     * * All functions except hypervolume with boost r-tree
     * * Hypervolume function with boost r-tree
     *
     * The reason for this order is that hypervolume benchmarks
     * take a lot longer than other benchmarks. Boost r-trees also
     * take a lot longer than other trees so we put these benchmarks
     * last because it was getting in the way of other experiments.
     * We were never getting the data from other containers just
     * waiting for boost.geometry on a few operations that it doesn't
     * do well. These are probably the last benchmarks with boost r-trees
     * because some operations just take too long there and it's been
     * more than enough to just prove a point. In the future, we can
     * compare with boost with specific benchmarks instead of maintaining
     * this huge wrapper.
     *
     * Within each group, we follow this order:
     * * Smaller dimensions first
     * * Then each function
     * * Then each data structure
     *
     */
    // All containers + all functions
    register_all_dimensions<false,false>();
    // All containers + hypervolume
    register_all_dimensions<true,false>();
    // Boost + all functions
    // register_all_dimensions<false,true>();
    // Boost + hypervolume
    // register_all_dimensions<true,true>();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}