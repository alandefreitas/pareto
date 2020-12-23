---
layout: default
title: Distribution
nav_order: 4
has_children: false
parent: Indicators
grand_parent: Front Container
has_toc: false
---
# Distribution

| Method                                                       |
| ------------------------------------------------------------ |
| **FrontContainer**                                          |
| Front Distribution                                           |
| `[[nodiscard]] double uniformity() const`                    |
| `[[nodiscard]] double average_distance() const`              |
| `[[nodiscard]] double average_nearest_distance(size_t k = 5) const` |
| `[[nodiscard]] double average_crowding_distance() const`     |
| Point Distribution                                           |
| `double crowding_distance(const_iterator element, key_type worst_point, key_type ideal_point) const` |
| `double crowding_distance(const_iterator element) const`     |
| `double crowding_distance(const key_type &point) const`    |

**Parameters**

* `k` - number of nearest elements to consider
* `element` - element for which we want the crowding distance (see below)
* `key_type` - point for which we want the crowding distance (see below)
* `worst_point`, `ideal_point` - reference extreme points for the crowding distance

**Return value**

* `uniformity`: minimal distance between two points in the front
* `average_distance`: average distance between points in the front
* `average_nearest_distance`: average distance between points and their nearest points
* `average_crowding_distance`: average crowding distance (see below) between points in the front
* `crowding_distance`: the crowding distance of a single element

**Complexity**

* `uniformity`: $O(m n \log n)$
* `average_distance`: $O(m n^2)$
* `average_nearest_distance`: $O(k m n \log n)$
* `average_crowding_distance`: $O(m n \log n)$
* `crowding_distance`: $O(m \log n)$

**Notes**

Distribution indicators measure how uniformly the points are distributed on the front. This is useful for a better approximation of the target front.

!!! info "The Crowding Distance"
    The crowding distance indicator replaces the usual euclidean distance between points ($\sqrt{\sum_{i=1}^m (p_i - q_i)^2}$) with the coordinate distance between the nearest points in each dimension ($\sum_{i=1}^m | p_i - nearest_{1}(p_i, m) | + | p_i - nearest_2(p_i, m)|$).

**Example**

=== "C++"

    ```cpp
    std::cout << "Uniformity: " << pf.uniformity() << std::endl;
    std::cout << "Average distance: " << pf.average_distance() << std::endl;
    std::cout << "Average nearest distance: " << pf.average_nearest_distance(5) << std::endl;
    auto near_origin = pf.find_nearest({0.0, 0.0, 0.0});
    std::cout << "Crowding distance: " << pf.crowding_distance(near_origin) << std::endl;
    std::cout << "Average crowding distance: " << pf.average_crowding_distance() << std::endl;
    ```

=== "Python"

    ```python
    print("Uniformity:", pf.uniformity())
    print("Average distance:", pf.average_distance())
    print("Average nearest distance:", pf.average_nearest_distance(5))
    near_origin = next(pf.find_nearest(pareto.point([0.0, 0.0])))
    print("Crowding distance:", pf.crowding_distance(near_origin[0]))
    print("Average crowding distance:", pf.average_crowding_distance())
    ```

=== "Output"

    ```console
    Uniformity: 0.355785
    Average distance: 2.75683
    Average nearest distance: 1.45177
    Crowding distance: 3.04714
    Average crowding distance: 4.04349
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
