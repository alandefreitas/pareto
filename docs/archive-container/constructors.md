---
layout: default
title: Constructors
nav_order: 3
has_children: false
parent: Archive Container
has_toc: false
---
# Constructors

| Method                                                       |
| ------------------------------------------------------------ |
| **Container** + **AllocatorAwareContainer** Constructors     |
| `explicit archive(const allocator_type &alloc = allocator_type())` |
| `archive(const archive &rhs)`                                    |
| `archive(const archive &rhs, const allocator_type &alloc)`       |
| `archive(archive &&rhs) noexcept`                                |
| `archive(archive &&rhs, const allocator_type &alloc) noexcept`   |
| **AssociativeContainer** + **AllocatorAwareContainer** Constructors |
| `explicit archive(const C &comp, const allocator_type &alloc = allocator_type())` |
| `template <class InputIt> archive(InputIt first, InputIt last, const C &comp = C(), const allocator_type &alloc = allocator_type())` |
| `archive(std::initializer_list<value_type> il, const C &comp = C(), const allocator_type &alloc = allocator_type())` |
| `template <class InputIt> archive(InputIt first, InputIt last, const allocator_type &alloc)` |
| `archive(std::initializer_list<value_type> il, const allocator_type &alloc)` |
| **FrontContainer** Constructors                                        |
| `template <class InputIt, class DirectionIt> archive(InputIt first, InputIt last, DirectionIt first_dir, DirectionIt last_dir,       const dimension_compare &comp = dimension_compare(), const allocator_type &alloc = allocator_type())` |
| `template <class DirectionIt> archive(std::initializer_list<value_type> il, DirectionIt first_dir, DirectionIt last_dir, const dimension_compare &comp = dimension_compare(), const allocator_type &alloc = construct_allocator<allocator_type>())` |
| `template <class InputIt> archive(InputIt first, InputIt last, std::initializer_list<bool> il_dir, const dimension_compare &comp = dimension_compare(), const allocator_type &alloc = construct_allocator<allocator_type>())` |
| `archive(std::initializer_list<value_type> il, std::initializer_list<bool> il_dir, const dimension_compare &comp = dimension_compare(),       const allocator_type &alloc = construct_allocator<allocator_type>())` |
| `archive(std::initializer_list<bool> il_dir, const dimension_compare &comp = dimension_compare(), const allocator_type &alloc = construct_allocator<allocator_type>())` |
| `template <class InputIt, class DirectionIt> archive(InputIt first, InputIt last, DirectionIt first_dir, DirectionIt last_dir, const allocator_type &alloc)` |
| `template <class DirectionIt> archive(std::initializer_list<value_type> il, DirectionIt first_dir, DirectionIt last_dir, const allocator_type &alloc)` |
| `template <class InputIt> archive(InputIt first, InputIt last, std::initializer_list<bool> il_dir, const allocator_type &alloc)` |
| `archive(std::initializer_list<value_type> il, std::initializer_list<bool> il_dir, const allocator_type &alloc)` |
| `archive(std::initializer_list<bool> il_dir, const allocator_type &alloc)` |
| **ArchiveContainer** Constructors     |
| `explicit archive(size_type capacity, const allocator_type &alloc = allocator_type())` |
| `archive(size_type capacity, const archive &rhs)`                                    |
| `archive(size_type capacity, const archive &rhs, const allocator_type &alloc)`       |
| `archive(size_type capacity, archive &&rhs) noexcept`                                |
| `archive(size_type capacity, archive &&rhs, const allocator_type &alloc) noexcept`   |
| `explicit archive(size_type capacity, const C &comp, const allocator_type &alloc = allocator_type())` |
| `template <class InputIt> archive(size_type capacity, InputIt first, InputIt last, const C &comp = C(), const allocator_type &alloc = allocator_type())` |
| `archive(size_type capacity, std::initializer_list<value_type> il, const C &comp = C(), const allocator_type &alloc = allocator_type())` |
| `template <class InputIt> archive(size_type capacity, InputIt first, InputIt last, const allocator_type &alloc)` |
| `archive(size_type capacity, std::initializer_list<value_type> il, const allocator_type &alloc)` |
| `template <class InputIt, class DirectionIt> archive(size_type capacity, InputIt first, InputIt last, DirectionIt first_dir, DirectionIt last_dir,       const dimension_compare &comp = dimension_compare(), const allocator_type &alloc = allocator_type())` |
| `template <class DirectionIt> archive(size_type capacity, std::initializer_list<value_type> il, DirectionIt first_dir, DirectionIt last_dir, const dimension_compare &comp = dimension_compare(), const allocator_type &alloc = construct_allocator<allocator_type>())` |
| `template <class InputIt> archive(size_type capacity, InputIt first, InputIt last, std::initializer_list<bool> il_dir, const dimension_compare &comp = dimension_compare(), const allocator_type &alloc = construct_allocator<allocator_type>())` |
| `archive(size_type capacity, std::initializer_list<value_type> il, std::initializer_list<bool> il_dir, const dimension_compare &comp = dimension_compare(),       const allocator_type &alloc = construct_allocator<allocator_type>())` |
| `archive(size_type capacity, std::initializer_list<bool> il_dir, const dimension_compare &comp = dimension_compare(), const allocator_type &alloc = construct_allocator<allocator_type>())` |
| `template <class InputIt, class DirectionIt> archive(size_type capacity, InputIt first, InputIt last, DirectionIt first_dir, DirectionIt last_dir, const allocator_type &alloc)` |
| `template <class DirectionIt> archive(size_type capacity, std::initializer_list<value_type> il, DirectionIt first_dir, DirectionIt last_dir, const allocator_type &alloc)` |
| `template <class InputIt> archive(size_type capacity, InputIt first, InputIt last, std::initializer_list<bool> il_dir, const allocator_type &alloc)` |
| `archive(size_type capacity, std::initializer_list<value_type> il, std::initializer_list<bool> il_dir, const allocator_type &alloc)` |
| `archive(size_type capacity, std::initializer_list<bool> il_dir, const allocator_type &alloc)` |
| **AssociativeContainer** + **AllocatorAwareContainer** Assignment |
| `archive &operator=(const archive &rhs)`                         |
| `archive &operator=(archive &&rhs) noexcept`                     |
| **AssociativeContainer** Assignment                          |
| `archive &operator=(std::initializer_list<value_type> il) noexcept` |

**Parameters**

| Parameter               | Description                                                  |
| ----------------------- | ------------------------------------------------------------ |
| `alloc`                 | allocator to use for all memory allocations of this container |
| `comp`                  | comparison function object to use for all comparisons of keys |
| `first`, `last`         | the range to copy the elements from                          |
| `rhs`                   | another container to be used as source to initialize the elements of the container with |
| `il`                    | initializer list to initialize the elements of the container with |
| `first_dir`, `last_dir` | the range to copy the target directions from                 |
| `il_dir`                | initializer list to initialize the target directions of the container with |
| `capacity`              | maximum archive capacity |

**Requirements**

| Type requirements                                            |
| ------------------------------------------------------------ |
| -`InputIt` and `DirectionIt` must meet the requirements of [*LegacyInputIterator*](https://en.cppreference.com/w/cpp/named_req/InputIterator). |
| -`Compare` must meet the requirements of [*Compare*](https://en.cppreference.com/w/cpp/named_req/Compare). |
| -`Allocator` must meet the requirements of [*Allocator*](https://en.cppreference.com/w/cpp/named_req/Allocator). |

**Complexity**

| Method                              | Complexity                                         |
| ----------------------------------- | -------------------------------------------------- |
| Empty constructor                   | $O(1)$                                             |
| Copy constructor                    | $O(mn)$                                            |
| Move constructor                    | $O(1)$ if `get_allocator() == rhs.get_allocator()` |
| Construct from range, or assignment | $O(m n \log n)$                                    |

**Notes**

An archive is also an adapter and an extension of spatial containers. An archive contains a `std::set` of fronts ordered by their dominance relationships. Unlike in a front, whenever we insert an element in the archive, it moves all elements dominated by the new element to higher fronts.

The archive constructors overload all front constructors with an extra parameter for the archive capacity. If no maximum capacity for the archive is set, the capacity is set by default to $\min(50 \times 2^m, 100000)$. The exponential factor $2^m$ in this heuristic is meant to take the curse of dimensionality into account. 

The container makes sure the archive never has more elements than allowed by the `capacity` parameter. If the capacity
exceeds, the container will remove the element in the most crowded regions of the worst front in the archive.

**Example**

=== "C++"

    ```cpp
    #include <pareto/archive.h>
    #include <pareto/kd_tree.h>
    // ...
    // Constructing the default archive
    size_t capacity = 1000;
    archive<double, 3, unsigned> ar(capacity, {min, max, min});
    // Constructing a archive based on kd trees
    archive<double, 3, unsigned, kd_tree<double, 3, unsigned>> ar2(capacity, {min, max, min});
    ```

=== "Python"

    ```python
    import pareto
    # ...
    # Constructing the default archive
    capacity = 1000
    ar = pareto.archive(capacity, ['min', 'max', 'min']);
    # Constructing a archive based on kd trees
    ar2 = pareto.kd_archive(capacity, ['min', 'max', 'min']);
    ```

!!! tip
    If you need to plot these archives, [`examples/matplotpp_example.cpp`](https://github.com/alandefreitas/pareto/blob/master/examples/matplotpp_example.cpp) includes an example that uses [Matplot++](https://github.com/alandefreitas/matplotplusplus). In Python, you can use [Matplotlib](https://matplotlib.org).



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
