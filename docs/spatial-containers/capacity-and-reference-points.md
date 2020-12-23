---
layout: default
title: Capacity and Reference Points
nav_order: 7
has_children: false
parent: Spatial Containers
has_toc: false
---
# Capacity and Reference Points

| Method                                               |
| ---------------------------------------------------- |
| **MultimapContainer**                                |
| Check size                                           |
| `[[nodiscard]] bool empty() const noexcept;`         |
| `[[nodiscard]] size_type size() const noexcept;`     |
| `[[nodiscard]] size_type max_size() const noexcept;` |
| **SpatialContainer**                                 |
| Check dimensions                                     |
| `[[nodiscard]] size_t dimensions() const noexcept;`  |
| Get max/min values                                   |
| `dimension_type max_value(size_t dimension) const;`     |
| `dimension_type min_value(size_t dimension) const;`     |

**Parameters**

* `dimension` - index of the dimension for which we want the minimum or maximum value

**Return value**

* `empty()`- `true` if and only if container (equivalent but more efficient than `begin() == end()`)
* `size()` - The number of elements in the container
* `max_size()` - An upper bound on the maximum number of elements the container can hold
* `dimensions()` - Number of dimensions in the container (same as `M`, when `M != 0`)
* `max_value()` - Maximum value in a given dimension
* `min_value()` - Minimum value in a given dimension

**Complexity**

$$
O(1)
$$

**Notes**

Because all container nodes keep their minimum bounding rectangles, we can get these values in constant time.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    if (!m.empty()) {
        std::cout << "Map is not empty" << std::endl;
    } else {
        std::cout << "Map is empty" << std::endl;
    }
    std::cout << m.size() << " elements in the spatial map" << std::endl;
    std::cout << m.dimensions() << " dimensions" << std::endl;
    for (size_t i = 0; i < m.dimensions(); ++i) {
        std::cout << "Min value in dimension " << i << ": " << m.min_value(i) << std::endl;
        std::cout << "Max value in dimension " << i << ": " << m.max_value(i) << std::endl;
    }
    ```

=== "Python"

    ```python
    if m:
        print('Map is not empty')
    else:
        print('Map is empty')

    print(len(m), 'elements in the spatial map')
    print(m.dimensions(), 'dimensions')
    for i in range(m.dimensions()):
        print('Min value in dimension', i, ': ', m.min_value(i))
        print('Max value in dimension', i, ': ', m.max_value(i))
    
    ```

=== "Output"

    ```console
    Map is not empty
    20 elements in the spatial map
    3 dimensions
    Min value in dimension 0: -2.57664
    Max value in dimension 0: 1.49101
    Min value in dimension 1: -1.52034
    Max value in dimension 1: 3.24052
    Min value in dimension 2: -2.92346
    Max value in dimension 2: 2.78224
    ```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
