---
layout: default
title: Correlation
nav_order: 5
has_children: false
parent: Indicators
grand_parent: Front Container
has_toc: false
---
# Correlation

| Method                                                       |
| ------------------------------------------------------------ |
| **FrontContainer**                                          |
| Conflict / Harmony                                           |
| `dimension_type direct_conflict(const size_t a, const size_t b) const` |
| `[[nodiscard]] double maxmin_conflict(const size_t a, const size_t b) const` |
| `[[nodiscard]] double conflict(const size_t a, const size_t b) const` |
| Normalized Conflict / Harmony                                |
| `[[nodiscard]] double normalized_direct_conflict(const size_t a, const size_t b) const` |
| `[[nodiscard]] double normalized_maxmin_conflict(const size_t a, const size_t b) const` |
| `[[nodiscard]] double normalized_conflict(const size_t a, const size_t b) const` |

**Parameters**

* `a`, `b` - dimension indices

**Return value**

* The direct, max-min, or non-parametric conflict between two objectives. 
* The normalized indicators divide the results by the maximum value possible for that correlation indicator.

**Complexity**

* Direct: $O(n)$
* Max-min: $O(n)$
* Non-parametric: $O(n \log n)$

**Notes**

[Correlation indicators](https://www.sciencedirect.com/science/article/pii/S0020025514011347?casa_token=wnN_oqBidiEAAAAA:zi9MWYuwle_I0ECfqoMibXr-xGkjsgs86mE2-S0pu2pSnkv-mgyR2wpT5X4cnv7s2gsm0lc8DWg) can measure the relationship between objectives in a front. The more conflict between a pair of objectives, the more important it is to focus on these objectives. Objectives with little conflict are good candidates to be latter aggregated into a simpler objective function.

**Example**

=== "C++"

    ```cpp
    std::cout << "Direct conflict(0,1): " << pf.direct_conflict(0,1) << std::endl;
    std::cout << "Normalized direct conflict(0,1): " << pf.normalized_direct_conflict(0,1) << std::endl;
    std::cout << "Maxmin conflict(0,1): " << pf.maxmin_conflict(0,1) << std::endl;
    std::cout << "Normalized maxmin conflict(0,1): " << pf.normalized_maxmin_conflict(0,1) << std::endl;
    std::cout << "Non-parametric conflict(0,1): " << pf.conflict(0,1) << std::endl;
    std::cout << "Normalized conflict(0,1): " << pf.normalized_conflict(0,1) << std::endl;

    std::cout << "Direct conflict(1,2): " << pf.direct_conflict(1,2) << std::endl;
    std::cout << "Normalized direct conflict(1,2): " << pf.normalized_direct_conflict(1,2) << std::endl;
    std::cout << "Maxmin conflict(1,2): " << pf.maxmin_conflict(1,2) << std::endl;
    std::cout << "Normalized maxmin conflict(1,2): " << pf.normalized_maxmin_conflict(1,2) << std::endl;
    std::cout << "Non-parametric conflict(1,2): " << pf.conflict(1,2) << std::endl;
    std::cout << "Normalized conflict(1,2): " << pf.normalized_conflict(1,2) << std::endl;
    ```

=== "Python"

    ```python
    print('Direct conflict(0,1):', pf.direct_conflict(0, 1))
    print('Normalized direct conflict(0,1):', pf.normalized_direct_conflict(0, 1))
    print('Maxmin conflict(0,1):', pf.maxmin_conflict(0, 1))
    print('Normalized maxmin conflict(0,1):', pf.normalized_maxmin_conflict(0, 1))
    print('Non-parametric conflict(0,1):', pf.conflict(0, 1))
    print('Normalized conflict(0,1):', pf.normalized_conflict(0, 1))
    
    print('Direct conflict(1,2):', pf.direct_conflict(1, 2))
    print('Normalized direct conflict(1,2):', pf.normalized_direct_conflict(1, 2))
    print('Maxmin conflict(1,2):', pf.maxmin_conflict(1, 2))
    print('Normalized maxmin conflict(1,2):', pf.normalized_maxmin_conflict(1, 2))
    print('Non-parametric conflict(1,2):', pf.conflict(1, 2))
    print('Normalized conflict(1,2):', pf.normalized_conflict(1, 2))
    ```

=== "Output"

    ```console
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
