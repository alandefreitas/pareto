---
layout: default
title: Hypervolume
nav_order: 1
has_children: false
parent: Indicators
grand_parent: Front Container
has_toc: false
---
# Hypervolume

| Method                                                       |
| ------------------------------------------------------------ |
| **FrontContainer**                                          |
| Exact Hypervolume                                            |
| `dimension_type hypervolume() const`                            |
| `dimension_type hypervolume(key_type reference_point) const`  |
| Monte-Carlo Hypervolume                                      |
| `dimension_type hypervolume(size_t sample_size) const`          |
| `dimension_type hypervolume(size_t sample_size, const key_type &reference_point) const` |

**Parameters**

* `reference_point` - point used as reference for the hypervolume calculation. When not provided, it defaults to the `nadir()` point.
* `sample_size` - number of samples for the hypervolume estimate

**Return value**

* Hypervolume indicator

**Complexity**

* Exact hypervolume: $O(n^{m-2} \log n)$
* Monte-Carlo hypervolume approximation:  $O(s m \log n)$, where $s$ is the number of samples

**Notes**

Because the solutions in a front are incomparable, we need performance indicators to infer the quality of a front. [Indicators](http://www.optimization-online.org/DB_FILE/2018/10/6887.pdf) can measure several front attributes, such as cardinality, convergence, distribution, and spread. Correlation indicators can also estimate the relationship between objectives in a set.

The most popular indicator of a front quality is its hypervolume, as it measures both convergence and distribution quality. The front hypervolume refers to the total hypervolume dominated by the front. 

!!! info "Hypervolume Approximation"
    When $m$ is large, the exact hypervolume calculation becomes impractical. Our benchmarks provide a reference on the impact of these approximations.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    std::cout << "Exact hypervolume: " << pf.hypervolume(pf.nadir()) << std::endl;
    std::cout << "Hypervolume approximation (10000 samples): " << pf.hypervolume(10000, pf.nadir()) << std::endl;
    ```

=== "Python"

    ```python
    print('Exact hypervolume:', pf.hypervolume(pf.nadir()))
    print('Hypervolume approximation (10000 samples):', pf.hypervolume(10000, pf.nadir()))
    ```

=== "Output"

    ```console
    Exact hypervolume: 55.4029
    Hypervolume approximation (10000 samples): 54.4734
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
