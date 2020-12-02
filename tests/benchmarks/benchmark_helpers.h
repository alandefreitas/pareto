//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_BENCHMARK_HELPERS_H
#define PARETO_BENCHMARK_HELPERS_H

#include <iostream>
#include <algorithm>
#include <random>
#include <thread>
#include <vector>

uint64_t seed() {
//    static uint64_t seed = static_cast<uint64_t>(std::random_device()()) |
    static uint64_t seed = static_cast<unsigned int>(std::random_device()()) |
                           static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::cout << "Test seed: " << seed << std::endl;
    return seed;
}

std::mt19937 &generator() {
    static std::mt19937 g(seed());
    return g;
}

bool rand_flip() {
    static std::uniform_int_distribution<unsigned> ud(0, 1);
    return ud(generator());
}

inline unsigned randi(size_t low = 0, size_t high = 40) {
    static std::uniform_int_distribution<unsigned> ud(static_cast<unsigned>(low), static_cast<unsigned>(high));
    return ud(generator());
}

inline double randu() {
    static std::uniform_real_distribution<double> ud(0., 1.);
    return ud(generator());
}

inline double randn() {
    static std::normal_distribution nd;
    return nd(generator());
}

template<size_t dimensions, typename TAG>
typename pareto::front<double, dimensions, unsigned, TAG>::point_type random_point() {
    typename pareto::front<double, dimensions, unsigned, TAG>::point_type p(dimensions);
    std::generate(p.begin(), p.end(), randn);
    return p;
}

template<size_t dimensions, typename TAG>
typename pareto::front<double, dimensions, unsigned, TAG>::value_type random_value() {
    auto v = std::make_pair<typename pareto::front<double, dimensions, unsigned, TAG>::point_type, unsigned>(
            random_point<dimensions, TAG>(), randi());
    return v;
}

template<size_t dimensions, typename TAG>
pareto::front<double, dimensions, unsigned, TAG>
create_test_pareto(size_t target_size) {
    pareto::front<double, dimensions, unsigned, TAG> pf;
    for (size_t i = 0; i < std::max(static_cast<size_t>(100000), target_size * 100) && pf.size() < target_size; ++i) {
        pf.insert(random_value<dimensions, TAG>());
    }
    return pf;
}

template<size_t dimensions, typename TAG>
std::vector<typename pareto::front<double, dimensions, unsigned, TAG>::value_type>
create_vector_with_values(size_t n) {
    std::vector<typename pareto::front<double, dimensions, unsigned, TAG>::value_type> v;
    v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        v.emplace_back(random_value<dimensions, TAG>());
    }
    return v;
}

#endif //PARETO_BENCHMARK_HELPERS_H
