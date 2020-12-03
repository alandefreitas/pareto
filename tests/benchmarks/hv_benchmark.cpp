#include <benchmark/benchmark.h>
#include <pareto/front.h>
#include "../test_helpers.h"

template<size_t dimensions, size_t runtime_dimensions>
typename pareto::front<double, dimensions, unsigned>::point_type random_point() {
    typename pareto::front<double, dimensions, unsigned>::point_type p(runtime_dimensions);
    std::generate(p.begin(), p.end(), randn);
    return p;
}

template<size_t dimensions, size_t runtime_dimensions>
typename pareto::front<double, dimensions, unsigned>::value_type random_value() {
    auto v = std::make_pair<typename pareto::front<double, dimensions, unsigned>::point_type, unsigned>(
            random_point<dimensions, runtime_dimensions>(), randi());
    return v;
}

template<typename FRONT_T, size_t dimensions, size_t runtime_dimensions>
FRONT_T generate_reference_set() {
    if constexpr (dimensions == 0) {
        return FRONT_T(runtime_dimensions);
    } else {
        return FRONT_T();
    }
}

template<size_t dimensions, size_t runtime_dimensions>
pareto::front<double, dimensions, unsigned>
create_test_pareto(size_t target_size) {
    static std::map<size_t, pareto::front<double, dimensions, unsigned>> cache;
    auto it = cache.find(target_size);
    if (it != cache.end()) {
        return it->second;
    }
    auto pf = generate_reference_set<pareto::front<double, dimensions, unsigned>, dimensions, runtime_dimensions>();
    for (size_t i = 0; i < std::max(static_cast<size_t>(1000000), target_size * 100) && pf.size() < target_size; ++i) {
        pf.insert(random_value<dimensions, runtime_dimensions>());
    }
    cache[target_size] = pf;
    return pf;
}

template<size_t dimensions>
void calculate_hypervolume(benchmark::State &state) {
    // using pareto_front_t = pareto::front<double, dimensions, unsigned>;
    // using point_t = typename pareto_front_t::point_type;
    double hv = 0.0;
    static std::map<size_t, double> known_hv;
    if constexpr (dimensions == 0) {
        if (known_hv.find(size_t(state.range(0))) != known_hv.end()) {
            hv = known_hv[state.range(0)];
        }
    }

    for (auto _ : state) {
        state.PauseTiming();
        auto pf = create_test_pareto<dimensions, dimensions>(state.range(0));
        auto nadir = pf.nadir();
        // size_t c = 0;

        state.ResumeTiming();
        if (state.range(1) == 0) {
            if (hv == 0.0) {
                benchmark::DoNotOptimize(hv = pf.hypervolume(nadir));
            }
        } else {
            benchmark::DoNotOptimize(hv = pf.hypervolume(nadir, state.range(1)));
        }
    }

    if constexpr (dimensions == 0) {
        if (known_hv.find(size_t(state.range(0))) == known_hv.end()) {
            known_hv[state.range(0)] = hv;
        }
    }

    state.counters["hv"] = hv;
}

constexpr size_t max_pareto_size = 5000;
constexpr size_t max_number_of_samples = 10000;

void pareto_sizes_and_samples(benchmark::internal::Benchmark *b) {
    for (long long i = 50; i <= static_cast<long long>(max_pareto_size); i *= 10) {
        b->Args({i, 0});
        for (long long j = 100; j <= static_cast<long long>(max_number_of_samples); j *= 10) {
            b->Args({i, j});
        }
    }
}

void pareto_sizes_and_samples2(benchmark::internal::Benchmark *b) {
    for (long long i = 50; i <= 200; i *= 2) {
        b->Args({i, 0});
        for (long long j = 100; j <= static_cast<long long>(max_number_of_samples); j *= 10) {
            b->Args({i, j});
        }
    }
    // if we calculate with n=200, we might as well try to calculate with n=500
    // but that's not usually feasiable though
    b->Args({500, 0});
    for (long long j = 100; j <= static_cast<long long>(max_number_of_samples); j *= 10) {
        b->Args({500, j});
    }
}

size_t number_of_threads = std::thread::hardware_concurrency();

BENCHMARK_TEMPLATE(calculate_hypervolume, 2)->Apply(pareto_sizes_and_samples)->Iterations(1);
#ifdef BUILD_LONG_TESTS
BENCHMARK_TEMPLATE(calculate_hypervolume, 3)->Apply(pareto_sizes_and_samples)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 5)->Apply(pareto_sizes_and_samples)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 9)->Apply(pareto_sizes_and_samples2)->Iterations(1);
BENCHMARK_TEMPLATE(calculate_hypervolume, 9)->Apply(pareto_sizes_and_samples)->Iterations(1);
#endif

BENCHMARK_MAIN();