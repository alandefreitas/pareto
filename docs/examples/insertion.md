---
layout: default
title: Insertion
nav_order: 2
has_children: false
parent: Examples
has_toc: false
---
# Insertion

The insertion and removal algorithms for all data structures can be accessed through overloaded operators or the usual `insert`/`erase` functions. These functions return an iterator to the new element.

```python
pf[-2.01773, -1.25209] = 27
pf[-2.31613, -0.219302] = 8
pf[1.51927, -0.923787] = 12
pf[-0.283076, -1.58758] = 2
del pf[-2.31613, -0.219302]
pf.clear()
```

```cpp
pf(-2.01773, -1.25209) = 27;
pf(-2.31613, -0.219302) = 8;
pf(1.51927, -0.923787) = 12;
pf(-0.283076, -1.58758) = 2;
pf.erase({-2.31613, -0.219302});
pf.clear();
```

We use `operator()` for C++ because the `operator[]` does not allow more than one element. We can use `operator[front::point_type]` though. The insertion operator will already remove any points that are worse than the new point in all dimensions.





<!-- Generated with mdsplit: https://github.com/alandefreitas/mdsplit -->
