---
layout: default
title: Constructors
nav_order: 3
has_children: false
parent: Spatial Containers
has_toc: false
---
# Constructors

The constructors defined by `pareto::spatial_map<K,M,T,C,A>::spatial_map` (or any other [spatial container](containers.md)) instantiate new containers from a variety of data sources and optionally using a user supplied allocator `alloc` or comparison function object `comp`.

| Method                                                       |
| ------------------------------------------------------------ |
| **Container** + **AllocatorAwareContainer**     |
| `explicit spatial_map(const allocator_type &alloc = allocator_type())` |
| `spatial_map(const spatial_map &rhs)`                        |
| `spatial_map(const spatial_map &rhs, const allocator_type &alloc)` |
| `spatial_map(spatial_map &&rhs) noexcept`                    |
| `spatial_map(spatial_map &&rhs, const allocator_type &alloc) noexcept` |
| **AssociativeContainer** + **AllocatorAwareContainer** |
| `explicit spatial_map(const C &comp, const allocator_type &alloc = allocator_type())` |
| `template <class InputIt> spatial_map(InputIt first, InputIt last, const C &comp = C(), const allocator_type &alloc = allocator_type())` |
| `spatial_map(std::initializer_list<value_type> il, const C &comp = C(), const allocator_type &alloc = allocator_type())` |
| `template <class InputIt> spatial_map(InputIt first, InputIt last, const allocator_type &alloc)` |
| `spatial_map(std::initializer_list<value_type> il, const allocator_type &alloc)` |
| **AssociativeContainer** + **AllocatorAwareContainer** Assignment |
| `spatial_map &operator=(const spatial_map &rhs)`             |
| `spatial_map &operator=(spatial_map &&rhs) noexcept`         |
| **AssociativeContainer** Assignment                          |
| `spatial_map &operator=(std::initializer_list<value_type> il) noexcept` |

**Parameters**

| Parameter       | Description                                                  |
| --------------- | ------------------------------------------------------------ |
| `alloc`         | allocator to use for all memory allocations of this container |
| `comp`          | comparison function object to use for all comparisons of keys |
| `first`, `last` | the range to copy the elements from                          |
| `rhs`           | another container to be used as source to initialize the elements of the container with |
| `il`            | initializer list to initialize the elements of the container with |

**Requirements**

| Type requirements                                            |
| ------------------------------------------------------------ |
| -`InputIt` must meet the requirements of [*LegacyInputIterator*](https://en.cppreference.com/w/cpp/named_req/InputIterator). |
| -`Compare` must meet the requirements of [*Compare*](https://en.cppreference.com/w/cpp/named_req/Compare). |
| -`Allocator` must meet the requirements of [*Allocator*](https://en.cppreference.com/w/cpp/named_req/Allocator). |

**Complexity**

| Method                              | Complexity                                         |
| ----------------------------------- | -------------------------------------------------- |
| Empty constructor                   | $O(1)$                                             |
| Copy constructor                    | $O(mn)$                                            |
| Move constructor                    | $O(1)$ if `get_allocator() == rhs.get_allocator()` |
| Construct from range, or assignment | $O(m n \log n)$                                    |

**Example**

=== "C++"

    ```cpp
    #include <pareto/spatial_map.h>
    #include <pareto/kd_tree.h>
    // ...
    // Constructing the default spatial map
    pareto::spatial_map<double, 3, unsigned> m;
    // Constructing a kd-tree spatial map
    pareto::kd_tree<double, 3, unsigned> m;
    ```

=== "Python"

    ```python
    import pareto
    # ...
    # Constructing the default spatial map
    m = pareto.spatial_map() 
    # // Constructing a kd-tree spatial map
    m = pareto.kd_tree() 
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
