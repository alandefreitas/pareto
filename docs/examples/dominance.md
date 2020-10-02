---
layout: default
title: Dominance
nav_order: 6
has_children: false
parent: Examples
has_toc: false
---
# Dominance

A solution `x1` (weakly) dominates `x2` if it `x1` is 1) better than `x2` in at least one dimension and 2) not worse than `x2` in any dimension:

![Point/point dominance](../img/point_dominance.svg)

We can check the dominance relationships between points with:

```python
p1 = pf.point([0, 0])
p2 = pf.point([1, 1])
print('p1 dominates p2' if p1.dominates(p2) else 'p1 does not dominate p2')
print('p1 strongly dominates p2' if p1.strongly_dominates(p2) else 'p1 does not strongly dominate p2')
print('p1 non-dominates p2' if p1.non_dominates(p2) else 'p1 does not non-dominate p2')
```

```cpp
front<double, 2, unsigned>::point_type p1({0, 0});
front<double, 2, unsigned>::point_type p2({1, 1});
cout << (p1.dominates(p2) ? "p1 dominates p2" : "p1 does not dominate p2") << endl;
cout << (p1.strongly_dominates(p2) ? "p1 strongly dominates p2" : "p1 does not strongly dominate p2") << endl;
cout << (p1.non_dominates(p2) ? "p1 non-dominates p2" : "p1 does not non-dominate p2") << endl;
```

The `point` function (Python) and the `point_type` alias (C++) are shortcuts to obtain the appropriate point type for a front (remember the front dimensions are defined at compile-time).

We can also check for dominance between fronts and points:

![Front/point dominance](../img/pareto_dominance.svg)

```python
print('pf dominates p2' if pf.dominates(p2) else 'pf does not dominate p2')
print('pf strongly dominates p2' if pf.strongly_dominates(p2) else 'pf does not strongly dominate p2')
print('pf non-dominates p2' if pf.non_dominates(p2) else 'pf does not non-dominate p2')
print('pf is partially dominated by p2' if pf.is_partially_dominated_by(p2) else 'pf is not is partially dominated by p2')
print('pf is completely dominated by p2' if pf.is_completely_dominated_by(p2) else 'pf is not is completely dominated by p2')
```

```cpp
cout << (pf.dominates(p2) ? "pf dominates p2" : "pf does not dominate p2") << endl;
cout << (pf.strongly_dominates(p2) ? "pf strongly dominates p2" : "pf does not strongly dominate p2") << endl;
cout << (pf.non_dominates(p2) ? "pf non-dominates p2" : "pf does not non-dominate p2") << endl;
cout << (pf.is_partially_dominated_by(p2) ? "pf is partially dominated by p2" : "pf is not is partially dominated by p2") << endl;
cout << (pf.is_completely_dominated_by(p2) ? "pf is completely dominated by p2" : "pf is not is completely dominated by p2") << endl;
```

At last, we can check the dominance relationship between fronts. `P1` dominates `P2` if `P1` dominates all points in `P2`.

```python
pf2 = pyfront.front(2)
for [p,v] in pf:
    pf2[p-1] = v
print('pf dominates pf2' if pf.dominates(pf2) else 'pf does not dominate pf2')
print('pf strongly dominates pf2' if pf.strongly_dominates(pf2) else 'pf does not strongly dominate pf2')
print('pf non-dominates pf2' if pf.non_dominates(pf2) else 'pf does not non-dominate pf2')
print('pf is partially dominated by pf2' if pf.is_partially_dominated_by(pf2) else 'pf is not is partially dominated by pf2')
print('pf is completely dominated by pf2' if pf.is_completely_dominated_by(pf2) else 'pf is not is completely dominated by pf2')
```

```cpp
front<double, 2, unsigned> pf2;
for (const auto& [p,v]: pf) {
    pf2[p - 1] = v;
}
cout << (pf.dominates(pf2) ? "pf dominates pf2" : "pf does not dominate pf2") << endl;
cout << (pf.strongly_dominates(pf2) ? "pf strongly dominates pf2" : "pf does not strongly dominate pf2") << endl;
cout << (pf.non_dominates(pf2) ? "pf non-dominates pf2" : "pf does not non-dominate pf2") << endl;
cout << (pf.is_partially_dominated_by(pf2) ? "pf is partially dominated by pf2" : "pf is not is partially dominated by pf2") << endl;
cout << (pf.is_completely_dominated_by(pf2) ? "pf is completely dominated by pf2" : "pf is not is completely dominated by pf2") << endl;
```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
