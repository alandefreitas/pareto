#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <pareto/archive.h>
#include <pareto/front.h>
#include "../test_helpers.h"

template <size_t COMPILE_DIMENSION, typename TAG = pareto::default_tag<COMPILE_DIMENSION>>
void test_archive(size_t RUNTIME_DIMENSION = COMPILE_DIMENSION,
                  const std::vector<uint8_t> &is_mini = {0x00}) {
    size_t test_dimension = COMPILE_DIMENSION != 0 ? COMPILE_DIMENSION : RUNTIME_DIMENSION;
    std::string section_name = std::to_string(test_dimension) + " dimensions - ";
    section_name += std::to_string(COMPILE_DIMENSION) + " compile dimensions - ";
    section_name += std::to_string(RUNTIME_DIMENSION) + " runtime dimensions - ";
    auto type_name = std::string(typeid(TAG).name());
    type_name = std::regex_replace(type_name, std::regex("(N\\d+)pareto([\\d]+)([^E]+)E"), "pareto::$3");
    section_name += type_name + " - ";
    if (is_mini[0]) {
        section_name += "{minimization";
    } else {
        section_name += "{maximization";
    }
    for (size_t i = 1; i < is_mini.size(); ++i) {
        if (is_mini[i]) {
            section_name += ", minimization";
        } else {
            section_name += ", maximization";
        }
    }
    section_name += "}";

    using namespace pareto;
    using archive_t = archive<double, COMPILE_DIMENSION, unsigned, TAG>;
    using point_type = typename archive_t::point_type;
    using value_type = typename archive_t::value_type;
    constexpr size_t max_size = 100;

    SECTION("Constructors " + section_name) {
        // dimensions
        archive_t ar1(max_size);
        // dimensions and direction
        archive_t ar2(max_size, true);
        // each direction (infer dimension from number of directions)
        archive_t ar3(max_size, is_mini);
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
        archive_t ar4(max_size, v.begin(), v.end());
        // iterators and direction
        archive_t ar5(max_size, v.begin(), v.end(), false);
        // iterators and directions
        archive_t ar6(max_size, v.begin(), v.end(), is_mini);
        // copy constructor
        archive_t ar7(ar6);
        // move constructor
        archive_t ar8(std::move(ar7));
        if (test_dimension == 2) {
            // initializer list
            archive_t ar9(max_size, {{{2.6, 3.4}, 6},
                                     {{6.5, 2.4}, 4}});
            // initializer list and direction
            archive_t ar10(max_size, {{{2.6, 3.4}, 6},
                                      {{6.5, 2.4}, 4}}, false);
        }
        // vector
        archive_t ar11(max_size, v);
        // vector and direction
        archive_t ar12(max_size, v, false);
        // vector and directions
        archive_t ar13(max_size, v, is_mini);
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
        archive_t ar(max_size, is_mini);
        point_type p1(test_dimension);
        point_type p2(test_dimension);
        for (size_t i = 0; i < test_dimension; ++i) {
            p1[i] = 2.5 + 1. * i;
            p2[i] = 1.5 + test_dimension - 1. * i;
        }
        ar.insert(p1, 2);
        ar.insert(p2, 3);
        // size_t s = ar.size();
        ar.emplace(random_value());
        std::vector v = {random_value(), random_value(), random_value()};
        ar.emplace(v.begin(), v.end());
        auto r = ar.insert(random_value());
        value_type v2 = random_value();
        r = ar.insert(std::move(v2));
        r = ar.insert(random_point(), randi());
        unsigned m = randi();
        r = ar.insert(random_point(), m);
        std::vector v3 = {random_value(), random_value(), random_value()};
        ar.insert(v3.begin(), v3.end());
        ar.insert({random_value(), random_value(), random_value()});
        for (size_t i = 0; i < 1000 / test_dimension; ++i) {
            auto x = random_value();
            try {
                ar.insert(x);
            } catch (const std::bad_variant_access &e) {
                std::cout << e.what() << std::endl;
                std::cout << "i - " << i << " - v: [" << x.first << ", " << x.second << "]" << std::endl;
            }
        }
        REQUIRE(ar.check_invariants());
        return ar;
    };

    SECTION("Container functions and iterators " + section_name) {
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

    SECTION ("Erasing") {
        auto ar = random_pareto_archive();
        auto ar2 = ar;
        REQUIRE(ar == ar2);
        REQUIRE(ar.size() == ar2.size());
        // erase by point / key
        size_t s = ar2.size();
        ar2.erase(ar2.begin()->first);
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

    SECTION("Merging / swapping archives " + section_name) {
        auto ar = random_pareto_archive();
        archive_t ar2(max_size, is_mini);
        for (size_t i = 0; i < 100; ++i) {
            ar2.insert(random_value());
        }
        archive_t ar3 = ar;
        REQUIRE_FALSE(ar.dominates(ar3));
        ar3.merge(ar2);
        REQUIRE_FALSE(ar2.dominates(ar3));
        size_t ars1 = ar.size();
        size_t ars2 = ar2.size();
        ar.swap(ar2);
        REQUIRE(ars1 == ar2.size());
        REQUIRE(ars2 == ar.size());
    }

    SECTION("Queries " + section_name) {
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
                REQUIRE(p.distance(k) >= p.distance(it->first));
            }
        }
        for (auto it = ar.find_nearest(p, 5); it != ar.end(); ++it) {
            size_t c = 0;
            for (auto &[k, v] : ar) {
                if (p.distance(k) < p.distance(it->first)) {
                    ++c;
                }
            }
            REQUIRE_FALSE(c > 5);
        }
    }

    SECTION("Indicators " + section_name) {
        if (RUNTIME_DIMENSION > 5) {
            return;
        }
        auto ar = random_pareto_archive();
        // Get indicators and check if they are in acceptable ranges
        if (ar.size() > 2 && ar.begin_front()->size() > 2) {
            REQUIRE(ar.hypervolume(ar.nadir()) >= 0);
            REQUIRE(ar.hypervolume(ar.nadir(), 10) >= 0);
            REQUIRE(ar.hypervolume(ar.nadir(), 100) >= 0);
            REQUIRE(ar.hypervolume(ar.nadir(), 1000) >= 0);
            REQUIRE(ar.hypervolume(ar.nadir(), 10000) >= 0);
            REQUIRE(ar.hypervolume(ar.nadir(), 100000) >= 0);
            // Compare set coverage
            archive_t ar_b(max_size, is_mini);
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
            archive_t pf_c(max_size, is_mini);
            for (const auto &[k, v] : *ar.begin_front()) {
                point_type p = k;
                for (size_t i = 0; i < p.dimensions(); ++i) {
                    p[i] += is_mini[i] ? -0.5 : 0.5;
                }
                pf_c.insert(p, v);
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

    SECTION("Pareto Dominance " + section_name) {
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
        archive_t ar2 = ar;
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
                k2[i] += (is_mini[i] ? -1 : +1);
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
            for (size_t i = 0; i < k.dimensions(); ++i) {
                k[i] = k[i] + (is_mini[i] ? 2 : -2);
            }
            ar2.emplace(k, v2);
        }
        REQUIRE(ar.dominates(ar2));
        REQUIRE(ar.strongly_dominates(ar2));
        REQUIRE_FALSE(ar.non_dominates(ar2));
        REQUIRE_FALSE(ar2.dominates(ar));
        REQUIRE_FALSE(ar2.strongly_dominates(ar));
        REQUIRE_FALSE(ar2.non_dominates(ar));
    }

    SECTION("Reference points " + section_name) {
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

template<size_t M>
void test_all_tags(const std::vector<uint8_t> &is_mini) {
    using namespace pareto;
    test_archive<M, vector_tree_tag>(M, is_mini);
    test_archive<0, vector_tree_tag>(M, is_mini);
    test_archive<M, quad_tree_tag>(M, is_mini);
    test_archive<0, quad_tree_tag>(M, is_mini);
    test_archive<M, kd_tree_tag>(M, is_mini);
    test_archive<0, kd_tree_tag>(M, is_mini);
    test_archive<M, boost_tree_tag>(M, is_mini);
    test_archive<M, r_tree_tag>(M, is_mini);
    test_archive<0, r_tree_tag>(M, is_mini);
    test_archive<M, r_star_tree_tag>(M, is_mini);
    test_archive<0, r_star_tree_tag>(M, is_mini);
}

#ifdef BUILD_LONG_TESTS

TEST_CASE("Archive - 1 dimension") {
    using namespace pareto;
    test_all_tags<1>({0});
}

#endif

TEST_CASE("Archive - 2 dimensions") {
    using namespace pareto;
    test_all_tags<2>({0, 0});
    test_all_tags<2>({0, 1});
    test_all_tags<2>({1, 0});
    test_all_tags<2>({1, 1});
}

#ifdef BUILD_LONG_TESTS
TEST_CASE("Archive - 3 dimensions") {
    using namespace pareto;
    test_all_tags<3>({0, 0, 0});
    test_all_tags<3>({0, 1, 0});
    test_all_tags<3>({1, 0, 0});

}

TEST_CASE("Archive - 5 dimensions") {
    using namespace pareto;
    test_all_tags<5>({0, 0, 0, 0, 0});
    test_all_tags<5>({0, 0, 1, 0, 0});
    test_all_tags<5>({1, 0, 0, 1, 0});
    test_all_tags<5>({0, 0, 0, 1, 0});

}

TEST_CASE("Archive - 9 dimensions") {
    using namespace pareto;
    test_all_tags<9>({0, 0, 0, 0, 0, 0, 0, 0, 0});
    test_all_tags<9>({0, 0, 0, 0, 0, 0, 0, 0, 1});
    test_all_tags<9>({0, 0, 0, 1, 0, 0, 0, 0, 0});
    test_all_tags<9>({0, 0, 0, 0, 1, 1, 0, 0, 1});
    test_all_tags<9>({0, 0, 0, 0, 0, 1, 0, 0, 1});

}

TEST_CASE("Archive - 13 dimensions") {
    using namespace pareto;
    test_all_tags<13>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
    test_all_tags<13>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
    test_all_tags<13>({0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1});
    test_all_tags<13>({0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0});
    test_all_tags<13>({0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1});
}
#endif
