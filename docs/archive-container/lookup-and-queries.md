---
layout: default
title: Lookup and Queries
nav_order: 11
has_children: false
parent: Archive Container
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
| **FrontContainer**                                          |
| Find sets of dominated elements                              |
| `const_iterator find_dominated(const key_type &p) const`   |
| `iterator find_dominated(const key_type &p)`               |
| Find nearest point excluding $p$                             |
| `const_iterator find_nearest_exclusive(const key_type &p) const` |
| `iterator find_nearest_exclusive(const key_type &p)`       |
| Find extreme elements                                        |
| `const_iterator ideal_element(size_t d) const`               |
| `iterator ideal_element(size_t d)`                           |
| `const_iterator nadir_element(size_t d) const`               |
| `iterator nadir_element(size_t d)`                           |
| `const_iterator worst_element(size_t d) const`               |
| `iterator worst_element(size_t d)`                           |
| **ArchiveContainer**                                          |
| Find sets of dominated elements                              |
| `front_set_type::const_iterator find_front(const key_type &p) const`   |
| `front_set_type::iterator find_front(const key_type &p)`               |

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
* `front_set_type::const_iterator` - find first front not dominated by `p`

**Complexity**

Let $|A|$ denote the number of fronts in the archive:

* `find_front` - $O(\log |A|)$
* others - O(m |A| \log n)

Due to the curse of dimensionality, we usually expect that $|A| \ll n$, especially as $m$ grows.

**Notes**

The function `find_front` will look for the first front that does not dominate the element `p`. This is an important sub-component of the insertion algorithm.

!!! note
    All other definitions and requirements of a **FrontContainer** also apply here.

**Examples**

Continuing from the previous example:

=== "C++"

    ```cpp
    for (auto it = ar.find_intersection(ar.ideal(), {-2.3912, 0.395611, 2.78224}); it != ar.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    for (auto it = ar.find_within(ar.ideal(), {-2.3912, 0.395611, 2.78224}); it != ar.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    for (auto it = ar.find_disjoint(ar.worst(), {+0.71, +1.19, +0.98}); it != ar.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    for (auto it = ar.find_nearest({-2.3912, 0.395611, 2.78224}, 2); it != ar.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    auto it_near = ar.find_nearest({2.5, 2.5, 2.5});
    std::cout << it_near->first << " -> " << it_near->second << std::endl;
    for (auto it = ar.find_dominated({-10, +10, -10}); it != ar.end(); ++it) {
        std::cout << it->first << " -> " << it->second << std::endl;
    }
    for (size_t i = 0; i < ar.dimensions(); ++i) {
        std::cout << "Ideal element in dimension " << i << ": " << ar.ideal_element(i)->first << std::endl;
        std::cout << "Nadir element in dimension " << i << ": " << ar.nadir_element(i)->first << std::endl;
        std::cout << "Worst element in dimension " << i << ": " << ar.worst_element(i)->first << std::endl;
    }
    ```

=== "Python"

    ```python
    for [k, v] in ar.find_intersection(ar.ideal(), pareto.point([-2.3912, 0.395611, 2.78224])):
        print(k, "->", v)
    
    for [k, v] in ar.find_within(ar.ideal(), pareto.point([-2.3912, 0.395611, 2.78224])):
        print(k, "->", v)
    
    for [k, v] in ar.find_disjoint(ar.worst(), pareto.point([+0.71, +1.19, +0.98])):
        print(k, "->", v)
    
    for [k, v] in ar.find_nearest(pareto.point([-2.3912, 0.395611, 2.78224]), 2):
        print(k, "->", v)
    
    for [k, v] in ar.find_nearest(pareto.point([2.5, 2.5, 2.5])):
        print(k, "->", v)
    
    for [k, v] in ar.find_dominated(pareto.point([-10, +10, -10])):
        print(k, "->", v)
    
    for i in range(ar.dimensions()):
        print('Ideal element in dimension', i, ': ', ar.ideal_element(i)[0])
        print('Nadir element in dimension', i, ': ', ar.nadir_element(i)[0])
        print('Worst element in dimension', i, ': ', ar.worst_element(i)[0])
    ```

=== "Output"

    ```console
    [-2.3912, 0.395611, 2.78224] -> 11
    [-2.14255, -0.518684, -2.92346] -> 32
    [-1.63295, 0.912108, -2.12953] -> 36
    [-0.653036, 0.927688, -0.813932] -> 13
    [-0.508188, 0.871096, -2.25287] -> 32
    [0.453686, 1.02632, -2.24833] -> 30
    [0.693712, 1.12267, -1.37375] -> 12
    [-2.57664, -1.52034, 0.600798] -> 17
    [-2.55905, -0.271349, 0.898137] -> 6
    [-2.31613, -0.219302, 0] -> 8
    [-0.894115, 1.01387, 0.462008] -> 11
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
    [-2.14255, -0.518684, -2.92346] -> 32
    [-1.63295, 0.912108, -2.12953] -> 36
    [-0.653036, 0.927688, -0.813932] -> 13
    [-0.508188, 0.871096, -2.25287] -> 32
    [0.453686, 1.02632, -2.24833] -> 30
    [0.693712, 1.12267, -1.37375] -> 12
    [-2.57664, -1.52034, 0.600798] -> 17
    [-2.55905, -0.271349, 0.898137] -> 6
    [-2.31613, -0.219302, 0] -> 8
    [-0.894115, 1.01387, 0.462008] -> 11
    [-2.3912, 0.395611, 2.78224] -> 11
    [-0.639149, 1.89515, 0.858653] -> 10
    [-0.401531, 2.30172, 0.58125] -> 39
    [-1.09756, 1.33135, 0.569513] -> 20
    [-1.45049, 1.35763, 0.606019] -> 17
    [-0.00292544, 1.29632, -0.578346] -> 20
    [0.0728106, 1.91877, 0.399664] -> 25
    [0.152711, 1.99514, -0.112665] -> 13
    [0.157424, 2.30954, -1.23614] -> 6
    Ideal element in dimension 0: [-2.57664, -1.52034, 0.600798]
    Nadir element in dimension 0: [0.693712, 1.12267, -1.37375]
    Worst element in dimension 0: [0.693712, 1.12267, -1.37375]
    Ideal element in dimension 1: [0.157424, 2.30954, -1.23614]
    Nadir element in dimension 1: [-2.57664, -1.52034, 0.600798]
    Worst element in dimension 1: [-2.57664, -1.52034, 0.600798]
    Ideal element in dimension 2: [-2.14255, -0.518684, -2.92346]
    Nadir element in dimension 2: [-2.3912, 0.395611, 2.78224]
    Worst element in dimension 2: [-2.3912, 0.395611, 2.78224]
    ```





<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
