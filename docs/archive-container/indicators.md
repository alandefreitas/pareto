---
layout: default
title: Indicators
nav_order: 9
has_children: false
parent: Archive Container
has_toc: false
---
# Indicators

| Method                                                       |
| ------------------------------------------------------------ |
| **FrontContainer**                                          |
| Exact Hypervolume                                            |
| `dimension_type hypervolume() const`                            |
| `dimension_type hypervolume(key_type reference_point) const`  |
| Monte-Carlo Hypervolume                                      |
| `dimension_type hypervolume(size_t sample_size) const`          |
| `dimension_type hypervolume(size_t sample_size, const key_type &reference_point) const` |
| Cardinality                             |
| `double coverage(const front &rhs) const`       |
| `double coverage_ratio(const front &rhs) const` |
| Convergence Indicators                              |
| `double gd(const front &reference) const`           |
| `double igd(const front &reference) const`          |
| `double igd_plus(const front &reference) const`     |
| `double hausdorff(const front &reference) const`    |
| Standard deviation of Convergence Indicators        |
| `double std_gd(const front &reference) const`       |
| `double std_igd(const front &reference) const`      |
| `double std_igd_plus(const front &reference) const` |
| First Front Distribution                                           |
| `[[nodiscard]] double uniformity() const`                    |
| `[[nodiscard]] double average_distance() const`              |
| `[[nodiscard]] double average_nearest_distance(size_t k = 5) const` |
| `[[nodiscard]] double average_crowding_distance() const`     |
| Point Distribution                                           |
| `double crowding_distance(const_iterator element, key_type worst_point, key_type ideal_point) const` |
| `double crowding_distance(const_iterator element) const`     |
| `double crowding_distance(const key_type &point) const`    |
| Conflict / Harmony                                           |
| `dimension_type direct_conflict(const size_t a, const size_t b) const` |
| `[[nodiscard]] double maxmin_conflict(const size_t a, const size_t b) const` |
| `[[nodiscard]] double conflict(const size_t a, const size_t b) const` |
| Normalized Conflict / Harmony                                |
| `[[nodiscard]] double normalized_direct_conflict(const size_t a, const size_t b) const` |
| `[[nodiscard]] double normalized_maxmin_conflict(const size_t a, const size_t b) const` |
| `[[nodiscard]] double normalized_conflict(const size_t a, const size_t b) const` |
| **ArchiveContainer**                             |
| Cardinality                             |
| `double coverage(const front &rhs) const`       |
| `double coverage_ratio(const front &rhs) const` |
| Convergence Indicators                              |
| `double gd(const front &reference) const`           |
| `double igd(const front &reference) const`          |
| `double igd_plus(const front &reference) const`     |
| `double hausdorff(const front &reference) const`    |
| Standard deviation of Convergence Indicators        |
| `double std_gd(const front &reference) const`       |
| `double std_igd(const front &reference) const`      |
| `double std_igd_plus(const front &reference) const` |

**Parameters**

* `reference_point` - point used as reference for the hypervolume calculation. When not provided, it defaults to the `nadir()` point.
* `sample_size` - number of samples for the hypervolume estimate
* `rhs` - front or archive being compared
* `reference` - Target front. An estimate of the best front possible for the problem.
* `k` - number of nearest elements to consider
* `element` - element for which we want the crowding distance (see below)
* `key_type` - point for which we want the crowding distance (see below)
* `worst_point`, `ideal_point` - reference extreme points for the crowding distance
* `a`, `b` - dimension indices

**Return value**

* (see section Indicators for fronts)

**Complexity**

* (see section Indicators for fronts)

**Notes**

The archive indicators use their first front as reference.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    // Hypervolume
    std::cout << "Exact hypervolume: " << ar.hypervolume(ar.nadir()) << std::endl;
    std::cout << "Hypervolume approximation (10000 samples): " << ar.hypervolume(10000, ar.nadir()) << std::endl;
    
    // Coverage
    std::cout << "C-metric: " << ar.coverage(ar2) << std::endl;
    std::cout << "Coverage ratio: " << ar.coverage_ratio(ar2) << std::endl;
    std::cout << "C-metric: " << ar2.coverage(ar) << std::endl;
    std::cout << "Coverage ratio: " << ar2.coverage_ratio(ar) << std::endl;
    
    // Convergence
    archive<double, 3, unsigned> ar_star({min, max, min});
    for (const auto &[p,v] : ar) {
        ar_star(p[0] - 1.0, p[1] + 1.0, p[2] - 1.0) = v;
    }
    assert(ar.is_completely_dominated_by(ar_star));
    
    std::cout << "GD: " << ar.gd(ar_star) << std::endl;
    std::cout << "STDGD: " << ar.std_gd(ar_star) << std::endl;
    std::cout << "IGD: " << ar.igd(ar_star) << std::endl;
    std::cout << "STDGD: " << ar.std_igd(ar_star) << std::endl;
    std::cout << "Hausdorff: " << ar.hausdorff(ar_star) << std::endl;
    std::cout << "IGD+: " << ar.igd_plus(ar_star) << std::endl;
    std::cout << "STDIGD+: " << ar.std_igd_plus(ar_star) << std::endl;
    
    // Distribution
    std::cout << "Uniformity: " << ar.uniformity() << std::endl;
    std::cout << "Average distance: " << ar.average_distance() << std::endl;
    std::cout << "Average nearest distance: " << ar.average_nearest_distance(5) << std::endl;
    auto near_origin = ar.find_nearest({0.0, 0.0, 0.0});
    std::cout << "Crowding distance: " << ar.crowding_distance(near_origin) << std::endl;
    std::cout << "Average crowding distance: " << ar.average_crowding_distance() << std::endl;
    
    // Correlation
    std::cout << "Direct conflict(0,1): " << ar.direct_conflict(0,1) << std::endl;
    std::cout << "Normalized direct conflict(0,1): " << ar.normalized_direct_conflict(0,1) << std::endl;
    std::cout << "Maxmin conflict(0,1): " << ar.maxmin_conflict(0,1) << std::endl;
    std::cout << "Normalized maxmin conflict(0,1): " << ar.normalized_maxmin_conflict(0,1) << std::endl;
    std::cout << "Non-parametric conflict(0,1): " << ar.conflict(0,1) << std::endl;
    std::cout << "Normalized conflict(0,1): " << ar.normalized_conflict(0,1) << std::endl;
    
    std::cout << "Direct conflict(1,2): " << ar.direct_conflict(1,2) << std::endl;
    std::cout << "Normalized direct conflict(1,2): " << ar.normalized_direct_conflict(1,2) << std::endl;
    std::cout << "Maxmin conflict(1,2): " << ar.maxmin_conflict(1,2) << std::endl;
    std::cout << "Normalized maxmin conflict(1,2): " << ar.normalized_maxmin_conflict(1,2) << std::endl;
    std::cout << "Non-parametric conflict(1,2): " << ar.conflict(1,2) << std::endl;
    std::cout << "Normalized conflict(1,2): " << ar.normalized_conflict(1,2) << std::endl;
    ```

=== "Python"

    ```python
    # Hypervolume
    print('Exact hypervolume:', ar.hypervolume(ar.nadir()))
    print('Hypervolume approximation (10000 samples):', ar.hypervolume(10000, ar.nadir()))
    
    # Coverage
    print('C-metric:', ar.coverage(ar2))
    print('Coverage ratio:', ar.coverage_ratio(ar2))
    print('C-metric:', ar2.coverage(ar))
    print('Coverage ratio:', ar2.coverage_ratio(ar))
    
    # Convergence
    ar_star = pareto.archive(['min', 'max', 'min'])
    for [p, v] in ar:
        ar_star[p[0] - 1.0, p[1] + 1.0, p[2] - 1.0] = v
    
    print('GD:', ar.gd(ar_star))
    print('STDGD:', ar.std_gd(ar_star))
    print('IGD:', ar.igd(ar_star))
    print('STDGD:', ar.std_igd(ar_star))
    print('Hausdorff:', ar.hausdorff(ar_star))
    print('IGD+:', ar.igd_plus(ar_star))
    print('STDIGD+:', ar.std_igd_plus(ar_star))
    
    # Distribution
    print("Uniformity:", ar.uniformity())
    print("Average distance:", ar.average_distance())
    print("Average nearest distance:", ar.average_nearest_distance(5))
    near_origin = next(ar.find_nearest(pareto.point([0.0, 0.0])))
    print("Crowding distance:", ar.crowding_distance(near_origin[0]))
    print("Average crowding distance:", ar.average_crowding_distance())
    
    # Correlation
    print('Direct conflict(0,1):', ar.direct_conflict(0, 1))
    print('Normalized direct conflict(0,1):', ar.normalized_direct_conflict(0, 1))
    print('Maxmin conflict(0,1):', ar.maxmin_conflict(0, 1))
    print('Normalized maxmin conflict(0,1):', ar.normalized_maxmin_conflict(0, 1))
    print('Non-parametric conflict(0,1):', ar.conflict(0, 1))
    print('Normalized conflict(0,1):', ar.normalized_conflict(0, 1))
    
    print('Direct conflict(1,2):', ar.direct_conflict(1, 2))
    print('Normalized direct conflict(1,2):', ar.normalized_direct_conflict(1, 2))
    print('Maxmin conflict(1,2):', ar.maxmin_conflict(1, 2))
    print('Normalized maxmin conflict(1,2):', ar.normalized_maxmin_conflict(1, 2))
    print('Non-parametric conflict(1,2):', ar.conflict(1, 2))
    print('Normalized conflict(1,2):', ar.normalized_conflict(1, 2))
    ```

=== "Output"

    ```console
    Exact hypervolume: 55.4029
    Hypervolume approximation (10000 samples): 56.0535
    C-metric: 0
    Coverage ratio: 0
    C-metric: 1
    Coverage ratio: inf
    GD: 1.54786
    STDGD: 0.0465649
    IGD: 1.52137
    STDGD: 0.0472864
    Hausdorff: 1.54786
    IGD+: 1.48592
    STDIGD+: 0.0522492
    Uniformity: 0.355785
    Average distance: 2.75683
    Average nearest distance: 1.45177
    Crowding distance: 3.04714
    Average crowding distance: 4.04349
    Direct conflict(0,1): 34.3539
    Normalized direct conflict(0,1): 0.360795
    Maxmin conflict(0,1): 7.77615
    Normalized maxmin conflict(0,1): 0.388808
    Non-parametric conflict(0,1): 184
    Normalized conflict(0,1): 0.92
    Direct conflict(1,2): 32.0107
    Normalized direct conflict(1,2): 0.280515
    Maxmin conflict(1,2): 5.85805
    Normalized maxmin conflict(1,2): 0.292903
    Non-parametric conflict(1,2): 146
    Normalized conflict(1,2): 0.73
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
