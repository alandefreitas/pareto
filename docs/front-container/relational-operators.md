---
layout: default
title: Relational Operators
nav_order: 13
has_children: false
parent: Front Container
has_toc: false
---
# Relational Operators

These are non-member functions.

| Method                                                       |
| ------------------------------------------------------------ |
| **Multimap**                                                 |
| Compares the values in the multimap                          |
| `template <class K, size_t M, class T, class C, class A> bool operator==(const front<K, M, T, C, A> &lhs, const front<K, M, T, C, A> &rhs);` |
| `template <class K, size_t M, class T, class C, class A> bool operator!=(const front<K, M, T, C, A> &lhs, const front<K, M, T, C, A> &rhs);` |
| **FrontContainer**                                          |
| Front-Front Comparison                                       |
| `template <typename K, size_t M, typename T, typename C> bool operator<(const front<K, M, T, C> &lhs, const front<K, M, T, C> &rhs);` |
| `template <typename K, size_t M, typename T, typename C> bool operator>(const front<K, M, T, C> &lhs, const front<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator<=(const front<K, M, T, C> &lhs, const front<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>=(const front<K, M, T, C> &lhs, const front<K, M, T, C> &rhs)` |
| Front-Point Comparison                                       |
| `template <typename K, size_t M, typename T, typename C> bool operator<(const front<K, M, T, C> &lhs, const typename front<K, M, T, C>::key_type &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>(const front<K, M, T, C> &lhs, const typename front<K, M, T, C>::key_type &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator<=(const front<K, M, T, C> &lhs, const typename front<K, M, T, C>::key_type &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>=(const front<K, M, T, C> &lhs, const typename front<K, M, T, C>::key_type &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator<(const typename front<K, M, T, C>::key_type &lhs, const front<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>(const typename front<K, M, T, C>::key_type &lhs, const front<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator<=(const typename front<K, M, T, C>::key_type &lhs, const front<K, M, T, C> &rhs)` |
| `template <typename K, size_t M, typename T, typename C> bool operator>=(const typename front<K, M, T, C>::key_type &lhs, const front<K, M, T, C> &rhs)` |

**Parameters**

* `lhs`, `rhs` - `front`s or `key_type`s whose contents to compare

**Return value**

* `operator==`, `operator!=`: `true` if the **internal** contents of the `front`s are equal, false otherwise. 
* `operator<`, `operator>`, `operator<=`, `operator>=`: true if front `lhs` (or a front containing only `lhs` as a point) dominates `rhs`

**Complexity**

* `operator==`, `operator!=`: $O(mn)$
* `operator<`, `operator>`, `operator<=`, `operator>=`: $O(m n \log n)$ for fronts and $O(m \log n)$ for points

$$
O(1)
$$

**Notes**

In addition to the equality and inequality operators defined for spatial containers, the front contains includes relational operators.

In the context of fronts, `operator<` return true if the front `lhs` dominates the front `rhs`. When one of these parameters is a point, we treat this point as if it were front with a single point.  

!!! info
    Although these operators could be defined in other ways, the operators `operator<`,  `operator>`,  `operator<=`,  `operator>=` as defined here are are later useful for `pareto::archive` containers, which need to sort fronts by their dominance relationships.

**Example**

=== "C++"

    ```cpp
    front<double, 3, unsigned> pf3(pf);
    if (pf == pf3) {
        std::cout << "The fronts have the same elements" << std::endl;
    } else {
        if (pf.size() != pf3.size()) {
            std::cout << "The fronts do not have the same elements" << std::endl;
        } else {
            std::cout << "The fronts might not have the same elements"
                      << std::endl;
        }
    }

    front<double, 3, unsigned> pf4(pf.begin(), pf.end());
    if (pf == pf4) {
        std::cout << "The fronts have the same elements" << std::endl;
    } else {
        if (pf.size() != pf4.size()) {
            std::cout << "The fronts do not have the same elements" << std::endl;
        } else {
            std::cout << "The fronts might not have the same elements"
                      << std::endl;
        }
    }

    if (pf_star < pf) {
        std::cout << "pf* dominates pf" << std::endl;
    } else {
        std::cout << "pf* does not dominate pf" << std::endl;
    }
    ```

=== "Python"

    ```python
    pf3 = pareto.front(pf)
    if pf == pf3:
        print('The containers have the same elements')
    else:
        if len(pf) != len(pf3):
            print('The containers do not have the same elements')
        else:
            print('The containers might not have the same elements')
            # You need a for loop after here to make sure
    
    pf4 = pareto.front()
    for [k, v] in pf:
        pf4[k] = v
    
    if pf == pf4:
        print('The containers have the same elements')
    else:
        if len(pf) != len(pf4):
            print('The containers do not have the same elements')
        else:
            print('The containers might not have the same elements')
            # You need a for loop after here to make sure
    
    if pf_star < pf:
        print('pf* dominates pf')
    else:
        print('pf* does not dominate pf')
    
    ```

=== "Output"

    ```console
    The fronts have the same elements
    The fronts do not have the same elements
    pf* dominates pf
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
