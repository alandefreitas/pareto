---
layout: default
title: Capacity and Reference Points
nav_order: 7
has_children: false
parent: Archive Container
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
| **ArchiveContainer**                                          |
| `[[nodiscard]] size_t capacity() const noexcept` |
| `size_type size_fronts() const noexcept` |

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
* `capacity()`: maximum number of elements in the archive
* `size_front()`: number of fronts in the archive

**Complexity**

$$
O(1)
$$

**Notes**

For archives, the `nadir` and `worst` function will not necessarily return the same points and values because the worst point in a given dimension does have to match the worst value in the first front of the archive.

!!! note
    All other requirements of fronts apply here.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    if (ar.empty()) {
        std::cout << "Front is empty" << std::endl;
    } else {
        std::cout << "Front is not empty" << std::endl;
    }
    std::cout << ar.size() << " elements in the front" << std::endl;
    std::cout << ar.dimensions() << " dimensions" << std::endl;
    for (size_t i = 0; i < ar.dimensions(); ++i) {
        if (ar.is_minimization(i)) {
            std::cout << "Dimension " << i << " is minimization" << std::endl;
        } else {
            std::cout << "Dimension " << i << " is maximization" << std::endl;
        }
        std::cout << "Best value in dimension " << i << ": " << ar.ideal(i) << std::endl;
        std::cout << "Min value in dimension " << i << ": " << ar.min_value(i) << std::endl;
        std::cout << "Max value in dimension " << i << ": " << ar.max_value(i) << std::endl;
        std::cout << "Best value in dimension " << i << ": " << ar.ideal(i) << std::endl;
        std::cout << "Nadir value in dimension " << i << ": " << ar.nadir(i) << std::endl;
        std::cout << "Worst value in dimension " << i << ": " << ar.worst(i) << std::endl;
    }
    std::cout << "Ideal point: " << ar.ideal() << std::endl;
    std::cout << "Nadir point: " << ar.nadir() << std::endl;
    std::cout << "Worst point: " << ar.worst() << std::endl;
    std::cout << "Capacity: " << ar.capacity() << std::endl;
    std::cout << "Number of fronts: " << ar.size_fronts() << std::endl;
    ```

=== "Python"

    ```python
    if ar:
        print('Front is not empty')
    else:
        print('Front is empty')
    
    print(len(ar), 'elements in the front')
    print(ar.dimensions(), 'dimensions')
    for i in range(ar.dimensions()):
        if ar.is_minimization(i):
            print('Dimension', i, ' is minimization')
        else:
            print('Dimension', i, ' is maximization')
        print('Best value in dimension', i, ':', ar.ideal(i))
        print('Min value in dimension', i, ':', ar.min_value(i))
        print('Max value in dimension', i, ':', ar.max_value(i))
        print('Best value in dimension', i, ':', ar.ideal(i))
        print('Nadir value in dimension', i, ':', ar.nadir(i))
        print('Worst value in dimension', i, ':', ar.worst(i))
    
    print('Ideal point:', ar.ideal())
    print('Nadir point:', ar.nadir())
    print('Worst point:', ar.worst())
    print('Capacity:', ar.capacity())
    print('Number of fronts:', ar.size_fronts())
    ```

=== "Output"

    ```console
    Archive is not empty
    20 elements in the archive
    3 dimensions
    Dimension 0  is minimization
    Best value in dimension 0 : -2.57664
    Min value in dimension 0 : -2.57664
    Max value in dimension 0 : 1.49101
    Best value in dimension 0 : -2.57664
    Nadir value in dimension 0 : 1.49101
    Worst value in dimension 0 : 1.49101
    Dimension 1  is maximization
    Best value in dimension 1 : 3.24052
    Min value in dimension 1 : -1.52034
    Max value in dimension 1 : 3.24052
    Best value in dimension 1 : 3.24052
    Nadir value in dimension 1 : -1.52034
    Worst value in dimension 1 : -1.52034
    Dimension 2  is minimization
    Best value in dimension 2 : -2.92346
    Min value in dimension 2 : -2.92346
    Max value in dimension 2 : 2.78224
    Best value in dimension 2 : -2.92346
    Nadir value in dimension 2 : 2.78224
    Worst value in dimension 2 : 2.78224
    Ideal point: [-2.57664, 3.24052, -2.92346]
    Nadir point: [1.49101, -1.52034, 2.78224]
    Worst point: [1.49101, -1.52034, 2.78224]
    Capacity: 1000
    Number of fronts: 1
    ```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
