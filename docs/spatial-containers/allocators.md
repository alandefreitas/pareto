---
layout: default
title: Allocators
nav_order: 4
has_children: false
parent: Spatial Containers
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

One of the reasons associative containers perform much worse than sequence containers for small containers is that associative containers, being internally represented as trees, require one memory allocation for each new element. An allocator is an object that defines how memory is allocated for a container. Because tree nodes usually have fixed size, pool allocators for associative containers usually allocate a large block of memory for nodes before new nodes are created. Thus, associative containers can have a performance similar to sequential containers even when the container has few elements.  


!!! info "The Allocator Concept"
    An allocator must implement the [Allocator](https://en.cppreference.com/w/cpp/named_req/Allocator) concept, while an allocator aware container must implement the [*AllocatorAwareContainer*](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer) concept, which includes constructors accepting allocators as parameters. Internally, a container that is allocator aware should use only the allocator to create new nodes.

Besides the constructors defined in the previous section, spatial containers also define the function `allocator_type get_allocator() const;` to return the current allocator being used by the container. If two allocators compare equal, that means they use the same memory resources. When two containers do not use the same allocator, the move constructor costs $O(mn)$ instead of $O(1)$.

!!! info "Default Allocator"
    By default, all containers in this library use a `std::pmr::polymorphic_allocator` with an internal `std::pmr::unsynchronized_pool_resource` as their default allocator (see our [Benchmarks](../benchmarks/index.md)). 

!!! warning "PMR implementations"
    Because many compilers haven't completely implemented `std::pmr` yet, the build script will look for `std::pmr` and fallback to `std::allocator` if `std::pmr` is not available yet.

!!! note "Note on previous versions of Pareto"
    Previous versions of this library included a stateful memory allocator based on pools and slots. Because the C++ requirements for allocators are not kind to simple stateful allocators whose elements have fixed size, our allocator ended up looking more and more like a simpler version of the `std::pmr::polymorphic_allocator`. 
    Fortunately, these `std::pmr` is now part of the standard library and our containers are now allocator aware, so you can just use `pmr` or any other efficient allocator for these containers. 

**Example**

=== "C++"

    ```cpp
    #include <pareto/spatial_map.h>
    // ...
    pareto::spatial_map<double, 3, unsigned> m;
    // Get a copy of the container allocator
    auto alloc = m.get_allocator();
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
