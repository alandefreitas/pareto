---
layout: default
title: Relational Operators
nav_order: 13
has_children: false
parent: Archive Container
has_toc: false
---
# Relational Operators

These are non-member functions.

| Method                                                       |
| ------------------------------------------------------------ |
| **Multimap**                                                 |
| Compares the values in the multimap                          |
| `template <class K, size_t M, class T, class C, class A> bool operator==(const archive<K, M, T, C, A> &lhs, const archive<K, M, T, C, A> &rhs);` |
| `template <class K, size_t M, class T, class C, class A> bool operator!=(const archive<K, M, T, C, A> &lhs, const archive<K, M, T, C, A> &rhs);` |
| **FrontContainer**                                          |
| Archive-Archive Comparison                                       |
| `template <typename K, size_t M, typename T, typename C> bool operator<(const archive<K, M, T, C> &lhs, const archive<K, M, T, C> &rhs);` |
| `template <typename K, size_t M, typename T, typename C> bool operator>(const archive<K, M, T, C> &lhs, const archive<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator<=(const archive<K, M, T, C> &lhs, const archive<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>=(const archive<K, M, T, C> &lhs, const archive<K, M, T, C> &rhs)` |
| Archive-Point Comparison                                       |
| `template <typename K, size_t M, typename T, typename C> bool operator<(const archive<K, M, T, C> &lhs, const typename archive<K, M, T, C>::key_type &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>(const archive<K, M, T, C> &lhs, const typename archive<K, M, T, C>::key_type &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator<=(const archive<K, M, T, C> &lhs, const typename archive<K, M, T, C>::key_type &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>=(const archive<K, M, T, C> &lhs, const typename archive<K, M, T, C>::key_type &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator<(const typename archive<K, M, T, C>::key_type &lhs, const archive<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>(const typename archive<K, M, T, C>::key_type &lhs, const archive<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator<=(const typename archive<K, M, T, C>::key_type &lhs, const archive<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>=(const typename archive<K, M, T, C>::key_type &lhs, const archive<K, M, T, C> &rhs)` |

**Complexity**

$$
O(mn)
$$

**Notes**

Same notes as **SpatialContainer**. 

**Example**

=== "C++"

    ```cpp
    archive<double, 3, unsigned> ar3(ar);
    if (ar == ar3) {
        std::cout << "The archives have the same elements" << std::endl;
    } else {
        if (ar.size() != ar3.size()) {
            std::cout << "The archives do not have the same elements" << std::endl;
        } else {
            std::cout << "The archives might not have the same elements"
                      << std::endl;
        }
    }

    archive<double, 3, unsigned> ar4(ar.begin(), ar.end());
    if (ar == ar4) {
        std::cout << "The archives have the same elements" << std::endl;
    } else {
        if (ar.size() != ar4.size()) {
            std::cout << "The archives do not have the same elements" << std::endl;
        } else {
            std::cout << "The archives might not have the same elements"
                      << std::endl;
        }
    }

    if (ar_star < ar) {
        std::cout << "ar* dominates ar" << std::endl;
    } else {
        std::cout << "ar* does not dominate ar" << std::endl;
    }
    ```

=== "Python"

    ```python
    ar3 = pareto.archive(ar)
    if ar == ar3:
        print('The containers have the same elements')
    else:
        if len(ar) != len(ar3):
            print('The containers do not have the same elements')
        else:
            print('The containers might not have the same elements')
            # You need a for loop after here to make sure
    
    ar4 = pareto.archive()
    for [k, v] in ar:
        ar4[k] = v
    
    if ar == ar4:
        print('The containers have the same elements')
    else:
        if len(ar) != len(ar4):
            print('The containers do not have the same elements')
        else:
            print('The containers might not have the same elements')
            # You need a for loop after here to make sure
    
    if ar_star < ar:
        print('ar* dominates ar')
    else:
        print('ar* does not dominate ar')
    
    ```

=== "Output"

    ```console
    The archives have the same elements
    The archives do not have the same elements
    ar* dominates ar
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
