---
layout: default
title: Containers
nav_order: 1
has_children: false
parent: Spatial Containers
has_toc: false
---
# Containers

Just like you can create a uni-dimensional map with:

=== "C++"

    ```cpp
    std::multimap<double, unsigned> m1;
    // or
    std::unordered_map<double, unsigned> m2;
    ```

=== "Python"

    ```python
    m1 = sortedcontainers.SortedDict()
    # or
    m2 = dict()
    ```

Spatial containers allow you to create an $m$-dimensional map with something like:

=== "C++"

    ```cpp
    pareto::spatial_map<double, 2, unsigned> m1;
    pareto::spatial_map<double, 3, unsigned> m2;
    pareto::spatial_map<double, 4, unsigned> m3;
    pareto::spatial_map<double, 5, unsigned> m4;
    ```

=== "Python"

    ```python
    # The dimension will be set when you insert the first point
    m1 = pareto.spatial_map()
    ```

A `spatial_map` is currently defined as an alias to an `r_tree`. If you want to be specific about which data structure to use, you can directly define:

=== "C++"

    ```cpp
    pareto::r_tree<double, 3, unsigned> m1;
    pareto::r_star_tree<double, 3, unsigned> m2;
    pareto::kd_tree<double, 3, unsigned> m3;
    pareto::quad_tree<double, 3, unsigned> m4;
    pareto::implicit_tree<double, 3, unsigned> m5;
    ```

=== "Python"

    ```python
    m1 = pareto.r_tree()
    m2 = pareto.r_star_tree()
    m3 = pareto.kd_tree()
    m4 = pareto.quad_tree()
    m5 = pareto.implicit_tree()
    ```

Here's a summary of what each container is good at:

| Container       | Best Application                                             | Optimal |
| --------------- | ------------------------------------------------------------ | ------- |
| `kd_tree`       | Non-uniformly distributed objects                           | Yes     |
| `r_tree`        | Non-uniformly distributed objects that might overlap in space | Yes     |
| `r_star_tree`   | Same as `r_tree` with more expensive insertion and less expensive queries | Yes     |
| `quad_tree`     | Uniformly distributed objects                               | No      |
| `implicit_tree` | Benchmarks only                                              | No      |

Although `pareto::front` and `pareto::archive` also implement the *SpatialContainer* concept, they serve a different purpose we discuss in Sections [Front Concept](../front-container/front-concept.md) and [Archive Concept](../archive-container/archive-concept.md). However, their interface remains unchanged for the most common use cases:

=== "C++"

    ```cpp
    pareto::front<double, 3, unsigned> pf;
    pareto::archive<double, 3, unsigned> ar;
    ```

=== "Python"

    ```python
    pf = pareto.front()
    ar = pareto.archive()
    ```

!!! info "Complexity"
    * Containers with optimal asymptotic complexity have a $O(m \log n)$ cost to search, insert and remove elements.

    * Quadtrees do not have optimal asymptotic complexity because removing elements might require reconstructing subtrees with cost $O(m n \log n)$. 

    * The container `implicit_tree` is emulates a tree with a `std::vector`. You can think of it as a multidimensional [`flat_map`](https://www.boost.org/doc/libs/1_75_0/doc/html/boost/container/flat_map.html). However, unlike a flat map, sorting the elements in a single dimension does not make operations much unless $m \leq 3$. Its basic operations cost $O(mn)$ and it's mostly used as a reference for our benchmarks.



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
