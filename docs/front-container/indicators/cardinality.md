---
layout: default
title: Cardinality
nav_order: 2
has_children: false
parent: Indicators
grand_parent: Front Container
has_toc: false
---
# Cardinality

| Method                                          |
| ----------------------------------------------- |
| **FrontContainer**                             |
| `double coverage(const front &rhs) const`       |
| `double coverage_ratio(const front &rhs) const` |

**Parameters**

* `rhs` - front being compared

**Return value**

* C-metric indicator

**Complexity**

$$
O(m n \log n)
$$

**Notes**

Cardinality indicators compare two fronts and indicate how many points in one front are non-dominated by points in the other front. The coverage ratio compares which front dominates more points in the other with `lhs.coverage(rhs) / rhs.coverage(lhs)`.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    std::cout << "C-metric: " << pf.coverage(pf2) << std::endl;
    std::cout << "Coverage ratio: " << pf.coverage_ratio(pf2) << std::endl;
    std::cout << "C-metric: " << pf2.coverage(pf) << std::endl;
    std::cout << "Coverage ratio: " << pf2.coverage_ratio(pf) << std::endl;
    ```

=== "Python"

    ```python
    print('C-metric:', pf.coverage(pf2))
    print('Coverage ratio:', pf.coverage_ratio(pf2))
    print('C-metric:', pf2.coverage(pf))
    print('Coverage ratio:', pf2.coverage_ratio(pf))
    ```

=== "Output"

    ```console
    C-metric: 0
    Coverage ratio: 0
    C-metric: 1
    Coverage ratio: inf
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
