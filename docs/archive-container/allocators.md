---
layout: default
title: Allocators
nav_order: 4
has_children: false
parent: Archive Container
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

Because an archive handles a set of containers, some which are exponentially smaller than other, the archive does not delegate the task of handling the allocators to its underlying containers. Instead, the archive keeps a copy of the allocator and constructs each new underlying container with this allocator. If the archive contains a PMR allocator, the new container also shares the same memory resources. This is intended to make memory allocations faster for the last fronts, which are usually allocated and deallocated often with few elements. 

!!! info
    See the section on spatial map allocators for more information.

**Example**

=== "C++"

    ```cpp
    #include <pareto/archive.h>
    // ...
    pareto::archive<double, 3, unsigned> ar;
    // Get a copy of the container allocator
    auto alloc = ar.get_allocator();
    ```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
