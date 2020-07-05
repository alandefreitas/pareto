#include <iostream>
#include <random>
#include <chrono>
#include <map>
#include <pareto_front/pareto_front.h>

int main() {
    using std::cout;
    using std::endl;
    using namespace pareto_front;
    front<double, 3, unsigned> pf({minimization, maximization, minimization});

    pf(-2.57664, -1.52034, 0.600798) = 17;
    pf(-2.14255, -0.518684, -2.92346) = 32;
    pf(-1.63295, 0.912108, -2.12953) = 36;
    pf(-0.653036, 0.927688, -0.813932) = 13;
    pf(-0.508188, 0.871096, -2.25287) = 32;
    pf(-2.55905, -0.271349, 0.898137) = 6;
    pf(-2.31613, -0.219302, 0) = 8;
    pf(-0.639149, 1.89515, 0.858653) = 10;
    pf(-0.401531, 2.30172, 0.58125) = 39;
    pf(0.0728106, 1.91877, 0.399664) = 25;
    pf(-1.09756, 1.33135, 0.569513) = 20;
    pf(-0.894115, 1.01387, 0.462008) = 11;
    pf(-1.45049, 1.35763, 0.606019) = 17;
    pf(0.152711, 1.99514, -0.112665) = 13;
    pf(-2.3912, 0.395611, 2.78224) = 11;
    pf(-0.00292544, 1.29632, -0.578346) = 20;
    pf(0.157424, 2.30954, -1.23614) = 6;
    pf(0.453686, 1.02632, -2.24833) = 30;
    pf(0.693712, 1.12267, -1.37375) = 12;
    pf(1.49101, 3.24052, 0.724771) = 24;

    pf.erase({1.49101, 3.24052, 0.724771});

    cout << pf << endl;
    cout << pf.size() << " elements in the front" << endl;
    if (!pf.empty()) {
        cout << "Front is not empty" << endl;
    }
    cout << pf.dimensions() << " dimensions" << endl;
    cout << (pf.is_minimization() ? "All" : "Not all") << " dimensions are minimization" << endl;
    cout << "Dimension 0 is " << (pf.is_minimization(0) ? "minimization" : "not minimization") << endl;
    cout << "Dimension 1 is " << (pf.is_maximization(1) ? "maximization" : "not maximization") << endl;
    cout << "Dimension 2 is " << (pf.is_minimization(2) ? "minimization" : "not minimization") << endl;

    cout << "Ideal point: " << pf.ideal() << endl;
    cout << "Nadir point: " << pf.nadir() << endl;
    cout << "Ideal point in dimension 0: " << pf.dimension_ideal(0)->first << endl;
    cout << "Ideal point in dimension 1: " << pf.dimension_ideal(1)->first << endl;
    cout << "Ideal point in dimension 2: " << pf.dimension_ideal(2)->first << endl;
    cout << "Nadir point in dimension 0: " << pf.dimension_nadir(0)->first << endl;
    cout << "Nadir point in dimension 1: " << pf.dimension_nadir(1)->first << endl;
    cout << "Nadir point in dimension 2: " << pf.dimension_nadir(2)->first << endl;

    pf.insert(std::make_pair(front<double, 3, unsigned>::point_type({1.49101, 3.24052, 0.724771}), 24));

    for (const auto& [point, value]: pf) {
        cout << point << " -> " << value << endl;
    }
    for (auto it = pf.rbegin(); it != pf.rend(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }

    for (auto it = pf.find_intersection(pf.ideal(), {-2.3912, 0.395611, 2.78224}); it != pf.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = pf.find_within(pf.ideal(), {-2.3912, 0.395611, 2.78224}); it != pf.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = pf.find_disjoint(pf.worst(), {+0.71, +1.19, +0.98}); it != pf.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = pf.find_nearest({-2.3912, 0.395611, 2.78224}, 2); it != pf.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    auto it = pf.find_nearest({2.5, 2.5, 2.5});
    cout << it->first << " -> " << it->second << endl;

    pareto_front<double, 3, unsigned>::point_type p1({0, 0});
    pareto_front<double, 3, unsigned>::point_type p2({1, 1});
    cout << (p1.dominates(p2) ? "p1 dominates p2" : "p1 does not dominate p2") << endl;
    cout << (p1.strongly_dominates(p2) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << endl;
    cout << (p1.non_dominates(p2) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << endl;

    cout << (pf.dominates(p2) ? "pf dominates p2" : "pf does not dominate p2") << endl;
    cout << (pf.strongly_dominates(p2) ? "pf strongly dominates p2" : "pf does not strongly dominate p2") << endl;
    cout << (pf.non_dominates(p2) ? "pf non-dominates p2" : "pf does not non-dominate p2") << endl;
    cout << (pf.is_partially_dominated_by(p2) ? "pf is partially dominated by p2" : "pf is not is partially dominated by p2") << endl;
    cout << (pf.is_completely_dominated_by(p2) ? "pf is completely dominated by p2" : "pf is not is completely dominated by p2") << endl;

    front<double, 3, unsigned> pf2({minimization, maximization});
    for (const auto& [p,v]: pf) {
        pf2[p - 1] = v;
    }

    cout << (pf.dominates(pf2) ? "pf dominates pf2" : "pf does not dominate pf2") << endl;
    cout << (pf.strongly_dominates(pf2) ? "pf strongly dominates pf2" : "pf does not strongly dominate pf2") << endl;
    cout << (pf.non_dominates(pf2) ? "pf non-dominates pf2" : "pf does not non-dominate pf2") << endl;
    cout << (pf.is_partially_dominated_by(pf2) ? "pf is partially dominated by pf2" : "pf is not is partially dominated by pf2") << endl;
    cout << (pf.is_completely_dominated_by(pf2) ? "pf is completely dominated by pf2" : "pf is not is completely dominated by pf2") << endl;

    cout << "Exact hypervolume: " << pf.hypervolume(pf.nadir()) << endl;
    cout << "Hypervolume approximation (10000 samples): " << pf.hypervolume(pf.nadir(), 10000) << endl;

    cout << "C-metric: " << pf.coverage(pf2) << endl;
    cout << "Coverage ratio: " << pf.coverage_ratio(pf2) << endl;

    front<double, 3, unsigned> pf_star({minimization, maximization});
    for (const auto &[p,v] : pf) {
        pf_star(p[0] - 1.0, p[1] + 1.0, p[1] - 1.0) = v;
    }
    assert(pf.is_completely_dominated_by(pf_star));

    cout << "GD: " << pf.gd(pf_star) << endl;
    cout << "STDGD: " << pf.std_gd(pf_star) << endl;
    cout << "IGD: " << pf.igd(pf_star) << endl;
    cout << "STDGD: " << pf.std_igd(pf_star) << endl;
    cout << "Hausdorff: " << pf.hausdorff(pf_star) << endl;
    cout << "IGD+: " << pf.igd_plus(pf_star) << endl;
    cout << "STDIGD+: " << pf.std_igd_plus(pf_star) << endl;

    cout << "Uniformity: " << pf.uniformity() << endl;
    cout << "Average distance: " << pf.average_distance() << endl;
    cout << "Average nearest distance: " << pf.average_nearest_distance(5) << endl;
    auto near_origin = pf.find_nearest({0.0, 0.0, 0.0});
    cout << "Crowding distance: " << pf.crowding_distance(near_origin) << endl;
    cout << "Average crowding distance: " << pf.average_crowding_distance() << endl;

    cout << "Direct conflict(0,1): " << pf.direct_conflict(0,1) << endl;
    cout << "Normalized direct conflict(0,1): " << pf.normalized_direct_conflict(0,1) << endl;
    cout << "Maxmin conflict(0,1): " << pf.maxmin_conflict(0,1) << endl;
    cout << "Normalized maxmin conflict(0,1): " << pf.normalized_maxmin_conflict(0,1) << endl;
    cout << "Non-parametric conflict(0,1): " << pf.conflict(0,1) << endl;
    cout << "Normalized conflict(0,1): " << pf.normalized_conflict(0,1) << endl;

    cout << "Direct conflict(1,2): " << pf.direct_conflict(1,2) << endl;
    cout << "Normalized direct conflict(1,2): " << pf.normalized_direct_conflict(1,2) << endl;
    cout << "Maxmin conflict(1,2): " << pf.maxmin_conflict(1,2) << endl;
    cout << "Normalized maxmin conflict(1,2): " << pf.normalized_maxmin_conflict(1,2) << endl;
    cout << "Non-parametric conflict(1,2): " << pf.conflict(1,2) << endl;
    cout << "Normalized conflict(1,2): " << pf.normalized_conflict(1,2) << endl;

    return 0;
}
