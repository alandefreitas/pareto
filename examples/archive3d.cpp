#include <chrono>
#include <iostream>
#include <map>
#include <pareto/archive.h>
#include <pareto/kd_tree.h>
#include <random>

int main() {
    using std::cout;
    using std::endl;
    using namespace pareto;
    // Constructor
    size_t capacity = 1000;
    archive<double, 3, unsigned> ar(capacity, {min, max, min});

    // Element access
    ar(-2.57664, -1.52034, 0.600798) = 17;
    ar(-2.14255, -0.518684, -2.92346) = 32;
    ar(-1.63295, 0.912108, -2.12953) = 36;
    ar(-0.653036, 0.927688, -0.813932) = 13;
    ar(-0.508188, 0.871096, -2.25287) = 32;
    ar(-2.55905, -0.271349, 0.898137) = 6;
    ar(-2.31613, -0.219302, 0) = 8;
    ar(-0.639149, 1.89515, 0.858653) = 10;
    ar(-0.401531, 2.30172, 0.58125) = 39;
    ar(0.0728106, 1.91877, 0.399664) = 25;
    ar(-1.09756, 1.33135, 0.569513) = 20;
    ar(-0.894115, 1.01387, 0.462008) = 11;
    ar(-1.45049, 1.35763, 0.606019) = 17;
    ar(0.152711, 1.99514, -0.112665) = 13;
    ar(-2.3912, 0.395611, 2.78224) = 11;
    ar(-0.00292544, 1.29632, -0.578346) = 20;
    ar(0.157424, 2.30954, -1.23614) = 6;
    ar(0.453686, 1.02632, -2.24833) = 30;
    ar(0.693712, 1.12267, -1.37375) = 12;
    ar(1.49101, 3.24052, 0.724771) = 24;

    if (ar.contains({1.49101, 3.24052, 0.724771})) {
        std::cout << "Element access: " << ar(1.49101, 3.24052, 0.724771) << std::endl;
    } else {
        std::cout << "{1.49101, 3.24052, 0.724771} was dominated" << std::endl;
    }

    // Iterators
    std::cout << "Iterators:" << std::endl;
    for (const auto& [point, value]: ar) {
        cout << point << " -> " << value << endl;
    }

    std::cout << "Reversed Iterators:" << std::endl;
    for (auto it = ar.rbegin(); it != ar.rend(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }

    std::cout << "Front Iterators:" << std::endl;
    for (auto it = ar.begin_front(); it != ar.end_front(); ++it) {
        cout << "Front with " << it->size() << " elements" << endl;
        for (const auto &[k, v] : *it) {
            cout << k << " -> " << v << endl;
        }
    }

    // Capacity and Reference Points
    if (ar.empty()) {
        cout << "Archive is empty" << endl;
    } else {
        cout << "Archive is not empty" << endl;
    }
    cout << ar.size() << " elements in the archive" << endl;
    cout << ar.dimensions() << " dimensions" << endl;
    for (size_t i = 0; i < ar.dimensions(); ++i) {
        if (ar.is_minimization(i)) {
            cout << "Dimension " << i << " is minimization" << endl;
        } else {
            cout << "Dimension " << i << " is maximization" << endl;
        }
        cout << "Best value in dimension " << i << ": " << ar.ideal(i) << endl;
        cout << "Min value in dimension " << i << ": " << ar.min_value(i) << endl;
        cout << "Max value in dimension " << i << ": " << ar.max_value(i) << endl;
        cout << "Best value in dimension " << i << ": " << ar.ideal(i) << endl;
        cout << "Nadir value in dimension " << i << ": " << ar.nadir(i) << endl;
        cout << "Worst value in dimension " << i << ": " << ar.worst(i) << endl;
    }
    std::cout << "Ideal point: " << ar.ideal() << std::endl;
    std::cout << "Nadir point: " << ar.nadir() << std::endl;
    std::cout << "Worst point: " << ar.worst() << std::endl;
    std::cout << "Capacity: " << ar.capacity() << std::endl;
    std::cout << "Number of fronts: " << ar.size_fronts() << std::endl;

    // Point-point dominance
    using point_type = archive<double, 3, unsigned>::key_type;
    point_type p1({0, 0, 0});
    point_type p2({1, 1, 1});
    std::vector<bool> is_minimization = {true, false, true};
    cout << (p1.dominates(p2, is_minimization) ? "p1 dominates p2" : "p1 does not dominate p2") << endl;
    cout << (p1.strongly_dominates(p2, is_minimization) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << endl;
    cout << (p1.non_dominates(p2, is_minimization) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << endl;

    // Archive-point dominance
    cout << (ar.dominates(p2) ? "ar dominates p2" : "ar does not dominate p2") << endl;
    cout << (ar.strongly_dominates(p2) ? "ar strongly dominates p2" : "ar does not strongly dominate p2") << endl;
    cout << (ar.non_dominates(p2) ? "ar non-dominates p2" : "ar does not non-dominate p2") << endl;
    cout << (ar.is_partially_dominated_by(p2) ? "ar is partially dominated by p2" : "ar is not is partially dominated by p2") << endl;
    cout << (ar.is_completely_dominated_by(p2) ? "ar is completely dominated by p2" : "ar is not is completely dominated by p2") << endl;

    // Archive-archive dominance
    archive<double, 3, unsigned> ar2({min, max, min});
    for (const auto& [p,v]: ar) {
        ar2[point_type({p[0] - 1, p[1] + 1, p[2] - 1})] = v;
    }
    cout << (ar.dominates(ar2) ? "ar dominates ar2" : "ar does not dominate ar2") << endl;
    cout << (ar.strongly_dominates(ar2) ? "ar strongly dominates ar2" : "ar does not strongly dominate ar2") << endl;
    cout << (ar.non_dominates(ar2) ? "ar non-dominates ar2" : "ar does not non-dominate ar2") << endl;
    cout << (ar.is_partially_dominated_by(ar2) ? "ar is partially dominated by ar2" : "ar is not is partially dominated by ar2") << endl;
    cout << (ar.is_completely_dominated_by(ar2) ? "ar is completely dominated by ar2" : "ar is not is completely dominated by ar2") << endl;

    // Indicators
    // Hypervolume
    cout << "Exact hypervolume: " << ar.hypervolume(ar.nadir()) << endl;
    cout << "Hypervolume approximation (10000 samples): " << ar.hypervolume(10000, ar.nadir()) << endl;

    // Coverage
    cout << "C-metric: " << ar.coverage(ar2) << endl;
    cout << "Coverage ratio: " << ar.coverage_ratio(ar2) << endl;
    cout << "C-metric: " << ar2.coverage(ar) << endl;
    cout << "Coverage ratio: " << ar2.coverage_ratio(ar) << endl;

    // Convergence
    archive<double, 3, unsigned> ar_star({min, max, min});
    for (const auto &[p,v] : ar) {
        ar_star(p[0] - 1.0, p[1] + 1.0, p[2] - 1.0) = v;
    }
    assert(ar.is_completely_dominated_by(ar_star));

    cout << "GD: " << ar.gd(ar_star) << endl;
    cout << "STDGD: " << ar.std_gd(ar_star) << endl;
    cout << "IGD: " << ar.igd(ar_star) << endl;
    cout << "STDGD: " << ar.std_igd(ar_star) << endl;
    cout << "Hausdorff: " << ar.hausdorff(ar_star) << endl;
    cout << "IGD+: " << ar.igd_plus(ar_star) << endl;
    cout << "STDIGD+: " << ar.std_igd_plus(ar_star) << endl;

    // Distribution
    cout << "Uniformity: " << ar.uniformity() << endl;
    cout << "Average distance: " << ar.average_distance() << endl;
    cout << "Average nearest distance: " << ar.average_nearest_distance(5) << endl;
    auto near_origin = ar.find_nearest({0.0, 0.0, 0.0});
    cout << "Crowding distance: " << ar.crowding_distance(near_origin) << endl;
    cout << "Average crowding distance: " << ar.average_crowding_distance() << endl;

    // Correlation
    cout << "Direct conflict(0,1): " << ar.direct_conflict(0,1) << endl;
    cout << "Normalized direct conflict(0,1): " << ar.normalized_direct_conflict(0,1) << endl;
    cout << "Maxmin conflict(0,1): " << ar.maxmin_conflict(0,1) << endl;
    cout << "Normalized maxmin conflict(0,1): " << ar.normalized_maxmin_conflict(0,1) << endl;
    cout << "Non-parametric conflict(0,1): " << ar.conflict(0,1) << endl;
    cout << "Normalized conflict(0,1): " << ar.normalized_conflict(0,1) << endl;

    cout << "Direct conflict(1,2): " << ar.direct_conflict(1,2) << endl;
    cout << "Normalized direct conflict(1,2): " << ar.normalized_direct_conflict(1,2) << endl;
    cout << "Maxmin conflict(1,2): " << ar.maxmin_conflict(1,2) << endl;
    cout << "Normalized maxmin conflict(1,2): " << ar.normalized_maxmin_conflict(1,2) << endl;
    cout << "Non-parametric conflict(1,2): " << ar.conflict(1,2) << endl;
    cout << "Normalized conflict(1,2): " << ar.normalized_conflict(1,2) << endl;

    // Modifiers
    ar.insert(std::make_pair(archive<double, 3, unsigned>::key_type({1.49101, 3.24052, 0.724771}), 24));
    ar.erase({1.49101, 3.24052, 0.724771});

    // Lookup and queries
    std::cout << "Lookup and queries" << std::endl;
    for (auto it = ar.find_intersection(ar.ideal(), {-2.3912, 0.395611, 2.78224}); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = ar.find_within(ar.ideal(), {-2.3912, 0.395611, 2.78224}); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = ar.find_disjoint(ar.worst(), {+0.71, +1.19, +0.98}); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = ar.find_nearest({-2.3912, 0.395611, 2.78224}, 2); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    auto it_near = ar.find_nearest({2.5, 2.5, 2.5});
    cout << it_near->first << " -> " << it_near->second << endl;
    for (auto it = ar.find_dominated({-10, +10, -10}); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (size_t i = 0; i < ar.dimensions(); ++i) {
        cout << "Ideal element in dimension " << i << ": " << ar.ideal_element(i)->first << endl;
        cout << "Nadir element in dimension " << i << ": " << ar.nadir_element(i)->first << endl;
        cout << "Worst element in dimension " << i << ": " << ar.worst_element(i)->first << endl;
    }

    // Observers
    auto fn = ar.dimension_comp();
    if (fn(2.,3.)) {
        std::cout << "2 is less than 3" << std::endl;
    } else {
        std::cout << "2 is not less than 3" << std::endl;
    }

    // Relational operators
    archive<double, 3, unsigned> ar3(ar);
    if (ar == ar3) {
        std::cout << "The archives have the same elements" << std::endl;
    } else {
        if (ar.size() != ar3.size()) {
            std::cout << "The archives do not have the same elements" << std::endl;
        } else {
            std::cout << "The archives might not have the same elements"
                      << std::endl;
        }
    }

    archive<double, 3, unsigned> ar4(ar.begin(), ar.end());
    if (ar == ar4) {
        std::cout << "The archives have the same elements" << std::endl;
    } else {
        if (ar.size() != ar4.size()) {
            std::cout << "The archives do not have the same elements" << std::endl;
        } else {
            std::cout << "The archives might not have the same elements"
                      << std::endl;
        }
    }

    if (ar_star < ar) {
        std::cout << "ar* dominates ar" << std::endl;
    } else {
        std::cout << "ar* does not dominate ar" << std::endl;
    }

    return 0;
}
