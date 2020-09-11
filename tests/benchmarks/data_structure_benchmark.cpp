#include <algorithm>
#include <benchmark/benchmark.h>
#include <iostream>
#include <pareto_front/front.h>
#include <random>
#include <thread>
#include <vector>

std::mt19937 &generator() {
    static std::mt19937 g(
            (std::random_device()()) | std::chrono::high_resolution_clock::now().time_since_epoch().count());
    return g;
}

bool rand_flip() {
    static std::uniform_int_distribution<unsigned> ud(0, 1);
    return ud(generator());
}

unsigned randi(size_t low = 0, size_t high = 40) {
    static std::uniform_int_distribution<unsigned> ud(low, high);
    return ud(generator());
}

double randu() {
    static std::uniform_real_distribution<double> ud(0., 1.);
    return ud(generator());
}

double randn() {
    static std::normal_distribution nd;
    return nd(generator());
}

template <size_t dimensions, typename TAG>
typename pareto::front<double, dimensions, unsigned, TAG>::point_type random_point() {
    typename pareto::front<double, dimensions, unsigned, TAG>::point_type p(dimensions);
    std::generate(p.begin(), p.end(), randn);
    return p;
};

template <size_t dimensions, typename TAG>
typename pareto::front<double, dimensions, unsigned, TAG>::value_type random_value() {
    auto v = std::make_pair<typename pareto::front<double, dimensions, unsigned, TAG>::point_type, unsigned>(random_point<dimensions, TAG>(), randi());
    return v;
};

template <size_t dimensions, typename TAG>
pareto::front<double, dimensions, unsigned, TAG>
create_test_pareto(size_t target_size) {
    pareto::front<double, dimensions, unsigned, TAG> pf;
    for (size_t i = 0; i < std::max(static_cast<size_t>(100000), target_size*100) && pf.size() < target_size; ++i) {
        pf.insert(random_value<dimensions, TAG>());
    }
    return pf;
}

template <size_t dimensions, typename TAG>
std::vector<typename pareto::front<double, dimensions, unsigned, TAG>::value_type>
create_vector_with_values(size_t n) {
    std::vector<typename pareto::front<double, dimensions, unsigned, TAG>::value_type> v;
    v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        v.emplace_back(random_value<dimensions, TAG>());
    }
    return v;
}

template <size_t dimensions, typename TAG>
void create_front_from_vector(benchmark::State &state) {
    size_t n = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        auto v = create_vector_with_values<dimensions, TAG>(n);

        state.ResumeTiming();
        benchmark::DoNotOptimize(pareto::front<double, dimensions, unsigned, TAG>(v.begin(), v.end()));
    }
}

template <size_t dimensions, typename TAG>
void insert_in_front(benchmark::State &state) {
    // using pareto_front_t = pareto::front<double, dimensions, unsigned, TAG>;
    // using point_t = typename pareto_front_t::point_type;
    size_t n = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        auto pf = create_test_pareto<dimensions, TAG>(n);
        auto to_insert = create_vector_with_values<dimensions, TAG>(n);

        state.ResumeTiming();
        for (size_t i = 0; i < n; ++i) {
            benchmark::DoNotOptimize(pf.insert(to_insert[i]));
        }
    }
}

template <size_t dimensions, typename TAG>
void erase_from_front(benchmark::State &state) {
    // using pareto_front_t = pareto::front<double, dimensions, unsigned, TAG>;
    // using point_t = typename pareto_front_t::point_type;
    size_t n = state.range(0);

    for (auto _ : state) {
        state.PauseTiming();
        auto pf = create_test_pareto<dimensions, TAG>(n);
        std::vector<typename pareto::front<double, dimensions, unsigned, TAG>::value_type> to_erase;
        to_erase.reserve(n);
        for (const auto& v: pf) {
            to_erase.emplace_back(v);
        }
        std::shuffle(to_erase.begin(), to_erase.end(), generator());

        state.ResumeTiming();
        for (size_t i = 0; i < n; ++i) {
            benchmark::DoNotOptimize(pf.erase(to_erase[i].first));
        }
    }
}

template <size_t dimensions, typename TAG>
void check_dominance(benchmark::State &state) {
    // using pareto_front_t = pareto::front<double, dimensions, unsigned, TAG>;
    // using point_t = typename pareto_front_t::point_type;
    auto pf = create_test_pareto<dimensions, TAG>(state.range(0));

    for (auto _ : state) {
        state.PauseTiming();
        auto p = random_point<dimensions, TAG>();

        state.ResumeTiming();
        benchmark::DoNotOptimize(pf.dominates(p));
    }
}

template <size_t dimensions, typename TAG>
void query_and_iterate(benchmark::State &state) {
    // using pareto_front_t = pareto::front<double, dimensions, unsigned, TAG>;
    // using point_t = typename pareto_front_t::point_type;
    auto pf = create_test_pareto<dimensions, TAG>(state.range(0));
    size_t c = 0;

    for (auto _ : state) {
        state.PauseTiming();
        auto p1 = random_point<dimensions, TAG>();
        // auto p2 = random_point<dimensions, TAG>();

        state.ResumeTiming();
        auto it = pf.find_intersection(p1,p1);
        for (; it != pf.end(); ++it) {
            benchmark::DoNotOptimize(c += 1);
        }
    }
}

template <size_t dimensions, typename TAG>
void nearest_and_iterate(benchmark::State &state) {
    // using pareto_front_t = pareto::front<double, dimensions, unsigned, TAG>;
    // using point_t = typename pareto_front_t::point_type;
    auto pf = create_test_pareto<dimensions, TAG>(state.range(0));
    size_t c = 0;

    for (auto _ : state) {
        state.PauseTiming();
        auto p = random_point<dimensions, TAG>();

        state.ResumeTiming();
        auto it = pf.find_nearest(p,5);
        for (; it != pf.end(); ++it) {
            benchmark::DoNotOptimize(c += 1);
        }
    }
}

template <size_t dimensions, typename TAG>
void calculate_hypervolume(benchmark::State &state) {
    // using pareto_front_t = pareto::front<double, dimensions, unsigned, TAG>;
    // using point_t = typename pareto_front_t::point_type;
    for (auto _ : state) {
        state.PauseTiming();
        auto pf = create_test_pareto<dimensions, TAG>(state.range(0));
        auto nadir = pf.nadir();
        // size_t c = 0;

        state.ResumeTiming();
        if (state.range(1) == 0) {
            benchmark::DoNotOptimize(pf.hypervolume(nadir));
        } else {
            benchmark::DoNotOptimize(pf.hypervolume(nadir, state.range(1)));
        }
    }
}

template <typename FRONT_T, size_t dimensions>
FRONT_T generate_reference_set() {
    if constexpr (dimensions == 0) {
        return FRONT_T(dimensions);
    } else {
        return FRONT_T();
    }
}

template <size_t dimensions, typename TAG>
void calculate_igd(benchmark::State &state) {
    using pareto_front_t = pareto::front<double, dimensions, unsigned, TAG>;
    // using point_t = typename pareto_front_t::point_type;
    for (auto _ : state) {
        state.PauseTiming();
        auto pf = create_test_pareto<dimensions, TAG>(state.range(0));
        std::vector v(pf.begin(), pf.end());
        auto reference_set = generate_reference_set<pareto_front_t,dimensions>();
        for (auto &[k, v] : v) {
            auto k2 = k - 2.0;
            reference_set.insert({k2,v});
        }
        // size_t c = 0;
        state.ResumeTiming();
        benchmark::DoNotOptimize(pf.igd(reference_set));
    }
}

constexpr size_t max_pareto_size = 5000;
void pareto_sizes(benchmark::internal::Benchmark* b) {
    for (size_t i = 50; i <= max_pareto_size; i *= 10) {
        b->Args({static_cast<long long>(i)});
    }
}

constexpr size_t max_number_of_samples = 10000;
void pareto_sizes_and_samples(benchmark::internal::Benchmark* b) {
    for (long long i = 50; i <= static_cast<long long>(max_pareto_size); i *= 10) {
        // b->Args({i,0});
        for (long long j = 100; j <= static_cast<long long>(max_number_of_samples); j *= 10) {
            b->Args({i,j});
        }
    }
}

void pareto_sizes_and_samples2(benchmark::internal::Benchmark* b) {
    for (long long i = 50; i <= 200; i *= 2) {
        // b->Args({i,0});
        for (long long j = 100; j <= static_cast<long long>(max_number_of_samples); j *= 10) {
            b->Args({i,j});
        }
    }
}

size_t number_of_threads = std::thread::hardware_concurrency();

/// Creating front: varying data structure, dimension and vector size
using pareto::vector_tree_tag;
using pareto::kd_tree_tag;
using pareto::quad_tree_tag;
using pareto::boost_tree_tag;
using pareto::r_tree_tag;
using pareto::r_star_tree_tag;

#ifdef BUILD_LONG_TESTS
/// Creating front: varying data structure, dimension, front size, and number of operations
BENCHMARK_TEMPLATE(create_front_from_vector, 1, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 1, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 1, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 1, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 1, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 1, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_TEMPLATE(create_front_from_vector, 2, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 2, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 2, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 2, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 2, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 2, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(create_front_from_vector, 3, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 3, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 3, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 3, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 3, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 3, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(create_front_from_vector, 5, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 5, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 5, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 5, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 5, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 5, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(create_front_from_vector, 9, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 9, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 9, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 9, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 9, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 9, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(create_front_from_vector, 13, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 13, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 13, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 13, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 13, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(create_front_from_vector, 13, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

/// Inserting in the front: varying data structure, dimension, front size, and number of operations
BENCHMARK_TEMPLATE(insert_in_front, 1, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 1, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 1, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 1, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 1, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 1, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_TEMPLATE(insert_in_front, 2, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 2, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 2, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 2, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 2, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 2, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(insert_in_front, 3, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 3, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 3, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 3, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 3, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 3, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(insert_in_front, 5, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 5, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 5, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 5, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 5, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 5, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(insert_in_front, 9, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 9, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 9, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 9, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 9, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 9, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(insert_in_front, 13, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 13, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 13, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 13, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 13, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(insert_in_front, 13, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);


/// Erasing from the front: varying data structure, dimension, front size, and number of operations
BENCHMARK_TEMPLATE(erase_from_front, 1, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 1, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 1, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 1, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 1, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 1, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_TEMPLATE(erase_from_front, 2, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 2, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 2, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 2, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 2, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 2, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(erase_from_front, 3, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 3, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 3, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 3, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 3, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 3, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(erase_from_front, 5, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 5, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 5, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 5, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 5, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 5, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(erase_from_front, 9, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 9, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 9, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 9, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 9, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 9, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(erase_from_front, 13, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 13, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 13, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 13, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 13, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(erase_from_front, 13, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);


/// Check dominance: varying data structure, dimension, front size, and number of operations
BENCHMARK_TEMPLATE(check_dominance, 1, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 1, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 1, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 1, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 1, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 1, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_TEMPLATE(check_dominance, 2, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 2, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 2, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 2, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 2, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 2, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(check_dominance, 3, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 3, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 3, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 3, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 3, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 3, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(check_dominance, 5, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 5, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 5, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 5, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 5, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 5, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(check_dominance, 9, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 9, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 9, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 9, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 9, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 9, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(check_dominance, 13, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 13, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 13, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 13, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 13, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(check_dominance, 13, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);


/// Querying and iterating: varying data structure, dimension, and front size
BENCHMARK_TEMPLATE(query_and_iterate, 1, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 1, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 1, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 1, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 1, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 1, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_TEMPLATE(query_and_iterate, 2, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 2, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 2, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 2, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 2, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 2, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(query_and_iterate, 3, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 3, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 3, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 3, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 3, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 3, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(query_and_iterate, 5, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 5, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 5, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 5, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 5, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 5, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(query_and_iterate, 9, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 9, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 9, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 9, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 9, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 9, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(query_and_iterate, 13, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 13, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 13, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 13, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 13, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(query_and_iterate, 13, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

/// Find nearest and iterate: varying data structure, dimension, and front size
BENCHMARK_TEMPLATE(nearest_and_iterate, 1, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 1, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 1, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 1, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 1, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 1, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_TEMPLATE(nearest_and_iterate, 2, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 2, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 2, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 2, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 2, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 2, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(nearest_and_iterate, 3, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 3, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 3, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 3, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 3, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 3, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(nearest_and_iterate, 5, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 5, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 5, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 5, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 5, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 5, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(nearest_and_iterate, 9, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 9, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 9, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 9, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 9, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 9, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(nearest_and_iterate, 13, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 13, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 13, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 13, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 13, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(nearest_and_iterate, 13, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);


/// IGD: varying data structure, dimension, and front size
BENCHMARK_TEMPLATE(calculate_igd, 1, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 1, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 1, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 1, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 1, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 1, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_TEMPLATE(calculate_igd, 2, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 2, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 2, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 2, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 2, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 2, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(calculate_igd, 3, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 3, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 3, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 3, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 3, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 3, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(calculate_igd, 5, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 5, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 5, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 5, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 5, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 5, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(calculate_igd, 9, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 9, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 9, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 9, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 9, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 9, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(calculate_igd, 13, vector_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 13, kd_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 13, quad_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 13, boost_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 13, r_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_igd, 13, r_star_tree_tag)->Apply(pareto_sizes)->Threads(number_of_threads)->Iterations(1);


/// Hypervolume: varying data structure, dimension, front size, and number of samples
/// The exact hypervolume function is not thread safe
BENCHMARK_TEMPLATE(calculate_hypervolume, 1, vector_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 1, kd_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 1, quad_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 1, boost_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 1, r_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 1, r_star_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_TEMPLATE(calculate_hypervolume, 2, vector_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 2, kd_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 2, quad_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 2, boost_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 2, r_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 2, r_star_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);

#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(calculate_hypervolume, 3, vector_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 3, kd_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 3, quad_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 3, boost_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 3, r_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 3, r_star_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(calculate_hypervolume, 5, vector_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 5, kd_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 5, quad_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 5, boost_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 5, r_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 5, r_star_tree_tag)->Apply(pareto_sizes_and_samples)->Threads(number_of_threads)->Iterations(1);

BENCHMARK_TEMPLATE(calculate_hypervolume, 9, vector_tree_tag)->Apply(pareto_sizes_and_samples2)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 9, kd_tree_tag)->Apply(pareto_sizes_and_samples2)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 9, quad_tree_tag)->Apply(pareto_sizes_and_samples2)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 9, boost_tree_tag)->Apply(pareto_sizes_and_samples2)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 9, r_tree_tag)->Apply(pareto_sizes_and_samples2)->Threads(number_of_threads)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 9, r_star_tree_tag)->Apply(pareto_sizes_and_samples2)->Threads(number_of_threads)->Iterations(1);
#endif

BENCHMARK_MAIN();

