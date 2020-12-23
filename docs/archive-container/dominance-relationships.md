---
layout: default
title: Dominance Relationships
nav_order: 8
has_children: false
parent: Archive Container
has_toc: false
---
# Dominance Relationships

| Method                                                     |
| ---------------------------------------------------------- |
| **FrontContainer**                                        |
| Archive-Point Dominance                                      |
| `bool dominates(const key_type &p)`                        |
| `bool strongly_dominates(const key_type &p) const`         |
| `bool is_partially_dominated_by(const key_type &p) const`  |
| `bool is_completely_dominated_by(const key_type &p) const` |
| `bool non_dominates(const key_type &p) const`              |
| Archive-Front Dominance                                      |
| `bool dominates(const front &P) const`                     |
| `bool strongly_dominates(const front &P) const`            |
| `bool is_partially_dominated_by(const front &P) const`     |
| `bool is_completely_dominated_by(const front &P) const`    |
| `bool non_dominates(const front &P) const`                 |
| Archive-Archive Dominance                                      |
| `bool dominates(const archive &A) const`                     |
| `bool strongly_dominates(const archive &A) const`            |
| `bool is_partially_dominated_by(const archive &A) const`     |
| `bool is_completely_dominated_by(const archive &A) const`    |
| `bool non_dominates(const archive &A) const`                 |

**Parameters**

* `p` - point we are checking for dominance
* `P` - front we are checking for dominance
* `A` - archive we are checking for dominance

**Return value**

* `bool`- `true` if and only if the point `p` (or front `P`, or archive `A`) is dominated, is strongly dominated, partially dominates, completely dominates, or non-dominantes `*this`

**Complexity**

* `is_completely_dominated_by`:  $O(1)$ for points and $O(n)$ for fronts
* All others:  $O(m \log n)$ for points and $O(m n \log n)$ for fronts

**Notes**

The dominance between archives is defined in terms of the first front in the archive.

!!! note
    See the section fronts / dominance relationships for more details 

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    // Point-point dominance
    using point_type = archive<double, 3, unsigned>::key_type;
    point_type p1({0, 0, 0});
    point_type p2({1, 1, 1});
    std::vector<bool> is_minimization = {true, false, true};
    std::cout << (p1.dominates(p2, is_minimization) ? "p1 dominates p2" : "p1 does not dominate p2") << std::endl;
    std::cout << (p1.strongly_dominates(p2, is_minimization) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << std::endl;
    std::cout << (p1.non_dominates(p2, is_minimization) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << std::endl;
    
    // Archive-point dominance
    std::cout << (ar.dominates(p2) ? "ar dominates p2" : "ar does not dominate p2") << std::endl;
    std::cout << (ar.strongly_dominates(p2) ? "ar strongly dominates p2" : "ar does not strongly dominate p2") << std::endl;
    std::cout << (ar.non_dominates(p2) ? "ar non-dominates p2" : "ar does not non-dominate p2") << std::endl;
    std::cout << (ar.is_partially_dominated_by(p2) ? "ar is partially dominated by p2" : "ar is not is partially dominated by p2") << std::endl;
    std::cout << (ar.is_completely_dominated_by(p2) ? "ar is completely dominated by p2" : "ar is not is completely dominated by p2") << std::endl;
    
    // Archive-archive dominance
    archive<double, 3, unsigned> ar2({min, max, min});
    for (const auto& [p,v]: ar) {
        ar2[point_type({p[0] - 1, p[1] + 1, p[2] - 1})] = v;
    }
    std::cout << (ar.dominates(ar2) ? "ar dominates ar2" : "ar does not dominate ar2") << std::endl;
    std::cout << (ar.strongly_dominates(ar2) ? "ar strongly dominates ar2" : "ar does not strongly dominate ar2") << std::endl;
    std::cout << (ar.non_dominates(ar2) ? "ar non-dominates ar2" : "ar does not non-dominate ar2") << std::endl;
    std::cout << (ar.is_partially_dominated_by(ar2) ? "ar is partially dominated by ar2" : "ar is not is partially dominated by ar2") << std::endl;
    std::cout << (ar.is_completely_dominated_by(ar2) ? "ar is completely dominated by ar2" : "ar is not is completely dominated by ar2") << std::endl;
    ```

=== "Python"

    ```python
    # Point-point dominance
    p1 = pareto.point([0, 0, 0])
    p2 = pareto.point([1, 1, 1])
    is_minimization = [True, False, True]
    print('p1 dominates p2' if p1.dominates(p2, is_minimization) else 'p1 does not dominate p2')
    print('p1 strongly dominates p2' if p1.strongly_dominates(p2, is_minimization) else 'p1 does not strongly dominate p2')
    print('p1 non-dominates p2' if p1.non_dominates(p2, is_minimization) else 'p1 does not non-dominate p2')
    
    # Archive-point dominance
    print('ar dominates p2' if ar.dominates(p2) else 'ar does not dominate p2')
    print('ar strongly dominates p2' if ar.strongly_dominates(p2) else 'ar does not strongly dominate p2')
    print('ar non-dominates p2' if ar.non_dominates(p2) else 'ar does not non-dominate p2')
    print('ar is partially dominated by p2' if ar.is_partially_dominated_by(p2) else 'ar is not is partially dominated by p2')
    print('ar is completely dominated by p2' if ar.is_completely_dominated_by(p2) else 'ar is not is completely dominated by p2')
    
    # Archive-archive dominance
    ar2 = pareto.archive(['min', 'max', 'min'])
    for [p, v] in ar:
        ar2[pareto.point([p[0] - 1, p[1] + 1, p[2] - 1])] = v
    
    print('ar dominates ar2' if ar.dominates(ar2) else 'ar does not dominate ar2')
    print('ar strongly dominates ar2' if ar.strongly_dominates(ar2) else 'ar does not strongly dominate ar2')
    print('ar non-dominates ar2' if ar.non_dominates(ar2) else 'ar does not non-dominate ar2')
    print('ar is partially dominated by ar2' if ar.is_partially_dominated_by(ar2) else 'ar is not is partially dominated by ar2')
    print('ar is completely dominated by ar2' if ar.is_completely_dominated_by(ar2) else 'ar is not is completely dominated by ar2')
    ```

=== "Output"

    ```console
    p1 does not dominate p2
    p1 does not strongly dominate p2
    p1 non-dominates p2
    ar dominates p2
    ar strongly dominates p2
    ar does not non-dominate p2
    ar is not is partially dominated by p2
    ar is not is completely dominated by p2
    ar does not dominate ar2
    ar does not strongly dominate ar2
    ar does not non-dominate ar2
    ar is partially dominated by ar2
    ar is completely dominated by ar2
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
