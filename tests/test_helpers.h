//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_TEST_HELPERS_H
#define PARETO_TEST_HELPERS_H

#include <algorithm>
#include <iostream>
#include <pareto/front.h>
#include <random>
#include <thread>
#include <vector>
#ifdef INCLUDE_MATPLOT
#include <pareto/matplot/archive.h>
#endif


/// \brief A fixed seed for tests
/// We do want predictable sequences sometimes (unit tests)
constexpr uint64_t fixed_seed() { return 3957603322; }

inline uint64_t seed() {
    static uint64_t seed =
        static_cast<unsigned int>(std::random_device()()) |
        static_cast<unsigned int>(std::chrono::high_resolution_clock::now()
                                      .time_since_epoch()
                                      .count());
    std::cout << "Test seed: " << seed << std::endl;
    return seed;
}

inline std::mt19937 &generator() {
    // NOLINT(cert-msc51-cpp)
    static std::mt19937 g(fixed_seed());
    return g;
}

inline bool rand_flip() {
    static std::uniform_int_distribution<unsigned> ud(0, 1);
    return ud(generator());
}

inline unsigned randi(size_t low = 0, size_t high = 40) {
    static std::uniform_int_distribution<unsigned> ud(
        static_cast<unsigned>(low), static_cast<unsigned>(high));
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

template <size_t dimensions, typename Container>
typename pareto::front<double, dimensions, unsigned, Container>::key_type
random_point() {
    typename pareto::front<double, dimensions, unsigned, Container>::key_type p(
        dimensions);
    std::generate(p.begin(), p.end(), randn);
    return p;
}

template <size_t dimensions, typename Container>
typename pareto::front<double, dimensions, unsigned, Container>::value_type
random_value() {
    auto v = std::make_pair<typename pareto::front<double, dimensions, unsigned,
                                                   Container>::key_type,
                            unsigned>(random_point<dimensions, Container>(),
                                      randi());
    return v;
}

template <size_t dimensions, typename Container>
void fill_test_pareto(
    pareto::front<double, dimensions, unsigned, Container> &pf,
    size_t target_size) {
    for (size_t i = 0;
         i < std::max(static_cast<size_t>(100000), target_size * 100) &&
         pf.size() < target_size;
         ++i) {
        pf.insert(random_value<dimensions, Container>());
        if (pf.size() >= target_size) {
            break;
        }
    }
}

template <size_t dimensions, typename Container>
pareto::front<double, dimensions, unsigned, Container>
create_test_pareto(size_t target_size) {
    pareto::front<double, dimensions, unsigned, Container> pf;
    fill_test_pareto(pf, target_size);
    return pf;
}

/// Get a pareto front from a cache of fronts
/// There are two reasons for doing this
/// 1) The experiments are paired and fairer
/// 2) We don't waste time with things we are not measuring
template <size_t dimensions, typename Container>
pareto::front<double, dimensions, unsigned, Container>
get_test_pareto_from_cache(size_t target_size, size_t replicate) {
    using front_type = pareto::front<double, dimensions, unsigned, Container>;
    using target_and_replicate_type = std::pair<size_t, size_t>;
    static std::map<target_and_replicate_type, front_type> cache;
    static std::mutex working_with_cache;
    // because we need a lock to access the cache, make sure
    // you are not using this function at a point where the
    // benchmark is measuring time
    working_with_cache.lock();
    target_and_replicate_type k = std::make_pair(target_size, replicate);
    auto it = cache.find(k);
    front_type result;
    if (it == cache.end()) {
        result = create_test_pareto<dimensions, Container>(target_size);
        cache[k] = result;
    } else {
        result = it->second;
    }
    working_with_cache.unlock();
    return result;
}

template <size_t dimensions, typename Container>
std::vector<
    typename pareto::front<double, dimensions, unsigned, Container>::value_type>
create_vector_with_values(size_t n) {
    std::vector<typename pareto::front<double, dimensions, unsigned,
                                       Container>::value_type>
        v;
    v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        v.emplace_back(random_value<dimensions, Container>());
    }
    return v;
}

using uint8_t_vector_iterator = std::vector<uint8_t>::iterator;

inline bool next_combination(uint8_t_vector_iterator first,
                             uint8_t_vector_iterator last, uint8_t max_value) {
    using value_type = uint8_t;
    if (first == last)
        return false;
    auto i = last;
    do {
        --i;
        if (*i == max_value) {
            *i = std::numeric_limits<value_type>::min();
        } else {
            ++(*i);
            return true;
        }
    } while (i != first);
    return false;
}

/// \brief Plot archive if matplot++ is available
/// If matplot++ is available, this is a function we can
/// use to debug new features.
template <typename ARCHIVE_TYPE>
[[maybe_unused]] void maybe_plot([[maybe_unused]] const ARCHIVE_TYPE &ar,
                                 [[maybe_unused]] size_t front_idx = 0) {
#ifdef INCLUDE_MATPLOT
    plot_archive(ar, front_idx);
#endif
}


#endif // PARETO_TEST_HELPERS_H
