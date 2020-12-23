
#include "../test_helpers.h"
#include <catch2/catch.hpp>
#include <pareto/common/demangle.h>
#ifdef implicit_TREETAG
#include <pareto/implicit_tree.h>
#elif quad_TREETAG
#include <pareto/quad_tree.h>
#elif kd_TREETAG
#include <pareto/kd_tree.h>
#elif boost_TREETAG
#include <pareto/boost_tree.h>
#elif r_TREETAG
#include <pareto/r_tree.h>
#elif r_star_TREETAG
#include <pareto/r_star_tree.h>
#endif

#include <pareto/archive.h>

template <size_t COMPILE_DIMENSION, typename Container>
void test_archive(size_t RUNTIME_DIMENSION = COMPILE_DIMENSION,
                  const std::vector<bool> &is_mini = {}) {
    size_t test_dimension =
        COMPILE_DIMENSION != 0 ? COMPILE_DIMENSION : RUNTIME_DIMENSION;
    using namespace pareto;
    using archive_type =
        archive<double, COMPILE_DIMENSION, unsigned, Container>;
    using point_type = typename archive_type::key_type;
    using value_type = typename archive_type::value_type;
    constexpr size_t max_size = 100;

    SECTION("Constructors") {
        // dimensions
        archive_type ar1(max_size);
        // dimensions and single direction for all dimensions
        archive_type ar2(max_size, {true});
        // each direction (infer dimension from number of directions)
        archive_type ar3(max_size, {}, is_mini.begin(), is_mini.end());
        // iterators
        point_type p1(test_dimension);
        point_type p2(test_dimension);
        for (size_t i = 0; i < test_dimension; ++i) {
            p1[i] = 2.5 + 1. * i;
            p2[i] = 2.5 - 1. * i;
        }
        auto v1 = std::make_pair(p1, 2);
        auto v2 = std::make_pair(p2, 5);
        std::vector<value_type> v = {v1, v2};
        archive_type ar4(max_size, v.begin(), v.end());
        // iterators and direction
        archive_type ar5(max_size, v.begin(), v.end(), {false});
        // iterators and directions
        archive_type ar6(max_size, v.begin(), v.end(), is_mini.begin(),
                         is_mini.end());
        // copy constructor
        archive_type ar7(ar6);
        // move constructor
        archive_type ar8(std::move(ar7));
        if constexpr (COMPILE_DIMENSION == 0 || COMPILE_DIMENSION == 2) {
            if (test_dimension == 2) {
                // initializer list
                archive_type ar9(max_size, {{{2.6, 3.4}, 6}, {{6.5, 2.4}, 4}});
                // initializer list and direction
                archive_type ar10(max_size, {{{2.6, 3.4}, 6}, {{6.5, 2.4}, 4}},
                                  {max, min});
            }
        }
        // vector
        archive_type ar11(max_size, v.begin(), v.end());
        // vector and direction
        archive_type ar12(max_size, v.begin(), v.end(), {false});
        // vector and directions
        archive_type ar13(max_size, v.begin(), v.end(), is_mini.begin(),
                          is_mini.end());
    }

    auto random_point = [&]() {
        point_type p(test_dimension);
        std::generate(p.begin(), p.end(), randn);
        return p;
    };

    auto random_value = [&]() {
        auto v = std::make_pair<point_type, unsigned>(random_point(), randi());
        return v;
    };

    auto random_pareto_archive = [&]() {
        archive_type ar(max_size, {}, is_mini.begin(), is_mini.end());
        point_type p1(test_dimension);
        point_type p2(test_dimension);
        for (size_t i = 0; i < test_dimension; ++i) {
            p1[i] = 2.5 + 1. * i;
            p2[i] = 1.5 + test_dimension - 1. * i;
            if (!is_mini[i]) {
                // make distribution symmetric
                // for tests
                p1[i] = -p1[i];
                p2[i] = -p2[i];
            }
        }

        ar.insert(std::make_pair(p1, 2));
        ar.insert(std::make_pair(p2, 3));

        ar.emplace(random_value());
        std::vector v = {random_value(), random_value(), random_value()};
        ar.insert(v.begin(), v.end());
        auto r = ar.insert(random_value());
        value_type v2 = random_value();
        r = ar.insert(std::move(v2));
        r = ar.insert(std::make_pair(random_point(), randi()));
        unsigned m = randi();
        r = ar.emplace(random_point(), m);
        std::vector v3 = {random_value(), random_value(), random_value()};
        REQUIRE(ar.check_invariants());
        ar.insert(v3.begin(), v3.end());
        ar.insert({random_value(), random_value(), random_value()});
        REQUIRE(ar.check_invariants());
        for (size_t i = 0; i < 1000 / test_dimension; ++i) {
            auto x = random_value();
            try {
                ar.insert(x);
            } catch (const std::bad_variant_access &e) {
                std::cout << e.what() << std::endl;
                std::cout << "i - " << i << " - v: [" << x.first << ", "
                          << x.second << "]" << std::endl;
            }
            if (!ar.check_invariants()) {
                std::cerr << "i: " << i << std::endl;
                REQUIRE(ar.check_invariants());
                break;
            }
        }
        return ar;
    };

    SECTION("Container functions and iterators") {
        // RUNTIME_DIMENSION = {size_t} 2’
        // test_dimension = {size_t} 2
        auto ar = random_pareto_archive();
        size_t counter = 0;
        for (auto fit = ar.begin_front(); fit != ar.end_front(); ++fit) {
            for (auto fitit = fit->begin(); fitit != fit->end(); ++fitit) {
                ++counter;
            }
        }
        REQUIRE(ar.size() == counter);
        counter = 0;
        for ([[maybe_unused]] auto &[k, v] : ar) {
            ++counter;
        }
        REQUIRE(counter == ar.size());
        REQUIRE((ar.size() > 10));
        REQUIRE_FALSE(ar.empty());
        REQUIRE(ar.dimensions() == test_dimension);

        point_type p2 = ar.begin()->first;
        REQUIRE(ar.find(p2) != ar.end());

        REQUIRE(ar.contains(p2));
        REQUIRE(ar.find(random_point()) == ar.end());
        REQUIRE(!ar.contains(random_point()));
        ar.clear();
        REQUIRE(ar.empty());
        REQUIRE(ar.size() == 0); // NOLINT(readability-container-size-empty)
        REQUIRE(ar.dimensions() == test_dimension);
    }

    SECTION("Erasing") {
        auto ar = random_pareto_archive();
        auto ar2 = ar;
        REQUIRE(ar == ar2);
        REQUIRE(ar.size() == ar2.size());
        // erase by point / key
        size_t s = ar2.size();
        REQUIRE(ar2.size() == ar2.total_front_sizes());
        ar2.erase(ar2.begin()->first);
        REQUIRE(ar2.size() == ar2.total_front_sizes());
        REQUIRE(ar.size() == ar.total_front_sizes());
        REQUIRE(ar2.check_invariants());
        REQUIRE(ar2.size() == s - 1);
        ar2.insert(random_value());
        s = ar2.size();
        // erase by iterator
        ar2.erase(ar2.begin());
        REQUIRE(ar2.size() == s - 1);
        ar2.insert(random_value());
        // erase by iterator
        ar2.erase(ar2.begin(), ar2.end());
        REQUIRE(ar2.size() == 0); // NOLINT(readability-container-size-empty)
        REQUIRE(ar2.empty());
        ar2 = ar;
        REQUIRE_FALSE(ar2.empty());
        ar2.clear();
        REQUIRE(ar2.empty());
    }

    SECTION("Merging / swapping archives") {
        auto ar = random_pareto_archive();
        archive_type ar2(max_size, {}, is_mini.begin(), is_mini.end());
        for (size_t i = 0; i < 100; ++i) {
            ar2.insert(random_value());
        }
        archive_type ar3 = ar;
        REQUIRE_FALSE(ar.dominates(ar3));
        ar3.merge(ar2);
        REQUIRE_FALSE(ar2.dominates(ar3));
        size_t ars1 = ar.size();
        size_t ars2 = ar2.size();
        ar.swap(ar2);
        REQUIRE(ars1 == ar2.size());
        REQUIRE(ars2 == ar.size());
    }

    SECTION("Queries") {
        auto ar = random_pareto_archive();
        auto p = random_point();
        auto ideal_ = ar.ideal();
        for (auto it = ar.find_intersection(p, ideal_); it != ar.end(); ++it) {
            for (size_t i = 0; i < test_dimension; ++i) {
                REQUIRE(it->first[i] <= std::max(ideal_[i], p[i]));
                REQUIRE(it->first[i] >= std::min(ideal_[i], p[i]));
            }
        }
        for (auto it = ar.find_within(p, ar.ideal()); it != ar.end(); ++it) {
            for (size_t i = 0; i < test_dimension; ++i) {
                REQUIRE(it->first[i] < std::max(ideal_[i], p[i]));
                REQUIRE(it->first[i] > std::min(ideal_[i], p[i]));
            }
        }
        for (auto it = ar.find_disjoint(p, ar.ideal()); it != ar.end(); ++it) {
            bool outside_the_box = false;
            for (size_t i = 0; i < test_dimension; ++i) {
                if (it->first[i] > std::max(ideal_[i], p[i])) {
                    outside_the_box = true;
                    break;
                }
                if (it->first[i] < std::min(ideal_[i], p[i])) {
                    outside_the_box = true;
                    break;
                }
            }
            REQUIRE(outside_the_box);
        }
        for (auto it = ar.find_nearest(p); it != ar.end(); ++it) {
            for (auto &[k, v] : ar) {
                if (!(p.distance(k) >= p.distance(it->first))) {
                    std::cout << "p: " << p << std::endl;
                    std::cout << "k: " << k << std::endl;
                    std::cout << "it->first: " << it->first << std::endl;
                    std::cout << "p.distance(k): " << p.distance(k) << std::endl;
                    std::cout << "p.distance(it->first): " << p.distance(it->first) << std::endl;
                    REQUIRE(p.distance(k) >= p.distance(it->first));
                }
            }
        }
        for (auto it = ar.find_nearest(p, 5); it != ar.end(); ++it) {
            size_t c = 0;
            for (auto &[key, v] : ar) {
                if (p.distance(key) < p.distance(it->first)) {
                    ++c;
                }
            }
            REQUIRE_FALSE(c > 5);
        }
    }

    SECTION("Indicators") {
        if (RUNTIME_DIMENSION > 5) {
            return;
        }
        auto ar = random_pareto_archive();
        // Get indicators and check if they are in acceptable ranges
        if (ar.size() > 2 && ar.begin_front()->size() > 2) {
            REQUIRE(ar.hypervolume(ar.nadir()) >= 0);
            REQUIRE(ar.hypervolume(10, ar.nadir()) >= 0);
            REQUIRE(ar.hypervolume(100, ar.nadir()) >= 0);
            REQUIRE(ar.hypervolume(1000, ar.nadir()) >= 0);
            REQUIRE(ar.hypervolume(10000, ar.nadir()) >= 0);
            REQUIRE(ar.hypervolume(100000, ar.nadir()) >= 0);
            // Compare set coverage
            archive_type ar_b(max_size, {}, is_mini.begin(), is_mini.end());
            for (size_t i = 0; i < 1000; ++i) {
                ar_b.insert({random_point(), randi()});
            }
            REQUIRE(ar.coverage(ar_b) >= 0);
            REQUIRE(ar.coverage(ar_b) <= 1);
            REQUIRE(ar_b.coverage(ar) >= 0);
            REQUIRE(ar_b.coverage(ar) <= 1);
            if (ar_b.coverage(ar) != 0.) {
                REQUIRE(ar.coverage_ratio(ar_b) >= 0);
            }
            // Convergence metrics
            archive_type pf_c(max_size, {}, is_mini.begin(), is_mini.end());
            for (const auto &[k, v] : *ar.begin_front()) {
                point_type p = k;
                for (size_t i = 0; i < p.dimensions(); ++i) {
                    p[i] += is_mini[i] ? -0.5 : 0.5;
                }
                pf_c.emplace(p, v);
            }
            REQUIRE(ar.gd(pf_c) >= 0.);
            REQUIRE(ar.std_gd(pf_c) >= 0.);
            REQUIRE(ar.igd(pf_c) >= 0.);
            REQUIRE(ar.std_igd(pf_c) >= 0.);
            REQUIRE(ar.hausdorff(pf_c) >= 0.);
            REQUIRE(ar.igd_plus(pf_c) >= 0.);
            REQUIRE(ar.std_igd_plus(pf_c) >= 0.);
            // Distribution and spread
            REQUIRE(ar.uniformity() >= 0);
            REQUIRE(ar.average_distance() >= 0);
            REQUIRE(ar.average_nearest_distance(1) >= 0);
            REQUIRE(ar.average_nearest_distance(2) >= 0);
            REQUIRE(ar.average_nearest_distance(5) >= 0);
            REQUIRE(ar.average_crowding_distance() >= 0);
        }
    }

    SECTION("Pareto Dominance") {
        auto ar = random_pareto_archive();
        REQUIRE(ar.check_invariants());
        // Point dominance
        REQUIRE_NOTHROW(ar.dominates(random_point()));
        point_type p = ar.begin()->first;
        for (size_t i = 0; i < p.dimensions(); ++i) {
            p[i] -= (is_mini[i] ? 1 : -1);
        }
        REQUIRE_FALSE(ar.dominates(p));
        REQUIRE_FALSE(ar.strongly_dominates(p));
        REQUIRE_FALSE(ar.non_dominates(p));
        for (size_t i = 0; i < p.dimensions(); ++i) {
            p[i] += 2 * (is_mini[i] ? 1 : -1);
        }
        REQUIRE(ar.dominates(p));
        REQUIRE(ar.strongly_dominates(p));
        REQUIRE_FALSE(ar.non_dominates(p));
        // Pareto dominance
        archive_type ar2 = ar;
        REQUIRE_FALSE(ar.dominates(ar2));
        REQUIRE_FALSE(ar.strongly_dominates(ar2));
        REQUIRE(ar.non_dominates(ar));
        REQUIRE(ar.non_dominates(ar2));
        REQUIRE_FALSE(ar2.dominates(ar));
        REQUIRE_FALSE(ar2.strongly_dominates(ar));
        REQUIRE(ar2.non_dominates(ar));
        std::vector<value_type> v(ar.begin(), ar.end());
        ar2.clear();
        for (const auto &[k, v2] : v) {
            point_type k2 = k;
            for (size_t i = 0; i < p.dimensions(); ++i) {
                k2[i] -= (is_mini[i] ? 1 : -1);
            }
            ar2.emplace(k2, v2);
        }
        REQUIRE(ar2.check_invariants());
        REQUIRE_FALSE(ar.dominates(ar2));
        REQUIRE_FALSE(ar.strongly_dominates(ar2));
        REQUIRE_FALSE(ar.non_dominates(ar2));
        REQUIRE(ar2.dominates(ar));
        REQUIRE(ar2.strongly_dominates(ar));
        REQUIRE_FALSE(ar2.non_dominates(ar));
        ar2.clear();
        for (auto &[k, v2] : v) {
            point_type unprotected_k = k;
            for (size_t i = 0; i < k.dimensions(); ++i) {
                unprotected_k[i] = k[i] + (is_mini[i] ? 2 : -2);
            }
            ar2.emplace(unprotected_k, v2);
        }
        REQUIRE(ar.dominates(ar2));
        REQUIRE(ar.strongly_dominates(ar2));
        REQUIRE_FALSE(ar.non_dominates(ar2));
        REQUIRE_FALSE(ar2.dominates(ar));
        REQUIRE_FALSE(ar2.strongly_dominates(ar));
        REQUIRE_FALSE(ar2.non_dominates(ar));
    }

    SECTION("Reference points") {
        auto ar = random_pareto_archive();
        point_type ideal_ = ar.ideal();
        for (const auto &[k, v] : ar) {
            REQUIRE_FALSE(k.dominates(ideal_, is_mini));
        }
        point_type worst_ = ar.worst();
        for (const auto &[k, v] : ar) {
            REQUIRE_FALSE(worst_.dominates(k, is_mini));
        }
        point_type nadir_ = ar.nadir();
        REQUIRE_FALSE(worst_.dominates(nadir_, is_mini));
    }
}

template <bool runtime,
          template <class, size_t, class, class, class> typename Container>
void test_all_dimensions() {
    using K = double;
    using T = unsigned;
    using C = std::less<K>;

    SECTION("1 dimension") {
        using namespace pareto;
        SECTION("Direction {0}") {
            constexpr size_t M = runtime ? 0 : 1;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(1, {0});
        }
    }

    SECTION("2 dimensions") {
        using namespace pareto;
        SECTION("Direction {0, 1}") {
            constexpr size_t M = runtime ? 0 : 2;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(2, {0, 1});
        }
        SECTION("Direction {0, 0}") {
            constexpr size_t M = runtime ? 0 : 2;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(2, {0, 0});
        }
        SECTION("Direction {1, 0}") {
            constexpr size_t M = runtime ? 0 : 2;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(2, {1, 0});
        }
        SECTION("Direction {1, 1}") {
            constexpr size_t M = runtime ? 0 : 2;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(2, {1, 1});
        }
    }

    SECTION("3 dimensions") {
        using namespace pareto;
        SECTION("Direction {0, 1, 0}") {
            constexpr size_t M = runtime ? 0 : 3;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(3, {0, 1, 0});
        }
        SECTION("Direction {0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 3;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(3, {0, 0, 0});
        }
        SECTION("Direction {1, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 3;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(3, {1, 0, 0});
        }
    }

#ifdef BUILD_LONG_TESTS
    SECTION("5 dimensions") {
        using namespace pareto;
        SECTION("Direction {0, 0, 1, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 5;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(5, {0, 0, 1, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 5;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(5, {0, 0, 0, 0, 0});
        }
        SECTION("Direction {1, 0, 0, 1, 0}") {
            constexpr size_t M = runtime ? 0 : 5;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(5, {1, 0, 0, 1, 0});
        }
        SECTION("Direction {0, 0, 0, 1, 0}") {
            constexpr size_t M = runtime ? 0 : 5;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(5, {0, 0, 0, 1, 0});
        }
    }

    SECTION("9 dimensions") {
        using namespace pareto;
        SECTION("Direction {0, 0, 0, 0, 0, 0, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                9, {0, 0, 0, 0, 0, 0, 0, 0, 1});
        }
        SECTION("Direction {0, 0, 0, 1, 0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                9, {0, 0, 0, 1, 0, 0, 0, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                9, {0, 0, 0, 0, 0, 0, 0, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 1, 1, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                9, {0, 0, 0, 0, 1, 1, 0, 0, 1});
        }
        SECTION("Direction {0, 0, 0, 0, 0, 1, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                9, {0, 0, 0, 0, 0, 1, 0, 0, 1});
        }
    }

    SECTION("13 dimensions") {
        using namespace pareto;

        SECTION("Direction {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                13, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        }
        SECTION("Direction {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                13, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1});
        }
        SECTION("Direction {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                13, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                13, {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_archive<M, Container<K, M, T, C, A>>(
                13, {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1});
        }
    }
#endif
}

#ifdef implicit_TREETAG
TEST_CASE("Implicit-Archive") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::implicit_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::implicit_tree>();
    }
}
#elif quad_TREETAG
TEST_CASE("Quad-Archive") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::quad_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::quad_tree>();
    }
}
#elif kd_TREETAG
TEST_CASE("kd-Archive") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::kd_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::kd_tree>();
    }
}
#elif boost_TREETAG
TEST_CASE("Boost-Archive") {
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::boost_tree>();
    }
}
#elif r_TREETAG
TEST_CASE("R-Archive") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::r_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::r_tree>();
    }
}
#elif r_star_TREETAG
TEST_CASE("R*-Archive") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::r_star_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::r_star_tree>();
    }
}
#endif