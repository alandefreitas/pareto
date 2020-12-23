---
layout: default
title: Observers
nav_order: 12
has_children: false
parent: Archive Container
has_toc: false
---
# Observers

| Method                                                       |
| ------------------------------------------------------------ |
| **Multimap**                                                 |
| Returns the function that compares keys                      |
| `key_compare key_comp() const noexcept;`                     |
| Returns the function that compares keys in objects of type value_type |
| `value_compare value_comp() const noexcept`                  |
| **SpatialMap**                                               |
| Returns the function that compares keys in a single dimension |
| `dimension_compare dimension_comp() const noexcept;`               |

**Return value**

A callable function that compares dimensions, keys, or values.

**Complexity**

$$
O(1)
$$

**Notes**

These functions return copies of the container's constructor argument `comp`, or a wrappers around these copies. 

**Example**

=== "C++"

    ```cpp
    auto fn = pf.dimension_comp();
    if (fn(2.,3.)) {
        std::cout << "2 is less than 3" << std::endl;
    } else {
        std::cout << "2 is not less than 3" << std::endl;
    }
    ```

=== "Output"

    ```console
    2 is less than 3
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
