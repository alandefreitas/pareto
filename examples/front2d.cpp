#include <chrono>
#include <iostream>
#include <map>
#include <pareto/front.h>
#include <random>

int main() {
    using std::cout;
    using std::endl;
    using namespace pareto;
    front<double, 2, unsigned> pf({minimization, maximization});

    pf(0.68322, 0.545438) = 17;
    pf(-0.204484, 0.819538) = 8;
    pf(1.10158, 0.977164) = 36;
    pf(-2.01773, -1.25209) = 27;
    pf(-1.18976, 1.04908) = 15;
    pf(1.00638, -0.988565) = 33;
    pf(-1.20382, 0.288829) = 23;
    pf(-0.230579, -1.90872) = 16;
    pf(-1.25501, 0.366831) = 17;
    pf(-1.5753, -0.600874) = 31;
    pf(0.623662, -0.835896) = 14;
    pf(-2.31613, -0.219302) = 8;
    pf(1.51927, -0.923787) = 12;
    pf(0.288334, -0.215865) = 7;
    pf(0.187418, -0.619002) = 40;
    pf(-0.283076, -1.58758) = 2;
    pf(-0.477429, 0.670057) = 7;
    pf(0.516849, 1.18725) = 0;
    pf(1.42374, 0.183388) = 27;

    pf.erase({-2.31613, -0.219302});

    cout << pf << endl;
    cout << pf.size() << " elements in the front" << endl;
    if (!pf.empty()) {
        cout << "Front is not empty" << endl;
    }
    cout << pf.dimensions() << " dimensions" << endl;
    cout << (pf.is_minimization() ? "All" : "Not all") << " dimensions are minimization" << endl;
    cout << "Dimension 0 is " << (pf.is_minimization(0) ? "minimization" : "not minimization") << endl;
    cout << "Dimension 1 is " << (pf.is_maximization(1) ? "maximization" : "not maximization") << endl;

    cout << "Ideal point: " << pf.ideal() << endl;
    cout << "Nadir point: " << pf.nadir() << endl;
    cout << "Ideal point in dimension 0: " << pf.dimension_ideal(0)->first << endl;
    cout << "Ideal point in dimension 1: " << pf.dimension_ideal(1)->first << endl;
    cout << "Nadir point in dimension 0: " << pf.dimension_nadir(0)->first << endl;
    cout << "Nadir point in dimension 1: " << pf.dimension_nadir(1)->first << endl;

    pf.insert(std::make_pair(front<double, 2, unsigned>::point_type({-2.31613, -0.219302}), 8));

    for (const auto& [point, value]: pf) {
        cout << point << " -> " << value << endl;
    }
    for (auto it = pf.rbegin(); it != pf.rend(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }

    for (auto it = pf.find_intersection(pf.ideal(), {-1.21188, -1.24192}); it != pf.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = pf.find_within(pf.ideal(), {-1.21188, -1.24192}); it != pf.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = pf.find_disjoint(pf.worst(), {+0.71, +1.19}); it != pf.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = pf.find_nearest({-1.21188, -1.24192}, 2); it != pf.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    auto it = pf.find_nearest({2.5, 2.5});
    cout << it->first << " -> " << it->second << endl;

    front<double, 2, unsigned>::point_type p1({0, 0});
    front<double, 2, unsigned>::point_type p2({1, 1});
    cout << (p1.dominates(p2) ? "p1 dominates p2" : "p1 does not dominate p2") << endl;
    cout << (p1.strongly_dominates(p2) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << endl;
    cout << (p1.non_dominates(p2) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << endl;

    cout << (pf.dominates(p2) ? "pf dominates p2" : "pf does not dominate p2") << endl;
    cout << (pf.strongly_dominates(p2) ? "pf strongly dominates p2" : "pf does not strongly dominate p2") << endl;
    cout << (pf.non_dominates(p2) ? "pf non-dominates p2" : "pf does not non-dominate p2") << endl;
    cout << (pf.is_partially_dominated_by(p2) ? "pf is partially dominated by p2" : "pf is not is partially dominated by p2") << endl;
    cout << (pf.is_completely_dominated_by(p2) ? "pf is completely dominated by p2" : "pf is not is completely dominated by p2") << endl;

    front<double, 2, unsigned> pf2({minimization, maximization});
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

    front<double, 2, unsigned> pf_star({minimization, maximization});
    for (const auto &[p,v] : pf) {
        pf_star(p[0] - 1.0, p[1] + 1.0) = v;
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
    auto near_origin = pf.find_nearest({0.0,0.0});
    cout << "Crowding distance: " << pf.crowding_distance(near_origin) << endl;
    cout << "Average crowding distance: " << pf.average_crowding_distance() << endl;

    cout << "Direct conflict: " << pf.direct_conflict(0,1) << endl;
    cout << "Normalized direct conflict: " << pf.normalized_direct_conflict(0,1) << endl;
    cout << "Maxmin conflict: " << pf.maxmin_conflict(0,1) << endl;
    cout << "Normalized maxmin conflict: " << pf.normalized_maxmin_conflict(0,1) << endl;
    cout << "Non-parametric conflict: " << pf.conflict(0,1) << endl;
    cout << "Normalized conflict: " << pf.normalized_conflict(0,1) << endl;

    return 0;
}
