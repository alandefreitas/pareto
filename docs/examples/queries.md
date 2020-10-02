---
layout: default
title: Queries
nav_order: 6
has_children: false
parent: Examples
has_toc: false
---
# Queries

All iterators can contain a list of predicates that limit iterators to query results. There are five types of predicates:

-   `intersects`: return only elements that intersect a given query box.
-   `within`: return only elements within a given query box. This is the same as `intersects` but it excludes the borders.
-   `disjoint`: return only elements that do not intersect a given query box.
-   `nearest`: return only the <img src="https://render.githubusercontent.com/render/math?math=k"> nearest elements to a reference point or query box.
-   `satisfies`: return only elements that pass a predicate provided by the user.

```python
for [point, value] in pf.find_intersection(pf.ideal(), pf.point([-1.21188, -1.24192])):
    print(point, '->', value)
for [point, value] in pf.find_within(pf.ideal(), pf.point([-1.21188, -1.24192])):
    print(point, '->', value)
for [point, value] in pf.find_disjoint(pf.worst(), pf.point([+0.71, +1.19])):
    print(point, '->', value)
for [point, value] in pf.find_nearest(pf.point([-1.21188, -1.24192]), 2):
    print(point, '->', value)
p = pf.get_nearest(pf.point([2.5, 2.5]))
print(p[0], '->', p[1])
```

```cpp
for (auto it = pf.find_intersection(pf.ideal(), {-1.21188, -1.24192}); it != pf.end(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
for (auto it = pf.find_within(pf.ideal(), {-1.21188, -1.24192}); it != pf.end(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
for (auto it = pf.find_disjoint(pf.worst(), {+0.71, +1.19}); it != pf.end(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
for (auto it = pf.find_nearest({-1.21188, -1.24192}, 2); it != pf.end(); ++it) {
    cout << it->first << " -> " << it->second << endl;
}
auto it = pf.find_nearest({2.5, 2.5});
cout << it->first << " -> " << it->second << endl;
```




<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
