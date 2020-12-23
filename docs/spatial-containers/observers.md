---
layout: default
title: Observers
nav_order: 10
has_children: false
parent: Spatial Containers
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

!!! info "Observers"
    These observers are useful in template functions that might receive spatial containers unknown to the function.    
    
    Most applications don't really need these observers. If you created the container, you already know the container compares its keys.  

=== "C++"

    ```cpp
    auto fn = m.dimension_comp();
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
