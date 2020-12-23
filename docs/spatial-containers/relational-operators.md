---
layout: default
title: Relational Operators
nav_order: 11
has_children: false
parent: Spatial Containers
has_toc: false
---
# Relational Operators

These are non-member functions.

| Method                                                       |
| ------------------------------------------------------------ |
| **Multimap**                                                 |
| Compares the values in the multimap                          |
| `template <class K, size_t M, class T, class C, class A> bool operator==(const spatial_map<K, M, T, C, A> &lhs, const spatial_map<K, M, T, C, A> &rhs);` |
| `template <class K, size_t M, class T, class C, class A> bool operator!=(const spatial_map<K, M, T, C, A> &lhs, const spatial_map<K, M, T, C, A> &rhs);` |

**Parameters**

* `lhs`, `rhs` - `spatial_map`s whose contents to compare

**Return value**

`true` if the **internal** contents of the `spatial_map`s are equal, false otherwise. 

**Complexity**

$$
O(n)
$$

**Notes**

!!! warning
    This operator tells us if the internal trees are equal and not if they contain the same elements. This is because the standard defines that this operation should take $O(n)$ time. Two trees might contain the same elements in different subtrees if their insertion order was different. 

    If you need to compare if *the elements* of `lhs` and `rhs` are the same, regardless of their internal representation, you have to iterate `lhs` and iteratively call `find` on the second container. This operation takes $O(m n \log n)$ time.

We do not include `operator<`,  `operator>`,  `operator<=`,  `operator>=` for spatial containers because [std::lexicographical_compare](https://en.cppreference.com/w/cpp/algorithm/lexicographical_compare) would be semantically meaningless in a multidimensional context where we need to return a value in $O(n)$ time and, by definition, there is no priority between key dimensions. 

**Example**

=== "C++"

    ```cpp
    spatial_map<double, 3, unsigned> m2(m);
    if (m == m2) {
        std::cout << "The containers have the same elements" << std::endl;
    } else {
        if (m.size() != m2.size()) {
            std::cout << "The containers do not have the same elements" << std::endl;
        } else {
            std::cout << "The containers might not have the same elements" << std::endl;
            // You need a for loop after here to make sure
        }
    }

    spatial_map<double, 3, unsigned> m3(m.begin(), m.end());
    if (m == m3) {
        std::cout << "The containers have the same elements" << std::endl;
    } else {
        if (m.size() != m3.size()) {
            std::cout << "The containers do not have the same elements" << std::endl;
        } else {
            std::cout << "The containers might not have the same elements" << std::endl;
            // You need a for loop after here to make sure
        }
    }
    ```

=== "Python"

    ```python
    m2 = pareto.spatial_map(m)
    if m == m2:
        print('The containers have the same elements')
    else:
        if len(m) != len(m2):
            print('The containers do not have the same elements')
        else:
            print('The containers might not have the same elements')
            # You need a for loop after here to make sure
    
    m3 = pareto.spatial_map()
    for [k, v] in m:
        m3[k] = v
    
    if m == m3:
        print('The containers have the same elements')
    else:
        if len(m) != len(m3):
            print('The containers do not have the same elements')
        else:
            print('The containers might not have the same elements')
            # You need a for loop after here to make sure
    ```

=== "Output"

    ```console
    The containers have the same elements
    The containers might not have the same elements
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
