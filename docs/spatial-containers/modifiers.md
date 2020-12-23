---
layout: default
title: Modifiers
nav_order: 8
has_children: false
parent: Spatial Containers
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
| `void merge(spatial_map &source) noexcept;`                      |
| `void merge(spatial_map &&source) noexcept;`                      |

**Parameters**

* `rhs` - container to exchange the contents with
* `v` - element value to insert
* `first`, `last` - range of elements to insert/erase
* `init` - initializer list to insert the values from
* `hint` - iterator, used as a suggestion as to where to start the search
* `position` - iterator pointer to element to erase
* `k` - key value of the elements to remove
* `source` - container to get elements from

**Return value**

* `iterator` - Iterator to the new element (`insert`) or following the last removed element (`erase`)
* `size_type` - Number of elements erased

**Complexity**

* `insert`, `emplace`,  `erase`: $O(m \log n)$
* `swap`: $O(1)$
* `merge`: $O(mn)$

**Notes**

The containers cannot take advantage of the hints yet.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    m.insert({ {1.49101, 3.24052, 0.724771}, 24});
    m.erase({1.49101, 3.24052, 0.724771});
    ```

=== "Python"

    ```python
    m.insert([pareto.point([1.49101, 3.24052, 0.724771]), 24])
    del m[1.49101, 3.24052, 0.724771]
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
