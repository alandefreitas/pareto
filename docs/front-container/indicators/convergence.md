---
layout: default
title: Convergence
nav_order: 3
has_children: false
parent: Indicators
grand_parent: Front Container
has_toc: false
---
# Convergence

| Method                                              |
| --------------------------------------------------- |
| **FrontContainer**                                 |
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

* `reference` - Target front. An estimate of the best front possible for the problem.

**Return value**

* How far the current front is from the reference front

**Complexity**

$$
O(m n \log n)
$$

**Notes**

Convergence indicators measure the distance from a front approximation to the exact Pareto front, or at least a better approximation of the exact front.

**Example**

=== "C++"

    ```cpp
    front<double, 3, unsigned> pf_star({min, max, min});
    for (const auto &[p,v] : pf) {
        pf_star(p[0] - 1.0, p[1] + 1.0, p[2] - 1.0) = v;
    }
    std::cout << "GD: " << pf.gd(pf_star) << std::endl;
    std::cout << "STDGD: " << pf.std_gd(pf_star) << std::endl;
    std::cout << "IGD: " << pf.igd(pf_star) << std::endl;
    std::cout << "STDGD: " << pf.std_igd(pf_star) << std::endl;
    std::cout << "Hausdorff: " << pf.hausdorff(pf_star) << std::endl;
    std::cout << "IGD+: " << pf.igd_plus(pf_star) << std::endl;
    std::cout << "STDIGD+: " << pf.std_igd_plus(pf_star) << std::endl;
    ```

=== "Python"

    ```python
    pf_star = pareto.front(['min', 'max', 'min'])
    for [p, v] in pf:
        pf_star[p[0] - 1.0, p[1] + 1.0, p[2] - 1.0] = v
    
    print('GD:', pf.gd(pf_star))
    print('STDGD:', pf.std_gd(pf_star))
    print('IGD:', pf.igd(pf_star))
    print('STDGD:', pf.std_igd(pf_star))
    print('Hausdorff:', pf.hausdorff(pf_star))
    print('IGD+:', pf.igd_plus(pf_star))
    print('STDIGD+:', pf.std_igd_plus(pf_star))
    ```

=== "Output"

    ```console
    GD: 1.54786
    STDGD: 0.0465649
    IGD: 1.52137
    STDGD: 0.0472864
    Hausdorff: 1.54786
    IGD+: 1.48592
    STDIGD+: 0.0522492
    ```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
