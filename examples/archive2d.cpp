#include <iostream>
#include <random>
#include <chrono>
#include <map>
#include <pareto/archive.h>

int main() {
    using std::cout;
    using std::endl;
    using namespace pareto;
    archive<double, 2, unsigned> ar(100, {minimization, maximization});

    ar(0.68322, 0.545438) = 17;
    ar(-0.204484, 0.819538) = 8;
    ar(1.10158, 0.977164) = 36;
    ar(-2.01773, -1.25209) = 27;
    ar(-1.18976, 1.04908) = 15;
    ar(1.00638, -0.988565) = 33;
    ar(-1.20382, 0.288829) = 23;
    ar(-0.230579, -1.90872) = 16;
    ar(-1.25501, 0.366831) = 17;
    ar(-1.5753, -0.600874) = 31;
    ar(0.623662, -0.835896) = 14;
    ar(-2.31613, -0.219302) = 8;
    ar(1.51927, -0.923787) = 12;
    ar(0.288334, -0.215865) = 7;
    ar(0.187418, -0.619002) = 40;
    ar(-0.283076, -1.58758) = 2;
    ar(-0.477429, 0.670057) = 7;
    ar(0.516849, 1.18725) = 0;
    ar(1.42374, 0.183388) = 27;

    ar.erase({-2.31613, -0.219302});

    cout << ar << endl;
    cout << ar.size() << " elements in the archive" << endl;
    if (!ar.empty()) {
        cout << "Front is not empty" << endl;
    }
    cout << ar.dimensions() << "dimensions" << endl;
    cout << (ar.is_minimization() ? "All" : "Not all") << " dimensions are minimization" << endl;
    cout << "Dimension 0 is " << (ar.is_minimization(0) ? "minimization" : "not minimization") << endl;
    cout << "Dimension 1 is " << (ar.is_maximization(0) ? "maximization" : "not maximization") << endl;

    cout << "Ideal point: " << ar.ideal() << endl;
    cout << "Nadir point: " << ar.nadir() << endl;
    cout << "Worst point: " << ar.worst() << endl;
    cout << "Ideal point in dimension 0: " << ar.dimension_ideal(0)->first << endl;
    cout << "Ideal point in dimension 1: " << ar.dimension_ideal(1)->first << endl;
    cout << "Nadir point in dimension 0: " << ar.dimension_nadir(0)->first << endl;
    cout << "Nadir point in dimension 1: " << ar.dimension_nadir(1)->first << endl;
    cout << "Worst point in dimension 0: " << ar.dimension_worst(0)->first << endl;
    cout << "Worst point in dimension 1: " << ar.dimension_worst(1)->first << endl;

    ar.insert(std::make_pair(archive<double, 2, unsigned>::point_type({-2.31613, -0.219302}), 8));

    for (const auto& [point, value]: ar) {
        cout << point << " -> " << value << endl;
    }

    for (auto it = ar.rbegin(); it != ar.rend(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }

    for (auto it = ar.find_intersection(ar.ideal(), {-1.21188, -1.24192}); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = ar.find_within(ar.ideal(), {-1.21188, -1.24192}); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = ar.find_disjoint(ar.worst(), {+0.71, +1.19}); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    for (auto it = ar.find_nearest({-1.21188, -1.24192}, 2); it != ar.end(); ++it) {
        cout << it->first << " -> " << it->second << endl;
    }
    auto it = ar.find_nearest({2.5, 2.5});
    cout << it->first << " -> " << it->second << endl;

    archive<double, 2, unsigned>::point_type p1({0, 0});
    archive<double, 2, unsigned>::point_type p2({1, 1});
    cout << (p1.dominates(p2) ? "p1 dominates p2" : "p1 does not dominate p2") << endl;
    cout << (p1.strongly_dominates(p2) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << endl;
    cout << (p1.non_dominates(p2) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << endl;

    cout << (ar.dominates(p2) ? "ar dominates p2" : "ar does not dominate p2") << endl;
    cout << (ar.strongly_dominates(p2) ? "ar strongly dominates p2" : "ar does not strongly dominate p2") << endl;
    cout << (ar.non_dominates(p2) ? "ar non-dominates p2" : "ar does not non-dominate p2") << endl;
    cout << (ar.is_partially_dominated_by(p2) ? "ar is partially dominated by p2" : "ar is not is partially dominated by p2") << endl;
    cout << (ar.is_completely_dominated_by(p2) ? "ar is completely dominated by p2" : "ar is not is completely dominated by p2") << endl;

    archive<double, 2, unsigned> ar2(100, {minimization, maximization});
    for (const auto& [p,v]: ar) {
        ar2[p - 1] = v;
    }

    cout << (ar.dominates(ar2) ? "ar dominates ar2" : "ar does not dominate ar2") << endl;
    cout << (ar.strongly_dominates(ar2) ? "ar strongly dominates ar2" : "ar does not strongly dominate ar2") << endl;
    cout << (ar.non_dominates(ar2) ? "ar non-dominates ar2" : "ar does not non-dominate ar2") << endl;
    cout << (ar.is_partially_dominated_by(ar2) ? "ar is partially dominated by ar2" : "ar is not is partially dominated by ar2") << endl;
    cout << (ar.is_completely_dominated_by(ar2) ? "ar is completely dominated by ar2" : "ar is not is completely dominated by ar2") << endl;

    cout << "Exact hypervolume: " << ar.hypervolume(ar.nadir()) << endl;
    cout << "Hypervolume approximation (10000 samples): " << ar.hypervolume(ar.nadir(), 10000) << endl;

    cout << "C-metric: " << ar.coverage(ar2) << endl;
    cout << "Coverage ratio: " << ar.coverage_ratio(ar2) << endl;

    archive<double, 2, unsigned> ar_star(100, {minimization, maximization});
    for (const auto &[p,v] : ar) {
        ar_star(p[0] - 1.0, p[1] + 1.0) = v;
    }
    assert(ar.is_completely_dominated_by(ar_star));

    cout << "GD: " << ar.gd(ar_star) << endl;
    cout << "STDGD: " << ar.std_gd(ar_star) << endl;
    cout << "IGD: " << ar.igd(ar_star) << endl;
    cout << "STDGD: " << ar.std_igd(ar_star) << endl;
    cout << "Hausdorff: " << ar.hausdorff(ar_star) << endl;
    cout << "IGD+: " << ar.igd_plus(ar_star) << endl;
    cout << "STDIGD+: " << ar.std_igd_plus(ar_star) << endl;

    cout << "Uniformity: " << ar.uniformity() << endl;
    cout << "Average distance: " << ar.average_distance() << endl;
    cout << "Average nearest distance: " << ar.average_nearest_distance(5) << endl;
    auto near_origin = ar.find_nearest({0.0,0.0});
    cout << "Crowding distance: " << ar.crowding_distance(near_origin) << endl;
    cout << "Average crowding distance: " << ar.average_crowding_distance() << endl;

    cout << "Direct conflict: " << ar.direct_conflict(0,1) << endl;
    cout << "Normalized direct conflict: " << ar.normalized_direct_conflict(0,1) << endl;
    cout << "Maxmin conflict: " << ar.maxmin_conflict(0,1) << endl;
    cout << "Normalized maxmin conflict: " << ar.normalized_maxmin_conflict(0,1) << endl;
    cout << "Non-parametric conflict: " << ar.conflict(0,1) << endl;
    cout << "Normalized conflict: " << ar.normalized_conflict(0,1) << endl;

    return 0;
}
