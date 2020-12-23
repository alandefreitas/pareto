---
layout: default
title: Dominance Relationships
nav_order: 8
has_children: false
parent: Front Container
has_toc: false
---
# Dominance Relationships

| Method                                                     |
| ---------------------------------------------------------- |
| **FrontContainer**                                        |
| Front-Point Dominance                                      |
| `bool dominates(const key_type &p)`                        |
| `bool strongly_dominates(const key_type &p) const`         |
| `bool is_partially_dominated_by(const key_type &p) const`  |
| `bool is_completely_dominated_by(const key_type &p) const` |
| `bool non_dominates(const key_type &p) const`              |
| Front-Front Dominance                                      |
| `bool dominates(const front &P) const`                     |
| `bool strongly_dominates(const front &P) const`            |
| `bool is_partially_dominated_by(const front &P) const`     |
| `bool is_completely_dominated_by(const front &P) const`    |
| `bool non_dominates(const front &P) const`                 |

**Parameters**

* `p` - point we are checking for dominance
* `P` - front we are checking for dominance

**Return value**

* `bool`- `true` if and only if the point `p` or front `P` is dominated, is strongly dominated, partially dominates, completely dominates, or non-dominates `*this`

**Complexity**

* `is_completely_dominated_by`:  $O(1)$ for points and $O(n)$ for fronts
* All others:  $O(m \log n)$ for points and $O(m n \log n)$ for fronts

**Notes**

A solution $x_1$ (weakly) dominates $x_2$  (denoted $x_1 \prec x_2$) if $x_1$ is 1) better than $x_2$ in at least one dimension and 2) not worse
than $x_2$ in any dimension:

!!! example "Point-point dominance"
    ![Point/point dominance](../img/point_dominance.svg)

    The `pareto::point` object contains function to check dominance between points without depending on the front. 

We can also check for dominance between fronts and points (denoted $p \prec P$ or $P \prec p$). This is a fundamental component of the insertion and removal algorithms.

!!! example "Front-point dominance"
    ![Front/point dominance](../img/pareto_dominance.svg)

!!! warning "Non-dominance"
    Saying $p$ non-dominates $P$ is different from saying $p$ does not dominate $P$. The first means $p$ and $P$ are incomparable, the second means $p$ and $P$ are either incomparable or $p$ dominates $P$.

At last, we can check dominance relationships between fronts. The dominance relationships between front $P_1$ and front $P_2$ (denoted $P_1 \prec P_2$) is defined when all points in $P_1$ are dominated by some point in $P_2$. This establishes an order relationship which is a chief component of the archive data structure.

**Example**

Continuing from the previous example:

=== "C++"

    ```cpp
    // Point-point dominance
    using point_type = front<double, 3, unsigned>::key_type;
    point_type p1({0, 0, 0});
    point_type p2({1, 1, 1});
    std::vector<bool> is_minimization = {true, false, true};
    std::cout << (p1.dominates(p2, is_minimization) ? "p1 dominates p2" : "p1 does not dominate p2") << std::endl;
    std::cout << (p1.strongly_dominates(p2, is_minimization) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << std::endl;
    std::cout << (p1.non_dominates(p2, is_minimization) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << std::endl;
    
    // Front-point dominance
    std::cout << (pf.dominates(p2) ? "pf dominates p2" : "pf does not dominate p2") << std::endl;
    std::cout << (pf.strongly_dominates(p2) ? "pf strongly dominates p2" : "pf does not strongly dominate p2") << std::endl;
    std::cout << (pf.non_dominates(p2) ? "pf non-dominates p2" : "pf does not non-dominate p2") << std::endl;
    std::cout << (pf.is_partially_dominated_by(p2) ? "pf is partially dominated by p2" : "pf is not is partially dominated by p2") << std::endl;
    std::cout << (pf.is_completely_dominated_by(p2) ? "pf is completely dominated by p2" : "pf is not is completely dominated by p2") << std::endl;
    
    // Front-front dominance
    front<double, 3, unsigned> pf2({min, max, min});
    for (const auto& [p,v]: pf) {
        pf2[point_type({p[0] - 1, p[1] + 1, p[2] - 1})] = v;
    }
    std::cout << (pf.dominates(pf2) ? "pf dominates pf2" : "pf does not dominate pf2") << std::endl;
    std::cout << (pf.strongly_dominates(pf2) ? "pf strongly dominates pf2" : "pf does not strongly dominate pf2") << std::endl;
    std::cout << (pf.non_dominates(pf2) ? "pf non-dominates pf2" : "pf does not non-dominate pf2") << std::endl;
    std::cout << (pf.is_partially_dominated_by(pf2) ? "pf is partially dominated by pf2" : "pf is not is partially dominated by pf2") << std::endl;
    std::cout << (pf.is_completely_dominated_by(pf2) ? "pf is completely dominated by pf2" : "pf is not is completely dominated by pf2") << std::endl;
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
    
    # Front-point dominance
    print('pf dominates p2' if pf.dominates(p2) else 'pf does not dominate p2')
    print('pf strongly dominates p2' if pf.strongly_dominates(p2) else 'pf does not strongly dominate p2')
    print('pf non-dominates p2' if pf.non_dominates(p2) else 'pf does not non-dominate p2')
    print('pf is partially dominated by p2' if pf.is_partially_dominated_by(p2) else 'pf is not is partially dominated by p2')
    print('pf is completely dominated by p2' if pf.is_completely_dominated_by(p2) else 'pf is not is completely dominated by p2')
    
    # Front-front dominance
    pf2 = pareto.front(['min', 'max', 'min'])
    for [p, v] in pf:
        pf2[pareto.point([p[0] - 1, p[1] + 1, p[2] - 1])] = v
    
    print('pf dominates pf2' if pf.dominates(pf2) else 'pf does not dominate pf2')
    print('pf strongly dominates pf2' if pf.strongly_dominates(pf2) else 'pf does not strongly dominate pf2')
    print('pf non-dominates pf2' if pf.non_dominates(pf2) else 'pf does not non-dominate pf2')
    print('pf is partially dominated by pf2' if pf.is_partially_dominated_by(pf2) else 'pf is not is partially dominated by pf2')
    print('pf is completely dominated by pf2' if pf.is_completely_dominated_by(pf2) else 'pf is not is completely dominated by pf2')
    ```

=== "Output"

    ```console
    p1 does not dominate p2
    p1 does not strongly dominate p2
    p1 non-dominates p2
    pf dominates p2
    pf strongly dominates p2
    pf does not non-dominate p2
    pf is not is partially dominated by p2
    pf is not is completely dominated by p2
    pf does not dominate pf2
    pf does not strongly dominate pf2
    pf does not non-dominate pf2
    pf is partially dominated by pf2
    pf is completely dominated by pf2
    ```



<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
