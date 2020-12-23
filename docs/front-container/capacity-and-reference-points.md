---
layout: default
title: Capacity and Reference Points
nav_order: 7
has_children: false
parent: Front Container
has_toc: false
---
# Capacity and Reference Points

| Method                                                       |
| ------------------------------------------------------------ |
| **MultimapContainer**                                        |
| Check size                                                   |
| `[[nodiscard]] bool empty() const noexcept;`                 |
| `[[nodiscard]] size_type size() const noexcept;`             |
| `[[nodiscard]] size_type max_size() const noexcept;`         |
| **SpatialContainer**                                         |
| Check dimensions                                             |
| `[[nodiscard]] size_type dimensions() const noexcept;`       |
| Get max/min values                                           |
| `dimension_type max_value(size_type dimension) const;`          |
| `dimension_type min_value(size_type dimension) const;`          |
| **FrontContainer**                                          |
| Reference points                                             |
| `key_type ideal() const;`                                    |
| `dimension_type ideal(size_type dimension) const;`              |
| `key_type nadir() const;`                                    |
| `dimension_type nadir(size_type dimension) const;`              |
| `key_type worst() const;`                                    |
| `dimension_type worst(size_type dimension) const;`              |
| Target directions                                            |
| `[[nodiscard]] bool is_minimization() const noexcept`        |
| `[[nodiscard]] bool is_maximization() const noexcept`        |
| `[[nodiscard]] bool is_minimization(size_t dimension) const noexcept` |
| `[[nodiscard]] bool is_maximization(size_t dimension) const noexcept` |

**Parameters**

* `dimension` - index of the dimension for which we want the minimum or maximum value

**Return value**

* `empty()`- `true` if and only if container (equivalent but more efficient than `begin() == end()`)
* `size()` - The number of elements in the container
* `max_size()` - An upper bound on the maximum number of elements the container can hold
* `dimensions()` - Number of dimensions in the container (same as `M`, when `M != 0`)
* `max_value(d)` - Maximum value in a given dimension `d`
* `min_value(d)` - Minimum value in a given dimension `d`
* `ideal()` - Key with best value possible in each dimension
* `ideal(d)` - Best value possible in a given dimension `d`
* `nadir()` - Key with worst value possible in each dimension
* `nadir(d)` - Worst value possible in a given dimension `d`
* `worst()` - Key with worst value possible in each dimension
* `worst(d)` - Worst value possible in a given dimension `d`
* `is_minimization()`, `is_maximization()`: true if and only if all directions are minimization / maximization
* `is_minimization(i)`, `is_maximization(i)`: true if and only if dimension `i` is minimization / maximization

**Complexity**

$$
O(1)
$$

**Notes**

Because all container nodes keep their minimum bounding rectangles, we can find these values in constant time. These reference points are important components of other queries and indicators for fronts, so it's useful to obtain these values in constant time. 

!!! info "The nadir point" 
    Although `nadir` and `worst` return the same values for fronts, they are semantically different and, do not return the same values for archives. The nadir point refers to the worst objective values over the efficient set of values in a multiobjective optimization problem, while the worst point simply refers to the worst values in a container.

    The nadir point approximation is usually obtained by iteratively optimizing a problem as $m$ uni-dimensional problems. The best estimate of the nadir point happens to be the worst point here because the front container doesn't have enough information about the underlying problem. This, however, is not the case for the archive container, which is the reason why we keep this distinction here.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    if (pf.empty()) {
        std::cout << "Front is empty" << std::endl;
    } else {
        std::cout << "Front is not empty" << std::endl;
    }
    std::cout << pf.size() << " elements in the front" << std::endl;
    std::cout << pf.dimensions() << " dimensions" << std::endl;
    for (size_t i = 0; i < pf.dimensions(); ++i) {
        if (pf.is_minimization(i)) {
            std::cout << "Dimension " << i << " is minimization" << std::endl;
        } else {
            std::cout << "Dimension " << i << " is maximization" << std::endl;
        }
        std::cout << "Best value in dimension " << i << ": " << pf.ideal(i) << std::endl;
        std::cout << "Min value in dimension " << i << ": " << pf.min_value(i) << std::endl;
        std::cout << "Max value in dimension " << i << ": " << pf.max_value(i) << std::endl;
        std::cout << "Best value in dimension " << i << ": " << pf.ideal(i) << std::endl;
        std::cout << "Nadir value in dimension " << i << ": " << pf.nadir(i) << std::endl;
        std::cout << "Worst value in dimension " << i << ": " << pf.worst(i) << std::endl;
    }
    std::cout << "Ideal point: " << pf.ideal() << std::endl;
    std::cout << "Nadir point: " << pf.nadir() << std::endl;
    std::cout << "Worst point: " << pf.worst() << std::endl;
    ```

=== "Python"

    ```python
    if pf:
        print('Front is not empty')
    else:
        print('Front is empty')
    
    print(len(pf), 'elements in the front')
    print(pf.dimensions(), 'dimensions')
    for i in range(pf.dimensions()):
        if pf.is_minimization(i):
            print('Dimension', i, ' is minimization')
        else:
            print('Dimension', i, ' is maximization')
        print('Best value in dimension', i, ':', pf.ideal(i))
        print('Min value in dimension', i, ':', pf.min_value(i))
        print('Max value in dimension', i, ':', pf.max_value(i))
        print('Best value in dimension', i, ':', pf.ideal(i))
        print('Nadir value in dimension', i, ':', pf.nadir(i))
        print('Worst value in dimension', i, ':', pf.worst(i))
    
    print('Ideal point:', pf.ideal())
    print('Nadir point:', pf.nadir())
    print('Worst point:', pf.worst())
    
    ```

=== "Output"

    ```console
    Front is not empty
    20 elements in the front
    3 dimensions
    Dimension 0 is minimization
    Best value in dimension 0: -2.57664
    Min value in dimension 0: -2.57664
    Max value in dimension 0: 1.49101
    Best value in dimension 0: -2.57664
    Nadir value in dimension 0: 1.49101
    Worst value in dimension 0: 1.49101
    Dimension 1 is maximization
    Best value in dimension 1: 3.24052
    Min value in dimension 1: -1.52034
    Max value in dimension 1: 3.24052
    Best value in dimension 1: 3.24052
    Nadir value in dimension 1: -1.52034
    Worst value in dimension 1: -1.52034
    Dimension 2 is minimization
    Best value in dimension 2: -2.92346
    Min value in dimension 2: -2.92346
    Max value in dimension 2: 2.78224
    Best value in dimension 2: -2.92346
    Nadir value in dimension 2: 2.78224
    Worst value in dimension 2: 2.78224
    Ideal point: [-2.57664, 3.24052, -2.92346]
    Nadir point: [1.49101, -1.52034, 2.78224]
    Worst point: [1.49101, -1.52034, 2.78224]
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
