#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <pareto_front/tree/quad_tree.h>
#include <pareto_front/tree/kd_tree.h>
#include <pareto_front/tree/r_tree.h>
#include <pareto_front/tree/r_star_tree.h>

uint64_t seed();
unsigned randi();
bool rand_flip();
double randn();
double randu();

template <class TREE_TYPE = pareto_front::r_tree<double, 0, unsigned>>
void test_tree() {
    using namespace pareto_front;
    using tree_type = TREE_TYPE;
    using value_type = typename tree_type::value_type;
    using point_type = typename tree_type::point_type;
    tree_type t;

    std::cout << "Inserting elements" << std::endl;
    value_type v(point_type({5.2,6.3,1.3}),7);
    auto [it, ok] = t.insert(v);
    REQUIRE(ok);
    REQUIRE(it->first == v.first);
    REQUIRE(it->second == v.second);

    std::tie(it, ok) = t.insert(std::make_pair(point_type({4.2,3.3,7.3}),unsigned(3)));
    REQUIRE(ok);
    REQUIRE(it->first == point_type({4.2,3.3,7.3}));
    REQUIRE(it->second == unsigned(3));

    for (size_t i = 0; i < 100; ++i) {
        value_type v(point_type({randn(), randn(), randn()}), randi());
        std::tie(it, ok) = t.insert(v);
        REQUIRE(ok);
        REQUIRE(it->first == v.first);
        REQUIRE(it->second == v.second);
        std::cout << i + 1 << " - " << it->first << ": " << it->second << std::endl;
    }
    REQUIRE(t.size() == 102);

    point_type p1(t.dimensions());
    point_type p2(t.dimensions());
    for (size_t i = 0; i < t.dimensions(); ++i) {
        p1[i] = 2.5 + 1. * i;
        p2[i] = 2.5 - 1. * i;
    }
    auto v1 = std::make_pair(p1, 2);
    auto v2 = std::make_pair(p2, 5);
    std::vector<value_type> v_ = {v1, v2};
    t.insert(v_.begin(), v_.end());
    REQUIRE(t.size() == 104);

    std::cout << "---------------" << std::endl;

    std::cout << "Clear elements" << std::endl;
    for (size_t j = 0; j < 2; ++j) {
        t.clear();
        // insert 100 elements
        for (size_t i = 0; i < 100; ++i) {
            value_type v(point_type({randn(), randn(), randn()}), randi());
            std::tie(it, ok) = t.insert(v);
            REQUIRE(ok);
            REQUIRE(it->first == v.first);
            REQUIRE(it->second == v.second);
            std::cout << i + 1 << " - " << it->first << ": " << it->second << std::endl;
        }
        // iterate from first to last
        size_t i = 0;
        auto end = t.end();
        for (auto it = t.begin(); it != end; ++it) {
            std::cout << i << " - " << it->first << ": " << it->second << std::endl;
            ++i;
        }
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Iterating" << std::endl;
    size_t i = 0;
    auto end = t.end();
    for (auto it = t.begin(); it != end; ++it) {
        std::cout << i << " - " << it->first << ": " << it->second << std::endl;
        ++i;
    }
    REQUIRE(t.size() == i);
    REQUIRE(t.size() == 100);
    std::cout << "---------------" << std::endl;

    std::cout << "Iterator--" << std::endl;
    // iterate from last to first
    i = 0;
    auto it_begin = t.begin();
    for (auto it = t.end(); it != it_begin;) {
        --it;
        std::cout << i << " - " << it->first << ": " << it->second << std::endl;
        ++i;
    }
    REQUIRE(i == 100);
    std::cout << "---------------" << std::endl;

    std::cout << "Reverse iterator" << std::endl;
    // iterate from last to first
    i = 0;
    auto rend = t.rend();
    for (auto it = t.rbegin(); it != rend; ++it) {
        std::cout << i << " - " << it->first << ": " << it->second << std::endl;
        ++i;
    }
    REQUIRE(i == 100);
    std::cout << "---------------" << std::endl;

    std::cout << "Iterating intersection" << std::endl;
    for (auto it = t.begin_intersection({-1,-1,-1}, {+1,+1,+1}); it != t.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
        REQUIRE(std::all_of(it->first.begin(), it->first.end(), [](const auto& x) {return x >= -1;}));
        REQUIRE(std::all_of(it->first.begin(), it->first.end(), [](const auto& x) {return x <= +1;}));
        ++i;
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Iterating intersection with predicate" << std::endl;
    it = t.begin_intersection({-5,-5,-5}, {+5,+5,+5}, [](const value_type& v) {
        return v.first[0] > -1.0 && v.first[1] < 1.0;
    });
    for (; it != t.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
        REQUIRE(std::all_of(it->first.begin(), it->first.end(), [](const auto& x) {return x >= -5;}));
        REQUIRE(std::all_of(it->first.begin(), it->first.end(), [](const auto& x) {return x <= +5;}));
        REQUIRE(it->first[0] > -1.0);
        REQUIRE(it->first[1] < 1.0);
        ++i;
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Iterating within" << std::endl;
    for (auto it = t.begin_within({-1,-1,-1}, {+1,+1,+1}); it != t.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
        REQUIRE(std::all_of(it->first.begin(), it->first.end(), [](const auto& x) {return x >= -1;}));
        REQUIRE(std::all_of(it->first.begin(), it->first.end(), [](const auto& x) {return x <= +1;}));
        ++i;
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Iterating disjoint" << std::endl;
    for (auto it = t.begin_disjoint({-1,-1,-1}, {+1,+1,+1}); it != t.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
        REQUIRE(std::any_of(it->first.begin(), it->first.end(), [](const auto& x) { return x <= -1 || x >= +1;}));
        ++i;
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Finding the nearest" << std::endl;
    for (auto it = t.begin_nearest({0,0,0}); it != t.end(); ++it) {
        std::cout << it->first << ": " << it->second << " - Distance to {0,0,0} = " << it->first.distance({0,0,0}) << std::endl;
        for (const auto& v: t) {
            REQUIRE(it->first.distance({0,0,0}) <= v.first.distance({0,0,0}));
        }
        ++i;
    }
    it = t.begin_nearest({0,0,0});
    if (it != t.end()) {
        for (auto it2 = t.begin(); it2 != t.end(); ++it2) {
            REQUIRE(it->first.distance({0,0,0}) <= it2->first.distance({0,0,0}));
            ++i;
        }
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Iterating the 7 closest points" << std::endl;
    for (auto it = t.begin_nearest({0,0,0},7); it != t.end(); ++it) {
        std::cout << it->first << ": " << it->second << " - Distance to {0,0,0} = " << it->first.distance({0,0,0}) << std::endl;
        ++i;
    }
    // iterate from last to first
    i = 0;
    auto begin = t.begin_nearest({0,0,0},7);
    auto last_nearest_it = t.begin_nearest({0,0,0},7);
    while (last_nearest_it != t.end()) {
        ++last_nearest_it;
    }
    while (last_nearest_it != begin) {
        --last_nearest_it;
        std::cout << i << " - " << last_nearest_it->first << ": " << last_nearest_it->second << std::endl;
        ++i;
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Finding values" << std::endl;
    t.insert(v);
    it = t.find(v.first);
    REQUIRE(it != t.end());
    REQUIRE(it->first == v.first);
    REQUIRE(it->second == v.second);
    std::cout << it->first << ": " << it->second << std::endl;
    ++it;
    for (; it != t.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
        ++i;
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Erasing elements" << std::endl;
    std::vector<value_type> to_erase;
    i = 0;
    for (auto it = t.begin(); it != t.end(); ++it) {
        if (i % 2 == 0) {
            to_erase.emplace_back(*it);
        }
        ++i;
    }
    REQUIRE(i == t.size());
    size_t previous_size = t.size();
    for (auto& v: to_erase) {
        std::vector<value_type> previous_tree_elements(t.begin(), t.end());
        std::sort(previous_tree_elements.begin(), previous_tree_elements.end());
        std::cout << "Erasing v.first: " << v.first << std::endl;
        REQUIRE(t.erase(v) == 1);
        std::vector<value_type> current_tree_elements(t.begin(), t.end());
        std::sort(current_tree_elements.begin(), current_tree_elements.end());
        auto lb = std::lower_bound(previous_tree_elements.begin(), previous_tree_elements.end(), v);
        REQUIRE(lb != previous_tree_elements.end());
        previous_tree_elements.erase(lb);
        if (previous_tree_elements != current_tree_elements) {
            for (size_t i = 0; i < previous_tree_elements.size(); ++i) {
                std::cout << "[" << previous_tree_elements[i].first << ", ";
                std::cout <<        current_tree_elements[i].first  << "], " << std::endl;
            }
            REQUIRE(false);
        }
    }
    i = 0;
    for (auto it = t.begin(); it != t.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
        ++i;
    }
    REQUIRE(i == previous_size/2);
    std::cout << "---------------" << std::endl;

    std::cout << "Erasing with iterator" << std::endl;
    for (size_t i = 0; i < 120; ++i) {
        // insert some more
        value_type v(point_type({randn(), randn(), randn()}), randi());
        std::tie(it, ok) = t.insert(v);
        REQUIRE(ok);
        std::cout << i + 1 << " - " << v.first << ": " << v.second << std::endl;
        REQUIRE(it->first == v.first);
        REQUIRE(it->second == v.second);
    }
    size_t s = t.size();
    it = t.begin();
    t.erase(it);
    it = t.begin();
    std::advance(it,5);
    t.erase(it);
    REQUIRE(t.size() == s-2);
    std::cout << "---------------" << std::endl;

    std::cout << "Min/max values and elements" << std::endl;
    for (size_t i = 0; i < t.dimensions(); ++i) {
        auto max_it = std::max_element(t.begin(), t.end(), [&i](const value_type& a, const value_type& b) { return a.first[i] < b.first[i]; });
        auto min_it = std::min_element(t.begin(), t.end(), [&i](const value_type& a, const value_type& b) { return a.first[i] < b.first[i]; });
        std::cout << "t.max_value(" << i << "): " << t.max_value(i) << std::endl;
        std::cout << "t.min_value(" << i << "): " << t.min_value(i) << std::endl;
        std::cout << "std::max_element(t.begin(), t.end(), [&" << i << "](...))->first: " << max_it->first << std::endl;
        std::cout << "std::min_element(t.begin(), t.end(), [&" << i << "](...))->first: " << min_it->first << std::endl;
        std::cout << "t.max_element(" << i << ")->first: " << t.max_element(i)->first << std::endl;
        std::cout << "t.min_element(" << i << ")->first: " << t.min_element(i)->first << std::endl;
        REQUIRE(max_it->first[i] == t.max_value(i));
        REQUIRE(min_it->first[i] == t.min_value(i));
        REQUIRE(*max_it == *t.max_element(i));
        REQUIRE(*min_it == *t.min_element(i));
    }
    std::cout << "---------------" << std::endl;

    std::cout << "Copy constructor" << std::endl;
    tree_type t2(t);
    REQUIRE(t.size() == t2.size());
    auto tit = t.begin();
    auto t2it = t2.begin();
    for (; tit != t.end() && t2it != t2.end(); ++tit, ++t2it) {
        REQUIRE(*tit == *t2it);
    }
    std::cout << "---------------" << std::endl;
}

TEST_CASE("Quad-Tree") {
    test_tree<pareto_front::quad_tree<double, 0, unsigned>>();
    test_tree<pareto_front::quad_tree<double, 3, unsigned>>();
}

TEST_CASE("kd-Tree") {
    test_tree<pareto_front::kd_tree<double, 0, unsigned>>();
    test_tree<pareto_front::kd_tree<double, 3, unsigned>>();
}

TEST_CASE("R-Tree") {
    test_tree<pareto_front::r_tree<double, 0, unsigned>>();
    test_tree<pareto_front::r_tree<double, 3, unsigned>>();
}

TEST_CASE("R*-Tree") {
    test_tree<pareto_front::r_star_tree<double, 0, unsigned>>();
    test_tree<pareto_front::r_star_tree<double, 3, unsigned>>();
}

uint64_t seed() {
    static uint64_t seed = static_cast<uint64_t>(std::random_device()()) |
    std::chrono::high_resolution_clock::now().time_since_epoch().count();
//    static uint64_t seed = 0;
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

unsigned randi() {
    static std::uniform_int_distribution<unsigned> ud(0, 40);
    return ud(generator());;
}

double randu() {
    static std::uniform_real_distribution<double> ud(0., 1.);
    return ud(generator());
}

double randn() {
    static std::normal_distribution nd;
    return nd(generator());
}
