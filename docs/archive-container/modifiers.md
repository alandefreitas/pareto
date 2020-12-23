---
layout: default
title: Modifiers
nav_order: 10
has_children: false
parent: Archive Container
has_toc: false
---
# Modifiers

| Method                                                       |
| ------------------------------------------------------------ |
| **Container** + **AllocatorAwareContainer**                  |
| Exchanges the contents of the container with those of `rhs`  |
| `void swap(kd_tree &rhs) noexcept;`                          |
| **Multimap**                                                 |
| Erases all elements from the container                       |
| `void clear();`                                              |
| Inserts element(s) into the container                        |
| `iterator insert(const value_type &v);`                      |
| `iterator insert(value_type &&v);`                           |
| `template <class P> iterator insert(P &&v);`                 |
| `iterator insert(iterator, const value_type &v);`            |
| `iterator insert(const_iterator, const value_type &v);`      |
| `iterator insert(const_iterator, value_type &&v);`           |
| `template <class P> iterator insert(const_iterator hint, P &&v);` |
| `template <class Inputiterator> void insert(Inputiterator first, Inputiterator last);` |
| `void insert(std::initializer_list<value_type> init);`       |
| Inserts a new element into the container constructed in-place with the given `args` |
| `template <class... Args> iterator emplace(Args &&...args);` |
| `template <class... Args> iterator emplace_hint(const_iterator, Args &&...args);` |
| Removes specified elements from the container                |
| `iterator erase(const_iterator position);`                   |
| `iterator erase(iterator position);`                         |
| `iterator erase(const_iterator first, const_iterator last);` |
| `size_type erase(const key_type &k);`                        |
| Attempts to extract ("splice") each element in `source` and insert it into `*this` |
| `void merge(archive &source) noexcept;`                      |
| `void merge(archive &&source) noexcept;`                      |
| **ArchiveContainer**                                                 |
| `void merge(front_type &source) noexcept;`                      |
| `void merge(front_type &&source) noexcept;`                      |
| `void resize(size_t new_size);`                      |

**Parameters**

* `rhs` - container to exchange the contents with
* `v` - element value to insert
* `first`, `last` - range of elements to insert/erase
* `init` - initializer list to insert the values from
* `hint` - iterator, used as a suggestion as to where to start the search
* `position` - iterator pointer to element to erase
* `k` - key value of the elements to remove
* `source` - container to get elements from
* `new_size` - new capacity of the archive

**Return value**

* `iterator` - Iterator to the new element (`insert`) or following the last removed element (`erase`)
* `size_type` - Number of elements erased

**Complexity**

* `insert`, `emplace`,  `erase`: $O(m \log n)$
* `swap`: $O(1)$
* `merge`: $O(mn)$

**Notes**

Manipulating archives does not have the same side effects as manipulating fronts:

1) The insertion operator will move any points that are worse than the new point to higher fronts. 
2) The removal operator will bring any previously dominated elements closer to the best fronts.

When `resize(size_t new_size)` is called with a new size smaller than the current number of elements in the archive, the archive if pruned. The pruning algorithm will remove the last front in the archive until the new size is achieved. If the last front has more elements that we need to remove, up to $2 * \log_2 capacity$ elements are removed by their crowding distances and other elements are removed randomly.  

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    ar.insert({ {1.49101, 3.24052, 0.724771}, 24});
    ar.erase({1.49101, 3.24052, 0.724771});
    ```

=== "Python"

    ```python
    ar.insert([pareto.point([1.49101, 3.24052, 0.724771]), 24])
    del ar[1.49101, 3.24052, 0.724771]
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
