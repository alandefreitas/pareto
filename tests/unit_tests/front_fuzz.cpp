#define CATCH_CONFIG_MAIN

#include "instantiation/test_instantiations.h"
#include <catch2/catch.hpp>

unsigned randi();
bool rand_flip();
double randn();
double randu();
using uint8_t_vector_iterator = std::vector<uint8_t>::iterator;
bool next_combination(uint8_t_vector_iterator first,
                      uint8_t_vector_iterator last, uint8_t max_value = 0x01);

template <size_t COMPILE_DIMENSION,
    typename TAG = pareto::default_tag<COMPILE_DIMENSION>>
void test_front(size_t RUNTIME_DIMENSION = COMPILE_DIMENSION,
                const std::vector<uint8_t> &is_mini = {0x00}) {
    size_t test_dimension =
        COMPILE_DIMENSION != 0 ? COMPILE_DIMENSION : RUNTIME_DIMENSION;
    // create a string to make section names unique
    // see: https://github.com/catchorg/Catch2/issues/816
    std::string section_name =
        std::to_string(test_dimension) + " dimensions - ";
    section_name +=
        std::to_string(COMPILE_DIMENSION) + " compile dimensions - ";
    section_name +=
        std::to_string(RUNTIME_DIMENSION) + " runtime dimensions - ";
    auto type_name = std::string(typeid(TAG).name());
    type_name = std::regex_replace(
        type_name, std::regex("(N\\d+)pareto([\\d]+)([^E]+)E"), "pareto::$3");
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
    using pareto_front_t = front<double, COMPILE_DIMENSION, unsigned, TAG>;
    using point_type = typename pareto_front_t::point_type;
    using value_type = typename pareto_front_t::value_type;

    SECTION("Constructors " + section_name) {
        // dimensions
        pareto_front_t pf1;
        // dimensions and direction
        pareto_front_t pf2(true);
        // each direction (infer dimension from number of directions)
        pareto_front_t pf3(is_mini);
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
        pareto_front_t pf4(v.begin(), v.end());
        // iterators and direction
        pareto_front_t pf5(v.begin(), v.end(), false);
        // iterators and directions
        pareto_front_t pf6(v.begin(), v.end(), is_mini);
        // copy constructor
        pareto_front_t pf7(pf6);
        // move constructor
        pareto_front_t pf8(std::move(pf7));
        if (test_dimension == 2) {
            // initializer list
            pareto_front_t pf9({{{2.6, 3.4}, 6}, {{6.5, 2.4}, 4}});
            // initializer list and direction
            pareto_front_t pf10({{{2.6, 3.4}, 6}, {{6.5, 2.4}, 4}}, false);
        }
        // vector
        pareto_front_t pf11(v);
        // vector and direction
        pareto_front_t pf12(v, false);
        // vector and directions
        pareto_front_t pf13(v, is_mini);
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
      pareto_front_t pf(is_mini);
      point_type p1(test_dimension);
      point_type p2(test_dimension);
      for (size_t i = 0; i < test_dimension; ++i) {
          p1[i] = 2.5 + 1. * i;
          p2[i] = 1.5 + test_dimension - 1. * i;
      }
      // 1.31336, 0.887686, -1.35926
      pf.insert(std::make_pair(p1, 2));
      pf.insert(std::make_pair(p2, 3));
      // size_t s = pf.size();
      pf.emplace(random_value());
      std::vector v = {random_value(), random_value(), random_value()};
      pf.insert(v.begin(), v.end());
      auto r = pf.insert(random_value());
      value_type v2 = random_value();
      r = pf.insert(std::move(v2));
      r = pf.insert(std::make_pair(random_point(), randi()));
      unsigned m = randi();
      r = pf.insert(std::make_pair(random_point(), std::move(m)));
      std::vector v3 = {random_value(), random_value(), random_value()};
      pf.insert(v3.begin(), v3.end());
      pf.insert({random_value(), random_value(), random_value()});
      for (size_t i = 0; i < 1000 / test_dimension; ++i) {
          pf.insert(random_value());
      }
      return pf;
    };

    SECTION("Container functions and iterators " + section_name) {
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
        REQUIRE(pf.dimensions() == test_dimension);
        point_type p2 = pf.begin()->first;
        REQUIRE(pf.find(p2) != pf.end());
        REQUIRE(pf.contains(p2));
        REQUIRE(pf.find(random_point()) == pf.end());
        REQUIRE(!pf.contains(random_point()));
        pf.clear();
        REQUIRE(pf.empty());
        REQUIRE(pf.size() == 0);
        REQUIRE(pf.dimensions() == test_dimension);
    }

    // Erasing items
    SECTION("Erasing " + section_name) {
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
        pf2.erase(pf2.begin(), pf2.end());
        REQUIRE(pf2.size() == 0);
        REQUIRE(pf2.empty());
        pf2 = pf;
        REQUIRE_FALSE(pf2.empty());
        pf2.clear();
        REQUIRE(pf2.empty());
    }

    SECTION("Merging / swapping fronts " + section_name) {
        auto pf = random_pareto_front();
        pareto_front_t pf2(is_mini);
        for (size_t i = 0; i < 100; ++i) {
            pf2.insert(random_value());
        }
        pareto_front_t pf3 = pf;
        REQUIRE_FALSE(pf.dominates(pf3));
        pf3.merge(pf2);
        REQUIRE_FALSE(pf2.dominates(pf3));
        size_t pfs1 = pf.size();
        size_t pfs2 = pf2.size();
        pf.swap(pf2);
        REQUIRE(pfs1 == pf2.size());
        REQUIRE(pfs2 == pf.size());
    }

    SECTION("Queries " + section_name) {
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

    SECTION("Indicators " + section_name) {
        if (RUNTIME_DIMENSION > 5) {
            return;
        }
        auto pf = random_pareto_front();
        // Get indicators and check if they are in acceptable ranges
        if (pf.size() > 2) {
            REQUIRE(pf.hypervolume(pf.nadir()) >= 0);
            REQUIRE(pf.hypervolume(pf.nadir(), 10) >= 0);
            REQUIRE(pf.hypervolume(pf.nadir(), 100) >= 0);
            REQUIRE(pf.hypervolume(pf.nadir(), 1000) >= 0);
            REQUIRE(pf.hypervolume(pf.nadir(), 10000) >= 0);
            REQUIRE(pf.hypervolume(pf.nadir(), 100000) >= 0);
            // Compare set coverage
            pareto_front_t pf_b(is_mini);
            for (size_t i = 0; i < 1000; ++i) {
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
            pareto_front_t pf_c(is_mini);
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

            // Distribution and spread
            REQUIRE(pf.uniformity() > 0);
            REQUIRE(pf.average_distance() > 0);
            REQUIRE(pf.average_nearest_distance(1) > 0);
            REQUIRE(pf.average_nearest_distance(2) > 0);
            REQUIRE(pf.average_nearest_distance(5) > 0);
            REQUIRE(pf.average_crowding_distance() > 0);
        }
    }

    SECTION("Pareto Dominance " + section_name) {
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
        pareto_front_t pf2 = pf;
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
            for (size_t i = 0; i < k.dimensions(); ++i) {
                k[i] = k[i] + (is_mini[i] ? 2 : -2);
            }
            pf2.emplace(k, v2);
        }
        REQUIRE(pf.dominates(pf2));
        REQUIRE(pf.strongly_dominates(pf2));
        REQUIRE_FALSE(pf.non_dominates(pf2));
        REQUIRE_FALSE(pf2.dominates(pf));
        REQUIRE_FALSE(pf2.strongly_dominates(pf));
        REQUIRE_FALSE(pf2.non_dominates(pf));
    }

    SECTION("Reference points " + section_name) {
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
    }
}

#ifdef BUILD_LONG_TESTS
TEST_CASE("Front - 1 dimension") {
    using namespace pareto;
    std::vector<uint8_t> is_mini = {0};
    test_front<1, vector_tree_tag>(1, is_mini);
    test_front<0, vector_tree_tag>(1, is_mini);
    test_front<1, quad_tree_tag>(1, is_mini);
    test_front<0, quad_tree_tag>(1, is_mini);
    test_front<1, kd_tree_tag>(1, is_mini);
    test_front<0, kd_tree_tag>(1, is_mini);
    test_front<1, boost_tree_tag>(1, is_mini);
    test_front<1, r_tree_tag>(1, is_mini);
    test_front<0, r_tree_tag>(1, is_mini);
    test_front<1, r_star_tree_tag>(1, is_mini);
    test_front<0, r_star_tree_tag>(1, is_mini);
}
#endif

TEST_CASE("Front - 2 dimensions") {
    using namespace pareto;
    std::vector<uint8_t> is_mini = {0, 0};
    test_front<2, vector_tree_tag>(2, is_mini);
    test_front<0, vector_tree_tag>(2, is_mini);
    test_front<2, quad_tree_tag>(2, is_mini);
    test_front<0, quad_tree_tag>(2, is_mini);
    test_front<2, kd_tree_tag>(2, is_mini);
    test_front<0, kd_tree_tag>(2, is_mini);
    test_front<2, boost_tree_tag>(2, is_mini);
    test_front<2, r_tree_tag>(2, is_mini);
    test_front<0, r_tree_tag>(2, is_mini);
    test_front<0, r_tree_tag>(2, is_mini);
    test_front<2, r_star_tree_tag>(2, is_mini);
    test_front<0, r_star_tree_tag>(2, is_mini);

    is_mini = {0, 1};
    test_front<2, vector_tree_tag>(2, is_mini);
    test_front<0, vector_tree_tag>(2, is_mini);
    test_front<2, quad_tree_tag>(2, is_mini);
    test_front<0, quad_tree_tag>(2, is_mini);
    test_front<2, kd_tree_tag>(2, is_mini);
    test_front<0, kd_tree_tag>(2, is_mini);
    test_front<2, boost_tree_tag>(2, is_mini);
    test_front<2, r_tree_tag>(2, is_mini);
    test_front<0, r_tree_tag>(2, is_mini);
    test_front<0, r_tree_tag>(2, is_mini);
    test_front<2, r_star_tree_tag>(2, is_mini);
    test_front<0, r_star_tree_tag>(2, is_mini);

    is_mini = {1, 0};
    test_front<2, vector_tree_tag>(2, is_mini);
    test_front<0, vector_tree_tag>(2, is_mini);
    test_front<2, quad_tree_tag>(2, is_mini);
    test_front<0, quad_tree_tag>(2, is_mini);
    test_front<2, kd_tree_tag>(2, is_mini);
    test_front<0, kd_tree_tag>(2, is_mini);
    test_front<2, boost_tree_tag>(2, is_mini);
    test_front<2, r_tree_tag>(2, is_mini);
    test_front<0, r_tree_tag>(2, is_mini);
    test_front<0, r_tree_tag>(2, is_mini);
    test_front<2, r_star_tree_tag>(2, is_mini);
    test_front<0, r_star_tree_tag>(2, is_mini);

    is_mini = {1, 1};
    test_front<2, vector_tree_tag>(2, is_mini);
    test_front<0, vector_tree_tag>(2, is_mini);
    test_front<2, quad_tree_tag>(2, is_mini);
    test_front<0, quad_tree_tag>(2, is_mini);
    test_front<2, kd_tree_tag>(2, is_mini);
    test_front<0, kd_tree_tag>(2, is_mini);
    test_front<2, boost_tree_tag>(2, is_mini);
    test_front<2, r_tree_tag>(2, is_mini);
    test_front<0, r_tree_tag>(2, is_mini);
    test_front<0, r_tree_tag>(2, is_mini);
    test_front<2, r_star_tree_tag>(2, is_mini);
    test_front<0, r_star_tree_tag>(2, is_mini);
}

#ifdef BUILD_LONG_TESTS
TEST_CASE("Front - 3 dimensions") {
    using namespace pareto;
    std::vector<uint8_t> is_mini = {0, 0, 0};
    test_front<3, vector_tree_tag>(3, is_mini);
    test_front<0, vector_tree_tag>(3, is_mini);
    test_front<3, quad_tree_tag>(3, is_mini);
    test_front<0, quad_tree_tag>(3, is_mini);
    test_front<3, kd_tree_tag>(3, is_mini);
    test_front<0, kd_tree_tag>(3, is_mini);
    test_front<3, boost_tree_tag>(3, is_mini);
    test_front<3, r_tree_tag>(3, is_mini);
    test_front<0, r_tree_tag>(3, is_mini);
    test_front<3, r_star_tree_tag>(3, is_mini);
    test_front<0, r_star_tree_tag>(3, is_mini);

    is_mini = {0, 1, 0};
    test_front<3, vector_tree_tag>(3, is_mini);
    test_front<0, vector_tree_tag>(3, is_mini);
    test_front<3, quad_tree_tag>(3, is_mini);
    test_front<0, quad_tree_tag>(3, is_mini);
    test_front<3, kd_tree_tag>(3, is_mini);
    test_front<0, kd_tree_tag>(3, is_mini);
    test_front<3, boost_tree_tag>(3, is_mini);
    test_front<3, r_tree_tag>(3, is_mini);
    test_front<0, r_tree_tag>(3, is_mini);
    test_front<3, r_star_tree_tag>(3, is_mini);
    test_front<0, r_star_tree_tag>(3, is_mini);

    is_mini = {1, 0, 0};
    test_front<3, vector_tree_tag>(3, is_mini);
    test_front<0, vector_tree_tag>(3, is_mini);
    test_front<3, quad_tree_tag>(3, is_mini);
    test_front<0, quad_tree_tag>(3, is_mini);
    test_front<3, kd_tree_tag>(3, is_mini);
    test_front<0, kd_tree_tag>(3, is_mini);
    test_front<3, boost_tree_tag>(3, is_mini);
    test_front<3, r_tree_tag>(3, is_mini);
    test_front<0, r_tree_tag>(3, is_mini);
    test_front<3, r_star_tree_tag>(3, is_mini);
    test_front<0, r_star_tree_tag>(3, is_mini);

}

TEST_CASE("Front - 5 dimensions") {
    using namespace pareto;
    std::vector<uint8_t> is_mini = {0, 0, 0, 0, 0};
    test_front<5, vector_tree_tag>(5, is_mini);
    test_front<0, vector_tree_tag>(5, is_mini);
    test_front<5, quad_tree_tag>(5, is_mini);
    test_front<0, quad_tree_tag>(5, is_mini);
    test_front<5, kd_tree_tag>(5, is_mini);
    test_front<0, kd_tree_tag>(5, is_mini);
    test_front<5, boost_tree_tag>(5, is_mini);
    test_front<5, r_tree_tag>(5, is_mini);
    test_front<0, r_tree_tag>(5, is_mini);
    test_front<5, r_star_tree_tag>(5, is_mini);
    test_front<0, r_star_tree_tag>(5, is_mini);

    is_mini = {0, 0, 1, 0, 0};
    test_front<5, vector_tree_tag>(5, is_mini);
    test_front<0, vector_tree_tag>(5, is_mini);
    test_front<5, quad_tree_tag>(5, is_mini);
    test_front<0, quad_tree_tag>(5, is_mini);
    test_front<5, kd_tree_tag>(5, is_mini);
    test_front<0, kd_tree_tag>(5, is_mini);
    test_front<5, boost_tree_tag>(5, is_mini);
    test_front<5, r_tree_tag>(5, is_mini);
    test_front<0, r_tree_tag>(5, is_mini);
    test_front<5, r_star_tree_tag>(5, is_mini);
    test_front<0, r_star_tree_tag>(5, is_mini);

    is_mini = {1, 0, 0, 1, 0};
    test_front<5, vector_tree_tag>(5, is_mini);
    test_front<0, vector_tree_tag>(5, is_mini);
    test_front<5, quad_tree_tag>(5, is_mini);
    test_front<0, quad_tree_tag>(5, is_mini);
    test_front<5, kd_tree_tag>(5, is_mini);
    test_front<0, kd_tree_tag>(5, is_mini);
    test_front<5, boost_tree_tag>(5, is_mini);
    test_front<5, r_tree_tag>(5, is_mini);
    test_front<0, r_tree_tag>(5, is_mini);
    test_front<5, r_star_tree_tag>(5, is_mini);
    test_front<0, r_star_tree_tag>(5, is_mini);

    is_mini = {0, 0, 0, 1, 0};
    test_front<5, vector_tree_tag>(5, is_mini);
    test_front<0, vector_tree_tag>(5, is_mini);
    test_front<5, quad_tree_tag>(5, is_mini);
    test_front<0, quad_tree_tag>(5, is_mini);
    test_front<5, kd_tree_tag>(5, is_mini);
    test_front<0, kd_tree_tag>(5, is_mini);
    test_front<5, boost_tree_tag>(5, is_mini);
    test_front<5, r_tree_tag>(5, is_mini);
    test_front<0, r_tree_tag>(5, is_mini);
    test_front<5, r_star_tree_tag>(5, is_mini);
    test_front<0, r_star_tree_tag>(5, is_mini);

}

TEST_CASE("Front - 9 dimensions") {
    using namespace pareto;
    std::vector<uint8_t> is_mini = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    test_front<9, vector_tree_tag>(9, is_mini);
    test_front<0, vector_tree_tag>(9, is_mini);
    test_front<9, quad_tree_tag>(9, is_mini);
    test_front<0, quad_tree_tag>(9, is_mini);
    test_front<9, kd_tree_tag>(9, is_mini);
    test_front<0, kd_tree_tag>(9, is_mini);
    test_front<9, boost_tree_tag>(9, is_mini);
    test_front<9, r_tree_tag>(9, is_mini);
    test_front<0, r_tree_tag>(9, is_mini);
    test_front<9, r_star_tree_tag>(9, is_mini);
    test_front<0, r_star_tree_tag>(9, is_mini);

    is_mini = {0, 0, 0, 0, 0, 0, 0, 0, 1};
    test_front<9, vector_tree_tag>(9, is_mini);
    test_front<0, vector_tree_tag>(9, is_mini);
    test_front<9, quad_tree_tag>(9, is_mini);
    test_front<0, quad_tree_tag>(9, is_mini);
    test_front<9, kd_tree_tag>(9, is_mini);
    test_front<0, kd_tree_tag>(9, is_mini);
    test_front<9, boost_tree_tag>(9, is_mini);
    test_front<9, r_tree_tag>(9, is_mini);
    test_front<0, r_tree_tag>(9, is_mini);
    test_front<9, r_star_tree_tag>(9, is_mini);
    test_front<0, r_star_tree_tag>(9, is_mini);

    is_mini = {0, 0, 0, 1, 0, 0, 0, 0, 0};
    test_front<9, vector_tree_tag>(9, is_mini);
    test_front<0, vector_tree_tag>(9, is_mini);
    test_front<9, quad_tree_tag>(9, is_mini);
    test_front<0, quad_tree_tag>(9, is_mini);
    test_front<9, kd_tree_tag>(9, is_mini);
    test_front<0, kd_tree_tag>(9, is_mini);
    test_front<9, boost_tree_tag>(9, is_mini);
    test_front<9, r_tree_tag>(9, is_mini);
    test_front<0, r_tree_tag>(9, is_mini);
    test_front<9, r_star_tree_tag>(9, is_mini);
    test_front<0, r_star_tree_tag>(9, is_mini);

    is_mini = {0, 0, 0, 0, 1, 1, 0, 0, 1};
    test_front<9, vector_tree_tag>(9, is_mini);
    test_front<0, vector_tree_tag>(9, is_mini);
    test_front<9, quad_tree_tag>(9, is_mini);
    test_front<0, quad_tree_tag>(9, is_mini);
    test_front<9, kd_tree_tag>(9, is_mini);
    test_front<0, kd_tree_tag>(9, is_mini);
    test_front<9, boost_tree_tag>(9, is_mini);
    test_front<9, r_tree_tag>(9, is_mini);
    test_front<0, r_tree_tag>(9, is_mini);
    test_front<9, r_star_tree_tag>(9, is_mini);
    test_front<0, r_star_tree_tag>(9, is_mini);

    is_mini = {0, 0, 0, 0, 0, 1, 0, 0, 1};
    test_front<9, vector_tree_tag>(9, is_mini);
    test_front<0, vector_tree_tag>(9, is_mini);
    test_front<9, quad_tree_tag>(9, is_mini);
    test_front<0, quad_tree_tag>(9, is_mini);
    test_front<9, kd_tree_tag>(9, is_mini);
    test_front<0, kd_tree_tag>(9, is_mini);
    test_front<9, boost_tree_tag>(9, is_mini);
    test_front<9, r_tree_tag>(9, is_mini);
    test_front<0, r_tree_tag>(9, is_mini);
    test_front<9, r_star_tree_tag>(9, is_mini);
    test_front<0, r_star_tree_tag>(9, is_mini);

}

TEST_CASE("Front - 13 dimensions") {
    using namespace pareto;
    std::vector<uint8_t> is_mini = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    test_front<13, vector_tree_tag>(13, is_mini);
    test_front<0, vector_tree_tag>(13, is_mini);
    test_front<13, quad_tree_tag>(13, is_mini);
    test_front<0, quad_tree_tag>(13, is_mini);
    test_front<13, kd_tree_tag>(13, is_mini);
    test_front<0, kd_tree_tag>(13, is_mini);
    test_front<13, boost_tree_tag>(13, is_mini);
    test_front<13, r_tree_tag>(13, is_mini);
    test_front<0, r_tree_tag>(13, is_mini);
    test_front<13, r_star_tree_tag>(13, is_mini);
    test_front<0, r_star_tree_tag>(13, is_mini);

    is_mini = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    test_front<13, vector_tree_tag>(13, is_mini);
    test_front<0, vector_tree_tag>(13, is_mini);
    test_front<13, quad_tree_tag>(13, is_mini);
    test_front<0, quad_tree_tag>(13, is_mini);
    test_front<13, kd_tree_tag>(13, is_mini);
    test_front<0, kd_tree_tag>(13, is_mini);
    test_front<13, boost_tree_tag>(13, is_mini);
    test_front<13, r_tree_tag>(13, is_mini);
    test_front<0, r_tree_tag>(13, is_mini);
    test_front<13, r_star_tree_tag>(13, is_mini);
    test_front<0, r_star_tree_tag>(13, is_mini);

    is_mini = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1};
    test_front<13, vector_tree_tag>(13, is_mini);
    test_front<0, vector_tree_tag>(13, is_mini);
    test_front<13, quad_tree_tag>(13, is_mini);
    test_front<0, quad_tree_tag>(13, is_mini);
    test_front<13, kd_tree_tag>(13, is_mini);
    test_front<0, kd_tree_tag>(13, is_mini);
    test_front<13, boost_tree_tag>(13, is_mini);
    test_front<13, r_tree_tag>(13, is_mini);
    test_front<0, r_tree_tag>(13, is_mini);
    test_front<13, r_star_tree_tag>(13, is_mini);
    test_front<0, r_star_tree_tag>(13, is_mini);

    is_mini = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
    test_front<13, vector_tree_tag>(13, is_mini);
    test_front<0, vector_tree_tag>(13, is_mini);
    test_front<13, quad_tree_tag>(13, is_mini);
    test_front<0, quad_tree_tag>(13, is_mini);
    test_front<13, kd_tree_tag>(13, is_mini);
    test_front<0, kd_tree_tag>(13, is_mini);
    test_front<13, boost_tree_tag>(13, is_mini);
    test_front<13, r_tree_tag>(13, is_mini);
    test_front<0, r_tree_tag>(13, is_mini);
    test_front<13, r_star_tree_tag>(13, is_mini);
    test_front<0, r_star_tree_tag>(13, is_mini);

    is_mini = {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1};
    test_front<13, vector_tree_tag>(13, is_mini);
    test_front<0, vector_tree_tag>(13, is_mini);
    test_front<13, quad_tree_tag>(13, is_mini);
    test_front<0, quad_tree_tag>(13, is_mini);
    test_front<13, kd_tree_tag>(13, is_mini);
    test_front<0, kd_tree_tag>(13, is_mini);
    test_front<13, boost_tree_tag>(13, is_mini);
    test_front<13, r_tree_tag>(13, is_mini);
    test_front<0, r_tree_tag>(13, is_mini);
    test_front<13, r_star_tree_tag>(13, is_mini);
    test_front<0, r_star_tree_tag>(13, is_mini);

}
#endif

uint64_t seed() {
    //    static uint64_t seed = static_cast<uint64_t>(std::random_device()()) |
    //                           std::chrono::high_resolution_clock::now().time_since_epoch().count();
    static uint64_t seed = 323122652497823;
    std::cout << "Test seed: " << seed << std::endl;
    return seed;
}

std::mt19937 &generator() {
    static std::mt19937 g(static_cast<unsigned int>(seed()));
    return g;
}

bool rand_flip() {
    static std::uniform_int_distribution<unsigned> ud(0, 1);
    return ud(generator());
}

unsigned randi() {
    static std::uniform_int_distribution<unsigned> ud(0, 40);
    return ud(generator());
    ;
}

double randu() {
    static std::uniform_real_distribution<double> ud(0., 1.);
    return ud(generator());
}

double randn() {
    static std::normal_distribution nd;
    return nd(generator());
}

bool next_combination(uint8_t_vector_iterator first,
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