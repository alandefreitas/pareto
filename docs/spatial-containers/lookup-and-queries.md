---
layout: default
title: Lookup and Queries
nav_order: 9
has_children: false
parent: Spatial Containers
has_toc: false
---
# Lookup and Queries

| Method                                                       |
| ------------------------------------------------------------ |
| **Multimap**                                                 |
| Returns the number of elements matching specific key         |
| `size_type count(const key_type &p) const;`                  |
| `template <class L> size_type count(const L &p) const`       |
| Finds element with specific key                              |
| `iterator find(const key_type &p);`                          |
| `const_iterator find(const key_type &p) const;`              |
| `template <class L> iterator find(const L &p)`               |
| `template <class L> const_iterator find(const L &p) const;`  |
| Checks if the container contains element with specific key   |
| `bool contains(const key_type &p) const;`                    |
| `template <class L> bool contains(const L &p) const;`        |
| **SpatialContainer**                                         |
| Get iterator to first element that passes the predicates     |
| `const_iterator find(const predicate_list_type &ps) const noexcept;` |
| `iterator find(const predicate_list_type &ps) noexcept;`     |
| Find intersection between point and container                |
| `iterator find_intersection(const key_type &p);`           |
| `const_iterator find_intersection(const key_type &p) const;` |
| Find intersection between container and query box            |
| `iterator find_intersection(const key_type &lb, const key_type &ub);` |
| `const_iterator find_intersection(const key_type &lb, const key_type &ub) const;` |
| Find points inside a query box (excluding borders)           |
| `iterator find_within(const key_type &lb, const key_type &ub);` |
| `const_iterator find_within(const key_type &lb, const key_type &ub) const` |
| Find points outside a query box                              |
| `iterator find_disjoint(const key_type &lb, const key_type &ub);` |
| `const_iterator find_disjoint(const key_type &lb, const key_type &ub) const;` |
| Find the elements closest to a point                         |
| `iterator find_nearest(const key_type &p);`                |
| `const_iterator find_nearest(const key_type &p) const;`    |
| `iterator find_nearest(const key_type &p, size_t k);`      |
| `const_iterator find_nearest(const key_type &p, size_t k) const;` |
| `iterator find_nearest(const box_type &b, size_t k);`        |
| `const_iterator find_nearest(const box_type &b, size_t k) const;` |
| Find min/max elements                                        |
| `iterator max_element(size_t dimension)`                     |
| `const_iterator max_element(size_t dimension) const`         |
| `iterator min_element(size_t dimension)`                     |
| `const_iterator min_element(size_t dimension) const`         |

**Parameters**

* `ps` - a list of predicates
* `p` - a point of type `key_value` or convertible to `key_value`
* `lb` and `ub` - lower and upper bounds of the query box
* `k` - number of nearest elements

**Return value**

* `count()`: `size_type`: number of elements with a given key
* `container()`: `bool`: `true` if and only if the container contains an element with the given key `p`
* `find_*`: `iterator` and `const_iterator` - Iterator to the first element that passes the query predicates
  * `find` returns a normal iterator
  * all other `find_*` functions return a query iterator (see below)
* `size_type` - Number of elements erased

**Complexity**

$$
O(m \log n)
$$

**Notes**

**Query iterators** might store a list of predicates that limit iterators to query results. A query iterator skips all elements that do not match its predicates.

There are five types of predicates:

|  Predicate type           |   Description     |
|---------------------------|-------------------|
| `intersects`            |   return only elements that intersect a given query box.   |
| `within`            |   return only elements within a given query box. This is the same as `intersects` but it excludes the borders.   |
| `disjoint`            |   return only elements that do not intersect a given query box.   |
| `nearest`            |   return only the $k$ nearest elements to a reference point or query box.   |
| `satisfies`            |   return only elements that pass a predicate provided by the user.   |

!!! info "Predicate lists"
    Query iterators contain an element of type `pareto::predicate_list`. 
    When a `predicate_list` is being constructed, it will:
    1) compress to predicates to eliminate any redundancy in the search requirements, and 
    2) sort the predicates by how restrictive they are so that the search for the next element is as efficient as possible.

!!! warning "Comparing Iterators"
    Although a normal iterator and a query iterator that point to the same element compare equal, this does not mean their `operator++` will return the same element. The past-the-end element of all query iterators is also the `end()` iterator.

!!! warning "Lower and Upper bounds"
    Because of how spatial container work, we do not guarantee equivalent elements are necessarily stored in sequence. Thus, unlike `std::multimap` there are no `equal_range`, `lower_bound` and `upper_bound` functions. The same behaviour must be achieved with the `find_intersection` function.

**Examples**

Continuing from the previous example:

=== "C++"

    ```cpp
    for (auto it = m.find_intersection({-10,-10,-10}, {-2.3912, 0.395611, 2.78224}); it != m.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    for (auto it = m.find_within({-10,-10,-10}, {-2.3912, 0.395611, 2.78224}); it != m.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    for (auto it = m.find_disjoint({-10,-10,-10}, {+0.71, +1.19, +0.98}); it != m.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    for (auto it = m.find_nearest({-2.3912, 0.395611, 2.78224}, 2); it != m.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    auto it = m.find_nearest({2.5, 2.5, 2.5});
    std::cout << it->first << " -> " << it->second << std::endl;
    ```

=== "Python"

    ```python
    for [point, value] in m.find_intersection(pareto.point([-10, -10, -10]), pareto.point([-1.21188, -1.24192, +10])):
        print(point, '->', value)

    for [point, value] in m.find_within(pareto.point([-10, -10, -10]), pareto.point([-1.21188, -1.24192, +10])):
        print(point, '->', value)
    
    for [point, value] in m.find_disjoint(pareto.point([+0.2, +0.19, -1]), pareto.point([+0.71, +1.19, +10])):
        print(point, '->', value)
    
    for [point, value] in m.find_nearest(pareto.point([-1.21188, -1.24192, 10]), 2):
        print(point, '->', value)
    
    for [point, value] in m.find_nearest(pareto.point([2.5, 2.5, 10])):
        print(point, '->', value)
    ```

=== "Output"

    ```console
    [-2.57664, -1.52034, 0.600798] -> 17
    [-2.55905, -0.271349, 0.898137] -> 6
    [-2.3912, 0.395611, 2.78224] -> 11
    [-2.57664, -1.52034, 0.600798] -> 17
    [-2.55905, -0.271349, 0.898137] -> 6
    [-2.3912, 0.395611, 2.78224] -> 11
    [-0.639149, 1.89515, 0.858653] -> 10
    [-0.401531, 2.30172, 0.58125] -> 39
    [-1.09756, 1.33135, 0.569513] -> 20
    [-1.45049, 1.35763, 0.606019] -> 17
    [-0.00292544, 1.29632, -0.578346] -> 20
    [0.0728106, 1.91877, 0.399664] -> 25
    [0.152711, 1.99514, -0.112665] -> 13
    [0.157424, 2.30954, -1.23614] -> 6
    [-2.3912, 0.395611, 2.78224] -> 11
    [-2.55905, -0.271349, 0.898137] -> 6
    [0.0728106, 1.91877, 0.399664] -> 25
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
