

#include <catch2/catch.hpp>

#include "../test_helpers.h"
#ifdef BUILD_BOOST_TREE
#include <pareto/boost_tree.h>
#endif
#include <pareto/front.h>
#include <pareto/implicit_tree.h>
#include <pareto/kd_tree.h>
#include <pareto/quad_tree.h>
#include <pareto/r_star_tree.h>
#include <pareto/r_tree.h>

template <class TREE_TYPE>
void test_tree() {
    using namespace pareto;
    using tree_type = TREE_TYPE;
    using value_type = typename tree_type::value_type;
    using key_type = typename tree_type::key_type;
    tree_type t;

    auto insert_some = [&]() {
        value_type v(key_type({5.2, 6.3, 1.3}), 7);
        auto it = t.insert(v);
        REQUIRE(it != t.end());
        REQUIRE(it->first == v.first);
        REQUIRE(it->second == v.second);

        it = t.insert(std::make_pair(key_type({4.2, 3.3, 7.3}), unsigned(3)));
        REQUIRE(it != t.end());
        REQUIRE(it->first == key_type({4.2, 3.3, 7.3}));
        REQUIRE(it->second == unsigned(3));

        for (size_t i = 0; i < 100; ++i) {
            value_type v2(key_type({randn(), randn(), randn()}), randi());
            it = t.insert(v2);
            REQUIRE(it != t.end());
            REQUIRE(it->first == v2.first);
            REQUIRE(it->second == v2.second);
        }
        REQUIRE(t.size() == 102);

        key_type p1(t.dimensions());
        key_type p2(t.dimensions());
        for (size_t i = 0; i < t.dimensions(); ++i) {
            p1[i] = 2.5 + 1. * i;
            p2[i] = 2.5 - 1. * i;
        }
        auto v1 = std::make_pair(p1, 2);
        auto v2 = std::make_pair(p2, 5);
        std::vector<value_type> v_ = {v1, v2};
        t.insert(v_.begin(), v_.end());
        REQUIRE(t.size() == 104);
    };

    auto clear_some = [&]() {
        for (size_t j = 0; j < 2; ++j) {
            t.clear();
            // insert 100 elements
            for (size_t i = 0; i < 100; ++i) {
                value_type v3(key_type({randn(), randn(), randn()}), randi());
                auto it = t.insert(v3);
                REQUIRE(it != t.end());
                REQUIRE(it->first == v3.first);
                REQUIRE(it->second == v3.second);
            }
            // iterate from first to last
            size_t i = 0;
            auto end = t.end();
            for (auto it2 = t.begin(); it2 != end; ++it2) {
                ++i;
            }
            REQUIRE(i == t.size());
        }
    };

    SECTION("Inserting elements") { insert_some(); }

    SECTION("Clear Some Elements") {
        insert_some();
        clear_some();
    }

    SECTION("Iterating") {
        insert_some();
        clear_some();
        size_t i = 0;
        auto end = t.end();
        for (auto it2 = t.begin(); it2 != end; ++it2) {
            ++i;
        }
        REQUIRE(t.size() == i);
        REQUIRE(t.size() == 100);
    }

    SECTION("Iterator--") {
        insert_some();
        clear_some();
        // iterate from last to first
        size_t i = 0;
        auto it_begin = t.begin();
        for (auto it2 = t.end(); it2 != it_begin;) {
            --it2;
            ++i;
        }
        REQUIRE(i == 100);
    }

    SECTION("Reverse iterator") {
        insert_some();
        clear_some();
        // iterate from last to first
        size_t i = 0;
        auto rend = t.rend();
        for (auto it2 = t.rbegin(); it2 != rend; ++it2) {
            ++i;
        }
        REQUIRE(i == 100);
    }

    SECTION("Iterating intersection") {
        insert_some();
        clear_some();
        for (auto it2 = t.find_intersection({-1, -1, -1}, {+1, +1, +1});
             it2 != t.end(); ++it2) {
            REQUIRE(std::all_of(it2->first.begin(), it2->first.end(),
                                [](const auto &x) { return x >= -1; }));
            REQUIRE(std::all_of(it2->first.begin(), it2->first.end(),
                                [](const auto &x) { return x <= +1; }));
        }
    }

    SECTION("Iterating intersection with predicate") {
        insert_some();
        clear_some();
        auto it = t.find_intersection(
            {-5, -5, -5}, {+5, +5, +5}, [](const value_type &v) {
                return v.first[0] > -1.0 && v.first[1] < 1.0;
            });
        for (; it != t.end(); ++it) {
            REQUIRE(std::all_of(it->first.begin(), it->first.end(),
                                [](const auto &x) { return x >= -5; }));
            REQUIRE(std::all_of(it->first.begin(), it->first.end(),
                                [](const auto &x) { return x <= +5; }));
            REQUIRE(it->first[0] > -1.0);
            REQUIRE(it->first[1] < 1.0);
        }
    }

    SECTION("Iterating within") {
        insert_some();
        clear_some();
        for (auto it2 = t.find_within({-1, -1, -1}, {+1, +1, +1});
             it2 != t.end(); ++it2) {
            REQUIRE(std::all_of(it2->first.begin(), it2->first.end(),
                                [](const auto &x) { return x >= -1; }));
            REQUIRE(std::all_of(it2->first.begin(), it2->first.end(),
                                [](const auto &x) { return x <= +1; }));
        }
    }

    SECTION("Iterating disjoint") {
        insert_some();
        clear_some();
        for (auto it2 = t.find_disjoint({-1, -1, -1}, {+1, +1, +1});
             it2 != t.end(); ++it2) {
            REQUIRE(
                std::any_of(it2->first.begin(), it2->first.end(),
                            [](const auto &x) { return x <= -1 || x >= +1; }));
        }
    }

    SECTION("Finding the nearest") {
        insert_some();
        clear_some();
        for (auto it2 = t.find_nearest({0, 0, 0}); it2 != t.end(); ++it2) {
            for (const auto &v3 : t) {
                REQUIRE(it2->first.distance({0, 0, 0}) <=
                        v3.first.distance({0, 0, 0}));
            }
        }
        auto it = t.find_nearest({0, 0, 0});
        if (it != t.end()) {
            for (auto it2 = t.begin(); it2 != t.end(); ++it2) {
                REQUIRE(it->first.distance({0, 0, 0}) <=
                        it2->first.distance({0, 0, 0}));
            }
        }
    }

    SECTION("Iterating the 7 closest points") {
        insert_some();
        clear_some();
        for (auto it2 = t.find_nearest({0, 0, 0}, 7); it2 != t.end(); ++it2) {
        }
        // iterate from last to first
        size_t i = 0;
        auto begin = t.find_nearest({0, 0, 0}, 7);
        auto last_nearest_it = t.find_nearest({0, 0, 0}, 7);
        while (last_nearest_it != t.end()) {
            ++last_nearest_it;
        }
        while (last_nearest_it != begin) {
            --last_nearest_it;
            ++i;
        }
    }

    SECTION("Finding values") {
        insert_some();
        clear_some();
        value_type v(key_type({5.2, 6.3, 1.3}), 7);
        t.insert(v);
        auto it = t.find(v.first);
        REQUIRE(it != t.end());
        REQUIRE(it->first == v.first);
        REQUIRE(it->second == v.second);
        ++it;
    }

    SECTION("Erasing elements") {
        insert_some();
        clear_some();

        // Create vector with the ones we want to erase
        std::vector<value_type> to_erase;
        size_t i = 0;
        for (auto it2 = t.begin(); it2 != t.end(); ++it2) {
            if (i % 2 == 0) {
                to_erase.emplace_back(*it2);
            }
            ++i;
        }
        REQUIRE(i == t.size());
        size_t previous_size = t.size();

        // For each of them
        auto comp_values = [](const value_type &v1, const value_type &v2) {
            if (v1.first == v2.first) {
                return v1.second < v2.second;
            } else {
                return v1.first.values() < v2.first.values();
            }
        };

        for (auto &v3 : to_erase) {
            // Make a copy of the containers in a vector
            using unprotected_value_type = std::pair<key_type, unsigned>;
            std::vector<unprotected_value_type> vector_replica(t.begin(), t.end());
            std::sort(vector_replica.begin(),
                      vector_replica.end(), comp_values);

            // Erase element from containers
            REQUIRE(t.erase(v3.first) == 1);

            // Make a copy of the new containers
            std::vector<unprotected_value_type> front_result(t.begin(), t.end());
            std::sort(front_result.begin(),
                      front_result.end(), comp_values);

            // Erase element from previous containers copy
            auto lb = std::lower_bound(vector_replica.begin(),
                                       vector_replica.end(), v3, comp_values);
            REQUIRE(lb != vector_replica.end());
            vector_replica.erase(lb);

            // REQUIRE that the copies match
            if (vector_replica != front_result) {
                std::cout << "Trees don't match!" << std::endl;
                std::cout << "Attempting to erase " << v3.first << ", "
                          << v3.second << std::endl;
                std::cout << "Tree Difference" << std::endl;
                size_t n = std::max(vector_replica.size(),
                                    front_result.size());
                for (size_t i2 = 0; i2 < n; ++i2) {
                    if (i2 < vector_replica.size()) {
                        std::cout << "Replica: ";
                        std::cout << "[" << vector_replica[i2].first
                                  << ", ";
                        std::cout << vector_replica[i2].second << "]";
                    }
                    if (i2 < front_result.size()) {
                        std::cout << " - Result: ";
                        std::cout << "[" << front_result[i2].first
                                  << ", ";
                        std::cout << front_result[i2].second << "]";
                    }
                    if (i2 < vector_replica.size() && i2 < front_result.size()) {
                        if (vector_replica[i2] != front_result[i2]) {
                            std::cout << " ***";
                        }
                    }
                    std::cout << std::endl;
                }
                // the containers after erasing does not have the elements it
                // should
                REQUIRE(false);
            }
        }

        i = 0;
        for (auto it2 = t.begin(); it2 != t.end(); ++it2) {
            ++i;
        }
        REQUIRE(i == previous_size / 2);
    }

    SECTION("Erasing with iterator") {
        insert_some();
        clear_some();
        for (size_t i2 = 0; i2 < 120; ++i2) {
            // insert some more
            value_type v3(key_type({randn(), randn(), randn()}), randi());
            auto it = t.insert(v3);
            REQUIRE(it != t.end());
            REQUIRE(it->first == v3.first);
            REQUIRE(it->second == v3.second);
        }
        size_t s = t.size();
        auto it = t.begin();
        t.erase(it);
        it = t.begin();
        std::advance(it, 5);
        t.erase(it);
        REQUIRE(t.size() == s - 2);
    }

    SECTION("Min/max values and elements") {
        insert_some();
        clear_some();
        for (size_t i2 = 0; i2 < t.dimensions(); ++i2) {
            auto max_it = std::max_element(
                t.begin(), t.end(),
                [&i2](const value_type &a, const value_type &b) {
                    return a.first[i2] < b.first[i2];
                });
            auto min_it = std::min_element(
                t.begin(), t.end(),
                [&i2](const value_type &a, const value_type &b) {
                    return a.first[i2] < b.first[i2];
                });
            REQUIRE(max_it->first[i2] == t.max_value(i2));
            REQUIRE(min_it->first[i2] == t.min_value(i2));
            REQUIRE(*max_it == *t.max_element(i2));
            REQUIRE(*min_it == *t.min_element(i2));
        }
    }

    SECTION("Copy constructor") {
        insert_some();
        clear_some();
        tree_type t2(t);
        REQUIRE(t.size() == t2.size());
        auto tit = t.begin();
        auto t2it = t2.begin();
        for (; tit != t.end() && t2it != t2.end(); ++tit, ++t2it) {
            REQUIRE(*tit == *t2it);
        }
    }
}

#ifdef implicit_TREETAG
TEST_CASE("Implicit-Tree") {
    SECTION("Runtime Dimension") {
        test_tree<pareto::implicit_tree<double, 0, unsigned>>();
    }
    SECTION("Compile Time Dimension") {
        test_tree<pareto::implicit_tree<double, 3, unsigned>>();
    }
}
#elif quad_TREETAG
TEST_CASE("Quad-Tree") {
    SECTION("Runtime Dimension") {
        test_tree<pareto::quad_tree<double, 0, unsigned>>();
    }
    SECTION("Compile Time Dimension") {
        test_tree<pareto::quad_tree<double, 3, unsigned>>();
    }
}
#elif kd_TREETAG
TEST_CASE("kd-Tree") {
    SECTION("Runtime Dimension") {
        test_tree<pareto::kd_tree<double, 0, unsigned>>();
    }
    SECTION("Compile Time Dimension") {
        test_tree<pareto::kd_tree<double, 3, unsigned>>();
    }
}
#elif boost_TREETAG
#ifdef BUILD_BOOST_TREE
TEST_CASE("Boost-Tree") {
    SECTION("Compile Time Dimension") {
        test_tree<pareto::r_tree<double, 3, unsigned>>();
    }
}
#endif
#elif r_TREETAG
TEST_CASE("R-Tree") {
    SECTION("Runtime Dimension") {
        test_tree<pareto::r_tree<double, 0, unsigned>>();
    }
    SECTION("Compile Time Dimension") {
        test_tree<pareto::r_tree<double, 3, unsigned>>();
    }
}
#elif r_star_TREETAG
TEST_CASE("R*-Tree") {
    SECTION("Runtime Dimension") {
        test_tree<pareto::r_star_tree<double, 0, unsigned>>();
    }
    SECTION("Compile Time Dimension") {
        test_tree<pareto::r_star_tree<double, 3, unsigned>>();
    }
}
#endif