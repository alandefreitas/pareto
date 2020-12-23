---
layout: default
title: Allocators
nav_order: 4
has_children: false
parent: Front Container
has_toc: false
---
# Allocators

| Method                                           |
| ------------------------------------------------ |
| **AllocatorAwareContainer**                      |
| `allocator_type get_allocator() const noexcept;` |

**Return value**

The associated allocator.

**Complexity**

$$
O(1)
$$

**Notes**

This function returns the allocator of the underlying container. 

!!! info
    See the section on spatial map allocators for more information.

**Example**

=== "C++"

    ```cpp
    #include <pareto/front.h>
    // ...
    pareto::front<double, 3, unsigned> pf;
    // Get a copy of the container allocator
    auto alloc = pf.get_allocator();
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
