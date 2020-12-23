
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

#include <pareto/front.h>

template <size_t COMPILE_DIMENSION, typename Container>
void test_front(size_t RUNTIME_DIMENSION = COMPILE_DIMENSION,
                const std::vector<bool> &is_mini = {}) {
    size_t test_dimension =
        COMPILE_DIMENSION != 0 ? COMPILE_DIMENSION : RUNTIME_DIMENSION;
    using namespace pareto;
    using front_type =
        front<double, COMPILE_DIMENSION, unsigned, Container>;
    using point_type = typename front_type::key_type;
    using value_type = typename front_type::value_type;

    SECTION("Constructors") {
        // dimensions
        front_type pf1;
        // dimensions and single direction for all dimensions
        front_type pf2({true});
        // each direction (infer dimension from number of directions)
        front_type pf3({}, is_mini.begin(), is_mini.end());
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
        front_type pf4(v.begin(), v.end());
        // iterators to initial elements and a single direction for all dimensions
        front_type pf5(v.begin(), v.end(), {false});
        // iterators and directions
        front_type pf6(v.begin(), v.end(), is_mini.begin(), is_mini.end());
        // copy constructor
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        front_type pf7(pf6);
        // move constructor
        front_type pf8(std::move(pf7));
        if constexpr (COMPILE_DIMENSION == 0 || COMPILE_DIMENSION == 2) {
            if (test_dimension == 2) {
                // from initializer list
                front_type pf9({{{2.6, 3.4}, 6}, {{6.5, 2.4}, 4}});
                // from initializer list and direction
                front_type pf10({{{2.6, 3.4}, 6}, {{6.5, 2.4}, 4}}, {max, min});
            }
        }
        // from vector
        front_type pf11(v.begin(), v.end());
        // from vector and direction
        front_type pf12(v.begin(), v.end(), {false});
        // from vector and directions
        front_type pf13(v.begin(), v.end(), is_mini.begin(), is_mini.end());
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

    auto random_pareto_front = [&]() {
        front_type pf({}, is_mini.begin(), is_mini.end());

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

        pf.insert(std::make_pair(p1, 2));
        pf.insert(std::make_pair(p2, 3));

        pf.emplace(random_value());
        std::vector v = {random_value(), random_value(), random_value()};
        pf.insert(v.begin(), v.end());
        auto r = pf.insert(random_value());
        value_type v2 = random_value();
        r = pf.insert(std::move(v2));
        r = pf.insert(std::make_pair(random_point(), randi()));
        unsigned m = randi();
        r = pf.insert(std::make_pair(random_point(), m));
        std::vector v3 = {random_value(), random_value(), random_value()};
        pf.insert(v3.begin(), v3.end());
        pf.insert({random_value(), random_value(), random_value()});
        for (size_t i = 0; i < 1000 / test_dimension; ++i) {
            pf.insert(random_value());
        }
        REQUIRE(pf.check_invariants());
        return pf;
    };

    SECTION("Container functions and iterators") {
        auto pf = random_pareto_front();
        size_t counter = 0;
        for ([[maybe_unused]] const auto &[k, v] : pf) {
            ++counter;
        }
        REQUIRE(counter == pf.size());
        counter = 0;
        for ([[maybe_unused]] auto &[k, v] : pf) {
            ++counter;
        }
        REQUIRE(counter == pf.size());
        REQUIRE_FALSE(pf.empty());
        bool b = pf.dimensions() == test_dimension;
        REQUIRE(b);
        point_type p2 = pf.begin()->first;
        REQUIRE(pf.find(p2) != pf.end());
        REQUIRE(pf.contains(p2));
        REQUIRE(pf.find(random_point()) == pf.end());
        REQUIRE(!pf.contains(random_point()));
        pf.clear();
        REQUIRE(pf.empty());
        REQUIRE(pf.size() == 0); // NOLINT(readability-container-size-empty)
        REQUIRE(pf.dimensions() == test_dimension);
    }

    // Erasing items
    SECTION("Erasing") {
        auto pf = random_pareto_front();
        auto pf2 = pf;
        REQUIRE(pf == pf2);
        REQUIRE(pf.size() == pf2.size());
        // erase by point / key
        size_t s = pf2.size();
        pf2.erase(pf2.begin()->first);
        REQUIRE(pf2.size() == s - 1);
        pf2.insert(random_value());
        s = pf2.size();
        // erase by iterator
        pf2.erase(pf2.begin());
        REQUIRE(pf2.size() == s - 1);
        pf2.insert(random_value());
        // erase by iterator
        typename front_type::iterator first = pf2.begin();
        typename front_type::iterator last = pf2.end();
        pf2.erase(first, last);
        REQUIRE(pf2.size() == 0); // NOLINT(readability-container-size-empty)
        REQUIRE(pf2.empty());
        pf2 = pf;
        REQUIRE_FALSE(pf2.empty());
        pf2.clear();
        REQUIRE(pf2.empty());
    }

    SECTION("Merging / swapping fronts") {
        auto pf = random_pareto_front();
        front_type pf2({}, is_mini.begin(), is_mini.end());
        for (size_t i = 0; i < 100; ++i) {
            pf2.insert(random_value());
        }
        front_type pf3 = pf;
        REQUIRE_FALSE(pf.dominates(pf3));
        pf3.merge(pf2);
        REQUIRE_FALSE(pf2.dominates(pf3));
        size_t pfs1 = pf.size();
        size_t pfs2 = pf2.size();
        pf.swap(pf2);
        REQUIRE(pfs1 == pf2.size());
        REQUIRE(pfs2 == pf.size());
    }

    SECTION("Queries") {
        auto pf = random_pareto_front();
        auto p = random_point();
        auto ideal_ = pf.ideal();
        for (auto it = pf.find_intersection(p, ideal_); it != pf.end(); ++it) {
            for (size_t i = 0; i < test_dimension; ++i) {
                REQUIRE(it->first[i] <= std::max(ideal_[i], p[i]));
                REQUIRE(it->first[i] >= std::min(ideal_[i], p[i]));
            }
        }
        for (auto it = pf.find_within(p, ideal_); it != pf.end(); ++it) {
            for (size_t i = 0; i < test_dimension; ++i) {
                REQUIRE(it->first[i] < std::max(ideal_[i], p[i]));
                REQUIRE(it->first[i] > std::min(ideal_[i], p[i]));
            }
        }
        for (auto it = pf.find_disjoint(p, ideal_); it != pf.end(); ++it) {
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
        for (auto it = pf.find_nearest(p); it != pf.end(); ++it) {
            for (auto &[k, v] : pf) {
                REQUIRE(p.distance(k) >= p.distance(it->first));
            }
        }
        for (auto it = pf.find_nearest(p, 5); it != pf.end(); ++it) {
            size_t c = 0;
            for (auto &[k, v] : pf) {
                if (p.distance(k) < p.distance(it->first)) {
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
        auto pf = random_pareto_front();
        // Get indicators and check if they are in acceptable ranges
        if (pf.size() > 2) {
            REQUIRE(pf.hypervolume(pf.nadir()) >= 0);
            REQUIRE(pf.hypervolume(10, pf.nadir()) >= 0);
            REQUIRE(pf.hypervolume(100, pf.nadir()) >= 0);
            REQUIRE(pf.hypervolume(1000, pf.nadir()) >= 0);
            REQUIRE(pf.hypervolume(10000, pf.nadir()) >= 0);
            REQUIRE(pf.hypervolume(100000, pf.nadir()) >= 0);
            // Compare set coverage
            front_type pf_b({}, is_mini.begin(), is_mini.end());
            for (size_t i = 0; i < 1000 / test_dimension; ++i) {
                pf_b.insert({random_point(), randi()});
            }
            REQUIRE(pf.coverage(pf_b) >= 0);
            REQUIRE(pf.coverage(pf_b) <= 1);
            REQUIRE(pf_b.coverage(pf) >= 0);
            REQUIRE(pf_b.coverage(pf) <= 1);
            if (pf_b.coverage(pf) != 0.) {
                REQUIRE(pf.coverage_ratio(pf_b) >= 0);
            }
            // Convergence metrics
            // Create a mock reference set / "real" pareto front
            front_type pf_c({}, is_mini.begin(), is_mini.end());
            // std::cout << "Mock front: " << pf_c << std::endl;
            for (const auto &[k, v] : pf) {
                point_type p = k;
                for (size_t i = 0; i < p.dimensions(); ++i) {
                    p[i] += is_mini[i] ? -0.5 : 0.5;
                }
                pf_c.insert(std::make_pair(p, v));
            }
            REQUIRE(pf.gd(pf_c) >= 0.);
            REQUIRE(pf.std_gd(pf_c) >= 0.);
            REQUIRE(pf.igd(pf_c) >= 0.);
            REQUIRE(pf.std_igd(pf_c) >= 0.);
            REQUIRE(pf.hausdorff(pf_c) > 0.);
            REQUIRE(pf.igd_plus(pf_c) >= 0.);
            REQUIRE(pf.std_igd_plus(pf_c) >= 0.);

            if (pf.dimensions() > 1) {
                // Distribution and spread
                REQUIRE(pf.uniformity() > 0);
                REQUIRE(pf.average_distance() > 0);
                REQUIRE(pf.average_nearest_distance(1) > 0);
                REQUIRE(pf.average_nearest_distance(2) > 0);
                REQUIRE(pf.average_nearest_distance(5) > 0);
                REQUIRE(pf.average_crowding_distance() > 0);
            }
        }
    }

    SECTION("Pareto Dominance") {
        auto pf = random_pareto_front();
        // Point dominance
        REQUIRE_NOTHROW(pf.dominates(random_point()));
        point_type p = pf.begin()->first;
        for (size_t i = 0; i < p.dimensions(); ++i) {
            p[i] -= (is_mini[i] ? 1 : -1);
        }
        REQUIRE_FALSE(pf.dominates(p));
        REQUIRE_FALSE(pf.strongly_dominates(p));
        REQUIRE_FALSE(pf.non_dominates(p));
        for (size_t i = 0; i < p.dimensions(); ++i) {
            p[i] += 2 * (is_mini[i] ? 1 : -1);
        }
        REQUIRE(pf.dominates(p));
        REQUIRE(pf.strongly_dominates(p));
        REQUIRE_FALSE(pf.non_dominates(p));
        // Pareto dominance
        front_type pf2 = pf;
        REQUIRE_FALSE(pf.dominates(pf2));
        REQUIRE_FALSE(pf.strongly_dominates(pf2));
        REQUIRE(pf.non_dominates(pf));
        REQUIRE(pf.non_dominates(pf2));
        REQUIRE_FALSE(pf2.dominates(pf));
        REQUIRE_FALSE(pf2.strongly_dominates(pf));
        REQUIRE(pf2.non_dominates(pf));
        std::vector<value_type> v(pf.begin(), pf.end());
        pf2.clear();
        for (const auto &[k, v2] : v) {
            point_type k2 = k;
            for (size_t i = 0; i < p.dimensions(); ++i) {
                k2[i] -= (is_mini[i] ? 1 : -1);
            }
            pf2.emplace(k2, v2);
        }
        REQUIRE_FALSE(pf.dominates(pf2));
        REQUIRE_FALSE(pf.strongly_dominates(pf2));
        REQUIRE_FALSE(pf.non_dominates(pf2));
        REQUIRE(pf2.dominates(pf));
        REQUIRE(pf2.strongly_dominates(pf));
        REQUIRE_FALSE(pf2.non_dominates(pf));
        pf2.clear();
        for (auto &[k, v2] : v) {
            point_type unprotected_k = k;
            for (size_t i = 0; i < k.dimensions(); ++i) {
                unprotected_k[i] = unprotected_k[i] + (is_mini[i] ? 2 : -2);
            }
            pf2.emplace(unprotected_k, v2);
        }
        REQUIRE(pf.dominates(pf2));
        REQUIRE(pf.strongly_dominates(pf2));
        REQUIRE_FALSE(pf.non_dominates(pf2));
        REQUIRE_FALSE(pf2.dominates(pf));
        REQUIRE_FALSE(pf2.strongly_dominates(pf));
        REQUIRE_FALSE(pf2.non_dominates(pf));
    }

    SECTION("Reference points") {
        auto pf = random_pareto_front();
        point_type ideal_ = pf.ideal();
        for (const auto &[k, v] : pf) {
            REQUIRE_FALSE(k.dominates(ideal_, is_mini));
        }
        point_type nadir_ = pf.nadir();
        for (const auto &[k, v] : pf) {
            REQUIRE_FALSE(nadir_.dominates(k, is_mini));
        }
        point_type worst_ = pf.worst();
        for (const auto &[k, v] : pf) {
            REQUIRE_FALSE(worst_.dominates(k, is_mini));
        }
        REQUIRE(nadir_ == worst_);

        REQUIRE(pf <= worst_);
        REQUIRE(ideal_ <= pf);
    }
}

template <bool runtime, template <class,size_t,class,class,class> typename Container> void test_all_dimensions() {
    using K = double;
    using T = unsigned;
    using C = std::less<K>;


#ifdef BUILD_LONG_TESTS
    SECTION("1 dimension") {
        using namespace pareto;
        SECTION("Direction {0}") {
            constexpr size_t M = runtime ? 0 : 1;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(1, {0});
        }
    }
#endif

    SECTION("2 dimensions") {
        using namespace pareto;
        SECTION("Direction {0, 1}") {
            constexpr size_t M = runtime ? 0 : 2;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(2, {0, 1});
        }
        SECTION("Direction {0, 0}") {
            constexpr size_t M = runtime ? 0 : 2;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(2, {0, 0});
        }
        SECTION("Direction {1, 0}") {
            constexpr size_t M = runtime ? 0 : 2;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(2, {1, 0});
        }
        SECTION("Direction {1, 1}") {
            constexpr size_t M = runtime ? 0 : 2;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(2, {1, 1});
        }
    }

    SECTION("3 dimensions") {
        using namespace pareto;
        SECTION("Direction {0, 1, 0}") {
            constexpr size_t M = runtime ? 0 : 3;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(3, {0, 1, 0});
        }
        SECTION("Direction {0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 3;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(3, {0, 0, 0});
        }
        SECTION("Direction {1, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 3;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(3, {1, 0, 0});
        }
    }

#ifdef BUILD_LONG_TESTS
    SECTION("5 dimensions") {
        using namespace pareto;
        SECTION("Direction {0, 0, 1, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 5;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(5, {0, 0, 1, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 5;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(5, {0, 0, 0, 0, 0});
        }
        SECTION("Direction {1, 0, 0, 1, 0}") {
            constexpr size_t M = runtime ? 0 : 5;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(5, {1, 0, 0, 1, 0});
        }
        SECTION("Direction {0, 0, 0, 1, 0}") {
            constexpr size_t M = runtime ? 0 : 5;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(5, {0, 0, 0, 1, 0});
        }
    }

    SECTION("9 dimensions") {
        using namespace pareto;
        SECTION("Direction {0, 0, 0, 0, 0, 0, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(9, {0, 0, 0, 0, 0, 0, 0, 0, 1});
        }
        SECTION("Direction {0, 0, 0, 1, 0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(9, {0, 0, 0, 1, 0, 0, 0, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(9, {0, 0, 0, 0, 0, 0, 0, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 1, 1, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(9, {0, 0, 0, 0, 1, 1, 0, 0, 1});
        }
        SECTION("Direction {0, 0, 0, 0, 0, 1, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 9;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(9, {0, 0, 0, 0, 0, 1, 0, 0, 1});
        }
    }

    SECTION("13 dimensions") {
        using namespace pareto;

        SECTION("Direction {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(
                13, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        }
        SECTION("Direction {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(
                13, {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1});
        }
        SECTION("Direction {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(
                13, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(
                13, {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0});
        }
        SECTION("Direction {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1}") {
            constexpr size_t M = runtime ? 0 : 13;
            using A = std::allocator<std::pair<const ::pareto::point<K, M>, T>>;
            test_front<M, Container<K,M,T,C,A>>(
                13, {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1});
        }
    }
#endif
}

#ifdef implicit_TREETAG
TEST_CASE("Implicit-Front") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::implicit_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::implicit_tree>();
    }
}
#elif quad_TREETAG
TEST_CASE("Quad-Front") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::quad_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::quad_tree>();
    }
}
#elif kd_TREETAG
TEST_CASE("kd-Front") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::kd_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::kd_tree>();
    }
}
#elif boost_TREETAG
TEST_CASE("Boost-Front") {
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::boost_tree>();
    }
}
#elif r_TREETAG
TEST_CASE("R-Front") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::r_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::r_tree>();
    }
}
#elif r_star_TREETAG
TEST_CASE("R*-Front") {
    SECTION("Runtime Dimension") {
        test_all_dimensions<true, pareto::r_star_tree>();
    }
    SECTION("Compile Time Dimension") {
        test_all_dimensions<false, pareto::r_star_tree>();
    }
}
#endif